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

namespace slog
{

/*!
 * 生成実行パラメータクラス
 */
class HtmlGenerator::Param
{
            /*!
             * 読み込んだファイルの内容
             */
public:     slog::String buffer;

            /*!
             * デフォルト変数リスト
             */
            VariableList defaultVariableList;

            /*!
             * 変数リスト
             */
            const VariableList* variableList;

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
public:     Param(const VariableList* variableList);

            /*!
             * デフォルトの変数リストか調べる
             */
            bool isDefaultVariableList() const;
};

/*!
 * \brief   コンストラクタ
 *
 * \param [in]  variableList    変数リスト
 */
HtmlGenerator::Param::Param(const VariableList* variableList)
{
    this->variableList = (variableList != nullptr
        ? variableList
        : &defaultVariableList);

    this->endPosition = 0;
    this->replaceResult = false;
}

/*!
 * デフォルトの変数リストか調べる
 */
bool HtmlGenerator::Param::isDefaultVariableList() const
{
    return (variableList == &defaultVariableList);
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
//      if (param->isDefaultVariableList())
        {
            String name( var->getBuffer(),  pos);
            String value(var->getBuffer() + pos + 1);
            param->defaultVariableList.push_back(new Variable(name.getBuffer(), value.getBuffer()));
        }

        return true;
    }

    // 変数を値に置換
    const VariableList* variableList = param->variableList;

    for (int32_t index = 0; index < 2; index++)
    {
        for (VariableList::const_iterator i = variableList->begin(); i != variableList->end(); i++)
        {
            auto variable = *i;

            if (var->equals(variable->name))
            {
                // 変数の値をappendする
                mHtml.append(variable->value);
                return true;
            }
        }

        if (param->isDefaultVariableList())
            break;

        variableList = &param->defaultVariableList;
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

        if (param->isDefaultVariableList() == false)
        {
            // "[sample]"配下のタグを全てスキップする
            param->endPosition = skipTags(&param->buffer, param->endPosition + 1);
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
    return execute(fileName, variableList, 0);
}

/*!
 * \brief   html生成を実行する
 *
 * \param [in]  fileName        ファイル名
 * \param [in]  variableList    変数リスト
 * \param [in]  depth           インクルード深度
 *
 * \retval  true    生成に成功した場合
 * \retval  false   ファイルの読み込みに失敗した場合
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
            // 残りを全てappendしループを抜ける
            mHtml.append(param.buffer.getBuffer() + index);
            break;
        }

        // "@"までのhtmlをappendする
        mHtml.append(param.buffer.getBuffer() + index, pos - index);

        if (param.buffer[pos + 1] == '@')
        {
            // "@@"を"@"としてappendする
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
                    // 他のファイルをインクルードする
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
                // インクルード、または変数の置換に失敗したのでそのままappendする
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
