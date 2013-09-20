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
 *  \brief  差を取得する
 */
uint32_t TimeSpan::operator-(const TimeSpan& timeSpan) const
{
#if !defined(_WINDOWS) && !defined(__x86_64)
    if (mMS < timeSpan.mMS)
        return ((0xFFFFFFFF - timeSpan.mMS + 1) + mMS);
#endif

    return (uint32_t)(mMS - timeSpan.mMS);
}

} // namespace slog
