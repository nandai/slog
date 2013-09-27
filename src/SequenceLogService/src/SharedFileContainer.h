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
 *  \file   SequenceLogService.h
 *  \brief  �V�[�P���X���O�T�[�r�X�N���X
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once

#include "slog/File.h"
#include "slog/Mutex.h"

namespace slog
{
/*!
 *  \brief  ���L�t�@�C���R���e�i�N���X
 */
class SharedFileContainer
{
            File                    mFile;                      //!< ���L�t�@�C��
            FixedString<MAX_PATH>   mBaseFileName;              //!< �x�[�X�t�@�C����
            FileInfo*               mFileInfo;                  //!< �V�[�P���X���O�t�@�C�����
            Mutex                   mMutex;                     //!< �~���[�e�b�N�X
            int32_t                 mReferenceCount;            //!< �Q�ƃJ�E���g

            /*!
             * �R���X�g���N�^
             */
public:     SharedFileContainer()
            {
                mFileInfo = nullptr;
                mReferenceCount = 1;
            }

            /*!
             * ���L�t�@�C��
             */
            File* getFile() const {return (File*)&mFile;}

            /*!
             * �x�[�X�t�@�C����
             */
            CoreString* getBaseFileName() const {return (CoreString*)&mBaseFileName;}

            /*!
             * �V�[�P���X���O�t�@�C�����
             */
            FileInfo* getFileInfo() const {return (FileInfo*)mFileInfo;}
            void setFileInfo(FileInfo* fileInfo) {mFileInfo = fileInfo;}

            /*!
             * �~���[�e�b�N�X
             */
            Mutex* getMutex() const {return (Mutex*)&mMutex;}

            /*!
             * �Q�ƃJ�E���g
             */
            void addReference() {mReferenceCount++;}
            bool removeReference()
            {
                mReferenceCount--;
                return (mReferenceCount == 0);
            }
};

} // namespace slog
