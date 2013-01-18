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
 *  \file   ByteBuffer.h
 *  \brief  バイトバッファクラス
 *  \author Copyright 2011-2013 printf.jp
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
            int32_t     mPosition;      //!< 位置
            int32_t     mCapacity;      //!< 容量
            uint8_t*    mBuffer;        //!< バッファ

public:      ByteBuffer(int32_t capacity);
            ~ByteBuffer();

            virtual char* getBuffer() const;
            virtual int32_t getCapacity() const;

            virtual int32_t getPosition() const;
            void    setPosition(int32_t position) throw(Exception);
private:    void    addPosition(int32_t len) throw(Exception);

public:     virtual void setLength(int32_t len) throw(Exception);

            char    get();
            void    put(char value) throw(Exception);

            short   getShort();
            void    putShort(short value) throw(Exception);

            int32_t getInt();
            void    putInt(int32_t value) throw(Exception);

            int64_t getLong();
            void    putLong(int64_t value) throw(Exception);

            char*   get(int32_t len);
            void    put(const Buffer* buffer, int32_t len) throw(Exception);
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
 *  \brief  バッファ使用サイズ取得
 */
inline void ByteBuffer::setLength(int32_t len) throw(Exception)
{
    Buffer::setLength(len);

    if (getPosition() > len)
        setPosition(len);
}

} // namespace slog
