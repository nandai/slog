/*
 * Copyright (C) 2011-2014 printf.jp
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
 *  \file   HttpResponse.h
 *  \brief  httpレスポンスクラス
 *  \author Copyright 2011-2014 printf.jp
 */
#pragma once
#include "slog/String.h"

namespace slog
{
class Socket;

/*!
 *  \brief  httpレスポンスクラス
 */
class SLOG_API HttpResponse
{
            /*!
             * ソケット
             */
            Socket* mSocket;

            /*!
             * レスポンス
             */
            String mResponse;

            /*!
             * コンストラクタ
             */
public:     HttpResponse(Socket* socket);

            /*!
             * 解析
             */
public:     bool analizeResponse();

            /*!
             * レスポンス取得
             */
            const CoreString* getResponse() const {return &mResponse;}
};

} // namespace slog
