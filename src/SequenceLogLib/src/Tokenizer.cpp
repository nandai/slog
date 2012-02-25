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
 *  \file   Tokenizer.cpp
 *  \brief  文字列分割クラス
 *  \author Copyright 2011 log-tools.net
 */
#include "slog/Tokenizer.h"

namespace slog
{

/*!
 *  \brief  文字列分割
 */
static const char* tokenize(
    CoreString* str,    //!< 結果を受け取るバッファ
    const char* p1,     //!< 分割対象文字列
    char aDelimiter)    //!< デリミタ（連続するスペースとタブは同一視）
{
    char delimiter[] = "xx";

    if (aDelimiter == ' ' || aDelimiter == '\t')
    {
        delimiter[0] = ' ';
        delimiter[1] = '\t';
    }
    else
    {
        delimiter[0] = aDelimiter;
        delimiter[1] = '\0';
    }

    const char* p2 = strpbrk(p1, delimiter);
    const char* p3 = p1;
    int32_t len;

    if (p2)
    {
        len = (int32_t)(p2 - p1);

        if (delimiter[0] != ' ')
        {
            p1 = p2 + 1;
        }
        else
        {
            // 連続するスペースとタブは同一視
            p1 = p2 + strspn(p2, delimiter);
        }
    }
    else
    {
        len = (int32_t)strlen(p1);
        p1 = NULL;
    }

    str->copy(p3, len);
    return p1;
}

/*!
 *  \brief  コンストラクタ
 *
 *  \note   フォーマットはキーとなる文字列を'['と']'で囲い、']'の直後の文字をデリミタと解釈する
 *          例："[key1],[key2]/[key3] [key4]"
 */
Tokenizer::Tokenizer(
    const CoreString& format)   //!< フォーマット
{
    mDelimiter = 0;
    const char* p1 = format.getBuffer();

    while (true)
    {
        const char* p2 = strchr(p1, '[');
        const char* p3 = strchr(p1, ']');

        if (p2 == NULL || p3 == NULL)
            break;

        String name(p2 + 1, (short)(p3 - p2 - 1));
        char delimiter = p3[1];

        if (mElements[name])
            break;

        mKeys.push_back(name);

        Element* element = new Element;
        element->delimiter = delimiter;

        mElements[name] = element;

        if (delimiter == '\0')
            break;

        p1 = p3 + 2;
    }
}

/*!
 *  \brief  コンストラクタ
 */
Tokenizer::Tokenizer(
    char delimiter)     //!< デリミタ
{
    mDelimiter = delimiter;
}

/*!
 *  \brief  デストラクタ
 */
Tokenizer::~Tokenizer()
{
    cleanUp();
}

/*!
 *  \brief  クリーンアップ
 */
void Tokenizer::cleanUp()
{
    for (Elements::iterator i = mElements.begin(); i != mElements.end(); i++)
    {
        Elements::value_type p = *i;
        delete p.second;
    }

    mElements.clear();
    mKeys.    clear();
}

/*!
 *  \brief  実行
 */
int32_t Tokenizer::exec(
    const CoreString &str)      //!< 分割対象文字列
{
    int32_t result;

    if (mDelimiter == 0)
        result = execNamed(str);
    else
        result = execIndexed(str);

    return result;
}

/*!
 *  \brief  実行
 */
int32_t Tokenizer::execNamed(
    const CoreString &str)      //!< 分割対象文字列
{
    const char* p1 = str.getBuffer();

    for (Elements::iterator i = mElements.begin(); i != mElements.end(); i++)
    {
        Elements::value_type p = *i;
        p.second->variant.mStr.setLength(0);
    }

    for (Keys::iterator i = mKeys.begin(); i != mKeys.end(); i++)
    {
        if (p1 == NULL)
            return -1;

        Element* element = mElements[*i];
        p1 = tokenize(&element->variant.mStr, p1, element->delimiter);
    }

    if (p1 == NULL)
        return str.getLength();

    return (int32_t)(p1 - str.getBuffer());
}

/*!
 *  \brief  実行
 */
int32_t Tokenizer::execIndexed(
    const CoreString &str)      //!< 分割対象文字列
{
    const char* p1 = str.getBuffer();
    String name;
    int32_t num = 1;

    cleanUp();

    while (true)
    {
        if (p1 == NULL)
            break;

        name.format("%d", num);
        num++;

        mKeys.push_back(name);

        Element* element = new Element;
        mElements[name] = element;

        p1 = tokenize(&element->variant.mStr, p1, mDelimiter);
    }

    return str.getLength();
}

/*!
 *  \brief  値取得
 */
const Variant& Tokenizer::getValue(const char* key) const
{
    Elements::const_iterator i = mElements.find(key);

    if (i == mElements.end())
        return mEmpty;

    return (*i).second->variant;
}

/*!
 *  \brief  値取得
 */
const Variant& Tokenizer::getValue(int32_t index) const
{
    String str;
    str.format("%d", index + 1);

    return getValue(str.getBuffer());
}

} // namespace slog
