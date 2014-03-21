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
 * \file    SequenceLogServiceWebServer.cpp
 * \brief   シーケンスログサービスWEBサーバークラス
 * \author  Copyright 2013-2014 printf.jp
 */
#include "SequenceLogServiceWebServer.h"
#include "SequenceLogServiceWebServerResponse.h"
#include "AccountResponse.h"
#include "GetLogResponse.h"
#include "SequenceLogService.h"

namespace slog
{

/*!
 * \brief   コンストラクタ
 */
SequenceLogServiceWebServer::SequenceLogServiceWebServer()
{
#if !defined(__ANDROID__)
    setRootDir("SequenceLogServiceWeb");
#else
    setRootDir("");
#endif
}

/*!
 * \brief   WEBサーバー応答スレッドオブジェクト生成リスト取得
 */
static WebServerResponse* createSequenceLogServiceWebServerResponse(HttpRequest* httpRequest) {return new SequenceLogServiceWebServerResponse(httpRequest);}
static WebServerResponse* createAccountResponse(                    HttpRequest* httpRequest) {return new AccountResponse(                    httpRequest);}
static WebServerResponse* createGetLogResponse(                     HttpRequest* httpRequest) {return new GetLogResponse(                     httpRequest);}
static WebServerResponse* createSequenceLogService(                 HttpRequest* httpRequest) {return new SequenceLogService(                 httpRequest);}

/*!
 * 
 */
const WebServer::CREATE* SequenceLogServiceWebServer::getCreateList() const
{
    static const CREATE creates[] =
    {
        {"getLog",       "", createGetLogResponse},
        {"outputLog",    "", createSequenceLogService},
        {"account.html", "", createAccountResponse},
        {"",             "", createSequenceLogServiceWebServerResponse}
    };
    return creates;
}

/*!
 * \brief   実行
 */
void SequenceLogServiceWebServer::run()
{
    WebServer::run();
}

/*!
 * \brief   onResponseStart
 */
void SequenceLogServiceWebServer::onResponseStart(WebServerResponse* response)
{
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    serviceMain->onResponseStart(response);
}

} // namespace slog
