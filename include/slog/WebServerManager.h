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
 *  \file   WebServerManager.h
 *  \brief  WEBサーバーマネージャークラス
 *  \author Copyright 2014 printf.jp
 */
#pragma once
#include "slog/slog.h"

namespace slog
{
class WebServer;

/*!
 *  \brief  WEBサーバーマネージャークラス
 */
class SLOG_API WebServerManager
{
            /*!
             * WEBサーバー (http)
             */
            WebServer* mWebServer;

            /*!
             * WEBサーバー (https)
             */
            WebServer* mSecureWebServer;

            /*!
             * コンストラクタ
             */
public:     WebServerManager();

            /*!
             * デストラクタ
             */
            ~WebServerManager();

            /*!
             * WEBサーバー取得
             */
            WebServer* getWebServer(bool secure) const;

            /*!
             * WEBサーバー設定
             */
            void setWebServer(WebServer* webServer, bool secure);

            /*!
             * 開始
             */
            void start();

            /*!
             * 停止
             */
            void stop();
};

} // namespace slog
