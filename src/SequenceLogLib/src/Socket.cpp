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
 *  \file   Socket.cpp
 *  \brief  ソケットクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/Socket.h"
#include "slog/ByteBuffer.h"
#include "slog/FixedString.h"

#if defined(__unix__)
    #include <sys/un.h>
    #include <unistd.h>
#endif

#if defined(MODERN_UI)
    #include <ppltasks.h>
    using namespace Concurrency;
    using namespace Windows::Foundation;
    using namespace Windows::Storage::Streams;
    using namespace Windows::Networking;
    using namespace Windows::Networking::Sockets;

    inline int waitForSocket(IAsyncInfo^ asyncInfo)
    {
        int result = 0;

        while (
            asyncInfo->Status != AsyncStatus::Completed &&
            asyncInfo->Status != AsyncStatus::Error)
        {
        }

        if (asyncInfo->Status != AsyncStatus::Completed)
            result = -1;

        return result;
    }
#endif

namespace slog
{

struct Socket::Data
{
    FixedString<16> mInetAddress;   //!< IPv4
};

/*!
 *  \brief  コンストラクタ
 */
Socket::Socket()
{
    mData = new Data;
#if defined(MODERN_UI)
    mSocket = nullptr;
#else
    mSocket = -1;
#endif
    mInet = true;
    mStream = true;
    mBuffer = new ByteBuffer(sizeof(int64_t));
    mConnect = false;
}

/*!
 *  \brief  デストラクタ
 */
Socket::~Socket()
{
    close();

    delete mBuffer;
    delete mData;
}

/*!
 *  \brief  オープン
 */
void Socket::open(bool inet, int type) throw(Exception)
{
#if defined(MODERN_UI)
    mStream = (type == SOCK_STREAM);
    mSocket = ref new Windows::Networking::Sockets::StreamSocket();
#else
    int af = AF_INET;

#if defined(__ANDROID__)
    if (inet == false)
    {
        af = AF_UNIX;
        mInet = false;
    }
#endif

    mStream = (type == SOCK_STREAM);
    mSocket = socket(af, type, 0);

    if (mSocket == -1)
    {
        Exception e;
        e.setMessage("Socket::open()");

        throw e;
    }
#endif
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
#if defined(MODERN_UI)
    if (mSocket == nullptr)
        return 0;

    int result = 0;

    delete mSocket;
    mSocket = nullptr;
#else
    if (mSocket == -1)
        return 0;

#if defined(_WINDOWS)
    int result = closesocket(mSocket);
#else
    int result = ::close(mSocket);
#endif

    mSocket = -1;
#endif

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
#if defined(MODERN_UI)
#else
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
#endif
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
#if defined(MODERN_UI)
#else
    int result = ::listen(mSocket, backlog);

    if (result != 0)
    {
        Exception e;
        e.setMessage("Socket::listen(%d)", backlog);

        throw e;
    }
#endif
}

/*!
 *  \brief  接続受付
 */
void Socket::accept(
    const Socket* servSocket)   //!< サーバーソケット

    throw(Exception)
{
#if defined(MODERN_UI)
#else
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
#endif
}

/*!
 *  \brief  接続
 */
void Socket::connect(
    const CoreString& ipAddress,    //!< IPアドレス
    unsigned short port)            //!< ポート番号

    throw(Exception)
{
    int result = 0;

#if defined(MODERN_UI)
    UTF16LE utf16le;
    utf16le.conv(ipAddress);

    Platform::String^ strIp = ref new Platform::String(utf16le.getBuffer());
    HostName^ hostName = ref new HostName(strIp);

    IAsyncAction^ action =
        mSocket->ConnectAsync(hostName, port.ToString(), Windows::Networking::Sockets::SocketProtectionLevel::PlainSocket);

    result = waitForSocket(action);
#else
    mAddr.sin_family = AF_INET;
    mAddr.sin_port = htons(port);
//  mAddr.sin_addr.S_un.S_addr = inet_addr(ipAddress);
    mAddr.sin_addr.s_addr =      inet_addr(ipAddress.getBuffer());

    if (mStream)
        result = ::connect(mSocket, (sockaddr*)&mAddr, sizeof(mAddr));
#endif

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
#if defined(MODERN_UI)
    return (mSocket != nullptr);
#else
    return (mSocket != -1);
#endif
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
int Socket::setReUseAddress(bool reUse)
{
#if defined(MODERN_UI)
    return 0;
#else
    int on = (reUse ? 1 : 0);   // アドレス再利用有効化（bind()のtime waitによるEADDRINUSE回避のため）

    int result = setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));
    return result;
#endif
}

