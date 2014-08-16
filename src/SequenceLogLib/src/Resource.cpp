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
 * \file    Resource.cpp
 * \brief   リソースクラス
 * \author  Copyright 2014 printf.jp
 */
#pragma execution_character_set("utf-8")

#include "slog/Resource.h"
#include "slog/PointerString.h"

namespace slog
{

/*!
 * \brief   コンストラクタ
 *
 * \param[in]   language                言語（"en"、"ja"など）
 * \param[in]   languageStringList      言語別文字列リスト
 * \param[in]   languageStringListCount 言語別文字列リスト数
 */
Resource::Resource(const CoreString* language, const LanguageStringList* languageStringList, int32_t languageStringListCount)
{
    mLanguageStringList = languageStringList;
    mLanguageStringListCount = languageStringListCount;

    setLanguage(language);
}

/*!
 * \brief   言語設定
 *
 * \param[in]   language                言語（"en"、"ja"など）
 */
void Resource::setLanguage(const CoreString* language)
{
    PointerString en("en");

    if (language == nullptr)
        language = &en;

    mStrings = getStrings(language);

    if (mStrings == nullptr)
        mStrings = getStrings(&en);

    if (mStrings == nullptr)
        mStrings = mLanguageStringList[0].strings;
}

/*!
 * \brief   文字列取得
 *
 * \param[in]   id  文字列リソースID
 */
const char* Resource::string(int32_t id) const
{
    const char* text = mStrings[id];

    if (text[0] != '\0')
        return text;

    PointerString en("en");
    const char** strings = getStrings(&en);

    if (strings == nullptr)
        strings = mLanguageStringList[0].strings;

    return strings[id];
}

/*!
 * \brief   文字列リスト取得
 *
 * \param[in]   language    言語（"en"、"ja"など）
 */
const char** Resource::getStrings(const CoreString* language) const
{
    for (int32_t i = 0; i < mLanguageStringListCount; i++)
    {
        if (language->indexOf(mLanguageStringList[i].language) == 0)
            return mLanguageStringList[i].strings;
    }

    return nullptr;
}

} // namespace slog
