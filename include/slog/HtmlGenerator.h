﻿/*
 * Copyright (C) 2013-2015 printf.jp
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
 *  \file   HtmlGenerator.h
 *  \brief  html生成クラス
 *  \author Copyright 2013-2015 printf.jp
 */
#pragma once

#include "slog/String.h"
#include "slog/Variable.h"

namespace slog
{
class DateTime;

/*!
 * \brief  html生成クラス
 */
class SLOG_API HtmlGenerator
{
            class Param;

            static const char* CLS_NAME;

            /*!
             * \brief   生成したhtml
             */
            String mHtml;

            /*!
             * \brief   変数リスト
             */
            const VariableList* mVariableList;

            /*!
             * \brief   読み込んだ変数のリスト
             */
            VariableList mReadVariableList;

            /*!
             * \brief   ルートディレクトリ
             */
            String mRootDir;

            /*!
             * \brief   最終書込日時
             */
            DateTime* mLastWriteTime;

            /*!
             * コンストラクタ
             */
public:     HtmlGenerator(const CoreString* rootDir);

            /*!
             * デストラクタ
             */
            ~HtmlGenerator();

            /*!
             * デフォルトの変数リストか調べる
             */
private:    bool isDefaultVariableList() const;

            /*!
             * タグをスキップする
             */
            int32_t skipTags(const CoreString* readHtml, int32_t pos, int32_t depth);

            /*!
             * タグをスキップする
             */
            int32_t skipTags(const CoreString* readHtml, int32_t pos);

            /*!
             * 変数を値に置換する
             */
            bool replaceVariable(Param* param, const CoreString* var);

            /*!
             * 置換する
             */
            void replace(Param* param, const CoreString* var);

            /*!
             * 検索開始文字列の位置までの文字列をappendする
             */
            bool append(Param* param, int32_t index, const char* from, const char* to, int32_t* pos, int32_t* endPos);

            /*!
             * インクルードパスを取得する
             */
            void getIncludePath(Param* param, CoreString* path, const CoreString* include);

            /*!
             * htmlを読み込む
             */
            bool readHtml(CoreString* readHtml, int32_t position, const CoreString* fileName);

            /*!
             * html生成を実行する
             */
public:     bool execute(const CoreString* fileName, const VariableList* variableList);

            /*!
             * html生成を実行する
             */
private:    bool expand(const CoreString* fileName, CoreString* writeBuffer, int32_t depth);

            /*!
             * html生成を実行する
             */
            void expand(Param* param);

            /*!
             * html生成を実行する
             */
            void expandCSS(Param* param);

            /*!
             * htmlを取得する
             */
public:    const CoreString* getHtml() const {return &mHtml;}

            /*!
             * 最終書込日時取得
             */
            const DateTime* getLastWriteTime() const;
};

} // namespace slog
