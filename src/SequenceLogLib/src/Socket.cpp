/*
 * Copyright (C) 2011 log-tools.net
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
 *  \file   Socket.cpp
 *  \brief  ソケットクラス
 *  \author Copyright 2011 log-tools.net
 */
#include "slog/Socket.h"
#include "slog/ByteBuffer.h"

#if defined(__unix__)
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <unistd.h>
#endif

namespace slog
{

/*!
 *  \brief  コンストラクタ
 */
Socket::Socket()
{
    mSocket = -1;
    mInet = true;
    mBuffer = new ByteBuffer(sizeof(int64_t));
    mConnect = false;
}

/*!
 *  \brief  デストラクタ
 */
Socket::~Socket()
{
    delete mBuffer;
    close();
}

/*!
 *  \brief  オープン
 */
void Socket::open(bool inet) throw(Exception)
{
    int af = AF_INET;

#if defined(__ANDROID__)
    if (inet == false)
    {
        af = AF_UNIX;
        mInet = false;
    }
#endif

    mSocket = socket(af, SOCK_STREAM, 0);

    if (mSocket == -1)
    {
        Exception e;
        e.setMessage("Socket::open()");

        throw e;
    }
}

/*!
 *  \brief  クローズ
 *
 *  \retval 0       正常終了
 *          0以外 異常終了
 *
 *  \note   クローズでは何か問題が発生しても例外は投げない。
 */
int Socket::close()
{
    if (mSocket == -1)
        return 0;

#if defined(_WINDOWS)
    int result = closesocket(mSocket);
#else
    int result = ::close(mSocket);
#endif

    mSocket = -1;
    mConnect = false;

    return result;
}

/*!
 *  \brief  接続準備
 */
void Socket::bind(
    unsigned short port)        //!< ポート番号

    throw(Exception)
{
    mAddr.sin_family = AF_INET;
    mAddr.sin_port = htons(port);
//  mAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    mAddr.sin_addr.s_addr =      INADDR_ANY;

    int result = ::bind(mSocket, (sockaddr*)&mAddr, sizeof(mAddr));

    if (result != 0)
    {
        Exception e;
        e.setMessage("Socket::bind(%u)", port);

        throw e;
    }
}

#if defined(__ANDROID__)
/*!
 *  \brief  接続準備
 */
void Socket::bind(
    const CoreString& path)     //!< パス

    throw(Exception)
{
    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path.getBuffer());

    int result = ::bind(mSocket, (sockaddr*)&addr, sizeof(addr));

    if (result != 0)
    {
        Exception e;
        e.setMessage("Socket::bind(\"%s\")", path.getBuffer());

        throw e;
    }
}
#endif

/*!
 *  \brief  接続待ち設定
 */
void Socket::listen(
    int backlog)                //!< 接続キューの数

    const
    throw(Exception)
{
    int result = ::listen(mSocket, backlog);

    if (result != 0)
    {
        Exception e;
        e.setMessage("Socket::listen(%d)", backlog);

        throw e;
    }
}

/*!
 *  \brief  接続受付
 */
void Socket::accept(
    const Socket* servSocket)   //!< サーバーソケット

    throw(Exception)
{
    if (mInet)
    {
#if defined(_WINDOWS)
        int len = sizeof(mAddr);
#else
        socklen_t len = sizeof(mAddr);
#endif

        mSocket = ::accept(servSocket->mSocket, (sockaddr*)&mAddr, &len);
    }
#if defined(__ANDROID__)
    else
    {
        sockaddr_un addr;
        socklen_t len = sizeof(addr);
        mSocket = ::accept(servSocket->mSocket, (sockaddr*)&addr, &len);
    }
#endif

    if (mSocket == -1)
        throw Exception();
}

/*!
 *  \brief  接続
 */
void Socket::connect(
    const CoreString& ipAddress,    //!< IPアドレス
    unsigned short port)            //!< ポート番号

    throw(Exception)
{
    mAddr.sin_family = AF_INET;
    mAddr.sin_port = htons(port);
//  mAddr.sin_addr.S_un.S_addr = inet_addr(ipAddress);
    mAddr.sin_addr.s_addr =      inet_addr(ipAddress.getBuffer());

    int result = ::connect(mSocket, (sockaddr*)&mAddr, sizeof(mAddr));

    if (result != 0)
    {
        Exception e;
        e.setMessage("Socket::connect(\"%s\", %u)", ipAddress.getBuffer(), port);

        throw e;
    }

    mConnect = true;
}

#if defined(__ANDROID__)

