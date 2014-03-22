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
 * \file    DB_MySQL.h
 * \brief   MySQL
 * \author  Copyright 2014 printf.jp
 */
#pragma once

#include "slog/DB.h"
#include "mysql.h"

namespace slog
{

/*!
 * \brief   MySQL
 */
class MySQL : public slog::DB
{
            MYSQL* mConn;

            /*!
             * コンストラクタ
             */
public:     MySQL() : mConn(nullptr) {}

            /*!
             * デストラクタ
             */
            virtual ~MySQL() override;

            /*!
             * 接続
             */
            virtual void connect(const char* host, const char* user, const char* password, const char* database) throw(slog::Exception) override;

            /*!
             * ステートメント生成
             */
            virtual slog::Statement* newStatement() const throw(slog::Exception) override;

            /*!
             * エラーメッセージ取得
             */
            virtual void getErrorMessage(slog::CoreString* str) const override;

            /*!
             * insert文のauto incrementを取得
             */
            virtual int64_t getInsertID() const override;
};

} // namespace slog
