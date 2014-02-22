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
 *  \file   Thread.cpp
 *  \brief  スレッドクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/Thread.h"

#if defined(_WINDOWS)
    #include <windows.h>
    #include <process.h>
#endif

#if defined(__unix__)
    #include <unistd.h>
    #include <sys/syscall.h>
#endif

namespace slog
{

/*!
 *  \brief  コントラクタ
 */
Thread::Thread()
{
    mHandle = 0/*nullptr*/;
    mInterrupted = false;
    mAlive = false;

    setListener(nullptr);
}

/*!
 *  \brief  デストラクタ
 */
Thread::~Thread()
{
}

/*!
 *  \brief  スレッド開始
 */
void Thread::start()
{
#if defined(_WINDOWS)
    #if !defined(MODERN_UI)
    mHandle = (int64_t)_beginthreadex(nullptr, 0, main, this, 0, nullptr);
    #endif
#else
    pthread_create(&mHandle, 0/*nullptr*/, main, this);
#endif

    mInterrupted = false;
    mAlive = true;
}

/*!
 *  \brief  スレッド終了待ち
 */
void Thread::join()
{
#if defined(_WINDOWS)
    #if !defined(MODERN_UI)
    HANDLE handle = (HANDLE)mHandle;

    WaitForSingleObject(handle, INFINITE);
    CloseHandle(handle);
    #endif
#else
    pthread_join(mHandle, 0/*nullptr*/);
#endif

    mHandle = 0/*nullptr*/;
}

/*!
 *  \brief  割り込み
 */
void Thread::interrupt()
{
    mInterrupted = true;
}

/*!
 *  \brief  リスナー除外
 */
void Thread::removeListener(ThreadListener* listener)
{
    ThreadListeners* listeners = getListeners();

    for (auto i = listeners->begin(); i != listeners->end(); i++)
    {
        if (*i == listener)
        {
            listeners->erase(i);
            break;
        }
    }
}

/*!
 *  \brief  スレッドエントリーポイント
 */
#if defined(_WINDOWS)
unsigned int __stdcall Thread::main(void* param)
#else
void* Thread::main(void* param)
#endif
{
    Thread* thread = (Thread*)param;

//  thread->mInterrupted = false;
//  thread->mAlive = true;

    if (thread->init())
    {
        ThreadListeners* listeners = thread->getListeners();

        for (auto i = listeners->begin(); i != listeners->end(); i++)
            (*i)->onInitialized(thread);

        thread->run();
    }

//  thread->mAlive = false;

    {
        ThreadListeners* listeners = thread->getListeners();
        ThreadListener* threadIsListener = dynamic_cast<ThreadListener*>(thread);
        ThreadListener* self = nullptr;

        for (auto i = listeners->begin(); i != listeners->end(); i++)
        {
            if (*i ==  threadIsListener)
                self = threadIsListener;

            else
                (*i)->onTerminated(thread);
        }

        thread->mAlive = false;     // 自身のonTerminated()でdeleteする可能性があるので
                                    // mAliveへの値設定はonTerminated()の前に行う

        if (self)
            self->onTerminated(thread);
    }

#if defined(_WINDOWS)
    #if !defined(MODERN_UI)
    _endthreadex(0);
    #endif
#else
    pthread_exit(0);
#endif

    return 0;
}

/*!
 *  \brief  カレントスレッドID取得
 */
uint32_t Thread::getCurrentId()
{
#if defined(_WINDOWS)
    return (uint32_t)GetCurrentThreadId();
#elif defined(__ANDROID__)
    return (uint32_t)gettid();
#else
//  return (uint32_t)syscall(224);
    return (uint32_t)syscall(SYS_gettid);
#endif
}

/*!
 *  \brief  スリープ
 */
void Thread::sleep(uint32_t ms)
{
#if defined(_WINDOWS)
    WaitForSingleObjectEx(GetCurrentThread(), ms, FALSE);
#else
    timespec req;
    req.tv_sec =   ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000 * 1000;

    nanosleep(&req, nullptr);
#endif
}

} // namespace slog
