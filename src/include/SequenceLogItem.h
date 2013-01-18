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
#include "slog/PointerString.h"
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
            char                        mClassName[256];    //!< クラス名
            char                        mFuncName [256];    //!< メソッド名
            char                        mMessage  [256];    //!< メッセージ

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

            PointerString getClassName() const;
            PointerString getFuncName() const;
            PointerString getMessage() const;
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
    mClassName[0] = '\0';
    mFuncName[0] = '\0';
    mMessage[0] = '\0';

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

    PointerString _ClassName(mClassName, sizeof(mClassName) - 1);
    PointerString _FuncName( mFuncName,  sizeof(mFuncName)  - 1);

    mSeqNo =      seq;
    mType =       STEP_IN;
//  mThreadId =   Thread::getCurrentId();
    mClassId =    0;
    _ClassName.copy(className);
    mFuncId =     0;
    _FuncName. copy(funcName);
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

    PointerString _FuncName(mFuncName, sizeof(mFuncName) - 1);

    mSeqNo =      seq;
    mType =       STEP_IN;
//  mThreadId =   Thread::getCurrentId();
    mClassId =    classID;
    mFuncId =     0;
    _FuncName.copy(funcName);
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

inline PointerString SequenceLogItem::getClassName() const {return PointerString((char*)mClassName, sizeof(mClassName) - 1);}
inline PointerString SequenceLogItem::getFuncName()  const {return PointerString((char*)mFuncName,  sizeof(mFuncName)  - 1);}
inline PointerString SequenceLogItem::getMessage()   const {return PointerString((char*)mMessage,   sizeof(mMessage)   - 1);}

#pragma pack(push, 4)
/*!
 *  \brief  シーケンスログ共有データ
 */
struct SLOG_ITEM_INFO
{
    SequenceLogItem item;                   //!< シーケンスログアイテム
    bool            ready;                  //!< 準備完了フラグ
    uint32_t        no;                     //!< 配列№
};

struct SLOG_SHM_HEADER
{
#if defined(__unix__)
    pthread_mutex_t mutex;                  //!< 同期オブジェクト
#endif

    uint32_t        seq;                    //!< 次に取得するシーケンス番号
    uint32_t        index;                  //!< 次に書き込むシーケンスログアイテムのインデックス（シーケンスログサービスによってログが取り込まれたら0に戻る）
    uint32_t        max;                    //!< 最大index（デバッグ用）
};

struct SLOG_SHM
{
    enum {BUFFER_COUNT = 3};                //!< WindowsのスレッドIDは偶数しかないようだ。2では意味がないため3にする。

    SLOG_SHM_HEADER header[BUFFER_COUNT];   //!< 共有メモリヘッダー
    uint32_t        count;                  //!< シーケンスログアイテム情報の配列数
    SLOG_ITEM_INFO  infoArray[1];           //!< シーケンスログアイテム情報の配列（count * BUFFER_COUNT個）
};
#pragma pack(pop)

/*!
 *  \brief  シーケンスログバイトバッファクラス
 */
class SequenceLogByteBuffer : public ByteBuffer
{
public:     SequenceLogByteBuffer(uint32_t capacity);

//          void     getSequenceLogItem(      SequenceLogItem* item);
            uint32_t putSequenceLogItem(const SequenceLogItem* item);
};

/*!
 *  \brief  コンストラクタ
 */
inline SequenceLogByteBuffer::SequenceLogByteBuffer(uint32_t capacity) : ByteBuffer(capacity)
{
}

/*!
 *  \brief  シーケンスログアイテム取得
 */
//inline void SequenceLogByteBuffer::getSequenceLogItem(SequenceLogItem* item)
//{
//  setPosition(0);
//
//  // シーケンス番号
//  uint32_t seq = getInt();
//  item->mSeqNo = seq;
//
//  // 日時
//  uint64_t datetime = getLong();
//  item->mDateTime.setValue(datetime);
//
//  // シーケンスログアイテム種別
//  SequenceLogItem::Type type = (SequenceLogItem::Type)get();
//  item->mType = type;
//
//  // ID
//  uint32_t threadId = getInt();
//  item->mThreadId = threadId;
//
//  switch (type)
//  {
//  case SequenceLogItem::STEP_IN:
//  {
//      // クラス名
//      uint32_t ID = getInt();
//      item->mClassId = ID;
//
//      if (ID == 0)
//      {
//          short classLen = getShort();
//          PointerString _ClassName = item->getClassName();
//
//          _ClassName.copy(get(classLen), classLen);
//      }
//
//      // 関数名
//      ID = getInt();
//      item->mFuncId = ID;
//
//      if (ID == 0)
//      {
//          short funcLen = getShort();
//          PointerString _FuncName = item->getFuncName();
//
//          _FuncName.copy(get(funcLen), funcLen);
//      }
//
//      break;
//  }
//
//  case SequenceLogItem::STEP_OUT:
//      break;
//
//  case SequenceLogItem::MESSAGE:
//  {
//      // メッセージ
//      SequenceLogLevel level = (SequenceLogLevel)get();
//      item->mLevel = level;
//
//      uint32_t ID = getInt();
//      item->mMessageId = ID;
//
//      if (ID == 0)
//      {
//          short msgLen = getShort();
//          PointerString _Message = item->getMessage();
//
//          _Message.copy(get(msgLen), msgLen);
//      }
//
//      break;
//  }
//
//  default:
//      break;
//  }
//}

/*!
 *  \brief  シーケンスログアイテム書き込み
 */
inline uint32_t SequenceLogByteBuffer::putSequenceLogItem(const SequenceLogItem* item)
{
    unsigned short size;
    int32_t len;

    setPosition(sizeof(size));

    // シーケンス番号
    putInt(item->mSeqNo);

    // 日時
    putLong(item->mDateTime.getValue());

    // シーケンスログアイテム種別
    put(item->mType);

    // スレッド ID
    putInt(item->mThreadId);

    switch (item->mType)
    {
    case SequenceLogItem::STEP_IN:
        // クラス名
        putInt(item->mClassId);

        if (item->mClassId == 0)
        {
            PointerString _ClassName = item->getClassName();
            len = _ClassName.getLength();

            putShort(len);
            put(&_ClassName, len);
        }

        // 関数名
        putInt(item->mFuncId);

        if (item->mFuncId == 0)
        {
            PointerString _FuncName = item->getFuncName();
            len = _FuncName.getLength();

            putShort(len);
            put(&_FuncName, len);
        }

        break;

    case SequenceLogItem::STEP_OUT:
        break;

    case SequenceLogItem::MESSAGE:
    {
        // メッセージ
        put(item->mLevel);
        putInt(item->mMessageId);

        if (item->mMessageId == 0)
        {
            PointerString _Message = item->getMessage();
            len = _Message.getLength();

            putShort(len);
            put(&_Message, len);
        }
        break;
    }

    default:
        break;
    }

    // 先頭にレコード長
    size = getPosition();

    setPosition(0);
    putShort(size);

    return size;
}

} // namespace slog
