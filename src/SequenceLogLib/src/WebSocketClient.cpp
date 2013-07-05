/*
 * Copyright (C) 2013 printf.jp
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
 *  \file   WebSocketClient.cpp
 *  \brief  Web Socket クライアントクラス
 *  \author Copyright 2013 printf.jp
 */
#include "slog/WebSocketClient.h"
#include "slog/HttpResponse.h"
#include "slog/String.h"

namespace slog
{

/*!
 *  \brief  接続
 */
void WebSocketClient::connect(const CoreString& url, unsigned short port) throw(Exception)
{
    String address;
    String path;
    int32_t i;
    Exception e;

    struct
    {
        const char*     protocol;
        unsigned short  port;
        bool            useSSL;
    }
    candidate[] =
    {
        {"ws://",   80, false},
        {"wss://", 443, true },
    };

    // ドメイン、パス、ポート番号を取得
    for (i = 0; i < sizeof(candidate) / sizeof(candidate[0]); i++)
    {
        int32_t index = url.indexOf(candidate[i].protocol);

        if (index == 0)
        {
            if (port == 0)
                port = candidate[i].port;

            int32_t domainIndex = (int32_t)strlen(candidate[i].protocol);
            int32_t pathIndex = url.indexOf("/", domainIndex);

            const char* p = url.getBuffer();

            if (0 < pathIndex)
            {
                address.copy(p + domainIndex, pathIndex - domainIndex);
                path.   copy(p + pathIndex);
            }
            else
            {
                address.copy(p + domainIndex);
                path.   copy("/");
            }

            break;
        }
    }

    if (address.getLength() == 0)
    {
        e.setMessage("URL '%s' が正しくありません。", url.getBuffer());
        throw e;
    }

    // 接続
    open();
    setRecvTimeOut(3000);
    setNoDelay(true);
    Socket::connect(address, port);

    notifyOpen();

    if (candidate[i].useSSL)
        useSSL();

    // WebSocketアップグレード
    String upgrade;
    upgrade.format(
        "GET %s HTTP/1.1\r\n"
        "Upgrade: websocket\r\n"
        "Sec-WebSocket-Key: m31EnckktzJZ/3ZWkvwNHQ==\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "\r\n",
        path.getBuffer());

    Socket::send(&upgrade, upgrade.getLength());

    // WebSocketアップグレードレスポンス受信
    HttpResponse httpResponse(this);

    if (httpResponse.analizeResponse() == false)
    {
        e.setMessage("WebSocketへのアップグレードに失敗しました。");
        throw e;
    }

    init();
}

} // namespace slog
