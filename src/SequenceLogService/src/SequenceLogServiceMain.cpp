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
#include "SequenceLogFileManager.h"
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
 *  \brief  コンストラクタ
 */
SequenceLogServiceMain::SequenceLogServiceMain()
{
    SLOG(CLS_NAME, "SequenceLogServiceMain");

    sServiceMain = this;

    mMaxFileSize = 1024 * 4;
    mMaxFileCount = 10;

    mSequenceLogFileManagerList = new SequenceLogFileManagerList;

    mCleanupFlag = false;

    mMutex = new Mutex;
    ScopedLock lock(mMutex, false);

    mStartRunTime = false;

    mWebServerManager.setWebServer(new SequenceLogServiceWebServer, false);

#if !defined(__ANDROID__)
    mWebServerManager.setWebServer(new SequenceLogServiceWebServer, true);
#endif
}

/*!
 * \brief   デストラクタ
 */
SequenceLogServiceMain::~SequenceLogServiceMain()
{
    SLOG(CLS_NAME, "~SequenceLogServiceMain");

    delete mSequenceLogFileManagerList;
    delete mMutex;
}

/*!
 * \brief   
 */
SequenceLogFileManager* SequenceLogServiceMain::getSequenceLogFileManager(int32_t userId) const
{
    SequenceLogFileManager* sequenceLogFileManager = mSequenceLogFileManagerList->get(userId);

    if (sequenceLogFileManager == nullptr)
    {
        sequenceLogFileManager = new SequenceLogFileManager(userId, mMaxFileSize, mMaxFileCount);
        sequenceLogFileManager->enumFileInfoList(&mLogFolderName);

        mSequenceLogFileManagerList->add(sequenceLogFileManager);
    }

    return sequenceLogFileManager;
}

/*!
 *  \brief  シーケンスログファイル情報登録
 */
void SequenceLogServiceMain::addFileInfo(FileInfo* info, int32_t userId)
{
    // シーケンスログファイル情報登録
    SequenceLogFileManager* sequenceLogFileManager = getSequenceLogFileManager(userId);
    sequenceLogFileManager->addFileInfo(info);
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

    SequenceLogService* sequenceLogService = dynamic_cast<SequenceLogService*>(response);

    if (sequenceLogService)
    {
        sequenceLogService->addThreadListener(this);
        sequenceLogService->addSequenceLogServiceListener(this);
    }
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
void SequenceLogServiceMain::printLog(const Buffer* text, int32_t len, int32_t userId)
{
#if !defined(__ANDROID__) || defined(__EXEC__)
    onUpdateLog(text, userId);
#endif
}

/*!
 *  \brief  共有ファイルコンテナ取得
 */
SharedFileContainer* SequenceLogServiceMain::getSharedFileContainer(const CoreString* baseFileName, int32_t userId)
{
    SequenceLogFileManager* sequenceLogFileManager = getSequenceLogFileManager(userId);
    return sequenceLogFileManager->getSharedFileContainer(baseFileName);
}

/*!
 *  \brief  共有ファイルコンテナリリース
 */
void SequenceLogServiceMain::releaseSharedFileContainer(SharedFileContainer* container, int32_t userId)
{
    SequenceLogFileManager* sequenceLogFileManager = getSequenceLogFileManager(userId);
    sequenceLogFileManager->releaseSharedFileContainer(container);
}

/*!
 * \brief   シーケンスログファイル情報取得
 */
std::list<FileInfo*>* SequenceLogServiceMain::getFileInfoArray(int32_t userId) const
{
    SequenceLogFileManager* sequenceLogFileManager = getSequenceLogFileManager(userId);
    return sequenceLogFileManager->getFileInfoList();
}

/*!
 *  \brief  シーケンスログフォルダ名設定
 */
void SequenceLogServiceMain::setLogFolderName(const CoreString* name)
{
    mLogFolderName.copy(name);
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
    mSequenceLogFileManagerList->setMaxFileSize(size);
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
    mSequenceLogFileManagerList->setMaxFileCount(count);
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
 *  \brief	シーケンスログサービススレッド初期化完了通知
 */
void SequenceLogServiceMain::onThreadInitialized(Thread* thread)
{
    SLOG(CLS_NAME, "onInitialized");
    SequenceLogService* response = dynamic_cast<SequenceLogService*>(thread);

    if (response == nullptr)
        return;

    ThreadListeners* listeners = getListeners();

    for (auto i = listeners->begin(); i != listeners->end(); i++)
        (*i)->onThreadInitialized(thread);
}

/*!
 *  \brief	シーケンスログサービススレッド終了通知
 */
void SequenceLogServiceMain::onThreadTerminated(Thread* thread)
{
    SLOG(CLS_NAME, "onTerminated");
    SequenceLogService* response = dynamic_cast<SequenceLogService*>(thread);

    if (response == nullptr)
        return;

    if (mCleanupFlag)
        return;

    ThreadListeners* listeners = getListeners();

    for (auto i = listeners->begin(); i != listeners->end(); i++)
        (*i)->onThreadTerminated(thread);
}

/*!
 *  \brief	シーケンスログファイル変更通知
 */
void SequenceLogServiceMain::onLogFileChanged(Thread* thread, const CoreString* fileName, int32_t userId)
{
    SLOG(CLS_NAME, "onLogFileChanged");

    for (auto i = mListeners.begin(); i != mListeners.end(); i++)
        (*i)->onLogFileChanged(thread, fileName, userId);
}

/*!
 *  \brief	シーケンスログ更新通知
 */
void SequenceLogServiceMain::onUpdateLog(const Buffer* text, int32_t userId)
{
//  SLOG(CLS_NAME, "onUpdateLog");

    for (auto i = mListeners.begin(); i != mListeners.end(); i++)
        (*i)->onUpdateLog(text, userId);
}

} // namespace slog
