﻿/*
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
 * \file    SequenceLogServiceMain.h
 * \brief   シーケンスログサービスメインクラス
 * \author  Copyright 2011-2014 printf.jp
 */
#pragma once

#include "slog/Thread.h"
#include "slog/Socket.h"
#include "slog/FixedString.h"
#include "slog/String.h"
#include "slog/WebServerManager.h"

#include <list>

namespace slog
{
class Mutex;
class WebServerResponse;
class SequenceLogService;
class SequenceLogServiceListener;
class SequenceLogFileManagerList;
class SequenceLogFileManager;
class SharedFileContainer;
class FileInfo;

/*!
 * \brief   シーケンスログサービスリスナークラス
 */
class SequenceLogServiceListener
{
            /*!
             * シーケンスログファイル変更通知
             */
public:     virtual void onLogFileChanged(Thread* thread, const CoreString* fileName, int32_t userId) {}

            /*!
             * シーケンスログ更新通知
             */
            virtual void onUpdateLog(const Buffer* text,  int32_t userId) {}
};

/*!
 * \brief   シーケンスログサービスメインクラス
 */
class SequenceLogServiceMain :
    public Thread,
    public ThreadListener,
    public SequenceLogServiceListener
{
            /*!
             * シーケンスログフォルダ名
             */
            FixedString<MAX_PATH> mLogFolderName;

            /*!
             * 最大ファイルサイズ
             */
            uint32_t mMaxFileSize;

            /*!
             * 最大ファイル数
             */
            int32_t mMaxFileCount;

            /*!
             * シーケンスログファイルマネージャーリスト
             */
            SequenceLogFileManagerList* mSequenceLogFileManagerList;

            /*!
             * クリーンアップフラグ
             */
            bool mCleanupFlag;

            /*!
             * ミューテックス
             */
            Mutex* mMutex;

            /*!
             * 実行時にサービスを開始するかどうか
             */
            bool mStartRunTime;

            /*!
             * WEBサーバーマネージャー
             */
            WebServerManager mWebServerManager;

            /*!
             * シーケンスログサーバーポート
             */
            uint16_t mSequenceLogServerPort;

            /*!
             * リスナーリスト
             */
            std::list<SequenceLogServiceListener*> mListeners;

            /*!
             * コンストラクタ
             */
public:     SequenceLogServiceMain();

            /*!
             * デストラクタ
             */
            virtual ~SequenceLogServiceMain() override;

            /*!
             * インスタンス取得
             */
            static SequenceLogServiceMain* getInstance();

            /*!
             * シーケンスログファイルマネージャー取得
             */
private:    SequenceLogFileManager* getSequenceLogFileManager(int32_t userId) const;

            /*!
             * 実行
             */
            virtual void run() override;

            /*!
             * 
             */
public:     virtual void onResponseStart(WebServerResponse* response);

            /*!
             * クリーンアップ
             */
private:    void cleanup();

            /*!
             * シーケンスログプリント関連
             */
public:     void printLog(const Buffer* text, int32_t len, int32_t userId);

            /*!
             * 共有ファイルコンテナ情報
             */
            SharedFileContainer* getSharedFileContainer(const CoreString* baseFileName, int32_t userId);
            void releaseSharedFileContainer(SharedFileContainer* container, int32_t userId);

            /*!
             * シーケンスログファイル情報
             */
            std::list<FileInfo*>* getFileInfoArray(int32_t userId) const;
            void addFileInfo(FileInfo* info, int32_t accountId);

            /*!
             * ミューテックス取得
             */
            Mutex* getMutex() const;

            /*!
             * 実行時にサービスを開始するかどうか
             */
            bool  isStartRunTime() const;
            void setStartRunTime(bool startRunTime);

            /*!
             * シーケンスログフォルダ名
             */
            const CoreString* getLogFolderName() const;
            void setLogFolderName(const CoreString* name);

            /*!
             * 最大ファイルサイズ
             */
            uint32_t getMaxFileSize() const;
            void     setMaxFileSize(uint32_t size);

            /*!
             * 最大ファイル数
             */
            int32_t  getMaxFileCount() const;
            void     setMaxFileCount(int32_t count);

            /*!
             * シーケンスログWEBサーバーポート
             */
            uint16_t getWebServerPort(bool secure) const;
            void     setWebServerPort(bool secure, uint16_t port);

            /*!
             * SSL関連
             */
            void setSSLFileName(const CoreString* certificate, const CoreString* privateKey);
            const CoreString* getCertificateFileName() const;
            const CoreString* getPrivateKeyFileName() const;

            /*!
             * シーケンスログWEBサーバーポート
             */
            uint16_t getSequenceLogServerPort() const;
            void     setSequenceLogServerPort(uint16_t port);

            /*!
             * リスナー追加
             */
            void addSequenceLogServiceListener(SequenceLogServiceListener* listener);

            /*!
             * リスナー解除
             */
            void removeSequenceLogServiceListener(SequenceLogServiceListener* listener);

            /*!
             * スレッド初期化完了通知
             */
private:    virtual void onThreadInitialized(Thread* thread) override;

            /*!
             * スレッド終了通知
             */
            virtual void onThreadTerminated(Thread* thread) override;

            virtual void onLogFileChanged(Thread* thread, const CoreString* fileName, int32_t userId) override;
            virtual void onUpdateLog(const Buffer* text,  int32_t userId) override;
};

/*!
 *  \brief  ミューテックス取得
 */
inline Mutex* SequenceLogServiceMain::getMutex() const
{
    return mMutex;
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
inline const CoreString* SequenceLogServiceMain::getLogFolderName() const
{
    return &mLogFolderName;
}

} // namespace slog
