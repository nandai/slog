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
 *  \file   Util.cpp
 *  \brief  ユーティリティクラス
 *  \author Copyright 2011-2015 printf.jp
 */
#include "slog/Util.h"
#include "slog/String.h"
#include "slog/Convert.h"
#include "slog/DateTime.h"

#if defined(_WINDOWS)
    #include <windows.h>
#endif

#if defined(__unix__) || defined(__APPLE__)
    #include <string.h>
    #include <limits.h>
    #include <unistd.h>
    #include <netdb.h>

    #if !defined(__ANDROID__)
        #include <iconv.h>
    #endif

    #if defined(__APPLE__)
        #include <libproc.h>
    #endif
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

#elif defined(__unix__)
    String linkPath;
    linkPath.format("/proc/%d/exe", getpid());

    char buffer[MAX_PATH];
    int32_t len = readlink(linkPath.getBuffer(), buffer, sizeof(buffer));
    buffer[len] = '\0';

#elif defined(__APPLE__)
    char buffer[PROC_PIDPATHINFO_MAXSIZE];
    proc_pidpath(getpid(), buffer, sizeof(buffer));
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
    static const char* table = "=ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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
 * \brief   Base64デコード
 *
 * \param[out]  dest    変換後の文字列を返す
 * \param[in]   src     変換前の文字列
 *
 * \return  なし
 */
void Util::decodeBase64(CoreString* dest, const char* src)
{
    static const char* table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    char b64[128];
    int32_t len = String::GetLength(src);
    int32_t i;

    if (len % 4)
        return;

    dest->setCapacity(len / 4 * 3);
    char* buffer = dest->getBuffer();

    for (i = 0; i < 65; i++)
    {
        char c = table[i];
        b64[c] = i % 64;
    }

    while (*src)
    {
        char c[4];

        for (i = 0; i < 4; i++)
        {
            c[i] = b64[*src];
            src++;
        }

        for (i = 0; i < 3; i++)
        {
            *buffer =
                (c[i + 0] << ( i * 2  + 2)) |
                (c[i + 1] >> ((2 - i) * 2));

            buffer++;
        }
    }
}

/*!
 * \brief   パーセントデコード
 *
 * \param [in,out]  str     デコード結果を返す
 *
 * \return  なし
 */
void Util::decodePercent(CoreString* str)
{
    char* p = str->getBuffer();
    decodePercent(nullptr, p, p + str->getLength());
    str->setLength(String::GetLength(p));
}

/*!
 * \brief   パーセントデコード
 *
 * \param [out]     str     デコード結果を返す（null可）
 * \param [in,out]  start   デコード開始位置（デコード処理により書き換わる）
 * \param [in]      end     デコード終了位置
 *
 * \return  なし
 */
void Util::decodePercent(CoreString* str, char* start, const char* end)
{
    const char* cursor = start;
    char* decodeCursor = start;

    while (cursor < end)
    {
        char c = *cursor;

        switch (c)
        {
        case '%':
        {
            c = Convert::toByte(cursor + 1, 16, 2);
            cursor += (1 + 2);
            break;
        }

        case '+':
            c =  ' ';
//          break;

        default:
            cursor++;
            break;
        }

        *decodeCursor = c;
         decodeCursor++;
    }

    if (str)
    {
        str->copy(start, (int32_t)(decodeCursor - start));
    }
    else
    {
        *decodeCursor = '\0';
    }
}

/*!
 * \brief   HTMLエンコード
 */
void Util::encodeHtml(CoreString* str)
{
    static struct
    {
        char        target;
        const char* encode;
        int32_t     len;
    }
    list[] =
    {
        {'"',  "&quot;", 6},
        {'&',  "&amp;",  5},
        {'<',  "&lt;",   4},
        {'>',  "&gt;",   4},
        {'\n', "<br />", 6},
    };

    int32_t len = str->getLength();
    char* buffer = new char[len * 6 + 1];
    char* dest = buffer;
    const char* src = str->getBuffer();

    for (int32_t i = 0; i < len; i++)
    {
        char c = *src;
        int32_t j;

        for (j = 0; j < sizeof(list) / sizeof(list[0]); j++)
        {
            if (c == list[j].target)
            {
                memcpy(dest, list[j].encode, list[j].len);
                dest += list[j].len;
                break;
            }
        }

        if (j == sizeof(list) / sizeof(list[0]))
        {
            *dest = c;
             dest++;
        }

        src++;
    }

    *dest = '\0';

    str->copy(buffer);
    delete [] buffer;
}

/*!
 * \brief   日時を文字列で取得する
 *
 * 例：2015/05/15 11:46:00 → "Fri, 15 May 2015 11:46:00 GMT"
 *
 * \param[out]  str         結果を受け取るバッファ
 * \param[in]   dateTime    日時
 *
 * \return  なし
 */
void Util::getDateString(CoreString* str, const DateTime* dateTime)
{
    static const char* week[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char* mon[] =  {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    str->format("%s, %02d %s %04d %02d:%02d:%02d GMT",
        week[dateTime->getWeekDay()],
             dateTime->getDay(),
        mon[ dateTime->getMonth() - 1],
             dateTime->getYear(),
             dateTime->getHour(),
             dateTime->getMinute(),
             dateTime->getSecond());
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
    int32_t domainPos = mailAddress->indexOf('@');

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

#if !defined(__ANDROID__)
/*!
 * \brief   SJISをUTF-8に変換
 */
void Util::shiftJIStoUTF8(CoreString* str, const char* sjis)
{
#if defined(_WINDOWS)
    // 一旦Unicodeに変換
    int32_t chars;
    chars = MultiByteToWideChar(CP_ACP, 0, sjis, -1, nullptr, 0) - 1;

    wchar_t* unicode = new wchar_t[chars + 1];
    chars = MultiByteToWideChar(CP_ACP, 0, sjis, -1, unicode, chars + 1);

    // 改めてUTF-8に変換
    str->conv(unicode);
    delete [] unicode;
#else
    size_t srcLen = String::GetLength(sjis) + 1;
    size_t dstLen = srcLen * 6;

    str->setCapacity(dstLen);
    char* dst = str->getBuffer();

    iconv_t icd = iconv_open("UTF-8", "Shift_JIS");
    iconv(icd, (char**)&sjis, &srcLen, &dst, &dstLen);
    iconv_close(icd);
#endif
}

/*!
 * UTF-8をSJISに変換
 */
void Util::UTF8toShiftJIS(CoreString* str, const char* utf8)
{
#if defined(_WINDOWS)
#else
    size_t srcLen = String::GetLength(utf8) + 1;
    size_t dstLen = srcLen;

    str->setCapacity(dstLen);
    char* dst = str->getBuffer();

    iconv_t icd = iconv_open("Shift_JIS", "UTF-8");
    iconv(icd, (char**)&utf8, &srcLen, &dst, &dstLen);
    iconv_close(icd);
#endif
}
#endif

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
