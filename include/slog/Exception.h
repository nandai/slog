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
 *  \file	Exception.h
 *  \brief	例外クラス
 *  \author	Copyright 2011 log-tools.net
 */
#pragma once
#include "slog/slog.h"

#if defined(__unix__)
	#include <errno.h>
#endif

namespace slog
{

/*!
 *  \brief	例外クラス
 */
class Exception
{
			int32_t		mErrorNo;			//!< エラー番号
			char		mMessage[255 + 1];	//!< メッセージ

public:		Exception();

			int32_t getErrorNo() const;
			const char* getMessage() const;

public:		void setMessage(const char* format, ...);
};

/*!
 *  \brief	コンストラクタ
 */
inline Exception::Exception()
{
	mErrorNo = 0;
}

/*!
 *  \brief	エラー番号取得
 */
inline int32_t Exception::getErrorNo() const
{
	return mErrorNo;
}

/*!
 *  \brief	メッセージ取得
 */
inline const char* Exception::getMessage() const
{
	return mMessage;
}

} // namespace slog
