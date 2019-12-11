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
#include <stdio.h>
#define XBYAK_NO_OP_NAMES
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <sstream>
#include "xbyak_aarch64.h"

#define NUM_OF_ARRAY(x) (sizeof(x) / sizeof(x[0]))

using namespace Xbyak;

const int bitEnd = 64;
/** Begin:Really used in this file. */
uint64_t flagBit = 0;
const uint64_t WREG  = 1ULL << flagBit++; /** Test vector is {w0, w1, ..., w30 } */
const uint64_t XREG  = 1ULL << flagBit++; /** Test vector is {x0, x1, ..., x30 } */
const uint64_t VREG  = 1ULL << flagBit++; /** Test vector is {v0, v1, ..., v31 } */
const uint64_t IMM0BIT   = 1ULL << flagBit++; /** Test vector is {0} */
const uint64_t IMM1BIT   = 1ULL << flagBit++; /** Test vector is {0, 1} */
const uint64_t IMM2BIT   = 1ULL << flagBit++; /** Test vector is {0, 1, 2, 3 } */
const uint64_t IMM3BIT   = 1ULL << flagBit++; /** Test vector is {0, 1, 2, 4, 7 } */
const uint64_t IMM4BIT   = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 8, 15 } */
const uint64_t IMM5BIT   = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 16, 31 } */
const uint64_t IMM6BIT   = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 32, 63 } */
const uint64_t IMM8BIT   = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 128, 255 } */
const uint64_t IMM12BIT  = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 2048, 4095 } */
const uint64_t IMM13BIT  = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 4096, 8191 } */
const uint64_t IMM16BIT  = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 4096, 1<<13, 1<<14, 1<<15, 1<<16-1 } */
const uint64_t IMM3BIT_N = 1ULL << flagBit++; /** Test vector is {1, 2, .., 8 } */
const uint64_t IMM4BIT_N = 1ULL << flagBit++; /** Test vector is {1, 2, .., 16 } */
const uint64_t IMM5BIT_N = 1ULL << flagBit++; /** Test vector is {1, 2, .., 32 } */
const uint64_t IMM6BIT_N = 1ULL << flagBit++; /** Test vector is {1, 2, .., 64 } */
const uint64_t FLOAT8BIT = 1ULL << flagBit++; /** Test vector is Table C-2- Floating-point constant values */

const uint64_t BREG = 1ULL << flagBit++; /** Test vector is { b0, b1, ..., b31 } */
const uint64_t HREG = 1ULL << flagBit++; /** Test vector is { h0, h1, ..., h31 } */
const uint64_t SREG = 1ULL << flagBit++; /** Test vector is { s0, s1, ..., s31 } */
const uint64_t DREG = 1ULL << flagBit++; /** Test vector is { d0, d1, ..., d31 } */
const uint64_t QREG = 1ULL << flagBit++; /** Test vector is { q0, q1, ..., q31 } */

const uint64_t BITMASK32  = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 2048, 4095 } */
const uint64_t BITMASK64  = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 4096, 8191 } */
const uint64_t LSL_IMM    = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t LSL32 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t LSL64 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_1 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_1 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_2 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_2 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_3 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_3 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SHIFT_AMOUNT32    = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SHIFT_AMOUNT64    = 1ULL << flagBit++; /** Test vector is generated on the fly. */

const uint64_t VREG_8B  = 1ULL << flagBit++; /** Test vector is { v0.8b, v1.8b, ..., v31.8b } */
const uint64_t VREG_16B = 1ULL << flagBit++;  /** Test vector is { v0.16b, v1.16b, ..., v31.16b } */
const uint64_t VREG_2H  = 1ULL << flagBit++;  /** Test vector is { v0.2h, v1.2h, ..., v31.2h } */
const uint64_t VREG_4H  = 1ULL << flagBit++;  /** Test vector is { v0.b, v1.b, ..., v31.b } */
const uint64_t VREG_8H  = 1ULL << flagBit++;  /** Test vector is { v0.b, v1.b, ..., v31.b } */
const uint64_t VREG_2S  = 1ULL << flagBit++;  /** Test vector is { v0.b, v1.b, ..., v31.b } */
const uint64_t VREG_4S  = 1ULL << flagBit++;  /** Test vector is { v0.b, v1.b, ..., v31.b } */
const uint64_t VREG_1D  = 1ULL << flagBit++;  /** Test vector is { v0.b, v1.b, ..., v31.b } */
const uint64_t VREG_2D  = 1ULL << flagBit++;  /** Test vector is { v0.b, v1.b, ..., v31.b } */
const uint64_t VREG_1Q  = 1ULL << flagBit++;  /** Test vector is { v0.1q, v1.1q, ..., v31.1q } */

const uint64_t VREG_8B_ELEM  = 1ULL << flagBit++;
const uint64_t VREG_16B_ELEM = 1ULL << flagBit++;
const uint64_t VREG_4H_ELEM = 1ULL << flagBit++;
const uint64_t VREG_8H_ELEM  = 1ULL << flagBit++;
const uint64_t VREG_2S_ELEM  = 1ULL << flagBit++;
const uint64_t VREG_4S_ELEM  = 1ULL << flagBit++;
const uint64_t VREG_1D_ELEM  = 1ULL << flagBit++;
const uint64_t VREG_2D_ELEM  = 1ULL << flagBit++;

const uint64_t VREG_B_ELEM = VREG_16B_ELEM;
const uint64_t VREG_H_ELEM = VREG_8H_ELEM;
const uint64_t VREG_S_ELEM = VREG_4S_ELEM;
const uint64_t VREG_D_ELEM = VREG_2D_ELEM;

