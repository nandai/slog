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
 * \file    AccountResponse.cpp
 * \brief   アカウント応答クラス
 * \author  Copyright 2014 printf.jp
 */
#include "AccountResponse.h"
#include "R.h"

#include "slog/HttpRequest.h"
#include "slog/Json.h"
#include "slog/SequenceLog.h"

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

    R r(mHttpRequest->getAcceptLanguage());

    mVariables.add("account",  r.string(R::account));
    mVariables.add("userName", r.string(R::user_name));
    mVariables.add("password", r.string(R::password));
    mVariables.add("change",   r.string(R::change));

    mVariables.add("userNameMax", Account::NAME_MAX);
    mVariables.add("passwordMax", Account::PASSWD_MAX);
}

/*!
 * \brief   アカウント
 */
bool AccountResponse::account()
{
    SLOG("AccountResponse", "account");
    mAccount.id = getUserId();

    R r(mHttpRequest->getAcceptLanguage());
    AccountLogic accountLogic;
    accountLogic.setResource(&r);

    if (mHttpRequest->getMethod() == HttpRequest::GET)
    {
        // アカウントページ表示
        accountLogic.getById(&mAccount);
        mVariables.add("userNameValue",    &mAccount.name);
        mVariables.add("userNameProperty", (mAccount.admin == 1 ? "" : "readonly"));
        return true;
    }

    String phase;
    mHttpRequest->getParam("phase",  &phase);
    mHttpRequest->getParam("name",   &mAccount.name);
    mHttpRequest->getParam("passwd", &mAccount.passwd);

    // アカウント変更
    bool res = accountLogic.canUpdate(&mAccount);

    // 検索結果検証
    if (phase.equals("validate"))
    {
        String result;
        accountLogic.getJSON()->serialize(&result);

        if (result.getLength() == 0)
            result.copy("{}");

//      SMSG(slog::DEBUG, result.getBuffer());
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
            accountLogic.update(&mAccount);

            String redirectUrl = "/";
            redirect(&redirectUrl);
            return false;
        }
    }

    return true;
}

} // namespace slog
