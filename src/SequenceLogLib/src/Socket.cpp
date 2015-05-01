/*
 * Copyright (C) 2011-2014 printf.jp
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
 * \file    Socket.cpp
 * \brief   ソケットクラス
 * \author  Copyright 2011-2014 printf.jp
 */
#include "slog/Socket.h"
#include "slog/ByteBuffer.h"
#include "slog/FixedString.h"
#include "slog/Thread.h"

#include <openssl/ssl.h>
#include <openssl/err.h>

#if defined(__unix__)
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <netinet/tcp.h>
    #include <unistd.h>
    #include <netdb.h>
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
const int Socket::STREAM = SOCK_STREAM;

struct Socket::Data
{
    FixedString<16> mInetAddress;   //!< 接続元／先IPアドレス
    FixedString<16> mMyInetAddress; //!< 自IPアドレス

    sockaddr_in     mAddr;          //!< ソケット情報

    SSL_CTX*        mCTX;           //!< SSLコンテキスト
    SSL*            mSSL;           //!< SSL
};

/*!
 * \brief   コンストラクタ
 */
Socket::Socket()
{
    mData = new Data;

    mData->mCTX = nullptr;
    mData->mSSL = nullptr;

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
 * \brief   デストラクタ
 */
Socket::~Socket()
{
    close();

    delete mBuffer;
    delete mData;
}

/*!
 * \brief   オープン
 *
 * \param[in]   inet    インターネットプロトコルか、ローカル通信か
 * \param[in]   type    通信方式
 *
 * \return  なし
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
 * \brief   クローズ
 *
 * \retval  0     正常終了
 * \retval  0以外 異常終了
 *
 * \note    クローズでは何か問題が発生しても例外は投げない。
 */
int Socket::close()
{
    if (mSocket == -1)
        return 0;

    if (mData->mSSL)
    {
        SSL_shutdown(mData->mSSL);
        SSL_free(    mData->mSSL);
        SSL_CTX_free(mData->mCTX);

        mData->mCTX = nullptr;
        mData->mSSL = nullptr;
    }

//  Thread::sleep(1000);

#if defined(_WINDOWS)
    int result = closesocket((SOCKET)mSocket);
#else
    int result = ::close(mSocket);
#endif

    mSocket = -1;
    mConnect = false;

    return result;
}

/*!
 * \brief   接続準備
 *
 * \param[in]   port    ポート番号
 *
 * \return  なし
 */
void Socket::bind(unsigned short port) throw(Exception)
{
#if defined(MODERN_UI)
#else
    sockaddr_in* addr = &mData->mAddr;

    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
//  addr->sin_addr.S_un.S_addr = INADDR_ANY;
    addr->sin_addr.s_addr =      INADDR_ANY;

#if defined(_WINDOWS)
    int result = ::bind((SOCKET)mSocket, (sockaddr*)addr, sizeof(*addr));
#else
    int result = ::bind(        mSocket, (sockaddr*)addr, sizeof(*addr));
#endif

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
 * \brief   接続準備
 *
 * \param[in]   path    パス
 *
 * \return  なし
 */
void Socket::bind(const CoreString& path)

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
 * \brief   接続待ち設定
 *
 * \param[in]   backlog 接続キューの数
 *
 * \return  なし
 */
void Socket::listen(int backlog) const throw(Exception)
{
#if defined(MODERN_UI)
#else
#if defined(_WINDOWS)
    int result = ::listen((SOCKET)mSocket, backlog);
#else
    int result = ::listen(        mSocket, backlog);
#endif

    if (result != 0)
    {
        Exception e;
        e.setMessage("Socket::listen(%d)", backlog);

        throw e;
    }
#endif
}

/*!
 * \brief   接続受付
 *
 * \param[in]   servSocket  サーバーソケット
 *
 * \return  なし
 */
void Socket::accept(const Socket* servSocket) throw(Exception)
{
#if defined(MODERN_UI)
#else
    if (mInet)
    {
        sockaddr_in* addr = &mData->mAddr;

#if defined(_WINDOWS)
        int       len = sizeof(*addr);
        mSocket = ::accept((SOCKET)servSocket->mSocket, (sockaddr*)addr, &len);
#else
        socklen_t len = sizeof(*addr);
        mSocket = ::accept(        servSocket->mSocket, (sockaddr*)addr, &len);
#endif
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
 * \brief   接続
 *
 * \param[in]   ipAddress   IPアドレス
 * \param[in]   port        ポート番号
 *
 * \return  なし
 */
void Socket::connect(const CoreString* ipAddress, unsigned short port)

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
    hostent* host = gethostbyname(ipAddress->getBuffer());
    sockaddr_in* addr = &mData->mAddr;

    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);

    if (host == nullptr)
    {
//      addr->sin_addr.S_un.S_addr = inet_addr(ipAddress);
        addr->sin_addr.s_addr =      inet_addr(ipAddress->getBuffer());
    }
    else
    {
        memcpy(&addr->sin_addr.s_addr, host->h_addr, host->h_length);
//      noticeLog("len:%d, %u.%u.%u.%u", host->h_length,
//          (uint8_t)host->h_addr[0],
//          (uint8_t)host->h_addr[1],
//          (uint8_t)host->h_addr[2],
//          (uint8_t)host->h_addr[3]);
    }

    if (mStream)
    {
#if defined(_WINDOWS)
        result = ::connect((SOCKET)mSocket, (sockaddr*)addr, sizeof(*addr));
#else
        result = ::connect(        mSocket, (sockaddr*)addr, sizeof(*addr));
#endif
    }
#endif

    if (result != 0)
    {
        Exception e;
        e.setMessage("Socket::connect(\"%s\", %u)", ipAddress->getBuffer(), port);

        throw e;
    }

    mConnect = true;
}

#if defined(__ANDROID__)

/*!
 * \brief   接続
 *
 * \param[in]   path    パス
 *
 * \return  なし
 */
void Socket::connect(const CoreString& path)

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
 * \brief   SSL使用
 *
 * \param[in]   certificate         証明書
 * \param[in]   privateKey          秘密鍵
 * \param[in]   certificateChain    中間証明書（null可）※未対応
 *
 * \return  なし
 */
void Socket::useSSL(const CoreString* certificate, const CoreString* privateKey, const CoreString* certificateChain) throw(Exception)
{
    mData->mCTX = SSL_CTX_new(SSLv23_server_method());
//  mData->mSSL = SSL_new(mData->mCTX);
//
//  SSL_set_options(mData->mSSL, SSL_OP_NO_SSLv2);
//  SSL_set_fd(     mData->mSSL, (int)mSocket);

    int res = 1;
    int phase;

    do
    {
//      if (certificateChain == nullptr || certificateChain->getLength() == 0)
        if (true)
        {
            // 証明書
//          res =     SSL_use_certificate_file(mData->mSSL, certificate->getBuffer(), SSL_FILETYPE_PEM);
            res = SSL_CTX_use_certificate_file(mData->mCTX, certificate->getBuffer(), SSL_FILETYPE_PEM);
            phase = 1;

            if (res != 1)
                break;
        }
        else
        {
            // 中間証明書
            res = SSL_CTX_use_certificate_chain_file(mData->mCTX, certificateChain->getBuffer());
            phase = 2;

            if (res != 1)
                break;
        }

        // プライベートキー
//      res =     SSL_use_PrivateKey_file(mData->mSSL, privateKey->getBuffer(), SSL_FILETYPE_PEM);
        res = SSL_CTX_use_PrivateKey_file(mData->mCTX, privateKey->getBuffer(), SSL_FILETYPE_PEM);
        phase = 3;

        if (res != 1)
            break;

        // ここでSSLオブジェクト生成
        mData->mSSL = SSL_new(mData->mCTX);

        SSL_set_options(mData->mSSL, SSL_OP_NO_SSLv2);
        SSL_set_fd(     mData->mSSL, (int)mSocket);

        // SSL accept
        res = SSL_accept(mData->mSSL);
        phase = 4;

        if (res != 1)
            break;
    }
    while (false);

    if (res != 1)
    {
        Exception e;
        char buffer[120];

        ERR_error_string_n(ERR_get_error(), buffer, sizeof(buffer));

        e.setMessage("useSSL [%d]: %s", phase, buffer);
        throw e;
    }
}

/*!
 * \brief   SSL使用
 *
 * \return  なし
 */
void Socket::useSSL()
{
    mData->mCTX = SSL_CTX_new(SSLv23_client_method());
    mData->mSSL = SSL_new(mData->mCTX);

    SSL_set_options(mData->mSSL, SSL_OP_NO_SSLv2);
    SSL_set_fd(     mData->mSSL, (int)mSocket);

    int res = SSL_connect(mData->mSSL);

    if (res != 1)
    {
        Exception e;
        char buffer[120];

        ERR_error_string_n(ERR_get_error(), buffer, sizeof(buffer));

        e.setMessage("useSSL: %s", buffer);
        throw e;
    }
}

/*!
 * \brief   オープンしているか
 *
 * \return  オープンしている場合はtrue、していない場合はfalse
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
 *  \brief  接続しているか
 *
 * \return  接続している場合はtrue、していない場合はfalse
 */
bool Socket::isConnect() const
{
    return mConnect;
}

/*!
 * \brief   アドレス再利用設定
 *
 * \param[in]   reUse   即座にアドレスを再利用する場合はtrue
 *
 * \return  
 */
int Socket::setReUseAddress(bool reUse)
{
#if defined(MODERN_UI)
    return 0;
#else
    int on = (reUse ? 1 : 0);   // アドレス再利用有効化（bind()のtime waitによるEADDRINUSE回避のため）

#if defined(_WINDOWS)
    int result = setsockopt((SOCKET)mSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));
#else
    int result = setsockopt(        mSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));
#endif

    return result;
#endif
}

