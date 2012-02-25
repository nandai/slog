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
 *  \file	Exception.cpp
 *  \brief	例外クラス
 *  \author	Copyright 2011 log-tools.net
 */
#include "slog/Exception.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#if defined(_WINDOWS)
#define snprintf	_snprintf
#endif

namespace slog
{

/*!
 *  \brief	メッセージ設定
 */
void Exception::setMessage(const char* format, ...)
{
#if defined(_WINDOWS)
	mErrorNo = GetLastError();
#else
	mErrorNo = errno;
#endif

	// メッセージ生成
	va_list arg;
	va_start(arg, format);

	int32_t capacity = sizeof(mMessage) - 1;
	int32_t len = vsnprintf(mMessage, capacity, format, arg);
	va_end(arg);

	// バッファが一杯か？
	if (len == -1)
	{
		mMessage[capacity] = '\0';
		return;
	}

	// APIエラーによる例外か？
	if (mErrorNo == 0)
		return;

	// APIエラーのメッセージ取得
	char* buffer = NULL;

#if defined(_WINDOWS)
	DWORD flags =
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS;

	FormatMessageA(flags, NULL, mErrorNo, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 0, NULL);
	buffer[strlen(buffer) - 2] = '\0';		// 改行除去
#else
	buffer = strerror(mErrorNo);
#endif

	if (len)
		len = snprintf(mMessage + len, capacity - len, " / %s(%d)", buffer, mErrorNo);
	else
		len = snprintf(mMessage + len, capacity - len, "%s",        buffer);

	if (len == -1)
		mMessage[capacity] = '\0';

#if defined(_WINDOWS)
	LocalFree(buffer);
#endif
}

}
