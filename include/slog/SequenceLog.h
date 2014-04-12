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
 *  \file   SequenceLog.h
 *  \brief  シーケンスログクラス
 *  \author Copyright 2011-2014 printf.jp
 */
#pragma once

#include "slog/slog.h"
#include <stdarg.h>

#if 1
    #undef DEBUG
    #undef INFO
    #undef WARN
    #undef ERROR
#endif

/******************************************************************************
*
* マクロ定義
*
******************************************************************************/
#if defined(__SLOG__) || defined(PHP_SLOG_H)
    #if defined(__cplusplus)
        #define SLOG     slog::SequenceLog _slog
        #define SMSG    _slog.message
    #endif

//  #define SLOG_STEPIN( className, funcName, outputFlag)   void* _slog = _slog_stepIn( className, funcName, outputFlag)
    #define SLOG_STEPIN( className, funcName)               void* _slog = _slog_stepIn( className, funcName)
//  #define SLOG_STEPIN2(classID,   funcName, outputFlag)   void* _slog = _slog_stepIn2(classID,   funcName, outputFlag)
//  #define SLOG_STEPIN2(classID,   funcName)               void* _slog = _slog_stepIn2(classID,   funcName)
//  #define SLOG_STEPIN3(classID,   funcID,   outputFlag)   void* _slog = _slog_stepIn3(classID,   funcID,   outputFlag)
//  #define SLOG_STEPIN3(classID,   funcID)                 void* _slog = _slog_stepIn3(classID,   funcID)
    #define SLOG_STEPOUT                _slog_stepOut( _slog)
    #define SMSGC( level, format, ...)  _slog_message( _slog, level, format, ##__VA_ARGS__)
//  #define SMSGC2(level, messageID)    _slog_message2(_slog, level, messageID)

#else  // defined(__SLOG__) || defined(PHP_SLOG_H)
    #if defined(__cplusplus)
        #define SLOG
        #define SMSG
    #endif

//  #define SLOG_STEPIN( className, funcName, outputFlag)   void* _slog = 0
    #define SLOG_STEPIN( className, funcName)               void* _slog = 0
//  #define SLOG_STEPIN2(classID,   funcName, outputFlag)   void* _slog = 0
//  #define SLOG_STEPIN2(classID,   funcName)               void* _slog = 0
//  #define SLOG_STEPIN3(classID,   funcID,   outputFlag)   void* _slog = 0
//  #define SLOG_STEPIN3(classID,   funcID)                 void* _slog = 0
    #define SLOG_STEPOUT
    #define SMSGC( level, format, ...)
//  #define SMSGC2(level, messageID)
#endif // defined(__SLOG__) || defined(PHP_SLOG_H)

/******************************************************************************
*
* 定数定義
*
******************************************************************************/
#if defined(__cplusplus)

namespace slog
{
#endif

/*!
 * \brief   ログレベル
 */
enum SequenceLogLevel
{
    DEBUG = 0,              //!< デバッグ
    INFO,                   //!< 情報
    WARN,                   //!< 警告
    ERROR,                  //!< エラー
};

#if defined(__cplusplus)
} // namespace slog
#endif

/******************************************************************************
*
* 関数定義
*
******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#else
typedef int32_t bool;
#endif

SLOG_API void* _slog_stepIn( const char* className, const char* funcName);
//OG_API void* _slog_stepIn3(uint32_t    classID,   uint32_t    funcID);
SLOG_API void  _slog_stepOut(void* p);
//OG_API void  _slog_message( void* p, SequenceLogLevel level, const char* format, ...);
SLOG_API void  _slog_message( void* p, int32_t          level, const char* format, ...);
//OG_API void  _slog_message2(void* p, int32_t          level, uint32_t messageID);

SLOG_API void loadSequenceLogConfig(const char* fileName);

#if defined(JNI_TRUE)
jint slog_JNI_OnLoad(JavaVM* vm, void* reserved);
#endif

#if defined(__cplusplus)
}
#endif

#if defined(__SLOG__) || defined(PHP_SLOG_H)

/******************************************************************************
*
* クラス定義
*
******************************************************************************/
#if defined(__cplusplus)
namespace slog
{

/*!
 *  \brief  シーケンスログクラス
 */
class SLOG_API SequenceLog
{
            /*!
             * シーケンス番号
             */
            uint32_t mSeqNo;

            //
            // コンストラクタ / デストラクタ
            //
public:      SequenceLog(const char* className, const char* funcName);
//           SequenceLog(uint32_t    classID,   uint32_t    funcID);
            ~SequenceLog();

private:    void init();

            //
            // メッセージ出力
            //
public:     void message( SequenceLogLevel level, const char* format, ...);
            void messageV(SequenceLogLevel level, const char* format, va_list arg);
//          void message( SequenceLogLevel level, uint32_t messageID);
};

} // namespace slog
#endif // defined(__cplusplus)
#endif // defined(__SLOG__) || defined(PHP_SLOG_H)
