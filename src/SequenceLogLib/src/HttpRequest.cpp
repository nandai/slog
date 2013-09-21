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
                analizePostParams(&params);
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

        if (p2 == NULL)
            return -1;

        p1++;
        mUrl.copy(p1, (int32_t)(p2 - p1));
        mMethod = method;
        return 0;
    }

    return 1;
}

/*!
 *  \brief  POST�p�����[�^���
 */
void HttpRequest::analizePostParams(ByteBuffer* params)
{
    const char* p1 = params->getBuffer();
    bool end = false;

    while (end == false)
    {
        // ��΂̃p�����[�^�����o��
        const char* p2 = strchr(p1, '&');

        if (p2 == NULL)
        {
            p2 = p1 + params->getLength();
            end = true;
        }

        // �p�����[�^���ƒl�ɕ�����
        const char* p3 = strchr(p1, '=');

        if (p3 == NULL)
            break;

        // �p�����[�^����L�[���擾
        String key(p1, (int32_t)(p3 - p1));

        // �p�����[�^����l���擾
        p3++;
        p1 = p3;
        char* p4 = (char*)p3;

        while (p1 < p2)
        {
            char c = *p1;

            switch (c)
            {
            case '%':
            {
                p1 = hexToValue(p1 + 1, &c);
                break;
            }

            case '+':
                c =  ' ';
//              break;

            default:
                p1++;
                break;
            }

            *p4 = c;
             p4++;
        }

        String value;
        value.copy(p3, (int32_t)(p4 - p3));

        p1 = p2 + 1;

        // �p�����[�^���X�g�ɒǉ�
        mPostParams.insert(pair<String, String>(key, value));
    }
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
}

/*!
 *  \brief  POST�p�����[�^�擾
 */
void HttpRequest::getParam(const char* name, CoreString* param)
{
    param->copy(mPostParams[name]);
}

/*!
 *  \brief  Sec-WebSocket-Key�擾
 */
const CoreString& HttpRequest::getWebSocketKey() const
{
    return mWebSocketKey;
}

} // namespace slog
