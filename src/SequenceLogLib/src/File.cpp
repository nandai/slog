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
 *  \file   File.cpp
 *  \brief  ファイルクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/File.h"

namespace slog
{

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY & 0x00000002/*WINAPI_PARTITION_APP*/)
inline HANDLE CreateFileA(
    const char* fileName,
    DWORD desiredAccess,
    DWORD shareMode,
    LPSECURITY_ATTRIBUTES securityAttributes,
    DWORD creationDisposition,
    DWORD flagsAndAttributes,
    HANDLE templateFile)
{
    UTF16LE utf16le;
    utf16le.conv(fileName);

    return CreateFile2(utf16le.getBuffer(), desiredAccess, shareMode, creationDisposition, nullptr);
}
#endif

/*!
 *  \brief  オープン
 */
void File::open(
    const CoreString& fileName,     //!< ファイル名
    Mode mode)                      //!< オープンモード

    throw(Exception)
{
    Exception e;
    const char* p = fileName.getBuffer();

    if (mHandle != NULL)
    {
        e.setMessage("File::open(\"%s\") : already opened.", p);
        throw e;
    }

#if defined(_WINDOWS)
    HANDLE handle;

    if (mode == READ)
    {
        // 書込み中のファイルを読めるようにFILE_SHARE_WRITEを付ける
//      handle = CreateFileA(p, GENERIC_READ,  FILE_SHARE_READ,                    NULL, OPEN_EXISTING, 0, NULL);
        handle = CreateFileA(p, GENERIC_READ,  FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    }
    else
    {
//      handle = CreateFileA(p, GENERIC_WRITE, 0,               NULL, CREATE_ALWAYS, 0, NULL);
        handle = CreateFileA(p, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
    }

    if (handle == INVALID_HANDLE_VALUE)
    {
        e.setMessage("File::open(\"%s\")", p);
        throw e;
    }
#else
    const char* _mode = (mode == READ ? "r" : "w");
    FILE* handle = fopen(p, _mode);

    if (handle == NULL)
    {
        e.setMessage("File::open(\"%s\")", p);
        throw e;
    }
#endif

    mHandle = handle;
}

/*!
 *  \brief  読み込み
 */
bool File::read(
    CoreString* str)    //!< 結果を受け取るバッファ

    const
    throw(Exception)
{
    char buffer[256];
    char* p = buffer;
    int32_t count = sizeof(buffer);
    int32_t index;
    int32_t result = 0;
    bool first = true;

    str->setLength(0);

    do
    {
#if defined(_WINDOWS)
        ::ReadFile(mHandle, p, count, (DWORD*)&result, NULL);
#else
        result = fread(p, 1, count, mHandle);
#endif

//      if (first && result == 0)
        if (         result == 0)
        {
            // EOF
//          return false;
            return (first == false);
        }

        first = false;

        for (index = 0; index < result; index++)
        {
            if (p[index] == '\r' || p[index] == '\n')
                break;
        }

        str->append(p, index);

        if (index < result)
            break;
    }
    while (true);

    // 読み込みすぎた部分を戻す
#if defined(_WINDOWS)
    LARGE_INTEGER move;
    move.QuadPart = index - result + 1;

    ::SetFilePointerEx(mHandle, move, NULL, FILE_CURRENT);

    if (p[index] == '\n')
        return true;

    ::ReadFile(mHandle, p, 1, (DWORD*)&result, NULL);

    if (p[0] != '\n')
    {
        move.QuadPart = -1;
        ::SetFilePointerEx(mHandle, move, NULL, FILE_CURRENT);
    }
#else
    fseek(mHandle, index - result + 1, SEEK_CUR);

    if (p[index] == '\n')
        return true;

    result = fread(p, 1, 1, mHandle);

    if (p[0] != '\n')
        fseek(mHandle, -1, SEEK_CUR);
#endif

    return true;
}

bool File::isEOF() const
{
#if defined(_WINDOWS)
	unsigned long cur = ::SetFilePointer(mHandle, 0,   NULL, FILE_CURRENT);
	unsigned long len = ::SetFilePointer(mHandle, 0,   NULL, FILE_END);
						::SetFilePointer(mHandle, cur, NULL, FILE_BEGIN);

	return (cur >= len);
#else
#endif
}

} // namespace slog
