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
 * \file    FileFind.h
 * \brief   ファイル検索クラス
 * \author  Copyright 2011-2015 printf.jp
 */
#pragma once
#include "slog/slog.h"

namespace slog
{

class CoreString;

/*!
 * \brief   ファイル検索リスナークラス
 */
class SLOG_API FileFindListener
{
public:     virtual void onFind(const CoreString& path) {}
};

/*!
 * \brief   ファイル検索クラス
 */
class SLOG_API FileFind
{
            /*!
             * \brief   デフォルトリスナー
             */
            FileFindListener mDefaultListener;

            /*!
             * \brief   リスナー
             */
            FileFindListener* mListener;

            /*!
             * コンストラクタ
             */
public:     FileFind();

            /*!
             * 検索実行
             */
            void exec(const CoreString* fileName) const;

            /*!
             * リスナー設定
             */
            void setListener(FileFindListener* listener);
};

/*!
 * \brief   リスナー設定
 */
inline void FileFind::setListener(FileFindListener* listener)
{
    mListener = (listener ? listener : &mDefaultListener);
}

} // namespace slog
