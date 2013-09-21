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
 *  \file   Process.h
 *  \brief  プロセスクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once
#include "slog/slog.h"

namespace slog
{

/*!
 *  \brief  プロセスクラス
 */
class SLOG_API Process
{
#if defined(_WINDOWS)
            int64_t     mHandle;                                //!< プロセスハンドル
#else
            char        mHandle[sizeof("/proc/-2147483648")];   //!< プロセスハンドル
#endif
            uint32_t    mId;                                    //!< プロセスID

public:      Process();
            ~Process();

            uint32_t getId() const;

#if !defined(MODERN_UI)
            void setId(uint32_t id);
            bool isAlive() const;
#endif
};

} // namespace slog
