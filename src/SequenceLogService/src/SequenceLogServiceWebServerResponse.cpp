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
 * \brief  変数初期化
 */
void SequenceLogServiceWebServerResponse::initVariables()
{
    mVariables.add("domain", "printf.jp");
//  mVariables.add("domain", "localhost");

    if (mHttpRequest->getAcceptLanguage()->indexOf("ja") == 0)
    {
        mVariables.add("startTime",   "開始日時");
        mVariables.add("endTime",     "終了日時");
        mVariables.add("logFileName", "ログファイル名");
        mVariables.add("logFileSize", "サイズ");
    }
    else
    {
        mVariables.add("startTime",   "Start time");
        mVariables.add("endTime",     "End time");
        mVariables.add("logFileName", "Log file name");
        mVariables.add("logFileSize", "Size");
    }
}

} // namespace slog
