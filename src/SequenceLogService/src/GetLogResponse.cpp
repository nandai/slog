﻿/*
 * Copyright (C) 2013-2014 printf.jp
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
 *  \file   GetLogResponse.cpp
 *  \brief  取得ログ送信クラス
 *  \author Copyright 2013-2014 printf.jp
 */
#pragma execution_character_set("utf-8")

#include "GetLogResponse.h"
#include "getSequenceLogListJson.h"

#include "slog/HttpRequest.h"
#include "slog/ByteBuffer.h"
#include "slog/File.h"
#include "slog/WebSocket.h"
#include "slog/Mutex.h"
#include "slog/FileInfo.h"

#if defined(__unix__)
    #include <string.h>
#endif

namespace slog
{

/*!
 * \brief   シーケンスログ送信スレッド
 */
class SendSequenceLogThread : public Thread, public ThreadListener
{
            String      mIP;
            uint16_t    mPort;
            String      mLogFilePath;

public:     SendSequenceLogThread(const CoreString* ip, uint16_t port, const CoreString* path);

private:    virtual void run() override;
            virtual void onTerminated(Thread* thread) override;
};

/*!
 * \brief   コンストラクタ
 */
SendSequenceLogThread::SendSequenceLogThread(const CoreString* ip, uint16_t port, const CoreString* path)
{
    mIP.copy(ip);
    mPort = port;
    mLogFilePath.copy(path);

    setListener(this);
}

/*!
 * \brief   実行
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
        file.open(&mLogFilePath, File::READ);

        result = true;

        // ソケット準備
        Socket viewerSocket;
        viewerSocket.open();
        viewerSocket.connect(&mIP, mPort);

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
    catch (Exception& e)
    {
        noticeLog("SendSequenceLogThread: %s", e.getMessage());
    }
}

/*!
 * \brief   スレッド終了通知
 */
void SendSequenceLogThread::onTerminated(Thread* thread)
{
    delete this;
}

/*!
 * \brief   コンストラクタ
 */
GetLogResponse::GetLogResponse(HttpRequest* httpRequest) : WebServerResponse(httpRequest)
{
}

/*!
 * \brief   デストラクタ
 */
GetLogResponse::~GetLogResponse()
{
}

/*!
 * \brief   実行
 */
void GetLogResponse::run()
{
    setListener(this);

    if (upgradeWebSocket() == false)
        return;

    if (getUserId() < 0)
        return;

    try
    {
        SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
        serviceMain->setListener(this);

        Socket* socket = mHttpRequest->getSocket();
        onLogFileChanged(nullptr, nullptr, getUserId());

        while (true)
        {
            bool isReceive = socket->isReceiveData(1000);

            if (isInterrupted())
                break;

            if (isReceive == false)
                continue;

            ByteBuffer* buffer = WebSocket::recv(socket, nullptr);

            if (buffer == nullptr)
                continue;

            int32_t cmd = buffer->getInt();

            if (cmd == 1)
            {
                SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
                ScopedLock lock(serviceMain->getMutex());

                auto sum = serviceMain->getFileInfoArray(getUserId());
                int32_t index = buffer->getInt();

                if (0 <= index && index < (int32_t)sum->size())
                {
                    auto i = sum->begin();
                    advance(i, index);

                    const FileInfo* fileInfo = *i;

                    // シーケンスログサーバーにシーケンスログを送信
                    int32_t len = buffer->getInt();

                    String fileName;
                    fileName.copy(buffer->get(len), len);

                    SendSequenceLogThread* thread = new SendSequenceLogThread(
                        socket->getInetAddress(),
                        serviceMain->getSequenceLogServerPort(),
//                      &fileName);
                        fileInfo->getCanonicalPath());

                    thread->start();
                }
            }

            delete buffer;
        }
    }
    catch (Exception& e)
    {
        noticeLog("GetLogResponse: %s", e.getMessage());
    }
}

/*!
 * \brief   スレッド終了通知
 */
void GetLogResponse::onTerminated(Thread* thread)
{
    if (thread == this)
    {
        SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
        serviceMain->removeListener(this);
    }
    else
    {
        onLogFileChanged(thread, nullptr, getUserId());
    }
}

/*!
 * \brief   シーケンスログ更新通知
 */
void GetLogResponse::onLogFileChanged(Thread* thread, const CoreString* fileName, int32_t userId)
{
    if (getUserId() != userId)
        return;

    String content;
    getSequenceLogListJson(&content, getUserId());

    send("0001", &content);
}

/*!
 * \brief   シーケンスログ更新通知
 */
void GetLogResponse::onUpdateLog(const Buffer* text, int32_t userId)
{
    if (getUserId() != userId)
        return;

    send("0002", text);
}

/*!
 * \brief   取得ログ送信
 */
void GetLogResponse::send(const char* commandNo, const Buffer* payloadData)
{
    uint32_t payloadDataLen = payloadData->getLength();
    uint32_t commandNoLen = (uint32_t)strlen(commandNo);
    int32_t totalLen = commandNoLen + payloadDataLen;

    if (commandNoLen != 4)
    {
        noticeLog("commandNoは４バイトでなければならない。");
        return;
    }

    try
    {
        Socket* socket = mHttpRequest->getSocket();

        WebSocket::sendHeader(socket, totalLen);
        socket->send(commandNo, commandNoLen);
        socket->send(payloadData, payloadDataLen);
    }
    catch (Exception&)
    {
        interrupt();
    }
}

} // namespace slog
