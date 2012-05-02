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
 *  \brief  �R�A������N���X
 *  \author Copyright 2011-2012 log-tools.net
 */
#include "slog/CoreString.h"
using namespace slog;

bool CoreString::sSJIS = true;

/*!
 *  \brief  ��������R�s�[����
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
 *  \brief  �������ǉ�����
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
 *  \brief  �t�H�[�}�b�g
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
        len = vsnprintf(p, capacity + 1, format, arg);  // �o�b�t�@�T�C�Y�ɂ͏I�[��'\0'���܂܂��
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

#if defined(_WINDOWS)
/*!
 *  \brief  SJIS�A�܂���UTF-8��UTF-16LE�ɕϊ�����
 *
 *  \note   �߂�l�̃o�b�t�@�͌Ăяo������delete���邱��
 */
void CoreString::toUTF16LE(UTF16LE* utf16le) const
{
    bool sjis = isSJIS();
    UINT codePage = (sjis ? CP_ACP : CP_UTF8);

	int chars = 
    MultiByteToWideChar(codePage, 0, getBuffer(), -1, NULL, 0) - 1;

    if (utf16le->chars < chars)
    {
        delete [] utf16le->buffer;
        utf16le->buffer = new wchar_t[chars + 1];
        utf16le->chars = chars;
    }

    MultiByteToWideChar(codePage, 0, getBuffer(), -1, utf16le->buffer, chars + 1);
}
#endif
