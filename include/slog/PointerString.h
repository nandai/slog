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
 *  \file	PointerString.h
 *  \brief	ポインタ文字列クラス
 *  \author	Copyright 2011 log-tools.net
 */
#pragma once
#include "slog/CoreString.h"

namespace slog
{

/*!
 *  \brief	ポインタ文字列クラス
 */
class PointerString : public CoreString
{
			int32_t		mCapacity;		//!< 容量
			char*		mBuffer;		//!< バッファ

private:	const PointerString& operator=(const char*);
			const PointerString& operator=(const PointerString&);

protected:	PointerString();
public:		PointerString(char* buffer);
			PointerString(char* buffer, int32_t capacity);

protected:	void init(char* buffer, int32_t capacity = -1);

public:		virtual char*   getBuffer() const;
			virtual int32_t getCapacity() const;
};

/*!
 *  \brief	コンストラクタ
 */
inline PointerString::PointerString()
{
	mCapacity = 0;
	mBuffer = NULL;
}

/*!
 *  \brief	コンストラクタ
 */
inline PointerString::PointerString(char* buffer)
{
	init(buffer);
}

/*!
 *  \brief	コンストラクタ
 */
inline PointerString::PointerString(char* buffer, int32_t capacity)
{
	init(buffer, capacity);
}

/*!
 *  \brief	初期化
 */
inline void PointerString::init(char* buffer, int32_t capacity)
{
	int32_t len = (int32_t)strlen(buffer);

	mCapacity = (capacity != -1 ? capacity : len);
	mBuffer = buffer;

	setLength(len);
}

/*!
 *  \brief	バッファアドレス取得
 */
inline char* PointerString::getBuffer() const
{
	return mBuffer;
}

/*!
 *  \brief	バッファサイズ取得
 */
inline int32_t PointerString::getCapacity() const
{
	return mCapacity;
}

} // namespace slog
