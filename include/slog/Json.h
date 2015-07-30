/*
 * Copyright (C) 2011-2015 printf.jp
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
 *  \file   Json.h
 *  \brief  JSONクラス
 *  \author Copyright 2011-2015 printf.jp
 */
#pragma once

#include "slog/String.h"
#include <list>

#pragma warning(disable:4251)

namespace slog
{

/*!
 * \brief   JSON基本クラス
 */
class SLOG_API JsonAbstract
{
            /*!
             * \brief   キーの名前
             */
protected:  String mName;

            /*!
             * コンストラクタ
             */
protected:  JsonAbstract(const char* name);

            /*!
             * デストラクタ
             */
public:     virtual ~JsonAbstract() {}

            /*!
             * シリアライズ
             */
            virtual void serializeChild(CoreString* content) const = 0;
};

/*!
 *  \brief  JSONクラス
 */
class SLOG_API Json : public JsonAbstract
{
            /*!
             * \brief   括弧：[]、または{}
             */
            char mBracket[2];

            /*!
             * \brief   JSONオブジェクトリスト
             */
            std::list<JsonAbstract*> mList;

            /*!
             * Jsonオブジェクト作成
             */
public:     static Json* getNewObject(const char* name = "")
            {
                return new Json(name);
            }

            /*!
             * コンストラクタ
             */
private:    Json(const char* name = "");

            /*!
             * デストラクタ
             */
public:     virtual ~Json() override;

            /*!
             * JSONオブジェクト追加
             */
            void add(const char* name, const CoreString* value);

            /*!
             * JSONオブジェクト追加
             */
            void add(const char* name, const char* value);

            /*!
             * JSONオブジェクト追加
             */
            void add(const char* name, int32_t value);

            /*!
             * JSONオブジェクト追加
             */
            void add(Json* json);

            /*!
             * シリアライズ
             */
            void serialize(CoreString* content) const;

            /*!
             * シリアライズ
             */
private:    virtual void serializeChild(CoreString* content) const override;
};

} // namespace slog
