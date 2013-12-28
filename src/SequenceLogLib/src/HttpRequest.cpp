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
 *  \file   HttpRequest.cpp
 *  \brief  http���N�G�X�g�N���X
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/HttpRequest.h"
#include "slog/Socket.h"
#include "slog/ByteBuffer.h"

#include <ctype.h>

#if defined(__linux__)
    #include <string.h>
    #include <stdlib.h>
#endif

using namespace std;

namespace slog
{

/*!
 *  \brief  16�i��������𐔒l�ɕϊ�
 */
template <class T>
inline const char* _hexToValue(const char* hex, T* value)
{
    int32_t i;
    int32_t size = sizeof(*value) * 2;
    *value = 0;

    for (i = 0; i < size; i++)
    {
        char c = toupper(hex[i]);

        if ('0' <= c && c <= '9')
        {
            c = c - '0';
        }
        else if ('A' <= c && c <= 'F')
        {
            c = c - 'A' + 0x0A;
        }
        else
        {
            break;
        }

        *value = (*value << 4) | c;
    }

    return (hex + i);
}

/*!
 *  \brief  16�i���������char�^�̐��l�ɕϊ�
 */
static const char* hexToValue(const char* hex, char* value)
{
    return _hexToValue(hex, value);
}

/*!
 *  \brief  �R���X�g���N�^
 */
HttpRequest::HttpRequest(Socket* socket, uint16_t port)
{
    mSocket = socket;
    mPort = port;
    mMethod = UNKNOWN;
    mAjax = false;
}

/*!
 *  \brief  �f�X�g���N�^
 */
HttpRequest::~HttpRequest()
{
    delete mSocket;
}

/*!
 *  \brief  �v�����
 */
bool HttpRequest::analizeRequest()
{
    int32_t size = 1;
    ByteBuffer buffer(size);

    char request[1024 + 1];
    int32_t i = 0;
    int32_t contentLen = 0;

    while (true)
    {
        // ��M
        buffer.setLength(0);
        mSocket->recv(&buffer, size);

        // ���s�܂Ń��N�G�X�g�o�b�t�@�ɒ��߂�
        char c = buffer.get();
//      noticeLog("%d: %c(%02X)", i, c, (uint8_t)c);

        if (c != '\r')
        {
            if (sizeof(request) <= i)
                return false;

            request[i] = c;
            i++;

            continue;
        }

        request[i] = '\0';
//      noticeLog("%s", request);

        // '\n'�̂�
        buffer.setLength(0);
        mSocket->recv(&buffer, size);

//      c = buffer.get();
//      noticeLog("%d: %c(%02X)", i + 1, c, (uint8_t)c);

        if (i == 0)
        {
            // ��s�������烋�[�v�𔲂���
            if (mMethod == POST && 0 < contentLen)
            {
                ByteBuffer params(contentLen);

                mSocket->recv(&params, contentLen);
                analizeParams(params.getBuffer(), params.getLength());
            }
//          noticeLog("analizeRequest ended");

            break;
        }
        else
        {
            if (mMethod == UNKNOWN)
            {
                // URL�擾
                if (analizeUrl(request, i, GET)  == -1)
                    return false;

                if (analizeUrl(request, i, POST) == -1)
                    return false;

                if (mMethod == UNKNOWN)
                    return false;
            }
            else
            {
                // Content-Length
                const char* compare = "Content-Length: ";
                int32_t compareLen = (int32_t)strlen(compare);

                if (strncmp(request, compare, compareLen) == 0)
                {
                    contentLen = atoi(request + compareLen);
                }

                // Accept
                compare = "Accept: ";
                compareLen = (int32_t)strlen(compare);

                if (strncmp(request, compare, compareLen) == 0)
                {
                    char* p = strchr(request, ',');

                    if (p)
                        p[0] = '\0';

                    mMimeType.setText(request + compareLen);
                }

                // X-Requested-With
                compare = "X-Requested-With: XMLHttpRequest";
                compareLen = (int32_t)strlen(compare);

                if (strncmp(request, compare, compareLen) == 0)
                {
                    mAjax = true;
                }

                // Sec-WebSocket-Key
                compare = "Sec-WebSocket-Key: ";
                compareLen = (int32_t)strlen(compare);

                if (strncmp(request, compare, compareLen) == 0)
                {
                    mWebSocketKey.copy(request + compareLen);
                }
            }
        }

        i = 0;
    }

    if (mAjax == false)
        mMimeType.analize(&mUrl);

    return true;
}

/*!
 *  \brief  URL���
 */
int32_t HttpRequest::analizeUrl(const char* request, int32_t len, METHOD method)
{
    const char* compare;

    switch (method)
    {
    case GET:
        compare = "GET ";
        break;

    case POST:
        compare = "POST ";
        break;

    default:
        return -1;
    }

    int32_t compareLen = (int32_t)strlen(compare);

    if (compareLen <= len && strncmp(request, compare, compareLen) == 0)
    {
        const char* p1 = request + compareLen;
        const char* p2 = strchr(p1, ' ');

        if (p2 == nullptr)
            return -1;

        if (method == GET)
        {
            const char* p3 = strchr(p1, '?');

            if (p3)
            {
                analizeParams(p3 + 1, p2 - (p3 + 1));
                p2 = p3;
            }
        }

        p1++;   // '/'���X�L�b�v

        decode(&mUrl, (char*)p1, p2);
        mMethod = method;
        return 0;
    }

    return 1;
}

/*!
 *  \brief  �p�����[�^���
 */
void HttpRequest::analizeParams(const char* buffer, int32_t len)
{
    const char* p1 = buffer;
    bool end = false;

    while (end == false)
    {
        // ��΂̃p�����[�^�����o��
        const char* p2 = strchr(p1, '&');

        if (p2 == nullptr)
        {
            p2 = buffer + len;
            end = true;
        }

        // �p�����[�^���ƒl�ɕ�����
        const char* p3 = strchr(p1, '=');

        if (p3 == nullptr)
            break;

        // �p�����[�^����L�[���擾
        String key(p1, (int32_t)(p3 - p1));

        // �p�����[�^����l���擾
        String  value;
        decode(&value, (char*)p3 + 1, p2);

        // �p�����[�^���X�g�ɒǉ�
        mParams.insert(pair<String, String>(key, value));

        p1 = p2 + 1;
    }
}

/*!
 * �p�[�Z���g�f�R�[�h
 */
void HttpRequest::decode(slog::CoreString* str, char* start, const char* end)
{
    const char* cursor = start;
    char* decodeCursor = start;

    while (cursor < end)
    {
        char c = *cursor;

        switch (c)
        {
        case '%':
        {
            cursor = hexToValue(cursor + 1, &c);
            break;
        }

        case '+':
            c =  ' ';
//          break;

        default:
            cursor++;
            break;
        }

        *decodeCursor = c;
         decodeCursor++;
    }

    str->copy(start, (int32_t)(decodeCursor - start));
}

/*!
 *  \brief  �\�P�b�g�擾
 */
Socket* HttpRequest::getSocket() const
{
    return mSocket;
}

/*!
 *  \brief  �|�[�g�擾
 */
uint16_t HttpRequest::getPort() const
{
    return mPort;
}

/*!
 *  \brief  HTTP���\�b�h�擾
 */
HttpRequest::METHOD HttpRequest::getMethod() const
{
    return mMethod;
}

/*!
 *  \brief  URL�擾
 */
const CoreString& HttpRequest::getUrl() const
{
    return mUrl;
}

/*!
 *  \brief  URL�ݒ�
 */
void HttpRequest::setUrl(const char* url)
{
    mUrl.copy(url);

    if (mAjax == false)
        mMimeType.analize(&mUrl);
}

/*!
 *  \brief  mime-type�擾
 */
const MimeType* HttpRequest::getMimeType()
{
    return &mMimeType;
}

/*!
 *  \brief  POST�p�����[�^�擾
 */
const CoreString* HttpRequest::getParam(const char* name, CoreString* param)
{
    param->copy(mParams[name]);
    return param;
}

/*!
 *  \brief  Ajax���ǂ������ׂ�
 */
bool HttpRequest::isAjax() const
{
    return mAjax;
}

/*!
 *  \brief  Sec-WebSocket-Key�擾
 */
const CoreString* HttpRequest::getWebSocketKey() const
{
    return &mWebSocketKey;
}

} // namespace slog
