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
 *  \file   WebServerResponseThread.h
 *  \brief  WEB�T�[�o�[�����X���b�h�N���X
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once
#include "slog/Thread.h"

namespace slog
{
class HttpRequest;
class CoreString;
class String;
class Socket;
class ByteBuffer;

/*!
 *  \brief  WEB�T�[�o�[�����X���b�h�N���X
 */
class SLOG_API WebServerResponseThread : public Thread
{
protected:  HttpRequest*    mHttpRequest;

            // �R���X�g���N�^ / �f�X�g���N�^
public:     WebServerResponseThread(HttpRequest* httpRequest);
            virtual ~WebServerResponseThread();

            // �h���C���擾
private:    virtual const char* getDomain() const {return NULL;}

            // ���[�g�f�B���N�g���擾
            virtual const char* getRootDir() const {return NULL;}

            // ���M
protected:  void send(const CoreString& content) const;

            // HTTP�w�b�_�[���M�i���ؒf�j
            void sendHttpHeader(int32_t contentLen) const;

            // �������e���M���ؒf
            void sendContent(const CoreString& content) const;

            // ���s
private:    virtual void run();

            // �R���e���c�擾
protected:  bool getContents(String* content, const char* url);

            // WebSocket�ɃA�b�v�O���[�h
protected:  bool upgradeWebSocket();

            // WebSocket�w�b�_�[���M
            void sendWebSocketHeader(uint64_t payloadLen, bool isText = true, bool toClient = true) const;
};

} // namespace slog
