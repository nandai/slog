﻿/*
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
 * \file    WebServerResponseThread.h
 * \brief   WEBサーバー応答クラス
 * \author  Copyright 2011-2014 printf.jp
 */
#pragma once

#include "slog/Thread.h"
#include "slog/HtmlGenerator.h"
#include "slog/Cookie.h"

namespace slog
{
class HttpRequest;
class CoreString;
class String;
class Socket;
class Buffer;

/*!
 * \brief   WEBサーバー応答クラス
 */
class SLOG_API WebServerResponse : public Thread
{
            /*!
             * httpリクエスト
             */
protected:  HttpRequest* mHttpRequest;

            /*!
             * 変数リスト
             */
            VariableList mVariables;

            /*!
             * Cookieリスト
             */
            CookieList mCookies;

            /*!
             * "Transfer-Encoding: chunked" で返すかどうか
             */
private:    bool mChunked;

            /*!
             * コンストラクタ
             */
public:     WebServerResponse(HttpRequest* httpRequest);

            /*!
             * デストラクタ
             */
            virtual ~WebServerResponse();

            /*!
             * 変数初期化
             */
protected:  virtual void initVariables() {}

            /*!
             * 送信
             */
            void send(const DateTime* lastModified, const Buffer* content) const;

            /*!
             * not found 送信
             */
            void sendNotFound(HtmlGenerator* generator) const;

            /*!
             * バイナリ送信
             */
            void sendBinary(HtmlGenerator* generator, const CoreString* path) const;

            /*!
             * HTTPヘッダー送信（＆切断）
             */
            void sendHttpHeader(const DateTime* lastModified, int32_t contentLen) const;

            /*!
             * 応答内容送信＆切断
             */
            void sendContent(const Buffer* content) const;

            /*!
             * リダイレクト
             */
            void redirect(const CoreString* url) const;

            /*!
             * BASIC認証
             */
            void basicAuth(const char* realm) const;

            /*!
             * 実行
             */
            virtual void run() override;

            /*!
             * "Set-Cookie"を文字列に追加
             */
private:    void appendCookiesString(CoreString* str) const;

            /*!
             * WebSocketにアップグレード
             */
protected:  bool upgradeWebSocket();

            /*!
             * WebSocketヘッダー送信
             */
            void sendWebSocketHeader(uint64_t payloadLen, bool isText = true, bool toClient = true) const;
};

} // namespace slog
