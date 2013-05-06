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
#include "slog/WebServerResponseThread.h"

#if !defined(MODERN_UI)
#include "slog/SharedMemory.h"
#endif

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

public:      SequenceLogClient();
            ~SequenceLogClient();

            void init();

public:     SLOG_ITEM_INFO* lock(uint32_t* seq);
            void sendItem(SLOG_ITEM_INFO* info, uint32_t* seq);
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
            mSocket.connect(address, SERVICE_PORT);
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
 *  \brief  ロック
 */
SLOG_ITEM_INFO* SequenceLogClient::lock(uint32_t* seq)
{
    SLOG_ITEM_INFO* info = NULL;

    if (this == NULL)
        return NULL;

    if (mSocket.isOpen() == false)
        return NULL;

    info = new SLOG_ITEM_INFO;
    info->item.setCurrentDateTime();
    info->item.mThreadId = Thread::getCurrentId();

    return info;
}

/*!
 *  \brief  シーケンスログアイテム送信
 */
void SequenceLogClient::sendItem(SLOG_ITEM_INFO* info, uint32_t* seq)
{
    try
    {
        ScopedLock lock(mSocketMutex);

        // シーケンスログアイテム送信
        WebServerResponseThread::sendWebSocketHeader(&mSocket, sizeof(info->item), false, false);
        mSocket.send((char*)&info->item, sizeof(info->item));

        // STEP_INの場合はシーケンス番号を受信する
        if (info->item.mType == SequenceLogItemCore::STEP_IN)
        {
            ByteBuffer buffer(sizeof(uint32_t));

            WebServerResponseThread::recvData(&mSocket, &buffer);
            *seq = buffer.getInt();
        }
    }
    catch (Exception e)
    {
        noticeLog("%s\n", e.getMessage());
        mSocket.close();
    }

    delete info;
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
    SLOG_ITEM_INFO* info = sClient->lock(&mSeqNo);

    if (info)
    {
        info->item.init(mSeqNo, mOutputFlag, className, funcName);
        info->ready = true;
        sClient->sendItem(info, &mSeqNo);
    }
#if defined(_DEBUG)
    else
    {
        TRACE("(flag:%d) %s::%s\n", mOutputFlag, className, funcName);
    }
#endif
}

/*!
 *  \brief  コンストラクタ
 */
//SequenceLog::SequenceLog(uint32_t classID, const char* funcName)//, SequenceLogOutputFlag outputFlag)
//{
//    SequenceLogOutputFlag outputFlag = ROOT;
//    init(outputFlag);
//    SLOG_ITEM_INFO* info = sClient->lock(&mSeqNo);
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
//    SLOG_ITEM_INFO* info = sClient->lock(&mSeqNo);
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
    SLOG_ITEM_INFO* info = sClient->lock(NULL);

    if (info)
    {
        info->item.init(mSeqNo, mOutputFlag);
        info->ready = true;
        sClient->sendItem(info, NULL);
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
    SLOG_ITEM_INFO* info = sClient->lock(NULL);

    if (info)
    {
        info->item.init(mSeqNo, mOutputFlag, level);
        info->item.mMessageId = 0;

        try
        {
            PointerString _Message = info->item.getMessage();
            _Message.formatV(format, arg);
        }
        catch (Exception /*e*/)
        {
            // 何もしない
        }

        info->ready = true;
        sClient->sendItem(info, NULL);
    }
}

/*!
 *  \brief  メッセージ出力
 */
//void SequenceLog::message(SequenceLogLevel level, uint32_t messageID)
//{
//    SLOG_ITEM_INFO* info = sClient->lock(NULL);
//
//    if (info)
//    {
//        info->item.init(mSeqNo, mOutputFlag, level);
//        info->item.mMessageId = messageID;
//        info->ready = true;
//        sClient->sendItem(info, NULL);
//    }
//}

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
extern "C" void setSequenceLogServiceAddress(const char* address)
{
    if (strlen(address) <= sizeof(slog::sSequenceLogServiceAddress) - 1)
    {
        strcpy(slog::sSequenceLogServiceAddress, address);
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
