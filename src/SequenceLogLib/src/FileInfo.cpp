﻿/*
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
 * \file    FileInfo.cpp
 * \brief   ファイル情報クラス
 * \author  Copyright 2011-2015 printf.jp
 */
#include "slog/FileInfo.h"
#include "slog/File.h"
#include "slog/Tokenizer.h"
#include "slog/FixedString.h"
#include "slog/Dir.h"

#include <list>
#include <sys/stat.h>

#if defined(_WINDOWS)
    #include <shlobj.h>
#else
    #include <stdlib.h>
    #include <unistd.h>
    #include <errno.h>
#endif

namespace slog
{

struct FileInfo::Data
{
    std::list<String>       mNames;         //!< ファイルパスの構成要素
    FixedString<MAX_PATH>   mCanonicalPath; //!< 正規のパス
    FixedString<255>        mMessage;       //!< メッセージ
};

/*!
 * \brief   パスの末尾に追加する
 *
 * \param[out]  path    パス（結果を受け取るバッファ）
 * \param[in]   name    追加するパス
 *
 * \return  なし
 */
static void appendPath(CoreString* path, const CoreString& name)
{
    const char DELIMITER[] = {PATH_DELIMITER, 0};

#if defined(_WINDOWS)
    if (path->getLength() != 0)
#endif
    {
        path->append(DELIMITER, 1);
    }

    path->append(&name);
}

/*!
 * \brief   コンストラクタ
 *
 * \param[in]   path    パス（ディレクトリは末尾に'\'、または'/'が必要）
 */
FileInfo::FileInfo(const CoreString* path)

    throw(Exception)
{
    mData = new Data;

    String absolutePath;
    const char* pszPath = path->getBuffer();
    String str;

    // ホームディレクトリ取得
    if (pszPath[0] == '~')
    {
#if defined(_WINDOWS)
        LPITEMIDLIST pidl;
        IMalloc* pMalloc;
        wchar_t unicode[MAX_PATH];

        SHGetMalloc(&pMalloc);
        SHGetSpecialFolderLocation(nullptr, CSIDL_PERSONAL, &pidl);
        SHGetPathFromIDListW(pidl, unicode);

        pMalloc->Free(pidl);
        pMalloc->Release();

        str.conv(unicode);
#else
        char* home = getenv("HOME");
        str.copy(home ? home : "/");
#endif

        absolutePath.format("%s%s", str.getBuffer(), pszPath + 1);
    }

    // カレントディレクトリ取得
#if defined(_WINDOWS)
    else if (pszPath[1] != ':')
    {
#else
    else if (pszPath[0] != '/')
    {
#endif
        Dir::getCurrent(&str);
        absolutePath.format("%s%c%s", str.getBuffer(), PATH_DELIMITER, pszPath);
    }

    // 絶対パス
    else
    {
        absolutePath.copy(path);
    }

    Tokenizer tokenizer(PATH_DELIMITER);
    tokenizer.exec(&absolutePath);
    mData->mNames.clear();

    // 正規のパス生成
    for (int32_t index = 0; index < tokenizer.getCount(); index++)
    {
        const CoreString* value = tokenizer.getValue(index);
        String str;
        str.copy(value);

        if (str == ".")
            continue;

        if (str.getLength() == 0 && index != tokenizer.getCount() - 1)
            continue;

        if (str == "..")
        {
            if (mData->mNames.size() == 0)
            {
                // 戻るべき親ディレクトリがない
                Exception e;
                e.setMessage("FileInfo::FileInfo(\"%s\") / illegal file path", pszPath);

                throw e;
            }

            mData->mNames.pop_back();
        }
        else
        {
            mData->mNames.push_back(str);
        }
    }

    for (auto i = mData->mNames.begin(); i != mData->mNames.end(); i++)
        appendPath(&mData->mCanonicalPath, *i);

    // ファイル情報更新
    update();
}

/*!
 * \brief   デストラクタ
 */
FileInfo::~FileInfo()
{
    delete mData;
}

/*!
 * \brief   ファイル情報更新
 */
void FileInfo::update(bool aUsing)
{
#if defined(_WINDOWS)
    UTF16LE utf16le;
    utf16le.conv(&mData->mCanonicalPath);

    struct _stati64 buf;
    int32_t result = _wstati64(utf16le.getBuffer(), &buf);
#else
    struct stat buf;
    int32_t result = stat(mData->mCanonicalPath.getBuffer(), &buf);
#endif

    if (result == 0)
    {
        mCreationTime. setTime_t(buf.st_ctime);
        mLastWriteTime.setTime_t(buf.st_mtime);
        mMode = buf.st_mode;
        mSize = buf.st_size;
        mData->mMessage.copy("");
    }
    else
    {
        Exception e(false);
        e.setMessage("");

        mCreationTime. setValue(0);
        mLastWriteTime.setValue(0);
        mMode = 0;
        mSize = 0;
        mData->mMessage.copy(e.getMessage());
    }

    mUsing = aUsing;
}

/*!
 * \brief   正規のパス取得
 */
const CoreString* FileInfo::getCanonicalPath() const
{
    return &mData->mCanonicalPath;
}

/*!
 * \brief   ディレクトリ作成
 */
void FileInfo::mkdir() const

    throw(Exception)
{
    int32_t index = 0;
    FixedString<MAX_PATH> path;

    for (auto i = mData->mNames.begin(); i != mData->mNames.end(); i++)
    {
        if (index == mData->mNames.size() - 1)
            break;

        appendPath(&path, *i);

#if defined(_WINDOWS)
        if (0 < index)  // Windowsの場合、index 0 はドライブなのでスキップ
        {
            bool success = Dir::create(&path);
            DWORD err = GetLastError();

            if (success == false && err == ERROR_ALREADY_EXISTS)
                success = true;
#else
        {
            errno = 0;
            bool success = Dir::create(&path);

            if (errno == EEXIST)
                success = true;
#endif

            if (success == false)
            {
                Exception e;
                e.setMessage("FileInfo::mkdir(\"%s\")", path.getBuffer());

                throw e;
            }
        }

        index++;
    }

    // ファイルが作成できるかチェック
    String name = "SequenceLogMakeDirectoryCheck";
    appendPath(&path, name);

    File file;
    file.open(&path, File::WRITE);
    file.close();

    File::unlink(&path);
}

/*!
 * \brief   ファイルかどうか調べる
 */
bool FileInfo::isFile() const
{
    return ((mMode & S_IFMT) == S_IFREG);
}

/*!
 * \brief   使用中かどうか調べる
 */
bool FileInfo::isUsing() const
{
    return mUsing;
}

/*!
 * \brief   メッセージ取得
 */
const CoreString* FileInfo::getMessage() const
{
    return &mData->mMessage;
}

} // namespace slog
