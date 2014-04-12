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
 *  \file   SequenceLogService.h
 *  \brief  シーケンスログサービスクラス
 *  \author Copyright 2011-2014 printf.jp
 */
#pragma once

#include "SequenceLogItem.h"

#include "slog/WebServerResponseThread.h"
#include "slog/File.h"
#include "slog/Process.h"
#include "slog/FixedString.h"

#include <map>

namespace slog
{
class FileInfo;
class ItemQueue;
class ItemList;
class SharedFileContainer;

typedef std::map<uint32_t, ItemQueue*> ItemQueueManager;        // キーはスレッドID

/*!
 *  \brief  シーケンスログサービスクラス
 */
class SequenceLogService : public WebServerResponse
{
            /*!
             * ログバッファ（旧 - 共有メモリ）
             */
private:    SLOG_SHM* mSHM;

            /*!
             * プロセスオブジェクト
             */
            Process mProcess;

            /*!
             * シーケンスログ出力リスト
             */
            ItemList* mOutputList;

            /*!
             * ファイル出力用バッファ
             */
            SequenceLogByteBuffer mFileOutputBuffer;

            /*!
             * ログレベル
             */
            int32_t mLogLevel;

            /*!
             * シーケンスログファイルタイプ
             */
            bool mBinaryLog;

            /*!
             * シーケンスログアイテムキューマネージャー
             */
            ItemQueueManager* mItemQueueManager;

            /*!
             * 未使用シーケンスログアイテムのストック
             */
            SequenceLogItem* mStockItems;

            /*!
             * シーケンスログ共有ファイルコンテナ
             */
            SharedFileContainer* mSharedFileContainer;

            /*!
             * コンストラクタ
             */
public:     SequenceLogService(HttpRequest* httpRequest);

            /*!
             * デストラクタ
             */
            virtual ~SequenceLogService() override;

            /*!
             * 初期化
             */
private:    virtual bool init() override;

            /*!
             * シーケンスログファイル情報取得
             */
public:     FileInfo* getFileInfo() const;

            /*!
             * シーケンスログスレッド関連
             */
private:    virtual void run() override;
            void writeMain();
            void callLogFileChanged();
            void cleanUp();

            /*!
             * シーケンスログアイテムキープ / 追加
             */
private:    void divideItems();

            void keep(   ItemQueue* queue, SequenceLogItem* item);
            void forward(ItemQueue* queue, SequenceLogItem* item);

            ItemQueue* getItemQueue(const SequenceLogItem& item) const;
            SequenceLogItem* createSequenceLogItem(ItemQueue* queue, const SequenceLogItem& src);

            void pushStockItem(SequenceLogItem* item);
            SequenceLogItem* popStockItem();

            /*!
             * シーケンスログファイル関連
             */
private:    const char* initBinaryOrText(CoreString* fileName);

            void  openSeqLogFile(    File& file) throw(Exception);
            void writeSeqLogFile(    File& file, SequenceLogItem*);
            void writeSeqLogFileText(File& file, SequenceLogItem*);

            /*!
             * 受信メイン
             */
public:     void receiveMain();
};

/*!
 * \brief   シーケンスログアイテムをストックに積む
 */
inline void SequenceLogService::pushStockItem(SequenceLogItem* item)
{
    item->mNext = (uint64_t)mStockItems;
    mStockItems = item;
}

/*!
 * \brief   シーケンスログアイテムをストックから取り出す
 */
inline SequenceLogItem* SequenceLogService::popStockItem()
{
    if (mStockItems == nullptr)
        mStockItems = new SequenceLogItem;

    SequenceLogItem* result = mStockItems;
    mStockItems = (SequenceLogItem*)mStockItems->mNext;
    return result;
}

} // namespace slog
