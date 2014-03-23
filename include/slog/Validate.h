/*
 * Copyright (C) 2014 printf.jp
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
 * \file    Validate.h
 * \brief   検証クラス
 * \author  Copyright 2014 printf.jp
 */
#pragma once

#include "slog/slog.h"
#include <list>

#pragma warning(disable:4251)

namespace slog
{
class CoreString;

/*!
 * \brief   検証クラス
 */
class SLOG_API Validate
{
            /*!
             * 検証結果クラス
             */
public:     class Result {};
            class Success  : public Result {};
            class Invalid  : public Result {};
            class Empty    : public Result {};
            class TooShort : public Result {};
            class TooLong  : public Result {};

            /*!
             * \brief   検証結果
             */
            static const Success*  SUCCESS;
            static const Invalid*  INVALID;
            static const Empty*    EMPTY;
            static const TooShort* TOO_SHORT;
            static const TooLong*  TOO_LONG;

            /*!
             * コンストラクタ
             */
public:     Validate() {}

            /*!
             * 検証実行
             */
            virtual const Result* execute() const = 0;

            /*!
             * 値のアドレスを取得
             */
            virtual void* getValueAddress() const = 0;
};

/*!
 * \brief   文字列検証クラス
 */
class SLOG_API StringValidate : public Validate
{
            /*!
             * 値
             */
            const CoreString* mValue;

            /*!
             * 最小文字数
             */
            int32_t mMinLen;

            /*!
             * 最大文字数
             */
            int32_t mMaxLen;

            /*!
             * コンストラクタ
             */
public:     StringValidate(const CoreString* value, int32_t minLen, int32_t maxLen);

            /*!
             * 検証実行
             */
            virtual const Result* execute() const override;

            /*!
             * 値のアドレスを取得
             */
            virtual void* getValueAddress() const override;
};

/*!
 * \brief   メールアドレス検証クラス
 */
class SLOG_API MailAddresValidate : public Validate
{
            /*!
             * 値
             */
            const CoreString* mValue;

            /*!
             * コンストラクタ
             */
public:     MailAddresValidate(const CoreString* value);

            /*!
             * 検証実行
             */
            virtual const Result* execute() const override;

            /*!
             * 値のアドレスを取得
             */
            virtual void* getValueAddress() const override;
};

/*!
 * 検証リスナークラス
 */
class SLOG_API ValidateListener
{
            /*!
             * 検証失敗イベント
             */
public:     virtual void onInvalid(const void* value, const Validate::Result* result) = 0;
};

/*!
 * 検証リスト
 */
class SLOG_API ValidateList
{
            std::list<Validate*> mList;

            /*!
             * デストラクタ
             */
public:     ~ValidateList();

            /*!
             * 検証オブジェクト追加
             */
            void add(Validate* validate);

            /*!
             * 検証実行
             */
            bool execute(ValidateListener* listener);
};

} // namespace slog
