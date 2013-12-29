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
 *  \file   HtmlGenerator.cpp
 *  \brief  html�����N���X
 *  \author Copyright 2013 printf.jp
 */
#include "slog/HtmlGenerator.h"

#include "slog/File.h"
#include "slog/MimeType.h"

namespace slog
{

/*!
 * �������s�p�����[�^�N���X
 */
class HtmlGenerator::Param
{
            /*!
             * �ǂݍ��񂾃t�@�C���̓��e
             */
public:     slog::String buffer;

            /*!
             * �f�t�H���g�ϐ����X�g
             */
            VariableList defaultVariableList;

            /*!
             * �ϐ����X�g
             */
            const VariableList* variableList = &defaultVariableList;

            /*!
             * ��͏I���ʒu
             */
            int32_t endPosition = 0;

            /*!
             * �u������
             */
            bool replaceResult = false;

            /*!
             * �R���X�g���N�^
             */
public:     Param(const VariableList* variableList);

            /*!
             * �f�t�H���g�̕ϐ����X�g�����ׂ�
             */
            bool isDefaultVariableList() const;
};

/*!
 * \brief   �R���X�g���N�^
 *
 * \param [in]  variableList    �ϐ����X�g
 */
HtmlGenerator::Param::Param(const VariableList* variableList)
{
    if (variableList != nullptr)
        this->variableList = variableList;
}

/*!
 * �f�t�H���g�̕ϐ����X�g�����ׂ�
 */
bool HtmlGenerator::Param::isDefaultVariableList() const
{
    return (variableList == &defaultVariableList);
}

/*!
 * \brief   �^�O���X�L�b�v����
 *
 * \param [in]  readHtml    �ǂݍ���html
 * \param [in]  pos         �X�L�b�v�J�n�ʒu
 * \param [in]  depth       �X�L�b�v�J�n�ʒu����̉�͐[�x�B0�ŉ�͂��I������B
 *
 * \return  �X�L�b�v�I���ʒu�i�X�L�b�v�����Ō�̃^�O��">"�̈ʒu�j
 */
int32_t HtmlGenerator::skipTags(const slog::CoreString* readHtml, int32_t pos, int32_t depth)
{
    int32_t startPos;
    int32_t endPos;

    while (true)
    {
        startPos = readHtml->indexOf("<", pos);
        endPos =   readHtml->indexOf(">", pos);

        if (startPos == -1 || endPos == -1)
        {
            endPos = pos;
            break;
        }

        String tag(readHtml->getBuffer() + startPos + 1, (endPos - 1) - startPos);

        if (tag[0] == '/')
        {
            // �I���^�O�Ȃ烋�[�v�𔲂���
            break;
        }

        if (tag[tag.getLength() - 1] != '/')
            endPos = skipTags(readHtml, endPos + 1, depth + 1);

        if (depth == 0)
        {
            // �[�x0�Ȃ烋�[�v�𔲂���
            break;
        }

        pos = endPos + 1;
    }

    return endPos;
}

/*!
 * \brief   �^�O���X�L�b�v����
 *
 * \param [in]  readHtml    �ǂݍ���html
 * \param [in]  pos         �X�L�b�v�J�n�ʒu
 *
 * \return  �X�L�b�v�I���ʒu�i�X�L�b�v�����Ō�̃^�O��">"�̈ʒu�j
 */
int32_t HtmlGenerator::skipTags(const slog::CoreString* readHtml, int32_t pos)
{
    return skipTags(readHtml, pos, 0);
}

/*!
 * \brief   �ϐ���l�ɒu������
 *
 * \param [in,out]  param   �������s�p�����[�^
 * \param [in]      var     �ϐ���
 *
 * \retval  true    �u���ł����ꍇ�B���������啶���̏ꍇ�͒u���ł��Ȃ��Ă�true��Ԃ��B
 * \retval  false   �u���ł��Ȃ������ꍇ
 */
bool HtmlGenerator::replaceVariable(Param* param, const slog::CoreString* var)
{
    // �f�t�H���g�l������Γo�^����
    int32_t pos = var->indexOf(":");

    if (pos != -1)
    {
//      if (param->isDefaultVariableList())
        {
            String name( var->getBuffer(),  pos);
            String value(var->getBuffer() + pos + 1);
            param->defaultVariableList.push_back(new Variable(name.getBuffer(), value.getBuffer()));
        }

        return true;
    }

    // �ϐ���l�ɒu��
    const VariableList* variableList = param->variableList;

    for (int32_t index = 0; index < 2; index++)
    {
        for (auto variable : *variableList)
        {
            if (var->equals(variable->name))
            {
                // �ϐ��̒l��append����
                mHtml.append(variable->value);
                return true;
            }
        }

        if (param->isDefaultVariableList())
            break;

        variableList = &param->defaultVariableList;
    }

    // �ϐ���l�ɒu���ł��Ȃ������ꍇ
    char c = (*var)[0];

    if ('A' <= c && c <= 'Z')
    {
        // ���������啶���Ȃ琬���Ƃ���
        return true;
    }

    return false;
}

/*!
 * \brief   �u������
 *
 * \param [in,out]  param   �������s�p�����[�^
 * \param [in]      var     �ϐ���
 *
 * \return  �Ȃ�
 */
void HtmlGenerator::replace(Param* param, const slog::CoreString* var)
{
    String sample = "[sample]";

    if (var->equals(sample))
    {
        param->replaceResult = true;

        if (param->isDefaultVariableList() == false)
        {
            // "[sample]"�z���̃^�O��S�ăX�L�b�v����
            param->endPosition = skipTags(&param->buffer, param->endPosition + 1);
        }
    }
    else
    {
        // �ϐ���l�ɒu������
        param->replaceResult = replaceVariable(param, var);
    }
}

/*!
 * \brief   html��ǂݍ���
 *
 * \param [out] readHtml    �t�@�C���̓��e��Ԃ�
 * \param [in]  fileName    �t�@�C����
 *
 * \retval  true    �ǂݍ��݂ɐ��������ꍇ
 * \retval  false   �ǂݍ��݂Ɏ��s�����ꍇ
 */
bool HtmlGenerator::readHtml(slog::CoreString* readHtml, const slog::CoreString* fileName)
{
    try
    {
        File file;
        file.open(*fileName, File::READ);

        int32_t count = (int32_t)file.getSize();

        readHtml->setCapacity(count);
        readHtml->setLength(  count);
        file.read(readHtml,   count);
    }
    catch (Exception)
    {
        return false;
    }

    return true;
}

/*!
 * \brief   html���������s����
 *
 * \param [in]  fileName        �t�@�C����
 * \param [in]  variableList    �ϐ����X�g
 *
 * \retval  true    �����ɐ��������ꍇ
 * \retval  false   �t�@�C���̓ǂݍ��݂Ɏ��s�����ꍇ
 */
bool HtmlGenerator::execute(const slog::CoreString* fileName, const VariableList* variableList)
{
    return execute(fileName, variableList, 0);
}

/*!
 * \brief   html���������s����
 *
 * \param [in]  fileName        �t�@�C����
 * \param [in]  variableList    �ϐ����X�g
 * \param [in]  depth           �C���N���[�h�[�x
 *
 * \retval  true    �����ɐ��������ꍇ
 * \retval  false   �t�@�C���̓ǂݍ��݂Ɏ��s�����ꍇ
 */
bool HtmlGenerator::execute(const slog::CoreString* fileName, const VariableList* variableList, int32_t depth)
{
    Param param(variableList);

    if (readHtml(&param.buffer, fileName) == false)
        return false;

    MimeType mimeType;
    mimeType.analize(fileName);

    if (0 < depth)
    {
        switch (mimeType.type)
        {
        case MimeType::Type::CSS:
            mHtml.append("<style type=\"text/css\">\n");
            break;

        case MimeType::Type::JAVASCRIPT:
            mHtml.append("<script type=\"text/javascript\">\n");
            break;
        }
    }

    int32_t index = 0;

    while (true)
    {
        int32_t pos = param.buffer.indexOf("@", index);

        if (pos == -1)
        {
            // �c���S��append�����[�v�𔲂���
            mHtml.append(param.buffer.getBuffer() + index);
            break;
        }

        // "@"�܂ł�html��append����
        mHtml.append(param.buffer.getBuffer() + index, pos - index);

        if (param.buffer[pos + 1] == '@')
        {
            // "@@"��"@"�Ƃ���append����
            mHtml.append("@");
            param.endPosition = pos + 1;
        }
        else
        {
            int32_t spacePos =  param.buffer.indexOf(" ", pos + 1);
            param.endPosition = param.buffer.indexOf(";", pos + 1);

            if (0 < spacePos && spacePos < param.endPosition)
            {
                if (param.buffer.indexOf("include ", pos + 1) != -1)
                {
                    // ���̃t�@�C�����C���N���[�h����
                    int32_t startPos = pos + 1 + sizeof("include ") - 1;
                    String include(param.buffer.getBuffer() + startPos, param.endPosition - startPos);

                    int32_t dirPos = fileName->lastIndexOf("/");
                    String path;

                    if (dirPos == -1)
                        dirPos = fileName->lastIndexOf("\\");

                    if (dirPos != -1)
                        path.copy(fileName->getBuffer(), dirPos + 1);

                    path.append(include);
                    param.replaceResult = execute(&path, variableList, depth + 1);
                }
            }
            else
            {
                int32_t startPos = pos + 1;
                String var(param.buffer.getBuffer() + startPos, param.endPosition - startPos);

                replace(&param, &var);
            }

            if (param.replaceResult == false)
            {
                // �C���N���[�h�A�܂��͕ϐ��̒u���Ɏ��s�����̂ł��̂܂�append����
                mHtml.append(param.buffer.getBuffer() + pos, (param.endPosition + 1) - pos);
            }
        }

        index = param.endPosition + 1;
    }

    if (0 < depth)
    {
        switch (mimeType.type)
        {
        case MimeType::Type::CSS:
            mHtml.append("</style>\n");
            break;

        case MimeType::Type::JAVASCRIPT:
            mHtml.append("</script>\n");
            break;
        }
    }

    return true;
}

} // namespace slog
