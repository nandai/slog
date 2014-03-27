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
 * \file    SequenceLogServiceDB.cpp
 * \brief   シーケンスログサービスDBクラス
 * \author  Copyright 2014 printf.jp
 */
#include "SequenceLogServiceDB.h"

#include "slog/String.h"
#include "slog/FileInfo.h"
#include "slog/Util.h"
#include "slog/SequenceLog.h"

#define VERSION_000 0
#define VERSION_001 1

namespace slog
{
const char* SequenceLogServiceDB::CLS_NAME = "SequenceLogServiceDB";

/*!
 * \brief   コンストラクタ
 */
SequenceLogServiceDB::SequenceLogServiceDB()
{
    SLOG(CLS_NAME, "SequenceLogServiceDB");

    String name;
    getName(&name);

    connect("localhost", "slog", "DPdhE8iv1HQIe6nL", name.getBuffer());
}

/*!
 * \brief   デストラクタ
 */
SequenceLogServiceDB::~SequenceLogServiceDB()
{
    SLOG(CLS_NAME, "~SequenceLogServiceDB");
}

/*!
 * \brief   データベース名取得
 *
 * \param[out]  name    データベース名を返す
 *
 * \return  なし
 */
void SequenceLogServiceDB::getName(CoreString* name)
{
    SLOG(CLS_NAME, "getName");

#if defined(USE_SQLITE)
    #if defined(__ANDROID__)
        Util::getProcessPath(name);
    #else
        name->copy("~");
        FileInfo fileInfo(name);

        name->copy(fileInfo.getCanonicalPath());
    #endif

    name->append("/SequenceLogService.db");
#else
    name->copy("SequenceLogService");
#endif
}

/*!
 * \brief   データベースバージョン取得
 *
 * \return  データベースバージョン
 */
int32_t SequenceLogServiceDB::getVersion() const
{
    SLOG(CLS_NAME, "getVersion");

    int32_t version = 0;
    Statement* stmt = nullptr;

    try
    {
        const char* sql = "select version from version_info";
        stmt = newStatement();
        stmt->prepare(sql);

        stmt->setIntResult(0, &version);

        stmt->bind();
        stmt->execute();
        stmt->fetch();
    }
    catch (Exception e)
    {
        // 初回起動時などテーブルが存在しない場合もあるので、
        // 例外が発生しても何もすることはない
        SMSG(slog::DEBUG, "%s", e.getMessage());
    }

    delete stmt;
    return version;
}

/*!
 * \brief   初期化
 *
 * \return  なし
 */
void SequenceLogServiceDB::init() const throw(Exception)
{
    SLOG(CLS_NAME, "init");
    int32_t version = getVersion();

    if (version == VERSION_000)
    {
        updateVersion1();
        version++;
    }
}

/*!
 * \brief   バージョン１にアップデート
 */
void SequenceLogServiceDB::updateVersion1() const throw(Exception)
{
    SLOG(CLS_NAME, "updateVersion1");
    Statement* stmt = nullptr;

    try
    {
        // バージョン情報テーブル作成
        query(
            "create table version_info("
            "    version   int     not null);");

        // バージョン登録
        stmt = newStatement();
        stmt->prepare("insert into version_info (version) values (1)");
        stmt->execute();

        delete stmt;
        stmt = nullptr;

        // アカウントテーブル作成
#if defined(USE_SQLITE)
        query(
            "create table user("
            "    id        integer     primary key autoincrement,"
            "    name      varchar     not null unique,"
            "    password  varchar     not null,"
            "    mail_addr varchar,"
            "    version   int         not null default 1,"
            "    admin     int         not null default 0);");
#else
        query(
            "create table user("
            "    id        int         primary key auto_increment,"
            "    name      varchar(20) not null unique,"
            "    password  varchar(64) not null,"
            "    mail_addr varchar(256),"
            "    version   tinyint     not null default 1,"
            "    admin     tinyint     not null default 0);");
#endif

        // 初期アカウント登録（パスワード：gols）
        stmt = newStatement();
        stmt->prepare("insert into user (name, password, admin) values ('slog', 'RrtQzcEv7FQ1QaazVN+ZXHHAS/5F/MVuDUffTotnFKk=', 1)");
        stmt->execute();
    }
    catch (Exception e)
    {
        SMSG(slog::DEBUG, "%s", e.getMessage());

        delete stmt;
        throw e;
    }

    delete stmt;
}

} // namespace slog
