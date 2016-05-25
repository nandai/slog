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
 * \file    DateTime.h
 * \brief   日付時間クラス
 * \author  Copyright 2011-2013 printf.jp
 */
#pragma once

#include "slog/slog.h"
struct tm;

#if defined(__unix__) || defined(__APPLE__)
    #include <sys/time.h>
#endif

namespace slog
{

/*!
 * \brief   日付時間クラス
 */
class SLOG_API DateTime
{
            /*!
             * 日付時間
             */
            uint64_t mValue;

            /*!
             * コンストラクタ
             */
public:     DateTime();

            /*!
             * 現在日時設定
             */
            void setCurrent();

            /*!
             * UTCからローカル時間に変換
             */
            void toLocal();

            /*!
             * 日付時間をuint64_t値で取得
             */
            uint64_t getValue() const;

            /*!
             * 日付時間をuint64_t値で設定
             */
            void setValue(uint64_t value);

            /*!
             * 日付時間をuint64_t値で設定
             */
            void toValue(tm* tm, uint32_t milliSecond);

            /*!
             * 日付時間をtime_tで設定
             */
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

            /*!
             * 日付時間をミリ秒に変換
             */
            int64_t toMilliSeconds() const;

            /*!
             * 曜日を取得する
             */
            int32_t getWeekDay() const;
};

} // namespace slog
