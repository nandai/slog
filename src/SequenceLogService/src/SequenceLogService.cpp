/*
 * Copyright (C) 2011-2013 printf.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*!
 *  \file   SequenceLogService.cpp
 *  \brief  シーケンスログサービスクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "SequenceLogService.h"
#include "SequenceLogServiceMain.h"
#include "SharedFileContainer.h"

#include "slog/WebSocketClient.h"
#include "slog/Mutex.h"
#include "slog/FileInfo.h"
#include "slog/DateTimeFormat.h"
#include "slog/HttpRequest.h"

#if defined(__unix__)
    #define stricmp strcasecmp
    #include <syslog.h>
#endif

namespace slog
{

/*!
 *  \brief  シーケンスログアイテムリストクラス
 */
class ItemList
{
            SequenceLogItem     mHeader;            //!< シーケンスログリストヘッダー

public:     ItemList();

            SequenceLogItem* front() const;
            SequenceLogItem* back() const;
            bool empty() const;
            bool isEnd(SequenceLogItem* item) const;

            void clear();
            void push_back(SequenceLogItem* item);
            void pop_back();
            void merge(ItemList& list);
};

/*!
 *  \brief  コンストラクタ
 */
ItemList::ItemList()
{
    clear();
}

/*!
 *  \brief  先頭アイテム取得
 */
inline SequenceLogItem* ItemList::front() const
{
    if (empty())
        return NULL;

    return (SequenceLogItem*)mHeader.mNext;
}

/*!
 *  \brief  末尾アイテム取得
 */
inline SequenceLogItem* ItemList::back() const
{
    if (empty())
        return NULL;

    return (SequenceLogItem*)mHeader.mPrev;
}

/*!
 *  \brief  リストが空か調べる
 */
inline bool ItemList::empty() const
{
    return ((uint64_t)&mHeader == mHeader.mNext);
}

/*!
 *  \brief  リストの終端か調べる
 */
inline bool ItemList::isEnd(SequenceLogItem* item) const
{
    return (&mHeader == item);
}

/*!
 *  \brief  クリア
 */
inline void ItemList::clear()
{
    mHeader.mPrev = (uint64_t)&mHeader;
    mHeader.mNext = (uint64_t)&mHeader;
}

/*!
 *  \brief  末尾にアイテムを追加する
 */
inline void ItemList::push_back(SequenceLogItem* item)
{
    SequenceLogItem* header = (SequenceLogItem*)&mHeader;
    SequenceLogItem* footer = (SequenceLogItem*) mHeader.mPrev;

    footer->mNext = (uint64_t)item;
    item->  mPrev = (uint64_t)footer;
    item->  mNext = (uint64_t)header;
    header->mPrev = (uint64_t)item;
}

/*!
 *  \brief  末尾のアイテムを削除する
 */
inline void ItemList::pop_back()
{
    if (empty())
        return;

    SequenceLogItem* header =    (SequenceLogItem*)&mHeader;
    SequenceLogItem* footer =    (SequenceLogItem*) mHeader.mPrev;
    SequenceLogItem* newFooter = (SequenceLogItem*) footer->mPrev;

    newFooter->mNext = (uint64_t)header;
    header->   mPrev = (uint64_t)newFooter;
}

/*!
 *  \brief  マージ
 */
inline void ItemList::merge(ItemList& list)
{
    if (list.empty())
        return;

    SequenceLogItem* headerDst = (SequenceLogItem*)&mHeader;
    SequenceLogItem* footerDst = (SequenceLogItem*) mHeader.mPrev;

    SequenceLogItem* headerSrc = (SequenceLogItem*)list.mHeader.mNext;
    SequenceLogItem* footerSrc = (SequenceLogItem*)list.mHeader.mPrev;

    footerDst->mNext = (uint64_t)headerSrc;
    headerSrc->mPrev = (uint64_t)footerDst;
    footerSrc->mNext = (uint64_t)headerDst;
    headerDst->mPrev = (uint64_t)footerSrc;

    list.clear();
}

