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
 *  \file   DateTimeFormat.h
 *  \brief  日付時間フォーマットクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once

#include "slog/CoreString.h"
#include "slog/DateTime.h"

namespace slog
{

/*!
 *  \brief  日付時間フォーマットクラス
 */
class DateTimeFormat
{
public:     enum class Format : int32_t
            {
                YYYYMMDDHHMISSMS,
                YYYYMMDDHHMISS,
                YYYYMMDDHHMI,
                YYYYMMDD,
                MMDD,
                HHMI,
            };

            enum class Length : int32_t
            {
                YYYYMMDDHHMISSMS = sizeof("YYYY/MM/DD HH:MI:SS.999") - 1,
                YYYYMMDDHHMISS =   sizeof("YYYY/MM/DD HH:MI:SS")     - 1,
                YYYYMMDDHHMI   =   sizeof("YYYY/MM/DD HH:MI")        - 1,
                YYYYMMDD =         sizeof("YYYY/MM/DD")              - 1,
                MMDD =             sizeof("MM/DD")                   - 1,
                HHMI =             sizeof("HH:MI")                   - 1,
            };

public:     static void toString(CoreString* str, const DateTime& dateTime, Format format);
};

/*
 *  \brief  日付時間を文字列で取得
 */
inline void DateTimeFormat::toString(
    CoreString* str,            //!< ここに結果を返す
    const DateTime& dateTime,   //!< 日付時間
    Format format)              //!< フォーマット
{
    class FormatInfo
    {
    public:     const char* szFormat;
                int32_t     indexes[7];
    };

    static const FormatInfo infoArray[] =
    {
        {"%04u/%02u/%02u %02u:%02u:%02u.%03u", {0, 1, 2, 3, 4, 5, 6}},
        {"%04u/%02u/%02u %02u:%02u:%02u",      {0, 1, 2 ,3, 4, 5, 0}},
        {"%04u/%02u/%02u %02u:%02u",           {0, 1, 2 ,3, 4, 0, 0}},
        {"%04u/%02u/%02u",                     {0, 1, 2, 0, 0, 0, 0}},
        {"%02u/%02u",                          {1, 2, 0, 0, 0, 0, 0}},
        {"%02u:%02u",                          {3, 4, 0, 0, 0, 0, 0}},
    };

    const FormatInfo* info = &infoArray[(int32_t)format];
    uint32_t values[] =
    {
        dateTime.getYear(),             // 0
        dateTime.getMonth(),            // 1
        dateTime.getDay(),              // 2
        dateTime.getHour(),             // 3
        dateTime.getMinute(),           // 4
        dateTime.getSecond(),           // 5
        dateTime.getMilliSecond()       // 6
    };

    str->format(
        info->szFormat,
        values[info->indexes[0]],
        values[info->indexes[1]],
        values[info->indexes[2]],
        values[info->indexes[3]],
        values[info->indexes[4]],
        values[info->indexes[5]],
        values[info->indexes[6]]);
}

} // namespace slog
