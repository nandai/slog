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
 *  \file   Util.h
 *  \brief  ���[�e�B���e�B�N���X
 *  \author Copyright 2011-2013 printf.jp
 */
#pragma once
#include "slog/slog.h"

namespace slog
{
class Thread;

/*!
 *  \brief  ���[�e�B���e�B�N���X
 */
class SLOG_API Util
{
public:     static void stopThread(Thread* thread, uint16_t port);
};

} // namespace slog