/*!
 *  \brief  シーケンスログアイテムキュークラス
 */
class ItemQueue
{
public:     long        mAlwaysCount;       //!< 常出力カウンタ（1以上の時はmOutputFlagがKEEPでも即出力する）
            ItemList    mList;              //!< シーケンスログアイテムリスト
            ItemList    mStepOutList;       //!< STEP_IN処理時にあらかじめ作成しておくSTEP_OUT用アイテムリスト

            ItemQueue()
            {
                mAlwaysCount = 0;
            }
};

/*!
 *  \brief  コンストラクタ
 */
SequenceLogService::SequenceLogService(HttpRequest* httpRequest) : WebServerResponseThread(httpRequest),
    mFileOutputBuffer(1024 * 2)
{
    mSHM = NULL;

    mOutputList =       NULL;
    mItemQueueManager = NULL;
    mStockItems =       NULL;

    mSharedFileContainer = NULL;
}

/*!
 *  \brief  デストラクタ
 */
SequenceLogService::~SequenceLogService()
{
}

/*!
 *  \brief  初期化
 */
bool SequenceLogService::init()
{
    bool result = true;

    if (upgradeWebSocket() == false)
        return false;

    try
    {
        Socket* socket = mHttpRequest->getSocket();
        ByteBuffer* buffer = WebSocket::recv(socket, NULL);

        // プロセスID取得
        uint32_t id = buffer->getInt();
        mProcess.setId(id);

        // シーケンスログファイル名取得
        int32_t len = buffer->getInt();
        String baseFileName(buffer->get(len), len);

        // バッファ削除
        delete buffer;

        // ファイル名チェック
//      if (baseFileName[len - 1] != '\0')
//      {
//          Exception e;
//          e.setMessage("SequenceLogService::init() / receive name is not null-terminated");
//
//          throw e;
//      }

        // 共有ファイルコンテナ取得
        SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
        mSharedFileContainer =  serviceMain->getSharedFileContainer(baseFileName);

//      openSeqLogFile(mFile);
        initBinaryOrText(&baseFileName);

        mOutputList =       new ItemList;
        mItemQueueManager = new ItemQueueManager;

        // ログバッファ（旧 - 共有メモリ）生成
        mSHM = new SLOG_SHM;
        mSHM->seq = 1;
    }
    catch (Exception e)
    {
        noticeLog("SequenceLogService: %s\n", e.getMessage());

        cleanUp();
        result = false;
    }

    return result;
}

/*
 *  \brief  シーケンスログファイル情報取得
 */
FileInfo* SequenceLogService::getFileInfo() const
{
    return mSharedFileContainer->getFileInfo();
}

/*
 *  \brief  シーケンスログサービススレッド
 */
void SequenceLogService::run()
{
    receiveMain();
    cleanUp();
}

/*!
 *  \brief  書き込みメイン
 */
void SequenceLogService::writeMain()
{
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    SequenceLogItem* item = mOutputList->front();

    // 共有ファイルコンテナをロック
    ScopedLock lock(mSharedFileContainer->getMutex());

    // シーケンスログ出力リストのログをすべて出力
    while (item && mOutputList->isEnd(item) == false)
    {
        File* file = mSharedFileContainer->getFile();

        if (file->isOpen() == false)
        {
            openSeqLogFile(*file);
            callLogFileChanged();
        }

        // 書き込み
        if (mBinaryLog)
            writeSeqLogFile(    *file, item);
        else
            writeSeqLogFileText(*file, item);

        // ローテーション
        uint32_t maxSize = serviceMain->getMaxFileSize();
        uint64_t size = file->getSize();

        if (maxSize != 0 && maxSize < size)
        {
            file->close();

            openSeqLogFile(*file);
            callLogFileChanged();
        }

        // 次のシーケンスログアイテムを取得
        SequenceLogItem* next = (SequenceLogItem*)item->mNext;

        pushStockItem(item);
        item = next;
    }

    // シーケンスログリスト初期化
    mOutputList->clear();
}

