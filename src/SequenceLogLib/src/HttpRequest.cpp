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
 *  \file   HttpRequest.cpp
 *  \brief  httpリクエストクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/HttpRequest.h"
#include "slog/Socket.h"
#include "slog/ByteBuffer.h"
#include "slog/FileInfo.h"

#include <ctype.h>

#if defined(__linux__)
    #include <string.h>
    #include <stdlib.h>
#endif

using namespace std;

namespace slog
{

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
 *  \brief  コンストラクタ
 */
HttpRequest::HttpRequest(Socket* socket, uint16_t port, const CoreString* rootDir)
{
    mSocket = socket;
    mPort = port;
    mRootDir.copy(*rootDir);
    mMethod = UNKNOWN;
    mAjax = false;
}

/*!
 *  \brief  デストラクタ
 */
HttpRequest::~HttpRequest()
{
    delete mSocket;
}

/*!
 *  \brief  要求解析
 */
bool HttpRequest::analizeRequest()
{
    int32_t contentLen = 0;

    while (true)
    {
        // 受信
        String str;
        mSocket->recv(&str);

        const char* request = str.getBuffer();
        int32_t i =           str.getLength();

        if (i == 0)
        {
            // 空行だったらループを抜ける
            if (mMethod == POST && 0 < contentLen)
            {
                ByteBuffer params(contentLen);

                mSocket->recv(&params, contentLen);
                analizeParams(params.getBuffer(), params.getLength());
            }
//          noticeLog("analizeRequest ended");

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

                if (mMethod == UNKNOWN)
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

                // Accept
                compare = "Accept: ";
                compareLen = (int32_t)strlen(compare);

                if (strncmp(request, compare, compareLen) == 0)
                {
                    char* p = strchr((char*)request, ',');

                    if (p)
                        p[0] = '\0';

                    mMimeType.setText(request + compareLen);
                }

                // X-Requested-With
                compare = "X-Requested-With: XMLHttpRequest";
                compareLen = (int32_t)strlen(compare);

                if (strncmp(request, compare, compareLen) == 0)
                {
                    mAjax = true;
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

    return true;
}

/*!
 *  \brief  URL解析
 */
int32_t HttpRequest::analizeUrl(const char* request, int32_t len, METHOD method)
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

        if (p2 == nullptr)
            return -1;

        if (method == GET)
        {
            const char* p3 = strchr(p1, '?');

            if (p3)
            {
                analizeParams(p3 + 1, (int32_t)(p2 - (p3 + 1)));
                p2 = p3;
            }
        }

        p1++;   // '/'をスキップ

        decode(&mUrl, (char*)p1, p2);
        setUrl( mUrl.getBuffer());

        mMethod = method;
        return 0;
    }

    return 1;
}

/*!
 *  \brief  パラメータ解析
 */
void HttpRequest::analizeParams(const char* buffer, int32_t len)
{
    const char* p1 = buffer;
    bool end = false;

    while (end == false)
    {
        // 一対のパラメータを取り出す
        const char* p2 = strchr(p1, '&');

        if (p2 == nullptr)
        {
            p2 = buffer + len;
            end = true;
        }

        // パラメータ名と値に分ける
        const char* p3 = strchr(p1, '=');

        if (p3 == nullptr)
            break;

        // パラメータからキーを取得
        String key(p1, (int32_t)(p3 - p1));

        // パラメータから値を取得
        String  value;
        decode(&value, (char*)p3 + 1, p2);

        // パラメータリストに追加
        mParams.insert(pair<String, String>(key, value));

        p1 = p2 + 1;
    }
}

/*!
 * パーセントデコード
 */
void HttpRequest::decode(slog::CoreString* str, char* start, const char* end)
{
    const char* cursor = start;
    char* decodeCursor = start;

    while (cursor < end)
    {
        char c = *cursor;

        switch (c)
        {
        case '%':
        {
            cursor = hexToValue(cursor + 1, &c);
            break;
        }

        case '+':
            c =  ' ';
//          break;

        default:
            cursor++;
            break;
        }

        *decodeCursor = c;
         decodeCursor++;
    }

    str->copy(start, (int32_t)(decodeCursor - start));
}

/*!
 *  \brief  ソケット取得
 */
Socket* HttpRequest::getSocket() const
{
    return mSocket;
}

/*!
 *  \brief  ポート取得
 */
uint16_t HttpRequest::getPort() const
{
    return mPort;
}

/*!
 *  \brief  ルートディレクトリ取得
 */
const CoreString* HttpRequest::getRootDir() const
{
    return &mRootDir;
}

/*!
 *  \brief  HTTPメソッド取得
 */
HttpRequest::METHOD HttpRequest::getMethod() const
{
    return mMethod;
}

/*!
 *  \brief  URL取得
 */
const CoreString& HttpRequest::getUrl() const
{
    return mUrl;
}

/*!
 *  \brief  URL設定
 */
void HttpRequest::setUrl(const char* url)
{
    mUrl.copy(url);

    String path = mRootDir;
    path.append(mUrl);

    int32_t len = path.getLength();

    if (path[len - 1] == '/')
        path.deleteLast();

    FileInfo info(path);

    if (info.isFile() == false && info.getMessage().getLength() == 0)
    {
        mUrl.copy(path.getBuffer() + mRootDir.getLength());
        mUrl.append("/index.html");
    }
}

/*!
 *  \brief  
 */
void HttpRequest::getPath(CoreString* path)
{
    path->copy(mRootDir);
    path->append(mUrl);
}

/*!
 *  \brief  mime-type取得
 */
const MimeType* HttpRequest::getMimeType()
{
    return &mMimeType;
}

/*!
 *  \brief  POSTパラメータ取得
 */
const CoreString* HttpRequest::getParam(const char* name, CoreString* param)
{
    param->copy(mParams[name]);
    return param;
}

/*!
 *  \brief  Ajaxかどうか調べる
 */
bool HttpRequest::isAjax() const
{
    return mAjax;
}

/*!
 *  \brief  Sec-WebSocket-Key取得
 */
const CoreString* HttpRequest::getWebSocketKey() const
{
    return &mWebSocketKey;
}

} // namespace slog
