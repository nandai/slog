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
 *  \file   stdint.h
 *  \brief  C99未対応（VS2010以前等）のコンパイラ用定義
 *  \author Copyright 2011 log-tools.net
 */
#pragma once

typedef signed char           int8_t;
typedef unsigned char        uint8_t;

typedef short                int16_t;
typedef unsigned short      uint16_t;

typedef int                  int32_t;
typedef unsigned int        uint32_t;

typedef long long            int64_t;
typedef unsigned long long  uint64_t;
