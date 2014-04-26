/*
 * Copyright (C) 2011-2014 printf.jp
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
 * \file    SequenceLog.cpp
 * \brief   シーケンスログクラス
 * \author  Copyright 2011-2014 printf.jp
 */
#pragma execution_character_set("utf-8")

#if defined(__unix__)
#include <string.h>
#endif

//nclude "SequenceLog.h"
#include "SequenceLogItem.h"

#include "slog/Mutex.h"
#include "slog/WebSocketClient.h"
#include "slog/Process.h"
#include "slog/FixedString.h"
#include "slog/PointerString.h"
#include "slog/File.h"
#include "slog/Tokenizer.h"
#include "slog/WebServerResponseThread.h"

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
    void* _slog_stepIn(const char* className, const char* funcName)
    {
        slog::SequenceLog* slog = new slog::SequenceLog(className, funcName);
        return slog;
    }

    /*!
     *  \brief  ステップイン時のログ出力
     */
//  void* _slog_stepIn2(uint32_t classID, const char* funcName)
//  {
//      slog::SequenceLog* slog = new slog::SequenceLog(classID, funcName);
//      return slog;
//  }

    /*!
     *  \brief  ステップイン時のログ出力
     */
//  void* _slog_stepIn3(uint32_t classID, uint32_t funcID)
//  {
//      slog::SequenceLog* slog = new slog::SequenceLog(classID, funcID);
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

//!< シーケンスログファイル名
static char sSequenceLogFileName[MAX_PATH + 1] = "";

//!< シーケンスログサービスアドレス
static char sSequenceLogServiceAddress[255 + 1] = "ws://127.0.0.1:8080";

//!< ユーザー名
static char sSequenceLogUserName[20 + 1] = "";

//!< パスワード
static char sSequenceLogPasswd[32 + 1] = "";

//!< ログレベル
static int32_t sLogLevel = DEBUG - 1;

//!< シーケンスログクライアントオブジェクト
class  SequenceLogClient;
static SequenceLogClient* sClient = nullptr;

//!< 初期化フラグ
static bool sClientInitialized = false;

/*!
 * \brief   シーケンスログクライアントクラス
 */
class SequenceLogClient : public WebSocketListener
{
            /*!
             * Web Socket
             */
            WebSocketClient mSocket;

            /*!
             * シーケンス番号
             */
            uint32_t mSeqNo;

            /*!
             * コンストラクタ
             */
public:      SequenceLogClient();

             /*!
             * デストラクタ
             */
            ~SequenceLogClient();

            /*!
             * 初期化
             */
public:     void init();

            /*!
             * Web Socket ハンドラ
             */
public:     virtual void onOpen() override;
            virtual void onError(const char* message) override;

            /*!
             * シーケンスログアイテム生成
             */
public:     SequenceLogItem* createItem();

            /*!
             * シーケンスログアイテム送信
             */
public:     void sendItem(SequenceLogItem* item, uint32_t* seq = nullptr);
};

/*!
 * \brief   コンストラクタ
 */
inline SequenceLogClient::SequenceLogClient()
{
    Socket::startup();
    mSeqNo = 1;
}

/*!
 * \brief   デストラクタ
 */
inline SequenceLogClient::~SequenceLogClient()
{
    mSocket.close();
    Socket::cleanup();
}

/*!
 * \brief   初期化
 */
void SequenceLogClient::init()
{
    noticeLog("client initialize.\n");

    if (sLogLevel == ERROR + 1/*NONE*/)
        return;

    // シーケンスログファイル名取得
    const char* p = sSequenceLogFileName;

    if (p == nullptr || p[0] == '\0')
    {
        noticeLog("SequenceLogClient::init() - failed\n");
        return;
    }

    // ソケット作成
    String url;
    url.format("%s/outputLog", sSequenceLogServiceAddress);

    mSocket.addWebSocketListener(this);
    mSocket.open(&url);
}

/*!
 * \brief   Web Socket onOpen
 */
void SequenceLogClient::onOpen()
{
    // WebSocketヘッダー送信
    Process process;
    uint32_t pid = process.getId();

//  PointerString   userName = sSequenceLogUserName;
    FixedString<20> userName = sSequenceLogUserName;
    int32_t userNameLen = userName.getLength() + 1;

    FixedString<32> passwd = sSequenceLogPasswd;
    int32_t passwdLen = passwd.getLength() + 1;

    FixedString<MAX_PATH> fileName = sSequenceLogFileName;
    int32_t fileNameLen = fileName.getLength() + 1;

    mSocket.sendHeader(
        sizeof(pid) +
        sizeof(userNameLen) + userNameLen +
        sizeof(passwdLen)   + passwdLen +
        sizeof(fileNameLen) + fileNameLen +
        sizeof(sLogLevel),
        false);

    // プロセスID送信
    mSocket.send(&pid);

    // ユーザー名送信
    mSocket.send(&userNameLen);
    mSocket.send(&userName, userNameLen);

    // パスワード送信
    mSocket.send(&passwdLen);
    mSocket.send(&passwd, passwdLen);

    // シーケンスログファイル名送信
    mSocket.send(&fileNameLen);
    mSocket.send(&fileName, fileNameLen);

    // ログレベル送信
    mSocket.send(&sLogLevel);
}

