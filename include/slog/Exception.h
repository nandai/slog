/*
 * Copyright (C) 2011-2014 printf.jp
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
 *  \file   Exception.h
 *  \brief  例外クラス
 *  \author Copyright 2011-2014 printf.jp
 */
#pragma once
#include "slog/slog.h"

namespace slog
{

/*!
 *  \brief  例外クラス
 */
class SLOG_API Exception
{
            /*!
             * noticeLog()でログを出力するか
             */
            bool mOutputLog;

            /*!
             * エラー番号
             */
            int32_t mErrorNo;

            /*!
             * メッセージ
             */
            char mMessage[255 + 1];

            /*!
             * コンストラクタ
             */
public:     Exception(bool outputLog = true);

            /*!
             * エラー番号取得
             */
            int32_t getErrorNo() const;

            /*!
             * メッセージ取得
             */
            const char* getMessage() const;

            /*!
             * メッセージ設定
             */
public:     void setMessage(const char* format, ...);
};

/*!
 * \brief   エラー番号取得
 */
inline int32_t Exception::getErrorNo() const
{
    return mErrorNo;
}

/*!
 * \brief   メッセージ取得
 */
inline const char* Exception::getMessage() const
{
    return mMessage;
}

} // namespace slog
