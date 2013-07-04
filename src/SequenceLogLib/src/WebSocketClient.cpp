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
#include "slog/ByteBuffer.h"

namespace slog
{

/*!
 * コンストラクタ
 */
WebSocket::WebSocket(bool isServer)
{
    mIsServer = isServer;
    mPayloadLen = 0;
    mIsText = false;
}

/*!
 * Web Socket ヘッダー送信
 */
void WebSocket::sendHeader(uint64_t payloadLen, bool isText, bool toClient) throw(Exception)
{
    if (mPayloadLen != 0)
    {
        Exception e;

        e.setMessage("送信予定のデータ長に満たないうちに次のデータを送信しようとしました。送信残 %s byte(s)", mPayloadLen);
        throw e;
    }

    mPayloadLen = payloadLen;
    mIsText = isText;

    WebSocket::sendHeader(this, payloadLen, isText, toClient);
}

/*!
 * Web Socket ヘッダー送信
 */
void WebSocket::sendHeader(Socket* socket, uint64_t payloadLen, bool isText, bool toClient) throw(Exception)
{
    ByteBuffer buffer(2 + 8 + 4);
    char opcode = 0x01;     // text frame
    char mask =   0x00;     // no mask

    if (isText == false)
    {
        // binary frame
        opcode = 0x02;
    }

    if (toClient == false)
    {
        // client -> server
        mask = (char)0x80;
    }

    // FIN & opcode
    buffer.put((char)0x80 | opcode);

    // MASK & Payload length
    if (payloadLen < 126)
    {
        // 0 ～ 125 bytes
        buffer.put(mask | (char)payloadLen);
    }
    else if (payloadLen <= 0xFFFF)
    {
        // 126 ～ 65535 bytes
        buffer.put(mask | (char)126);
        buffer.putShort((short)payloadLen);
    }
    else
    {
        // 65536 ～
        buffer.put(mask | (char)127);
        buffer.putLong(payloadLen);
    }

    // Masking-key
    if (toClient == false)
    {
        buffer.put((char)0x00);
        buffer.put((char)0x00);
        buffer.put((char)0x00);
        buffer.put((char)0x00);
    }

    socket->Socket::send(&buffer, buffer.getLength());
}

/*!
 *  \brief  送信前チェック
 */
void WebSocket::check(uint64_t len) throw(Exception)
{
    if (mPayloadLen == 0)
    {
        sendHeader(len, false, mIsServer);
    }
    else if (mIsText)
    {
        Exception e;

        e.setMessage("データタイプがテキストに設定されている時に、バイナリデータを送信しようとしました。");
        throw e;
    }

    mPayloadLen -= len;

    if (mPayloadLen < 0)
    {
        Exception e;

        e.setMessage("送信予定のデータ長を超えてしまいました。");
        throw e;
    }
}

/*!
 *  \brief  バイナリ送信
 */
void WebSocket::send(const int32_t* value) const throw(Exception)
{
    ((WebSocket*)this)->check(sizeof(*value));
    Socket::send(value);
}

/*!
 *  \brief  バイナリ送信
 */
void WebSocket::send(const uint32_t* value) const throw(Exception)
{
    ((WebSocket*)this)->check(sizeof(*value));
    Socket::send(value);
}

/*!
 *  \brief  バイナリ送信
 */
void WebSocket::send(const Buffer* buffer, int32_t len) const throw(Exception)
{
    ((WebSocket*)this)->check(len);
    Socket::send(buffer, len);
}

/*!
 *  \brief  バイナリ送信
 */
void WebSocket::send(const char* buffer, int32_t len) const throw(Exception)
{
    ((WebSocket*)this)->check(len);
    Socket::send(buffer, len);
}

/*!
 * テキスト送信
 */
void WebSocket::send(const CoreString& str) const throw(Exception)
{
    WebSocket* self = (WebSocket*)this;
    uint64_t len = str.getLength();

    self->sendHeader(len, true, mIsServer);
    self->mPayloadLen -= len;
}

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
}

} // namespace slog
