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

/*!
 *  \brief  シーケンスログ送信スレッド
 */
class SendSequenceLogThread : public Thread, public ThreadListener
{
            String      mIP;
            uint16_t    mPort;
            String      mLogFilePath;

public:     SendSequenceLogThread(const CoreString& ip, uint16_t port, const CoreString& path);

private:    virtual void run();
            virtual void onTerminated(Thread* thread);
};

/*!
 *  \brief  コンストラクタ
 */
SendSequenceLogThread::SendSequenceLogThread(const CoreString& ip, uint16_t port, const CoreString& path)
{
    mIP.copy(ip);
    mPort = port;
    mLogFilePath.copy(path);
}

/*!
 *  \brief  実行
 */
void SendSequenceLogThread::run()
{
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    bool result = false;

    String name = strrchr(mLogFilePath.getBuffer(), PATH_DELIMITER) + 1;
    int32_t len = name.getLength();

    try
    {
        // ファイルオープン
        File file;
        file.open(mLogFilePath, File::READ);

        result = true;

        // ソケット準備
        Socket viewerSocket;
        viewerSocket.open();
        viewerSocket.connect(mIP, mPort);

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
}

/*!
 *  \brief  スレッド終了通知
 */
void SendSequenceLogThread::onTerminated(Thread* thread)
{
    delete this;
}

/*!
 *  \brief  文字列にシーケンスログリストを追加
 */
static void appendSequenceLogList(String* buffer, FileInfo* info)
{
    DateTime dateTime;

    // 開始日時
    String strCreationTime;
    dateTime = info->getCreationTime();
    dateTime.toLocal();

    if (dateTime.getValue())
    {
        DateTimeFormat::toString(&strCreationTime, dateTime, DateTimeFormat::DATE_TIME);
    }

    // 終了日時
    String strLastWriteTime;
    dateTime = info->getLastWriteTime();
    dateTime.toLocal();

    if (dateTime.getValue())
    {
        DateTimeFormat::toString(&strLastWriteTime, dateTime, DateTimeFormat::DATE_TIME);
    }

    // ログファイル名
    const CoreString& strFileName= info->getCanonicalPath();

    // サイズ
    String strSize;
    const CoreString& message = info->getMessage();

    if (info->isUsing() == false)
    {
        double size = (double)info->getSize();

        if (size < 1024)
        {
            strSize.format("%s %d byte(s)", message.getBuffer(), (int64_t)size);
        }

        else
        if (size < 1024 * 1024)
        {
            strSize.format("%s %.1f KB", message.getBuffer(), size / 1024);
        }

        else
        {
            strSize.format("%s %.1f MB", message.getBuffer(), size / (1024 * 1024));
        }
    }

    // 
    const char* p = strFileName.getBuffer();
    int32_t len =   strFileName.getLength() * 2;
    char* filePath = new char[len + 1];
    int32_t pos = 0;

    while (*p)
    {
        if (*p != '\\')
        {
            filePath[pos] = *p;
            pos++;
        }
        else
        {
            filePath[pos + 0] = '\\';
            filePath[pos + 1] = '\\';
            pos += 2;
        }

        p++;
    }

    filePath[pos] = '\0';

    // 文字列追加
    String str;
    str.format(
        "{"
            "\"creationTime\" :\"%s\","
            "\"lastWriteTime\":\"%s\","
            "\"canonicalPath\":\"%s\","
            "\"size\"         :\"%s\""
        "}",
        strCreationTime. getBuffer(),
        strLastWriteTime.getBuffer(),
        filePath,
        strSize.         getBuffer());

    buffer->append(str);
    delete [] filePath;
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
        "<script type=\"text/javascript\" src=\"http://" DOMAIN "/js/SequenceLogService.js\"></script>");
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
            "<title>Sequence Log Service</title>"
            "<link type=\"text/css\" rel=\"stylesheet\" href=\"http://" DOMAIN "/css/SequenceLogService.css\" media=\"screen\" />");

    appendJavaScript(buffer);
    appendCSS(buffer);

    buffer->append(
        "</head>"

        "<body>"
            "<h1>Sequence Log Service</h1>"
            "<div align=\"center\">"
                "<table id=\"sequenceLogList\">"

                    // シーケンスログリストヘッダー
                    "<tr>"
                        "<th>開始日時</th>"
                        "<th>終了日時</th>"
                        "<th>ログファイル名</th>"
                        "<th>サイズ</th>"
                    "</tr>"
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
        String content;
        bool requestOK = analizeRequest();

        if (requestOK)
        {
            // URL取得
            const CoreString& url = getUrl();

            if (0 == url.getLength())
            {
                if (getMethod() == GET)
                {
                    appendRequestContents(&content);
                }
                else
                {
                    String fileName;
                    getParam("fileName", &fileName);

                    if (fileName.getLength())
                    {
                        SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();

//                      requestOK = sendSequenceLog(
                        SendSequenceLogThread* thread = new SendSequenceLogThread(
                            serviceMain->getSequenceLogServerIP(),
                            serviceMain->getSequenceLogServerPort(),
                            fileName);

                        thread->start();
                    }
                }
            }
            else
            {
                if (getMethod() == GET)
                {
                    requestOK = false;
                }
                else
                {
                    String getSequenceLogList = "getSequenceLogList";

                    if (url.equals(getSequenceLogList))
                    {
                        SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
                        ScopedLock lock(serviceMain->getMutex());

                        FileInfoArray* sum = serviceMain->getFileInfoArray();
                        char sep = '[';

                        for (FileInfoArray::iterator i = sum->begin(); i != sum->end(); i++)
                        {
                            content.append(&sep, 1);
                            appendSequenceLogList(&content, *i);
                            sep = ',';
                        }

                        content.append("]", 1);
                    }
                    else
                        content.copy("failed!");
                }
            }
        }

        // 応答内容生成
        if (requestOK == false && getMethod() == GET)
        {
            appendNotFoundContents(&content);
        }

        // HTTPヘッダー送信
        int32_t contentLen = content.getLength();
        sendHttpHeader(contentLen);

        // 応答内容送信
        sendContent(&content);
    }
    catch (Exception& e)
    {
        noticeLog(e.getMessage());
    }
}

} // namespace slog
