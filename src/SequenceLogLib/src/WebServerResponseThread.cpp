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
 *  \file   WebServerResponseThread.cpp
 *  \brief  WEBサーバー応答スレッドクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/WebServerResponseThread.h"
#include "slog/HttpRequest.h"
#include "slog/Socket.h"
#include "slog/Util.h"
#include "slog/File.h"
#include "slog/ByteBuffer.h"

#include "sha1.h"

namespace slog
{

/*!
 *  \brief  コンストラクタ
 */
WebServerResponseThread::WebServerResponseThread(HttpRequest* httpRequest)
{
    mHttpRequest = httpRequest;
}

/*!
 *  \brief  デストラクタ
 */
WebServerResponseThread::~WebServerResponseThread()
{
    delete mHttpRequest;
}

/*!
 *  \brief  送信
 */
void WebServerResponseThread::send(const CoreString& content) const
{
    // HTTPヘッダー送信
    int32_t contentLen = content.getLength();
    sendHttpHeader(contentLen);

    // コンテンツ送信
    sendContent(content);
}

/*!
 *  \brief  HTTPヘッダー送信（＆切断）
 */
void WebServerResponseThread::sendHttpHeader(int32_t contentLen) const
{
    String str;
    str.format(
        "HTTP/1.1 200 OK\n"
        "Content-type: text/html; charset=UTF-8\n"
        "Content-length: %d\n"
        "\n",
        contentLen);

    Socket* socket = mHttpRequest->getSocket();
    socket->send(&str, str.getLength());

    if (contentLen == 0)
        socket->close();
}

/*!
 *  \brief  応答内容送信＆切断
 */
void WebServerResponseThread::sendContent(const CoreString& content) const
{
    Socket* socket = mHttpRequest->getSocket();

    socket->send(&content, content.getLength());
    socket->close();
}

/*!
 *  \brief  実行
 */
void WebServerResponseThread::run()
{
    try
    {
        String content;

        HttpRequest::METHOD method = mHttpRequest->getMethod();
        const CoreString& url =      mHttpRequest->getUrl();

        do
        {
            if (getContents(&content, url.getBuffer()))
                break;

            if (getContents(&content, "notfound.html"))
                break;

            mHttpRequest->getSocket()->close();
            return;
        }
        while (false);

        // 送信
        send(content);
    }
    catch (Exception& e)
    {
        noticeLog(e.getMessage());
    }
}

/*!
 *  \brief  コンテンツ取得
 */
bool WebServerResponseThread::getContents(String* content, const char* url)
{
    try
    {
        const CoreString& ip = mHttpRequest->getSocket()->getMyInetAddress();

        String processPath;
        Util::getProcessPath(&processPath);
        const char* rootDir = getRootDir();

        String htmlPath;
        htmlPath.format("%s%c%s%c%s",
            processPath.getBuffer(),
            PATH_DELIMITER,
            rootDir,
            PATH_DELIMITER,
            url);

        File file;
        file.open(htmlPath, File::READ);

        String buffer;
        while (file.read(&buffer))
        {
            const char* p = buffer.getBuffer();
            int32_t index = 0;

            while (true)
            {
                // DOMAIN変換
                const char* find = "<? DOMAIN ?>";
                int32_t pos = buffer.indexOf(find, index);

                if (0 <= pos)
                {
                    content->append(p + index, pos - index);
                    content->append(getDomain());
                    index = (int32_t)(pos + strlen(find));
                    continue;
                }

                // WS変換
                find = "<? WS ?>";
                pos = buffer.indexOf(find);

                if (0 <= pos)
                {
                    String ws;
                    ws.format("%s:%u", ip.getBuffer(), mHttpRequest->getPort());

                    content->append(p + index, pos - index);
                    content->append(ws.getBuffer());
                    index = (int32_t)(pos + strlen(find));
                }

                // その他
                content->append(p + index);
                break;
            }

            content->append("\n");
        }
    }
    catch (Exception& e)
    {
        noticeLog(e.getMessage());
        return false;
    }

    return true;
}

/*!
 *  \brief  WebSocketにアップグレード
 */
bool WebServerResponseThread::upgradeWebSocket()
{
    const CoreString& webSocketKey = mHttpRequest->getWebSocketKey();

    if (webSocketKey.getLength() == 0)
        return false;

    String mes;
    mes.format("%s%s", webSocketKey.getBuffer(), "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

    uint8_t digest[SHA1HashSize];

    SHA1Context sha;
    SHA1Reset( &sha);
    SHA1Input( &sha, (uint8_t*)mes.getBuffer(), mes.getLength());
    SHA1Result(&sha, digest);

    String resValue;
    Util::encodeBase64(&resValue, (char*)digest, SHA1HashSize);

    // 応答内容送信
    String str;
    str.format(
        "HTTP/1.1 101 OK\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n"
        "\r\n",
        resValue.getBuffer());

    mHttpRequest->getSocket()->send(&str, str.getLength());
    return true;
}

/*!
 *  \brief  WebSocketヘッダー送信
 */
void WebServerResponseThread::sendWebSocketHeader(uint64_t payloadLen, bool isText, bool toClient) const
{
    Socket* socket = mHttpRequest->getSocket();
    sendWebSocketHeader(socket, payloadLen, isText, toClient);
}

void WebServerResponseThread::sendWebSocketHeader(Socket* socket, uint64_t payloadLen, bool isText, bool toClient)
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
        // 0 〜 125 bytes
        buffer.put(mask | (char)payloadLen);
    }
    else if (payloadLen <= 0xFFFF)
    {
        // 126 〜 65535 bytes
        buffer.put(mask | (char)126);
        buffer.putShort((short)payloadLen);
    }
    else
    {
        // 65536 〜
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

    socket->send(&buffer, buffer.getLength());
}

/*!
 *  \brief  データ受信
 */
ByteBuffer* WebServerResponseThread::recvData(Socket* socket, ByteBuffer* dataBuffer)
{
    ByteBuffer buffer(2 + 8 + 4);
    const char* p = buffer.getBuffer();

    socket->recv(&buffer, 2);

    // opcode
    char opcode = p[0] & 0x0F;

    if (opcode != 0x01 && opcode != 0x02)
    {
        noticeLog("opcode=0x%02X", opcode);
        return NULL;
    }

    // MASK & Payload length
    bool mask = ((p[1] & 0x80) == 0x80);
    uint64_t payloadLen = p[1] & 0x7F;

    if (payloadLen == 126)
    {
        socket->recv(&buffer, 2);
        payloadLen = buffer.getShort();
    }
    else if (payloadLen == 127)
    {
        socket->recv(&buffer, 8);
        payloadLen = buffer.getLong();
    }

    // Masking-key
    if (mask)
        socket->recv(&buffer, 4);

    // Payload Data
    if (dataBuffer)
    {
        if (payloadLen != dataBuffer->getCapacity())
        {
            Exception e;
            e.setMessage("payloadLen=%d, dataBufferLen=%d", payloadLen, dataBuffer->getCapacity());

            throw e;
        }
    }
    else
    {
        dataBuffer = new ByteBuffer((int32_t)payloadLen);
    }

//  socket->recv(dataBuffer, payloadLen);   どうするか検討
    socket->recv(dataBuffer, (int32_t)payloadLen);

    if (mask)
    {
        char* p2 = dataBuffer->getBuffer();

        for (uint64_t i = 0; i < payloadLen; i++)
            p2[i] ^= p[i % 4];
    }

    return dataBuffer;
}

} // namespace slog
