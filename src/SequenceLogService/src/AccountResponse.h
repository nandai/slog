﻿/*
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
 *  \file   AccountResponse.h
 *  \brief  アカウント応答クラス
 *  \author Copyright 2014 printf.jp
 */
#pragma once

#include "slog/WebServerResponseThread.h"
#include "Account.h"

namespace slog
{

/*!
 *  \brief  シーケンスログサービスWEBサーバー応答クラス
 */
class AccountResponse : public WebServerResponse
{
            /*!
             * アカウント
             */
            Account mAccount;

            /*!
             * コンストラクタ
             */
public:     AccountResponse(HttpRequest* httpRequest);

            /*!
             * 実行
             */
private:    virtual void run() override;

            /*!
             * 変数初期化
             */
            virtual void initVariables();

            /*!
             * アカウント
             */
            bool account();
};

} // namespace slog
