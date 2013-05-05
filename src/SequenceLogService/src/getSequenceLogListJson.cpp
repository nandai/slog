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
 *  \file   getSequenceLogListJson.cpp
 *  \brief  �V�[�P���X���O���X�g�iJSON�j�擾
 *  \author Copyright 2013 printf.jp
 */
#include "getSequenceLogListJson.h"
#include "SequenceLogServiceMain.h"

#include "slog/Json.h"
#include "slog/FileInfo.h"
#include "slog/PointerString.h"
#include "slog/Tokenizer.h"
#include "slog/DateTime.h"
#include "slog/DateTimeFormat.h"
#include "slog/Mutex.h"

namespace slog
{

/*!
 *  \brief  �V�[�P���X���O���X�gJSON�쐬
 */
static void createSequenceLogListJson(Json* json, FileInfo* info)
{
    DateTime dateTime;

    // ���O�t�@�C����
    const CoreString& strCanonicalPath = info->getCanonicalPath();

    // �J�n����
    String strCreationTime = "Unknown";

#if 0 // linux�ł͍쐬�����擾�ł��Ȃ��̂ŁA�t�@�C�����Ɋ܂߂��������g���悤�ɕύX
    dateTime = info->getCreationTime();

    if (dateTime.getValue())
    {
        dateTime.toLocal();
        DateTimeFormat::toString(&strCreationTime, dateTime, DateTimeFormat::DATE_TIME);
    }
#else
    PointerString fileName = strrchr(strCanonicalPath.getBuffer(), PATH_DELIMITER);
    Tokenizer tokenizer('-');
    tokenizer.exec(fileName);

    if (4 <= tokenizer.getCount())
    {
        const CoreString& strDate = tokenizer.getValue(2);
        const CoreString& strTime = tokenizer.getValue(3);

        if (strDate.getLength() == 8 && strTime.getLength() == 6)
        {
            const char* pDate = strDate.getBuffer();
            const char* pTime = strTime.getBuffer();

            strCreationTime.format("%.4s/%.2s/%.2s %.2s:%.2s:%.2s",
                pDate + 0,
                pDate + 4,
                pDate + 6,
                pTime + 0,
                pTime + 2,
                pTime + 4);
        }
    }
#endif

    // �I������
    String strLastWriteTime;
    dateTime = info->getLastWriteTime();

    if (dateTime.getValue())
    {
        dateTime.toLocal();
        DateTimeFormat::toString(&strLastWriteTime, dateTime, DateTimeFormat::DATE_TIME);
    }

    // ���O�t�@�C����
//  const CoreString& strCanonicalPath = info->getCanonicalPath();

    // �T�C�Y
    String strSize;
    const CoreString& message = info->getMessage();

    if (info->isUsing() == false)
    {
        double size = (double)info->getSize();

        if (size < 1024)
        {
            strSize.format("%s %d byte(s)", message.getBuffer(), (int64_t)size);
        }

        else
        if (size < 1024 * 1024)
        {
            strSize.format("%s %.1f KB", message.getBuffer(), size / 1024);
        }

        else
        {
            strSize.format("%s %.1f MB", message.getBuffer(), size / (1024 * 1024));
        }
    }

    // JSON�쐬
    json->add("creationTime",  strCreationTime);
    json->add("lastWriteTime", strLastWriteTime);
    json->add("canonicalPath", strCanonicalPath);
    json->add("size",          strSize);
}

/*!
 *  \brief  �V�[�P���X���O���X�g�iJSON�j�擾
 */
void getSequenceLogListJson(String* content)
{
    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    ScopedLock lock(serviceMain->getMutex());

    FileInfoArray* sum = serviceMain->getFileInfoArray();
    Json* json = Json::getNewObject();

    for (FileInfoArray::iterator i = sum->begin(); i != sum->end(); i++)
    {
        Json* jsonSequenceLogInfo = Json::getNewObject();

        createSequenceLogListJson(jsonSequenceLogInfo, *i);
        json->add(jsonSequenceLogInfo);
    }

    json->serialize(content);
    delete json;
}

} // namespace slog
