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
 *  \file   SequenceLogServiceMain.h
 *  \brief  シーケンスログサービスメインクラス
 *  \author Copyright 2011 log-tools.net
 */
#pragma once

#include "slog/Thread.h"
#include "slog/FileFind.h"
#include "slog/Socket.h"
#include "slog/FixedString.h"

#include <list>

namespace slog
{
class FileInfo;
class Mutex;
class SequenceLogService;

typedef std::list<SequenceLogService*>  SequenceLogServiceManager;
typedef std::list<FileInfo*>            FileInfoArray;

/*!
 *  \brief  シーケンスログサービスリスナークラス
 */
class SequenceLogServiceThreadListener : public ThreadListener
{
public:     virtual void onLogFileChanged(Thread* thread) {}
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

            Socket                              mSocketPrint;           //!< シーケンスログプリントとの接続用ソケット
            Socket                              mSocket;                //!< シーケンスログクライアントの接続待ち受けソケット
            SequenceLogServiceManager           mServiceManager;        //!< シーケンスログサービスマネージャー
            SequenceLogServiceThreadListener*   mServiceListener;       //!< シーケンスログサービスリスナー
            FileInfoArray                       mFileInfoArray;         //!< シーケンスログファイル情報

            Mutex*                              mMutex;
            bool                                mRootAlways;
            bool                                mStartRunTime;

public:     SequenceLogServiceMain();
            virtual ~SequenceLogServiceMain();

            virtual void interrupt();

            static SequenceLogServiceMain* getInstance();

private:    virtual void run();
public:     void cleanup();

            // シーケンスログプリント関連
            bool  isConnectSequenceLogPrint() const;
            void    connectSequenceLogPrint(const CoreString& ip);
            void disconnectSequenceLogPrint();
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

            const CoreString& getLogFolderName() const;
            void setLogFolderName(const CoreString& name);

            uint32_t getMaxFileSize() const;
            void     setMaxFileSize(uint32_t size);

            int32_t  getMaxFileCount() const;
            void     setMaxFileCount(int32_t count);

private:    virtual void onFind(const CoreString& path);
};

/*!
 *  \brief  シーケンスログプリントと接続しているか調べる
 */
inline bool SequenceLogServiceMain::isConnectSequenceLogPrint() const
{
    return mSocketPrint.isOpen();
}

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
 *  \brief  ルートをALWAYSとするかどうか調べる
 */
inline bool SequenceLogServiceMain::isRootAlways() const
{
    return mRootAlways;
}

/*!
 *  \brief  ルートをALWAYSとするかどうか設定する
 */
inline void SequenceLogServiceMain::setRootAlways(bool always)
{
    mRootAlways = always;
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
