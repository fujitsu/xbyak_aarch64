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
const uint64_t ZREG = 1ULL << flagBit++; /** Test vector is { z0, z1, ..., z31 } */

const uint64_t SPECIFIC32 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_1 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_1 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_2 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_2 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_3 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_3 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_4 = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_4 = 1ULL << flagBit++; /** Test vector is generated on the fly. */

const uint64_t ZREG_B   = 1ULL << flagBit++;
const uint64_t ZREG_H   = 1ULL << flagBit++;
const uint64_t ZREG_S   = 1ULL << flagBit++;
const uint64_t ZREG_D   = 1ULL << flagBit++;
const uint64_t ZREG_Q   = 1ULL << flagBit++;

const uint64_t ZREG_B_ELEM = 1ULL << flagBit++;
const uint64_t ZREG_H_ELEM = 1ULL << flagBit++;
const uint64_t ZREG_S_ELEM = 1ULL << flagBit++;
const uint64_t ZREG_D_ELEM = 1ULL << flagBit++;

const uint64_t PREG_B   = 1ULL << flagBit++;
const uint64_t PREG_H   = 1ULL << flagBit++;
const uint64_t PREG_S   = 1ULL << flagBit++;
const uint64_t PREG_D   = 1ULL << flagBit++;

const uint64_t PG_M     = 1ULL << flagBit++;
const uint64_t PG_Z     = 1ULL << flagBit++;
const uint64_t PG       = 1ULL << flagBit++;
const uint64_t PG_ZM    = PG_M | PG_Z;

const uint64_t PATTERN  = 1ULL << flagBit++;
const uint64_t SFLOAT8BIT = 1ULL << flagBit++;

const uint64_t NOPARA = 1ULL << (bitEnd - 1);



#define PUT0(name, nm)				\
  void put##name() const			\
  {						\
    std::vector<std::string> nemonic(nm);	\
    put(nemonic);				\
  }						\

