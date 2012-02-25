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
 *  \file	DateTimeFormat.h
 *  \brief	日付時間フォーマットクラス
 *  \author	Copyright 2011 log-tools.net
 */
#pragma once

#include "slog/CoreString.h"
#include "slog/DateTime.h"

namespace slog
{

/*!
 *  \brief	日付時間フォーマットクラス
 */
class DateTimeFormat
{
public:		enum Format
			{
				DATE_TIME,
				DATE_TIME_MS,
			};

			enum Length
			{
				DATE_TIME_LEN =    sizeof("YYYY/MM/DD HH:MI:SS")     - 1,
				DATE_TIME_MS_LEN = sizeof("YYYY/MM/DD HH:MI:SS.999") - 1,
			};

public:		static void toString(CoreString* str, const DateTime& dateTime, Format format);
};

/*
 *  \brief	日付時間を文字列で取得
 */
inline void DateTimeFormat::toString(
	CoreString* str,			//!< ここに結果を返す
	const DateTime& dateTime,	//!< 日付時間
	Format format)				//!< フォーマット
{
	static const char* szFormat[] =
	{
		"%04u/%02u/%02u %02u:%02u:%02u",
		"%04u/%02u/%02u %02u:%02u:%02u.%03u",
	};

    str->format(
		szFormat[format],
        dateTime.getYear(), dateTime.getMonth(),  dateTime.getDay(),
        dateTime.getHour(), dateTime.getMinute(), dateTime.getSecond(), dateTime.getMilliSecond());
}

} // namespace slog
