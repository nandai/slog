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
 *  \file   Account.cpp
 *  \brief  アカウントクラス
 *  \author Copyright 2014 printf.jp
 */
#pragma execution_character_set("utf-8")

#include "Account.h"
#include "R.h"

#include "slog/Json.h"
#include "slog/SHA256.h"
#include "slog/ByteBuffer.h"
#include "slog/Util.h"
#include "slog/FileInfo.h"
#include "slog/SequenceLog.h"

#define     USE_SQLITE
#if defined(USE_SQLITE)
    #include "SQLite.h"
#else
    #include "DB_MySQL.h"
#endif

#undef max
#undef NAME_MAX

#include <algorithm>

#define LATEST_HASH_VERSION 1

namespace slog
{
const char* AccountLogic::CLS_NAME = "AccountLogic";

const int32_t Account::NAME_MIN =        4;
const int32_t Account::NAME_MAX =       20;
const int32_t Account::PASSWD_MIN =      8;
const int32_t Account::PASSWD_MAX =     32;
const int32_t Account::MAIL_ADDR_MAX = 256;

/*!
 * \brief   コンストラクタ
 */
Account::Account()
{
    this->id = -1;
    this->version = 0;
    this->admin = 0;
}

/*!
 * \brief   コンストラクタ
 */
AccountLogic::AccountLogic()
{
    SLOG(CLS_NAME, "AccountLogic");

    mAccount = nullptr;
    mJson = Json::getNewObject();
    r = nullptr;

#if defined(USE_SQLITE)
    String dbPath;

    #if defined(__ANDROID__)
        Util::getProcessPath(&dbPath);
    #else
        dbPath.copy("~");
        FileInfo fileInfo(&dbPath);

        dbPath.copy(fileInfo.getCanonicalPath());
    #endif

    dbPath.append("/SequenceLogService.db");
    const char* db = dbPath.getBuffer();

    mDB = new SQLite;
#else
    const char* db = "SequenceLogService";
    mDB = new MySQL;
#endif

    try
    {
        mDB->connect("localhost", "slog", "DPdhE8iv1HQIe6nL", db);
    }
    catch (Exception e)
    {
        SMSG(slog::DEBUG, "%s", e.getMessage());

        delete mDB;
        mDB = nullptr;

        return;
    }

#if 1 // とりあえずここでテスト
    int32_t version = 0;
    Statement* stmt = nullptr;

    try
    {
        const char* sql = "select version from version_info";
        stmt = mDB->newStatement();
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
    stmt = nullptr;

    if (version == 0)
    {
        try
        {
            // バージョン情報テーブル
            mDB->query(
                "create table version_info("
                "    version   int     not null);");

            stmt = mDB->newStatement();
            stmt->prepare("insert into version_info (version) values (1)");
            stmt->execute();

            delete stmt;
            stmt = nullptr;

            // アカウントテーブル
#if defined(USE_SQLITE)
            mDB->query(
                "create table user("
                "    id        integer     primary key autoincrement,"
                "    name      varchar     not null unique,"
                "    password  varchar     not null,"
                "    mail_addr varchar,"
                "    version   int         not null default 1,"
                "    admin     int         not null default 0);");
#else
            mDB->query(
                "create table user("
                "    id        int         primary key auto_increment,"
                "    name      varchar(20) not null unique,"
                "    password  varchar(64) not null,"
                "    mail_addr varchar(256),"
                "    version   tinyint     not null default 1,"
                "    admin     tinyint     not null default 0);");
#endif

            stmt = mDB->newStatement();
            stmt->prepare("insert into user (name, password, admin) values ('slog', 'RrtQzcEv7FQ1QaazVN+ZXHHAS/5F/MVuDUffTotnFKk=', 1)");
            stmt->execute();
        }
        catch (Exception e)
        {
            SMSG(slog::DEBUG, "%s", e.getMessage());
        }

        delete stmt;
        stmt = nullptr;
    }
#endif
}

/*!
 * \brief   デストラクタ
 */
AccountLogic::~AccountLogic()
{
    SLOG(CLS_NAME, "~AccountLogic");

    delete mDB;
    delete mJson;
}

/*!
 * \brief   ユーザー名とパスワードでアカウントを取得する
 *
 * \param[in,out]   account アカウント
 *
 * \return  取得できた場合はtrue、できなかった場合はfalseを返す
 */
bool AccountLogic::getByNamePassword(Account* account) const
{
    SLOG(CLS_NAME, "getByNamePassword");

    if (mDB == nullptr)
        return false;

    String passwd = account->passwd;

//  std::unique_ptr<Statement> stmt(mDB->newStatement());
    Statement* stmt =               mDB->newStatement();

    prepare(stmt, account, "name=?");
    stmt->setStringParam(0, &account->name);

    stmt->bind();
    stmt->execute();

    bool res = stmt->fetch();
    delete stmt;

    if (res)
    {
        String hashPasswd;
        getHashPassword(&hashPasswd, &account->name, &passwd, account->version);

        res = account->passwd.equals(&hashPasswd);
    }

    SMSG(slog::DEBUG, "id:%d, name:%s, version:%d, admin:%d",
        account->id,
        account->name.getBuffer(),
        account->version,
        account->admin);

    return res;
}

/*!
 * \brief   ユーザーIDでアカウントを取得する
 *
 * \param[in,out]   account アカウント
 *
 * \return  取得できた場合はtrue、できなかった場合はfalseを返す
 */
bool AccountLogic::getById(Account* account) const
{
    SLOG(CLS_NAME, "getById");

    if (mDB == nullptr)
        return false;

    String passwd = account->passwd;

//  std::unique_ptr<Statement> stmt(mDB->newStatement());
    Statement* stmt =               mDB->newStatement();

    prepare(stmt, account, "id=?");
    stmt->setLongParam(0, account->id);

    stmt->bind();
    stmt->execute();

    bool res = stmt->fetch();
    delete stmt;

    SMSG(slog::DEBUG, "id:%d, name:%s, version:%d, admin:%d",
        account->id,
        account->name.getBuffer(),
        account->version,
        account->admin);

    return res;
}

/*!
 * \brief   SQL準備
 *
 * \param[out]  stmt    ステートメント
 * \param[out]  account アカウント
 * \param[in]   where   検索条件（NULL可）
 *
 * \return  なし
 */
void AccountLogic::prepare(Statement* stmt, Account* account, const char* where) const
{
    static const char* baseSQL = "select id, name, password, mail_addr, version, admin from user";
    String sql;

    if (where == nullptr)
    {
        sql.copy(baseSQL);
    }
    else
    {
        sql.format("%s where %s", baseSQL, where);
    }

    stmt->prepare(sql.getBuffer());

    stmt->setIntResult(   0, &account->id);
    stmt->setStringResult(1, &account->name,     Account::NAME_MAX);
    stmt->setStringResult(2, &account->passwd,   64);
    stmt->setStringResult(3, &account->mailAddr, Account::MAIL_ADDR_MAX);
    stmt->setIntResult(   4, &account->version);
    stmt->setIntResult(   5, &account->admin);
}

/*!
 * \brief   アカウント更新可能かどうか
 *
 * \param[in]   account アカウント
 *
 * \return  アカウント操作結果
 */
bool AccountLogic::canUpdate(const Account* account)
{
    SLOG(CLS_NAME, "canUpdate");

    if (mDB == nullptr)
        return false;

    mAccount = account;

    ValidateList list;
    list.add(new StringValidate(&mAccount->name,   Account::NAME_MIN,   Account::NAME_MAX));
    list.add(new StringValidate(&mAccount->passwd, Account::PASSWD_MIN, Account::PASSWD_MAX));

    if (list.execute(this) == false)
        return false;

//  std::unique_ptr<Statement> stmt(mDB->newStatement());
    Statement* stmt =               mDB->newStatement();

    Account nowAccount;
    prepare(stmt, &nowAccount, "id=?");
    stmt->setLongParam(0, mAccount->id);

    stmt->bind();
    stmt->execute();
    stmt->fetch();

    delete stmt;
    stmt = nullptr;

    if (nowAccount.admin != 1 && nowAccount.name.equals(&mAccount->name) == false)
    {
        // 一般ユーザーはユーザー名の変更不可
        mJson->add("", r->string(R::msg003));
        return false;
    }

    // 同名のユーザーが存在しないかチェック
    stmt = mDB->newStatement();
    stmt->prepare("select count(*) from user where id<>? and name=?");
    stmt->setIntParam(   0,  mAccount->id);
    stmt->setStringParam(1, &mAccount->name);

    int32_t count = -1;
    stmt->setIntResult(0, &count);

    stmt->bind();
    stmt->execute();
    stmt->fetch();

    delete stmt;

    if (count != 0)
        mJson->add("", r->string(R::msg002));

    return (count == 0);
}

/*!
 * \brief   検証失敗イベント
 *
 * \param[in]   value   
 * \param[in]   result  
 *
 * \return  なし
 */
void AccountLogic::onInvalid(const void* value, const slog::Validate::Result* result)
{
    SLOG(CLS_NAME, "onInvalid");

    String str;
    const char* variableName = nullptr;
    const char* displayName =  nullptr;

    if (value == &mAccount->name)
    {
        variableName = "name";
        displayName = r->string(R::user_name);
    }

    if (value == &mAccount->passwd)
    {
        variableName = "passwd";
        displayName = r->string(R::password);
    }

    if (variableName)
    {
        if (result == slog::Validate::INVALID)
            str.format(r->string(R::msg004), displayName);

        else if (result == slog::Validate::EMPTY)
            str.format(r->string(R::msg005), displayName);

        else if (result == slog::Validate::TOO_SHORT)
            str.format(r->string(R::msg006), displayName);

        else if (result == slog::Validate::TOO_LONG)
            str.format(r->string(R::msg007), displayName);

        SMSG(slog::DEBUG, str.getBuffer());
        mJson->add(variableName, &str);
    }
}

/*!
 * \brief   アカウント更新
 *
 * \param[in]   account アカウント
 *
 * \return  なし
 */
void AccountLogic::update(const Account* account) const
{
    SLOG(CLS_NAME, "update");

    if (mDB == nullptr)
        return;

    String hashPasswd;
    Statement* stmt = nullptr;

    if (getHashPassword(&hashPasswd, &account->name, &account->passwd, LATEST_HASH_VERSION) == false)
        return;

    try
    {
//      std::unique_ptr<Statement> stmt(mDB->newStatement());
        stmt =                          mDB->newStatement();
        stmt->prepare("update user set name=?, password=?, version=? where id=?");
        stmt->setStringParam(0, &account->name);
        stmt->setStringParam(1, &hashPasswd);
        stmt->setParam(      2,  LATEST_HASH_VERSION);
        stmt->setLongParam(  3,  account->id);

        stmt->bind();
        stmt->execute();
    }
    catch (Exception e)
    {
        SMSG(slog::DEBUG, "%s", e.getMessage());
    }

    delete stmt;
}

/*!
 * \brief   ハッシュ化パスワード取得
 *
 * \param[out]  hashPasswd  ハッシュ化パスワード
 * \param[in]   name        ユーザー名
 * \param[in]   passwd      パスワード
 * \param[in]   version     ハッシュバージョン
 *
 * \return  未対応のハッシュバージョンの場合はfalse、そうでなければtrueを返す
 */
bool AccountLogic::getHashPassword(CoreString* hashPasswd, const CoreString* name, const CoreString* passwd, int32_t version) const
{
    SLOG(CLS_NAME, "getHashPassword");

    if (version < 1 || LATEST_HASH_VERSION < version)
    {
        // 現状ハッシュバージョン１以外は不一致となるようにする
        SMSG(slog::DEBUG, "バージョンエラー (%d)", version);
        hashPasswd->format("__%s__", passwd->getBuffer());
        return false;
    }

    String salt;
    salt.format("%s%s", name->getBuffer(), "SequenceLogService");

    SHA256 hash;
    int32_t hashDataSize = salt.getLength() + std::max(passwd->getLength(), hash.getHashSize());

    ByteBuffer hashData(hashDataSize);
    hashData.put(&salt, salt.getLength());

    const char* p = passwd->getBuffer();
    int32_t len =   passwd->getLength();

    int32_t stretchingCount = 10000;
    for (int32_t i = 0; i < stretchingCount; i++)
    {
        hashData.setPosition(salt.getLength());
        hashData.put(p, len);

        hash.execute(&hashData);

        p = (const char*)hash.getMessageDigest();
        len =            hash.getHashSize();
    }

    Util::encodeBase64(hashPasswd, (const char*)hash.getMessageDigest(), hash.getHashSize());
    return true;
}

} // namespace slog
