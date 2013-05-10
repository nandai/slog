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
#include "SequenceLogServiceWebServerResponse.h"
#include "GetLogResponse.h"
#include "SequenceLogService.h"

namespace slog
{

/*!
 *  \brief  WEBサーバー応答スレッドオブジェクト生成リスト取得
 */
static WebServerResponseThread* createSequenceLogServiceWebServerResponse(HttpRequest* httpRequest) {return new SequenceLogServiceWebServerResponse(httpRequest);}
static WebServerResponseThread* createGetLogResponse(                     HttpRequest* httpRequest) {return new GetLogResponse(                     httpRequest);}
static WebServerResponseThread* createSequenceLogService(                 HttpRequest* httpRequest) {return new SequenceLogService(                 httpRequest);}

const WebServerThread::CREATE* SequenceLogServiceWebServerThread::getCreateList() const
{
    static const CREATE creates[] =
    {
        {HttpRequest::GET,     "",                   "index.html", createSequenceLogServiceWebServerResponse},
        {HttpRequest::GET,     "getLog",             "",           createGetLogResponse},
        {HttpRequest::GET,     "outputLog",          "",           createSequenceLogService},
        {HttpRequest::UNKNOWN, "",                   "",           createSequenceLogServiceWebServerResponse}
    };
    return creates;
}

void SequenceLogServiceWebServerThread::run()
{
    WebServerThread::run();
}

} // namespace slog
