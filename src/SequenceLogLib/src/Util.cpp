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
 *  \file   Util.cpp
 *  \brief  ユーティリティクラス
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/Util.h"
#include "slog/String.h"

#if defined(__linux__)
#include <unistd.h>
#endif

namespace slog
{

/*!
 *  \brief  プロセスの実行ファイルパスを取得
 */
void Util::getProcessPath(String* path)
{
#if defined(_WINDOWS)
    wchar_t fullName[MAX_PATH];
    GetModuleFileNameW(NULL, fullName, sizeof(fullName));

    String str;
    str.conv(fullName);

    char* buffer = str.getBuffer();
#else
    String linkPath;
    linkPath.format("/proc/%d/exe", getpid());

    char buffer[MAX_PATH];
    readlink(linkPath.getBuffer(), buffer, sizeof(buffer));
#endif

    char* fileName = strrchr(buffer, PATH_DELIMITER);
    path->copy(buffer, (int32_t)(fileName - buffer));
}

/*!
 *  \brief  ビット指定で値を取得
 */
int64_t Util::getBitsValue(const char* p, int32_t len, int32_t bitPos, int32_t count)
{
    int32_t pos =       bitPos / CHAR_BIT;
    int32_t charInPos = bitPos % CHAR_BIT;

    if (len <= pos)
        return -1;

    unsigned char c = (p[pos] << charInPos);
    int64_t res = 0;

    for (int32_t i = 0; i < count; i++)
    {
        res = (res << 1) | (c & 0x80 ? 0x01 : 0x00);
        c <<= 1;
        charInPos++;

        if (charInPos == CHAR_BIT)
        {
            pos++;

            if (pos < len)
            {
                charInPos = 0;
                c = p[pos];
            }
        }
    }

    return res;
}

/*!
 *  \brief  Base64エンコード
 */
void Util::encodeBase64(String* dest, const char* src, int32_t srcLen)
{
    const char* table = "=ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int32_t destLen = (srcLen *  4 / 3 + 3) / 4 * 4;
    char* buffer = new char[destLen];
    int32_t bitPos = 0;

    for (int32_t i = 0; i < destLen; i++)
    {
        int64_t value = getBitsValue(src, srcLen, bitPos, 6);
        buffer[i] = table[value + 1];
        bitPos += 6;
    }

    dest->copy(buffer, destLen);
    delete [] buffer;
}

} // namespace slog
