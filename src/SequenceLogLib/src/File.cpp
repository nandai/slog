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
#include "slog/String.h"

#if defined(_WINDOWS)
    #include <windows.h>
#endif

#if defined(__unix__)
    #include <stdio.h>
    #include <unistd.h>
#endif

namespace slog
{

/*!
 *  \brief  コンストラクタ
 */
File::File()
{
    mHandle = 0;
}

/*!
 *  \brief  デストラクタ
 */
File::~File()
{
    close();
}

/*!
 *  \brief  オープンしているか調べる
 */
bool File::isOpen() const
{
    return (mHandle != 0);
}

/*!
 *  \brief  オープン
 */
void File::open(
    const CoreString& fileName,     //!< ファイル名
    Mode mode)                      //!< オープンモード

    throw(Exception)
{
    Exception e;

    if (mHandle != 0)
    {
        e.setMessage("File::open(\"%s\") : already opened.", fileName.getBuffer());
        throw e;
    }

#if defined(_WINDOWS)
    UTF16LE utf16le;
    utf16le.conv(fileName);

    const wchar_t* p = utf16le.getBuffer();
    HANDLE handle;

    if (mode == READ)
    {
        // 書込み中のファイルを読めるようにFILE_SHARE_WRITEを付ける
//      handle = CreateFileW(p, GENERIC_READ,  FILE_SHARE_READ,                    NULL, OPEN_EXISTING, 0, NULL);
        handle = CreateFileW(p, GENERIC_READ,  FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    }
    else
    {
//      handle = CreateFileW(p, GENERIC_WRITE, 0,               NULL, CREATE_ALWAYS, 0, NULL);
        handle = CreateFileW(p, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
    }

    if (handle == INVALID_HANDLE_VALUE)
    {
        e.setMessage("File::open(\"%s\")", fileName.getBuffer());
        throw e;
    }
#else
    const char* p =  fileName.getBuffer();
    const char* _mode = (mode == READ ? "r" : "w");
    FILE* handle = fopen(p, _mode);

    if (handle == NULL)
    {
        e.setMessage("File::open(\"%s\")", p);
        throw e;
    }
#endif

    mHandle = (int64_t)handle;
}

/*!
 *  \brief  クローズ
 */
void File::close()
{
    if (mHandle == 0)
        return;

#if defined(_WINDOWS)
    ::CloseHandle((HANDLE)mHandle);
#else
    fclose((FILE*)mHandle);
#endif

    mHandle = 0;
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

#if defined(_WINDOWS)
    HANDLE handle = (HANDLE)mHandle;
#endif

    do
    {
#if defined(_WINDOWS)
        ::ReadFile(handle, p, count, (DWORD*)&result, NULL);
#else
        result = fread(p, 1, count, (FILE*)mHandle);
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

    ::SetFilePointerEx(handle, move, NULL, FILE_CURRENT);

    if (p[index] == '\n')
        return true;

    ::ReadFile(handle, p, 1, (DWORD*)&result, NULL);

    if (p[0] != '\n')
    {
        move.QuadPart = -1;
        ::SetFilePointerEx(handle, move, NULL, FILE_CURRENT);
    }
#else
    FILE* handle = (FILE*)mHandle;
    fseek(handle, index - result + 1, SEEK_CUR);

    if (p[index] == '\n')
        return true;

    result = fread(p, 1, 1, handle);

    if (p[0] != '\n')
        fseek(handle, -1, SEEK_CUR);
#endif

    return true;
}

/*!
 *  \brief  読み込み
 */
int32_t File::read(Buffer* buffer, int32_t count) const throw(Exception)
{
	int32_t result = 0;
    int32_t position = 0;

    buffer->validateOverFlow(position, count);
    char* p = buffer->getBuffer() + position;

    if (mHandle != 0)
    {
#if defined(_WINDOWS)
        ::ReadFile((HANDLE)mHandle, p, count, (DWORD*)&result, NULL);
#else
        result = fread(p, 1, count, (FILE*)mHandle);
#endif
    }

    return result;
}

/*!
 *  \brief  書き込み
 */
void File::write(const Buffer* buffer, int32_t count) const throw(Exception)
{
    write(buffer, 0, count);
}

/*!
 *  \brief  書き込み
 */
void File::write(const Buffer* buffer, int32_t position, int32_t count) const throw(Exception)
{
    buffer->validateOverFlow(position, count);
    const char* p = buffer->getBuffer() + position;

    if (mHandle != 0)
    {
#if defined(_WINDOWS)
        DWORD result = 0;
        ::WriteFile((HANDLE)mHandle, p, count, &result, NULL);
#else
        fwrite(p, 1, count, (FILE*)mHandle);
#endif
    }
}

/*!
 *  \brief  ファイル削除
 */
void File::unlink(const CoreString& fileName) throw(Exception)
{
    const char* p = fileName.getBuffer();

#if defined(_WINDOWS)
    UTF16LE utf16le;
    utf16le.conv(fileName);

    bool result = (::DeleteFileW(utf16le.getBuffer()) == TRUE);
#else
    bool result = (::unlink(p) == 0);
#endif

    if (result == false)
    {
        Exception e;
        e.setMessage("File::unlink(\"%s\")", p);

        throw e;
    }
}

//void File::flush()
//{
//  if (mHandle != NULL)
//  {
//#if defined(_WINDOWS)
//      ::FlushFileBuffers(mHandle);
//#else
//      fflush(mHandle);
//#endif
//  }
//}

/*!
 *  \brief  ファイルサイズ取得
 */
int64_t File::getSize() const
{
#if defined(_WINDOWS)
    HANDLE handle = (HANDLE)mHandle;
    LARGE_INTEGER move = {0, 0};
    LARGE_INTEGER pos;
    LARGE_INTEGER size;

    ::SetFilePointerEx(handle, move, &pos,  FILE_CURRENT);
    ::SetFilePointerEx(handle, move, &size, FILE_END);
    ::SetFilePointerEx(handle, pos,  NULL,  FILE_BEGIN);

    return size.QuadPart;
#else
    FILE* handle = (FILE*)mHandle;

    int64_t pos = ftell(handle);
    fseek(handle, 0, SEEK_END);

    int64_t size = ftell(handle);
    fseek(handle, pos, SEEK_SET);

    return size;
#endif
}

/*!
 *  \brief  ファイルポインタの現在位置取得
 */
int64_t File::getPosition() const
{
#if defined(_WINDOWS)
    LARGE_INTEGER move = {0, 0};
    LARGE_INTEGER pos;

    ::SetFilePointerEx((HANDLE)mHandle, move, &pos, FILE_CURRENT);
    return pos.QuadPart;
#else
    int64_t pos = ftell((FILE*)mHandle);
    return pos;
#endif
}

/*!
 * 
 */
bool File::isEOF() const
{
#if defined(_WINDOWS)
    HANDLE handle = (HANDLE)mHandle;

    unsigned long cur = ::SetFilePointer(handle, 0,   NULL, FILE_CURRENT);
	unsigned long len = ::SetFilePointer(handle, 0,   NULL, FILE_END);
						::SetFilePointer(handle, cur, NULL, FILE_BEGIN);

	return (cur >= len);
#else
#endif
}

/*!
 * \brief   ファイルコピー
 */
bool File::copy(const CoreString* aSrc, const CoreString* aDst)
{
#if defined(_WINDOWS)
    UTF16LE src;
    src.conv(*aSrc);

    UTF16LE dst;
    dst.conv(*aDst);

    return (CopyFileW(src.getBuffer(), dst.getBuffer(), FALSE) == TRUE);
#else
#endif
}

/*!
 * ファイル移動
 */
bool File::move(const CoreString* aSrc, const CoreString* aDst)
{
#if defined(_WINDOWS)
    UTF16LE src;
    src.conv(*aSrc);

    UTF16LE dst;
    dst.conv(*aDst);

    return (MoveFileExW(src.getBuffer(), dst.getBuffer(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED) == TRUE);
#else
#endif
}

} // namespace slog
