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
 * \file    WebServerResponseThread.h
 * \brief   WEBサーバー応答スレッドクラス
 * \author  Copyright 2011-2014 printf.jp
 */
#pragma once

#include "slog/Thread.h"
#include "slog/HtmlGenerator.h"

namespace slog
{
class HttpRequest;
class CoreString;
class String;
class Socket;
class Buffer;

/*!
 * \brief   WEBサーバー応答スレッドクラス
 */
class SLOG_API WebServerResponseThread : public Thread
{
            /*!
             * httpリクエスト
             */
protected:  HttpRequest* mHttpRequest;

            /*!
             * 変数リスト
             */
            slog::VariableList mVariables;

            /*!
             * コンストラクタ
             */
public:     WebServerResponseThread(HttpRequest* httpRequest);

            /*!
             * デストラクタ
             */
            virtual ~WebServerResponseThread();

            /*!
             * 変数初期化
             */
protected:  virtual void initVariables() {}

            /*!
             * 送信
             */
            void send(const Buffer* content) const;

            /*!
             * not found 送信
             */
            void sendNotFound(HtmlGenerator* generator) const;

            /*!
             * バイナリ送信
             */
            void sendBinary(HtmlGenerator* generator, const slog::CoreString* path) const;

            /*!
             * HTTPヘッダー送信（＆切断）
             */
            void sendHttpHeader(int32_t contentLen) const;

            /*!
             * 応答内容送信＆切断
             */
            void sendContent(const Buffer* content) const;

            /*!
             * 実行
             */
            virtual void run();

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
