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
 *  \file   ByteBuffer.cpp
 *  \brief  バイトバッファクラス
 *  \author Copyright 2011 log-tools.net
 */
#include "slog/ByteBuffer.h"

#if defined(__unix__)
    #include <string.h>
#endif

namespace slog
{

/*!
 *  \brief  コンストラクタ
 */
ByteBuffer::ByteBuffer(int32_t capacity)
{
    mPosition = 0;
    mCapacity = capacity;
    mBuffer = new uint8_t[mCapacity];
}

/*!
 *  \brief  デストラクタ
 */
ByteBuffer::~ByteBuffer()
{
    delete [] mBuffer;
}

/*!
 *  \brief  位置設定
 */
void ByteBuffer::setPosition(int32_t position) throw(Exception)
{
    if (position < 0 || getCapacity() < position)
    {
        Exception e;

        e.setMessage(
            "ByteBuffer::setPosition() length=%d, position=%d / illegal position",
            getLength(), position);

        throw e;
    }

    mPosition = position;

    if (getLength() < position)
        setLength(position);
}

/*!
 *  \brief  char値取得
 */
char ByteBuffer::get()
{
    char value;

    int32_t len = sizeof(value);
    validateOverFlow(len);

    uint8_t* p = mBuffer + mPosition;
    value = p[0];

    addPosition(len);
    return value;
}

/*!
 *  \brief  char値書き込み
 */
void ByteBuffer::put(char value) throw(Exception)
{
    int32_t len = sizeof(value);
    validateOverFlow(len);

    uint8_t* p = mBuffer + mPosition;
    p[0] = value;

    addPosition(len);
}

/*!
 *  \brief  short値取得
 */
short ByteBuffer::getShort()
{
    short value;

    int32_t len = sizeof(value);
    validateOverFlow(len);

    uint8_t* p = mBuffer + mPosition;
    value =
        ((uint16_t)p[0] << 8) |
        ((uint16_t)p[1]     );

    addPosition(len);
    return value;
}

/*!
 *  \brief  short値書き込み
 */
void ByteBuffer::putShort(short value) throw(Exception)
{
    int32_t len = sizeof(value);
    validateOverFlow(len);

    uint8_t* p = mBuffer + mPosition;

    p[0] = (value >> 8) & 0xFF;
    p[1] = (value     ) & 0xFF;

    addPosition(len);
}

/*!
 *  \brief  int32_t値取得
 */
int32_t ByteBuffer::getInt()
{
    int32_t value;

    int32_t len = sizeof(value);
    validateOverFlow(len);

    uint8_t* p = mBuffer + mPosition;
    value =
        ((uint32_t)p[0] << 24) |
        ((uint32_t)p[1] << 16) |
        ((uint32_t)p[2] <<  8) |
        ((uint32_t)p[3]      );

    addPosition(len);
    return value;
}

/*!
 *  \brief  int32_t値書き込み
 */
void ByteBuffer::putInt(int32_t value) throw(Exception)
{
    int32_t len = sizeof(value);
    validateOverFlow(len);

    uint8_t* p = mBuffer + mPosition;

    p[0] = (value >> 24) & 0xFF;
    p[1] = (value >> 16) & 0xFF;
    p[2] = (value >>  8) & 0xFF;
    p[3] = (value      ) & 0xFF;

    addPosition(len);
}

/*!
 *  \brief  int64_t値取得
 */
int64_t ByteBuffer::getLong()
{
    int64_t value;

    int32_t len = sizeof(value);
    validateOverFlow(len);

    uint8_t* p = mBuffer + mPosition;
    value =
        ((uint64_t)p[0] << 56) |
        ((uint64_t)p[1] << 48) |
        ((uint64_t)p[2] << 40) |
        ((uint64_t)p[3] << 32) |
        ((uint64_t)p[4] << 24) |
        ((uint64_t)p[5] << 16) |
        ((uint64_t)p[6] <<  8) |
        ((uint64_t)p[7]      );

    addPosition(len);
    return value;
}

/*!
 *  \brief  int64_t値書き込み
 */
void ByteBuffer::putLong(int64_t value) throw(Exception)
{
    int32_t len = sizeof(value);
    validateOverFlow(len);

    uint8_t* p = mBuffer + mPosition;

    p[0] = (value >> 56) & 0xFF;
    p[1] = (value >> 48) & 0xFF;
    p[2] = (value >> 40) & 0xFF;
    p[3] = (value >> 32) & 0xFF;
    p[4] = (value >> 24) & 0xFF;
    p[5] = (value >> 16) & 0xFF;
    p[6] = (value >>  8) & 0xFF;
    p[7] = (value      ) & 0xFF;

    addPosition(len);
}

/*!
 *  \brief  一括取得
 */
char* ByteBuffer::get(int32_t len)
{
    validateOverFlow(len);

    uint8_t* p = mBuffer + mPosition;

    addPosition(len);
    return (char*)p;
}

/*!
 *  \brief  一括書き込み
 */
void ByteBuffer::put(const Buffer* buffer, int32_t len) throw(Exception)
{
    buffer->validateOverFlow(0, len);
    validateOverFlow(len);

    uint8_t* p = mBuffer + mPosition;

    memcpy(p, buffer->getBuffer(), len);
    addPosition(len);
}

} // namespace slog
