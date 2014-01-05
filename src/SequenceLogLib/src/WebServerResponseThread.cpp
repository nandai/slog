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
#include "slog/WebSocket.h"
#include "slog/Util.h"
#include "slog/File.h"
#include "slog/FileInfo.h"
#include "slog/ByteBuffer.h"

#include "sha1.h"

#if defined(__linux__)
    #include <string.h>
    #include <strings.h>
#endif

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

    for (VariableList::iterator i = mVariables.begin(); i != mVariables.end(); i++)
        delete *i;
}

/*!
 *  \brief  送信
 */
void WebServerResponseThread::send(const Buffer* content) const
{
    // HTTPヘッダー送信
    int32_t contentLen = content->getLength();
    sendHttpHeader(contentLen);

    // コンテンツ送信
    sendContent(content);
}

/*!
 *  \brief  not found 送信
 */
void WebServerResponseThread::sendNotFound(HtmlGenerator* generator) const
{
    MimeType* mimeType = (MimeType*)mHttpRequest->getMimeType();
    mimeType->setType(MimeType::Type::HTML);

    String path;
    mHttpRequest->setUrl("notFound.html");
    mHttpRequest->getPath(&path);

    String notFound = "404 not found.";
    const Buffer* writeBuffer = nullptr;

    if (generator->execute(&path, &mVariables))
    {
        // notFound.html
        writeBuffer = generator->getHtml();
    }
    else
    {
        // notFound.htmlもなかった場合
        writeBuffer = &notFound;
    }

    send(writeBuffer);
}

/*!
 *  \brief  送信
 */
void WebServerResponseThread::sendBinary(HtmlGenerator* generator, const slog::CoreString* path) const
{
    Socket* socket = mHttpRequest->getSocket();

    try
    {
        File file;
        file.open(*path, File::READ);

        int32_t len = (int32_t)file.getSize();

        // HTTPヘッダー送信
        sendHttpHeader(len);

        // コンテンツ送信
        ByteBuffer buffer(1024 * 1024);
        int32_t readLen;

        while (0 < (readLen = (int32_t)file.read(&buffer, buffer.getCapacity())))
            socket->send(&buffer, readLen);

        socket->close();
    }
    catch (Exception)
    {
        sendNotFound(generator);
    }
}

/*!
 *  \brief  HTTPヘッダー送信（＆切断）
 */
void WebServerResponseThread::sendHttpHeader(int32_t contentLen) const
{
    String str;
    str.format(
        "HTTP/1.1 200 OK\r\n"
        "Content-type: %s; charset=UTF-8\r\n"
        "Content-length: %d\r\n"
        "Connection: Close\r\n"
        "\r\n",
        mHttpRequest->getMimeType()->text.getBuffer(),
        contentLen);

    Socket* socket = mHttpRequest->getSocket();
    socket->send(&str, str.getLength());

    if (contentLen == 0)
        socket->close();
}

/*!
 *  \brief  応答内容送信＆切断
 */
void WebServerResponseThread::sendContent(const Buffer* content) const
{
    Socket* socket = mHttpRequest->getSocket();

    socket->send(content, content->getLength());
    socket->close();
}

/*!
 *  \brief  実行
 */
void WebServerResponseThread::run()
{
    try
    {
        const CoreString& url = mHttpRequest->getUrl();

        MimeType* mimeType = (MimeType*)mHttpRequest->getMimeType();

        if (mHttpRequest->isAjax() == false)
            mimeType->analize(&url);

        String privateRootDir;
        privateRootDir.format("%s../private", mHttpRequest->getRootDir()->getBuffer());

        HtmlGenerator generator(&privateRootDir);
        String path;

        if (mimeType->binary == false)
        {
            mHttpRequest->getPath(&path);
            initVariables();

            if (generator.execute(&path, &mVariables))
            {
                // 正常時
                send(generator.getHtml());
            }
            else
            {
                // 異常時
                sendNotFound(&generator);
            }
        }
        else
        {
            // バイナリ送信
            path.format("%s/%s", privateRootDir.getBuffer(), url.getBuffer());
            sendBinary(&generator, &path);
        }
    }
    catch (Exception& e)
    {
        noticeLog("WebServerResponseThread: %s", e.getMessage());
    }
}

/*!
 *  \brief  WebSocketにアップグレード
 */
bool WebServerResponseThread::upgradeWebSocket()
{
    const CoreString* webSocketKey = mHttpRequest->getWebSocketKey();

    if (webSocketKey->getLength() == 0)
        return false;

    String mes;
    mes.format("%s%s", webSocketKey->getBuffer(), "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

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
    WebSocket::sendHeader(socket, payloadLen, isText, toClient);
}

} // namespace slog
