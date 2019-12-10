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

enum TpPredType {
		 TP_z,  /** Zeroing is allowed. */
		 TP_m,  /** Merging is allowed. */
		 TP_zm, /** Both of zeroing and merging are allowed. */
		 TP_NONE /** No indication. */
};

uint64_t flagBit = 0;
const uint64_t WREG  = flagBit++; /** Test vector is {w0, w1, ..., w30 } */
const uint64_t XREG  = flagBit++; /** Test vector is {x0, x1, ..., x30 } */
const uint64_t VREG  = flagBit++; /** Test vector is {v0, v1, ..., v31 } */
const uint64_t WSP   = flagBit++; /** Test vector is { wsp } */
const uint64_t XSP   = flagBit++; /** Test vector is { sp } */
const uint64_t WNSP  = flagBit++;
const uint64_t XNSP  = flagBit++;
const uint64_t IMM0BIT   = flagBit++; /** Test vector is {0} */
const uint64_t IMM1BIT   = flagBit++; /** Test vector is {0, 1} */
const uint64_t IMM2BIT   = flagBit++; /** Test vector is {0, 1, 2, 3 } */
const uint64_t IMM3BIT   = flagBit++; /** Test vector is {0, 1, 2, 4, 7 } */
const uint64_t IMM4BIT   = flagBit++; /** Test vector is {0, 1, ..., 8, 15 } */
const uint64_t IMM5BIT   = flagBit++; /** Test vector is {0, 1, ..., 16, 31 } */
const uint64_t IMM6BIT   = flagBit++; /** Test vector is {0, 1, ..., 32, 63 } */
const uint64_t IMM8BIT   = flagBit++; /** Test vector is {0, 1, ..., 128, 255 } */
const uint64_t IMM12BIT  = flagBit++; /** Test vector is {0, 1, ..., 2048, 4095 } */
const uint64_t IMM13BIT  = flagBit++; /** Test vector is {0, 1, ..., 4096, 8191 } */
const uint64_t IMM16BIT  = flagBit++; /** Test vector is {0, 1, ..., 4096, 1<<13, 1<<14, 1<<15, 1<<16-1 } */
const uint64_t IMM3BIT_N = flagBit++; /** Test vector is {1, 2, .., 8 } */
const uint64_t IMM4BIT_N = flagBit++; /** Test vector is {1, 2, .., 16 } */
const uint64_t IMM5BIT_N = flagBit++; /** Test vector is {1, 2, .., 32 } */
const uint64_t IMM6BIT_N = flagBit++; /** Test vector is {1, 2, .., 64 } */
const uint64_t FLOAT8BIT = flagBit++; /** Test vector is Table C-2- Floating-point constant values */

const uint64_t ZREG = flagBit++; /** Test vector is { z0, z1, ..., z31 } */
const uint64_t SPECIFIC32 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_1 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_1 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_2 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_2 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_3 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_3 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_4 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_4 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_5 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_5 = flagBit++; /** Test vector is generated on the fly. */

const uint64_t SPECIFIC0 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC1 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC2 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC3 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC4 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC5 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC6 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC7 = flagBit++; /** Test vector is generated on the fly. */

const uint64_t ZREG_B   = flagBit++;
const uint64_t ZREG_H   = flagBit++;
const uint64_t ZREG_S   = flagBit++;
const uint64_t ZREG_D   = flagBit++;
const uint64_t ZREG_Q   = flagBit++;

const uint64_t PREG_B   = flagBit++;
const uint64_t PREG_H   = flagBit++;
const uint64_t PREG_S   = flagBit++;
const uint64_t PREG_D   = flagBit++;

const uint64_t PG_M     = flagBit++;
const uint64_t PG_Z     = flagBit++;
const uint64_t PG       = flagBit++;
const uint64_t PG_ZM    = flagBit++;

const uint64_t PTR_O    = flagBit++;
const uint64_t PTR_C    = flagBit++;

const uint64_t BRA_O    = flagBit++; /** Test vector is { "{" } */
const uint64_t BRA_C    = flagBit++; /** Test vector is { "}" } */

const uint64_t XTW      = flagBit++;
const uint64_t PRFOP    = flagBit++;
const uint64_t PRFOP_SVE= flagBit++;
const uint64_t MULVL    = flagBit++;


const uint64_t NOPARA = 100000;


