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
 *  \file   Variable.cpp
 *  \brief  変数リストクラス
 *  \author Copyright 2014 printf.jp
 */
#include "slog/Variable.h"

namespace slog
{

/*!
 * \brief   コンストラクタ
 */
Variable::Variable(const char* name, const char* value)
{
    this->name. copy(name);
    this->value.copy(value);
}

/*!
 * \brief   コンストラクタ
 */
Variable::Variable(const char* name, int32_t value)
{
    this->name. copy(name);
    this->value.format("%d", value);
}

/*!
 * \brief   デストラクタ
 */
VariableList::~VariableList()
{
    for (auto i = mList.begin(); i != mList.end(); i++)
        delete *i;
}

/*!
 * \brief   変数を取得する
 */
const Variable* VariableList::get(int32_t index) const
{
    auto i = mList.begin();

    for (int32_t count = index; 0 < count; count--)
        i++;

    return *i;
}

/*!
 * \brief   変数を検索する
 *
 * \param[in]   name    変数名
 *
 * \return  値
 */
const CoreString* VariableList::find(const CoreString* name) const
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
 * \brief   変数を追加する
 *
 * \param[in]   name    変数名
 * \param[in]   value   値
 *
 * \return  なし
 */
void VariableList::add(const CoreString* name, const CoreString* value)
{
    add(name->getBuffer(), value->getBuffer());
}

/*!
 * \brief   変数を追加する
 *
 * \param[in]   name    変数名
 * \param[in]   value   値
 *
 * \return  なし
 */
void VariableList::add(const char* name, const CoreString* value)
{
    add(name, value->getBuffer());
}

/*!
 * \brief   変数を追加する
 *
 * \param[in]   name    変数名
 * \param[in]   value   値
 *
 * \return  なし
 */
void VariableList::add(const char* name, const char* value)
{
    mList.push_back(new Variable(name, value));
}

/*!
 * \brief   変数を追加する
 *
 * \param[in]   name    変数名
 * \param[in]   value   値
 *
 * \return  なし
 */
void VariableList::add(const char* name, int32_t value)
{
    mList.push_back(new Variable(name, value));
}

} // namespace slog
