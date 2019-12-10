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

enum AddressingType {
		     TP_PreImm,  /** pre-indexded immediate */
		     TP_PostImm, /** post-indexed immediate */
		     TP_PostReg, /** Post-indexed register */
		     TP_None };

uint64_t flagBit = 0;
const uint64_t WREG  = flagBit++; /** Test vector is {w0, w1, ..., w30 } */
const uint64_t WREG3 = flagBit++;
const uint64_t XREG  = flagBit++; /** Test vector is {x0, x1, ..., x30 } */
const uint64_t XREG2 = flagBit++; /** Test vector is {x0, x1, ..., x30 } */
const uint64_t XREG3 = flagBit++;
const uint64_t WSP   = flagBit++; /** Test vector is {wsp} */
const uint64_t XSP   = flagBit++; /** Test vector is {sp} */
const uint64_t XNSP  = flagBit++;
const uint64_t XNSP2 = flagBit++;
const uint64_t XNSP3 = flagBit++;
const uint64_t IMM4BIT   = flagBit++; /** Test vector is {0, 1, ..., 8, 15 } */
const uint64_t IMM5BIT   = flagBit++; /** Test vector is {0, 1, ..., 16, 31 } */
const uint64_t IMM6BIT   = flagBit++; /** Test vector is {0, 1, ..., 32, 63 } */
const uint64_t IMM12BIT  = flagBit++; /** Test vector is {0, 1, ..., 2048, 4095 } */
const uint64_t IMM13BIT  = flagBit++; /** Test vector is {0, 1, ..., 4096, 8191 } */
const uint64_t IMM16BIT  = flagBit++; /** Test vector is {0, 1, ..., 4096, 1<<13, 1<<14, 1<<15, 1<<16-1 } */
const uint64_t IMM9BIT_PM = flagBit++; /** Test vector is {-256, -255, ..., 255 } */
const uint64_t IMM7BIT_MUL4 = flagBit++; /** Test vector is { 0, 4, 8, ..., 127*4 } */
const uint64_t IMM7BIT_MUL8 = flagBit++; /** Test vector is { 0, 8, 16, ..., 127*8 } */
const uint64_t IMM7BIT_MUL16 = flagBit++; /** Test vector is { 0, 16, 32, ..., 127*16 } */
const uint64_t IMM12BIT_MUL2 = flagBit++; /** Test vector is {0, 4, 8, ..., 8190 } */
const uint64_t IMM12BIT_MUL4 = flagBit++; /** Test vector is {0, 4, 8, ..., 16380 } */
const uint64_t IMM12BIT_MUL8 = flagBit++; /** Test vector is {0, 8, 16, ..., 32760 } */
const uint64_t IMM19BIT_MUL4 = flagBit++; /** Test vector is {-2^19*4, ...., (2^19-1)*4 } */
const uint64_t COND      = flagBit++; /** Test vector is { EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, AL } */
const uint64_t COND_WO_AL = flagBit++; /** Test vector is { EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE } */
const uint64_t NZCV       = flagBit++; /** Test vector is { 0, 1, ..., 15 } */
const uint64_t BITMASK32  = flagBit++; /** Test vector is {0, 1, ..., 2048, 4095 } */
const uint64_t BITMASK64  = flagBit++; /** Test vector is {0, 1, ..., 4096, 8191 } */
const uint64_t LSL_IMM    = flagBit++; /** Test vector is generated on the fly. */
const uint64_t LSL32 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t LSL64 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_1 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_1 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_2 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_2 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC32_3 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC64_3 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC0 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC1 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC2 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC3 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC4 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC5 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC6 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC7 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC8 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC9 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC10 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SPECIFIC11 = flagBit++; /** Test vector is generated on the fly. */
const uint64_t IMM_0_OR_1   = flagBit++; /** Test vector is { "0", "1" }; */
const uint64_t IMM_0_OR_2   = flagBit++; /** Test vector is { "0", "2" }; */
const uint64_t SHIFT_AMOUNT32    = flagBit++; /** Test vector is generated on the fly. */
const uint64_t SHIFT_AMOUNT64    = flagBit++; /** Test vector is generated on the fly. */
const uint64_t EXT_AMOUNT32    = flagBit++; /** Test vector is generated on the fly. */
const uint64_t EXT_AMOUNT64    = flagBit++; /** Test vector is generated on the fly. */

const uint64_t PTR_O    = flagBit++;
const uint64_t PTR_C    = flagBit++;

const uint64_t BRA_O    = flagBit++; /** Test vector is { "{" } */
const uint64_t BRA_C    = flagBit++; /** Test vector is { "}" } */

const uint64_t PAR_O    = flagBit++; /** Test vector is { "(" } */
const uint64_t PAR_C    = flagBit++; /** Test vector is { ">" } */

//const uint64_t POST_PTR  = flagBit++;


const uint64_t T_LSL      = flagBit++; /** Test vector is { "LSL" } */
const uint64_t T_UXTW     = flagBit++; /** Test vector is { "UXTW", "UXT" } */
const uint64_t T_SXTW     = flagBit++; /** Test vector is { "SXTW", "SXT" } */
const uint64_t T_SXTX     = flagBit++; /** Test vector is { "SXTX", "SXT" } */



const uint64_t NOPARA = 100000;