/*!
 * \brief   受信タイムアウト設定
 *
 * \param[in]   msec    受信タイムアウト値
 *
 * \return  
 */
int Socket::setRecvTimeOut(int32_t msec)
{
#if defined(MODERN_UI)
    return 0;
#else
    timeval tm;
    tm.tv_sec = msec;
    tm.tv_usec = 0;

#if defined(_WINDOWS)
    int result = setsockopt((SOCKET)mSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tm, sizeof(timeval));
#else
    int result = setsockopt(        mSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tm, sizeof(timeval));
#endif

    return result;
#endif
}

/*!
 * \brief   Nagleアルゴリズム設定
 *
 * \param[in]   noDelay 遅延させない場合はtrue
 *
 * \return  
 */
int Socket::setNoDelay(bool noDelay)
{
#if defined(MODERN_UI)
    return 0;
#else
    int on = (noDelay ? 1 : 0);

#if defined(_WINDOWS)
    int result = setsockopt((SOCKET)mSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on));
#else
    int result = setsockopt(        mSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on));
#endif

    return result;
#endif
}

/*!
 * \brief   接続元／先ホスト名取得
 *
 * \param[out]  hostName    接続元／先ホスト名を返す
 *
 * \return  なし
 */
void Socket::getHostName(slog::CoreString* hostName) const
{
    in_addr addr;
    addr.s_addr = inet_addr(getInetAddress()->getBuffer());

    hostent* host = gethostbyaddr((const char*)&addr.s_addr, sizeof(addr.s_addr), AF_INET);

    if (host != nullptr)
        hostName->copy(host->h_name);
}

