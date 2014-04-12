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

#include "slog/HttpRequest.h"
#include "slog/Json.h"
#include "slog/SequenceLog.h"

#include <stdlib.h>

using namespace std;

namespace slog
{
const char* AccountResponse::CLS_NAME = "AccountResponse";

/*!
 *  \brief  コンストラクタ
 */
AccountResponse::AccountResponse(HttpRequest* httpRequest) : WebServerResponse(httpRequest),
    r(httpRequest->getAcceptLanguage())
{
    mAccountLogic = nullptr;
}

/*!
 * デストラクタ
 */
AccountResponse::~AccountResponse()
{
    delete mAccountLogic;
}

/*!
 * \brief   実行
 */
void AccountResponse::run()
{
    SLOG(CLS_NAME, "run");

    if (getUserId() < 0)
    {
        // 未ログイン
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
    SLOG(CLS_NAME, "initVariables");

    mVariables.add("domain", "printf.jp");
//  mVariables.add("domain", "localhost");

    mVariables.add("account",       r.string(R::account));
    mVariables.add("userName",      r.string(R::user_name));
    mVariables.add("password",      r.string(R::password));
    mVariables.add("change",        r.string(R::change));
    mVariables.add("newAccount",    r.string(R::new_account));
    mVariables.add("delete",        r.string(R::del));
    mVariables.add("back",          r.string(R::back));
    mVariables.add("admin",         r.string(R::administrator));
    mVariables.add("canNotConnect", r.string(R::msg010));

    mVariables.add("userNameMax", Account::NAME_MAX);
    mVariables.add("passwordMax", Account::PASSWD_MAX);
}

/*!
 * \brief   アカウント処理
 */
bool AccountResponse::account()
{
    SLOG(CLS_NAME, "account");
    initMembers();

    if (mHttpRequest->getMethod() == HttpRequest::GET)
    {
        showPage();
        return true;
    }

    // パラメータから情報を取得
    getParams();

    if (mPhase.equals("delete"))
    {
        del();
        return false;
    }

    // 正当性検証
    bool validate = mAccountLogic->validate(&mChangeAccount, &mAccount);

    if (mPhase.equals("validate"))
    {
        sendValidateResult();
        return false;
    }

    if (validate == false)
    {
        // 通常であればまず検証し、問題があればここまで来ることはなく、
        // この段階で検証に問題があるのはおかしい
        mHttpRequest->setUrl("notFound.html");
        return true;
    }

    // アカウント更新
    update();
    return false;
}

/*!
 * \brief   初期処理
 */
void AccountResponse::initMembers()
{
    mAccount.id = getUserId();

    mAccountLogic = new AccountLogic;
    mAccountLogic->setResource(&r);
    mAccountLogic->getById(&mAccount);
}

/*!
 * \brief   パラメータから情報を取得
 */
void AccountResponse::getParams()
{
    String id;

    mHttpRequest->getParam("phase",  &mPhase);
    mHttpRequest->getParam("id",     &id);
    mHttpRequest->getParam("name",   &mChangeAccount.name);
    mHttpRequest->getParam("passwd", &mChangeAccount.passwd);

    mChangeAccount.id = (id.getLength() == 0
        ? mAccount.id
        : atoi(id.getBuffer()));
}

/*!
 * \brief   画面表示
 */
void AccountResponse::showPage()
{
    mVariables.add("userNameValue",      &mAccount.name);
    mVariables.add("userNameProperty",   (mAccount.admin == 1 ? ""       : "readonly"));
    mVariables.add("displayNewAccount",  (mAccount.admin == 1 ? "inline" : "none"));
    mVariables.add("displayAccountList", (mAccount.admin == 1 ? "block"  : "none"));

    // JSONアカウントリスト作成
    Json* json = Json::getNewObject();

    if (mAccount.admin == 1)
    {
        list<Account*> accountList;
        mAccountLogic->getList(&accountList);

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
}

/*!
 * \brief   検証結果送信
 */
void AccountResponse::sendValidateResult() const
{
    String result;
    mAccountLogic->getJSON()->serialize(&result);

//  SMSG(slog::DEBUG, result.getBuffer());
    send(nullptr, &result);
}

/*!
 * \brief   アカウント更新
 */
void AccountResponse::update() const
{
    if (mChangeAccount.id == -1)
        mAccountLogic->insert(&mChangeAccount);
    else
        mAccountLogic->update(&mChangeAccount);

    redirect("/");
}

/*!
 * \brief   アカウント削除
 */
void AccountResponse::del() const
{
    if (mAccountLogic->validateDelete(&mChangeAccount, &mAccount))
        mAccountLogic->del(&mChangeAccount);

    String result;
    mAccountLogic->getJSON()->serialize(&result);

    send(nullptr, &result);
}

} // namespace slog
