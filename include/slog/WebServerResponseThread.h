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
#include "slog/HtmlGenerator.h"

namespace slog
{
class HttpRequest;
class CoreString;
class String;
class Socket;
class Buffer;

/*!
 *  \brief  WEB�T�[�o�[�����X���b�h�N���X
 */
class SLOG_API WebServerResponseThread : public Thread
{
protected:  HttpRequest*        mHttpRequest;
            slog::VariableList  mVariables;

            // �R���X�g���N�^ / �f�X�g���N�^
public:     WebServerResponseThread(HttpRequest* httpRequest);
            virtual ~WebServerResponseThread();

            // �h���C���擾
private:    virtual const char* getDomain() const {return nullptr;}

            // ���[�g�f�B���N�g���擾
            virtual const char* getRootDir() const {return nullptr;}

            // �ϐ�������
protected:  virtual void initVariables() {}

            // �t�@�C���p�X�擾
            void getFilePath(slog::CoreString* path, const slog::CoreString* url) const;

            // ���M
            void send(const Buffer* content) const;

            // �o�C�i�����M
            void sendBinary(const slog::CoreString* path) const;

            // HTTP�w�b�_�[���M�i���ؒf�j
            void sendHttpHeader(int32_t contentLen) const;

            // �������e���M���ؒf
            void sendContent(const Buffer* content) const;

            // ���s
            virtual void run() override;

            // WebSocket�ɃA�b�v�O���[�h
protected:  bool upgradeWebSocket();

            // WebSocket�w�b�_�[���M
            void sendWebSocketHeader(uint64_t payloadLen, bool isText = true, bool toClient = true) const;
};

} // namespace slog