/*!
*  \brief  リスナーのonLogFileChanged()をコール
 */
void SequenceLogService::callLogFileChanged()
{
    ThreadListeners* listeners = getListeners();

    for (ThreadListeners::iterator i = listeners->begin(); i != listeners->end(); i++)
    {
        SequenceLogServiceThreadListener* listener = dynamic_cast<SequenceLogServiceThreadListener*>(*i);
        listener->onLogFileChanged(this);
    }
}

/*!
 *  \brief  クリーンアップ
 */
void SequenceLogService::cleanUp()
{
    // シーケンスログアイテムキューマネージャー削除
    if (mItemQueueManager)
    {
        for (ItemQueueManager::iterator i = mItemQueueManager->begin(); i != mItemQueueManager->end(); i++)
        {
            std::pair<uint32_t, ItemQueue*> pair = *i;
            ItemQueue* queue = pair.second;

            delete queue;
        }

        mItemQueueManager->clear();
        delete mItemQueueManager;
        mItemQueueManager = NULL;
    }

    // シーケンスログ出力リスト削除
    delete mOutputList;
    mOutputList = NULL;

    // シーケンスログアイテムのストックを削除
    SequenceLogItem* item = mStockItems;
    mStockItems = NULL;

    while (item)
    {
        SequenceLogItem* next = (SequenceLogItem*)item->mNext;
        delete item;
        item = next;
    }

    // ソケット、共有メモリ、ファイルをクローズ
//  if (mSocket->isOpen())
//      mSocket->close();

    // ログバッファ削除
    delete mSHM;

    // シーケンスログ共有ファイルコンテナリリース
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    serviceMain->releaseSharedFileContainer(mSharedFileContainer);

//  mSharedFileContainer = NULL;
}

/*!
 *  \brief  シーケンスログファイルがバイナリーかテキストか
 */
const char* SequenceLogService::initBinaryOrText(CoreString* fileName)
{
    // 拡張子取得
    const char* ext = strrchr(fileName->getBuffer(), '.');
    static const char defaultExt[] = "slog";

    if (ext)
    {
        *(char*)ext = '\0';
        ext++;
    }
    else
    {
        ext = defaultExt;
    }

    mBinaryLog = (stricmp(ext, "slog") == 0);
    return ext;
}

/*!
 *  \brief  シーケンスログファイルオープン
 */
void SequenceLogService::openSeqLogFile(File& file) throw(Exception)
{
    // ベースファイル名取得
    FixedString<MAX_PATH> fileName = mSharedFileContainer->getBaseFileName()->getBuffer();

    // 拡張子取得
    const char* ext = initBinaryOrText(&fileName);

    // 開始日時取得
    DateTime dateTime;
    dateTime.setCurrent();
    dateTime.toLocal();

    // シーケンスログファイル名作成
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    FixedString<MAX_PATH> str;

    str.format(
        "%s%c%s-%05d-%04d%02d%02d-%02d%02d%02d-%03d.%s",
        serviceMain->getLogFolderName().getBuffer(),
        PATH_DELIMITER,
        fileName.getBuffer(),
        mProcess.getId(),
        dateTime.getYear(),
        dateTime.getMonth(),
        dateTime.getDay(),
        dateTime.getHour(),
        dateTime.getMinute(),
        dateTime.getSecond(),
        dateTime.getMilliSecond(),
        ext);

    // ファイル情報更新
    FileInfo* fileInfo = mSharedFileContainer->getFileInfo();

    if (fileInfo)
    {
        fileInfo->update();
        // 下記でnewしているが、オブジェクトはSequenceLogServiceMainで
        // 管理しているのでメモリリークの心配はない（delete不要）
    }

    // 新たなファイル情報を生成
    fileInfo = new FileInfo(str);
    fileInfo->setCreationTime(dateTime);

    // 共有ファイルコンテナにセット
    mSharedFileContainer->setFileInfo(fileInfo);

    // SequenceLogServiceMainにもセット
    ScopedLock lock(serviceMain->getMutex());
    serviceMain->addFileInfo(fileInfo);

    // ファイル作成
    const CoreString& canonicalPath = fileInfo->getCanonicalPath();
    TRACE("    openSeqLogFile(): '%s'\n", canonicalPath.getBuffer());

    fileInfo->mkdir();
    file.open(canonicalPath, File::WRITE);

    // ファイル情報更新
    fileInfo->update(true);
    fileInfo->setLastWriteTime(DateTime());
}

