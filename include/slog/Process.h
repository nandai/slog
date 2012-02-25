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
 *  \file   Process.h
 *  \brief  プロセスクラス
 *  \author Copyright 2011 log-tools.net
 */
#pragma once
#include "slog/slog.h"

#if defined(__unix__)
    #include <stdio.h>
    #include <unistd.h>
    #include <sys/stat.h>
#endif

namespace slog
{

/*!
 *  \brief  プロセスクラス
 */
class Process
{
#if defined(_WINDOWS)
            HANDLE      mHandle;                                //!< プロセスハンドル
#else
            char        mHandle[sizeof("/proc/-2147483648")];   //!< プロセスハンドル
#endif
            uint32_t    mId;                                    //!< プロセスID

public:      Process();
            ~Process();

            uint32_t getId() const;
            void     setId(uint32_t id);

            bool isAlive() const;
};

/*!
 *  \brief  コンストラクタ
 */
inline Process::Process()
{
#if defined(_WINDOWS)
    mHandle = NULL;
    mId = (uint32_t)GetCurrentProcessId();
#else
    mHandle[0] = '\0';
    mId = (uint32_t)getpid();
#endif
}

/*!
 *  \brief  デストラクタ
 */
inline Process::~Process()
{
#if defined(_WINDOWS)
    if (mHandle)
        CloseHandle(mHandle);
#endif
}

/*!
 *  \brief  プロセスID取得
 */
inline uint32_t Process::getId() const
{
    return mId;
}

/*!
 *  \brief  プロセスID設定
 */
inline void Process::setId(uint32_t id)
{
#if defined(_WINDOWS)
    mHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
#else
    sprintf(mHandle, "/proc/%d", id);
#endif

    mId = id;
}

/*!
 *  \brief  生存確認
 */
inline bool Process::isAlive() const
{
#if defined(_WINDOWS)
    DWORD exitCode;
    GetExitCodeProcess(mHandle, &exitCode);

    return (exitCode == STILL_ACTIVE);
#else
    struct stat buf;
    return (stat(mHandle, &buf) == 0);
#endif
}

} // namespace slog
