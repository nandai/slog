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
 * \file    String.h
 * \brief   可変長文字列クラス
 * \author  Copyright 2011-2014 printf.jp
 */
#pragma once
#include "slog/CoreString.h"

namespace slog
{

/*!
 * \brief   可変長文字列クラス
 */
class SLOG_API String : public CoreString
{
            /*!
             * \brief   バッファ
             */
            char* mBuffer;

            /*!
             * \brief   バッファ容量
             */
            int32_t mCapacity;

            /*!
             * 代入
             */
private:    const String& operator=(const char*) = delete;

            /*!
             * 代入
             */
public:     const String& operator=(const String&);

            /*!
             * コンストラクタ
             */
public:     String();

            /*!
             * コンストラクタ
             */
            String(const String& str);

            /*!
             * コンストラクタ
             */
            String(const char* text);

            /*!
             * コンストラクタ
             */
            String(const char* text, int16_t len);

            /*!
             * デストラクタ
             */
            virtual ~String() override;

            /*!
             * 初期化
             */
private:    void init(const char* text, int16_t len);

            /*!
             * バッファアドレス取得
             */
public:     virtual char* getBuffer() const override;

            /*!
             * バッファサイズ取得
             */
            virtual int32_t getCapacity() const override;

            /*!
             * バッファサイズ設定
             */
            virtual void setCapacity(int32_t capacity) throw(Exception) override;
};

/*!
 * \brief   代入
 *
 * \note    std::map等で必要とされる場面があるためpublicで定義してある。基本的にはCoreString::copy()を使用すること。
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
 * \brief   コンストラクタ
 */
inline String::String()
{
    init("", 0);
}

/*!
 * \brief   コンストラクタ
 */
inline String::String(const String& str) : CoreString(str)
{
//  init(str.getBuffer(), str.getCapacity());
    init(str.getBuffer(), str.getLength());
}

/*!
 * \brief   コンストラクタ
 */
inline String::String(const char* text, int16_t len)
{
    for (int16_t i = 0; i < len; i++)
    {
        if (text[i] == '\0')
        {
            len = i;
            break;
        }
    }

    init(text, len);
}

/*!
 * \brief   コンストラクタ
 */
inline String::~String()
{
    delete [] mBuffer;
}

/*!
 * \brief   バッファ取得
 */
inline char* String::getBuffer() const
{
    return mBuffer;
}

/*!
 * \brief   バッファサイズ取得
 */
inline int32_t String::getCapacity() const
{
    return mCapacity;
}

/*!
 * \brief   文字列比較
 */
bool operator<(const String& str1, const String& str2);

} // namespace slog
