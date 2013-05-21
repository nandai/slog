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
 *  \file   SequenceLogServiceMain.h
 *  \brief  シーケンスログサービスメインクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once

#include "slog/Thread.h"
#include "slog/FileFind.h"
#include "slog/Socket.h"
#include "slog/FixedString.h"
#include "slog/String.h"

#include "SequenceLogServiceWebServer.h"
#include <list>

namespace slog
{
class FileInfo;
class Mutex;
class WebServerResponseThread;
class SequenceLogService;
class SequenceLogServiceThreadListener;

typedef std::list<Thread*>      SequenceLogServiceManager;
typedef std::list<FileInfo*>    FileInfoArray;

/*!
 *  \brief  シーケンスログサービスリスナークラス
 */
class SequenceLogServiceThreadListener : public ThreadListener
{
public:     virtual void onLogFileChanged(Thread* thread) {}
            virtual void onUpdateLog(const Buffer* text) {}
};

/*!
 *  \brief  シーケンスログサービスメインクラス
 */
class SequenceLogServiceMain :
    public SequenceLogServiceWebServerThread,
    public FileFindListener,
    public SequenceLogServiceThreadListener
{
            FixedString<MAX_PATH>       mLogFolderName;         //!< シーケンスログフォルダ名
            uint32_t                    mMaxFileSize;           //!< 最大ファイルサイズ
            int32_t                     mMaxFileCount;          //!< 最大ファイル数

            SequenceLogServiceManager   mServiceManager;        //!< シーケンスログサービスマネージャー
            FileInfoArray               mFileInfoArray;         //!< シーケンスログファイル情報
            bool                        mCleanupFlag;

            Mutex*                      mMutex;
            bool                        mStartRunTime;
            bool                        mOutputScreen;          //!< ログを画面に表示するかどうか

            uint16_t                    mSequenceLogServerPort; //!< シーケンスログサーバーポート

public:     SequenceLogServiceMain();
            virtual ~SequenceLogServiceMain();

            static SequenceLogServiceMain* getInstance();

private:    virtual void run();
            virtual void onResponseStart(WebServerResponseThread* response);
public:     void cleanup();

            // シーケンスログプリント関連
            void printLog(const Buffer* text, int32_t len);

            // その他
            FileInfoArray* getFileInfoArray() const;
            void        deleteFileInfoArray();
            void addFileInfo(FileInfo* info);

            Mutex* getMutex() const;

            bool  isStartRunTime() const;
            void setStartRunTime(bool startRunTime);

            bool  isOutputScreen() const;
            void setOutputScreen(bool outputScreen);

            const CoreString& getLogFolderName() const;
            void setLogFolderName(const CoreString& name);

            uint32_t getMaxFileSize() const;
            void     setMaxFileSize(uint32_t size);

            int32_t  getMaxFileCount() const;
            void     setMaxFileCount(int32_t count);

            uint16_t getWebServerPort() const;
            void     setWebServerPort(uint16_t port);

            uint16_t getSequenceLogServerPort() const;
            void     setSequenceLogServerPort(uint16_t port);

private:    virtual void onFind(const CoreString& path);
            virtual void onInitialized(   Thread* thread);
            virtual void onTerminated(    Thread* thread);
            virtual void onLogFileChanged(Thread* thread);
            virtual void onUpdateLog(const Buffer* text);
};

/*!
 *  \brief  シーケンスログファイル情報取得
 */
inline FileInfoArray* SequenceLogServiceMain::getFileInfoArray() const
{
    return (FileInfoArray*)&mFileInfoArray;
}

/*!
 *  \brief  ミューテックス取得
 */
inline Mutex* SequenceLogServiceMain::getMutex() const
{
    return mMutex;
}

/*!
 *  \brief  ログを画面に表示するかどうか調べる
 */
inline bool SequenceLogServiceMain::isOutputScreen() const
{
    return mOutputScreen;
}

/*!
 *  \brief  ログを画面に表示するかどうか設定する
 */
inline void SequenceLogServiceMain::setOutputScreen(bool always)
{
    mOutputScreen = always;
}

/*!
 *  \brief  実行時にサービスを開始するかどうか調べる
 */
inline bool SequenceLogServiceMain::isStartRunTime() const
{
    return mStartRunTime;
}

/*!
 *  \brief  実行時にサービスを開始するかどうか設定する
 */
inline void SequenceLogServiceMain::setStartRunTime(bool startRunTime)
{
    mStartRunTime = startRunTime;
}

/*!
 *  \brief  シーケンスログフォルダ名取得
 */
inline const CoreString& SequenceLogServiceMain::getLogFolderName() const
{
    return mLogFolderName;
}

} // namespace slog
