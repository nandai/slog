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
 *  \file   Json.cpp
 *  \brief  JSONクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/Json.h"
using namespace std;

namespace slog
{

/*!
 *  \brief  エスケープエンコード
 */
static void encodeEscape(String* dest, const CoreString& src)
{
    static const char targets[] = {'\\', '"'};

    const char* p = src.getBuffer();
    int32_t len =   src.getLength() * 2;    // 全ての文字をエスケープするとしても２倍のバッファがあれば足りる
    char* buffer = new char[len + 1];
    int32_t pos = 0;

    while (*p)
    {
        bool isTarget = false;

        // エスケープ対象の文字かどうか
        for (int32_t i = 0; i < sizeof(targets); i++)
        {
            if (targets[i] != *p)
                continue;

            isTarget = true;
            break;
        }

        if (isTarget == false)
        {
            // 単純にコピー
            buffer[pos] = *p;
            pos++;
        }
        else
        {
            // エスケープ
            buffer[pos + 0] = '\\';
            buffer[pos + 1] = *p;
            pos += 2;
        }

        p++;
    }

    buffer[pos] = '\0';
    dest->copy(buffer);

    delete [] buffer;
}

/*!
 *  \brief  コンストラクタ
 */
JsonAbstract::JsonAbstract(const char* name)
{
    mName.copy(name);
}

/*!
 *  \brief  コンストラクタ
 */
JsonValue::JsonValue(const char* name, const CoreString& value) : JsonAbstract(name)
{
    encodeEscape(&mValue, value);
}

/*!
 *  \brief  シリアライズ
 */
void JsonValue::serialize(String* content) const
{
    content->format("\"%s\":\"%s\"", mName.getBuffer(), mValue.getBuffer());
}

/*!
 *  \brief  コンストラクタ
 */
Json::Json(const char* name) : JsonAbstract(name)
{
    mBracket[0] = '[';
    mBracket[1] = ']';
}

/*!
 *  \brief  デストラクタ
 */
Json::~Json()
{
    for (list<JsonAbstract*>::iterator i = mList.begin(); i != mList.end(); i++)
        delete *i;
}

/*!
 *  \brief  JSONデータ追加
 */
void Json::add(const char* name, const CoreString& value)
{
    mName.setLength(0);
    mBracket[0] = '{';
    mBracket[1] = '}';
    mList.push_back(new JsonValue(name, value));
}

/*!
 *  \brief  JSONオブジェクト追加
 */
void Json::add(Json* json)
{
    mList.push_back(json);
}

/*!
 *  \brief  シリアライズ
 */
void Json::serialize(String* content) const
{
    if (mList.size() == 0)
        return;

    String work;

    if (mName.getLength())
    {
        work.format("\"%s\":%c", mName.getBuffer(), mBracket[0]);
    }
    else
    {
        work.copy(&mBracket[0], 1);
    }

    String tmp;
    char sep = ' ';

    for (list<JsonAbstract*>::const_iterator i = mList.begin(); i != mList.end(); i++)
    {
        tmp.setLength(0);
        (*i)->serialize(&tmp);

        work.append(&sep, 1);
        work.append(tmp);

        sep = ',';
    }

    work.append(&mBracket[1], 1);
    content->append(work);
}

} // namespace slog
