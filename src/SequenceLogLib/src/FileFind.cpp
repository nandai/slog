/*
 * Copyright (C) 2011 log-tools.net
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
 *  \author Copyright 2011 log-tools.net
 */
#include "slog/FileFind.h"
#include "slog/FileInfo.h"
#include "slog/FixedString.h"
#include "slog/PointerString.h"

#if defined(_WINDOWS)
// no implement
#elif defined(__linux__) && !defined(__ANDROID__)
    #include <glob.h>
#else
    #include <dirent.h>
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
    WIN32_FIND_DATAA fd;
    HANDLE handle = FindFirstFileExA(p, FindExInfoStandard, &fd, FindExSearchNameMatch, NULL, 0);

    int32_t len = (int32_t)(strrchr(p, '\\') - p);
    FixedString<MAX_PATH> path;

    if (handle == INVALID_HANDLE_VALUE)
        return;

    while (FindNextFileA(handle, &fd))
    {
        if (strcmp(fd.cFileName, "..") == 0)
            continue;

        path.format("%.*s\\%s", len, p, fd.cFileName);
        TRACE("    FileFind::exec() / path='%s'\n", path.getBuffer());

        mListener->onFind(path);
    }

    FindClose(handle);

#elif defined(__linux__) && !defined(__ANDROID__)
    glob_t globbuf;
    int32_t result = glob(p, GLOB_NOSORT, NULL, &globbuf);

    if (result != 0)
        return;

    for (int32_t index = 0; index < globbuf.gl_pathc; index++)
    {
        PointerString path = globbuf.gl_pathv[index];
        TRACE("    FileFind::exec() / path='%s'\n", path.getBuffer());

        mListener->onFind(path);
    }

    globfree(&globbuf);

#else
    // Android用手抜きバージョン
    *(strrchr(p, '/')) = '\0';

    DIR* dir = opendir(p);
    dirent* ent;

    if (dir == NULL)
        return;

    FixedString<MAX_PATH> path;

    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        path.format("%s/%s", p, ent->d_name);
        TRACE("    FileFind::exec() / path='%s'\n", path.getBuffer());

        mListener->onFind(path);
    }

    closedir(dir);
#endif
}

} // namespace slog
