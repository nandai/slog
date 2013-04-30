﻿/*
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
 *  \file   SequenceLogServiceMain.cpp
 *  \brief  シーケンスログサービスメインクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "SequenceLogServiceMain.h"
#include "SequenceLogService.h"
#include "SequenceLogServiceWebServer.h"

#include "slog/Mutex.h"
#include "slog/TimeSpan.h"
#include "slog/FileInfo.h"
#include "slog/Util.h"

namespace slog
{
static SequenceLogServiceMain* sServiceMain = NULL;

/*!
 *  \brief  シーケンスログファイル情報比較
 */
struct CompareFileInfo
{
    bool operator() (const FileInfo* info1, const FileInfo* info2) const
    {
        uint64_t value1 = info1->getLastWriteTime().getValue();
        uint64_t value2 = info2->getLastWriteTime().getValue();

        if (value1 < value2)
            return true;

        if (value1 > value2)
            return false;

        value1 = info1->getCreationTime().getValue();
        value2 = info2->getCreationTime().getValue();

        if (value1 < value2)
            return true;

        if (value1 > value2)
            return false;

        int32_t result = strcmp(
            info1->getCanonicalPath().getBuffer(),
            info2->getCanonicalPath().getBuffer());

        if (result < 0)
            return true;

        return false;
    }
};

/*!
 *  \brief  コンストラクタ
 */
SequenceLogServiceMain::SequenceLogServiceMain()
{
    sServiceMain = this;

    mSharedMemoryItemCount = 100;

    mMaxFileSize = 1024 * 4;
    mMaxFileCount = 10;
    mWebServerPort = 0;

    mMutex = new Mutex;
    ScopedLock lock(mMutex, false);

    mStartRunTime = false;
    mOutputScreen = true;

    mWebServer = new SequenceLogServiceWebServerThread;
}

/*!
 *  \brief  デストラクタ
 */
SequenceLogServiceMain::~SequenceLogServiceMain()
{
    deleteFileInfoArray();

    delete mMutex;
    delete mWebServer;
}

/*!
 *  \brief  シーケンスログファイル情報削除
 */
void SequenceLogServiceMain::deleteFileInfoArray()
{
    for (FileInfoArray::iterator i = mFileInfoArray.begin(); i != mFileInfoArray.end(); i++)
        delete *i;

    mFileInfoArray.clear();
}

/*!
 *  \brief  シーケンスログファイル情報登録
 */
void SequenceLogServiceMain::addFileInfo(FileInfo* info)
{
    TRACE("[S] SequenceLogServiceMain::addFileInfo()\n", 0);
    mFileInfoArray.push_back(info);

    int32_t maxCount = getMaxFileCount();
    int32_t size = (int32_t)mFileInfoArray.size();

    FileInfoArray::iterator i = mFileInfoArray.begin();

    while (i != mFileInfoArray.end())
    {
        if (maxCount == 0 || size <= maxCount)
            break;

        try
        {
            FileInfo* info = *i;

            if (info->isUsing() == false)
            {
                File::unlink(info->getCanonicalPath());

                i = mFileInfoArray.erase(i);
                delete info;
            }
            else
            {
                i++;
            }
        }
        catch (Exception e)
        {
            TRACE("    %s\n", e.getMessage());
            i++;
        }

        size--;
    }

    TRACE("[E] SequenceLogServiceMain::addFileInfo()\n", 0);
}

/*!
 *  \brief  シーケンスログサービスメインスレッド
 */
