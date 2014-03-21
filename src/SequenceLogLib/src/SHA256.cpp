/*
 * Copyright (C) 2014 printf.jp
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
 *  \file   SHA256.cpp
 *  \brief  SHA256クラス
 *  \author Copyright 2014 printf.jp
 */
#include "slog/SHA256.h"
#include "slog/Buffer.h"

#if defined(_WINDOWS)
    #include <windows.h>
    #define SHA256_DIGEST_LENGTH 32
#else
    #include <openssl/sha.h>
#endif

namespace slog
{

class SHA256::Data
{
            /*!
             * メッセージダイジェスト
             */
public:     uint8_t mMessageDigest[SHA256_DIGEST_LENGTH];
};

/*!
 * \brief   コンストラクタ
 */
SHA256::SHA256()
{
    mData = new Data;
}

/*!
 * デストラクタ
 */
SHA256::~SHA256()
{
    delete mData;
}

/*!
 * \brief   ハッシュ計算実行
 */
void SHA256::execute(const Buffer* message)
{
    execute(message->getBuffer(), message->getLength());
}

/*!
 * \brief   ハッシュ計算実行
 */
void SHA256::execute(const char* message, int32_t size)
{
#if defined(_WINDOWS)
    HCRYPTPROV hProv = NULL;
    HCRYPTHASH hHash = NULL;

    do
    {
        if (CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT) == FALSE)
            break;

        if (CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash) == FALSE)
            break;

        if (CryptHashData(hHash, (uint8_t*)message, size, 0) == FALSE)
            break;

        int32_t hashSize = getHashSize();
        CryptGetHashParam(hHash, HP_HASHVAL, mData->mMessageDigest, (DWORD*)&hashSize, 0);
    }
    while (false);

    if (hHash)
        CryptDestroyHash(hHash);

    if (hProv)
        CryptReleaseContext(hProv, 0);
#else
    SHA256_CTX context;

    SHA256_Init(  &context);
    SHA256_Update(&context, message, size);
    SHA256_Final(mData->mMessageDigest, &context);
#endif
}

/*!
 * メッセージダイジェスト取得
 */
const uint8_t* SHA256::getMessageDigest() const
{
    return mData->mMessageDigest;
}

/*!
 * ハッシュサイズ取得
 */
int32_t SHA256::getHashSize() const
{
    return SHA256_DIGEST_LENGTH;
}

} // namespace slog
