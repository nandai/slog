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
 * \brief   WEBサーバー応答クラス
 * \author  Copyright 2011-2014 printf.jp
 */
#include "slog/WebServerResponseThread.h"
#include "slog/HttpRequest.h"
#include "slog/WebSocket.h"
#include "slog/Util.h"
#include "slog/File.h"
#include "slog/FileInfo.h"
#include "slog/ByteBuffer.h"
#include "slog/SHA1.h"
#include "slog/PointerString.h"
#include "slog/Session.h"

#undef __SLOG__
#include "slog/SequenceLog.h"

#if defined(__linux__)
    #include <string.h>
    #include <strings.h>
#endif

namespace slog
{

const char* WebServerResponse::CLS_NAME = "WebServerResponse";

/*!
 * \brief   コンストラクタ
 */
WebServerResponse::WebServerResponse(HttpRequest* httpRequest)
{
    mHttpRequest = httpRequest;
    mUserId = -1;
    mChunked = false;
}

/*!
 * \brief デストラクタ
 */
WebServerResponse::~WebServerResponse()
{
}

/*!
 * \brief   セッション生成
 */
void WebServerResponse::generateSession(int32_t userId)
{
    const CoreString* ip =        mHttpRequest->getSocket()->getInetAddress();
    const CoreString* userAgent = mHttpRequest->getUserAgent();
    bool secure =                (mHttpRequest->getScheme() == HttpRequest::SCHEME::HTTPS);

    Session* session = SessionManager::getByUserId(userId, userAgent);

    if (session == nullptr)
    {
        session = new Session(userId, userAgent);
        SessionManager::add(session);
    }

    session->setIP(ip);
    session->setSecure(secure);
    session->generate();

    mCookies.add(Session::NAME, session->getId(), "/", nullptr, secure, true);
    setUserId(userId);
}

/*!
 * \brief   セッション削除
 */
void WebServerResponse::removeSession(int32_t userId)
{
    const CoreString* userAgent = mHttpRequest->getUserAgent();
    Session* session = SessionManager::getByUserId(userId, userAgent);

    if (session)
    {
        SessionManager::remove(session);
        delete session;
    }

    bool secure = (mHttpRequest->getScheme() == HttpRequest::SCHEME::HTTPS);
    mCookies.remove(Session::NAME, "/", secure, true);
}

/*!
 * \brief   送信
 *
 * \param[in]   lastModified    最終書込日時（NULL可）
 * \param[in]   content         コンテンツ
 *
 * \return  なし
 */
void WebServerResponse::send(const DateTime* lastModified, const Buffer* content) const
{
    SLOG(CLS_NAME, "send");

    // HTTPヘッダー送信
    int32_t contentLen = content->getLength();
    sendHttpHeader(lastModified, contentLen);

    // コンテンツ送信
    sendContent(content);
}

/*!
 * \brief   not found 送信
 *
 * \param[in,out]   generator   HtmlGenerator
 *
 * \return  なし
 */
void WebServerResponse::sendNotFound(HtmlGenerator* generator) const
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
 * \brief   バイナリ送信
 *
 * \param[in,out]   generator   HtmlGenerator
 * \param[in]       path        ファイルパス
 *
 * \return  なし
 */
void WebServerResponse::sendBinary(HtmlGenerator* generator, const CoreString* path) const
{
    SLOG(CLS_NAME, "sendBinary");
    Socket* socket = mHttpRequest->getSocket();

    try
    {
        File file;
        file.open(path, File::READ);

        int32_t len = (int32_t)file.getSize();

        // HTTPヘッダー送信
        sendHttpHeader(file.getLastWriteTime(), len);

        // コンテンツ送信
        ByteBuffer buffer(1024 * 1024);
        int32_t readLen;

        while (0 < (readLen = (int32_t)file.read(&buffer, buffer.getCapacity())))
            socket->send(&buffer, readLen);

//      socket->close();
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
void WebServerResponse::sendHttpHeader(const DateTime* lastModified, int32_t contentLen) const
{
    SLOG(CLS_NAME, "sendHttpHeader");

    DateTime now;
    now.setCurrent();

    if (lastModified == nullptr)
        lastModified = &now;

    String dateString;
    Util::getDateString(&dateString, &now);

    String lastModifiedString;
    Util::getDateString(&lastModifiedString, lastModified);

    // ヘッダー生成
    const MimeType* mimeType = mHttpRequest->getMimeType();
    String str;
    String work;

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
        work.format(
//          "Connection: Close\r\n"
            "Content-Length: %d\r\n", contentLen);
        str.append(&work);
    }
    else
    {
        str.append("Transfer-Encoding: chunked\r\n");
        (bool&)mChunked = true;
    }

    appendCookiesString(&str);
    str.append("\r\n");

    // 送信
    Socket* socket = mHttpRequest->getSocket();
    socket->send(&str, str.getLength());

//  if (contentLen == 0)
//      socket->close();
}

/*!
 * \brief   応答内容送信＆切断
 *
 * \param[in]   content         コンテンツ
 *
 * \return  なし
 */
void WebServerResponse::sendContent(const Buffer* content) const
{
    SLOG(CLS_NAME, "sendContent");
    Socket* socket = mHttpRequest->getSocket();

    if (mChunked == false)
    {
        socket->send(content, content->getLength());
//      socket->close();
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
void WebServerResponse::redirect(const char* url) const
{
    PointerString str = url;
    redirect(&str);
}

/*!
 * \brief   リダイレクト
 *
 * \param[in]   url リダイレクト先URL
 *
 * \return  なし
 */
void WebServerResponse::redirect(const CoreString* url) const
{
    String str;
    str.format(
        "HTTP/1.1 302 Found\r\n"
        "Location: %s\r\n",
        url->getBuffer());

    appendCookiesString(&str);
    str.append("\r\n");

    Socket* socket = mHttpRequest->getSocket();
    socket->send(&str, str.getLength());

    // HTTPのリダイレクトではブラウザが「provisional headers are shown」で待ち状態となり
    // 先に進まないので、リダイレクトの場合はクローズする。
    //
    // ※HTTPSの場合はクローズなしでも問題なかった。
    socket->close();
}

/*!
 * \brief   BASIC認証
 *
 * \param[in]   realm   認証領域名
 *
 * \return  なし
 */
void WebServerResponse::basicAuth(const char* realm) const
{
    String str;
    str.format(
        "HTTP/1.1 401 Authorization Required\r\n"
        "WWW-Authenticate: Basic realm=\"%s\"\r\n"
        "\r\n",
        realm);

    Socket* socket = mHttpRequest->getSocket();
    socket->send(&str, str.getLength());
//  socket->close();
}

/*!
 * \brief   実行
 */
void WebServerResponse::run()
{
    try
    {
        const CoreString* url = mHttpRequest->getUrl();

        MimeType* mimeType = (MimeType*)mHttpRequest->getMimeType();

        if (mHttpRequest->isAjax() == false)
            mimeType->analize(url);

        String privateRootDir;
#if !defined(__ANDROID__)
        privateRootDir.format("%s../private", mHttpRequest->getRootDir()->getBuffer());
#else
        privateRootDir.copy(mHttpRequest->getRootDir());
#endif

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
        noticeLog("WebServerResponse: %s", e.getMessage());
    }
}

/*!
 * \brief   "Set-Cookie"を文字列に追加
 */
void WebServerResponse::appendCookiesString(CoreString* str) const
{
    String work;

    for (int32_t i = 0; i < mCookies.getCount(); i++)
    {
        const Cookie* cookie = mCookies.get(i);

        work.format(
            "Set-Cookie: %s=%s; path=%s",
            cookie->name. getBuffer(),
            cookie->value.getBuffer(),
            cookie->path. getBuffer());
        str->append(&work);

        if (cookie->expires.getLength())
        {
            work.format(
                "; expires=%s",
                cookie->expires.getBuffer());
            str->append(&work);
        }

        if (cookie->secure)
            str->append("; secure");

        if (cookie->httpOnly)
            str->append("; httpOnly");

        str->append("\r\n");
    }
}

/*!
 * \brief   WebSocketにアップグレード
 */
bool WebServerResponse::upgradeWebSocket()
{
    const CoreString* webSocketKey = mHttpRequest->getWebSocketKey();

    if (webSocketKey->getLength() == 0)
        return false;

    String mes;
    mes.format("%s%s", webSocketKey->getBuffer(), "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

    SHA1 hash;
    hash.execute(&mes);

    String resValue;
    Util::encodeBase64(&resValue, (const char*)hash.getMessageDigest(), hash.getHashSize());

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
void WebServerResponse::sendWebSocketHeader(uint64_t payloadLen, bool isText, bool toClient) const
{
    Socket* socket = mHttpRequest->getSocket();
    WebSocket::sendHeader(socket, payloadLen, isText, toClient);
}

} // namespace slog
