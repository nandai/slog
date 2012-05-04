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
 *  \file   Log.cpp
 *  \brief  シーケンスログ（C#）
 *  \author Copyright 2011 log-tools.net
 */
#include "slog/SequenceLog.h"
#include "slog/CSharpString.h"

#include "Log.h"

using namespace slog;
using namespace System::Runtime::InteropServices;

namespace Slog
{

/*!
 *  \brief  シーケンスログファイル名設定
 */
void Log::SetFileName(String^ aName)
{
    CSharpString name = aName;
    setSequenceLogFileName(name.getBuffer());
}

/*!
 *  \brief  ROOTの既定値を設定する
 */
void Log::SetRootFlag(int32_t outputFlag)
{
    setRootFlag(outputFlag);
}

/*!
 *  \brief  ステップイン
 */
int64_t Log::StepIn(String^ aClassName, String^ aFuncName, int32_t outputFlag)
{
    CSharpString className = aClassName;
    CSharpString funcName = aFuncName;

    SequenceLog* slogObj = new SequenceLog(className.getBuffer(), funcName.getBuffer(), (SequenceLogOutputFlag)outputFlag);
    return (int64_t)slogObj;
}

/*!
 *  \brief  ステップイン
 */
int64_t Log::StepIn(int32_t classID, String^ aFuncName, int32_t outputFlag)
{
    CSharpString funcName = aFuncName;

    SequenceLog* slogObj = new SequenceLog(classID, funcName.getBuffer(), (SequenceLogOutputFlag)outputFlag);
    return (int64_t)slogObj;
}

/*!
 *  \brief  ステップイン
 */
int64_t Log::StepIn(int32_t classID, int32_t funcID, int32_t outputFlag)
{
    SequenceLog* slogObj = new SequenceLog(classID, funcID, (SequenceLogOutputFlag)outputFlag);
    return (int64_t)slogObj;
}

/*!
 *  \brief  ステップアウト
 */
void Log::StepOut(int64_t slog)
{
    SequenceLog* slogObj = (SequenceLog*)slog;
    delete slogObj;
}

/*!
 *  \brief  メッセージ
 */
void Log::Message(int32_t level, String^ aMessage, int64_t slog)
{
    SequenceLog* slogObj = (SequenceLog*)slog;
    CSharpString message = aMessage;
    slogObj->message((SequenceLogLevel)level, "%s", message.getBuffer());
}

/*!
 *  \brief  メッセージ
 */
void Log::Message(int32_t level, int32_t messageID, int64_t slog)
{
    SequenceLog* slogObj = (SequenceLog*)slog;
    slogObj->message((SequenceLogLevel)level, messageID);
}

}

/*!
 *  \brief  シーケンスログファイル名取得
 */
//const char* getSequenceLogFileName()
//{
//    return NULL;
//}
