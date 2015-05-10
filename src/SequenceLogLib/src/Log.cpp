/*
 * Copyright (C) 2011-2015 printf.jp
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
 * \author  Copyright 2011-2015 printf.jp
 */
#include "slog/SequenceLog.h"
#include "slog/CSharpString.h"

#include "Log.h"

namespace Slog
{

/*!
 * \brief   シーケンスログコンフィグを読み込む
 */
void Log::LoadConfig(String^ aFileName)
{
    slog::CSharpString fileName = aFileName;
    loadSequenceLogConfig(fileName.getBuffer());
}

/*!
 * \brief   ステップイン
 */
int64_t Log::StepIn(String^ aClassName, String^ aFuncName)
{
    slog::CSharpString className = aClassName;
    slog::CSharpString funcName = aFuncName;

    slog::SequenceLog* slogObj = new slog::SequenceLog(className.getBuffer(), funcName.getBuffer());
    return (int64_t)slogObj;
}

/*!
 * \brief   ステップイン
 */
//int64_t Log::StepIn(int32_t classID, String^ aFuncName)
//{
//    slog::CSharpString funcName = aFuncName;
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
    slog::CSharpString message = aMessage;

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
    slog::CSharpString assertName = aAssertName;

    slogObj->assert(assertName.getBuffer(), result.Equals(true));
}

} // namespace Slog
