﻿/*
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
 *  \file   Session.h
 *  \brief  セッションクラス
 *  \author Copyright 2014 printf.jp
 */
#pragma once

#include "slog/String.h"
#include "slog/Mutex.h"

#include <list>

namespace slog
{

/*!
 *  \brief  セッションクラス
 */
class Session
{
            /*!
             * セッション名
             */
public:     static const char* NAME;

            /*!
             * セッションID
             */
private:    String mId;

            /*!
             * ユーザーID
             */
            int32_t mUserId;

            /*!
             * IPアドレス
             */
            String mIP;

            /*!
             * User-Agent
             */
            String mUserAgent;

            /*!
             * コンストラクタ
             */
public:     Session(int32_t userId, const CoreString* ip, const CoreString* userAgent);

            /*!
             * セッション生成
             */
            void generate();

            /*!
             * セッションID取得
             */
            const CoreString* getId() const {return &mId;}

            /*!
             * ユーザーID取得
             */
            int32_t getUserId() const {return mUserId;}

            /*!
             * IPアドレス取得
             */
            const CoreString* getIP() const {return &mIP;}

            /*!
             * User-Agent取得
             */
            const CoreString* getUserAgent() const {return &mUserAgent;}
};

/*!
 *  \brief  セッションマネージャークラス
 */
class SessionManager
{
            /*!
             * セッションミューテックス
             */
            Mutex mSessionMutex;

            /*!
             * セッションリスト
             */
            std::list<Session*> mSessionList;

            /*!
             * クリア
             */
public:     static void clear();

            /*
             * セッション取得
             */
            static Session* get(const CoreString* ip, const CoreString* userAgent);

            /*!
             * セッション追加
             */
            static void add(Session* session);
};

} // namespace slog