/*!
 * \brief   接続元／先IPアドレス取得
 *
 * \return  接続元／先IPアドレス
 */
const CoreString* Socket::getInetAddress() const
{
    CoreString* inetAddress = &mData->mInetAddress;

#if defined(MODERN_UI)
#else
    if (isOpen() == false)
        inetAddress->setLength(0);
    else
        inetAddress->copy(inet_ntoa(mData->mAddr.sin_addr));
#endif

    return inetAddress;
}

/*!
 * \brief   自IPアドレス取得
 *
 * \return  自IPアドレス
 */
const CoreString* Socket::getMyInetAddress() const
{
    CoreString* inetAddress = &mData->mMyInetAddress;
    sockaddr_in addr;

#if defined(_WINDOWS)
    int       len = sizeof(addr);
    getsockname((SOCKET)mSocket, (sockaddr*)&addr, &len);
#else
    socklen_t len = sizeof(addr);
    getsockname(        mSocket, (sockaddr*)&addr, &len);
#endif

    if (isOpen() == false)
        inetAddress->setLength(0);
    else
        inetAddress->copy(inet_ntoa(addr.sin_addr));

    return inetAddress;
}

/*!
 * \brief   送信
 */
void Socket::send(const int32_t* value) const throw(Exception)
{
    Socket::send((uint32_t*)value);
}

/*!
 * \brief   送信
 */
void Socket::send(const uint32_t* value) const throw(Exception)
{
    mBuffer->setPosition(0);
    mBuffer->putInt(*value);

    Socket::send(mBuffer, sizeof(*value));
}

/*!
 * \brief   送信
 */
void Socket::send(
    const Buffer* buffer,   //!< 送信バッファ
    int32_t len)            //!< 送信サイズ

    const
    throw(Exception)
{
    buffer->validateOverFlow(0, len);
    Socket::send(buffer->getBuffer(), len);
}

