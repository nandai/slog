/*
 * Copyright (C) 2015 printf.jp
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
 * \file    ExtraString.h
 * \brief   拡張文字列クラス
 * \author  Copyright 2015 printf.jp
 */
#pragma once

#include "slog/String.h"
#include <string>

namespace slog
{

/*!
 * \brief   拡張文字列クラス
 */
class ExtraString : public String
{
            /*!
             * \brief   最小文字数
             */
public:     int32_t min;

            /*!
             * \brief   最大文字数
             */
            int32_t max;

            /*!
             * \brief   コンストラクタ
             */
public:     ExtraString()
            {
                setMinMax(0, 0);
            }

            /*!
             * \brief   最小文字数と最大文字数を設定
             */
            void setMinMax(int32_t min, int32_t max)
            {
                this->min = min;
                this->max = max;
            }
};

} // namespace slog
