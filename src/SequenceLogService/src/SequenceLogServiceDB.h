/*
 * Copyright (C) 2014 printf.jp
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
 * \file    SequenceLogServiceDB.h
 * \brief   シーケンスログサービスDBクラス
 * \author  Copyright 2014 printf.jp
 */
#pragma once

#define     USE_SQLITE
#if defined(USE_SQLITE)
    #include "SQLite.h"
#else
    #include "DB_MySQL.h"
#endif


namespace slog
{

/*!
 * \brief   シーケンスログサービスDBクラス
 */
class SequenceLogServiceDB
#if defined(USE_SQLITE)
    : public SQLite
#else
    : public MySQL
#endif
{
            static const char* CLS_NAME;

            /*!
             * コンストラクタ
             */
public:     SequenceLogServiceDB();

            /*!
             * デストラクタ
             */
            virtual ~SequenceLogServiceDB() override;

            /*!
             * データベース名取得
             */
private:    static void SequenceLogServiceDB::getName(CoreString* name);

            /*!
             * データベースバージョン取得
             */
            int32_t getVersion() const;

            /*!
             * 初期化
             */
public:     void init() const throw(Exception);

            /*!
             * バージョン１にアップデート
             */
private:    void updateVersion1() const throw(Exception);
};

} // namespace slog
