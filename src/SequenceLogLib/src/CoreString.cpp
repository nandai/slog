/*
 * Copyright (C) 2011-2012 printf.jp
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
 *  \author Copyright 2011-2012 printf.jp
 */
#include "slog/CoreString.h"
using namespace slog;

bool CoreString::sSJIS = true;

/*!
 *  \brief  次の文字へのバイト数を取得する
 *  \note   参考：http://ja.wikipedia.org/wiki/UTF-8
 */
static int32_t getNextCharBytes(const char* text)
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
 *  \brief  前の文字へのバイト数を取得する
 */
static int32_t getPrevCharBytes(const char* text)
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
 *  \brief  文字列をコピーする
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
 *  \brief  文字列を追加する
 */
void CoreString::append(const char* text, int32_t len) throw(Exception)
{
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
 *  \brief  文字列を挿入する
 */
void CoreString::insert(int32_t pos, const char* aText, int32_t aLen) throw(Exception)
{
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

/*!
 *  \brief  文字数を取得する
 */
int32_t CoreString::getCharacters() const
{
    const char* text = getBuffer();
	int32_t num = 0;

	while (*text)
	{
		text += getNextCharBytes(text);
		num++;
	}

	return num;
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

/*!
 *  \brief  SJIS、またはUTF-8をUTF-16LEに変換する
 */
void UTF16LE::realloc(int32_t chars)
{
    if (mChars >= chars)
        return;

    delete [] mBuffer;
    mBuffer = new wchar_t[chars + 1];
    mChars = chars;
}
#endif