void SequenceLogServiceMain::run()
{
    TRACE("[S] SequenceLogServiceMain::run()\n", 0);
    Socket* socket = NULL;

    try
    {
        // サーバーソケット作成
#if !defined(__ANDROID__) || !defined(__EXEC__) || 1
        mSocket.open();
        mSocket.setReUseAddress(true);

        mSocket.bind(SERVICE_PORT);
        mSocket.listen();
#else
        mSocket.open(false);
        mSocket.setReUseAddress(true);

        mSocket.bind(FixedString<108>("/dev/socket/slog"));
        mSocket.listen();
#endif
    }
    catch (Exception e)
    {
        noticeLog("%s\n", e.getMessage());
        cleanup();
        return;
    }

    while (true)
    {
        try
        {
            bool isReceive = mSocket.isReceiveData(3000);

            if (isInterrupted())
                break;

            if (isReceive == false)
                continue;

            socket = new Socket();
            socket->accept(&mSocket);

            SequenceLogService* service = new SequenceLogService(socket);
            mServiceManager.push_back(service);

            service->setListener(this);
            service->start();
        }
        catch (Exception /*e*/)
        {
            delete socket;
        }

        socket = NULL;
    }

    cleanup();
    TRACE("[E] SequenceLogServiceMain::run()\n", 0);
}

/*!
 *  \brief  クリーンアップ
 */
void SequenceLogServiceMain::cleanup()
{
    long index = 0;

    do
    {
        index = 0;

        for (SequenceLogServiceManager::iterator i = mServiceManager.begin(); i != mServiceManager.end(); i++)
        {
            SequenceLogService* service = *i;

            if (service->isAlive())
            {
                index++;
                service->interrupt();
            }
        }
    }
    while (index);

    for (SequenceLogServiceManager::iterator i = mServiceManager.begin(); i != mServiceManager.end(); i++)
        delete *i;

    mServiceManager.clear();
    mSocket.close();
}

/*!
 *  \brief  インスタンス取得
 */
SequenceLogServiceMain* SequenceLogServiceMain::getInstance()
{
    return sServiceMain;
}

/*!
 *  \brief  接続スレッド
 */
class ConnectThread : public Thread, public ThreadListener
{
            Socket* mSocket;        //!< ソケット
            String  mIp;            //!< IPアドレス

public:     ConnectThread(Socket* socket, const CoreString& ip);

private:    virtual void run();
            virtual void onTerminated( Thread* thread);
};

/*!
 *  \brief  コンストラクタ
 */
ConnectThread::ConnectThread(Socket* socket, const CoreString& ip)
{
    mSocket = socket;
    mIp.copy(ip);
    setListener(this);
}

/*!
 *  \brief  実行
 */
void ConnectThread::run()
{
    TRACE("[S] ConnectThread::run\n", 0);

    try
    {
        mSocket->open(true, SOCK_DGRAM);
        mSocket->connect(mIp, 59108);
    }
    catch (Exception e)
    {
        TRACE("    %s\n", e.getMessage());
        mSocket->close();
    }

    TRACE("[E] ConnectThread::run\n", 0);
}

/*!
 *  \brief  スレッド終了通知
 */
void ConnectThread::onTerminated( Thread* thread)
{
    delete this;
}

/*!
 *  \brief  シーケンスログプリントに出力（送信）
 */
void SequenceLogServiceMain::printLog(const Buffer* text, int32_t len)
{
#if !defined(__ANDROID__) || defined(__EXEC__)
    onUpdateLog(text);
#endif
}

/*!
 *  \brief 共有メモリパス取得
 */
const CoreString& SequenceLogServiceMain::getSharedMemoryPathName()
{
    return mSharedMemoryPathName;
}

#if defined(__unix__)
/*!
 *  \brief 共有メモリパス設定
 */
void SequenceLogServiceMain::setSharedMemoryPathName(
    const CoreString& pathName) //!< パス名
{
    mSharedMemoryPathName.format("%s/", pathName.getBuffer());
}
#endif

/*!
 *  \brief  シーケンスログフォルダ名設定
 */
void SequenceLogServiceMain::setLogFolderName(const CoreString& name)
{
    deleteFileInfoArray();
    mLogFolderName.copy(name);

    FixedString<MAX_PATH> path;
    FileFind find;
    find.setListener(this);

    path.format("%s%c*.slog", mLogFolderName.getBuffer(), PATH_DELIMITER);
    find.exec(path);

    path.format("%s%c*.log",  mLogFolderName.getBuffer(), PATH_DELIMITER);
    find.exec(path);

    mFileInfoArray.sort(CompareFileInfo());
}

