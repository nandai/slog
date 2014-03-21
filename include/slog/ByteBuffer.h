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
 * \file    ByteBuffer.h
 * \brief   バイトバッファクラス
 * \author  Copyright 2011-2014 printf.jp
 */
#pragma once
#include "slog/Buffer.h"

namespace slog
{

/*!
 *  \brief  バイトバッファクラス
 */
class SLOG_API ByteBuffer : public Buffer
{
            /*!
             * 位置
             */
            int32_t mPosition;

            /*!
             * 容量
             */
            int32_t mCapacity;

            /*!
             * バッファ
             */
            uint8_t* mBuffer;

            /*!
             * コンストラクタ
             */
public:      ByteBuffer(int32_t capacity);

            /*!
             * デストラクタ
             */
            ~ByteBuffer();

            /*!
             * バッファアドレス取得
             */
            virtual char* getBuffer() const override;

            /*!
             * 容量取得
             */
            virtual int32_t getCapacity() const override;

            /*!
             * 位置取得
             */
            virtual int32_t getPosition() const override;

            /*!
             * 位置設定
             */
            void setPosition(int32_t position) throw(Exception);

            /*!
             * 位置を進める
             */
private:    void addPosition(int32_t len) throw(Exception);

            /*!
             * バッファ使用サイズ設定
             */
public:     virtual void setLength(int32_t len) throw(Exception) override;

            /*!
             * char値取得
             */
            char get();

            /*!
             * char値書き込み
             */
            void put(char value) throw(Exception);

            /*!
             * short値取得
             */
            short getShort();

            /*!
             * short値書き込み
             */
            void putShort(short value) throw(Exception);

            /*!
             * int32_t値取得
             */
            int32_t getInt();

            /*!
             * int32_t値書き込み
             */
            void putInt(int32_t value) throw(Exception);

            /*!
             * int64_t値取得
             */
            int64_t getLong();

            /*!
             * int64_t値書き込み
             */
            void putLong(int64_t value) throw(Exception);

            /*!
             * 一括取得
             */
            char* get(int32_t len);

            /*!
             * 一括書き込み
             */
            void put(const Buffer* buffer, int32_t len) throw(Exception);

            /*!
             * 一括書き込み
             */
            void put(const char* buffer, int32_t len) throw(Exception);
};

/*!
 *  \brief  バッファアドレス取得
 */
inline char* ByteBuffer::getBuffer() const
{
    return (char*)mBuffer;
}

/*!
 *  \brief  バッファサイズ取得
 */
inline int32_t ByteBuffer::getCapacity() const
{
    return mCapacity;
}

/*!
 *  \brief  位置取得
 */
inline int32_t ByteBuffer::getPosition() const
{
    return mPosition;
}

/*!
 *  \brief  位置を進める
 */
inline void ByteBuffer::addPosition(int32_t len) throw(Exception)
{
    setPosition(getPosition() + len);
}

/*!
 *  \brief  バッファ使用サイズ設定
 */
inline void ByteBuffer::setLength(int32_t len) throw(Exception)
{
    Buffer::setLength(len);

    if (getPosition() > len)
        setPosition(len);
}

} // namespace slog
