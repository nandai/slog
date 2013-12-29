/*
 * Copyright (C) 2013 printf.jp
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
 *  \file   HtmlGenerator.h
 *  \brief  html�����N���X
 *  \author Copyright 2013 printf.jp
 */
#pragma once

#include "slog/String.h"
#include <list>

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
public:     slog::String name;

            /*!
             * �l
             */
            slog::String value;

            /*!
             * �R���X�g���N�^
             */
public:     Variable(const char* name, const char* value)
            {
                this->name. copy(name);
                this->value.copy(value);
            }
};

/*!
 * \brief  �ϐ����X�g
 */
typedef std::list<Variable*> VariableList;

/*!
 * \brief  html�����N���X
 */
class SLOG_API HtmlGenerator
{
            class Param;

            /*!
             * ��������html
             */
            slog::String mHtml;

            /*!
             * �R���X�g���N�^
             */
public:     HtmlGenerator() {}

            /*!
             * �^�O���X�L�b�v����
             */
private:    int32_t skipTags(const slog::CoreString* readHtml, int32_t pos, int32_t depth);

            /*!
             * �^�O���X�L�b�v����
             */
            int32_t skipTags(const slog::CoreString* readHtml, int32_t pos);

            /*!
             * �ϐ���l�ɒu������
             */
            bool replaceVariable(Param* param, const slog::CoreString* var);

            /*!
             * �u������
             */
            void replace(Param* param, const slog::CoreString* var);

            /*!
             * html��ǂݍ���
             */
public:     static bool readHtml(slog::CoreString* readHtml, const slog::CoreString* fileName);

            /*!
             * html���������s����
             */
public:     bool execute(const slog::CoreString* fileName, const VariableList* variableList);

            /*!
             * html���������s����
             */
private:    bool execute(const slog::CoreString* fileName, const VariableList* variableList, int32_t depth);

            /*!
             * html���擾����
             */
public:    const slog::CoreString* getHtml() const {return &mHtml;}
};

} // namespace slog