/*!
 * \brief   送信
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

        if (mData->mSSL)
        {
            result = SSL_write(mData->mSSL, p, remains);
        }
        else
        {
            sockaddr_in* addr = &mData->mAddr;

            result = (mStream
#if defined(_WINDOWS)
                ? ::send(  (SOCKET)mSocket, p, remains, flag)
                : ::sendto((SOCKET)mSocket, p, remains, flag, (sockaddr*)addr, sizeof(*addr))
#else
                ? ::send(          mSocket, p, remains, flag)
                : ::sendto(        mSocket, p, remains, flag, (sockaddr*)addr, sizeof(*addr))
#endif
            );
        }

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
 * \brief   受信
 */
void Socket::recv(
    int32_t* value)         //!< 受信データ

    const
    throw(Exception)
{
    recv((uint32_t*)value);
}

/*!
 * \brief   受信
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
 * \brief   受信
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
        if (mData->mSSL)
        {
            result = SSL_read(mData->mSSL, p, remains);
        }
        else
        {
#if defined(_WINDOWS)
            result = ::recv((SOCKET)mSocket, p, remains, 0);
#else
            result = ::recv(        mSocket, p, remains, 0);
#endif
        }

        if (result <= 0)
            break;

        p += result;
        remains -= result;

        loopCount++;
    }
#endif

    if (result == 0)
    {
        // 接続先から切断された
        Exception e;
        e.setMessage("Socket::recv(%d) retval 0", len);

        throw e;
    }

    if (result < 0)
    {
        // エラー発生
#if defined(_WINDOWS)
        if (GetLastError() == WSAETIMEDOUT)
#else
        if (errno == ETIMEDOUT)
#endif
        {
            // タイムアウト後にclose()すると落ちるので、その対応
            ((Socket*)this)->mSocket = -1;
            ((Socket*)this)->mConnect = false;
        }

        Exception e;
        e.setMessage("Socket::recv(%d)", len);

        throw e;
    }

    buffer->setLength(len);
}

/*!
 * \brief   受信
 *
 * \param[out]  buffer  受信バッファ
 *
 * \return  なし
 */
void Socket::recv(CoreString* buffer)

    const
    throw(Exception)
{
    int32_t size = 1;
    ByteBuffer recvBuffer(size);

    char* p =buffer->getBuffer();
    int32_t capacity = buffer->getCapacity();
    int32_t i = 0;

    while (true)
    {
        // 受信
        recvBuffer.setLength(0);
        recv(&recvBuffer, size);

        // 改行までリクエストバッファに貯める
        char c = recvBuffer.get();
//      noticeLog("%d: %c(%02X)", i, c, (uint8_t)c);

        if (c == '\r')
            break;

        if (capacity <= i)
        {
            capacity += 256;
            buffer->setLength(i);
            buffer->setCapacity(capacity);
            p = buffer->getBuffer();
        }

        p[i] = c;
        i++;
    }

    buffer->setLength(i);
//  noticeLog("%s", buffer->getBuffer());

    // '\n'捨て
    recvBuffer.setLength(0);
    recv(&recvBuffer, size);

//  char c = recvBuffer.get();
//  noticeLog("%d: %c(%02X)", i + 1, c, (uint8_t)c);
}

/*!
 * \brief   受信データが有るかどうか
 */
bool Socket::isReceiveData(int32_t timeoutMS)

    const
    throw(Exception)
{
    Exception e;

    if (mSocket == -1)
    {
        e.setMessage("Socket::isReceiveData()");
        throw e;
    }

    timeval timeout =
    {
         timeoutMS / 1000,
        (timeoutMS % 1000) * 1000
    };
    fd_set fds;

    FD_ZERO(&fds);
#if defined(_WINDOWS)
    FD_SET((SOCKET)mSocket, &fds);
#else
    FD_SET(        mSocket, &fds);
#endif

    int n = select((int)mSocket + 1, &fds, nullptr, nullptr, &timeout);

    if (n < 0)
    {
        e.setMessage("Socket::isReceiveData()");
        throw e;
    }

    return (0 < n);
}

/*!
 * \brief   スタートアップ
 */
void Socket::startup()
{
#if defined(_WINDOWS) && !defined(MODERN_UI)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,0), &wsaData);
#endif

    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_crypto_strings();
}

/*!
 * \brief   クリーンアップ
 */
void Socket::cleanup()
{
#if defined(_WINDOWS) && !defined(MODERN_UI)
    WSACleanup();
#endif
}

/*!
 * \brief   ドメインが存在するかどうか
 */
bool Socket::existsDomain(const char* domain)
{
    hostent* host = gethostbyname(domain);
    return (host != nullptr);
}

} // namespace slog
