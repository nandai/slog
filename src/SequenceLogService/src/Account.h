/*
 * Copyright (C) 2014 printf.jp
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
 *  \file   Account.h
 *  \brief  アカウントクラス
 *  \author Copyright 2014 printf.jp
 */
#pragma once

#include "slog/String.h"
#include "slog/Validate.h"

#undef NAME_MAX

namespace slog
{
class DB;
class Statement;
class Json;
class R;

/*!
 *  \brief  アカウントクラス
 */
class Account
{
public:     static const int32_t NAME_MIN;
            static const int32_t NAME_MAX;
            static const int32_t PASSWD_MIN;
            static const int32_t PASSWD_MAX;
            static const int32_t MAIL_ADDR_MAX;

            /*!
             * ID
             */
            int32_t id;

            /*!
             * ユーザー名
             */
            String name;

            /*!
             * パスワード
             */
            String passwd;

            /*!
             * メールアドレス
             */
            String mailAddr;

            /*!
             * ハッシュバージョン
             */
            int32_t version;

            /*!
             * 管理者フラグ
             */
            int32_t admin;

            /*!
             * コンストラクタ
             */
public:     Account();
};

/*!
 *  \brief  アカウントロジッククラス
 */
class AccountLogic : public slog::ValidateListener
{
            static const char* CLS_NAME;

            /*!
             * データベース
             */
            DB* mDB;

            /*!
             * アカウント
             */
            const Account* mAccount;

            /*!
             * JSON
             */
            slog::Json* mJson;

            /*!
             * リソース
             */
            const R* r;

            /*!
             * コンストラクタ
             */
public:     AccountLogic();

            /*!
             * デストラクタ   
             */
            ~AccountLogic();

            /*!
             * リソース設定
             */
            void setResource(const R* resource) {r = resource;}

            /*!
             * JSON取得
             */
            const slog::Json* getJSON() const {return mJson;}

            /*!
             * ユーザー名とパスワードでアカウントを取得する
             */
            bool getByNamePassword(Account* account) const;

            /*!
             * ユーザーIDでアカウントを取得する
             */
            bool getById(Account* account) const;

            /*!
             * SQL準備
             */
private:   void prepare(Statement* stmt, Account* account, const char* where) const;

            /*!
             * アカウント更新可能かどうか
             */
public:    bool canUpdate(const Account* account);

            /*!
             * 検証失敗イベント
             */
private:    virtual void onInvalid(const void* value, const slog::Validate::Result* result) override;

            /*!
             * アカウント更新
             */
public:     void update(const Account* account) const;

            /*!
             * ハッシュ化パスワード取得
             */
private:    bool getHashPassword(CoreString* hashPasswd, const CoreString* name, const CoreString* passwd, int32_t version) const;
};

} // namespace slog
