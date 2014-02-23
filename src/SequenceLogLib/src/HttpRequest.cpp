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
 * \file    HttpRequest.cpp
 * \brief   httpリクエストクラス
 * \author  Copyright 2011-2014 printf.jp
 */
#include "slog/HttpRequest.h"
#include "slog/Socket.h"
#include "slog/ByteBuffer.h"
#include "slog/FileInfo.h"
#include "slog/Util.h"

#include <ctype.h>

#if defined(__linux__)
    #include <string.h>
    #include <stdlib.h>
#endif

using namespace std;

namespace slog
{

/*!
 * \brief   文字列のペアを取得する
 *
 * \param[in]   maps        マップ
 * \param[in]   buffer      解析対象文字列
 * \param[in]   len         解析対象文字列の長さ
 * \param[in]   separator   セパレータ
 *
 * \return  なし
 */
static void getStringPairs(map<String, String>* maps, const char* buffer, int32_t len, const char* separator)
{
    const char* p1 = buffer;
    bool end = false;
    int32_t sepLen = (int32_t)strlen(separator);

    while (end == false)
    {
        // 一対のパラメータを取り出す
        const char* p2 = nullptr;
        int32_t index = 0;

        while (true)
        {
            if (p1 + index == buffer + len)
            {
                p2 = buffer + len;
                end = true;
                break;
            }

            if (memcmp(p1 + index, separator, sepLen) == 0)
            {
                p2 = p1 + index;
                break;
            }

            index++;
        }

//      noticeLog("getStringPairs: %.*s(%d)", (p2 - p1), p1, (p2 - p1));

        // パラメータ名と値に分ける
        const char* p3 = strchr(p1, '=');

        if (p3 == nullptr)
            break;

        // パラメータからキーを取得
        String key(p1, (int32_t)(p3 - p1));

        // パラメータから値を取得
        String  value;
        Util::decodePercent(&value, (char*)p3 + 1, p2);

        // パラメータリストに追加
        maps->insert(pair<String, String>(key, value));

        p1 = p2 + sepLen;
    }
}

/*!
 * \brief   コンストラクタ
 *
 * \param[in]   socket  ソケット
 * \param[in]   scheme  スキーマ
 * \param[in]   port    ポート番号
 * \param[in]   rootDir ルートディレクトリ
 */
HttpRequest::HttpRequest(SCHEME scheme, Socket* socket, uint16_t port, const CoreString* rootDir)
{
    mSocket = socket;
    mScheme = scheme;
    mPort = port;
    mRootDir.copy(*rootDir);
    mMethod = UNKNOWN;
    mAjax = false;
    mListener = &mDefaultListener;
}

/*!
 * \brief   デストラクタ
 */
HttpRequest::~HttpRequest()
{
    delete mSocket;
}

/*!
 * リスナー設定
 */
void HttpRequest::setListener(HttpRequestListener* listener)
{
    mListener = listener;
}

/*!
 * \brief   要求解析
 *
 * \retval  true    解析に成功した場合
 * \retval  false   解析に失敗した場合
 */
bool HttpRequest::analizeRequest()
{
    int32_t contentLen = 0;
    String str;

    while (true)
    {
        // 受信
        mSocket->recv(&str);
        mListener->onHeader(&str);
//      noticeLog("request: %s", str.getBuffer());

        const char* request = str.getBuffer();
        int32_t i =           str.getLength();

        if (i == 0)
        {
            // 空行だったらループを抜ける
            if (mMethod == POST && 0 < contentLen)
            {
                ByteBuffer params(contentLen);

                mSocket->recv(&params, contentLen);
                getStringPairs(&mParams, params.getBuffer(), params.getLength(), "&");
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

                // Authorization
                compare = "Authorization: Basic ";
                compareLen = (int32_t)strlen(compare);

                if (strncmp(request, compare, compareLen) == 0)
                {
                    String basicAuth;
                    Util::decodeBase64(&basicAuth, request + compareLen);

                    int32_t pos =   basicAuth.find(':');
                    const char* p = basicAuth.getBuffer();

                    if (pos != -1)
                    {
                        mUser.    copy(p, pos);
                        mPassword.copy(p + pos + 1);
                    }
                }

                // Cookie
                compare = "Cookie: ";
                compareLen = (int32_t)strlen(compare);

                if (strncmp(request, compare, compareLen) == 0)
                {
                    getStringPairs(&mCookies, request + compareLen, i - compareLen, "; ");
                }
            }
        }

        i = 0;
    }

    return true;
}

/*!
 * \brief   URL解析
 *
 * \param[in]   request 解析対象文字列
 * \param[in]   len     解析対象文字列の長さ
 * \param[in]   method  解析対象とするメソッド種別
 *
 * \retval  -1  methodがGETでもPOSTでもなかった、またはrequestが正しいフォーマット（"GET <ドメイン> HTTP/1.1"等）ではなかった場合
 * \retval   0  解析成功
 * \retval   1  requestに"GET"、または"POST"が含まれていなかった場合
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

//      if (method == GET)
        {
            const char* p3 = strchr(p1, '?');

            if (p3)
            {
                getStringPairs(&mParams, p3 + 1, (int32_t)(p2 - (p3 + 1)), "&");
                p2 = p3;
            }
        }

        p1++;   // "/www.printf.jp"等の先頭の'/'をスキップ

        Util::decodePercent(&mUrl, (char*)p1, p2);
        setUrl( mUrl.getBuffer());

        mMethod = method;
        return 0;
    }

    return 1;
}

/*!
 * \brief   ソケット取得
 */
Socket* HttpRequest::getSocket() const
{
    return mSocket;
}

/*!
 * \brief   ポート番号取得
 */
uint16_t HttpRequest::getPort() const
{
    return mPort;
}

/*!
 * \brief   ルートディレクトリ取得
 */
const CoreString* HttpRequest::getRootDir() const
{
    return &mRootDir;
}

/*!
 * \brief   スキーマ取得
 */
HttpRequest::SCHEME HttpRequest::getScheme() const
{
    return mScheme;
}

/*!
 * \brief   メソッド取得
 */
HttpRequest::METHOD HttpRequest::getMethod() const
{
    return mMethod;
}

/*!
 * \brief   URL取得
 */
const CoreString* HttpRequest::getUrl() const
{
    return &mUrl;
}

/*!
 * \brief   URL設定
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
 * \brief   ファイルパス取得
 */
void HttpRequest::getPath(CoreString* path)
{
    path->copy(mRootDir);
    path->append(mUrl);
}

/*!
 * \brief   mime-type取得
 */
const MimeType* HttpRequest::getMimeType()
{
    return &mMimeType;
}

/*!
 * \brief   Cookie取得
 */
const CoreString* HttpRequest::getCookie(const char* name, CoreString* str)
{
    str->copy(mCookies[name]);
    return str;
}

/*!
 * \brief   パラメータ取得
 */
const CoreString* HttpRequest::getParam(const char* name, CoreString* str)
{
    str->copy(mParams[name]);
    return str;
}

/*!
 * \brief   Ajaxかどうか調べる
 */
bool HttpRequest::isAjax() const
{
    return mAjax;
}

/*!
 * \brief   Sec-WebSocket-Key取得
 */
const CoreString* HttpRequest::getWebSocketKey() const
{
    return &mWebSocketKey;
}

/*!
 * \brief   ユーザー取得
 */
const CoreString* HttpRequest::getUser() const
{
    return &mUser;
}

/*!
 * \brief   パスワード取得
 */
const CoreString* HttpRequest::getPassword() const
{
    return &mPassword;
}

} // namespace slog
