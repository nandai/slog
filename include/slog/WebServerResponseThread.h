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
 *  \file   WebServerResponseThread.h
 *  \brief  WEBサーバー応答スレッドクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once
#include "slog/Thread.h"

namespace slog
{
class HttpRequest;
class CoreString;
class String;

/*!
 *  \brief  WEBサーバー応答スレッドクラス
 */
class SLOG_API WebServerResponseThread : public Thread
{
protected:  HttpRequest*    mHttpRequest;

public:     WebServerResponseThread(HttpRequest* httpRequest);
            virtual ~WebServerResponseThread();

private:    virtual const char* getDomain() const {return NULL;}
            virtual const char* getRootDir() const {return NULL;}

protected:  void send(const CoreString& content) const;
            void sendHttpHeader(int32_t contentLen) const;
            void sendContent(const CoreString& content) const;

private:    virtual void run();
protected:  bool getContents(String* content, const char* url);

protected:  bool upgradeWebSocket();
};

} // namespace slog
