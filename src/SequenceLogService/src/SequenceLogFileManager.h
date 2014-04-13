/*
 * Copyright (C) 2014 printf.jp
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
 *  \file   SequenceLogFileManager.h
 *  \brief  シーケンスログファイルマネージャークラス
 *  \author Copyright 2014 printf.jp
 */
#pragma once

#include "slog/FileFind.h"
#include <list>

namespace slog
{
class SharedFileContainer;
class FileInfo;

/*!
 * \brief   シーケンスログファイルマネージャークラス
 */
class SequenceLogFileManager : public FileFindListener
{
            /*!
             * アカウントID
             */
            int32_t mAccountId;

            /*!
             * 最大ファイルサイズ
             */
            uint32_t mMaxFileSize;

            /*!
             * 最大ファイル数
             */
            int32_t mMaxFileCount;

            /*!
             * 共有ファイルコンテナ情報
             */
            std::list<SharedFileContainer*> mSharedFileContainerArray;

            /*!
             * シーケンスログファイル情報
             */
            std::list<FileInfo*> mFileInfoArray;

            /*!
             * コンストラクタ
             */
public:     SequenceLogFileManager(int32_t accountId, uint32_t maxFileSize, int32_t maxFileCount);

            /*!
             * デストラクタ
             */
            ~SequenceLogFileManager();

            /*!
             * アカウントID取得
             */
            int32_t getAccountId() const {return mAccountId;}

            /*!
             * 最大ファイルサイズ設定
             */
            void setMaxFileSize(uint32_t maxFileSize) {mMaxFileSize = maxFileSize;}

            /*!
             * 最大ファイル数設定
             */
            void setMaxFileCount(int32_t maxFileCount) {mMaxFileCount = maxFileCount;}

            /*!
             * 共有ファイルコンテナ取得
             */
            SharedFileContainer* getSharedFileContainer(const CoreString* baseFileName);

            /*!
             * 共有ファイルコンテナリリース
             */
            void releaseSharedFileContainer(SharedFileContainer* container);

            /*!
             * シーケンスログファイル情報登録
             */
            void addFileInfo(FileInfo* info);

            /*!
             * シーケンスログファイル情報取得
             */
            std::list<FileInfo*>* getFileInfoList() const {return (std::list<FileInfo*>*)&mFileInfoArray;}

            /*!
             * 
             */
private:    void clearFileInfoList();

            /*!
             * 
             */
public:     void enumFileInfoList(const CoreString* dirName);

            /*!
             * 
             */
private:    virtual void onFind(const CoreString& path) override;
};

/*!
 * \brief   シーケンスログファイルマネージャーリストクラス
 */
class SequenceLogFileManagerList
{
            /*!
             * シーケンスログファイルマネージャーリスト
             */
            std::list<SequenceLogFileManager*> mList;

            /*!
             * デストラクタ
             */
public:     ~SequenceLogFileManagerList() {clear();}

            /*!
             * シーケンスログファイルマネージャー追加
             */
            void add(SequenceLogFileManager* sequenceLogFileManager);

            /*!
             * シーケンスログファイルマネージャー取得
             */
            SequenceLogFileManager* get(int32_t accountId);

            /*!
             * クリア
             */
            void clear();
};

} // namespace slog
