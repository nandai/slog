/*
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
#include "slog/Util.h"

namespace slog
{

/*!
 * 応答スレッド生成クラス
 */
class CreateResponseThread : public Thread
{
            WebServerThread*    mWebServer;
            HttpRequest*        mHttpRequest;

public:     CreateResponseThread(WebServerThread* webServer, HttpRequest* httpRequest);
            virtual ~CreateResponseThread();

            virtual void run();
};

/*!
 * コンストラクタ
 */
CreateResponseThread::CreateResponseThread(WebServerThread* webServer, HttpRequest* httpRequest)
{
    mWebServer = webServer;
    mHttpRequest = httpRequest;
}

/*!
 * デストラクタ
 */
CreateResponseThread::~CreateResponseThread()
{
}

/*!
 * スレッド実行
 */
void CreateResponseThread::run()
{
    try
    {
        WebServerResponseThread* response = nullptr;

        // リクエスト解析
        if (mHttpRequest->analizeRequest())
        {
            noticeLog("request URL: /%s", mHttpRequest->getUrl().getBuffer());
            response = mWebServer->createResponse(mHttpRequest);
        }

        // 応答スレッド実行
        if (response)
        {
            mWebServer->onResponseStart(response);
            response->start();
        }
    }
    catch (Exception& e)
    {
        noticeLog("CreateResponseThread: %s", e.getMessage());
    }
}

/*!
 *  \brief  コンストラクタ
 */
WebServerThread::WebServerThread()
{
    mPort = 8080;
}

/*!
 * ルートディレクトリ設定
 */
void WebServerThread::setRootDir(const char* rootDir)
{
    if (rootDir == nullptr)
        rootDir = "";

    if (rootDir[0] == '/' || rootDir[1] == ':')
    {
        mRootDir.format(
            "%s/",
            rootDir);
    }
    else
    {
        String processPath;
        Util::getProcessPath(&processPath);

        mRootDir.format(
            "%s/%s/",
            processPath.getBuffer(),
            rootDir);
    }
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
 * SSL関連
 */
void WebServerThread::setSSLFileName(const CoreString& certificate, const CoreString& privateKey)
{
    mCertificate.copy(certificate);
    mPrivateKey. copy(privateKey);
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
    server.setNoDelay(true);
    server.bind(mPort);
    server.listen();

    // 要求待ち
    while (true)
    {
        Socket* client = nullptr;

        try
        {
            bool isReceive = server.isReceiveData(3000);

            if (isInterrupted())
                break;

            if (isReceive == false)
                continue;

            // accept
            client = new Socket;
            client->accept(&server);
            client->setNoDelay(true);

            // SSL関連ファイルが設定されていたらSSL有効化
            if (0 < mCertificate.getLength() && 0 < mPrivateKey.getLength())
                client->useSSL(mCertificate, mPrivateKey);

            // 応答スレッドを生成するスレッドを実行
            HttpRequest* httpRequest = new HttpRequest(client, mPort, &mRootDir);
            CreateResponseThread* createResponseThread = new CreateResponseThread(this, httpRequest);

            createResponseThread->start();
        }
        catch (Exception& e)
        {
            noticeLog("WebServerThread: %s", e.getMessage());
            continue;
        }
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
    WebServerResponseThread* response = nullptr;

    // 条件に一致する応答スレッドを検索
    while (createList->method != HttpRequest::UNKNOWN)
    {
        String tmp = createList->url;

        if (createList->method == method && url.equals(tmp))
        {
            // HTTPメソッドとURLが一致
            if (createList->replaceUrl[0] != '\0')
            {
                // URL置換
                httpRequest->setUrl(createList->replaceUrl);
            }

            // 応答スレッド生成
            response = (*createList->proc)(httpRequest);
            break;
        }

        createList++;
    }

    // 一致するものがなければデフォルトの応答スレッドを生成
//  if (response == nullptr && HttpRequest::GET == method)
    if (response == nullptr)
        response = (*createList->proc)(httpRequest);

    return response;
}

} // namespace slog
