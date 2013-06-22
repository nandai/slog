/*
 * Copyright (C) 2011-2013 printf.jp
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
 *  \file   WebServerThread.h
 *  \brief  WEBサーバースレッドクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once

#include "slog/Thread.h"
#include "slog/HttpRequest.h"

namespace slog
{
class WebServerResponseThread;

/*!
 *  \brief  WEBサーバースレッドクラス
 */
class SLOG_API WebServerThread : public Thread
{
protected:  typedef WebServerResponseThread* (*WEBCREATEPROC)(HttpRequest* httpRequest);
            struct CREATE
            {
                HttpRequest::METHOD method;         // メソッド
                const char*         url;            // URL
                const char*         replaceUrl;     // 置換URL
                WEBCREATEPROC       proc;           // プロシージャ
            };

            uint16_t    mPort;          // WEBサーバーのポート番号
private:    String      mCertificate;   // 証明書ファイル名
            String      mPrivateKey;    // プライベートキーファイル名

            /*!
             * コンストラクタ
             */
public:     WebServerThread();

            /*!
             * WEBサーバーのポート番号
             */
            uint16_t getPort() const;
            void     setPort(uint16_t port);

            /*!
             * SSL関連
             */
            void setSSLFileName(const CoreString& certificate, const CoreString& privateKey);

            /*!
             * スレッド実行
             */
protected:  virtual void run();
public:     virtual void onResponseStart(WebServerResponseThread* response) {}

            /*!
             * 応答スレッド関連
             */
private:    virtual const CREATE* getCreateList() const = 0;
public:     WebServerResponseThread* createResponse(HttpRequest* httpRequest);
};

} // namespace slog
