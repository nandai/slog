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
 *  \file   AccountResponse.cpp
 *  \brief  アカウント応答クラス
 *  \author Copyright 2014 printf.jp
 */
#pragma execution_character_set("utf-8")

#include "AccountResponse.h"
#include "Account.h"

#include "slog/HttpRequest.h"
#include "slog/Json.h"

namespace slog
{

/*!
 *  \brief  コンストラクタ
 */
AccountResponse::AccountResponse(HttpRequest* httpRequest) :
    WebServerResponse(httpRequest)
{
}

/*!
 * \brief   実行
 */
void AccountResponse::run()
{
    if (getUserId() < 0)
    {
        String redirectUrl = "/";
        redirect(&redirectUrl);
        return;
    }

    if (account() == false)
        return;

    WebServerResponse::run();
}

/*!
 * \brief  変数初期化
 */
void AccountResponse::initVariables()
{
    mVariables.add("domain", "printf.jp");
//  mVariables.add("domain", "localhost");

    if (mHttpRequest->getAcceptLanguage()->indexOf("ja") == 0)
    {
        mVariables.add("account",     "アカウント");
        mVariables.add("userName",    "ユーザー名");
        mVariables.add("password",    "パスワード");
        mVariables.add("change",      "変更");
    }
    else
    {
        mVariables.add("account",     "Account");
        mVariables.add("userName",    "User name");
        mVariables.add("password",    "Password");
        mVariables.add("change",      "Change");
    }

    mVariables.add("userNameMax", Account::NAME_MAX);
    mVariables.add("passwordMax", Account::PASSWD_MAX);
}

/*!
 * \brief   アカウント
 */
bool AccountResponse::account()
{
    Account account;
    account.id = getUserId();

    AccountLogic accountLogic;
    accountLogic.setJapanese(mHttpRequest->getAcceptLanguage()->indexOf("ja") == 0);

    if (mHttpRequest->getMethod() == HttpRequest::GET)
    {
        // アカウントページ表示
        accountLogic.getById(&account);
        mVariables.add("userNameValue",    &account.name);
        mVariables.add("userNameProperty", (account.admin == 1 ? "" : "readonly"));
        return true;
    }

    String phase;
    mHttpRequest->getParam("phase",  &phase);
    mHttpRequest->getParam("name",   &account.name);
    mHttpRequest->getParam("passwd", &account.passwd);

    // アカウント変更
    bool res = accountLogic.canUpdate(&account);

    // 検索結果検証
    if (phase.equals("validate"))
    {
        String result;
        accountLogic.getJSON()->serialize(&result);

        if (result.getLength() == 0)
            result.copy("{}");

        send(nullptr, &result);
        return false;
    }
    else
    {
        if (res == false)
        {
            // 通常であればまず検証し、問題なければログインとなるはずで、
            // この段階で検証に問題があるのはおかしいためnotFoundを返す
            mHttpRequest->setUrl("notFound.html");
        }
        else
        {
            accountLogic.update(&account);

            String redirectUrl = "/";
            redirect(&redirectUrl);
            return false;
        }
    }

    return true;
}

} // namespace slog