const uint64_t NOPARA = 1ULL << (bitEnd - 1);


  
#define PUT1(name, nm, op_1)			\
  void put##name() const			\
  {						\
    std::vector<std::string> nemonic(nm);	\
    std::vector<uint64_t> op1(op_1);		\
    put(nemonic, op1, #name, 0);			\
  }						\

#define PUT2(name, nm, op_1, op_2)		\
  void put##name() const			\
  {						\
    std::vector<std::string> nemonic(nm);	\
    std::vector<uint64_t> op1(op_1);		\
    std::vector<uint64_t> op2(op_2);		\
    put(nemonic, op1, #name, 0);			\
    put(nemonic, op2, #name, 1);				\
  }						\

#define PUT3(name, nm, op_1, op_2, op_3)	\
  void put##name() const			\
  {						\
    std::vector<std::string> nemonic(nm);	\
    std::vector<uint64_t> op1(op_1);		\
    std::vector<uint64_t> op2(op_2);		\
    std::vector<uint64_t> op3(op_3);		\
    put(nemonic, op1, #name, 0);			\
    put(nemonic, op2, #name, 1);				\
    put(nemonic, op3, #name, 2);					\
  }						\

#define PUT4(name, nm, op_1, op_2, op_3, op_4)	\
  void put##name() const			\
  {						\
    std::vector<std::string> nemonic(nm);	\
    std::vector<uint64_t> op1(op_1);		\
    std::vector<uint64_t> op2(op_2);		\
    std::vector<uint64_t> op3(op_3);		\
    std::vector<uint64_t> op4(op_4);		\
    put(nemonic, op1, #name, 0);					\
    put(nemonic, op2, #name, 1);					\
    put(nemonic, op3, #name, 2);					\
    put(nemonic, op4, #name, 3);					\
  }						\

#define PUT5(name, nm, op_1, op_2, op_3, op_4, op_5)	\
  void put##name() const				\
  {							\
    std::vector<std::string> nemonic(nm);		\
    std::vector<uint64_t> op1(op_1);			\
    std::vector<uint64_t> op2(op_2);			\
    std::vector<uint64_t> op3(op_3);			\
    std::vector<uint64_t> op4(op_4);			\
    std::vector<uint64_t> op5(op_5);			\
    put(nemonic, op1, #name, 0);				\
    put(nemonic, op2, #name, 1);					\
    put(nemonic, op3, #name, 2);						\
    put(nemonic, op4, #name, 3);						\
    put(nemonic, op5, #name, 4);						\
  }							\

#define PUT6(name, nm, op_1, op_2, op_3, op_4, op_5, op_6)	\
  void put##name() const					\
  {								\
    std::vector<std::string> nemonic(nm);			\
    std::vector<uint64_t> op1(op_1);				\
    std::vector<uint64_t> op2(op_2);				\
    std::vector<uint64_t> op3(op_3);				\
    std::vector<uint64_t> op4(op_4);				\
    std::vector<uint64_t> op5(op_5);				\
    std::vector<uint64_t> op6(op_6);				\
    put(nemonic, op1, #name, 0);					\
    put(nemonic, op2, #name, 1);					\
    put(nemonic, op3, #name, 2);					\
    put(nemonic, op4, #name, 3);					\
    put(nemonic, op5, #name, 4);					\
    put(nemonic, op6, #name, 5);						\
  }								\
  
#define PUT7(name, nm, op_1, op_2, op_3, op_4, op_5, op_6, op_7)	\
  void put##name() const						\
  {									\
    std::vector<std::string> nemonic(nm);				\
    std::vector<uint64_t> op1(op_1);					\
    std::vector<uint64_t> op2(op_2);					\
    std::vector<uint64_t> op3(op_3);					\
    std::vector<uint64_t> op4(op_4);					\
    std::vector<uint64_t> op5(op_5);					\
    std::vector<uint64_t> op6(op_6);					\
    std::vector<uint64_t> op7(op_7);					\
    put(nemonic, op1, #name, 0);						\
    put(nemonic, op2, #name, 1);						\
    put(nemonic, op3, #name, 2);						\
    put(nemonic, op4, #name, 3);						\
    put(nemonic, op5, #name, 4);						\
    put(nemonic, op6, #name, 5);						\
    put(nemonic, op7, #name, 6);						\
  }									\

#define PUT8(name, nm, op_1, op_2, op_3, op_4, op_5, op_6, op_7, op_8)	\
  void put##name() const						\
  {									\
    std::vector<std::string> nemonic(nm);				\
    std::vector<uint64_t> op1(op_1);					\
    std::vector<uint64_t> op2(op_2);					\
    std::vector<uint64_t> op3(op_3);					\
    std::vector<uint64_t> op4(op_4);					\
    std::vector<uint64_t> op5(op_5);					\
    std::vector<uint64_t> op6(op_6);					\
    std::vector<uint64_t> op7(op_7);					\
    std::vector<uint64_t> op8(op_8);					\
    put(nemonic, op1, #name, 0);						\
    put(nemonic, op2, #name, 1);						\
    put(nemonic, op3, #name, 2);						\
    put(nemonic, op4, #name, 3);						\
    put(nemonic, op5, #name, 4);						\
    put(nemonic, op6, #name, 5);						\
    put(nemonic, op7, #name, 6);						\
    put(nemonic, op8, #name, 7);						\
  }									\

#define PUT9(name, nm, op_1, op_2, op_3, op_4, op_5, op_6, op_7, op_8, op_9) \
  void put##name() const						\
  {									\
    std::vector<std::string> nemonic(nm);				\
    std::vector<uint64_t> op1(op_1);					\
    std::vector<uint64_t> op2(op_2);					\
    std::vector<uint64_t> op3(op_3);					\
    std::vector<uint64_t> op4(op_4);					\
    std::vector<uint64_t> op5(op_5);					\
    std::vector<uint64_t> op6(op_6);					\
    std::vector<uint64_t> op7(op_7);					\
    std::vector<uint64_t> op8(op_8);					\
    std::vector<uint64_t> op9(op_9);					\
    put(nemonic, op1, #name, 0);						\
    put(nemonic, op2, #name, 1);						\
    put(nemonic, op3, #name, 2);						\
    put(nemonic, op4, #name, 3);						\
    put(nemonic, op5, #name, 4);						\
    put(nemonic, op6, #name, 5);						\
    put(nemonic, op7, #name, 6);						\
    put(nemonic, op8, #name, 7);						\
    put(nemonic, op9, #name, 8);						\
  }									\

#define PUT10(name, nm, op_1, op_2, op_3, op_4, op_5, op_6, op_7, op_8, op_9, op_10) \
  void put##name() const						\
  {									\
    std::vector<std::string> nemonic(nm);				\
    std::vector<uint64_t> op1(op_1);					\
    std::vector<uint64_t> op2(op_2);					\
    std::vector<uint64_t> op3(op_3);					\
    std::vector<uint64_t> op4(op_4);					\
    std::vector<uint64_t> op5(op_5);					\
    std::vector<uint64_t> op6(op_6);					\
    std::vector<uint64_t> op7(op_7);					\
    std::vector<uint64_t> op8(op_8);					\
    std::vector<uint64_t> op9(op_9);					\
    std::vector<uint64_t> op10(op_10);					\
    put(nemonic, op1, #name, 0);						\
    put(nemonic, op2, #name, 1);						\
    put(nemonic, op3, #name, 2);						\
    put(nemonic, op4, #name, 3);						\
    put(nemonic, op5, #name, 4);						\
    put(nemonic, op6, #name, 5);						\
    put(nemonic, op7, #name, 6);						\
    put(nemonic, op8, #name, 7);						\
    put(nemonic, op9, #name, 8);						\
    put(nemonic, op10, #name, 9);						\
  }									\

#define PUT16(name, nm, op_1, op_2, op_3, op_4, op_5, op_6, op_7, op_8, op_9, op_10, op_11, op_12, op_13, op_14, op_15, op_16) \
  void put##name() const						\
  {									\
    std::vector<std::string> nemonic(nm);				\
    std::vector<uint64_t> op1(op_1);					\
    std::vector<uint64_t> op2(op_2);					\
    std::vector<uint64_t> op3(op_3);					\
    std::vector<uint64_t> op4(op_4);					\
    std::vector<uint64_t> op5(op_5);					\
    std::vector<uint64_t> op6(op_6);					\
    std::vector<uint64_t> op7(op_7);					\
    std::vector<uint64_t> op8(op_8);					\
    std::vector<uint64_t> op9(op_9);					\
    std::vector<uint64_t> op10(op_10);					\
    std::vector<uint64_t> op11(op_11);					\
    std::vector<uint64_t> op12(op_12);					\
    std::vector<uint64_t> op13(op_13);					\
    std::vector<uint64_t> op14(op_14);					\
    std::vector<uint64_t> op15(op_15);					\
    std::vector<uint64_t> op16(op_16);					\
    put(nemonic, op1, #name, 0);						\
    put(nemonic, op2, #name, 1);						\
    put(nemonic, op3, #name, 2);						\
    put(nemonic, op4, #name, 3);						\
    put(nemonic, op5, #name, 4);						\
    put(nemonic, op6, #name, 5);						\
    put(nemonic, op7, #name, 6);						\
    put(nemonic, op8, #name, 7);						\
    put(nemonic, op9, #name, 8);						\
    put(nemonic, op10, #name, 9);						\
    put(nemonic, op11, #name, 10);						\
    put(nemonic, op12, #name, 11);						\
    put(nemonic, op13, #name, 12);						\
    put(nemonic, op14, #name, 13);						\
    put(nemonic, op15, #name, 14);						\
    put(nemonic, op16, #name, 15);						\
  }									\


#define OPS( ... ) { __VA_ARGS__ }
#define NM( ... ) { __VA_ARGS__ }

class Test {
  Test(const Test&);
  void operator=(const Test&);
  const bool isXbyak_;
  int funcNum_;
  
  

  /** String used as test vector */
  std::vector<std::string> tv_WREG = { "w0", "w1", "w2", "w4", "w8", "w16", "w30"};
  std::vector<std::string> tv_XREG = { "x0", "x1", "x2", "x4", "x8", "x16", "x30"};
  std::vector<std::string> tv_VREG = { "v0", "v1", "v2", "v4", "v8", "v16", "v31"};
  std::vector<std::string> tv_IMM0BIT = { "0" };
  std::vector<std::string> tv_IMM1BIT = { "1", "0" };
  std::vector<std::string> tv_IMM2BIT = { "3", "0", "1", "2" };
  std::vector<std::string> tv_IMM3BIT = { "7", "0", "1", "2", "4" };
  std::vector<std::string> tv_IMM4BIT = { "7", "0", "1", "2", "4", "8", "15" };
  std::vector<std::string> tv_IMM5BIT = { "0x1e", "0", "1", "2", "4", "8", "16", "31" };
  std::vector<std::string> tv_IMM6BIT = { "0x39", "0", "1", "2", "4", "8", "16", "32", "63" };
  std::vector<std::string> tv_IMM8BIT = { "0x39", "0", "1", "2", "4", "8", "16", "32", "64", "128", "255" };
  std::vector<std::string> tv_IMM12BIT = { "0x2aa", "0", "1", "2", "4", "8", "16", "32", "64",
					   "128", "256", "512", "1024", "2048", "4095" };
  std::vector<std::string> tv_IMM13BIT = { "0x1999", "0", "1", "2", "4", "8", "16", "32", "64",
					   "128", "256", "512", "1024", "2048", "4096", "8191"};
  std::vector<std::string> tv_IMM16BIT = { "0xe38e", "0", "1", "2", "4", "8", "16", "32", "64",
					   "128", "256", "512", "1024", "2048", "4096", "8191",
					   "1<<14", "1<<15", "(1<<16)-1" };
  std::vector<std::string> tv_IMM3BIT_N = { "1", "2", "4", "8" };
  std::vector<std::string> tv_IMM4BIT_N = { "1", "2", "4", "8", "16" };
  std::vector<std::string> tv_IMM5BIT_N = { "1", "2", "4", "8", "16", "32" };
  std::vector<std::string> tv_IMM6BIT_N = { "1", "2", "4", "8", "16", "32", "64" };
  std::vector<std::string> tv_FLOAT8BIT = { "2.0", "4.0", "8.0", "16.0", "0.125", "0.25", "0.5", "1.0",
					    "2.125", "4.5", "9.5", "20.0", "0.1640625", "0.34375", "0.71875", "1.5",
					    "0.78125", "0.40625", "0.2109375", "28.0", "14.5", "7.5", "3.875",
					    "1.9375" };

  std::vector<std::string> tv_BREG = { "b7", "b0", "b1", "b2", "b4", "b8", "b16", "b31" };
  std::vector<std::string> tv_HREG = { "h7", "h0", "h1", "h2", "h4", "h8", "h16", "h31" };
  std::vector<std::string> tv_SREG = { "s7", "s0", "s1", "s2", "s4", "s8", "s16", "s31" };
  std::vector<std::string> tv_DREG = { "d7", "d0", "d1", "d2", "d4", "d8", "d16", "d31" };
  std::vector<std::string> tv_QREG = { "q7", "q0", "q1", "q2", "q4", "q8", "q16", "q31" };

  std::vector<std::string> tv_BITMASK32;
  std::vector<std::string> tv_BITMASK64;
  std::vector<std::string> tv_LSL_IMM, jtv_LSL_IMM;
  std::vector<std::string> tv_LSL32, jtv_LSL32;
  std::vector<std::string> tv_LSL64, jtv_LSL64;
  std::vector<std::string> tv_SPECIFIC32, tv_SPECIFIC64, tv_SPECIFIC32_1, tv_SPECIFIC64_1, tv_SPECIFIC32_2, tv_SPECIFIC64_2,
    tv_SPECIFIC32_3, tv_SPECIFIC64_3;
  std::vector<std::string> jtv_SPECIFIC32, jtv_SPECIFIC64, jtv_SPECIFIC32_1, jtv_SPECIFIC64_1, jtv_SPECIFIC32_2, jtv_SPECIFIC64_2,
    jtv_SPECIFIC32_3, jtv_SPECIFIC64_3;
  std::vector<std::string> tv_SHIFT_AMOUNT32, jtv_SHIFT_AMOUNT32;
  std::vector<std::string> tv_SHIFT_AMOUNT64, jtv_SHIFT_AMOUNT64;

  std::vector<std::string> tv_VREG_8B  = { "v7.8b", "v0.8b", "v1.8b", "v2.8b", "v4.8b", "v8.8b", "v16.8b", "v31.8b" };  
  std::vector<std::string> tv_VREG_16B = { "v7.16b", "v0.16b", "v1.16b", "v2.16b", "v4.16b", "v8.16b", "v16.16b", "v31.16b" };
  std::vector<std::string> tv_VREG_2H  = { "v7.2h", "v0.2h", "v1.2h", "v2.2h", "v4.2h", "v8.2h", "v16.2h", "v31.2h" };  
  std::vector<std::string> tv_VREG_4H  = { "v7.4h", "v0.4h", "v1.4h", "v2.4h", "v4.4h", "v8.4h", "v16.4h", "v31.4h" };  
  std::vector<std::string> tv_VREG_8H  = { "v7.8h", "v0.8h", "v1.8h", "v2.8h", "v4.8h", "v8.8h", "v16.8h", "v31.8h" };
  std::vector<std::string> tv_VREG_2S  = { "v7.2s", "v0.2s", "v1.2s", "v2.2s", "v4.2s", "v8.2s", "v16.2s", "v31.2s" };  
  std::vector<std::string> tv_VREG_4S  = { "v7.4s", "v0.4s", "v1.4s", "v2.4s", "v4.4s", "v8.4s", "v16.4s", "v31.4s" };  
  std::vector<std::string> tv_VREG_1D  = { "v7.1d", "v0.1d", "v1.1d", "v2.1d", "v4.1d", "v8.1d", "v16.1d", "v31.1d" };  
  std::vector<std::string> tv_VREG_2D  = { "v7.2d", "v0.2d", "v1.2d", "v2.2d", "v4.2d", "v8.2d", "v16.2d", "v31.2d" };  
  std::vector<std::string> tv_VREG_1Q  = { "v7.1q", "v0.1q", "v1.1q", "v2.1q", "v4.1q", "v8.1q", "v16.1q", "v31.1q" };  

  std::vector<std::string> jtv_VREG_8B  = { "v7.b8", "v0.b8", "v1.b8", "v2.b8", "v4.b8", "v8.b8", "v16.b8", "v31.b8" };  
  std::vector<std::string> jtv_VREG_16B = { "v7.b16", "v0.b16", "v1.b16", "v2.b16", "v4.b16", "v8.b16", "v16.b16", "v31.b16" }; 
  std::vector<std::string> jtv_VREG_2H  = { "v7.h2", "v0.h2", "v1.h2", "v2.h2", "v4.h2", "v8.h2", "v16.h2", "v31.h2" };  
  std::vector<std::string> jtv_VREG_4H  = { "v7.h4", "v0.h4", "v1.h4", "v2.h4", "v4.h4", "v8.h4", "v16.h4", "v31.h4" };  
  std::vector<std::string> jtv_VREG_8H  = { "v7.h8", "v0.h8", "v1.h8", "v2.h8", "v4.h8", "v8.h8", "v16.h8", "v31.h8" };
  std::vector<std::string> jtv_VREG_2S  = { "v7.s2", "v0.s2", "v1.s2", "v2.s2", "v4.s2", "v8.s2", "v16.s2", "v31.s2" };  
  std::vector<std::string> jtv_VREG_4S  = { "v7.s4", "v0.s4", "v1.s4", "v2.s4", "v4.s4", "v8.s4", "v16.s4", "v31.s4" };  
  std::vector<std::string> jtv_VREG_1D  = { "v7.d1", "v0.d1", "v1.d1", "v2.d1", "v4.d1", "v8.d1", "v16.d1", "v31.d1" };  
  std::vector<std::string> jtv_VREG_2D  = { "v7.d2", "v0.d2", "v1.d2", "v2.d2", "v4.d2", "v8.d2", "v16.d2", "v31.d2" };  
  std::vector<std::string> jtv_VREG_1Q  = { "v7.q1", "v0.q1", "v1.q1", "v2.q1", "v4.q1", "v8.q1", "v16.q1", "v31.q1" };  

  std::vector<std::string> tv_VREG_8B_ELEM,  jtv_VREG_8B_ELEM;
  std::vector<std::string> tv_VREG_16B_ELEM, jtv_VREG_16B_ELEM; 
  std::vector<std::string> tv_VREG_4H_ELEM,  jtv_VREG_4H_ELEM; 
  std::vector<std::string> tv_VREG_8H_ELEM,  jtv_VREG_8H_ELEM;  
  std::vector<std::string> tv_VREG_2S_ELEM,  jtv_VREG_2S_ELEM;  
  std::vector<std::string> tv_VREG_4S_ELEM,  jtv_VREG_4S_ELEM;  
  std::vector<std::string> tv_VREG_1D_ELEM,  jtv_VREG_1D_ELEM;  
  std::vector<std::string> tv_VREG_2D_ELEM,  jtv_VREG_2D_ELEM;  

  
  std::vector<std::vector<std::string> *> tv_VectorsAs = { &tv_WREG, &tv_XREG, &tv_VREG,
							   &tv_IMM0BIT, &tv_IMM1BIT, &tv_IMM2BIT, &tv_IMM3BIT, &tv_IMM4BIT, &tv_IMM5BIT, &tv_IMM6BIT, &tv_IMM8BIT,
							   &tv_IMM12BIT, &tv_IMM13BIT, &tv_IMM16BIT,
							   &tv_IMM3BIT_N, &tv_IMM4BIT_N, &tv_IMM5BIT_N, &tv_IMM6BIT_N, &tv_FLOAT8BIT,
							   &tv_BREG, &tv_HREG, &tv_SREG, &tv_DREG, &tv_QREG,
							   &tv_BITMASK32, &tv_BITMASK64,
							   &tv_LSL_IMM, &tv_LSL32, &tv_LSL64,
							   &tv_SPECIFIC32, &tv_SPECIFIC64, &tv_SPECIFIC32_1, &tv_SPECIFIC64_1,
							   &tv_SPECIFIC32_2, &tv_SPECIFIC64_2, &tv_SPECIFIC32_3, &tv_SPECIFIC64_3,
							   &tv_SHIFT_AMOUNT32, &tv_SHIFT_AMOUNT64,
							   &tv_VREG_8B, &tv_VREG_16B, &tv_VREG_2H, &tv_VREG_4H, &tv_VREG_8H,
							   &tv_VREG_2S, &tv_VREG_4S,  &tv_VREG_1D, &tv_VREG_2D, &tv_VREG_1Q,
							   &tv_VREG_8B_ELEM, &tv_VREG_16B_ELEM, &tv_VREG_4H_ELEM, &tv_VREG_8H_ELEM,
							   &tv_VREG_2S_ELEM, &tv_VREG_4S_ELEM,  &tv_VREG_1D_ELEM, &tv_VREG_2D_ELEM }; 
  std::vector<std::vector<std::string> *> tv_VectorsJit = { &tv_WREG, &tv_XREG, &tv_VREG,
							    &tv_IMM0BIT, &tv_IMM1BIT, &tv_IMM2BIT, &tv_IMM3BIT, &tv_IMM4BIT, &tv_IMM5BIT, &tv_IMM6BIT, &tv_IMM8BIT,
							    &tv_IMM12BIT, &tv_IMM13BIT, &tv_IMM16BIT,
							    &tv_IMM3BIT_N, &tv_IMM4BIT_N, &tv_IMM5BIT_N, &tv_IMM6BIT_N, &tv_FLOAT8BIT,
							    &tv_BREG, &tv_HREG, &tv_SREG, &tv_DREG, &tv_QREG,
							    &tv_BITMASK32, &tv_BITMASK64, &jtv_LSL_IMM, &jtv_LSL32, &jtv_LSL64,
							    &jtv_SPECIFIC32, &jtv_SPECIFIC64, &jtv_SPECIFIC32_1, &jtv_SPECIFIC64_1,
							    &jtv_SPECIFIC32_2, &jtv_SPECIFIC64_2, &jtv_SPECIFIC32_3, &jtv_SPECIFIC64_3,
							    &jtv_SHIFT_AMOUNT32, &jtv_SHIFT_AMOUNT64,
							    &jtv_VREG_8B, &jtv_VREG_16B, &jtv_VREG_2H, &jtv_VREG_4H, &jtv_VREG_8H,
							    &jtv_VREG_2S, &jtv_VREG_4S,  &jtv_VREG_1D, &jtv_VREG_2D, &jtv_VREG_1Q,
							    &jtv_VREG_8B_ELEM, &jtv_VREG_16B_ELEM, &jtv_VREG_4H_ELEM, &jtv_VREG_8H_ELEM,
							    &jtv_VREG_2S_ELEM, &jtv_VREG_4S_ELEM,  &jtv_VREG_1D_ELEM, &jtv_VREG_2D_ELEM }; 

  std::vector<std::vector<std::string> *>& tv_Vectors = tv_VectorsAs;

  std::vector<std::string> tmpV   = { "v7", "v0", "v1", "v2", "v4", "v8", "v16", "v31" };
  std::vector<int>         idx8B  = { 7, 0, 1, 2, 4 };
  std::vector<int>         idx16B = { 7, 0, 1, 2, 4, 8, 15 };
  std::vector<int>         idx4H  = { 3, 0, 1, 2 };
  std::vector<int>         idx8H  = { 7, 0, 1, 2, 4 };
  std::vector<int>         idx2S  = { 1, 0 };
  std::vector<int>         idx4S  = { 3, 0, 1, 2 };
  std::vector<int>         idx1D  = { 0 };
  std::vector<int>         idx2D  = { 1, 0 };
  
  std::vector<std::string> tmp8B  = { "8b[7]", "8b[0]", "8b[1]", "8b[2]", "8b[4]" };
  std::vector<std::string> tmp16B = { "16b[15]", "16b[0]", "16b[1]", "16b[2]", "16b[4]", "16b[8]" };
  std::vector<std::string> tmp4H  = { "4h[3]", "4h[0]", "4h[1]", "4h[2]" };
  std::vector<std::string> tmp8H  = { "8h[7]", "8h[0]", "8h[1]", "8h[2]", "8h[4]" };
  std::vector<std::string> tmp2S  = { "2s[1]", "2s[0]" };
  std::vector<std::string> tmp4S  = { "4s[3]", "4s[0]", "4s[1]", "4s[2]" };
  std::vector<std::string> tmp1D  = { "1d[0]" };
  std::vector<std::string> tmp2D  = { "2d[1]", "2d[0]" };
  
  std::vector<std::string> tmpB = { "b[15]", "b[0]", "b[1]", "b[2]", "b[4]", "b[8]" };
  std::vector<std::string> tmpH = { "h[7]", "h[0]", "h[1]", "h[2]", "h[4]" };
  std::vector<std::string> tmpS = { "s[3]", "s[0]", "s[1]", "s[2]" };
  std::vector<std::string> tmpD = { "d[1]", "d[0]" };
  
  std::vector<std::string> jtmp8B  = { "b8[7]", "b8[0]", "b8[1]", "b8[2]", "b8[4]" };
  std::vector<std::string> jtmp16B = { "b16[15]", "b16[0]", "b16[1]", "b16[2]", "b16[4]", "b16[8]" };
  std::vector<std::string> jtmp4H  = { "h4[3]", "h4[0]", "h4[1]", "h4[2]" };
  std::vector<std::string> jtmp8H  = { "h8[7]", "h8[0]", "h8[1]", "h8[2]", "h8[4]" };
  std::vector<std::string> jtmp2S  = { "s2[1]", "s2[0]" };
  std::vector<std::string> jtmp4S  = { "s4[3]", "s4[0]", "s4[1]", "s4[2]" };
  std::vector<std::string> jtmp1D  = { "d1[0]" };
  std::vector<std::string> jtmp2D  = { "d2[1]", "d2[0]" };
  
  std::vector<std::string> jtmpB = { "b[15]", "b[0]", "b[1]", "b[2]", "b[4]", "b[8]" };
  std::vector<std::string> jtmpH = { "h[7]", "h[0]", "h[1]", "h[2]", "h[4]" };
  std::vector<std::string> jtmpS = { "s[3]", "s[0]", "s[1]", "s[2]" };
  std::vector<std::string> jtmpD = { "d[1]", "d[0]" };
  
  /*
    and_, or_, xor_, not_ => and, or, xor, not
  */
  std::string removeUnderScore(std::string s) const
  {
    if (!isXbyak_ && s[s.size() - 1] == '_') s.resize(s.size() - 1);
    return s;
  }



  void put(std::vector<std::string> &n, std::vector<uint64_t> &opSet, std::string name, int serial=0) const
  {
    std::cout << "//" << name << ":" << serial << std::endl; /** For easy debug */
    
    for (size_t i = 0; i < n.size(); i++) {
      const char *nm = removeUnderScore(n[i]).c_str();
      put(nm, opSet);
    }
  }

  //  char* getBaseStr(uint64_t op1)
  const char* getBaseStr(uint64_t op1) const
  {
    for (int i = 0; i < bitEnd; i++) {
      if (op1 & (1ULL << i)) {
	return get(1ULL<<i, 0);
      }
    }

    std::cerr << std::endl << __FILE__ << ":" << __LINE__ << ", Something wrong. op1=" << op1 << std::endl;
    assert(0);
    return NULL;
  }
  
  /** check all op1, op2, op3, op4, op5, op6, op7, op8 */
  void put(const char *nm, std::vector<uint64_t>& ops) const
  {
    std::vector<std::string> strBase;
    std::string hoge;
    int i, j, k;
    uint jj;
    int num_ops = ops.size();

    for(i = 0; i < num_ops; i++) {
      for(j = 0; j < bitEnd; j++) {
	if(ops[i] & (1ULL << j)) {
	  strBase.push_back(getBaseStr(ops[i]));
	  break;
	}
      }
    }

    /** No operand exists. */
    if(num_ops == 0) {
      std::cout << nm << " ";

      if(isXbyak_) {
	std::cout << "();";
      }

      std::cout << std::endl;

      return;
    }

    /** Some operands exist.
	Example:nm = add, num_ops = 2, op1 = Xn, op2 = Xn

	The following 64 patterns are printed.
	Expand for op1
	add(X0, X0); dump();
	add(x1, x0); dump();
	...
	add(x31, x0); dump();
	Expand for op2
	add(x0, x0); dump();
	add(x0, x1); dump();
	...
	add(x0, x31); dump();
    */
    for(i = 0; i < num_ops; i++) {
      for(j = 0; j < bitEnd; j++) {
	uint64_t bitpos = 1ULL << j;
	
	if(!(ops[i] & bitpos)) continue;

	for(jj = 0; jj < getNum(bitpos); jj++) {
	  /** print nemonic */
	  std::cout << nm;

	  if(isXbyak_) {
	    std::cout << "(";
	  } else {
	    std::cout << " ";
	  }
	    
	  for(k = 0; k < i; k++) {
	    if(k != 0) {
	      std::cout << ", ";
	    }
	    std::cout << strBase[k];
	  }
	    
	  if(i != 0) {
	    std::cout << ", ";
	  }
	  std::cout << get(bitpos, jj);
	    
	  for(k = i+1; k < num_ops; k++) {
	    std::cout << ", " << strBase[k];
	  }
	    
	  if(isXbyak_) {
	    std::cout << "); dump();";
	  }
	    
	  std::cout << std::endl;
	}
      }
    }
  }
    
  uint64_t getNum(uint64_t type) const
  {
    if(type==NOPARA) {
      return 0;
    }

    for(int i=0; i<bitEnd; i++) {
      if((type >> i) & 0x1) {
	return tv_Vectors[i]->size();
      }
    }

    std::cerr << std::endl << __FILE__ << ":" << __LINE__ << ", Something wrong. type=" << type << std::endl;
    assert(0);
    return 0;
  }

  const char *get(uint64_t type, uint64_t index) const
  {
    if(type==NOPARA) {
      std::cerr << std::endl << __FILE__ << ":" << __LINE__ << ", Something wrong. type=" << type << " index=" << index << std::endl;
      assert(0);
      return NULL;
    }

    for(int i=0; i<bitEnd; i++) {
      if((type >> i) & 0x1) {
	return tv_Vectors[i]->at(index).c_str();
      }
    }


    std::cerr << std::endl << __FILE__ << ":" << __LINE__ << ", Something wrong. type=" << type << " index=" << index << std::endl;
    assert(0);
    return NULL;
  }

  void setShiftAmount() {
    std::vector<std::string> tmpPtn32 = {"0x15", "0", "1", "2", "4", "8", "16", "31"};
    std::vector<std::string> tmpPtn64 = {"0x2a", "0", "1", "2", "4", "8", "16", "32", "63"};

    tv_SHIFT_AMOUNT32.clear();
    jtv_SHIFT_AMOUNT32.clear();
    tv_SHIFT_AMOUNT64.clear();
    jtv_SHIFT_AMOUNT64.clear();

    for(std::string i : tmpPtn32) {
      tv_SHIFT_AMOUNT32.push_back("LSL " + i);
      tv_SHIFT_AMOUNT32.push_back("LSR " + i);
      tv_SHIFT_AMOUNT32.push_back("ASR " + i);

      jtv_SHIFT_AMOUNT32.push_back("LSL, " + i);
      jtv_SHIFT_AMOUNT32.push_back("LSR, " + i);
      jtv_SHIFT_AMOUNT32.push_back("ASR, " + i);
    }

    /** Debug
	for(std::string i : tv_SHIFT_AMOUNT32) {
	std::cout << "//" << i << std::endl;
	}
    */
    
    for(std::string i : tmpPtn64) {
      tv_SHIFT_AMOUNT64.push_back("LSL " + i);
      tv_SHIFT_AMOUNT64.push_back("LSR " + i);
      tv_SHIFT_AMOUNT64.push_back("ASR " + i);

      jtv_SHIFT_AMOUNT64.push_back("LSL, " + i);
      jtv_SHIFT_AMOUNT64.push_back("LSR, " + i);
      jtv_SHIFT_AMOUNT64.push_back("ASR, " + i);
    }
  }

  void setShiftAmountWithRor() {
    std::vector<std::string> tmpPtn32 = {"0x15", "0", "1", "2", "4", "8", "16", "31"};
    std::vector<std::string> tmpPtn64 = {"0x2a", "0", "1", "2", "4", "8", "16", "32", "63"};

    tv_SHIFT_AMOUNT32.clear();
    jtv_SHIFT_AMOUNT32.clear();
    tv_SHIFT_AMOUNT64.clear();
    jtv_SHIFT_AMOUNT64.clear();

    for(std::string i : tmpPtn32) {
      tv_SHIFT_AMOUNT32.push_back("LSL " + i);
      tv_SHIFT_AMOUNT32.push_back("LSR " + i);
      tv_SHIFT_AMOUNT32.push_back("ASR " + i);
      tv_SHIFT_AMOUNT32.push_back("ROR " + i);

      jtv_SHIFT_AMOUNT32.push_back("LSL, " + i);
      jtv_SHIFT_AMOUNT32.push_back("LSR, " + i);
      jtv_SHIFT_AMOUNT32.push_back("ASR, " + i);
      jtv_SHIFT_AMOUNT32.push_back("ROR, " + i);
    }

    /** Debug
	for(std::string i : tv_SHIFT_AMOUNT32) {
	std::cout << "//" << i << std::endl;
	}
    */
    
    for(std::string i : tmpPtn64) {
      tv_SHIFT_AMOUNT64.push_back("LSL " + i);
      tv_SHIFT_AMOUNT64.push_back("LSR " + i);
      tv_SHIFT_AMOUNT64.push_back("ASR " + i);
      tv_SHIFT_AMOUNT64.push_back("ROR " + i);

      jtv_SHIFT_AMOUNT64.push_back("LSL, " + i);
      jtv_SHIFT_AMOUNT64.push_back("LSR, " + i);
      jtv_SHIFT_AMOUNT64.push_back("ASR, " + i);
      jtv_SHIFT_AMOUNT64.push_back("ROR, " + i);
    }
  }

  
public:
  Test(bool isXbyak)
    : isXbyak_(isXbyak)
    , funcNum_(1)
  {
    std::stringstream ss;
    ss << std::hex << std::showbase;

    for(uint64_t onesLen=1; onesLen<=31; onesLen++) { // Inall-one bit is reserved.
      uint64_t bitmask = 0;

      for(uint64_t i=1; i<=onesLen; i++) {
	bitmask = (bitmask<<1) + 1;
      }

      for(uint64_t shift=0; shift<=32-onesLen; shift++) {
	ss.str("");
	ss << bitmask;
	tv_BITMASK32.push_back(ss.str() + "<<" + std::to_string(shift));
	/** Debug	std::cout << tv_BITMASK32.at(tv_BITMASK32.size()-1) << std::endl; */
      }
    }	
    
    for(uint64_t onesLen=1; onesLen<=63; onesLen++) { // Inall-one bit is reserved.
      uint64_t bitmask = 0;

      for(uint64_t i=1; i<=onesLen; i++) {
	bitmask = (bitmask<<1) + 1;
      }

      for(uint64_t shift=0; shift<=64-onesLen; shift++) {
	ss.str("");
	ss << bitmask;
	tv_BITMASK64.push_back(ss.str() + "<<" + std::to_string(shift));
	/** Debug std::cout << tv_BITMASK64.at(tv_BITMASK64.size()-1) << std::endl; */
      }
    }	

    setAllVregElem();
    
    if (!isXbyak_) {
      tv_Vectors = tv_VectorsAs;
      return;
    } else {
      tv_Vectors = tv_VectorsJit;
      printf("%s",
	     "    void gen0()\n"
	     "    {\n");

      declareAllVregElem();
    }
  }

  /*
    gcc and vc give up to compile this source,
    so I split functions.
  */
  void separateFunc()
  {
    if (!isXbyak_) return;
    printf(
	   "    }\n"
	   "    void gen%d()\n"
	   "    {\n", funcNum_++);

    declareAllVregElem();
  }
  ~Test()
  {
    if (!isXbyak_) return;
    printf("%s",
	   "    }\n"
	   "    void gen()\n"
	   "    {\n");
    for (int i = 0; i < funcNum_; i++) {
      printf(
	     "        gen%d();\n", i);
    }
    printf(
	   "    }\n");
  }

  void clearVregElem() {
    tv_VREG_8B_ELEM.clear();
    tv_VREG_16B_ELEM.clear();
    tv_VREG_4H_ELEM.clear();
    tv_VREG_8H_ELEM.clear();
    tv_VREG_2S_ELEM.clear();
    tv_VREG_4S_ELEM.clear();
    tv_VREG_1D_ELEM.clear();
    tv_VREG_2D_ELEM.clear();

    jtv_VREG_8B_ELEM.clear();
    jtv_VREG_16B_ELEM.clear();
    jtv_VREG_4H_ELEM.clear();
    jtv_VREG_8H_ELEM.clear();
    jtv_VREG_2S_ELEM.clear();
    jtv_VREG_4S_ELEM.clear();
    jtv_VREG_1D_ELEM.clear();
    jtv_VREG_2D_ELEM.clear();
  }

  /**
     @brief
     Output for assembler file.
     v7.8b[7]
     v7.8b[7]
     
     Output for JIT compile file.
     VRegBElem v0b_7 v0.b[7]; <--- Freely declared by Xbyak user.
     v7.b8[7]                 <--- pre-defined by Xbyak for AArch64. 
     v7b_7                    <--- Freely declared by Xbyak user.
  */
  void setVregElem(std::vector<std::string>& tv, std::vector<std::string>& jtv,
		   std::string datasize, int lanenum,
		   std::vector<std::string>& vreg, const std::vector<int>& idx) {
    std::string fixedIdx = std::to_string(idx[0]);
    std::string fixedIdxPar = "[" + fixedIdx + "]";
    std::string Lanenum = std::to_string(lanenum);

    tv.clear();
    jtv.clear();

    /** Register index:rotate, Element index:fix */
    for(std::string i : vreg) {
      tv.push_back(i + "." + Lanenum + datasize + fixedIdxPar);
      tv.push_back(i + "." + Lanenum + datasize + fixedIdxPar);

      jtv.push_back(i + "." + datasize + Lanenum + fixedIdxPar);
      jtv.push_back(i + datasize + Lanenum + "_" + fixedIdx);
    }

    fixedIdx = vreg[0];
    
    /** Register index:fix, Element index:rotate */
    for(auto itr = idx.begin() + 1; itr != idx.end(); ++itr) {
      std::string tmp = std::to_string(*itr);
      std::string tmpPar = "[" + tmp + "]";
      tv.push_back(fixedIdx + "." + Lanenum + datasize + tmpPar);
      tv.push_back(fixedIdx + "." + Lanenum + datasize + tmpPar);

      jtv.push_back(fixedIdx + "." + datasize + Lanenum + tmpPar);
      jtv.push_back(fixedIdx + datasize + Lanenum + "_" + tmp);
    }

    /** Debug
	for(std::string i : tv) {
	std::cout << i << std::endl;
	}
	std::cout << std::endl;
	for(std::string i : jtv) {
	std::cout << i << std::endl;
	}
	std::cout << std::endl;
    */

  }
  
  void declareVregElem(std::string datasize, int lanenum,
		       std::vector<std::string>& vreg, const std::vector<int>& idx) {
    std::string fixedIdx = std::to_string(idx[0]);
    std::string fixedIdxPar = "[" + fixedIdx + "]";
    std::string Lanenum = std::to_string(lanenum);
    std::string Type = datasize;

    std::transform(Type.begin(), Type.end(), Type.begin(), ::toupper);

    if(!isXbyak_) {
      return;
    }

    for(std::string i : vreg) {
      std::cout << "VReg" + Type + "Elem " + i + datasize + Lanenum + "_" + fixedIdx
		<< "= " + i + "." + datasize + Lanenum + fixedIdxPar + ";" << std::endl;
    }

    fixedIdx = vreg[0];

    for(auto itr = idx.begin() + 1; itr != idx.end(); ++itr) {
      std::string tmp = std::to_string(*itr);
      std::string tmpPar = "[" + tmp + "]";
      std::cout << "VReg" + Type + "Elem " + fixedIdx + datasize + Lanenum + "_" + tmp
		<< "= " + fixedIdx + "." + datasize + Lanenum + tmpPar + ";" << std::endl;
    }
  }

  void declareAllVregElem() {
    declareVregElem("b", 8,  tmpV, idx8B);
    declareVregElem("b", 16, tmpV, idx16B);
    declareVregElem("h", 4,  tmpV, idx4H);
    declareVregElem("h", 8,  tmpV, idx8H);
    declareVregElem("s", 2,  tmpV, idx2S);
    declareVregElem("s", 4,  tmpV, idx4S);
    declareVregElem("d", 1,  tmpV, idx1D);
    declareVregElem("d", 2,  tmpV, idx2D);
  }    
  
  void setAllVregElem() {
    setVregElem(tv_VREG_8B_ELEM,  jtv_VREG_8B_ELEM,  "b", 8,  tmpV, idx8B);
    setVregElem(tv_VREG_16B_ELEM, jtv_VREG_16B_ELEM, "b", 16, tmpV, idx16B);
    setVregElem(tv_VREG_4H_ELEM,  jtv_VREG_4H_ELEM,  "h", 4,  tmpV, idx4H);
    setVregElem(tv_VREG_8H_ELEM,  jtv_VREG_8H_ELEM,  "h", 8,  tmpV, idx8H);
    setVregElem(tv_VREG_2S_ELEM,  jtv_VREG_2S_ELEM,  "s", 2,  tmpV, idx2S);
    setVregElem(tv_VREG_4S_ELEM,  jtv_VREG_4S_ELEM,  "s", 4,  tmpV, idx4S);
    setVregElem(tv_VREG_1D_ELEM,  jtv_VREG_1D_ELEM,  "d", 1,  tmpV, idx1D);
    setVregElem(tv_VREG_2D_ELEM,  jtv_VREG_2D_ELEM,  "d", 2,  tmpV, idx2D);
  }    
  
  /** MACRO is expanded in order from the outside. */
  /*** SIMD move on page C3-212 */
  /** C7.2.32 DUP (element)*/
  /** C7.2.184 MOV (scalar) */
  /** Scalar */
  PUT4(DataProcSimd_Move0,
       NM("dup", "mov"),
       OPS(BREG, VREG_B_ELEM), OPS(HREG, VREG_H_ELEM), OPS(SREG, VREG_S_ELEM), OPS(DREG, VREG_D_ELEM));
  /** Vector */
  PUT7(DataProcSimd_Move1,
       NM("dup"),
       OPS(VREG_8B, VREG_B_ELEM), OPS(VREG_16B, VREG_B_ELEM),
       OPS(VREG_4H, VREG_H_ELEM), OPS(VREG_8H, VREG_H_ELEM),
       OPS(VREG_2S, VREG_S_ELEM), OPS(VREG_4S, VREG_S_ELEM),
       OPS(VREG_2D, VREG_D_ELEM));
  /** C7.2.33 DUP (general) */
  /** Advanced SIMD */
  PUT7(DataProcSimd_Move2,
       NM("dup"),
       OPS(VREG_8B, WREG), OPS(VREG_16B, WREG), OPS(VREG_4H, WREG), OPS(VREG_8H, WREG),
       OPS(VREG_2S, WREG), OPS(VREG_4S, WREG), OPS(VREG_2D, XREG));
  /** C7.2.160 INS (element) */
  /** C7.2.185 MOV (element) */
  /** Advanced SIMD */
  PUT3(DataProcSimd_Move3,
       NM("ins", "mov"),
       OPS(VREG_B_ELEM, VREG_B_ELEM), OPS(VREG_H_ELEM, VREG_H_ELEM), OPS(VREG_S_ELEM, VREG_S_ELEM));
  PUT1(DataProcSimd_Move4,
       NM("ins"),
       OPS(VREG_D_ELEM, VREG_D_ELEM));
  /** C7.2.161 INS (general) */
  /** C7.2.186 MOV (general) */
  /** Advanced SIMD */
  PUT4(DataProcSimd_Move5,
       NM("ins", "mov"),
       OPS(VREG_B_ELEM, WREG), OPS(VREG_H_ELEM, WREG), OPS(VREG_S_ELEM, WREG), OPS(VREG_D_ELEM, XREG));
  /** C7.2.188 MOV (to general) */
  /** 32-bit, 64-bit */
  PUT2(DataProcSimd_Move6,
       NM("mov"),
       OPS(WREG, VREG_S_ELEM), OPS(XREG, VREG_D_ELEM));
  /** C7.2.353 UMOV */
  /** 32-bit, 64-bit */
  PUT4(DataProcSimd_Move7,
       NM("umov"),
       OPS(WREG, VREG_B_ELEM), OPS(WREG, VREG_H_ELEM), OPS(WREG, VREG_S_ELEM),
       OPS(XREG, VREG_D_ELEM));
  /** C7.2.263 SMOV */
  /** 32-bit, 64-bit */
  PUT5(DataProcSimd_Move8,
       NM("smov"),
       OPS(WREG, VREG_B_ELEM), OPS(WREG, VREG_H_ELEM),
       OPS(XREG, VREG_B_ELEM), OPS(XREG, VREG_H_ELEM), OPS(XREG, VREG_S_ELEM));

  void putDataProcSimd_Move() {
    putDataProcSimd_Move0();
    putDataProcSimd_Move1();
    putDataProcSimd_Move2();
    putDataProcSimd_Move3();
    putDataProcSimd_Move4();
    putDataProcSimd_Move5();
    putDataProcSimd_Move6();
    putDataProcSimd_Move7();
    putDataProcSimd_Move8();
  }
  
  /*** SIMD arithmetic on page C3-213. */
  /** C7.2.2 ADD (vector) */
  /** Scalar */
  PUT1(DataProcSimd_Arithmetic0,
       NM("add"),
       OPS(DREG, DREG, DREG));
  /** Vector */
  PUT4(DataProcSimd_Arithmetic1,
       NM("add"),
       OPS(VREG_8B, VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B, VREG_16B), OPS(VREG_4H, VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H, VREG_8H));
  PUT3(DataProcSimd_Arithmetic2,
       NM("add"),
       OPS(VREG_2S, VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S, VREG_4S), OPS(VREG_2D, VREG_2D, VREG_2D));
  /** C7.2.11 AND (vector) */
  /** Three register s of the smae type variant */
  PUT2(DataProcSimd_Arithmetic3,
       NM("and_"),
       OPS(VREG_8B, VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B, VREG_16B));
  /** C7.2.14 BIC (vector, register) */
  /** C7.2.15 BIF */
  /** C7.2.16 BIT */
  /** C7.2.17 BSL */
  /** C7.2.34 EOR (vector) */
  /** C7.2.196 ORN (vector) */
  /** C7.2.198 ORR (vector, register) */
  /** C7.2.199 PMUL */
  /** Three registers of the smae type variant */
  PUT2(DataProcSimd_Arithmetic4,
       NM("bic", "bif", "bit", "bsl", "eor", "orn", "orr", "pmul"),
       OPS(VREG_8B, VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B, VREG_16B));
  /** C7.2.187 MOV (vector) */
  /** Three registers of the smae type variant */
  PUT2(DataProcSimd_Arithmetic5,
       NM("mov"),
       OPS(VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B));
  /** C7.2.37 FABD */
  /** C7.2.131 FMULX */
  /** C7.2.138 FRECPS */
  /** C7.2.155 FRSQRTS */
  /** Scalar half precision variant */
  /** Scalar single-precision and double-precision */
  /** Vector half precision */
  /** Vector single-precision and double-precision */
  PUT8(DataProcSimd_Arithmetic6,
       NM("fabd", "fmulx", "frecps", "frsqrts"),
       OPS(HREG, HREG, HREG), OPS(SREG, SREG, SREG), OPS(DREG, DREG, DREG),
       OPS(VREG_8H, VREG_8H, VREG_8H), OPS(VREG_4H, VREG_4H, VREG_4H), OPS(VREG_4S, VREG_4S, VREG_4S), OPS(VREG_2S, VREG_2S, VREG_2S),
       OPS(VREG_2D, VREG_2D, VREG_2D));
  /** C7.2.43 FADD (scalar) */
  /** C7.2.91 FDIV (scalar) */
  PUT3(DataProcSimd_Arithmetic7,
       NM("fadd", "fdiv"),
       OPS(HREG, HREG, HREG), OPS(SREG, SREG, SREG), OPS(DREG, DREG, DREG));
  /** C7.2.90 FDIV (vector) */
  /** C7.2.102 FMAXP (vector) */
  /** C7.2.96 FMAXNM (vector) */
  /** C7.2.104 FMIN (vector) */
  /** C7.2.106 FMINNM (vector) */
  /** C7.2.115 FMLA (vector) */
  /** C7.2.119 FMLS (vector) */
  /** C7.2.128 FMUL (vector) */
  /** C7.2.158 FSUB (vector) */
  PUT5(DataProcSimd_Arithmetic8,
       NM("fdiv", "fmaxp", "fmaxnm", "fmin", "fminnm", "fmla", "fmls", "fmul", "fsub"),
       OPS(VREG_8H, VREG_8H, VREG_8H), OPS(VREG_4H, VREG_4H, VREG_4H), OPS(VREG_2S, VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S, VREG_4S),
       OPS(VREG_2D, VREG_2D, VREG_2D));
  /** C7.2.117 FMLAL, FMLAL2 (vector) */
  /** C7.2.121 FMLSL, FMLSL2 (vector) */
  /** FMLAL variant, FMLAL2 variant */
  PUT2(DataProcSimd_Arithmetic9,
       NM("fmlal", "fmlal", "fmlsl", "fmlsl2"),
       OPS(VREG_2S, VREG_2H, VREG_2H), OPS(VREG_4S, VREG_4H, VREG_4H));
  /** C7.2.182 MLA (vector) */
  /** C7.2.183 MLS (vector) */
  /** C7.2.209 SABA */
  /** C7.2.211 SABD */
  /** C7.2.238 SHADD */
  /** C7.2.242 SHSUB */
  /** C7.2.253 SMAX */
  /** C7.2.256 SMIN */
  /** C7.2.294 SRHADD */
  /** C7.2.326 UABA */
  /** C7.2.328 UABD */
  /** C7.2.341 UHADD */
  /** C7.2.342 UHSUB */
  /** C7.2.343 UMAX */
  /** C7.2.346 UMIN */
  /** C7.2.365 URHADD */
  /** Three registers of the smae type variant */
  PUT6(DataProcSimd_Arithmetic10,
       NM("mla", "mls", "mul", "saba", "sabd", "shadd", "shsub", "smax",
	  "smin", "srhadd", "uaba", "uabd", "uhadd", "uhsub", "umax", "umin",
	  "urhadd"),
       OPS(VREG_8B, VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B, VREG_16B), OPS(VREG_4H, VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H, VREG_8H),
       OPS(VREG_2S, VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S, VREG_4S));
  /** C7.2.267 SQADD */
  /** C7.2.283 SQRSHL */
  /** C7.2.287 SQSHL (register) */
  /** C7.2.291 SQSUB */
  /** C7.2.356 UQADD */
  /** C7.2.357 UQRSHL */
  /** C7.2.360 UQSHL (register) */
  /** C7.2.362 UQSUB */
  /** Scalar variant */
  /** Vector variant */
  PUT8(DataProcSimd_Arithmetic11,
       NM("sqadd", "sqrshl", "sqshl", "sqsub", "uqadd", "uqrshl", "uqshl", "uqsub"),
       OPS(BREG, BREG, BREG), OPS(HREG, HREG, HREG), OPS(SREG, SREG, SREG), OPS(DREG, DREG, DREG),
       OPS(VREG_8B, VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B, VREG_16B), OPS(VREG_4H, VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H, VREG_8H));
  PUT3(DataProcSimd_Arithmetic12,
       NM("sqadd", "sqrshl", "sqshl", "sqsub", "uqadd", "uqrshl", "uqshl", "uqsub"),
       OPS(VREG_2S, VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S, VREG_4S), OPS(VREG_2D, VREG_2D, VREG_2D));
  /** C7.2.273 SQDMULH (vector) */
  /** C7.2.278 SQRDMLAH (vector) */
  /** C7.2.280 SQRDMLSH (vector) */
  /** C7.2.282 SQRDMULH (vector) */
  /** Scalar variant */
  /** Vector variant */
  PUT6(DataProcSimd_Arithmetic13,
       NM("sqdmulh", "sqrdmlah", "sqrdmlsh", "sqrdmulh"),
       OPS(HREG, HREG, HREG), OPS(SREG, SREG, SREG), OPS(VREG_4H, VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H, VREG_8H),
       OPS(VREG_2S, VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S, VREG_4S));
  /** C7.2.296 SRSHL */
  /** C7.2.299 SSHL */
  /** C7.2.318 SUB (vector) */
  /** C7.2.366 URSHL */
  /** C7.2.370 USHL */
  /** Scalar variant */
  /** Vector variant */
  PUT8(DataProcSimd_Arithmetic14,
       NM("srshl", "sshl", "sub", "urshl", "ushl"),
       OPS(DREG, DREG, DREG),
       OPS(VREG_8B, VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B, VREG_16B), OPS(VREG_4H, VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H, VREG_8H),
       OPS(VREG_2S, VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S, VREG_4S), OPS(VREG_2D, VREG_2D, VREG_2D));

  void putDataProcSimd_Arithmetic() {
    putDataProcSimd_Arithmetic0();
    putDataProcSimd_Arithmetic1();
    putDataProcSimd_Arithmetic2();
    putDataProcSimd_Arithmetic3();
    putDataProcSimd_Arithmetic4();
    putDataProcSimd_Arithmetic5();
    putDataProcSimd_Arithmetic6();
    putDataProcSimd_Arithmetic7();
    putDataProcSimd_Arithmetic8();
    putDataProcSimd_Arithmetic9();
    putDataProcSimd_Arithmetic10();
    putDataProcSimd_Arithmetic11();
    putDataProcSimd_Arithmetic12();
    putDataProcSimd_Arithmetic13();
    putDataProcSimd_Arithmetic14();
  }    

  /*** SIMD compare on page C3-216. */
  /** C7.2.20 CMEQ (register) */
  /** C7.2.27 CMHS (register) */
  /** C7.2.22 CMGE (register) */
  /** C7.2.26 CMHI (register) */
  /** C7.2.24 CMGT (register) */
  /** C7.2.30 CMTST */
  /** Scalar variant */
  /** Vector variant */
  PUT8(DataProcSimd_Compare0,
       NM("cmeq", "cmhs", "cmge", "cmhi", "cmgt", "cmtst"),
       OPS(DREG, DREG, DREG),
       OPS(VREG_8B, VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B, VREG_16B), OPS(VREG_4H, VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H, VREG_8H),
       OPS(VREG_2S, VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S, VREG_4S), OPS(VREG_2D, VREG_2D, VREG_2D));
  /** C7.2.21 CMEQ (zero) */
  /** C7.2.23 CMGE (zero) */
  /** C7.2.25 CMGT (zero) */
  /** C7.2.28 CMLE (zero) */
  /** C7.2.29 CMLT (zero) */
  /** Scalar variant */
  /** Vector variant */
  PUT8(DataProcSimd_Compare1,
       NM("cmeq", "cmge", "cmgt", "cmle", "cmlt"),
       OPS(DREG, DREG, SPECIFIC64),
       OPS(VREG_8B, VREG_8B, SPECIFIC64), OPS(VREG_16B, VREG_16B, SPECIFIC64),
       OPS(VREG_4H, VREG_4H, SPECIFIC64), OPS(VREG_8H, VREG_8H, SPECIFIC64),
       OPS(VREG_2S, VREG_2S, SPECIFIC64), OPS(VREG_4S, VREG_4S, SPECIFIC64),
       OPS(VREG_2D, VREG_2D, SPECIFIC64));
  /** C7.2.49 FCMEQ (register) */
  /** C7.2.51 FCMGE (register) */
  /** C7.2.53 FCMGT (register) */
  /** C7.2.40 FACGE */
  /** C7.2.41 FACGT */
  /** Scalar half precision variant */
  /** Scalar single-precision and double-precision */
  /** Vector half precision */
  /** Vector single-precision and double-precision */
  PUT8(DataProcSimd_Compare2,
       NM("fcmeq", "fcmge", "fcmgt", "facge", "facgt"),
       OPS(HREG, HREG, HREG), OPS(SREG, SREG, SREG), OPS(DREG, DREG, DREG),
       OPS(VREG_8H, VREG_8H, VREG_8H), OPS(VREG_4H, VREG_4H, VREG_4H), OPS(VREG_4S, VREG_4S, VREG_4S), OPS(VREG_2S, VREG_2S, VREG_2S),
       OPS(VREG_2D, VREG_2D, VREG_2D));
  /** C7.2.50 FCMEQ (zero) */
  /** C7.2.52 FCMGE (zero) */
  /** C7.2.54 FCMGT (zero) */
  /** C7.2.57 FCMLE (zero) */
  /** C7.2.58 FCMLT (zero) */
  PUT8(DataProcSimd_Compare3,
       NM("fcmeq", "fcmge", "fcmgt", "fcmle", "fcmlt"),
       OPS(HREG, HREG, SPECIFIC64), OPS(SREG, SREG, SPECIFIC64), OPS(DREG, DREG, SPECIFIC64),
       OPS(VREG_8H, VREG_8H, SPECIFIC64), OPS(VREG_4H, VREG_4H, SPECIFIC64),
       OPS(VREG_4S, VREG_4S, SPECIFIC64), OPS(VREG_2S, VREG_2S, SPECIFIC64),
       OPS(VREG_2D, VREG_2D, SPECIFIC64));

  void putDataProcSimd_Compare() {
    tv_SPECIFIC64.clear();
    jtv_SPECIFIC64.clear();

    tv_SPECIFIC64.push_back("0");
    jtv_SPECIFIC64.push_back("0");

    putDataProcSimd_Compare0();
    putDataProcSimd_Compare1();
    putDataProcSimd_Compare2();
    putDataProcSimd_Compare3();
  }

  /*** SIMD widening and narrowing arithmetic on page C3-216. */
  /** C7.2.3 ADDHN, ADDHN2 */
  /** C7.2.201 RADDHN, RADDHN2 */
  /** C7.2.208 RSUBHN, RSUBHN2 */
  /** C7.2.319 SUBHN, SUBHN2 */
  /** Three registers, not all the same type variant */
  PUT3(DataProcSimd_WideningAndNarrowingArithmetic0,
       NM("addhn", "raddhn", "rsubhn", "subhn"),
       OPS(VREG_8B, VREG_8H, VREG_8H), OPS(VREG_4H, VREG_4S, VREG_4S),
       OPS(VREG_2S, VREG_2D, VREG_2D)); 
  PUT3(DataProcSimd_WideningAndNarrowingArithmetic1,
       NM("addhn2", "raddhn2", "rsubhn2", "subhn2"),
       OPS(VREG_16B, VREG_8H, VREG_8H), OPS(VREG_8H, VREG_4S, VREG_4S),
       OPS(VREG_4S, VREG_2D, VREG_2D));
  /** C7.2.200 PMULL, PMULL2 */
  /** Three registers, not all the same type variant */
  PUT2(DataProcSimd_WideningAndNarrowingArithmetic2,
       NM("pmull"),
       OPS(VREG_8H, VREG_8B, VREG_8B), OPS(VREG_1Q, VREG_1D, VREG_1D));
  PUT2(DataProcSimd_WideningAndNarrowingArithmetic3,
       NM("pmull2"),
       OPS(VREG_8H, VREG_16B, VREG_16B), OPS(VREG_1Q, VREG_2D, VREG_2D));
  /** C7.2.210 SABAL, SABAL2 */
  /** C7.2.212 SABDL, SABDL2 */
  /** C7.2.214 SADDL, SADDL2 */
  /** C7.2.260 SMLAL, SMLAL2 (vector) */
  /** C7.2.262 SMLSL, SMLSL2 (vector) */
  /** C7.2.265 SMULL, SMULL2 (vector) */
  /** C7.2.303 SSUBL, SSUBL2 */
  /** Three registers, not all the same type variant */
  PUT3(DataProcSimd_WideningAndNarrowingArithmetic4,
       NM("sabal", "sabdl", "saddl", "smlal", "smlsl", "smull", "ssubl"),
       OPS(VREG_8H, VREG_8B, VREG_8B), OPS(VREG_4S, VREG_4H, VREG_4H),
       OPS(VREG_2D, VREG_2S, VREG_2S));
  PUT3(DataProcSimd_WideningAndNarrowingArithmetic5,
       NM("sabal2", "sabdl2", "saddl2", "smlal2", "smlsl2", "smull2", "ssubl2"),
       OPS(VREG_8H, VREG_16B, VREG_16B), OPS(VREG_4S, VREG_8H, VREG_8H),
       OPS(VREG_2D, VREG_4S, VREG_4S));
  /** C7.2.217 SADDW, SADDW2 */
  PUT3(DataProcSimd_WideningAndNarrowingArithmetic6,
       NM("saddw"),
       OPS(VREG_8H, VREG_8H, VREG_8B), OPS(VREG_4S, VREG_4S, VREG_4H),
       OPS(VREG_2D, VREG_2D, VREG_2S));
  PUT3(DataProcSimd_WideningAndNarrowingArithmetic7,
       NM("saddw2"),
       OPS(VREG_8H, VREG_8H, VREG_16B), OPS(VREG_4S, VREG_4S, VREG_8H),
       OPS(VREG_2D, VREG_2D, VREG_4S));
  /** C7.2.269 SQDMLAL, SQDMLAL2 (vector) */
  /** C7.2.271 SQDMLSL, SQDMLSL2 (vector) */
  /** C7.2.275 SQDMULL, SQDMULL2 (vector) */
  /** Scalar variant */
  /** Vector variant */
  PUT4(DataProcSimd_WideningAndNarrowingArithmetic8,
       NM("sqdmlal", "sqdmlsl", "sqdmull"),
       OPS(SREG, HREG, HREG), OPS(DREG, SREG, SREG),
       OPS(VREG_4S, VREG_4H, VREG_4H), OPS(VREG_2D, VREG_2S, VREG_2S));
  PUT2(DataProcSimd_WideningAndNarrowingArithmetic9,
       NM("sqdmlal2", "sqdmlsl2", "sqdmull2"),
       OPS(VREG_4S, VREG_8H, VREG_8H), OPS(VREG_2D, VREG_4S, VREG_4S));
  /** C7.2.304 SSUBW, SSUBW2 */
  /** C7.2.334 UADDW, UADDW2 */
  /** C7.2.376 USUBW, USUBW2 */
  /** Three registers, not all the same type variant */
  PUT3(DataProcSimd_WideningAndNarrowingArithmetic10,
       NM("ssubw", "uaddw", "usubw"),
       OPS(VREG_8H, VREG_8H, VREG_8B), OPS(VREG_4S, VREG_4S, VREG_4H), 
       OPS(VREG_2D, VREG_2D, VREG_2S));
  PUT3(DataProcSimd_WideningAndNarrowingArithmetic11,
       NM("ssubw2", "uaddw2", "usubw2"),
       OPS(VREG_8H, VREG_8H, VREG_16B), OPS(VREG_4S, VREG_4S, VREG_8H),
       OPS(VREG_2D, VREG_2D, VREG_4S));
  /** C7.2.327 UABAL, UABAL2 */
  /** C7.2.329 UABDL, UABDL2 */
  /** C7.2.331 UADDL, UADDL2 */
  /** C7.2.350 UMLAL, UMLAL2 (vector) */
  /** C7.2.352 UMLSL, UMLSL2 (vector) */
  /** C7.2.355 UMULL, UMULL2 (vector) */
  /** C7.2.375 USUBL, USUBL2 */
  PUT3(DataProcSimd_WideningAndNarrowingArithmetic12,
       NM("uabal", "uabdl", "uaddl", "umlal", "umlsl", "umull", "usubl"),
       OPS(VREG_8H, VREG_8B, VREG_8B), OPS(VREG_4S, VREG_4H, VREG_4H), 
       OPS(VREG_2D, VREG_2S, VREG_2S));
  PUT3(DataProcSimd_WideningAndNarrowingArithmetic13,
       NM("uabal2", "uabdl2", "uaddl2", "umlal2", "umlsl2", "umull2", "usubl2"),
       OPS(VREG_8H, VREG_16B, VREG_16B), OPS(VREG_4S, VREG_8H, VREG_8H),
       OPS(VREG_2D, VREG_4S, VREG_4S));

  void putDataProcSimd_WideningAndNarrowingArithmetic() {
    putDataProcSimd_WideningAndNarrowingArithmetic0();
    putDataProcSimd_WideningAndNarrowingArithmetic1();
    putDataProcSimd_WideningAndNarrowingArithmetic2();
    putDataProcSimd_WideningAndNarrowingArithmetic3();
    putDataProcSimd_WideningAndNarrowingArithmetic4();
    putDataProcSimd_WideningAndNarrowingArithmetic5();
    putDataProcSimd_WideningAndNarrowingArithmetic6();
    putDataProcSimd_WideningAndNarrowingArithmetic7();
    putDataProcSimd_WideningAndNarrowingArithmetic8();
    putDataProcSimd_WideningAndNarrowingArithmetic9();
    putDataProcSimd_WideningAndNarrowingArithmetic10();
    putDataProcSimd_WideningAndNarrowingArithmetic11();
    putDataProcSimd_WideningAndNarrowingArithmetic12();
    putDataProcSimd_WideningAndNarrowingArithmetic13();
  }    
  

  /*** SIMD unary arithmetic on page C3-218. */
  /** C7.2.1 ABS */
  /** C7.2.194 NEG (vector) */
  /** Scalar variant */
  /** Vector variant */
  PUT8(DataProcSimd_UnaryArithmetic0,
       NM("abs", "neg"),
       OPS(DREG, DREG),
       OPS(VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B), OPS(VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H),
       OPS(VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S), OPS(VREG_2D, VREG_2D));
  /** C7.2.18 CLS (vector) */
  /** C7.2.19 CLZ (vector) */
  /** C7.2.06 REV64 */
  /** Vector variant */
  PUT6(DataProcSimd_UnaryArithmetic1,
       NM("cls", "clz", "rev64"),
       OPS(VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B), OPS(VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H),
       OPS(VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S));
  /** C7.2.31 CNT */
  /** C7.2.192 MVN */
  /** C7.2.195 NOT */
  /** C7.2.203 RBIT (vector) */
  /** C7.2.204 REV16 (vector) */
  /** Vector variant */
  PUT2(DataProcSimd_UnaryArithmetic2,
       NM("cnt", "mvn", "not_", "rbit", "rev16"),
       OPS(VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B));
  /** C7.2.38 FABS (vector) */
  /** C7.2.132 FNEG (vector) */
  /** C7.2.142 FRINTI (vector) */
  /** C7.2.144 FRINTM (vector) */
  /** C7.2.146 FRINTN (vector) */
  /** C7.2.148 FRINTP (vector) */
  /** C7.2.150 FRINTX (vector) */
  /** C7.2.152 FRINTZ (vector) */
  /** C7.2.156 FSQRT (vector) */
  /** Half-precision variant */
  /** Single-precision and double-precision */
  PUT5(DataProcSimd_UnaryArithmetic3,
       NM("fabs", "fneg", "frinti", "frintm", "frintn", "frintp", "frintx", "frintz",
	  "fsqrt"),
       OPS(VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H), OPS(VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S),
       OPS(VREG_2D, VREG_2D));
  /** C7.2.67 FCVTL, FCVTL2 */
  /** Vector single-precison and doiuble-precision variant */
  PUT2(DataProcSimd_UnaryArithmetic4,
       NM("fcvtl"),
       OPS(VREG_4S, VREG_4H), OPS(VREG_2D, VREG_2S));
  PUT2(DataProcSimd_UnaryArithmetic5,
       NM("fcvtl2"),
       OPS(VREG_4S, VREG_8H), OPS(VREG_2D, VREG_4S));
  /** C7.2.72 FCVTN, FCVTN2 */
  /** Vector single-precison and doiuble-precision variant */
  PUT2(DataProcSimd_UnaryArithmetic6,
       NM("fcvtn"),
       OPS(VREG_4H, VREG_4S), OPS(VREG_2S, VREG_2D));
  PUT2(DataProcSimd_UnaryArithmetic7,
       NM("fcvtn2"),
       OPS(VREG_8H, VREG_4S), OPS(VREG_4S, VREG_2D));
  /** C7.2.81 FCVTXN, FCVTXN2 */
  /** Scalar variant */
  /** Vector variant */
  PUT2(DataProcSimd_UnaryArithmetic8,
       NM("fcvtxn"),
       OPS(SREG, DREG), OPS(VREG_2S, VREG_2D));
  PUT1(DataProcSimd_UnaryArithmetic9,
       NM("fcvtxn2"),
       OPS(VREG_4S, VREG_2D));
  /** C7.2.137 FRECPE */
  /** C7.2.154 FRSQRTE */
  /** Scalar half precision variant */
  /** Scalar single-precision and double-precision */
  /** Vector half precision */
  /** Vector single-precision and double-precision */
  PUT8(DataProcSimd_UnaryArithmetic10,
       NM("frecpe", "frsqrte"),
       OPS(HREG, HREG), OPS(SREG, SREG), OPS(DREG, DREG),
       OPS(VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H), OPS(VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S),
       OPS(VREG_2D, VREG_2D));
  /** C7.2.139 FRECPX */
  /** C7.2.141 FRINTA (scalar) */
  /** Half-precisoin variant */
  /** Single-precision and double-precision */
  PUT3(DataProcSimd_UnaryArithmetic11,
       NM("frecpx", "frinta"),
       OPS(HREG, HREG), OPS(SREG, SREG), OPS(DREG, DREG));
  /** C7.2.205 REV32 (vector) */
  PUT4(DataProcSimd_UnaryArithmetic12,
       NM("rev32"),
       OPS(VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B), OPS(VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H));
  /** C7.2.213 SADALP */
  /** C7.2.215 SADDLP */
  /** C7.2.330 UADALP */
  /** C7.2.332 UADDLP */
  /** Vector variant */
  PUT6(DataProcSimd_UnaryArithmetic13,
       NM("sadalp", "saddlp", "uadalp", "uaddlp"),
       OPS(VREG_4H, VREG_8B), OPS(VREG_8H, VREG_16B), OPS(VREG_2S, VREG_4H), OPS(VREG_4S, VREG_8H),
       OPS(VREG_1D, VREG_2S), OPS(VREG_2D, VREG_4S));
  /** C7.2.321 SXTL, SXTL2 */
  PUT3(DataProcSimd_UnaryArithmetic14,
       NM("sxtl"),
       OPS(VREG_8H, VREG_8B), OPS(VREG_4S, VREG_4H), OPS(VREG_2D, VREG_2S));
  PUT3(DataProcSimd_UnaryArithmetic15,
       NM("sxtl2"),
       OPS(VREG_8H, VREG_16B), OPS(VREG_4S, VREG_8H), OPS(VREG_2D, VREG_4S));
  /** C7.2.266 SQABS */
  /** C7.2.276 SQNEG */
  /** C7.2.320 SUQADD */
  /** C7.2.373 USQADD */
  /** Scalar variant */
  /** Vector variant */
  PUT8(DataProcSimd_UnaryArithmetic16,
       NM("sqabs", "sqneg", "suqadd", "usqadd"),
       OPS(BREG, BREG), OPS(HREG, HREG), OPS(SREG, SREG), OPS(DREG, DREG),
       OPS(VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B), OPS(VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H));
  PUT3(DataProcSimd_UnaryArithmetic17,
       NM("sqabs", "sqneg", "suqadd", "usqadd"),
       OPS(VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S), OPS(VREG_2D, VREG_2D));
  /** C7.2.292 SQXTN, SQXTN2 */
  /** C7.2.293 SQXTUN, SQXTUN2 */
  /** C7.2.363 UQXTN, UQXTN2 */
  /** Scalar variant */
  /** Vecotr variant */
  PUT6(DataProcSimd_UnaryArithmetic18,
       NM("sqxtn", "sqxtun", "uqxtn"),
       OPS(BREG, HREG), OPS(HREG, SREG), OPS(SREG, DREG),
       OPS(VREG_8B, VREG_8H), OPS(VREG_4H, VREG_4S),
       OPS(VREG_2S, VREG_2D));
  PUT3(DataProcSimd_UnaryArithmetic19,
       NM("sqxtn2", "sqxtun2", "uqxtn2"),
       OPS(VREG_16B, VREG_8H), OPS(VREG_8H, VREG_4S),
       OPS(VREG_4S, VREG_2D));
  /** C7.2.364 URECPE */
  /** C7.2.368 URSQRTE */
  /** Vector variant */
  PUT2(DataProcSimd_UnaryArithmetic20,
       NM("urecpe", "ursqrte"),
       OPS(VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S));
  /** C7.2.377 UXTL, UXTL2 */  
  /** Vector variant */
  PUT3(DataProcSimd_UnaryArithmetic21,
       NM("uxtl"),
       OPS(VREG_8H, VREG_8B), OPS(VREG_4S, VREG_4H),
       OPS(VREG_2D, VREG_2S));
  PUT3(DataProcSimd_UnaryArithmetic22,
       NM("uxtl2"),
       OPS(VREG_8H, VREG_16B), OPS(VREG_4S, VREG_8H),
       OPS(VREG_2D, VREG_4S));
  /** C7.2.381 XTN, XTN2 */
  /** Vector variant */
  PUT3(DataProcSimd_UnaryArithmetic23,
       NM("xtn"),
       OPS(VREG_8B, VREG_8H), OPS(VREG_4H, VREG_4S),
       OPS(VREG_2S, VREG_2D));
  PUT3(DataProcSimd_UnaryArithmetic24,
       NM("xtn2"),
       OPS(VREG_16B, VREG_8H), OPS(VREG_8H, VREG_4S),
       OPS(VREG_4S, VREG_2D));

  void putDataProcSimd_UnaryArithmetic() {
    putDataProcSimd_UnaryArithmetic0();
    putDataProcSimd_UnaryArithmetic1();
    putDataProcSimd_UnaryArithmetic2();
    putDataProcSimd_UnaryArithmetic3();
    putDataProcSimd_UnaryArithmetic4();
    putDataProcSimd_UnaryArithmetic5();
    putDataProcSimd_UnaryArithmetic6();
    putDataProcSimd_UnaryArithmetic7();
    putDataProcSimd_UnaryArithmetic8();
    putDataProcSimd_UnaryArithmetic9();
    putDataProcSimd_UnaryArithmetic10();
    putDataProcSimd_UnaryArithmetic11();
    putDataProcSimd_UnaryArithmetic12();
    putDataProcSimd_UnaryArithmetic13();
    putDataProcSimd_UnaryArithmetic14();
    putDataProcSimd_UnaryArithmetic15();
    putDataProcSimd_UnaryArithmetic16();
    putDataProcSimd_UnaryArithmetic17();
    putDataProcSimd_UnaryArithmetic18();
    putDataProcSimd_UnaryArithmetic19();
    putDataProcSimd_UnaryArithmetic20();
    putDataProcSimd_UnaryArithmetic21();
    putDataProcSimd_UnaryArithmetic22();
    putDataProcSimd_UnaryArithmetic23();
    putDataProcSimd_UnaryArithmetic24();
  }

  /*** SIMD by element arithmetic on page C3-219. */
  /**** C7.2.114 FMLA (by element) */
  /**** C7.2.118 FMLS (by element) */
  /**** C7.2.127 FMUL (by element) */
  /**** C7.2.130 FMULX (by element) */
  PUT8(DataProcSimd_ByElementArithmetic0,
       NM("fmla", "fmls", "fmul", "fmulx"),
       // Scalar, half-precision variant
       // FMLA <Hd>, <Hn>, <Vm>.H[<index>]
       OPS(HREG, HREG, SPECIFIC32),
       // Scalar, single-precision and double-precision
       // FMLA <V><d>, <V><n>, <Vm>.<Ts>[<index>]
       OPS(SREG, SREG, VREG_S_ELEM), OPS(DREG, DREG, VREG_D_ELEM),
       // Vector, half-precision variant
       // FMLA <Vd>.<T>, <Vn>.<T>, <Vm>.H[<index>]
       OPS(VREG_4H, VREG_4H, SPECIFIC32), OPS(VREG_8H, VREG_8H, SPECIFIC32),
       // Vector, single-precision and double-precision variant
       // FMLA <Vd>.<T>, <Vn>.<T>, <Vm>.<Ts>[<index>]
       OPS(VREG_2S, VREG_2S, VREG_S_ELEM), OPS(VREG_4S, VREG_4S, VREG_S_ELEM),
       OPS(VREG_2D, VREG_2D, VREG_D_ELEM));
  /** C7.2.116 FMLAL, FMLAL2 (by element) */
  /** C7.2.120 FMLSL, FMLSL2 (by element) */
  /** FMLAL variant */
  PUT2(DataProcSimd_ByElementArithmetic1,
       NM("fmlal", "fmlal2", "fmlsl", "fmlsl2"),
       OPS(VREG_2S, VREG_2H, SPECIFIC32),
       OPS(VREG_4S, VREG_4H, SPECIFIC32));
  /** C7.2.180 MLA (by element) */
  /** C7.2.182 MLS (by element) */
  /** C7.2.190 MUL (by element) */
  /** Vecotr variant */
  PUT4(DataProcSimd_ByElementArithmetic2,
       NM("mla", "mls", "mul"),
       OPS(VREG_4H, VREG_4H, SPECIFIC32), OPS(VREG_8H, VREG_8H, SPECIFIC32),
       OPS(VREG_2S, VREG_2S, VREG_S_ELEM), OPS(VREG_4S, VREG_4S, VREG_S_ELEM));
  /** C7.2.259 SMLAL, SMLAL2 (by element) */
  /** C7.2.261 SMLSL, SMLSL2 (by element) */
  /** C7.2.264 SMULL, SMULL2 (by element) */
  /** Vecotr variant */
  PUT2(DataProcSimd_ByElementArithmetic3,
       NM("smlal", "smull"),
       OPS(VREG_4S, VREG_4H, SPECIFIC32), 
       OPS(VREG_2D, VREG_2S, VREG_S_ELEM));
  PUT2(DataProcSimd_ByElementArithmetic4,
       NM("smlal2", "smull2"),
       OPS(VREG_4S, VREG_8H, SPECIFIC32),
       OPS(VREG_2D, VREG_4S, VREG_S_ELEM));
  /** C7.2.268 SQDMLAL, SQDMLAL2 (by element) */
  /** C7.2.270 SQDMLSL, SQDMLSL2 (by element) */
  /** C7.2.274 SQDMULL, SQDMULL2 (by element) */
  /** Scara varaint */
  /** Vector variant */
  PUT4(DataProcSimd_ByElementArithmetic5,
       NM("sqdmlal", "sqdmlsl", "sqdmull"),
       OPS(SREG, HREG, SPECIFIC32), OPS(DREG, SREG, VREG_S_ELEM),
       OPS(VREG_4S, VREG_4H, SPECIFIC32),
       OPS(VREG_2D, VREG_2S, VREG_S_ELEM));
  PUT2(DataProcSimd_ByElementArithmetic6,
       NM("sqdmlal2", "sqdmlsl2", "sqdmull2"),
       OPS(VREG_4S, VREG_8H, SPECIFIC32),
       OPS(VREG_2D, VREG_4S, VREG_S_ELEM));
  /** C7.2.272 SQDMULH (by element) */
  /** C7.2.279 SQRDMLSH (by elemetn) */
  /** C7.2.281 SQRDMULH (by element) */
  /** Scalar variant */
  /** Vector variatn */
  PUT6(DataProcSimd_ByElementArithmetic7,
       NM("sqdmulh", "sqrdmlsh", "sqrdmulh"),
       OPS(HREG, HREG, SPECIFIC32), OPS(SREG, SREG, VREG_S_ELEM),
       OPS(VREG_4H, VREG_4H, SPECIFIC32), OPS(VREG_8H, VREG_8H, SPECIFIC32),
       OPS(VREG_2S, VREG_2S, VREG_S_ELEM), OPS(VREG_4S, VREG_4S, VREG_S_ELEM));
  /** C7.2.280 SQRDMLSH (vector) */
  /** Scalar variant */
  /** Vector variant */
  PUT6(DataProcSimd_ByElementArithmetic8,
       NM("sqrdmlsh"),
       OPS(HREG, HREG, HREG), OPS(SREG, SREG, SREG),
       OPS(VREG_4H, VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H, VREG_8H),
       OPS(VREG_2S, VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S, VREG_4S));
  /** C7.2.349 UMLAL, UMLAL2 (by element) */
  /** C7.2.351 UMLSL, UMLSL2 (by element) */
  /** C7.2.354 UMULL, UMULL2 (by element) */
  /** Vecotr variant */
  PUT2(DataProcSimd_ByElementArithmetic9,
       NM("umlal", "umlsl", "umull"),
       OPS(VREG_4S, VREG_4H, SPECIFIC32),
       OPS(VREG_2D, VREG_2S, VREG_S_ELEM));
  PUT2(DataProcSimd_ByElementArithmetic10,
       NM("umlal2", "umlsl2", "umull2"),
       OPS(VREG_4S, VREG_8H, SPECIFIC32),
       OPS(VREG_2D, VREG_4S, VREG_S_ELEM));

  void putDataProcSimd_ByElementArithmetic() {
    std::vector<std::string> vreg = { "v7", "v0", "v1", "v2", "v4", "v8", "v15" };
    std::vector<std::string> vreg_elem = { "v6.8h[7]", "v0.8h[7]", "v1.8h[7]", "v2.8h[7]", "v4.8h[7]", "v8.8h[7]", "v15.8h[7]",
					   "v15.8h[0]", "v15.8h[1]", "v15.8h[2]", "v15.8h[4]" };
    std::vector<std::string> jvreg_elem = { "v6.h8[7]", "v0.h8[7]", "v1.h8[7]", "v2.h8[7]", "v4.h8[7]", "v8.h8[7]", "v15.h8[7]",
					    "v15.h8[0]", "v15.h8[1]", "v15.h8[2]", "v15.h8[4]" };
    
    /** FMLA instruction restricts Vreg 0-15. */
    tv_SPECIFIC32.clear();
    jtv_SPECIFIC32.clear();
    for(std::string i : vreg_elem) {
      tv_SPECIFIC32.push_back(i);
    }
    for(std::string i : jvreg_elem) {
      jtv_SPECIFIC32.push_back(i);
    }

    putDataProcSimd_ByElementArithmetic0();
    putDataProcSimd_ByElementArithmetic1();
    putDataProcSimd_ByElementArithmetic2();
    putDataProcSimd_ByElementArithmetic3();
    putDataProcSimd_ByElementArithmetic4();
    putDataProcSimd_ByElementArithmetic5();
    putDataProcSimd_ByElementArithmetic6();
    putDataProcSimd_ByElementArithmetic7();
    putDataProcSimd_ByElementArithmetic8();
    putDataProcSimd_ByElementArithmetic9();
    putDataProcSimd_ByElementArithmetic10();
  }



  /*** SIMD permute on page C3-221. */
  /** C7.2.36 EXT */
  /** Advanced SIMD variant */
  PUT2(DataProcSimd_Permute0,
       NM("ext"),
       OPS(VREG_8B, VREG_8B, VREG_8B, IMM3BIT), OPS(VREG_16B, VREG_16B, VREG_16B, IMM4BIT));
  /** C7.2.324 TRN1 */
  /** C7.2.325 TRN2 */
  /** C7.2.378 UZP1 */
  /** C7.2.379 UZP2 */
  /** C7.2.382 ZIP1 */
  /** C7.2.383 ZIP2 */
  /** Advanced SIMD varinat */
  PUT7(DataProcSimd_Permute1,
       NM("trn1", "trn2", "uzp1", "uzp2", "zip1", "zip2"),
       OPS(VREG_8B, VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B, VREG_16B),
       OPS(VREG_4H, VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H, VREG_8H),
       OPS(VREG_2S, VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S, VREG_4S),
       OPS(VREG_2D, VREG_2D, VREG_2D));

  void putDataProcSimd_Permute() {
    putDataProcSimd_Permute0();
    putDataProcSimd_Permute1();
  }    

  /*** SIMD immediate on page C3-221. */
  /** C7.2.13 BIC (vector, immediate) */
  /** C7.2.197 ORR (vector, immediate */
  /** 16-bit variant */
  /** 32-bit variant */
  PUT8(DataProcSimd_Immediate0,
       NM("bic"),
       OPS(VREG_4H, IMM8BIT), OPS(VREG_4H, IMM8BIT, SHIFT_AMOUNT32),
       OPS(VREG_8H, IMM8BIT), OPS(VREG_8H, IMM8BIT, SHIFT_AMOUNT32),
       OPS(VREG_2S, IMM8BIT), OPS(VREG_2S, IMM8BIT, SHIFT_AMOUNT64),
       OPS(VREG_4S, IMM8BIT), OPS(VREG_4S, IMM8BIT, SHIFT_AMOUNT64));
  /** C7.2.122 FMOV (vector, immediate) */
  /** Half-precison variatn */
  /** Single-precision variant */
  /** Double-precisoin variant */
  PUT7(DataProcSimd_Immediate1,
       NM("fmov"),
       OPS(VREG_4H, FLOAT8BIT), OPS(VREG_8H, FLOAT8BIT),
       OPS(VREG_2S, FLOAT8BIT), OPS(VREG_2S, FLOAT8BIT),
       OPS(VREG_4S, FLOAT8BIT), OPS(VREG_4S, FLOAT8BIT),
       OPS(VREG_2D, FLOAT8BIT));
  /** C7.2.189 MOVI */
  PUT8(DataProcSimd_Immediate2,
       NM("movi"),
       // 8-bit variant
       OPS(VREG_8B, IMM8BIT), OPS(VREG_8B, IMM8BIT, SPECIFIC32),
       OPS(VREG_16B, IMM8BIT), OPS(VREG_16B, IMM6BIT, SPECIFIC32),
       // 16-bit shifted immediate variant
       OPS(VREG_4H, IMM8BIT), OPS(VREG_4H, IMM8BIT, SHIFT_AMOUNT32),
       OPS(VREG_8H, IMM8BIT), OPS(VREG_8H, IMM8BIT, SHIFT_AMOUNT32));
  PUT8(DataProcSimd_Immediate3,
       NM("movi"),
       // 32-bit shifted immediate variatn
       OPS(VREG_2S, IMM8BIT), OPS(VREG_2S, IMM8BIT, SHIFT_AMOUNT64),
       OPS(VREG_4S, IMM8BIT), OPS(VREG_4S, IMM8BIT, SHIFT_AMOUNT64),
       // 32-bit shifting ones variant
       OPS(VREG_2S, IMM8BIT, SHIFT_AMOUNT32), OPS(VREG_4S, IMM8BIT, SHIFT_AMOUNT32),
       // 64-bit scalar variatn
       // 64-bit vector variant
       OPS(DREG, LSL32), OPS(VREG_2D, LSL32));
  /** C7.2.193 MVNI */
  PUT8(DataProcSimd_Immediate4,
       NM("mvni"),
       // 16-bit shifted immediate variant
       OPS(VREG_4H, IMM8BIT), OPS(VREG_4H, IMM8BIT, SHIFT_AMOUNT32),
       OPS(VREG_8H, IMM8BIT), OPS(VREG_8H, IMM8BIT, SHIFT_AMOUNT32),
       // 32-bit shifted immeidate variant
       OPS(VREG_2S, IMM8BIT), OPS(VREG_2S, IMM8BIT, SHIFT_AMOUNT64),
       // 32-bit shifting ones variant
       OPS(VREG_2S, IMM8BIT, SPECIFIC64), OPS(VREG_4S, IMM8BIT, SPECIFIC64));


  void putDataProcSimd_Immediate() {
    std::vector<std::string> tmp32 = { "8", "0" };
    std::vector<std::string> tmp64 = { "24", "0", "8", "16", "24" };
    std::vector<std::string> tmp8bit = { "0xff", "0xff00", "0x00ff0000", "0xff000000",
					 "0x00ff00000000", "0xff0000000000", "0x00ff000000000000", "0xff00000000000000" }; 

    tv_SPECIFIC32.clear();
    jtv_SPECIFIC32.clear();
    tv_SPECIFIC32.push_back("LSL 0");
    jtv_SPECIFIC32.push_back("LSL, 0");
    
    tv_SPECIFIC64.clear();
    jtv_SPECIFIC64.clear();
    tv_SPECIFIC64.push_back("MSL 8");
    tv_SPECIFIC64.push_back("MSL 16");
    jtv_SPECIFIC64.push_back("MSL, 8");
    jtv_SPECIFIC64.push_back("MSL, 16");


    tv_SHIFT_AMOUNT32.clear();
    jtv_SHIFT_AMOUNT32.clear();
    tv_SHIFT_AMOUNT64.clear();
    jtv_SHIFT_AMOUNT64.clear();

    for(std::string i : tmp32) {
      tv_SHIFT_AMOUNT32.push_back("LSL " + i);
      jtv_SHIFT_AMOUNT32.push_back("LSL, " + i);
    }

    for(std::string i : tmp64) {
      tv_SHIFT_AMOUNT64.push_back("LSL " + i);
      jtv_SHIFT_AMOUNT64.push_back("LSL, " + i);
    }

    tv_LSL32.clear();
    jtv_LSL32.clear();
    for(std::string i : tmp8bit) {
      tv_LSL32.push_back(i);
      jtv_LSL32.push_back(i);
    }
    
    putDataProcSimd_Immediate0();
    putDataProcSimd_Immediate1();
    putDataProcSimd_Immediate2();
    putDataProcSimd_Immediate3();
    putDataProcSimd_Immediate4();
  }    

  /*** SIMD shift (immediate) on page C3-221. */
  /************************************************************************/
  /* C3.5.20 SIMD shift (immediate) */
  /************************************************************************/
  /** C7.2.207 RSHRN, RSHRN2 */
  /** C7.2.241 SHRN, SHRN2 */
  /** Vector variant */
  PUT3(DataProcSimd_ShiftImmediate0,
       NM("rshrn", "shrn"),
       OPS(VREG_8B, VREG_8H, IMM3BIT_N),
       OPS(VREG_4H, VREG_4S, IMM4BIT_N),
       OPS(VREG_2S, VREG_2D, IMM5BIT_N));
  PUT3(DataProcSimd_ShiftImmediate1,
       NM("rshrn2", "shrn2"),
       OPS(VREG_16B, VREG_8H, IMM3BIT_N),
       OPS(VREG_8H, VREG_4S, IMM4BIT_N),
       OPS(VREG_4S, VREG_2D, IMM5BIT_N));
  /** C7.2.239 SHL */
  /** C7.2.243 SLI */
  /** Scalar variant */
  /** Vector variant */
  PUT8(DataProcSimd_ShiftImmediate2,
       NM("shl", "sli"),
       OPS(DREG, DREG, IMM6BIT),
       OPS(VREG_8B, VREG_8B, IMM3BIT), OPS(VREG_16B, VREG_16B, IMM3BIT),
       OPS(VREG_4H, VREG_4H, IMM4BIT), OPS(VREG_8H, VREG_8H, IMM4BIT),
       OPS(VREG_2S, VREG_2S, IMM5BIT), OPS(VREG_4S, VREG_4S, IMM4BIT),
       OPS(VREG_2D, VREG_2D, IMM6BIT));
  /** C7.2.240 SHLL, SHLL2 */
  /** C7.2.377 UXTL, UXTL2 */
  /** Vector variant */
  PUT3(DataProcSimd_ShiftImmediate3,
       NM("shll"),
       OPS(VREG_8H, VREG_8B, SPECIFIC32),
       OPS(VREG_4S, VREG_4H, SPECIFIC32_1),
       OPS(VREG_2D, VREG_2S, SPECIFIC32_2));
  PUT3(DataProcSimd_ShiftImmediate4,
       NM("shll2"),
       OPS(VREG_8H, VREG_16B, SPECIFIC32),
       OPS(VREG_4S, VREG_8H, SPECIFIC32_1),
       OPS(VREG_2D, VREG_4S, SPECIFIC32_2));
  /** C7.2.377 UXTL, UXTL2 */
  /** Vector variant */
  PUT3(DataProcSimd_ShiftImmediate5,
       NM("uxtl"),
       OPS(VREG_8H, VREG_8B),
       OPS(VREG_4S, VREG_4H),
       OPS(VREG_2D, VREG_2S));
  PUT3(DataProcSimd_ShiftImmediate6,
       NM("uxtl2"),
       OPS(VREG_8H, VREG_16B),
       OPS(VREG_4S, VREG_8H),
       OPS(VREG_2D, VREG_4S));
  /** C7.2.284 SQRSHRN, SQRSHRN2 */
  /** C7.2.285 SQRSHRUN, SQRSHRUN2 */
  /** C7.2.289 SQSHRN, SQSHRN2 */
  /** C7.2.290 SQSHRUN, SQSHRUN2 */
  /** C7.2.358 UQRSHRN, UQRSHRN2 */
  /** C7.2.361 UQSHRN, UQSHRN2 */
  /** Scalar variant */
  /** Vector variant */
  PUT6(DataProcSimd_ShiftImmediate7,
       NM("sqrshrn", "sqrshrun", "sqshrn", "sqshrun", "uqrshrn", "uqshrn"),
       OPS(BREG, HREG, IMM3BIT_N), OPS(HREG, SREG, IMM4BIT_N), OPS(SREG, DREG, IMM5BIT_N),
       OPS(VREG_8B, VREG_8H, IMM3BIT_N),
       OPS(VREG_4H, VREG_4S, IMM4BIT_N),
       OPS(VREG_2S, VREG_2D, IMM5BIT_N));
  PUT3(DataProcSimd_ShiftImmediate8,
       NM("sqrshrn2", "sqrshrun2", "sqshrn2", "sqshrun2", "uqrshrn2", "uqshrn2"),
       OPS(VREG_16B, VREG_8H, IMM3BIT_N),
       OPS(VREG_8H, VREG_4S, IMM4BIT_N),
       OPS(VREG_4S, VREG_2D, IMM5BIT_N));
  /** C7.2.286 SQSHL (immediate) */
  /** C7.2.288 SQSHLU */
  /** C7.2.359 UQSHL (immediate) */
  /** Scalar variant */
  /** Vector variant */
  PUT8(DataProcSimd_ShiftImmediate9,
       NM("sqshl", "sqshlu", "uqshl"),
       OPS(BREG, BREG, IMM3BIT), OPS(HREG, HREG, IMM4BIT),
       OPS(SREG, SREG, IMM5BIT), OPS(DREG, DREG, IMM6BIT),
       OPS(VREG_8B, VREG_8B, IMM3BIT), OPS(VREG_16B, VREG_16B, IMM3BIT),
       OPS(VREG_4H, VREG_4H, IMM4BIT), OPS(VREG_8H, VREG_8H, IMM4BIT));
  PUT3(DataProcSimd_ShiftImmediate10,
       NM("sqshl", "sqshlu", "uqshl"),
       OPS(VREG_2S, VREG_2S, IMM5BIT), OPS(VREG_4S, VREG_4S, IMM5BIT),
       OPS(VREG_2D, VREG_2D, IMM6BIT));
  /** C7.2.295 SRI */
  /** C7.2.297 SRSHR */
  /** C7.2.298 SRSRA */
  /** C7.2.301 SSHR */
  /** C7.2.302 SSRA */
  /** C7.2.367 URSHR */
  /** C7.2.369 URSRA */
  /** C7.2.372 USHR */
  /** C7.2.374 USRA */
  /** Scalar variant */
  /** Vector variant */
  PUT8(DataProcSimd_ShiftImmediate11,
       NM("sri", "srshr", "srsra", "sshr", "ssra", "urshr", "ursra", "usra"),
       OPS(DREG, DREG, IMM6BIT_N),
       OPS(VREG_8B, VREG_8B, IMM3BIT_N), OPS(VREG_16B, VREG_16B, IMM3BIT_N),
       OPS(VREG_4H, VREG_4H, IMM4BIT_N), OPS(VREG_8H, VREG_8H, IMM4BIT_N),
       OPS(VREG_2S, VREG_2S, IMM5BIT_N), OPS(VREG_4S, VREG_4S, IMM5BIT_N),
       OPS(VREG_2D, VREG_2D, IMM6BIT_N));
  /** C7.2.300 SSHLL, SSHLL2 */
  /** C7.2.371 USHLL, USHLL2 */
  /** Vector variant */
  PUT3(DataProcSimd_ShiftImmediate12,
       NM("sshll", "ushll"),
       OPS(VREG_8H, VREG_8B, IMM3BIT),
       OPS(VREG_4S, VREG_4H, IMM4BIT),
       OPS(VREG_2D, VREG_2S, IMM5BIT));
  PUT3(DataProcSimd_ShiftImmediate13,
       NM("sshll2", "ushll2"),
       OPS(VREG_8H, VREG_16B, IMM3BIT),
       OPS(VREG_4S, VREG_8H, IMM4BIT),
       OPS(VREG_2D, VREG_4S, IMM5BIT));
  /** C7.2.321 SXTL, SXTL2 */
  PUT3(DataProcSimd_ShiftImmediate14,
       NM("sxtl"),
       OPS(VREG_8H, VREG_8B),
       OPS(VREG_4S, VREG_4H),
       OPS(VREG_2D, VREG_2S));
  PUT3(DataProcSimd_ShiftImmediate15,
       NM("sxtl2"),
       OPS(VREG_8H, VREG_16B),
       OPS(VREG_4S, VREG_8H),
       OPS(VREG_2D, VREG_4S));

  void putDataProcSimd_ShiftImmediate() {
    tv_SPECIFIC32.clear();
    tv_SPECIFIC32_1.clear();
    tv_SPECIFIC32_2.clear();
    jtv_SPECIFIC32.clear();
    jtv_SPECIFIC32_1.clear();
    jtv_SPECIFIC32_2.clear();
    tv_SPECIFIC32.push_back("8");
    tv_SPECIFIC32_1.push_back("16");
    tv_SPECIFIC32_2.push_back("32");
    jtv_SPECIFIC32.push_back("8");
    jtv_SPECIFIC32_1.push_back("16");
    jtv_SPECIFIC32_2.push_back("32");

    putDataProcSimd_ShiftImmediate0();
    putDataProcSimd_ShiftImmediate1();
    putDataProcSimd_ShiftImmediate2();
    putDataProcSimd_ShiftImmediate3();
    putDataProcSimd_ShiftImmediate4();
    putDataProcSimd_ShiftImmediate5();
    putDataProcSimd_ShiftImmediate6();
    putDataProcSimd_ShiftImmediate7();
    putDataProcSimd_ShiftImmediate8();
    putDataProcSimd_ShiftImmediate9();
    putDataProcSimd_ShiftImmediate10();
    putDataProcSimd_ShiftImmediate11();
    putDataProcSimd_ShiftImmediate12();
    putDataProcSimd_ShiftImmediate13();
    putDataProcSimd_ShiftImmediate14();
    putDataProcSimd_ShiftImmediate15();
  }    

  /*** SIMD floating-point and integer conversion on page C3-222. */
  /************************************************************************/
  /* C3.5.21 SIMD floating-point and integer conversion */
  /************************************************************************/
  /** C7.2.63 FCVTAS (vector) */
  /** C7.2.65 FCVTAU (vector) */
  /** C7.2.68 FCVTMS (vector) */
  /** C7.2.70 FCVTMU (vector) */
  /** C7.2.73 FCVTNS (vector) */
  /** C7.2.75 FCVTNU (vector) */
  /** C7.2.77 FCVTPS (vector) */
  /** C7.2.79 FCVTPU (vector) */
  /** C7.2.83 FCVTZS (vector, integer) */
  /** C7.2.87 FCVTZU (vector, integer) */
  /** C7.2.219 SCVTF (vector, integer) */
  /** C7.2.336 UCVTF (vector, integer) */
  /** Scalar half precison variant */
  /** Scalar single-precision and double-precision variant */
  /** Vector half precision variant */
  /** Vector single-precision and double-precison variant */
  PUT8(DataProcSimd_FloatingPointAndIntergerConversion0,
       NM("fcvtas", "fcvtau", "fcvtms", "fcvtmu", "fcvtns", "fcvtnu", "fcvtps", "fcvtpu",
	  "fcvtzs", "fcvtzu", "scvtf", "ucvtf"),
       OPS(HREG, HREG), OPS(SREG, SREG), OPS(DREG, DREG),
       OPS(VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H), OPS(VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S),
       OPS(VREG_2D, VREG_2D));
  /** C7.2.82 FCVTZS (vector, fixed-point) */
  /** C7.2.86 FCVTZU (vector, fixed-point) */
  /** C7.2.218 SCVTF (vector, fixed-point) */
  /** C7.2.335 UCVTF (vector, fixed-point) */
  /** Scalar variant */
  /** Vector variant */
  PUT8(DataProcSimd_FloatingPointAndIntergerConversion1,
       NM("fcvtzs", "fcvtzu", "scvtf", "ucvtf"),
       OPS(HREG, HREG, IMM4BIT_N), OPS(SREG, SREG, IMM5BIT_N), OPS(DREG, DREG, IMM6BIT_N),
       OPS(VREG_4H, VREG_4H, IMM4BIT_N), OPS(VREG_8H, VREG_8H, IMM4BIT_N),
       OPS(VREG_2S, VREG_2S, IMM5BIT_N), OPS(VREG_2S, VREG_2S, IMM5BIT_N),
       OPS(VREG_2D, VREG_2D, IMM6BIT_N));

  void putDataProcSimd_FloatingPointAndIntergerConversion() {
    putDataProcSimd_FloatingPointAndIntergerConversion0();
    putDataProcSimd_FloatingPointAndIntergerConversion1();
  }
    
  /*** SIMD reduce (across vector lanes) on page C3-223. */
  /************************************************************************/
  /* C3.5.22 SIMD reduce (across vector lanes)                            */
  /************************************************************************/
  /** C7.2.6 ADDV */
  /** Advanced SIMD variant */
  PUT5(DataProcSimd_ReduceAcrossVectorLanes0,
       NM("addv"),
       OPS(BREG, VREG_8B), OPS(BREG, VREG_16B), OPS(HREG, VREG_4H), OPS(HREG, VREG_8H),
       OPS(SREG, VREG_4S));
  /** C7.2.100 FMAXNMV */
  /** C7.2.103 FMAXV */
  /** C7.2.110 FMINNMV */
  /** C7.2.113 FMINV */
  /** Half-precison variant */
  /** Single-precision and double-precison variant */
  PUT3(DataProcSimd_ReduceAcrossVectorLanes1,
       NM("fmaxnmv", "fmaxv", "fminnmv", "fminv"),
       // Half-precision variant
       // FMAXNMV <V><d>, <Vn>.<T>	 
       OPS(HREG, VREG_4H), OPS(HREG, VREG_8H),
       // Single-precision and double-precision variant
       // FMAXNMV <V><d>, <Vn>.<T>
       OPS(SREG, VREG_4S));
  /** C7.2.216 SADDLV */
  /** C7.2.333 UADDLV */
  /** Advanced SIMD variant */
  PUT5(DataProcSimd_ReduceAcrossVectorLanes2,
       NM("saddlv", "uaddlv"),
       OPS(HREG, VREG_8B), OPS(HREG, VREG_16B), OPS(SREG, VREG_4H), OPS(SREG, VREG_8H),
       OPS(DREG, VREG_4S));
  /** C7.2.255 SMAXV */
  /** C7.2.258 SMINV */
  /** C7.2.345 UMAXV */
  /** C7.2.348 UMINV */
  /** Advanced SIMD variant */
  PUT5(DataProcSimd_ReduceAcrossVectorLanes3,
       NM("smaxv", "sminv", "umaxv", "uminv"),
       OPS(BREG, VREG_8B), OPS(BREG, VREG_16B), OPS(HREG, VREG_4H), OPS(HREG, VREG_8H),
       OPS(SREG, VREG_4S));
  void putDataProcSimd_ReduceAcrossVectorLanes() {
    putDataProcSimd_ReduceAcrossVectorLanes0();
    putDataProcSimd_ReduceAcrossVectorLanes1();
    putDataProcSimd_ReduceAcrossVectorLanes2();
    putDataProcSimd_ReduceAcrossVectorLanes3();
  }    


  /*** SIMD pairwise arithmetic on page C3-224. */
  /************************************************************************/
  /* C3.5.23 SIMD pairwise arithmetic                                     */
  /************************************************************************/
  /** C7.2.5 ADDP (vector) */
  /** Three registers of the same type variant */
  PUT7(DataProcSimd_PairwiseArithmetic0,
       NM("addp"),
       OPS(VREG_8B, VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B, VREG_16B),
       OPS(VREG_4H, VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H, VREG_8H),
       OPS(VREG_2S, VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S, VREG_4S),
       OPS(VREG_2D, VREG_2D, VREG_2D));
  /** C7.2.4 ADDP (scalar) */
  PUT1(DataProcSimd_PairwiseArithmetic1,
       NM("addp"),
       OPS(DREG, VREG_2D));
  /** C7.2.45 FADDP (vector) */
  /** C7.2.99 FMAXNMP (vector) */
  /** C7.2.102 FMAXP (vector) */
  /** C7.2.109 FMINNMP (vector) */
  /** C7.2.112 FMINP (vector) */
  /** Half-precision variant */
  /** Single-precision and double-precison */
  PUT5(DataProcSimd_PairwiseArithmetic2,
       NM("faddp", "fmaxnmp", "fmaxp", "fminnmp", "fminp"),
       OPS(VREG_4H, VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H, VREG_8H),
       OPS(VREG_2S, VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S, VREG_4S),
       OPS(VREG_2D, VREG_2D, VREG_2D));
  /** C7.2.44 FADDP (scalar) */
  /** C7.2.98 FMAXNMP (scalar) */
  /** C7.2.101 FMAXP (scalar) */
  /** C7.2.108 FMINNMP (scalar) */
  /** C7.2.111 FMINP (scalar) */
  /** half-precison variant */
  /** Single-precision and double-precison */
  PUT3(DataProcSimd_PairwiseArithmetic3,
       NM("faddp", "fmaxnmp", "fmaxp", "fminnmp"),
       OPS(HREG, VREG_2H), OPS(SREG, VREG_2S), OPS(DREG, VREG_2D));
  /** C7.2.254 SMAXP */
  /** C7.2.257 SMINP */
  /** C7.2.344 UMAXP */
  /** C7.2.347 UMINP */
  /** Three registers of the same type variant */
  PUT6(DataProcSimd_PairwiseArithmetic4,
       NM("smaxp", "sminp", "umaxp", "uminp"),
       OPS(VREG_8B, VREG_8B, VREG_8B), OPS(VREG_16B, VREG_16B, VREG_16B),
       OPS(VREG_4H, VREG_4H, VREG_4H), OPS(VREG_8H, VREG_8H, VREG_8H),
       OPS(VREG_2S, VREG_2S, VREG_2S), OPS(VREG_4S, VREG_4S, VREG_4S));

  void  putDataProcSimd_PairwiseArithmetic() {
    putDataProcSimd_PairwiseArithmetic0();
    putDataProcSimd_PairwiseArithmetic1();
    putDataProcSimd_PairwiseArithmetic2();
    putDataProcSimd_PairwiseArithmetic3();
    putDataProcSimd_PairwiseArithmetic4();
  }

  /*** SIMD dot product on page C3-225. */
  /************************************************************************/
  /* C3.5.24 SIMD dot product                                             */
  /************************************************************************/
  /** C7.2.223 SDOT (vector) */
  /** C7.2.340 UDOT (vector) */
  /** Three register of the same type variant */
  PUT2(DataProcSimd_DotProduct0,
       NM("sdot", "udot"),
       OPS(VREG_2S, VREG_8B, VREG_8B), OPS(VREG_4S, VREG_16B, VREG_16B));
  /** C7.2.222 SDOT (by element) */
  /** C7.2.339 UDOT (by element) */
  PUT2(DataProcSimd_DotProduct1,
       NM("sdot", "udot"),
       OPS(VREG_2S, VREG_8B, SPECIFIC32), OPS(VREG_4S, VREG_16B, SPECIFIC32));

  void putDataProcSimd_DotProduct() {
    std::vector<std::string> tmpReg = { "v15", "v0", "v1", "v2", "v4", "v8", "v16", "v31" };
    std::vector<std::string> tmpIdx = { "4b[3]", "4b[0]", "4b[1]", "4b[2]" };
    std::vector<std::string> jtmpIdx = { "b4[3]", "b4[0]", "b4[1]", "b4[2]" };

    /** Vector variant
	SDOT <Vd>.<Ta>, <Vn>.<Tb>, <Vm>.4B[<index>] */
    tv_SPECIFIC32.clear();
    jtv_SPECIFIC32.clear();

    for(std::string i : tmpReg) {
      tv_SPECIFIC32.push_back(i + "." + tmpIdx[0]);
      jtv_SPECIFIC32.push_back(i + "." + jtmpIdx[0]);
    }

    for(std::string i : tmpIdx) {
      tv_SPECIFIC32.push_back(tmpReg[0] + "." + i);
    }
    for(std::string i : jtmpIdx) {
      jtv_SPECIFIC32.push_back(tmpReg[0] + "." + i);
    }
      
    putDataProcSimd_DotProduct0();
    putDataProcSimd_DotProduct1();
  }

  /*** SIMD table lookup on page C3-225. */
  /************************************************************************/
  /* C3.5.25 SIMD table lookup                                            */
  /************************************************************************/
  /** C7.2.322 TBL */
  /** C7.2.323 TBX */
  /** Two register table variant */
  /** Threee register table variant */
  /** Four register table varinat */
  /** Single register table variant */
  PUT2(DataProcSimd_TableLookup0,
       NM("tbl", "tbx"),
       OPS(VREG_8B, SPECIFIC32, VREG_8B), OPS(VREG_16B, SPECIFIC32, VREG_16B));

  void putDataProcSimd_TableLookup() {
    std::vector<int> tmpReg = { 15, 0, 1, 2, 4, 8, 16, 31 };

    tv_SPECIFIC32.clear();
    jtv_SPECIFIC32.clear();

    /** Single register */
    for(int i: tmpReg) {
      tv_SPECIFIC32.push_back("{ v" + std::to_string(i) + ".16b }");
      jtv_SPECIFIC32.push_back("v" + std::to_string(i) + ".b16, 1");
    }
    
    /** Two register */
    for(int i: tmpReg) {
      tv_SPECIFIC32.push_back("{ v" + std::to_string(i)
			      + ".16b, v" + std::to_string((i+1) % 32) + ".16b }");
      jtv_SPECIFIC32.push_back("v" + std::to_string(i) + ".b16, 2");
    }

    /** Three register */
    for(int i: tmpReg) {
      tv_SPECIFIC32.push_back("{ v" + std::to_string(i)
			      + ".16b, v" + std::to_string((i+1) % 32)
			      + ".16b, v" + std::to_string((i+2) % 32) + ".16b }");
      jtv_SPECIFIC32.push_back("v" + std::to_string(i) + ".b16, 3");
    }

    /** Three register */
    for(int i: tmpReg) {
      tv_SPECIFIC32.push_back("{ v" + std::to_string(i)
			      + ".16b, v" + std::to_string((i+1) % 32)
			      + ".16b, v" + std::to_string((i+2) % 32)
			      + ".16b, v" + std::to_string((i+3) % 32) + ".16b }");
      jtv_SPECIFIC32.push_back("v" + std::to_string(i) + ".b16, 4");
    }

    putDataProcSimd_TableLookup0();
  }


  /*** SIMD complex number arithmetic on page C3-225. */
  /************************************************************************/
  /* C3.5.26 SIMD complex number arithmetic                               */
  /************************************************************************/
  /** C7.2.46 FCADD */
  /* Three registers of the same type variant */
  PUT5(DataProcSimd_ComplexNumberArithmetic0,
       NM("fcadd"),
       OPS(VREG_4H, VREG_4H, VREG_4H, SPECIFIC32), OPS(VREG_8H, VREG_8H, VREG_8H, SPECIFIC32),
       OPS(VREG_2S, VREG_2S, VREG_2S, SPECIFIC32), OPS(VREG_4S, VREG_4S, VREG_4S, SPECIFIC32),
       OPS(VREG_2D, VREG_2D, VREG_2D, SPECIFIC32));
  /** C7.2.56 FCMLA */
  /* Three registers of the smae type variant */
  PUT5(DataProcSimd_ComplexNumberArithmetic1,
       NM("fcmla"),
       OPS(VREG_4H, VREG_4H, VREG_4H, SPECIFIC64), OPS(VREG_8H, VREG_8H, VREG_8H, SPECIFIC64),
       OPS(VREG_2S, VREG_2S, VREG_2S, SPECIFIC64), OPS(VREG_4S, VREG_4S, VREG_4S, SPECIFIC64),
       OPS(VREG_2D, VREG_2D, VREG_2D, SPECIFIC64));
  /** C7.2.55 FCMLA (by element) */
  PUT3(DataProcSimd_ComplexNumberArithmetic2,
       NM("fcmla"),
       OPS(VREG_4H, VREG_4H, SPECIFIC32_1, SPECIFIC64), OPS(VREG_8H, VREG_8H, SPECIFIC32_1, SPECIFIC64),
       OPS(VREG_4S, VREG_4S, SPECIFIC32_2, SPECIFIC64));
  void putDataProcSimd_ComplexNumberArithmetic() {
    tv_SPECIFIC32.clear();
    tv_SPECIFIC64.clear();
    jtv_SPECIFIC32.clear();
    jtv_SPECIFIC64.clear();

    tv_SPECIFIC32.push_back("90");
    tv_SPECIFIC32.push_back("270");
    jtv_SPECIFIC32.push_back("90");
    jtv_SPECIFIC32.push_back("270");

    tv_SPECIFIC64.push_back("0");
    tv_SPECIFIC64.push_back("90");
    tv_SPECIFIC64.push_back("180");
    tv_SPECIFIC64.push_back("270");
    jtv_SPECIFIC64.push_back("0");
    jtv_SPECIFIC64.push_back("90");
    jtv_SPECIFIC64.push_back("180");
    jtv_SPECIFIC64.push_back("270");



    /** FCMLA <Vd>.<T>, <Vn>.<T>, <Vm>.<Ts>[<index>], #<rotate>
	<index> Is the element index, encoded in the "size:H:L" field. It can have the following values:
	H:L when size = 01
	H when size = 10 */
    //    std::vector<std::string> tmpReg = { "v15", "v0", "v1", "v2", "v4", "v8", "v16", "v31" };
    std::vector<std::string> tmpReg = { "v15", "v0", "v1", "v2", "v4", "v8" };

    tv_SPECIFIC32_1.clear();
    tv_SPECIFIC32_2.clear();
    jtv_SPECIFIC32_1.clear();
    jtv_SPECIFIC32_2.clear();

    for(std::string i : tmpReg) {
      //      tv_SPECIFIC32_1.push_back(i + ".4h[3]");
      tv_SPECIFIC32_1.push_back(i + ".4h[0]");
      tv_SPECIFIC32_1.push_back(i + ".4h[1]");
      //      tv_SPECIFIC32_1.push_back(i + ".4h[2]");

      //      jtv_SPECIFIC32_1.push_back(i + ".h4[3]");
      jtv_SPECIFIC32_1.push_back(i + ".h4[0]");
      jtv_SPECIFIC32_1.push_back(i + ".h4[1]");
      //      jtv_SPECIFIC32_1.push_back(i + ".h4[2]");

      tv_SPECIFIC32_2.push_back(i + ".s[1]");
      tv_SPECIFIC32_2.push_back(i + ".s[0]");
      jtv_SPECIFIC32_2.push_back(i + ".s2[1]");
      jtv_SPECIFIC32_2.push_back(i + ".s2[0]");

      //      tv_SPECIFIC32_2.push_back(i + ".2s");
      //      tv_SPECIFIC32_2.push_back(i + ".2s");
      //      jtv_SPECIFIC32_2.push_back(i + ".s2");
      //      jtv_SPECIFIC32_2.push_back(i + ".s2");

    }      
    
    putDataProcSimd_ComplexNumberArithmetic0();
    putDataProcSimd_ComplexNumberArithmetic1();
    putDataProcSimd_ComplexNumberArithmetic2();
  }

  
  /************************************************************************/
  /* C3.5.27 The Cryptographic Extension                                  */
  /************************************************************************/
  /** C7.2.7 AESD */
  /** C7.2.8 AESE */
  /** C7.2.9 AESIMC */
  /** C7.2.10 AESMC */
  /** Adavanced SIMD variant */
  PUT1(DataProcSimd_CryptographicExtension0,
       NM("aesd", "aese", "aesimc", "aesmc"),
       OPS(VREG_16B, VREG_16B));
  /** C7.2.200 PMULL, PMULL2 is already tested. */
  /** C7.2.224 SHA1C */
  /** C7.2.226 SHA1M */
  /** C7.2.227 SHA1P */
  /** Advanced SIMD variant */
  PUT1(DataProcSimd_CryptographicExtension1,
       NM("sha1c", "sha1m", "sha1p"),
       OPS(QREG, SREG, VREG_4S));
  /** C7.2.225 SHA1H */
  /** Advanced SIMD variant */
  PUT1(DataProcSimd_CryptographicExtension2,
       NM("sha1h"),
       OPS(SREG, SREG));
  /** C7.2.228 SHA1SU0 */
  /** C7.2.233 SHA256SU1 */
  /** C7.2.244 SM3PARTW1 */
  /** C7.2.245 SM3PARTW2 */
  /** Advanced SIMD variant */
  PUT1(DataProcSimd_CryptographicExtension3,
       NM("sha1su0", "sha256su1", "sm3partw1", "sm3partw2"),
       OPS(VREG_4S, VREG_4S, VREG_4S));
  /** C7.2.229 SHA1SU1 */
  /** C7.2.232 SHA256SU0 */
  /** Advanced SIMD variant */
  PUT1(DataProcSimd_CryptographicExtension4,
       NM("sha1su1", "sha256su0"),
       OPS(VREG_4S, VREG_4S));
  /** C7.2.231 SHA256H */
  /** C7.2.230 SHA256H2 */
  /** Advanced SIMD variant */
  PUT1(DataProcSimd_CryptographicExtension5,
       NM("sha256h", "sha256h2"),
       OPS(QREG, QREG, VREG_4S));
  /** C7.2.234 SHA512H */
  /** C7.2.235 SHA512H2 */
  /** Advanced SIMD variant */
  PUT1(DataProcSimd_CryptographicExtension6,
       NM("sha512h", "sha512h2"),
       OPS(QREG, QREG, VREG_2D));
  /** C7.2.236 SHA512SU0 */
  /** Advanced SIMD variant */
  PUT1(DataProcSimd_CryptographicExtension7,
       NM("sha512su0"),
       OPS(VREG_2D, VREG_2D));
  /** C7.2.237 SHA512SU1 */
  /** Advanced SIMD variant */
  PUT1(DataProcSimd_CryptographicExtension8,
       NM("sha512su1"),
       OPS(VREG_2D, VREG_2D, VREG_2D));
  /** C7.2.35 EOR3 */
  /** C7.2.12 BCAX */
  /** Advanced SIMD variant */
  PUT1(DataProcSimd_CryptographicExtension9,
       NM("eor3", "bcax"),
       OPS(VREG_16B, VREG_16B, VREG_16B, VREG_16B));
  /** C7.2.202 RAX1 */
  /** Advanced SIMD variant */
  PUT1(DataProcSimd_CryptographicExtension10,
       NM("rax1"),
       OPS(VREG_2D, VREG_2D, VREG_2D));
  /** C7.2.380 XAR */
  /** Advanced SIMD variant */
  PUT1(DataProcSimd_CryptographicExtension11,
       NM("xar"),
       OPS(VREG_2D, VREG_2D, VREG_2D, IMM6BIT));
  /** C7.2.246 SM3SS1 */
  /** Advanced SIMD variant */
  PUT1(DataProcSimd_CryptographicExtension12,
       NM("sm3ss1"),
       OPS(VREG_4S, VREG_4S, VREG_4S, VREG_4S));
  /** C7.2.247 SM3TT1A */
  /** C7.2.248 SM3TT1B */
  /** C7.2.249 SM3TT2A */
  /** C7.2.250 SM3TT2B */
  /** Advanced SIMD variant */
  PUT1(DataProcSimd_CryptographicExtension13,
       NM("sm3tt1a", "sm3tt1b", "sm3tt2a", "sm3tt2b"),
       OPS(VREG_4S, VREG_4S, VREG_S_ELEM));
  void putDataProcSimd_CryptographicExtension() {
    putDataProcSimd_CryptographicExtension0();
    putDataProcSimd_CryptographicExtension1();
    putDataProcSimd_CryptographicExtension2();
    putDataProcSimd_CryptographicExtension3();
    putDataProcSimd_CryptographicExtension4();
    putDataProcSimd_CryptographicExtension5();
    putDataProcSimd_CryptographicExtension6();
    putDataProcSimd_CryptographicExtension7();
    putDataProcSimd_CryptographicExtension8();
    putDataProcSimd_CryptographicExtension9();
    putDataProcSimd_CryptographicExtension10();
    putDataProcSimd_CryptographicExtension11();
    putDataProcSimd_CryptographicExtension12();
    putDataProcSimd_CryptographicExtension13();
  }

  
  /*      class Ops {
	  public:
	  Ops();
	  ~Ops();
	  pushNm(std::string) {
	  
	  };
	  pushOp1();
	  pushOp2();
	  pushOp3();
	  pushOp4();
	  pushOp5();
	  private:
	  std::vector<std::string> nm;
	  std::vector<std::string> op1;
	  std::vector<std::string> op2;
	  std::vector<std::string> op3;
	  std::vector<std::string> op4;
	  std::vector<std::string> op5;
	  }
  */

  
  void putDataProcSimd()
  {
    putDataProcSimd_Move();
    putDataProcSimd_Arithmetic();
    putDataProcSimd_Compare();
    putDataProcSimd_WideningAndNarrowingArithmetic();
    putDataProcSimd_UnaryArithmetic();
    putDataProcSimd_ByElementArithmetic();
    putDataProcSimd_Permute();
    putDataProcSimd_Immediate();
    putDataProcSimd_ShiftImmediate();
    putDataProcSimd_FloatingPointAndIntergerConversion();
    putDataProcSimd_ReduceAcrossVectorLanes();
    putDataProcSimd_PairwiseArithmetic();
    putDataProcSimd_DotProduct();
    putDataProcSimd_TableLookup();
    putDataProcSimd_ComplexNumberArithmetic();
    putDataProcSimd_CryptographicExtension();
  }

  void putDataProcSimdFp()
  {
    putDataProcSimd();
  }
  
  void put()
  {
    putDataProcSimdFp();
    
    //    Ops hoge();
    //    hoge.pushNm({"add", "sub"});
    
    
  }
  
};

  
int main(int argc, char *[])
{
  if(flagBit >= bitEnd) {
    std::cerr << "Test vector variation must be less than " << bitEnd << std::endl;
    return 1;
  }

  Test test(argc > 1);
  test.put();
}
