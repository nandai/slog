/*
 * Copyright (C) 2011-2015 printf.jp
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
 * \file    FileInfo.h
 * \brief   ファイル情報クラス
 * \author  Copyright 2011-2015 printf.jp
 */
#pragma once

#include "slog/CoreString.h"
#include "slog/DateTime.h"

namespace slog
{

/*!
 * \brief   ファイル情報クラス
 */
class SLOG_API FileInfo
{
            struct Data;
            Data* mData;

            /*!
             * \brief   作成日時
             */
            DateTime mCreationTime;

            /*!
             * \brief   最終書込日時
             */
            DateTime mLastWriteTime;

            /*!
             * \brief   ファイルモード
             */
            uint32_t mMode;

            /*!
             * \brief   ファイルサイズ
             */
            uint64_t mSize;

            /*!
             * \brief   使用中かどうか
             */
            bool mUsing;

            /*!
             * コンストラクタ
             */
public:      FileInfo(const CoreString* path) throw(Exception);

            /*!
             * デストラクタ
             */
            ~FileInfo();

            /*!
             * ファイル情報更新
             */
            void update(bool aUsing = false);

            /*!
             * 正規のパス取得
             */
            const CoreString* getCanonicalPath() const;

            /*!
             * 作成日時
             */
            void setCreationTime(const DateTime& dateTime);
            const DateTime& getCreationTime() const;

            /*!
             * 最終書込日時
             */
            void setLastWriteTime(const DateTime& dateTime);
            const DateTime& getLastWriteTime() const;

            /*!
             * ファイルかどうか調べる
             */
            bool isFile() const;

            /*!
             * ファイルサイズ取得
             */
            uint64_t getSize() const;

            /*!
             * 使用中かどうか調べる
             */
            bool isUsing() const;

            /*!
             * メッセージ取得
             */
            const CoreString* getMessage() const;

            /*!
             * ディレクトリ作成
             */
            void mkdir() const throw(Exception);
};

/*!
 * \brief   作成日時取得
 */
inline const DateTime& FileInfo::getCreationTime() const
{
    return mCreationTime;
}

/*!
 * \brief   作成日時設定
 */
inline void FileInfo::setCreationTime(const DateTime& dateTime)
{
    mCreationTime = dateTime;
}

/*!
 * \brief   最終書込日時取得
 */
inline const DateTime& FileInfo::getLastWriteTime() const
{
    return mLastWriteTime;
}

/*!
 * \brief   最終書込日時設定
 */
inline void FileInfo::setLastWriteTime(const DateTime& dateTime)
{
    mLastWriteTime = dateTime;
}

/*!
 * \brief   ファイルサイズ取得
 */
inline uint64_t FileInfo::getSize() const
{
    return mSize;
}

} // namespace slog
