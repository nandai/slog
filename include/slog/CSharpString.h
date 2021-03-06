﻿/*
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
 *  \file   CSharpString.h
 *  \brief  C#文字列クラス
 *  \author Copyright 2011-2014 printf.jp
 */
#pragma once

#include "slog/String.h"
#include "slog/Util.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace slog
{

/*!
 * \brief   C#文字列クラス
 */
class CSharpString : public slog::String
{
            /*!
             * コンストラクタ
             */
public:     CSharpString(System::String^ str);

            /*!
             * デストラクタ
             */
            virtual ~CSharpString() override;
};

/*!
 * \brief   コンストラクタ
 */
inline CSharpString::CSharpString(System::String^ str)
{
    IntPtr p = Marshal::StringToHGlobalAnsi(str);
    char* _p = (char*)p.ToPointer();

    Util::shiftJIStoUTF8(this, _p);
    Marshal::FreeHGlobal(p);
}

/*!
 * \brief   デストラクタ
 */
inline CSharpString::~CSharpString()
{
}

} // namespace slog
