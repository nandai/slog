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
 * \file    Log.cpp
 * \brief   シーケンスログ（C#）
 * \author  Copyright 2011-2014 printf.jp
 */
#include "slog/SequenceLog.h"

#if !defined(MODERN_UI)
#include "slog/CSharpString.h"
#else
#include "slog/String.h"
#endif

#include "Log.h"

namespace Slog
{

/*!
 * \brief   シーケンスログコンフィグを読み込む
 */
void Log::LoadConfig(String^ aFileName)
{
#if !defined(MODERN_UI)
    slog::CSharpString fileName = aFileName;
#else
    slog::String fileName;
    fileName.conv(aFileName->getBuffer());
#endif
    loadSequenceLogConfig(fileName.getBuffer());
}

/*!
 * \brief   ステップイン
 */
int64_t Log::StepIn(String^ aClassName, String^ aFuncName)
{
#if !defined(MODERN_UI)
    slog::CSharpString className = aClassName;
    slog::CSharpString funcName = aFuncName;
#else
    slog::String className;
    slog::String funcName;

    className.conv(aClassName->Data());
    funcName. conv(aFuncName-> Data());
#endif

    slog::SequenceLog* slogObj = new slog::SequenceLog(className.getBuffer(), funcName.getBuffer());
    return (int64_t)slogObj;
}

/*!
 * \brief   ステップイン
 */
//int64_t Log::StepIn(int32_t classID, String^ aFuncName)
//{
//#if !defined(MODERN_UI)
//    slog::CSharpString funcName = aFuncName;
//#else
//    slog::String funcName;
//    funcName.conv(aFuncName->Data());
//#endif
//
//    slog::SequenceLog* slogObj = new slog::SequenceLog(classID, funcName.getBuffer());
//    return (int64_t)slogObj;
//}

/*!
 * \brief   ステップイン
 */
//int64_t Log::StepIn(int32_t classID, int32_t funcID)
//{
//    slog::SequenceLog* slogObj = new slog::SequenceLog(classID, funcID);
//    return (int64_t)slogObj;
//}

/*!
 * \brief   ステップアウト
 */
void Log::StepOut(int64_t slog)
{
    slog::SequenceLog* slogObj = (slog::SequenceLog*)slog;
    delete slogObj;
}

/*!
 * \brief   メッセージ
 */
void Log::Message(int32_t level, String^ aMessage, int64_t slog)
{
    slog::SequenceLog* slogObj = (slog::SequenceLog*)slog;

#if !defined(MODERN_UI)
    slog::CSharpString message = aMessage;
#else
    slog::String message;
    message.conv(aMessage->Data());
#endif

    slogObj->message((slog::SequenceLogLevel)level, "%s", message.getBuffer());
}

/*!
 * \brief   メッセージ
 */
//void Log::Message(int32_t level, int32_t messageID, int64_t slog)
//{
//    slog::SequenceLog* slogObj = (slog::SequenceLog*)slog;
//    slogObj->message((slog::SequenceLogLevel)level, messageID);
//}

void Log::Assert(int64_t slog, String^ aAssertName, Boolean result)
{
    slog::SequenceLog* slogObj = (slog::SequenceLog*)slog;

#if !defined(MODERN_UI)
    slog::CSharpString assertName = aAssertName;
#else
    slog::String assertName;
    assertName.conv(aAssertName->Data());
#endif

    slogObj->assert(assertName.getBuffer(), result.Equals(true));
}

} // namespace Slog
