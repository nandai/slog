﻿/*
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
 * \file    WebSocket.cpp
 * \brief   Web Socket クラス
 * \author  Copyright 2013-2014 printf.jp
 */
#pragma execution_character_set("utf-8")

#include "slog/slog.h"
#include "slog/WebSocket.h"
#include "slog/String.h"
#include "slog/ByteBuffer.h"
#include "slog/Thread.h"
#include "slog/Mutex.h"

namespace slog
{

/*!
 * \brief   Web Socket 受信クラス
 */
class WebSocketReceiver : public Thread
{
            WebSocket* mWebSocket;

            /*!
             * リスナーリスト
             */
            std::list<WebSocketListener*> mListeners;

            /*!
             * コンストラクタ／デストラクタ
             */
public:     WebSocketReceiver(WebSocket* webSocket)
            {
                mWebSocket = webSocket;
            }

            /*!
             * リスナー追加
             */
            void addWebSocketListener(WebSocketListener* listener) {mListeners.push_back(listener);}

            /*!
             * スレッド実行
             */
private:    virtual void run() override;

            /*!
             * リスナーに通知
             */
public:     void notifyOpen();
            void notifyError(const char* message);
private:    void notifyMessage(const ByteBuffer& buffer);
public:     void notifyClose();
};

/*!
 * \brief   スレッド実行
 */
void WebSocketReceiver::run()
{
    Mutex* mutex = mWebSocket->getMutex();

    try
    {
        while (true)
        {
            Thread::sleep(1);

            // ロックしてからチェック
            ScopedLock lock(mutex);

            bool isReceive = mWebSocket->isReceiveData(0);

            if (isInterrupted())
                break;

            if (isReceive == false)
                continue;

//          if (mWebSocket->isReceiveData(0) == false)
//              continue;

            // データ受信
            ByteBuffer* buffer = mWebSocket->recv(nullptr);

            if (buffer == nullptr)
                continue;

            // リスナーにメッセージ通知
            notifyMessage(*buffer);

            // データバッファ削除
            delete buffer;
        }
    }
    catch (Exception& e)
    {
        const char* message = e.getMessage();

        noticeLog("WebSocket: %s", message);
        notifyError(message);
    }
}

/*!
 * \brief   リスナーにオープン通知
 */
void WebSocketReceiver::notifyOpen()
{
    for (auto i = mListeners.begin(); i != mListeners.end(); i++)
        (*i)->onOpen();
}

/*!
 * \brief   リスナーにエラー通知
 */
void WebSocketReceiver::notifyError(const char* message)
{
    for (auto i = mListeners.begin(); i != mListeners.end(); i++)
        (*i)->onError(message);
}

/*!
 * \brief   リスナーにメッセージ通知
 */
void WebSocketReceiver::notifyMessage(const ByteBuffer& buffer)
{
    for (auto i = mListeners.begin(); i != mListeners.end(); i++)
        (*i)->onMessage(buffer);
}

/*!
 * \brief   リスナーにクローズ通知
 */
void WebSocketReceiver::notifyClose()
{
    for (auto i = mListeners.begin(); i != mListeners.end(); i++)
        (*i)->onClose();
}

/*!
 * \brief   コンストラクタ
 */
WebSocket::WebSocket(bool isServer)
{
    mIsServer = isServer;
    mPayloadLen = 0;
    mIsText = false;
    mMutex = new Mutex();
    mReceiver = new WebSocketReceiver(this);
}

/*!
 * \brief   デストラクタ
 */
WebSocket::~WebSocket()
{
    delete mReceiver;
    mReceiver = nullptr;

    delete mMutex;
    mMutex = nullptr;
}

/*!
 * \brief   初期化
 */
void WebSocket::init()
{
    mReceiver->start();
}

/*!
 * \brief   クローズ
 */
int WebSocket::close()
{
    ScopedLock lock(mMutex);

    if (mReceiver->isAlive())
    {
        mReceiver->interrupt();
        mReceiver->join();
        mReceiver->notifyClose();
    }

    return Socket::close();
}

/*!
 * \brief   リスナー追加
 */
void WebSocket::addWebSocketListener(WebSocketListener* listener)
{
    mReceiver->addWebSocketListener(listener);
}

/*!
 * \brief   Web Socket ヘッダー送信
 */
void WebSocket::sendHeader(uint64_t payloadLen, bool isText) throw(Exception)
{
    if (mPayloadLen != 0)
    {
        Exception e;

        e.setMessage("送信予定のデータ長に満たないうちに次のデータを送信しようとしました。送信残 %s byte(s)", mPayloadLen);
        throw e;
    }

    mPayloadLen = payloadLen;
    mIsText = isText;

    WebSocket::sendHeader(this, payloadLen, isText, mIsServer);
}

/*!
 * \brief   Web Socket ヘッダー送信
 */
void WebSocket::sendHeader(Socket* socket, uint64_t payloadLen, bool isText, bool toClient) throw(Exception)
{
    ByteBuffer buffer(2 + 8 + 4);
    char opcode = 0x01;     // text frame
    char mask =   0x00;     // no mask

    if (isText == false)
    {
        // binary frame
        opcode = 0x02;
    }

    if (toClient == false)
    {
        // client -> server
        mask = (char)0x80;
    }

    // FIN & opcode
    buffer.put((char)0x80 | opcode);

    // MASK & Payload length
    if (payloadLen < 126)
    {
        // 0 ～ 125 bytes
        buffer.put(mask | (char)payloadLen);
    }
    else if (payloadLen <= 0xFFFF)
    {
        // 126 ～ 65535 bytes
        buffer.put(mask | (char)126);
        buffer.putShort((short)payloadLen);
    }
    else
    {
        // 65536 ～
        buffer.put(mask | (char)127);
        buffer.putLong(payloadLen);
    }

    // Masking-key
    if (toClient == false)
    {
        buffer.put((char)0x00);
        buffer.put((char)0x00);
        buffer.put((char)0x00);
        buffer.put((char)0x00);
    }

    socket->Socket::send(&buffer, buffer.getLength());
}

/*!
 * \brief   送信前チェック
 */
void WebSocket::check(uint64_t len, bool isText) throw(Exception)
{
    if (mIsText != isText)
    {
        Exception e;
        static const char* message[] =
        {
            "データタイプがバイナリに設定されている時に、テキストデータを送信しようとしました。",
            "データタイプがテキストに設定されている時に、バイナリデータを送信しようとしました。",
        };

        e.setMessage(message[mIsText]);
        throw e;
    }

    mPayloadLen -= len;

    if ((int64_t)mPayloadLen < 0)
    {
        Exception e;

        e.setMessage("送信予定のデータ長を超えてしまいました。");
        throw e;
    }
}

/*!
 * \brief   バイナリ送信
 */
void WebSocket::send(const int32_t* value) const throw(Exception)
{
    WebSocket* self = (WebSocket*)this;
    int32_t len = sizeof(*value);

    if (mPayloadLen == 0)
    {
        ScopedLock lock(mMutex);
        sendHeader(self, len, false, mIsServer);
        Socket::send(value);
    }
    else
    {
        self->check(len, false);
        Socket::send(value);
    }
}

/*!
 * \brief   バイナリ送信
 */
void WebSocket::send(const uint32_t* value) const throw(Exception)
{
    WebSocket* self = (WebSocket*)this;
    int32_t len = sizeof(*value);

    if (mPayloadLen == 0)
    {
        ScopedLock lock(mMutex);
        sendHeader(self, len, false, mIsServer);
        Socket::send(value);
    }
    else
    {
        self->check(len, false);
        Socket::send(value);
    }
}

/*!
 * \brief   バイナリ送信
 */
void WebSocket::send(const Buffer* buffer, int32_t len) const throw(Exception)
{
    WebSocket* self = (WebSocket*)this;

    if (mPayloadLen == 0)
    {
        ScopedLock lock(mMutex);
        sendHeader(self, len, false, mIsServer);
        Socket::send(buffer, len);
    }
    else
    {
        self->check(len, false);
        Socket::send(buffer, len);
    }
}

/*!
 * \brief   バイナリ送信
 */
void WebSocket::send(const char* buffer, int32_t len) const throw(Exception)
{
    WebSocket* self = (WebSocket*)this;

    if (mPayloadLen == 0)
    {
        ScopedLock lock(mMutex);
        sendHeader(self, len, false, mIsServer);
        Socket::send(buffer, len);
    }
    else
    {
        self->check(len, false);
        Socket::send(buffer, len);
    }
}

/*!
 * \brief   テキスト送信
 */
void WebSocket::send(const CoreString* str) const throw(Exception)
{
    WebSocket* self = (WebSocket*)this;
    int32_t len = str->getLength();

    if (mPayloadLen == 0)
    {
        ScopedLock lock(mMutex);
        sendHeader(self, len, true, mIsServer);
        Socket::send(str, len);
    }
    else
    {
        self->check(len, true);
        Socket::send(str, len);
    }
}

/*!
 * \brief   受信
 */
ByteBuffer* WebSocket::recv(ByteBuffer* dataBuffer) const throw(Exception)
{
    return recv((WebSocket*)this, dataBuffer);
}

ByteBuffer* WebSocket::recv(Socket* socket, ByteBuffer* dataBuffer) throw(Exception)
{
    #define OPE_TEXT    0x01
    #define OPE_BINARY  0x02
    #define OPE_CLOSE   0x08
    #define OPE_PONG    0x0A

    ByteBuffer buffer(2 + 8 + 4);
    const char* p = buffer.getBuffer();

    socket->recv(&buffer, 2);

    // opcode
    char opcode = p[0] & 0x0F;

    if (opcode != OPE_TEXT &&
        opcode != OPE_BINARY &&
        opcode != OPE_CLOSE &&
        opcode != OPE_PONG)
    {
        noticeLog("unknown opcode=0x%02X", opcode);
        return nullptr;
    }

    // MASK & Payload length
    bool mask = ((p[1] & 0x80) == 0x80);
    uint64_t payloadLen = p[1] & 0x7F;

    if (payloadLen == 126)
    {
        socket->recv(&buffer, 2);
        payloadLen = (uint16_t)buffer.getShort();
    }
    else if (payloadLen == 127)
    {
        socket->recv(&buffer, 8);
        payloadLen = buffer.getLong();
    }

    // Masking-key
    if (mask)
        socket->recv(&buffer, 4);

    // Payload Data
    ByteBuffer* newDataBuffer = nullptr;

    if (dataBuffer)
    {
        if (opcode == OPE_TEXT || opcode == OPE_BINARY)
        {
            // 受信バッファが指定されていて、受信データ（テキスト／バイナリ）長が異なる場合は例外スロー
            if (payloadLen != dataBuffer->getCapacity())
            {
                Exception e;
                e.setMessage("opcode=0x%02X, payloadLen=%d, dataBufferLen=%d", opcode, payloadLen, dataBuffer->getCapacity());

                throw e;
            }
        }
        else
        {
            // 受信バッファが指定されていて、OPE_TEXTでもOPE_BINARYでもない場合は、
            // 指定された受信バッファのサイズが受信データ長未満の可能性を考慮して
            // 内部で確保するバッファを使用することとし、受信バッファは一旦nullptrにする
            dataBuffer = nullptr;
        }
    }

    if (dataBuffer == nullptr)
    {
        if (payloadLen != 0)
        {
            newDataBuffer = new ByteBuffer((int32_t)payloadLen);
            dataBuffer = newDataBuffer;
        }
    }

    if (payloadLen != 0)
    {
        socket->recv(dataBuffer, (int32_t)payloadLen);

        if (mask)
        {
            char* p2 = dataBuffer->getBuffer();

            for (uint64_t i = 0; i < payloadLen; i++)
            {
//              noticeLog("%03u: %02X ^ %02X = %02X", i, (uint8_t)p2[i], (uint8_t)p[i % 4], (uint8_t)(p2[i] ^ p[i % 4]));
                p2[i] ^= p[i % 4];
            }
        }
    }

    if (opcode != OPE_TEXT &&
        opcode != OPE_BINARY)
    {
        noticeLog("opcode=0x%02X", opcode);

        delete newDataBuffer;
        newDataBuffer = nullptr;
        dataBuffer = nullptr;
    }

    if (opcode == OPE_CLOSE)
        socket->close();

    return dataBuffer;
}

/*!
 * \brief   リスナーにオープン通知
 */
void WebSocket::notifyOpen()
{
    mReceiver->notifyOpen();
}

/*!
 * \brief   リスナーにエラー通知
 */
void WebSocket::notifyError(const char* message)
{
    mReceiver->notifyError(message);
}

} // namespace slog
