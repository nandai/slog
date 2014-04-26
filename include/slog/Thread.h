/*
 * Copyright (C) 2011-2014 printf.jp
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
 * \file    Thread.h
 * \brief   スレッドクラス
 * \author  Copyright 2011-2014 printf.jp
 */
#pragma once

#include "slog/slog.h"
#include <list>

#pragma warning(disable:4251)

#if defined(__unix__)
    #include <pthread.h>
#endif

namespace slog
{
class ThreadListener;
class Thread;

typedef std::list<ThreadListener*> ThreadListeners;

/*!
 * \brief   スレッドリスナークラス
 */
class SLOG_API ThreadListener
{
            /*!
             * スレッド初期化完了通知
             */
public:     virtual void onThreadInitialized(Thread* thread) {}

            /*!
             * スレッド終了通知
             */
            virtual void onThreadTerminated(Thread* thread) {}
};

/*!
 * \brief   スレッドクラス
 */
class SLOG_API Thread
{
            /*!
             * スレッドハンドル
             */
#if defined(_WINDOWS)
            int64_t mHandle;
#else
            pthread_t mHandle;
#endif

            /*!
             * 割り込みフラグ
             */
            bool mInterrupted;

            /*!
             * 生存フラグ
             */
            bool mAlive;

            /*!
             * リスナーリスト
             */
            ThreadListeners mListeners;

            /*!
             * コンストラクタ
             */
public:     Thread();

            /*!
             * デストラクタ
             */
            virtual ~Thread();

            /*!
             * スレッド開始
             */
            void start();

            /*!
             * スレッド終了待ち
             */
            void join();

            /*!
             * 初期化
             */
private:    virtual bool init() {return true;}

            /*!
             * スレッド実行
             */
            virtual void run() = 0;

            /*!
             * 割り込み
             */
public:     virtual void interrupt();

            /*!
             * 割り込まれているか調べる
             */
            bool isInterrupted() const;

            /*!
             * 生存しているか調べる
             */
            bool isAlive() const;

            /*!
             * リスナー追加
             */
            void addThreadListener(ThreadListener* listener);

            /*!
             * リスナー解除
             */
           void removeThreadListener(ThreadListener* listener);

            /*!
             * リスナーリスト取得
             */
            ThreadListeners* getListeners() const;

            /*!
             * スレッドメイン
             */
private:
#if defined(_WINDOWS)
            static unsigned int __stdcall main(void* param);
#else
            static void* main(void* param);
#endif

            /*!
             * カレントスレッドID取得
             */
public:     static uint32_t getCurrentId();

            /*!
             * スリープ
             */
            static void sleep(uint32_t ms);
};

/*!
 * \brief   割り込まれているか調べる
 */
inline bool Thread::isInterrupted() const
{
    return mInterrupted;
}

/*!
 * \brief   生存確認
 */
inline bool Thread::isAlive() const
{
    return mAlive;
}

/*!
 * \brief   リスナー追加
 */
inline void Thread::addThreadListener(ThreadListener* listener)
{
    mListeners.push_back(listener);
}

/*!
 * \brief   リスナーリスト取得
 */
inline ThreadListeners* Thread::getListeners() const
{
    return (ThreadListeners*)&mListeners;
}

} // namespace slog
