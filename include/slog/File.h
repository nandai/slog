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
 *  \file   File.h
 *  \brief  ファイルクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once

#include "slog/CoreString.h"
#include "slog/Exception.h"

namespace slog
{

/*!
 *  \brief  ファイルクラス
 */
class SLOG_API File
{
            class IO;
            class FileIO;
            class BufferIO;

public:     enum Mode
            {
                READ,
                WRITE,
            };

            /*!
             * I/O
             */
private:    IO* mIO;

            /*!
             * コンストラクタ
             */
public:     File();

            /*!
             * デストラクタ
             */
            ~File();

            bool isOpen() const;
            void open(const CoreString& fileName, Mode mode) throw(Exception);
            void close();

            void write(const Buffer* buffer, int64_t count) const throw(Exception);
            void write(const Buffer* buffer, int64_t position, int64_t count) const throw(Exception);

            bool read(CoreString* str) const throw(Exception);
            int64_t read(Buffer* buffer, int64_t count) const throw(Exception);

//          void flush();

            bool isEOF() const;

            /*!
             * ファイルサイズ取得
             */
            int64_t getSize() const;

            /*!
             * ファイルポインタの現在位置取得
             */
            int64_t getPosition() const;

            /*!
             * ファイルポインタの現在位置設定
             */
            void setPosition(int64_t pos) const;

            /*!
             * ファイルポインタ移動
             */
            int64_t movePosition(int64_t count) const;

            /*!
             * ファイルポインタ移動
             */
            int64_t moveLastPosition() const;

            /*!
             * ファイル削除
             */
            static void unlink(const CoreString& fileName) throw(Exception);

            /*!
             * ファイルコピー
             */
            static bool copy(const CoreString* aSrc, const CoreString* aDst);

            /*!
             * ファイル移動
             */
            static bool move(const CoreString* aSrc, const CoreString* aDst);
};

} // namespace slog
