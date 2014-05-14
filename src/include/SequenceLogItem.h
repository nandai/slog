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
 * \file    SequenceLogItem.h
 * \brief   シーケンスログアイテムクラス
 * \author  Copyright 2011-2014 printf.jp
 */
#pragma once

#include "slog/SequenceLog.h"
#include "slog/String.h"
#include "slog/DateTime.h"
#include "slog/ByteBuffer.h"
#include "slog/Thread.h"

namespace slog
{

static const unsigned short SERVICE_PORT = 59106;

#pragma pack(push, 4)
/*!
 * \brief   シーケンスログアイテムクラス
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

            void init(uint32_t seq, const char* className, const char* funcName);
            void init(uint32_t seq, uint32_t    classID,   const char* funcName);
            void init(uint32_t seq, uint32_t    classID,   uint32_t    funcID);
            void init(uint32_t seq);
            void init(uint32_t seq, SequenceLogLevel level);

            CoreString* getClassName() const;
            CoreString* getFuncName() const;
            CoreString* getMessage() const;
};
#pragma pack(pop)

/*!
 * \brief   コンストラクタ
 */
inline SequenceLogItemCore::SequenceLogItemCore()
{
    mThreadId = 0;

    mClassId = 0;
    mFuncId = 0;
    mMessageId = 0;
}

/*!
 * \brief   コンストラクタ
 */
inline SequenceLogItem::SequenceLogItem() : SequenceLogItemCore()
{
    mPrev = 0;
    mNext = 0;
}

/*!
 * \brief   初期化
 *
 * \param[in]   seq         シーケンス番号
 * \param[in]   className   クラス名
 * \param[in]   funcName    メソッド名
 *
 * \return  なし
 */
inline void SequenceLogItem::init(uint32_t seq, const char* className, const char* funcName)
{
    if (this == nullptr)
        return;

    mSeqNo =      seq;
    mType =       STEP_IN;
//  mThreadId =   Thread::getCurrentId();
    mClassId =    0;
    mClassName.copy(className);
    mFuncId =     0;
    mFuncName. copy(funcName);
}

/*!
 * \brief   初期化
 *
 * \param[in]   seq         シーケンス番号
 * \param[in]   classID     クラスID
 * \param[in]   funcName    メソッド名
 *
 * \return  なし
 */
inline void SequenceLogItem::init(uint32_t seq, uint32_t classID, const char* funcName)
{
    if (this == nullptr)
        return;

    mSeqNo =      seq;
    mType =       STEP_IN;
//  mThreadId =   Thread::getCurrentId();
    mClassId =    classID;
    mFuncId =     0;
    mFuncName.copy(funcName);
}

/*!
 * \brief   初期化
 *
 * \param[in]   seq         シーケンス番号
 * \param[in]   classID     クラス名
 * \param[in]   funcID      メソッド名
 *
 * \return  なし
 */
inline void SequenceLogItem::init(uint32_t seq, uint32_t classID, uint32_t funcID)
{
    if (this == nullptr)
        return;

    mSeqNo =      seq;
    mType =       STEP_IN;
//  mThreadId =   Thread::getCurrentId();
    mClassId =    classID;
    mFuncId =     funcID;
}

/*!
 * \brief   初期化
 *
 * \param[in]   seq         シーケンス番号
 *
 * \return  なし
 */
inline void SequenceLogItem::init(uint32_t seq)
{
    if (this == nullptr)
        return;

    mSeqNo =      seq;
    mType =       STEP_OUT;
//  mThreadId =   Thread::getCurrentId();
}

/*!
 * \brief   初期化
 *
 * \param[in]   seq         シーケンス番号
 * \param[in]   level       ログレベル
 *
 * \return  なし
 */
inline void SequenceLogItem::init(uint32_t seq, SequenceLogLevel level)
{
    if (this == nullptr)
        return;

    mSeqNo =      seq;
    mType =       MESSAGE;
//  mThreadId =   Thread::getCurrentId();
    mLevel =      level;
}

/*!
 * \brief   現在日時設定
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
 * \brief   シーケンスログバイトバッファクラス
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
            uint32_t putSequenceLogItem(const SequenceLogItem* item, bool isOutputDateTime = false);
};

} // namespace slog
