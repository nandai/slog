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

#if defined(__unix__)
    #include <pthread.h>
#endif

namespace slog
{
class ScopedLock;

#if defined(MODERN_UI)
inline HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName)
{
    return CreateMutexExA(lpMutexAttributes, lpName, (bInitialOwner ? CREATE_MUTEX_INITIAL_OWNER : 0), 0);
}

inline HANDLE OpenMutexA(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName)
{
    UTF16LE utf16le;
    utf16le.conv(lpName);

    return OpenMutexW(dwDesiredAccess, bInheritHandle, utf16le.getBuffer());
}
#endif

/*!
 *  \brief  ミューテックスクラス
 */
class Mutex
{
            friend class ScopedLock;

#if defined(_WINDOWS)
            HANDLE              mHandle;    //!< ミューテックスハンドル
#else
            pthread_mutex_t     mPrivate;   //!< プロセス内ミューテックス
            pthread_mutex_t*    mHandle;    //!< ミューテックスの実体
            bool                mCreate;    //!< 作成フラグ
#endif

public:
            Mutex() throw(Exception);

#if defined(_WINDOWS)
            Mutex(bool create, const CoreString& name) throw(Exception);
#else
            Mutex(bool create, pthread_mutex_t* mutex);
#endif
            ~Mutex();

private:    void lock();
            void unlock();
};

/*!
 *  \brief  コンストラクタ
 */
inline Mutex::Mutex() throw(Exception)
{
#if defined(_WINDOWS)
//  mHandle = CreateMutexA(NULL, TRUE,  NULL);
    mHandle = CreateMutexA(NULL, FALSE, NULL);

    if (mHandle == NULL)
    {
        Exception e;
        e.setMessage("Mutex::Mutex()");

        throw e;
    }
#else
    mHandle = &mPrivate;
    mCreate = true;
    pthread_mutex_init(mHandle, NULL);
#endif
}

#if defined(_WINDOWS)
/*!
 *  \brief  コンストラクタ
 */
inline Mutex::Mutex(
    bool create,                //!< 作成フラグ
    const CoreString& name)     //!< ミューテックス名

    throw(Exception)
{
    if (create)
        mHandle = CreateMutexA(NULL, TRUE, name.getBuffer());
    else
        mHandle = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, name.getBuffer());

    if (mHandle == NULL)
    {
        Exception e;
        e.setMessage("Mutex::Mutex(create:%s, \"%s\")", (create ? "true" : "false"), name);

        throw e;
    }
}
#else
/*!
 *  \brief  コンストラクタ
 */
inline Mutex::Mutex(
    bool create,                //!< 作成フラグ
    pthread_mutex_t* mutex)     //!< ミューテックスの実体
{
    mHandle = mutex;
    mCreate = create;

    if (mCreate)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

        pthread_mutex_init(mHandle, &attr);
    }
}
#endif

/*!
 *  \brief  デストラクタ
 */
inline Mutex::~Mutex()
{
#if defined(_WINDOWS)
    CloseHandle(mHandle);
#else
    if (mCreate)
        pthread_mutex_destroy(mHandle);
#endif
}

/*!
 *  \brief  ロック
 */
inline void Mutex::lock()
{
#if defined(_WINDOWS)
    WaitForSingleObjectEx(mHandle, INFINITE, FALSE);
#else
    pthread_mutex_lock(mHandle);
#endif
}

/*!
 *  \brief  アンロック
 */
inline void Mutex::unlock()
{
    if (this == NULL)
        return;

#if defined(_WINDOWS)
    ReleaseMutex(mHandle);
#else
    pthread_mutex_unlock(mHandle);
#endif
}

/*!
 *  \brief  スコープドロッククラス
 */
class ScopedLock
{
            Mutex*  mMutex;

public:      ScopedLock(Mutex* mutex, bool callLock = true);
            ~ScopedLock();

            void release();
};

/*!
 *  \brief  コンストラクタ
 */
inline ScopedLock::ScopedLock(Mutex* mutex, bool callLock)
{
    mMutex = mutex;

    if (mMutex && callLock)
        mMutex->lock();
}

/*!
 *  \brief  デストラクタ
 */
inline ScopedLock::~ScopedLock()
{
    if (mMutex)
        mMutex->unlock();
}

/*!
 *  \brief  リリース
 */
inline void ScopedLock::release()
{
    mMutex = NULL;
}

} // namespace slog
