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
 *  \file   SHA1.h
 *  \brief  SHA1クラス
 *  \author Copyright 2014 printf.jp
 */
#pragma once
#include "slog/slog.h"

/*!
 * \brief   SHA1クラス
 */
namespace slog
{
class CoreString;

class SLOG_API SHA1
{
            class  Data;
            Data* mData;

            /*!
             * コンストラクタ
             */
public:     SHA1();

            /*!
             * デストラクタ
             */
            ~SHA1();

            /*!
             * ハッシュ計算実行
             */
            void execute(const CoreString* message);

            /*!
             * メッセージダイジェスト取得
             */
            const uint8_t* getMessageDigest() const;

            /*!
             * ハッシュサイズ取得
             */
            int32_t getHashSize() const;
};

} // namespace slog
