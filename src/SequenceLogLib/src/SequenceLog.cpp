/*
 * Copyright (C) 2011 log-tools.net
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
 *  \author Copyright 2011 log-tools.net
 */
//nclude "SequenceLog.h"
#include "SequenceLogItem.h"

#include "slog/Mutex.h"
#include "slog/Socket.h"
#include "slog/SharedMemory.h"
#include "slog/TimeSpan.h"
#include "slog/Process.h"

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
    void* _slog_stepIn(const char* className, const char* funcName, int32_t               outputFlag)
    {
        slog::SequenceLog* slog = new slog::SequenceLog(className, funcName, (slog::SequenceLogOutputFlag)outputFlag);
        return slog;
    }

    /*!
     *  \brief  ステップイン時のログ出力
     */
    void* _slog_stepIn2(uint32_t classID, const char* funcName, int32_t outputFlag)
    {
        slog::SequenceLog* slog = new slog::SequenceLog(classID, funcName, (slog::SequenceLogOutputFlag)outputFlag);
        return slog;
    }

    /*!
     *  \brief  ステップイン時のログ出力
     */
    void* _slog_stepIn3(uint32_t classID, uint32_t funcID, int32_t outputFlag)
    {
        slog::SequenceLog* slog = new slog::SequenceLog(classID, funcID, (slog::SequenceLogOutputFlag)outputFlag);
        return slog;
    }

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
    void  _slog_message2(void* p, int32_t level, uint32_t messageID)
    {
        slog::SequenceLog* slog = (slog::SequenceLog*)p;
        slog->message((slog::SequenceLogLevel)level, messageID);
    }
}

/*-----------------------------------------------------------------------------
|
| 静的メンバー変数定義
|
+----------------------------------------------------------------------------*/
namespace slog
{

static char                     sSequenceLogFileName[MAX_PATH + 1] = "";    //!< シーケンスログファイル名
static SequenceLogOutputFlag    sRootFlag = ROOT;

class  SequenceLogClient;
static SequenceLogClient*       sClient = NULL;                             //!< シーケンスログクライアントオブジェクト
static bool                     sClientInitialized = false;                 //!< 初期化フラグ

/*!
 *  \brief  シーケンスログソケットクラス
 */
class SequenceLogSocket : public Socket
{
            FixedString<MAX_PATH>   mShmName;       //!< 共有メモリ名

public:     void canUseService() const throw(Exception);
            const CoreString& recvSharedMemoryName() throw(Exception);
};

/*!
 *  \brief  シーケンスログサービスが使用可能か確認する
 */
void SequenceLogSocket::canUseService() const throw(Exception)
{
#if defined(__unix__)
    Exception e;

    int32_t len = sizeof(pthread_mutex_t);
    send(&len);

    int32_t canUse = 0;
    recv(&canUse);

    if (canUse == 0)
    {
#if defined(__x86_64)
        printf("slogsvc(32 bits) can't use, because this process is 64 bits.\n");
#else
        printf("slogsvc(64 bits) can't use, because this process is 32 bits.\n");
#endif

        e.setMessage("SequenceLogSocket::canUseService() / can not use");
        throw e;
    }

    if (canUse != 1)
    {
        e.setMessage("SequenceLogSocket::canUseService() / receive illegal value(%d)", canUse);
        throw e;
    }
#endif
}

/*!
 *  \brief  共有メモリ名受信
 */
const CoreString& SequenceLogSocket::recvSharedMemoryName() throw(Exception)
{
    int32_t len = 0;

    recv(&len);
    recv(&mShmName, len);

    if (mShmName[len - 1] != '\0')
    {
        Exception e;
        e.setMessage("SequenceLogSocket::recvSharedMemoryName() / receive name is not null-terminated");

        throw e;
    }

    return mShmName;
}

/*!
 *  \brief  シーケンスログ共有メモリクラス
 */
class SequenceLogSharedMemory : public SharedMemory<SLOG_SHM*>
{
public:     void validate() const throw(Exception);

