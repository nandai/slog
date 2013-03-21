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

#include <list>

namespace slog
{
class FileInfo;
class Mutex;
class SequenceLogService;
class SequenceLogServiceWebServerThread;

typedef std::list<SequenceLogService*>  SequenceLogServiceManager;
typedef std::list<FileInfo*>            FileInfoArray;

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
class SequenceLogServiceMain : public Thread, public FileFindListener
{
            FixedString<MAX_PATH>               mSharedMemoryPathName;  //!< 共有メモリパス
            int32_t                             mSharedMemoryItemCount; //!< 共有メモリに格納できるシーケンスログアイテムの最大数
            FixedString<MAX_PATH>               mLogFolderName;         //!< シーケンスログフォルダ名
            uint32_t                            mMaxFileSize;           //!< 最大ファイルサイズ
            int32_t                             mMaxFileCount;          //!< 最大ファイル数
            uint16_t                            mWebServerPort;         //!< シーケンスログWEBサーバーポート

            Socket                              mSocket;                //!< シーケンスログクライアントの接続待ち受けソケット
            SequenceLogServiceManager           mServiceManager;        //!< シーケンスログサービスマネージャー
            SequenceLogServiceThreadListener*   mServiceListener;       //!< シーケンスログサービスリスナー
            FileInfoArray                       mFileInfoArray;         //!< シーケンスログファイル情報

            Mutex*                              mMutex;
            bool                                mStartRunTime;
            bool                                mOutputScreen;          //!< ログを画面に表示するかどうか

            SequenceLogServiceWebServerThread*  mWebServer;             //!< シーケンスログWEBサーバースレッド

            String                              mSequenceLogServerIp;   //!< シーケンスログサーバーIP
            uint16_t                            mSequenceLogServerPort; //!< シーケンスログサーバーポート

public:     SequenceLogServiceMain();
            virtual ~SequenceLogServiceMain();

            static SequenceLogServiceMain* getInstance();

private:    virtual void run();
public:     void cleanup();

            // シーケンスログプリント関連
            void printLog(const Buffer* text, int32_t len);

            // 共有メモリパス
            const CoreString& getSharedMemoryPathName();
#if defined(__unix__)
            void setSharedMemoryPathName(const CoreString& pathName);
#endif
            int32_t getSharedMemoryItemCount() const;
            void    setSharedMemoryItemCount(int32_t count);

            // その他
            void setServiceListener(SequenceLogServiceThreadListener* listener);
            SequenceLogServiceManager* getSequenceLogServiceManager() const;

            FileInfoArray* getFileInfoArray() const;
            void        deleteFileInfoArray();
            void addFileInfo(FileInfo* info);

            Mutex* getMutex() const;

            bool  isRootAlways() const;
            void setRootAlways(bool always);

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

            const CoreString& getSequenceLogServerIP() const;
            uint16_t          getSequenceLogServerPort() const;
            void              setSequenceLogServer(const CoreString& ip, uint16_t port);

private:    virtual void onFind(const CoreString& path);
};

/*!
 *  \brief  共有メモリに格納できるシーケンスログアイテムの最大数を取得
 */
inline int32_t SequenceLogServiceMain::getSharedMemoryItemCount() const
{
    return mSharedMemoryItemCount;
}

/*!
 *  \brief  共有メモリに格納できるシーケンスログアイテムの最大数を設定
 */
inline void SequenceLogServiceMain::setSharedMemoryItemCount(int32_t count)
{
    mSharedMemoryItemCount = count;
}

/*!
 *  \brief  シーケンスログサービスリスナー設定
 */
inline void SequenceLogServiceMain::setServiceListener(SequenceLogServiceThreadListener* listener)
{
    mServiceListener = listener;
}

/*!
 *  \brief  シーケンスログサービスマネージャー取得
 */
inline SequenceLogServiceManager* SequenceLogServiceMain::getSequenceLogServiceManager() const
{
    return (SequenceLogServiceManager*)&mServiceManager;
}

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
