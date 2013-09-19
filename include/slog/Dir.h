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
 *  \file   Dir.h
 *  \brief  ディレクトリクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once
#include "slog/slog.h"

namespace slog
{
class CoreString;

/*!
 *  \brief  ディレクトリクラス
 */
class SLOG_API Dir
{
            /*!
             * ディレクトリ作成
             */
public:     static bool create(const CoreString* aDirName);

            /*!
             * ディレクトリ削除
             */
            static bool remove(const CoreString* aDirName);

            /*!
             * カレントディレクトリ取得
             */
            static void getCurrent(CoreString* aDirName);

            /*!
             * カレントディレクトリ設定
             */
            static bool setCurrent(const CoreString* aDirName);
};

} // namespace slog
