/*
 * Copyright (C) 2011 log-tools.net
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
 *  \file   CoreString.h
 *  \brief  コア文字列クラス
 *  \author Copyright 2011 log-tools.net
 */
#pragma once

#include "slog/Buffer.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

namespace slog
{

/*!
 *  \brief  コア文字列クラス
 */
class CoreString : public Buffer
{
private:    const CoreString& operator=(const char*);
            const CoreString& operator=(const CoreString&);

public:     void copy(const char* text, int32_t len = -1) throw(Exception);
            void copy(const CoreString& str) throw(Exception) {copy(str.getBuffer(), str.getLength());}

            void append(const char* text, int32_t len = -1) throw(Exception);
            void append(const CoreString& str) throw(Exception) {append(str.getBuffer(), str.getLength());}

            char operator[](int32_t index) const;

            virtual void setLength(int32_t len) throw(Exception);

            void format( const char* format, ...) throw(Exception);
            void formatV(const char* format, va_list arg) throw(Exception);
};

/*!
 *  \brief  文字列をコピーする
 */
inline void CoreString::copy(const char* text, int32_t len) throw(Exception)
{
    if (getBuffer() == text)
        return;

    if (len == -1)
        len = (int32_t)strlen(text);

    if (getCapacity() < len)
        setCapacity(len);

    strncpy(getBuffer(), text, len);
    setLength(len);
}

/*!
 *  \brief  文字列を追加する
 */
inline void CoreString::append(const char* text, int32_t len) throw(Exception)
{
    if (len == -1)
        len = (int32_t)strlen(text);

    int32_t capacity = getCapacity();

    if (capacity <  getLength() + len)
        setCapacity(getLength() + len);

    strncpy(getBuffer() + getLength(), text, len);
    setLength(getLength() + len);
}

/*!
 *  \brief  指定位置の文字取得
 */
inline char CoreString::operator[](int index) const
{
    char* p = getBuffer();
    return p[index];
}

/*!
 *  \brief  バッファ使用サイズ取得
 */
inline void CoreString::setLength(int32_t len) throw(Exception)
{
    Buffer::setLength(len);

    char* p = getBuffer();
    p[len] = '\0';
}

/*!
 *  \brief  フォーマット
 */
inline void CoreString::format(const char* format, ...) throw(Exception)
{
    va_list arg;
    va_start(arg, format);

    formatV(format, arg);
    va_end(arg);
}

/*!
 *  \brief  フォーマット
 */
inline void CoreString::formatV(const char* format, va_list arg) throw(Exception)
{
    int32_t len;

#if defined(__unix__)
    va_list argCopy;
    va_copy(argCopy, arg);
#endif

    do
    {
        char* p = getBuffer();
        int32_t capacity = getCapacity();

#if defined(_WINDOWS)
        len = vsnprintf(p, capacity,     format, arg);
#else
        len = vsnprintf(p, capacity + 1, format, arg);  // バッファサイズには終端の'\0'も含まれる
#endif

//      if (len != -1)
        if (len != -1 && len <= capacity)
            break;

        p[capacity] = '\0';

#if defined(_WINDOWS)
        setCapacity(capacity + 256);
#else
        setCapacity(len + 1);
        va_copy(arg, argCopy);
#endif
    }
    while (true);

    setLength(len);
}

/*!
 *  \brief  文字列比較
 */
inline bool operator==(const CoreString& str1, const char* str2)
{
    return (strcmp(str1.getBuffer(), str2) == 0);
}

} // namespace slog
