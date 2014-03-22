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
#include "Account.h"

#include "slog/SHA256.h"
#include "slog/ByteBuffer.h"
#include "slog/Util.h"

#include "SQLite.h"

#include <algorithm>

namespace slog
{
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
    String dbPath;
    Util::getProcessPath(&dbPath);
    dbPath.append("/SequenceLogService.db");

    mDB = new SQLite;
    mDB->connect("", "", "", dbPath.getBuffer());
}

/*!
 * \brief   デストラクタ
 */
AccountLogic::~AccountLogic()
{
    delete mDB;
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

        res = account->passwd.equals(hashPasswd);
    }

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
    String passwd = account->passwd;

//  std::unique_ptr<Statement> stmt(mDB->newStatement());
    Statement* stmt =               mDB->newStatement();

    prepare(stmt, account, "id=?");
    stmt->setIntParam(0, account->id);

    stmt->bind();
    stmt->execute();

    bool res = stmt->fetch();
    delete stmt;

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
    stmt->setStringResult(2, &account->passwd,   Account::PASSWD_MAX);
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
AccountLogic::Result AccountLogic::canUpdate(const Account* account) const
{
//  std::unique_ptr<Statement> stmt(mDB->newStatement());
    Statement* stmt =               mDB->newStatement();

    Account nowAccount;
    prepare(stmt, &nowAccount, "id=?");
    stmt->setIntParam(0, account->id);

    stmt->bind();
    stmt->execute();
    stmt->fetch();

    delete stmt;
    stmt = nullptr;

    if (nowAccount.admin != 1 && nowAccount.name.equals(account->name) == false)
    {
        // 一般ユーザーはユーザー名の変更不可
        return Result::CANT_CHANGE_USER_NAME;
    }

    // 同名のユーザーが存在しないかチェック
    stmt = mDB->newStatement();
    stmt->prepare("select count(*) from user where id<>? and name=?");
    stmt->setIntParam(   0,  account->id);
    stmt->setStringParam(1, &account->name);

    int32_t count = -1;
    stmt->setIntResult(0, &count);

    stmt->bind();
    stmt->execute();
    stmt->fetch();

    delete stmt;

    return (count == 0
        ? Result::OK
        : Result::ALREADY_USER_EXISTS);
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
    String hashPasswd;

    if (getHashPassword(&hashPasswd, &account->name, &account->passwd, account->version) == false)
        return;

//  std::unique_ptr<Statement> stmt(mDB->newStatement());
    Statement* stmt =               mDB->newStatement();
    stmt->prepare("update user set name=?, password=? where id=?");
    stmt->setStringParam(0, &account->name);
    stmt->setStringParam(1, &hashPasswd);
    stmt->setIntParam(   2,  account->id);

    stmt->bind();
    stmt->execute();

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
    if (version != 1)
    {
        // 現状ハッシュバージョン１以外は不一致となるようにする
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
