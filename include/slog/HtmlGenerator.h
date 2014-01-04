/*
 * Copyright (C) 2013 printf.jp
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
 *  \author Copyright 2013 printf.jp
 */
#pragma once

#include "slog/String.h"
#include <list>

#pragma warning(disable:4251)

namespace slog
{

/*!
 * \brief  変数クラス
 */
class SLOG_API Variable
{
            /*!
             * 変数名
             */
public:     slog::String name;

            /*!
             * 値
             */
            slog::String value;

            /*!
             * コンストラクタ
             */
public:     Variable(const char* name, const char* value)
            {
                this->name. copy(name);
                this->value.copy(value);
            }
};

/*!
 * \brief  変数リスト
 */
typedef std::list<Variable*> VariableList;

/*!
 * \brief  html生成クラス
 */
class SLOG_API HtmlGenerator
{
            class Param;

            /*!
             * 生成したhtml
             */
            slog::String mHtml;

            /*!
             * 変数リスト
             */
            const VariableList* mVariableList;

            /*!
             * 読み込んだ変数のリスト
             */
            VariableList mReadVariableList;

            /*!
             * ルートディレクトリ
             */
            slog::String mRootDir;

            /*!
             * コンストラクタ
             */
public:     HtmlGenerator(const slog::CoreString* rootDir);

            /*!
             * デフォルトの変数リストか調べる
             */
private:    bool isDefaultVariableList() const;

            /*!
             * タグをスキップする
             */
            int32_t skipTags(const slog::CoreString* readHtml, int32_t pos, int32_t depth);

            /*!
             * タグをスキップする
             */
            int32_t skipTags(const slog::CoreString* readHtml, int32_t pos);

            /*!
             * 変数を値に置換する
             */
            bool replaceVariable(Param* param, const slog::CoreString* var);

            /*!
             * 置換する
             */
            void replace(Param* param, const slog::CoreString* var);

            /*!
             * 検索開始文字列の位置までの文字列をappendする
             */
            bool append(Param* param, int32_t index, const char* from, const char* to, int32_t* pos, int32_t* endPos);

            /*!
             * インクルードパスを取得する
             */
            void getIncludePath(Param* param, slog::CoreString* path, const slog::CoreString* include);

            /*!
             * htmlを読み込む
             */
public:     static bool readHtml(slog::CoreString* readHtml, int32_t position, const slog::CoreString* fileName);

            /*!
             * html生成を実行する
             */
            bool execute(const slog::CoreString* fileName, const VariableList* variableList);

            /*!
             * html生成を実行する
             */
private:    bool expand(const slog::CoreString* fileName, CoreString* writeBuffer, int32_t depth);

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
public:    const slog::CoreString* getHtml() const {return &mHtml;}
};

} // namespace slog
