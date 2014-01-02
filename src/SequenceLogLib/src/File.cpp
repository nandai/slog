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
#include "slog/FileInfo.h"
#include "slog/String.h"
#include "slog/ByteBuffer.h"

#include <list>

#if defined(_WINDOWS)
    #include <windows.h>
#endif

#if defined(__unix__)
    #include <stdio.h>
    #include <unistd.h>
#endif

namespace slog
{

class FileCache
{
            const FileInfo* mFileInfo;
            const ByteBuffer* mBuffer;

            /*!
             * コンストラクタ
             */
public:     FileCache(const FileInfo* fileInfo);

            /*!
             * デストラクタ
             */
            ~FileCache();

            /*!
             * ファイル情報取得
             */
            const FileInfo* getFileInfo() const {return mFileInfo;}

            /*!
             * バッファ取得
             */
            const ByteBuffer* getBuffer() const {return mBuffer;}

            /*!
             * バッファ設定
             */
            void setBuffer(const ByteBuffer* buffer);

};

/*!
 * コンストラクタ
 */
FileCache::FileCache(const FileInfo* fileInfo)
{
    mFileInfo = fileInfo;
    mBuffer = nullptr;
}

/*!
 * デストラクタ
 */
FileCache::~FileCache()
{
    delete mFileInfo;
    delete mBuffer;
}

/*!
 * バッファ設定
 */
void FileCache::setBuffer(const ByteBuffer* buffer)
{
    delete mBuffer;
    mBuffer = buffer;
}

class FileCacheList
{
            std::list<FileCache*> mList;

            /*!
             * デストラクタ
             */
public:     ~FileCacheList();

            /*!
             * ファイルキャッシュ追加
             */
            void add(FileCache* fileBuffer);

            /*!
             * ファイルキャッシュ取得
             */
            FileCache* get(const CoreString* path) const;
};

/*!
 * デストラクタ
 */
FileCacheList::~FileCacheList()
{
    for (auto  fileBuffer : mList)
        delete fileBuffer;

    mList.clear();
}

/*!
 * ファイルキャッシュ追加
 */
void FileCacheList::add(FileCache* fileBuffer)
{
    mList.push_back(fileBuffer);
}

/*!
 * ファイルキャッシュ取得
 */
FileCache* FileCacheList::get(const CoreString* path) const
{
    for (auto fileBuffer : mList)
    {
        if (fileBuffer->getFileInfo()->getCanonicalPath().equals(*path))
            return fileBuffer;
    }

    return nullptr;
}

FileCacheList fileCacheList;  // 動作確認用

class File::IO
{
            /*!
             * オープンしているか調べる
             */
public:     virtual bool isOpen() const = 0;

            /*!
             * オープン
             */
            virtual bool open(const CoreString& fileName, File::Mode mode) = 0;

            /*!
             * クローズ
             */
            virtual void close() = 0;

            /*!
             * 読み込み
             */
            virtual int64_t read(char* buffer, int64_t count) = 0;

            /*!
             * 書き込み
             */
            virtual void write(const char* buffer, int64_t count) = 0;

            /*!
             * ファイルポインタの現在位置設定
             */
            virtual void setPosition(int64_t pos) = 0;

            /*!
             * ファイルポインタ移動
             */
            virtual int64_t movePosition(int64_t count) = 0;

            /*!
             *  \brief  ファイルポインタ移動
             */
            virtual int64_t moveLastPosition() = 0;
};

class File::FileIO : public File::IO
{
            /*!
             * ファイルハンドル
             */
#if defined(_WINDOWS)
            HANDLE mHandle;
#else
            FILE*  mHandle;
#endif

            /*!
             * コンストラクタ
             */
public:     FileIO();

            /*!
             * オープンしているか調べる
             */
            virtual bool isOpen() const {return (mHandle != 0);}

            /*!
             * オープン
             */
            virtual bool open(const CoreString& fileName, File::Mode mode);

            /*!
             * クローズ
             */
            virtual void close();

            /*!
             * 読み込み
             */
            virtual int64_t read(char* buffer, int64_t count);

            /*!
             * 書き込み
             */
            virtual void write(const char* buffer, int64_t count);

            /*!
             * ファイルポインタの現在位置設定
             */
            virtual void setPosition(int64_t pos);

            /*!
             * ファイルポインタ移動
             */
            virtual int64_t movePosition(int64_t count);

