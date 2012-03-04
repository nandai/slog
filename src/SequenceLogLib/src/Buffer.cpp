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
 *  \file   Buffer.cpp
 *  \brief  バッファクラス
 *  \author Copyright 2011 log-tools.net
 */
#include "slog/Buffer.h"

namespace slog
{

/*!
 *  \brief  バッファ使用サイズ設定
 */
void Buffer::setLength(int32_t len) throw(Exception)
{
    if (len < 0 || getCapacity() < len)
    {
        Exception e;
        e.setMessage("Buffer::setLength(%d) capacity=%d / illegal length", len, getCapacity());

        throw e;
    }

    mLen = len;
}

/*!
 *  \brief  バッファオーバーフローしないか確認する
 */
void Buffer::validateOverFlow(int32_t position, int32_t len) const throw(Exception)
{
    if (getCapacity() < position + len)
    {
        Exception e;

        e.setMessage(
            "Buffer::validateOverFlow() capacity=%d, position=%d, len=%d / buffer overflow",
            getCapacity(), getPosition(), len);

        throw e;
    }
}

} // namespace slog
