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
 *  \file   main.cpp
 *  \brief  シーケンスログプリント
 *  \author Copyright 2011 log-tools.net
 */
#include "slog/Socket.h"
#include "slog/FixedString.h"

#include <stdio.h>

using namespace slog;

#if !defined(_WINDOWS)
    #define WSAEADDRINUSE   EADDRINUSE
#endif

/*!
 *  \brief  メイン
 */
int main()
{
#if defined(_WINDOWS)
    SetConsoleTitleA("Sequence Log Print");

    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    WORD wAttributes;

    GetConsoleScreenBufferInfo(hStdout, &csbi);
#endif

    Socket::startup();
    Socket sock;

    try
    {
        sock.open(true, SOCK_DGRAM);
        sock.setReUseAddress(true);
        sock.bind(59108);
    }
    catch (Exception e)
    {
        int32_t errorNo = e.getErrorNo();

        if (errorNo == WSAEADDRINUSE)
            printf("port in use.\n");
        else
            printf("unknown error (0x%08X).\n", (unsigned int)errorNo);

        return 1;
    }

    printf("--- Sequence Log Print started. ---\n");

//  while (true)
    {
        while (true)
        {
            static const int32_t len = 768;
//          FixedString<1024> buffer;
            FixedString<len>  buffer;

            try
            {
//          sock.recv(&len);
            sock.recv(&buffer, len);

            switch (buffer[0])
            {
            case 'd':
#if defined(_WINDOWS)
                wAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
#else
                printf("\x1B[32;49;0m");
#endif
                break;

            case 'i':
#if defined(_WINDOWS)
                wAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
#else
                printf("\x1B[37;49;1m");
#endif
                break;

            case 'w':
#if defined(_WINDOWS)
                wAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
#else
                printf("\x1B[33;49;1m");
#endif
                break;

            case 'e':
#if defined(_WINDOWS)
                wAttributes = FOREGROUND_RED | FOREGROUND_INTENSITY;
#else
                printf("\x1B[31;49;1m");
#endif
                break;

            default:
#if defined(_WINDOWS)
                wAttributes = FOREGROUND_BLUE;
#else
#endif
                break;
            }
            }
            catch (Exception e)
            {
                printf("%s\n", e.getMessage());
//              break;
            }

#if defined(_WINDOWS)
            SetConsoleTextAttribute(hStdout, wAttributes);
#endif

            buffer.setLength(len);
            printf("%s", buffer.getBuffer() + 1);
        }

        printf("\n");
    }

#if defined(_WINDOWS)
    SetConsoleTextAttribute(hStdout, csbi.wAttributes);
#else
    printf("\x1B[39;49;0m");
#endif

    Socket::cleanup();
    return 0;
}

//extern "C" const char* getSequenceLogFileName()
//{
//    return NULL;
//}
