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
 * \file    R.cpp
 * \brief   リソースクラス
 * \author  Copyright 2014 printf.jp
 */
#pragma execution_character_set("utf-8")

#include "R.h"
#include "slog/PointerString.h"

namespace slog
{

/*!
 * \brief   英語
 */
static const char* en[] =
{
    "Login",
    "Logout",
    "Log file list",
    "Account",
    "User name",
    "Password",
    "Change",
    "New account",
    "Delete",
    "Back",
    "Start time",
    "End time",
    "Log file name",
    "Size",
    "Administrator",
    "The username or password you entered is incorrect.",
    "That user name is already in use.",
    "Can not change the user name.",
    "Incorrect %s",
    "The %s has not been entered.",
    "Number of characters of the %s is missing.",
    "Number of characters of the %s exceeds the upper limit.",
    "Account does not match.",
    "Can not delete.",
    "It was not possible to connect to the server.",
};

/*!
 * \brief   日本語
 */
static const char* ja[] =
{
    "ログイン",
    "ログアウト",
    "ログファイルリスト",
    "アカウント",
    "ユーザー名",
    "パスワード",
    "変更",
    "新規作成",
    "削除",
    "戻る",
    "開始日時",
    "終了日時",
    "ログファイル名",
    "サイズ",
    "管理者",
    "ユーザー名、またはパスワードが正しくありません。",
    "そのユーザー名は既に使われています。",
    "ユーザー名は変更できません。",
    "%sが正しくありません。",
    "%sが入力されていません。",
    "%sの文字数が不足しています。",
    "%sの文字数が上限を超えています。",
    "アカウントが一致しません。",
    "削除はできません。",
    "サーバーに接続できませんでした。",
};

/*!
 * \brief   台語
 */
static const char* zh_TW[] =
{
    "註冊",
    "註銷",
    "日誌文件列表",
    "帳戶",
    "用戶名",
    "密碼",
    "變化",
    "新賬戶",
    "刪除",
    "回歸",
    "開始日期及時間",
    "結束日期及時間",
    "日誌文件名",
    "大小",
    "管理員",
    "用戶名或密碼不正確",
    "該用戶名已被使用",
    "您不能更改用戶名",
    "不正確的%s",
    "%s尚未進入",
    "%s的字符數是缺少",
    "%s的字符數超過了上限值",
    "賬戶不符",
    "您無法刪除",
    "這是不可能連接到服務器",
};

/*!
 * \brief   言語リスト
 */
struct
{
    const char*     lang;
    const char**    strings;
}
static languages[] =
{
    {"en",    en   },
    {"ja",    ja   },
    {"zh-TW", zh_TW},
};

/*!
 * \brief   文字列リスト取得
 */
static const char** getStrings(const CoreString* lang)
{
    for (int32_t i = 0; i < sizeof(languages) / sizeof(languages[0]); i++)
    {
        if (lang->indexOf(languages[i].lang) == 0)
            return languages[i].strings;
    }

    return nullptr;
}

/*!
 * \brief   コンストラクタ
 */
R::R(const CoreString* aLang)
{
    mStrings = getStrings(aLang);
    PointerString en("en");

    if (mStrings == nullptr)
        mStrings = getStrings(&en);

    if (mStrings == nullptr)
        mStrings = languages[0].strings;
}

/*!
 * \brief   文字列取得
 */
const char* R::string(int32_t id) const
{
    const char* text = mStrings[id];

    if (text[0] != '\0')
        return text;

    PointerString en("en");
    const char** strings = getStrings(&en);

    if (strings == nullptr)
        strings = languages[0].strings;

    return strings[id];
}

} // namespace slog
