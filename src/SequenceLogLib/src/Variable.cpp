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
 *  \file   Variable.cpp
 *  \brief  �ϐ����X�g�N���X
 *  \author Copyright 2014 printf.jp
 */
#include "slog/Variable.h"

namespace slog
{

/*!
 * \brief   �R���X�g���N�^
 */
Variable::Variable(const char* name, const char* value)
{
    this->name. copy(name);
    this->value.copy(value);
}

/*!
 * \brief   �R���X�g���N�^
 */
Variable::Variable(const char* name, int32_t value)
{
    this->name. copy(name);
    this->value.format("%d", value);
}

/*!
 * \brief   �f�X�g���N�^
 */
VariableList::~VariableList()
{
    for (auto i = mList.begin(); i != mList.end(); i++)
        delete *i;
}

/*!
 * \brief   �ϐ����擾����
 */
const Variable* VariableList::get(int32_t index) const
{
    auto i = mList.begin();

    for (int32_t count = index; 0 < count; count--)
        i++;

    return *i;
}

/*!
 * \brief   �ϐ�����������
 *
 * \param[in]   name    �ϐ���
 *
 * \return  �l
 */
const CoreString* VariableList::find(const CoreString* name) const
{
    for (auto i = mList.begin(); i != mList.end(); i++)
    {
        auto variable = *i;

        if (name->equals(variable->name))
            return &variable->value;
    }

    return nullptr;
}

/*!
 * \brief   �ϐ���ǉ�����
 *
 * \param[in]   name    �ϐ���
 * \param[in]   value   �l
 *
 * \return  �Ȃ�
 */
void VariableList::add(const CoreString* name, const CoreString* value)
{
    add(name->getBuffer(), value->getBuffer());
}

/*!
 * \brief   �ϐ���ǉ�����
 *
 * \param[in]   name    �ϐ���
 * \param[in]   value   �l
 *
 * \return  �Ȃ�
 */
void VariableList::add(const char* name, const CoreString* value)
{
    add(name, value->getBuffer());
}

/*!
 * \brief   �ϐ���ǉ�����
 *
 * \param[in]   name    �ϐ���
 * \param[in]   value   �l
 *
 * \return  �Ȃ�
 */
void VariableList::add(const char* name, const char* value)
{
    mList.push_back(new Variable(name, value));
}

/*!
 * \brief   �ϐ���ǉ�����
 *
 * \param[in]   name    �ϐ���
 * \param[in]   value   �l
 *
 * \return  �Ȃ�
 */
void VariableList::add(const char* name, int32_t value)
{
    mList.push_back(new Variable(name, value));
}

} // namespace slog
