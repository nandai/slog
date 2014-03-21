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
 * \file    SQLite.cpp
 * \brief   SQLite
 * \author  Copyright 2014 printf.jp
 */
#pragma execution_character_set("utf-8")

#include "SQLite.h"

#include "slog/String.h"
#include "slog/SequenceLog.h"

#include <string.h>

namespace slog
{

/*!
 * \brief   SQLiteステートメントクラス
 */
class SQLiteStatement : public Statement
{
            static const char* CLS_NAME;

            /*!
             * 結果タイプ
             */
            enum class ResultType : int32_t
            {
                UNKNOWN,
                INT,
            };

            /*!
             * 結果
             */
            class Result
            {
                    /*!
                     * 結果タイプ
                     */
            public: ResultType type;

                    /*!
                     * 値
                     */
                    union Value
                    {
                        int32_t* value32;
                    } value;

                    /*!
                     * コンストラクタ
                     */
                    Result()
                    {
                        type = ResultType::UNKNOWN;
                    }
            };

            /*!
             * ステートメント
             */
            sqlite3_stmt* mStmt;

            /*!
             * パラメータ数
             */
            int32_t mParamCount;

            /*!
             * 結果数
             */
            int32_t mResultCount;

            /*!
             * 結果
             */
            Result* mResult;

            /*!
             * コンストラクタ
             */
public:     SQLiteStatement(const DB* db);

            /*!
             * デストラクタ
             */
            virtual ~SQLiteStatement() override;

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

const char* SQLiteStatement::CLS_NAME = "SQLiteStatement";

/*!
 * \brief   コンストラクタ
 */
SQLiteStatement::SQLiteStatement(const DB* db) : Statement(db)
{
    mStmt = nullptr;

    mParamCount = 0;
    mResultCount = 0;
}

/*!
 * \brief   デストラクタ
 */
SQLiteStatement::~SQLiteStatement()
{
    if (mStmt)
        sqlite3_finalize(mStmt);

    delete [] mResult;
}

/*!
 * \brief   実行準備
 */
void SQLiteStatement::prepare(const char* sql) throw(Exception)
{
    SLOG(CLS_NAME, "prepare");
    SMSG(slog::DEBUG, "%s", sql);

    sqlite3* db = dynamic_cast<const SQLite*>(mDB)->mConn;

    if (sqlite3_prepare_v2(db, sql, -1, &mStmt, nullptr) != SQLITE_OK)
        throwException();

    // パラメータ準備
    mParamCount = sqlite3_bind_parameter_count(mStmt);
    SMSG(slog::DEBUG, "mParamCount: %d", mParamCount);

    // 結果準備
    mResultCount = sqlite3_column_count(mStmt);
    mResult = new Result[mResultCount];
}

/*!
 * \brief   パラメータ設定
 */
void SQLiteStatement::setStringParam(int32_t index, const CoreString* value)
{
    if (index < 0 || mParamCount <= index)
        return;

    sqlite3_bind_text(mStmt, 1 + index, value->getBuffer(), -1, SQLITE_STATIC);
}

/*!
 * \brief   パラメータ設定
 */
void SQLiteStatement::setParam(int32_t index, int8_t value)
{
    if (index < 0 || mParamCount <= index)
        return;

    sqlite3_bind_int64(mStmt, 1 + index, value);
}

/*!
 * \brief   パラメータ設定
 */
void SQLiteStatement::setShortParam(int32_t index, int16_t value)
{
    if (index < 0 || mParamCount <= index)
        return;

    sqlite3_bind_int64(mStmt, 1 + index, value);
}

/*!
 * \brief   パラメータ設定
 */
void SQLiteStatement::setIntParam(int32_t index, int32_t value)
{
    if (index < 0 || mParamCount <= index)
        return;

    sqlite3_bind_int64(mStmt, 1 + index, value);
}

/*!
 * \brief   パラメータ設定
 */
void SQLiteStatement::setLongParam(int32_t index, int64_t value)
{
    if (index < 0 || mParamCount <= index)
        return;

    sqlite3_bind_int64(mStmt, 1 + index, value);
}

/*!
 * \brief   結果設定
 */
void SQLiteStatement::setIntResult(int32_t index, int32_t* result) const
{
    if (index < 0 || mResultCount <= index)
        return;

    mResult[index].type = ResultType::INT;
    mResult[index].value.value32 = result;
}

/*!
 * \brief   バインド
 */
void SQLiteStatement::bind() const throw(Exception)
{
    // no implementation
}

/*!
 * \brief   実行
 */
void SQLiteStatement::execute() throw(Exception)
{
    if (sqlite3_stmt_readonly(mStmt) != 0)
    {
        // select文の場合は何もしない
        return;
    }

    if (sqlite3_step(mStmt) != SQLITE_DONE)
        throwException();
}

/*!
 * \brief   フェッチ
 */
bool SQLiteStatement::fetch() const
{
    if (sqlite3_step(mStmt) == SQLITE_DONE)
        return false;

    for (int32_t i = 0; i < mResultCount; i++)
    {
        switch (mResult[i].type)
        {
        case ResultType::INT:
            *mResult[i].value.value32 = (int32_t)sqlite3_column_int64(mStmt, i);
            break;
        }
    }

    return true;
}

/*!
 * \brief   デストラクタ
 */
SQLite::~SQLite()
{
    if (mConn)
        sqlite3_close_v2(mConn);
}

/*!
 * \brief   接続
 */
void SQLite::connect(const char* host, const char* user, const char* password, const char* db) throw(Exception)
{
    Exception e;

    if (mConn != nullptr)
    {
        e.setMessage("既に接続しています。");
        throw e;
    }

    if (sqlite3_open_v2(db, &mConn, SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX, nullptr) != SQLITE_OK)
    {
        e.setMessage("%s", sqlite3_errmsg(mConn));
        throw e;
    }
}

/*!
 * \brief   ステートメント生成
 */
Statement* SQLite::newStatement() const throw(Exception)
{
    Exception e;

    if (mConn == nullptr)
    {
        e.setMessage("接続していません。");
        throw e;
    }

    return new SQLiteStatement(this);
}

/*!
 * \brief   エラーメッセージ取得
 */
void SQLite::getErrorMessage(CoreString* str) const
{
    str->copy(sqlite3_errmsg(mConn));
}

/*!
 * \brief   insert文のauto incrementを取得
 */
int64_t SQLite::getInsertID() const
{
    return sqlite3_last_insert_rowid(mConn);
}

} // namespace slog
