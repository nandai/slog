/*
 * Copyright (C) 2013 printf.jp
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
 *  \author Copyright 2013 printf.jp
 */
#include "SequenceLogServiceWebServerResponse.h"

namespace slog
{

/*!
 *  \brief  コンストラクタ
 */
SequenceLogServiceWebServerResponse::SequenceLogServiceWebServerResponse(HttpRequest* httpRequest) :
    WebServerResponseThread(httpRequest)
{
}

/*!
 *  \brief  ドメイン取得
 */
const char* SequenceLogServiceWebServerResponse::getDomain() const
{
    static const char* domain = "printf.jp";
    return domain;
}

/*!
 *  \brief  ルートディレクトリ取得
 */
const char* SequenceLogServiceWebServerResponse::getRootDir() const
{
#if !defined(__ANDROID__)
    static const char* rootDir = "SequenceLogServiceWeb";
#else
    static const char* rootDir = "";
#endif

    return rootDir;
}

/*!
 * \brief  変数初期化
 */
void SequenceLogServiceWebServerResponse::initVariables()
{
    mVariables.push_back(new Variable("domain", "printf.jp"));
}

} // namespace slog
