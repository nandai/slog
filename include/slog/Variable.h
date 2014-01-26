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
 *  \file   Variable.h
 *  \brief  �ϐ����X�g�N���X
 *  \author Copyright 2014 printf.jp
 */
#pragma once

#include "slog/String.h"
#include <list>

#pragma warning(disable:4251)

namespace slog
{

/*!
 * \brief  �ϐ��N���X
 */
class SLOG_API Variable
{
            /*!
             * �ϐ���
             */
public:     String name;

            /*!
             * �l
             */
            String value;

            /*!
             * �R���X�g���N�^
             */
public:     Variable(const char* name, const char* value);

            /*!
             * �R���X�g���N�^
             */
            Variable(const char* name, int32_t value);
};

/*!
 * \brief  �ϐ����X�g
 */
class SLOG_API VariableList
{
            std::list<Variable*> mList;

            /*!
             * �f�X�g���N�^
             */
public:     ~VariableList();

            /*!
             * �ϐ��̐����擾����
             */
            int32_t getCount() const {return (int32_t)mList.size();}

            /*!
             * �ϐ����擾����
             */
            const Variable* get(int32_t index) const;

            /*!
             * �ϐ�����������
             */
            const CoreString* find(const CoreString* name) const;

            /*!
             * �ϐ���ǉ�����
             */
            void add(const CoreString* name, const CoreString* value);

            /*!
             * �ϐ���ǉ�����
             */
            void add(const char* name, const CoreString* value);

            /*!
             * �ϐ���ǉ�����
             */
            void add(const char* name, const char* value);

            /*!
             * �ϐ���ǉ�����
             */
            void add(const char* name, int32_t value);
};

} // namespace slog
