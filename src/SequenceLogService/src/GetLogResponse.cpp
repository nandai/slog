/*
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
class SendSequenceLogThread : public Thread
{
            String      mIP;
            uint16_t    mPort;
            String      mLogFilePath;

            /*!
             * コンストラクタ
             */
public:     SendSequenceLogThread(const CoreString* ip, uint16_t port, const CoreString* path);

            /*!
             * スレッド実行
             */
private:    virtual void run() override;
};

/*!
 * \brief   コンストラクタ
 */
SendSequenceLogThread::SendSequenceLogThread(const CoreString* ip, uint16_t port, const CoreString* path)
{
    mIP.copy(ip);
    mPort = port;
    mLogFilePath.copy(path);
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
 * \brief   コンストラクタ
 */
GetLogResponse::GetLogResponse(HttpRequest* httpRequest) : WebServerResponse(httpRequest)
{
    mSendSequenceLogThread = nullptr;
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
    if (upgradeWebSocket() == false)
        return;

    if (getUserId() < 0)
        return;

    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    serviceMain->addThreadListener(this);
    serviceMain->addSequenceLogServiceListener(this);

    try
    {
        // シーケンスログファイル変更通知によりログファイルリストをブラウザに送信する
        onLogFileChanged(nullptr, nullptr, getUserId());

        // シーケンスログ送信ループ
        Socket* socket = mHttpRequest->getSocket();

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

            if (cmd == 1 && (mSendSequenceLogThread == nullptr || mSendSequenceLogThread->isAlive() == false))
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

                    delete mSendSequenceLogThread;
                    mSendSequenceLogThread = new SendSequenceLogThread(
                        socket->getInetAddress(),
                        serviceMain->getSequenceLogServerPort(),
//                      &fileName);
                        fileInfo->getCanonicalPath());

                    mSendSequenceLogThread->start();
                }
            }

            delete buffer;
        }
    }
    catch (Exception& e)
    {
        noticeLog("GetLogResponse: %s", e.getMessage());
    }

    // シーケンスログ送信スレッド終了待ち
    if (mSendSequenceLogThread)
    {
        mSendSequenceLogThread->join();

        delete mSendSequenceLogThread;
        mSendSequenceLogThread = nullptr;
    }

    // シーケンスログサービスメインへのリスナー登録を解除
    serviceMain->removeThreadListener(this);
    serviceMain->removeSequenceLogServiceListener(this);
}

/*!
 * \brief   スレッド終了通知
 */
void GetLogResponse::onThreadTerminated(Thread* thread)
{
    // ログ出力スレッドの終了時もシーケンスログファイル変更通知を行う
    // thread != this
    onLogFileChanged(nullptr, nullptr, getUserId());
}

/*!
 * \brief   シーケンスログファイル変更通知
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
