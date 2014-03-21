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

#include <memory>
#include <algorithm>

namespace slog
{

/*!
 * \brief   コンストラクタ
 */
Account::Account()
{
    String dbPath;
    Util::getProcessPath(&dbPath);
    dbPath.append("/SequenceLogService.db");

    mDB = new SQLite;
    mDB->connect("", "", "", dbPath.getBuffer());

    this->id = -1;
}

/*!
 * \brief   デストラクタ
 */
Account::~Account()
{
    delete mDB;
}

/*!
 * \brief   検証
 */
bool Account::validate()
{
    String hashPasswd;
    getHashPassword(&hashPasswd);

    std::unique_ptr<Statement> stmt(mDB->newStatement());
    stmt->prepare("select id from user where name=? and password=?");
    stmt->setStringParam(0, &this->name);
    stmt->setStringParam(1, &hashPasswd);

    stmt->setIntResult(0, &this->id);

    stmt->bind();
    stmt->execute();

    return stmt->fetch();
}

/*!
 * \brief   アカウント更新可能かどうか
 */
bool Account::canUpdate() const
{
    std::unique_ptr<Statement> stmt(mDB->newStatement());
    stmt->prepare("select count(*) from user where id<>? and name=?");
    stmt->setIntParam(   0,  this->id);
    stmt->setStringParam(1, &this->name);

    int32_t count = -1;
    stmt->setIntResult(0, &count);

    stmt->bind();
    stmt->execute();
    stmt->fetch();

    return (count == 0);
}

/*!
 * \brief   アカウント更新
 */
void Account::update() const
{
    String hashPasswd;
    getHashPassword(&hashPasswd);

    std::unique_ptr<Statement> stmt(mDB->newStatement());
    stmt->prepare("update user set name=?, password=? where id=?");
    stmt->setStringParam(0, &this->name);
    stmt->setStringParam(1, &hashPasswd);
    stmt->setIntParam(   2,  this->id);

    stmt->bind();
    stmt->execute();
}

/*!
 * \brief   ハッシュ化パスワード取得
 */
void Account::getHashPassword(CoreString* hashPasswd) const
{
    String salt;
    salt.format("%s%s", this->name.getBuffer(), "SequenceLogService");

    SHA256 hash;
    int32_t hashDataSize = salt.getLength() + std::max(this->passwd.getLength(), hash.getHashSize());

    ByteBuffer hashData(hashDataSize);
    hashData.put(&salt, salt.getLength());

    const char* p = this->passwd.getBuffer();
    int32_t len =   this->passwd.getLength();

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
}

} // namespace slog
