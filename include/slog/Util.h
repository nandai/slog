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
 *  \file   Util.h
 *  \brief  ユーティリティクラス
 *  \author Copyright 2011-2014 printf.jp
 */
#pragma once
#include "slog/slog.h"

namespace slog
{
class CoreString;

/*!
 *  \brief  ユーティリティクラス
 */
class SLOG_API Util
{
            /*!
             * プロセスの実行ファイルパスを取得
             */
public:     static void getProcessPath(CoreString* path);

            /*!
             * ビット指定で値を取得
             */
            static int64_t getBitsValue(const char* p, int32_t len, int32_t bitPos, int32_t count);

            /*!
             * Base64エンコード
             */
            static void encodeBase64(CoreString* dest, const char* src, int32_t srcLen);

            /*!
             * メールアドレスを検証
             */
            static bool validateMailAddress(const CoreString* mailAddress);

#if defined(_WINDOWS)
            /*!
             * UnicodeをUTF-8に変換
             */
            static int32_t toUTF8(char* utf8, int32_t size, const wchar_t* unicode);

            /*!
             * UTF-8をUnicodeに変換
             */
            static int32_t toUnicode(wchar_t* unicode, int32_t size, const char* utf8);
#endif
};

} // namespace slog
