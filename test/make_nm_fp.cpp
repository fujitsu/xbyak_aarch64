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
const uint64_t WSP   = 1ULL << flagBit++; /** Test vector is {wsp} */
const uint64_t XSP   = 1ULL << flagBit++; /** Test vector is {sp} */
const uint64_t WREG_WSP = WREG | WSP;
const uint64_t XREG_XSP = XREG | XSP;
const uint64_t IMM4BIT   = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 8, 15 } */
const uint64_t IMM5BIT   = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 16, 31 } */
const uint64_t IMM6BIT   = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 32, 63 } */
const uint64_t IMM8BIT   = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 128, 255 } */
const uint64_t IMM12BIT  = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 2048, 4095 } */
const uint64_t IMM13BIT  = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 4096, 8191 } */
const uint64_t IMM16BIT  = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 4096, 1<<13, 1<<14, 1<<15, 1<<16-1 } */
const uint64_t IMM5BIT_N = 1ULL << flagBit++; /** Test vector is {0, 1, 2, .., 32 } */
const uint64_t IMM6BIT_N = 1ULL << flagBit++; /** Test vector is {0, 1, 2, .., 64 } */
const uint64_t FLOAT8BIT = 1ULL << flagBit++; /** Test vector is Table C-2- Floating-point constant values */
const uint64_t COND      = 1ULL << flagBit++; /** Test vector is { EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, AL } */
const uint64_t COND_WO_AL = 1ULL << flagBit++; /** Test vector is { EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE } */
const uint64_t NZCV       = 1ULL << flagBit++; /** Test vector is { 0, 1, ..., 15 } */

const uint64_t BREG = 1ULL << flagBit++; /** Test vector is { b0, b1, ..., b31 } */
const uint64_t HREG = 1ULL << flagBit++; /** Test vector is { h0, h1, ..., h31 } */
const uint64_t SREG = 1ULL << flagBit++; /** Test vector is { s0, s1, ..., s31 } */
const uint64_t DREG = 1ULL << flagBit++; /** Test vector is { d0, d1, ..., d31 } */

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
const uint64_t EXT_AMOUNT32    = 1ULL << flagBit++; /** Test vector is generated on the fly. */
const uint64_t EXT_AMOUNT64    = 1ULL << flagBit++; /** Test vector is generated on the fly. */

const uint64_t VREG_ELEM    = 1ULL << flagBit++; /** Test vector is generated on the fly. */



