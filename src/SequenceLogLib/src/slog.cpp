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
 *  \file   slog.cpp
 *  \brief  シーケンスログネームスペース
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/slog.h"
#include "slog/FixedString.h"

#if defined(__ANDROID__)
    #include <android/log.h>
#endif

#if defined(__unix__)
    #include <syslog.h>
#endif

using namespace slog;

/*!
 *  \brief  通知
 */
extern "C" void noticeLog(const char* format, ...)
{
    va_list arg;
    va_start(arg, format);

    FixedString<255> str;

    try
    {
        str.formatV(format, arg);
    }
    catch (Exception /*e*/)
    {
    }

#if defined(_WINDOWS)
    UTF16LE utf16le;
    utf16le.conv(str);
    const wchar_t* p = utf16le.getBuffer();

    OutputDebugStringW(p);

    if (p[utf16le.getChars() - 1] != '\n')
        OutputDebugStringW(L"\n");

#elif defined(__ANDROID__)
    __android_log_write(ANDROID_LOG_INFO, "slog", str.getBuffer());
#else
    syslog(LOG_NOTICE, "%s", str.getBuffer());
#endif
}