            /*!
             * ファイルポインタ移動
             */
            virtual int64_t moveLastPosition();
};

/*!
 *  \brief  コンストラクタ
 */
File::FileIO::FileIO()
{
    mHandle = 0;
}

/*!
 *  \brief  オープン
 */
bool File::FileIO::open(const CoreString& fileName, File::Mode mode)
{
#if defined(_WINDOWS)
    UTF16LE utf16le;
    utf16le.conv(fileName);

    const wchar_t* p = utf16le.getBuffer();
    HANDLE handle;

    if (mode == File::READ)
    {
        // 書込み中のファイルを読めるようにFILE_SHARE_WRITEを付ける
//      handle = CreateFileW(p, GENERIC_READ,  FILE_SHARE_READ,                    nullptr, OPEN_EXISTING, 0, nullptr);
        handle = CreateFileW(p, GENERIC_READ,  FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    }
    else
    {
//      handle = CreateFileW(p, GENERIC_WRITE, 0,               nullptr, CREATE_ALWAYS, 0, nullptr);
        handle = CreateFileW(p, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, nullptr);
    }

    if (handle == INVALID_HANDLE_VALUE)
        return false;

    mHandle = handle;
#else
    const char* p =  fileName.getBuffer();
    const char* _mode = (mode == READ ? "r" : "w");
    FILE* handle = fopen(p, _mode);

    if (handle == nullptr)
        return false;

    mHandle = handle;
#endif

    return true;
}

/*!
 *  \brief  クローズ
 */
void File::FileIO::close()
{
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
int64_t File::FileIO::read(char* buffer, int64_t count)
{
	int32_t result = 0;

#if defined(_WINDOWS)
    ::ReadFile((HANDLE)mHandle, buffer, (int32_t)count, (DWORD*)&result, nullptr);
#else
    result = fread(buffer, 1, count, (FILE*)mHandle);
#endif

    return result;
}

/*!
 *  \brief  書き込み
 */
void File::FileIO::write(const char* buffer, int64_t count)
{
#if defined(_WINDOWS)
    DWORD result = 0;
    ::WriteFile(mHandle, buffer, (int32_t)count, &result, nullptr);
#else
    fwrite(p, 1, count, mHandle);
#endif
}

/*!
 *  \brief  ファイルポインタの現在位置設定
 */
void File::FileIO::setPosition(int64_t pos)
{
#if defined(_WINDOWS)
    LARGE_INTEGER move;
    move.QuadPart = pos;

    ::SetFilePointerEx(mHandle, move, nullptr, FILE_BEGIN);
#else
    fseek(mHandle, pos, SEEK_SET);
#endif
}

/*!
 *  \brief  ファイルポインタ移動
 */
int64_t File::FileIO::movePosition(int64_t count)
{
#if defined(_WINDOWS)
    LARGE_INTEGER move;
    move.QuadPart = count;

    LARGE_INTEGER pos;
    ::SetFilePointerEx(mHandle, move, &pos, FILE_CURRENT);

    return pos.QuadPart;
#else
    fseek(mHandle, count, SEEK_CUR);

    int64_t pos = ftell(mHandle);
    return  pos;
#endif
}

/*!
 *  \brief  ファイルポインタ移動
 */
int64_t File::FileIO::moveLastPosition()
{
#if defined(_WINDOWS)
    LARGE_INTEGER move;
    move.QuadPart = 0;

    LARGE_INTEGER pos;
    ::SetFilePointerEx(mHandle, move, &pos, FILE_END);

    return pos.QuadPart;
#else
    fseek(mHandle, 0, SEEK_END);

    int64_t pos = ftell(mHandle);
    return  pos;
#endif
}

class File::CacheIO : public File::IO
{
            const ByteBuffer* mBuffer;
            int64_t mPosition;

            /*!
             * コンストラクタ
             */
public:     CacheIO(const ByteBuffer* buffer);

            /*!
             * オープンしているか調べる
             */
            virtual bool isOpen() const;

            /*!
             * オープン
             */
            virtual bool open(const CoreString& fileName, File::Mode mode);

            /*!
             * クローズ
             */
            virtual void close();

            /*!
             * 読み込み
             */
            virtual int64_t read(char* buffer, int64_t count);

            /*!
             * 書き込み
             */
            virtual void write(const char* buffer, int64_t count);

            /*!
             * ファイルポインタの現在位置設定
             */
            virtual void setPosition(int64_t pos);

            /*!
             * ファイルポインタ移動
             */
            virtual int64_t movePosition(int64_t count);

            /*!
             * ファイルポインタ移動
             */
            virtual int64_t moveLastPosition();
};

/*!
 *  \brief  コンストラクタ
 */
File::CacheIO::CacheIO(const ByteBuffer* buffer)
{
    mBuffer = buffer;
    mPosition = 0;
}

/*!
 * オープンしているか調べる
 */
bool File::CacheIO::isOpen() const
{
    return true;
}

/*!
 *  \brief  オープン
 */
bool File::CacheIO::open(const CoreString& fileName, File::Mode /*mode*/)
{
    return true;
}

/*!
 *  \brief  クローズ
 */
void File::CacheIO::close()
{
}

/*!
 *  \brief  読み込み
 */
int64_t File::CacheIO::read(char* buffer, int64_t count)
{
    int64_t capacity = mBuffer->getCapacity();

    if (count > capacity - mPosition)
        count = capacity - mPosition;

    memcpy(buffer, mBuffer->getBuffer() + mPosition, (size_t)count);
    mPosition += count;

    return count;
}

/*!
 *  \brief  書き込み
 */
void File::CacheIO::write(const char* buffer, int64_t count)
{
    noticeLog("*** File::CacheIO::write() not implement.");
}

/*!
 *  \brief  ファイルポインタの現在位置設定
 */
void File::CacheIO::setPosition(int64_t pos)
{
    mPosition = pos;
}

/*!
 *  \brief  ファイルポインタ移動
 */
int64_t File::CacheIO::movePosition(int64_t count)
{
    mPosition += count;
    return mPosition;
}

/*!
 *  \brief  ファイルポインタ移動
 */
int64_t File::CacheIO::moveLastPosition()
{
    mPosition = mBuffer->getCapacity();
    return mPosition;
}

/*!
 *  \brief  コンストラクタ
 */
File::File()
{
    mIO = nullptr;
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
    return (mIO && mIO->isOpen());
}

/*!
 *  \brief  オープン
 */
void File::open(
    const CoreString& fileName,     //!< ファイル名
    Mode mode)                      //!< オープンモード

    throw(Exception)
{
    FileInfo* info = new FileInfo(fileName);
    FileCache* fileCache = fileCacheList.get(&info->getCanonicalPath());

    if (fileCache == nullptr || mode == Mode::WRITE)
    {
        Exception e;

        if (mIO)
        {
            e.setMessage("File::open(\"%s\") : already opened.", fileName.getBuffer());
            throw e;
        }

        mIO = new FileIO;

        if (mIO->open(fileName, mode) == false)
        {
            e.setMessage("File::open(\"%s\")", fileName.getBuffer());
            throw e;
        }
    }

#if 0
    if (mode == Mode::READ)
    {
        if (fileCache == nullptr)
        {
            int64_t size = getSize();
            ByteBuffer* buffer = new ByteBuffer((int32_t)size);

            read(buffer, size);
            close();

            mIO = new CacheIO(buffer);

            fileCache = new FileCache(info);
            fileCache->setBuffer(buffer);

            fileCacheList.add(fileCache);
        }
        else
        {
            mIO = new CacheIO(fileCache->getBuffer());
            delete info;
        }
    }
    else
#endif
    {
        delete info;
    }
}

/*!
 *  \brief  クローズ
 */
void File::close()
{
    if (isOpen())
        mIO->close();

    delete mIO;
    mIO = nullptr;
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
    int64_t count = sizeof(buffer);
    int64_t index;
    int64_t result = 0;
    bool first = true;

    str->setLength(0);

    do
    {
        result = mIO->read(p, count);

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

        str->append(p, (int32_t)index);

        if (index < result)
            break;
    }
    while (true);

    // 読み込みすぎた部分を戻す
    movePosition(index - result + 1);

    if (p[index] == '\n')
        return true;

    mIO->read(p, 1);

    if (p[0] != '\n')
        movePosition(-1);

    return true;
}

/*!
 *  \brief  読み込み
 */
int64_t File::read(Buffer* buffer, int64_t count) const throw(Exception)
{
    return read(buffer, 0, count);
}

/*!
 *  \brief  読み込み
 */
int64_t File::read(Buffer* buffer, int64_t position, int64_t count) const throw(Exception)
{
    if (isOpen() == false)
        return 0;

    buffer->validateOverFlow((int32_t)position, (int32_t)count);
    return mIO->read(buffer->getBuffer() + position, count);
}

/*!
 *  \brief  書き込み
 */
void File::write(const Buffer* buffer, int64_t count) const throw(Exception)
{
    write(buffer, 0, count);
}

/*!
 *  \brief  書き込み
 */
void File::write(const Buffer* buffer, int64_t position, int64_t count) const throw(Exception)
{
    if (isOpen() == false)
        return;

    buffer->validateOverFlow((int32_t)position, (int32_t)count);
    mIO->write(buffer->getBuffer() + position, count);
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
//  if (mHandle != nullptr)
//  {
//#if defined(_WINDOWS)
//      ::FlushFileBuffers(mHandle);
//#else
//      fflush(mHandle);
//#endif
//  }
//}

/*!
 * 
 */
bool File::isEOF() const
{
    int64_t pos =  getPosition();
    int64_t size = moveLastPosition();
                   setPosition(pos);

    return (pos >= size);
}

/*!
 *  \brief  ファイルサイズ取得
 */
int64_t File::getSize() const
{
    int64_t pos =  getPosition();
    int64_t size = moveLastPosition();
                   setPosition(pos);

    return size;
}

/*!
 *  \brief  ファイルポインタの現在位置取得
 */
int64_t File::getPosition() const
{
    return movePosition(0);
}

/*!
 *  \brief  ファイルポインタの現在位置設定
 */
void File::setPosition(int64_t pos) const
{
    mIO->setPosition(pos);
}

/*!
 *  \brief  ファイルポインタ移動
 */
int64_t File::movePosition(int64_t count) const
{
    return mIO->movePosition(count);
}

/*!
 *  \brief  ファイルポインタ移動
 */
int64_t File::moveLastPosition() const
{
    return mIO->moveLastPosition();
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
    noticeLog("*** File::copy() no implement.");
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
    noticeLog("*** File::move() no implement.");
#endif
}

} // namespace slog
