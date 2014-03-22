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
 * \file    DB_MySQL.cpp
 * \brief   MySQL
 * \author  Copyright 2014 printf.jp
 */
#pragma execution_character_set("utf-8")

#include "DB_MySQL.h"
#include "slog/String.h"
#include "slog/SequenceLog.h"

#include <string.h>

namespace slog
{

/*!
 * \brief   MySQLステートメントクラス
 */
class MySQLStatement : public Statement
{
            static const char* CLS_NAME;

            /*!
             * パラメータ
             */
            union NumberValue
            {
                int8_t  value8;
                int16_t value16;
                int32_t value32;
                int64_t value64;
            };

            /*!
             * ステートメント
             */
            MYSQL_STMT* mStmt;

            /*!
             * パラメータ数
             */
            int32_t mParamCount;

            /*!
             * パラメータ
             */
            MYSQL_BIND* mParam;

            /*!
             * パラメータリスト
             */
            NumberValue* mParamList;

            /*!
             * 結果数
             */
            int32_t mResultCount;

            /*!
             * 結果
             */
            MYSQL_BIND* mResult;

            /*!
             * コンストラクタ
             */
public:     MySQLStatement(const DB* db, MYSQL_STMT* stmt);

            /*!
             * デストラクタ
             */
            virtual ~MySQLStatement() override;

            /*!
             * 実行準備
             */
            virtual void prepare(const char* sql) throw(Exception) override;

            /*!
             * パラメータ設定
             */
            virtual void setStringParam(int32_t index, const CoreString* value) override;

            /*!
             * パラメータ設定
             */
            virtual void setParam(int32_t index, int8_t value) override;

            /*!
             * パラメータ設定
             */
            virtual void setShortParam(int32_t index, int16_t value) override;

            /*!
             * パラメータ設定
             */
            virtual void setIntParam(int32_t index, int32_t value) override;

            /*!
             * パラメータ設定
             */
            virtual void setLongParam(int32_t index, int64_t value) override;

            /*!
             * 結果設定
             */
            virtual void setStringResult(int32_t index, CoreString* result, int32_t size) const override;

            /*!
             * 結果設定
             */
            virtual void setIntResult(int32_t index, int32_t* result) const override;

            /*!
             * バインド
             */
            virtual void bind() const throw(Exception) override;

            /*!
             * 実行
             */
            virtual void execute() throw(Exception) override;

