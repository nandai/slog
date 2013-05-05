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
 *  \file   HttpRequest.h
 *  \brief  httpリクエストクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once

#include "slog/String.h"
#include <map>

#pragma warning(disable:4251)

namespace slog
{
class Socket;
class ByteBuffer;

/*!
 *  \brief  httpリクエストクラス
 */
class SLOG_API HttpRequest
{
public:     enum METHOD
            {
                UNKNOWN,
                GET,
                POST,
            };

private:    Socket*                     mSocket;
            uint16_t                    mPort;
            METHOD                      mMethod;        // 要求メソッド
            String                      mUrl;           // 要求URL
            std::map<String, String>    mPostParams;    // POSTパラメータ
            String                      mWebSocketKey;  // Sec-WebSocket-Key

public:      HttpRequest(Socket* socket, uint16_t port);
            ~HttpRequest();

public:     bool    analizeRequest();
private:    int32_t analizeUrl(const char* request, int32_t len, METHOD method);
            void    analizePostParams(ByteBuffer* params);

public:     Socket* getSocket() const;
            uint16_t getPort() const;
            METHOD getMethod() const;
            const CoreString& getUrl() const;
            void setUrl(const char* url);
            void getParam(const char* name, CoreString* param);
            const CoreString& getWebSocketKey() const;
};

} // namespace slog
