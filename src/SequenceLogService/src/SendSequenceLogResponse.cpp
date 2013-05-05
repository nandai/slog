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
 *  \file   SendSequenceLogResponse.cpp
 *  \brief  Sequence Log サーバーにシーケンスログ送信
 *  \author Copyright 2013 printf.jp
 */
#include "SendSequenceLogResponse.h"
#include "SequenceLogServiceMain.h"

#include "slog/HttpRequest.h"
#include "slog/File.h"
#include "slog/ByteBuffer.h"

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
 *  \brief	Sequence Log サーバーにシーケンスログ送信
 */
void SendSequenceLogResponse::run()
{
    String fileName;
    mHttpRequest->getParam("fileName", &fileName);

    if (fileName.getLength())
    {
        SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();

        SendSequenceLogThread* thread = new SendSequenceLogThread(
            serviceMain->getSequenceLogServerIP(),
            serviceMain->getSequenceLogServerPort(),
            fileName);

        thread->start();
    }

    sendHttpHeader(0);
}

} // namespace slog