/*!
 *  \brief  最大ファイルサイズ取得
 */
uint32_t SequenceLogServiceMain::getMaxFileSize() const
{
    return mMaxFileSize;
}

/*!
 *  \brief  最大ファイルサイズ設定
 */
void SequenceLogServiceMain::setMaxFileSize(uint32_t size)
{
    mMaxFileSize = size;
}

/*!
 *  \brief  最大ファイル数取得
 */
int32_t SequenceLogServiceMain::getMaxFileCount() const
{
    return mMaxFileCount;
}

/*!
 *  \brief  最大ファイル数設定
 */
void SequenceLogServiceMain::setMaxFileCount(int32_t count)
{
    mMaxFileCount = count;
}

/*!
 *  \brief  シーケンスログWEBサーバーポート取得
 */
uint16_t SequenceLogServiceMain::getWebServerPort() const
{
    return mWebServerPort;
}

/*!
 *  \brief  シーケンスログWEBサーバーポート設定
 */
void SequenceLogServiceMain::setWebServerPort(uint16_t port)
{
    if (mWebServerPort == port)
        return;

    if (mWebServer->isAlive())
    {
//      Util::stopThread(mWebServer, mWebServerPort);
        mWebServer->interrupt();
        mWebServer->join();
    }

    mWebServerPort = port;
    mWebServer->setPort(port);
    mWebServer->start();
}

/*!
 *  \brief  シーケンスログサーバーIP取得
 */
const CoreString& SequenceLogServiceMain::getSequenceLogServerIP() const
{
    return mSequenceLogServerIp;
}

/*!
 *  \brief  シーケンスログサーバーポート取得
 */
uint16_t SequenceLogServiceMain::getSequenceLogServerPort() const
{
    return mSequenceLogServerPort;
}

/*!
 *  \brief  シーケンスログサーバーのIPとポートを設定する
 */
void SequenceLogServiceMain::setSequenceLogServer(const CoreString& ip, uint16_t port)
{
    mSequenceLogServerIp.copy(ip);
    mSequenceLogServerPort = port;
}

/*!
 *  \brief  ファイル検索
 */
void SequenceLogServiceMain::onFind(const CoreString& path)
{
    FileInfo* info = new FileInfo(path);

    if (info->isFile() == false)
    {
        delete info;
        return;
    }

    mFileInfoArray.push_back(info);
}

/*!
 *  \brief	シーケンスログサービススレッド初期化完了通知
 */
void SequenceLogServiceMain::onInitialized(Thread* thread)
{
    ThreadListeners* listeners = getListeners();

    for (ThreadListeners::iterator i = listeners->begin(); i != listeners->end(); i++)
        (*i)->onInitialized(thread);
}

/*!
 *  \brief	シーケンスログサービススレッド終了通知
 */
void SequenceLogServiceMain::onTerminated(Thread* thread)
{
    ThreadListeners* listeners = getListeners();

    for (ThreadListeners::iterator i = listeners->begin(); i != listeners->end(); i++)
        (*i)->onTerminated(thread);
}

/*!
 *  \brief	シーケンスログファイル変更通知
 */
void SequenceLogServiceMain::onLogFileChanged(Thread* thread)
{
    ThreadListeners* listeners = getListeners();

    for (ThreadListeners::iterator i = listeners->begin(); i != listeners->end(); i++)
    {
        SequenceLogServiceThreadListener* listener = dynamic_cast<SequenceLogServiceThreadListener*>(*i);

        if (listener)
            listener->onLogFileChanged(thread);
    }
}

/*!
 *  \brief	シーケンスログ更新通知
 */
void SequenceLogServiceMain::onUpdateLog(const Buffer* text)
{
    ThreadListeners* listeners = getListeners();

    for (ThreadListeners::iterator i = listeners->begin(); i != listeners->end(); i++)
    {
        SequenceLogServiceThreadListener* listener = dynamic_cast<SequenceLogServiceThreadListener*>(*i);

        if (listener)
            listener->onUpdateLog(text);
    }
}

} // namespace slog