            SLOG_ITEM_INFO* getSequenceLogItem(uint32_t threadId, uint32_t bufferIndex, uint32_t* seq) const;
};

/*!
 *  \brief  共有メモリが有効か確認する
 */
void SequenceLogSharedMemory::validate() const throw(Exception)
{
    if (getSize() < sizeof(SLOG_SHM) + sizeof(SequenceLogItem) * ((*this)->count - 1))
    {
        Exception e;
        e.setMessage("SequenceLogSharedMemory::validate() / illegal shared memory size");

        throw e;
    }
}

/*!
 *  \brief  シーケンスログアイテム取得
 */
SLOG_ITEM_INFO* SequenceLogSharedMemory::getSequenceLogItem(uint32_t threadId, uint32_t bufferIndex, uint32_t* seq) const
{
    SLOG_SHM* shm = getBuffer();
    SLOG_SHM_HEADER* header = &shm->header[bufferIndex];

    if (header->index >= shm->count)
        return NULL;

    SLOG_ITEM_INFO* infoArray = &shm->infoArray[bufferIndex * shm->count];
    uint32_t index = header->index;
    index = infoArray[index].no;

    SLOG_ITEM_INFO* info = &infoArray[index];
    header->index++;

    info->item.setCurrentDateTime();
    info->item.mThreadId = threadId;

    if (seq)
    {
        *seq = header->seq;
        header->seq++;
    }

    return info;
}

/*!
 *  \brief  シーケンスログクライアントクラス
 */
class SequenceLogClient
{
            SequenceLogSocket       mSocket;                        //!< ソケット
            SequenceLogSharedMemory mSHM;                           //!< 共有メモリ
            Mutex*                  mMutex[SLOG_SHM::BUFFER_COUNT]; //!< ミューテックス

public:      SequenceLogClient();
            ~SequenceLogClient();

