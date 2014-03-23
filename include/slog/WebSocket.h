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
 * \file    WebSocket.h
 * \brief   Web Socket クラス
 * \author  Copyright 2013-2014 printf.jp
 */
#pragma once

#include "slog/Socket.h"
#include "slog/Thread.h"

namespace slog
{
class Mutex;
class WebSocketReceiver;
class WebSocketListener;

/*!
 * \brief   Web Socket クラス
 */
class SLOG_API WebSocket : public Socket
{
            bool                mIsServer;      //!< サーバー側ソケットかどうか
            uint64_t            mPayloadLen;    //!< データ長
            bool                mIsText;        //!< 送信データがテキストかどうか
            Mutex*              mMutex;         //!< ミューテックス
            WebSocketReceiver*  mReceiver;      //!< Web Socket 受信

            /*!
             * コンストラクタ／デストラクタ
             */
public:     WebSocket(bool isServer);
            virtual ~WebSocket() override;

            /*!
             * 初期化
             */
protected:  void init();

            /*!
             * クローズ
             */
public:     virtual int close() override;

            /*!
             * リスナー設定
             */
            void setListener(WebSocketListener* listener);

            /*!
             * Web Socket ヘッダー送信
             */
public:            void sendHeader(                uint64_t payloadLen, bool isText = true)                       throw(Exception);
            static void sendHeader(Socket* socket, uint64_t payloadLen, bool isText = true, bool toClient = true) throw(Exception);

            /*!
             * 送信前チェック
             */
private:    void check(uint64_t len, bool isText) throw(Exception);

            /*!
             * バイナリ送信
             */
public:     virtual void send(const  int32_t* value) const throw(Exception) override;
            virtual void send(const uint32_t* value) const throw(Exception) override;
            virtual void send(const Buffer* buffer, int32_t len) const throw(Exception) override;
            virtual void send(const char*   buffer, int32_t len) const throw(Exception) override;

            /*!
             * テキスト送信
             */
            virtual void send(const CoreString* str) const throw(Exception);

            /*!
             * 受信
             */
                   ByteBuffer* recv(                ByteBuffer* dataBuffer) const throw(Exception);
            static ByteBuffer* recv(Socket* socket, ByteBuffer* dataBuffer)       throw(Exception);

            /*!
             * ミューテックス
             */
            Mutex* getMutex() const {return (Mutex*)mMutex;}

            /*!
             * リスナーに通知
             */
protected:  void notifyOpen();
            void notifyError(const char* message);
};

/*!
 * \brief   Web Socket リスナークラス
 */
class SLOG_API WebSocketListener : public ThreadListener
{
public:     virtual void onOpen() {}
            virtual void onError(const char* message) {}
            virtual void onMessage(const ByteBuffer& buffer) {}
            virtual void onClose() {}
};

} // namespace slog
