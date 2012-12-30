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
 *  \file   File.h
 *  \brief  ファイルクラス
 *  \author Copyright 2011 log-tools.net
 */
#pragma once

#include "slog/CoreString.h"
#include "slog/Exception.h"

#if defined(__unix__)
    #include <unistd.h>
#endif

namespace slog
{

/*!
 *  \brief  ファイルクラス
 */
class SLOG_API File
{
public:     enum Mode
            {
                READ,
                WRITE,
            };

private:
#if defined(_WINDOWS)
            HANDLE  mHandle;    //!< ファイルハンドル
#else
            FILE*   mHandle;    //!< ファイルハンドル
#endif

public:      File();
            ~File();

            bool isOpen() const;
            void open(const CoreString& fileName, Mode mode) throw(Exception);
            void close();

            void write(const Buffer* buffer, int32_t count) const throw(Exception);
            void write(const Buffer* buffer, int32_t position, int32_t count) const throw(Exception);

            bool read(CoreString* str) const throw(Exception);
            int32_t read(Buffer* buffer, int32_t count) const throw(Exception);

//          void flush();

            int64_t getSize() const;
            int64_t getPosition() const;

            static void unlink(const CoreString& fileName) throw(Exception);
};

/*!
 *  \brief  コンストラクタ
 */
inline File::File()
{
    mHandle = NULL;
}

/*!
 *  \brief  デストラクタ
 */
inline File::~File()
{
    close();
}

/*!
 *  \brief  オープンしているか調べる
 */
inline bool File::isOpen() const
{
    return (mHandle != NULL);
}

/*!
 *  \brief  クローズ
 */
inline void File::close()
{
    if (mHandle == NULL)
        return;

#if defined(_WINDOWS)
    ::CloseHandle(mHandle);
#else
    fclose(mHandle);
#endif

    mHandle = NULL;
}

/*!
 *  \brief  書き込み
 */
inline void File::write(const Buffer* buffer, int32_t count) const throw(Exception)
{
    write(buffer, 0, count);
}

/*!
 *  \brief  書き込み
 */
inline void File::write(const Buffer* buffer, int32_t position, int32_t count) const throw(Exception)
{
    buffer->validateOverFlow(position, count);
    const char* p = buffer->getBuffer() + position;

    if (mHandle != NULL)
    {
#if defined(_WINDOWS)
        DWORD result = 0;
        ::WriteFile(mHandle, p, count, &result, NULL);
#else
        fwrite(p, 1, count, mHandle);
#endif
    }
}

/*!
 *  \brief  読み込み
 */
inline int32_t File::read(Buffer* buffer, int32_t count) const throw(Exception)
{
	int32_t result = 0;
    int32_t position = 0;

    buffer->validateOverFlow(position, count);
    char* p = buffer->getBuffer() + position;

    if (mHandle != NULL)
    {
#if defined(_WINDOWS)
        ::ReadFile(mHandle, p, count, (DWORD*)&result, NULL);
#else
        result = fread(p, 1, count, mHandle);
#endif
    }

    return result;
}

//inline void File::flush()
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
inline int64_t File::getSize() const
{
#if defined(_WINDOWS)
    LARGE_INTEGER move = {0, 0};
    LARGE_INTEGER pos;
    LARGE_INTEGER size;

    ::SetFilePointerEx(mHandle, move, &pos,  FILE_CURRENT);
    ::SetFilePointerEx(mHandle, move, &size, FILE_END);
    ::SetFilePointerEx(mHandle, pos,  NULL,  FILE_BEGIN);

    return size.QuadPart;
#else
    int64_t pos = ftell(mHandle);
    fseek(mHandle, 0, SEEK_END);

    int64_t size = ftell(mHandle);
    fseek(mHandle, pos, SEEK_SET);

    return size;
#endif
}

/*!
 *  \brief  ファイルポインタの現在位置取得
 */
inline int64_t File::getPosition() const
{
#if defined(_WINDOWS)
    LARGE_INTEGER move = {0, 0};
    LARGE_INTEGER pos;

    ::SetFilePointerEx(mHandle, move, &pos, FILE_CURRENT);
    return pos.QuadPart;
#else
    int64_t pos = ftell(mHandle);
    return pos;
#endif
}

/*!
 *  \brief  ファイル削除
 */
inline void File::unlink(const CoreString& fileName) throw(Exception)
{
    const char* p = fileName.getBuffer();

#if defined(_WINDOWS)
    bool result = (::DeleteFileA(p) == TRUE);
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

} // namespace slog
