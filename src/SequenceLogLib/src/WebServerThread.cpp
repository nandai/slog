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
#include "slog/ByteBuffer.h"
#include "slog/Util.h"
#include "slog/File.h"

#include "sha1.h"

#if defined(__linux__)
    #include <stdlib.h>
    #include <ctype.h>
#endif

using namespace std;

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
            {
                sleep(1);
                continue;
            }

            client = new Socket;
            client->accept(&server);

            WebServerResponseThread* response = createResponseThread(client);
            response->start();
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
 *  \brief  コンストラクタ
 */
WebServerResponseThread::WebServerResponseThread(Socket* socket, uint16_t port)
{
    mSocket = socket;
    mPort = port;
    mMethod = UNKNOWN;
}

/*!
 *  \brief  デストラクタ
 */
WebServerResponseThread::~WebServerResponseThread()
{
    delete mSocket;
}

/*!
 *  \brief  要求解析
 */
bool WebServerResponseThread::analizeRequest()
{
    int32_t size = 1;
    ByteBuffer buffer(size);

    char request[1024 + 1];
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
            if (sizeof(request) <= i)
                return false;

            request[i] = c;
            i++;

            continue;
        }

        request[i] = '\0';

        // '\n'捨て
        buffer.setLength(0);
        mSocket->recv(&buffer, size);

        if (i == 0)
        {
            // 空行だったらループを抜ける
            if (mMethod == POST && 0 < contentLen)
            {
                ByteBuffer params(contentLen);

                mSocket->recv(&params, contentLen);
                analizePostParams(&params);
            }

            break;
        }
        else
        {
            if (mMethod == UNKNOWN)
            {
                // URL取得
                if (analizeUrl(request, i, GET)  == -1)
                    return false;

                if (analizeUrl(request, i, POST) == -1)
                    return false;
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

                // Sec-WebSocket-Key
                compare = "Sec-WebSocket-Key: ";
                compareLen = (int32_t)strlen(compare);

                if (strncmp(request, compare, compareLen) == 0)
                {
                    mWebSocketKey.copy(request + compareLen);
                }
            }
        }

        i = 0;
    }

    if (0 < mWebSocketKey.getLength())
        upgradeWebSocket();

    return true;
}

/*!
 *  \brief  URL解析
 */
int32_t WebServerResponseThread::analizeUrl(const char* request, int32_t len, METHOD method)
{
    const char* compare;

    switch (method)
    {
    case GET:
        compare = "GET ";
        break;

    case POST:
        compare = "POST ";
        break;

    default:
        return -1;
    }

    int32_t compareLen = (int32_t)strlen(compare);

    if (compareLen <= len && strncmp(request, compare, compareLen) == 0)
    {
        const char* p1 = request + compareLen;
        const char* p2 = strchr(p1, ' ');

        if (p2 == NULL)
            return -1;

        p1++;
        mUrl.copy(p1, (int32_t)(p2 - p1));
        mMethod = method;
        return 0;
    }

    return 1;
}

/*!
 *  \brief  16進数文字列を数値に変換
 */
template <class T>
inline const char* _hexToValue(const char* hex, T* value)
{
    int32_t i;
    int32_t size = sizeof(*value) * 2;
    *value = 0;

    for (i = 0; i < size; i++)
    {
        char c = toupper(hex[i]);

        if ('0' <= c && c <= '9')
        {
            c = c - '0';
        }
        else if ('A' <= c && c <= 'F')
        {
            c = c - 'A' + 0x0A;
        }
        else
        {
            break;
        }

        *value = (*value << 4) | c;
    }

    return (hex + i);
}

/*!
 *  \brief  16進数文字列をchar型の数値に変換
 */
static const char* hexToValue(const char* hex, char* value)
{
    return _hexToValue(hex, value);
}

/*!
 *  \brief  POSTパラメータ解析
 */
void WebServerResponseThread::analizePostParams(ByteBuffer* params)
{
    const char* p1 = params->getBuffer();
    bool end = false;

    while (end == false)
    {
        // 一対のパラメータを取り出す
        const char* p2 = strchr(p1, '&');

        if (p2 == NULL)
        {
            p2 = p1 + params->getLength();
            end = true;
        }

        // パラメータ名と値に分ける
        const char* p3 = strchr(p1, '=');

        if (p3 == NULL)
            break;

        // パラメータからキーを取得
        String key(p1, (int32_t)(p3 - p1));

        // パラメータから値を取得
        p3++;
        p1 = p3;
        char* p4 = (char*)p3;

        while (p1 < p2)
        {
            char c = *p1;

            switch (c)
            {
            case '%':
            {
                p1 = hexToValue(p1 + 1, &c);
                break;
            }

            case '+':
                c =  ' ';
//              break;

            default:
                p1++;
                break;
            }

            *p4 = c;
             p4++;
        }

        String value;
        value.setSJIS(0);
        value.copy(p3, (int32_t)(p4 - p3));

        p1 = p2 + 1;

        // パラメータリストに追加
        mPostParams.insert(pair<String, String>(key, value));
    }
}

/*!
 *  \brief  HTTPメソッド取得
 */
WebServerResponseThread::METHOD WebServerResponseThread::getMethod() const
{
    return mMethod;
}

/*!
 *  \brief  URL取得
 */
const CoreString& WebServerResponseThread::getUrl() const
{
    return mUrl;
}

/*!
 *  \brief  POSTパラメータ取得
 */
void WebServerResponseThread::getParam(const char* name, CoreString* param)
{
    param->copy(mPostParams[name]);
}

/*!
 *  \brief  HTTPヘッダー送信
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

    mSocket->send(&str, str.getLength());
}

/*!
 *  \brief  応答内容送信＆切断
 */
void WebServerResponseThread::sendContent(String* content) const
{
    mSocket->send(content, content->getLength());
    mSocket->close();
}

/*!
 *  \brief  実行
 */
void WebServerResponseThread::run()
{
    try
    {
        String content;

        if (analizeRequest())
        {
            // URL取得
            const CoreString& url = getUrl();

            // URLマップに一致する処理を行う
            const URLMAP* urlmap = getUrlMaps();
            bool res = false;

            while (urlmap->method != UNKNOWN)
            {
                String tmp = urlmap->url;

                if (urlmap->method == getMethod() && url.equals(tmp))
                {
                    WEBPROC proc = urlmap->proc;
                    res = (this->*proc)(&content, (urlmap->replaceUrl[0] == '\0'
                        ? urlmap->url
                        : urlmap->replaceUrl));

                    break;
                }

                urlmap++;
            }

            if (res == false && GET == getMethod())
            {
                do
                {
                    if (getContents(&content, url.getBuffer()))
                        break;

                    if (getContents(&content, "notfound.html"))
                        break;

                    return;
                }
                while (false);
            }
        }

        // HTTPヘッダー送信
        int32_t contentLen = content.getLength();
        sendHttpHeader(contentLen);

        // コンテンツ送信
        sendContent(&content);
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
        const CoreString& ip = mSocket->getMyInetAddress();

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
                    ws.format("%s:%u", ip.getBuffer(), mPort);

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
    mes.format("%s%s", mWebSocketKey.getBuffer(), "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

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

    mSocket->send(&str, str.getLength());
    WebSocketMain();
}

} // namespace slog
