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
 *  \file   GetLogResponse.cpp
 *  \brief  取得ログ送信クラス
 *  \author Copyright 2013 printf.jp
 */
#include "GetLogResponse.h"
#include "getSequenceLogListJson.h"

#include "slog/HttpRequest.h"
#include "slog/ByteBuffer.h"

namespace slog
{

/*!
 *  \brief	
 */
void GetLogResponse::run()
{
    setListener(this);

    if (upgradeWebSocket() == false)
        return;

    try
    {
        SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
        serviceMain->setListener(this);

        Socket* socket = mHttpRequest->getSocket();

        while (true)
        {
            bool isReceive = socket->isReceiveData(1000);

            if (isInterrupted())
                break;

            if (isReceive == false)
                continue;

            ByteBuffer* buffer = recvData(socket, NULL);

            if (buffer)
            {
                noticeLog("GetLobResponse: データを受信する予定はない");
                delete buffer;
            }
        }
    }
    catch (Exception& e)
    {
        noticeLog("GetLogResponse: %s", e.getMessage());
    }
}

/*!
 *  \brief  スレッド終了通知
 */
void GetLogResponse::onTerminated(Thread* thread)
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
void GetLogResponse::onLogFileChanged(Thread* thread)
{
    String content;
    getSequenceLogListJson(&content);

    send("0001", &content);
}

/*!
 *  \brief	シーケンスログ更新通知
 */
void GetLogResponse::onUpdateLog(const Buffer* text)
{
    send("0002", text);
}

/*!
 *  \brief	取得ログ送信
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

        sendWebSocketHeader(totalLen);
        socket->send(commandNo, commandNoLen);
        socket->send(payloadData, payloadDataLen);
    }
    catch (Exception&)
    {
        interrupt();
    }
}

} // namespace slog
