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
 *  \file   HttpResponse.cpp
 *  \brief  httpレスポンスクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/HttpResponse.h"
#include "slog/Socket.h"
#include "slog/ByteBuffer.h"

namespace slog
{

/*!
 *  \brief  コンストラクタ
 */
HttpResponse::HttpResponse(Socket* socket)
{
    mSocket = socket;
}

/*!
 *  \brief  レスポンス解析
 */
bool HttpResponse::analizeResponse()
{
    int32_t size = 1;
    ByteBuffer buffer(size);

    char response[1024 + 1];
    int32_t i = 0;
    int32_t contentLen = 0;

    while (true)
    {
        // 受信
        buffer.setLength(0);
        mSocket->recv(&buffer, size);

        // 改行までリクエストバッファに貯める
        char c = buffer.get();

        if (c != '\r')
        {
            if (sizeof(response) <= i)
                return false;

            response[i] = c;
            i++;

            continue;
        }

        response[i] = '\0';

        // '\n'捨て
        buffer.setLength(0);
        mSocket->recv(&buffer, size);

        if (i == 0)
            break;

        i = 0;
    }

    return true;
}

} // namespace slog