            /*!
             * フェッチ
             */
            virtual bool fetch() const;
};

const char* MySQLStatement::CLS_NAME = "MySQLStatement";

/*!
 * \brief   コンストラクタ
 */
MySQLStatement::MySQLStatement(const DB* db, MYSQL_STMT* stmt) : Statement(db)
{
    mStmt = stmt;

    mParamCount = 0;
    mParam = nullptr;
    mParamList = nullptr;

    mResultCount = 0;
    mResult = nullptr;
}

/*!
 * \brief   デストラクタ
 */
MySQLStatement::~MySQLStatement()
{
    if (mStmt)
    {
        mysql_stmt_free_result(mStmt);
        mysql_stmt_close(mStmt);
    }

    delete [] mParam;
    delete [] mParamList;
    delete [] mResult;
}

/*!
 * \brief   実行準備
 */
void MySQLStatement::prepare(const char* sql) throw(Exception)
{
    SLOG(CLS_NAME, "prepare");
    SMSG(slog::DEBUG, "%s", sql);

    if (mysql_stmt_prepare(mStmt, sql, strlen(sql)) != 0)
        throwException();

    // パラメータ準備
    mParamCount = mysql_stmt_param_count(mStmt);
    SMSG(slog::DEBUG, "mParamCount: %d", mParamCount);

    if (0 < mParamCount)
    {
        mParam =     new MYSQL_BIND[ mParamCount];
        mParamList = new NumberValue[mParamCount];

        for (int32_t i = 0; i < mParamCount; i++)
            memset(&mParam[i], 0, sizeof(MYSQL_BIND));
    }

    // 結果準備
    MYSQL_RES *res = mysql_stmt_result_metadata(mStmt);

    if (res)
    {
        mResultCount = mysql_num_fields(res);
        mResult = new MYSQL_BIND[mResultCount];
        SMSG(slog::DEBUG, "mResultCount: %d", mResultCount);

        for (int32_t i = 0; i < mResultCount; i++)
            memset(&mResult[i], 0, sizeof(MYSQL_BIND));

        mysql_free_result(res);
    }
}

/*!
 * \brief   パラメータ設定
 */
void MySQLStatement::setStringParam(int32_t index, const CoreString* value)
{
    if (index < 0 || mParamCount <= index)
        return;

    MYSQL_BIND* bind = &mParam[index];
    bind->buffer_type = MYSQL_TYPE_VAR_STRING;
    bind->buffer =        value->getBuffer();
    bind->buffer_length = value->getLength();
    bind->is_null = 0;
}

/*!
 * \brief   パラメータ設定
 */
void MySQLStatement::setParam(int32_t index, int8_t value)
{
    if (index < 0 || mParamCount <= index)
        return;

    mParamList[index].value8 = value;

    MYSQL_BIND* bind = &mParam[index];
    bind->buffer_type = MYSQL_TYPE_TINY;
    bind->buffer = &mParamList[index].value8;
    bind->is_null = 0;
}

/*!
 * \brief   パラメータ設定
 */
void MySQLStatement::setShortParam(int32_t index, int16_t value)
{
    if (index < 0 || mParamCount <= index)
        return;

    mParamList[index].value16 = value;

    MYSQL_BIND* bind = &mParam[index];
    bind->buffer_type = MYSQL_TYPE_SHORT;
    bind->buffer = &mParamList[index].value16;
    bind->is_null = 0;
}

/*!
 * \brief   パラメータ設定
 */
void MySQLStatement::setIntParam(int32_t index, int32_t value)
{
    if (index < 0 || mParamCount <= index)
        return;

    mParamList[index].value16 = value;

    MYSQL_BIND* bind = &mParam[index];
    bind->buffer_type = MYSQL_TYPE_LONG;
    bind->buffer = &mParamList[index].value32;
    bind->is_null = 0;
}

/*!
 * \brief   パラメータ設定
 */
void MySQLStatement::setLongParam(int32_t index, int64_t value)
{
    if (index < 0 || mParamCount <= index)
        return;

    mParamList[index].value64 = value;

    MYSQL_BIND* bind = &mParam[index];
    bind->buffer_type = MYSQL_TYPE_LONGLONG;
    bind->buffer = &mParamList[index].value64;
    bind->is_null = 0;
}

/*!
 * \brief   結果設定
 *
 * \param[in]   index   インデックス
 * \param[in]   result  結果を受け取る文字列バッファ
 * \param[in]   size    文字数
 *
 * \return  なし
 */
void MySQLStatement::setStringResult(int32_t index, CoreString* result, int32_t size) const
{
    if (index < 0 || mResultCount <= index)
        return;

    size *= 3;  // 文字数をUTF-8（日本語）が収まるバイト数に変換

    if (result->getCapacity() < size)
        result->setCapacity(    size);

    MYSQL_BIND* bind = &mResult[index];
    bind->buffer_type = MYSQL_TYPE_VAR_STRING;
    bind->buffer =        result->getBuffer();
    bind->buffer_length = size;
    bind->is_null = 0;
}

/*!
 * 結果設定
 */
void MySQLStatement::setIntResult(int32_t index, int32_t* result) const
{
    if (index < 0 || mResultCount <= index)
        return;

    MYSQL_BIND* bind = &mResult[index];
    bind->buffer_type = MYSQL_TYPE_LONG;
    bind->buffer = result;
    bind->is_null = 0;
}

/*!
 * \brief   バインド
 */
void MySQLStatement::bind() const throw(Exception)
{
    if (0 < mParamCount)
    {
        if (mysql_stmt_bind_param(mStmt, mParam) != 0)
            throwException();
    }

    if (0 < mResultCount)
    {
        mysql_stmt_free_result(mStmt);

        if (mysql_stmt_bind_result(mStmt, mResult) != 0)
            throwException();
    }
}

/*!
 * \brief   実行
 */
void MySQLStatement::execute() throw(Exception)
{
    if (mysql_stmt_execute(mStmt) != 0)
        throwException();
}

/*!
 * \brief   フェッチ
 */
bool MySQLStatement::fetch() const
{
    if (mysql_stmt_fetch(mStmt) != 0)
        return false;

    return true;
}

/*!
 * \brief   デストラクタ
 */
MySQL::~MySQL()
{
    if (mConn)
        mysql_close(mConn);
}

/*!
 * \brief   接続
 */
void MySQL::connect(const char* host, const char* user, const char* password, const char* db) throw(Exception)
{
    Exception e;

    if (mConn != nullptr)
    {
        e.setMessage("既に接続しています。");
        throw e;
    }

    mConn = mysql_init(nullptr);

    if (mysql_real_connect(mConn, host, user, password, db, 0, nullptr, 0) == nullptr)
    {
        e.setMessage("%s", mysql_error(mConn));
        throw e;
    }

    mysql_set_character_set(mConn, "utf8");
}

/*!
 * \brief   ステートメント生成
 */
Statement* MySQL::newStatement() const throw(Exception)
{
    Exception e;

    if (mConn == nullptr)
    {
        e.setMessage("接続していません。");
        throw e;
    }

    auto stmt = mysql_stmt_init(mConn);
    return (new MySQLStatement(this, stmt));
}

/*!
 * \brief   エラーメッセージ取得
 */
void MySQL::getErrorMessage(CoreString* str) const
{
    str->copy(mysql_error(mConn));
}

/*!
 * \brief   insert文のauto incrementを取得
 */
int64_t MySQL::getInsertID() const
{
    return mysql_insert_id(mConn);
}

} // namespace slog
