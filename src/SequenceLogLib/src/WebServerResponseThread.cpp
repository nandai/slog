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
 * \file    WebServerResponseThread.cpp
 * \brief   WEBサーバー応答スレッドクラス
 * \author  Copyright 2011-2014 printf.jp
 */
#include "slog/WebServerResponseThread.h"
#include "slog/HttpRequest.h"
#include "slog/WebSocket.h"
#include "slog/Util.h"
#include "slog/File.h"
#include "slog/FileInfo.h"
#include "slog/ByteBuffer.h"

#include "slog/sha1.h"

#if defined(__linux__)
    #include <string.h>
    #include <strings.h>
#endif

/*!
 * 日時を文字列で取得する
 */
static void getDateString(slog::CoreString* str, const slog::DateTime* dateTime)
{
    static const char* week[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char* mon[] =  {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    str->format("%s, %02d %s %04d %02d:%02d:%02d GMT",
        week[dateTime->getWeekDay()],
             dateTime->getDay(),
        mon[ dateTime->getMonth() - 1],
             dateTime->getYear(),
             dateTime->getHour(),
             dateTime->getMinute(),
             dateTime->getSecond());
}

namespace slog
{

/*!
 * \brief   コンストラクタ
 */
WebServerResponseThread::WebServerResponseThread(HttpRequest* httpRequest)
{
    mHttpRequest = httpRequest;
    mChunked = false;
}

/*!
 * \brief デストラクタ
 */
WebServerResponseThread::~WebServerResponseThread()
{
    delete mHttpRequest;
}

/*!
 * \brief   送信
 */
void WebServerResponseThread::send(const DateTime* lastModified, const Buffer* content) const
{
    // HTTPヘッダー送信
    int32_t contentLen = content->getLength();
    sendHttpHeader(lastModified, contentLen);

    // コンテンツ送信
    sendContent(content);
}

/*!
 * \brief   not found 送信
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
    const DateTime* lastModified = nullptr;

    if (generator->execute(&path, &mVariables))
    {
        // notFound.html
        writeBuffer =  generator->getHtml();
        lastModified = generator->getLastWriteTime();
    }
    else
    {
        // notFound.htmlもなかった場合
        writeBuffer = &notFound;
    }

    send(lastModified, writeBuffer);
}

/*!
 * \brief   送信
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
        sendHttpHeader(file.getLastWriteTime(), len);

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
 * \brief   HTTPヘッダー送信（＆切断）
 *
 * \param[in]   lastModified    最終書込日時（NULL可）
 * \param[in]   contentLen      コンテンツの長さ
 *
 * \return  なし
 */
void WebServerResponseThread::sendHttpHeader(const DateTime* lastModified, int32_t contentLen) const
{
    DateTime now;
    now.setCurrent();

    if (lastModified == nullptr)
        lastModified = &now;

    String dateString;
    getDateString(&dateString, &now);

    String lastModifiedString;
    getDateString(&lastModifiedString, lastModified);

    // ヘッダー生成
    const MimeType* mimeType = mHttpRequest->getMimeType();

    String str;
    str.format(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s%s\r\n"
        "Date: %s\r\n"
        "Last-Modified: %s\r\n",
         mimeType->text.getBuffer(),
        (mimeType->binary == false ? "; charset=UTF-8" : ""),
        dateString.getBuffer(),
        lastModifiedString.getBuffer());

    if (0 <= contentLen)
    {
        String work;
        work.format(
            "Connection: Close\r\n"
            "Content-Length: %d\r\n", contentLen);
        str.append(work);
    }
    else
    {
        str.append("Transfer-Encoding: chunked\r\n");
        (bool&)mChunked = true;
    }

    str.append("\r\n");

    // 送信
    Socket* socket = mHttpRequest->getSocket();
    socket->send(&str, str.getLength());

    if (contentLen == 0)
        socket->close();
}

/*!
 * \brief   応答内容送信＆切断
 */
void WebServerResponseThread::sendContent(const Buffer* content) const
{
    Socket* socket = mHttpRequest->getSocket();

    if (mChunked == false)
    {
        socket->send(content, content->getLength());
        socket->close();
    }
    else
    {
        String str;

        if (content)
        {
            str.format(
                "%x\r\n"
                "%s\r\n",
                content->getLength(),
                content->getBuffer());
        }
        else
        {
            str.copy(
                "0\r\n"
                "\r\n");
        }

        socket->send(&str, str.getLength());
    }
}

/*!
 * \brief   リダイレクト
 *
 * \param[in]   url リダイレクト先URL
 *
 * \return  なし
 */
void WebServerResponseThread::redirect(const CoreString* url) const
{
    String str;
    str.format(
        "HTTP/1.1 302 Found\r\n"
        "Location: %s\r\n"
        "\r\n",
        url->getBuffer());

    Socket* socket = mHttpRequest->getSocket();
    socket->send(&str, str.getLength());
    socket->close();
}

/*!
 * \brief   BASIC認証
 *
 * \param[in]   realm   認証領域名
 *
 * \return  なし
 */
void WebServerResponseThread::basicAuth(const char* realm) const
{
    String str;
    str.format(
        "HTTP/1.1 401 Authorization Required\r\n"
        "WWW-Authenticate: Basic realm=\"%s\"\r\n"
        "\r\n",
        realm);

    Socket* socket = mHttpRequest->getSocket();
    socket->send(&str, str.getLength());
    socket->close();
}

/*!
 * \brief   実行
 */
void WebServerResponseThread::run()
{
    try
    {
        const CoreString* url = mHttpRequest->getUrl();

        MimeType* mimeType = (MimeType*)mHttpRequest->getMimeType();

        if (mHttpRequest->isAjax() == false)
            mimeType->analize(url);

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
                send(generator.getLastWriteTime(), generator.getHtml());
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
            path.format("%s/%s", privateRootDir.getBuffer(), url->getBuffer());
            sendBinary(&generator, &path);
        }
    }
    catch (Exception& e)
    {
        noticeLog("WebServerResponseThread: %s", e.getMessage());
    }
}

/*!
 * \brief   WebSocketにアップグレード
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
 * \brief   WebSocketヘッダー送信
 */
void WebServerResponseThread::sendWebSocketHeader(uint64_t payloadLen, bool isText, bool toClient) const
{
    Socket* socket = mHttpRequest->getSocket();
    WebSocket::sendHeader(socket, payloadLen, isText, toClient);
}

} // namespace slog
