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

namespace slog
{
class DB;
class Statement;

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
class AccountLogic
{
            static const char* CLS_NAME;

            /*!
             * アカウント操作結果
             */
public:     enum class Result : int32_t
            {
                OK,
                CANT_CHANGE_USER_NAME,
                ALREADY_USER_EXISTS,
            };

            /*!
             * データベース
             */
private:    DB* mDB;

            /*!
             * コンストラクタ
             */
public:     AccountLogic();

            /*!
             * デストラクタ   
             */
            ~AccountLogic();

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
public:    Result canUpdate(const Account* account) const;

            /*!
             * アカウント更新
             */
            void update(const Account* account) const;

            /*!
             * ハッシュ化パスワード取得
             */
private:    bool getHashPassword(CoreString* hashPasswd, const CoreString* name, const CoreString* passwd, int32_t version) const;
};

} // namespace slog
