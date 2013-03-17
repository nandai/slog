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
 *  \file   WebServerThread.cpp
 *  \brief  WEBサーバースレッドクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/WebServerThread.h"
#include "slog/Socket.h"
#include "slog/ByteBuffer.h"

using namespace std;

namespace slog
{

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
            client = new Socket;
            client->accept(&server);

            if (isInterrupted())
            {
                client->close();
                delete client;
                break;
            }

            WebServerResponseThread* response = createResponseThread(client);
            response->start();
        }
        catch (Exception& e)
        {
            noticeLog(e.getMessage());
            client->close();
            delete client;
            break;
        }
    }
}

/*!
 *  \brief  コンストラクタ
 */
WebServerResponseThread::WebServerResponseThread(Socket* socket)
{
    mSocket = socket;
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
 *  \brief  スレッド終了通知
 */
void WebServerResponseThread::onTerminated(Thread* thread)
{
    delete this;
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
                const char* compare = "Content-Length: ";
                int32_t compareLen = (int32_t)strlen(compare);

                if (strncmp(request, compare, compareLen) == 0)
                {
                    contentLen = atoi(request + compareLen);
                }
            }
        }

        i = 0;
    }

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
#if defined(_WINDOWS)
        "Content-type: text/html; charset=SHIFT_JIS\n"
#else
        "Content-type: text/html; charset=UTF-8\n"
#endif
        "Content-length: %d\n"
        "\n",
        contentLen);

    mSocket->send(str.getBuffer(), str.getLength());
}

/*!
 *  \brief  応答内容送信＆切断
 */
void WebServerResponseThread::sendContent(String* content) const
{
    mSocket->send(content, content->getLength());
    mSocket->close();
}

} // namespace slog