const uint64_t NOPARA = 1ULL << (bitEnd - 1);


  
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
  std::vector<std::string> tv_WSP  = { "wsp" };
  std::vector<std::string> tv_XSP  = { "sp" };
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
  std::vector<std::string> tv_IMM5BIT_N = { "1", "2", "4", "8", "16", "32" };
  std::vector<std::string> tv_IMM6BIT_N = { "1", "2", "4", "8", "16", "32", "64" };
  std::vector<std::string> tv_FLOAT8BIT = { "2.0", "4.0", "8.0", "16.0", "0.125", "0.25", "0.5", "1.0",
					    "2.125", "4.5", "9.5", "20.0", "0.1640625", "0.34375", "0.71875", "1.5",
					    "0.78125", "0.40625", "0.2109375", "28.0", "14.5", "7.5", "3.875",
					    "1.9375" };
  std::vector<std::string> tv_COND       = { "EQ", "NE", "CS", "CC", "MI", "PL", "VS", "VC", "HI", "LS", "GE", "LT", "GT", "LE", "AL" };
  std::vector<std::string> tv_COND_WO_AL = { "EQ", "NE", "CS", "CC", "MI", "PL", "VS", "VC", "HI", "LS", "GE", "LT", "GT", "LE" };
  std::vector<std::string> tv_NZCV     = { "15", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14" };

  std::vector<std::string> tv_BREG = { "b7", "b0", "b1", "b2", "b4", "b8", "b16", "b31" };
  std::vector<std::string> tv_HREG = { "h7", "h0", "h1", "h2", "h4", "h8", "h16", "h31" };
  std::vector<std::string> tv_SREG = { "s7", "s0", "s1", "s2", "s4", "s8", "s16", "s31" };
  std::vector<std::string> tv_DREG = { "d7", "d0", "d1", "d2", "d4", "d8", "d16", "d31" };

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
  std::vector<std::string> tv_EXT_AMOUNT32, jtv_EXT_AMOUNT32;
  std::vector<std::string> tv_EXT_AMOUNT64, jtv_EXT_AMOUNT64;
  std::vector<std::string> tv_VREG_ELEM, jtv_VREG_ELEM;

  
  std::vector<std::vector<std::string> *> tv_VectorsAs = { &tv_WREG, &tv_XREG, &tv_WSP, &tv_XSP,
							   &tv_IMM4BIT, &tv_IMM5BIT, &tv_IMM6BIT, &tv_IMM8BIT,
							   &tv_IMM12BIT, &tv_IMM13BIT,
							   &tv_IMM16BIT, &tv_IMM5BIT_N, &tv_IMM6BIT_N, &tv_FLOAT8BIT,
							   &tv_COND, &tv_COND_WO_AL, &tv_NZCV,
							   &tv_BREG, &tv_HREG, &tv_SREG, &tv_DREG,
							   &tv_BITMASK32, &tv_BITMASK64,
							   &tv_LSL_IMM, &tv_LSL32, &tv_LSL64,
							   &tv_SPECIFIC32, &tv_SPECIFIC64, &tv_SPECIFIC32_1, &tv_SPECIFIC64_1,
							   &tv_SPECIFIC32_2, &tv_SPECIFIC64_2, &tv_SPECIFIC32_3, &tv_SPECIFIC64_3,
							   &tv_SHIFT_AMOUNT32, &tv_SHIFT_AMOUNT64,
							   &tv_EXT_AMOUNT32, &tv_EXT_AMOUNT64,
							   &tv_VREG_ELEM };
  std::vector<std::vector<std::string> *> tv_VectorsJit = { &tv_WREG, &tv_XREG, &tv_WSP, &tv_XSP,
							    &tv_IMM4BIT, &tv_IMM5BIT, &tv_IMM6BIT, &tv_IMM8BIT,
							    &tv_IMM12BIT, &tv_IMM13BIT,
							    &tv_IMM16BIT, &tv_IMM5BIT_N, &tv_IMM6BIT_N, &tv_FLOAT8BIT,
							    &tv_COND, &tv_COND_WO_AL, &tv_NZCV,
							    &tv_BREG, &tv_HREG, &tv_SREG, &tv_DREG,
							    &tv_BITMASK32, &tv_BITMASK64, &jtv_LSL_IMM, &jtv_LSL32, &jtv_LSL64,
							    &jtv_SPECIFIC32, &jtv_SPECIFIC64, &jtv_SPECIFIC32_1, &jtv_SPECIFIC64_1,
							    &jtv_SPECIFIC32_2, &jtv_SPECIFIC64_2, &jtv_SPECIFIC32_3, &jtv_SPECIFIC64_3,
							    &jtv_SHIFT_AMOUNT32, &jtv_SHIFT_AMOUNT64,
							    &jtv_EXT_AMOUNT32, &jtv_EXT_AMOUNT64,
							    &jtv_VREG_ELEM };
  std::vector<std::vector<std::string> *>& tv_Vectors = tv_VectorsAs;

  
  /*
    and_, or_, xor_, not_ => and, or, xor, not
  */
  std::string removeUnderScore(std::string s) const
  {
    if (!isXbyak_ && s[s.size() - 1] == '_') s.resize(s.size() - 1);
    return s;
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
  /*** Floating-point move (register) on page C3-207. */
  /**** Floating-point move register without conversion FMOV (register) on page C7-1533 */
  PUT3(DataProcFp_FloatingPointMoveRegister0,
       NM("fmov"),
       OPS(HREG, HREG), OPS(SREG, SREG), OPS(DREG, DREG));
  /**** Floating-point move to or from general-purpose register without conversion FMOV (general) on page C7-1535 */
  PUT8(DataProcFp_FloatingPointMoveRegister1,
       NM("fmov"),
       OPS(WREG, HREG), OPS(XREG, HREG), OPS(HREG, WREG), OPS(SREG, WREG),
       OPS(WREG, SREG), OPS(HREG, XREG), OPS(DREG, XREG), OPS(VREG_ELEM, XREG));
  PUT2(DataProcFp_FloatingPointMoveRegister2,
       NM("fmov"),
       OPS(XREG, DREG), OPS(XREG, VREG_ELEM));

  void putDataProcFp_FloatingPointMoveRegister() {
    tv_VREG_ELEM.clear();
    jtv_VREG_ELEM.clear();

    tv_VREG_ELEM.push_back("v7.d[1]");
    tv_VREG_ELEM.push_back("v0.d[1]");
    tv_VREG_ELEM.push_back("v1.d[1]");
    tv_VREG_ELEM.push_back("v2.d[1]");
    tv_VREG_ELEM.push_back("v4.d[1]");
    tv_VREG_ELEM.push_back("v8.d[1]");
    tv_VREG_ELEM.push_back("v16.d[1]");
    tv_VREG_ELEM.push_back("v31.d[1]");

    tv_VREG_ELEM.push_back("v7.d[1]");
    tv_VREG_ELEM.push_back("v0.d[1]");
    tv_VREG_ELEM.push_back("v1.d[1]");
    tv_VREG_ELEM.push_back("v2.d[1]");
    tv_VREG_ELEM.push_back("v4.d[1]");
    tv_VREG_ELEM.push_back("v8.d[1]");
    tv_VREG_ELEM.push_back("v16.d[1]");
    tv_VREG_ELEM.push_back("v31.d[1]");

    
    jtv_VREG_ELEM.push_back("v7.d2[1]");
    jtv_VREG_ELEM.push_back("v0.d2[1]");
    jtv_VREG_ELEM.push_back("v1.d2[1]");
    jtv_VREG_ELEM.push_back("v2.d2[1]");
    jtv_VREG_ELEM.push_back("v4.d2[1]");
    jtv_VREG_ELEM.push_back("v8.d2[1]");
    jtv_VREG_ELEM.push_back("v16.d2[1]");
    jtv_VREG_ELEM.push_back("v31.d2[1]");

    jtv_VREG_ELEM.push_back("v7d_1");
    jtv_VREG_ELEM.push_back("v0d_1");
    jtv_VREG_ELEM.push_back("v1d_1");
    jtv_VREG_ELEM.push_back("v2d_1");
    jtv_VREG_ELEM.push_back("v4d_1");
    jtv_VREG_ELEM.push_back("v8d_1");
    jtv_VREG_ELEM.push_back("v16d_1");
    jtv_VREG_ELEM.push_back("v31d_1");

    if(isXbyak_) {
      std::cout << "VRegDElem v7d_1 = v7.d2[1];" << std::endl;
      std::cout << "VRegDElem v0d_1 = v0.d2[1];" << std::endl;
      std::cout << "VRegDElem v1d_1 = v1.d2[1];" << std::endl;
      std::cout << "VRegDElem v2d_1 = v2.d2[1];" << std::endl;
      std::cout << "VRegDElem v4d_1 = v4.d2[1];" << std::endl;
      std::cout << "VRegDElem v8d_1 = v8.d2[1];" << std::endl;
      std::cout << "VRegDElem v16d_1 = v16.d2[1];" << std::endl;
      std::cout << "VRegDElem v31d_1 = v31.d2[1];" << std::endl;
    }

    /** Debug
    for(std::string i : tv_VREG_ELEM) {
      std::cout << i << std::endl;
    }
    */

    putDataProcFp_FloatingPointMoveRegister0();
    putDataProcFp_FloatingPointMoveRegister1();
    putDataProcFp_FloatingPointMoveRegister2();
  }


  
  
  /*** Floating-point move (immediate) on page C3-207. */
  /**** FMOV Floating-point move immediate FMOV (scalar, immediate) on page C7-1538 */
  PUT3(DataProcFp_FloatingPointMoveImmediate0,
	 NM("fmov"),
	 OPS(HREG, FLOAT8BIT), OPS(SREG, FLOAT8BIT), OPS(DREG, FLOAT8BIT));

  void putDataProcFp_FloatingPointMoveImmediate() {
    putDataProcFp_FloatingPointMoveImmediate0();
  }
  
  /*** Floating-point conversion on page C3-208. */
  /**** FCVT Floating-point convert precision (scalar) FCVT on page C7-1396 */
  PUT6(DataProcFp_FloatingPointConversion0,
       NM("fcvt"),
       OPS(SREG, HREG), OPS(DREG, HREG), OPS(HREG, SREG), OPS(DREG, SREG),
       OPS(HREG, DREG), OPS(SREG, DREG));
  /**** FCVTAS Floating-point scalar convert to signed integer, rounding to nearest
	with ties to away (scalar form)
	FCVTAS (scalar) on page C7-1401 */
  /**** FCVTAU Floating-point scalar convert to unsigned integer, rounding to 
	nearest with ties to away (scalar form)
	FCVTAU (scalar) on page C7-1406 */
  /**** FCVTMS Floating-point scalar convert to signed integer, rounding toward
	minus infinity (scalar form)
	FCVTMS (scalar) on page C7-1413 */
  /**** FCVTMU Floating-point scalar convert to unsigned integer, rounding toward
	minus infinity (scalar form)
	FCVTMU (scalar) on page C7-1418 */
  /**** FCVTNS Floating-point scalar convert to signed integer, rounding to nearest
	with ties to even (scalar form)
	FCVTNS (scalar) on page C7-1425 */
  /**** FCVTNU Floating-point scalar convert to unsigned integer, rounding to
	nearest with ties to even (scalar form)
	FCVTNU (scalar) on page C7-1430 */
  /**** FCVTPS Floating-point scalar convert to signed integer, rounding toward
	positive infinity (scalar form)
	FCVTPS (scalar) on page C7-1435 */
  /**** FCVTPU Floating-point scalar convert to unsigned integer, rounding toward
	positive infinity (scalar form)
	FCVTPU (scalar) on page C7-1440 */
  /**** FCVTZS Floating-point scalar convert to signed integer, rounding toward
	zero (scalar form)
	FCVTZS (scalar, integer) on page C7-1452 */
  /**** FCVTZU Floating-point scalar convert to unsigned integer, rounding toward
	zero (scalar form)
	FCVTZU (scalar, integer) on page C7-1462 */
  PUT6(DataProcFp_FloatingPointConversion1,
       NM("fcvtas", "fcvtau", "fcvtms", "fcvtmu", "fcvtns", "fcvtnu", "fcvtps", "fcvtpu",
	  "fcvtzs", "fcvtzu"),
       OPS(WREG, HREG), OPS(XREG, HREG), OPS(WREG, SREG), OPS(XREG, SREG),
       OPS(WREG, DREG), OPS(XREG, DREG));
  /**** Floating-point convert to signed fixed-point, rounding toward zero
	(scalar form)
	FCVTZS (scalar, fixed-point) on page C7-1450 */
  /**** Floating-point scalar convert to unsigned fixed-point, rounding
	toward zero (scalar form)
	FCVTZU (scalar, fixed-point) on page C7-1460 */
  PUT6(DataProcFp_FloatingPointConversion2,
       NM("fcvtzs", "fcvtzu"),
       OPS(WREG, HREG, IMM5BIT_N), OPS(XREG, HREG, IMM6BIT_N),
       OPS(WREG, SREG, IMM5BIT_N), OPS(XREG, SREG, IMM6BIT_N),
       OPS(WREG, DREG, IMM5BIT_N), OPS(XREG, DREG, IMM6BIT_N));
  /**** FJCVTZS Floating-point Javascript convert to signed fixed-point, rounding
	toward zero
	FJCVTZS on page C7-1468 */
  PUT1(DataProcFp_FloatingPointConversion3,
       NM("fjcvtzs"),
       OPS(WREG, DREG));
  /**** SCVTF Signed integer scalar convert to floating-point, using the current
	rounding mode (scalar form)
	SCVTF (scalar, integer) on page C7-1761 */
  /**** UCVTF Unsigned integer scalar convert to floating-point, using the current
	rounding mode (scalar form)
	UCVTF (scalar, integer) on page C7-2032 */
  PUT6(DataProcFp_FloatingPointConversion4,
       NM("scvtf", "ucvtf"),
       OPS(HREG, WREG), OPS(SREG, WREG),
       OPS(DREG, WREG), OPS(HREG, XREG),
       OPS(SREG, XREG), OPS(DREG, XREG));
  /**** Signed fixed-point convert to floating-point, using the current
	rounding mode (scalar form)
	SCVTF (scalar, fixed-point) on page C7-1759 */
  /**** Unsigned fixed-point convert to floating-point, using the current
	rounding mode (scalar form)
	UCVTF (scalar, fixed-point) on page C7-2030 */
  PUT6(DataProcFp_FloatingPointConversion5,
       NM("scvtf", "ucvtf"),
       OPS(HREG, WREG, IMM5BIT_N), OPS(SREG, WREG, IMM5BIT_N),
       OPS(DREG, WREG, IMM5BIT_N), OPS(HREG, XREG, IMM6BIT_N),
       OPS(SREG, XREG, IMM6BIT_N), OPS(DREG, XREG, IMM6BIT_N));
  
  void putDataProcFp_FloatingPointConversion() {
    putDataProcFp_FloatingPointConversion0();
    putDataProcFp_FloatingPointConversion1();
    putDataProcFp_FloatingPointConversion2();
    putDataProcFp_FloatingPointConversion3();
    putDataProcFp_FloatingPointConversion4();
    putDataProcFp_FloatingPointConversion5();
  }

  /*** Floating-point round to integer on page C3-209. */
  /**** C7.2.141 FRINTA (scalar) */
  /**** C7.2.143 FRINTI (scalar) */
  /**** C7.2.145 FRINTM (scalar) */
  /**** C7.2.147 FRINTN (scalar) */
  /**** C7.2.149 FRINTP (scalar) */
  /**** C7.2.151 FRINTX (scalar) */
  /**** C7.2.153 FRINTZ (scalar) */
  PUT3(DataProcFp_FloatingPointRoundToInteger0,
	 NM("frinta", "frinti", "frintm", "frintn", "frintp", "frintx", "frintz"),
	 OPS(HREG, HREG), OPS(SREG, SREG), OPS(DREG, DREG));

  void putDataProcFp_FloatingPointRoundToInteger() {
    putDataProcFp_FloatingPointRoundToInteger0();
  }

  /*** Floating-point multiply-add on page C3-210. */
  /**** C7.2.93 FMADD */
  PUT3(DataProcFp_FloatingPointMultiplyAdd0,
       NM("fmadd", "fmsub", "fnmadd", "fnmsub"),
       OPS(HREG, HREG, HREG, HREG), OPS(SREG, SREG, SREG, SREG), OPS(DREG, DREG, DREG, DREG));

  void putDataProcFp_FloatingPointMultiplyAdd() {
    putDataProcFp_FloatingPointMultiplyAdd0();
  }

  /*** Floating-point arithmetic (one source) on page C3-210. */
  PUT3(DataProcFp_FloatingPointArithmeticOneSource0,
       NM("fabs", "fneg", "fsqrt"),
       OPS(HREG, HREG), OPS(SREG, SREG), OPS(DREG, DREG));

  void putDataProcFp_FloatingPointArithmeticOneSource() {
    putDataProcFp_FloatingPointArithmeticOneSource0();
  }

  /*** Floating-point arithmetic (two sources) on page C3-211. */
  PUT3(DataProcFp_FloatingPointArithmeticTwoSources0,
	 NM("fadd", "fdiv", "fmul", "fnmul", "fsub"),
	 OPS(HREG, HREG, HREG), OPS(SREG, SREG, SREG), OPS(DREG, DREG, DREG));

  void putDataProcFp_FloatingPointArithmeticTwoSources() {
    putDataProcFp_FloatingPointArithmeticTwoSources0();
  }

  /*** Floating-point minimum and maximum on page C3-211. */
  PUT3(DataProcFp_FloatingPointMinimumAndMaximum0,
       NM("fmax", "fmaxnm", "fmin", "fminnm"),
       OPS(HREG, HREG, HREG), OPS(SREG, SREG, SREG), OPS(DREG, DREG, DREG));

  void putDataProcFp_FloatingPointMinimumAndMaximum() {
    putDataProcFp_FloatingPointMinimumAndMaximum0();
  }

  /*** Floating-point comparison on page C3-211. */
  /**** C7.2.59 FCMP */
  /**** C7.2.60 FCMPE */
  PUT6(DataProcFp_FloatingPointComparison0,
       NM("fcmp", "fcmpe"),
       OPS(HREG, HREG), OPS(HREG, SPECIFIC64),
       OPS(SREG, SREG), OPS(SREG, SPECIFIC64),
       OPS(DREG, DREG), OPS(DREG, SPECIFIC64));
  /**** C7.2.47 FCCMP */
  /**** C7.2.48 FCCMPE */
  PUT3(DataProcFp_FloatingPointComparison1,
       NM("fccmp", "fccmpe"),
       OPS(HREG, HREG, NZCV, COND), OPS(SREG, SREG, NZCV, COND), OPS(DREG, DREG, NZCV, COND));

  void putDataProcFp_FloatingPointComparison() {
    tv_SPECIFIC64.clear();
    jtv_SPECIFIC64.clear();

    tv_SPECIFIC64.push_back("0.0");
    jtv_SPECIFIC64.push_back("0.0");

    putDataProcFp_FloatingPointComparison0();
    putDataProcFp_FloatingPointComparison1();
  }
  
  /*** Floating-point conditional select on page C3-212. */
  PUT3(DataProcFp_FloatingPointConditionalSelect0,
       NM("fcsel"),
       OPS(HREG, HREG, HREG, COND), OPS(SREG, SREG, SREG, COND), OPS(DREG, DREG, DREG, COND));
  
  void putDataProcFp_FloatingPointConditionalSelect() {
    putDataProcFp_FloatingPointConditionalSelect0();
  }

  void putDataProcFp() {
      putDataProcFp_FloatingPointMoveRegister();
      putDataProcFp_FloatingPointMoveImmediate();
      putDataProcFp_FloatingPointConversion();
      putDataProcFp_FloatingPointRoundToInteger();
      putDataProcFp_FloatingPointMultiplyAdd();
      putDataProcFp_FloatingPointArithmeticOneSource();
      putDataProcFp_FloatingPointArithmeticTwoSources();
      putDataProcFp_FloatingPointMinimumAndMaximum();
      putDataProcFp_FloatingPointComparison();
      putDataProcFp_FloatingPointConditionalSelect();
  }

  

    void putDataProcSimdFp()
    {
      /** Floating-point operations */
      putDataProcFp();
    
      /** SIMD operations */
      //putDataProcSimd();
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
