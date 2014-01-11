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
 *  \file   MimeType.cpp
 *  \brief  mime-typeクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/MimeType.h"
#include <string.h>

namespace slog
{

struct
{
    const char*             ext;
    bool                    binary;
    const MimeType::Type    type;
    const char*             text;
    const char*             tag;
}
static const mimeArray[] =
{
    {"html",  false, MimeType::Type::HTML,       "text/html",                     ""      },
    {"css",   false, MimeType::Type::CSS,        "text/css",                      "style" },
    {"js",    false, MimeType::Type::JAVASCRIPT, "text/javascript",               "script"},
    {"json",  false, MimeType::Type::JSON,       "application/json",              ""      },

    {"pcss",  false, MimeType::Type::TEXT,       "text/css",                      "style" },
    {"pjs",   false, MimeType::Type::TEXT,       "text/javascript",               "script"},

    {"png",   true,  MimeType::Type::IMAGE,      "image/png",                     ""      },
    {"jpg",   true,  MimeType::Type::IMAGE,      "image/jpeg",                    ""      },
    {"jpeg",  true,  MimeType::Type::IMAGE,      "image/jpeg",                    ""      },
    {"gif",   true,  MimeType::Type::IMAGE,      "image/gif",                     ""      },
    {"ico",   true,  MimeType::Type::IMAGE,      "image/vnd.microsoft.icon",      ""      },
    {"zip",   true,  MimeType::Type::BINARY,     "application/zip",               ""      },
    {"exe",   true,  MimeType::Type::BINARY,     "application/octet-stream",      ""      },
    {"dll",   true,  MimeType::Type::BINARY,     "application/octet-stream",      ""      },
    {"dat",   true,  MimeType::Type::BINARY,     "application/octet-stream",      ""      },
    {"swf",   true,  MimeType::Type::BINARY,     "application/x-shockwave-Flash", ""      },

    {nullptr, false, MimeType::Type::TEXT,       "text/plain",                    ""      },
};

/*!
 * MimeTypeを設定する
 */
static void setMimeType(MimeType* mimeType, int32_t i)
{
    mimeType->binary =  mimeArray[i].binary;
    mimeType->type =    mimeArray[i].type;
    mimeType->text.copy(mimeArray[i].text);
    mimeType->tag. copy(mimeArray[i].tag);
}

/*!
 * コンストラクタ
 */
MimeType::MimeType()
{
    setMimeType(this, sizeof(mimeArray) / sizeof(mimeArray[0]) - 1);
}

/*!
 * \brief   mime-typeを解析する
 *
 * \param[in]   path    パス
 *
 * \return  なし
 */
void MimeType::analize(const CoreString* path)
{
    int32_t extPos = path->lastIndexOf(".");

    if (extPos == -1)
        extPos = path->getLength();

    const char* p = path->getBuffer();
    const char* ext = p + extPos + 1;

    int32_t i = 0;

    while (mimeArray[i].ext)
    {
#if defined(_WINDOWS)
        if (_stricmp(
#else
        if (strcasecmp(
#endif
            ext, mimeArray[i].ext) == 0)
        {
            break;
        }

        i++;
    }

    setMimeType(this, i);
}

/*!
 * mime-type（タイプ）を設定する
 */
void MimeType::setType(Type type)
{
    int32_t i = 0;

    while (mimeArray[i].ext)
    {
        if (mimeArray[i].type == type)
            break;

        i++;
    }

    setMimeType(this, i);
}

/*!
 * mime-type（文字列）を設定する
 */
void MimeType::setText(const char* text)
{
    int32_t i = 0;

    while (mimeArray[i].ext)
    {
#if defined(_WINDOWS)
        if (_stricmp(
#else
        if (strcasecmp(
#endif
            text, mimeArray[i].text) == 0)
        {
            break;
        }

        i++;
    }

    setMimeType(this, i);
}

} // namespace slog
