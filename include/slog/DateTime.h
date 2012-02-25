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
 *  \file   DateTime.h
 *  \brief  日付時間クラス
 *  \author Copyright 2011 log-tools.net
 */
#pragma once

#include "slog/slog.h"
#include <time.h>

#if defined(__unix__)
    #include <sys/time.h>
#endif

namespace slog
{

/*!
 *  \brief  日付時間クラス
 */
class DateTime
{
            uint64_t    mValue;     //!< 日付時間

public:     DateTime();
            void setCurrent();
            void toLocal();

            uint64_t getValue() const;
            void setValue(uint64_t value);
            void toValue(tm* tm, uint32_t milliSecond);
            void setTime_t(time_t value, uint32_t milliSecond = 0);

//          operator uint64_t() const;
//          DateTime& operator=(uint64_t value);

            uint32_t getYear()        const {return ((mValue >> 56) &   0xFF) + 1900;}
            uint32_t getMonth()       const {return ((mValue >> 48) &   0xFF);}
            uint32_t getDay()         const {return ((mValue >> 40) &   0xFF);}
            uint32_t getHour()        const {return ((mValue >> 32) &   0xFF);}
            uint32_t getMinute()      const {return ((mValue >> 24) &   0xFF);}
            uint32_t getSecond()      const {return ((mValue >> 16) &   0xFF);}
            uint32_t getMilliSecond() const {return ( mValue        & 0xFFFF);}

            void setYear(       uint32_t year)        {mValue = (mValue & 0x00FFFFFFFFFFFFFFLL) | ((uint64_t)(year - 1900) << 56);} //!< 年設定
            void setMonth(      uint32_t month)       {mValue = (mValue & 0xFF00FFFFFFFFFFFFLL) | ((uint64_t) month        << 48);} //!< 月設定
            void setDay(        uint32_t day)         {mValue = (mValue & 0xFFFF00FFFFFFFFFFLL) | ((uint64_t) day          << 40);} //!< 日設定
            void setHour(       uint32_t hour)        {mValue = (mValue & 0xFFFFFF00FFFFFFFFLL) | ((uint64_t) hour         << 32);} //!< 時設定
            void setMinute(     uint32_t minute)      {mValue = (mValue & 0xFFFFFFFF00FFFFFFLL) | ((uint64_t) minute       << 24);} //!< 分設定
            void setSecond(     uint32_t second)      {mValue = (mValue & 0xFFFFFFFFFF00FFFFLL) | ((uint64_t) second       << 16);} //!< 秒設定
            void setMilliSecond(uint32_t milliSecond) {mValue = (mValue & 0xFFFFFFFFFFFF0000LL) | ((uint64_t) milliSecond       );} //!< ミリ秒設定

            int64_t toMilliSeconds() const;
};

/*!
 *  \brief  コンストラクタ
 */
inline DateTime::DateTime()
{
    mValue = 0;
}

/*!
 *  \brief  現在日時設定
 */
inline void DateTime::setCurrent()
{
#if defined(_WINDOWS)
    SYSTEMTIME now;
//  GetLocalTime( &now);
    GetSystemTime(&now);

    mValue =
        ((uint64_t)(now.wYear - 1900) << 56) |
        ((uint64_t) now.wMonth        << 48) |
        ((uint64_t) now.wDay          << 40) |
        ((uint64_t) now.wHour         << 32) |
        ((uint64_t) now.wMinute       << 24) |
        ((uint64_t) now.wSecond       << 16) |
        ((uint64_t) now.wMilliseconds      );
#else
#if 0
    timeval tv;

    gettimeofday(&tv, NULL);
    setTime_t(tv.tv_sec, (uint64_t)tv.tv_usec / 1000);
#else   
    timespec tv;

    clock_gettime(CLOCK_REALTIME, &tv);
    setTime_t(tv.tv_sec, (uint64_t)tv.tv_nsec / (1000 * 1000));
#endif
#endif
}

/*!
 *  \brief  UTCからローカル時間に変換
 */
inline void DateTime::toLocal()
{
#if defined(_WINDOWS)
    TIME_ZONE_INFORMATION timeZone;
    GetTimeZoneInformation(&timeZone);

    int32_t bias = -timeZone.Bias / 60;
#else
    struct timezone tz;
    gettimeofday(NULL, &tz);

    int32_t bias = -tz.tz_minuteswest / 60;
#endif

    struct tm tm;
    tm.tm_year = getYear() - 1900;
    tm.tm_mon =  getMonth() - 1;
    tm.tm_mday = getDay();
    tm.tm_hour = getHour() + bias;
    tm.tm_min =  getMinute();
    tm.tm_sec =  getSecond();
    tm.tm_yday =
    tm.tm_wday =
    tm.tm_isdst = 0;

    time_t time = mktime(&tm);
    struct tm* tmLocal = localtime(&time);

    toValue(tmLocal, getMilliSecond());
}

/*!
 *  \brief  日付時間をuint64_t値で取得
 */
inline uint64_t DateTime::getValue() const
{
    return mValue;
}

/*!
 *  \brief  日付時間をuint64_t値で設定
 */
inline void DateTime::setValue(uint64_t value)
{
    mValue = value;
}

/*!
 *  \brief  日付時間をuint64_t値で設定
 */
inline void DateTime::toValue(tm* tm, uint32_t milliSecond)
{
    if (tm == NULL)
        return;

    mValue =
        ((uint64_t) tm->tm_year     << 56) |
        ((uint64_t)(tm->tm_mon + 1) << 48) |
        ((uint64_t) tm->tm_mday     << 40) |
        ((uint64_t) tm->tm_hour     << 32) |
        ((uint64_t) tm->tm_min      << 24) |
        ((uint64_t) tm->tm_sec      << 16) |
        milliSecond;
}

/*!
 *  \brief  日付時間をtime_tで設定
 */
inline void DateTime::setTime_t(time_t value, uint32_t milliSecond)
{
//  tm* tm = localtime(&value);
    tm* tm = gmtime(&value);

    toValue(tm, milliSecond);
}

/*!
 *  \brief  日付時間をミリ秒に変換
 */
inline int64_t DateTime::toMilliSeconds() const
{
    enum
    {
        January =                0,
        February =  January +   31,
        March =     February +  28,
        April =     March +     31,
        May =       April +     30,
        June =      May +       31,
        July =      June +      30,
        August =    July +      31,
        September = August +    31,
        October =   September + 30,
        November =  October +   31,
        December =  November +  30,
    };

    static const int64_t daySpan[] =
    {
        January,
        February,
        March,
        April,
        May,
        June,
        July,
        August,
        September,
        October,
        November,
        December,
    };

    int64_t y = getYear();
    int64_t m = getMonth();
    int64_t d = getDay();

    int64_t dy = (y - 1) * 365;                     // 年数分の日数
    int64_t dl = (y / 4) - (y / 100) + (y / 400);   // 閏年分の日数
    int64_t dm = daySpan[m - 1];                    // 1月1日からm月1日までの日数
    int64_t result = dy + dl + dm + d - 1;

    result =
        (result * 24 * 60 * 60 * 1000) +
        (getHour()   * 60 * 60 * 1000) + 
        (getMinute()      * 60 * 1000) +
        (getSecond()           * 1000) +
         getMilliSecond();

    return result;
}

} // namespace slog
