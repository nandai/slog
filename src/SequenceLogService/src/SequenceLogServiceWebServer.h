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
 *  \file   SequenceLogServiceWebServer.h
 *  \brief  シーケンスログサービスWEBサーバークラス
 *  \author Copyright 2013 printf.jp
 */
#pragma once

#include "slog/WebServerThread.h"
#include "SequenceLogServiceMain.h"

namespace slog
{

/*!
 *  \brief  シーケンスログサービスWEBサーバースレッドクラス
 */
class SequenceLogServiceWebServerThread : public WebServerThread
{
            virtual const CREATE* getCreateList() const;
};

/*!
 *  \brief  シーケンスログサービスWEBサーバー応答スレッドクラス
 */
class SequenceLogServiceWebServerResponseThread : public WebServerResponseThread
{
public:     SequenceLogServiceWebServerResponseThread(HttpRequest* httpRequest);

private:    virtual const char* getDomain() const;
            virtual const char* getRootDir() const;
};

/*!
 *  \brief  シーケンスログリスト（JSON）送信
 */
class GetSequenceLogListResponse : public WebServerResponseThread
{
public:     GetSequenceLogListResponse(HttpRequest* httpRequest) : WebServerResponseThread(httpRequest) {}
private:    virtual void run();
};

/*!
 *  \brief  Sequence Log サーバーにシーケンスログ送信
 */
class SendSequenceLogResponse : public WebServerResponseThread
{
public:     SendSequenceLogResponse(HttpRequest* httpRequest) : WebServerResponseThread(httpRequest) {}
private:    virtual void run();
};

/*!
 *  \brief  
 */
class GetLogResponse :
    public WebServerResponseThread,
    public SequenceLogServiceThreadListener
{
public:     GetLogResponse(HttpRequest* httpRequest) : WebServerResponseThread(httpRequest) {}
private:    virtual void run();

public:     virtual void onInitialized(   Thread* thread) {}
            virtual void onTerminated(    Thread* thread);
            virtual void onLogFileChanged(Thread* thread);
            virtual void onUpdateLog(const Buffer* text);

            void send(const char* commandNo, const Buffer* payloadData);
};

} // namespace slog
