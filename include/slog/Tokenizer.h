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
 *  \file   Tokenizer.h
 *  \brief  文字列分割クラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once
#include "slog/String.h"

namespace slog
{

/*!
 *  \brief  バリアントクラス
 */
class SLOG_API Variant
{
public:     String  mStr;

//blic:     Variant(const Variant& variant)
//          {
//              mStr = variant.mStr;
//          }

public:     operator int32_t() const;
            operator const CoreString&() const {return mStr;}
};

/*!
 *  \brief  文字列分割クラス
 */
class SLOG_API Tokenizer
{
            struct Data;

            Data*       mData;
            Variant     mEmpty;         //!< 空要素
            char        mDelimiter;     //!< デリミタ

            // 初期化／後処理
public:      Tokenizer(const CoreString& format);
             Tokenizer(char delimiter);
            ~Tokenizer();

private:    void cleanUp();

            // 実行
public:     int32_t exec(       const CoreString& str);
private:    int32_t execNamed(  const CoreString& str);
            int32_t execIndexed(const CoreString& str);

            // 取得
public:     const Variant& getValue(const char* key) const;
            const Variant& getValue(int32_t index) const;

            int32_t getCount() const;
};

} // namespace slog
