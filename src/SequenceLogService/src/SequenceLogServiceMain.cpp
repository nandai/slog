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
 *  \file   SequenceLogServiceMain.cpp
 *  \brief  シーケンスログサービスメインクラス
 *  \author Copyright 2011-2014 printf.jp
 */
#include "SequenceLogServiceMain.h"
#include "SequenceLogService.h"
#include "SequenceLogServiceWebServer.h"
#include "SequenceLogServiceWebServerResponse.h"
#include "SharedFileContainer.h"

#include "slog/Mutex.h"
#include "slog/FileInfo.h"

#if defined(__unix__)
    #include <string.h>
    #include <algorithm>
#endif

static const char* CLS_NAME = "SequenceLogServiceMain";

namespace slog
{
static SequenceLogServiceMain* sServiceMain = nullptr;

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
            info1->getCanonicalPath()->getBuffer(),
            info2->getCanonicalPath()->getBuffer());

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
    SLOG(CLS_NAME, "SequenceLogServiceMain");

    sServiceMain = this;

    mMaxFileSize = 1024 * 4;
    mMaxFileCount = 10;

    mCleanupFlag = false;

    mMutex = new Mutex;
    ScopedLock lock(mMutex, false);

    mStartRunTime = false;
    mOutputScreen = true;

    mWebServerManager.setWebServer(new SequenceLogServiceWebServer, false);

#if !defined(__ANDROID__)
    mWebServerManager.setWebServer(new SequenceLogServiceWebServer, true);
#endif
}

/*!
 *  \brief  デストラクタ
 */
SequenceLogServiceMain::~SequenceLogServiceMain()
{
    SLOG(CLS_NAME, "~SequenceLogServiceMain");

    deleteFileInfoArray();
    delete mMutex;
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
    // シーケンスログファイル情報登録
    mFileInfoArray.push_back(info);

    // ログファイルの上限と現在のファイル数を取得
    int32_t maxCount = getMaxFileCount();
    int32_t size = (int32_t)mFileInfoArray.size();

    // 古いファイルを削除
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
                // 使用されていない（オープンされていない）場合

                // ファイルを削除
                File::unlink(info->getCanonicalPath());

                // リストからファイル情報を除外
                i = mFileInfoArray.erase(i);

                // ファイル情報を削除
                delete info;
            }
            else
            {
                // 使用中
                i++;
            }
        }
        catch (Exception e)
        {
            e;
//          noticeLog("    %s\n", e.getMessage());
            i++;
        }

        size--;
    }
}

/*!
 *  \brief  シーケンスログサービスメインスレッド
 */
void SequenceLogServiceMain::run()
{
    SLOG(CLS_NAME, "run");
    mWebServerManager.start();

    while (isInterrupted() == false)
        sleep(2000);

    mWebServerManager.stop();
    cleanup();
}

/*!
 * \brief   onResponseStart
 */
void SequenceLogServiceMain::onResponseStart(WebServerResponse* response)
{
    SLOG(CLS_NAME, "onResponseStart");
    response->setListener(this);
}

/*!
 * \brief   クリーンアップ
 */
void SequenceLogServiceMain::cleanup()
{
    SLOG(CLS_NAME, "cleanup");
    mCleanupFlag = true;
}

/*!
 * \brief   インスタンス取得
 */
SequenceLogServiceMain* SequenceLogServiceMain::getInstance()
{
    return sServiceMain;
}

/*!
 * \brief   シーケンスログプリントに出力（送信）
 */
void SequenceLogServiceMain::printLog(const Buffer* text, int32_t len)
{
#if !defined(__ANDROID__) || defined(__EXEC__)
    onUpdateLog(text);
#endif
}

/*!
 *  \brief  共有ファイルコンテナ取得
 */
SharedFileContainer* SequenceLogServiceMain::getSharedFileContainer(const CoreString& baseFileName)
{
    SharedFileContainer* container;

    for (SharedFileContainerArray::iterator i = mSharedFileContainerArray.begin(); i != mSharedFileContainerArray.end(); i++)
    {
        container = *i;

        if (container->getBaseFileName()->equals(baseFileName))
        {
            // ベースファイル名が同じだったら既存の共有ファイルコンテナを返す
            container->addReference();
            return container;
        }
    }

    // 該当する共有ファイルコンテナがなかったら新規作成
    container = new SharedFileContainer();
    container->getBaseFileName()->copy(baseFileName);

    mSharedFileContainerArray.push_back(container);
    return container;
}

/*!
 *  \brief  共有ファイルコンテナリリース
 */
