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
 *  \file   Json.h
 *  \brief  JSONクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once

#include "slog/String.h"
#include <list>

#pragma warning(disable:4251)

namespace slog
{

/*!
 *  \brief  JSON基本クラス
 */
class SLOG_API JsonAbstract
{
protected:  String      mName;          // キーの名前

            // コンストラクタ / デストラクタ
protected:  JsonAbstract(const char* name);
public:     virtual ~JsonAbstract() {}

            // シリアライズ
            virtual void serialize(String* content) const = 0;
};

/*!
 *  \brief  JSON値クラス
 */
class SLOG_API JsonValue : public JsonAbstract
{
            String      mValue;         // 値

            // コンストラクタ
public:     JsonValue(const char* name, const CoreString& value);

            // シリアライズ
            virtual void serialize(String* content) const;
};

/*!
 *  \brief  JSONクラス
 */
class SLOG_API Json : public JsonAbstract
{
            char                        mBracket[2];    // 括弧：[]、または{}
            std::list<JsonAbstract*>    mList;          // JSONオブジェクトリスト

public:     static Json* getNewObject()
            {
                return new Json;
            }

            // コンストラクタ / デストラクタ
private:    Json(const char* name = "");
public:     virtual ~Json();

            // JSONオブジェクト追加
            void add(const char* name, const CoreString& value);
            void add(Json* json);

            virtual void serialize(String* content) const;
};

} // namespace slog
