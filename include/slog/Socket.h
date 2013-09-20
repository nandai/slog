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
 *  \file   Socket.h
 *  \brief  ソケットクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once

#include "slog/Exception.h"

#if defined(_WINDOWS)
    #if !defined(MODERN_UI)
        #pragma comment(lib, "ws2_32.lib")
    #endif
#else
    #include <arpa/inet.h>
    #include <sys/socket.h>
#endif

namespace slog
{
class Buffer;
class ByteBuffer;
class CoreString;

/*!
 *  \brief  ソケットクラス
 */
class SLOG_API Socket
{
public:     static const int STREAM;
private:    struct Data;

#if defined(_WINDOWS)
    #if defined(MODERN_UI)
            #define SOCK_STREAM 1
            Windows::Networking::Sockets::StreamSocket^     mSocket;
            Windows::Storage::Streams::DataWriter^          mWriter;
            Windows::Storage::Streams::DataReader^          mReader;
    #else
            int64_t         mSocket;        //!< ソケット
    #endif
#else
            int             mSocket;        //!< ソケット
#endif

            Data*           mData;
            bool            mInet;          //!< true:AF_INET、false:AF_UNIX
            bool            mStream;        //!< true:SOCK_STREAM, false:SOCK_DGRAM
            ByteBuffer*     mBuffer;        //!< 数値用送受信バッファ
            bool            mConnect;       //!< 接続しているかどうか

            /*!
             * コンストラクタ／デストラクタ
             */
public:     Socket();
            virtual ~Socket();

            /*!
             * オープン／クローズ
             */
            void open(bool inet = true, int type = STREAM) throw(Exception);
            virtual int close();

            /*!
             * 接続関連
             */
            void bind(unsigned short port) throw(Exception);
            void listen(int backlog = 5) const throw(Exception);
            void accept(const Socket* servSocket) throw(Exception);
            virtual void connect(const CoreString& ipAddress, unsigned short port) throw(Exception);

#if defined(__ANDROID__)
            void bind(   const CoreString& path) throw(Exception);
            void connect(const CoreString& path) throw(Exception);
#endif

            /*!
             * SSL使用
             */
            void useSSL(const CoreString& certificate, const CoreString& privateKey);
            void useSSL();

            /*!
             * オープンしているか
             */
            bool isOpen() const;

            /*!
             * 接続しているか
             */
            bool isConnect() const;

            /*!
             * ソケットオプション設定
             */
            int setReUseAddress(bool reUse);
            int setRecvTimeOut(int32_t msec);
            int setNoDelay(bool noDelay);

            /*!
             * IP 取得
             */
            const CoreString& getInetAddress() const;
            const CoreString& getMyInetAddress() const;

            /*!
             * 送信
             */
            virtual void send(const  int32_t* value) const throw(Exception);
            virtual void send(const uint32_t* value) const throw(Exception);
            virtual void send(const Buffer* buffer, int32_t len) const throw(Exception);
            virtual void send(const char*   buffer, int32_t len) const throw(Exception);

            /*!
             * 受信
             */
            void recv( int32_t* value) const throw(Exception);
            void recv(uint32_t* value) const throw(Exception);
            void recv(Buffer* buffer, int32_t len) const throw(Exception);

            /*!
             * 受信データがあるか
             */
            bool isReceiveData(int32_t timeoutMS = 0) const throw(Exception);

            /*!
             *  \brief  スタートアップ／クリーンアップ
             */
public:     static void startup();
            static void cleanup();
};

} // namespace slog
