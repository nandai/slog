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
 *  \file   SequenceLogService.h
 *  \brief  シーケンスログサービスクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once

#include "slog/File.h"
#include "slog/Mutex.h"

namespace slog
{
/*!
 *  \brief  共有ファイルコンテナクラス
 */
class SharedFileContainer
{
            File                    mFile;                      //!< 共有ファイル
            FixedString<MAX_PATH>   mBaseFileName;              //!< ベースファイル名
            FileInfo*               mFileInfo;                  //!< シーケンスログファイル情報
            Mutex                   mMutex;                     //!< ミューテックス
            int32_t                 mReferenceCount;            //!< 参照カウント

            /*!
             * コンストラクタ
             */
public:     SharedFileContainer()
            {
                mFileInfo = nullptr;
                mReferenceCount = 1;
            }

            /*!
             * 共有ファイル
             */
            File* getFile() const {return (File*)&mFile;}

            /*!
             * ベースファイル名
             */
            CoreString* getBaseFileName() const {return (CoreString*)&mBaseFileName;}

            /*!
             * シーケンスログファイル情報
             */
            FileInfo* getFileInfo() const {return (FileInfo*)mFileInfo;}
            void setFileInfo(FileInfo* fileInfo) {mFileInfo = fileInfo;}

            /*!
             * ミューテックス
             */
            Mutex* getMutex() const {return (Mutex*)&mMutex;}

            /*!
             * 参照カウント
             */
            void addReference() {mReferenceCount++;}
            bool removeReference()
            {
                mReferenceCount--;
                return (mReferenceCount == 0);
            }
};

} // namespace slog
