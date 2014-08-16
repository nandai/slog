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
 * \file    Resource.h
 * \brief   リソースクラス
 * \author  Copyright 2014 printf.jp
 */
#pragma once
#include "slog/slog.h"

namespace slog
{
class CoreString;

class SLOG_API Resource
{
            /*!
             * \brief   言語別文字列リスト
             */
protected:  class LanguageStringList
            {
            public:         const char*     language;
                            const char**    strings;
            };

            /*!
             * 言語別文字列リスト
             */
private:    const LanguageStringList* mLanguageStringList;

            /*!
             * 言語別文字列リスト数
             */
            int32_t mLanguageStringListCount;

            /*!
             * 文字列リスト
             */
            const char** mStrings;

            /*!
             * コンストラクタ
             */
            Resource() = delete;

            /*!
             * コンストラクタ
             */
protected:  Resource(const CoreString* language, const LanguageStringList* languageStringList, int32_t languageStringListCount);

            /*!
             * 言語設定
             */
public:     void setLanguage(const CoreString* language);

            /*!
             * 文字列取得
             */
            const char* string(int32_t id) const;

            /*!
             * 文字列リスト取得
             */
private:    const char** getStrings(const CoreString* language) const;
};

} // namespace slog
