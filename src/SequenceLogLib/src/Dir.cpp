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
 *  \file   Dir.cpp
 *  \brief  ディレクトリクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/Dir.h"
#include "slog/CoreString.h"

#if defined(_WINDOWS)
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace slog
{

/*!
 * ディレクトリ作成
 */
bool Dir::create(const CoreString* aDirName)
{
    bool res;

#if defined(_WINDOWS)
    UTF16LE dirName;
    dirName.conv(aDirName);

    res = (CreateDirectoryW(dirName.getBuffer(), nullptr) == TRUE);
#else
    res = (mkdir(aDirName->getBuffer(), 0755) == 0);
#endif

    return res;
}

/*!
 * ディレクトリ削除
 */
bool Dir::remove(const CoreString* aDirName)
{
    bool res;

#if defined(_WINDOWS)
    UTF16LE dirName;
    dirName.conv(aDirName);

    res = (RemoveDirectoryW(dirName.getBuffer()) == TRUE);
#else
    res = (rmdir(aDirName->getBuffer()) == 0);
#endif

    return res;
}

/*!
 * カレントディレクトリ取得
 */
void Dir::getCurrent(CoreString* aDirName)
{
#if defined(_WINDOWS)
    wchar_t dirName[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, dirName);

    aDirName->conv(dirName);
#else
    char dirName[MAX_PATH];
    getcwd(dirName, sizeof(dirName));

    aDirName->copy(dirName);
#endif
}

/*!
 * カレントディレクトリ設定
 */
bool Dir::setCurrent(const CoreString* aDirName)
{
    bool res;

#if defined(_WINDOWS)
    UTF16LE dirName;
    dirName.conv(aDirName);

    res = (SetCurrentDirectoryW(dirName.getBuffer()) == TRUE);
#else
    res = (chdir(aDirName->getBuffer()) == 0);
#endif

    return res;
}

} // namespace slog
