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

#if !defined(MODERN_UI)
#include "slog/CSharpString.h"
#else
#include "slog/String.h"
#endif

#include "Log.h"

namespace Slog
{

/*!
 *  \brief  シーケンスログファイル名を設定する
 */
void Log::SetFileName(String^ aName)
{
#if !defined(MODERN_UI)
    slog::CSharpString name = aName;
#else
    slog::String name;
    name.conv(aName->Data());
#endif
    setSequenceLogFileName(name.getBuffer());
}

/*!
 *  \brief  シーケンスログサービスのアドレスを設定する
 */
void Log::SetServiceAddress(String^ aAddress)
{
#if !defined(MODERN_UI)
    slog::CSharpString address = aAddress;
#else
    slog::String address;
    address.conv(aAddress->Data());
#endif
    setSequenceLogServiceAddress(address.getBuffer());
}

/*!
 *  \brief  ROOTの既定値を設定する
 */
//void Log::SetRootFlag(int32_t outputFlag)
//{
//    setRootFlag(outputFlag);
//}

void Log::EnableOutput(bool enable)
{
    enableOutput(enable);
}

/*!
 *  \brief  ステップイン
 */
int64_t Log::StepIn(String^ aClassName, String^ aFuncName)//, int32_t outputFlag)
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

    slog::SequenceLog* slogObj = new slog::SequenceLog(className.getBuffer(), funcName.getBuffer());//, (slog::SequenceLogOutputFlag)outputFlag);
    return (int64_t)slogObj;
}

/*!
 *  \brief  ステップイン
 */
int64_t Log::StepIn(int32_t classID, String^ aFuncName)//, int32_t outputFlag)
{
#if !defined(MODERN_UI)
    slog::CSharpString funcName = aFuncName;
#else
    slog::String funcName;
    funcName.conv(aFuncName->Data());
#endif

    slog::SequenceLog* slogObj = new slog::SequenceLog(classID, funcName.getBuffer());//, (slog::SequenceLogOutputFlag)outputFlag);
    return (int64_t)slogObj;
}

/*!
 *  \brief  ステップイン
 */
int64_t Log::StepIn(int32_t classID, int32_t funcID)//, int32_t outputFlag)
{
    slog::SequenceLog* slogObj = new slog::SequenceLog(classID, funcID);//, (slog::SequenceLogOutputFlag)outputFlag);
    return (int64_t)slogObj;
}

/*!
 *  \brief  ステップアウト
 */
void Log::StepOut(int64_t slog)
{
    slog::SequenceLog* slogObj = (slog::SequenceLog*)slog;
    delete slogObj;
}

/*!
 *  \brief  メッセージ
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
 *  \brief  メッセージ
 */
void Log::Message(int32_t level, int32_t messageID, int64_t slog)
{
    slog::SequenceLog* slogObj = (slog::SequenceLog*)slog;
    slogObj->message((slog::SequenceLogLevel)level, messageID);
}

} // namespace Slog
