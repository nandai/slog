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
 *  \file   Cookie.cpp
 *  \brief  Cookieクラス
 *  \author Copyright 2014 printf.jp
 */
#include "slog/Cookie.h"
#include "slog/Util.h"

namespace slog
{

/*!
 * \brief   コンストラクタ
 */
Cookie::Cookie(const char* name, const char* value, const char* path, const char* expires, bool secure, bool httpOnly)
{
    this->name. copy(name);
    this->value.copy(value);

    if (path)
        this->path.copy(path);

    if (expires)
        this->expires.copy(expires);

    this->secure = secure;
    this->httpOnly = httpOnly;
}

/*!
 * \brief   デストラクタ
 */
CookieList::~CookieList()
{
    for (auto i = mList.begin(); i != mList.end(); i++)
        delete *i;
}

/*!
 * \brief   Cookieを取得する
 */
const Cookie* CookieList::get(int32_t index) const
{
    auto i = mList.begin();

    for (int32_t count = index; 0 < count; count--)
        i++;

    return *i;
}

/*!
 * \brief   Cookieを検索する
 *
 * \param[in]   name    Cookie名
 *
 * \return  値
 */
const CoreString* CookieList::find(const CoreString* name) const
{
    for (auto i = mList.begin(); i != mList.end(); i++)
    {
        auto variable = *i;

        if (name->equals(variable->name))
            return &variable->value;
    }

    return nullptr;
}

/*!
 * \brief   Cookieを追加する
 *
 * \param[in]   name    Cookie名
 * \param[in]   value   値
 * \param[in]   path    パス
 *
 * \return  なし
 */
void CookieList::add(const char* name, const CoreString* value, const char* path, const char* expires, bool secure, bool httpOnly)
{
    mList.push_back(new Cookie(name, value->getBuffer(), path, expires, secure, httpOnly));
}

/*!
 * \brief   Cookieを削除する
 *
 * \param[in]   name    Cookie名
 * \param[in]   path    パス
 *
 * \return  なし
 */
void CookieList::remove(const char* name, const char* path)
{
    mList.push_back(new Cookie(name, "", path, "Mon, 31-Dec-2001 23:59:59 GMT", false, false));
}

} // namespace slog
