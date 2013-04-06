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
#include "slog/Thread.h"
#include "slog/Socket.h"
#include "slog/FixedString.h"

namespace slog
{

/*!
 *  \brief  スレッドを終了させる
 */
//void Util::stopThread(Thread* thread, uint16_t port)
//{
//    if (thread->isAlive() == false)
//        return;
//
//    thread->interrupt();
//
//    // 接続
//    Socket sock;
//    sock.open();
//    sock.connect(FixedString<16>("127.0.0.1"), port);
//
//    // スレッド終了待ち
//    thread->join();
//
//    // 切断
//    sock.close();
//}

} // namespace slog
