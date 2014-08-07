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
 *  \file   Variable.h
 *  \brief  変数クラス
 *  \author Copyright 2014 printf.jp
 */
#pragma once

#include "slog/String.h"
#include <list>

#pragma warning(disable:4251)

namespace slog
{

/*!
 * \brief  変数クラス
 */
class SLOG_API Variable
{
            /*!
             * 変数名
             */
public:     String name;

            /*!
             * 値
             */
            String value;

            /*!
             * コンストラクタ
             */
public:     Variable(const char* name, const char* value);

            /*!
             * コンストラクタ
             */
            Variable(const char* name, int32_t value);
};

/*!
 * \brief  変数リストクラス
 */
class SLOG_API VariableList
{
            std::list<Variable*> mList;

            /*!
             * デストラクタ
             */
public:     ~VariableList();

            /*!
             * 変数の数を取得する
             */
            int32_t getCount() const {return (int32_t)mList.size();}

            /*!
             * 変数を取得する
             */
            const Variable* get(int32_t index) const;

            /*!
             * 変数を検索する
             */
            const CoreString* find(const CoreString* name) const;

            /*!
             * 変数を追加する
             */
            void add(const CoreString* name, const CoreString* value);

            /*!
             * 変数を追加する
             */
            void add(const char* name, const CoreString* value);

            /*!
             * 変数を追加する
             */
            void add(const char* name, const char* value);

            /*!
             * 変数を追加する
             */
            void add(const char* name, int32_t value);
};

} // namespace slog
