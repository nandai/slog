/*
 * Copyright (C) 2014 printf.jp
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
 *  \file   SequenceLogFileManager.cpp
 *  \brief  シーケンスログファイルマネージャークラス
 *  \author Copyright 2014 printf.jp
 */
#include "SequenceLogFileManager.h"
#include "SharedFileContainer.h"

#include "slog/FileInfo.h"

#if defined(__unix__)
    #include <string.h>
    #include <algorithm>
#endif

namespace slog
{

/*!
 * \brief   シーケンスログファイル情報比較
 */
struct CompareFileInfo
{
    bool operator() (const FileInfo* info1, const FileInfo* info2) const
    {
        uint64_t value1 = info1->getLastWriteTime().getValue();
        uint64_t value2 = info2->getLastWriteTime().getValue();

        if (value1 < value2)
            return true;

        if (value1 > value2)
            return false;

        value1 = info1->getCreationTime().getValue();
        value2 = info2->getCreationTime().getValue();

        if (value1 < value2)
            return true;

        if (value1 > value2)
            return false;

        int32_t result = strcmp(
            info1->getCanonicalPath()->getBuffer(),
            info2->getCanonicalPath()->getBuffer());

        if (result < 0)
            return true;

        return false;
    }
};

/*!
 * \brief   コンストラクタ
 */
SequenceLogFileManager::SequenceLogFileManager(int32_t userId, uint32_t maxFileSize, int32_t maxFileCount)
{
    mUserId = userId;
    mMaxFileSize = maxFileSize;
    mMaxFileCount = maxFileCount;
}

/*!
 * \brief   デストラクタ
 */
SequenceLogFileManager::~SequenceLogFileManager()
{
    // 共有ファイルコンテナ情報削除
    for (auto i = mSharedFileContainerArray.begin(); i != mSharedFileContainerArray.end(); i++)
        delete *i;

    mSharedFileContainerArray.clear();

    // シーケンスログファイル情報削除
    clearFileInfoList();
}

/*!
 * \brief   共有ファイルコンテナ取得
 */
SharedFileContainer* SequenceLogFileManager::getSharedFileContainer(const CoreString* baseFileName)
{
    SharedFileContainer* container;

    for (auto i = mSharedFileContainerArray.begin(); i != mSharedFileContainerArray.end(); i++)
    {
        container = *i;

        if (container->getBaseFileName()->equals(baseFileName))
        {
            // ベースファイル名が同じだったら既存の共有ファイルコンテナを返す
            container->addReference();
            return container;
        }
    }

    // 該当する共有ファイルコンテナがなかったら新規作成
    container = new SharedFileContainer();
    container->getBaseFileName()->copy(baseFileName);

    mSharedFileContainerArray.push_back(container);
    return container;
}

/*!
 * \brief   共有ファイルコンテナリリース
 */
void SequenceLogFileManager::releaseSharedFileContainer(SharedFileContainer* container)
{
    if (container == nullptr)
        return;

    if (container->removeReference() == false)
        return;

    // 共有ファイルクローズ
    container->getFile()->close();

    // 共有ファイル情報更新
    FileInfo* fileInfo = container->getFileInfo();

    if (fileInfo)
        fileInfo->update();

    // リストから除外
    auto i = std::find(mSharedFileContainerArray.begin(), mSharedFileContainerArray.end(), container);
    mSharedFileContainerArray.erase(i);

    // 共有ファイルコンテナ削除
    delete container;
}

/*!
 * \brief   シーケンスログファイル情報登録
 */
void SequenceLogFileManager::addFileInfo(FileInfo* info)
{
    mFileInfoArray.push_back(info);

    // ログファイルの上限と現在のファイル数を取得
    int32_t maxCount = mMaxFileCount;
    int32_t size = (int32_t)mFileInfoArray.size();

    // 古いファイルを削除
    auto i = mFileInfoArray.begin();

    while (i != mFileInfoArray.end())
    {
        if (maxCount == 0 || size <= maxCount)
            break;

        try
        {
            FileInfo* info = *i;

            if (info->isUsing() == false)
            {
                // 使用されていない（オープンされていない）場合

                // ファイルを削除
                File::unlink(info->getCanonicalPath());

                // リストからファイル情報を除外
                i = mFileInfoArray.erase(i);

                // ファイル情報を削除
                delete info;
            }
            else
            {
                // 使用中
                i++;
            }
        }
        catch (Exception e)
        {
            e;
//          noticeLog("    %s\n", e.getMessage());
            i++;
        }

        size--;
    }
}

/*!
 * シーケンスログファイル情報取得
 */
std::list<FileInfo*>* SequenceLogFileManager::getFileInfoList() const
{
    return (std::list<FileInfo*>*)&mFileInfoArray;
}

/*!
 * \brief   
 */
void SequenceLogFileManager::clearFileInfoList()
{
    for (auto i = mFileInfoArray.begin(); i != mFileInfoArray.end(); i++)
        delete *i;

    mFileInfoArray.clear();
}

/*!
 * \brief   
 */
void SequenceLogFileManager::enumFileInfoList(const CoreString* dirName)
{
    if (mFileInfoArray.size() > 0)
        return;

    FixedString<MAX_PATH> path;
    FileFind find;
    find.setListener(this);

    path.format("%s%c%08d%c*.slog", dirName->getBuffer(), PATH_DELIMITER, mUserId, PATH_DELIMITER);
    find.exec(&path);

    path.format("%s%c%08d%c*.log",  dirName->getBuffer(), PATH_DELIMITER, mUserId, PATH_DELIMITER);
    find.exec(&path);

    mFileInfoArray.sort(CompareFileInfo());
}

/*!
 * \brief   
 */
void SequenceLogFileManager::onFind(const CoreString& path)
{
    FileInfo* info = new FileInfo(&path);

    if (info->isFile() == false)
    {
        delete info;
        return;
    }

    mFileInfoArray.push_back(info);
}

/*!
 * シーケンスログファイルマネージャー追加
 */
void SequenceLogFileManagerList::add(SequenceLogFileManager* sequenceLogFileManager)
{
    mList.push_back(sequenceLogFileManager);
}

/*!
 * \brief   シーケンスログファイルマネージャー取得
 */
SequenceLogFileManager* SequenceLogFileManagerList::get(int32_t accountId)
{
    for (auto i = mList.begin(); i != mList.end(); i++)
    {
        SequenceLogFileManager* sequenceLogFileManager = *i;

        if (sequenceLogFileManager->getUserId() == accountId)
            return sequenceLogFileManager;
    }

    return nullptr;
}

/*!
 * \brief   クリア
 */
void SequenceLogFileManagerList::clear()
{
    for (auto i = mList.begin(); i != mList.end(); i++)
        delete *i;

    mList.clear();
}

/*!
 * \brief   最大ファイルサイズ設定
 */
void SequenceLogFileManagerList::setMaxFileSize(uint32_t maxFileSize)
{
    for (auto i = mList.begin(); i != mList.end(); i++)
        (*i)->setMaxFileSize(maxFileSize);
}

/*!
 * \brief   最大ファイル数設定
 */
void SequenceLogFileManagerList::setMaxFileCount(int32_t maxFileCount)
{
    for (auto i = mList.begin(); i != mList.end(); i++)
        (*i)->setMaxFileCount(maxFileCount);
}

} // namespace slog
