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
#include "SequenceLogServiceMain.h"
#include "R.h"

#include "slog/HttpRequest.h"
#include "slog/Json.h"
#include "slog/Mutex.h"
#include "slog/FileInfo.h"
#include "slog/File.h"
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
            // 未ログイン
            redirect("/login.html");
            return;
        }

        mAccount.id = getUserId();
        mAccountLogic.getById(&mAccount);
        mVariables.add("userNameValue", &mAccount.name);

        const CoreString* url = mHttpRequest->getUrl();
//      if (url->equals("logout"))
        if (url->equals("logoff"))
        {
            logout();
            return;
        }

        String logsPath = "logs/";
        if (url->indexOf(logsPath.getBuffer()) == 0)
        {
            SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
            ScopedLock lock(serviceMain->getMutex());

            auto sum = serviceMain->getFileInfoArray(getUserId());

            for (auto i = sum->begin(); i != sum->end(); i++)
            {
                const FileInfo* fileInfo = *i;
                const CoreString* canonicalPath = fileInfo->getCanonicalPath();

                if (canonicalPath->lastIndexOf(url->getBuffer() + logsPath.getLength()) > 0)
                {
                    MimeType* mimeType = (MimeType*)mHttpRequest->getMimeType();
                    String ext = ".exe";
                    mimeType->analize(&ext);
                    sendBinary(nullptr, canonicalPath);
                    return;
                }
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

    mVariables.add("logFileList",   r.string(R::logFileList));
    mVariables.add("account",       r.string(R::account));
    mVariables.add("logout",        r.string(R::logout));

    mVariables.add("startTime",     r.string(R::start_time));
    mVariables.add("endTime",       r.string(R::end_time));
    mVariables.add("logFileName",   r.string(R::log_file_name));
    mVariables.add("fileTransfer",  r.string(R::file_transfer));
    mVariables.add("logFileSize",   r.string(R::log_file_size));
}

/*!
 * \brief   ログアウト
 */
void SequenceLogServiceWebServerResponse::logout()
{
    removeSession(mAccount.id);
    redirect("/login.html");
}

} // namespace slog
