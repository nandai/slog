﻿/*
 * Copyright (C) 2011-2013 printf.jp
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
 *  \file   WebServerThread.cpp
 *  \brief  WEBサーバースレッドクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/WebServerThread.h"
#include "slog/WebServerResponseThread.h"
#include "slog/Socket.h"

namespace slog
{

/*!
 *  \brief  コンストラクタ
 */
WebServerThread::WebServerThread()
{
    mPort = 8080;
}

/*!
 *  \brief  ポート取得
 */
uint16_t WebServerThread::getPort() const
{
    return mPort;
}

/*!
 *  \brief  ポート設定
 */
void WebServerThread::setPort(uint16_t port)
{
    mPort = port;
}

/*!
 *  \brief  実行
 */
void WebServerThread::run()
{
    // WEBサーバーソケット準備
    Socket server;
    server.open();
    server.setReUseAddress(true);
    server.bind(mPort);
    server.listen();

    // 要求待ち
    Socket* client = NULL;

    while (true)
    {
        try
        {
            bool isReceive = server.isReceiveData(3000);

            if (isInterrupted())
                break;

            if (isReceive == false)
                continue;

            client = new Socket;
            client->accept(&server);

            HttpRequest* httpRequest = new HttpRequest(client, mPort);
            WebServerResponseThread* response = NULL;

            if (httpRequest->analizeRequest())
                response = createResponse(httpRequest);

            if (response)
            {
                response->start();
            }
            else
            {
                delete httpRequest;
            }
        }
        catch (Exception& e)
        {
            noticeLog(e.getMessage());
            delete client;
            break;
        }

        client = NULL;
    }
}

/*!
 *  \brief  WEBサーバー応答スレッドオブジェクト生成
 */
WebServerResponseThread* WebServerThread::createResponse(HttpRequest* httpRequest)
{
    HttpRequest::METHOD method = httpRequest->getMethod();
    const CoreString& url =      httpRequest->getUrl();

    const CREATE* createList = getCreateList();
    WebServerResponseThread* response = NULL;

    while (createList->method != HttpRequest::UNKNOWN)
    {
        String tmp = createList->url;

        if (createList->method == method && url.equals(tmp))
        {
            if (createList->replaceUrl[0] != '\0')
                httpRequest->setUrl(createList->replaceUrl);

            response = (*createList->proc)(httpRequest);
            break;
        }

        createList++;
    }

    if (response == NULL && HttpRequest::GET == method)
        response = (*createList->proc)(httpRequest);

    return response;
}

} // namespace slog
