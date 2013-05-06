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

#include "slog/Socket.h"
#include "slog/Mutex.h"
#include "slog/FileInfo.h"
#include "slog/DateTimeFormat.h"
#include "slog/WebServerResponseThread.h"

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
 *  \brief  シーケンスログ受信クラス
 */
class SequenceLogReceiver : public Thread
{
            SequenceLogService* mService;

public:     SequenceLogReceiver(SequenceLogService* service) {mService = service;}

private:    virtual void run() {mService->receiveMain();}
};

/*!
 *  \brief  コンストラクタ
 */
SequenceLogService::SequenceLogService(slog::Socket* socket) :
    mFileOutputBuffer(1024 * 2)
{
    mSocket = socket;
    mMutex = NULL;
    mSHM = NULL;

    mOutputList =       NULL;
    mItemQueueManager = NULL;
    mStockItems =       NULL;

    mFileInfo = NULL;
}

/*!
 *  \brief  デストラクタ
 */
SequenceLogService::~SequenceLogService()
{
    delete mSocket;
}

/*!
 *  \brief  初期化
 */
bool SequenceLogService::init()
{
    bool result = true;

    try
    {
        ByteBuffer* buffer = WebServerResponseThread::recvData(mSocket, NULL);

        // プロセスID取得
        uint32_t id = buffer->getInt();
        mProcess.setId(id);

        // シーケンスログファイル名取得
        int32_t len = buffer->getInt();
        mBaseFileName.copy(buffer->get(len), len);

        // バッファ削除
        delete buffer;

        // ファイル名チェック
        if (mBaseFileName[len - 1] != '\0')
        {
            Exception e;
            e.setMessage("SequenceLogService::init() / receive name is not null-terminated");

            throw e;
        }

//      openSeqLogFile(mFile);

        FixedString<MAX_PATH> shmName;

        mOutputList =       new ItemList;
        mItemQueueManager = new ItemQueueManager;

        // ログバッファ（旧 - 共有メモリ）生成
        SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
        int32_t size = sizeof(SLOG_ITEM_INFO);
        int32_t count = serviceMain->getSharedMemoryItemCount();
        len = sizeof(SLOG_SHM) + size * (count - 1);

        mSHM = (SLOG_SHM*)new char[len];

        // ミューテックス生成
#if defined(_WINDOWS)
        FixedString<MAX_PATH> name;
        name.format("slogMutex%d", mProcess.getId());

        mMutex = new slog::Mutex(true, name);
#else
        mMutex = new slog::Mutex(true, &mSHM->header.mutex);
#endif
        ScopedLock lock(mMutex, false);

        // 初期化：共有メモリヘッダー
        SLOG_SHM_HEADER* header = &mSHM->header;
        header->seq = 1;
        header->index = 0;
        header->max = 0;

        // 初期化：シーケンスログアイテム情報の配列数
        mSHM->count = count;

        // 初期化：シーケンスログアイテム情報の配列
        SLOG_ITEM_INFO* infoArray = &mSHM->infoArray[0];

        for (int32_t index = 0; index < count; index++)
        {
            infoArray[index].ready = false;
            infoArray[index].no = index;
        }
    }
    catch (Exception e)
    {
        noticeLog("%s\n", e.getMessage());

        cleanUp();
        result = false;
    }

    return result;
}

/*
 *  \brief  シーケンスログサービススレッド
 */
void SequenceLogService::run()
{
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    SequenceLogReceiver receiver(this);

    receiver.start();
    Thread::sleep(100);

    // 書き込みループ
    try
    {
        while (true)
        {
            if (receiver.isAlive() == false)
                interrupt();

            divideItems();
            writeMain();
            sleep(1);

            if (receiver.isAlive() == false)
                break;
        }
    }
    catch (Exception e)
    {
        noticeLog(e.getMessage());
    }

    receiver.interrupt();
    receiver.join();

    cleanUp();
    mSocket->close();
}

/*!
 *  \brief  書き込みメイン
 */
