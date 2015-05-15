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
 *  \file   Integer.h
 *  \brief  Integer�N���X
 *  \author Copyright 2015 printf.jp
 */
#pragma once

#include "slog/slog.h"
#include <stdlib.h>

namespace slog
{

    /*!
 *  \brief  Integer�N���X
 */
class SLOG_API Integer
{
            /*!
             * \brief   ������𐔒l�ɕϊ�
             */
public:     static int32_t parse(const char* text)
            {
                return atoi(text);
            }
};

} // namespace slog
