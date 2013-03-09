/*
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

    char work[1024 + 1];
    int32_t i = 0;

    while (true)
    {
        // 受信
        buffer.setLength(0);
        mSocket->recv(&buffer, size);

        // 改行までワークに貯める
        char c = buffer.get();

        if (c != '\r')
        {
            if (sizeof(work) <= i)
                return false;

            work[i] = c;
            i++;

            continue;
        }

        // '\n'捨て
        buffer.setLength(0);
        mSocket->recv(&buffer, size);

        // 空行だったらループを抜ける
        work[i] = '\0';

        if (i == 0)
            break;

        // URL取得
        if (4 <= i && strncmp(work, "GET ", 4) == 0)
        {
            const char* p1 = work + 4;
            const char* p2 = strchr(p1, ' ');

            if (p2 == NULL)
                return false;

            p1++;
            mUrl.copy(p1, p2 - p1);
        }

        i = 0;
    }

    return true;
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
static bool sendSequenceLog(const CoreString& ip, uint16_t port, const CoreString& name)
{
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    int32_t len = name.getLength();
    bool result = false;

    FixedString<MAX_PATH> path;
    path.format(
        "%s%c%s",
        serviceMain->getLogFolderName().getBuffer(),
        PATH_DELIMITER,
        name.getBuffer());

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
    String strFileName;
    const char* canonicalPath = info->getCanonicalPath().getBuffer();

    strFileName.format(
        "<td><a href=\"%s\">%s</a></td>",
        strrchr(canonicalPath, PATH_DELIMITER) + 1,
        canonicalPath);

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

            if (0 < url.getLength())
            {
                // シーケンスログ送信
                SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();

                requestOK = sendSequenceLog(
                    serviceMain->getSequenceLogServerIP(),
                    serviceMain->getSequenceLogServerPort(),
                    url);
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
