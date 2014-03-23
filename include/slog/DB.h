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
 * \file    DB.h
 * \brief   データベースクラス
 * \author  Copyright 2014 printf.jp
 */
#pragma once
#include "slog/Exception.h"

namespace slog
{
class CoreString;
class Statement;

/*!
 * \brief   データベースクラス
 */
class DB
{
            /*!
             * デストラクタ
             */
public:     virtual ~DB() {}

            /*!
             * 接続
             */
            virtual void connect(const char* host, const char* user, const char* password, const char* database) throw(Exception) = 0;

            /*!
             * ステートメント生成
             */
            virtual Statement* newStatement() const throw(Exception) = 0;

            /*!
             * エラーメッセージ取得
             */
            virtual void getErrorMessage(CoreString* str) const = 0;

            /*!
             * insert文のauto incrementを取得
             */
            virtual int64_t getInsertID() const = 0;

            /*!
             * クエリー実行
             */
            virtual void query(const char* sql) const throw(Exception) = 0;
};

/*!
 * \brief   データベースステートメントクラス
 */
class SLOG_API Statement
{
            /*!
             * データベース
             */
protected:  const DB* mDB;

            /*!
             * コンストラクタ
             */
protected:  Statement(const DB* db) : mDB(db) {}

            /*!
             * デストラクタ
             */
public:    virtual ~Statement() {}

            /*!
             * 実行準備
             */
            virtual void prepare(const char* sql) throw(Exception) = 0;

            /*!
             * パラメータ設定
             */
            virtual void setStringParam(int32_t index, const CoreString* value) = 0;

            /*!
             * パラメータ設定
             */
            virtual void setParam(int32_t index, int8_t value) = 0;

            /*!
             * パラメータ設定
             */
            virtual void setShortParam(int32_t index, int16_t value) = 0;

            /*!
             * パラメータ設定
             */
            virtual void setIntParam(int32_t index, int32_t value) = 0;

            /*!
             * パラメータ設定
             */
            virtual void setLongParam(int32_t index, int64_t value) = 0;

            /*!
             * 結果設定
             */
            virtual void setStringResult(int32_t index, CoreString* result, int32_t size) const = 0;

            /*!
             * 結果設定
             */
            virtual void setIntResult(int32_t index, int32_t* result) const = 0;

            /*!
             * バインド
             */
            virtual void bind() const throw(Exception) = 0;

            /*!
             * 実行
             */
            virtual void execute() throw(Exception) = 0;

            /*!
             * フェッチ
             */
            virtual bool fetch() const = 0;

            /*!
             * 例外スロー
             */
protected:  void throwException() const throw(Exception);
};

} // namespace slog
