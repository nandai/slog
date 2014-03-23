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
#pragma execution_character_set("utf-8")

#include "SequenceLogServiceWebServerResponse.h"

#include "slog/HttpRequest.h"
#include "slog/Json.h"

namespace slog
{

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
    mAccountLogic.setJapanese(mHttpRequest->getAcceptLanguage()->indexOf("ja") == 0);

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

            if (mHttpRequest->getUrl()->equals("logout"))
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
    mVariables.add("domain", "printf.jp");
//  mVariables.add("domain", "localhost");

    if (mHttpRequest->getAcceptLanguage()->indexOf("ja") == 0)
    {
        mVariables.add("login",       "ログイン");
        mVariables.add("userName",    "ユーザー名");
        mVariables.add("password",    "パスワード");

        mVariables.add("account",     "アカウント");
        mVariables.add("logout",      "ログアウト");
        mVariables.add("startTime",   "開始日時");
        mVariables.add("endTime",     "終了日時");
        mVariables.add("logFileName", "ログファイル名");
        mVariables.add("logFileSize", "サイズ");
    }
    else
    {
        mVariables.add("login",       "Login");
        mVariables.add("userName",    "User name");
        mVariables.add("password",    "Password");

        mVariables.add("account",     "Account");
        mVariables.add("logout",      "Logout");
        mVariables.add("startTime",   "Start time");
        mVariables.add("endTime",     "End time");
        mVariables.add("logFileName", "Log file name");
        mVariables.add("logFileSize", "Size");
    }
}

/*!
 * \brief   ログイン
 */
bool SequenceLogServiceWebServerResponse::login()
{
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
    String result;

    if (pass == false)
    {
        String message;

        if (mHttpRequest->getAcceptLanguage()->indexOf("ja") == 0)
            message.copy("ユーザー名、またはパスワードが正しくありません。");
        else
            message.copy("The username or password you entered is incorrect.");

        Json* json = Json::getNewObject();
        json->add("", &message);
        json->serialize(&result);

        delete json;
    }

    if (phase.equals("validate"))
    {
        if (result.getLength() == 0)
            result.copy("{}");

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
    removeSession();

    String redirectUrl = "/";
    redirect(&redirectUrl);
}

} // namespace slog
