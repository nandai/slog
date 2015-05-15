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
 * \file    FixedString.h
 * \brief   固定長文字列クラス
 * \author  Copyright 2011-2015 printf.jp
 */
#pragma once
#include "slog/CoreString.h"

namespace slog
{

/*!
 * \brief   固定長文字列クラス
 */
template <int i>
class FixedString : public CoreString
{
            /*!
             * \brief   バッファサイズ
             */
            static const int CAPACITY = i;

            /*!
             * \brief   バッファ
             */
            char mBuffer[CAPACITY + 1];

            /*!
             * コンストラクタ
             */
public:     FixedString();

            /*!
             * コンストラクタ
             */
            FixedString(const char* text);

            /*!
             * 代入
             */
private:    const FixedString& operator=(const char*);

            /*!
             * 代入
             */
public:     const FixedString& operator=(const FixedString&);

            /*!
             * バッファアドレス取得
             */
            virtual char* getBuffer() const override;

            /*!
             * バッファサイズ取得
             */
            virtual int32_t getCapacity() const override;
};

/*!
 * \brief   コンストラクタ
 */
template <int i>
inline FixedString<i>::FixedString()
{
    mBuffer[0] = '\0';
}

/*!
 * \brief   コンストラクタ
 */
template <int i>
inline FixedString<i>::FixedString(const char* text)
{
    copy(text);
}

/*!
 * \brief   代入
 */
template <int i>
inline const FixedString<i>& FixedString<i>::operator=(const FixedString& str)
{
    copy(&str);
    return *this;
}

/*!
 * \brief   バッファアドレス取得
 */
template <int i>
char* FixedString<i>::getBuffer() const
{
    return (char*)mBuffer;
}

/*!
 * \brief   バッファサイズ取得
 */
template <int i>
int32_t FixedString<i>::getCapacity() const
{
    return CAPACITY;
}

} // namespace slog
