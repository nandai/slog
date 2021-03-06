﻿/*
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
 *  \file   slog.h
 *  \brief  シーケンスログネームスペース
 *  \author Copyright 2011-2015 printf.jp
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

    #if defined(SLOG_EXPORTS)
        #define SLOG_API __declspec(dllexport)
    #else
        #define SLOG_API __declspec(dllimport)
    #endif

//  #if !defined(_WINDLL)
    #if !defined(SLOG_EXPORTS)
//      #elif defined(_DEBUG)
//          #pragma comment(lib, "slogd.lib")
//      #else
            #pragma comment(lib, "slog.lib")
//      #endif
    #endif

//  #pragma comment(lib, "winmm.lib")
#else
    #define SLOG_API
#endif

#if defined(_WINDOWS)
    #pragma warning(disable : 4290)
    #pragma warning(disable : 4793)
#endif

#if defined(_WINDOWS)
    #define PATH_DELIMITER  '\\'
#else
    #define PATH_DELIMITER  '/'
#endif

#if defined(__ANDROID__)
    #define nullptr 0
    #define override
#elif defined(__linux__) && defined(__GNUC__)
    #if __GNUC__ < 4 || __GNUC_MINOR__ < 6
        #define nullptr 0
        #define override
    #endif
#endif

#define MAX_PATH 260

#if defined(__cplusplus)
extern "C" {
#endif

SLOG_API void noticeLog(const char* format, ...);

#if defined(__cplusplus)
}
#endif
