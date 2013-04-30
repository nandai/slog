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

#include "slog/String.h"
#include "slog/PointerString.h"
#include "slog/ByteBuffer.h"
#include "slog/Socket.h"
#include "slog/File.h"
#include "slog/DateTime.h"
#include "slog/DateTimeFormat.h"
#include "slog/FileInfo.h"
#include "slog/Mutex.h"
#include "slog/Json.h"
#include "slog/Tokenizer.h"

#define DOMAIN  "printf.jp"
#define FAVICON "<link rel=\"shortcut icon\" href=\"http://" DOMAIN "/images/SequenceLogService.ico\">"

namespace slog
{

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

    setListener(this);
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
 *  \brief  シーケンスログリストJSON作成
 */
static void createSequenceLogListJson(Json* json, FileInfo* info)
{
    DateTime dateTime;

    // ログファイル名
    const CoreString& strCanonicalPath = info->getCanonicalPath();

    // 開始日時
    String strCreationTime = "Unknown";

#if 0 // linuxでは作成日が取得できないので、ファイル名に含めた日時を使うように変更
    dateTime = info->getCreationTime();

    if (dateTime.getValue())
    {
        dateTime.toLocal();
        DateTimeFormat::toString(&strCreationTime, dateTime, DateTimeFormat::DATE_TIME);
    }
#else
    PointerString fileName = strrchr(strCanonicalPath.getBuffer(), PATH_DELIMITER);
    Tokenizer tokenizer('-');
    tokenizer.exec(fileName);

    if (4 <= tokenizer.getCount())
    {
        const CoreString& strDate = tokenizer.getValue(2);
        const CoreString& strTime = tokenizer.getValue(3);

        if (strDate.getLength() == 8 && strTime.getLength() == 6)
        {
            const char* pDate = strDate.getBuffer();
            const char* pTime = strTime.getBuffer();

            strCreationTime.format("%.4s/%.2s/%.2s %.2s:%.2s:%.2s",
                pDate + 0,
                pDate + 4,
                pDate + 6,
                pTime + 0,
                pTime + 2,
                pTime + 4);
        }
    }
#endif

    // 終了日時
    String strLastWriteTime;
    dateTime = info->getLastWriteTime();

    if (dateTime.getValue())
    {
        dateTime.toLocal();
        DateTimeFormat::toString(&strLastWriteTime, dateTime, DateTimeFormat::DATE_TIME);
    }

    // ログファイル名
//  const CoreString& strCanonicalPath = info->getCanonicalPath();

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

    // JSON作成
    json->add("creationTime",  strCreationTime);
    json->add("lastWriteTime", strLastWriteTime);
    json->add("canonicalPath", strCanonicalPath);
    json->add("size",          strSize);
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
static void appendJavaScript(String* buffer, const CoreString& ip, uint16_t port)
{
    String str;
    str.format(
        "<script type=\"text/javascript\" src=\"http://ajax.googleapis.com/ajax/libs/jquery/1.7.2/jquery.min.js\"></script>"
        "<script type=\"text/javascript\" src=\"http://" DOMAIN "/js/SequenceLogService.js\"></script>\n"

        "<script type=\"text/javascript\">"
        "var conn = new WebSocket('ws://%s:%u/');"
        "conn.onmessage = function(e) {"
            "var json = eval(\"(\" + e.data + \")\");"
            "$.sequenceLogList.update(json);"
        "};"
        "</script>\n",
        ip.getBuffer(),
        port);

    buffer->append(str);
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
static void appendRequestContents(String* buffer, const CoreString& ip, uint16_t port)
{
    buffer->append(
        "<head>"
            FAVICON
            "<title>Sequence Log Service</title>"
            "<link type=\"text/css\" rel=\"stylesheet\" href=\"http://" DOMAIN "/css/SequenceLogService.css\" media=\"screen\" />");

    appendJavaScript(buffer, ip, port);
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
 *  \brief  WEBサーバー応答スレッドオブジェクト生成
 */
WebServerResponseThread* SequenceLogServiceWebServerThread::createResponseThread(Socket* socket) const
{
    return new SequenceLogServiceWebServerResponseThread(socket, getPort());
}

/*!
 *  \brief  コンストラクタ
 */
SequenceLogServiceWebServerResponseThread::SequenceLogServiceWebServerResponseThread(Socket* socket, uint16_t port) :
    WebServerResponseThread(socket)
{
    mPort = port;
    setListener(this);
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
                    const CoreString& ip = mSocket->getMyInetAddress();
                    appendRequestContents(&content, ip, mPort);
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
                        getJsonContent(&content);
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

/*!
 *  \brief  JSONコンテンツ取得
 */
void SequenceLogServiceWebServerResponseThread::getJsonContent(String* content) const
{
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    ScopedLock lock(serviceMain->getMutex());

    FileInfoArray* sum = serviceMain->getFileInfoArray();
    Json* json = Json::getNewObject();

    for (FileInfoArray::iterator i = sum->begin(); i != sum->end(); i++)
    {
        Json* jsonSequenceLogInfo = Json::getNewObject();

        createSequenceLogListJson(jsonSequenceLogInfo, *i);
        json->add(jsonSequenceLogInfo);
    }

    json->serialize(content);
    delete json;
}

/*!
 *  \brief  WebSocketメイン
 */
void SequenceLogServiceWebServerResponseThread::WebSocketMain()
{
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    serviceMain->setServiceListener(this);

    while (true)
    {
        if (mSocket->isReceiveData(1 * 1000))
        {
            ByteBuffer buffer(1);
            mSocket->recv(&buffer, buffer.getLength());

            const char* p = buffer.getBuffer();

            String str;
            str.format("%02X", (uint32_t)(uint8_t)p[0]);
            noticeLog(str.getBuffer());

//          if ((p[0] & 0x0F) == 0x08)
//              break;
        }

        if (isInterrupted())
            break;
    }
}

/*!
 *  \brief  スレッド終了通知
 */
void SequenceLogServiceWebServerResponseThread::onTerminated(Thread* thread)
{
    if (thread == this)
        delete this;

    else
        onLogFileChanged(thread);
}

/*!
 *  \brief  
 */
void SequenceLogServiceWebServerResponseThread::onLogFileChanged(Thread* thread)
{
    String content;
    getJsonContent(&content);

    int16_t len = content.getLength();
    char buffer[] = {(char)0x81, (char)126, (char)(len >> 8), (char)(len & 0xFF)};

    try
    {
        mSocket->send(buffer, sizeof(buffer));
        mSocket->send(&content, len);
    }
    catch (Exception&)
    {
        interrupt();
    }
}

} // namespace slog
