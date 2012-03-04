/*
 * Copyright (C) 2011 log-tools.net
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
 *  \author Copyright 2011 log-tools.net
 */
#include "slog/Thread.h"

#if defined(_WINDOWS)
    #include <process.h>
#endif

namespace slog
{

/*!
 *  \brief  コントラクタ
 */
Thread::Thread()
{
    mHandle = 0/*NULL*/;
    mInterrupted = false;
    mAlive = false;

    setListener(NULL);
}

/*!
 *  \brief  デストラクタ
 */
Thread::~Thread()
{
    TRACE("Thread::~Thread()\n", 0);
}

/*!
 *  \brief  スレッド開始
 */
void Thread::start()
{
#if defined(_WINDOWS)
    mHandle = (HANDLE)_beginthreadex(NULL, 0, main, this, 0, NULL);
#else
    pthread_create(&mHandle, 0/*NULL*/, main, this);
#endif
}

/*!
 *  \brief  スレッド終了待ち
 */
void Thread::join()
{
#if defined(_WINDOWS)
    WaitForSingleObject(mHandle, INFINITE);
    CloseHandle(mHandle);
#else
    pthread_join(mHandle, 0/*NULL*/);
#endif

    mHandle = 0/*NULL*/;
}

/*!
 *  \brief  割り込み
 */
void Thread::interrupt()
{
    mInterrupted = true;
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

    thread->mInterrupted = false;
    thread->mAlive = true;

    if (thread->init())
    {
        thread->mListener->onInitialized(thread);
        thread->run();
    }

    thread->mAlive = false;
    thread->mListener->onTerminated(thread);

#if defined(_WINDOWS)
    _endthreadex(0);
#else
    pthread_exit(0);
#endif

    return 0;
}

} // namespace slog
