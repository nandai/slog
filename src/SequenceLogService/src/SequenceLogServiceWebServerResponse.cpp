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
 *  \file   SequenceLogServiceWebServerResponse.cpp
 *  \brief  シーケンスログサービスWEBサーバー応答クラス
 *  \author Copyright 2013-2014 printf.jp
 */
#include "SequenceLogServiceWebServerResponse.h"
#include "R.h"

#include "slog/HttpRequest.h"
#include "slog/Json.h"
#include "slog/SequenceLog.h"

namespace slog
{
const char* SequenceLogServiceWebServerResponse::CLS_NAME = "SequenceLogServiceWebServerResponse";

/*!
 *  \brief  コンストラクタ
 */
SequenceLogServiceWebServerResponse::SequenceLogServiceWebServerResponse(HttpRequest* httpRequest) :
    WebServerResponse(httpRequest)
{
}

/*!
 * \brief   実行
 */
void SequenceLogServiceWebServerResponse::run()
{
    SLOG(CLS_NAME, "run");

    R r(mHttpRequest->getAcceptLanguage());
    mAccountLogic.setResource(&r);

    if (mHttpRequest->getMimeType()->type == MimeType::Type::HTML ||
        mHttpRequest->getMimeType()->type == MimeType::Type::JSON)
    {
        if (getUserId() < 0)
        {
            if (login() == false)
                return;
        }
        else
        {
            mAccount.id = getUserId();
            mAccountLogic.getById(&mAccount);
            mVariables.add("userNameValue", &mAccount.name);

//          if (mHttpRequest->getUrl()->equals("logout"))
            if (mHttpRequest->getUrl()->equals("logoff"))
            {
                logout();
                return;
            }
        }
    }

    WebServerResponse::run();
}

/*!
 * \brief  変数初期化
 */
void SequenceLogServiceWebServerResponse::initVariables()
{
    R r(mHttpRequest->getAcceptLanguage());

    mVariables.add("domain",        "printf.jp");
//  mVariables.add("domain",        "localhost");

    mVariables.add("login",         r.string(R::login));
    mVariables.add("userName",      r.string(R::user_name));
    mVariables.add("password",      r.string(R::password));
    mVariables.add("canNotConnect", r.string(R::msg010));

    mVariables.add("account",       r.string(R::account));
    mVariables.add("logout",        r.string(R::logout));

    mVariables.add("startTime",     r.string(R::start_time));
    mVariables.add("endTime",       r.string(R::end_time));
    mVariables.add("logFileName",   r.string(R::log_file_name));
    mVariables.add("logFileSize",   r.string(R::log_file_size));
}

/*!
 * \brief   ログイン
 */
bool SequenceLogServiceWebServerResponse::login()
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
    else
    {
        if (pass == false)
        {
            // 通常であればまず検証し、問題なければログインとなるはずで、
            // この段階で検証に問題があるのはおかしいためnotFoundを返す
            mHttpRequest->setUrl("notFound.html");
        }
        else
        {
            generateSession(mAccount.id);
            mVariables.add("userNameValue", &mAccount.name);
        }
    }

    return true;
}

/*!
 * \brief   ログアウト
 */
void SequenceLogServiceWebServerResponse::logout()
{
    removeSession(mAccount.id);
    redirect("/");
}

} // namespace slog