/*!
 *  \brief  接続
 */
void Socket::connect(
    const CoreString& path)     //!< パス

    throw(Exception)
{
    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path.getBuffer());

    int result = ::connect(mSocket, (sockaddr*)&addr, sizeof(addr));

    if (result != 0)
    {
        Exception e;
        e.setMessage("Socket::connect(\"%s\")", path.getBuffer());

        throw e;
    }

    mConnect = true;
}
#endif

/*!
 *  \brief  オープンしているかどうか調べる
 */
bool Socket::isOpen() const
{
    return (mSocket != -1);
}

/*!
 *  \brief  接続しているかどうか調べる
 */
bool Socket::isConnect() const
{
    return mConnect;
}

/*!
 *  \brief  アドレス再利用設定
 */
int Socket::setReUseAddress(bool reUse) const
{
    int on = (reUse ? 1 : 0);   // アドレス再利用有効化（bind()のtime waitによるEADDRINUSE回避のため）

    int result = setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));
    return result;
}

/*!
 *  \brief  受信タイムアウト設定
 */
int Socket::setRecvTimeOut(int32_t msec) const
{
    timeval tm;
    tm.tv_sec = msec;
    tm.tv_usec = 0;

    int result = setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tm, sizeof(timeval));
    return result;
}

/*!
 *  \brief  接続先IPアドレス取得
 *
 *  \retval NULL以外  接続先IPアドレス
 *  \retval NULL        未接続
 */
const CoreString& Socket::getInetAddress() const
{
    CoreString& inetAddress = (CoreString&)mInetAddress;

    if (isOpen() == false)
        inetAddress.setLength(0);
    else
        inetAddress.copy(inet_ntoa(mAddr.sin_addr));

    return inetAddress;
}

/*!
 *  \brief  送信
 */
void Socket::send(const int32_t* value) const throw(Exception)
{
    send((uint32_t*)value);
}

/*!
 *  \brief  送信
 */
void Socket::send(const uint32_t* value) const throw(Exception)
{
    mBuffer->setPosition(0);
    mBuffer->putInt(*value);

    send(mBuffer, sizeof(*value));
}

/*!
 *  \brief  送信
 */
void Socket::send(
    const Buffer* buffer,   //!< 送信バッファ
    int32_t len)            //!< 送信サイズ

    const
    throw(Exception)
{
    buffer->validateOverFlow(0, len);

    const char* p = buffer->getBuffer();
    int32_t remains = len;
    int32_t loopCount = 0;  // デバッグ用（条件付きブレークポイントで使用）

    while (remains)
    {
#if defined(_WINDOWS)
        int result = ::send(mSocket, p, remains, 0);
#else
        int result = ::send(mSocket, p, remains, MSG_NOSIGNAL);
#endif

        if (result == -1)
        {
            Exception e;
            e.setMessage("Socket::send(%d)", len);

            throw e;
        }

        p += result;
        remains -= result;

        loopCount++;
    }
}

/*!
 *  \brief  受信
 */
void Socket::recv(
    int32_t* value)         //!< 受信データ

    const
    throw(Exception)
{
    recv((uint32_t*)value);
}

/*!
 *  \brief  受信
 */
void Socket::recv(
    uint32_t* value)        //!< 受信データ

    const
    throw(Exception)
{
    mBuffer->setPosition(0);
    recv(mBuffer, sizeof(*value));

    *value = mBuffer->getInt();
}

/*!
 *  \brief  受信
 */
void Socket::recv(
    Buffer* buffer,         //!< 受信バッファ
    int32_t len)            //!< 受信バッファサイズ

    const
    throw(Exception)
{
    buffer->validateOverFlow(len);

    char* p = buffer->getBuffer();
    int32_t remains = len;
    int32_t loopCount = 0;  // デバッグ用（条件付きブレークポイントで使用）

    while (remains > 0)
    {
        int result = ::recv(mSocket, p, remains, 0);

        if (result <= 0)
        {
            Exception e;

            e.setMessage(result == -1
                ? "Socket::recv(%d)"                // エラー発生
                : "Socket::recv(%d) retval 0",      // 接続先から切断された
                len);

            throw e;
        }

        p += result;
        remains -= result;

        loopCount++;
    }
}

/*!
 *  \brief  スタートアップ
 */
void Socket::startup()
{
#if defined(_WINDOWS)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,0), &wsaData);
#endif
}

/*!
 *  \brief  クリーンアップ
 */
void Socket::cleanup()
{
#if defined(_WINDOWS)
    WSACleanup();
#endif
}

} // namespace slog
