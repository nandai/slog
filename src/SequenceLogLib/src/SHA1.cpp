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
#include "slog/SHA1.h"
#include "slog/CoreString.h"

#if defined(_WINDOWS)
    #include <windows.h>
    #define SHA_DIGEST_LENGTH 20
#else
    #include <openssl/sha.h>
#endif

namespace slog
{

class SHA1::Data
{
            /*!
             * メッセージダイジェスト
             */
public:     uint8_t mMessageDigest[SHA_DIGEST_LENGTH];
};

/*!
 * \brief   コンストラクタ
 */
SHA1::SHA1()
{
    mData = new Data;
}

/*!
 * デストラクタ
 */
SHA1::~SHA1()
{
    delete mData;
}

/*!
 * \brief   ハッシュ計算実行
 */
void SHA1::execute(const CoreString* message)
{
    execute(message->getBuffer(), message->getLength());
}

/*!
 * \brief   ハッシュ計算実行
 */
void SHA1::execute(const char* message, int32_t size)
{
#if defined(_WINDOWS)
    HCRYPTPROV hProv = NULL;
    HCRYPTHASH hHash = NULL;

    do
    {
        if (CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)
            break;

        if (CryptCreateHash(hProv, CALG_SHA, 0, 0, &hHash) == FALSE)
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
    SHA_CTX context;

    SHA1_Init(  &context);
    SHA1_Update(&context, message, size);
    SHA1_Final(mData->mMessageDigest, &context);
#endif
}

/*!
 * メッセージダイジェスト取得
 */
const uint8_t* SHA1::getMessageDigest() const
{
    return mData->mMessageDigest;
}

/*!
 * ハッシュサイズ取得
 */
int32_t SHA1::getHashSize() const
{
    return SHA_DIGEST_LENGTH;
}

} // namespace slog
