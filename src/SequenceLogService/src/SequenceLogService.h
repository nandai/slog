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
 *  \file	SequenceLogService.h
 *  \brief	シーケンスログサービスクラス
 *  \author	Copyright 2011 log-tools.net
 */
#pragma once

#include "SequenceLogItem.h"

#include "slog/SharedMemory.h"
#include "slog/Thread.h"
#include "slog/File.h"
#include "slog/Process.h"
#include "slog/FixedString.h"

#include <map>

namespace slog
{
class Socket;
class Mutex;
class FileInfo;
class ItemQueue;
class ItemList;

typedef	std::map<uint32_t, ItemQueue*> ItemQueueManager;		// キーはスレッドID

/*!
 *  \brief	シーケンスログサービスクラス
 */
class SequenceLogService : public Thread
{
private:	Socket*					mSocket;					//!< シーケンスログクライアントとの接続用ソケット
            Mutex*					mMutex[SLOG_SHM::BUFFER_COUNT]; //!< ミューテックス
			SharedMemory<SLOG_SHM*>	mSHM;						//!< 共有メモリ

			Process					mProcess;					//!< プロセスオブジェクト

			ItemList*				mOutputList;				//!< シーケンスログ出力リスト

			SequenceLogByteBuffer	mFileOutputBuffer;			//!< ファイル出力用バッファ
			bool					mBinaryLog;					//!< シーケンスログファイルタイプ

			ItemQueueManager*		mItemQueueManager;			//!< シーケンスログアイテムキューマネージャー

			File					mFile;						//!< シーケンスログファイル
			FixedString<MAX_PATH>	mBaseFileName;				//!< シーケンスログベースファイル名
			FileInfo*				mFileInfo;					//!< シーケンスログファイル情報

			// 初期化 / 破棄
public:		 SequenceLogService(Socket* socket);
			~SequenceLogService();

private:	virtual bool init();

			// 取得
public:		FileInfo* getFileInfo() const {return mFileInfo;}	//!< シーケンスログファイル情報取得

			// シーケンスログスレッド
private:	virtual void run();
			void cleanUp();

			// シーケンスログアイテムキープ / 追加
private:	void divideItems();

			void keep(   ItemQueue* queue, SequenceLogItem* item);
			void forward(ItemQueue* queue, SequenceLogItem* item);

			ItemQueue* getItemQueue(const SequenceLogItem& item) const;
			SequenceLogItem* createSequenceLogItem(ItemQueue* queue, const SequenceLogItem& src);

			// シーケンスログファイル関連
private:	void  openSeqLogFile(    File& file) throw(Exception);
			void writeSeqLogFile(    File& file, SequenceLogItem*);
			void writeSeqLogFileText(File& file, SequenceLogItem*);
};

} // namespace slog
