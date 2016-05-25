/*
 * Copyright (C) 2011-2015 printf.jp
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
 *  \file   Process.cpp
 *  \brief  プロセスクラス
 *  \author Copyright 2011-2015 printf.jp
 */
#include "slog/Process.h"

#if defined(_WINDOWS)
    #include <windows.h>
#endif

#if defined(__unix__) || defined(__APPLE__)
    #include <stdio.h>
    #include <unistd.h>
    #include <sys/stat.h>
#endif

namespace slog
{

/*!
 *  \brief  コンストラクタ
 */
Process::Process()
{
#if defined(_WINDOWS)
    mHandle = 0;
    mId = (uint32_t)GetCurrentProcessId();
#else
    mHandle[0] = '\0';
    mId = (uint32_t)getpid();
#endif
}

/*!
 *  \brief  デストラクタ
 */
Process::~Process()
{
#if defined(_WINDOWS)
    if (mHandle)
    {
        CloseHandle((HANDLE)mHandle);
        mHandle = 0;
    }
#endif
}

/*!
 *  \brief  プロセスID取得
 */
uint32_t Process::getId() const
{
    return mId;
}

/*!
 *  \brief  プロセスID設定
 */
void Process::setId(uint32_t id)
{
#if defined(_WINDOWS)
    mHandle = (int64_t)OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
#else
    sprintf(mHandle, "/proc/%d", id);
#endif

    mId = id;
}

/*!
 *  \brief  生存確認
 */
bool Process::isAlive() const
{
#if defined(_WINDOWS)
    DWORD exitCode;
    GetExitCodeProcess((HANDLE)mHandle, &exitCode);

    return (exitCode == STILL_ACTIVE);
#else
    struct stat buf;
    return (stat(mHandle, &buf) == 0);
#endif
}

} // namespace slog
