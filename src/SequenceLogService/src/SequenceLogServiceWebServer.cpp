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
#include "slog/Json.h"

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

    // 開始日時
    String strCreationTime;
    dateTime = info->getCreationTime();

    if (dateTime.getValue())
    {
        dateTime.toLocal();
        DateTimeFormat::toString(&strCreationTime, dateTime, DateTimeFormat::DATE_TIME);
    }

    // 終了日時
    String strLastWriteTime;
    dateTime = info->getLastWriteTime();

    if (dateTime.getValue())
    {
        dateTime.toLocal();
        DateTimeFormat::toString(&strLastWriteTime, dateTime, DateTimeFormat::DATE_TIME);
    }

    // ログファイル名
    const CoreString& strCanonicalPath = info->getCanonicalPath();

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
 *  \brief  WEBサーバー応答スレッドオブジェクト生成
 */
WebServerResponseThread* SequenceLogServiceWebServerThread::createResponseThread(Socket* socket) const
{
    return new SequenceLogServiceWebServerResponseThread(socket);
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
                        Json* json = Json::getNewObject();

                        for (FileInfoArray::iterator i = sum->begin(); i != sum->end(); i++)
                        {
                            Json* jsonSequenceLogInfo = Json::getNewObject();

                            createSequenceLogListJson(jsonSequenceLogInfo, *i);
                            json->add(jsonSequenceLogInfo);
                        }

                        json->serialize(&content);
                        delete json;
                    }
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
