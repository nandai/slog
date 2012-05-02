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
            static bool sSJIS;  // 共通設定                 false: UTF-8, true: SJIS
            int32_t     mSJIS;  // 個別設定 -1: sJISに依存,     0: UTF-8,    1: SJIS

public:     CoreString() {mSJIS = -1;}

private:    const CoreString& operator=(const char*);
protected:  const CoreString& operator=(const CoreString& str)
            {
                if (this != &str)
                    copy(str);

                return *this;
            }

public:     void copy(const char* text, int32_t len = -1) throw(Exception);
            void copy(const CoreString& str) throw(Exception)
            {
                mSJIS = str.mSJIS;
                copy(str.getBuffer(), str.getLength());
            }

            void append(const char* text, int32_t len = -1) throw(Exception);
            void append(const CoreString& str) throw(Exception) {append(str.getBuffer(), str.getLength());}

            char operator[](int32_t index) const;

            virtual void setLength(int32_t len) throw(Exception);

            void format( const char* format, ...) throw(Exception);
            void formatV(const char* format, va_list arg) throw(Exception);

            bool equals(const CoreString& str) const {return (strcmp(getBuffer(), str.getBuffer()) == 0);}

    		static bool  isCommonSJIS()   {return sSJIS;}
			static void setCommonSJIS(bool sjis) {sSJIS = sjis;}

            bool  isSJIS() const {return (mSJIS == -1 ? isCommonSJIS() : (mSJIS == 1));}
            void setSJIS(int32_t sjis) {mSJIS = sjis;}

#if defined(_WINDOWS)
            void conv(const wchar_t* text);
#endif
};

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
 *  \brief  文字列比較
 */
inline bool operator==(const CoreString& str1, const char* str2)
{
    return (strcmp(str1.getBuffer(), str2) == 0);
}


#if defined(_WINDOWS)
class UTF16LE
{
            wchar_t*    mBuffer;
            int32_t     mChars;

public:     UTF16LE()
            {
                mBuffer = NULL;
                mChars = 0;
            }

            ~UTF16LE()
            {
                delete [] mBuffer;
            }

            wchar_t* getBuffer() const {return mBuffer;}
            int32_t getChars() const {return mChars;}

            void conv(const char* text, int32_t sjis = -1);
            void conv(const CoreString& str) {conv(str.getBuffer(), str.isSJIS());}

private:    void realloc(int32_t chars);
};
#endif

} // namespace slog
