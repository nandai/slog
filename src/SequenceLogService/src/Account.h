﻿/*
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

/*!
 *  \brief  アカウントクラス
 */
class Account
{
            DB* mDB;

            /*!
             * ID
             */
public:     int32_t id;

            /*!
             * ユーザー名
             */
            String name;

            /*!
             * パスワード
             */
            String passwd;

            /*!
             * コンストラクタ
             */
public:     Account();

            /*!
             * デストラクタ   
             */
            ~Account();

            /*!
             * 検証
             */
            bool validate();

            /*!
             * アカウント更新可能かどうか
             */
            bool canUpdate() const;

            /*!
             * アカウント更新
             */
            void update() const;

            /*!
             * ハッシュ化パスワード取得
             */
private:    void getHashPassword(CoreString* hashPasswd) const;
};

} // namespace slog