/*!
 * \brief   Web Socket onError
 */
void SequenceLogClient::onError(const char* message)
{
    noticeLog("onError: %s\n", message);
}

/*!
 * \brief   シーケンスログアイテム生成
 */
SequenceLogItem* SequenceLogClient::createItem()
{
    if (this == nullptr)
    {
        // sClient（唯一の SequenceLogClient インスタンス）が存在しないうちに呼ばれたら nullptr を返す
        return nullptr;
    }

    if (mSocket.isOpen() == false)
    {
        // ソケットが閉じられている場合はシーケンスログアイテムを生成する意味がないので（送信しないから）
        // nullptr を返す
        return nullptr;
    }

    // シーケンスログアイテム生成
    SequenceLogItem* item = new SequenceLogItem;

    item->setCurrentDateTime();
    item->mThreadId = Thread::getCurrentId();

    return item;
}

/*!
 * \brief   シーケンスログアイテム送信
 */
void SequenceLogClient::sendItem(
    SequenceLogItem* item,  // 送信するシーケンスログアイテム
    uint32_t* seq)          // シーケンス番号
                            //     サーバーから返されたシーケンス番号を seq に格納する
                            //     送信するシーケンスログアイテムのタイプが STEP_IN の場合は必須
{
    try
    {
        ScopedLock lock(mSocket.getMutex());

        // シーケンスログアイテムをバイトバッファに格納
        uint32_t capacity =
            sizeof(int16_t) +                                       // 全体のレコード長
            sizeof(SequenceLogItemCore) +                           // シーケンスログアイテム基本データ
            sizeof(int16_t) + item->getClassName()->getLength() +   // クラス名の長さ＋クラス名
            sizeof(int16_t) + item->getFuncName()-> getLength() +   // 関数名の長さ＋関数名
            sizeof(int16_t) + item->getMessage()->  getLength();    // メッセージの長さ＋メッセージ

        // STEP_INの場合はシーケンス番号を更新する
        if (item->mType == SequenceLogItemCore::STEP_IN)
        {
            item->mSeqNo = mSeqNo;
            *seq = mSeqNo++;
        }

        SequenceLogByteBuffer buffer(capacity);
        uint32_t size = buffer.putSequenceLogItem(item);

        // シーケンスログアイテム送信
        mSocket.sendHeader(size, false);
        mSocket.send(&buffer, size);

        // STEP_INの場合はシーケンス番号を受信する
//      if (item->mType == SequenceLogItemCore::STEP_IN)
//      {
//          ByteBuffer buffer(sizeof(uint32_t));
//
//          mSocket.recv(&buffer);
//          *seq = buffer.getInt();
//      }
    }
    catch (Exception e)
    {
        // 異常発生。ソケットを閉じる
        noticeLog("sendItem: %s\n", e.getMessage());
        mSocket.close();
    }

    // 送信済みシーケンスログアイテムを削除
    delete item;
}

/*!
 * \brief   シーケンスログクライアントデリータクラス
 */
class SequenceLogClientDeleter
{
public:     ~SequenceLogClientDeleter()
            {
                delete sClient;
                sClient = nullptr;
            }
};

static SequenceLogClientDeleter s_deleter;

/******************************************************************************
*
* シーケンスログクラス
*
******************************************************************************/

/*!
 * \brief   コンストラクタ
 *
 * \param[in]   className   クラス名
 * \param[in]   funcName    メソッド名
 */
SequenceLog::SequenceLog(const char* className, const char* funcName)
{
    init();
    SequenceLogItem* item = sClient->createItem();

    if (item)
    {
        item->init(mSeqNo, className, funcName);
        sClient->sendItem(item, &mSeqNo);
    }
}

/*!
 * \brief   コンストラクタ
 */
//SequenceLog::SequenceLog(uint32_t classID, const char* funcName)
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
//        noticeLog("(flag:%d) %d::%s\n", mOutputFlag, classID, funcName);
//    }
//#endif
//}

/*!
 * \brief   コンストラクタ
 */
//SequenceLog::SequenceLog(uint32_t classID, uint32_t funcID)
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
//        noticeLog("(flag:%d) %d::%d\n", mOutputFlag, classID, funcID);
//    }
//#endif
//}

/*!
 * \brief   デストラクタ
 */
