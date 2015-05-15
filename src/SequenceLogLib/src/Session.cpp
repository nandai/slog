/*
 * Copyright (C) 2014-2015 printf.jp
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
 *  \file   Session.cpp
 *  \brief  セッションクラス
 *  \author Copyright 2014-2015 printf.jp
 */
#include "slog/Session.h"
#include "slog/DateTime.h"
#include "slog/SHA1.h"

#include <stdio.h>
#include <stdlib.h>

namespace slog
{

const char* Session::NAME = "CPPSESSIONID";

/*!
 * セッションマネージャー
 */
static SessionManager sSessionManager;

/*!
 * \brief   コンストラクタ
 */
Session::Session(int32_t userId, const CoreString* userAgent)
{
    mUserId = userId;
    mUserAgent.copy(userAgent);
    mSecure = false;
}

/*!
 * \brief   セッション生成
 */
void Session::generate()
{
    DateTime dateTime;
    dateTime.setCurrent();

    String key;
    key.format("%s%s%llu", mIP.getBuffer(), mUserAgent.getBuffer(), dateTime.toMilliSeconds());

    SHA1 hash;
    hash.execute(&key);

    mId.setCapacity(hash.getHashSize() * 2);
    mId.setLength(  hash.getHashSize() * 2);

    char* p = mId.getBuffer();
    const uint8_t* md = hash.getMessageDigest();

    for (int32_t i = 0; i < hash.getHashSize(); i++)
    {
        sprintf(&p[i * 2], (rand() % 2 ? "%02x" : "%02X"), md[i]);
    }
}

/*!
 * \brief   クリア
 */
void SessionManager::clear()
{
    ScopedLock lock(&sSessionManager.mSessionMutex);

    for (auto i = sSessionManager.mSessionList.begin(); i != sSessionManager.mSessionList.end(); i++)
        delete *i;

    sSessionManager.mSessionList.clear();
}

/*!
 * \brief   セッション取得
 *
 * \param[in]   userId      ユーザーID
 * \param[in]   userAgent   ユーザーエージェント
 *
 * \return  セッション
 */
Session* SessionManager::getByUserId(int32_t userId, const CoreString* userAgent)
{
    if (userId == -1 || userAgent == nullptr)
        return nullptr;

    Session* session = find(nullptr, userId, nullptr, userAgent);
    return   session;
}

/*!
 * \brief   セッション取得
 *
 * \param[in]   userId      ユーザーID
 * \param[in]   ip          IPアドレス
 *
 * \return  セッション
 */
Session* SessionManager::getByUserIdAndIp(int32_t userId, const CoreString* ip)
{
    if (userId == -1 || ip == nullptr)
        return nullptr;

    Session* session = find(nullptr, userId, ip, nullptr);
    return   session;
}

/*!
 * \brief   セッション取得
 *
 * \param[in]   id          セッションID（null可）
 * \param[in]   ip          IPアドレス
 * \param[in]   userAgent   ユーザーエージェント
 *
 * \return  セッション
 */
Session* SessionManager::getBySessionIdAndIp(const CoreString* id, const CoreString* ip, const CoreString* userAgent)
{
    if (ip == nullptr || userAgent == nullptr)
        return nullptr;

    Session* session = find(id, -1, ip, userAgent);
    return   session;
}

/*!
 * \brief   セッション取得
 *
 * \param[in]   id          セッションID        （null: 無条件）
 * \param[in]   userId      ユーザーID          （-1  : 無条件）
 * \param[in]   ip          IPアドレス          （null: 無条件）
 * \param[in]   userAgent   ユーザーエージェント（null: 無条件）
 *
 * \return  セッション
 */
Session* SessionManager::find(const CoreString* id, int32_t userId, const CoreString* ip, const CoreString* userAgent)
{
    ScopedLock lock(&sSessionManager.mSessionMutex);
    Session* session;

    for (auto i = sSessionManager.mSessionList.begin(); i != sSessionManager.mSessionList.end(); i++)
    {
        session = *i;

        if (id         && session->getId()->equals(id) == false)
            continue;

        if (userId > 0 && session->getUserId() != userId)
            continue;

        if (ip         && session->getIP()->equals(ip) == false)
            continue;

        if (userAgent  && session->getUserAgent()->equals(userAgent) == false)
            continue;

        return session;
    }

    return nullptr;
}

/*!
 * \brief   セッション追加
 *
 * \param[in]   session セッション
 *
 * \return  なし
 */
void SessionManager::add(Session* session)
{
    ScopedLock lock(&sSessionManager.mSessionMutex);
    sSessionManager.mSessionList.push_back(session);
}

/*!
 * \brief   セッション削除
 *
 * \param[in]   session セッション
 *
 * \return  なし
 */
void SessionManager::remove(Session* session)
{
    ScopedLock lock(&sSessionManager.mSessionMutex);
    sSessionManager.mSessionList.remove(session);
}

} // namespace slog
