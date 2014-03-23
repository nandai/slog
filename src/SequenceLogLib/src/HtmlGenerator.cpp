/*
 * Copyright (C) 2013-2014 printf.jp
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
 *  \author Copyright 2013-2014 printf.jp
 */
#include "slog/HtmlGenerator.h"

#include "slog/File.h"
#include "slog/MimeType.h"
#include "slog/ByteBuffer.h"
#include "slog/Util.h"
#include "slog/DateTime.h"

#undef __SLOG__
#include "slog/SequenceLog.h"

#if defined(__unix__)
    #include <string.h>
#endif

namespace slog
{

const char* HtmlGenerator::CLS_NAME = "HtmlGenerator";

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
 * \brief   コンストラクタ
 */
HtmlGenerator::HtmlGenerator(const CoreString* rootDir)
{
    mRootDir.copy(rootDir);

    // appendの度にバッファ拡張処理させないように、ある程度の領域を確保しておく
    mHtml.setCapacity(1024 * 1024 * 2);

    mLastWriteTime = new DateTime;
}

/*!
 * \brief   デストラクタ
 */
HtmlGenerator::~HtmlGenerator()
{
    delete mLastWriteTime;
}

/*!
 * \brief   デフォルトの変数リストか調べる
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
int32_t HtmlGenerator::skipTags(const CoreString* readHtml, int32_t pos, int32_t depth)
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
int32_t HtmlGenerator::skipTags(const CoreString* readHtml, int32_t pos)
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
bool HtmlGenerator::replaceVariable(Param* param, const CoreString* var)
{
    // 変数名チェック
    int32_t pos = var->indexOf(":");
    int32_t varEndPos = (0 < pos ? pos : var->getLength());

    for (int32_t index = 0; index < varEndPos; index++)
    {
        // 変数名は英数、及び'_'のみ有効
        char c = var->at(index);
        bool pass =
            ('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z') ||
            ('0' <= c && c <= '9') ||
            ('_' == c);

        if (pass == false)
        {
            param->endPosition -= (var->getLength() - index);
            return false;
        }
    }

    // デフォルト値があれば登録する
    if (0 < pos)
    {
//      if (isDefaultVariableList())
        {
            String name(var->getBuffer(), pos);
            String value;
            value.setCapacity(1024 * 1024);

            Param valueParam(param->fileName, &value, param->depth);
            valueParam.readBuffer.format("%s;", var->getBuffer() + pos + 1);

            expand(&valueParam);

            if (value[value.getLength() - 1] == ';')
                *(value.getBuffer() + value.getLength() - 1) = '\0';

            mReadVariableList.add(&name, &value);
        }

        return true;
    }

    // 変数を値に置換
    const VariableList* variableList = mVariableList;

    for (int32_t index = 0; index < 2; index++)
    {
        const CoreString* value = variableList->find(var);

        if (value)
        {
            // 変数の値をappendする
            param->writeBuffer->append(value);
            return true;
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
void HtmlGenerator::replace(Param* param, const CoreString* var)
{
    if (var->equals("[sample]"))
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
 * \brief   検索開始文字列の位置までの文字列をappendする。
 *          検索開始、及び検索終了文字列が見つからなければ、全てappendする。
 *
 * \param [in,out]  param   生成実行パラメータ
 * \param [in]      index   検索開始位置
 * \param [in]      from    検索開始文字列
 * \param [in]      to      検索終了文字列
 * \param [out]     pos     検索開始文字列の位置
 * \param [out]     endPos  検索終了文字列の位置
 *
 * \retval  true    検索開始、及び検索終了文字列が見つかった場合
 * \retval  false   検索開始、及び検索終了文字列が見つからなかった場合
 */
bool HtmlGenerator::append(Param* param, int32_t index, const char* from, const char* to, int32_t* pos, int32_t* endPos)
{
    *pos = param->readBuffer.indexOf(from, index);
    *endPos = -1;

    if (*pos != -1)
        *endPos = param->readBuffer.indexOf(to, *pos + 1);

    if (*endPos != -1)
    {
        // fromまでappendする
        param->writeBuffer->append(
            param->readBuffer.getBuffer() + index,
            *pos - index);
    }
    else
    {
        // 残り全てappendする
        param->writeBuffer->append(
            param->readBuffer.getBuffer() + index,
            param->readBuffer.getLength() - index);
    }

    return (*endPos != -1);
}

/*!
 * \brief   インクルードパスを取得する
 *
 * \param [in]  param       生成実行パラメータ
 * \param [out] path        結果を返す
 * \param [in]  fileName    ファイル名
 *
 * \return  なし
 */
void HtmlGenerator::getIncludePath(Param* param, CoreString* path, const CoreString* fileName)
{
    if (fileName->at(0) == '/')
    {
        path->copy(&mRootDir);
    }
    else
    {
        int32_t dirPos = param->fileName->lastIndexOf("/");

        if (dirPos == -1)
            dirPos =     param->fileName->lastIndexOf("\\");

        if (dirPos != -1)
            path->copy(  param->fileName->getBuffer(), dirPos + 1);
    }

    path->append(fileName);
}

/*!
 * \brief   htmlを読み込む
 *
 * \param [out] readHtml    ファイルの内容を返す
 * \param [in]  position    
 * \param [in]  fileName    ファイル名
 *
 * \retval  true    読み込みに成功した場合
 * \retval  false   読み込みに失敗した場合
 */
bool HtmlGenerator::readHtml(CoreString* readHtml, int32_t position, const CoreString* fileName)
{
    try
    {
        File file;
        file.open(fileName, File::READ);

        uint64_t value = file.getLastWriteTime()->getValue();

        if (mLastWriteTime->getValue() < value)
            mLastWriteTime->setValue(    value);

        int32_t count = (int32_t)file.getSize();

        if (readHtml->getCapacity() < position + count)
            readHtml->setCapacity(    position + count);

        readHtml->setLength(position + count);
        file.read(readHtml, position,  count);
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
bool HtmlGenerator::execute(const CoreString* fileName, const VariableList* variableList)
{
    SLOG(CLS_NAME, "execute");

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
bool HtmlGenerator::expand(const CoreString* fileName, CoreString* writeBuffer, int32_t depth)
{
    MimeType mimeType;
    mimeType.analize(fileName);

    if (mimeType.type == MimeType::Type::IMAGE)
    {
        try
        {
            File file;
            file.open(fileName, File::READ);

            uint64_t value = file.getLastWriteTime()->getValue();

            if (mLastWriteTime->getValue() < value)
                mLastWriteTime->setValue(    value);

            int32_t count = (int32_t)file.getSize();

            ByteBuffer buffer( count);
            file.read(&buffer, count);

            String readBuffer;
            Util::encodeBase64(&readBuffer, buffer.getBuffer(), count);

            writeBuffer->append("data:", 5);
            writeBuffer->append(&mimeType.text);
            writeBuffer->append(";base64,", 8);
            writeBuffer->append(&readBuffer);
            return true;
        }
        catch (Exception)
        {
            return false;
        }
    }

    if (0 < depth && mimeType.tag.getLength())
    {
        String tag;
        tag.format("<%s type=\"%s\">\n", mimeType.tag.getBuffer(), mimeType.text.getBuffer());
        writeBuffer->append(&tag);
    }

    Param param(fileName, writeBuffer, depth);
    bool result;

    switch (mimeType.type)
    {
    case MimeType::Type::JAVASCRIPT:
        result = readHtml(writeBuffer, writeBuffer->getLength(), fileName);
        break;

    case MimeType::Type::CSS:
        result = readHtml(&param.readBuffer, 0, fileName);

        if (result)
            expandCSS(&param);

        break;

    default:
        result = readHtml(&param.readBuffer, 0, fileName);

        if (result)
            expand(&param);

        break;
    }

    if (0 < depth && mimeType.tag.getLength())
    {
        String tag;
        tag.format("</%s>\n", mimeType.tag.getBuffer());
        writeBuffer->append(&tag);
    }

    return result;
}

/*!
 * \brief   html生成を実行する
 *
 * \param [in,out]  param   生成実行パラメータ
 *
 * \return  なし
 */
void HtmlGenerator::expand(Param* param)
{
    int32_t index = 0;

    while (true)
    {
        int32_t pos;
        int32_t endPos;

        if (append(param, index, "@", ";", &pos, &endPos) == false)
            break;

        if (param->readBuffer.at(pos + 1) == '@')
        {
            // "@@"を"@"としてappendする
            param->writeBuffer->append("@");
            param->endPosition = pos + 1;
        }
        else
        {
            param->endPosition = endPos;

//          if (param->readBuffer.indexOf("include ", pos + 1) == pos + 1)
            if (strncmp(param->readBuffer.getBuffer() + pos + 1, "include ", 8) == 0)
            {
                // 他のファイルをインクルードする
                int32_t startPos = pos + 1 + sizeof("include ") - 1;
                String include(param->readBuffer.getBuffer() + startPos, param->endPosition - startPos);

                String path;
                getIncludePath(param, &path, &include);

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

/*!
 * \brief   html生成を実行する
 *
 * \param [in,out]  param   生成実行パラメータ
 *
 * \return  なし
 */
void HtmlGenerator::expandCSS(Param* param)
{
    int32_t index = 0;

    while (true)
    {
        int32_t pos;
        int32_t endPos;

        if (append(param, index, "url(", ")", &pos, &endPos) == false)
            break;

        {
            param->endPosition = endPos;

            {
                // 他のファイルをインクルードする
                int32_t startPos = pos + sizeof("url(") - 1;
                char c = param->readBuffer[startPos];

                if (c == '\'' || c == '"')
                {
                    startPos++;
                    endPos--;
                }

                String include(param->readBuffer.getBuffer() + startPos, endPos - startPos);

                String path;
                getIncludePath(param, &path, &include);

                param->writeBuffer->append("url(");
                param->replaceResult = expand(&path, param->writeBuffer, param->depth + 1);
                param->writeBuffer->append(")");
            }

            if (param->replaceResult == false)
            {
                // インクルードに失敗したのでそのままappendする
                param->writeBuffer->append(param->readBuffer.getBuffer() + pos, (param->endPosition + 1) - pos);
            }
        }

        index = param->endPosition + 1;
    }
}

/*!
 * 最終書込日時取得
 */
const DateTime* HtmlGenerator::getLastWriteTime() const
{
    return mLastWriteTime;
}

} // namespace slog
