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
 *  \file   SharedMemory.h
 *  \brief  共有メモリクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once
#include "slog/Exception.h"

#if defined(__unix__)
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/mman.h>
#endif

namespace slog
{

/*!
 *  \brief  共有メモリクラス
 */
template <class T>
class SharedMemory
{
#if defined(_WINDOWS)
            HANDLE      mHandle;            //!< 共有メモリハンドル
#else
            int         mHandle;            //!< 共有メモリハンドル

            bool        mCreate;            //!< 作成フラグ
            char        mName[MAX_PATH];    //!< 共有メモリ名
#endif

            T           mPointer;           //!< 共有メモリポインタ
            uint32_t    mSize;              //!< 共有メモリサイズ

public:      SharedMemory();
            ~SharedMemory();

            T operator->() const;
            T getBuffer() const;

            void create(const CoreString& name, uint32_t size) throw(Exception);
            void open(  const CoreString& name) throw(Exception);
            void close();

            bool isOpen() const;
            uint32_t getSize() const;

private:    void init();
};

/*!
 *  \brief  コンストラクタ
 */
template <class T>
inline SharedMemory<T>::SharedMemory()
{
    init();
}

/*!
 *  \brief  デストラクタ
 */
template <class T>
inline SharedMemory<T>::~SharedMemory()
{
    close();
}

/*!
 *  \brief  実体を取得する
 */
template <class T>
inline T SharedMemory<T>::operator->() const
{
    return mPointer;
}

/*!
 *  \brief  実体を取得する
 */
template <class T>
inline T SharedMemory<T>::getBuffer() const
{
    return mPointer;
}

/*!
 *  \brief  作成
 */
template <class T>
inline void SharedMemory<T>::create(const CoreString& name, uint32_t size) throw(Exception)
{
    Exception e;
    const char* p = name.getBuffer();

#if defined(_WINDOWS)
    HANDLE handle = CreateFileMappingA((HANDLE)-1, NULL, PAGE_READWRITE, 0, size, p);

    if (handle == NULL)
    {
        e.setMessage("SharedMemory<T>::create(\"%s\", %u)", p, size);
        throw e;
    }

    T pointer = (T)MapViewOfFile(handle, FILE_MAP_WRITE, 0, 0, 0);

    if (pointer == NULL)
    {
        e.setMessage("SharedMemory<T>::create(\"%s\", %u)", p, size);
        CloseHandle(handle);

        throw e;
    }
#else
    umask(0);
    int handle = ::open(p, O_RDWR | O_CREAT, 0666);

    if (handle == -1)
    {
        e.setMessage("SharedMemory<T>::create(\"%s\", %u)", p, size);
        throw e;
    }

//  int32_t pageSize = sysconf(_SC_PAGE_SIZE);
//  size = (size / pageSize + 1) * pageSize;

    if (ftruncate(handle, size) == -1)
    {
        e.setMessage("SharedMemory<T>::create(\"%s\", %u)", p, size);
        ::close(handle);

        throw e;
    }

    T pointer = (T)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);

    if (pointer == MAP_FAILED)
    {
        e.setMessage("SharedMemory<T>::create(\"%s\", %u)", p, size);
        ::close(handle);

        throw e;
    }

    mCreate = true;
    strcpy(mName, p);
#endif

    mHandle = handle;
    mPointer = pointer;
    mSize = size;
}

/*!
 *  \brief  オープン
 */
template <class T>
inline void SharedMemory<T>::open(const CoreString& name) throw(Exception)
{
    Exception e;
    const char* p = name.getBuffer();

#if defined(_WINDOWS)
    mHandle = OpenFileMappingA(FILE_MAP_WRITE, FALSE, p);

    if (mHandle == NULL)
    {
        e.setMessage("SharedMemory<T>::open(\"%s\")", p);
        throw e;
    }

    mPointer = (T)MapViewOfFile(mHandle, FILE_MAP_WRITE, 0, 0, 0);

    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(mPointer, &mbi, sizeof(mbi));

    mSize = (uint32_t)mbi.RegionSize;
#else
    int handle = ::open(p, O_RDWR);

    if (handle == -1)
    {
        e.setMessage("SharedMemory<T>::open(\"%s\")", p);
        throw e;
    }

    struct stat statbuf;
    fstat(handle, &statbuf);

    mHandle = handle;
    mPointer = (T)mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, mHandle, 0);

    mCreate = false;
    mSize = statbuf.st_size;
#endif
}

/*!
 *  \brief  クローズ
 */
template <class T>
inline void SharedMemory<T>::close()
{
    if (isOpen() == false)
        return;

#if defined(_WINDOWS)
    UnmapViewOfFile(mPointer);
    CloseHandle(mHandle);
#else
    struct stat buf;
    fstat(mHandle, &buf);

    munmap(mPointer, buf.st_size);
    ::close(mHandle);

    if (mCreate)
        unlink(mName);
#endif

    init();
}

/*!
 *  \brief  オープンしているかどうか調べる
 */
template <class T>
inline bool SharedMemory<T>::isOpen() const
{
    return (mHandle != 0/*NULL*/);
}

/*!
 *  \brief  共有メモリサイズ取得
 */
template <class T>
inline uint32_t SharedMemory<T>::getSize() const
{
    return mSize;
}

/*!
 *  \brief  初期化
 */
template <class T>
inline void SharedMemory<T>::init()
{
    mHandle = 0/*NULL*/;
    mPointer = NULL;
    mSize = 0;
}

} // namespace slog
