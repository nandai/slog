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
 * \file    R.h
 * \brief   リソースクラス
 * \author  Copyright 2014 printf.jp
 */
#pragma once
#include "slog/slog.h"

namespace slog
{
class CoreString;

class R
{
            const char** mStrings;

public:     static const int32_t login =          0;
            static const int32_t logout =         1;
            static const int32_t account =        2;
            static const int32_t user_name =      3;
            static const int32_t password =       4;
            static const int32_t change =         5;
            static const int32_t start_time =     6;
            static const int32_t end_time =       7;
            static const int32_t log_file_name =  8;
            static const int32_t log_file_size =  9;
            static const int32_t msg001 =        10;
            static const int32_t msg002 =        11;
            static const int32_t msg003 =        12;
            static const int32_t msg004 =        13;
            static const int32_t msg005 =        14;
            static const int32_t msg006 =        15;
            static const int32_t msg007 =        16;

            /*!
             * コンストラクタ
             */
public:     R(const CoreString* aLang);

            /*!
             * 文字列取得
             */
            const char* string(int32_t id) const;
};

} // namespace slog
