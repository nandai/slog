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
 *  \file   GetLogResponse.h
 *  \brief  取得ログ送信クラス
 *  \author Copyright 2013-2014 printf.jp
 */
#pragma once

#include "slog/WebServerResponseThread.h"
#include "SequenceLogServiceMain.h"

namespace slog
{

/*!
 *  \brief  取得ログ送信クラス
 */
class GetLogResponse :
    public WebServerResponse,
    public SequenceLogServiceThreadListener
{
            /*!
             * コンストラクタ
             */
public:     GetLogResponse(HttpRequest* httpRequest) : WebServerResponse(httpRequest) {}

            /*!
             * デストラクタ
             */
            virtual~GetLogResponse() override;

            /*!
             * 実行
             */
private:    virtual void run() override;

public:     virtual void onInitialized(   Thread* thread) override {}
            virtual void onTerminated(    Thread* thread) override;
            virtual void onLogFileChanged(Thread* thread) override;
            virtual void onUpdateLog(const Buffer* text)  override;

            void send(const char* commandNo, const Buffer* payloadData);
};

} // namespace slog
