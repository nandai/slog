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
#include "slog/Resource.h"

namespace slog
{
/*!
 * \brief   リソースクラス
 */
class R : public Resource
{
            static const LanguageStringList mLanguageStringList[];

public:     static const int32_t login =          0;
            static const int32_t logout =         1;
            static const int32_t logFileList =    2;
            static const int32_t account =        3;
            static const int32_t user_name =      4;
            static const int32_t password =       5;
            static const int32_t change =         6;
            static const int32_t new_account =    7;
            static const int32_t del =            8;
            static const int32_t back =           9;
            static const int32_t start_time =    10;
            static const int32_t end_time =      11;
            static const int32_t log_file_name = 12;
            static const int32_t file_transfer = 13;
            static const int32_t log_file_size = 14;
            static const int32_t administrator = 15;
            static const int32_t msg001 =        16;
            static const int32_t msg002 =        17;
            static const int32_t msg003 =        18;
            static const int32_t msg004 =        19;
            static const int32_t msg005 =        20;
            static const int32_t msg006 =        21;
            static const int32_t msg007 =        22;
            static const int32_t msg008 =        23;
            static const int32_t msg009 =        24;
            static const int32_t msg010 =        25;

            /*!
             * コンストラクタ
             */
public:     R(const CoreString* language);
};

} // namespace slog
