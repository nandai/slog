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
 *  \file   HttpResponse.cpp
 *  \brief  httpレスポンスクラス
 *  \author Copyright 2011-2014 printf.jp
 */
#include "slog/HttpResponse.h"
#include "slog/Socket.h"
#include "slog/ByteBuffer.h"

#include <string.h>
#include <stdlib.h>

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
    int32_t contentLen = 0;

    while (true)
    {
        String str;
        mSocket->recv(&str);

        const char* request = str.getBuffer();
        int32_t i =           str.getLength();

        if (i == 0)
        {
            // 空行だったらループを抜ける
            if (0 < contentLen)
            {
                mResponse.setCapacity(contentLen);
                mSocket->recv(&mResponse, contentLen);
                break;
            }
//          noticeLog("analizeRequest ended");

            break;
        }
        else
        {
            // Content-Length
            const char* compare = "Content-Length: ";
            int32_t compareLen = (int32_t)strlen(compare);

            if (strncmp(request, compare, compareLen) == 0)
            {
                contentLen = atoi(request + compareLen);
            }
        }
    }

    return true;
}

} // namespace slog
