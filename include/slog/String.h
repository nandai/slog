/*
 * Copyright (C) 2011-2013 printf.jp
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
 *  \file   String.h
 *  \brief  可変長文字列クラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once
#include "slog/CoreString.h"

namespace slog
{

/*!
 *  \brief  可変長文字列クラス
 */
class SLOG_API String : public CoreString
{
            char*   mBuffer;    //!< バッファ
            int32_t mCapacity;  //!< バッファ容量

private:    const String& operator=(const char*);
//          const String& operator=(const String&);
public:     const String& operator=(const String&);

            /*!
             * コンストラクタ／デストラクタ
             */
public:     String();
            String(const String& str);
            String(const char* text);
            String(const char* text, int16_t len);

            virtual ~String();

private:    void init(const char* text, int16_t len);

public:     virtual char* getBuffer() const;

            virtual int32_t getCapacity() const;
            virtual void setCapacity(int32_t capacity) throw(Exception);
};

/*!
 *  \brief  代入
 *
 *  \note   std::map等で必要とされる場面があるためpublicで定義してある。基本的にはCoreString::copy()を使用すること。
 */
inline const String& String::operator=(const String& str)
{
    if (this != &str)
    {
        CoreString::operator=(str);

        delete [] mBuffer;
//      init(str.getBuffer(), str.getCapacity());
        init(str.getBuffer(), str.getLength());
    }

    return *this;
}

/*!
 *  \brief  コンストラクタ
 */
inline String::String()
{
    init("", 0);
}

/*!
 *  \brief  コンストラクタ
 */
inline String::String(const String& str) : CoreString(str)
{
//  init(str.getBuffer(), str.getCapacity());
    init(str.getBuffer(), str.getLength());
}

/*!
 *  \brief  コンストラクタ
 */
inline String::String(const char* text, int16_t len)
{
    init(text, len);
}

/*!
 *  \brief  コンストラクタ
 */
inline String::~String()
{
    delete [] mBuffer;
}

/*!
 *  \brief  バッファ取得
 */
inline char* String::getBuffer() const
{
    return mBuffer;
}

/*!
 *  \brief  バッファサイズ取得
 */
inline int32_t String::getCapacity() const
{
    return mCapacity;
}

/*!
 *  \brief  文字列比較
 */
bool operator<(const String& str1, const String& str2);

} // namespace slog
