/*******************************************************************************
 * Copyright 2019 FUJITSU LIMITED
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/
#pragma once
#ifndef _XBYAK_AARCH64_
#define _XBYAK_AARCH64_

#include <algorithm>
#include <deque>
#include <initializer_list>
#include <iostream>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//#define XBYAK_USE_MMAP_ALLOCATOR
#if !defined(__GNUC__) || defined(__MINGW32__)
#undef XBYAK_USE_MMAP_ALLOCATOR
#endif

#include <functional>

#include <cmath>

#ifdef _WIN32
#include <assert.h>
#include <malloc.h>
#include <windows.h>
//#include <winsock2.h>
#elif defined(__GNUC__)
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cassert>
#endif

#include <iomanip>
#include <sstream>
#ifndef NDEBUG
#include <iostream>
#endif

namespace Xbyak {

#ifndef MIE_INTEGER_TYPE_DEFINED
#define MIE_INTEGER_TYPE_DEFINED
#ifdef _MSC_VER
typedef unsigned __int64 uint64;
typedef __int64 sint64;
#else
typedef uint64_t uint64;
typedef int64_t sint64;
#endif
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
#endif

#include "xbyak_aarch64_gen.h"
}

#endif