            void init();

public:     SLOG_ITEM_INFO* lock(uint32_t* seq);
};

/*!
 *  \brief  コンストラクタ
 */
inline SequenceLogClient::SequenceLogClient()
{
//  TRACE("[S] SequenceLogClient::SequenceLogClient()\n", 0);

    Socket::startup();

    for (int32_t index = 0; index < SLOG_SHM::BUFFER_COUNT; index++)
        mMutex[index] = NULL;

//  TRACE("[E] SequenceLogClient::SequenceLogClient()\n", 0);
}

/*!
 *  \brief  デストラクタ
 */
inline SequenceLogClient::~SequenceLogClient()
{
//  TRACE("[S] SequenceLogClient::~SequenceLogClient()\n", 0);

    for (int32_t index = 0; index < SLOG_SHM::BUFFER_COUNT; index++)
        delete mMutex[index];

    mSocket.close();

    Socket::cleanup();
//  TRACE("[E] SequenceLogClient::~SequenceLogClient()\n", 0);
}

/*!
 *  \brief  初期化
 */
void SequenceLogClient::init()
{
    TRACE("[S] SequenceLogClient::init()\n", 0);
    noticeLog("client initialize.\n");

    // シーケンスログファイル名取得
    const char* p = sSequenceLogFileName;
    TRACE("    sSequenceLogFileName='%s'\n", p);

    if (p[0] == '\0')
    {
        TRACE("    getSequenceLogFileName() calling...\n", 0);
        p = getSequenceLogFileName();
    }

    if (p == NULL || p[0] == '\0')
    {
        TRACE("[E] SequenceLogClient::init() - failed\n", 0);
        return;
    }

    FixedString<MAX_PATH> name = p;
    int32_t len = name.getLength() + 1;

    try
    {
        // 初期化処理のためのソケット作成
#if defined(__ANDROID__)
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
        {
#endif
            mSocket.open();
            mSocket.setRecvTimeOut(3000);
            mSocket.connect(FixedString<16>("127.0.0.1"), SERVICE_PORT);
#if defined(__ANDROID__)
        }
#endif

        // プロセスID送信
        Process process;
        uint32_t pid = process.getId();
        mSocket.send(&pid);

        // シーケンスログサービスが使用可能か調べる
        mSocket.canUseService();

        // シーケンスログファイル名送信
        mSocket.send(&len);
        mSocket.send(&name, len);

        // 共有メモリ名受信
        const CoreString& shmName = mSocket.recvSharedMemoryName();
        TRACE("    shmName='%s'\n", shmName.getBuffer());

        // 共有メモリ取得
        mSHM.open(shmName);
        mSHM.validate();

        // ミューテックス取得
        for (int32_t index = 0; index < SLOG_SHM::BUFFER_COUNT; index++)
        {
#if defined(_WINDOWS)
            name.format("slogMutex%d-%d", pid, index);
            mMutex[index] = new Mutex(false, name);
#else
            mMutex[index] = new Mutex(false, &mSHM->header[index].mutex);
#endif
        }
    }
    catch (Exception e)
    {
        noticeLog("%s\n", e.getMessage());
        mSocket.close();
    }

    TRACE("[E] SequenceLogClient::init()\n", 0);
}

/*!
 *  \brief  ロック
 */
SLOG_ITEM_INFO* SequenceLogClient::lock(uint32_t* seq)
{
    if (this == NULL)
        return NULL;

    if (mSocket.isOpen() == false)
        return NULL;

    static const uint32_t TIMEOUT = 1000;
    TimeSpan timeSpan1;
    SLOG_ITEM_INFO* info;

    while (true)
    {
        uint32_t threadId = Thread::getCurrentId();
        uint32_t bufferIndex = (threadId % SLOG_SHM::BUFFER_COUNT);

        ScopedLock lock(mMutex[bufferIndex]);
        info = mSHM.getSequenceLogItem(threadId, bufferIndex, seq);

        if (info)
            break;

#if 0
        TimeSpan timeSpan2;

        if (timeSpan2 - timeSpan1 > TIMEOUT)
        {
            // 1000ミリ秒以上ログ出力出来なかったので以降のログ出力をキャンセルする
            TRACE("    SequenceLogClient::lock() %d > %d ms\n", timeSpan2 - timeSpan1, TIMEOUT);
            noticeLog("Could not log output more than %d ms, the logging stopped.\n",  TIMEOUT);

            mSocket.close();
            return NULL;
        }
#endif
    }

#if 1
    TimeSpan timeSpan2;

    if (timeSpan2 - timeSpan1 > TIMEOUT)
        noticeLog("Log output elapsed time: %d ms\n", timeSpan2 - timeSpan1);
#endif

    return info;
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
    const char* funcName,               //!< メソッド名
    SequenceLogOutputFlag outputFlag)   //!< 出力フラグ
{
    init(outputFlag);
    SLOG_ITEM_INFO* info = sClient->lock(&mSeqNo);

    if (info)
    {
        info->item.init(mSeqNo, mOutputFlag, className, funcName);
        info->ready = true;
    }
}

/*!
 *  \brief  コンストラクタ
 */
SequenceLog::SequenceLog(uint32_t classID, const char* funcName, SequenceLogOutputFlag outputFlag)
{
    init(outputFlag);
    SLOG_ITEM_INFO* info = sClient->lock(&mSeqNo);

    if (info)
    {
        info->item.init(mSeqNo, mOutputFlag, classID, funcName);
        info->ready = true;
    }
}

/*!
 *  \brief  コンストラクタ
 */
SequenceLog::SequenceLog(uint32_t classID, uint32_t funcID, SequenceLogOutputFlag outputFlag)
{
    init(outputFlag);
    SLOG_ITEM_INFO* info = sClient->lock(&mSeqNo);

    if (info)
    {
        info->item.init(mSeqNo, mOutputFlag, classID, funcID);
        info->ready = true;
    }
}

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
    }
}

/*!
 *  \brief  メッセージ出力
 */
void SequenceLog::message(SequenceLogLevel level, uint32_t messageID)
{
    SLOG_ITEM_INFO* info = sClient->lock(NULL);

    if (info)
    {
        info->item.init(mSeqNo, mOutputFlag, level);
        info->item.mMessageId = messageID;
        info->ready = true;
    }
}

} // namespace slog

/*!
 *  \brief  シーケンスログファイル名を設定する
 */
extern "C" void setSequenceLogFileName(const char* fileName)
{
    strcpy(slog::sSequenceLogFileName, fileName);
}

/*!
 *  \brief  ROOTの既定値を設定する
 */
extern "C" void setRootFlag(int32_t outputFlag)
{
    slog::sRootFlag = (slog::SequenceLogOutputFlag)outputFlag;
}
