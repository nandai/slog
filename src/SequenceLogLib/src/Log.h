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
 * \file    Log.h
 * \brief   シーケンスログ（C#）
 * \author  Copyright 2011-2014 printf.jp
 */
#pragma once

#if !defined(MODERN_UI)
using namespace System;
#define sealed
#else
using namespace Platform;
#endif

/*!
 * \brief   Sequence Log (C#)
 */
namespace Slog
{

/*!
 * \brief   シーケンスログクラス
 */
public ref class Log sealed
{
private:    Log() {}

public:     static void LoadConfig(String^ aFileName);

            static int64_t StepIn(String^ aClassName, String^ aFuncName);
//          static int64_t StepIn(int32_t classID,    String^ aFuncName);
//          static int64_t StepIn(int32_t classID,    int32_t funcID);

            static void StepOut(int64_t slog);

private:    static void Message(int32_t level, String^ aMessage,  int64_t slog);
//          static void Message(int32_t level, int32_t messageID, int64_t slog);

public:     static void V(int64_t slog, String^ message)   {Message(slog::DEBUG, message,   slog);}
            static void D(int64_t slog, String^ message)   {Message(slog::DEBUG, message,   slog);}
            static void I(int64_t slog, String^ message)   {Message(slog::INFO,  message,   slog);}
            static void W(int64_t slog, String^ message)   {Message(slog::WARN,  message,   slog);}
            static void E(int64_t slog, String^ message)   {Message(slog::ERROR, message,   slog);}

//          static void V(int64_t slog, int32_t messageID) {Message(slog::DEBUG, messageID, slog);}
//          static void D(int64_t slog, int32_t messageID) {Message(slog::DEBUG, messageID, slog);}
//          static void I(int64_t slog, int32_t messageID) {Message(slog::INFO,  messageID, slog);}
//          static void W(int64_t slog, int32_t messageID) {Message(slog::WARN,  messageID, slog);}
//          static void E(int64_t slog, int32_t messageID) {Message(slog::ERROR, messageID, slog);}
};

} // namespace Slog
