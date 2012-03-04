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
 *  \file   slog.h
 *  \brief  シーケンスログネームスペース
 *  \author Copyright 2011 log-tools.net
 */
#pragma once

#if !defined(_MSC_VER) || _MSC_VER > 1500
    #include <stdint.h>
#else
    #include "slog/stdint.h"
#endif

#if defined(_MSC_VER)
    #undef  _WINDOWS
    #define _WINDOWS

    #include <windows.h>

    #if !defined(_WINDLL)
        #if defined(_DEBUG)
            #if defined(_DLL)
                #pragma comment(lib, "SequenceLogMDd.lib")
            #else
                #pragma comment(lib, "SequenceLogd.lib")
            #endif
        #else
            #if defined(_DLL)
                #pragma comment(lib, "SequenceLogMD.lib")
            #else
                #pragma comment(lib, "SequenceLog.lib")
            #endif
        #endif
    #endif

    #pragma comment(lib, "winmm.lib")
#endif

#if defined(_WINDOWS)
    #pragma warning(disable : 4290)
    #pragma warning(disable : 4793)
#endif

#if defined(_DEBUG)
    #undef  TRACE
    #define TRACE(format, ...)  _printDebug(format, __VA_ARGS__)
#else
    #undef  TRACE
    #define TRACE
#endif

#if defined(_WINDOWS)
    #define PATH_DELIMITER  '\\'
#else
    #include <limits.h>
    #define  MAX_PATH PATH_MAX
    #define PATH_DELIMITER  '/'
#endif

#if defined(__cplusplus)
extern "C" {
#endif

void _printDebug(const char* format, ...);
void noticeLog(  const char* format, ...);

#if defined(__cplusplus)
}
#endif
