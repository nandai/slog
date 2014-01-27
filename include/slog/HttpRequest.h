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
 * \file    HttpRequest.h
 * \brief   httpリクエストクラス
 * \author  Copyright 2011-2014 printf.jp
 */
#pragma once

#include "slog/String.h"
#include "slog/MimeType.h"

#include <map>

#pragma warning(disable:4251)

namespace slog
{
class Socket;
class ByteBuffer;

/*!
 *  \brief  httpリクエストクラス
 */
class SLOG_API HttpRequest
{
            /*!
             * メソッド種別
             */
public:     enum METHOD
            {
                UNKNOWN,
                GET,
                POST,
            };

            /*!
             * ソケット
             */
private:    Socket* mSocket;

            /*!
             * ポート番号
             */
            uint16_t mPort;

            /*!
             * ルートディレクトリ
             */
            String mRootDir;

            /*!
             * 要求メソッド
             */
            METHOD mMethod;

            /*!
             * 要求URL
             */
            String mUrl;

            /*!
             * mime-type
             */
            MimeType mMimeType;

            /*!
             * パラメータ
             */
            std::map<String, String> mParams;

            /*!
             * Ajaxかどうか
             */
            bool mAjax;

            /*!
             * Sec-WebSocket-Key
             */
            String mWebSocketKey;

            /*!
             * ユーザー
             */
            String mUser;

            /*!
             * パスワード
             */
            String mPassword;

            /*!
             * コンストラクタ
             */
public:     HttpRequest(Socket* socket, uint16_t port, const CoreString* rootDir);

            /*!
             * デストラクタ
             */
            ~HttpRequest();

            /*!
             * 要求解析
             */
            bool analizeRequest();

            /*!
             * URL解析
             */
private:    int32_t analizeUrl(const char* request, int32_t len, METHOD method);

            /*!
             * パラメータ解析
             */
            void analizeParams(const char* buffer, int32_t len);

            /*!
             * ソケット取得
             */
public:     Socket* getSocket() const;

            /*!
             * ポート番号取得
             */
            uint16_t getPort() const;

            /*!
             * ルートディレクトリ取得
             */
            const CoreString* getRootDir() const;

            /*!
             * HTTPメソッド取得
             */
            METHOD getMethod() const;

            /*!
             * URL取得
             */
            const CoreString* getUrl() const;

            /*!
             * URL設定
             */
            void setUrl(const char* url);

            /*!
             * ファイルパス取得
             */
            void getPath(CoreString* path);

            /*!
             * mime-type取得
             */
            const MimeType* getMimeType();

            /*!
             * パラメータ取得
             */
            const CoreString* getParam(const char* name, CoreString* param);

            /*!
             * Ajaxかどうか調べる
             */
            bool isAjax() const;

            /*!
             * Sec-WebSocket-Key取得
             */
            const CoreString* getWebSocketKey() const;

            /*!
             * ユーザー取得
             */
            const CoreString* getUser() const;

            /*!
             * パスワード取得
             */
            const CoreString* getPassword() const;
};

} // namespace slog
