/*
 * Copyright (C) 2013-2014 printf.jp
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
 *  \file   LoginResponse.cpp
 *  \brief  ログイン応答クラス
 *  \author Copyright 2013-2014 printf.jp
 */
#include "LoginResponse.h"
#include "R.h"

#include "slog/HttpRequest.h"
#include "slog/Json.h"
#include "slog/SequenceLog.h"

namespace slog
{
const char* LoginResponse::CLS_NAME = "LoginResponse";

/*!
 *  \brief  コンストラクタ
 */
LoginResponse::LoginResponse(HttpRequest* httpRequest) :
    WebServerResponse(httpRequest)
{
}

/*!
 * \brief   実行
 */
void LoginResponse::run()
{
    SLOG(CLS_NAME, "run");

    R r(mHttpRequest->getAcceptLanguage());
    mAccountLogic.setResource(&r);

    if (login() == false)
        return;

    WebServerResponse::run();
}

/*!
 * \brief  変数初期化
 */
void LoginResponse::initVariables()
{
    R r(mHttpRequest->getAcceptLanguage());

    mVariables.add("domain",        "printf.jp");
//  mVariables.add("domain",        "localhost");

    mVariables.add("login",         r.string(R::login));
    mVariables.add("userName",      r.string(R::user_name));
    mVariables.add("password",      r.string(R::password));
    mVariables.add("canNotConnect", r.string(R::msg010));
}

/*!
 * \brief   ログイン
 */
bool LoginResponse::login()
{
    SLOG(CLS_NAME, "login");

    if (mHttpRequest->getMethod() == HttpRequest::GET)
    {
        // ログインページ表示
        mHttpRequest->setUrl("login.html");
        return true;
    }

    // ログイン認証
    String phase;
    mHttpRequest->getParam("phase", &phase);

    mHttpRequest->getParam("name",   &mAccount.name);
    mHttpRequest->getParam("passwd", &mAccount.passwd);

    // ユーザー検証
    bool pass = mAccountLogic.getByNamePassword(&mAccount);

    // 検索結果検証
    if (phase.equals("validate"))
    {
        Json* json = Json::getNewObject();

        if (pass == false)
        {
            R r(mHttpRequest->getAcceptLanguage());
            const char* message = r.string(R::msg001);

            json->add("", message);
        }

        String result;
        json->serialize(&result);

        delete json;

        send(nullptr, &result);
        return false;
    }

    if (pass == false)
    {
        // 通常であればまず検証し、問題なければログインとなるはずで、
        // この段階で検証に問題があるのはおかしいためnotFoundを返す
        mHttpRequest->setUrl("notFound.html");
        return true;
    }

    generateSession(mAccount.id);
    redirect("/");
    return false;
}

} // namespace slog
