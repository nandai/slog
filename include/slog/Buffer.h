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
 *  \file   Buffer.h
 *  \brief  バッファクラス
 *  \author Copyright 2011-2014 printf.jp
 */
#pragma once
#include "slog/Exception.h"

namespace slog
{

/*!
 *  \brief  バッファクラス
 */
class SLOG_API Buffer
{
            /*!
             * バッファ使用サイズ
             */
            int32_t mLen;

            /*!
             * コンストラクタ
             */
public:     Buffer();

            /*!
             * デストラクタ
             */
            virtual ~Buffer();


            /*!
             * バッファアドレス取得
             */
            virtual char* getBuffer() const = 0;

            /*!
             * バッファサイズ取得
             */
            virtual int32_t getCapacity() const = 0;

            /*!
             * バッファサイズ設定
             */
            virtual void setCapacity(int32_t capacity) throw(Exception);

            /*!
             * 位置取得
             */
            virtual int32_t getPosition() const;

            /*!
             * バッファ使用サイズ取得
             */
            int32_t getLength() const;

            /*!
             * バッファ使用サイズ設定
             */
            virtual void setLength(int32_t len) throw(Exception);

            /*!
             * オーバーフロー検証
             */
            void validateOverFlow(int32_t len) const throw(Exception);

            /*!
             * オーバーフロー検証
             */
            void validateOverFlow(int32_t position, int32_t len) const throw(Exception);
};

/*!
 *  \brief  コンストラクタ
 */
inline Buffer::Buffer()
{
    mLen = 0;
}

/*!
 *  \brief  デストラクタ
 */
inline Buffer::~Buffer()
{
}

/*!
 *  \brief  バッファサイズ設定
 */
inline void Buffer::setCapacity(int32_t capacity) throw(Exception)
{
    Exception e;
    e.setMessage("Buffer::setCapacity / unsupported");

    throw e;
}

/*!
 *  \brief  位置取得
 */
inline int32_t Buffer::getPosition() const
{
    return 0;
}

/*!
 *  \brief  バッファ使用サイズ取得
 */
inline int32_t Buffer::getLength() const
{
    return mLen;
}

/*!
 *  \brief  オーバーフロー検証
 */
inline void Buffer::validateOverFlow(int32_t len) const throw(Exception)
{
    validateOverFlow(getPosition(), len);
}

} // namespace slog
