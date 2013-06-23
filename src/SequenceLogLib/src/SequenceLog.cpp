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
 *  \file   SequenceLog.cpp
 *  \brief  シーケンスログクラス
 *  \author Copyright 2011-2013 printf.jp
 */
//nclude "SequenceLog.h"
#include "SequenceLogItem.h"

#include "slog/Mutex.h"
#include "slog/Socket.h"
#include "slog/TimeSpan.h"
#include "slog/Process.h"
#include "slog/FixedString.h"
#include "slog/PointerString.h"

#include "slog/WebServerResponseThread.h"
#include "slog/HttpResponse.h"

/******************************************************************************
*
* C言語用
*
******************************************************************************/
extern "C"
{
    /*!
     *  \brief  ステップイン時のログ出力
     */
//  void* _slog_stepIn(const char* className, const char* funcName, SequenceLogOutputFlag outputFlag)
    void* _slog_stepIn(const char* className, const char* funcName)//, int32_t            outputFlag)
    {
        slog::SequenceLog* slog = new slog::SequenceLog(className, funcName);//, (slog::SequenceLogOutputFlag)outputFlag);
        return slog;
    }

    /*!
     *  \brief  ステップイン時のログ出力
     */
//  void* _slog_stepIn2(uint32_t classID, const char* funcName)//, int32_t outputFlag)
//  {
//      slog::SequenceLog* slog = new slog::SequenceLog(classID, funcName);//, (slog::SequenceLogOutputFlag)outputFlag);
//      return slog;
//  }

    /*!
     *  \brief  ステップイン時のログ出力
     */
//  void* _slog_stepIn3(uint32_t classID, uint32_t funcID)//, int32_t outputFlag)
//  {
//      slog::SequenceLog* slog = new slog::SequenceLog(classID, funcID);//, (slog::SequenceLogOutputFlag)outputFlag);
//      return slog;
//  }

    /*!
     *  \brief  ステップアウト時のログ出力
     */
    void _slog_stepOut(void* p)
    {
        slog::SequenceLog* slog = (slog::SequenceLog*)p;
        delete slog;
    }

    /*!
     *  \brief  メッセージ出力
     */
//  void  _slog_message(void* p, SequenceLogLevel level, const char* format, ...)
    void  _slog_message(void* p, int32_t          level, const char* format, ...)
    {
        slog::SequenceLog* slog = (slog::SequenceLog*)p;

        va_list arg;
        va_start(arg, format);

        slog->messageV((slog::SequenceLogLevel)level, format, arg);
    }

    /*!
     *  \brief  メッセージ出力
     */
//  void  _slog_message2(void* p, int32_t level, uint32_t messageID)
//  {
//      slog::SequenceLog* slog = (slog::SequenceLog*)p;
//      slog->message((slog::SequenceLogLevel)level, messageID);
//  }
}

/*-----------------------------------------------------------------------------
|
| 静的メンバー変数定義
|
+----------------------------------------------------------------------------*/
namespace slog
{

static char                     sSequenceLogFileName[MAX_PATH + 1] = "";            //!< シーケンスログファイル名
static char                     sSequenceLogServiceAddress[255 + 1] = "127.0.0.1";  //!< シーケンスログサービスアドレス
static uint16_t                 sSequenceLogServicePort = 8080;                     //!< シーケンスログサービスポート
static bool                     sUseSSL = false;
static SequenceLogOutputFlag    sRootFlag = ROOT;

class  SequenceLogClient;
static SequenceLogClient*       sClient = NULL;                 //!< シーケンスログクライアントオブジェクト
static bool                     sClientInitialized = false;     //!< 初期化フラグ

/*!
 *  \brief  シーケンスログクライアントクラス
 */
class SequenceLogClient
{
            Socket  mSocket;        //!< ソケット
            Mutex*  mSocketMutex;   //!< ソケット用ミューテックス

            /*!
             * コンストラクタ／デストラクタ
             */
public:      SequenceLogClient();
            ~SequenceLogClient();

            /*!
             * 初期化
             */
            void init();

            /*!
             * シーケンスログアイテム生成
             */
public:     SequenceLogItem* createItem();

