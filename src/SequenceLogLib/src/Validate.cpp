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
 * \file    Validate.cpp
 * \brief   検証クラス
 * \author  Copyright 2014 printf.jp
 */
#include "slog/Validate.h"
#include "slog/CoreString.h"
#include "slog/Util.h"

namespace slog
{
static Validate::Success  sSuccess;
static Validate::Invalid  sInvalid;
static Validate::Empty    sEmpty;
static Validate::TooShort sTooShort;
static Validate::TooLong  sTooLong;

const  Validate::Success*  Validate::SUCCESS =   &sSuccess;
const  Validate::Invalid*  Validate::INVALID =   &sInvalid;
const  Validate::Empty*    Validate::EMPTY =     &sEmpty;
const  Validate::TooShort* Validate::TOO_SHORT = &sTooShort;
const  Validate::TooLong*  Validate::TOO_LONG =  &sTooLong;

/*!
 * \brief   コンストラクタ
 */
StringValidate::StringValidate(const CoreString* value, int32_t minLen, int32_t maxLen) : Validate()
{
    mValue = value;
    mMinLen = minLen;
    mMaxLen = maxLen;
}

/*!
 * \brief   検証実行
 */
const Validate::Result* StringValidate::execute() const
{
    int32_t len = mValue->getCharacters();

    if (len < mMinLen)
    {
        if (len == 0)
            return Validate::EMPTY;
        else
            return Validate::TOO_SHORT;
    }

    if (mMaxLen < len)
        return Validate::TOO_LONG;

    return Validate::SUCCESS;
}

/*!
 * \brief   値のアドレスを取得
 */
void* StringValidate::getValueAddress() const
{
    return (void*)mValue;
}

/*!
 * \brief   コンストラクタ
 */
MailAddresValidate::MailAddresValidate(const CoreString* value) : Validate()
{
    mValue = value;
}

/*!
 * \brief   検証実行
 */
const Validate::Result* MailAddresValidate::execute() const
{
    if (mValue->getLength() == 0)
        return Validate::EMPTY;

    if (Util::validateMailAddress(mValue) == false)
        return Validate::INVALID;

    return Validate::SUCCESS;
}

/*!
 * \brief   値のアドレスを取得
 */
void* MailAddresValidate::getValueAddress() const
{
    return (void*)mValue;
}

/*!
 * \brief   デストラクタ
 */
ValidateList::~ValidateList()
{
    for (std::list<Validate*>::iterator i = mList.begin(); i != mList.end(); i++)
        delete *i;
}

/*!
 * \brief   検証オブジェクト追加
 */
void ValidateList::add(Validate* validate)
{
    mList.push_back(validate);
}

/*!
 * \brief   検証実行
 */
bool ValidateList::execute(ValidateListener* listener)
{
    bool valid = true;

    for (std::list<Validate*>::iterator i = mList.begin(); i != mList.end(); i++)
    {
        auto validate = *i;
        auto result = validate->execute();

        if (result != Validate::SUCCESS)
        {
            listener->onInvalid(validate->getValueAddress(), result);
            valid = false;
        }
    }

    return valid;
}

} // namespace slog
