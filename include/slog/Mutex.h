/*
 * Copyright (C) 2011-2013 printf.jp
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
 *  \file   Mutex.h
 *  \brief  ミューテックスクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once
#include "slog/Exception.h"

#if defined(__unix__) || defined(__APPLE__)
    #include <pthread.h>
#endif

namespace slog
{
class CoreString;
class ScopedLock;

/*!
 *  \brief  ミューテックスクラス
 */
class SLOG_API Mutex
{
            friend class ScopedLock;

#if defined(_WINDOWS)
            int64_t             mHandle;    //!< ミューテックスハンドル
#else
            pthread_mutex_t     mPrivate;   //!< プロセス内ミューテックス
            pthread_mutex_t*    mHandle;    //!< ミューテックスの実体
            bool                mCreate;    //!< 作成フラグ
#endif

            /*!
             * コンストラクタ／デストラクタ
             */
public:     Mutex() throw(Exception);

#if defined(_WINDOWS)
            Mutex(bool create, const CoreString* name) throw(Exception);
#else
            Mutex(bool create, pthread_mutex_t* mutex);
#endif
            ~Mutex();

            /*!
             * ロック／アンロック
             */
private:    void lock();
            void unlock();
};

/*!
 *  \brief  スコープドロッククラス
 */
class SLOG_API ScopedLock
{
            Mutex*  mMutex;

            /*!
             * コンストラクタ／デストラクタ
             */
public:      ScopedLock(Mutex* mutex, bool callLock = true);
            ~ScopedLock();

            /*!
             * リリース
             */
            void release();
};

} // namespace slog
