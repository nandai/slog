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
 *  \file   WebServerThread.h
 *  \brief  WEBサーバースレッドクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once

#include "slog/Thread.h"
#include "slog/String.h"

#include <map>

#pragma warning(disable:4251)

namespace slog
{
class Socket;
class ByteBuffer;

class WebServerResponseThread;

/*!
 *  \brief  WEBサーバースレッドクラス
 */
class SLOG_API WebServerThread : public Thread
{
            uint16_t    mPort;

public:     WebServerThread() {mPort = 8080;}
            void setPort(uint16_t port) {mPort = port;}

private:    virtual void run();
            virtual WebServerResponseThread* createResponseThread(Socket* socket) const = 0;
};

/*!
 *  \brief  WEBサーバー応答スレッドクラス
 */
class SLOG_API WebServerResponseThread : public Thread, public ThreadListener
{
public:     enum METHOD
            {
                UNKNOWN,
                GET,
                POST,
            };

private:    Socket*                     mSocket;
            METHOD                      mMethod;        // 要求メソッド
            String                      mUrl;           // 要求URL
            std::map<String, String>    mPostParams;    // POSTパラメータ

public:     WebServerResponseThread(Socket* socket);
            virtual ~WebServerResponseThread();

private:    virtual void onTerminated(Thread* thread);

public:     bool    analizeRequest();
            int32_t analizeUrl(const char* request, int32_t len, METHOD method);
            void    analizePostParams(ByteBuffer* params);

            METHOD getMethod() const;
            const CoreString& getUrl() const;
            void getParam(const char* name, CoreString* param);

            void sendHttpHeader(int32_t contentLen) const;
            void sendContent(String* content) const;
};

} // namespace slog
