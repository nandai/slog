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
 *  \file   GetSequenceLogListResponse.h
 *  \brief  シーケンスログリスト（JSON）送信クラス
 *  \author Copyright 2013 printf.jp
 */
#pragma once
#include "slog/WebServerResponseThread.h"

namespace slog
{

/*!
 *  \brief  シーケンスログリスト（JSON）送信クラス
 */
class GetSequenceLogListResponse : public WebServerResponseThread
{
public:     GetSequenceLogListResponse(HttpRequest* httpRequest) : WebServerResponseThread(httpRequest) {}
private:    virtual void run();
};

} // namespace slog
