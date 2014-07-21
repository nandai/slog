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
 *  \file   Convert.cpp
 *  \brief  コンバートクラス
 *  \author Copyright 2014 printf.jp
 */
#include "slog/Convert.h"
#include <ctype.h>

namespace slog
{

/*!
 * \brief   10進数文字列を数値に変換する
 */
template <class T>
inline const char* decToValue(T* value, const char* dec, int32_t len)
{
    int32_t i = 0;
    T sign = 1;
    *value = 0;

    if (dec[0] == '-')
    {
        sign = -1;
        i++;
    }

    while (true)
    {
        if (0 < len && len <= i)
            break;

        char c = dec[i];

        if ('0' <= c && c <= '9')
        {
            c = c - '0';
        }
        else
        {
            break;
        }

        *value = *value * 10 + c;
        i++;
    }

    *value *= sign;
    return (dec + i);
}

/*!
 * \brief   16進数文字列を数値に変換する
 */
template <class T>
inline const char* hexToValue(T* value, const char* hex, int32_t len)
{
    int32_t i = 0;
    *value = 0;

    while (true)
    {
        if (0 < len && len <= i)
            break;

        char c = toupper(hex[i]);

        if ('0' <= c && c <= '9')
        {
            c = c - '0';
        }
        else if ('A' <= c && c <= 'F')
        {
            c = c - 'A' + 0x0A;
        }
        else
        {
            break;
        }

        *value = (*value << 4) | c;
        i++;
    }

    return (hex + i);
}

/*!
 * \brief   指定した基数の文字列を数値に変換する
 */
template <class T>
void toValue(T* value, const char* str, int32_t fromBase, int32_t len)
{
    switch (fromBase)
    {
    case 10:
        decToValue(value, str, len);
        break;

    case 16:
        hexToValue(value, str, len);
        break;
    }
}

/*!
 * \brief   指定した基数の文字列を数値に変換する
 *
 * \param[in]   str         変換する文字列
 * \param[in]   fromBase    基数。10、または16
 * \param[in]   len         変換する文字列の長さ。0の場合は'\0'まで。
 *
 * \return  変換した数値
 */
int8_t Convert::toByte(const char* str, int32_t fromBase, int32_t len)
{
    int8_t value = 0;
    toValue(&value, str, fromBase, len);

    return value;
}

/*!
 * \brief   指定した基数の文字列を数値に変換する
 *
 * \param[in]   str         変換する文字列
 * \param[in]   fromBase    基数。10、または16
 * \param[in]   len         変換する文字列の長さ。0の場合は'\0'まで。
 *
 * \return  変換した数値
 */
int32_t Convert::toInt(const char* str, int32_t fromBase, int32_t len)
{
    int32_t value = 0;
    toValue(&value, str, fromBase, len);

    return value;
}

} // namespace slog
