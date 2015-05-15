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
 *  \file   PointerString.cpp
 *  \brief  ポインタ文字列クラス
 *  \author Copyright 2011-2014 printf.jp
 */
#include "slog/PointerString.h"
#include <string.h>

namespace slog
{

/*!
 *  \brief  コンストラクタ
 */
PointerString::PointerString()
{
    mCapacity = 0;
    mBuffer = nullptr;
}

/*!
 *  \brief  コンストラクタ
 */
PointerString::PointerString(const char* buffer, int32_t capacity)
{
    init(buffer, capacity);
}

/*!
 *  \brief  バッファアドレス取得
 */
char* PointerString::getBuffer() const
{
    return mBuffer;
}

/*!
 *  \brief  バッファサイズ取得
 */
int32_t PointerString::getCapacity() const
{
    return mCapacity;
}

/*!
 *  \brief  初期化
 */
void PointerString::init(const char* buffer, int32_t capacity)
{
    int32_t len = GetLength(buffer);

    mCapacity = (capacity != -1 ? capacity : len);
    mBuffer = (char*)buffer;

//          setLength(len);
    Buffer::setLength(len);
}

} // namespace slog
