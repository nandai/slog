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
 *  \file   FileFind.cpp
 *  \brief  ファイル検索クラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/FileFind.h"
#include "slog/FileInfo.h"
#include "slog/String.h"
#include "slog/FixedString.h"
#include "slog/PointerString.h"

#if defined(_WINDOWS)
    #include <windows.h>
#elif defined(__linux__)
    #include <stddef.h>
    #if !defined(__ANDROID__)
        #include <glob.h>
    #else
        #include <dirent.h>
    #endif
#endif

namespace slog
{

/*!
 *  \brief  コンストラクタ
 */
FileFind::FileFind()
{
    setListener(NULL);
}

/*!
 *  \brief  検索実行
 */
void FileFind::exec(
    const CoreString& fileName) const   //!< 検索パス
{
    FileInfo info = fileName;
    const CoreString& canonicalPath = info.getCanonicalPath();
    const char* p =   canonicalPath.getBuffer();

#if defined(_WINDOWS)
    UTF16LE utf16le;
    utf16le.conv(canonicalPath);

    WIN32_FIND_DATAW fd;
    HANDLE handle = FindFirstFileExW(utf16le.getBuffer(), FindExInfoStandard, &fd, FindExSearchNameMatch, NULL, 0);

    int32_t len = (int32_t)(strrchr(p, '\\') - p);
    FixedString<MAX_PATH> path;

    if (handle == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (lstrcmpW(fd.cFileName, L".") == 0 ||
            lstrcmpW(fd.cFileName, L"..") == 0)
        {
            continue;
        }

        String fileName;
        fileName.conv(fd.cFileName);

        path.format("%.*s\\%s", len, p, fileName.getBuffer());
        mListener->onFind(path);
    }
    while (FindNextFileW(handle, &fd));

    FindClose(handle);

#elif defined(__linux__) && !defined(__ANDROID__)
    glob_t globbuf;
    int32_t result = glob(p, GLOB_NOSORT, NULL, &globbuf);

    if (result != 0)
        return;

    for (int32_t index = 0; index < globbuf.gl_pathc; index++)
    {
        PointerString path = globbuf.gl_pathv[index];
        mListener->onFind(path);
    }

    globfree(&globbuf);

#else
    // Android用手抜きバージョン

    // ファイル名取得
    char* searchFileName = strrchr(p, '/');
    searchFileName[0] = '\0';
    searchFileName++;

    // 拡張子取得
    char* ext = strchr(searchFileName, '.');

    if (ext == NULL)
        return;

    bool any = (strcmp(ext, ".*") == 0);

    // ディレクトリ内検索
    DIR* dir = opendir(p);
    dirent* ent;

    if (dir == NULL)
        return;

    FixedString<MAX_PATH> path;

    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        if (any == false)
        {
            char* entExt = strchr(ent->d_name, '.');

            if (entExt == NULL)
                continue;

            if (strcmp(ext, entExt) != 0)
                continue;
        }

        path.format("%s/%s", p, ent->d_name);
        mListener->onFind(path);
    }

    closedir(dir);
#endif
}

} // namespace slog
