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

using namespace std;

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
        redirect("/");
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

    mVariables.add("account",    r.string(R::account));
    mVariables.add("userName",   r.string(R::user_name));
    mVariables.add("password",   r.string(R::password));
    mVariables.add("change",     r.string(R::change));
    mVariables.add("newAccount", r.string(R::new_account));
    mVariables.add("back",       r.string(R::back));
    mVariables.add("admin",      r.string(R::administrator));

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
    accountLogic.getById(&mAccount);

    if (mHttpRequest->getMethod() == HttpRequest::GET)
    {
        // アカウントページ表示
        mVariables.add("userNameValue",      &mAccount.name);
        mVariables.add("userNameProperty",   (mAccount.admin == 1 ? "" : "readonly"));
        mVariables.add("displayAccountList", (mAccount.admin == 1 ? "block" : "none"));

        // アカウントリスト表示
        Json* json = Json::getNewObject();

        if (mAccount.admin == 1)
        {
            list<Account*> accountList;
            accountLogic.getList(&accountList);

            for (auto i = accountList.begin(); i != accountList.end(); i++)
            {
                Account* account = *i;
                Json* jsonAccount = Json::getNewObject();

                String id;
                id.format("%d", account->id);

                jsonAccount->add("id",       &id);
                jsonAccount->add("userName", &account->name);
                jsonAccount->add("admin",    (account->admin ? "Y" : ""));
                json->add(jsonAccount);
            }
        }

        String accountListValue;
        json->serialize(&accountListValue);

        delete json;

        mVariables.add("accountList", &accountListValue);
        return true;
    }

    String phase;
    String id;
    Account changeAccount;

    mHttpRequest->getParam("phase",  &phase);
    mHttpRequest->getParam("id",     &id);
    mHttpRequest->getParam("name",   &changeAccount.name);
    mHttpRequest->getParam("passwd", &changeAccount.passwd);

    changeAccount.id = (id.getLength() == 0
        ? mAccount.id
        : atoi(id.getBuffer()));

    // アカウント変更
    bool res = accountLogic.canUpdate(&changeAccount, &mAccount);

    // 検索結果検証
    if (phase.equals("validate"))
    {
        String result;
        accountLogic.getJSON()->serialize(&result);

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
            if (changeAccount.id == -1)
                accountLogic.insert(&changeAccount);
            else
                accountLogic.update(&changeAccount);

            redirect("/");

            return false;
        }
    }

    return true;
}

} // namespace slog
