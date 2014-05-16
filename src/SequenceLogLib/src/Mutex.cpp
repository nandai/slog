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
 *  \file   Mutex.cpp
 *  \brief  ミューテックスクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/Mutex.h"
#include "slog/CoreString.h"

#if defined(_WINDOWS)
    #include <windows.h>
#endif

namespace slog
{

/*!
 *  \brief  コンストラクタ
 */
Mutex::Mutex() throw(Exception)
{
#if defined(_WINDOWS)
//  mHandle = (int64_t)CreateMutexW(nullptr, TRUE,  nullptr);
    mHandle = (int64_t)CreateMutexW(nullptr, FALSE, nullptr);

    if (mHandle == 0)
    {
        Exception e;
        e.setMessage("Mutex::Mutex()");

        throw e;
    }
#else
    mHandle = &mPrivate;
    mCreate = true;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);

    pthread_mutex_init(mHandle, &attr);
#endif
}

#if defined(_WINDOWS)
/*!
 * \brief   コンストラクタ
 *
 * \param[in]   create  作成フラグ
 * \param[in]   name    ミューテックス名
 */
Mutex::Mutex(bool create, const CoreString* name)

    throw(Exception)
{
    UTF16LE utf16le;
    utf16le.conv(name);

    if (create)
        mHandle = (int64_t)CreateMutexW(nullptr, TRUE, utf16le.getBuffer());
    else
        mHandle = (int64_t)OpenMutexW(MUTEX_ALL_ACCESS, FALSE, utf16le.getBuffer());

    if (mHandle == 0)
    {
        Exception e;
        e.setMessage("Mutex::Mutex(create:%s, \"%s\")", (create ? "true" : "false"), name->getBuffer());

        throw e;
    }
}
#else
/*!
 *  \brief  コンストラクタ
 */
Mutex::Mutex(
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
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);

        pthread_mutex_init(mHandle, &attr);
    }
}
#endif

/*!
 *  \brief  デストラクタ
 */
Mutex::~Mutex()
{
#if defined(_WINDOWS)
    CloseHandle((HANDLE)mHandle);
#else
    if (mCreate)
        pthread_mutex_destroy(mHandle);
#endif
}

/*!
 *  \brief  ロック
 */
void Mutex::lock()
{
#if defined(_WINDOWS)
    WaitForSingleObjectEx((HANDLE)mHandle, INFINITE, FALSE);
#else
    pthread_mutex_lock(mHandle);
#endif
}

/*!
 *  \brief  アンロック
 */
void Mutex::unlock()
{
    if (this == nullptr)
        return;

#if defined(_WINDOWS)
    ReleaseMutex((HANDLE)mHandle);
#else
    pthread_mutex_unlock(mHandle);
#endif
}

/*!
 *  \brief  コンストラクタ
 */
ScopedLock::ScopedLock(Mutex* mutex, bool callLock)
{
    mMutex = mutex;

    if (mMutex && callLock)
        mMutex->lock();
}

/*!
 *  \brief  デストラクタ
 */
ScopedLock::~ScopedLock()
{
    if (mMutex)
        mMutex->unlock();
}

/*!
 *  \brief  リリース
 */
void ScopedLock::release()
{
    mMutex = nullptr;
}

} // namespace slog