            /*!
             * シーケンスログアイテム送信
             */
            void sendItem(SequenceLogItem* item, uint32_t* seq = NULL);
};

/*!
 *  \brief  コンストラクタ
 */
inline SequenceLogClient::SequenceLogClient()
{
    Socket::startup();
    mSocketMutex = NULL;
}

/*!
 *  \brief  デストラクタ
 */
inline SequenceLogClient::~SequenceLogClient()
{
    delete mSocketMutex;
    mSocket.close();

    Socket::cleanup();
}

/*!
 *  \brief  初期化
 */
void SequenceLogClient::init()
{
    noticeLog("client initialize.\n");

    // シーケンスログファイル名取得
    const char* p = sSequenceLogFileName;

    if (p == NULL || p[0] == '\0')
    {
        TRACE("[E] SequenceLogClient::init() - failed\n", 0);
        return;
    }

    try
    {
        // 初期化処理のためのソケット作成
#if defined(__ANDROID__) && 0
        const char* SOCKET_SLOG = "/dev/socket/slog";
        struct stat buf;

        bool success = (stat(SOCKET_SLOG, &buf) == 0);
//      success = true;

        if (success)
        {
            mSocket.open(false);
            mSocket.setRecvTimeOut(3000);
            mSocket.connect(FixedString<108>(SOCKET_SLOG));
        }
        else
#endif
        {
            PointerString address = sSequenceLogServiceAddress;

            mSocket.open();
            mSocket.setRecvTimeOut(3000);
            mSocket.setNoDelay(true);
            mSocket.connect(address, sSequenceLogServicePort);
        }

        if (sUseSSL)
            mSocket.useSSL();

        // WebSocketアップグレード
        String upgrade =
            "GET /outputLog HTTP/1.1\r\n"
            "Upgrade: websocket\r\n"
            "Sec-WebSocket-Key: m31EnckktzJZ/3ZWkvwNHQ==\r\n"
            "Sec-WebSocket-Version: 13\r\n"
            "\r\n";

        mSocket.send(&upgrade, upgrade.getLength());

        // WebSocketアップグレードレスポンス受信
        HttpResponse httpResponse(&mSocket);

        if (httpResponse.analizeResponse() == false)
        {
            return;
        }

        // WebSocketヘッダー送信
        Process process;
        uint32_t pid = process.getId();

        FixedString<MAX_PATH> name = p;
        int32_t len = name.getLength() + 1;

        WebServerResponseThread::sendWebSocketHeader(&mSocket,
            sizeof(pid) + sizeof(len) + len,
            false, false);

        // プロセスID送信
        mSocket.send(&pid);

        // シーケンスログファイル名送信
        mSocket.send(&len);
        mSocket.send(&name, len);

        // ソケット用ミューテックス生成
        mSocketMutex = new Mutex();
    }
    catch (Exception e)
    {
        noticeLog("%s\n", e.getMessage());
        mSocket.close();
    }
}

/*!
 *  \brief  シーケンスログアイテム生成
 */
SequenceLogItem* SequenceLogClient::createItem()
{
    if (this == NULL)
    {
        // sClient（唯一の SequenceLogClient インスタンス）が存在しないうちに呼ばれたら NULL を返す
        return NULL;
    }

    if (mSocket.isOpen() == false)
    {
        // ソケットが閉じられている場合はシーケンスログアイテムを生成する意味がないので（送信しないから）
        // NULL を返す
        return NULL;
    }

    // シーケンスログアイテム生成
    SequenceLogItem* item = new SequenceLogItem;

    item->setCurrentDateTime();
    item->mThreadId = Thread::getCurrentId();

    return item;
}

/*!
 *  \brief  シーケンスログアイテム送信
 */
void SequenceLogClient::sendItem(
    SequenceLogItem* item,  // 送信するシーケンスログアイテム
    uint32_t* seq)          // シーケンス番号
                            //     サーバーから返されたシーケンス番号を seq に格納する
                            //     送信するシーケンスログアイテムのタイプが STEP_IN の場合は必須
{
    try
    {
        ScopedLock lock(mSocketMutex);

        // シーケンスログアイテムをバイトバッファに格納
        uint32_t capacity =
            sizeof(int16_t) +                                       // 全体のレコード長
            sizeof(SequenceLogItemCore) +                           // シーケンスログアイテム基本データ
            sizeof(int16_t) + item->getClassName()->getLength() +   // クラス名の長さ＋クラス名
            sizeof(int16_t) + item->getFuncName()-> getLength() +   // 関数名の長さ＋関数名
            sizeof(int16_t) + item->getMessage()->  getLength();    // メッセージの長さ＋メッセージ

        SequenceLogByteBuffer buffer(capacity);
        uint32_t size = buffer.putSequenceLogItem(item, true);

        // シーケンスログアイテム送信
        WebServerResponseThread::sendWebSocketHeader(&mSocket, size, false, false);
        mSocket.send(&buffer, size);

        // STEP_INの場合はシーケンス番号を受信する
        if (item->mType == SequenceLogItemCore::STEP_IN)
        {
            ByteBuffer buffer(sizeof(uint32_t));

            WebServerResponseThread::recvData(&mSocket, &buffer);
            *seq = buffer.getInt();
        }
    }
    catch (Exception e)
    {
        // 異常発生。ソケットを閉じる
        noticeLog("%s\n", e.getMessage());
        mSocket.close();
    }

    // 送信済みシーケンスログアイテムを削除
    delete item;
}

/*!
 *  \brief  シーケンスログクライアントデリータクラス
 */
class SequenceLogClientDeleter
{
public:     ~SequenceLogClientDeleter()
            {
                delete sClient;
                sClient = NULL;
            }
};

static SequenceLogClientDeleter s_deleter;

/******************************************************************************
*
* シーケンスログクラス
*
******************************************************************************/

/*!
 *  \brief  コンストラクタ
 */
SequenceLog::SequenceLog(
    const char* className,              //!< クラス名
    const char* funcName)               //!< メソッド名
//  SequenceLogOutputFlag outputFlag)   //!< 出力フラグ
{
    SequenceLogOutputFlag outputFlag = ROOT;
    init(outputFlag);
    SequenceLogItem* item = sClient->createItem();

    if (item)
    {
        item->init(mSeqNo, mOutputFlag, className, funcName);
        sClient->sendItem(item, &mSeqNo);
    }
}

/*!
 *  \brief  コンストラクタ
 */
//SequenceLog::SequenceLog(uint32_t classID, const char* funcName)//, SequenceLogOutputFlag outputFlag)
//{
//    SequenceLogOutputFlag outputFlag = ROOT;
//    init(outputFlag);
//    SLOG_ITEM_INFO* info = sClient->createItem();
//
//    if (info)
//    {
//        info->item.init(mSeqNo, mOutputFlag, classID, funcName);
//        info->ready = true;
//        sClient->sendItem(info, &mSeqNo);
//    }
//#if defined(_DEBUG)
//    else
//    {
//        TRACE("(flag:%d) %d::%s\n", mOutputFlag, classID, funcName);
//    }
//#endif
//}

/*!
 *  \brief  コンストラクタ
 */
//SequenceLog::SequenceLog(uint32_t classID, uint32_t funcID)//, SequenceLogOutputFlag outputFlag)
//{
//    SequenceLogOutputFlag outputFlag = ROOT;
//    init(outputFlag);
//    SLOG_ITEM_INFO* info = sClient->createItem();
//
//    if (info)
//    {
//        info->item.init(mSeqNo, mOutputFlag, classID, funcID);
//        info->ready = true;
//        sClient->sendItem(info, &mSeqNo);
//    }
//#if defined(_DEBUG)
//    else
//    {
//        TRACE("(flag:%d) %d::%d\n", mOutputFlag, classID, funcID);
//    }
//#endif
//}

/*!
 *  \brief  デストラクタ
 */
SequenceLog::~SequenceLog()
{
    SequenceLogItem* item = sClient->createItem();

    if (item)
    {
        item->init(mSeqNo, mOutputFlag);
        sClient->sendItem(item);
    }
}

/*!
 *  \brief  初期化
 */
void SequenceLog::init(SequenceLogOutputFlag outputFlag)
{
    if (sClientInitialized == false)
    {
        sClientInitialized = true;

        sClient = new SequenceLogClient;
        sClient->init();
    }

    mSeqNo = 0;
    mOutputFlag = (outputFlag != ROOT ? outputFlag : sRootFlag);
}

/*!
 *  \brief  メッセージ出力
 */
void SequenceLog::message(SequenceLogLevel level, const char* format, ...)
{
    va_list arg;
    va_start(arg, format);

    messageV(level, format, arg);
}

/*!
 *  \brief  メッセージ出力
 */
void SequenceLog::messageV(SequenceLogLevel level, const char* format, va_list arg)
{
    SequenceLogItem* item = sClient->createItem();

    if (item)
    {
        item->init(mSeqNo, mOutputFlag, level);
        item->mMessageId = 0;

        try
        {
            CoreString* message = item->getMessage();
            message->formatV(format, arg);
        }
        catch (Exception /*e*/)
        {
            // 何もしない
        }

        sClient->sendItem(item);
    }
}

/*!
 *  \brief  メッセージ出力
 */
//void SequenceLog::message(SequenceLogLevel level, uint32_t messageID)
//{
//    SLOG_ITEM_INFO* info = sClient->createItem();
//
//    if (info)
//    {
//        info->item.init(mSeqNo, mOutputFlag, level);
//        info->item.mMessageId = messageID;
//        info->ready = true;
//        sClient->sendItem(info);
//    }
//}

/*!
 *  \brief  シーケンスログアイテム取得
 */
void SequenceLogByteBuffer::getSequenceLogItem(SequenceLogItem* item) throw(Exception)
{
    Exception e;
    setPosition(0);

    // レコード長
    uint16_t size = getShort();

    // シーケンス番号
    uint32_t seq = getInt();
    item->mSeqNo = seq;

    // 日時
    uint64_t datetime = getLong();
    item->mDateTime.setValue(datetime);

    // シーケンスログアイテム種別
    SequenceLogItem::Type type = (SequenceLogItem::Type)get();
    item->mType = type;

    // ID
    uint32_t threadId = getInt();
    item->mThreadId = threadId;

    switch (type)
    {
    case SequenceLogItem::STEP_IN:
    {
        // クラス名
        uint32_t ID = getInt();
        item->mClassId = ID;

        if (ID == 0)
        {
            short classLen = getShort();
            CoreString* className = item->getClassName();

            className->copy(get(classLen), classLen);
        }

        // 関数名
        ID = getInt();
        item->mFuncId = ID;

        if (ID == 0)
        {
            short funcLen = getShort();
            CoreString* funcName = item->getFuncName();

            funcName->copy(get(funcLen), funcLen);
        }

        break;
    }

    case SequenceLogItem::STEP_OUT:
        break;

    case SequenceLogItem::MESSAGE:
    {
        // メッセージ
        SequenceLogLevel level = (SequenceLogLevel)get();
        item->mLevel = level;

        uint32_t ID = getInt();
        item->mMessageId = ID;

        if (ID == 0)
        {
            short msgLen = getShort();
            CoreString* message = item->getMessage();

            message->copy(get(msgLen), msgLen);
        }

        break;
    }

    default:
        e.setMessage("シーケンスログアイテム種別(%d)が正しくありません。", type);
        throw e;
    }

    item->mOutputFlag = getInt();

    if (getPosition() != size)
    {
        e.setMessage("データが異常です(%d, %d)。シーケンスログアイテムを設定できませんでした。", getPosition(), size);
        throw e;
    }
}

/*!
 *  \brief  シーケンスログアイテム書き込み
 */
uint32_t SequenceLogByteBuffer::putSequenceLogItem(const SequenceLogItem* item, bool enableOutputFlag)
{
    unsigned short size;
    int32_t len;

    setPosition(sizeof(size));

    // シーケンス番号
    putInt(item->mSeqNo);

    // 日時
    putLong(item->mDateTime.getValue());

    // シーケンスログアイテム種別
    put(item->mType);

    // スレッド ID
    putInt(item->mThreadId);

    switch (item->mType)
    {
    case SequenceLogItem::STEP_IN:
        // クラス名
        putInt(item->mClassId);

        if (item->mClassId == 0)
        {
            CoreString* className = item->getClassName();
            len = className->getLength();

            putShort(len);
            put(className, len);
        }

        // 関数名
        putInt(item->mFuncId);

        if (item->mFuncId == 0)
        {
            CoreString* funcName = item->getFuncName();
            len = funcName->getLength();

            putShort(len);
            put(funcName, len);
        }

        break;

    case SequenceLogItem::STEP_OUT:
        break;

    case SequenceLogItem::MESSAGE:
    {
        // メッセージ
        put(item->mLevel);
        putInt(item->mMessageId);

        if (item->mMessageId == 0)
        {
            CoreString* message = item->getMessage();
            len = message->getLength();

            putShort(len);
            put(message, len);
        }
        break;
    }

    default:
        break;
    }

    if (enableOutputFlag)
        putInt(item->mOutputFlag);

    // 先頭にレコード長
    size = getPosition();

    setPosition(0);
    putShort(size);

    return size;
}

} // namespace slog

/*!
 *  \brief  シーケンスログファイル名を設定する
 */
extern "C" void setSequenceLogFileName(const char* fileName)
{
    if (strlen(fileName) <= sizeof(slog::sSequenceLogFileName) - 1)
        strcpy(slog::sSequenceLogFileName, fileName);
}

/*!
 *  \brief  シーケンスログサービスのアドレスを設定する
 */
extern "C" void setSequenceLogServiceAddress(const char* address, uint16_t port, bool useSSL)
{
    if (strlen(address) <= sizeof(slog::sSequenceLogServiceAddress) - 1)
    {
        strcpy(slog::sSequenceLogServiceAddress, address);
        slog::sSequenceLogServicePort = port;
        slog::sUseSSL = useSSL;
    }
}

/*!
 *  \brief  ROOTの既定値を設定する
 */
//extern "C" void setRootFlag(int32_t outputFlag)
//{
//    slog::sRootFlag = (slog::SequenceLogOutputFlag)outputFlag;
//}

extern "C" void enableOutput(int32_t enable)
{
    slog::sRootFlag = (enable != 0 ? slog::ROOT : slog::KEEP);
}
