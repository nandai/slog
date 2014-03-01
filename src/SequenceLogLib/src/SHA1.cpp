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

#include <openssl/sha.h>

namespace slog
{

class SHA1::Data
{
            /*!
             * SHA1コンテキスト
             */
public:     SHA_CTX mContext;

            /*!
             * メッセージダイジェスト
             */
            uint8_t mMessageDigest[SHA_DIGEST_LENGTH];
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
    SHA1_Init(  &mData->mContext);
    SHA1_Update(&mData->mContext, message->getBuffer(), message->getLength());
    SHA1_Final(  mData->mMessageDigest, &mData->mContext);
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