/*!
 *  \brief  シーケンスログファイルに書き込む
 */
void SequenceLogService::writeSeqLogFile(File& file, SequenceLogItem* item)
{
    uint32_t size = mFileOutputBuffer.putSequenceLogItem(item, false);
    file.write(&mFileOutputBuffer, size);

    // シーケンスログプリントにログを送信
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();

    if (serviceMain->isOutputScreen())
    {
        writeSeqLogFileText(file, item);
    }
}

/*!
 *  \brief  シーケンスログファイルに書き込む
 */
void SequenceLogService::writeSeqLogFileText(File& file, SequenceLogItem* item)
{
    char levelChars[] = "diwe"; // Debug, Info, Warn, Error
    char lc = levelChars[1];

    FixedString<DateTimeFormat::DATE_TIME_MS_LEN> strDateTime;
    DateTimeFormat::toString(&strDateTime, item->mDateTime, DateTimeFormat::DATE_TIME_MS);

    FixedString<768> str;

    switch (item->mType)
    {
    case SequenceLogItem::STEP_IN:
        if (item->mClassId == 0 && item->mFuncId == 0)
        {
            str.format(
                "%c%d %s %d %d"
                " 0 %s 0 %s\n",
                lc, item->mSeqNo, strDateTime.getBuffer(), item->mType, item->mThreadId,
                item->getClassName()->getBuffer(), item->getFuncName()->getBuffer());
        }

        if (item->mClassId != 0 && item->mFuncId == 0)
        {
            str.format(
                "%c%d %s %d %d"
                " 1 %d 0 %s\n",
                lc, item->mSeqNo, strDateTime.getBuffer(), item->mType, item->mThreadId,
                item->mClassId, item->getFuncName()->getBuffer());
        }

        if (item->mClassId != 0 && item->mFuncId != 0)
        {
            str.format(
                "%c%d %s %d %d"
                " 1 %d 1 %d\n",
                lc, item->mSeqNo, strDateTime.getBuffer(), item->mType, item->mThreadId,
                item->mClassId, item->mFuncId);
        }

        break;

    case SequenceLogItem::STEP_OUT:
        {
            str.format(
                "%c%d %s %d %d"
                "\n",
                lc, item->mSeqNo, strDateTime.getBuffer(), item->mType, item->mThreadId);
        }
        break;

    case SequenceLogItem::MESSAGE:
        lc = levelChars[item->mLevel];

        if (item->mMessageId == 0)
        {
            str.format(
                "%c%d %s %d %d"
                " %d 0 %s\n",
                lc, item->mSeqNo, strDateTime.getBuffer(), item->mType, item->mThreadId,
                item->mLevel, item->getMessage()->getBuffer());
        }
        else
        {
            str.format(
                "%c%d %s %d %d"
                " %d 1 %d\n",
                lc, item->mSeqNo, strDateTime.getBuffer(), item->mType, item->mThreadId,
                item->mLevel, item->mMessageId);
        }

        break;
    }

    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    int32_t len = str.getLength();

    if (mBinaryLog == false)
        file.write(&str, 1, len - 1);

    ScopedLock lock(serviceMain->getMutex());
//  serviceMain->printLog(&str, len);
    serviceMain->printLog(&str, str.getCapacity());
}

/*!
 *  \brief  シーケンスログアイテムを振り分ける
 */
