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
 *  \file   TimeSpan.cpp
 *  \brief  経過時間クラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/TimeSpan.h"

#if defined(_WINDOWS)
    #include <windows.h>
#endif

namespace slog
{

/*!
 *  \brief  コンストラクタ
 */
TimeSpan::TimeSpan()
{
    now();
}

/*!
 * \brief   現在時間設定
 */
void TimeSpan::now()
{
#if defined(_WINDOWS)
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    mMS = now.QuadPart * 1000 / freq.QuadPart;
#else
    mMS = clock() / (CLOCKS_PER_SEC / 1000);
#endif
}

/*!
 * 時間設定
 */
void TimeSpan::set(int64_t ms)
{
    mMS = ms;
}

/*!
 * 加算
 */
void TimeSpan::add(int64_t value)
{
    mMS += value;
}

/*!
 *  \brief  時間差を取得する
 */
int64_t TimeSpan::operator-(const TimeSpan& timeSpan) const
{
    return (mMS - timeSpan.mMS);
}

} // namespace slog
