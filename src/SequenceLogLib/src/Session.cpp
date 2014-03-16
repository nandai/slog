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
 *  \file   Session.cpp
 *  \brief  セッションクラス
 *  \author Copyright 2014 printf.jp
 */
#include "Session.h"

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
Session::Session(int32_t userId, const CoreString* ip, const CoreString* userAgent)
{
    mUserId = userId;
    mIP.copy(*ip);
    mUserAgent.copy(*userAgent);
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
    for (auto i = sSessionManager.mSessionList.begin(); i != sSessionManager.mSessionList.end(); i++)
        delete *i;

    sSessionManager.mSessionList.clear();
}

/*!
 * \brief   セッション取得
 */
Session* SessionManager::get(const CoreString* ip, const CoreString* userAgent)
{
    ScopedLock lock(&sSessionManager.mSessionMutex);
    Session* session;

    for (auto i = sSessionManager.mSessionList.begin(); i != sSessionManager.mSessionList.end(); i++)
    {
        session = *i;

        if (session->getIP()->       equals(*ip) &&
            session->getUserAgent()->equals(*userAgent))
        {
            return session;
        }
    }

    return nullptr;
}

/*!
 * \brief   セッション追加
 */
void SessionManager::add(Session* session)
{
    ScopedLock lock(&sSessionManager.mSessionMutex);
    sSessionManager.mSessionList.push_back(session);
}

} // namespace slog