void SequenceLogService::divideItems()
{
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();

    // 振り分け処理
    do
    {
        const SequenceLogItem& src = mSHM->item;

        if (src.mType != SequenceLogItem::STEP_IN  &&
            src.mType != SequenceLogItem::STEP_OUT &&
            src.mType != SequenceLogItem::MESSAGE)
        {
            noticeLog("Illegal value: src.mType=%d\n", src.mType);
            interrupt();
            break;
        }

        ItemQueue* queue = getItemQueue(src);
        SequenceLogItem* item = createSequenceLogItem(queue, src);
        bool isKeep = false;

        if (item == NULL)
            continue;

        if (item->mOutputFlag == slog::ROOT)
            item->mOutputFlag =  slog::ALWAYS;

        if (item->mType == SequenceLogItem::STEP_IN)
        {
            isKeep = true;

            // あらかじめSTEP_OUTのアイテムを作成しておく
            SequenceLogItem* stepOutItem = popStockItem();
            stepOutItem->init(item->mSeqNo, item->mOutputFlag);
            stepOutItem->mThreadId = item->mThreadId;

            queue->mStepOutList.push_back(stepOutItem);
        }

        if (item->mType == SequenceLogItem::MESSAGE)
        {
            if (item->mLevel == slog::DEBUG)
                isKeep = true;
        }

        if (isKeep)
            keep(   queue, item);
        else
            forward(queue, item);
    }
    while (false);

    if (isInterrupted())
    {
        // 終了処理。保留中のログを全て出力する
        SequenceLogItem* item;

        for (ItemQueueManager::iterator i = mItemQueueManager->begin(); i != mItemQueueManager->end(); i++)
        {
            std::pair<uint32_t, ItemQueue*> pair = *i;
            ItemQueue* queue = pair.second;

            mOutputList->merge(queue->mList);

            while ((item = queue->mStepOutList.back()) != NULL)
            {
                queue->mStepOutList.pop_back();

                item->setCurrentDateTime();
                mOutputList->push_back(item);
            }
        }
    }
}

/*!
 *  \brief  シーケンスログアイテムをキープする
 */
void SequenceLogService::keep(ItemQueue* queue, SequenceLogItem* item)
{
    bool bForward = false;

    if (queue->mAlwaysCount ||
        (item->mOutputFlag & slog::ALWAYS))
    {
        // 常出力カウンタが1以上、または出力指示なのでキープせずに出力する
        bForward = true;
    }

    else
    if ( item->mType == SequenceLogItem::STEP_IN &&
        (item->mOutputFlag & slog::OUTPUT_ALL))
    {
        bForward = true;
    }

    if (bForward)
    {
        forward(queue, item);

//      if (item->mOutputFlag & slog::ALWAYS)
        if (item->mOutputFlag & slog::ALWAYS && item->mType == SequenceLogItem::STEP_IN)
        {
            queue->mAlwaysCount++;
//          TRACE("keep(): mSeqNo=%d, mAlwaysCount=%d\n", item->mSeqNo, queue->mAlwaysCount);
        }

        return;
    }

    //
    // キープ
    //
    queue->mList.push_back(item);
}

/*!
 *  \brief  シーケンスログリストに追加する
 */
void SequenceLogService::forward(ItemQueue* queue, SequenceLogItem* item)
{
    do
    {
        if (item->mType != SequenceLogItem::STEP_OUT)
        {
            mOutputList->merge(queue->mList);
            break;
        }

        if (item->mOutputFlag & slog::ALWAYS)
        {
            queue->mAlwaysCount--;
//          TRACE("forward(): mSeqNo=%d, mAlwaysCount=%d\n", item->mSeqNo, queue->mAlwaysCount);
        }

        if (queue->mList.empty())
            break;

        //
        // シーケンスログアイテムキューから、シーケンス番号が一致するアイテムを全て削除する
        //
        SequenceLogItem* delItem = queue->mList.back();
        uint32_t type = SequenceLogItem::MESSAGE;

        while (delItem && delItem->mSeqNo == item->mSeqNo)
        {
            type = delItem->mType;

#if 0
            if (type == SequenceLogItem::STEP_IN)
            {
                TRACE(
                    "    discard mSeqNo=%d, tid=%d, %s::%s\n",
                    delItem->mSeqNo,
                    delItem->mThreadId,
                    delItem->getClassName().getBuffer(),
                    delItem->getFuncName().getBuffer());
            }
#endif

            queue->mList.pop_back();
            pushStockItem(delItem);

            delItem = queue->mList.back();
        }

        if (type == SequenceLogItem::STEP_IN)
        {
            pushStockItem(item);
            return;
        }
    }
    while (false);

    mOutputList->push_back(item);
}