void SequenceLogServiceMain::releaseSharedFileContainer(SharedFileContainer* container)
{
    if (container == nullptr)
        return;

    if (container->removeReference() == false)
        return;

    // 共有ファイルクローズ
    container->getFile()->close();

    // 共有ファイル情報更新
    FileInfo* fileInfo = container->getFileInfo();

    if (fileInfo)
        fileInfo->update();

    // リストから除外
    SharedFileContainerArray::iterator i = std::find(mSharedFileContainerArray.begin(), mSharedFileContainerArray.end(), container);
    mSharedFileContainerArray.erase(i);

    // 共有ファイルコンテナ削除
    delete container;
}

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
    find.exec(&path);

    path.format("%s%c*.log",  mLogFolderName.getBuffer(), PATH_DELIMITER);
    find.exec(&path);

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
uint16_t SequenceLogServiceMain::getWebServerPort(bool secure) const
{
    auto webServer = mWebServerManager.getWebServer(secure);

    if (webServer)
        return webServer->getPort();

    return 0;
}

/*!
 *  \brief  シーケンスログWEBサーバーポート設定
 */
void SequenceLogServiceMain::setWebServerPort(bool secure, uint16_t port)
{
    auto webServer = mWebServerManager.getWebServer(secure);

    if (webServer)
        webServer->setPort(port);
}

/*!
 * SSL関連
 */
void SequenceLogServiceMain::setSSLFileName(const CoreString* certificate, const CoreString* privateKey)
{
    auto webServer = mWebServerManager.getWebServer(true);

    if (webServer)
        webServer->setSSLFileName(certificate, privateKey);
}

/*!
 * 証明書ファイル名取得
 */
const CoreString* SequenceLogServiceMain::getCertificateFileName() const
{
    auto webServer = mWebServerManager.getWebServer(true);

    if (webServer)
        return webServer->getCertificateFileName();

    return nullptr;
}

/*!
 * プライベート機ファイル名取得
 */
const CoreString* SequenceLogServiceMain::getPrivateKeyFileName() const
{
    auto webServer = mWebServerManager.getWebServer(true);

    if (webServer)
        return webServer->getPrivateKeyFileName();

    return nullptr;
}

/*!
 *  \brief  シーケンスログサーバーポート取得
 */
uint16_t SequenceLogServiceMain::getSequenceLogServerPort() const
{
    return mSequenceLogServerPort;
}

/*!
 *  \brief  シーケンスログサーバーポート設定
 */
void SequenceLogServiceMain::setSequenceLogServerPort(uint16_t port)
{
    mSequenceLogServerPort = port;
}

/*!
 *  \brief  ファイル検索
 */
void SequenceLogServiceMain::onFind(const CoreString& path)
{
    FileInfo* info = new FileInfo(&path);

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
    SLOG(CLS_NAME, "onInitialized");
    SequenceLogService* response = dynamic_cast<SequenceLogService*>(thread);

    if (response == nullptr)
        return;

    ThreadListeners* listeners = getListeners();

    for (ThreadListeners::iterator i = listeners->begin(); i != listeners->end(); i++)
    {
        SequenceLogServiceThreadListener* listener = dynamic_cast<SequenceLogServiceThreadListener*>(*i);

        if (listener)
            listener->onInitialized(thread);
    }
}

/*!
 *  \brief	シーケンスログサービススレッド終了通知
 */
void SequenceLogServiceMain::onTerminated(Thread* thread)
{
    SLOG(CLS_NAME, "onTerminated");
    SequenceLogService* response = dynamic_cast<SequenceLogService*>(thread);

    if (response == nullptr)
        return;

    if (mCleanupFlag)
        return;

    ThreadListeners* listeners = getListeners();

    for (ThreadListeners::iterator i = listeners->begin(); i != listeners->end(); i++)
    {
        SequenceLogServiceThreadListener* listener = dynamic_cast<SequenceLogServiceThreadListener*>(*i);

        if (listener)
            listener->onTerminated(thread);
    }
}

/*!
 *  \brief	シーケンスログファイル変更通知
 */
void SequenceLogServiceMain::onLogFileChanged(Thread* thread)
{
    SLOG(CLS_NAME, "onLogFileChanged");
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
//  SLOG(CLS_NAME, "onUpdateLog");
    ThreadListeners* listeners = getListeners();

    for (ThreadListeners::iterator i = listeners->begin(); i != listeners->end(); i++)
    {
        SequenceLogServiceThreadListener* listener = dynamic_cast<SequenceLogServiceThreadListener*>(*i);

        if (listener)
            listener->onUpdateLog(text);
    }
}

} // namespace slog