#define PUT0(name, nm)					\
  void put##name() const				\
  {							\
    std::vector<std::string> nemonic(nm);		\
    put(nemonic, #name, 0);			\
  }							\
  
#define PUT1(name, nm, op_1)				\
  void put##name() const				\
  {							\
    std::vector<std::string> nemonic(nm);		\
    std::vector<uint64_t> op1(op_1);			\
    put(nemonic, op1, #name, 0);			\
  }							\
  
#define PUT2(name, nm, op_1, op_2)			\
  void put##name() const				\
  {							\
    std::vector<std::string> nemonic(nm);		\
    std::vector<uint64_t> op1(op_1);			\
    std::vector<uint64_t> op2(op_2);			\
    put(nemonic, op1, #name, 0);			\
    put(nemonic, op2, #name, 1);				\
  }							\

#define PUT3(name, nm, op_1, op_2, op_3)		\
  void put##name() const				\
  {							\
    std::vector<std::string> nemonic(nm);		\
    std::vector<uint64_t> op1(op_1);			\
    std::vector<uint64_t> op2(op_2);			\
    std::vector<uint64_t> op3(op_3);			\
    put(nemonic, op1, #name, 0);			\
    put(nemonic, op2, #name, 1);				\
    put(nemonic, op3, #name, 2);					\
  }							\

#define PUT4(name, nm, op_1, op_2, op_3, op_4)		\
  void put##name() const				\
  {							\
    std::vector<std::string> nemonic(nm);		\
    std::vector<uint64_t> op1(op_1);			\
    std::vector<uint64_t> op2(op_2);			\
    std::vector<uint64_t> op3(op_3);			\
    std::vector<uint64_t> op4(op_4);			\
    put(nemonic, op1, #name, 0);					\
    put(nemonic, op2, #name, 1);					\
    put(nemonic, op3, #name, 2);					\
    put(nemonic, op4, #name, 3);					\
  }							\

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
  std::vector<std::string> tv_WSP  = { "wsp" };
  std::vector<std::string> tv_XSP  = { "sp" };
  std::vector<std::string> tv_WNSP  = { "w7", "w0", "w1", "w2", "w4", "w8", "w16", "w30", "wsp"};
  std::vector<std::string> tv_XNSP  = { "x7", "x0", "x1", "x2", "x4", "x8", "x16", "x30", "sp"};
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

  std::vector<std::string> tv_ZREG = { "z7", "z0", "z1", "z2", "z4", "z8", "z16", "z31" };
  std::vector<std::string> tv_SPECIFIC32, tv_SPECIFIC64, tv_SPECIFIC32_1, tv_SPECIFIC64_1, tv_SPECIFIC32_2, tv_SPECIFIC64_2,
    tv_SPECIFIC32_3, tv_SPECIFIC64_3, tv_SPECIFIC32_4, tv_SPECIFIC64_4, tv_SPECIFIC32_5, tv_SPECIFIC64_5;
  std::vector<std::string> jtv_SPECIFIC32, jtv_SPECIFIC64, jtv_SPECIFIC32_1, jtv_SPECIFIC64_1, jtv_SPECIFIC32_2, jtv_SPECIFIC64_2,
    jtv_SPECIFIC32_3, jtv_SPECIFIC64_3, jtv_SPECIFIC32_4, jtv_SPECIFIC64_4, jtv_SPECIFIC32_5, jtv_SPECIFIC64_5;

  std::vector<std::string> tv_SPECIFIC0, tv_SPECIFIC1, tv_SPECIFIC2, tv_SPECIFIC3, tv_SPECIFIC4, tv_SPECIFIC5, tv_SPECIFIC6, tv_SPECIFIC7;
  std::vector<std::string> jtv_SPECIFIC0, jtv_SPECIFIC1, jtv_SPECIFIC2, jtv_SPECIFIC3, jtv_SPECIFIC4, jtv_SPECIFIC5, jtv_SPECIFIC6, jtv_SPECIFIC7;
  
  std::vector<std::string> tv_ZREG_B = { "z7.b", "z0.b", "z1.b", "z2.b", "z4.b", "z8.b", "z16.b", "z31.b" };
  std::vector<std::string> tv_ZREG_H = { "z7.h", "z0.h", "z1.h", "z2.h", "z4.h", "z8.h", "z16.h", "z31.h" };
  std::vector<std::string> tv_ZREG_S = { "z7.s", "z0.s", "z1.s", "z2.s", "z4.s", "z8.s", "z16.s", "z31.s" };
  std::vector<std::string> tv_ZREG_D = { "z7.d", "z0.d", "z1.d", "z2.d", "z4.d", "z8.d", "z16.d", "z31.d" };
  std::vector<std::string> tv_ZREG_Q = { "z7.q", "z0.q", "z1.q", "z2.q", "z4.q", "z8.q", "z16.q", "z31.q" };

  std::vector<std::string> tv_PREG_B = { "p7.b", "p0.b", "p1.b", "p2.b", "p4.b" };
  std::vector<std::string> tv_PREG_H = { "p7.h", "p0.h", "p1.h", "p2.h", "p4.h" };
  std::vector<std::string> tv_PREG_S = { "p7.s", "p0.s", "p1.s", "p2.s", "p4.s" };
  std::vector<std::string> tv_PREG_D = { "p7.d", "p0.d", "p1.d", "p2.d", "p4.d" };

  
  std::vector<std::string> tv_PG_M = { "p7/m", "p0/m", "p1/m", "p2/m", "p4/m" };
  std::vector<std::string> tv_PG_Z = { "p7/z", "p0/z", "p1/z", "p2/z", "p4/z" };
  std::vector<std::string> tv_PG_ZM = { "p7/m", "p0/m", "p1/m", "p2/m", "p4/m",
					"p7/z", "p0/z", "p1/z", "p2/z", "p4/z" };
  std::vector<std::string> jtv_PG_M = { "p7/T_m", "p0/T_m", "p1/T_m", "p2/T_m", "p4/T_m" };
  std::vector<std::string> jtv_PG_Z = { "p7/T_z", "p0/T_z", "p1/T_z", "p2/T_z", "p4/T_z" };
  std::vector<std::string> jtv_PG_ZM = { "p7/T_m", "p0/T_m", "p1/T_m", "p2/T_m", "p4/T_m",
					 "p7/T_z", "p0/T_z", "p1/T_z", "p2/T_z", "p4/T_z" };
  std::vector<std::string> tv_PG   = { "p7", "p0", "p1", "p2", "p4" };
  std::vector<std::string> tv_PTR_O = { "[" };
  std::vector<std::string> tv_PTR_C = { "]" };
  std::vector<std::string> jtv_PTR_O = { "ptr(" };
  std::vector<std::string> jtv_PTR_C = { ")" };

  std::vector<std::string> tv_BRA_O = { "{" };
  std::vector<std::string> tv_BRA_C = { "}" };
  std::vector<std::string> jtv_BRA_O = { "" };
  std::vector<std::string> jtv_BRA_C = { "" };

  
  std::vector<std::string> tv_XTW  = { "UXTW", "SXTW" };
  std::vector<std::string> jtv_XTW = { "UXT", "SXT" };

  std::vector<std::string> tv_PRFOP = { "PLDL1KEEP", "PLDL1STRM", "PLDL2KEEP", "PLDL2STRM", "PLDL3KEEP", "PLDL3STRM",
					"PSTL1KEEP", "PSTL1STRM", "PSTL2KEEP", "PSTL2STRM", "PSTL3KEEP", "PSTL3STRM",
					"PLIL1KEEP", "PLIL1STRM", "PLIL2KEEP", "PLIL2STRM", "PLIL3KEEP", "PLIL3STRM" };
  std::vector<std::string> tv_PRFOP_SVE = { "PLDL1KEEP", "PLDL1STRM", "PLDL2KEEP", "PLDL2STRM",
				     "PLDL3KEEP", "PLDL3STRM", "PSTL1KEEP", "PSTL1STRM",
				     "PSTL2KEEP", "PSTL2STRM", "PSTL3KEEP", "PSTL3STRM" };
  std::vector<std::string> jtv_PRFOP_SVE = { "PLDL1KEEP_SVE", "PLDL1STRM_SVE", "PLDL2KEEP_SVE", "PLDL2STRM_SVE",
				     "PLDL3KEEP_SVE", "PLDL3STRM_SVE", "PSTL1KEEP_SVE", "PSTL1STRM_SVE",
				     "PSTL2KEEP_SVE", "PSTL2STRM_SVE", "PSTL3KEEP_SVE", "PSTL3STRM_SVE" };

  std::vector<std::string> tv_MUL_VL = { "MUL VL" };
  std::vector<std::string> jtv_MUL_VL = { "MUL_VL" };
  
  std::vector<std::vector<std::string> *> tv_VectorsAs = { &tv_WREG, &tv_XREG, &tv_VREG,
							   &tv_WSP, &tv_XSP, &tv_WNSP, &tv_XNSP,
							   &tv_IMM0BIT, &tv_IMM1BIT, &tv_IMM2BIT, &tv_IMM3BIT, &tv_IMM4BIT, &tv_IMM5BIT, &tv_IMM6BIT, &tv_IMM8BIT,
							   &tv_IMM12BIT, &tv_IMM13BIT, &tv_IMM16BIT,
							   &tv_IMM3BIT_N, &tv_IMM4BIT_N, &tv_IMM5BIT_N, &tv_IMM6BIT_N, &tv_FLOAT8BIT,
							   &tv_ZREG,
							   &tv_SPECIFIC32, &tv_SPECIFIC64, &tv_SPECIFIC32_1, &tv_SPECIFIC64_1,
							   &tv_SPECIFIC32_2, &tv_SPECIFIC64_2, &tv_SPECIFIC32_3, &tv_SPECIFIC64_3,
							   &tv_SPECIFIC32_4, &tv_SPECIFIC64_4, &tv_SPECIFIC32_5, &tv_SPECIFIC64_5,
							   &tv_SPECIFIC0, &tv_SPECIFIC1, &tv_SPECIFIC2, &tv_SPECIFIC3,
							   &tv_SPECIFIC4, &tv_SPECIFIC5, &tv_SPECIFIC6, &tv_SPECIFIC7,
							   &tv_ZREG_B, &tv_ZREG_H, &tv_ZREG_S, &tv_ZREG_D, &tv_ZREG_Q,
							   &tv_PREG_B, &tv_PREG_H, &tv_PREG_S, &tv_PREG_D,
							   &tv_PG_M, &tv_PG_Z, &tv_PG, &tv_PG_ZM,
							   &tv_PTR_O, &tv_PTR_C, &tv_BRA_O, &tv_BRA_C,
							   &tv_XTW, &tv_PRFOP, &tv_PRFOP_SVE, &tv_MUL_VL };
  std::vector<std::vector<std::string> *> tv_VectorsJit = { &tv_WREG, &tv_XREG, &tv_VREG,
							    &tv_WSP, &tv_XSP, &tv_WNSP, &tv_XNSP,
							    &tv_IMM0BIT, &tv_IMM1BIT, &tv_IMM2BIT, &tv_IMM3BIT, &tv_IMM4BIT, &tv_IMM5BIT, &tv_IMM6BIT, &tv_IMM8BIT,
							    &tv_IMM12BIT, &tv_IMM13BIT, &tv_IMM16BIT,
							    &tv_IMM3BIT_N, &tv_IMM4BIT_N, &tv_IMM5BIT_N, &tv_IMM6BIT_N, &tv_FLOAT8BIT,
							    &tv_ZREG,
							    &jtv_SPECIFIC32, &jtv_SPECIFIC64, &jtv_SPECIFIC32_1, &jtv_SPECIFIC64_1,
							    &jtv_SPECIFIC32_2, &jtv_SPECIFIC64_2, &jtv_SPECIFIC32_3, &jtv_SPECIFIC64_3,
							    &jtv_SPECIFIC32_4, &jtv_SPECIFIC64_4, &jtv_SPECIFIC32_5, &jtv_SPECIFIC64_5,
							    &jtv_SPECIFIC0, &jtv_SPECIFIC1, &jtv_SPECIFIC2, &jtv_SPECIFIC3,
							    &jtv_SPECIFIC4, &jtv_SPECIFIC5, &jtv_SPECIFIC6, &jtv_SPECIFIC7,
							    &tv_ZREG_B, &tv_ZREG_H, &tv_ZREG_S, &tv_ZREG_D, &tv_ZREG_Q,
							    &tv_PREG_B, &tv_PREG_H, &tv_PREG_S, &tv_PREG_D,
							    &jtv_PG_M, &jtv_PG_Z, &tv_PG, &jtv_PG_ZM,
							    &jtv_PTR_O, &jtv_PTR_C, &jtv_BRA_O, &jtv_BRA_C,
							    &jtv_XTW, &tv_PRFOP, &jtv_PRFOP_SVE, &jtv_MUL_VL };

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

  bool isRequireComma(int num_ops, int idx, std::string curr, std::string next) const {
    if(idx == num_ops-1) { /** Last operand isn't followed by ",". */
      return false;
    }

    if(curr == "" || curr == "[" || curr == "ptr(" || curr == "{") {
      return false;
    }

    if(next == "]" || next == ")" || next == "}" || next == " ") {
      return false;
    }

    return true;
  }
  
  /*
    and_, or_, xor_, not_ => and, or, xor, not
  */
  std::string removeUnderScore(std::string s) const
  {
    if (!isXbyak_ && s[s.size() - 1] == '_') s.resize(s.size() - 1);
    return s;
  }


  void put(std::vector<std::string> &n, std::string name, int serial=0) const
  {
    std::cout << "//" << name << ":" << serial << std::endl; /** For easy debug */

    for(std::string i : n) {
      const char *nm = removeUnderScore(i).c_str();
      if(isXbyak_) {
	std::cout << nm << "();";
      } else {
	std::cout << nm << std::endl;
	  }
	}
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
    return get(op1, (uint64_t) 0);
  }
  
  /** check all op1, op2, op3, op4, op5, op6, op7, op8 */
  void put(const char *nm, std::vector<uint64_t>& ops) const
  {
    std::vector<std::string> strBase;
    std::string hoge;
    int i, k;
    uint jj;
    int num_ops = ops.size();
    
    for(i = 0; i < num_ops; i++) {
      strBase.push_back(getBaseStr(ops[i]));
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
      for(jj = 0; jj < getNum(ops[i]); jj++) {
	/** print nemonic */
	std::cout << nm;
	
	if(isXbyak_) {
	  std::cout << "(";
	} else {
	  std::cout << " ";
	}

	/** Print 0 - (i-1)th operand */
	for(k = 0; k < i; k++) {
	  std::string tmp = strBase[k];
	  std::string nextStr = "";
	  std::cout << tmp;

	  if(k+1 < num_ops) {
	    nextStr = strBase[k+1];
	  }
	  if(isRequireComma(num_ops, k, tmp, nextStr)) {
	    std::cout << ", ";
	  } else {
	    std::cout << " ";
	  }
	}

	/** Print i-th operand, which is rotated. */
	{
	  std::string tmp = get(ops[i], jj);
	  std::string nextStr = "";
	  std::cout << get(ops[i], jj);

	  if(i+1 < num_ops) {
	    nextStr = strBase[i+1];
	  }
	  if(isRequireComma(num_ops, i, tmp, nextStr)){
	    std::cout << ", ";
	  } else {
	    std::cout << " ";
	  }
	}

	/** Print (i+1) to (num_ops-1)-th operand */
	for(k = i+1; k < num_ops; k++) {
	  std::string tmp = strBase[k];
	  std::string nextStr = "";
	  std::cout << tmp;

	  if(k+1 < num_ops) {
	    nextStr = strBase[k+1];
	  }

	  if(isRequireComma(num_ops, k, tmp, nextStr)) {
	    std::cout << ", ";
	  } else {
	    std::cout << " ";
	  }
	}

	if(isXbyak_) {
	  std::cout << "); dump();";
	}

	std::cout << std::endl;
      }
    }
  }

  uint64_t getNum(uint64_t type) const
  {
    if(type==NOPARA) {
      return 0;
    }

    return tv_Vectors[type]->size();
  }

  const char *get(uint64_t type, uint64_t index) const
  {
    if(type==NOPARA) {
      std::cerr << std::endl << __FILE__ << ":" << __LINE__ << ", Something wrong. type=" << type << " index=" << index << std::endl;
      assert(0);
      return NULL;
    }

    return tv_Vectors[type]->at(index).c_str();
  }

  void clearTvAndJtv() {
    tv_SPECIFIC32.clear();
    tv_SPECIFIC32_1.clear();
    tv_SPECIFIC32_2.clear();
    tv_SPECIFIC32_3.clear();
    tv_SPECIFIC32_4.clear();
    tv_SPECIFIC32_5.clear();
    tv_SPECIFIC64.clear();
    tv_SPECIFIC64.clear();
    tv_SPECIFIC64_1.clear();
    tv_SPECIFIC64_2.clear();
    tv_SPECIFIC64_3.clear();
    tv_SPECIFIC64_4.clear();
    tv_SPECIFIC64_5.clear();
    
    jtv_SPECIFIC32.clear();
    jtv_SPECIFIC32_1.clear();
    jtv_SPECIFIC32_2.clear();
    jtv_SPECIFIC32_3.clear();
    jtv_SPECIFIC32_4.clear();
    jtv_SPECIFIC32_5.clear();
    jtv_SPECIFIC64.clear();
    jtv_SPECIFIC64.clear();
    jtv_SPECIFIC64_1.clear();
    jtv_SPECIFIC64_2.clear();
    jtv_SPECIFIC64_3.clear();
    jtv_SPECIFIC64_4.clear();
    jtv_SPECIFIC64_5.clear();


    tv_SPECIFIC0.clear();
    tv_SPECIFIC1.clear();
    tv_SPECIFIC2.clear();
    tv_SPECIFIC3.clear();
    tv_SPECIFIC4.clear();
    tv_SPECIFIC5.clear();
    tv_SPECIFIC6.clear();

    jtv_SPECIFIC0.clear();
    jtv_SPECIFIC1.clear();
    jtv_SPECIFIC2.clear();
    jtv_SPECIFIC3.clear();
    jtv_SPECIFIC4.clear();
    jtv_SPECIFIC5.clear();
    jtv_SPECIFIC6.clear();
  }

public:
  Test(bool isXbyak)
    : isXbyak_(isXbyak)
    , funcNum_(1)
  {
    //    setAllVregElem();
    
    if (!isXbyak_) {
      tv_Vectors = tv_VectorsAs;
      return;
    } else {
      tv_Vectors = tv_VectorsJit;
    printf("%s",
	   "    void gen0()\n"
	   "    {\n");
  }
  }

  void setZregElem(std::vector<std::string>& tv,
		   std::vector<int>& regs, std::vector<int>& index,
		   std::string type) {
    tv.clear();

    /** Rgister rotation */
    std::string tmpTypeIdx = "." + type + "[" + std::to_string(index[0]) + "]";
    for(int i : regs) {
      std::string tmpReg = std::to_string(i);
      tv.push_back("z" + tmpReg + tmpTypeIdx);
    }

    /** Index rotation */
    std::string tmpReg = "z" + std::to_string(regs[0]) + "." + type + "[";
    for(int i : index) {
      tv.push_back(tmpReg + std::to_string(i) + "]");
    }
  }  
  
  void setDstPred1stSrcZreg(TpPredType predType, std::vector<std::string>& tv, std::vector<std::string>& jtv, std::string type,
			   std::vector<std::string>& tmpDst) {
    std::vector<std::string> tmpPre = { "p7", "p0", "p1", "p2", "p4" };
    
    std::vector<std::string> tmpZero = { "/z" };
    std::vector<std::string> jtmpZero = { "/T_z" };
    std::vector<std::string> tmpMerge = { "/m" };
    std::vector<std::string> jtmpMerge = { "/T_m" };
    std::vector<std::string> tmpMZ = { "/m", "/z" };
    std::vector<std::string> jtmpMZ = { "/T_m", "/T_z" };
    std::vector<std::string> tmp0 = { "" };
    std::vector<std::string> jtmp0 = { "" };
    
    std::vector<std::string>& pred = tmpZero;
    std::vector<std::string>& jpred = jtmpZero;

    std::string tmpType = "";
    if(type.length()) {
      tmpType = tmpType + "." + type;
    }

    switch(predType) {
    case TP_z:  pred  = tmpZero;  jpred = jtmpZero;  break;
    case TP_m:  pred  = tmpMerge; jpred = jtmpMerge; break;
    case TP_zm: pred  = tmpMZ;    jpred = jtmpMZ;    break;
    case TP_NONE: pred = tmp0;    jpred = jtmp0;     break;
    default:    pred  = tmpZero;  jpred = jtmpZero;  break;
    }      
      
    tv.clear();
    jtv.clear();

    /** Dst register rotation */
    for(std::size_t j=0; j<pred.size(); j++) {
      std::string strPred  = tmpPre[0] + pred[j];
      std::string jstrPred = tmpPre[0] + jpred[j];
      for(std::string i : tmpDst) {
	tv.push_back (i + tmpType + ", " + strPred + ", " + i + tmpType);
	jtv.push_back(i + tmpType + ", " + jstrPred);
      }
    }

    /** Predication register rotation */
    for(std::size_t j=0; j<pred.size(); j++) {
      std::string strDst = tmpDst[0] + tmpType;
      for(std::string i : tmpPre) {
	tv.push_back (strDst + ", " + i + pred[j] + ", " + strDst);
	jtv.push_back(strDst + ", " + i + jpred[j]);
      }
    }

    /** Debug
	for(std::string i : tv) {
	std::cout << i << std::endl;
	}
	for(std::string i : jtv) {
	std::cout << i << std::endl;
	} */
  }    

  void setDst1stSrcZreg(std::vector<std::string>& tv, std::vector<std::string>& jtv, std::string type,
			   std::vector<std::string>& tmpDst) {

    std::string tmpType = "";
    if(type.length()) {
      tmpType = tmpType + "." + type;
    }

    tv.clear();
    jtv.clear();

    /** Dst register rotation */
    for(std::string i : tmpDst) {
      tv.push_back(i + tmpType + ", " + i);
      jtv.push_back(i + tmpType);
    }
  }    

  
  void setRotatedOnes(std::vector<std::string>& tv, std::vector<std::string>& jtv, int dataSize) {
    uint64_t mask = 0;
    std::vector<uint64_t> base = { 1, 3, 7, 15 };

    tv.clear();
    jtv.clear();
    

    switch(dataSize) {
    case 8: mask = 0xff; break;
    case 16: mask = 0xffff; break;
    case 32: mask = 0xffffffff; break;
    case 64: mask = 0xffffffffffffffff; break;
    default: 
      std::cerr << __FILE__ << ":" << __LINE__ << ":" << "Invalid datasize=" << dataSize << std::endl;
      std::exit(1);
      break;
    }      
      
    
    for(int len=0; len< dataSize/4; len++) {
      for(const uint64_t i : base) {
	uint64_t hoge = i;

	for(int j=1; j<=len; j++) {
	  hoge = (hoge << 4) + 0xf;
	}

	for(int rot=0; rot<dataSize; rot++) {
	  std::stringstream ss;
	  ss << std::hex << std::showbase;

	  hoge = (hoge<<rot) + (hoge>>(dataSize-rot));
	  hoge = hoge & mask;

	  /** All bits equal to '1' is prohibited. */
	  if(hoge == mask) {
	    continue;
	  }

	  
	  ss.str("");
	  ss << hoge;
	  
	  tv.push_back(ss.str());
	  jtv.push_back(ss.str());
	}
      }
    }
  }    

  void setZregTypeIndex(std::vector<std::string>& tv, std::vector<std::string>& jtv,
			std::vector<std::string>& baseVec, std::vector<int> idxVec, std::string type) {
    tv.clear();
    jtv.clear();
    
    /** Src register index rotation */
    for(std::string i : baseVec) {
      std::string tmpStr = i + "[" + std::to_string(idxVec[0]) + "]";
      tv.push_back(tmpStr);
      jtv.push_back(tmpStr);
    }
    /** element index rotation */
    for(int i : idxVec) {
      std::string tmpStr = "z15." + type + "[" + std::to_string(i) + "]";
      tv.push_back(tmpStr);
      jtv.push_back(tmpStr);
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

  /** MACRO is expanded in order from the outside. */
  
  /** SVE Memory - 32-bit Gather and Unsized Contiguous */
  /*** SVE 32-bit gather load (scalar plus 32-bit unscaled offsets) */
  PUT1(SVE_32_bit_gather_load_scalar_plus_32_bit_unscaled_offsets0,
       NM("ld1sb", "ldff1sb", "ld1b", "ldff1b"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, XTW, PTR_C));
  PUT1(SVE_32_bit_gather_load_scalar_plus_32_bit_unscaled_offsets1,
       NM("ld1sb", "ldff1sb", "ld1b", "ldff1b"),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, ZREG_S, XTW, PTR_C));
  PUT1(SVE_32_bit_gather_load_scalar_plus_32_bit_unscaled_offsets2,
       NM("ld1sh", "ldff1sh", "ld1h", "ldff1h"),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, ZREG_S, SPECIFIC32_1, PTR_C));
  PUT1(SVE_32_bit_gather_load_scalar_plus_32_bit_unscaled_offsets3,
       NM("ld1w", "ldff1w"),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, ZREG_S, SPECIFIC32_2, PTR_C));

  void putSVE_32_bit_gather_load_scalar_plus_32_bit_unscaled_offsets() {
    putSVE_32_bit_gather_load_scalar_plus_32_bit_unscaled_offsets0();
    putSVE_32_bit_gather_load_scalar_plus_32_bit_unscaled_offsets1();
    putSVE_32_bit_gather_load_scalar_plus_32_bit_unscaled_offsets2();
    putSVE_32_bit_gather_load_scalar_plus_32_bit_unscaled_offsets3();
  }    

  /*** SVE contiguous prefetch (scalar plus scalar) */
  PUT1(SVE_contiguous_prefetch_scalar_plus_scalar0,
       NM("prfb"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, XREG, PTR_C));
  PUT1(SVE_contiguous_prefetch_scalar_plus_scalar1,
       NM("prfh"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, XREG, SPECIFIC64_4, PTR_C));
  PUT1(SVE_contiguous_prefetch_scalar_plus_scalar2,
       NM("prfw"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, XREG, SPECIFIC64_3, PTR_C));
  PUT1(SVE_contiguous_prefetch_scalar_plus_scalar3,
       NM("prfd"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, XREG, SPECIFIC64_2, PTR_C));

  void putSVE_contiguous_prefetch_scalar_plus_scalar() {
    putSVE_contiguous_prefetch_scalar_plus_scalar0();
    putSVE_contiguous_prefetch_scalar_plus_scalar1();
    putSVE_contiguous_prefetch_scalar_plus_scalar2();
    putSVE_contiguous_prefetch_scalar_plus_scalar3();
  }

  /*** SVE 32-bit gather prefetch (vector plus immediate) */
  PUT4(SVE_32_bit_gather_prefetch_vector_plus_immediate0,
       NM("prfb"),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, IMM5BIT, PTR_C),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, IMM5BIT, PTR_C));
  PUT4(SVE_32_bit_gather_prefetch_vector_plus_immediate1,
       NM("prfh"),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, SPECIFIC1, PTR_C),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, SPECIFIC1, PTR_C));
  PUT4(SVE_32_bit_gather_prefetch_vector_plus_immediate2,
       NM("prfw"),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, SPECIFIC2, PTR_C),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, SPECIFIC2, PTR_C));
  PUT4(SVE_32_bit_gather_prefetch_vector_plus_immediate3,
       NM("prfd"),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, SPECIFIC3, PTR_C),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, SPECIFIC3, PTR_C));

  void putSVE_32_bit_gather_prefetch_vector_plus_immediate() {
    putSVE_32_bit_gather_prefetch_vector_plus_immediate0();
    putSVE_32_bit_gather_prefetch_vector_plus_immediate1();
    putSVE_32_bit_gather_prefetch_vector_plus_immediate2();
    putSVE_32_bit_gather_prefetch_vector_plus_immediate3();
  }

  /*** SVE 32-bit gather load (vector plus immediate) */
  PUT2(SVE_32_bit_gather_load_vector_plus_immediate0,
       NM("ld1sb", "ldff1sb", "ld1b", "ldff1b"),
       OPS(ZREG_S, PG_Z, PTR_O, ZREG_S, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, ZREG_S, IMM5BIT, PTR_C));
  PUT2(SVE_32_bit_gather_load_vector_plus_immediate1,
       NM("ld1sh", "ldff1sh", "ld1h", "ldff1h"),
       OPS(ZREG_S, PG_Z, PTR_O, ZREG_S, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, ZREG_S, SPECIFIC1, PTR_C));
  PUT2(SVE_32_bit_gather_load_vector_plus_immediate2,
       NM("ld1w", "ldff1w"),
       OPS(ZREG_S, PG_Z, PTR_O, ZREG_S, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, ZREG_S, SPECIFIC2, PTR_C));

  void putSVE_32_bit_gather_load_vector_plus_immediate() {
    putSVE_32_bit_gather_load_vector_plus_immediate0();
    putSVE_32_bit_gather_load_vector_plus_immediate1();
    putSVE_32_bit_gather_load_vector_plus_immediate2();
  }    

  /*** SVE load and broadcast element */
  PUT8(SVE_load_and_broadcast_element0,
       NM("ld1rb"),
       OPS(ZREG_B, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_B, PG_Z, PTR_O, XNSP, IMM6BIT, PTR_C),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_H, PG_Z, PTR_O, XNSP, IMM6BIT, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, XNSP, IMM6BIT, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, IMM6BIT, PTR_C));
  PUT2(SVE_load_and_broadcast_element1,
       NM("ld1rsw"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC2, PTR_C));
  PUT6(SVE_load_and_broadcast_element2,
       NM("ld1rh"),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_H, PG_Z, PTR_O, XNSP, SPECIFIC1, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC1, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC1, PTR_C));
  PUT4(SVE_load_and_broadcast_element3,
       NM("ld1rsh"),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC1, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC1, PTR_C));
  PUT4(SVE_load_and_broadcast_element4,
       NM("ld1rw"),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC2, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC2, PTR_C));
  PUT6(SVE_load_and_broadcast_element5,
       NM("ld1rsb"),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_H, PG_Z, PTR_O, XNSP, IMM6BIT, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, XNSP, IMM6BIT, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, IMM6BIT, PTR_C));
  PUT2(SVE_load_and_broadcast_element6,
       NM("ld1rd"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC3, PTR_C));

  
  void putSVE_load_and_broadcast_element() {
    putSVE_load_and_broadcast_element0();
    putSVE_load_and_broadcast_element1();
    putSVE_load_and_broadcast_element2();
    putSVE_load_and_broadcast_element3();
    putSVE_load_and_broadcast_element4();
    putSVE_load_and_broadcast_element5();
    putSVE_load_and_broadcast_element6();
  }
    
  /*** SVE 32-bit gather prefetch (scalar plus 32-bit scaled offsets) */
  PUT1(SVE_32_bit_gather_prefetch_scalar_plus_32_bit_scaled_offsets0,
       NM("prfb"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, ZREG_S, XTW, PTR_C));
  PUT1(SVE_32_bit_gather_prefetch_scalar_plus_32_bit_scaled_offsets1,
       NM("prfh"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, ZREG_S, SPECIFIC32_1, PTR_C));
  PUT1(SVE_32_bit_gather_prefetch_scalar_plus_32_bit_scaled_offsets2,
       NM("prfw"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, ZREG_S, SPECIFIC32_2, PTR_C));
  PUT1(SVE_32_bit_gather_prefetch_scalar_plus_32_bit_scaled_offsets3,
       NM("prfd"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, ZREG_S, SPECIFIC32_3, PTR_C));

  void putSVE_32_bit_gather_prefetch_scalar_plus_32_bit_scaled_offsets() {
    putSVE_32_bit_gather_prefetch_scalar_plus_32_bit_scaled_offsets0();
    putSVE_32_bit_gather_prefetch_scalar_plus_32_bit_scaled_offsets1();
    putSVE_32_bit_gather_prefetch_scalar_plus_32_bit_scaled_offsets2();
    putSVE_32_bit_gather_prefetch_scalar_plus_32_bit_scaled_offsets3();
  }
  
  /*** SVE 32-bit gather load halfwords (scalar plus 32-bit scaled offsets) */
  PUT6(SVE_32_bit_gather_load_halfwords_scalar_plus_32_bit_scaled_offsets0,
       NM("ld1sh", "ldff1sh", "ld1h", "ldff1h"),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, ZREG_S, SPECIFIC32_1, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, SPECIFIC32_1, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, XTW, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, ZREG_S, XTW, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, SPECIFIC64_4, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, PTR_C));

  void putSVE_32_bit_gather_load_halfwords_scalar_plus_32_bit_scaled_offsets() {
    putSVE_32_bit_gather_load_halfwords_scalar_plus_32_bit_scaled_offsets0();
  }
  
  /*** SVE 32-bit gather load words (scalar plus 32-bit scaled offsets) */
  PUT6(SVE_32_bit_gather_load_words_scalar_plus_32_bit_scaled_offsets0,
       NM("ld1w", "ldff1w"),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, ZREG_S, SPECIFIC32_2, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, SPECIFIC32_2, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, XTW, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, ZREG_S, XTW, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, SPECIFIC64_3, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, PTR_C));

  void putSVE_32_bit_gather_load_words_scalar_plus_32_bit_scaled_offsets() {
    putSVE_32_bit_gather_load_words_scalar_plus_32_bit_scaled_offsets0();
  }
  
  /*** SVE load predicate register */
  PUT2(SVE_load_predicate_register0,
       NM("ldr"),
       OPS(SPECIFIC64_1, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC64_1, PTR_O, XNSP, SPECIFIC64, MULVL, PTR_C));

  void putSVE_load_predicate_register() {
    putSVE_load_predicate_register0();
  }

  /*** SVE load vector register */
  PUT2(SVE_load_vector_register0,
       NM("ldr"),
       OPS(ZREG, PTR_O, XNSP, PTR_C),
       OPS(ZREG, PTR_O, XNSP, SPECIFIC64, MULVL));


  void putSVE_load_vector_register() {
    putSVE_load_vector_register0();
  }

  /*** SVE contiguous prefetch (scalar plus immediate) */
  PUT2(SVE_contiguous_prefetch_scalar_plus_immediate0,
       NM("prfb", "prfh", "prfw", "prfd"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, PTR_C),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, SPECIFIC32_5, MULVL, PTR_C));

  void putSVE_contiguous_prefetch_scalar_plus_immediate() {
      putSVE_contiguous_prefetch_scalar_plus_immediate0();
    }

  /** SVE Memory - Contiguous Load */
  /*** SVE load and broadcast quadword (scalar plus scalar) */
  PUT1(SVE_load_and_broadcast_quadword_scalar_plus_scalar0,
       NM("ld1rqb"),
       OPS(ZREG_B, PG_Z, PTR_O, XNSP, XREG, PTR_C));
  PUT1(SVE_load_and_broadcast_quadword_scalar_plus_scalar1,
       NM("ld1rqh"),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, XREG, SPECIFIC1, PTR_C));
  PUT1(SVE_load_and_broadcast_quadword_scalar_plus_scalar2,
       NM("ld1rqw"),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, XREG, SPECIFIC2, PTR_C));
  PUT1(SVE_load_and_broadcast_quadword_scalar_plus_scalar3,
       NM("ld1rqd"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, XREG, SPECIFIC3, PTR_C));

  void putSVE_load_and_broadcast_quadword_scalar_plus_scalar() {
    putSVE_load_and_broadcast_quadword_scalar_plus_scalar0();
    putSVE_load_and_broadcast_quadword_scalar_plus_scalar1();
    putSVE_load_and_broadcast_quadword_scalar_plus_scalar2();
    putSVE_load_and_broadcast_quadword_scalar_plus_scalar3();
  }
  
  /*** SVE contiguous load (scalar plus scalar) */
  PUT4(SVE_contiguous_load_scalar_plus_scalar0,
       NM("ld1b"),
       OPS(ZREG_B, PG_Z, PTR_O, XNSP, XREG, PTR_C),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, XREG, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, XREG, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, XREG, PTR_C));
  PUT1(SVE_contiguous_load_scalar_plus_scalar1,
       NM("ld1sw"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, XREG, SPECIFIC2, PTR_C));
  PUT3(SVE_contiguous_load_scalar_plus_scalar2,
       NM("ld1h"),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, XREG, SPECIFIC1, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, XREG, SPECIFIC1, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, XREG, SPECIFIC1, PTR_C));
  PUT2(SVE_contiguous_load_scalar_plus_scalar3,
       NM("ld1sh"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, XREG, SPECIFIC1, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, XREG, SPECIFIC1, PTR_C));
  PUT2(SVE_contiguous_load_scalar_plus_scalar4,
       NM("ld1w"),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, XREG, SPECIFIC2, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, XREG, SPECIFIC2, PTR_C));
  PUT3(SVE_contiguous_load_scalar_plus_scalar5,
       NM("ld1sb"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, XREG, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, XREG, PTR_C),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, XREG, PTR_C));
  PUT1(SVE_contiguous_load_scalar_plus_scalar6,
       NM("ld1d"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, XREG, SPECIFIC3, PTR_C));

  void putSVE_contiguous_load_scalar_plus_scalar() {
    putSVE_contiguous_load_scalar_plus_scalar0();
    putSVE_contiguous_load_scalar_plus_scalar1();
    putSVE_contiguous_load_scalar_plus_scalar2();
    putSVE_contiguous_load_scalar_plus_scalar3();
    putSVE_contiguous_load_scalar_plus_scalar4();
    putSVE_contiguous_load_scalar_plus_scalar5();
    putSVE_contiguous_load_scalar_plus_scalar6();
  }

  /*** SVE contiguous first-fault load (scalar plus scalar) */
  PUT8(SVE_contiguous_first_fault_load_scalar_plus_scalar0,
       NM("ldff1b"),
       OPS(ZREG_B, PG_Z, PTR_O, XNSP, SPECIFIC64, PTR_C), OPS(ZREG_B, PG_Z, PTR_O, XNSP, SPECIFIC64_1, PTR_C),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, SPECIFIC64, PTR_C), OPS(ZREG_H, PG_Z, PTR_O, XNSP, SPECIFIC64_1, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC64, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC64_1, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC64, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC64_1, PTR_C));
  PUT2(SVE_contiguous_first_fault_load_scalar_plus_scalar1,
       NM("ldff1sw"),
       // Following test gives ommited operands to xbyak_aarch64,
       // full operands to aarch64-linux-as. as can't handle with ommited operands.
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC64_2, SPECIFIC5, PTR_C),
       // Following test gives full operands to both of xbyak_aarch64 and aarch64-linux-as.
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC64_1, SPECIFIC2, PTR_C));
  PUT6(SVE_contiguous_first_fault_load_scalar_plus_scalar2,
       NM("ldff1h"),
       // Following test gives ommited operands to xbyak_aarch64,
       // full operands to aarch64-linux-as. as can't handle with ommited operands.
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, SPECIFIC64_2, SPECIFIC4, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC64_2, SPECIFIC4, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC64_2, SPECIFIC4, PTR_C),
       // Following test gives full operands to both of xbyak_aarch64 and aarch64-linux-as.
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, SPECIFIC64_1, SPECIFIC1, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC64_1, SPECIFIC1, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC64_1, SPECIFIC1, PTR_C));
  PUT4(SVE_contiguous_first_fault_load_scalar_plus_scalar3,
       NM("ldff1sh"),
       // Following test gives ommited operands to xbyak_aarch64,
       // full operands to aarch64-linux-as. as can't handle with ommited operands.
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC64_2, SPECIFIC4, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC64_2, SPECIFIC4, PTR_C),
       // Following test gives full operands to both of xbyak_aarch64 and aarch64-linux-as.
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC64_1, SPECIFIC1, PTR_C),  
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC64_1, SPECIFIC1, PTR_C));
  PUT4(SVE_contiguous_first_fault_load_scalar_plus_scalar4,
       NM("ldff1w"),
       // Following test gives ommited operands to xbyak_aarch64,
       // full operands to aarch64-linux-as. as can't handle with ommited operands.
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC64_2, SPECIFIC5, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC64_2, SPECIFIC5, PTR_C),
       // Following test gives full operands to both of xbyak_aarch64 and aarch64-linux-as.
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC64_1, SPECIFIC2, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC64_1, SPECIFIC2, PTR_C));
  PUT6(SVE_contiguous_first_fault_load_scalar_plus_scalar5,
       NM("ldff1sb"),
       // Following test gives ommited operands to xbyak_aarch64,
       // full operands to aarch64-linux-as. as can't handle with ommited operands.
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC64_2, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC64_2, PTR_C),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, SPECIFIC64_2, PTR_C),
       // Following test gives full operands to both of xbyak_aarch64 and aarch64-linux-as.
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC64_1, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC64_1, PTR_C),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, SPECIFIC64_1, PTR_C));
  PUT2(SVE_contiguous_first_fault_load_scalar_plus_scalar6,
       NM("ldff1d"),
       // Following test gives ommited operands to xbyak_aarch64,
       // full operands to aarch64-linux-as. as can't handle with ommited operands.
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC64_2, SPECIFIC6, PTR_C),
       // Following test gives full operands to both of xbyak_aarch64 and aarch64-linux-as.
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC64_1, SPECIFIC3, PTR_C));

  void putSVE_contiguous_first_fault_load_scalar_plus_scalar() {
    putSVE_contiguous_first_fault_load_scalar_plus_scalar0();
    putSVE_contiguous_first_fault_load_scalar_plus_scalar1();
    putSVE_contiguous_first_fault_load_scalar_plus_scalar2();
    putSVE_contiguous_first_fault_load_scalar_plus_scalar3();
    putSVE_contiguous_first_fault_load_scalar_plus_scalar4();
    putSVE_contiguous_first_fault_load_scalar_plus_scalar5();
    putSVE_contiguous_first_fault_load_scalar_plus_scalar6();
  }

  /*** SVE load multiple structures (scalar plus scalar) */
  PUT1(SVE_load_multiple_structures_scalar_plus_scalar0,
       NM("ld2b"),
       OPS(SPECIFIC0, PG_Z, PTR_O, XNSP, XREG, PTR_C));
  PUT1(SVE_load_multiple_structures_scalar_plus_scalar1,
       NM("ld3b"),
       OPS(SPECIFIC1, PG_Z, PTR_O, XNSP, XREG, PTR_C));
  PUT1(SVE_load_multiple_structures_scalar_plus_scalar2,
       NM("ld4b"),
       OPS(SPECIFIC2, PG_Z, PTR_O, XNSP, XREG, PTR_C));
  PUT1(SVE_load_multiple_structures_scalar_plus_scalar3,
       NM("ld2h"),
       OPS(SPECIFIC3, PG_Z, PTR_O, XNSP, XREG, SPECIFIC64_3, PTR_C));
  PUT1(SVE_load_multiple_structures_scalar_plus_scalar4,
       NM("ld3h"),
       OPS(SPECIFIC32, PG_Z, PTR_O, XNSP, XREG, SPECIFIC64_3, PTR_C));
  PUT1(SVE_load_multiple_structures_scalar_plus_scalar5,
       NM("ld4h"),
       OPS(SPECIFIC32_1, PG_Z, PTR_O, XNSP, XREG, SPECIFIC64_3, PTR_C));
  PUT1(SVE_load_multiple_structures_scalar_plus_scalar6,
       NM("ld2w"),
       OPS(SPECIFIC32_2, PG_Z, PTR_O, XNSP, XREG, SPECIFIC64_4, PTR_C));
  PUT1(SVE_load_multiple_structures_scalar_plus_scalar7,
       NM("ld3w"),
       OPS(SPECIFIC32_3, PG_Z, PTR_O, XNSP, XREG, SPECIFIC64_4, PTR_C));
  PUT1(SVE_load_multiple_structures_scalar_plus_scalar8,
       NM("ld4w"),
       OPS(SPECIFIC32_4, PG_Z, PTR_O, XNSP, XREG, SPECIFIC64_4, PTR_C));
  PUT1(SVE_load_multiple_structures_scalar_plus_scalar9,
       NM("ld2d"),
       OPS(SPECIFIC64, PG_Z, PTR_O, XNSP, XREG, SPECIFIC64_5, PTR_C));
  PUT1(SVE_load_multiple_structures_scalar_plus_scalar10,
       NM("ld3d"),
       OPS(SPECIFIC64_1, PG_Z, PTR_O, XNSP, XREG, SPECIFIC64_5, PTR_C));
  PUT1(SVE_load_multiple_structures_scalar_plus_scalar11,
       NM("ld4d"),
       OPS(SPECIFIC64_2, PG_Z, PTR_O, XNSP, XREG, SPECIFIC64_5, PTR_C));

  void putSVE_load_multiple_structures_scalar_plus_scalar() {
    putSVE_load_multiple_structures_scalar_plus_scalar0();
    putSVE_load_multiple_structures_scalar_plus_scalar1();
    putSVE_load_multiple_structures_scalar_plus_scalar2();
    putSVE_load_multiple_structures_scalar_plus_scalar3();
    putSVE_load_multiple_structures_scalar_plus_scalar4();
    putSVE_load_multiple_structures_scalar_plus_scalar5();
    putSVE_load_multiple_structures_scalar_plus_scalar6();
    putSVE_load_multiple_structures_scalar_plus_scalar7();
    putSVE_load_multiple_structures_scalar_plus_scalar8();
    putSVE_load_multiple_structures_scalar_plus_scalar9();
    putSVE_load_multiple_structures_scalar_plus_scalar10();
    putSVE_load_multiple_structures_scalar_plus_scalar11();
  }

  /*** SVE load and broadcast quadword (scalar plus immediate) */
  PUT2(SVE_load_and_broadcast_quadword_scalar_plus_immediate0,
       NM("ld1rqb"),
       OPS(ZREG_B, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_B, PG_Z, PTR_O, XNSP, SPECIFIC32_5, PTR_C));
  PUT2(SVE_load_and_broadcast_quadword_scalar_plus_immediate1,
       NM("ld1rqh"),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_H, PG_Z, PTR_O, XNSP, SPECIFIC32_5, PTR_C));
  PUT2(SVE_load_and_broadcast_quadword_scalar_plus_immediate2,
       NM("ld1rqw"),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC32_5, PTR_C));
  PUT2(SVE_load_and_broadcast_quadword_scalar_plus_immediate3,
       NM("ld1rqd"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC32_5, PTR_C));

  void putSVE_load_and_broadcast_quadword_scalar_plus_immediate() {
    putSVE_load_and_broadcast_quadword_scalar_plus_immediate0();
    putSVE_load_and_broadcast_quadword_scalar_plus_immediate1();
    putSVE_load_and_broadcast_quadword_scalar_plus_immediate2();
    putSVE_load_and_broadcast_quadword_scalar_plus_immediate3();
  }
    
  /*** SVE contiguous load (scalar plus immediate) */
  PUT8(SVE_contiguous_load_scalar_plus_immediate0,
       NM("ld1b"),
       OPS(ZREG_B, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_B, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C), 
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_H, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C), 
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C), 
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C)); 
  PUT2(SVE_contiguous_load_scalar_plus_immediate1,
       NM("ld1sw", "ld1d"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C));
  PUT6(SVE_contiguous_load_scalar_plus_immediate2,
       NM("ld1h", "ld1sb"),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_H, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C));
  PUT4(SVE_contiguous_load_scalar_plus_immediate3,
       NM("ld1sh", "ld1w"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C));

  void putSVE_contiguous_load_scalar_plus_immediate() {
    putSVE_contiguous_load_scalar_plus_immediate0();
    putSVE_contiguous_load_scalar_plus_immediate1();
    putSVE_contiguous_load_scalar_plus_immediate2();
    putSVE_contiguous_load_scalar_plus_immediate3();
  }

  /*** SVE load multiple structures (scalar plus immediate) */
  PUT2(SVE_load_multiple_structures_scalar_plus_immediate0,
       NM("ld2b"),
       OPS(SPECIFIC0, PG_Z, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC0, PG_Z, PTR_O, XNSP, SPECIFIC4, MULVL, PTR_C));
  PUT2(SVE_load_multiple_structures_scalar_plus_immediate1,
       NM("ld3b"),
       OPS(SPECIFIC1, PG_Z, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC1, PG_Z, PTR_O, XNSP, SPECIFIC5, MULVL, PTR_C));
  PUT2(SVE_load_multiple_structures_scalar_plus_immediate2,
       NM("ld4b"),
       OPS(SPECIFIC2, PG_Z, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC2, PG_Z, PTR_O, XNSP, SPECIFIC6, MULVL, PTR_C));
  PUT2(SVE_load_multiple_structures_scalar_plus_immediate3,
       NM("ld2h"),
       OPS(SPECIFIC3, PG_Z, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC3, PG_Z, PTR_O, XNSP, SPECIFIC4, MULVL, PTR_C));
  PUT2(SVE_load_multiple_structures_scalar_plus_immediate4,
       NM("ld3h"),
       OPS(SPECIFIC32, PG_Z, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC32, PG_Z, PTR_O, XNSP, SPECIFIC5, MULVL, PTR_C));
  PUT2(SVE_load_multiple_structures_scalar_plus_immediate5,
       NM("ld4h"),
       OPS(SPECIFIC32_1, PG_Z, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC32_1, PG_Z, PTR_O, XNSP, SPECIFIC6, MULVL, PTR_C));
  PUT2(SVE_load_multiple_structures_scalar_plus_immediate6,
       NM("ld2w"),
       OPS(SPECIFIC32_2, PG_Z, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC32_2, PG_Z, PTR_O, XNSP, SPECIFIC4, MULVL, PTR_C));
  PUT2(SVE_load_multiple_structures_scalar_plus_immediate7,
       NM("ld3w"),
       OPS(SPECIFIC32_3, PG_Z, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC32_3, PG_Z, PTR_O, XNSP, SPECIFIC5, MULVL, PTR_C));
  PUT2(SVE_load_multiple_structures_scalar_plus_immediate8,
       NM("ld4w"),
       OPS(SPECIFIC32_4, PG_Z, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC32_4, PG_Z, PTR_O, XNSP, SPECIFIC6, MULVL, PTR_C));
  PUT2(SVE_load_multiple_structures_scalar_plus_immediate9,
       NM("ld2d"),
       OPS(SPECIFIC64, PG_Z, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC64, PG_Z, PTR_O, XNSP, SPECIFIC4, MULVL, PTR_C));
  PUT2(SVE_load_multiple_structures_scalar_plus_immediate10,
       NM("ld3d"),
       OPS(SPECIFIC64_1, PG_Z, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC64_1, PG_Z, PTR_O, XNSP, SPECIFIC5, MULVL, PTR_C));
  PUT2(SVE_load_multiple_structures_scalar_plus_immediate11,
       NM("ld4d"),
       OPS(SPECIFIC64_2, PG_Z, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC64_2, PG_Z, PTR_O, XNSP, SPECIFIC6, MULVL, PTR_C));

  void putSVE_load_multiple_structures_scalar_plus_immediate() {
    putSVE_load_multiple_structures_scalar_plus_immediate0();
    putSVE_load_multiple_structures_scalar_plus_immediate1();
    putSVE_load_multiple_structures_scalar_plus_immediate2();
    putSVE_load_multiple_structures_scalar_plus_immediate3();
    putSVE_load_multiple_structures_scalar_plus_immediate4();
    putSVE_load_multiple_structures_scalar_plus_immediate5();
    putSVE_load_multiple_structures_scalar_plus_immediate6();
    putSVE_load_multiple_structures_scalar_plus_immediate7();
    putSVE_load_multiple_structures_scalar_plus_immediate8();
    putSVE_load_multiple_structures_scalar_plus_immediate9();
    putSVE_load_multiple_structures_scalar_plus_immediate10();
    putSVE_load_multiple_structures_scalar_plus_immediate11();
  }  

  /*** SVE contiguous non-fault load (scalar plus immediate) */
  PUT8(SVE_contiguous_non_fault_load_scalar_plus_immediate0,
       NM("ldnf1b"),
       OPS(ZREG_B, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_B, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL,PTR_C),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_H, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL,PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL,PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL,PTR_C));
  PUT2(SVE_contiguous_non_fault_load_scalar_plus_immediate1,
       NM("ldnf1sw", "ldnf1d"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C));
  PUT6(SVE_contiguous_non_fault_load_scalar_plus_immediate2,
       NM("ldnf1h", "ldnf1sb"),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_H, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C));
  PUT4(SVE_contiguous_non_fault_load_scalar_plus_immediate3,
       NM("ldnf1sh", "ldnf1w"),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C));

  void putSVE_contiguous_non_fault_load_scalar_plus_immediate() {
    putSVE_contiguous_non_fault_load_scalar_plus_immediate0();
    putSVE_contiguous_non_fault_load_scalar_plus_immediate1();
    putSVE_contiguous_non_fault_load_scalar_plus_immediate2();
    putSVE_contiguous_non_fault_load_scalar_plus_immediate3();
  }

  /*** SVE contiguous non-temporal load (scalar plus scalar) */
  PUT1(SVE_contiguous_non_temporal_load_scalar_plus_scalar0,
       NM("ldnt1b"),
       OPS(ZREG_B, PG_Z, PTR_O, XNSP, XREG, PTR_C));
  PUT1(SVE_contiguous_non_temporal_load_scalar_plus_scalar1,
       NM("ldnt1h"),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, XREG, SPECIFIC1, PTR_C));
  PUT1(SVE_contiguous_non_temporal_load_scalar_plus_scalar2,
       NM("ldnt1w"),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, XREG, SPECIFIC2, PTR_C));
  PUT1(SVE_contiguous_non_temporal_load_scalar_plus_scalar3,
       NM("ldnt1d"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, XREG, SPECIFIC3, PTR_C));

  void putSVE_contiguous_non_temporal_load_scalar_plus_scalar() {
    putSVE_contiguous_non_temporal_load_scalar_plus_scalar0();
    putSVE_contiguous_non_temporal_load_scalar_plus_scalar1();
    putSVE_contiguous_non_temporal_load_scalar_plus_scalar2();
    putSVE_contiguous_non_temporal_load_scalar_plus_scalar3();
  }
    /*** SVE contiguous non-temporal load (scalar plus immediate) */
  PUT2(SVE_contiguous_non_temporal_load_scalar_plus_immediate0,
       NM("ldnt1b"),
       OPS(ZREG_B, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_B, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C));
  PUT2(SVE_contiguous_non_temporal_load_scalar_plus_immediate1,
       NM("ldnt1h"),
       OPS(ZREG_H, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_H, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C));
  PUT2(SVE_contiguous_non_temporal_load_scalar_plus_immediate2,
       NM("ldnt1w"),
       OPS(ZREG_S, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_S, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C));
  PUT2(SVE_contiguous_non_temporal_load_scalar_plus_immediate3,
       NM("ldnt1d"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, PTR_C), OPS(ZREG_D, PG_Z, PTR_O, XNSP, SPECIFIC32, MULVL, PTR_C));

  void putSVE_contiguous_non_temporal_load_scalar_plus_immediate() {
    putSVE_contiguous_non_temporal_load_scalar_plus_immediate0();
    putSVE_contiguous_non_temporal_load_scalar_plus_immediate1();
    putSVE_contiguous_non_temporal_load_scalar_plus_immediate2();
    putSVE_contiguous_non_temporal_load_scalar_plus_immediate3();
  }

  /** SVE Memory - 64-bit Gather */
  /*** SVE 64-bit gather load (scalar plus unpacked 32-bit unscaled offsets) */
  PUT1(SVE_64_bit_gather_load_scalar_plus_unpacked_32_bit_unscaled_offsets0,
       NM("ldff1sb", "ld1b", "ldff1b", "ld1sb", "ld1sh", "ldff1sh", "ld1h", "ldff1h",
	  "ld1sw", "ldff1sw", "ld1w", "ldff1w", "ld1d", "ldff1d"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, XTW, PTR_C));

  void putSVE_64_bit_gather_load_scalar_plus_unpacked_32_bit_unscaled_offsets() {
    putSVE_64_bit_gather_load_scalar_plus_unpacked_32_bit_unscaled_offsets0();
  }

  /*** SVE 64-bit gather load (scalar plus 32-bit unpacked scaled offsets) */
  PUT1(SVE_64_bit_gather_load_scalar_plus_32_bit_unpacked_scaled_offsets0,
       NM("ld1sh", "ldff1sh", "ld1h", "ldff1h"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, SPECIFIC0, PTR_C));
  PUT1(SVE_64_bit_gather_load_scalar_plus_32_bit_unpacked_scaled_offsets1,
       NM("ld1sw", "ldff1sw", "ld1w", "ldff1w"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, SPECIFIC1, PTR_C));
  PUT1(SVE_64_bit_gather_load_scalar_plus_32_bit_unpacked_scaled_offsets2,
       NM("ld1d", "ldff1d"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, SPECIFIC2, PTR_C));

  void putSVE_64_bit_gather_load_scalar_plus_32_bit_unpacked_scaled_offsets() {
    putSVE_64_bit_gather_load_scalar_plus_32_bit_unpacked_scaled_offsets0();
    putSVE_64_bit_gather_load_scalar_plus_32_bit_unpacked_scaled_offsets1();
    putSVE_64_bit_gather_load_scalar_plus_32_bit_unpacked_scaled_offsets2();
  }

  /*** SVE 64-bit gather prefetch (vector plus immediate) */
  PUT4(SVE_64_bit_gather_prefetch_vector_plus_immediate0,
       NM("prfb"),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, SPECIFIC32_3, PTR_C),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, SPECIFIC32_3, PTR_C));
  PUT4(SVE_64_bit_gather_prefetch_vector_plus_immediate1,
       NM("prfh"),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, SPECIFIC32_4, PTR_C),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, SPECIFIC32_4, PTR_C));
  PUT4(SVE_64_bit_gather_prefetch_vector_plus_immediate2,
       NM("prfw"),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, SPECIFIC32_5, PTR_C),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, SPECIFIC32_5, PTR_C));
  PUT4(SVE_64_bit_gather_prefetch_vector_plus_immediate3,
       NM("prfd"),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_S, SPECIFIC64, PTR_C),
       OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, PTR_C), OPS(PRFOP_SVE, PG, PTR_O, ZREG_D, SPECIFIC64, PTR_C));

  void putSVE_64_bit_gather_prefetch_vector_plus_immediate() {
    putSVE_64_bit_gather_prefetch_vector_plus_immediate0();
    putSVE_64_bit_gather_prefetch_vector_plus_immediate1();
    putSVE_64_bit_gather_prefetch_vector_plus_immediate2();
    putSVE_64_bit_gather_prefetch_vector_plus_immediate3();
  }

  /*** SVE 64-bit gather load (vector plus immediate) */
  PUT2(SVE_64_bit_gather_load_vector_plus_immediate0,
       NM("ld1sb", "ldff1sb", "ld1b","ldff1b"),
       OPS(BRA_O, ZREG_D, BRA_C, PG_Z, PTR_O, ZREG_D, PTR_C), OPS(BRA_O, ZREG_D, BRA_C, PG_Z, PTR_O, ZREG_D, IMM5BIT, PTR_C));

  PUT2(SVE_64_bit_gather_load_vector_plus_immediate1,
       NM("ld1sh", "ldff1sh", "ld1h", "ldff1h"),
       OPS(BRA_O, ZREG_D, BRA_C, PG_Z, PTR_O, ZREG_D, PTR_C), OPS(BRA_O, ZREG_D, BRA_C, PG_Z, PTR_O, ZREG_D, SPECIFIC32_4, PTR_C));
  
  PUT2(SVE_64_bit_gather_load_vector_plus_immediate2,
       NM("ld1sw", "ldff1sw"),
       OPS(BRA_O, ZREG_D, BRA_C, PG_Z, PTR_O, ZREG_D, PTR_C), OPS(BRA_O, ZREG_D, BRA_C, PG_Z, PTR_O, ZREG_D, SPECIFIC32_5, PTR_C));
  
  PUT2(SVE_64_bit_gather_load_vector_plus_immediate3,
       NM("ld1d", "ldff1d"),
       OPS(BRA_O, ZREG_D, BRA_C, PG_Z, PTR_O, ZREG_D, PTR_C), OPS(BRA_O, ZREG_D, BRA_C, PG_Z, PTR_O, ZREG_D, SPECIFIC64, PTR_C));

  void putSVE_64_bit_gather_load_vector_plus_immediate() {
    putSVE_64_bit_gather_load_vector_plus_immediate0();
    putSVE_64_bit_gather_load_vector_plus_immediate1();
    putSVE_64_bit_gather_load_vector_plus_immediate2();
    putSVE_64_bit_gather_load_vector_plus_immediate3();
  }
  
  /*** SVE 64-bit gather load (scalar plus 64-bit unscaled offsets) */
  PUT1(SVE_64_bit_gather_load_scalar_plus_64_bit_unscaled_offset0,
       NM("ld1sb", "ldff1sb", "ld1b", "ldff1b", "ld1sh", "ldff1sh", "ld1h", "ldff1h",
	  "ld1sw", "ldff1sw", "ld1w", "ldff1w", "ld1d", "ldff1d"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, PTR_C));

  void putSVE_64_bit_gather_load_scalar_plus_64_bit_unscaled_offset() {
    putSVE_64_bit_gather_load_scalar_plus_64_bit_unscaled_offset0();
  }
  
  /*** SVE 64-bit gather load (scalar plus 64-bit scaled offsets) */
  PUT1(SVE_64_bit_gather_load_scalar_plus_64_bit_scaled_offsets0,
       NM("ld1sh", "ldff1sh", "ld1h", "ldff1h"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, SPECIFIC32, PTR_C));
  PUT1(SVE_64_bit_gather_load_scalar_plus_64_bit_scaled_offsets1,
       NM("ld1sw", "ldff1sw", "ld1w", "ldff1w"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, SPECIFIC32_1, PTR_C));
  PUT1(SVE_64_bit_gather_load_scalar_plus_64_bit_scaled_offsets2,
       NM("ld1d", "ldff1d"),
       OPS(ZREG_D, PG_Z, PTR_O, XNSP, ZREG_D, SPECIFIC32_2, PTR_C));

  void putSVE_64_bit_gather_load_scalar_plus_64_bit_scaled_offsets() {
    putSVE_64_bit_gather_load_scalar_plus_64_bit_scaled_offsets0();
    putSVE_64_bit_gather_load_scalar_plus_64_bit_scaled_offsets1();
    putSVE_64_bit_gather_load_scalar_plus_64_bit_scaled_offsets2();
  }

  /*** SVE 64-bit gather prefetch (scalar plus unpacked 32-bit scaled offsets) */
  PUT1(SVE_64_bit_gather_prefetch_scalar_plus_unpacked_32_bit_scaled_offsets0,
       NM("prfb"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, ZREG_D, XTW, PTR_C));
  PUT1(SVE_64_bit_gather_prefetch_scalar_plus_unpacked_32_bit_scaled_offsets1,
       NM("prfh"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, ZREG_D, SPECIFIC32, PTR_C));
  PUT1(SVE_64_bit_gather_prefetch_scalar_plus_unpacked_32_bit_scaled_offsets2,
       NM("prfw"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, ZREG_D, SPECIFIC32_1, PTR_C));
  PUT1(SVE_64_bit_gather_prefetch_scalar_plus_unpacked_32_bit_scaled_offsets3,
       NM("prfd"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, ZREG_D, SPECIFIC32_2, PTR_C));
  void putSVE_64_bit_gather_prefetch_scalar_plus_unpacked_32_bit_scaled_offsets() {
    putSVE_64_bit_gather_prefetch_scalar_plus_unpacked_32_bit_scaled_offsets0();
    putSVE_64_bit_gather_prefetch_scalar_plus_unpacked_32_bit_scaled_offsets1();
    putSVE_64_bit_gather_prefetch_scalar_plus_unpacked_32_bit_scaled_offsets2();
    putSVE_64_bit_gather_prefetch_scalar_plus_unpacked_32_bit_scaled_offsets3();
  }

  /*** SVE 64-bit gather prefetch (scalar plus 64-bit scaled offsets) */
  PUT1(SVE_64_bit_gather_prefetch_scalar_plus_64_bit_scaled_offsets0,
       NM("prfb"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, ZREG_D, PTR_C));
  PUT1(SVE_64_bit_gather_prefetch_scalar_plus_64_bit_scaled_offsets1,
       NM("prfh"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, ZREG_D, SPECIFIC32, PTR_C));
  PUT1(SVE_64_bit_gather_prefetch_scalar_plus_64_bit_scaled_offsets2,
       NM("prfw"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, ZREG_D, SPECIFIC32_1, PTR_C));
  PUT1(SVE_64_bit_gather_prefetch_scalar_plus_64_bit_scaled_offsets3,
       NM("prfd"),
       OPS(PRFOP_SVE, PG, PTR_O, XNSP, ZREG_D, SPECIFIC32_2, PTR_C));

  void putSVE_64_bit_gather_prefetch_scalar_plus_64_bit_scaled_offsets() {
    putSVE_64_bit_gather_prefetch_scalar_plus_64_bit_scaled_offsets0();
    putSVE_64_bit_gather_prefetch_scalar_plus_64_bit_scaled_offsets1();
    putSVE_64_bit_gather_prefetch_scalar_plus_64_bit_scaled_offsets2();
    putSVE_64_bit_gather_prefetch_scalar_plus_64_bit_scaled_offsets3();
  }
  
  /** SVE Memory - Store */
  /*** SVE contiguous store (scalar plus scalar) */
  PUT4(SVE_contiguous_store_scalar_plus_scalar0,
       NM("st1b"),
       OPS(BRA_O, ZREG_B, BRA_C, PG, PTR_O, XNSP, XREG, PTR_C),
       OPS(BRA_O, ZREG_H, BRA_C, PG, PTR_O, XNSP, XREG, PTR_C),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, XREG, PTR_C),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, XREG, PTR_C));
  PUT3(SVE_contiguous_store_scalar_plus_scalar1,
       NM("st1h"),
       OPS(BRA_O, ZREG_H, BRA_C, PG, PTR_O, XNSP, XREG, SPECIFIC0, PTR_C),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, XREG, SPECIFIC0, PTR_C),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, XREG, SPECIFIC0, PTR_C));
  PUT2(SVE_contiguous_store_scalar_plus_scalar2,
       NM("st1w"),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, XREG, SPECIFIC1, PTR_C),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, XREG, SPECIFIC1, PTR_C));
  PUT1(SVE_contiguous_store_scalar_plus_scalar3,
       NM("st1d"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, XREG, SPECIFIC2, PTR_C));

  void putSVE_contiguous_store_scalar_plus_scalar() {
    putSVE_contiguous_store_scalar_plus_scalar0();
    putSVE_contiguous_store_scalar_plus_scalar1();
    putSVE_contiguous_store_scalar_plus_scalar2();
    putSVE_contiguous_store_scalar_plus_scalar3();
  }
    
  /*** SVE store multiple structures (scalar plus scalar) */
  PUT1(SVE_store_multiple_structures_scalar_plus_scalar0,
       NM("st2b"),
       OPS(SPECIFIC0, PG, PTR_O, XNSP, XREG, PTR_C));
  PUT1(SVE_store_multiple_structures_scalar_plus_scalar1,
       NM("st3b"),
       OPS(SPECIFIC1, PG, PTR_O, XNSP, XREG, PTR_C));
  PUT1(SVE_store_multiple_structures_scalar_plus_scalar2,
       NM("st4b"),
       OPS(SPECIFIC2, PG, PTR_O, XNSP, XREG, PTR_C));
  PUT1(SVE_store_multiple_structures_scalar_plus_scalar3,
       NM("st2h"),
       OPS(SPECIFIC3, PG, PTR_O, XNSP, XREG, SPECIFIC64_3, PTR_C));
  PUT1(SVE_store_multiple_structures_scalar_plus_scalar4,
       NM("st3h"),
       OPS(SPECIFIC32, PG, PTR_O, XNSP, XREG, SPECIFIC64_3, PTR_C));
  PUT1(SVE_store_multiple_structures_scalar_plus_scalar5,
       NM("st4h"),
       OPS(SPECIFIC32_1, PG, PTR_O, XNSP, XREG, SPECIFIC64_3, PTR_C));
  PUT1(SVE_store_multiple_structures_scalar_plus_scalar6,
       NM("st2w"),
       OPS(SPECIFIC32_2, PG, PTR_O, XNSP, XREG, SPECIFIC64_4, PTR_C));
  PUT1(SVE_store_multiple_structures_scalar_plus_scalar7,
       NM("st3w"),
       OPS(SPECIFIC32_3, PG, PTR_O, XNSP, XREG, SPECIFIC64_4, PTR_C));
  PUT1(SVE_store_multiple_structures_scalar_plus_scalar8,
       NM("st4w"),
       OPS(SPECIFIC32_4, PG, PTR_O, XNSP, XREG, SPECIFIC64_4, PTR_C));
  PUT1(SVE_store_multiple_structures_scalar_plus_scalar9,
       NM("st2d"),
       OPS(SPECIFIC64, PG, PTR_O, XNSP, XREG, SPECIFIC64_5, PTR_C));
  PUT1(SVE_store_multiple_structures_scalar_plus_scalar10,
       NM("st3d"),
       OPS(SPECIFIC64_1, PG, PTR_O, XNSP, XREG, SPECIFIC64_5, PTR_C));
  PUT1(SVE_store_multiple_structures_scalar_plus_scalar11,
       NM("st4d"),
       OPS(SPECIFIC64_2, PG, PTR_O, XNSP, XREG, SPECIFIC64_5, PTR_C));

  void putSVE_store_multiple_structures_scalar_plus_scalar() {
    putSVE_store_multiple_structures_scalar_plus_scalar0();
    putSVE_store_multiple_structures_scalar_plus_scalar1();
    putSVE_store_multiple_structures_scalar_plus_scalar2();
    putSVE_store_multiple_structures_scalar_plus_scalar3();
    putSVE_store_multiple_structures_scalar_plus_scalar4();
    putSVE_store_multiple_structures_scalar_plus_scalar5();
    putSVE_store_multiple_structures_scalar_plus_scalar6();
    putSVE_store_multiple_structures_scalar_plus_scalar7();
    putSVE_store_multiple_structures_scalar_plus_scalar8();
    putSVE_store_multiple_structures_scalar_plus_scalar9();
    putSVE_store_multiple_structures_scalar_plus_scalar10();
    putSVE_store_multiple_structures_scalar_plus_scalar11();
  }

  /*** SVE contiguous store (scalar plus immediate) */
  PUT8(SVE_contiguous_store_scalar_plus_immediate0,
       NM("st1b"),
       OPS(BRA_O, ZREG_B, BRA_C, PG, PTR_O, XNSP, PTR_C), OPS(BRA_O, ZREG_B, BRA_C, PG, PTR_O, XNSP, SPECIFIC64_1, MULVL, PTR_C), 
       OPS(BRA_O, ZREG_H, BRA_C, PG, PTR_O, XNSP, PTR_C), OPS(BRA_O, ZREG_H, BRA_C, PG, PTR_O, XNSP, SPECIFIC64_1, MULVL, PTR_C), 
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, PTR_C), OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, SPECIFIC64_1, MULVL, PTR_C), 
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, PTR_C), OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, SPECIFIC64_1, MULVL, PTR_C));
  PUT6(SVE_contiguous_store_scalar_plus_immediate1,
       NM("st1h"),
       OPS(BRA_O, ZREG_H, BRA_C, PG, PTR_O, XNSP, PTR_C), OPS(BRA_O, ZREG_H, BRA_C, PG, PTR_O, XNSP, SPECIFIC64_1, MULVL, PTR_C), 
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, PTR_C), OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, SPECIFIC64_1, MULVL, PTR_C), 
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, PTR_C), OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, SPECIFIC64_1, MULVL, PTR_C));
  PUT4(SVE_contiguous_store_scalar_plus_immediate2,
       NM("st1w"),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, PTR_C), OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, SPECIFIC64_1, MULVL, PTR_C),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, PTR_C), OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, SPECIFIC64_1, MULVL, PTR_C));
  PUT2(SVE_contiguous_store_scalar_plus_immediate3,
       NM("st1d"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, PTR_C), OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, SPECIFIC64_1, MULVL, PTR_C));

  void putSVE_contiguous_store_scalar_plus_immediate() {
    putSVE_contiguous_store_scalar_plus_immediate0();
    putSVE_contiguous_store_scalar_plus_immediate1();
    putSVE_contiguous_store_scalar_plus_immediate2();
    putSVE_contiguous_store_scalar_plus_immediate3();
  }

  /*** SVE store multiple structures (scalar plus immediate) */
  PUT2(SVE_store_multiple_structures_scalar_plus_immediate0,
       NM("st2b"),
       OPS(SPECIFIC0, PG, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC0, PG, PTR_O, XNSP, SPECIFIC32_5, MULVL, PTR_C));
  PUT2(SVE_store_multiple_structures_scalar_plus_immediate1,
       NM("st3b"),
       OPS(SPECIFIC1, PG, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC1, PG, PTR_O, XNSP, SPECIFIC4, MULVL, PTR_C));
  PUT2(SVE_store_multiple_structures_scalar_plus_immediate2,
       NM("st4b"),
       OPS(SPECIFIC2, PG, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC2, PG, PTR_O, XNSP, SPECIFIC5, MULVL, PTR_C)); 
  PUT2(SVE_store_multiple_structures_scalar_plus_immediate3,
       NM("st2h"),
       OPS(SPECIFIC3, PG, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC3, PG, PTR_O, XNSP, SPECIFIC32_5, MULVL, PTR_C));
  PUT2(SVE_store_multiple_structures_scalar_plus_immediate4,
       NM("st3h"),
       OPS(SPECIFIC32, PG, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC32, PG, PTR_O, XNSP, SPECIFIC4, MULVL, PTR_C));
  PUT2(SVE_store_multiple_structures_scalar_plus_immediate5,
       NM("st4h"),
       OPS(SPECIFIC32_1, PG, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC32_1, PG, PTR_O, XNSP, SPECIFIC5, MULVL, PTR_C));
  PUT2(SVE_store_multiple_structures_scalar_plus_immediate6,
       NM("st2w"),
       OPS(SPECIFIC32_2, PG, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC32_2, PG, PTR_O, XNSP, SPECIFIC32_5, MULVL, PTR_C));
  PUT2(SVE_store_multiple_structures_scalar_plus_immediate7,
       NM("st3w"),
       OPS(SPECIFIC32_3, PG, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC32_3, PG, PTR_O, XNSP, SPECIFIC4, MULVL, PTR_C));
  PUT2(SVE_store_multiple_structures_scalar_plus_immediate8,
       NM("st4w"),
       OPS(SPECIFIC32_4, PG, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC32_4, PG, PTR_O, XNSP, SPECIFIC5, MULVL, PTR_C));
  PUT2(SVE_store_multiple_structures_scalar_plus_immediate9,
       NM("st2d"),
       OPS(SPECIFIC64, PG, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC64, PG, PTR_O, XNSP, SPECIFIC32_5, MULVL, PTR_C));
  PUT2(SVE_store_multiple_structures_scalar_plus_immediate10,
       NM("st3d"),
       OPS(SPECIFIC64_1, PG, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC64_1, PG, PTR_O, XNSP, SPECIFIC4, MULVL, PTR_C));
  PUT2(SVE_store_multiple_structures_scalar_plus_immediate11,
       NM("st4d"),
       OPS(SPECIFIC64_2, PG, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC64_2, PG, PTR_O, XNSP, SPECIFIC5, MULVL, PTR_C));

  void putSVE_store_multiple_structures_scalar_plus_immediate() {
    putSVE_store_multiple_structures_scalar_plus_immediate0();
    putSVE_store_multiple_structures_scalar_plus_immediate1();
    putSVE_store_multiple_structures_scalar_plus_immediate2();
    putSVE_store_multiple_structures_scalar_plus_immediate3();
    putSVE_store_multiple_structures_scalar_plus_immediate4();
    putSVE_store_multiple_structures_scalar_plus_immediate5();
    putSVE_store_multiple_structures_scalar_plus_immediate6();
    putSVE_store_multiple_structures_scalar_plus_immediate7();
    putSVE_store_multiple_structures_scalar_plus_immediate8();
    putSVE_store_multiple_structures_scalar_plus_immediate9();
    putSVE_store_multiple_structures_scalar_plus_immediate10();
    putSVE_store_multiple_structures_scalar_plus_immediate11();
  }

  /*** SVE contiguous non-temporal store (scalar plus scalar) */
  PUT1(SVE_contiguous_non_temporal_store_scalar_plus_scalar0,
       NM("stnt1b"),
       OPS(BRA_O, ZREG_B, BRA_C, PG, PTR_O, XNSP, XREG, PTR_C));
  PUT1(SVE_contiguous_non_temporal_store_scalar_plus_scalar1,
       NM("stnt1h"),
       OPS(BRA_O, ZREG_H, BRA_C, PG, PTR_O, XNSP, XREG, SPECIFIC0, PTR_C));
  PUT1(SVE_contiguous_non_temporal_store_scalar_plus_scalar2,
       NM("stnt1w"),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, XREG, SPECIFIC1, PTR_C));
  PUT1(SVE_contiguous_non_temporal_store_scalar_plus_scalar3,
       NM("stnt1d"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, XREG, SPECIFIC2, PTR_C));

  void putSVE_contiguous_non_temporal_store_scalar_plus_scalar() {
    putSVE_contiguous_non_temporal_store_scalar_plus_scalar0();
    putSVE_contiguous_non_temporal_store_scalar_plus_scalar1();
    putSVE_contiguous_non_temporal_store_scalar_plus_scalar2();
    putSVE_contiguous_non_temporal_store_scalar_plus_scalar3();
  }

  /*** SVE 64-bit scatter store (scalar plus unpacked 32-bit unscaled offsets) */
  PUT1(SVE_64_bit_scatter_store_scalar_plus_unpacked_32_bit_unscaled_offsets0,
       NM("st1b", "st1h", "st1w", "st1d"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, ZREG_D, XTW, PTR_C));

  void putSVE_64_bit_scatter_store_scalar_plus_unpacked_32_bit_unscaled_offsets() {
    putSVE_64_bit_scatter_store_scalar_plus_unpacked_32_bit_unscaled_offsets0();
  }

  /*** SVE 64-bit scatter store (scalar plus 64-bit unscaled offsets) */
  PUT1(SVE_64_bit_scatter_store_scalar_plus_64_bit_unscaled_offsets0,
       NM("st1b", "st1h", "st1w", "st1d"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, ZREG_D, PTR_C));

  void putSVE_64_bit_scatter_store_scalar_plus_64_bit_unscaled_offsets() {
    putSVE_64_bit_scatter_store_scalar_plus_64_bit_unscaled_offsets0();
  }

  /*** SVE contiguous non-temporal store (scalar plus immediate) */
  PUT2(SVE_contiguous_non_temporal_store_scalar_plus_immeidate0,
       NM("stnt1b"),
       OPS(BRA_O, ZREG_B, BRA_C, PG, PTR_O, XNSP, PTR_C),
       OPS(BRA_O, ZREG_B, BRA_C, PG, PTR_O, XNSP, SPECIFIC64_1, MULVL, PTR_C));
  PUT2(SVE_contiguous_non_temporal_store_scalar_plus_immeidate1,
       NM("stnt1h"),
       OPS(BRA_O, ZREG_H, BRA_C, PG, PTR_O, XNSP, PTR_C),
       OPS(BRA_O, ZREG_H, BRA_C, PG, PTR_O, XNSP, SPECIFIC64_1, MULVL,  PTR_C));
  PUT2(SVE_contiguous_non_temporal_store_scalar_plus_immeidate2,
       NM("stnt1w"),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, PTR_C),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, SPECIFIC64_1, MULVL, PTR_C));
  PUT2(SVE_contiguous_non_temporal_store_scalar_plus_immeidate3,
       NM("stnt1d"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, PTR_C),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, SPECIFIC64_1, MULVL, PTR_C));

  void putSVE_contiguous_non_temporal_store_scalar_plus_immeidate() {
    putSVE_contiguous_non_temporal_store_scalar_plus_immeidate0();
    putSVE_contiguous_non_temporal_store_scalar_plus_immeidate1();
    putSVE_contiguous_non_temporal_store_scalar_plus_immeidate2();
    putSVE_contiguous_non_temporal_store_scalar_plus_immeidate3();
  }

  /*** SVE 64-bit scatter store (scalar plus unpacked 32-bit scaled offsets) */
  PUT1(SVE_64_bit_scatter_store_scalar_plus_unpacked_32_bit_scaled_offsets0,
       NM("st1h"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, ZREG_D, SPECIFIC3, PTR_C));
  PUT1(SVE_64_bit_scatter_store_scalar_plus_unpacked_32_bit_scaled_offsets1,
       NM("st1w"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, ZREG_D, SPECIFIC4, PTR_C));
  PUT1(SVE_64_bit_scatter_store_scalar_plus_unpacked_32_bit_scaled_offsets2,
       NM("st1d"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, ZREG_D, SPECIFIC5, PTR_C));

  void putSVE_64_bit_scatter_store_scalar_plus_unpacked_32_bit_scaled_offsets() {
    putSVE_64_bit_scatter_store_scalar_plus_unpacked_32_bit_scaled_offsets0();
    putSVE_64_bit_scatter_store_scalar_plus_unpacked_32_bit_scaled_offsets1();
    putSVE_64_bit_scatter_store_scalar_plus_unpacked_32_bit_scaled_offsets2();
  }

  /*** SVE 64-bit scatter store (scalar plus 64-bit scaled offsets) */
  PUT1(SVE_64_bit_scatter_store_scalar_plus_64_bit_scaled_offsets0,
       NM("st1h"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, ZREG_D, SPECIFIC3, PTR_C));
  PUT1(SVE_64_bit_scatter_store_scalar_plus_64_bit_scaled_offsets1,
       NM("st1w"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, ZREG_D, SPECIFIC4, PTR_C));
  PUT1(SVE_64_bit_scatter_store_scalar_plus_64_bit_scaled_offsets2,
       NM("st1d"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, XNSP, ZREG_D, SPECIFIC5, PTR_C));

  void putSVE_64_bit_scatter_store_scalar_plus_64_bit_scaled_offsets() {
    putSVE_64_bit_scatter_store_scalar_plus_64_bit_scaled_offsets0();
    putSVE_64_bit_scatter_store_scalar_plus_64_bit_scaled_offsets1();
    putSVE_64_bit_scatter_store_scalar_plus_64_bit_scaled_offsets2();
  }

  /*** SVE 32-bit scatter store (scalar plus 32-bit unscaled offsets) */
  PUT1(SVE_32_bit_scatter_store_scalar_plus_32_bit_unscaled_offsets0,
       NM("st1b"),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, ZREG_S, XTW, PTR_C));
  PUT1(SVE_32_bit_scatter_store_scalar_plus_32_bit_unscaled_offsets1,
       NM("st1h"),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, ZREG_S, XTW, PTR_C));
  PUT1(SVE_32_bit_scatter_store_scalar_plus_32_bit_unscaled_offsets2,
       NM("st1w"),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, ZREG_S, XTW, PTR_C));

  void putSVE_32_bit_scatter_store_scalar_plus_32_bit_unscaled_offsets() {
    putSVE_32_bit_scatter_store_scalar_plus_32_bit_unscaled_offsets0();
    putSVE_32_bit_scatter_store_scalar_plus_32_bit_unscaled_offsets1();
    putSVE_32_bit_scatter_store_scalar_plus_32_bit_unscaled_offsets2();
  }

  /*** SVE 64-bit scatter store (vector plus immediate) */
  PUT2(SVE_64_bit_scatter_store_vector_plus_immediate0,
       NM("st1b"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, ZREG_D, PTR_C), OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, ZREG_D, IMM5BIT, PTR_C));

  PUT2(SVE_64_bit_scatter_store_vector_plus_immediate1,
       NM("st1h"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, ZREG_D, PTR_C), OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, ZREG_D, SPECIFIC32_1, PTR_C));
  
  PUT2(SVE_64_bit_scatter_store_vector_plus_immediate2,
       NM("st1w"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, ZREG_D, PTR_C), OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, ZREG_D, SPECIFIC32_2, PTR_C));
  
  PUT2(SVE_64_bit_scatter_store_vector_plus_immediate3,
       NM("st1d"),
       OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, ZREG_D, PTR_C), OPS(BRA_O, ZREG_D, BRA_C, PG, PTR_O, ZREG_D, SPECIFIC32_3, PTR_C));
  
  void putSVE_64_bit_scatter_store_vector_plus_immediate() {
    putSVE_64_bit_scatter_store_vector_plus_immediate0();
    putSVE_64_bit_scatter_store_vector_plus_immediate1();
    putSVE_64_bit_scatter_store_vector_plus_immediate2();
    putSVE_64_bit_scatter_store_vector_plus_immediate3();
  }

  /*** SVE 32-bit scatter store (scalar plus 32-bit scaled offsets) */
  PUT1(SVE_32_bit_scatter_store_scalar_plus_32_bit_scaled_offsets0,
       NM("st1h"),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, ZREG_S, SPECIFIC3, PTR_C));
  PUT1(SVE_32_bit_scatter_store_scalar_plus_32_bit_scaled_offsets1,
       NM("st1w"),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, XNSP, ZREG_S, SPECIFIC4, PTR_C));

  void putSVE_32_bit_scatter_store_scalar_plus_32_bit_scaled_offsets() {
    putSVE_32_bit_scatter_store_scalar_plus_32_bit_scaled_offsets0();
    putSVE_32_bit_scatter_store_scalar_plus_32_bit_scaled_offsets1();
  }

  /*** SVE 32-bit scatter store (vector plus immediate) */
  PUT2(SVE_32_bit_scatter_store_vector_plus_immediate0,
       NM("st1b"),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, ZREG_S, PTR_C), OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, ZREG_S, IMM5BIT, PTR_C));
  PUT2(SVE_32_bit_scatter_store_vector_plus_immediate1,
       NM("st1h"),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, ZREG_S, PTR_C), OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, ZREG_S, SPECIFIC32_1, PTR_C));
  PUT2(SVE_32_bit_scatter_store_vector_plus_immediate2,
       NM("st1w"),
       OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, ZREG_S, PTR_C), OPS(BRA_O, ZREG_S, BRA_C, PG, PTR_O, ZREG_S, SPECIFIC32_2, PTR_C));

  void putSVE_32_bit_scatter_store_vector_plus_immediate() {
    putSVE_32_bit_scatter_store_vector_plus_immediate0();
  }

  /*** SVE store predicate register */
  PUT2(SVE_store_predicate_register0,
       NM("str"),
       OPS(SPECIFIC32, PTR_O, XNSP, PTR_C),
       OPS(SPECIFIC32, PTR_O, XNSP, SPECIFIC32_4, MULVL, PTR_C));

  void putSVE_store_predicate_register() {
    putSVE_store_predicate_register0();
  }
  /*** SVE store vector register */
  PUT2(SVE_store_vector_register0,
       NM("str"),
       OPS(ZREG, PTR_O, XNSP, PTR_C), OPS(ZREG, PTR_O, XNSP, SPECIFIC32_4, MULVL, PTR_C));

  void putSVE_store_vector_register() {
    putSVE_store_vector_register0();
  }


  void putSVE_Memory32bitGatherandUnsizedContiguous() {
    std::vector<int> tmp = { 127, -256, -128, -64, -32, -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 16, 32, 64, 128, 255 };
    std::vector<int> tmp0 = { 15, 0, 1, 2, 4, 8, 16, 31 };
    std::vector<int> tmp2 = { 30, 0, 2, 4, 8, 16, 32, 62 };
    std::vector<int> tmp3 = { -8, -4, -2, -1, 0, 1, 2, 4, 7 };
    std::vector<int> tmp4 = { 124, 0, 2, 4, 8, 16, 32, 64, 126 };
    std::vector<std::string> preg = { "p7", "p0", "p1", "p2", "p4" };
    
    clearTvAndJtv();
    
    tv_SPECIFIC32_1.push_back("UXTW 1");
    tv_SPECIFIC32_1.push_back("SXTW 1");
    tv_SPECIFIC32_2.push_back("UXTW 2");
    tv_SPECIFIC32_2.push_back("SXTW 2");
    tv_SPECIFIC32_3.push_back("UXTW 3");
    tv_SPECIFIC32_3.push_back("SXTW 3");
    jtv_SPECIFIC32_1.push_back("UXTW, 1");
    jtv_SPECIFIC32_1.push_back("SXTW, 1");
    jtv_SPECIFIC32_2.push_back("UXTW, 2");
    jtv_SPECIFIC32_2.push_back("SXTW, 2");
    jtv_SPECIFIC32_3.push_back("UXTW, 3");
    jtv_SPECIFIC32_3.push_back("SXTW, 3");

    for(int i : tmp) {
      tv_SPECIFIC64.push_back(std::to_string(i));
      jtv_SPECIFIC64.push_back(std::to_string(i));
    }

    for(std::string i : preg) {
      tv_SPECIFIC64_1.push_back(i);
      jtv_SPECIFIC64_1.push_back(i);
    }

    tv_SPECIFIC64_4.push_back("LSL 1");
    jtv_SPECIFIC64_4.push_back("LSL, 1");
    tv_SPECIFIC64_3.push_back("LSL 2");
    jtv_SPECIFIC64_3.push_back("LSL, 2");
    tv_SPECIFIC64_2.push_back("LSL 3");
    jtv_SPECIFIC64_2.push_back("LSL, 3");

    for(int i : tmp2) {
      tv_SPECIFIC32_4.push_back(std::to_string(i));
      jtv_SPECIFIC32_4.push_back(std::to_string(i));
    }


    for(int i : tmp3) {
      tv_SPECIFIC32_5.push_back(std::to_string(i));
      jtv_SPECIFIC32_5.push_back(std::to_string(i));
    }

    for(int i : tmp0) {
      tv_SPECIFIC1.push_back(std::to_string(i*2));
      tv_SPECIFIC2.push_back(std::to_string(i*4));
      tv_SPECIFIC3.push_back(std::to_string(i*8));
      jtv_SPECIFIC1.push_back(std::to_string(i*2));
      jtv_SPECIFIC2.push_back(std::to_string(i*4));
      jtv_SPECIFIC3.push_back(std::to_string(i*8));
    }

    putSVE_32_bit_gather_load_scalar_plus_32_bit_unscaled_offsets();
    putSVE_contiguous_prefetch_scalar_plus_scalar();
    putSVE_32_bit_gather_prefetch_vector_plus_immediate();
    putSVE_32_bit_gather_load_vector_plus_immediate();
    putSVE_load_and_broadcast_element();
    putSVE_32_bit_gather_prefetch_scalar_plus_32_bit_scaled_offsets();
    putSVE_32_bit_gather_load_halfwords_scalar_plus_32_bit_scaled_offsets();
    putSVE_32_bit_gather_load_words_scalar_plus_32_bit_scaled_offsets();
    putSVE_load_predicate_register();
    putSVE_contiguous_prefetch_scalar_plus_immediate();
  }
  
  void putSVE_MemoryContiguousLoad0() {
    std::vector<int> tmp0 = { 7, -8, -4, -2, -1, 0, 1, 2, 4 };
    clearTvAndJtv();
    
    for(int i : tmp0) {
      tv_SPECIFIC0.push_back(std::to_string(i*16));
      jtv_SPECIFIC0.push_back(std::to_string(i*16));
    }

    tv_SPECIFIC1.push_back("LSL 1");
    tv_SPECIFIC2.push_back("LSL 2");
    tv_SPECIFIC3.push_back("LSL 3");
    jtv_SPECIFIC1.push_back("LSL, 1");
    jtv_SPECIFIC2.push_back("LSL, 2");
    jtv_SPECIFIC3.push_back("LSL, 3");

    /** aarch64-linux-gnu-as can't encode
	ldff1sw { <Zt>.D }, <Pg>/Z, [<Xn|SP>]
	so that
	it should be feeded
	ldff1sw { <Zt>.D }, <Pg>/Z, [<Xn|SP>{, <Xm>, LSL #2}]
    */
    tv_SPECIFIC4.push_back("LSL 1");
    tv_SPECIFIC5.push_back("LSL 2");
    tv_SPECIFIC6.push_back("LSL 3");
    /** Don't add "LSL 1", "LSL 2", "LSL 3"
	to test xbyak_aarch64 with ommitted operands. */
    jtv_SPECIFIC4.push_back(" "); /** Space is required to distinguish isRequireComma function . */
    jtv_SPECIFIC5.push_back(" "); /** Space is required to distinguish isRequireComma function . */
    jtv_SPECIFIC6.push_back(" "); /** Space is required to distinguish isRequireComma function . */

    for(int i : tmp0) {
      tv_SPECIFIC32.push_back(std::to_string(i));
      tv_SPECIFIC32_1.push_back(std::to_string(i*2));
      jtv_SPECIFIC32.push_back(std::to_string(i));
      jtv_SPECIFIC32_1.push_back(std::to_string(i*2));
    }

    tv_SPECIFIC64.push_back("xzr");
    jtv_SPECIFIC64.push_back("xzr");

    for(std::string i : tv_XREG) {
      tv_SPECIFIC64_1.push_back(i);
      jtv_SPECIFIC64_1.push_back(i);
    }
    tv_SPECIFIC64_1.push_back("xzr");
    jtv_SPECIFIC64_1.push_back("xzr");
    
    
    for(std::string i : tv_XREG) {
      tv_SPECIFIC64_2.push_back(i);
      jtv_SPECIFIC64_2.push_back(i);
    }
    tv_SPECIFIC64_2.push_back("xzr");
    jtv_SPECIFIC64_2.push_back("xzr");


    putSVE_load_and_broadcast_quadword_scalar_plus_scalar();
    putSVE_contiguous_load_scalar_plus_scalar();
    putSVE_contiguous_first_fault_load_scalar_plus_scalar();
  }


  void putSVE_MemoryContiguousLoad1() {
    std::vector<int> tmp0 = { 15, 0, 1, 2, 4, 8, 16, 31 };
    std::vector<int> tmp1 = { 7, -8, -4, -2, -1, 0, 1, 2, 4 };
    clearTvAndJtv();

    for(int i : tmp0) {
      std::string tmpStr = "z" + std::to_string(i);
      std::string tmpStr1 = "z" + std::to_string((i+1)%32);
      std::string tmpStr2 = "z" + std::to_string((i+2)%32);
      std::string tmpStr3 = "z" + std::to_string((i+3)%32);
      
      tv_SPECIFIC0.push_back("{ " + tmpStr + ".b, " + tmpStr1 + ".b }");
      jtv_SPECIFIC0.push_back(tmpStr + ".b");

      tv_SPECIFIC1.push_back("{ " + tmpStr + ".b, " + tmpStr1 + ".b, " + tmpStr2 + ".b }");
      jtv_SPECIFIC1.push_back(tmpStr + ".b");
    
      tv_SPECIFIC2.push_back("{ " + tmpStr + ".b, " + tmpStr1 + ".b, " + tmpStr2 + ".b, " + tmpStr3 + ".b }");
      jtv_SPECIFIC2.push_back(tmpStr + ".b");

      tv_SPECIFIC3.push_back("{ " + tmpStr + ".h, " + tmpStr1 + ".h }");
      jtv_SPECIFIC3.push_back(tmpStr + ".h");

      tv_SPECIFIC32.push_back("{ " + tmpStr + ".h, " + tmpStr1 + ".h, " + tmpStr2 + ".h }");
      jtv_SPECIFIC32.push_back(tmpStr + ".h");
    
      tv_SPECIFIC32_1.push_back("{ " + tmpStr + ".h, " + tmpStr1 + ".h, " + tmpStr2 + ".h, " + tmpStr3 + ".h }");
      jtv_SPECIFIC32_1.push_back(tmpStr + ".h");
      
      tv_SPECIFIC32_2.push_back("{ " + tmpStr + ".s, " + tmpStr1 + ".s }");
      jtv_SPECIFIC32_2.push_back(tmpStr + ".s");

      tv_SPECIFIC32_3.push_back("{ " + tmpStr + ".s, " + tmpStr1 + ".s, " + tmpStr2 + ".s }");
      jtv_SPECIFIC32_3.push_back(tmpStr + ".s");
    
      tv_SPECIFIC32_4.push_back("{ " + tmpStr + ".s, " + tmpStr1 + ".s, " + tmpStr2 + ".s, " + tmpStr3 + ".s }");
      jtv_SPECIFIC32_4.push_back(tmpStr + ".s");

      tv_SPECIFIC64.push_back("{ " + tmpStr + ".d, " + tmpStr1 + ".d }");
      jtv_SPECIFIC64.push_back(tmpStr + ".d");

      tv_SPECIFIC64_1.push_back("{ " + tmpStr + ".d, " + tmpStr1 + ".d, " + tmpStr2 + ".d }");
      jtv_SPECIFIC64_1.push_back(tmpStr + ".d");
    
      tv_SPECIFIC64_2.push_back("{ " + tmpStr + ".d, " + tmpStr1 + ".d, " + tmpStr2 + ".d, " + tmpStr3 + ".d }");
      jtv_SPECIFIC64_2.push_back(tmpStr + ".d");
    }

    tv_SPECIFIC64_3.push_back("LSL 1");
    jtv_SPECIFIC64_3.push_back("LSL, 1");
    tv_SPECIFIC64_4.push_back("LSL 2");
    jtv_SPECIFIC64_4.push_back("LSL, 2");
    tv_SPECIFIC64_5.push_back("LSL 3");
    jtv_SPECIFIC64_5.push_back("LSL, 3");

    
    for(int i : tmp1) {
      tv_SPECIFIC32_5.push_back(std::to_string(i*16));
      jtv_SPECIFIC32_5.push_back(std::to_string(i*16));

      tv_SPECIFIC4.push_back(std::to_string(i*2));
      jtv_SPECIFIC4.push_back(std::to_string(i*2));

      tv_SPECIFIC5.push_back(std::to_string(i*3));
      jtv_SPECIFIC5.push_back(std::to_string(i*3));

      tv_SPECIFIC6.push_back(std::to_string(i*4));
      jtv_SPECIFIC6.push_back(std::to_string(i*4));
    }

    putSVE_load_multiple_structures_scalar_plus_scalar();
    putSVE_load_and_broadcast_quadword_scalar_plus_immediate();
    putSVE_load_multiple_structures_scalar_plus_immediate();
  }

  void putSVE_MemoryContiguousLoad2() {
    std::vector<int> tmp0 = { 7, -8, -4, -2, -1, 0, 1, 2, 4 };
    clearTvAndJtv();

    for(int i : tmp0) {
      tv_SPECIFIC32.push_back(std::to_string(i));
      jtv_SPECIFIC32.push_back(std::to_string(i));
    }      

    tv_SPECIFIC1.push_back("LSL 1");
    tv_SPECIFIC2.push_back("LSL 2");
    tv_SPECIFIC3.push_back("LSL 3");
    jtv_SPECIFIC1.push_back("LSL, 1");
    jtv_SPECIFIC2.push_back("LSL, 2");
    jtv_SPECIFIC3.push_back("LSL, 3");
    
    putSVE_contiguous_load_scalar_plus_immediate();
    putSVE_contiguous_non_fault_load_scalar_plus_immediate();
    putSVE_contiguous_non_temporal_load_scalar_plus_scalar();
    putSVE_contiguous_non_temporal_load_scalar_plus_immediate();
  }    

  void putSVE_Memory64bitGather() {
    std::vector<int> tmp0 = { 15, 0, 1, 2, 4, 8, 16, 31 };
    
    clearTvAndJtv();

    tv_SPECIFIC0.push_back("UXTW 1");
    tv_SPECIFIC0.push_back("SXTW 1");
    tv_SPECIFIC1.push_back("UXTW 2");
    tv_SPECIFIC1.push_back("SXTW 2");
    tv_SPECIFIC2.push_back("UXTW 3");
    tv_SPECIFIC2.push_back("SXTW 3");
    tv_SPECIFIC32.push_back("LSL 1");
    tv_SPECIFIC32_1.push_back("LSL 2");
    tv_SPECIFIC32_2.push_back("LSL 3");

    
    jtv_SPECIFIC0.push_back("UXTW, 1");
    jtv_SPECIFIC0.push_back("SXTW, 1");
    jtv_SPECIFIC1.push_back("UXTW, 2");
    jtv_SPECIFIC1.push_back("SXTW, 2");
    jtv_SPECIFIC2.push_back("UXTW, 3");
    jtv_SPECIFIC2.push_back("SXTW, 3");
    jtv_SPECIFIC32.push_back("LSL, 1");
    jtv_SPECIFIC32_1.push_back("LSL, 2");
    jtv_SPECIFIC32_2.push_back("LSL, 3");

    for(int i : tmp0) {
      std::string tmp = std::to_string(i);
      tv_SPECIFIC32_3.push_back(tmp);
      jtv_SPECIFIC32_3.push_back(tmp);

      tmp = std::to_string(i*2);
      tv_SPECIFIC32_4.push_back(tmp);
      jtv_SPECIFIC32_4.push_back(tmp);

      tmp = std::to_string(i*4);
      tv_SPECIFIC32_5.push_back(tmp);
      jtv_SPECIFIC32_5.push_back(tmp);

      tmp = std::to_string(i*8);
      tv_SPECIFIC64.push_back(tmp);
      jtv_SPECIFIC64.push_back(tmp);
    }
    
    putSVE_64_bit_gather_load_scalar_plus_unpacked_32_bit_unscaled_offsets();
    putSVE_64_bit_gather_load_scalar_plus_32_bit_unpacked_scaled_offsets();
    putSVE_64_bit_gather_prefetch_vector_plus_immediate();
    putSVE_64_bit_gather_load_vector_plus_immediate();
    putSVE_64_bit_gather_load_scalar_plus_64_bit_unscaled_offset();
    putSVE_64_bit_gather_load_scalar_plus_64_bit_scaled_offsets();
    putSVE_64_bit_gather_prefetch_scalar_plus_unpacked_32_bit_scaled_offsets();
    putSVE_64_bit_gather_prefetch_scalar_plus_64_bit_scaled_offsets();
  }

  void putSVE_MemoryStore0() {
    std::vector<int> tmp0 = { 7, 0, 1, 2, 4 };
    std::vector<int> tmp1 = { 127, -256, -128, -64, -32, -16, -4, -2, -1, 0, 1, 2, 4, 8, 16, 32, 64, 128, 255 };
    std::vector<int> tmp2 = { 7, -8, -4, -2, -1, 0, 1, 2, 4 };
    std::vector<int> tmp3 = { 15, 0, 1, 2, 4, 8, 16, 31 };
    std::vector<int> tmp4 = { 127, -256, -128, -64, -32, -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 16, 32, 64, 128, 255 };
    clearTvAndJtv();

    for(int i : tmp0) {
      tv_SPECIFIC32.push_back("p" + std::to_string(i));
      jtv_SPECIFIC32.push_back("p" + std::to_string(i));
    }

    for(int i : tmp1) {
      tv_SPECIFIC64.push_back(std::to_string(i));
      jtv_SPECIFIC64.push_back(std::to_string(i));
    }

    for(int i : tmp2) {
      tv_SPECIFIC64_1.push_back(std::to_string(i));
      jtv_SPECIFIC64_1.push_back(std::to_string(i));
    }
    
    tv_SPECIFIC0.push_back("LSL 1");
    tv_SPECIFIC1.push_back("LSL 2");
    tv_SPECIFIC2.push_back("LSL 3");
    jtv_SPECIFIC0.push_back("LSL, 1");
    jtv_SPECIFIC1.push_back("LSL, 2");
    jtv_SPECIFIC2.push_back("LSL, 3");
   
    tv_SPECIFIC3.push_back("UXTW 1");
    tv_SPECIFIC3.push_back("SXTW 1");
    tv_SPECIFIC4.push_back("UXTW 2");
    tv_SPECIFIC4.push_back("SXTW 2");
    tv_SPECIFIC5.push_back("UXTW 3");
    tv_SPECIFIC5.push_back("SXTW 3");
    jtv_SPECIFIC3.push_back("UXTW, 1");
    jtv_SPECIFIC3.push_back("SXTW, 1");
    jtv_SPECIFIC4.push_back("UXTW, 2");
    jtv_SPECIFIC4.push_back("SXTW, 2");
    jtv_SPECIFIC5.push_back("UXTW, 3");
    jtv_SPECIFIC5.push_back("SXTW, 3");

    for(int i : tmp3) {
      tv_SPECIFIC32_1.push_back(std::to_string(i*2));
      jtv_SPECIFIC32_1.push_back(std::to_string(i*2));
      tv_SPECIFIC32_2.push_back(std::to_string(i*4));
      jtv_SPECIFIC32_2.push_back(std::to_string(i*4));
      tv_SPECIFIC32_3.push_back(std::to_string(i*8));
      jtv_SPECIFIC32_3.push_back(std::to_string(i*8));
    }

    for(int i : tmp4) {
      tv_SPECIFIC32_4.push_back(std::to_string(i));
      jtv_SPECIFIC32_4.push_back(std::to_string(i));
    }

    putSVE_contiguous_store_scalar_plus_scalar();
    putSVE_contiguous_store_scalar_plus_immediate();
    putSVE_contiguous_non_temporal_store_scalar_plus_scalar();
    putSVE_64_bit_scatter_store_scalar_plus_unpacked_32_bit_unscaled_offsets();
    putSVE_64_bit_scatter_store_scalar_plus_64_bit_unscaled_offsets();
    putSVE_contiguous_non_temporal_store_scalar_plus_immeidate();
    putSVE_64_bit_scatter_store_scalar_plus_unpacked_32_bit_scaled_offsets();
    putSVE_64_bit_scatter_store_scalar_plus_64_bit_scaled_offsets();
    putSVE_32_bit_scatter_store_scalar_plus_32_bit_unscaled_offsets();
    putSVE_64_bit_scatter_store_vector_plus_immediate();
    putSVE_32_bit_scatter_store_scalar_plus_32_bit_scaled_offsets();
    putSVE_32_bit_scatter_store_vector_plus_immediate();
    putSVE_store_predicate_register();
    putSVE_store_vector_register();
  }

  void putSVE_MemoryStore1() {
    std::vector<int> tmp0 = { 15, 0, 1, 2, 4, 8, 16, 31 };
    std::vector<int> tmp1 = { 7, -8, -4, -2, -1, 0, 1, 2, 4 };
    clearTvAndJtv();

    for(int i : tmp0) {
      std::string tmpStr = "z" + std::to_string(i);
      std::string tmpStr1 = "z" + std::to_string((i+1)%32);
      std::string tmpStr2 = "z" + std::to_string((i+2)%32);
      std::string tmpStr3 = "z" + std::to_string((i+3)%32);
      
      tv_SPECIFIC0.push_back("{ " + tmpStr + ".b, " + tmpStr1 + ".b }");
      jtv_SPECIFIC0.push_back(tmpStr + ".b");

      tv_SPECIFIC1.push_back("{ " + tmpStr + ".b, " + tmpStr1 + ".b, " + tmpStr2 + ".b }");
      jtv_SPECIFIC1.push_back(tmpStr + ".b");
    
      tv_SPECIFIC2.push_back("{ " + tmpStr + ".b, " + tmpStr1 + ".b, " + tmpStr2 + ".b, " + tmpStr3 + ".b }");
      jtv_SPECIFIC2.push_back(tmpStr + ".b");

      tv_SPECIFIC3.push_back("{ " + tmpStr + ".h, " + tmpStr1 + ".h }");
      jtv_SPECIFIC3.push_back(tmpStr + ".h");

      tv_SPECIFIC32.push_back("{ " + tmpStr + ".h, " + tmpStr1 + ".h, " + tmpStr2 + ".h }");
      jtv_SPECIFIC32.push_back(tmpStr + ".h");
    
      tv_SPECIFIC32_1.push_back("{ " + tmpStr + ".h, " + tmpStr1 + ".h, " + tmpStr2 + ".h, " + tmpStr3 + ".h }");
      jtv_SPECIFIC32_1.push_back(tmpStr + ".h");
      
      tv_SPECIFIC32_2.push_back("{ " + tmpStr + ".s, " + tmpStr1 + ".s }");
      jtv_SPECIFIC32_2.push_back(tmpStr + ".s");

      tv_SPECIFIC32_3.push_back("{ " + tmpStr + ".s, " + tmpStr1 + ".s, " + tmpStr2 + ".s }");
      jtv_SPECIFIC32_3.push_back(tmpStr + ".s");
    
      tv_SPECIFIC32_4.push_back("{ " + tmpStr + ".s, " + tmpStr1 + ".s, " + tmpStr2 + ".s, " + tmpStr3 + ".s }");
      jtv_SPECIFIC32_4.push_back(tmpStr + ".s");

      tv_SPECIFIC64.push_back("{ " + tmpStr + ".d, " + tmpStr1 + ".d }");
      jtv_SPECIFIC64.push_back(tmpStr + ".d");

      tv_SPECIFIC64_1.push_back("{ " + tmpStr + ".d, " + tmpStr1 + ".d, " + tmpStr2 + ".d }");
      jtv_SPECIFIC64_1.push_back(tmpStr + ".d");
    
      tv_SPECIFIC64_2.push_back("{ " + tmpStr + ".d, " + tmpStr1 + ".d, " + tmpStr2 + ".d, " + tmpStr3 + ".d }");
      jtv_SPECIFIC64_2.push_back(tmpStr + ".d");
    }

    tv_SPECIFIC64_3.push_back("LSL 1");
    jtv_SPECIFIC64_3.push_back("LSL, 1");
    tv_SPECIFIC64_4.push_back("LSL 2");
    jtv_SPECIFIC64_4.push_back("LSL, 2");
    tv_SPECIFIC64_5.push_back("LSL 3");
    jtv_SPECIFIC64_5.push_back("LSL, 3");

    for(int i : tmp1) {
      tv_SPECIFIC32_5.push_back(std::to_string(i*2));
      jtv_SPECIFIC32_5.push_back(std::to_string(i*2));

      tv_SPECIFIC4.push_back(std::to_string(i*3));
      jtv_SPECIFIC4.push_back(std::to_string(i*3));

      tv_SPECIFIC5.push_back(std::to_string(i*4));
      jtv_SPECIFIC5.push_back(std::to_string(i*4));
    }
    
    putSVE_store_multiple_structures_scalar_plus_scalar();
    putSVE_store_multiple_structures_scalar_plus_immediate();

  }




  
  void putSVE_addr() {

    putSVE_Memory32bitGatherandUnsizedContiguous();
    /** Because various operand variation are required, 
	MemoryContiguousLoad is devided to three parts. */
    putSVE_MemoryContiguousLoad0(); 
    putSVE_MemoryContiguousLoad1(); 
    putSVE_MemoryContiguousLoad2(); 
    putSVE_Memory64bitGather();
    /** Because various operand variation are required, 
	MemoryStore is devided to two parts. */
    putSVE_MemoryStore0();
    putSVE_MemoryStore1();
  }

  void put()
  {
    putSVE_addr();
    //    Ops hoge();
    //    hoge.pushNm({"add", "sub"});
    
    
  }

};
  
int main(int argc, char *[])
{
  Test test(argc > 1);
  test.put();
}
