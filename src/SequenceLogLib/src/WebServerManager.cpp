/*
 * Copyright (C) 2014 printf.jp
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
 *  \file   WebServerManager.cpp
 *  \brief  WEBサーバーマネージャークラス
 *  \author Copyright 2014 printf.jp
 */
#include "slog/WebServerManager.h"
#include "slog/WebServerThread.h"

#include "Session.h"

namespace slog
{

/*!
 * \brief   コンストラクタ
 */
WebServerManager::WebServerManager()
{
    mWebServer = nullptr;
    mSecureWebServer = nullptr;
}

/*!
 * \brief   デストラクタ
 */
WebServerManager::~WebServerManager()
{
    delete mWebServer;
    delete mSecureWebServer;
}

/*!
 * \brief   WEBサーバー取得
 */
WebServer* WebServerManager::getWebServer(bool secure) const
{
    if (secure == false)
    {
        return mWebServer;
    }
    else
    {
        return mSecureWebServer;
    }
}

/*!
 * \brief   WEBサーバー設定
 */
void WebServerManager::setWebServer(WebServer* webServer, bool secure)
{
    if (secure == false)
    {
        mWebServer = webServer;
    }
    else
    {
        mSecureWebServer = webServer;
    }
}

/*!
 * \brief   開始
 */
void WebServerManager::start()
{
    if (mWebServer)
        mWebServer->start();

    if (mSecureWebServer)
        mSecureWebServer->start();
}

/*!
 * \brief   停止
 */
void WebServerManager::stop()
{
    if (mWebServer)
        mWebServer->interrupt();

    if (mSecureWebServer)
        mSecureWebServer->interrupt();

    if (mWebServer)
        mWebServer->join();

    if (mSecureWebServer)
        mSecureWebServer->join();

    SessionManager::clear();
}

} // namespace slog