/*!
 *  \brief  受信タイムアウト設定
 */
int Socket::setRecvTimeOut(int32_t msec)
{
#if defined(MODERN_UI)
    return 0;
#else
    timeval tm;
    tm.tv_sec = msec;
    tm.tv_usec = 0;

    int result = setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tm, sizeof(timeval));
    return result;
#endif
}

/*!
 *  \brief  接続先IPアドレス取得
 *
 *  \retval NULL以外  接続先IPアドレス
 *  \retval NULL        未接続
 */
const CoreString& Socket::getInetAddress() const
{
    CoreString& inetAddress = (CoreString&)mData->mInetAddress;

#if defined(MODERN_UI)
#else
    if (isOpen() == false)
        inetAddress.setLength(0);
    else
        inetAddress.copy(inet_ntoa(mAddr.sin_addr));
#endif

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
    send(buffer->getBuffer(), len);
}

/*!
 *  \brief  送信
 */
void Socket::send(
    const char* buffer,     //!< 送信バッファ
    int32_t len)            //!< 送信サイズ

    const
    throw(Exception)
{
    const char* p = buffer;
    int32_t remains = len;
    int32_t loopCount = 0;  // デバッグ用（条件付きブレークポイントで使用）
    int32_t result = 0;

#if defined(MODERN_UI)
    if (mWriter == nullptr)
    {
        ((Windows::Storage::Streams::DataWriter^)mWriter) =
            ref new Windows::Storage::Streams::DataWriter(mSocket->OutputStream);
    }

    Platform::Array<unsigned char>^ arr = ref new Platform::Array<unsigned char>((unsigned char*)p, remains);
    mWriter->WriteBytes(arr);

    DataWriterStoreOperation^ ope = mWriter->StoreAsync();
    result = waitForSocket(ope);
#else
    while (remains)
    {
#if defined(_WINDOWS)
        int flag = 0;
#else
        int flag = MSG_NOSIGNAL;
#endif

        result = (mStream
            ? ::send(  mSocket, p, remains, flag)
            : ::sendto(mSocket, p, remains, flag, (sockaddr*)&mAddr, sizeof(mAddr))
        );

        if (result == -1)
            break;

        p += result;
        remains -= result;

        loopCount++;
    }
#endif // MODERN_UI

    if (result == -1)
    {
        Exception e;
        e.setMessage("Socket::send(%d)", len);

        throw e;
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
    int32_t result = 0;

#if defined(MODERN_UI)
    if (mReader == nullptr)
    {
        ((Windows::Storage::Streams::DataReader^)mReader) =
            ref new Windows::Storage::Streams::DataReader(mSocket->InputStream);
    }

    Platform::Array<unsigned char>^ arr = ref new Platform::Array<unsigned char>(remains);
    DataReaderLoadOperation^ ope = mReader->LoadAsync(len);

    if (waitForSocket(ope) == 0)
    {
        mReader->ReadBytes(arr);
        result = len;

        char* src = (char*)arr->begin();
        for (int i = 0; i < len; i++)
            p[i] = src[i];
    }
#else
    while (remains > 0)
    {
        result = ::recv(mSocket, p, remains, 0);

        if (result <= 0)
            break;

        p += result;
        remains -= result;

        loopCount++;
    }
#endif

    if (result <= 0)
    {
        Exception e;

        e.setMessage(result == -1
            ? "Socket::recv(%d)"                // エラー発生
            : "Socket::recv(%d) retval 0",      // 接続先から切断された
            len);

        throw e;
    }

    buffer->setLength(len);
}

/*!
 *  \brief  受信データが有るかどうか
 */
bool Socket::isReceiveData(int32_t timeoutMS) const
{
    timeval timeout =
    {
         timeoutMS / 1000,
        (timeoutMS % 1000) * 1000
    };
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(mSocket, &fds);

    int n = select((int)mSocket + 1, &fds, NULL, NULL, &timeout);
    return (0 < n);
}

/*!
 *  \brief  スタートアップ
 */
void Socket::startup()
{
#if defined(_WINDOWS) && !defined(MODERN_UI)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,0), &wsaData);
#endif
}

/*!
 *  \brief  クリーンアップ
 */
void Socket::cleanup()
{
#if defined(_WINDOWS) && !defined(MODERN_UI)
    WSACleanup();
#endif
}

} // namespace slog
