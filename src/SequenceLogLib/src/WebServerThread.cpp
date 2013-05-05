﻿/*
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
 *  \file   WebServerThread.cpp
 *  \brief  WEBサーバースレッドクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/WebServerThread.h"
#include "slog/Socket.h"
#include "slog/Util.h"
#include "slog/File.h"

#include "sha1.h"

#if defined(__linux__)
    #include <stdlib.h>
    #include <ctype.h>
#endif

namespace slog
{

/*!
 *  \brief  コンストラクタ
 */
WebServerThread::WebServerThread()
{
    mPort = 8080;
}

/*!
 *  \brief  ポート取得
 */
uint16_t WebServerThread::getPort() const
{
    return mPort;
}

/*!
 *  \brief  ポート設定
 */
void WebServerThread::setPort(uint16_t port)
{
    mPort = port;
}

/*!
 *  \brief  実行
 */
void WebServerThread::run()
{
    // WEBサーバーソケット準備
    Socket server;
    server.open();
    server.setReUseAddress(true);
    server.bind(mPort);
    server.listen();

    // 要求待ち
    Socket* client = NULL;

    while (true)
    {
        try
        {
            bool isReceive = server.isReceiveData(3000);

            if (isInterrupted())
                break;

            if (isReceive == false)
                continue;

            client = new Socket;
            client->accept(&server);

            HttpRequest* httpRequest = new HttpRequest(client, mPort);
            WebServerResponseThread* response = NULL;

            if (httpRequest->analizeRequest())
                response = createResponse(httpRequest);

            if (response)
            {
                response->start();
            }
            else
            {
                delete httpRequest;
            }
        }
        catch (Exception& e)
        {
            noticeLog(e.getMessage());
            delete client;
            break;
        }

        client = NULL;
    }
}

/*!
 *  \brief  WEBサーバー応答スレッドオブジェクト生成
 */
WebServerResponseThread* WebServerThread::createResponse(HttpRequest* httpRequest)
{
    HttpRequest::METHOD method = httpRequest->getMethod();
    const CoreString& url =      httpRequest->getUrl();

    const CREATE* createList = getCreateList();
    WebServerResponseThread* response = NULL;

    while (createList->method != HttpRequest::UNKNOWN)
    {
        String tmp = createList->url;

        if (createList->method == method && url.equals(tmp))
        {
            if (createList->replaceUrl[0] != '\0')
                httpRequest->setUrl(createList->replaceUrl);

            response = (*createList->proc)(httpRequest);
            break;
        }

        createList++;
    }

    if (response == NULL && HttpRequest::GET == method)
        response = (*createList->proc)(httpRequest);

    return response;
}

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
void WebServerResponseThread::upgradeWebSocket()
{
    String mes;
    mes.format("%s%s", mHttpRequest->getWebSocketKey().getBuffer(), "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

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
}

} // namespace slog
