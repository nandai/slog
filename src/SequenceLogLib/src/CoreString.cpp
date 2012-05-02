/*
 * Copyright (C) 2011-2012 log-tools.net
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
 *  \file   CoreString.cpp
 *  \brief  コア文字列クラス
 *  \author Copyright 2011-2012 log-tools.net
 */
#include "slog/CoreString.h"
using namespace slog;

bool CoreString::sSJIS = true;

/*!
 *  \brief  文字列をコピーする
 */
void CoreString::copy(const char* text, int32_t len) throw(Exception)
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
void CoreString::append(const char* text, int32_t len) throw(Exception)
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
 *  \brief  フォーマット
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
 *  \brief  検索
 */
int32_t CoreString::find(char c) const
{
    const char* buffer = getBuffer();
    const char* p = strchr(buffer, c);

    if (p == NULL)
        return -1;

    return (int32_t)(p - buffer);
}

#if defined(_WINDOWS)
/*!
 *  \brief  UTF-16LEをSJIS、またはUTF-8に変換する
 */
void CoreString::conv(const wchar_t* text)
{
    UINT codePage = (isSJIS() ? CP_ACP : CP_UTF8);
    int32_t len = (int32_t)wcslen(text);

	long size =
	WideCharToMultiByte(codePage, 0, text, len + 1, NULL, NULL, NULL, NULL);

    setCapacity(size - 1/*sizeof('\0')*/);
	WideCharToMultiByte(codePage, 0, text, len + 1, getBuffer(), size, NULL, NULL);

    setLength(size - 1);
}

/*!
 *  \brief  SJIS、またはUTF-8をUTF-16LEに変換する
 */
void UTF16LE::conv(const char* text, int32_t sjis)
{
    bool isSJIS = (sjis == -1 ? CoreString::isCommonSJIS() : (sjis == 1));
    UINT codePage = (isSJIS ? CP_ACP : CP_UTF8);

    int32_t chars = 
    MultiByteToWideChar(codePage, 0, text, -1, NULL, 0) - 1;

    realloc(chars);
    MultiByteToWideChar(codePage, 0, text, -1, mBuffer, chars + 1);
}

void UTF16LE::realloc(int32_t chars)
{
    if (mChars >= chars)
        return;

    delete [] mBuffer;
    mBuffer = new wchar_t[chars + 1];
    mChars = chars;
}
#endif