SequenceLog::~SequenceLog()
{
    SequenceLogItem* item = sClient->createItem();

    if (item)
    {
        item->init(mSeqNo);
        sClient->sendItem(item);
    }
}

/*!
 * \brief   初期化
 */
void SequenceLog::init()
{
    if (sClientInitialized == false)
    {
        sClientInitialized = true;

        sClient = new SequenceLogClient;
        sClient->init();
    }

    mSeqNo = 0;
}

/*!
 * \brief   メッセージ出力
 */
void SequenceLog::message(SequenceLogLevel level, const char* format, ...)
{
    va_list arg;
    va_start(arg, format);

    messageV(level, format, arg);
}

/*!
 * \brief   メッセージ出力
 */
void SequenceLog::messageV(SequenceLogLevel level, const char* format, va_list arg)
{
    SequenceLogItem* item = sClient->createItem();

    if (item)
    {
        item->init(mSeqNo, level);
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
 * \brief   メッセージ出力
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
 * \brief   シーケンスログアイテム取得
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

    if (getPosition() != size)
    {
        e.setMessage("データが異常です(%d, %d)。シーケンスログアイテムを設定できませんでした。", getPosition(), size);
        throw e;
    }
}

/*!
 * \brief   シーケンスログアイテム書き込み
 */
uint32_t SequenceLogByteBuffer::putSequenceLogItem(const SequenceLogItem* item)
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

    // 先頭にレコード長
    size = getPosition();

    setPosition(0);
    putShort(size);

    return size;
}

} // namespace slog

/*!
 * \brief   シーケンスログファイル名を設定する
 */
static void setSequenceLogFileName(const slog::CoreString* fileName)
{
    if (fileName->getLength() <= sizeof(slog::sSequenceLogFileName) - 1)
        strcpy(slog::sSequenceLogFileName, fileName->getBuffer());
}

/*!
 * \brief   シーケンスログサービスのアドレスを設定する
 */
static void setSequenceLogServiceAddress(const slog::CoreString* url)
{
    if (url->getLength() <= sizeof(slog::sSequenceLogServiceAddress) - 1)
        strcpy(slog::sSequenceLogServiceAddress, url->getBuffer());
}

/*!
 * \brief   ユーザー名を設定する
 */
static void setSequenceLogUserName(const slog::CoreString* userName)
{
    if (userName->getLength() <= sizeof(slog::sSequenceLogUserName) - 1)
        strcpy(slog::sSequenceLogUserName, userName->getBuffer());
}

/*!
 * \brief   パスワードを設定する
 */
static void setSequenceLogPassword(const slog::CoreString* passwd)
{
    if (passwd->getLength() <= sizeof(slog::sSequenceLogPasswd) - 1)
        strcpy(slog::sSequenceLogPasswd, passwd->getBuffer());
}

/*!
 * \brief   ログレベルを設定する
 */
static void setLogLevel(const slog::CoreString* logLevel)
{
    if (logLevel->equals("ALL"))   slog::sLogLevel = slog::DEBUG - 1;
    if (logLevel->equals("DEBUG")) slog::sLogLevel = slog::DEBUG;
    if (logLevel->equals("INFO"))  slog::sLogLevel = slog::INFO;
    if (logLevel->equals("WARN"))  slog::sLogLevel = slog::WARN;
    if (logLevel->equals("ERROR")) slog::sLogLevel = slog::ERROR;
    if (logLevel->equals("NONE"))  slog::sLogLevel = slog::ERROR + 1;
}

/*!
 * \brief   シーケンスログコンフィグを読み込む
 */
extern "C" void loadSequenceLogConfig(const char* fileName)
{
    try
    {
        slog::String url;
        slog::String userName;
        slog::String passwd;
        slog::String logFileName;
        slog::String logLevel;

        slog::File file;
        file.open(fileName, slog::File::READ);

        slog::String str;
        slog::String fmt1 = "[key] [value1]";
        slog::Tokenizer tokenizer(&fmt1);

        while (file.read(&str))
        {
            tokenizer.exec(&str);

            const slog::CoreString* key = tokenizer.getValue("key");
            const slog::Variant& value1 = tokenizer.getValue("value1");

            if (key->equals("SEQUENCE_LOG_SERVICE"))
                url.copy(value1);

            if (key->equals("USER_NAME"))
                userName.copy(value1);

            if (key->equals("PASSWORD"))
                passwd.copy(value1);

            if (key->equals("LOG_FILE_NAME"))
                logFileName.copy(value1);

            if (key->equals("LOG_LEVEL"))
                logLevel.copy(value1);
        }

        setSequenceLogServiceAddress(&url);
        setSequenceLogUserName(&userName);
        setSequenceLogPassword(&passwd);
        setSequenceLogFileName(&logFileName);
        setLogLevel(&logLevel);
    }
    catch (slog::Exception e)
    {
        noticeLog("%s", e.getMessage());
    }
}
