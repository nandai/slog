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
 *  \file   Cookie.h
 *  \brief  Cookieクラス
 *  \author Copyright 2014 printf.jp
 */
#pragma once

#include "slog/String.h"
#include <list>

#pragma warning(disable:4251)

namespace slog
{

/*!
 * \brief   Cookieクラス
 */
class SLOG_API Cookie
{
            /*!
             * クッキー名
             */
public:     String name;

            /*!
             * 値
             */
            String value;

            /*!
             * パス
             */
            String path;

            /*!
             * 有効期限
             */
            String expires;

            /*!
             * 
             */
            bool secure;

            /*!
             * 
             */
            bool httpOnly;

            /*!
             * コンストラクタ
             */
public:     Cookie(const char* name, const char* value, const char* path, const char* expires, bool secure, bool httpOnly);
};

/*!
 * \brief   Cookieリストクラス
 */
class SLOG_API CookieList
{
            std::list<Cookie*> mList;

            /*!
             * デストラクタ
             */
public:     ~CookieList();

            /*!
             * Cookieの数を取得する
             */
            int32_t getCount() const {return (int32_t)mList.size();}

            /*!
             * Cookieを取得する
             */
            const Cookie* get(int32_t index) const;

            /*!
             * 変数を検索する
             */
            const CoreString* find(const CoreString* name) const;

            /*!
             * Cookieを追加する
             */
            void add(const char* name, const CoreString* value, const char* path, const char* expires = nullptr, bool secure = false, bool httpOnly = false);

            /*!
             * Cookieを削除する
             */
            void remove(const char* name, const char* path);
};

} // namespace slog
