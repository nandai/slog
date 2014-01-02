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
 *  \brief  html生成クラス
 *  \author Copyright 2013 printf.jp
 */
#include "slog/HtmlGenerator.h"

#include "slog/File.h"
#include "slog/MimeType.h"
#include "slog/ByteBuffer.h"
#include "slog/Util.h"

namespace slog
{

/*!
 * 生成実行パラメータクラス
 */
class HtmlGenerator::Param
{
            /*!
             * ファイル名
             */
public:     const CoreString* fileName;

            /*!
             * 読み込みバッファ
             */
            String readBuffer;

            /*!
             * 書き込みバッファ
             */
            CoreString* writeBuffer;

            /*!
             * インクルード深度
             */
            int32_t depth;

            /*!
             * 解析終了位置
             */
            int32_t endPosition;

            /*!
             * 置換結果
             */
            bool replaceResult;

            /*!
             * コンストラクタ
             */
public:     Param(const CoreString* fileName, CoreString* writeBuffer, int32_t depth);
};

/*!
 * \brief   コンストラクタ
 *
 * \param [in]  fileName        ファイル名
 * \param [in]  writeBuffer     書き込みバッファ
 * \param [in]  variableList    変数リスト
 */
HtmlGenerator::Param::Param(const CoreString* fileName, CoreString* writeBuffer, int32_t depth)
{
    this->fileName = fileName;
    this->writeBuffer = writeBuffer;

    this->depth = depth;
    this->endPosition = 0;
    this->replaceResult = false;
}

/*!
 * デフォルトの変数リストか調べる
 */
bool HtmlGenerator::isDefaultVariableList() const
{
    return (mVariableList == &mReadVariableList);
}

/*!
 * \brief   タグをスキップする
 *
 * \param [in]  readHtml    読み込んだhtml
 * \param [in]  pos         スキップ開始位置
 * \param [in]  depth       スキップ開始位置からの解析深度。0で解析を終了する。
 *
 * \return  スキップ終了位置（スキップした最後のタグの">"の位置）
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
            // 終了タグならループを抜ける
            break;
        }

        if (tag[tag.getLength() - 1] != '/')
            endPos = skipTags(readHtml, endPos + 1, depth + 1);

        if (depth == 0)
        {
            // 深度0ならループを抜ける
            break;
        }

        pos = endPos + 1;
    }

    return endPos;
}

/*!
 * \brief   タグをスキップする
 *
 * \param [in]  readHtml    読み込んだhtml
 * \param [in]  pos         スキップ開始位置
 *
 * \return  スキップ終了位置（スキップした最後のタグの">"の位置）
 */
int32_t HtmlGenerator::skipTags(const slog::CoreString* readHtml, int32_t pos)
{
    return skipTags(readHtml, pos, 0);
}

/*!
 * \brief   変数を値に置換する
 *
 * \param [in,out]  param   生成実行パラメータ
 * \param [in]      var     変数名
 *
 * \retval  true    置換できた場合。頭文字が大文字の場合は置換できなくてもtrueを返す。
 * \retval  false   置換できなかった場合
 */
bool HtmlGenerator::replaceVariable(Param* param, const slog::CoreString* var)
{
    // デフォルト値があれば登録する
    int32_t pos = var->indexOf(":");

    if (pos != -1)
    {
//      if (isDefaultVariableList())
        {
            String name( var->getBuffer(),  pos);
            String value;

            Param valueParam(param->fileName, &value, param->depth);
            valueParam.readBuffer.format("%s;", var->getBuffer() + pos + 1);

            expand(&valueParam);

            if (value[value.getLength() - 1] == ';')
                *(value.getBuffer() + value.getLength() - 1) = '\0';

            mReadVariableList.push_back(new Variable(name.getBuffer(), value.getBuffer()));
        }

        return true;
    }

    // 変数を値に置換
    const VariableList* variableList = mVariableList;

    for (int32_t index = 0; index < 2; index++)
    {
        for (VariableList::const_iterator i = variableList->begin(); i != variableList->end(); i++)
        {
            auto variable = *i;

            if (var->equals(variable->name))
            {
                // 変数の値をappendする
                param->writeBuffer->append(variable->value);
                return true;
            }
        }

        if (isDefaultVariableList())
            break;

        variableList = &mReadVariableList;
    }

    // 変数を値に置換できなかった場合
    char c = (*var)[0];

    if ('A' <= c && c <= 'Z')
    {
        // 頭文字が大文字なら成功とする
        return true;
    }

    return false;
}

/*!
 * \brief   置換する
 *
 * \param [in,out]  param   生成実行パラメータ
 * \param [in]      var     変数名
 *
 * \return  なし
 */
void HtmlGenerator::replace(Param* param, const slog::CoreString* var)
{
    String sample = "[sample]";

    if (var->equals(sample))
    {
        param->replaceResult = true;

        if (isDefaultVariableList() == false)
        {
            // "[sample]"配下のタグを全てスキップする
            param->endPosition = skipTags(&param->readBuffer, param->endPosition + 1);
        }
    }
    else
    {
        // 変数を値に置換する
        param->replaceResult = replaceVariable(param, var);
    }
}

/*!
 * \brief   htmlを読み込む
 *
 * \param [out] readHtml    ファイルの内容を返す
 * \param [in]  fileName    ファイル名
 *
 * \retval  true    読み込みに成功した場合
 * \retval  false   読み込みに失敗した場合
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
 * \brief   html生成を実行する
 *
 * \param [in]  fileName        ファイル名
 * \param [in]  variableList    変数リスト
 *
 * \retval  true    生成に成功した場合
 * \retval  false   ファイルの読み込みに失敗した場合
 */
bool HtmlGenerator::execute(const slog::CoreString* fileName, const VariableList* variableList)
{
    mVariableList = (variableList != nullptr
        ? variableList
        : &mReadVariableList);

    return expand(fileName, &mHtml, 0);
}

/*!
 * \brief   html生成を実行する
 *
 * \param [in]  fileName        ファイル名
 * \param [in]  writeBuffer     書き込みバッファ
 * \param [in]  depth           インクルード深度
 *
 * \retval  true    生成に成功した場合
 * \retval  false   ファイルの読み込みに失敗した場合
 */
bool HtmlGenerator::expand(const slog::CoreString* fileName, CoreString* writeBuffer, int32_t depth)
{
    MimeType mimeType;
    mimeType.analize(fileName);

    if (mimeType.type == MimeType::Type::IMAGE)
    {
        try
        {
            File file;
            file.open(*fileName, File::READ);

            int32_t count = (int32_t)file.getSize();

            ByteBuffer buffer( count);
            file.read(&buffer, count);

            String readBuffer;
            Util::encodeBase64(&readBuffer, buffer.getBuffer(), count);

            writeBuffer->append("data:");
            writeBuffer->append(mimeType.text);
            writeBuffer->append(";base64,");
            writeBuffer->append(readBuffer);
            return true;
        }
        catch (Exception)
        {
            return false;
        }
    }

    Param param(fileName, writeBuffer, depth);

    if (readHtml(&param.readBuffer, fileName) == false)
        return false;

    if (0 < depth && mimeType.tag.getLength())
    {
        String tag;
        tag.format("<%s type=\"%s\">\n", mimeType.tag.getBuffer(), mimeType.text.getBuffer());
        writeBuffer->append(tag);
    }

    if (mimeType.type == MimeType::Type::CSS ||
        mimeType.type == MimeType::Type::JAVASCRIPT)
    {
        writeBuffer->append(param.readBuffer);
    }
    else
    {
        expand(&param);
    }

    if (0 < depth && mimeType.tag.getLength())
    {
        String tag;
        tag.format("</%s>\n", mimeType.tag.getBuffer());
        writeBuffer->append(tag);
    }

    return true;
}

/*!
 * html生成を実行する
 */
void HtmlGenerator::expand(Param* param)
{
    int32_t index = 0;

    while (true)
    {
        int32_t pos = param->readBuffer.indexOf("@", index);

        if (pos == -1)
        {
            // 残りを全てappendしループを抜ける
            param->writeBuffer->append(param->readBuffer.getBuffer() + index);
            break;
        }

        // "@"までのhtmlをappendする
        param->writeBuffer->append(param->readBuffer.getBuffer() + index, pos - index);

        if (param->readBuffer.at(pos + 1) == '@')
        {
            // "@@"を"@"としてappendする
            param->writeBuffer->append("@");
            param->endPosition = pos + 1;
        }
        else
        {
            param->endPosition = param->readBuffer.indexOf(";", pos + 1);

            if (param->readBuffer.indexOf("include ", pos + 1) == pos + 1)
            {
                // 他のファイルをインクルードする
                int32_t startPos = pos + 1 + sizeof("include ") - 1;
                String include(param->readBuffer.getBuffer() + startPos, param->endPosition - startPos);

                int32_t dirPos = param->fileName->lastIndexOf("/");
                String path;

                if (dirPos == -1)
                    dirPos = param->fileName->lastIndexOf("\\");

                if (dirPos != -1)
                    path.copy(param->fileName->getBuffer(), dirPos + 1);

                path.append(include);
                param->replaceResult = expand(&path, param->writeBuffer, param->depth + 1);
            }
            else
            {
                int32_t startPos = pos + 1;
                String var(param->readBuffer.getBuffer() + startPos, param->endPosition - startPos);

                replace(param, &var);
            }

            if (param->replaceResult == false)
            {
                // インクルード、または変数の置換に失敗したのでそのままappendする
                param->writeBuffer->append(param->readBuffer.getBuffer() + pos, (param->endPosition + 1) - pos);
            }
        }

        index = param->endPosition + 1;
    }
}

} // namespace slog