void SequenceLogService::writeMain()
{
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    SequenceLogItem* item = mOutputList->front();

    while (item && mOutputList->isEnd(item) == false)
    {
        if (mFile.isOpen() == false)
        {
            openSeqLogFile(mFile);
            callLogFileChanged();
        }

        // 書き込み
        if (mBinaryLog)
            writeSeqLogFile(    mFile, item);
        else
            writeSeqLogFileText(mFile, item);

        // ローテーション
        uint32_t maxSize = serviceMain->getMaxFileSize();
        uint64_t size = mFile.getSize();

        if (maxSize != 0 && maxSize < size)
        {
            mFile.close();

            openSeqLogFile(mFile);
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
    delete mMutex;
    mMutex = NULL;

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

    delete [] (char*)mSHM;
    mFile.close();

    if (mFileInfo)
        mFileInfo->update();

    TRACE("[E] SequenceLogService::CleanUp()\n", 0);
}

/*!
 *  \brief  シーケンスログファイルオープン
 */
void SequenceLogService::openSeqLogFile(File& file) throw(Exception)
{
    FixedString<MAX_PATH> fileName = mBaseFileName;
    char* ext = (char*)strrchr(fileName.getBuffer(), '.');
    char defaultExt[] = "slog";

    if (ext)
    {
        *ext = '\0';
        ext++;
    }
    else
        ext = defaultExt;

    mBinaryLog = (stricmp(ext, "slog") == 0);

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

    if (mFileInfo)
    {
        mFileInfo->update();
        // 下記でnewしているが、オブジェクトはSequenceLogServiceMainで
        // 管理しているのでメモリリークの心配はない（delete不要）
    }

    mFileInfo = new FileInfo(str);
    mFileInfo->setCreationTime(dateTime);

    ScopedLock lock(serviceMain->getMutex());
    serviceMain->addFileInfo(mFileInfo);

    const CoreString& canonicalPath = mFileInfo->getCanonicalPath();
    TRACE("    openSeqLogFile(): '%s'\n", canonicalPath.getBuffer());

    mFileInfo->mkdir();
    file.open(canonicalPath, File::WRITE);

    mFileInfo->update(true);
    mFileInfo->setLastWriteTime(DateTime());
}

/*!
 *  \brief  シーケンスログファイルに書き込む
 */
void SequenceLogService::writeSeqLogFile(File& file, SequenceLogItem* item)
{
    uint32_t size = mFileOutputBuffer.putSequenceLogItem(item);
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
                item->getClassName().getBuffer(), item->getFuncName().getBuffer());
        }

        if (item->mClassId != 0 && item->mFuncId == 0)
        {
            str.format(
                "%c%d %s %d %d"
                " 1 %d 0 %s\n",
                lc, item->mSeqNo, strDateTime.getBuffer(), item->mType, item->mThreadId,
                item->mClassId, item->getFuncName().getBuffer());
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
                item->mLevel, item->getMessage().getBuffer());
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

    int32_t putOff = 0;
    ScopedLock lock(mMutex);

    SLOG_SHM_HEADER* header =   &mSHM->header;
    SLOG_ITEM_INFO* infoArray = &mSHM->infoArray[0];

    if (header->max < header->index)
    {
//      noticeLog("inside information: update entry item max (%d -> %d)\n", header->max, header->index);
        header->max = header->index;
    }

//  if (header->index == mSHM->count)
//      noticeLog("inside information: ** LIMIT ** entry item\n");

    // 振り分け処理
    for (int32_t index = 0; index < (int32_t)header->index; index++)
    {
        SLOG_ITEM_INFO* info = &infoArray[index];
        int32_t no = info->no;
        info = &infoArray[no];

        if (info->ready == false)
        {
            infoArray[index]. no = infoArray[putOff].no;
            infoArray[putOff].no = no;
            putOff++;
            continue;
        }

        const SequenceLogItem& src = info->item;
        info->ready = false;

        if (src.mType != SequenceLogItem::STEP_IN  &&
            src.mType != SequenceLogItem::STEP_OUT &&
            src.mType != SequenceLogItem::MESSAGE)
        {
            TRACE("Illegal value: src.mType=%d\n", src.mType);
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
    header->index = putOff;

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
    SequenceLogItem* item;
    ByteBuffer buffer(sizeof(*item));

    item = (SequenceLogItem*)buffer.getBuffer();

    try
    {
        while (true)
        {
            SLOG_ITEM_INFO* info;
            bool isReceive = mSocket->isReceiveData(3000);

            if (isInterrupted())
                break;

            if (isReceive == false)
                continue;

            // シーケンスログアイテム受信
            if (WebServerResponseThread::recvData(mSocket, &buffer) == NULL)
                break;

            while (true)
            {
                uint32_t threadId = item->mThreadId;

                ScopedLock lock(mMutex);
                SLOG_SHM_HEADER* header = &mSHM->header;

                if (header->index >= mSHM->count)
                    continue;

                SLOG_ITEM_INFO* infoArray = &mSHM->infoArray[0];
                uint32_t index = header->index;
                index = infoArray[index].no;

                info = &infoArray[index];
                header->index++;

                if (item->mType == SequenceLogItem::STEP_IN)
                {
                    item->mSeqNo = header->seq;
                    header->seq++;

                    ByteBuffer seqNoBuf(sizeof(item->mSeqNo));
                    seqNoBuf.putInt(item->mSeqNo);

                    WebServerResponseThread::sendWebSocketHeader(mSocket, sizeof(item->mSeqNo), false);
                    mSocket->send(&buffer, sizeof(item->mSeqNo));
                }

                info->item = *item;
                info->ready = true;
                break;
            }
        }
    }
    catch (Exception e)
    {
        noticeLog("receiveMain: %s", e.getMessage());
    }
}

} // namespace slog
