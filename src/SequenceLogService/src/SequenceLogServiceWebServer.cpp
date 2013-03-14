﻿/*
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
 *  \file   SequenceLogServiceWebServer.cpp
 *  \brief  シーケンスログサービスWEBサーバークラス
 *  \author Copyright 2013 printf.jp
 */
#include "SequenceLogServiceWebServer.h"
#include "SequenceLogServiceMain.h"

#include "slog/String.h"
#include "slog/ByteBuffer.h"
#include "slog/Socket.h"
#include "slog/File.h"
#include "slog/DateTime.h"
#include "slog/DateTimeFormat.h"
#include "slog/FileInfo.h"
#include "slog/Mutex.h"

using namespace std;

#define DOMAIN  "printf.jp"
#define FAVICON "<link rel=\"shortcut icon\" href=\"http://" DOMAIN "/images/SequenceLogService.ico\">"

namespace slog
{

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
            if (mMethod == POST)
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
            p2 = p1 + strlen(p1);
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
 *  \brief  URL取得
 */
const CoreString& WebServerResponseThread::getUrl() const
{
    return mUrl;
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
 *  \brief  シーケンスログ送信
 */
static bool sendSequenceLog(const CoreString& ip, uint16_t port, const CoreString& path)
{
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    bool result = false;

    String name = strrchr(path.getBuffer(), PATH_DELIMITER);
    int32_t len = name.getLength();

    try
    {
        // ファイルオープン
        File file;
        file.open(path, File::READ);

        result = true;

        // ソケット準備
        Socket viewerSocket;
        viewerSocket.open();
        viewerSocket.connect(ip, port);

        // ファイル名送信
        viewerSocket.send(&len);
        viewerSocket.send(&name, len);

        // ファイルサイズ送信
        int32_t size = (int32_t)file.getSize();
        viewerSocket.send(&size);

        // ファイル内容送信
        ByteBuffer buffer(size);

        file.read(        &buffer, size);
        viewerSocket.send(&buffer, size);

        // ステータス受信
        int32_t status;
        viewerSocket.recv(&status);

        // 後始末
        viewerSocket.close();
        file.close();
    }
    catch (Exception&)
    {
    }

    return result;
}

/*!
 *  \brief  文字列にシーケンスログリストを追加
 */
static void appendSequenceLogList(String* buffer, FileInfo* info)
{
    DateTime dateTime;
    FixedString<DateTimeFormat::DATE_TIME_MS_LEN> strDt;

    // 開始日時
    String strCreationTime;
    dateTime = info->getCreationTime();
    dateTime.toLocal();

    if (dateTime.getValue())
    {
        DateTimeFormat::toString(&strDt, dateTime, DateTimeFormat::DATE_TIME);
        strCreationTime.format("<td>%s</td>", strDt.getBuffer());
    }
    else
    {
        strCreationTime.copy("<td></td>");
    }

    // 終了日時
    String strLastWriteTime;
    dateTime = info->getLastWriteTime();
    dateTime.toLocal();

    if (dateTime.getValue())
    {
        DateTimeFormat::toString(&strDt, dateTime, DateTimeFormat::DATE_TIME);
        strLastWriteTime.format("<td>%s</td>", strDt.getBuffer());
    }
    else
    {
        strLastWriteTime.copy("<td></td>");
    }

    // ログファイル名
    const char* canonicalPath = info->getCanonicalPath().getBuffer();

    String strFileName;
    strFileName.format("<td><a href=\"#\">%s</a></td>", canonicalPath);

    // サイズ
    String strSize;
    const CoreString& message = info->getMessage();

    if (info->isUsing() == false)
    {
        double size = (double)info->getSize();

        if (size < 1024)
        {
            strSize.format("<td align=\"right\">%s %d byte(s)</td>", message.getBuffer(), (int64_t)size);
        }

        else
        if (size < 1024 * 1024)
        {
            strSize.format("<td align=\"right\">%s %.1f KB</td>", message.getBuffer(), size / 1024);
        }

        else
        {
            strSize.format("<td align=\"right\">%s %.1f MB</td>", message.getBuffer(), size / (1024 * 1024));
        }
    }
    else
    {
        strSize.copy("<td></td>");
    }

    // 文字列追加
    String str;
    str.format(
        "<tr>"
            "%s%s%s%s"
        "</tr>",
        strCreationTime. getBuffer(),
        strLastWriteTime.getBuffer(),
        strFileName.     getBuffer(),
        strSize.         getBuffer());

    buffer->append(str);
}

/*!
 *  \brief  文字列にcssを追加
 */
static void appendCSS(String* buffer)
{
    buffer->append(
        "<style type=\"text/css\">"
            "body"
            "{"
                "font-size: 14px;"
                "font-family: meiryo;"
                "background: #FFFFFF;"
            "}"

            "table"
            "{"
                "border-collapse: collapse;"
            "}"

            "th, td"
            "{"
                "border: 1px solid #808080;"
                "padding: 5px;"
                "white-space: nowrap;"
            "}"

            "th"
            "{"
            "    background-color: #80FF80;"
            "}"
        "</style>");
}

/*!
 *  \brief  文字列にjavascriptを追加
 */
static void appendJavaScript(String* buffer)
{
    buffer->append(
        "<script type=\"text/javascript\" src=\"http://ajax.googleapis.com/ajax/libs/jquery/1.7.2/jquery.min.js\"></script>"
        "<script type=\"text/javascript\">"
        "$(function()"
        "{"
            "$('a[href=#]').click(function()"
            "{"
                "$.ajax("
                "{"
                    "url: '/',"
                    "type: 'POST',"
                    "dataType: 'text',"
                    "data: "
                    "{"
                        "fileName: $(this).text()"
                    "}"
                "});"
                "return false;"
            "});"
        "});"
        "</script>");
}

/*!
 *  \brief  文字列にフッターを追加
 */
static void appendFooter(String* buffer)
{
    buffer->append(
        "<hr />"
        "<div align=\"right\">"
            "Copyright (c) 2013 <a href=\"http://" DOMAIN "\" target=\"_blank\">" DOMAIN "</a> All rights reserved."
        "</div>");
}

/*!
 *  \brief  文字列に要求内容を追加
 */
static void appendRequestContents(String* buffer)
{
    buffer->append(
        "<head>"
            FAVICON
            "<title>Sequence Log Service</title>");

    appendJavaScript(buffer);
    appendCSS(buffer);

    buffer->append(
        "</head>"

        "<body>"
            "<h1>Sequence Log Service</h1>"
            "<div align=\"center\">"
                "<table>"

                    // シーケンスログリストヘッダー
                    "<tr>"
                        "<th>開始日時</th>"
                        "<th>終了日時</th>"
                        "<th>ログファイル名</th>"
                        "<th>サイズ</th>"
                    "</tr>");

    // シーケンスログリスト
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    ScopedLock lock(serviceMain->getMutex());

    FileInfoArray* sum = serviceMain->getFileInfoArray();

    for (FileInfoArray::iterator i = sum->begin(); i != sum->end(); i++)
    {
        appendSequenceLogList(buffer, *i);
    }

    buffer->append(
            "</table>"
        "</div>"
        "<br />");

    appendFooter(buffer);
}

/*!
 *  \brief  文字列に"Not Found"を追加
 */
static void appendNotFoundContents(String* buffer)
{
    buffer->append(
        "<head>"
            FAVICON
            "<title>404 Not Found</title>");

    appendCSS(buffer);

    buffer->append(
        "</head>"

        "<body>"
            "<h1>Sequence Log Service</h1>"
            "<div align=\"center\">"
                "<h2>Not Found</h2>"
            "</div>");

    appendFooter(buffer);
}

/*!
 *  \brief  実行
 */
void SequenceLogServiceWebServerThread::run()
{
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();

    // WEBサーバーソケット準備
    Socket server;
    server.open();
    server.setReUseAddress(true);
    server.bind(serviceMain->getWebServerPort());
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

            SequenceLogServiceWebServerResponseThread* response = new SequenceLogServiceWebServerResponseThread(client);
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
SequenceLogServiceWebServerResponseThread::SequenceLogServiceWebServerResponseThread(Socket* socket) :
    WebServerResponseThread(socket)
{
}

/*!
 *  \brief  実行
 */
void SequenceLogServiceWebServerResponseThread::run()
{
    try
    {
        bool requestOK = analizeRequest();

        if (requestOK)
        {
            // URL取得
            const CoreString& url = getUrl();

            if (0 == url.getLength())
            {
                String fileNameKey = "fileName";

                for (map<String, String>::iterator i = mPostParams.begin(); i != mPostParams.end(); i++)
                {
                    if (i->first.equals(fileNameKey))
                    {
                        SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();

                        requestOK = sendSequenceLog(
                            serviceMain->getSequenceLogServerIP(),
                            serviceMain->getSequenceLogServerPort(),
                            i->second);
                        break;
                    }
                }
            }
            else
            {
                requestOK = false;
            }
        }

        // 応答内容生成
        String content;

        if (requestOK)
        {
            appendRequestContents(&content);
        }
        else
        {
            appendNotFoundContents(&content);
        }

        // HTTPヘッダー送信
        sendHttpHeader(content.getLength());

        // 応答内容送信
        mSocket->send(&content, content.getLength());
        mSocket->close();
    }
    catch (Exception& e)
    {
        noticeLog(e.getMessage());
    }
}

} // namespace slog
