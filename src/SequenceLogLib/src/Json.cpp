/*
 * Copyright (C) 2011-2014 printf.jp
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
 *  \author Copyright 2011-2014 printf.jp
 */
#include "slog/Json.h"

#if defined(__linux__)
    #include <string.h>
#endif

using namespace std;

namespace slog
{

/*!
 * \brief   エスケープエンコード
 *
 * \param[out]  dest    変換後の文字列を返す
 * \param[in]   p       変換前の文字列
 *
 * \return  なし
 */
static void encodeEscape(String* dest, const char* p)
{
    static const char targets[] = {'\\', '"'};

    int32_t len = (int32_t)strlen(p) * 2;   // 全ての文字をエスケープするとしても２倍のバッファがあれば足りる

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
 * \brief   コンストラクタ
 *
 * \param[in]   name    キーの名前
 */
JsonAbstract::JsonAbstract(const char* name)
{
    mName.copy(name);
}

/*!
 * \brief   JSON値クラス
 */
class JsonValue : public JsonAbstract
{
            /*!
             * 値
             */
            String mValue;

            /*!
             * コンストラクタ
             */
public:     JsonValue(const char* name, const char* value);

            /*!
             * シリアライズ
             */
private:    virtual void serializeChild(CoreString* content) const override;
};

/*!
 * \brief   コンストラクタ
 *
 * \param[in]   name    キーの名前
 * \param[in]   value   値
 */
JsonValue::JsonValue(const char* name, const char* value) : JsonAbstract(name)
{
    encodeEscape(&mValue, value);
}

/*!
 * \brief   シリアライズ
 *
 * \param[out]  content JSON形式の文字列を返す
 *
 * \return  なし
 */
void JsonValue::serializeChild(CoreString* content) const
{
    content->format("\"%s\":\"%s\"", mName.getBuffer(), mValue.getBuffer());
}

/*!
 * \brief   コンストラクタ
 *
 * \param[in]   name    キーの名前
 */
Json::Json(const char* name) : JsonAbstract(name)
{
    mBracket[0] = '[';
    mBracket[1] = ']';
}

/*!
 * \brief   デストラクタ
 */
Json::~Json()
{
    for (auto i = mList.begin(); i != mList.end(); i++)
        delete *i;
}

/*!
 * \brief   JSONデータ追加
 *
 * \param[in]   name    キーの名前
 * \param[in]   value   値
 *
 * \return  なし
 */
void Json::add(const char* name, const CoreString* value)
{
    add(name, value->getBuffer());
}

/*!
 * \brief   JSONデータ追加
 *
 * \param[in]   name    キーの名前
 * \param[in]   value   値
 *
 * \return  なし
 */
void Json::add(const char* name, const char* value)
{
    mName.setLength(0);
    mBracket[0] = '{';
    mBracket[1] = '}';
    mList.push_back(new JsonValue(name, value));
}

/*!
 * \brief   JSONオブジェクト追加
 *
 * \param[in]   json    JSONオブジェクト
 *
 * \return  なし
 */
void Json::add(Json* json)
{
    mList.push_back(json);
}

/*!
 * \brief   シリアライズ
 *
 * \param[out]  content JSON形式の文字列を返す
 *
 * \return  なし
 */
void Json::serialize(CoreString* content) const
{
    content->setLength(0);
    serializeChild(content);

    if (content->getLength() == 0)
        content->copy("{}");
}

/*!
 * \brief   シリアライズ
 *
 * \param[out]  content JSON形式の文字列を返す
 *
 * \return  なし
 */
void Json::serializeChild(CoreString* content) const
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
    const char* sep = "";

    for (auto i = mList.begin(); i != mList.end(); i++)
    {
        tmp.setLength(0);
        (*i)->serializeChild(&tmp);

        work.append(sep);
        work.append(&tmp);

        sep = ",";
    }

    work.append(&mBracket[1], 1);
    content->append(&work);
}

} // namespace slog
