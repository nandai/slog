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
 * \file    R.cpp
 * \brief   リソースクラス
 * \author  Copyright 2014 printf.jp
 */
#pragma execution_character_set("utf-8")

#include "R.h"

namespace slog
{

/*!
 * \brief   英語
 */
static const char* en[] =
{
#include "R/string/en.inc"
};

/*!
 * \brief   日本語
 */
static const char* ja[] =
{
#include "R/string/ja.inc"
};

/*!
 * \brief   台語
 */
static const char* zh_TW[] =
{
#include "R/string/zh_TW.inc"
};

/*!
 * \brief   言語別文字列リスト
 */
const Resource::LanguageStringList R::mLanguageStringList[] =
{
    {"en",    en   },
    {"ja",    ja   },
    {"zh-TW", zh_TW},
};

/*!
 * \brief   コンストラクタ
 *
 * \param[in]   language    言語（"en"、"ja"など）
 */
R::R(const CoreString* language) : Resource(language, mLanguageStringList, sizeof(mLanguageStringList) / sizeof(mLanguageStringList[0]))
{
}

} // namespace slog
