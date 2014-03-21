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
 *  \file   SHA256.h
 *  \brief  SHA256クラス
 *  \author Copyright 2014 printf.jp
 */
#pragma once
#include "slog/slog.h"

/*!
 * \brief   SHA256クラス
 */
namespace slog
{
class Buffer;

class SLOG_API SHA256
{
            class  Data;
            Data* mData;

            /*!
             * コンストラクタ
             */
public:     SHA256();

            /*!
             * デストラクタ
             */
            ~SHA256();

            /*!
             * ハッシュ計算実行
             */
            void execute(const Buffer* message);

            /*!
             * ハッシュ計算実行
             */
            void execute(const char* message, int32_t size);

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
