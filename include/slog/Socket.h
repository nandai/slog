/*
 * Copyright (C) 2011-2015 printf.jp
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
 * \file    Socket.h
 * \brief   ソケットクラス
 * \author  Copyright 2011-2015 printf.jp
 */
#pragma once

#include "slog/Exception.h"

#if defined(_WINDOWS)
    #if !defined(MODERN_UI)
        #pragma comment(lib, "ws2_32.lib")
    #endif
#endif

namespace slog
{
class Buffer;
class ByteBuffer;
class CoreString;

/*!
 * \brief   ソケットクラス
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
            /*!
             * ソケット
             */
            int64_t mSocket;
    #endif
#else
            /*!
             * ソケット
             */
            int mSocket;
#endif

            /*!
             * 内部データ
             */
            Data* mData;

            /*!
             * true:AF_INET、false:AF_UNIX
             */
            bool mInet;

            /*!
             * true:SOCK_STREAM, false:SOCK_DGRAM
             */
            bool mStream;

            /*!
             * 数値用送受信バッファ
             */
            ByteBuffer* mBuffer;

            /*!
             * 接続しているかどうか
             */
            bool mConnect;

            /*!
             * コンストラクタ
             */
public:     Socket();

            /*!
             * デストラクタ
             */
            virtual ~Socket();

            /*!
             * オープン
             */
            void open(bool inet = true, int type = STREAM) throw(Exception);

            /*!
             * クローズ
             */
            virtual int close();

            /*!
             * 接続準備
             */
            void bind(unsigned short port) throw(Exception);

            /*!
             * 接続待ち設定
             */
            void listen(int backlog = 5) const throw(Exception);

            /*!
             * 接続受付
             */
            void accept(const Socket* servSocket) throw(Exception);

            /*!
             * 接続
             */
            virtual void connect(const CoreString* ipAddress, unsigned short port) throw(Exception);

#if defined(__ANDROID__)
            void bind(   const CoreString& path) throw(Exception);
            void connect(const CoreString& path) throw(Exception);
#endif

            /*!
             * SSL使用
             */
            void useSSL(const CoreString* certificate, const CoreString* privateKey, const CoreString* certificateChain = nullptr) throw(Exception);

            /*!
             * SSL使用
             */
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
             * アドレス再利用設定
             */
            int setReUseAddress(bool reUse);

            /*!
             * 受信タイムアウト設定
             */
            int setRecvTimeOut(int32_t msec);

            /*!
             * Nagleアルゴリズム設定
             */
            int setNoDelay(bool noDelay);

            /*!
             * 接続元／先ホスト名取得
             */
            void getHostName(slog::CoreString* hostName) const;

            /*!
             * 接続元／先IPアドレス取得
             */
            const CoreString* getInetAddress() const;

            /*!
             * 自IPアドレス取得
             */
            const CoreString* getMyInetAddress() const;

            /*!
             * 送信
             */
            virtual void send(const int32_t* value) const throw(Exception);

            /*!
             * 送信
             */
            virtual void send(const uint32_t* value) const throw(Exception);

            /*!
             * 送信
             */
            virtual void send(const Buffer* buffer, int32_t len) const throw(Exception);

            /*!
             * 送信
             */
            virtual void send(const char* buffer, int32_t len) const throw(Exception);

            /*!
             * 受信
             */
            void recv(int32_t* value) const throw(Exception);

            /*!
             * 受信
             */
            void recv(uint32_t* value) const throw(Exception);

            /*!
             * 受信
             */
            void recv(Buffer* buffer, int32_t len) const throw(Exception);

            /*!
             * 受信
             */
            void recv(CoreString* buffer) const throw(Exception);

            /*!
             * 受信データがあるか
             */
            bool isReceiveData(int32_t timeoutMS = 0) const throw(Exception);

            /*!
             * スタートアップ
             */
public:     static void startup();

            /*!
             * クリーンアップ
             */
            static void cleanup();

            /*!
             * ドメインが存在するかどうか
             */
            static bool existsDomain(const char* domain);
};

} // namespace slog
