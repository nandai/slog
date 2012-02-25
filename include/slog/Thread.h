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
 *  \file   Thread.h
 *  \brief  スレッドクラス
 *  \author Copyright 2011 log-tools.net
 */
#pragma once
#include "slog/slog.h"

#if defined(__unix__)
    #include <pthread.h>
    #include <unistd.h>
    #include <sys/syscall.h>
#endif

namespace slog
{
class Thread;

/*!
 *  \brief  スレッドリスナークラス
 */
class ThreadListener
{
public:     virtual void onInitialized(Thread* thread) {}   //!< スレッド初期化完了通知
            virtual void onTerminated( Thread* thread) {}   //!< スレッド終了通知
};

/*!
 *  \brief  スレッドクラス
 */
class Thread
{
#if defined(_WINDOWS)
            HANDLE          mHandle;            //!< スレッドハンドル
#else
            pthread_t       mHandle;            //!< スレッドハンドル
#endif

            bool            mInterrupted;       //!< 割り込みフラグ
            bool            mAlive;             //!< 生存フラグ

            ThreadListener  mDefaultListener;   //!< デフォルトリスナー
            ThreadListener* mListener;          //!< リスナー

public:     Thread();
            virtual ~Thread();

            void start();
            void join();

private:    virtual bool init() {return true;}  //!< 初期化
            virtual void run() = 0;             //!< スレッド実行

public:     virtual void interrupt();
            bool isInterrupted() const;
            bool isAlive() const;

            ThreadListener* getListener() const;
            void setListener(ThreadListener* listener);

private:
#if defined(_WINDOWS)
            static unsigned int __stdcall main(void* param);
#else
            static void* main(void* param);
#endif

public:     static uint32_t getCurrentId();
            static void sleep(uint32_t ms);
};

/*!
 *  \brief  割り込まれているか調べる
 */
inline bool Thread::isInterrupted() const
{
    return mInterrupted;
}

/*!
 *  \brief  生存確認
 */
inline bool Thread::isAlive() const
{
    return mAlive;
}

/*!
 *  \brief  リスナー取得
 */
inline ThreadListener* Thread::getListener() const
{
    return mListener;
}

/*!
 *  \brief  リスナー設定
 */
inline void Thread::setListener(ThreadListener* listener)
{
    mListener = (listener ? listener : &mDefaultListener);
}

/*!
 *  \brief  カレントスレッドID取得
 */
inline uint32_t Thread::getCurrentId()
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
inline void Thread::sleep(uint32_t ms)
{
#if defined(_WINDOWS)
    Sleep(ms);
#else
    timespec req;
    req.tv_sec =   ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000 * 1000;

    nanosleep(&req, NULL);
#endif
}

} // namespace slog
