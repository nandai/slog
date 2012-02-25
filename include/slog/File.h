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
class File
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

            void open(const CoreString& fileName, Mode mode) throw(Exception);
            void close();

            void write(const Buffer* buffer, int32_t count) const throw(Exception);
            void write(const Buffer* buffer, int32_t position, int32_t count) const throw(Exception);

            bool read(CoreString* str) throw(Exception);

//          void flush();

            uint32_t getSize() const;

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
inline uint32_t File::getSize() const
{
#if defined(_WINDOWS)
    DWORD size = ::GetFileSize(mHandle, NULL);
    return size;
#else
    uint32_t size = ftell(mHandle);
    return size;
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
