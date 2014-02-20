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
 * \file    WebServerThread.h
 * \brief   WEBサーバークラス
 * \author  Copyright 2011-2014 printf.jp
 */
#pragma once

#include "slog/Thread.h"
#include "slog/HttpRequest.h"

namespace slog
{
class WebServerResponse;

/*!
 * \brief   WEBサーバークラス
 */
class SLOG_API WebServer :
    public Thread,
    public HttpRequestListener
{
protected:  typedef WebServerResponse* (*WEBCREATEPROC)(HttpRequest* httpRequest);

            struct CREATE
            {
                /*!
                 * URL
                 */
                const char* url;

                /*!
                 * 置換URL
                 */
                const char* replaceUrl;

                /*!
                 * プロシージャ
                 */
                WEBCREATEPROC proc;
            };

            /*!
             * WEBサーバーのポート番号
             */
            uint16_t mPort;

            /*!
             * ルートディレクトリ
             */
private:    String mRootDir;

            /*!
             * 証明書ファイル名
             */
            String mCertificate;

            /*!
             * プライベートキーファイル名
             */
            String mPrivateKey;

            /*!
             * 中間証明書ファイル名
             */
            String mCertificateChain;

            /*!
             * コンストラクタ
             */
public:     WebServer();

            /*!
             * ルートディレクトリ設定
             */
            void setRootDir(const char* rootDir);

            /*!
             * WEBサーバーのポート番号取得
             */
            uint16_t getPort() const;

            /*!
             * WEBサーバーのポート番号設定
             */
            void setPort(uint16_t port);

            /*!
             * SSL関連ファイル設定
             */
            void setSSLFileName(
                const CoreString* certificate,
                const CoreString* privateKey,
                const CoreString* certificateChain = nullptr);

            /*!
             * 公開鍵ファイル名取得
             */
            const CoreString* getCertificateFileName() const {return &mCertificate;}

            /*!
             * 秘密鍵ファイル名取得
             */
            const CoreString* getPrivateKeyFileName() const {return &mPrivateKey;}

            /*!
             * スレッド実行
             */
protected:  virtual void run();
private:    virtual void onResponseStart(WebServerResponse* response) {}

            /*!
             * リクエスト実行
             */
public:     void executeRequest(HttpRequest* httpRequest);

            /*!
             * 
             */
private:    virtual const CREATE* getCreateList() const = 0;

            /*!
             * WEBサーバー応答オブジェクト生成
             */
            WebServerResponse* createResponse(HttpRequest* httpRequest);
};

} // namespace slog
