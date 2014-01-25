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
 *  \file   Util.cpp
 *  \brief  ユーティリティクラス
 *  \author Copyright 2011-2014 printf.jp
 */
#include "slog/Util.h"
#include "slog/String.h"

#if defined(_WINDOWS)
    #include <windows.h>
#endif

#if defined(__linux__)
    #include <string.h>
    #include <limits.h>
    #include <unistd.h>
    #include <netdb.h>
#endif

namespace slog
{

/*!
 * \brief   プロセスの実行ファイルパスを取得
 *
 * \param[out]  path    プロセスの実行ファイルパスを返す
 *
 * \return  なし
 */
void Util::getProcessPath(CoreString* path)
{
#if defined(_WINDOWS)
    wchar_t fullName[MAX_PATH];
    GetModuleFileNameW(nullptr, fullName, sizeof(fullName));

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
 * \brief   ビット指定で値を取得
 *
 * \param[in]   p       バッファアドレス
 * \param[in]   len     バッファの長さ
 * \param[in]   bitPos  取得開始位置
 * \param[in]   count   取得ビット数
 *
 * \return  指定範囲の値を返す
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
 * \brief   Base64エンコード
 *
 * \param[out]  dest    変換後の文字列を返す
 * \param[in]   src     変換前の文字列
 * \param[in]   srcLen  変換前の文字列の長さ
 *
 * \return  なし
 */
void Util::encodeBase64(CoreString* dest, const char* src, int32_t srcLen)
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

/*!
 * \brief   メールアドレスを検証
 *
 * \param[in]   mailAddress メールアドレス
 *
 * \return  正しいメールアドレスならtrue、正しくなければfalseを返す
 */
bool Util::validateMailAddress(const CoreString* mailAddress)
{
    // メールアドレスの長さをチェック
    if (256 < mailAddress->getLength())
        return false;

    // 宛先ユーザー名の長さをチェック
    int32_t domainPos = mailAddress->indexOf("@");

    if (domainPos <= 0)
        return false;

    // ドメイン名の長さをチェック
    const char* domain = mailAddress->getBuffer() + domainPos + 1;

    if (*domain == '\0')
        return false;

    // ドメインの存在確認
    hostent* host = gethostbyname(domain);

    if (host == nullptr)
        return false;

    // メールアドレスに問題なし
    return true;
}

#if defined(_WINDOWS)
/*!
 * \brief   UnicodeをUTF-8に変換
 */
int32_t Util::toUTF8(char* utf8, int32_t size, const wchar_t* unicode)
{
    int32_t len = (int32_t)wcslen(unicode);

    int32_t bytes = WideCharToMultiByte(CP_UTF8, 0, unicode, len + 1, utf8, size, nullptr, nullptr);
    return (bytes - 1/* 末尾の'\0'分を引く */);
}

/*!
 * \brief   UTF-8をUnicodeに変換
 */
int32_t Util::toUnicode(wchar_t* unicode, int32_t size, const char* utf8)
{
    int32_t chars = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, unicode, size);
    return (chars - 1/* 末尾の'\0'分を引く */);
}
#endif

} // namespace slog
