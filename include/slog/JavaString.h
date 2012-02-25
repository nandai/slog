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
 *  \file   JavaString.h
 *  \brief  Java文字列クラス
 *  \author Copyright 2011 log-tools.net
 */
#pragma once

#include "slog/PointerString.h"
#include <jni.h>

namespace slog
{

/*!
 *  \brief  Java文字列クラス
 */
class JavaString : public PointerString
{
            JNIEnv* mEnv;
            jstring mStr;

public:     JavaString(JNIEnv* env, jstring str);
            virtual ~JavaString();
};

/*!
 *  \brief  コンストラクタ
 */
inline JavaString::JavaString(JNIEnv* env, jstring str)
{
    mEnv = env;
    mStr = str;

    char* p = (char*)mEnv->GetStringUTFChars(mStr, NULL);
    init(p);
}

/*!
 *  \brief  デストラクタ
 */
inline JavaString::~JavaString()
{
    mEnv->ReleaseStringUTFChars(mStr, getBuffer());
}

} // namespace slog
