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
 *  \file   FileInfo.h
 *  \brief  ファイル情報クラス
 *  \author Copyright 2011 log-tools.net
 */
#pragma once

#include "slog/String.h"
#include "slog/FixedString.h"
#include "slog/DateTime.h"

#include <list>

namespace slog
{

/*!
 *  \brief  ファイル情報クラス
 */
class FileInfo
{
            std::list<String>       mNames;             //!< ファイルパスの構成要素
            FixedString<MAX_PATH>   mCanonicalPath;     //!< 正規のパス
            DateTime                mCreationTime;      //!< 作成日時
            DateTime                mLastWriteTime;     //!< 最終書込日時
            uint32_t                mMode;              //!< ファイルモード
            bool                    mUsing;             //!< 使用中かどうか
            FixedString<255>        mMessage;           //!< メッセージ

public:     FileInfo(const CoreString& path) throw(Exception);
            void update(bool aUsing = false);

            const CoreString& getCanonicalPath() const;

            void setCreationTime(const DateTime& dateTime);
            const DateTime& getCreationTime() const;

            void setLastWriteTime(const DateTime& dateTime);
            const DateTime& getLastWriteTime() const;

            bool isFile() const;

            bool isUsing() const;

            const CoreString& getMessage() const;

            void mkdir() const throw(Exception);
};

/*!
 *  \brief  正規のパス取得
 */
inline const CoreString& FileInfo::getCanonicalPath() const
{
    return mCanonicalPath;
}

/*!
 *  \brief  作成日時取得
 */
inline const DateTime& FileInfo::getCreationTime() const
{
    return mCreationTime;
}

/*!
 *  \brief  作成日時設定
 */
inline void FileInfo::setCreationTime(const DateTime& dateTime)
{
    mCreationTime = dateTime;
}

/*!
 *  \brief  最終書込日時取得
 */
inline const DateTime& FileInfo::getLastWriteTime() const
{
    return mLastWriteTime;
}

/*!
 *  \brief  最終書込日時設定
 */
inline void FileInfo::setLastWriteTime(const DateTime& dateTime)
{
    mLastWriteTime = dateTime;
}

/*!
 *  \brief  メッセージ取得
 */
inline const CoreString& FileInfo::getMessage() const
{
    return mMessage;
}

} // namespace slog