#define PUT0(name, nm)				\
  void put##name() const			\
  {						\
    std::vector<std::string> nemonic(nm);	\
    put(nemonic, #name, 0);			\
  }						\

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
  std::vector<std::string> tv_WREG3 = { "w6", "w16", "w28"};
  std::vector<std::string> tv_XREG = { "x7", "x0", "x1", "x2", "x4", "x8", "x16", "x30"};
  std::vector<std::string> tv_XREG2 = { "x3", "x0", "x1", "x2", "x4", "x8", "x16", "x30"};
  std::vector<std::string> tv_XREG3 = { "x5", "x15", "x29"};
  std::vector<std::string> tv_WSP  = { "wsp" };
  std::vector<std::string> tv_XSP  = { "sp" };
  std::vector<std::string> tv_XNSP  = { "x7", "x0", "x1", "x2", "x4", "x8", "x16", "x30", "sp"};
  std::vector<std::string> tv_XNSP2 = { "x3", "x0", "x1", "x2", "x4", "x8", "x16", "x30", "sp"};
  std::vector<std::string> tv_XNSP3 = { "x5", "x15", "x29"};
  std::vector<std::string> tv_IMM4BIT = { "7", "0", "1", "2", "4", "8", "15" };
  std::vector<std::string> tv_IMM5BIT = { "0x1e", "0", "1", "2", "4", "8", "16", "31" };
  std::vector<std::string> tv_IMM6BIT = { "0x39", "0", "1", "2", "4", "8", "16", "32", "63" };
  std::vector<std::string> tv_IMM12BIT = { "0x2aa", "0", "1", "2", "4", "8", "16", "32", "64",
					   "128", "256", "512", "1024", "2048", "4095" };
  std::vector<std::string> tv_IMM13BIT = { "0x1999", "0", "1", "2", "4", "8", "16", "32", "64",
					   "128", "256", "512", "1024", "2048", "4096", "8191"};
  std::vector<std::string> tv_IMM16BIT = { "0xe38e", "0", "1", "2", "4", "8", "16", "32", "64",
					   "128", "256", "512", "1024", "2048", "4096", "8191",
					   "1<<14", "1<<15", "(1<<16)-1" };
  std::vector<std::string> tv_IMM9BIT_PM = { "127", "-256", "-128", "-64", "-32", "-16", "-8", "-4", "-2", "-1",
					     "0", "1", "2", "4", "8", "16", "32", "64", "128", "255" };
  std::vector<std::string> tv_IMM7BIT_MUL4 , tv_IMM7BIT_MUL8, tv_IMM7BIT_MUL16;
  std::vector<std::string> tv_IMM12BIT_MUL2, tv_IMM12BIT_MUL4, tv_IMM12BIT_MUL8;
  std::vector<std::string> tv_IMM19BIT_MUL4;
  std::vector<std::string> tv_COND       = { "EQ", "NE", "CS", "CC", "MI", "PL", "VS", "VC", "HI", "LS", "GE", "LT", "GT", "LE", "AL" };
  std::vector<std::string> tv_COND_WO_AL = { "EQ", "NE", "CS", "CC", "MI", "PL", "VS", "VC", "HI", "LS", "GE", "LT", "GT", "LE" };
  std::vector<std::string> tv_NZCV     = { "15", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14" };
  std::vector<std::string> tv_BITMASK32;
  std::vector<std::string> tv_BITMASK64;
  std::vector<std::string> tv_LSL_IMM, jtv_LSL_IMM;
  std::vector<std::string> tv_LSL32, jtv_LSL32;
  std::vector<std::string> tv_LSL64, jtv_LSL64;
  std::vector<std::string> tv_SPECIFIC32, tv_SPECIFIC64, tv_SPECIFIC32_1, tv_SPECIFIC64_1, tv_SPECIFIC32_2, tv_SPECIFIC64_2,
    tv_SPECIFIC32_3, tv_SPECIFIC64_3;
  std::vector<std::string> jtv_SPECIFIC32, jtv_SPECIFIC64, jtv_SPECIFIC32_1, jtv_SPECIFIC64_1, jtv_SPECIFIC32_2, jtv_SPECIFIC64_2,
    jtv_SPECIFIC32_3, jtv_SPECIFIC64_3;

  std::vector<std::string> tv_SPECIFIC0, tv_SPECIFIC1, tv_SPECIFIC2, tv_SPECIFIC3,
    tv_SPECIFIC4, tv_SPECIFIC5, tv_SPECIFIC6, tv_SPECIFIC7,
    tv_SPECIFIC8, tv_SPECIFIC9, tv_SPECIFIC10, tv_SPECIFIC11;
  std::vector<std::string> jtv_SPECIFIC0, jtv_SPECIFIC1, jtv_SPECIFIC2, jtv_SPECIFIC3,
    jtv_SPECIFIC4, jtv_SPECIFIC5, jtv_SPECIFIC6, jtv_SPECIFIC7,
    jtv_SPECIFIC8, jtv_SPECIFIC9, jtv_SPECIFIC10, jtv_SPECIFIC11;
  
  std::vector<std::string> tv_IMM_0_OR_1 = { "1", "0" };
  std::vector<std::string> tv_IMM_0_OR_2 = { "2", "0" };

  std::vector<std::string> tv_SHIFT_AMOUNT32, jtv_SHIFT_AMOUNT32;
  std::vector<std::string> tv_SHIFT_AMOUNT64, jtv_SHIFT_AMOUNT64;
  std::vector<std::string> tv_EXT_AMOUNT32, jtv_EXT_AMOUNT32;
  std::vector<std::string> tv_EXT_AMOUNT64, jtv_EXT_AMOUNT64;

  std::vector<std::string> tv_PTR_O = { "[" };
  std::vector<std::string> tv_PTR_C = { "]" };
  std::vector<std::string> jtv_PTR_O = { "ptr(" };
  std::vector<std::string> jtv_PTR_C = { ")" };

  std::vector<std::string> tv_BRA_O = { "{" };
  std::vector<std::string> tv_BRA_C = { "}" };
  std::vector<std::string> jtv_BRA_O = { "" };
  std::vector<std::string> jtv_BRA_C = { "" };

  std::vector<std::string> tv_PAR_O = { "(" };
  std::vector<std::string> tv_PAR_C = { ")" };
  std::vector<std::string> jtv_PAR_O = { "" };
  std::vector<std::string> jtv_PAR_C = { "" };
  

  std::vector<std::string> tv_LSL = { "LSL" };

  std::vector<std::string> tv_UXTW = { "UXTW", "UXTW" };
  std::vector<std::string> tv_SXTW = { "SXTW", "SXTW" };
  std::vector<std::string> tv_SXTX = { "SXTX", "SXTX" };
  std::vector<std::string> jtv_UXTW = { "UXT", "UXTW" };
  std::vector<std::string> jtv_SXTW = { "SXT", "SXTW" };
  std::vector<std::string> jtv_SXTX = { "SXT", "SXTX" };

  

  
  std::vector<std::vector<std::string> *> tv_VectorsAs = { &tv_WREG, &tv_WREG3, &tv_XREG, &tv_XREG2, &tv_XREG3, &tv_WSP, &tv_XSP,
							   &tv_XNSP, &tv_XNSP2, &tv_XNSP3,
							   &tv_IMM4BIT, &tv_IMM5BIT, &tv_IMM6BIT,
							   &tv_IMM12BIT, &tv_IMM13BIT,
							   &tv_IMM16BIT, &tv_IMM9BIT_PM,
							   &tv_IMM7BIT_MUL4 , &tv_IMM7BIT_MUL8, &tv_IMM7BIT_MUL16,
							   &tv_IMM12BIT_MUL2, &tv_IMM12BIT_MUL4, &tv_IMM12BIT_MUL8, &tv_IMM19BIT_MUL4,
							   &tv_COND, &tv_COND_WO_AL, &tv_NZCV,
							   &tv_BITMASK32, &tv_BITMASK64,
							   &tv_LSL_IMM, &tv_LSL32, &tv_LSL64,
							   &tv_SPECIFIC32, &tv_SPECIFIC64, &tv_SPECIFIC32_1, &tv_SPECIFIC64_1,
							   &tv_SPECIFIC32_2, &tv_SPECIFIC64_2, &tv_SPECIFIC32_3, &tv_SPECIFIC64_3,
							   &tv_SPECIFIC0, &tv_SPECIFIC1, &tv_SPECIFIC2, &tv_SPECIFIC3,
							   &tv_SPECIFIC4, &tv_SPECIFIC5, &tv_SPECIFIC6, &tv_SPECIFIC7,
							   &tv_SPECIFIC8, &tv_SPECIFIC9, &tv_SPECIFIC10, &tv_SPECIFIC11,
							   &tv_IMM_0_OR_1, &tv_IMM_0_OR_2,
							   &tv_SHIFT_AMOUNT32, &tv_SHIFT_AMOUNT64,
							   &tv_EXT_AMOUNT32, &tv_EXT_AMOUNT64,
							   &tv_PTR_O, &tv_PTR_C, &tv_BRA_O, &tv_BRA_C,
							   &tv_PAR_O, &tv_PAR_C,
							   &tv_LSL,
							   &tv_UXTW, &tv_SXTW, &tv_SXTX };
  std::vector<std::vector<std::string> *> tv_VectorsJit = { &tv_WREG, &tv_WREG3, &tv_XREG, &tv_XREG2, &tv_XREG3, &tv_WSP, &tv_XSP,
							    &tv_XNSP, &tv_XNSP2, &tv_XNSP3,
							    &tv_IMM4BIT, &tv_IMM5BIT, &tv_IMM6BIT,
							    &tv_IMM12BIT, &tv_IMM13BIT,
							    &tv_IMM16BIT, &tv_IMM9BIT_PM,
							    &tv_IMM7BIT_MUL4 , &tv_IMM7BIT_MUL8, &tv_IMM7BIT_MUL16,
							    &tv_IMM12BIT_MUL2, &tv_IMM12BIT_MUL4, &tv_IMM12BIT_MUL8, &tv_IMM19BIT_MUL4,
							    &tv_COND, &tv_COND_WO_AL, &tv_NZCV,
							    &tv_BITMASK32, &tv_BITMASK64, &jtv_LSL_IMM, &jtv_LSL32, &jtv_LSL64,
							    &jtv_SPECIFIC32, &jtv_SPECIFIC64, &jtv_SPECIFIC32_1, &jtv_SPECIFIC64_1,
							    &jtv_SPECIFIC32_2, &jtv_SPECIFIC64_2, &jtv_SPECIFIC32_3, &jtv_SPECIFIC64_3,
							    &jtv_SPECIFIC0, &jtv_SPECIFIC1, &jtv_SPECIFIC2, &jtv_SPECIFIC3,
							    &jtv_SPECIFIC4, &jtv_SPECIFIC5, &jtv_SPECIFIC6, &jtv_SPECIFIC7,
							    &jtv_SPECIFIC8, &jtv_SPECIFIC9, &jtv_SPECIFIC10, &jtv_SPECIFIC11,
							    &tv_IMM_0_OR_1, &tv_IMM_0_OR_2,
							    &jtv_SHIFT_AMOUNT32, &jtv_SHIFT_AMOUNT64,
							    &jtv_EXT_AMOUNT32, &jtv_EXT_AMOUNT64,
							    &jtv_PTR_O, &jtv_PTR_C, &jtv_BRA_O, &jtv_BRA_C,
							    &jtv_PAR_O, &jtv_PAR_C,
							    &tv_LSL,
							    &jtv_UXTW, &jtv_SXTW, &jtv_SXTX };
  std::vector<std::vector<std::string> *>& tv_Vectors = tv_VectorsAs;

  bool isRequireComma(int num_ops, int idx, std::string curr, std::string next) const {
    if(idx == num_ops-1) { /** Last operand isn't followed by ",". */
      return false;
    }

    if(curr == "" || curr == "[" || curr == "ptr(" || curr == "{" || curr == "(") {
      return false;
    }

    if(next == "]" || next == ")" || next == "}") {
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

  void initTv() {
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

    initTvIMM12BIT();
    initTvIMM7BIT();
    initTvImm19BIT();
  }    

  void initTvIMM12BIT() {
    std::vector<int> tmp= { 1023, 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4095 };
    tv_IMM12BIT_MUL2.clear();
    tv_IMM12BIT_MUL4.clear();
    tv_IMM12BIT_MUL8.clear();

    for(int i : tmp) {
      tv_IMM12BIT_MUL2.push_back(std::to_string(i*2));
      tv_IMM12BIT_MUL4.push_back(std::to_string(i*4));
      tv_IMM12BIT_MUL8.push_back(std::to_string(i*8));
    }
  }    

  void initTvIMM7BIT() {
    std::vector<int> tmp = { 127, 0, 1, 2, 4, 8, 16, 32, 64, 128, 255 };
    tv_IMM7BIT_MUL4.clear();
    tv_IMM7BIT_MUL8.clear();
    tv_IMM7BIT_MUL16.clear();

    for(int i : tmp) {
      tv_IMM7BIT_MUL4.push_back(std::to_string(i*4));
      tv_IMM7BIT_MUL8.push_back(std::to_string(i*8));
      tv_IMM7BIT_MUL16.push_back(std::to_string(i*16));
    }
  }

  void initTvImm19BIT() {
    tv_IMM19BIT_MUL4.clear();

    tv_IMM19BIT_MUL4.push_back("4*((1<<16)-1)");

    for(int i=18; i>=0; i--) {
      tv_IMM19BIT_MUL4.push_back("-4*(1<<" + std::to_string(i) + ")");
    }

    tv_IMM19BIT_MUL4.push_back("0");

    for(int i=0; i<18; i++) {
      tv_IMM19BIT_MUL4.push_back("4*(1<<" + std::to_string(i) + ")");
    }

    tv_IMM19BIT_MUL4.push_back("4*((1<<18)-1)");

    /** Debug
	for(std::string i : tv_IMM19BIT_MUL4) {
	std::cout << "IMM19BIT_MUL4=" << i << std::endl;
	}
    */
  }
  
  void setTvAddressing(std::vector<std::string>& tv, std::vector<std::string>& jtv,
		       std::vector<std::string>& base, std::vector<std::string>& imm,
		       AddressingType type) {
    std::string prefix = "[";
    std::string jprefix = "ptr(";
    std::string suffix = "]";
    std::string jsuffix = ")";

    tv.clear();
    jtv.clear();

    switch(type) {
    case TP_PreImm:  /** pre-indexded immediate */
      prefix = "[";
      suffix = "]!";
      jprefix = "pre_ptr(";
      jsuffix = ")";
      break;

    case TP_PostImm: /** post-indexed immediate */
      prefix = "[";
      suffix = "]";
      jprefix = "post_ptr(";
      jsuffix = ")";
      break;

    case TP_PostReg: /** Post-indexed register */
      prefix = "[";
      suffix = "]";
      jprefix = "post_ptr(";
      jsuffix = ")";
      break;

    case TP_None:
      prefix = "[";
      suffix = "]";
      jprefix = "ptr(";
      jsuffix = ")";
      break;
      
    default:
      std::cerr << __FILE__ << ":" << __LINE__ << "Invalid addresing type=" << type << std::endl;
      assert(NULL);
      break;
    }

    /** base rotation */
    for(std::string i : base) {
      if(type == TP_PreImm || type == TP_None) {
	tv.push_back(prefix + i + ", " + imm[0] + suffix);
      } else {
	tv.push_back(prefix + i + suffix + ", "  + imm[0]);
      }

      jtv.push_back(jprefix + i + ", " + imm[0] + jsuffix);
    }

    /** imm rotation */
    for(std::string i : imm) {
      if(type == TP_PreImm || type == TP_None) {
	tv.push_back(prefix + base[0] + ", " + i + suffix);
      } else {
	tv.push_back(prefix + base[0] + suffix + ", "  + i);
      }

      jtv.push_back(jprefix + base[0] + ", " + i + jsuffix);
    }
  }
  
  void putDataProcImm_PcRelAddressing() const
  {
    /*    static const char tbl[][16] = {
	  "adr",
	  "adrp",
	  };
	  for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
	  const char *p = tbl[i];
	  put(p, */
    printf("Under construction!\n");
  }


  void clearTvAndJtv() {
    tv_SPECIFIC32.clear();
    tv_SPECIFIC32_1.clear();
    tv_SPECIFIC32_2.clear();
    tv_SPECIFIC32_3.clear();
    //    tv_SPECIFIC32_4.clear();
    //    tv_SPECIFIC32_5.clear();
    tv_SPECIFIC64.clear();
    tv_SPECIFIC64.clear();
    tv_SPECIFIC64_1.clear();
    tv_SPECIFIC64_2.clear();
    tv_SPECIFIC64_3.clear();
    //    tv_SPECIFIC64_4.clear();
    //    tv_SPECIFIC64_5.clear();
    
    jtv_SPECIFIC32.clear();
    jtv_SPECIFIC32_1.clear();
    jtv_SPECIFIC32_2.clear();
    jtv_SPECIFIC32_3.clear();
    //    jtv_SPECIFIC32_4.clear();
    //    jtv_SPECIFIC32_5.clear();
    jtv_SPECIFIC64.clear();
    jtv_SPECIFIC64.clear();
    jtv_SPECIFIC64_1.clear();
    jtv_SPECIFIC64_2.clear();
    jtv_SPECIFIC64_3.clear();
    //    jtv_SPECIFIC64_4.clear();
    //    jtv_SPECIFIC64_5.clear();


    tv_SPECIFIC0.clear();
    tv_SPECIFIC1.clear();
    tv_SPECIFIC2.clear();
    tv_SPECIFIC3.clear();
    tv_SPECIFIC4.clear();
    tv_SPECIFIC5.clear();
    tv_SPECIFIC6.clear();
    tv_SPECIFIC7.clear();
    tv_SPECIFIC8.clear();
    tv_SPECIFIC9.clear();
    tv_SPECIFIC10.clear();
    tv_SPECIFIC11.clear();
    
    jtv_SPECIFIC0.clear();
    jtv_SPECIFIC1.clear();
    jtv_SPECIFIC2.clear();
    jtv_SPECIFIC3.clear();
    jtv_SPECIFIC4.clear();
    jtv_SPECIFIC5.clear();
    jtv_SPECIFIC6.clear();
    jtv_SPECIFIC7.clear();
    jtv_SPECIFIC8.clear();
    jtv_SPECIFIC9.clear();
    jtv_SPECIFIC10.clear();
    jtv_SPECIFIC11.clear();
  }
  
public:
  Test(bool isXbyak)
    : isXbyak_(isXbyak)
    , funcNum_(1)
  {

    initTv();
    
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


  /** Load/Store register. */
  /*** Load register (register offset) LDR (register) on page C6-891 */
  /*** Store register (register offset) STR (register) on page C6-1137 */
  PUT6(Load_Store_register0,
       NM("ldr", "str"),
       // size == 10
       //// option<0> is set to 0 -> UXTW or SXTW
       ////// UXTW, amount and SXTW, amount
       OPS(WREG, PTR_O, XNSP, WREG, SPECIFIC0, PTR_C),
       //// option<0> is set to 1 -> Xm
       ////// LSL
       OPS(WREG, PTR_O, XNSP, XREG, PTR_C),
       ////// SXTX, amount
       OPS(WREG, PTR_O, XNSP, XREG, SPECIFIC1, PTR_C),
       // size == 11
       //// option<0> is set to 0 -> UXTW or SXTW
       ////// UXTW, amount and SXTW, amount
       OPS(XREG, PTR_O, XNSP, WREG, SPECIFIC7, PTR_C),
       //// option<0> is set to 1
       ////// LSL
       OPS(XREG, PTR_O, XNSP, XREG, PTR_C),
       ////// SXTX, amount
       OPS(XREG, PTR_O, XNSP, XREG, SPECIFIC6, PTR_C));
  /*** Load register (immediate offset) LDR (immediate) on page C6-886 */
  /*** Store register (immediate offset) STR (immediate) on page C6-1134 */
  PUT8(Load_Store_register1,
       NM("ldr"),
       // Post-index
       OPS(WREG, SPECIFIC32),
       OPS(XREG, SPECIFIC64),
       // Pre-index
       OPS(WREG, SPECIFIC32_1),
       OPS(XREG, SPECIFIC64_1),
       // Unsigned offset
       OPS(WREG, PTR_O, XNSP2, PTR_C), 
       OPS(XREG, PTR_O, XNSP2, PTR_C), 
       OPS(WREG, PTR_O, XNSP2, IMM12BIT_MUL4, PTR_C),
       OPS(XREG, PTR_O, XNSP2, IMM12BIT_MUL8, PTR_C));
  /*** Load register (PC-relative literal) LDR (literal) on page C6-889 */
  PUT2(Load_Store_register2,
       NM("ldr"),
       OPS(WREG, IMM19BIT_MUL4),
       OPS(XREG, IMM19BIT_MUL4));
  /*** Load byte (register offset) LDRB (register) on page C6-897 */
  /*** Store byte (register offset) STRB (register) on page C6-1141 */
  PUT8(Load_Store_register3,
       NM("ldrb", "strb"),
       // option<0> is set to 0 -> Wm
       //// option == 010
       OPS(WREG, PTR_O, XNSP2, WREG, T_UXTW, PTR_C),
       OPS(WREG, PTR_O, XNSP2, WREG, SPECIFIC8, PTR_C),
       //// option == 110
       OPS(WREG, PTR_O, XNSP2, WREG, T_SXTW, PTR_C),
       OPS(WREG, PTR_O, XNSP2, WREG, SPECIFIC9, PTR_C),
       // option == 111
       OPS(WREG, PTR_O, XNSP2, XREG, T_SXTX, PTR_C),
       OPS(WREG, PTR_O, XNSP2, XREG, SPECIFIC10, PTR_C),
       // option == 011
       OPS(WREG, PTR_O, XNSP2, XREG, PTR_C),
       OPS(WREG, PTR_O, XNSP2, XREG, SPECIFIC11, PTR_C));
  /*** Load byte (immediate offset) LDRB (immediate) on page C6-895 */
  /*** Store byte (immediate offset) STRB (immediate) on page C6-1139 */
  PUT4(Load_Store_register4,
       NM("ldrb", "strb"),
       // Post-index
       OPS(WREG, SPECIFIC32),
       // Pre-index
       OPS(WREG, SPECIFIC32_1),
       // Unsigned offset
       OPS(WREG, PTR_O, XNSP2, PTR_C), OPS(WREG, SPECIFIC64_2));
  /*** Load signed byte (immediate offset) LDRSB (immediate) on page C6-903 */
  PUT8(Load_Store_register5,
       NM("ldrsb"),
       // Post-index
       OPS(WREG, SPECIFIC32),
       OPS(XREG, SPECIFIC64),
       // Pre-index
       OPS(WREG, SPECIFIC32_1),
       OPS(XREG, SPECIFIC64_1),
       // Unsigned offset
       OPS(WREG, PTR_O, XNSP2, PTR_C), OPS(WREG, SPECIFIC32_2),
       OPS(XREG, PTR_O, XNSP2, PTR_C), OPS(XREG, SPECIFIC64_2));
  /*** Load signed byte (register offset) LDRSB (register) on page C6-906 */
  PUT8(Load_Store_register6,
       NM("ldrsb"),
       // opc == 11 && option == 010 -> Wm
       OPS(WREG, PTR_O, XNSP2, WREG, T_UXTW, PTR_C),
       OPS(WREG, PTR_O, XNSP2, WREG, SPECIFIC8, PTR_C),
       // opc == 11 && option == 110 -> Wm
       OPS(WREG, PTR_O, XNSP2, WREG, T_SXTW, PTR_C),
       OPS(WREG, PTR_O, XNSP2, WREG, SPECIFIC9, PTR_C),
       // opc == 11 && option == 111 -> Wm
       OPS(WREG, PTR_O, XNSP2, XREG, T_SXTX, PTR_C),
       OPS(WREG, PTR_O, XNSP2, XREG, SPECIFIC10, PTR_C),
       // opc == 11 && option == 011 -> Wm
       OPS(WREG, PTR_O, XNSP2, XREG, PTR_C),
       OPS(WREG, PTR_O, XNSP2, XREG, SPECIFIC11, PTR_C));
  PUT8(Load_Store_register7,
       NM("ldrsb"),
       // opc == 10 && option == 010 -> Wm
       OPS(XREG, PTR_O, XNSP2, WREG, T_UXTW, PTR_C),
       OPS(XREG, PTR_O, XNSP2, WREG, SPECIFIC8, PTR_C),
       // opc == 10 && option == 110 -> Wm
       OPS(XREG, PTR_O, XNSP2, WREG, T_SXTW, PTR_C),
       OPS(XREG, PTR_O, XNSP2, WREG, SPECIFIC9, PTR_C),
       // opc == 10 && option == 111 -> Xm
       OPS(XREG, PTR_O, XNSP2, XREG, T_SXTX, PTR_C),
       OPS(XREG, PTR_O, XNSP2, XREG, SPECIFIC10, PTR_C),
       // opc == 10 && option == 011 -> Xm
       OPS(XREG, PTR_O, XNSP2, XREG, PTR_C),
       OPS(XREG, PTR_O, XNSP2, XREG, SPECIFIC11, PTR_C));
  /*** Load signed halfword (register offset) LDRSH (register) on page C6-911 */
  PUT7(Load_Store_register8,
       NM("ldrsh"),
       // opc == 11
       //// option == 010 -> Wm
       OPS(WREG, PTR_O, XNSP2, WREG, T_UXTW, PTR_C),
       OPS(WREG, PTR_O, XNSP2, WREG, SPECIFIC2, PTR_C),
       //// option == 110 -> Wm
       OPS(WREG, PTR_O, XNSP2, WREG, T_SXTW, PTR_C),
       OPS(WREG, PTR_O, XNSP2, WREG, SPECIFIC3, PTR_C),
       //// option == 111 -> Xm
       OPS(WREG, PTR_O, XNSP2, XREG, T_SXTX, PTR_C),
       OPS(WREG, PTR_O, XNSP2, XREG, SPECIFIC5, PTR_C),
       //// option == 011 -> Xm
       OPS(WREG, PTR_O, XNSP2, XREG, PTR_C));
  PUT7(Load_Store_register9,
       NM("ldrsh"),
       // opc == 10
       //// option == 010 -> Wm
       OPS(XREG, PTR_O, XNSP2, WREG, T_UXTW, PTR_C),
       OPS(XREG, PTR_O, XNSP2, WREG, SPECIFIC2, PTR_C),
       //// option == 110 -> Wm
       OPS(XREG, PTR_O, XNSP2, WREG, T_SXTW, PTR_C),
       OPS(XREG, PTR_O, XNSP2, WREG, SPECIFIC3, PTR_C),
       //// option == 111 -> Xm
       OPS(XREG, PTR_O, XNSP2, XREG, T_SXTX, PTR_C),
       OPS(XREG, PTR_O, XNSP2, XREG, SPECIFIC5, PTR_C),
       //// option == 011 -> Xm
       OPS(XREG, PTR_O, XNSP2, XREG, PTR_C));
  /*** Load halfword (register offset) LDRH (register) on page C6-901 */
  /*** Store halfword (register offset) STRH (register) on page C6-1145 */
  PUT7(Load_Store_register10,
       NM("ldrh", "strh"),
       // option = 010
       OPS(WREG, PTR_O, XNSP2, WREG, T_UXTW, PTR_C),
       OPS(WREG, PTR_O, XNSP2, WREG, SPECIFIC2, PTR_C),
       // option = 011
       OPS(WREG, PTR_O, XNSP2, XREG, PTR_C),
       // option = 110
       OPS(WREG, PTR_O, XNSP2, WREG, T_SXTW, PTR_C),
       OPS(WREG, PTR_O, XNSP2, WREG, SPECIFIC3, PTR_C),
       // option = 111
       OPS(WREG, PTR_O, XNSP2, XREG, T_SXTX, PTR_C),
       OPS(WREG, PTR_O, XNSP2, XREG, SPECIFIC5, PTR_C));
  /*** Load signed word (register offset) LDRSW (register) on page C6-916 */
  PUT7(Load_Store_register11,
       NM("ldrsw"),
       // option = 010
       OPS(XREG, PTR_O, XNSP2, WREG, T_UXTW, PTR_C),
       OPS(XREG, PTR_O, XNSP2, WREG, SPECIFIC0, PTR_C),
       // option = 011
       OPS(XREG, PTR_O, XNSP2, XREG, PTR_C),
       // option = 110
       OPS(XREG, PTR_O, XNSP2, WREG, T_SXTW, PTR_C),
       OPS(XREG, PTR_O, XNSP2, WREG, SPECIFIC0, PTR_C),
       // option = 111
       OPS(XREG, PTR_O, XNSP2, XREG, T_SXTX, PTR_C),
       OPS(XREG, PTR_O, XNSP2, XREG, SPECIFIC1, PTR_C));
  /*** Load halfword (immediate offset) LDRH (immediate) on page C6-899 */
  /*** Store halfword (immediate offset) STRH (immediate) on page C6-1143 */
  PUT4(Load_Store_register12,
       NM("ldrh", "strh"),
       /**** Post-index */
       OPS(WREG, SPECIFIC64),
       /**** Pre-index */
       OPS(WREG, SPECIFIC64_1),
       /**** Unsigned offset */
       OPS(WREG, PTR_O, XNSP, PTR_C),
       OPS(WREG, PTR_O, XNSP, IMM12BIT_MUL2, PTR_C));
  /*** Load signed halfword (immediate offset) LDRSH (immediate) on page C6-908 */
  PUT8(Load_Store_register13,
       NM("ldrsh"),
       /** Post-index */
       OPS(WREG, SPECIFIC64),
       OPS(XREG, SPECIFIC64),
       /** Pre-index */
       OPS(WREG, SPECIFIC64_1),
       OPS(XREG, SPECIFIC64_1),
       /** Unsigned offset */
       OPS(WREG, PTR_O, XNSP2, PTR_C), OPS(WREG, PTR_O, XNSP2, IMM12BIT_MUL2, PTR_C),
       OPS(XREG, PTR_O, XNSP2, PTR_C), OPS(XREG, PTR_O, XNSP2, IMM12BIT_MUL2, PTR_C));
  /*** Load signed word (immediate offset) LDRSW (immediate) on page C6-913 */
  PUT4(Load_Store_register14,
       NM("ldrsw"),
       /**** Post-index */
       OPS(XREG, SPECIFIC32),
       /**** Pre-index */
       OPS(XREG, SPECIFIC64),
       /**** Unsigned offset */
       OPS(XREG, PTR_O, XNSP2, PTR_C),
       OPS(XREG, PTR_O, XNSP2, IMM12BIT_MUL4, PTR_C));
  /*** Load signed word (PC-relative literal) LDRSW (literal) on page C6-915 */
  PUT1(Load_Store_register15,
       NM("ldrsw"),
       OPS(XREG, IMM19BIT_MUL4));
  
  void putLoadStoreRegisterOffset_core() {
    putLoad_Store_register0();
    putLoad_Store_register1();
    putLoad_Store_register2();
    putLoad_Store_register3();
    putLoad_Store_register4();
    putLoad_Store_register5();
    putLoad_Store_register6();
    putLoad_Store_register7();
    putLoad_Store_register8();
    putLoad_Store_register9();
    putLoad_Store_register10();
    putLoad_Store_register11();
    putLoad_Store_register12();
    putLoad_Store_register13();
    putLoad_Store_register14();
    putLoad_Store_register15();
  }

  /** Load/Store register (unscaled offset) on page C3-178. */
  /*** LDUR Load register (unscaled offset) LDUR on page C6-965 */
  /*** LDURSB Load signed byte (unscaled offset) LDURSB on page C6-969 */
  /*** LDURSH Load signed halfword (unscaled offset) LDURSH on page C6-971 */
  /*** STUR Store register (unscaled offset) STUR on page C6-1183 */
  PUT4(Load_Store_register_unscaled_offset0,
       NM("ldur", "ldursb", "ldursh", "stur"),
       OPS(WREG, PTR_O, XNSP2, PTR_C),
       OPS(XREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, PTR_O, XNSP2, IMM9BIT_PM, PTR_C),
       OPS(XREG, PTR_O, XNSP2, IMM9BIT_PM, PTR_C));
  /*** LDURB Load byte (unscaled offset) LDURB on page C6-967 */
  /*** LDURH Load halfword (unscaled offset) LDURH on page C6-968 */
  /*** STURB Store byte (unscaled offset) STURB on page C6-1185 */
  /*** STURH Store halfword (unscaled offset) STURH on page C6-1186 */
  PUT2(Load_Store_register_unscaled_offset1,
       NM("ldurb", "ldurh", "sturb", "sturh"),
       OPS(WREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, PTR_O, XNSP2, IMM9BIT_PM, PTR_C));
  /*** LDURSW Load signed word (unscaled offset) LDURSW on page C6-973 */
  PUT2(Load_Store_register_unscaled_offset2,
       NM("ldursw"),
       OPS(XREG, PTR_O, XNSP2, PTR_C),
       OPS(XREG, PTR_O, XNSP2, IMM9BIT_PM, PTR_C));

  void putLoadStoreRegisterUnscaledOffset_core() {
    putLoad_Store_register_unscaled_offset0();
    putLoad_Store_register_unscaled_offset1();
    putLoad_Store_register_unscaled_offset2();
  }

  /** Load/Store Pair on page C3-179. */
  /*** LDP Load Pair LDP on page C6-880 */
  /*** STP Store Pair STP on page C6-1131 */
  PUT16(Load_Store_Pair0,
       NM("ldp", "stp"),
       /**** Post-index */
       OPS(WREG, WREG3, SPECIFIC0),
       OPS(XREG, XREG3, SPECIFIC1),
       OPS(WREG3,WREG,  SPECIFIC0),
       OPS(XREG3,XREG,  SPECIFIC1),
       /**** Pre-index */
       OPS(WREG, WREG3, SPECIFIC2),
       OPS(XREG, XREG3, SPECIFIC3),
       OPS(WREG3,WREG,  SPECIFIC2),
       OPS(XREG3,XREG,  SPECIFIC3),
       /**** Sined offset */
       OPS(WREG, WREG3, PTR_O, XNSP2, PTR_C),
       OPS(WREG, WREG3, SPECIFIC4),
       OPS(XREG, XREG3, PTR_O, XNSP2, PTR_C),
       OPS(XREG, XREG3, SPECIFIC5),
       OPS(WREG3,WREG,  PTR_O, XNSP2, PTR_C),
       OPS(WREG3,WREG,  SPECIFIC4),
       OPS(XREG3,XREG,  PTR_O, XNSP2, PTR_C),
       OPS(XREG3,XREG,  SPECIFIC5));
  /*** LDPSW Load Pair signed words LDPSW on page C6-883 */
  PUT8(Load_Store_Pair1,
       NM("ldpsw"),
       /**** Post-index */
       OPS(XREG, XREG3, SPECIFIC0),
       OPS(XREG3,XREG,  SPECIFIC0),
       /**** Pre-index */
       OPS(XREG, XREG3, SPECIFIC2),
       OPS(XREG3,XREG,  SPECIFIC2),
       /**** Sined offset */
       OPS(XREG, XREG3, PTR_O, XNSP2, PTR_C),
       OPS(XREG, XREG3, SPECIFIC4),
       OPS(XREG3,XREG,  PTR_O, XNSP2, PTR_C),
       OPS(XREG3,XREG,  SPECIFIC4));

  void putLoadStorePair_core() {
    putLoad_Store_Pair0();
    putLoad_Store_Pair1();
  }

  /** Load/Store Non-temporal Pair on page C3-180. */
  /*** LDNP Load Non-temporal Pair LDNP on page C6-878 */
  /*** STNP Store Non-temporal Pair STNP on page C6-1129 */
  PUT8(Load_Store_Non_temporal_Pair0,
       NM("ldnp", "stnp"),
       OPS(WREG, WREG3, PTR_O, XNSP2, PTR_C),
       OPS(WREG, WREG3, PTR_O, XNSP2, SPECIFIC0, PTR_C),
       OPS(XREG, XREG3, PTR_O, XNSP2, PTR_C),
       OPS(XREG, XREG3, PTR_O, XNSP2, SPECIFIC1, PTR_C),
       OPS(WREG3,WREG,  PTR_O, XNSP2, PTR_C),
       OPS(WREG3,WREG,  PTR_O, XNSP2, SPECIFIC0, PTR_C),
       OPS(XREG3,XREG,  PTR_O, XNSP2, PTR_C),
       OPS(XREG3,XREG,  PTR_O, XNSP2, SPECIFIC1, PTR_C));

  void putLoadStoreNontemporalPair_core() {
    putLoad_Store_Non_temporal_Pair0();
  }
  
  /** Load/Store unprivileged on page C3-181. */
  /*** LDTR Load unprivileged register LDTR on page C6-939 */
  /*** LDTRSB Load unprivileged signed byte LDTRSB on page C6-945 */
  /*** LDTRSH Load unprivileged signed halfword LDTRSH on page C6-947 */
  /*** STTR Store unprivileged register STTR on page C6-1165 */
  PUT4(Load_Store_unprivileged0,
       NM("ldtr", "ldtrsb", "ldtrsh", "sttr"),
       OPS(WREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, PTR_O, XNSP2, IMM9BIT_PM, PTR_C),
       OPS(XREG, PTR_O, XNSP2, PTR_C),
       OPS(XREG, PTR_O, XNSP2, IMM9BIT_PM, PTR_C));
  /*** LDTRB Load unprivileged byte LDTRB on page C6-941 */
  /*** LDTRH Load unprivileged halfword LDTRH on page C6-943 */
  /*** STTRB Store unprivileged byte STTRB on page C6-1167 */
  /*** STTRH Store unprivileged halfword STTRH on page C6-1169 */
  PUT2(Load_Store_unprivileged1,
       NM("ldtrb", "ldtrh", "sttrb", "sttrh"),
       OPS(WREG, PTR_O, XNSP2, PTR_C), 
       OPS(WREG, PTR_O, XNSP2, IMM9BIT_PM, PTR_C)); 
  /*** LDTRSW Load unprivileged signed word LDTRSW on page C6-949 */
  PUT2(Load_Store_unprivileged2,
       NM("ldtrsw"),
       OPS(XREG, PTR_O, XNSP2, PTR_C),
       OPS(XREG, PTR_O, XNSP2, IMM9BIT_PM, PTR_C));

  void putLoadStoreUnprivileged_core() {
    putLoad_Store_unprivileged0();
    putLoad_Store_unprivileged1();
    putLoad_Store_unprivileged2();
  }

  /** Load-Exclusive/Store-Exclusive on page C3-181. */
  /*** LDXR Load Exclusive register LDXR on page C6-976 */
  PUT4(Load_Exclusive_Store_Exclusive0,
       NM("ldxr"),
       OPS(WREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, PTR_O, XNSP2, SPECIFIC0, PTR_C),
       OPS(XREG, PTR_O, XNSP2, PTR_C),
       OPS(XREG, PTR_O, XNSP2, SPECIFIC0, PTR_C));
  /*** LDXRB Load Exclusive byte LDXRB on page C6-978 */
  /*** LDXRH Load Exclusive halfword LDXRH on page C6-979 */
  PUT2(Load_Exclusive_Store_Exclusive1,
       NM("ldxrb", "ldxrh"),
       OPS(WREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, PTR_O, XNSP2, SPECIFIC0, PTR_C));
  /*** LDXP Load Exclusive pair LDXP on page C6-974 */
  PUT4(Load_Exclusive_Store_Exclusive2,
       NM("ldxp"),
       OPS(WREG, WREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, WREG, PTR_O, XNSP2, SPECIFIC0, PTR_C),
       OPS(XREG, XREG, PTR_O, XNSP2, PTR_C),
       OPS(XREG, XREG, PTR_O, XNSP2, SPECIFIC0, PTR_C));
  /*** STXR Store Exclusive register STXR on page C6-1190 */
  PUT4(Load_Exclusive_Store_Exclusive3,
       NM("stxr"),
       OPS(WREG, WREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, WREG, PTR_O, XNSP2, SPECIFIC0, PTR_C),
       OPS(WREG, XREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, XREG, PTR_O, XNSP2, SPECIFIC0, PTR_C));
  /*** STXRB Store Exclusive byte STXRB on page C6-1192 */
  /*** STXRH Store Exclusive halfword STXRH on page C6-1194 */
  PUT2(Load_Exclusive_Store_Exclusive4,
       NM("stxrb", "stxrh"),
       OPS(WREG, WREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, WREG, PTR_O, XNSP2, SPECIFIC0, PTR_C));
  /*** STXP Store Exclusive pair STXP on page C6-1187 */
  PUT4(Load_Exclusive_Store_Exclusive5,
       NM("stxp"),
       OPS(WREG, WREG, WREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, WREG, WREG, PTR_O, XNSP2, SPECIFIC0, PTR_C),
       OPS(WREG, XREG, XREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, XREG, XREG, PTR_O, XNSP2, SPECIFIC0, PTR_C));

  void putLoadExclusiveStoreExclusive_core() {
    putLoad_Exclusive_Store_Exclusive0();
    putLoad_Exclusive_Store_Exclusive1();
    putLoad_Exclusive_Store_Exclusive2();
    putLoad_Exclusive_Store_Exclusive3();
    putLoad_Exclusive_Store_Exclusive4();
    putLoad_Exclusive_Store_Exclusive5();
  }

  /** Load-Acquire/Store-Release on page C3-182. */
  /*** LDAPR Load-Acquire RCpc Register LDAPR on page C6-834 */
  /*** LDAR Load-Acquire Register LDAR on page C6-850 */
  /*** STLR Store-Release Register STLR on page C6-1113 */
  /*** LDAXR Load-Acquire Exclusive register LDAXR on page C6-856 */
  PUT4(Load_Acquire_Store_Release0,
       NM("ldapr", "ldar", "stlr", "ldaxr"),
       OPS(WREG, PTR_O, XNSP, PTR_C),
       OPS(WREG, PTR_O, XNSP, SPECIFIC0, PTR_C),
       OPS(XREG, PTR_O, XNSP, PTR_C),
       OPS(XREG, PTR_O, XNSP, SPECIFIC0, PTR_C));
  /*** LDAPRB Load-Acquire RCpc Register Byte LDAPRB on page C6-836 */
  /*** LDAPRH Load-Acquire RCpc Register Halfword LDAPRH on page C6-837 */
  /*** LDARB Load-Acquire Byte LDARB on page C6-852 */
  /*** LDARH Load-Acquire Halfword LDARH on page C6-853 */
  /*** STLRB Store-Release Byte STLRB on page C6-1114 */
  /*** STLRH Store-Release Halfword STLRH on page C6-1115 */
  /*** LDAXRB Load-Acquire Exclusive byte LDAXRB on page C6-858 */
  /*** LDAXRH Load-Acquire Exclusive halfword LDAXRH on page C6-859 */
  PUT2(Load_Acquire_Store_Release1,
       NM("ldaprb", "ldaprh", "ldarb", "ldarh", "stlrb", "stlrh", "ldaxrb", "ldaxrh"),
       OPS(WREG, PTR_O, XNSP, PTR_C),
       OPS(WREG, PTR_O, XNSP, SPECIFIC0, PTR_C));
  /*** LDAPUR Load-Acquire RCpc Register (unscaled) LDAPUR on page C6-838 */
  /*** LDAPURSB Load-Acquire RCpc Register Signed Byte (unscaled) 32-bit LDAPURSB on page C6-844 */
  /*** LDAPURSB Load-Acquire RCpc Register Signed Byte (unscaled) 64-bit LDAPURSB on page C6-844 */
  /*** LDAPURSH Load-Acquire RCpc Register Signed Halfword (unscaled) 32-bit LDAPURSH on page C6-846 */
  /*** LDAPURSH Load-Acquire RCpc Register Signed Halfword (unscaled) 64-bit LDAPURSH on page C6-846 */
  /*** STLUR Store-Release Register (unscaled) STLUR on page C6-1116 */
  PUT4(Load_Acquire_Store_Release2,
       NM("ldapur", "ldapursb", "ldapursh", "stlur"),
       OPS(WREG, PTR_O, XNSP, PTR_C),
       OPS(WREG, PTR_O, XNSP, IMM9BIT_PM, PTR_C),
       OPS(XREG, PTR_O, XNSP, PTR_C),
       OPS(XREG, PTR_O, XNSP, IMM9BIT_PM, PTR_C));
  /*** LDAPURB Load-Acquire RCpc Register Byte (unscaled) LDAPURB on page C6-840 */
  /*** LDAPURH Load-Acquire RCpc Register Halfword (unscaled) LDAPURH on page C6-842 */
  /*** STLURB Store-Release Register Byte (unscaled) STLURB on page C6-1118 */
  /*** STLURH Store-Release Register Halfword (unscaled) STLURH on page C6-1119 */
  PUT2(Load_Acquire_Store_Release3,
       NM("ldapurb", "ldapurh", "stlurb", "stlurh"),
       OPS(WREG, PTR_O, XNSP, PTR_C),
       OPS(WREG, PTR_O, XNSP, IMM9BIT_PM, PTR_C));
  /*** LDAPURSW Load-Acquire RCpc Register Signed Word (unscaled) LDAPURSW on page C6-848 */
  PUT2(Load_Acquire_Store_Release4,
       NM("ldapursw"),
       OPS(XREG, PTR_O, XNSP, PTR_C),
       OPS(XREG, PTR_O, XNSP, IMM9BIT_PM, PTR_C));
  /*** LDAXP Load-Acquire Exclusive pair LDAXP on page C6-854 */
  PUT4(Load_Acquire_Store_Release5,
       NM("ldaxp"),
       OPS(WREG, WREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, WREG, PTR_O, XNSP2, SPECIFIC0, PTR_C),
       OPS(XREG, XREG, PTR_O, XNSP2, PTR_C),
       OPS(XREG, XREG, PTR_O, XNSP2, SPECIFIC0, PTR_C));
  /*** STLXR Store-Release Exclusive register STLXR on page C6-1123 */
  PUT4(Load_Acquire_Store_Release6,
       NM("stlxr"),
       OPS(WREG, WREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, WREG, PTR_O, XNSP2, SPECIFIC0, PTR_C),
       OPS(WREG, XREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, XREG, PTR_O, XNSP2, SPECIFIC0, PTR_C));
  /*** STLXRB Store-Release Exclusive byte STLXRB on page C6-1125 */
  /*** STLXRH Store-Release Exclusive halfword STLXRH on page C6-1127 */
  PUT2(Load_Acquire_Store_Release7,
       NM("stlxrb", "stlxrb"),
       OPS(WREG, WREG, PTR_O, XNSP, PTR_C),
       OPS(WREG, WREG, PTR_O, XNSP, SPECIFIC0, PTR_C));
  /*** STLXP Store-Release Exclusive pair STLXP on page C6-1120 */
  PUT4(Load_Acquire_Store_Release8,
       NM("stlxp"),
       OPS(WREG, WREG, WREG, PTR_O, XNSP, PTR_C),
       OPS(WREG, WREG, WREG, PTR_O, XNSP, SPECIFIC0, PTR_C),
       OPS(WREG, XREG, XREG, PTR_O, XNSP, PTR_C),
       OPS(WREG, XREG, XREG, PTR_O, XNSP, SPECIFIC0, PTR_C));

  void putLoadAcquireStoreRelease_core() {
    putLoad_Acquire_Store_Release0();
    putLoad_Acquire_Store_Release1();
    putLoad_Acquire_Store_Release2();
    putLoad_Acquire_Store_Release3();
    putLoad_Acquire_Store_Release4();
    putLoad_Acquire_Store_Release5();
    putLoad_Acquire_Store_Release6();
    putLoad_Acquire_Store_Release7();
    putLoad_Acquire_Store_Release8();
  }

  /** LoadLOAcquire/StoreLORelease on page C3-184. */
  /*** LDLARB LoadLOAcquire byte LDLARB on page C6-874 */
  /*** LDLARH LoadLOAcquire halfword LDLARH on page C6-875 */
  /*** STLLRB StoreLORelease byte STLLRB on page C6-1110 */
  /*** STLLRH StoreLORelease halfword STLLRH on page C6-1111 */
  PUT2(LoadLOAcquire_StoreLORelease0,
       NM("ldlarb", "ldlarh", "stllrb", "stllrh"),
       OPS(WREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, PTR_O, XNSP2, SPECIFIC0, PTR_C));
  /*** LDLAR LoadLOAcquire register LDLAR on page C6-876 */
  /*** STLLR StoreLORelease register STLLR on page C6-1112 */
  PUT4(LoadLOAcquire_StoreLORelease1,
       NM("ldlar", "stllr"),
       OPS(WREG, PTR_O, XNSP2, PTR_C),
       OPS(WREG, PTR_O, XNSP2, SPECIFIC0, PTR_C),
       OPS(XREG, PTR_O, XNSP2, PTR_C),
       OPS(XREG, PTR_O, XNSP2, SPECIFIC0, PTR_C));

  void putLoadLOAcquireStoreLORelease_core() {
    putLoadLOAcquire_StoreLORelease0();
    putLoadLOAcquire_StoreLORelease1();
  }

  void putLoadStoreRegisterOffset() {
    clearTvAndJtv();
    
    tv_SPECIFIC0.push_back("UXTW 2");
    tv_SPECIFIC0.push_back("UXTW 0");
    tv_SPECIFIC0.push_back("UXTW 2");
    tv_SPECIFIC0.push_back("UXTW 0");
    tv_SPECIFIC0.push_back("SXTW 2");
    tv_SPECIFIC0.push_back("SXTW 0");
    tv_SPECIFIC0.push_back("SXTW 2");
    tv_SPECIFIC0.push_back("SXTW 0");
    jtv_SPECIFIC0.push_back("UXTW, 2");
    jtv_SPECIFIC0.push_back("UXTW, 0");
    jtv_SPECIFIC0.push_back("UXT, 2");
    jtv_SPECIFIC0.push_back("UXT, 0");
    jtv_SPECIFIC0.push_back("SXTW, 2");
    jtv_SPECIFIC0.push_back("SXTW, 0");
    jtv_SPECIFIC0.push_back("SXT, 2");
    jtv_SPECIFIC0.push_back("SXT, 0");

    tv_SPECIFIC1.push_back("SXTX 2");
    tv_SPECIFIC1.push_back("SXTX 0");
    tv_SPECIFIC1.push_back("SXTX 2");
    tv_SPECIFIC1.push_back("SXTX 0");
    jtv_SPECIFIC1.push_back("SXTX, 2");
    jtv_SPECIFIC1.push_back("SXTX, 0");
    jtv_SPECIFIC1.push_back("SXT, 2");
    jtv_SPECIFIC1.push_back("SXT, 0");

    tv_SPECIFIC2.push_back("UXTW 1");
    tv_SPECIFIC2.push_back("UXTW 0");
    tv_SPECIFIC2.push_back("UXTW 1");
    tv_SPECIFIC2.push_back("UXTW 0");
    jtv_SPECIFIC2.push_back("UXTW, 1");
    jtv_SPECIFIC2.push_back("UXTW, 0");
    jtv_SPECIFIC2.push_back("UXT, 1");
    jtv_SPECIFIC2.push_back("UXT, 0");

  tv_SPECIFIC2.push_back("UXTW 1");
    tv_SPECIFIC2.push_back("UXTW 0");
    tv_SPECIFIC2.push_back("UXTW 1");
    tv_SPECIFIC2.push_back("UXTW 0");
    jtv_SPECIFIC2.push_back("UXTW, 1");
    jtv_SPECIFIC2.push_back("UXTW, 0");
    jtv_SPECIFIC2.push_back("UXT, 1");
    jtv_SPECIFIC2.push_back("UXT, 0");
  
    tv_SPECIFIC3.push_back("SXTW 1");
    tv_SPECIFIC3.push_back("SXTW 0");
    tv_SPECIFIC3.push_back("SXTW 1");
    tv_SPECIFIC3.push_back("SXTW 0");
    jtv_SPECIFIC3.push_back("SXTW, 1");
    jtv_SPECIFIC3.push_back("SXTW, 0");
    jtv_SPECIFIC3.push_back("SXT, 1");
    jtv_SPECIFIC3.push_back("SXT, 0");

    tv_SPECIFIC4.push_back("LSL 1");
    tv_SPECIFIC4.push_back("LSL 0");
    jtv_SPECIFIC4.push_back("LSL, 1");
    jtv_SPECIFIC4.push_back("LSL, 0");

    tv_SPECIFIC5.push_back("SXTX 1");
    tv_SPECIFIC5.push_back("SXTX 0");
    tv_SPECIFIC5.push_back("SXTX 1");
    tv_SPECIFIC5.push_back("SXTX 0");
    jtv_SPECIFIC5.push_back("SXTX, 1");
    jtv_SPECIFIC5.push_back("SXTX, 0");
    jtv_SPECIFIC5.push_back("SXT, 1");
    jtv_SPECIFIC5.push_back("SXT, 0");

    tv_SPECIFIC6.push_back("SXTX 3");
    tv_SPECIFIC6.push_back("SXTX 0");
    tv_SPECIFIC6.push_back("SXTX 3");
    tv_SPECIFIC6.push_back("SXTX 0");
    jtv_SPECIFIC6.push_back("SXTX, 3");
    jtv_SPECIFIC6.push_back("SXTX, 0");
    jtv_SPECIFIC6.push_back("SXT, 3");
    jtv_SPECIFIC6.push_back("SXT, 0");

    tv_SPECIFIC7.push_back("UXTW 3");
    tv_SPECIFIC7.push_back("UXTW 0");
    tv_SPECIFIC7.push_back("UXTW 3");
    tv_SPECIFIC7.push_back("UXTW 0");
    tv_SPECIFIC7.push_back("SXTW 3");
    tv_SPECIFIC7.push_back("SXTW 0");
    tv_SPECIFIC7.push_back("SXTW 3");
    tv_SPECIFIC7.push_back("SXTW 0");
    jtv_SPECIFIC7.push_back("UXTW, 3");
    jtv_SPECIFIC7.push_back("UXTW, 0");
    jtv_SPECIFIC7.push_back("UXT, 3");
    jtv_SPECIFIC7.push_back("UXT, 0");
    jtv_SPECIFIC7.push_back("SXTW, 3");
  jtv_SPECIFIC7.push_back("SXTW, 0");
  jtv_SPECIFIC7.push_back("SXT, 3");
  jtv_SPECIFIC7.push_back("SXT, 0");

  tv_SPECIFIC8.push_back("UXTW 0");
  tv_SPECIFIC8.push_back("UXTW 0");
  jtv_SPECIFIC8.push_back("UXTW, 0");
  jtv_SPECIFIC8.push_back("UXT, 0");

  tv_SPECIFIC9.push_back("SXTW 0");
  tv_SPECIFIC9.push_back("SXTW 0");
  jtv_SPECIFIC9.push_back("SXTW, 0");
  jtv_SPECIFIC9.push_back("SXT, 0");

  tv_SPECIFIC10.push_back("SXTX 0");
  tv_SPECIFIC10.push_back("SXTX 0");
  jtv_SPECIFIC10.push_back("SXTX, 0");
  jtv_SPECIFIC10.push_back("SXT, 0");

  tv_SPECIFIC11.push_back("LSL 0");
  jtv_SPECIFIC11.push_back("LSL, 0");
  
  
  {
    std::vector<std::string> tmpW = tv_WREG;
    std::vector<std::string> tmpX = tv_XREG3;
    tmpW.push_back("wsp");
    tmpX.push_back("sp");  
    
    setTvAddressing(tv_SPECIFIC32, jtv_SPECIFIC32, tmpX, tv_IMM9BIT_PM, TP_PostImm);
    setTvAddressing(tv_SPECIFIC64, jtv_SPECIFIC64, tmpX, tv_IMM9BIT_PM, TP_PostImm);
    setTvAddressing(tv_SPECIFIC32_1, jtv_SPECIFIC32_1, tmpX, tv_IMM9BIT_PM, TP_PreImm);
    setTvAddressing(tv_SPECIFIC64_1, jtv_SPECIFIC64_1, tmpX, tv_IMM9BIT_PM, TP_PreImm);

    setTvAddressing(tv_SPECIFIC32_2, jtv_SPECIFIC32_2, tmpX, tv_IMM12BIT, TP_None);
    setTvAddressing(tv_SPECIFIC64_2, jtv_SPECIFIC64_2, tmpX, tv_IMM12BIT, TP_None);
  }

    putLoadStoreRegisterOffset_core();
  }

  void putLoadStoreRegisterUnscaledOffset() {
    putLoadStoreRegisterUnscaledOffset_core();
  }

void putLoadStorePair() {
    std::vector<std::string> tmpW = tv_WREG;
    std::vector<std::string> tmpX = tv_XREG3;
    std::vector<int>    imm7bit_pm = { 31, -64, -32, -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 16, 32, 63 };
    std::vector<std::string>    imm7bit_pm_mul4;
    std::vector<std::string>    imm7bit_pm_mul8;

    clearTvAndJtv();
    
    tmpW.push_back("wsp");
    tmpX.push_back("sp");  

    for(int i : imm7bit_pm) {
      imm7bit_pm_mul4.push_back(std::to_string(i*4));
      imm7bit_pm_mul8.push_back(std::to_string(i*8));
  }

    
    
    setTvAddressing(tv_SPECIFIC0, jtv_SPECIFIC0, tmpX, imm7bit_pm_mul4, TP_PostImm);
    setTvAddressing(tv_SPECIFIC1, jtv_SPECIFIC1, tmpX, imm7bit_pm_mul8, TP_PostImm);

    setTvAddressing(tv_SPECIFIC2, jtv_SPECIFIC2, tmpX, imm7bit_pm_mul4, TP_PreImm);
    setTvAddressing(tv_SPECIFIC3, jtv_SPECIFIC3, tmpX, imm7bit_pm_mul8, TP_PreImm);

    setTvAddressing(tv_SPECIFIC4, jtv_SPECIFIC4, tmpX, imm7bit_pm_mul4, TP_None);
    setTvAddressing(tv_SPECIFIC5, jtv_SPECIFIC5, tmpX, imm7bit_pm_mul8, TP_None);

    putLoadStorePair_core();
  }

void putLoadStoreNontemporalPair() {
    std::vector<std::string> tmpW = tv_WREG;
    std::vector<std::string> tmpX = tv_XREG2;
    std::vector<int>    imm7bit_pm = { 31, -64, -32, -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 16, 32, 63 };

        clearTvAndJtv();

    tmpW.push_back("wsp");
    tmpX.push_back("sp");  

    for(int i : imm7bit_pm) {
      tv_SPECIFIC0.push_back(std::to_string(i*4));
      jtv_SPECIFIC0.push_back(std::to_string(i*4));
      tv_SPECIFIC1.push_back(std::to_string(i*8));
      jtv_SPECIFIC1.push_back(std::to_string(i*8));
    }

    putLoadStoreNontemporalPair_core();
}

  void putLoadStoreUnprivileged() {
    putLoadStoreUnprivileged_core();
  }

  void putLoadExclusiveStoreExclusive() {
    clearTvAndJtv();

    tv_SPECIFIC0.push_back("0");
    jtv_SPECIFIC0.push_back("0");

    putLoadExclusiveStoreExclusive_core();
  }

  void putLoadAcquireStoreRelease() {
    clearTvAndJtv();

    tv_SPECIFIC0.push_back("0");
    jtv_SPECIFIC0.push_back("0");

    putLoadAcquireStoreRelease_core();
  }

  void putLoadLOAcquireStoreLORelease() {
    clearTvAndJtv();

    tv_SPECIFIC0.push_back("0");
    jtv_SPECIFIC0.push_back("0");

    putLoadLOAcquireStoreLORelease_core();
  }


  void put() {
    putLoadStoreRegisterOffset();
    putLoadStoreRegisterUnscaledOffset();
    putLoadStorePair();
    putLoadStoreNontemporalPair();
    putLoadStoreUnprivileged();
    putLoadExclusiveStoreExclusive();
    putLoadAcquireStoreRelease();
    putLoadLOAcquireStoreLORelease();

    //    Ops hoge();
    //    hoge.pushNm({"add", "sub"});
    
    
  }

};
  
  
int main(int argc, char *[])
{
  Test test(argc > 1);
  test.put();
}