/*!
 *  \brief  シーケンスログアイテムキュー取得
 */
ItemQueue* SequenceLogService::getItemQueue(const SequenceLogItem& item) const
{
    uint32_t threadId = item.mThreadId;
    ItemQueue* queue = (*mItemQueueManager)[threadId];

    if (queue == NULL)
    {
        queue = new ItemQueue;
        (*mItemQueueManager)[threadId] = queue;
    }

    return queue;
}

/*!
 *  \brief  シーケンスログアイテム作成
 */
SequenceLogItem* SequenceLogService::createSequenceLogItem(
    ItemQueue* queue,               //!< シーケンスログアイテムキュー
    const SequenceLogItem& src)     //!< 作成元シーケンスログアイテム
{
    SequenceLogItem* item;

    if (src.mType == SequenceLogItem::STEP_IN)
    {
         item = popStockItem();
        *item = src;
        return item;
    }

    do
    {
        item = queue->mStepOutList.back();

        if (item == NULL)
        {
            // あらかじめ作成してあるはずのアイテムがない（fork()を使っているとここに来る）
//          TRACE("    queue->mStepOutList.back() is NULL! (did fork?) src.mSeqNo=%d\n", src.mSeqNo);
            return NULL;
        }

        if (item->mSeqNo == src.mSeqNo)
            break;

        queue->mStepOutList.pop_back();

        item->mDateTime = src.mDateTime;
        forward(queue, item);
    }
    while (true);

    if (src.mType == SequenceLogItem::STEP_OUT)
    {
        queue->mStepOutList.pop_back();
        *item = src;
    }
    else
    {
         item = popStockItem();
        *item = src;
    }

    return item;
}

/*!
 *  \brief  シーケンスログ受信スレッド
 */
void SequenceLogService::receiveMain()
{
    try
    {
        Socket* socket = mHttpRequest->getSocket();

        while (true)
        {
            bool isReceive = socket->isReceiveData(3000);

            if (isInterrupted())
                break;

            if (isReceive == false)
                continue;

            // シーケンスログアイテム受信
            ByteBuffer* buffer = WebSocket::recv(socket, NULL);

            if (buffer == NULL)
                continue;

            // バッファからシーケンスログアイテムを設定
            ((SequenceLogByteBuffer*)buffer)->getSequenceLogItem(&mSHM->item);
            delete buffer;

            // シーケンスログアイテムのタイプが STEP_IN の場合はシーケンス番号を返信する
            if (mSHM->item.mType == SequenceLogItem::STEP_IN)
            {
                mSHM->item.mSeqNo = mSHM->seq;
                mSHM->seq++;

                ByteBuffer seqNoBuf(sizeof(mSHM->item.mSeqNo));
                seqNoBuf.putInt(mSHM->item.mSeqNo);

                WebSocket::sendHeader(socket, sizeof(mSHM->item.mSeqNo), false);
                socket->send(&seqNoBuf, sizeof(mSHM->item.mSeqNo));
            }

            divideItems();
            writeMain();
        }
    }
    catch (Exception& e)
    {
        noticeLog("receiveMain: %s", e.getMessage());
    }

    try
    {
        divideItems();
        writeMain();
    }
    catch (Exception& e)
    {
        noticeLog("receiveMain: %s", e.getMessage());
    }
}

} // namespace slog
