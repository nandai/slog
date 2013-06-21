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
 *  \file   SequenceLogItem.h
 *  \brief  シーケンスログアイテムクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once

#include "slog/SequenceLog.h"
#include "slog/String.h"
#include "slog/DateTime.h"
#include "slog/ByteBuffer.h"
#include "slog/Thread.h"

#include <stdio.h>

#if defined(_WINDOWS)
    #undef UNICODE
    #pragma warning(disable:4996)
#endif

namespace slog
{

static const unsigned short SERVICE_PORT = 59106;

#pragma pack(push, 4)
/*!
 *  \brief  シーケンスログアイテムクラス
 */
class SequenceLogItemCore
{
public:     enum Type
            {
                STEP_IN,        //!< ステップイン
                STEP_OUT,       //!< ステップアウト
                MESSAGE,        //!< メッセージ
            };

            //
            // STEP_IN, STEP_OUT, MESSAGE
            //
public:     uint32_t                    mSeqNo;             //!< シーケンス番号
            DateTime                    mDateTime;          //!< ログ出力日時
//          Type                        mType;
            uint32_t                    mType;              //!< タイプ
            uint32_t                    mThreadId;          //!< スレッドID

            //
            // STEP_IN
            //
public:     uint32_t                    mClassId;           //!< クラスID

public:     uint32_t                    mFuncId;            //!< メソッドID

            //
            // STEP_IN, STEP_OUT, MESSAGE
            //
//blic:     SequenceLogOutputFlag       mOutputFlag;
public:     uint32_t                    mOutputFlag;        //!< 出力フラグ

            //
            // MESSAGE
            //
//blic:     SequenceLogLevel            mLevel;
public:     uint32_t                    mLevel;             //!< ログレベル
            uint32_t                    mMessageId;         //!< メッセージID

            //
            // コンストラクタ
            //
public:     SequenceLogItemCore();
//          SequenceLogItemCore(const SequenceLogItem&);

public:     void setCurrentDateTime();
};

class SequenceLogItem : public SequenceLogItemCore
{
            String                      mClassName;         //!< クラス名
            String                      mFuncName;          //!< メソッド名
            String                      mMessage;           //!< メッセージ

//          SequenceLogItem*            mPrev;
//          SequenceLogItem*            mNext;
public:     uint64_t                    mPrev;              //!< 前のシーケンスログアイテム
            uint64_t                    mNext;              //!< 次のシーケンスログアイテム

public:     SequenceLogItem();

            void init(uint32_t seq, uint32_t outputFlag, const char* className, const char* funcName);
            void init(uint32_t seq, uint32_t outputFlag, uint32_t    classID,   const char* funcName);
            void init(uint32_t seq, uint32_t outputFlag, uint32_t    classID,   uint32_t    funcID);
            void init(uint32_t seq, uint32_t outputFlag);
            void init(uint32_t seq, uint32_t outputFlag, SequenceLogLevel level);

            CoreString* getClassName() const;
            CoreString* getFuncName() const;
            CoreString* getMessage() const;
};
#pragma pack(pop)

/*!
 *  \brief  コンストラクタ
 */
inline SequenceLogItemCore::SequenceLogItemCore()
{
    mThreadId = 0;

    mClassId = 0;
    mFuncId = 0;
    mMessageId = 0;
}

/*!
 *  \brief  コンストラクタ
 */
inline SequenceLogItem::SequenceLogItem() : SequenceLogItemCore()
{
    mPrev = 0;
    mNext = 0;
}

/*!
 *  \brief  初期化
 */
inline void SequenceLogItem::init(
    uint32_t seq,                       //!< シーケンス番号
//  SequenceLogOutputFlag outputFlag,
    uint32_t              outputFlag,   //!< 出力フラグ
    const char* className,              //!< クラス名
    const char* funcName)               //!< メソッド名
{
    if (this == NULL)
        return;

    mSeqNo =      seq;
    mType =       STEP_IN;
//  mThreadId =   Thread::getCurrentId();
    mClassId =    0;
    mClassName.copy(className);
    mFuncId =     0;
    mFuncName. copy(funcName);
    mOutputFlag = outputFlag;
}

/*!
 *  \brief  初期化
 */
inline void SequenceLogItem::init(
    uint32_t seq,                       //!< シーケンス番号
//  SequenceLogOutputFlag outputFlag,
    uint32_t              outputFlag,   //!< 出力フラグ
    uint32_t classID,                   //!< クラスID
    const char* funcName)               //!< メソッド名
{
    if (this == NULL)
        return;

    mSeqNo =      seq;
    mType =       STEP_IN;
//  mThreadId =   Thread::getCurrentId();
    mClassId =    classID;
    mFuncId =     0;
    mFuncName.copy(funcName);
    mOutputFlag = outputFlag;
}

/*!
 *  \brief  初期化
 */
inline void SequenceLogItem::init(
    uint32_t seq,                       //!< シーケンス番号
//  SequenceLogOutputFlag outputFlag,
    uint32_t              outputFlag,   //!< 出力フラグ
    uint32_t classID,                   //!< クラスID
    uint32_t funcID)                    //!< メソッドID
{
    if (this == NULL)
        return;

    mSeqNo =      seq;
    mType =       STEP_IN;
//  mThreadId =   Thread::getCurrentId();
    mClassId =    classID;
    mFuncId =     funcID;
    mOutputFlag = outputFlag;
}

/*!
 *  \brief  初期化
 */
inline void SequenceLogItem::init(
    uint32_t seq,                       //!< シーケンス番号
//  SequenceLogOutputFlag outputFlag)
    uint32_t              outputFlag)   //!< 出力フラグ
{
    if (this == NULL)
        return;

    mSeqNo =      seq;
    mType =       STEP_OUT;
//  mThreadId =   Thread::getCurrentId();
    mOutputFlag = outputFlag;
}

/*!
 *  \brief  初期化
 */
inline void SequenceLogItem::init(
    uint32_t seq,                       //!< シーケンス番号
//  SequenceLogOutputFlag outputFlag,
    uint32_t              outputFlag,   //!< 出力フラグ
    SequenceLogLevel level)             //!< ログレベル
{
    if (this == NULL)
        return;

    mSeqNo =      seq;
    mType =       MESSAGE;
//  mThreadId =   Thread::getCurrentId();
    mOutputFlag = outputFlag;
    mLevel =      level;
}

/*!
 *  \brief  現在日時設定
 */
inline void SequenceLogItemCore::setCurrentDateTime()
{
    mDateTime.setCurrent();
}

inline CoreString* SequenceLogItem::getClassName() const {return (CoreString*)&mClassName;}
inline CoreString* SequenceLogItem::getFuncName()  const {return (CoreString*)&mFuncName;}
inline CoreString* SequenceLogItem::getMessage()   const {return (CoreString*)&mMessage;}

#pragma pack(push, 4)
struct SLOG_SHM
{
    uint32_t        seq;        //!< 次に取得するシーケンス番号
    SequenceLogItem item;       //!< シーケンスログアイテム
};
#pragma pack(pop)

/*!
 *  \brief  シーケンスログバイトバッファクラス
 */
class SLOG_API SequenceLogByteBuffer : public ByteBuffer
{
            /*!
             * コンストラクタ
             */
public:     SequenceLogByteBuffer(uint32_t capacity) : ByteBuffer(capacity) {}

            /*!
             * シーケンスログ読み込み／書き込み
             */
            void     getSequenceLogItem(      SequenceLogItem* item) throw(Exception);
            uint32_t putSequenceLogItem(const SequenceLogItem* item, bool enableOutputFlag);
};

} // namespace slog
