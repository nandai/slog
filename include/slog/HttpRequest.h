/*
 * Copyright (C) 2011-2015 printf.jp
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
 * \author  Copyright 2011-2015 printf.jp
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
 * httpリクエストリスナークラス
 */
class SLOG_API HttpRequestListener
{
            /*!
             * ヘッダー通知
             */
public:     virtual void onHeader(const CoreString* str) {}
};

/*!
 *  \brief  httpリクエストクラス
 */
class SLOG_API HttpRequest
{
            /*!
             * スキーマ
             */
public:     enum SCHEME
            {
                HTTP,
                HTTPS,
            };

            /*!
             * メソッド種別
             */
            enum METHOD
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
             * スキーマ
             */
            SCHEME mScheme;

            /*!
             * ポート番号
             */
            uint16_t mPort;

            /*!
             * ルートディレクトリ
             */
            String mRootDir;

            /*!
             * メソッド
             */
            METHOD mMethod;

            /*!
             * URL
             */
            String mUrl;

            /*!
             * mime-type
             */
            MimeType mMimeType;

            /*!
             * Cookie
             */
            std::map<String, String> mCookies;

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
             * Accept-Language
             */
            String mAcceptLanguage;

            /*!
             * User-Agent
             */
            String mUserAgent;

            /*!
             * Content-Type
             */
            String mContentType;

            /*!
             * デフォルトリスナー
             */
            HttpRequestListener mDefaultListener;

            /*!
             * リスナー
             */
            HttpRequestListener* mListener;

            /*!
             * コンストラクタ
             */
public:     HttpRequest(SCHEME scheme, Socket* socket, uint16_t port, const CoreString* rootDir);

            /*!
             * デストラクタ
             */
            ~HttpRequest();

            /*!
             * リスナー設定
             */
            void setListener(HttpRequestListener* listener);

            /*!
             * 要求解析
             */
            bool analizeRequest();

            /*!
             * URL解析
             */
private:    int32_t analizeUrl(const char* request, int32_t len, METHOD method);

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
             * スキーマ取得
             */
            SCHEME getScheme() const;

            /*!
             * メソッド取得
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
             * Cookie取得
             */
            const CoreString* getCookie(const char* name, CoreString* str);

            /*!
             * パラメータ取得
             */
            const CoreString* getParam(const char* name, CoreString* str);

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

            /*!
             * Accept-Language取得
             */
            const CoreString* getAcceptLanguage() const;

            /*!
             * User-Agent取得
             */
            const CoreString* getUserAgent() const;

            /*
             * Content-Type取得
             */
            const CoreString* getContentType() const;

            /*!
             * リセット
             */
private:    void reset();
};

} // namespace slog
