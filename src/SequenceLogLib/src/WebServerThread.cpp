/*
 * Copyright (C) 2011-2014 printf.jp
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
 * \file    WebServerThread.cpp
 * \brief   WEBサーバークラス
 * \author  Copyright 2011-2014 printf.jp
 */
#include "slog/WebServerThread.h"
#include "slog/WebServerResponseThread.h"
#include "slog/Socket.h"
#include "slog/Util.h"
#include "slog/Session.h"

namespace slog
{

/*!
 * \brief   応答スレッド生成クラス
 */
class CreateResponseThread : public Thread
{
            /*!
             * WEBサーバー
             */
            WebServer* mWebServer;

            /*!
             * httpリクエスト
             */
            HttpRequest* mHttpRequest;

            /*!
             * コンストラクタ
             */
public:     CreateResponseThread(WebServer* webServer, HttpRequest* httpRequest);

            /*!
             * デストラクタ
             */
            virtual ~CreateResponseThread();

            /*!
             * スレッド実行
             */
            virtual void run() override;
};

/*!
 * \brief   コンストラクタ
 */
CreateResponseThread::CreateResponseThread(WebServer* webServer, HttpRequest* httpRequest)
{
    mWebServer = webServer;
    mHttpRequest = httpRequest;
}

/*!
 * \brief   デストラクタ
 */
CreateResponseThread::~CreateResponseThread()
{
}

/*!
 * \brief   スレッド実行
 */
void CreateResponseThread::run()
{
    try
    {
        while (true)
        {
            Socket* socket = mHttpRequest->getSocket();
            bool isReceive = socket->isReceiveData(3000);

            if (isInterrupted())
                break;

            if (isReceive == false)
                continue;

            // リクエスト解析
            WebServerResponse* response = nullptr;

            if (mHttpRequest->analizeRequest())
            {
                noticeLog("request URL: /%s", mHttpRequest->getUrl()->getBuffer());
                response = mWebServer->createResponse(mHttpRequest);
            }

            // 応答スレッド実行
            if (response)
            {
                mWebServer->onResponseStart(response);
                response->start();

                while (response->isAlive())
                {
                    if (isInterrupted() == false)
                    {
                        sleep(2000);
                    }
                    else
                    {
                        response->interrupt();
                        response->join();
                    }
                }

                delete response;
            }
        }
    }
    catch (Exception& e)
    {
        noticeLog("CreateResponseThread: %s", e.getMessage());
    }

    delete mHttpRequest;
    mHttpRequest = nullptr;
}

/*!
 * \brief   コンストラクタ
 */
WebServer::WebServer()
{
    mPort = 8080;
}

/*!
 * \brief   ルートディレクトリ設定
 *
 * \param[in]   rootDir ルートディレクトリ
 *
 * \return  なし
 */
void WebServer::setRootDir(const char* rootDir)
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
 * \brief   ポート取得
 *
 * \return  ポート番号
 */
uint16_t WebServer::getPort() const
{
    return mPort;
}

/*!
 * \brief   ポート設定
 *
 * \param[in]   port    ポート番号
 */
void WebServer::setPort(uint16_t port)
{
    mPort = port;
}

/*!
 * \brief   SSL関連ファイル設定
 *
 * \param[in]   certificate         証明書
 * \param[in]   privateKey          秘密鍵
 * \param[in]   certificateChain    中間証明書（null可）※未対応
 *
 * \return  なし
 */
void WebServer::setSSLFileName(
    const CoreString* certificate,
    const CoreString* privateKey,
    const CoreString* certificateChain)
{
    mCertificate.copy(certificate);
    mPrivateKey. copy(privateKey);

    if (certificateChain && certificateChain->getLength())
        mCertificateChain.copy(certificateChain);
}

/*!
 * \brief   実行
 */
void WebServer::run()
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
        Socket* socket = nullptr;

        try
        {
            bool isReceive = server.isReceiveData(3000);

            if (isInterrupted())
                break;

            if (isReceive == false)
                continue;

            // accept
            socket = new Socket;
            socket->accept(&server);
            socket->setNoDelay(true);

            // SSL関連ファイルが設定されていたらSSL有効化
            HttpRequest::SCHEME scheme = HttpRequest::HTTP;

            if (0 < mCertificate.getLength() && 0 < mPrivateKey.getLength())
            {
                scheme = HttpRequest::HTTPS;
                socket->useSSL(&mCertificate, &mPrivateKey, &mCertificateChain);
            }

            // 応答スレッドを生成するスレッドを実行
            HttpRequest* httpRequest = new HttpRequest(scheme, socket, mPort, &mRootDir);
            httpRequest->setListener(this);

            CreateResponseThread* createResponseThread = new CreateResponseThread(this, httpRequest);
            createResponseThread->start();

            mCreateResponseList.push_back(createResponseThread);
        }
        catch (Exception& e)
        {
            noticeLog("WebServerThread: %s", e.getMessage());
            continue;
        }
    }

    // 終了処理
    for (auto i = mCreateResponseList.begin(); i != mCreateResponseList.end(); i++)
    {
        Thread* thread = *i;
        thread->interrupt();
    }

    for (auto i = mCreateResponseList.begin(); i != mCreateResponseList.end(); i++)
    {
        Thread* thread = *i;
        thread->join();
        delete thread;
    }

    mCreateResponseList.clear();
}

/*!
 * \brief   WEBサーバー応答オブジェクト生成
 *
 * \param[in,out]   httpRequest HTTPリクエスト
 *
 * \return  WEBサーバーレスポンス
 */
WebServerResponse* WebServer::createResponse(HttpRequest* httpRequest)
{
    HttpRequest::METHOD method = httpRequest->getMethod();
    const CoreString* url =      httpRequest->getUrl();

    const CREATE* createList = getCreateList();
    WebServerResponse* response = nullptr;

    // 条件に一致する応答スレッドを検索
    while (createList->url && createList->url[0] != '\0')
    {
        if (url->equals(createList->url))
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

    // セッション管理
    do
    {
        String sessionId;
        httpRequest->getCookie(Session::NAME, &sessionId);

        if (0 == sessionId.getLength())
            break;

        const CoreString* ip =        httpRequest->getSocket()->getInetAddress();
        const CoreString* userAgent = httpRequest->getUserAgent();

        Session* session = SessionManager::getBySessionIdAndIp(&sessionId, ip, userAgent);

        if (session == nullptr)
            break;

        if (userAgent->indexOf("Firefox") == -1)    // Firefoxの場合、再生成すると以降のアクセス時にセッションIDが一致しないので（原因不明）
                                                    // 再生成しないようにする
        {
            if (httpRequest->getMimeType()->type == MimeType::Type::HTML)
            {
                // アクセス毎にセッションIDを再生成する
                session->generate();
                response->getCookieList()->add(Session::NAME, session->getId(), "/", nullptr, session->isSecure(), true);
            }
        }

        response->setUserId(session->getUserId());
    }
    while (false);

    return response;
}

} // namespace slog
