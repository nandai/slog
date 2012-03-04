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
 *  \file   CSharpString.h
 *  \brief  C#文字列クラス
 *  \author Copyright 2011 log-tools.net
 */
#pragma once
#include "slog/PointerString.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace slog
{

/*!
 *  \brief  C#文字列クラス
 */
class CSharpString : public PointerString
{
public:     CSharpString(String^ str);
            virtual ~CSharpString();
};

/*!
 *  \brief  コンストラクタ
 */
inline CSharpString::CSharpString(String^ str)
{
    IntPtr p = Marshal::StringToHGlobalAnsi(str);
    char* _p = (char*)p.ToPointer();

    init(_p);
}

/*!
 *  \brief  デストラクタ
 */
inline CSharpString::~CSharpString()
{
    IntPtr p(getBuffer());
    Marshal::FreeHGlobal(p);
}

} // namespace slog
