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
 *  \brief  WEBサーバー応答スレッドオブジェクト生成
 */
WebServerResponseThread* SequenceLogServiceWebServerThread::createResponseThread(HttpRequest* httpRequest) const
{
    return new SequenceLogServiceWebServerResponseThread(httpRequest);
}

/*!
 *  \brief  コンストラクタ
 */
SequenceLogServiceWebServerResponseThread::SequenceLogServiceWebServerResponseThread(HttpRequest* httpRequest) :
    WebServerResponseThread(httpRequest)
{
    setListener(this);
}

/*!
 *  \brief  URLマップ取得
 */
const WebServerResponseThread::URLMAP* SequenceLogServiceWebServerResponseThread::getUrlMaps() const
{
    #define self (WEBPROC)&SequenceLogServiceWebServerResponseThread
    static const URLMAP urlmaps[] =
    {
        {HttpRequest::GET,  "",                   "index.html", self::getContents},
        {HttpRequest::POST, "",                   "",           self::webSendSequenceLog},
        {HttpRequest::POST, "getSequenceLogList", "",           self::webGetSequenceLogList},
        {HttpRequest::UNKNOWN}
    };
    return urlmaps;
}

/*!
 *  \brief  ドメイン取得
 */
const char* SequenceLogServiceWebServerResponseThread::getDomain() const
{
    static const char* domain = "printf.jp";
    return domain;
}

/*!
 *  \brief  ルートディレクトリ取得
 */
const char* SequenceLogServiceWebServerResponseThread::getRootDir() const
{
#if !defined(__ANDROID__)
    static const char* rootDir = "SequenceLogServiceWeb";
#else
    static const char* rootDir = "";
#endif

    return rootDir;
}

/*!
 *  \brief  シーケンスログリスト（JSON）
 */
bool SequenceLogServiceWebServerResponseThread::webGetSequenceLogList(String* content, const char* url)
{
    getJsonContent(content);
    return true;
}

/*!
 *  \brief  Sequence Log サーバーにシーケンスログ送信
 */
bool SequenceLogServiceWebServerResponseThread::webSendSequenceLog(String* content, const char* url)
{
    String fileName;
    mHttpRequest->getParam("fileName", &fileName);

    if (fileName.getLength() == 0)
        return false;

    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();

    SendSequenceLogThread* thread = new SendSequenceLogThread(
        serviceMain->getSequenceLogServerIP(),
        serviceMain->getSequenceLogServerPort(),
        fileName);

    thread->start();
    return true;
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
    serviceMain->setListener(this);

    Socket* socket = mHttpRequest->getSocket();

    while (true)
    {
        if (socket->isReceiveData(1 * 1000))
        {
            ByteBuffer buffer(1);
            socket->recv(&buffer, buffer.getLength());

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
    {
        SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
        serviceMain->removeListener(this);
        delete this;
    }
    else
    {
        onLogFileChanged(thread);
    }
}

/*!
 *  \brief	シーケンスログ更新通知
 */
void SequenceLogServiceWebServerResponseThread::onLogFileChanged(Thread* thread)
{
    String content;
    getJsonContent(&content);

    send("0001", &content);
}

/*!
 *  \brief	シーケンスログ更新通知
 */
void SequenceLogServiceWebServerResponseThread::onUpdateLog(const Buffer* text)
{
    send("0002", text);
}

/*!
 *  \brief	
 */
void SequenceLogServiceWebServerResponseThread::send(const char* commandNo, const Buffer* payloadData)
{
    uint32_t payloadDataLen = payloadData->getLength();
    uint32_t commandNoLen = (uint32_t)strlen(commandNo);

    if (commandNoLen != 4)
    {
        noticeLog("commandNoは４バイトでなければならない。");
        return;
    }

    char frame[4];
    int32_t frameLen = 0;
    int32_t totalLen = commandNoLen + payloadDataLen;

    if (totalLen < 126)
    {
        frame[frameLen++] = (char)0x81;
        frame[frameLen++] = (char)totalLen;
    }
    else
    {
        frame[frameLen++] = (char)0x81;
        frame[frameLen++] = (char)126;
        frame[frameLen++] = (char)(totalLen >> 8);
        frame[frameLen++] = (char)(totalLen & 0xFF);
    }

    try
    {
        Socket* socket = mHttpRequest->getSocket();

        socket->send(frame, frameLen);
        socket->send(commandNo, commandNoLen);
        socket->send(payloadData, payloadDataLen);
    }
    catch (Exception&)
    {
        interrupt();
    }
}

} // namespace slog
