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
 * \file    String.cpp
 * \brief   可変長文字列クラス
 * \author  Copyright 2011-2014 printf.jp
 */
#include "slog/String.h"
#include <string.h>

namespace slog
{

/*!
 * \brief   コンストラクタ
 */
String::String(const char* text)
{
    init(text, (int32_t)strlen(text));
}

/*!
 * \brief   初期化
 */
void String::init(const char* text, int16_t len)
{
    mBuffer = new char[len + 1];
    mCapacity = len;

    memcpy(mBuffer, text, len);
    setLength(len);
}

/*!
 * \brief   バッファサイズ設定
 */
void String::setCapacity(int32_t capacity) throw(Exception)
{
    char* oldBuffer = mBuffer;
    int32_t oldLen = getLength();

    mBuffer = new char[capacity + 1];
    mCapacity = capacity;

    int32_t len = (mCapacity < oldLen ? mCapacity : oldLen);
    memcpy(mBuffer, oldBuffer, len);

    delete [] oldBuffer;
    setLength(len);
}

/*!
 * \brief   文字列比較
 */
bool operator<(const String& str1, const String& str2)
{
    return (strcmp(str1.getBuffer(), str2.getBuffer()) < 0);
}

} // namespace slog
