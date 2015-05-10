/*
 * Copyright (C) 2011-2015 printf.jp
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
 * \file    CoreString.h
 * \brief   コア文字列クラス
 * \author  Copyright 2011-2015 printf.jp
 */
#pragma once

#include "slog/Buffer.h"
#include <stdarg.h>

namespace slog
{
int32_t SLOG_API getNextCharBytes(const char* text);
int32_t SLOG_API getPrevCharBytes(const char* text);

/*!
 * \brief   コア文字列クラス
 */
class SLOG_API CoreString : public Buffer
{
            /*!
             * コンストラクタ
             */
public:     CoreString() {}

            /*!
             * 代入
             */
private:    const CoreString& operator=(const char*);

            /*!
             * 代入
             */
protected:  const CoreString& operator=(const CoreString& str)
            {
                if (this != &str)
                    copy(&str);

                return *this;
            }

            /*!
             * コピー
             */
public:     void copy(const char* text, int32_t len = -1) throw(Exception);

            /*!
             * コピー
             */
            void copy(const CoreString* str) throw(Exception)
            {
                copy(str->getBuffer(), str->getLength());
            }

            /*!
             * 連結
             */
            void append(const char* text, int32_t len = -1) throw(Exception);

            /*!
             * 連結
             */
            void append(const CoreString* str) throw(Exception) {append(str->getBuffer(), str->getLength());}

            /*!
             * 挿入
             */
            void insert(int32_t pos, const char* text, int32_t len = -1) throw(Exception);

            /*!
             * 最後の１文字を削除
             */
            void deleteLast();

            /*!
             * １文字取得
             */
            char at(int32_t index) const;

            /*!
             * １文字取得
             */
            char operator[](int32_t index) const {return at(index);}

            /*!
             * 文字列長（バイト数）設定
             */
            virtual void setLength(int32_t len) throw(Exception) override;

            /*!
             * フォーマット
             */
            void format( const char* format, ...) throw(Exception);

            /*!
             * フォーマット
             */
            void formatV(const char* format, va_list arg) throw(Exception);

            /*!
             * 比較
             */
            bool equals(const CoreString* str) const;

            /*!
             * 比較
             */
            bool equals(const char* text) const;

            /*!
             * 比較
             */
            int32_t compareTo(const CoreString* str) const;

            /*!
             * 検索
             */
            int32_t indexOf(char c) const;

            /*!
             * 検索
             */
            int32_t indexOf(const char* find, int32_t startIndex = 0, int32_t count = -1) const;

            /*!
             * 検索
             */
            int32_t lastIndexOf(const char* find, int32_t index = -1) const;

            /*!
             * 文字数取得
             */
            int32_t getCharacters() const;

            /*!
             * 次の文字へのバイト数を取得する
             */
            int32_t getNextCharBytes(int32_t pos) const;

#if defined(_WINDOWS)
            /*!
             * UTF-16LEをUTF-8に変換する
             */
            void conv(const wchar_t* text);
#endif
};

/*!
 * \brief   １文字取得
 */
inline char CoreString::at(int index) const
{
    char* p = getBuffer();
    return p[index];
}

/*!
 * \brief   文字列長（バイト数）設定
 */
inline void CoreString::setLength(int32_t len) throw(Exception)
{
    Buffer::setLength(len);

    char* p = getBuffer();
    p[len] = '\0';
}

/*!
 * \brief   文字列比較
 */
bool SLOG_API operator==(const CoreString& str1, const char* str2);

#if defined(_WINDOWS)
class SLOG_API UTF16LE
{
            wchar_t*    mBuffer;
            int32_t     mChars;

            /*!
             * コンストラクタ
             */
public:      UTF16LE();

             /*!
             * デストラクタ
             */
            ~UTF16LE()
            {
                delete [] mBuffer;
            }

            wchar_t* getBuffer() const {return mBuffer;}
            int32_t getChars() const {return mChars;}

            void conv(const char* text);
            void conv(const CoreString* str) {conv(str->getBuffer());}

private:    void realloc(int32_t chars);
};
#endif

} // namespace slog