#define PUT1(name, nm, op_1)			\
  void put##name() const			\
  {						\
    std::vector<std::string> nemonic(nm);	\
    std::vector<uint64_t> op1(op_1);		\
    put(nemonic, op1, #name);					\
  }						\

#define PUT2(name, nm, op_1, op_2)		\
  void put##name() const			\
  {						\
    std::vector<std::string> nemonic(nm);	\
    std::vector<uint64_t> op1(op_1);		\
    std::vector<uint64_t> op2(op_2);		\
    put(nemonic, op1, #name);					\
    put(nemonic, op2, #name);					\
  }						\

#define PUT3(name, nm, op_1, op_2, op_3)	\
  void put##name() const			\
  {						\
    std::vector<std::string> nemonic(nm);	\
    std::vector<uint64_t> op1(op_1);		\
    std::vector<uint64_t> op2(op_2);		\
    std::vector<uint64_t> op3(op_3);		\
    put(nemonic, op1, #name);					\
    put(nemonic, op2, #name);					\
    put(nemonic, op3, #name);					\
  }						\

#define PUT4(name, nm, op_1, op_2, op_3, op_4)	\
  void put##name() const			\
  {						\
    std::vector<std::string> nemonic(nm);	\
    std::vector<uint64_t> op1(op_1);		\
    std::vector<uint64_t> op2(op_2);		\
    std::vector<uint64_t> op3(op_3);		\
    std::vector<uint64_t> op4(op_4);		\
    put(nemonic, op1, #name);					\
    put(nemonic, op2, #name);					\
    put(nemonic, op3, #name);					\
    put(nemonic, op4, #name);					\
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
    put(nemonic, op1, #name);					\
    put(nemonic, op2, #name);					\
    put(nemonic, op3, #name);					\
    put(nemonic, op4, #name);					\
    put(nemonic, op5, #name);					\
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
    put(nemonic, op1, #name);						\
    put(nemonic, op2, #name);						\
    put(nemonic, op3, #name);						\
    put(nemonic, op4, #name);						\
    put(nemonic, op5, #name);						\
    put(nemonic, op6, #name);							\
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
    put(nemonic, op1, #name);							\
    put(nemonic, op2, #name);							\
    put(nemonic, op3, #name);							\
    put(nemonic, op4, #name);							\
    put(nemonic, op5, #name);							\
    put(nemonic, op6, #name);							\
    put(nemonic, op7, #name);							\
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
    put(nemonic, op1, #name);							\
    put(nemonic, op2, #name);							\
    put(nemonic, op3, #name);							\
    put(nemonic, op4, #name);							\
    put(nemonic, op5, #name);							\
    put(nemonic, op6, #name);							\
    put(nemonic, op7, #name);							\
    put(nemonic, op8, #name);							\
  }									\
  
#define PUT9(name, nm, op_1, op_2, op_3, op_4, op_5, op_6, op_7, op_8, op_9) \
  PUT8(name, nm, op_1, op_2, op_3, op_4, op_5, op_6, op_7, op_8)	\
  PUT1(name, nm, op_9)							\

#define PUT10(name, nm, op_1, op_2, op_3, op_4, op_5, op_6, op_7, op_8, op_9, op_10) \
  PUT8(name, nm, op_1, op_2, op_3, op_4, op_5, op_6, op_7, op_8)	\
  PUT2(name, nm, op_9, op_10)						\
  
#define PUT11(name, nm, op_1, op_2, op_3, op_4, op_5, op_6, op_7, op_8, op_9, op_10, op_11) \
  PUT8(name, nm, op_1, op_2, op_3, op_4, op_5, op_6, op_7, op_8)	\
  PUT2(name, nm, op_9, op_10, op_11)					\
  
#define PUT17(name, nm, op_1, op_2, op_3, op_4, op_5, op_6, op_7, op_8, op_9, op_10, op_11, op_12, op_13, op_14, op_15, op_16, op_17) \
  PUT8(name, nm, op_1, op_2, op_3, op_4, op_5, op_6, op_7, op_8)	\
  PUT8(name, nm, op_9, op_10, op_11, op_12, op_13, op_14, op_15, op_16)	\
  PUT1(name, nm, op_17)							\


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
  std::vector<std::string> tv_ZREG = { "z7", "z0", "z1", "z2", "z4", "z8", "z16", "z31" };

  std::vector<std::string> tv_SPECIFIC32, tv_SPECIFIC64, tv_SPECIFIC32_1, tv_SPECIFIC64_1, tv_SPECIFIC32_2, tv_SPECIFIC64_2,
    tv_SPECIFIC32_3, tv_SPECIFIC64_3, tv_SPECIFIC32_4, tv_SPECIFIC64_4;
  std::vector<std::string> jtv_SPECIFIC32, jtv_SPECIFIC64, jtv_SPECIFIC32_1, jtv_SPECIFIC64_1, jtv_SPECIFIC32_2, jtv_SPECIFIC64_2,
    jtv_SPECIFIC32_3, jtv_SPECIFIC64_3, jtv_SPECIFIC32_4, jtv_SPECIFIC64_4;

  std::vector<std::string> tv_ZREG_B = { "z7.b", "z0.b", "z1.b", "z2.b", "z4.b", "z8.b", "z16.b", "z31.b" };
  std::vector<std::string> tv_ZREG_H = { "z7.h", "z0.h", "z1.h", "z2.h", "z4.h", "z8.h", "z16.h", "z31.h" };
  std::vector<std::string> tv_ZREG_S = { "z7.s", "z0.s", "z1.s", "z2.s", "z4.s", "z8.s", "z16.s", "z31.s" };
  std::vector<std::string> tv_ZREG_D = { "z7.d", "z0.d", "z1.d", "z2.d", "z4.d", "z8.d", "z16.d", "z31.d" };
  std::vector<std::string> tv_ZREG_Q = { "z7.q", "z0.q", "z1.q", "z2.q", "z4.q", "z8.q", "z16.q", "z31.q" };

  std::vector<std::string> tv_ZREG_B_ELEM, tv_ZREG_H_ELEM, tv_ZREG_S_ELEM, tv_ZREG_D_ELEM;
  
  std::vector<std::string> tv_PREG_B = { "p7.b", "p0.b", "p1.b", "p2.b", "p4.b" };
  std::vector<std::string> tv_PREG_H = { "p7.h", "p0.h", "p1.h", "p2.h", "p4.h" };
  std::vector<std::string> tv_PREG_S = { "p7.s", "p0.s", "p1.s", "p2.s", "p4.s" };
  std::vector<std::string> tv_PREG_D = { "p7.d", "p0.d", "p1.d", "p2.d", "p4.d" };

  
  std::vector<std::string> tv_PG_M = { "p7/m", "p0/m", "p1/m", "p2/m", "p4/m" };
  std::vector<std::string> tv_PG_Z = { "p7/z", "p0/z", "p1/z", "p2/z", "p4/z" };
  std::vector<std::string> jtv_PG_M = { "p7/T_m", "p0/T_m", "p1/T_m", "p2/T_m", "p4/T_m" };
  std::vector<std::string> jtv_PG_Z = { "p7/T_z", "p0/T_z", "p1/T_z", "p2/T_z", "p4/T_z" };
  std::vector<std::string> tv_PG   = { "p7", "p0", "p1", "p2", "p4" };
  std::vector<std::string> tv_PATTERN   = { "POW2", "VL1", "VL2", "VL3", "VL4", "VL5", "VL6", "VL7",
					    "VL8", "VL16", "VL32", "VL64", "VL128", "VL256", "MUL4", "MUL3",
					    "ALL" };
  std::vector<std::string> tv_SFLOAT8BIT = { "0.2421875", "-0.46875", "-0.90625", "-1.75", "-3.375", "-6.5", "-12.5", "-24.0",
					     "-0.125", "0.25", "0.53125", "1.125", "2.375", "5.0", "10.5", "22.0",
					     "0.2421875", "0.484375", "0.96875", "1.9375", "3.875", "7.75", "15.5", "31.0" };
  
  
  std::vector<std::vector<std::string> *> tv_VectorsAs = { &tv_WREG, &tv_XREG, &tv_VREG,
							   &tv_IMM0BIT, &tv_IMM1BIT, &tv_IMM2BIT, &tv_IMM3BIT, &tv_IMM4BIT, &tv_IMM5BIT, &tv_IMM6BIT, &tv_IMM8BIT,
							   &tv_IMM12BIT, &tv_IMM13BIT, &tv_IMM16BIT,
							   &tv_IMM3BIT_N, &tv_IMM4BIT_N, &tv_IMM5BIT_N, &tv_IMM6BIT_N, &tv_FLOAT8BIT,
							   &tv_BREG, &tv_HREG, &tv_SREG, &tv_DREG, &tv_QREG, &tv_ZREG,
							   &tv_SPECIFIC32, &tv_SPECIFIC64, &tv_SPECIFIC32_1, &tv_SPECIFIC64_1,
							   &tv_SPECIFIC32_2, &tv_SPECIFIC64_2, &tv_SPECIFIC32_3, &tv_SPECIFIC64_3,
							   &tv_SPECIFIC32_4, &tv_SPECIFIC64_4,
							   &tv_ZREG_B, &tv_ZREG_H, &tv_ZREG_S, &tv_ZREG_D, &tv_ZREG_Q,
							   &tv_ZREG_B_ELEM, &tv_ZREG_H_ELEM, &tv_ZREG_S_ELEM, &tv_ZREG_D_ELEM,
							   &tv_PREG_B, &tv_PREG_H, &tv_PREG_S, &tv_PREG_D,
							   &tv_PG_M, &tv_PG_Z, &tv_PG, &tv_PATTERN, &tv_SFLOAT8BIT };
  std::vector<std::vector<std::string> *> tv_VectorsJit = { &tv_WREG, &tv_XREG, &tv_VREG,
							    &tv_IMM0BIT, &tv_IMM1BIT, &tv_IMM2BIT, &tv_IMM3BIT, &tv_IMM4BIT, &tv_IMM5BIT, &tv_IMM6BIT, &tv_IMM8BIT,
							    &tv_IMM12BIT, &tv_IMM13BIT, &tv_IMM16BIT,
							    &tv_IMM3BIT_N, &tv_IMM4BIT_N, &tv_IMM5BIT_N, &tv_IMM6BIT_N, &tv_FLOAT8BIT,
							    &tv_BREG, &tv_HREG, &tv_SREG, &tv_DREG, &tv_QREG, &tv_ZREG,
							    &jtv_SPECIFIC32, &jtv_SPECIFIC64, &jtv_SPECIFIC32_1, &jtv_SPECIFIC64_1,
							    &jtv_SPECIFIC32_2, &jtv_SPECIFIC64_2, &jtv_SPECIFIC32_3, &jtv_SPECIFIC64_3,
							    &jtv_SPECIFIC32_4, &jtv_SPECIFIC64_4,
							    &tv_ZREG_B, &tv_ZREG_H, &tv_ZREG_S, &tv_ZREG_D, &tv_ZREG_Q,
							    &tv_ZREG_B_ELEM, &tv_ZREG_H_ELEM, &tv_ZREG_S_ELEM, &tv_ZREG_D_ELEM,
							    &tv_PREG_B, &tv_PREG_H, &tv_PREG_S, &tv_PREG_D,
							    &jtv_PG_M, &jtv_PG_Z, &tv_PG, &tv_PATTERN, &tv_SFLOAT8BIT };

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


  void put(std::vector<std::string> &n) const
  {
    for(std::string i : n) {
      const char *nm = removeUnderScore(i).c_str();
      if(isXbyak_) {
	std::cout << nm << "();";
      } else {
	std::cout << nm << std::endl;
      }
    }
  }

  void put(std::vector<std::string> &n, std::vector<uint64_t> &opSet, std::string name) const
  {
    std::cout << "//" << name << std::endl; /** For easy debug */
    
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

    /** Debug
    std::cout << "Debug:" << __LINE__ << ":num_ops=" << num_ops << std::endl;
    */
    
    for(i = 0; i < num_ops; i++) {
      /** Debug
      std::cout << std::endl;
      std::cout << "Debug:" << __LINE__ << ":i=" << i << std::endl;
      */
      for(j = 0; j < bitEnd; j++) {
	/** Debug
	std::cout << "Debug:" << __LINE__ << ":i=" << i << std::endl;
	std::cout << "Debug:" << __LINE__ << ":j=" << j << std::endl;
	*/

	if(ops[i] & (1ULL << j)) {
	  strBase.push_back(getBaseStr(ops[i]));

	  /** Debug
	  std::cout << "Debug:" << __LINE__ << ":i=" << i << std::endl;
	  std::cout << "Debug:" << __LINE__ << ":j=" << j << std::endl;
	  std::cout << "Debug:" << __LINE__ << ":" << ops[i] << std::endl;
	  std::cout << "Debug:" << __LINE__ << ":" << getBaseStr(ops[i]) << std::endl;
	  */
	  
	  break;
	}

	if(j==bitEnd) {
	  std::cerr << __FILE__ << ":" << __LINE__ << ":" << "No bitflag exists!" << std::endl;
	  assert(NULL);
	}
      }
    }
    
    /** Debug
    std::cout << "Debug:" << __LINE__ << std::endl;
    for(std::string i : strBase) {
      std::cout << i << std::endl;
    }      
    */

    
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

  void clearTvAndJtv() {
    tv_SPECIFIC32.clear();
    tv_SPECIFIC32_1.clear();
    tv_SPECIFIC32_2.clear();
    tv_SPECIFIC32_3.clear();
    tv_SPECIFIC32_4.clear();
    tv_SPECIFIC64.clear();
    tv_SPECIFIC64_1.clear();
    tv_SPECIFIC64_2.clear();
    tv_SPECIFIC64_3.clear();
    tv_SPECIFIC64_4.clear();

    jtv_SPECIFIC32.clear();
    jtv_SPECIFIC32_1.clear();
    jtv_SPECIFIC32_2.clear();
    jtv_SPECIFIC32_3.clear();
    jtv_SPECIFIC32_4.clear();
    jtv_SPECIFIC64.clear();
    jtv_SPECIFIC64_1.clear();
    jtv_SPECIFIC64_2.clear();
    jtv_SPECIFIC64_3.clear();
    jtv_SPECIFIC64_4.clear();
  }
  
public:
  Test(bool isXbyak)
    : isXbyak_(isXbyak)
    , funcNum_(1)
  {
    //    setAllVregElem();
    setAllZregElem();
    
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


  void setAllZregElem() {
    std::vector<int> tmpReg = { 15, 0, 1, 2, 4, 8, 16, 31 };
    std::vector<int> tmpIdxH = { 7, 0, 1, 2, 4 };
    std::vector<int> tmpIdxS = { 3, 0, 1, 2 };
    std::vector<int> tmpIdxD = { 1, 0 };
    setZregElem(tv_ZREG_H_ELEM, tmpReg, tmpIdxH, "h");
    setZregElem(tv_ZREG_S_ELEM, tmpReg, tmpIdxS, "s");
    setZregElem(tv_ZREG_D_ELEM, tmpReg, tmpIdxD, "d");
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
  /** SVE Integer Binary Arithmetic - Predicated Group */
  /*** SVE integer add/subtract vectors (predicated) */
  PUT4(SVE_integer_add_subtract_vectors_predicated0,
       NM("add", "sub", "subr"),
       OPS(SPECIFIC32, ZREG_B), OPS(SPECIFIC32_1, ZREG_H),
       OPS(SPECIFIC32_2, ZREG_S), OPS(SPECIFIC32_3, ZREG_D));
  
  void putSVE_integer_add_subtract_vectors_predicated() {
    putSVE_integer_add_subtract_vectors_predicated0();
  }
  
  /*** SVE integer min/max/difference (predicated) */
  PUT4(SVE_integer_min_max_difference_predicated0,
       NM("smax", "umax", "smin", "umin", "sabd", "uabd"),
       OPS(SPECIFIC32, ZREG_B), OPS(SPECIFIC32_1, ZREG_H),
       OPS(SPECIFIC32_2, ZREG_S), OPS(SPECIFIC32_3, ZREG_D));
  
  void putSVE_integer_min_max_difference_predicated() {
    putSVE_integer_min_max_difference_predicated0();
  }
  /*** SVE integer multiply vectors (predicated) */
  PUT4(SVE_integer_multiply_vectors_predicated0,
       NM("mul", "smulh", "umulh"),
       OPS(SPECIFIC32, ZREG_B), OPS(SPECIFIC32_1, ZREG_H),
       OPS(SPECIFIC32_2, ZREG_S), OPS(SPECIFIC32_3, ZREG_D));
  
  void putSVE_integer_multiply_vectors_predicated() {
    putSVE_integer_multiply_vectors_predicated0();
  }
  
  /*** SVE integer divide vectors (predicated) */
  PUT2(SVE_integer_divide_vectors_predicate0,
       NM("sdiv", "udiv", "sdivr", "udivr"),
       OPS(SPECIFIC32_2, ZREG_S), OPS(SPECIFIC32_3, ZREG_D));

  void putSVE_integer_divide_vectors_predicate() {
    putSVE_integer_divide_vectors_predicate0();
  }
    
  /*** SVE bitwise logical operations (predicated) */
  PUT4(SVE_bitwise_logical_operations_predicated0,
       NM("orr", "eor", "and_", "bic"),
       OPS(SPECIFIC32, ZREG_B), OPS(SPECIFIC32_1, ZREG_H),
       OPS(SPECIFIC32_2, ZREG_S), OPS(SPECIFIC32_3, ZREG_D));

  void putSVE_bitwise_logical_operations_predicated() {
    putSVE_bitwise_logical_operations_predicated0();
  }
  
  /** SVE Integer Reduction Group */
  /*** SVE integer add reduction (predicated) */
  PUT3(SVE_integer_add_reduction_predicated0,
       NM("saddv", "uaddv"),
       OPS(DREG, PG, ZREG_B), OPS(DREG, PG, ZREG_H), OPS(DREG, PG, ZREG_S));

  void putSVE_integer_add_reduction_predicated() {
    putSVE_integer_add_reduction_predicated0();
  }

  /*** SVE integer min/max reduction (predicated) */
  PUT4(SVE_integer_min_max_reduction_predicated0,
       NM("smaxv", "umaxv", "sminv", "uminv"),
       OPS(BREG, PG, ZREG_B), OPS(HREG, PG, ZREG_H), OPS(SREG, PG, ZREG_S), OPS(DREG, PG, ZREG_D));

  void putSVE_integer_min_max_reduction_predicated() {
    putSVE_integer_min_max_reduction_predicated0();
  }

  /*** SVE constructive prefix (predicated) */
  PUT4(SVE_constructive_prefix_predicated0,
       NM("movprfx"),
       OPS(ZREG_B, PG_ZM, ZREG_B), OPS(ZREG_H, PG_ZM, ZREG_H), OPS(ZREG_S, PG_ZM, ZREG_S), OPS(ZREG_D, PG_ZM, ZREG_D));

  void putSVE_constructive_prefix_predicated() {
    putSVE_constructive_prefix_predicated0();
  }

  /*** SVE bitwise logical reduction (predicated) */
  PUT4(SVE_bitwise_logical_reduction_predicated0,
       NM("orv", "eorv", "andv"),
       OPS(BREG, PG, ZREG_B), OPS(HREG, PG, ZREG_H), OPS(SREG, PG, ZREG_S), OPS(DREG, PG, ZREG_D));
  
  void putSVE_bitwise_logical_reduction_predicated() {
    putSVE_bitwise_logical_reduction_predicated0();
  }    

  /** SVE Bitwise Shift - Predicated Group */
  /*** SVE bitwise shift by immediate (predicated) */
  PUT4(SVE_bitwise_shift_by_immediate_predicated0,
       NM("asr", "lsr", "asrd"),
       OPS(SPECIFIC32, IMM3BIT_N), OPS(SPECIFIC32_1, IMM4BIT_N),
       OPS(SPECIFIC32_2, IMM5BIT_N), OPS(SPECIFIC32_3, IMM6BIT_N));
  PUT4(SVE_bitwise_shift_by_immediate_predicated1,
       NM("lsl"),
       OPS(SPECIFIC32, IMM3BIT), OPS(SPECIFIC32_1, IMM4BIT),
       OPS(SPECIFIC32_2, IMM5BIT), OPS(SPECIFIC32_3, IMM6BIT));

  void putSVE_bitwise_shift_by_immediate_predicated() {
    putSVE_bitwise_shift_by_immediate_predicated0();
    putSVE_bitwise_shift_by_immediate_predicated1();
  }    

  /*** SVE bitwise shift by vector (predicated) */
  PUT4(SVE_bitwise_shift_by_vector_predicated0,
       NM("asr","lsr", "lsl", "asrr", "lsrr", "lslr"),
       OPS(SPECIFIC32, ZREG_B), OPS(SPECIFIC32_1, ZREG_H),
       OPS(SPECIFIC32_2, ZREG_S), OPS(SPECIFIC32_3, ZREG_D));

  void putSVE_bitwise_shift_by_vector_predicated() {
    putSVE_bitwise_shift_by_vector_predicated0();
  }

  /*** SVE bitwise shift by wide elements (predicated)*/
  PUT3(SVE_bitwise_shift_by_wide_elements_predicated0,
       NM("asr", "lsr", "lsl"),
       OPS(SPECIFIC32, ZREG_D), OPS(SPECIFIC32_1, ZREG_D),
       OPS(SPECIFIC32_2, ZREG_D));

  void putSVE_bitwise_shift_by_wide_elements_predicated() {
    putSVE_bitwise_shift_by_wide_elements_predicated0();
  }    

  /** SVE Integer Unary Arithmetic - Predicated Group */
  /*** SVE integer unary operations (predicated) */
  PUT3(SVE_integer_unary_operations_predicated0,
       NM("sxtb", "uxtb"),
       OPS(ZREG_H, PG_M, ZREG_H), OPS(ZREG_S, PG_M, ZREG_S), OPS(ZREG_D, PG_M, ZREG_D));
  PUT2(SVE_integer_unary_operations_predicated1,
       NM("sxth", "uxth"),
       OPS(ZREG_S, PG_M, ZREG_S), OPS(ZREG_D, PG_M, ZREG_D));
  PUT1(SVE_integer_unary_operations_predicated2,
       NM("sxtw", "uxtw"),
       OPS(ZREG_D, PG_M, ZREG_D));
  PUT4(SVE_integer_unary_operations_predicated3,
       NM("abs", "neg"),
       OPS(ZREG_B, PG_M, ZREG_B), OPS(ZREG_H, PG_M, ZREG_H), OPS(ZREG_S, PG_M, ZREG_S), OPS(ZREG_D, PG_M, ZREG_D));

  void putSVE_integer_unary_operations_predicated() {
    putSVE_integer_unary_operations_predicated0();
    putSVE_integer_unary_operations_predicated1();
    putSVE_integer_unary_operations_predicated2();
    putSVE_integer_unary_operations_predicated3();
  }

  /*** SVE bitwise unary operations (predicated) */
  PUT4(SVE_bitwise_unary_operations_predicated0,
       NM("cls", "clz", "cnt", "cnot", "not_"),
       OPS(ZREG_B, PG_M, ZREG_B), OPS(ZREG_H, PG_M, ZREG_H), OPS(ZREG_S, PG_M, ZREG_S), OPS(ZREG_D, PG_M, ZREG_D));
  PUT3(SVE_bitwise_unary_operations_predicated1,
       NM("fabs", "fneg"),
       OPS(ZREG_H, PG_M, ZREG_H), OPS(ZREG_S, PG_M, ZREG_S), OPS(ZREG_D, PG_M, ZREG_D));

  void putSVE_bitwise_unary_operations_predicated() {
    putSVE_bitwise_unary_operations_predicated0();
  }
  
  /** SVE Integer Multiply-Add - Predicated Group */
  /*** SVE integer multiply-accumulate writing addend (predicated) */
  PUT4(SVE_integer_multiply_accumulate_writing_addend_predicated0,
       NM("mla", "mls"),
       OPS(ZREG_B, PG_M, ZREG_B, ZREG_B), OPS(ZREG_H, PG_M, ZREG_H, ZREG_H),
       OPS(ZREG_S, PG_M, ZREG_S, ZREG_S), OPS(ZREG_D, PG_M, ZREG_D, ZREG_D));

  void putSVE_integer_multiply_accumulate_writing_addend_predicated() {
    putSVE_integer_multiply_accumulate_writing_addend_predicated0();
  }

  /*** SVE integer multiply-add writing multiplicand (predicated) */
  PUT4(SVE_integer_multiply_add_writing_multiplicand_predicated0,
       NM("mad", "msb"),
       OPS(ZREG_B, PG_M, ZREG_B, ZREG_B), OPS(ZREG_H, PG_M, ZREG_H, ZREG_H),
       OPS(ZREG_S, PG_M, ZREG_S, ZREG_S), OPS(ZREG_D, PG_M, ZREG_D, ZREG_D));

  void putSVE_integer_multiply_add_writing_multiplicand_predicated() {
    putSVE_integer_multiply_add_writing_multiplicand_predicated0();
  }
    
  /** SVE Integer Arithmetic - Unpredicated */
  /*** SVE integer add/subtract vectors (unpredicated)*/
  PUT4(SVE_integer_add_subtract_vectors_unpredicated0,
       NM("add", "sub", "sqadd", "uqadd", "sqsub", "uqsub"),
       OPS(ZREG_B, ZREG_B, ZREG_B), OPS(ZREG_H, ZREG_H, ZREG_H),
       OPS(ZREG_S, ZREG_S, ZREG_S), OPS(ZREG_D, ZREG_D, ZREG_D));

  void putSVE_integer_add_subtract_vectors_unpredicated() {
    putSVE_integer_add_subtract_vectors_unpredicated0();
  }

  /** SVE Bitwise Logical - Unpredicated */
  /*** SVE bitwise logical operations (unpredicated) */
  PUT1(SVE_bitwise_logical_operations_unpredicated0,
       NM("and_", "orr", "eor", "bic"),
       OPS(ZREG_D, ZREG_D, ZREG_D));

  void putSVE_bitwise_logical_operations_unpredicated() {
    putSVE_bitwise_logical_operations_unpredicated0();
  }

  /** SVE Index Generation */
  /*** SVE index generation (immediate start, immediate increment) */
  PUT4(SVE_index_generation_immediate_start_immediate_increment0,
       NM("index"),
       OPS(ZREG_B, SPECIFIC32, SPECIFIC32), OPS(ZREG_H, SPECIFIC32, SPECIFIC32),
       OPS(ZREG_S, SPECIFIC32, SPECIFIC32), OPS(ZREG_D, SPECIFIC32, SPECIFIC32));

  void putSVE_index_generation_immediate_start_immediate_increment() {
    putSVE_index_generation_immediate_start_immediate_increment0();
  }

  /*** SVE index generation (register start, immediate increment) */
  PUT4(SVE_index_generation_register_start_immediate_increment0,
       NM("index"),
       OPS(ZREG_B, SPECIFIC32, WREG), OPS(ZREG_H, SPECIFIC32, WREG),
       OPS(ZREG_S, SPECIFIC32, WREG), OPS(ZREG_D, SPECIFIC32, XREG));

  void putSVE_index_generation_register_start_immediate_increment() {
    putSVE_index_generation_register_start_immediate_increment0();
  }

  /*** SVE index generation (immediate start, register increment) */
  PUT4(SVE_index_generation_immediate_start_register_increment0,
       NM("index"),
       OPS(ZREG_B, WREG, SPECIFIC32), OPS(ZREG_H, WREG, SPECIFIC32),
       OPS(ZREG_S, WREG, SPECIFIC32), OPS(ZREG_D, XREG, SPECIFIC32));

  void putSVE_index_generation_immediate_start_register_increment() {
    putSVE_index_generation_immediate_start_register_increment0();
  }

  /*** SVE index generation (register start, register increment) */
  PUT4(SVE_index_generation_register_start_register_increment0,
       NM("index"),
       OPS(ZREG_B, WREG, WREG), OPS(ZREG_H, WREG, WREG), 
       OPS(ZREG_S, WREG, WREG), OPS(ZREG_D, XREG, XREG));

  void putSVE_index_generation_register_start_register_increment() {
    putSVE_index_generation_register_start_register_increment0();
  }

  /** SVE Stack Allocation */
  /*** SVE stack frame adjustment */
  PUT1(SVE_stack_frame_adjustment0,
       NM("addvl", "addpl"),
       OPS(XREG, XREG, SPECIFIC32));

  void putSVE_stack_frame_adjustment() {
    putSVE_stack_frame_adjustment0();
  }

  /*** SVE stack frame size */
  PUT1(SVE_stack_frame_size0,
       NM("rdvl"),
       OPS(XREG, SPECIFIC32));

  void putSVE_stack_frame_size() {
    putSVE_stack_frame_size0();
  }
  
  /** SVE Bitwise Shift - Unpredicated */
  /*** SVE bitwise shift by wide elements (unpredicated) */
  PUT3(SVE_bitwise_shift_by_wide_elements_unpredicated0,
       NM("asr", "lsr", "lsl"),
       OPS(ZREG_B, ZREG_B, ZREG_D), OPS(ZREG_H, ZREG_H, ZREG_D), OPS(ZREG_S, ZREG_S, ZREG_D));
  
  void putSVE_bitwise_shift_by_wide_elements_unpredicate() {
    putSVE_bitwise_shift_by_wide_elements_unpredicated0();
  }
  
  /*** SVE bitwise shift by immediate (unpredicated) */
  PUT4(SVE_bitwise_shift_by_immediate_unpredicated0,
       NM("asr", "lsr"),
       OPS(ZREG_B, ZREG_B, IMM3BIT_N), OPS(ZREG_H, ZREG_H, IMM4BIT_N), 
       OPS(ZREG_S, ZREG_S, IMM5BIT_N), OPS(ZREG_D, ZREG_D, IMM6BIT_N));
  PUT4(SVE_bitwise_shift_by_immediate_unpredicated1,
       NM("lsl"),
       OPS(ZREG_B, ZREG_B, IMM3BIT), OPS(ZREG_H, ZREG_H, IMM4BIT), 
       OPS(ZREG_S, ZREG_S, IMM5BIT), OPS(ZREG_D, ZREG_D, IMM6BIT));

  void putSVE_bitwise_shift_by_immediate_unpredicated() {
    putSVE_bitwise_shift_by_immediate_unpredicated0();
    putSVE_bitwise_shift_by_immediate_unpredicated1();
  }
  
  /** SVE Address Generation */
  /*** SVE address generation */
  /**** Unpacked 32-bit signed offsets */
  /**** Unpacked 32-bit unsigned offsets */
  PUT1(SVE_address_generation0,
       NM("adr"),
       OPS(ZREG_D, SPECIFIC32));
  PUT2(SVE_address_generation1,
       NM("adr"),
       OPS(ZREG_S, SPECIFIC32_1),
       OPS(ZREG_D, SPECIFIC32_2));

  void putSVE_address_generation() {
    putSVE_address_generation0();
    putSVE_address_generation1();
  }
  
  /** SVE Integer Misc - Unpredicated */
  /*** SVE floating-point trig select coefficient */
  PUT3(SVE_floating_point_trig_select_coefficient0,
       NM("ftssel"),
       OPS(ZREG_H, ZREG_H, ZREG_H), OPS(ZREG_S, ZREG_S, ZREG_S), OPS(ZREG_D, ZREG_D, ZREG_D));

  void putSVE_floating_point_trig_select_coefficient() {
    putSVE_floating_point_trig_select_coefficient0();
  }

  /*** SVE floating-point exponential accelerator */
  PUT3(SVE_floating_point_exponential_accelerator0,
       NM("fexpa"),
       OPS(ZREG_H, ZREG_H), OPS(ZREG_S, ZREG_S), OPS(ZREG_D, ZREG_D));

  void putSVE_floating_point_exponential_accelerator() {
    putSVE_floating_point_exponential_accelerator0();
  }

  /*** SVE constructive prefix (unpredicated) */
  PUT1(SVE_constructive_prefix_unpredicated0,
       NM("movprfx"),
       OPS(ZREG, ZREG));

  void putSVE_constructive_prefix_unpredicated() {
    putSVE_constructive_prefix_unpredicated0();
  }

  /** SVE Element Count */
  /*** SVE saturating inc/dec register by element count */
  PUT6(SVE_saturating_inc_dec_register_by_element_count0,
       NM("sqincb", "sqdecb", "sqinch", "sqdech", "sqincw", "sqdecw", "sqincd", "sqdecd"),
       OPS(SPECIFIC32_1),
       OPS(SPECIFIC32_1, PATTERN),
       OPS(SPECIFIC32_1, PATTERN, SPECIFIC32),
       OPS(XREG),
       OPS(XREG, PATTERN),
       OPS(XREG, PATTERN, SPECIFIC32));
  PUT6(SVE_saturating_inc_dec_register_by_element_count1,
       NM("uqincb", "uqdecb", "uqinch", "uqdech", "uqincw", "uqdecw", "uqincd", "uqdecd"),
       OPS(WREG), OPS(WREG, PATTERN), OPS(WREG, PATTERN, SPECIFIC32),
       OPS(XREG), OPS(XREG, PATTERN), OPS(XREG, PATTERN, SPECIFIC32));

  void putSVE_saturating_inc_dec_register_by_element_count() {
    putSVE_saturating_inc_dec_register_by_element_count0();
    putSVE_saturating_inc_dec_register_by_element_count1();
  }
    
  /*** SVE saturating inc/dec vector by element count */
  PUT3(SVE_saturating_inc_dec_vector_by_element_count0,
       NM("sqinch", "uqinch", "sqdech", "uqdech"),
       OPS(ZREG_H), OPS(ZREG_H, PATTERN), OPS(ZREG_H, PATTERN, SPECIFIC32));
  PUT3(SVE_saturating_inc_dec_vector_by_element_count1,
       NM("sqincw", "uqincw", "sqdecw", "uqdecw"),
       OPS(ZREG_S), OPS(ZREG_S, PATTERN), OPS(ZREG_S, PATTERN, SPECIFIC32));
  PUT3(SVE_saturating_inc_dec_vector_by_element_count2,
       NM("sqincd", "uqincd", "sqdecd", "uqdecd"),
       OPS(ZREG_D,), OPS(ZREG_D, PATTERN), OPS(ZREG_D, PATTERN, SPECIFIC32));

  void putSVE_saturating_inc_dec_vector_by_element_count() {
    putSVE_saturating_inc_dec_vector_by_element_count0();
    putSVE_saturating_inc_dec_vector_by_element_count1();
    putSVE_saturating_inc_dec_vector_by_element_count2();
  }
    
  /*** SVE element count */
  PUT3(SVE_element_count0,
       NM("cntb", "cntd", "cnth", "cntw"),
       OPS(XREG), OPS(XREG, PATTERN), OPS(XREG, PATTERN, SPECIFIC32));

  void putSVE_element_count() {
    putSVE_element_count0();
  }

  /*** SVE inc/dec vector by element count */
  PUT3(SVE_inc_dec_vector_by_element_count0,
       NM("incd", "decd"),
       OPS(ZREG_D), OPS(ZREG_D, PATTERN), OPS(ZREG_D, PATTERN, SPECIFIC32));
  PUT3(SVE_inc_dec_vector_by_element_count1,
       NM("inch", "dech"),
       OPS(ZREG_H), OPS(ZREG_H, PATTERN), OPS(ZREG_H, PATTERN, SPECIFIC32));
  PUT3(SVE_inc_dec_vector_by_element_count2,
       NM("incw", "decw"),
       OPS(ZREG_S), OPS(ZREG_S, PATTERN), OPS(ZREG_S, PATTERN, SPECIFIC32));
  void putSVE_inc_dec_vector_by_element_count() {
    putSVE_inc_dec_vector_by_element_count0();
    putSVE_inc_dec_vector_by_element_count1();
    putSVE_inc_dec_vector_by_element_count2();
  }
    
  /*** SVE inc/dec register by element count*/
  PUT3(SVE_inc_dec_register_by_element_count0,
       NM("incb", "decb"),
       OPS(XREG), OPS(XREG, PATTERN), OPS(XREG, PATTERN, SPECIFIC32));
  PUT3(SVE_inc_dec_register_by_element_count1,
       NM("incd", "decd"),
       OPS(XREG), OPS(XREG, PATTERN), OPS(XREG, PATTERN, SPECIFIC32));
  PUT3(SVE_inc_dec_register_by_element_count2,
       NM("inch", "dech"),
       OPS(XREG), OPS(XREG, PATTERN), OPS(XREG, PATTERN, SPECIFIC32));
  PUT3(SVE_inc_dec_register_by_element_count3,
       NM("incw", "decw"),
       OPS(XREG), OPS(XREG, PATTERN), OPS(XREG, PATTERN, SPECIFIC32));

  void putSVE_inc_dec_register_by_element_count() {
    putSVE_inc_dec_register_by_element_count0();
    putSVE_inc_dec_register_by_element_count1();
    putSVE_inc_dec_register_by_element_count2();
    putSVE_inc_dec_register_by_element_count3();
  }    
  
  /** SVE Bitwise Immediate */
  /*** SVE bitwise logical with immediate (unpredicated) */
  /**** Source and destination Reg, length of non-zero bits, shift amount */
  PUT4(SVE_bitwise_logical_with_immediate_unpredicated0,
       NM("orr", "eor", "and_"),
       OPS(SPECIFIC32, SPECIFIC64), OPS(SPECIFIC32_1, SPECIFIC64_1), 
       OPS(SPECIFIC32_2, SPECIFIC64_2), OPS(SPECIFIC32_3, SPECIFIC64_3));

  void putSVE_bitwise_logical_with_immediate_unpredicated() {
    putSVE_bitwise_logical_with_immediate_unpredicated0();
  }

  /*** SVE broadcast bitmask immediate*/
  PUT4(SVE_broadcast_bitmask_immediate0,
       NM("dupm"),
       OPS(ZREG_D, SPECIFIC64_3), OPS(ZREG_S, SPECIFIC64_2),
       OPS(ZREG_H, SPECIFIC64_1), OPS(ZREG_B, SPECIFIC64));

  void putSVE_broadcast_bitmask_immediate() {
    putSVE_broadcast_bitmask_immediate0();
  }

  /** SVE Integer Wide Immediate - Predicated */
  /*** SVE copy integer immediate (predicated) */
  PUT8(SVE_copy_integer_immediate_predicated0,
       NM("cpy"),
       OPS(ZREG_B, PG_ZM, SPECIFIC32), OPS(ZREG_B, PG_ZM, SPECIFIC32, SPECIFIC64_1),
       OPS(ZREG_H, PG_ZM, SPECIFIC32), OPS(ZREG_H, PG_ZM, SPECIFIC32, SPECIFIC64),
       OPS(ZREG_S, PG_ZM, SPECIFIC32), OPS(ZREG_S, PG_ZM, SPECIFIC32, SPECIFIC64),
       OPS(ZREG_D, PG_ZM, SPECIFIC32), OPS(ZREG_D, PG_ZM, SPECIFIC32, SPECIFIC64));

  void putSVE_copy_integer_immediate_predicated() {
    putSVE_copy_integer_immediate_predicated0();
  }
  
  /*** SVE copy floating-point immediate (predicated) */
  PUT3(SVE_copy_floating_point_immediate_predicated0,
       NM("fcpy"),
       OPS(ZREG_H, PG_M, SFLOAT8BIT),
       OPS(ZREG_S, PG_M, SFLOAT8BIT),
       OPS(ZREG_D, PG_M, SFLOAT8BIT));

  void putSVE_copy_floating_point_immediate_predicated() {
    putSVE_copy_floating_point_immediate_predicated0();
  }
  
  /** SVE Permute Vector - Extract */
  /*** SVE extract vector (immediate offset, destructive) */
  PUT1(SVE_extract_vector_immediate_offset_destructive0,
       NM("ext"),
       OPS(SPECIFIC32, ZREG_B, IMM8BIT));

  void putSVE_extract_vector_immediate_offset_destructive() {
    putSVE_extract_vector_immediate_offset_destructive0();
  }
  
  /** SVE Permute Vector - Unpredicated */
  /*** SVE broadcast indexed element */
  PUT5(SVE_broadcast_indexed_element0,
       NM("dup"),
       OPS(ZREG_B, SPECIFIC32), OPS(ZREG_H, SPECIFIC32_1), OPS(ZREG_S, SPECIFIC32_2),
       OPS(ZREG_D, SPECIFIC32_3), OPS(ZREG_Q, SPECIFIC64));

  void putSVE_broadcast_indexed_element() {
    putSVE_broadcast_indexed_element0();
  }
    
  /*** SVE table lookup */
  PUT4(SVE_table_lookup0,
       NM("tbl"),
       OPS(ZREG_B, ZREG_B, ZREG_B), OPS(ZREG_H, ZREG_H, ZREG_H),
       OPS(ZREG_S, ZREG_S, ZREG_S), OPS(ZREG_D, ZREG_D, ZREG_D));

  void putSVE_table_lookup() {
    putSVE_table_lookup0();
  }

  /*** SVE broadcast general register */
  PUT4(SVE_broadcast_general_register0,
       NM("dup"),
       OPS(ZREG_B, WREG), OPS(ZREG_H, WREG), OPS(ZREG_S, WREG), OPS(ZREG_D, XREG));

  void putSVE_broadcast_general_register() {
    putSVE_broadcast_general_register0();
  }

  /*** SVE insert general register */
  PUT4(SVE_insert_general_register0,
       NM("insr"),
       OPS(ZREG_B, WREG), OPS(ZREG_H, WREG), OPS(ZREG_S, WREG), OPS(ZREG_D, XREG));

  void putSVE_insert_general_register() {
    putSVE_insert_general_register0();
  }

  /*** SVE unpack vector elements */
  PUT3(SVE_unpack_vector_elements0,
       NM("sunpkhi", "sunpklo", "uunpkhi", "uunpklo"),
       OPS(ZREG_H, ZREG_B), OPS(ZREG_S, ZREG_H), OPS(ZREG_D, ZREG_S));
  void putSVE_unpack_vector_elements() {
    putSVE_unpack_vector_elements0();
  }

  /*** SVE insert SIMD&FP scalar register */
  PUT4(SVE_insert_SIMD_FP_scalar_register0,
       NM("insr"),
       OPS(ZREG_B, BREG), OPS(ZREG_H, HREG), OPS(ZREG_S, SREG), OPS(ZREG_D, DREG));

  void putSVE_insert_SIMD_FP_scalar_register() {
    putSVE_insert_SIMD_FP_scalar_register0();
  }
  
  /*** SVE reverse vector elements */
  PUT4(SVE_reverse_vector_elements0,
       NM("rev"),
       OPS(ZREG_B, ZREG_B), OPS(ZREG_H, ZREG_H), OPS(ZREG_S, ZREG_S), OPS(ZREG_D, ZREG_D));

  void putSVE_reverse_vector_elements() {
    putSVE_reverse_vector_elements0();
  }
  
  /** SVE Permute Predicate */
  /*** SVE permute predicate elements */
  PUT4(SVE_permute_predicate_elements0,
       NM("zip1", "zip2", "uzp1", "uzp2", "trn1", "trn2"),
       OPS(PREG_B, PREG_B, PREG_B), OPS(PREG_H, PREG_H, PREG_H),
       OPS(PREG_S, PREG_S, PREG_S), OPS(PREG_D, PREG_D, PREG_D));

  void putSVE_permute_predicate_elements() {
    putSVE_permute_predicate_elements0();
  }

  /*** SVE reverse predicate elements */
  PUT4(SVE_reverse_predicate_elements0,
       NM("rev"),
       OPS(PREG_B, PREG_B), OPS(PREG_H, PREG_H), OPS(PREG_S, PREG_S), OPS(PREG_D, PREG_D));

  void putSVE_reverse_predicate_elements() {
    putSVE_reverse_predicate_elements0();
  }

  /*** SVE unpack predicate elements */
  PUT1(SVE_unpack_predicate_elements0,       
       NM("punpkhi", "punpklo"),
       OPS(PREG_H, PREG_B));

  void putSVE_unpack_predicate_elements() {
    putSVE_unpack_predicate_elements0();
  }
    
  /** SVE Permute Vector - Interleaving */
  /*** SVE permute vector elements */
  PUT4(SVE_permute_vector_elements0,
       NM("zip1", "zip2", "uzp1", "uzp2", "trn1", "trn2"),
       OPS(ZREG_B, ZREG_B, ZREG_B), OPS(ZREG_H, ZREG_H, ZREG_H), 
       OPS(ZREG_S, ZREG_S, ZREG_S), OPS(ZREG_D, ZREG_D, ZREG_D)); 

  void putSVE_permute_vector_elements() {
    putSVE_permute_vector_elements0();
  }
  
  /** SVE Permute Vector - Predicated */
  /*** SVE extract element to general register */
  PUT4(SVE_extract_element_to_general_register0,
       NM("lasta", "lastb"),
       OPS(WREG, PG, ZREG_B), OPS(WREG, PG, ZREG_H), 
       OPS(WREG, PG, ZREG_S), OPS(XREG, PG, ZREG_D)); 

  void putSVE_extract_element_to_general_register() {
    putSVE_extract_element_to_general_register0();
  }

  /*** SVE copy SIMD&FP scalar register to vector (predicated) */
  PUT4(SVE_copy_SIMD_FP_scalar_register_to_vector_predicated0,
       NM("cpy"),
       OPS(ZREG_B, PG_M, BREG), OPS(ZREG_H, PG_M, HREG),
       OPS(ZREG_S, PG_M, SREG), OPS(ZREG_D, PG_M, DREG));

  void putSVE_copy_SIMD_FP_scalar_register_to_vector_predicated() {
    putSVE_copy_SIMD_FP_scalar_register_to_vector_predicated0();
  }

  /*** SVE extract element to SIMD&FP scalar register */
  PUT4(SVE_extract_element_to_SIMD_FP_scalar_register0,
       NM("lasta", "lastb"),
       OPS(BREG, PG, ZREG_B), OPS(HREG, PG, ZREG_H),
       OPS(SREG, PG, ZREG_S), OPS(DREG, PG, ZREG_D));

  void putSVE_extract_element_to_SIMD_FP_scalar_register() {
    putSVE_extract_element_to_SIMD_FP_scalar_register0();
  }

  /*** SVE reverse within elements */
  PUT3(SVE_reverse_within_elements0,
       NM("revb"),
       OPS(ZREG_H, PG_M, ZREG_H), OPS(ZREG_S, PG_M, ZREG_S), OPS(ZREG_D, PG_M, ZREG_D));
  PUT2(SVE_reverse_within_elements1,
       NM("revh"),
       OPS(ZREG_S, PG_M, ZREG_S), OPS(ZREG_D, PG_M, ZREG_D));
  PUT1(SVE_reverse_within_elements2,
       NM("revw"),
       OPS(ZREG_D, PG_M, ZREG_D));
  PUT4(SVE_reverse_within_elements3,
       NM("rbit"),
       OPS(ZREG_B, PG_M, ZREG_B), OPS(ZREG_H, PG_M, ZREG_H),
       OPS(ZREG_S, PG_M, ZREG_S), OPS(ZREG_D, PG_M, ZREG_D));

  void putSVE_reverse_within_elements() {
    putSVE_reverse_within_elements0();
    putSVE_reverse_within_elements1();
    putSVE_reverse_within_elements2();
    putSVE_reverse_within_elements3();
  }
  
  /*** SVE conditionally broadcast element to vector */
  PUT4(SVE_conditionally_broadcast_element_to_vector0,
       NM("clasta", "clastb"),
       OPS(SPECIFIC32, ZREG_B), OPS(SPECIFIC32_1, ZREG_H),
       OPS(SPECIFIC32_2, ZREG_S), OPS(SPECIFIC32_3, ZREG_D));

  void putSVE_conditionally_broadcast_element_to_vector() {
    putSVE_conditionally_broadcast_element_to_vector0();
  }    

  /*** SVE copy general register to vector (predicated) */
  PUT4(SVE_copy_general_register_to_vector_predicated0,
       NM("cpy"),
       OPS(ZREG_B, PG_M, WREG), OPS(ZREG_H, PG_M, WREG),
       OPS(ZREG_S, PG_M, WREG), OPS(ZREG_D, PG_M, XREG));

  void putSVE_copy_general_register_to_vector_predicated() {
    putSVE_copy_general_register_to_vector_predicated0();
  }

  /*** SVE conditionally extract element to SIMD&FP scalar */
  PUT4(SVE_conditionally_extract_element_to_SIMD_FP_scalar0,
       NM("clasta", "clastb"),
       OPS(SPECIFIC64, ZREG_B), OPS(SPECIFIC64_1, ZREG_H),
       OPS(SPECIFIC64_2, ZREG_S), OPS(SPECIFIC64_3, ZREG_D));

  void putSVE_conditionally_extract_element_to_SIMD_FP_scalar() {
    putSVE_conditionally_extract_element_to_SIMD_FP_scalar0();
  }

  /*** SVE vector splice (destructive) */
  PUT4(SVE_vector_splice_destructive0,
       NM("splice"),
       OPS(SPECIFIC64, ZREG_B), OPS(SPECIFIC64_1, ZREG_H),
       OPS(SPECIFIC64_2, ZREG_S), OPS(SPECIFIC64_3, ZREG_D));

  void putSVE_vector_splice_destructive() {
    putSVE_vector_splice_destructive0();
  }

  /*** SVE conditionally extract element to general register */
  PUT4(SVE_conditionally_extract_element_to_general_register0,
       NM("clasta", "clastb"),
       OPS(SPECIFIC32_4, ZREG_B), OPS(SPECIFIC32_4, ZREG_H),
       OPS(SPECIFIC32_4, ZREG_S), OPS(SPECIFIC64_4, ZREG_D));

  void putSVE_conditionally_extract_element_to_general_register() {
    putSVE_conditionally_extract_element_to_general_register0();
  }
  
  /*** SVE compress active elements */
  PUT2(SVE_compress_active_elements0,
       NM("compact"),
       OPS(ZREG_S, PG, ZREG_S), OPS(ZREG_D, PG, ZREG_D));

  void putSVE_compress_active_elements() {
    putSVE_compress_active_elements0();
  }

  /** SVE Vector Select */
  /*** SVE select vector elements (predicated) */
  PUT4(SVE_select_vector_elements_predicated0,
       NM("sel"),
       OPS(ZREG_B, PG, ZREG_B, ZREG_B), OPS(ZREG_H, PG, ZREG_H, ZREG_H), 
       OPS(ZREG_S, PG, ZREG_S, ZREG_S), OPS(ZREG_D, PG, ZREG_D, ZREG_D));

  void putSVE_select_vector_elements_predicated() {
    putSVE_select_vector_elements_predicated0();
  }
  
  /** SVE Integer Compare - Vectors */
  /*** SVE integer compare vectors */
  PUT4(SVE_integer_compare_vectors0,
       NM("cmphs", "cmphi", "cmpge", "cmpge", "cmpeq", "cmpne"),
       OPS(PREG_B, PG_Z, ZREG_B, ZREG_B), OPS(PREG_H, PG_Z, ZREG_H, ZREG_H), 
       OPS(PREG_S, PG_Z, ZREG_S, ZREG_S), OPS(PREG_D, PG_Z, ZREG_D, ZREG_D));
  PUT3(SVE_integer_compare_vectors1,
       NM("cmpeq", "cmpne"),
       OPS(PREG_B, PG_Z, ZREG_B, ZREG_D), OPS(PREG_H, PG_Z, ZREG_H, ZREG_D), 
       OPS(PREG_S, PG_Z, ZREG_S, ZREG_D));
  void putSVE_integer_compare_vectors() {
    putSVE_integer_compare_vectors0();
    putSVE_integer_compare_vectors1();
  }

  /*** SVE integer compare with wide elements */
  PUT3(SVE_integer_compare_with_wide_elements0,
       NM("cmpge", "cmpgt", "cmplt", "cmple", "cmphs", "cmphi", "cmplo", "cmpls"),
       OPS(PREG_B, PG_Z, ZREG_B, ZREG_D), OPS(PREG_H, PG_Z, ZREG_H, ZREG_D), 
       OPS(PREG_S, PG_Z, ZREG_S, ZREG_D));

  void putSVE_integer_compare_with_wide_elements() {
    void putSVE_integer_compare_with_wide_elements0();
  }
    
  /** SVE Integer Compare - Unsigned Immediate */
  /*** SVE integer compare with unsigned immediate */
  PUT4(SVE_integer_compare_with_unsigned_immediate0,
       NM("cmphs", "cmphi", "cmplo", "cmpls"),
       OPS(PREG_B, PG_Z, ZREG_B, IMM5BIT), OPS(PREG_H, PG_Z, ZREG_H, IMM5BIT), 
       OPS(PREG_S, PG_Z, ZREG_S, IMM5BIT), OPS(PREG_D, PG_Z, ZREG_D, IMM5BIT));

  void putSVE_integer_compare_with_unsigned_immediate() {
    putSVE_integer_compare_with_unsigned_immediate0();
  }

  /** SVE Predicate Logical Operations */
  /*** SVE predicate logical operations */
  PUT1(SVE_predicate_logical_operations0,
       NM("and_", "ands", "bic", "bics", "eor", "eors", "orr", "orrs",
	  "orn", "orns", "nor", "nors", "nand", "nands"),
       OPS(PREG_B, PG_Z, PREG_B, PREG_B));
  PUT1(SVE_predicate_logical_operations1,
       NM("sel"),
       OPS(PREG_B, PG, PREG_B, PREG_B));

  void putSVE_predicate_logical_operations() {
    putSVE_predicate_logical_operations0();
    putSVE_predicate_logical_operations1();
  }
  
  /** SVE Propagate Break */
  /*** SVE propagate break from previous partition */
  PUT1(SVE_propagate_break_from_previous_partition0,
       NM("brkpa", "brkpb", "brkpas", "brkpbs"),
       OPS(PREG_B, PG_Z, PREG_B, PREG_B));

  void putSVE_propagate_break_from_previous_partition() {
    putSVE_propagate_break_from_previous_partition0();
  }
  
  /** SVE Partition Break */
  /*** SVE partition break condition */
  PUT1(SVE_partition_break_condition0,
       NM("brka", "brkb"),
       OPS(PREG_B, PG_ZM, PREG_B));
  PUT1(SVE_partition_break_condition1,
       NM("brka", "brkb"),
       OPS(PREG_B, PG_Z, PREG_B));

  void putSVE_partition_break_condition() {
    putSVE_partition_break_condition0();
    putSVE_partition_break_condition1();
  }    

  /*** SVE propagate break to next partition */
  PUT1(SVE_propagate_break_to_next_partition0,
       NM("brkn", "brkns"),
       OPS(SPECIFIC32));

  void putSVE_propagate_break_to_next_partition() {
    putSVE_propagate_break_to_next_partition0();
  }

  /** SVE Predicate Misc */
  /*** SVE predicate test*/
  PUT1(SVE_predicate_test0,
       NM("ptest"),
       OPS(PG, PREG_B));

  void putSVE_predicate_test() {
    putSVE_predicate_test0();
  }
  
  /*** SVE predicate initialize */
  PUT8(SVE_predicate_initialize0,
       NM("ptrue", "ptrues"),
       OPS(PREG_B), OPS(PREG_B, PATTERN), OPS(PREG_H), OPS(PREG_H, PATTERN),
       OPS(PREG_S), OPS(PREG_S, PATTERN), OPS(PREG_D), OPS(PREG_D, PATTERN));

  void putSVE_predicate_initialize() {
    putSVE_predicate_initialize0();
  }
  
  /*** SVE predicate first active */
  PUT1(SVE_predicate_first_active0,
       NM("pfirst"),
       OPS(SPECIFIC32));

  void putSVE_predicate_first_active() {
    putSVE_predicate_first_active0();
  }

  /*** SVE predicate zero */
  PUT1(SVE_predicate_zero0,
       NM("pfalse"),
       OPS(PREG_B));

  void putSVE_predicate_zero() {
    putSVE_predicate_zero0();
  }

  /*** SVE predicate read from FFR (predicated) */
  PUT1(SVE_predicate_read_from_FFR_predicated0,
       NM("rdffr", "rdffrs"),
       OPS(PREG_B, PG_Z));

  void putSVE_predicate_read_from_FFR_predicated() {
    putSVE_predicate_read_from_FFR_predicated0();
  }

  /*** SVE predicate next active */
  PUT4(SVE_predicate_next_active0,
       NM("pnext"),
       OPS(SPECIFIC32), OPS(SPECIFIC32_1), OPS(SPECIFIC32_2), OPS(SPECIFIC32_3));

  void putSVE_predicate_next_active() {
    putSVE_predicate_next_active0();
  }
  
  /*** SVE predicate read from FFR (unpredicated) */
  PUT1(SVE_predicate_read_from_FFR_unpredicated0,
       NM("rdffr"),
       OPS(PREG_B));

  void putSVE_predicate_read_from_FFR_unpredicated() {
    putSVE_predicate_read_from_FFR_unpredicated0();
  }
  
  /** SVE Integer Compare - Signed Immediate */
  /*** SVE integer compare with signed immediate */
  PUT4(SVE_integer_compare_with_signed_immediate0,
       NM("cmpge", "cmpgt", "cmplt", "cmple", "cmpeq", "cmpne"),
       OPS(PREG_B, PG_Z, ZREG_B, SPECIFIC32), OPS(PREG_H, PG_Z, ZREG_H, SPECIFIC32),
       OPS(PREG_S, PG_Z, ZREG_S, SPECIFIC32), OPS(PREG_D, PG_Z, ZREG_D, SPECIFIC32));

  void putSVE_integer_compare_with_signed_immediate() {
    putSVE_integer_compare_with_signed_immediate0();
  }    

  /** SVE Predicate Count */
  /*** SVE predicate count */
  PUT4(SVE_predicate_count0,
       NM("cntp"),
       OPS(XREG, PG, PREG_B), OPS(XREG, PG, PREG_H), OPS(XREG, PG, PREG_S), OPS(XREG, PG, PREG_D));

  void putSVE_predicate_count() {
    putSVE_predicate_count0();
  }

  /** SVE Inc/Dec by Predicate Count */
  /*** SVE saturating inc/dec vector by predicate count */
  PUT3(SVE_saturating_inc_dec_vector_by_predicate_count0,
       NM("sqincp", "uqincp", "sqdecp", "uqdecp"),
       OPS(ZREG_H, PG), OPS(ZREG_S, PG), OPS(ZREG_D, PG));

  void putSVE_saturating_inc_dec_vector_by_predicate_count() {
    putSVE_saturating_inc_dec_vector_by_predicate_count0();
  }

  /*** SVE saturating inc/dec register by predicate count */
  PUT4(SVE_saturating_inc_dec_register_by_predicate_count0,
       NM("sqincp", "sqdecp"),
       OPS(SPECIFIC32), OPS(SPECIFIC32_1), OPS(SPECIFIC32_2), OPS(SPECIFIC32_3));
  PUT4(SVE_saturating_inc_dec_register_by_predicate_count1,
       NM("sqincp", "sqdecp"),
       OPS(XREG, PREG_B), OPS(XREG, PREG_H), OPS(XREG, PREG_S), OPS(XREG, PREG_D));
  PUT4(SVE_saturating_inc_dec_register_by_predicate_count2,
       NM("uqincp", "uqdecp"),
       OPS(WREG, PREG_B), OPS(WREG, PREG_H), OPS(WREG, PREG_S), OPS(WREG, PREG_D));
  PUT4(SVE_saturating_inc_dec_register_by_predicate_count3,
       NM("uqincp", "uqdecp"),
       OPS(XREG, PREG_B), OPS(XREG, PREG_H), OPS(XREG, PREG_S), OPS(XREG, PREG_D));

  void putSVE_saturating_inc_dec_register_by_predicate_count() {
    putSVE_saturating_inc_dec_register_by_predicate_count0();
    putSVE_saturating_inc_dec_register_by_predicate_count1();
    putSVE_saturating_inc_dec_register_by_predicate_count2();
    putSVE_saturating_inc_dec_register_by_predicate_count3();
  }    

  /*** SVE inc/dec vector by predicate count */
  PUT3(SVE_inc_dec_vector_by_predicate_count0,
       NM("incp", "decp"),
       OPS(ZREG_H, PG), OPS(ZREG_S, PG), OPS(ZREG_D, PG));

  void putSVE_inc_dec_vector_by_predicate_count() {
    putSVE_inc_dec_vector_by_predicate_count0();
  }
    
  /*** SVE inc/dec register by predicate count */
  PUT4(SVE_inc_dec_register_by_predicate_count0,
       NM("incp", "decp"),
       OPS(XREG, PREG_B), OPS(XREG, PREG_H), OPS(XREG, PREG_S), OPS(XREG, PREG_D));

  void putSVE_inc_dec_register_by_predicate_count() {
    putSVE_inc_dec_register_by_predicate_count0();
  }
  
  /** SVE Write FFR */
  /*** SVE FFR write from predicate */
  PUT1(SVE_FFR_write_from_predicate0,
       NM("wrffr"),
       OPS(PREG_B));

  void putSVE_FFR_write_from_predicate() {
    putSVE_FFR_write_from_predicate0();
  }    

  /*** SVE FFR initialise */
  PUT0(SVE_FFR_initialise0,
      NM("setffr"));

  void putSVE_FFR_initialise() {
    putSVE_FFR_initialise0();
  }

  /** SVE Integer Compare - Scalars */
  /*** SVE integer compare scalar count and limit */
  PUT8(SVE_integer_compare_scalar_count_and_limit0,
       NM("whilelt", "whilele", "whilelo", "whilels"),
       OPS(PREG_B, WREG, WREG), OPS(PREG_H, WREG, WREG),
       OPS(PREG_S, WREG, WREG), OPS(PREG_D, WREG, WREG),
       OPS(PREG_B, XREG, XREG), OPS(PREG_H, XREG, XREG),
       OPS(PREG_S, XREG, XREG), OPS(PREG_D, XREG, XREG));


  void putSVE_integer_compare_scalar_count_and_limit() {
    putSVE_integer_compare_scalar_count_and_limit0();
  }

  /*** SVE conditionally terminate scalars */
  PUT2(SVE_conditionally_terminate_scalars0,
       NM("ctermeq", "ctermne"),
       OPS(WREG, WREG), OPS(XREG, XREG));

  void putSVE_conditionally_terminate_scalars() {
    putSVE_conditionally_terminate_scalars0();
  }

  /** SVE Integer Wide Immediate - Unpredicated */
  /*** SVE integer add/subtract immediate (unpredicated) */
  PUT8(SVE_integer_add_subtract_immediate_unpredicated0,
       NM("add", "sub", "subr", "sqadd", "uqadd", "sqsub", "uqsub"),
       OPS(SPECIFIC32, IMM8BIT), OPS(SPECIFIC32, IMM8BIT, SPECIFIC64),
       OPS(SPECIFIC32_1, IMM8BIT), OPS(SPECIFIC32_1, IMM8BIT, SPECIFIC64_1),
       OPS(SPECIFIC32_2, IMM8BIT), OPS(SPECIFIC32_2, IMM8BIT, SPECIFIC64_1),
       OPS(SPECIFIC32_3, IMM8BIT), OPS(SPECIFIC32_3, IMM8BIT, SPECIFIC64_1));

  void putSVE_integer_add_subtract_immediate_unpredicated() {
    putSVE_integer_add_subtract_immediate_unpredicated0();
  }

  /*** SVE integer min/max immediate (unpredicated) */
  PUT4(SVE_integer_min_max_immediate_unpredicated0,
       NM("smax", "smin"),
       OPS(SPECIFIC32, SPECIFIC64_3), OPS(SPECIFIC32_1, SPECIFIC64_3), OPS(SPECIFIC32_2, SPECIFIC64_3), OPS(SPECIFIC32_3, SPECIFIC64_3));
  PUT4(SVE_integer_min_max_immediate_unpredicated1,
       NM("umax", "umin"),
       OPS(SPECIFIC32, IMM8BIT), OPS(SPECIFIC32_1, IMM8BIT), OPS(SPECIFIC32_2, IMM8BIT), OPS(SPECIFIC32_3, IMM8BIT));

  void putSVE_integer_min_max_immediate_unpredicated() {
    putSVE_integer_min_max_immediate_unpredicated0();
    putSVE_integer_min_max_immediate_unpredicated1();
  }

  /*** SVE integer multiply immediate (unpredicated) */
  PUT4(SVE_integer_multiply_immediate_unpredicated0,
       NM("mul"),
       OPS(SPECIFIC32, SPECIFIC64_3), OPS(SPECIFIC32_1, SPECIFIC64_3), OPS(SPECIFIC32_2, SPECIFIC64_3), OPS(SPECIFIC32_3, SPECIFIC64_3));

  void putSVE_integer_multiply_immediate_unpredicated() {
    putSVE_integer_multiply_immediate_unpredicated0();
  }

  /*** SVE broadcast integer immediate (unpredicated) */
  PUT8(SVE_broadcast_integer_immediate_unpredicated0,
       NM("dup"),
       OPS(ZREG_B, SPECIFIC64_3), OPS(ZREG_B, SPECIFIC64_3, SPECIFIC64),
       OPS(ZREG_H, SPECIFIC64_3), OPS(ZREG_H, SPECIFIC64_3, SPECIFIC64_1),
       OPS(ZREG_S, SPECIFIC64_3), OPS(ZREG_S, SPECIFIC64_3, SPECIFIC64_1),
       OPS(ZREG_D, SPECIFIC64_3), OPS(ZREG_D, SPECIFIC64_3, SPECIFIC64_1));

  void putSVE_broadcast_integer_immediate_unpredicated() {
    putSVE_broadcast_integer_immediate_unpredicated0();
  }      

  /*** SVE broadcast floating-point immediate (unpredicated) */
  PUT3(SVE_broadcast_floating_point_immediate_unpredicated0,
       NM("fdup"),
       OPS(ZREG_H, FLOAT8BIT), OPS(ZREG_S, FLOAT8BIT), OPS(ZREG_D, FLOAT8BIT));

  void putSVE_broadcast_floating_point_immediate_unpredicated() {
    putSVE_broadcast_floating_point_immediate_unpredicated0();
  }

  /** SVE Integer Multiply-Add - Unpredicated */
  /*** SVE integer dot product (unpredicated) */
  PUT2(SVE_integer_dot_product_unpredicated0,
       NM("sdot", "udot"),
       OPS(ZREG_S, ZREG_B, ZREG_B), OPS(ZREG_D, ZREG_H, ZREG_H));

  void putSVE_integer_dot_product_unpredicated() {
    putSVE_integer_dot_product_unpredicated0();
  }
  
  /** SVE Multiply - Indexed */
  /*** SVE integer dot product (indexed) */
  PUT1(SVE_integer_dot_product_indexed0,
       NM("sdot", "udot"),
       OPS(ZREG_S, ZREG_B, SPECIFIC32));
  PUT1(SVE_integer_dot_product_indexed1,
       NM("sdot", "udot"),
       OPS(ZREG_D, ZREG_H, SPECIFIC64));

  void putSVE_integer_dot_product_indexed() {
    putSVE_integer_dot_product_indexed0();
    putSVE_integer_dot_product_indexed1();
  }
    
  /** SVE Floating Point Complex Addition */
  /*** SVE floating-point complex add (predicated) */
  PUT3(SVE_floating_point_complex_add_predicated0,
       NM("fcadd"),
       OPS(SPECIFIC64,   ZREG_H, SPECIFIC32),
       OPS(SPECIFIC64_1, ZREG_S, SPECIFIC32),
       OPS(SPECIFIC64_2, ZREG_D, SPECIFIC32));

  void putSVE_floating_point_complex_add_predicated() {
    putSVE_floating_point_complex_add_predicated0();
  }
  
  /** SVE Floating Point Complex Multiply-Add */
  /*** SVE floating-point complex multiply-add (predicated) */
  PUT3(SVE_floating_point_complex_multiply_add_predicated0,
       NM("fcmla"),
       OPS(ZREG_H, PG_M, ZREG_H, ZREG_H, IMM2BIT),
       OPS(ZREG_S, PG_M, ZREG_S, ZREG_S, IMM2BIT),
       OPS(ZREG_D, PG_M, ZREG_D, ZREG_D, IMM2BIT));

  void putSVE_floating_point_complex_multiply_add_predicated() {
    putSVE_floating_point_complex_multiply_add_predicated0();
  }

  /** SVE Floating Point Multiply-Add - Indexed */
  /*** SVE floating-point multiply-add (indexed) */
  PUT1(SVE_floating_point_multiply_add_indexed0,
       NM("fmla", "fmls"),
       OPS(ZREG_H, ZREG_H, SPECIFIC32));
  PUT1(SVE_floating_point_multiply_add_indexed1,
       NM("fmla", "fmls"),
       OPS(ZREG_S, ZREG_S, SPECIFIC32_1));
  PUT1(SVE_floating_point_multiply_add_indexed2,
       NM("fmla", "fmls"),
       OPS(ZREG_D, ZREG_D, SPECIFIC32_2));

  void putSVE_floating_point_multiply_add_indexed() {
    putSVE_floating_point_multiply_add_indexed0();
    putSVE_floating_point_multiply_add_indexed1();
    putSVE_floating_point_multiply_add_indexed2();
  }
  
  /** SVE Floating Point Complex Multiply-Add - Indexed */
  /*** SVE floating-point complex multiply-add (indexed) */
  PUT1(SVE_floating_point_complex_multiply_add_indexed0,
       NM("fcmla"),
       OPS(ZREG_H, ZREG_H, SPECIFIC32, SPECIFIC64));
  PUT1(SVE_floating_point_complex_multiply_add_indexed1,
       NM("fcmla"),
       OPS(ZREG_S, ZREG_S, SPECIFIC32_1, SPECIFIC64));

  void putSVE_floating_point_complex_multiply_add_indexed() {
    putSVE_floating_point_complex_multiply_add_indexed0();
    putSVE_floating_point_complex_multiply_add_indexed1();
  }

  /** SVE Floating Point Multiply - Indexed */
  /*** SVE floating-point multiply (indexed) */
  PUT1(SVE_floating_point_multiply_indexed0,
       NM("fmul"),
       OPS(ZREG_H, ZREG_H, SPECIFIC32));
  PUT1(SVE_floating_point_multiply_indexed1,
       NM("fmul"),
       OPS(ZREG_S, ZREG_S, SPECIFIC32_1));
  PUT1(SVE_floating_point_multiply_indexed2,
       NM("fmul"),
       OPS(ZREG_D, ZREG_D, SPECIFIC32_2));

  void putSVE_floating_point_multiply_indexed() {
    putSVE_floating_point_multiply_indexed0();
    putSVE_floating_point_multiply_indexed1();
    putSVE_floating_point_multiply_indexed2();
  }
  
  /** SVE Floating Point Fast Reduction */
  /***	SVE floating-point recursive reduction */
  PUT3(SVE_floating_point_recursive_reduction0,
       NM("faddv", "fmaxnmv", "fminnmv", "fmaxv", "fminv"),
       OPS(HREG, PG, ZREG_H), OPS(SREG, PG, ZREG_S), OPS(DREG, PG, ZREG_D));

  void putSVE_floating_point_recursive_reduction() {
    putSVE_floating_point_recursive_reduction0();
  }

  /** SVE Floating Point Unary Operations - Unpredicated */
  /*** SVE floating-point reciprocal estimate (unpredicated) */
  PUT3(SVE_floating_point_reciprocal_estimate_unpredicated0,
       NM("frecpe", "frsqrte"),
       OPS(ZREG_H, ZREG_H), OPS(ZREG_S, ZREG_S), OPS(ZREG_D, ZREG_D));

  void putSVE_floating_point_reciprocal_estimate_unpredicated() {
    putSVE_floating_point_reciprocal_estimate_unpredicated0();
  }

  /** SVE Floating Point Compare - with Zero */
  /*** SVE floating-point compare with zero */
  PUT3(SVE_floating_point_compare_with_zero0,
       NM("fcmge", "fcmgt", "fcmlt", "fcmle", "fcmeq", "fcmne"),
       OPS(PREG_H, PG_Z, ZREG_H, SPECIFIC32), OPS(PREG_S, PG_Z, ZREG_S, SPECIFIC32), OPS(PREG_D, PG_Z, ZREG_D, SPECIFIC32));

  void putSVE_floating_point_compare_with_zero() {
    putSVE_floating_point_compare_with_zero0();
  }

  /** SVE Floating Point Accumulating Reduction */
  /*** SVE floating-point serial reduction (predicated) */
  PUT3(SVE_floating_point_serial_reduction_predicated0,
       NM("fadda"),
       OPS(SPECIFIC32, ZREG_H), OPS(SPECIFIC32_1, ZREG_S), OPS(SPECIFIC32_2, ZREG_D));

  void putSVE_floating_point_serial_reduction_predicated() {
    putSVE_floating_point_serial_reduction_predicated0();
  }

  /** SVE Floating Point Arithmetic - Unpredicated */
  /*** SVE floating-point arithmetic (unpredicated) */
  PUT3(SVE_floating_point_arithmetic_unpredicated0,
       NM("fadd", "fsub", "fmul", "ftsmul", "frecps", "frsqrts"),
       OPS(ZREG_H, ZREG_H, ZREG_H), OPS(ZREG_S, ZREG_S, ZREG_S),
       OPS(ZREG_D, ZREG_D, ZREG_D));

  void putSVE_floating_point_arithmetic_unpredicated() {
    putSVE_floating_point_arithmetic_unpredicated0();
  }

  /** SVE Floating Point Arithmetic - Predicated */
  /*** SVE floating-point arithmetic (predicated) */
  PUT3(SVE_floating_point_arithmetic_predicated0,
       NM("fadd", "fsub", "fmul", "fsubr", "fmaxnm", "fminnm", "fmax", "fmin",
	  "fabd", "fscale", "fmulx", "fdivr", "fdiv"),
       OPS(SPECIFIC32, ZREG_H), OPS(SPECIFIC32_1, ZREG_S), 
       OPS(SPECIFIC32_2, ZREG_D));

  void putSVE_floating_point_arithmetic_predicated() {
    putSVE_floating_point_arithmetic_predicated0();
  }

  /*** SVE floating-point trig multiply-add coefficient */
  PUT3(SVE_floating_point_trig_multiply_add_coefficient0,
       NM("ftmad"),
       OPS(SPECIFIC64, ZREG_H, IMM3BIT), OPS(SPECIFIC64_1, ZREG_S, IMM3BIT), OPS(SPECIFIC64_2, ZREG_D, IMM3BIT));

  void putSVE_floating_point_trig_multiply_add_coefficient() {
    putSVE_floating_point_trig_multiply_add_coefficient0();
  }
    
  /*** SVE floating-point arithmetic with immediate (predicated) */
  PUT3(SVE_floating_point_arithmetic_with_immediate_predicated0,
       NM("fadd", "fsub", "fsubr"),
       OPS(SPECIFIC32, SPECIFIC32_3), OPS(SPECIFIC32_1, SPECIFIC32_3), OPS(SPECIFIC32_2, SPECIFIC32_3));

  PUT3(SVE_floating_point_arithmetic_with_immediate_predicated1,
       NM("fmul"),
       OPS(SPECIFIC32, SPECIFIC64_3), OPS(SPECIFIC32_1, SPECIFIC64_3), OPS(SPECIFIC32_2, SPECIFIC64_3));

  PUT3(SVE_floating_point_arithmetic_with_immediate_predicated2,
       NM("fmaxnm", "fminnm", "fmax", "fmin"),
       OPS(SPECIFIC32, SPECIFIC64_4), OPS(SPECIFIC32_1, SPECIFIC64_4), OPS(SPECIFIC32_2, SPECIFIC64_4));

  void putSVE_floating_point_arithmetic_with_immediate_predicated() {
    putSVE_floating_point_arithmetic_with_immediate_predicated0();
    putSVE_floating_point_arithmetic_with_immediate_predicated1();
    putSVE_floating_point_arithmetic_with_immediate_predicated2();
  }

  /** SVE Floating Point Unary Operations - Predicated */
  /*** SVE floating-point round to integral value */
  PUT3(SVE_floating_point_round_to_integral_value0,
       NM("frintn", "frintp", "frintm", "frintz", "frinta", "frintx", "frinti"),
       OPS(ZREG_H, PG_M, ZREG_H), OPS(ZREG_S, PG_M, ZREG_S), OPS(ZREG_D, PG_M, ZREG_D));

  void putSVE_floating_point_round_to_integral_value() {
    putSVE_floating_point_round_to_integral_value0();
  }

  /*** SVE floating-point convert precision */
  PUT6(SVE_floating_point_convert_precision0,
       NM("fcvt"),
       OPS(ZREG_H, PG_M, ZREG_S), OPS(ZREG_S, PG_M, ZREG_H), OPS(ZREG_H, PG_M, ZREG_D),
       OPS(ZREG_D, PG_M, ZREG_H), OPS(ZREG_S, PG_M, ZREG_D), OPS(ZREG_D, PG_M, ZREG_S));

  void putSVE_floating_point_convert_precision() {
    putSVE_floating_point_convert_precision0();
  }

  /*** SVE floating-point unary operations */
  PUT3(SVE_floating_point_unary_operations0,
       NM("frecpx", "fsqrt"),
       OPS(ZREG_H, PG_M, ZREG_H), OPS(ZREG_S, PG_M, ZREG_S), OPS(ZREG_D, PG_M, ZREG_D));

  void putSVE_floating_point_unary_operations() {
    putSVE_floating_point_unary_operations0();
  }

  /*** SVE integer convert to floating-point */
  PUT1(SVE_integer_convert_to_floating_point0,
       NM("scvtf", "ucvtf"),
       OPS(ZREG_H, PG_M, ZREG_H));
  PUT1(SVE_integer_convert_to_floating_point1,
       NM("scvtf", "ucvtf"),
       OPS(ZREG_H, PG_M, ZREG_S));
  PUT1(SVE_integer_convert_to_floating_point2,
       NM("scvtf", "ucvtf"),
       OPS(ZREG_H, PG_M, ZREG_D));
  PUT1(SVE_integer_convert_to_floating_point3,
       NM("scvtf", "ucvtf"),
       OPS(ZREG_S, PG_M, ZREG_S));
  PUT1(SVE_integer_convert_to_floating_point4,
       NM("scvtf", "ucvtf"),
       OPS(ZREG_D, PG_M, ZREG_S));
  PUT1(SVE_integer_convert_to_floating_point5,
       NM("scvtf", "ucvtf"),
       OPS(ZREG_S, PG_M, ZREG_D));
  PUT1(SVE_integer_convert_to_floating_point6,
       NM("scvtf", "ucvtf"),
       OPS(ZREG_D, PG_M, ZREG_D));

  void nputSVE_integer_convert_to_floating_point() {
    putSVE_integer_convert_to_floating_point0();
    putSVE_integer_convert_to_floating_point1();
    putSVE_integer_convert_to_floating_point2();
    putSVE_integer_convert_to_floating_point3();
    putSVE_integer_convert_to_floating_point4();
    putSVE_integer_convert_to_floating_point5();
    putSVE_integer_convert_to_floating_point6();
  }

  /*** SVE floating-point convert to integer*/
  PUT1(SVE_floating_point_convert_to_integer0,
       NM("fcvtzs", "fcvtzu"),
       OPS(ZREG_H, PG_M, ZREG_H));
  PUT1(SVE_floating_point_convert_to_integer1,
       NM("fcvtzs", "fcvtzu"),
       OPS(ZREG_S, PG_M, ZREG_H));
  PUT1(SVE_floating_point_convert_to_integer2,
       NM("fcvtzs", "fcvtzu"),
       OPS(ZREG_D, PG_M, ZREG_H));
  PUT1(SVE_floating_point_convert_to_integer3,
       NM("fcvtzs", "fcvtzu"),
       OPS(ZREG_S, PG_M, ZREG_S));
  PUT1(SVE_floating_point_convert_to_integer4,
       NM("fcvtzs", "fcvtzu"),
       OPS(ZREG_S, PG_M, ZREG_D));
  PUT1(SVE_floating_point_convert_to_integer5,
       NM("fcvtzs", "fcvtzu"),
       OPS(ZREG_D, PG_M, ZREG_S));
  PUT1(SVE_floating_point_convert_to_integer6,
       NM("fcvtzs", "fcvtzu"),
       OPS(ZREG_D, PG_M, ZREG_D));

  void putSVE_floating_point_convert_to_integer() {
    putSVE_floating_point_convert_to_integer0();
    putSVE_floating_point_convert_to_integer1();
    putSVE_floating_point_convert_to_integer2();
    putSVE_floating_point_convert_to_integer3();
    putSVE_floating_point_convert_to_integer4();
    putSVE_floating_point_convert_to_integer5();
    putSVE_floating_point_convert_to_integer6();
  }

  /** SVE Floating Point Compare - Vectors */
  /*** SVE floating-point compare vectors */
  PUT3(SVE_floating_point_compare_vectors0,
       NM("fcmge", "fcmgt", "fcmeq", "fcmne", "fcmuo", "facge", "facgt"),
       OPS(PREG_H, PG_Z, ZREG_H, ZREG_H), OPS(PREG_S, PG_Z, ZREG_S, ZREG_S),
       OPS(PREG_D, PG_Z, ZREG_D, ZREG_D));

  void putSVE_floating_point_compare_vectors() {
    putSVE_floating_point_compare_vectors0();
  }

  /** SVE Floating Point Multiply-Add */
  /*** SVE floating-point multiply-accumulate writing addend */
  PUT3(SVE_floating_point_multiply_accumulate_writing_addend0,
       NM("fmla", "fmls", "fnmla", "fnmls"),
       OPS(ZREG_H, PG_M, ZREG_H, ZREG_H), OPS(ZREG_S, PG_M, ZREG_S, ZREG_S),
       OPS(ZREG_D, PG_M, ZREG_D, ZREG_D));

  void putSVE_floating_point_multiply_accumulate_writing_addend() {
    putSVE_floating_point_multiply_accumulate_writing_addend0();
  }
    
  /*** SVE floating-point multiply-accumulate writing multiplicand */
  PUT3(SVE_floating_point_multiply_accumulate_writing_multiplicand0,
       NM("fmad", "fmsb", "fnmad", "fnmsb"),
       OPS(ZREG_H, PG_M, ZREG_H, ZREG_H), OPS(ZREG_S, PG_M, ZREG_S, ZREG_S),
       OPS(ZREG_D, PG_M, ZREG_D, ZREG_D));

  void putSVE_floating_point_multiply_accumulate_writing_multiplicand() {
    putSVE_floating_point_multiply_accumulate_writing_multiplicand0();
  }

  void putSVE_IntegerBinaryArithmeticPredicatedGroup() {
    setDstPred1stSrcZreg(TP_m, tv_SPECIFIC32, jtv_SPECIFIC32, "b", tv_ZREG);
    setDstPred1stSrcZreg(TP_m, tv_SPECIFIC32_1, jtv_SPECIFIC32_1, "h", tv_ZREG);
    setDstPred1stSrcZreg(TP_m, tv_SPECIFIC32_2, jtv_SPECIFIC32_2, "s", tv_ZREG);
    setDstPred1stSrcZreg(TP_m, tv_SPECIFIC32_3, jtv_SPECIFIC32_3, "d", tv_ZREG);

    putSVE_integer_add_subtract_vectors_predicated();
    putSVE_integer_min_max_difference_predicated();
    putSVE_integer_multiply_vectors_predicated();
    putSVE_integer_divide_vectors_predicate();
    putSVE_bitwise_logical_operations_predicated();
  }

    
  void putSVE_IntegerReductionGroup() {
    putSVE_integer_add_reduction_predicated();
    putSVE_integer_min_max_reduction_predicated();
    putSVE_constructive_prefix_predicated();
    putSVE_bitwise_logical_reduction_predicated();
  }
  
  void putSVE_BitwiseShiftPredicatedGroup() {
    setDstPred1stSrcZreg(TP_m, tv_SPECIFIC32, jtv_SPECIFIC32, "b", tv_ZREG);
    setDstPred1stSrcZreg(TP_m, tv_SPECIFIC32_1, jtv_SPECIFIC32_1, "h", tv_ZREG);
    setDstPred1stSrcZreg(TP_m, tv_SPECIFIC32_2, jtv_SPECIFIC32_2, "s", tv_ZREG);
    setDstPred1stSrcZreg(TP_m, tv_SPECIFIC32_3, jtv_SPECIFIC32_3, "d", tv_ZREG);

    putSVE_bitwise_shift_by_immediate_predicated();
    putSVE_bitwise_shift_by_vector_predicated();
    putSVE_bitwise_shift_by_wide_elements_predicated();
  }

  void putSVE_IntegerUnaryArithmeticPredicatedGroup() {
    putSVE_integer_unary_operations_predicated();
    putSVE_bitwise_unary_operations_predicated();
  }

  void putSVE_IntegerMultiplyAddPredicatedGroup() {
    putSVE_integer_multiply_accumulate_writing_addend_predicated();
    putSVE_integer_multiply_add_writing_multiplicand_predicated();
  }

  void putSVE_IntegerArithmeticUnpredicated() {
    putSVE_integer_add_subtract_vectors_unpredicated();
  }

  void putSVE_BitwiseLogicalUnpredicated() {
    putSVE_bitwise_logical_operations_unpredicated();
  }

  void putSVE_IndexGeneration() {
    std::vector<int> tmp = { -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 15 };

    tv_SPECIFIC32.clear();
    jtv_SPECIFIC32.clear();

    for(int i : tmp) {
      tv_SPECIFIC32.push_back(std::to_string(i));
      jtv_SPECIFIC32.push_back(std::to_string(i));
    }      
    
    putSVE_index_generation_immediate_start_immediate_increment();
    putSVE_index_generation_register_start_immediate_increment();
    putSVE_index_generation_immediate_start_register_increment();
    putSVE_index_generation_register_start_register_increment();
  }    

  void putSVE_StackAllocation() {
    std::vector<int> tmp = { -15, -32, -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 16, 31 };

    tv_SPECIFIC32.clear();
    jtv_SPECIFIC32.clear();

    for(int i : tmp) {
      tv_SPECIFIC32.push_back(std::to_string(i));
      jtv_SPECIFIC32.push_back(std::to_string(i));
    }
    
    putSVE_stack_frame_adjustment();
    putSVE_stack_frame_size();
  }    
  
  void putSVE_BitwiseShiftUnpredicated() {
    putSVE_bitwise_shift_by_wide_elements_unpredicate();
    putSVE_bitwise_shift_by_immediate_unpredicated();
  }

  void  putSVE_AddressGenerationGroup() {
    std::vector<std::string> tmpReg = { "z15", "z0", "z1", "z2", "z3", "z4", "z8", "z16", "z31" };
    std::vector<std::string> tmpSh  = { "SXTW", "UXTW" };
    std::vector<std::string> tmpAmount = { "1", "2", "3" };

    /** Unpacked 32-bit signed offsets */
    /** Unpacked 32-bit unsigned offsets */
    tv_SPECIFIC32.clear();
    jtv_SPECIFIC32.clear();

    /*** Base scalable vector register rotation */
    for(std::string j : tmpSh) {
      for(std::string i : tmpReg) {
	tv_SPECIFIC32.push_back("[" + i + ".d, " + tmpReg[0] + ".d, " + j + "]");
	jtv_SPECIFIC32.push_back("ptr(" + i + ".d, " + tmpReg[0] + ".d, " + j + ")");

	tv_SPECIFIC32.push_back("[" + i + ".d, " + tmpReg[0] + ".d, " + j + " 1]");
	jtv_SPECIFIC32.push_back("ptr(" + i + ".d, " + tmpReg[0] + ".d, " + j + ", 1)");
	tv_SPECIFIC32.push_back("[" + i + ".d, " + tmpReg[0] + ".d, " + j + " 2]");
	jtv_SPECIFIC32.push_back("ptr(" + i + ".d, " + tmpReg[0] + ".d, " + j + ", 2)");
	tv_SPECIFIC32.push_back("[" + i + ".d, " + tmpReg[0] + ".d, " + j + " 3]");
	jtv_SPECIFIC32.push_back("ptr(" + i + ".d, " + tmpReg[0] + ".d, " + j + ", 3)");
      }
    }

    /*** Offset scalable vector register rotation */
    for(std::string j : tmpSh) {
      for(std::string i : tmpReg) {
	tv_SPECIFIC32.push_back("[" + tmpReg[0] + ".d, " + i + ".d, " + j + "]");
	jtv_SPECIFIC32.push_back("ptr(" + tmpReg[0] + ".d, " + i + ".d, " + j + ")");

	tv_SPECIFIC32.push_back("[" + tmpReg[0] + ".d, " + i + ".d, " + j + " 1]");
	jtv_SPECIFIC32.push_back("ptr(" + tmpReg[0] + ".d, " + i + ".d, " + j + ", 1)");
	tv_SPECIFIC32.push_back("[" + tmpReg[0] + ".d, " + i + ".d, " + j + " 2]");
	jtv_SPECIFIC32.push_back("ptr(" + tmpReg[0] + ".d, " + i + ".d, " + j + ", 2)");
	tv_SPECIFIC32.push_back("[" + tmpReg[0] + ".d, " + i + ".d, " + j + " 3]");
	jtv_SPECIFIC32.push_back("ptr(" + tmpReg[0] + ".d, " + i + ".d, " + j + ", 3)");
      }
    }

    /** Packed offsets */
    tv_SPECIFIC32_1.clear();
    tv_SPECIFIC32_2.clear();
    jtv_SPECIFIC32_1.clear();
    jtv_SPECIFIC32_2.clear();

    /*** Base scalable vector register rotation */
    for(std::string i : tmpReg) {
      tv_SPECIFIC32_1.push_back("[" + i + ".s, " + tmpReg[0] + ".s]");
      jtv_SPECIFIC32_1.push_back("ptr(" + i + ".s, " + tmpReg[0] + ".s)");

      tv_SPECIFIC32_1.push_back("[" + i + ".s, " + tmpReg[0] + ".s, LSL 1]");
      jtv_SPECIFIC32_1.push_back("ptr(" + i + ".s, " + tmpReg[0] + ".s, LSL, 1)");
      tv_SPECIFIC32_1.push_back("[" + i + ".s, " + tmpReg[0] + ".s, LSL 2]");
      jtv_SPECIFIC32_1.push_back("ptr(" + i + ".s, " + tmpReg[0] + ".s, LSL, 2)");
      tv_SPECIFIC32_1.push_back("[" + i + ".s, " + tmpReg[0] + ".s, LSL 3]");
      jtv_SPECIFIC32_1.push_back("ptr(" + i + ".s, " + tmpReg[0] + ".s, LSL, 3)");

      tv_SPECIFIC32_2.push_back("[" + i + ".d, " + tmpReg[0] + ".d]");
      jtv_SPECIFIC32_2.push_back("ptr(" + i + ".d, " + tmpReg[0] + ".d)");

      tv_SPECIFIC32_2.push_back("[" + i + ".d, " + tmpReg[0] + ".d, LSL 1]");
      jtv_SPECIFIC32_2.push_back("ptr(" + i + ".d, " + tmpReg[0] + ".d, LSL, 1)");
      tv_SPECIFIC32_2.push_back("[" + i + ".d, " + tmpReg[0] + ".d, LSL 2]");
      jtv_SPECIFIC32_2.push_back("ptr(" + i + ".d, " + tmpReg[0] + ".d, LSL, 2)");
      tv_SPECIFIC32_2.push_back("[" + i + ".d, " + tmpReg[0] + ".d, LSL 3]");
      jtv_SPECIFIC32_2.push_back("ptr(" + i + ".d, " + tmpReg[0] + ".d, LSL, 3)");
    }

    /*** Offset scalable vector register rotation */
    for(std::string i : tmpReg) {
      tv_SPECIFIC32_1.push_back("[" + tmpReg[0] + ".s, " + i + ".s]");
      jtv_SPECIFIC32_1.push_back("ptr(" + tmpReg[0] + ".s, " + i + ".s)");

      tv_SPECIFIC32_1.push_back("[" + tmpReg[0] + ".s, " + i + ".s, LSL 1]");
      jtv_SPECIFIC32_1.push_back("ptr(" + tmpReg[0] + ".s, " + i + ".s, LSL, 1)");
      tv_SPECIFIC32_1.push_back("[" + tmpReg[0] + ".s, " + i + ".s, LSL 2]");
      jtv_SPECIFIC32_1.push_back("ptr(" + tmpReg[0] + ".s, " + i + ".s, LSL, 2)");
      tv_SPECIFIC32_1.push_back("[" + tmpReg[0] + ".s, " + i + ".s, LSL 3]");
      jtv_SPECIFIC32_1.push_back("ptr(" + tmpReg[0] + ".s, " + i + ".s, LSL, 3)");

      tv_SPECIFIC32_2.push_back("[" + tmpReg[0] + ".d, " + i + ".d]");
      jtv_SPECIFIC32_2.push_back("ptr(" + tmpReg[0] + ".d, " + i + ".d)");

      tv_SPECIFIC32_2.push_back("[" + tmpReg[0] + ".d, " + i + ".d, LSL 1]");
      jtv_SPECIFIC32_2.push_back("ptr(" + tmpReg[0] + ".d, " + i + ".d, LSL, 1)");
      tv_SPECIFIC32_2.push_back("[" + tmpReg[0] + ".d, " + i + ".d, LSL 2]");
      jtv_SPECIFIC32_2.push_back("ptr(" + tmpReg[0] + ".d, " + i + ".d, LSL, 2)");
      tv_SPECIFIC32_2.push_back("[" + tmpReg[0] + ".d, " + i + ".d, LSL 3]");
      jtv_SPECIFIC32_2.push_back("ptr(" + tmpReg[0] + ".d, " + i + ".d, LSL, 3)");
    }
    
    putSVE_address_generation();
  }

  void putSVE_IntegerMiscUnpredicated() {
    putSVE_floating_point_trig_select_coefficient();
    putSVE_floating_point_exponential_accelerator();
    putSVE_constructive_prefix_unpredicated();
  }

  void putSVE_ElementCount() {
    std::vector<int> tmpIdx = { 15, 0, 1, 2, 4, 8, 16, 30 };
    
    tv_SPECIFIC32.clear();
    jtv_SPECIFIC32.clear();

    for(std::string i : tv_IMM4BIT_N) {
      tv_SPECIFIC32.push_back("MUL " + i);
      jtv_SPECIFIC32.push_back("MUL, " + i);
    }

    tv_SPECIFIC32_1.clear();
    jtv_SPECIFIC32_1.clear();
    
    for(int i : tmpIdx) {
      tv_SPECIFIC32_1.push_back("x" + std::to_string(i) + ", w" + std::to_string(i));
      jtv_SPECIFIC32_1.push_back("w" + std::to_string(i));
    }
    
    putSVE_saturating_inc_dec_register_by_element_count();
    putSVE_saturating_inc_dec_vector_by_element_count();
    putSVE_element_count();
    putSVE_inc_dec_vector_by_element_count();
    putSVE_inc_dec_register_by_element_count();
  }
    
  void putSVE_BitwiseImmediate() {
    std::vector<int> tmpIdx = { 15, 0, 1, 2, 4, 8, 16, 30 };

    tv_SPECIFIC32.clear();    jtv_SPECIFIC32.clear();    
    tv_SPECIFIC32_1.clear();  jtv_SPECIFIC32_1.clear();  
    tv_SPECIFIC32_2.clear();  jtv_SPECIFIC32_2.clear();  
    tv_SPECIFIC32_3.clear();  jtv_SPECIFIC32_3.clear();  
    tv_SPECIFIC64.clear();    jtv_SPECIFIC64.clear();    
    tv_SPECIFIC64_1.clear();  jtv_SPECIFIC64_1.clear();  
    tv_SPECIFIC64_2.clear();  jtv_SPECIFIC64_2.clear();  
    tv_SPECIFIC64_3.clear();  jtv_SPECIFIC64_3.clear();  

    /** Dst and src register rotation */
    for(int i : tmpIdx) {
      tv_SPECIFIC32.push_back("z" + std::to_string(i) + ".b, " + "z" + std::to_string(i) + ".b");
      tv_SPECIFIC32_1.push_back("z" + std::to_string(i) + ".h, " + "z" + std::to_string(i) + ".h");
      tv_SPECIFIC32_2.push_back("z" + std::to_string(i) + ".s, " + "z" + std::to_string(i) + ".s");
      tv_SPECIFIC32_3.push_back("z" + std::to_string(i) + ".d, " + "z" + std::to_string(i) + ".d");

      jtv_SPECIFIC32.push_back("z" + std::to_string(i) + ".b");
      jtv_SPECIFIC32_1.push_back("z" + std::to_string(i) + ".h");
      jtv_SPECIFIC32_2.push_back("z" + std::to_string(i) + ".s");
      jtv_SPECIFIC32_3.push_back("z" + std::to_string(i) + ".d");
    }    


    setRotatedOnes(tv_SPECIFIC64, jtv_SPECIFIC64, 8);
    setRotatedOnes(tv_SPECIFIC64_1, jtv_SPECIFIC64_1, 16);
    setRotatedOnes(tv_SPECIFIC64_2, jtv_SPECIFIC64_2, 32);
    setRotatedOnes(tv_SPECIFIC64_3, jtv_SPECIFIC64_3, 64);


    /** Debug
	for(std::string i : tv_SPECIFIC64_3) {
	std::cout << i << std::endl;
	} */
    
    putSVE_bitwise_logical_with_immediate_unpredicated();
    putSVE_broadcast_bitmask_immediate();
  }

  void putSVE_IntegerWideImmediatePredicated() {
    std::vector<int> tmp = { 127, -128, -64, -32, -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 16, 32, 64 };
    tv_SPECIFIC32.clear();
    tv_SPECIFIC64.clear();
    tv_SPECIFIC64_1.clear();
    jtv_SPECIFIC32.clear();
    jtv_SPECIFIC64.clear();
    jtv_SPECIFIC64_1.clear();

    for(int i : tmp) {
      tv_SPECIFIC32.push_back(std::to_string(i));
      jtv_SPECIFIC32.push_back(std::to_string(i));
    }

    tv_SPECIFIC64.push_back("LSL 0");
    tv_SPECIFIC64.push_back("LSL 8");
    jtv_SPECIFIC64.push_back("LSL, 0");
    jtv_SPECIFIC64.push_back("LSL, 8");
    
    tv_SPECIFIC64_1.push_back("LSL 0");
    jtv_SPECIFIC64_1.push_back("LSL, 0");

    putSVE_copy_integer_immediate_predicated();
    putSVE_copy_floating_point_immediate_predicated();
  }

  void putSVE_PermuteVectorExtract() {
    tv_SPECIFIC32.clear();
    jtv_SPECIFIC32.clear();

    for(std::string i : tv_ZREG_B) {
      tv_SPECIFIC32.push_back(i + ", " + i);
      jtv_SPECIFIC32.push_back(i);
    }

    putSVE_extract_vector_immediate_offset_destructive();
  }
 
  void putSVE_PermuteVectorUnpredicated() {
    std::vector<int> tmpB = { 31, 0, 1, 2, 4, 8, 16, 32, 63 };
    std::vector<int> tmpH = { 31, 0, 1, 2, 4, 8, 16 };
    std::vector<int> tmpS = { 7, 0, 1, 2, 4, 8, 15 };
    std::vector<int> tmpD= { 7, 0, 1, 2, 4 };
    std::vector<int> tmpQ= { 3, 0, 1, 2 };

    tv_SPECIFIC32.clear();
    tv_SPECIFIC32_1.clear();
    tv_SPECIFIC32_2.clear();
    tv_SPECIFIC32_3.clear();
    tv_SPECIFIC64.clear();
    jtv_SPECIFIC32.clear();
    jtv_SPECIFIC32_1.clear();
    jtv_SPECIFIC32_2.clear();
    jtv_SPECIFIC32_3.clear();
    jtv_SPECIFIC64.clear();

    setZregTypeIndex(tv_SPECIFIC32, jtv_SPECIFIC32, tv_ZREG_B, tmpB, "b");
    setZregTypeIndex(tv_SPECIFIC32_1, jtv_SPECIFIC32_1, tv_ZREG_H, tmpH, "h");
    setZregTypeIndex(tv_SPECIFIC32_2, jtv_SPECIFIC32_2, tv_ZREG_S, tmpS, "s");
    setZregTypeIndex(tv_SPECIFIC32_3, jtv_SPECIFIC32_3, tv_ZREG_D, tmpD, "d");
    setZregTypeIndex(tv_SPECIFIC64, jtv_SPECIFIC64, tv_ZREG_Q, tmpQ, "q");
    
    putSVE_broadcast_indexed_element();
    putSVE_table_lookup();
    putSVE_broadcast_general_register();
    putSVE_insert_general_register();
    putSVE_unpack_vector_elements();
    putSVE_insert_SIMD_FP_scalar_register();
    putSVE_reverse_vector_elements();
  }

  void putSVE_PermutePredicate() {
    putSVE_permute_predicate_elements();
    putSVE_reverse_predicate_elements();
    putSVE_unpack_predicate_elements();
  }

  void putSVE_PermuteVectorInterleaving() {
    putSVE_permute_vector_elements();
  }

  void putSVE_PermuteVectorPredicated() {
    tv_SPECIFIC32.clear();
    tv_SPECIFIC32_1.clear();
    tv_SPECIFIC32_2.clear();
    tv_SPECIFIC32_3.clear();
    jtv_SPECIFIC32.clear();
    jtv_SPECIFIC32_1.clear();
    jtv_SPECIFIC32_2.clear();
    jtv_SPECIFIC32_3.clear();

    setDstPred1stSrcZreg(TP_NONE, tv_SPECIFIC32, jtv_SPECIFIC32, "b", tv_ZREG);
    setDstPred1stSrcZreg(TP_NONE, tv_SPECIFIC32_1, jtv_SPECIFIC32_1, "h", tv_ZREG);
    setDstPred1stSrcZreg(TP_NONE, tv_SPECIFIC32_2, jtv_SPECIFIC32_2, "s", tv_ZREG);
    setDstPred1stSrcZreg(TP_NONE, tv_SPECIFIC32_3, jtv_SPECIFIC32_3, "d", tv_ZREG);

    tv_SPECIFIC64.clear();
    tv_SPECIFIC64_1.clear();
    tv_SPECIFIC64_2.clear();
    tv_SPECIFIC64_3.clear();
    jtv_SPECIFIC64.clear();
    jtv_SPECIFIC64_1.clear();
    jtv_SPECIFIC64_2.clear();
    jtv_SPECIFIC64_3.clear();

    setDstPred1stSrcZreg(TP_NONE, tv_SPECIFIC64, jtv_SPECIFIC64, "", tv_ZREG_B);
    setDstPred1stSrcZreg(TP_NONE, tv_SPECIFIC64_1, jtv_SPECIFIC64_1, "", tv_ZREG_H);
    setDstPred1stSrcZreg(TP_NONE, tv_SPECIFIC64_2, jtv_SPECIFIC64_2, "", tv_ZREG_S);
    setDstPred1stSrcZreg(TP_NONE, tv_SPECIFIC64_3, jtv_SPECIFIC64_3, "", tv_ZREG_D);

    tv_SPECIFIC32_4.clear();
    tv_SPECIFIC64_4.clear();
    jtv_SPECIFIC32_4.clear();
    jtv_SPECIFIC64_4.clear();

    setDstPred1stSrcZreg(TP_NONE, tv_SPECIFIC32_4, jtv_SPECIFIC32_4, "", tv_WREG);
    setDstPred1stSrcZreg(TP_NONE, tv_SPECIFIC64_4, jtv_SPECIFIC64_4, "", tv_XREG);
    
    putSVE_extract_element_to_general_register();
    putSVE_copy_SIMD_FP_scalar_register_to_vector_predicated();
    putSVE_extract_element_to_SIMD_FP_scalar_register();
    putSVE_reverse_within_elements();
    putSVE_conditionally_broadcast_element_to_vector();
    putSVE_copy_general_register_to_vector_predicated();
    putSVE_conditionally_extract_element_to_SIMD_FP_scalar();
    putSVE_vector_splice_destructive();
    putSVE_conditionally_extract_element_to_general_register();
    putSVE_compress_active_elements();
  }    

  void putSVE_VectorSelect() {
    putSVE_select_vector_elements_predicated();
  }

  void putSVE_IntegerCompareVectors() {
    putSVE_integer_compare_vectors();
    putSVE_integer_compare_with_wide_elements();
  }

  void  putSVE_IntegerCompareUnsignedImmediate() {
    putSVE_integer_compare_with_unsigned_immediate();
  }

  void putSVE_PredicateLogicalOperations() {
    putSVE_predicate_logical_operations();
  }

  void putSVE_PropagateBreak() {
    putSVE_propagate_break_from_previous_partition();
  }

  void putSVE_PartitionBreak() {
    tv_SPECIFIC32.clear();
    jtv_SPECIFIC32.clear();

    /** Dst and 2nd src register rotation */
    for(std::string i : tv_PREG_B) {
      std::string tmp = i + ", " + tv_PG_Z[0] + ", " + tv_PREG_B[0] + ", " + i;
      std::string jtmp =i + ", " + jtv_PG_Z[0] + ", " + tv_PREG_B[0];
      tv_SPECIFIC32.push_back(tmp);
      jtv_SPECIFIC32.push_back(jtmp);
    }

    /** PG_Z rotation */
    for(std::string i : tv_PG_Z) {
      std::string tmp = tv_PREG_B[0] + ", " + i + ", " + tv_PREG_B[0] + ", " + tv_PREG_B[0];
      tv_SPECIFIC32.push_back(tmp);
    }
    for(std::string i : jtv_PG_Z) {
      std::string jtmp= tv_PREG_B[0] + ", " + i + ", " + tv_PREG_B[0];
      jtv_SPECIFIC32.push_back(jtmp);
    }

    /** 1st src register rotation */
    for(std::string i : tv_PREG_B) {
      std::string tmp = tv_PREG_B[0] + ", " + tv_PG_Z[0] + ", " +  i + ", " + tv_PREG_B[0];
      std::string jtmp= tv_PREG_B[0] + ", " + jtv_PG_Z[0] + ", " +  i;
      tv_SPECIFIC32.push_back(tmp);
      jtv_SPECIFIC32.push_back(jtmp);
    }
										      
    putSVE_partition_break_condition();
    putSVE_propagate_break_to_next_partition();
  }

  void putSVE_PredicateMisc() {
    tv_SPECIFIC32.clear();
    jtv_SPECIFIC32.clear();

    setDstPred1stSrcZreg(TP_NONE, tv_SPECIFIC32, jtv_SPECIFIC32, "", tv_PREG_B);
    setDstPred1stSrcZreg(TP_NONE, tv_SPECIFIC32_1, jtv_SPECIFIC32_1, "", tv_PREG_H);
    setDstPred1stSrcZreg(TP_NONE, tv_SPECIFIC32_2, jtv_SPECIFIC32_2, "", tv_PREG_S);
    setDstPred1stSrcZreg(TP_NONE, tv_SPECIFIC32_3, jtv_SPECIFIC32_3, "", tv_PREG_D);

    putSVE_predicate_test();
    putSVE_predicate_initialize();
    putSVE_predicate_first_active();
    putSVE_predicate_zero();
    putSVE_predicate_read_from_FFR_predicated();
    putSVE_predicate_next_active();
    putSVE_predicate_read_from_FFR_unpredicated();
  }

  void putSVE_IntegerCompareSignedImmediate() {
    std::vector<int> tmp = { -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 15 };

    tv_SPECIFIC32.clear();
    jtv_SPECIFIC32.clear();

    for(int i : tmp) {
      tv_SPECIFIC32.push_back(std::to_string(i));
      jtv_SPECIFIC32.push_back(std::to_string(i));
    }      

    putSVE_integer_compare_with_signed_immediate();
  }

  void putSVE_PredicateCount() {
    putSVE_predicate_count();
  }

  void putSVE_IncDecbyPredicateCount() {
    std::vector<int> tmpIdx = { 15, 0, 1, 2, 4, 8, 16, 3 };
    tv_SPECIFIC32.clear();
    tv_SPECIFIC32_1.clear();
    tv_SPECIFIC32_2.clear();
    tv_SPECIFIC32_3.clear();
    jtv_SPECIFIC32.clear();
    jtv_SPECIFIC32_1.clear();
    jtv_SPECIFIC32_2.clear();
    jtv_SPECIFIC32_3.clear();

    /** Dst register rotation */
    for(int i : tmpIdx) {
      tv_SPECIFIC32.push_back   ("x" + std::to_string(i) + ", " + tv_PREG_B[0] + ", w" + std::to_string(i));
      jtv_SPECIFIC32.push_back  ("w" + std::to_string(i) + ", " + tv_PREG_B[0]);
      tv_SPECIFIC32_1.push_back ("x" + std::to_string(i) + ", " + tv_PREG_H[0] + ", w" + std::to_string(i));
      jtv_SPECIFIC32_1.push_back("w" + std::to_string(i) + ", " + tv_PREG_H[0]);
      tv_SPECIFIC32_2.push_back ("x" + std::to_string(i) + ", " + tv_PREG_S[0] + ", w" + std::to_string(i));
      jtv_SPECIFIC32_2.push_back("w" + std::to_string(i) + ", " + tv_PREG_S[0]);
      tv_SPECIFIC32_3.push_back ("x" + std::to_string(i) + ", " + tv_PREG_D[0] + ", w" + std::to_string(i));
      jtv_SPECIFIC32_3.push_back("w" + std::to_string(i) + ", " + tv_PREG_D[0]);
    }    
    /** Predicate register rotation */
    for(size_t i=0; i<tv_PREG_B.size(); i++) {
      tv_SPECIFIC32.push_back   ("x" + std::to_string(tmpIdx[0]) + ", " + tv_PREG_B[i] + ", " "w" + std::to_string(tmpIdx[0]));
      jtv_SPECIFIC32.push_back  ("w" + std::to_string(tmpIdx[0]) + ", " + tv_PREG_B[i]);
      tv_SPECIFIC32_1.push_back ("x" + std::to_string(tmpIdx[0]) + ", " + tv_PREG_H[i] + ", " "w" + std::to_string(tmpIdx[0]));
      jtv_SPECIFIC32_1.push_back("w" + std::to_string(tmpIdx[0]) + ", " + tv_PREG_H[i]);
      tv_SPECIFIC32_2.push_back ("x" + std::to_string(tmpIdx[0]) + ", " + tv_PREG_S[i] + ", " "w" + std::to_string(tmpIdx[0]));
      jtv_SPECIFIC32_2.push_back("w" + std::to_string(tmpIdx[0]) + ", " + tv_PREG_S[i]);
      tv_SPECIFIC32_3.push_back ("x" + std::to_string(tmpIdx[0]) + ", " + tv_PREG_D[i] + ", " "w" + std::to_string(tmpIdx[0]));
      jtv_SPECIFIC32_3.push_back("w" + std::to_string(tmpIdx[0]) + ", " + tv_PREG_D[i]);
    }
    
    putSVE_saturating_inc_dec_vector_by_predicate_count();
    putSVE_saturating_inc_dec_register_by_predicate_count();
    putSVE_inc_dec_vector_by_predicate_count();
    putSVE_inc_dec_register_by_predicate_count();
  }

  void putSVE_WriteFFR() {
    putSVE_FFR_write_from_predicate();
    putSVE_FFR_initialise();
  }

  void putSVE_IntegerCompareScalars() {
    putSVE_integer_compare_scalar_count_and_limit();
    putSVE_conditionally_terminate_scalars();
  }

  void putSVE_IntegerWideImmediateUnpredicated() {
    std::vector<int> tmp = { 127, -128, -64, -32, -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 16, 32, 64 };

    clearTvAndJtv();

    setDst1stSrcZreg(tv_SPECIFIC32, jtv_SPECIFIC32, "", tv_ZREG_B);
    setDst1stSrcZreg(tv_SPECIFIC32_1, jtv_SPECIFIC32_1, "", tv_ZREG_H);
    setDst1stSrcZreg(tv_SPECIFIC32_2, jtv_SPECIFIC32_2, "", tv_ZREG_S);
    setDst1stSrcZreg(tv_SPECIFIC32_3, jtv_SPECIFIC32_3, "", tv_ZREG_D);

    tv_SPECIFIC64.push_back("LSL 0");
    jtv_SPECIFIC64.push_back("LSL, 0");

    tv_SPECIFIC64_1.push_back("LSL 0");
    tv_SPECIFIC64_1.push_back("LSL 8");
    jtv_SPECIFIC64_1.push_back("LSL, 0");
    jtv_SPECIFIC64_1.push_back("LSL, 8");

    for(int i : tmp) {
      tv_SPECIFIC64_3.push_back(std::to_string(i));
      jtv_SPECIFIC64_3.push_back(std::to_string(i));
    }


    putSVE_integer_add_subtract_immediate_unpredicated();
    putSVE_integer_min_max_immediate_unpredicated();
    putSVE_integer_multiply_immediate_unpredicated();
    putSVE_broadcast_integer_immediate_unpredicated();
    putSVE_broadcast_floating_point_immediate_unpredicated();
  }

  void putSVE_IntegerMultiplyAddUnpredicated() {
    std::vector<int> tmp = { 127, -128, -64, -32, -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 16, 32, 64 };

    clearTvAndJtv();

    setDst1stSrcZreg(tv_SPECIFIC32, jtv_SPECIFIC32, "", tv_ZREG_B);
    setDst1stSrcZreg(tv_SPECIFIC32_1, jtv_SPECIFIC32, "", tv_ZREG_H);
    setDst1stSrcZreg(tv_SPECIFIC32_2, jtv_SPECIFIC32, "", tv_ZREG_S);
    setDst1stSrcZreg(tv_SPECIFIC32_3, jtv_SPECIFIC32, "", tv_ZREG_D);

    for(int i : tmp) {
      tv_SPECIFIC64.push_back(std::to_string(i));
      jtv_SPECIFIC64.push_back(std::to_string(i));
    }

    tv_SPECIFIC64_1.push_back("LSL 0");
    jtv_SPECIFIC64_1.push_back("LSL 0");

    tv_SPECIFIC64_2.push_back("LSL 0");
    tv_SPECIFIC64_2.push_back("LSL 8");
    jtv_SPECIFIC64_2.push_back("LSL, 0");
    jtv_SPECIFIC64_2.push_back("LSL, 8");
    
    putSVE_integer_dot_product_unpredicated();
  }

  void putSVE_MultiplyIndexed() {
    std::vector<std::string> tmpReg32 = { "z7", "z0", "z1", "z2", "z4" };
    std::vector<std::string> tmpReg64 = { "z7", "z0", "z1", "z2", "z4", "z8", "z15" };
    std::vector<std::string> tmpIdx32 = { "3", "0", "1", "2" };
    std::vector<std::string> tmpIdx64 = { "1", "0" };

    clearTvAndJtv();

    for(std::string j : tmpIdx32) {
      for(std::string i : tmpReg32) {
	std::string tmp = i + ".b[" + j + "]";
	tv_SPECIFIC32.push_back(tmp);
	jtv_SPECIFIC32.push_back(tmp);
      }
    }

    for(std::string j : tmpIdx64) {
      for(std::string i : tmpReg64) {
	std::string tmp = i + ".h[" + j + "]";
	tv_SPECIFIC64.push_back(tmp);
	jtv_SPECIFIC64.push_back(tmp);
      }
    }
    
    putSVE_integer_dot_product_indexed();
  }

  void putSVE_FloatingPointComplexAddition() {
    std::vector<std::string> tmpDst = { "z15", "z0", "z1", "z2", "z4", "z8", "z16", "z31" };
    tv_SPECIFIC32.clear();
    jtv_SPECIFIC32.clear();


    tv_SPECIFIC32.push_back("90");
    jtv_SPECIFIC32.push_back("90");
    tv_SPECIFIC32.push_back("270");
    jtv_SPECIFIC32.push_back("270");

    setDstPred1stSrcZreg(TP_m, tv_SPECIFIC64,   jtv_SPECIFIC64,   "h", tmpDst);
    setDstPred1stSrcZreg(TP_m, tv_SPECIFIC64_1, jtv_SPECIFIC64_1, "s", tmpDst);
    setDstPred1stSrcZreg(TP_m, tv_SPECIFIC64_2, jtv_SPECIFIC64_2, "d", tmpDst);
    
    putSVE_floating_point_complex_add_predicated();
  }

  void putSVE_FloatingPointComplexMultiplyAdd() {
    putSVE_floating_point_complex_multiply_add_predicated();
  }

  void putSVE_FloatingPointMultiplyAddIndexed() {
    clearTvAndJtv();

    std::vector<std::string> zreg0 = { "z3.", "z0.", "z1.", "z2.", "z4.", "z7." };
    std::vector<std::string> zreg1 = { "z3.", "z0.", "z1.", "z2.", "z4.", "z8.", "z15." };
    std::vector<std::string> idxH = { "h[7]", "h[0]", "h[1]", "h[2]", "h[4]" };
    std::vector<std::string> idxS = { "s[3]", "s[0]", "s[1]", "s[2]" };
    std::vector<std::string> idxD = { "d[1]", "d[0]" };
    
    for(std::string j : zreg0) {
      for(std::string i : idxH) {
	tv_SPECIFIC32.push_back(j+i);
	jtv_SPECIFIC32.push_back(j+i);
      }
    }

    for(std::string j : zreg0) {
      for(std::string i : idxS) {
	tv_SPECIFIC32_1.push_back(j+i);
	jtv_SPECIFIC32_1.push_back(j+i);
      }
    }

    for(std::string j : zreg1) {
      for(std::string i : idxD) {
	tv_SPECIFIC32_2.push_back(j+i);
	jtv_SPECIFIC32_2.push_back(j+i);
      }
    }

    putSVE_floating_point_multiply_add_indexed();
  }

  void putSVE_FloatingPointComplexMultiplyAddIndexed() {
    clearTvAndJtv();

    std::vector<std::string> zreg0 = { "z3.", "z0.", "z1.", "z2.", "z4.", "z7." };
    std::vector<std::string> zreg1 = { "z3.", "z0.", "z1.", "z2.", "z4.", "z8.", "z15." };
    std::vector<std::string> idxH = { "h[3]", "h[0]", "h[1]", "h[2]" };
    std::vector<std::string> idxS = { "s[1]", "s[0]" };
    std::vector<std::string> rot  = { "180", "0", "90", "270" }; 
    
    for(std::string j : zreg0) {
      for(std::string i : idxH) {
	tv_SPECIFIC32.push_back(j+i);
	jtv_SPECIFIC32.push_back(j+i);
      }
    }

    for(std::string j : zreg1) {
      for(std::string i : idxS) {
	tv_SPECIFIC32_1.push_back(j+i);
	jtv_SPECIFIC32_1.push_back(j+i);
      }
    }

    for(std::string i : rot) {
      tv_SPECIFIC64.push_back(i);
      jtv_SPECIFIC64.push_back(i);
    }

    putSVE_floating_point_complex_multiply_add_indexed();
  }

  void putSVE_FloatingPointMultiplyIndexed() {
    clearTvAndJtv();

    std::vector<std::string> zreg0 = { "z3.", "z0.", "z1.", "z2.", "z4.", "z7." };
    std::vector<std::string> zreg1 = { "z3.", "z0.", "z1.", "z2.", "z4.", "z8.", "z15." };
    std::vector<std::string> idxH = { "h[7]", "h[0]", "h[1]", "h[2]", "h[4]" };
    std::vector<std::string> idxS = { "s[3]", "s[0]", "s[1]", "s[2]" };
    std::vector<std::string> idxD = { "d[1]", "d[0]" };
    
    for(std::string j : zreg0) {
      for(std::string i : idxH) {
	tv_SPECIFIC32.push_back(j+i);
	jtv_SPECIFIC32.push_back(j+i);
      }
    }

    for(std::string j : zreg0) {
      for(std::string i : idxS) {
	tv_SPECIFIC32_1.push_back(j+i);
	jtv_SPECIFIC32_1.push_back(j+i);
      }
    }

    for(std::string j : zreg1) {
      for(std::string i : idxD) {
	tv_SPECIFIC32_2.push_back(j+i);
	jtv_SPECIFIC32_2.push_back(j+i);
      }
    }

    putSVE_floating_point_multiply_indexed();
  }

  void putSVE_FloatingPointFastReduction() {
    putSVE_floating_point_recursive_reduction();
  }

  void putSVE_FloatingPointUnaryOperationsUnpredicated() {
    putSVE_floating_point_reciprocal_estimate_unpredicated();
  }

  void putSVE_FloatingPointComparewithZero() {
    clearTvAndJtv();

    tv_SPECIFIC32.push_back("0.0");
    jtv_SPECIFIC32.push_back("0.0");

    putSVE_floating_point_compare_with_zero();
  }

  void putSVE_FloatingPointAccumulatingReduction() {
    clearTvAndJtv();

    for(std::string j : tv_HREG) {
      for(std::string i : tv_PG) {
	tv_SPECIFIC32.push_back(j + ", " + i + ", " + j);
	jtv_SPECIFIC32.push_back(j + ", " + i);
      }
    }

    for(std::string j : tv_SREG) {
      for(std::string i : tv_PG) {
	tv_SPECIFIC32_1.push_back(j + ", " + i + ", " + j);
	jtv_SPECIFIC32_1.push_back(j + ", " + i);
      }
    }
    
    for(std::string j : tv_DREG) {
      for(std::string i : tv_PG) {
	tv_SPECIFIC32_2.push_back(j + ", " + i + ", " + j);
	jtv_SPECIFIC32_2.push_back(j + ", " + i);
      }
    }

    putSVE_floating_point_serial_reduction_predicated();
  }

  void putSVE_FloatingPointArithmeticUnpredicated() {
    putSVE_floating_point_arithmetic_unpredicated();
  }

  void putSVE_FloatingPointArithmeticPredicated() {
    clearTvAndJtv();

    setDstPred1stSrcZreg(TP_m, tv_SPECIFIC32, jtv_SPECIFIC32, "h", tv_ZREG);
    setDstPred1stSrcZreg(TP_m, tv_SPECIFIC32_1, jtv_SPECIFIC32_1, "s", tv_ZREG);
    setDstPred1stSrcZreg(TP_m, tv_SPECIFIC32_2, jtv_SPECIFIC32_2, "d", tv_ZREG);

    for(std::string i : tv_ZREG) {
      tv_SPECIFIC64.push_back(i + ".h, " + i + ".h");
      jtv_SPECIFIC64.push_back(i + ".h");
      tv_SPECIFIC64_1.push_back(i + ".s, " + i + ".s");
      jtv_SPECIFIC64_1.push_back(i + ".s");
      tv_SPECIFIC64_2.push_back(i + ".d, " + i + ".d");
      jtv_SPECIFIC64_2.push_back(i + ".d");
    }      

    tv_SPECIFIC32_3.push_back("1.0");
    jtv_SPECIFIC32_3.push_back("1.0");
    tv_SPECIFIC32_3.push_back("0.5");
    jtv_SPECIFIC32_3.push_back("0.5");
    
    tv_SPECIFIC64_3.push_back("2.0");
    jtv_SPECIFIC64_3.push_back("2.0");
    tv_SPECIFIC64_3.push_back("0.5");
    jtv_SPECIFIC64_3.push_back("0.5");
    
    tv_SPECIFIC64_4.push_back("1.0");
    jtv_SPECIFIC64_4.push_back("1.0");
    tv_SPECIFIC64_4.push_back("0.0");
    jtv_SPECIFIC64_4.push_back("0.0");

    putSVE_floating_point_arithmetic_predicated();
    putSVE_floating_point_trig_multiply_add_coefficient();
    putSVE_floating_point_arithmetic_with_immediate_predicated();
  }

  void putSVE_FloatingPointUnaryOperationsPredicated() {
    putSVE_floating_point_round_to_integral_value();
    putSVE_floating_point_convert_precision();
    putSVE_floating_point_unary_operations();
    nputSVE_integer_convert_to_floating_point();
    putSVE_floating_point_convert_to_integer();
  }

  void putSVE_FloatingPointCompareVectors() {
    putSVE_floating_point_compare_vectors();
  }

  void putSVE_FloatingPointMultiplyAdd() {
    putSVE_floating_point_multiply_accumulate_writing_addend();
    putSVE_floating_point_multiply_accumulate_writing_multiplicand();
  }

  
  void putSVE() {
    putSVE_IntegerBinaryArithmeticPredicatedGroup();
    putSVE_IntegerReductionGroup();
    putSVE_BitwiseShiftPredicatedGroup();
    putSVE_IntegerUnaryArithmeticPredicatedGroup();
    putSVE_IntegerMultiplyAddPredicatedGroup();
    putSVE_IntegerArithmeticUnpredicated();
    putSVE_BitwiseLogicalUnpredicated(); 
    putSVE_IndexGeneration();
    putSVE_StackAllocation();
    putSVE_BitwiseShiftUnpredicated();
    putSVE_AddressGenerationGroup();
    putSVE_IntegerMiscUnpredicated();
    putSVE_ElementCount();
    putSVE_BitwiseImmediate();
    putSVE_IntegerWideImmediatePredicated();
    putSVE_PermuteVectorExtract();
    putSVE_PermuteVectorUnpredicated();
    putSVE_PermutePredicate();
    putSVE_PermuteVectorInterleaving();
    putSVE_PermuteVectorPredicated();
    putSVE_VectorSelect();
    putSVE_IntegerCompareVectors();
    putSVE_IntegerCompareUnsignedImmediate();
    putSVE_PredicateLogicalOperations();
    putSVE_PropagateBreak();
    putSVE_PartitionBreak();
    putSVE_PredicateMisc();
    putSVE_IntegerCompareSignedImmediate();
    putSVE_PredicateCount();
    putSVE_IncDecbyPredicateCount();
    putSVE_WriteFFR();
    putSVE_IntegerCompareScalars();
    putSVE_IntegerWideImmediateUnpredicated();
    putSVE_IntegerMultiplyAddUnpredicated();
    putSVE_MultiplyIndexed();

    putSVE_FloatingPointComplexAddition();
    putSVE_FloatingPointMultiplyAddIndexed();
    putSVE_FloatingPointComplexMultiplyAddIndexed();
    putSVE_FloatingPointMultiplyIndexed();
    putSVE_FloatingPointFastReduction();
    putSVE_FloatingPointUnaryOperationsUnpredicated();
    putSVE_FloatingPointComparewithZero();
    putSVE_FloatingPointAccumulatingReduction();
    putSVE_FloatingPointArithmeticUnpredicated();
    putSVE_FloatingPointArithmeticPredicated();
    putSVE_FloatingPointUnaryOperationsPredicated();
    putSVE_FloatingPointCompareVectors(); 
    putSVE_FloatingPointMultiplyAdd();
  }
  
  void put()
  {
    putSVE();
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
