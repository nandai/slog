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
            virtual WebServerResponseThread* createResponseThread(HttpRequest* httpRequest) const;
};

/*!
 *  \brief  シーケンスログサービスWEBサーバー応答スレッドクラス
 */
class SequenceLogServiceWebServerResponseThread :
    public WebServerResponseThread,
    public SequenceLogServiceThreadListener
{
public:     SequenceLogServiceWebServerResponseThread(HttpRequest* httpRequest);

private:    virtual const URLMAP* getUrlMaps() const;
            virtual const char* getDomain() const;
            virtual const char* getRootDir() const;

private:    virtual void WebSocketMain();

            bool webGetSequenceLogList(String* content, const char* url);
            bool webSendSequenceLog(   String* content, const char* url);

            void getJsonContent(String* content) const;

public:     virtual void onInitialized(   Thread* thread) {}
            virtual void onTerminated(    Thread* thread);
            virtual void onLogFileChanged(Thread* thread);
            virtual void onUpdateLog(const Buffer* text);

            void send(const char* commandNo, const Buffer* payloadData);
};

} // namespace slog
