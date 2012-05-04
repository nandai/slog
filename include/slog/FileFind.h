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
 *  \file   FileFind.h
 *  \brief  ファイル検索クラス
 *  \author Copyright 2011 log-tools.net
 */
#pragma once
#include "slog/CoreString.h"

namespace slog
{

/*!
 *  \brief  ファイル検索リスナークラス
 */
class SLOG_API FileFindListener
{
public:     virtual void onFind(const CoreString& path) {}
};

/*!
 *  \brief  ファイル検索クラス
 */
class SLOG_API FileFind
{
            FileFindListener    mDefaultListener;   //!< デフォルトリスナー
            FileFindListener*   mListener;          //!< リスナー

public:     FileFind();

            void exec(const CoreString& fileName) const;
            void setListener(FileFindListener* listener);
};

/*!
 *  \brief  リスナー設定
 */
inline void FileFind::setListener(FileFindListener* listener)
{
    mListener = (listener ? listener : &mDefaultListener);
}

} // namespace slog
