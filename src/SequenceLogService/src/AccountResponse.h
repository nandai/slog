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
 * \file    AccountResponse.h
 * \brief   アカウント応答クラス
 * \author  Copyright 2014 printf.jp
 */
#pragma once

#include "slog/WebServerResponseThread.h"

#include "R.h"
#include "Account.h"

namespace slog
{

/*!
 *  \brief  シーケンスログサービスWEBサーバー応答クラス
 */
class AccountResponse : public WebServerResponse
{
            static const char* CLS_NAME;

            /*!
             * リソース
             */
            R r;

            /*!
             * アカウントロジック
             */
            AccountLogic* mAccountLogic;

            /*!
             * アカウント
             */
            Account mAccount;

            /*!
             * 変更対象アカウント
             */
            Account mChangeAccount;

            /*!
             * 実行フェーズ
             */
            String mPhase;

            /*!
             * コンストラクタ
             */
public:     AccountResponse(HttpRequest* httpRequest);

            /*!
             * デストラクタ
             */
            virtual ~AccountResponse() override;

            /*!
             * 実行
             */
private:    virtual void run() override;

            /*!
             * 変数初期化
             */
            virtual void initVariables() override;

            /*!
             * アカウント処理
             */
            bool account();

            /*!
             * 初期処理
             */
            void initMembers();

            /*!
             * パラメータから情報を取得
             */
            void getParams();

            /*!
             * 画面表示
             */
            void showPage();

            /*!
             * 検証結果送信
             */
            void sendValidateResult() const;

            /*!
             * アカウント更新
             */
            void update() const;

            /*!
             * アカウント削除
             */
            void del() const;
};

} // namespace slog
