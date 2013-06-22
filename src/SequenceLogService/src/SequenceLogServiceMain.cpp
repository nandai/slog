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
#include "SequenceLogServiceWebServerResponse.h"
#include "SharedFileContainer.h"

#include "slog/Mutex.h"
#include "slog/TimeSpan.h"
#include "slog/FileInfo.h"
#include "slog/Util.h"

#include <algorithm>

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

    mMaxFileSize = 1024 * 4;
    mMaxFileCount = 10;

    mCleanupFlag = false;

    mMutex = new Mutex;
    ScopedLock lock(mMutex, false);

    mStartRunTime = false;
    mOutputScreen = true;
}

/*!
 *  \brief  デストラクタ
 */
SequenceLogServiceMain::~SequenceLogServiceMain()
{
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
            TRACE("    %s\n", e.getMessage());
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
    mWebServer[0].start();
    mWebServer[1].start();

    while (isInterrupted() == false)
        sleep(2000);

    mWebServer[0].interrupt();
    mWebServer[1].interrupt();

    mWebServer[0].join();
    mWebServer[1].join();

    cleanup();
}

/*!
 *  \brief  onResponseStart
 */
void SequenceLogServiceMain::onResponseStart(WebServerResponseThread* response)
{
    mServiceManager.push_back(response);
    response->setListener(this);
}

/*!
 *  \brief  クリーンアップ
 */
void SequenceLogServiceMain::cleanup()
{
    mCleanupFlag = true;

    for (SequenceLogServiceManager::iterator i = mServiceManager.begin(); i != mServiceManager.end(); i++)
    {
        Thread* thread = *i;
        thread->interrupt();
    }

    for (SequenceLogServiceManager::iterator i = mServiceManager.begin(); i != mServiceManager.end(); i++)
    {
        Thread* thread = *i;
        thread->join();
        delete thread;
    }

    mServiceManager.clear();
}

/*!
 *  \brief  インスタンス取得
 */
SequenceLogServiceMain* SequenceLogServiceMain::getInstance()
{
    return sServiceMain;
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
    return mWebServer[0].getPort();
}

/*!
 *  \brief  シーケンスログWEBサーバーポート設定
 */
void SequenceLogServiceMain::setWebServerPort(uint16_t port)
{
    mWebServer[0].setPort(port);
    mWebServer[1].setPort(8443);
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
    SequenceLogService* response = dynamic_cast<SequenceLogService*>(thread);

    if (response == NULL)
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
    SequenceLogService* response = dynamic_cast<SequenceLogService*>(thread);

    if (response == NULL)
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
