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
 * \file    CoreString.cpp
 * \brief   コア文字列クラス
 * \author  Copyright 2011-2014 printf.jp
 */
#include "slog/CoreString.h"
#include "slog/Util.h"

#include <stdio.h>
#include <string.h>

namespace slog
{

/*!
 * \brief   次の文字へのバイト数を取得する
 * \note    参考：http://ja.wikipedia.org/wiki/UTF-8
 */
int32_t getNextCharBytes(const char* text)
{
    uint8_t c = *text;
    int32_t bytes;

         if (c < 0xC0) bytes = 1;
    else if (c < 0xE0) bytes = 2;
    else if (c < 0xF0) bytes = 3;
    else if (c < 0xF8) bytes = 4;
    else if (c < 0xFC) bytes = 5;
    else               bytes = 6;

    return bytes;
}

/*!
 * \brief   前の文字へのバイト数を取得する
 */
int32_t getPrevCharBytes(const char* text)
{
    const uint8_t* p = (uint8_t*)text;
    uint8_t c;

    do
    {
        p--;
        c = *p;
    }
    while (0x80 <= c && c < 0xC0);

    return (int32_t)((uint8_t*)text - p);
}

/*!
 * \brief   文字列をコピーする
 */
void CoreString::copy(const char* text, int32_t len) throw(Exception)
{
    if (getBuffer() == text)
        return;

    if (len == -1)
        len = (int32_t)strlen(text);

    // バッファ容量が足りなければ拡張する
    int32_t capacity = getCapacity();

    if (capacity < len)
    {
        setCapacity(len);
    }

    // 文字列コピー
    strncpy(getBuffer(), text, len);
    setLength(len);
}

/*!
 * \brief   文字列を追加する
 */
void CoreString::append(const char* text, int32_t len) throw(Exception)
{
    if (getBuffer() == text)
        return;

    if (len == -1)
        len = (int32_t)strlen(text);

    // バッファ容量が足りなければ拡張する
    int32_t capacity = getCapacity();
    int32_t newLen = getLength() + len;

    if (capacity < newLen)
    {
        setCapacity(newLen);
    }

    // 文字列追加
    strncpy(getBuffer() + getLength(), text, len);
    setLength(newLen);
}

/*!
 * \brief   文字列を挿入する
 */
void CoreString::insert(int32_t pos, const char* text, int32_t len) throw(Exception)
{
    if (getBuffer() == text)
        return;

    if (len == -1)
        len = (int32_t)strlen(text);

    // バッファ容量が足りなければ拡張する
    int32_t capacity = getCapacity();
    int32_t newLen = getLength() + len;

    if (capacity < newLen)
    {
        setCapacity(newLen);
    }

    // 文字列挿入
    char* p = getBuffer();

    memmove(
        p + pos + len,
        p + pos,
        getLength() - pos);

    memcpy(
        p + pos,
        text,
        len);

    setLength(newLen);
}

/*!
 * \brief   最後の１文字を削除
 */
void CoreString::deleteLast()
{
    char* p = getBuffer();
    int32_t len = getLength();

    if (0 < len)
    {
        int32_t num = getPrevCharBytes(p + len);

        len -= num;
        setLength(len);
    }
}

/*!
 * \brief   フォーマット
 */
void CoreString::format(const char* format, ...) throw(Exception)
{
    va_list arg;
    va_start(arg, format);

    formatV(format, arg);
    va_end(arg);
}

/*!
 * \brief   フォーマット
 */
void CoreString::formatV(const char* format, va_list arg) throw(Exception)
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
 * \brief   比較
 */
bool CoreString::equals(const CoreString* str) const
{
    return (strcmp(getBuffer(), str->getBuffer()) == 0);
}

/*!
 * \brief   比較
 */
bool CoreString::equals(const char* text) const
{
    return (strcmp(getBuffer(), text) == 0);
}

/*!
 * \brief   検索
 */
int32_t CoreString::find(char c) const
{
    const char* buffer = getBuffer();
    const char* p = strchr(buffer, c);

    if (p == nullptr)
        return -1;

    return (int32_t)(p - buffer);
}

/*!
 * \brief   前方検索
 */
int32_t CoreString::indexOf(const char* find, int32_t index, int32_t count) const
{
    int32_t findLen = (int32_t)strlen(find);
    int32_t len = getLength();

    if (count == -1)
        count = len - index;

    if (count < findLen)
        return -1;

    if (index < 0 || len < index + count)
        return -1;

    const char* buffer = getBuffer();
    const char* p1 = buffer + index;
    const char* p2 = p1 + (count - findLen);

    do
    {
        if (strncmp(p1, find, findLen) == 0)
            return (int32_t)(p1 - buffer);

        p1++;
    }
    while (p1 <= p2);

    return -1;
}

/*!
 * \brief   後方検索
 */
int32_t CoreString::lastIndexOf(const char* find, int32_t index) const
{
    if (index == -1)
        index = getLength() - 1;

    if (index < 0 || getLength() <= index)
        return -1;

    const char* buffer = getBuffer();

    for (; 0 <= index; index--)
    {
        const char* p = strstr(buffer + index, find);

        if (p)
            return (int32_t)(p - buffer);
    }

    return -1;
}

/*!
 * \brief   文字数を取得する
 */
int32_t CoreString::getCharacters() const
{
    const char* text = getBuffer();
    int32_t num = 0;

    while (*text)
    {
        text += slog::getNextCharBytes(text);
        num++;
    }

    return num;
}

/*!
 * \brief   次の文字へのバイト数を取得する
 */
int32_t CoreString::getNextCharBytes(int32_t pos) const
{
    const char* text = getBuffer();
    return slog::getNextCharBytes(text + pos);
}

#if defined(_WINDOWS)
/*!
 * \brief   UTF-16LEをUTF-8に変換する
 */
void CoreString::conv(const wchar_t* text)
{
    int32_t len = Util::toUTF8(nullptr, 0, text);

    setCapacity(len);
    Util::toUTF8(getBuffer(), len + 1, text);

    setLength(len);
}

/*!
 * \brief   コンストラクタ
 */
UTF16LE::UTF16LE()
{
    mBuffer = nullptr;
    mChars = 0;
}

/*!
 * \brief   UTF-8をUTF-16LEに変換する
 */
void UTF16LE::conv(const char* text)
{
    int32_t chars = Util::toUnicode(nullptr, 0, text);

    realloc(chars);
    Util::toUnicode(mBuffer, chars + 1, text);
}

/*!
 * \brief   バッファを再確保する
 */
void UTF16LE::realloc(int32_t chars)
{
    if (mChars > chars)
        return;

    delete [] mBuffer;
    mBuffer = new wchar_t[chars + 1];
    mChars = chars;
}
#endif

/*!
 * \brief   文字列比較
 */
bool operator==(const CoreString& str1, const char* str2)
{
    return (strcmp(str1.getBuffer(), str2) == 0);
}

} // namespace slog
