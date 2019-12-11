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
const uint64_t IMM12BIT  = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 2048, 4095 } */
const uint64_t IMM13BIT  = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 4096, 8191 } */
const uint64_t IMM16BIT  = 1ULL << flagBit++; /** Test vector is {0, 1, ..., 4096, 1<<13, 1<<14, 1<<15, 1<<16-1 } */
const uint64_t COND      = 1ULL << flagBit++; /** Test vector is { EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, AL } */
const uint64_t COND_WO_AL = 1ULL << flagBit++; /** Test vector is { EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE } */
const uint64_t NZCV       = 1ULL << flagBit++; /** Test vector is { 0, 1, ..., 15 } */
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
  std::vector<std::string> tv_WSP  = { "wsp" };
  std::vector<std::string> tv_XSP  = { "sp" };
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
  std::vector<std::string> tv_COND       = { "EQ", "NE", "CS", "CC", "MI", "PL", "VS", "VC", "HI", "LS", "GE", "LT", "GT", "LE", "AL" };
  std::vector<std::string> tv_COND_WO_AL = { "EQ", "NE", "CS", "CC", "MI", "PL", "VS", "VC", "HI", "LS", "GE", "LT", "GT", "LE" };
  std::vector<std::string> tv_NZCV     = { "15", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14" };
  std::vector<std::string> tv_BITMASK32, jtv_BITMASK32;
  std::vector<std::string> tv_BITMASK64, jtv_BITMASK64;
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



  
  std::vector<std::vector<std::string> *> tv_VectorsAs = { &tv_WREG, &tv_XREG, &tv_WSP, &tv_XSP, &tv_IMM4BIT, &tv_IMM5BIT, &tv_IMM6BIT,
							   &tv_IMM12BIT, &tv_IMM13BIT,
							   &tv_IMM16BIT, &tv_COND, &tv_COND_WO_AL, &tv_NZCV,
							   &tv_BITMASK32, &tv_BITMASK64,
							   &tv_LSL_IMM, &tv_LSL32, &tv_LSL64,
							   &tv_SPECIFIC32, &tv_SPECIFIC64, &tv_SPECIFIC32_1, &tv_SPECIFIC64_1,
							   &tv_SPECIFIC32_2, &tv_SPECIFIC64_2, &tv_SPECIFIC32_3, &tv_SPECIFIC64_3,
							   &tv_SHIFT_AMOUNT32, &tv_SHIFT_AMOUNT64,
							   &tv_EXT_AMOUNT32, &tv_EXT_AMOUNT64 };
  std::vector<std::vector<std::string> *> tv_VectorsJit = { &tv_WREG, &tv_XREG, &tv_WSP, &tv_XSP, &tv_IMM4BIT, &tv_IMM5BIT, &tv_IMM6BIT,
							    &tv_IMM12BIT, &tv_IMM13BIT,
							    &tv_IMM16BIT, &tv_COND, &tv_COND_WO_AL, &tv_NZCV,
							    &jtv_BITMASK32, &jtv_BITMASK64,
							    &jtv_LSL_IMM, &jtv_LSL32, &jtv_LSL64,
							    &jtv_SPECIFIC32, &jtv_SPECIFIC64, &jtv_SPECIFIC32_1, &jtv_SPECIFIC64_1,
							    &jtv_SPECIFIC32_2, &jtv_SPECIFIC64_2, &jtv_SPECIFIC32_3, &jtv_SPECIFIC64_3,
							    &jtv_SHIFT_AMOUNT32, &jtv_SHIFT_AMOUNT64,
							    &jtv_EXT_AMOUNT32, &jtv_EXT_AMOUNT64 };
  std::vector<std::vector<std::string> *>& tv_Vectors = tv_VectorsAs;

  
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
    std::stringstream jss;
    ss << std::hex << std::showbase;
    jss << std::hex << std::showbase;

    for(uint64_t onesLen=1; onesLen<=31; onesLen++) { // Inall-one bit is reserved.
      uint64_t bitmask = 0;

      for(uint64_t i=1; i<=onesLen; i++) {
	bitmask = (bitmask<<1) + 1;
      }

      for(uint64_t shift=0; shift<=32-onesLen; shift++) {
	ss.str("");
	ss << bitmask;
	tv_BITMASK32.push_back(ss.str() + "<<" + std::to_string(shift));
	jtv_BITMASK32.push_back(ss.str() + "<<" + std::to_string(shift));
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
	jss.str("");
	ss << bitmask;
	jss << "((uint64_t) " << bitmask;
	tv_BITMASK64.push_back(ss.str() + "<<" + std::to_string(shift));
	jtv_BITMASK64.push_back(jss.str() + ") <<" + std::to_string(shift));
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
  /** Data processing - immediate */
  /*** Arithmetic (immediate). */
  /**** ADD Add ADD (immediate) on page C6-686 ***/
  /**** ADDS Add and set flags ADDS (immediate) on page C6-693 */
  /**** SUB Subtract SUB (immediate) on page C6-1199 */
  /**** SUBS Subtract and set flags SUBS (immediate) on page C6-1206 */
  PUT4(DataProcImm_ArithmeticImmediate0,
       NM("add", "sub"),
       OPS(WREG_WSP, WREG_WSP, IMM12BIT), OPS(WREG_WSP, WREG_WSP, IMM12BIT, LSL_IMM),
       OPS(XREG_XSP, XREG_XSP, IMM12BIT), OPS(XREG_XSP, XREG_XSP, IMM12BIT, LSL_IMM));
  PUT4(DataProcImm_ArithmeticImmediate1,
       NM("adds", "subs"),
       OPS(WREG, WREG_WSP, IMM12BIT), OPS(WREG, WREG_WSP, IMM12BIT, LSL_IMM),
       OPS(XREG, XREG_XSP, IMM12BIT), OPS(XREG, XREG_XSP, IMM12BIT, LSL_IMM));
  /**** CMP Compare CMP (immediate) on page C6-778 */
  /**** CMN Compare negative CMN (immediate) on page C6-772 */
  PUT4(DataProcImm_ArithmeticImmediate2,
       NM("cmp", "cmn"),
       OPS(WREG, IMM12BIT), OPS(WREG, IMM12BIT, LSL_IMM),
       OPS(XREG, IMM12BIT), OPS(XREG, IMM12BIT, LSL_IMM));
  /*** Logical (immediate). */
  /**** AND Bitwise AND AND (immediate) on page C6-699 */
  /**** EOR Bitwise exclusive OR EOR (immediate) on page C6-812 */
  /**** ORR Bitwise inclusive OR ORR (immediate) on page C6-1031 */
  PUT2(DataProcImm_LogicalImmediate0,
       NM("and_", "eor", "orr"),
       OPS(WREG_WSP, WREG, BITMASK32), OPS(XREG_XSP, XREG, BITMASK64));
  /**** ANDS Bitwise AND and set flags ANDS (immediate) on page C6-703 */
  PUT2(DataProcImm_LogicalImmediate1,
       NM("ands"),
       OPS(WREG, WREG, BITMASK32), OPS(XREG, XREG, BITMASK64));
  /**** TST Test bits TST (immediate) on page C6-1230 */
  PUT2(DataProcImm_LogicalImmediate2,
       NM("tst"),
       OPS(WREG, BITMASK32), OPS(XREG, BITMASK64));
  /*** Move (wide immediate) on page C3-194. */
  /**** MOVZ Move wide with zero MOVZ on page C6-1009 */
  /**** MOVN Move wide with NOT MOVN on page C6-1007 */
  /**** MOVK Move wide with keep MOVK on page C6-1005 */
  PUT4(DataProcImm_MoveWideImmediate0,
       NM("movz", "movn", "movk"),
       OPS(WREG, IMM16BIT), OPS(WREG, IMM16BIT, LSL32),
       OPS(XREG, IMM16BIT), OPS(XREG, IMM16BIT, LSL64));
  /*** Move (immediate) on page C3-194. */
  /**** Move (inverted wide immediate) MOV (inverted wide immediate) on page C6-997 */
  /**** Move (wide immediate) MOV (wide immediate) on page C6-999 */
  /**** Move (bitmask immediate) MOV (bitmask immediate) on page C6-1001 */
  /*** PC-relative address calculation on page C3-195. */
  /**** ADRP Compute address of 4KB page at a PC-relative offset ADRP on page C6-698 */
  /**** ADR Compute address of label at a PC-relative offset. ADR on page C6-697 */
  /*** Bitfield move on page C3-195. */
  /**** BFM Bitfield move BFM on page C6-727 */
  /**** SBFM Signed bitfield move SBFM on page C6-1075 */
  /**** UBFM Unsigned bitfield move (32-bit) UBFM on page C6-1235 */
  PUT2(DataProcImm_BitfieldMove0,
       NM("bfm", "sbfm", "ubfm"),
       OPS(WREG, WREG, IMM5BIT, IMM5BIT), OPS(XREG, XREG, IMM6BIT, IMM6BIT));
  /*** Bitfield insert and extract on page C3-196 */
  /**** BFC Bitfield insert clear BFC on page C6-723 */
  PUT2(DataProcImm_BitfieldInsertAndExtract0,
       NM("bfc"),
       OPS(WREG, SPECIFIC32), OPS(XREG, SPECIFIC64));
  /**** BFI Bitfield insert BFI on page C6-725 */
  /**** BFXIL Bitfield extract and insert low BFXIL on page C6-729 */
  /**** SBFIZ Signed bitfield insert in zero SBFIZ on page C6-1073 */
  /**** SBFX Signed bitfield extract SBFX on page C6-1077 */
  /**** UBFIZ Unsigned bitfield insert in zero UBFIZ on page C6-1233 */
  /**** UBFX Unsigned bitfield extract UBFX on page C6-1237 */
  PUT2(DataProcImm_BitfieldInsertAndExtract1,
       NM("bfi", "bfxil", "sbfiz", "sbfx", "ubfiz", "ubfx"),
       OPS(WREG, WREG, SPECIFIC32), OPS(XREG, XREG, SPECIFIC64));
  /*** Extract register on page C3-196. */
  /**** EXTR Extract register from pair EXTR on page C6-819 */
  PUT2(DataProcImm_ExtractRegister0,
       NM("extr"),
       OPS(WREG, WREG, WREG, IMM5BIT), OPS(XREG, XREG, XREG, IMM6BIT));
  /*** Shift (immediate) on page C3-196. */
  /**** ASR Arithmetic shift right ASR (immediate) on page C6-709 */
  /**** LSL Logical shift left LSL (immediate) on page C6-982 */
  /**** LSR Logical shift right LSR (immediate) on page C6-988 */
  /**** ROR Rotate right ROR (immediate) on page C6-1063 */
  PUT2(DataProcImm_ShiftImmediate0,
       NM("asr", "lsl", "lsr", "ror"),
       OPS(WREG, WREG, IMM5BIT), OPS(XREG, XREG, IMM6BIT));
  /*** Sign-extend and Zero-extend on page C3-197. */
  /**** SXTB Sign-extend byte SXTB on page C6-1217 */
  /**** SXTH Sign-extend halfword SXTH on page C6-1219 */
  PUT2(DataProcImm_SignExtendAndZeroExtend0,
       NM("sxtb", "sxth"),
       OPS(WREG, WREG), OPS(XREG, WREG));
  /**** SXTW Sign-extend word SXTW on page C6-1221 */
  PUT1(DataProcImm_SignExtendAndZeroExtend1,
       NM("sxtw"),
       OPS(XREG, WREG));
  /**** UXTB Unsigned extend byte UXTB on page C6-1248 */
  /**** UXTH Unsigned extend halfword UXTH on page C6-1249 */
  PUT1(DataProcImm_SignExtendAndZeroExtend2,
       NM("uxtb", "uxth"),
       OPS(WREG, WREG));
  
  
  
  void putDataProcImm_ArithmeticImmediate() {
    tv_LSL_IMM.clear();
    tv_LSL_IMM.push_back("LSL 0");
    tv_LSL_IMM.push_back("LSL 12");
    
    jtv_LSL_IMM.clear();
    jtv_LSL_IMM.push_back("0");
    jtv_LSL_IMM.push_back("12");
    
    putDataProcImm_ArithmeticImmediate0();
    putDataProcImm_ArithmeticImmediate1();
    putDataProcImm_ArithmeticImmediate2();
  }
  
  void putDataProcImm_LogicalImmediate() {
    putDataProcImm_LogicalImmediate0();
    putDataProcImm_LogicalImmediate1();
    putDataProcImm_LogicalImmediate2();
  }
  
  void putDataProcImm_MoveWideImmediate() {
    tv_LSL32.clear();
    tv_LSL32.push_back("LSL 0");
    tv_LSL32.push_back("LSL 16");
    
    jtv_LSL32.clear();
    jtv_LSL32.push_back("0");
    jtv_LSL32.push_back("16");
    
    tv_LSL64.clear();
    tv_LSL64.push_back("LSL 0");
    tv_LSL64.push_back("LSL 16");
    tv_LSL64.push_back("LSL 32");
    tv_LSL64.push_back("LSL 48");
    
    jtv_LSL64.clear();
    jtv_LSL64.push_back("0");
    jtv_LSL64.push_back("16");
    jtv_LSL64.push_back("32");
    jtv_LSL64.push_back("48");
    
    putDataProcImm_MoveWideImmediate0();
  }
  
  void putDataProcImm_MoveImmediate() {
  }
  
  void putDataProcImm_BitfieldMove() {
    putDataProcImm_BitfieldMove0();
  }
  
  void putDataProcImm_BitfieldInsertAndExtract() {
    tv_SPECIFIC32.clear();
    jtv_SPECIFIC32.clear();
    for(uint64_t i=1; i<=31; i++) {
      tv_SPECIFIC32.push_back(std::to_string(i) + ", " + std::to_string(32-i));
      jtv_SPECIFIC32.push_back(std::to_string(i) + ", " + std::to_string(32-i));
    }

    tv_SPECIFIC64.clear();
    jtv_SPECIFIC64.clear();
    for(uint64_t i=1; i<=63; i++) {
      tv_SPECIFIC64.push_back(std::to_string(i) + ", " + std::to_string(64-i));
      jtv_SPECIFIC64.push_back(std::to_string(i) + ", " + std::to_string(64-i));
    }

    putDataProcImm_BitfieldInsertAndExtract0();
    putDataProcImm_BitfieldInsertAndExtract1();
  }

  void putDataProcImm_ExtractRegister() {
    putDataProcImm_ExtractRegister0();
  }

  void putDataProcImm_ShiftImmediate() {
    putDataProcImm_ShiftImmediate0();
  }

  void putDataProcImm_SignExtendAndZeroExtend() {
    putDataProcImm_SignExtendAndZeroExtend0();
    putDataProcImm_SignExtendAndZeroExtend1();
    putDataProcImm_SignExtendAndZeroExtend2();
  }    
  
  void putDataProcImm() {
    putDataProcImm_ArithmeticImmediate();
    putDataProcImm_LogicalImmediate();
    putDataProcImm_MoveWideImmediate();
    //    putDataProcImm_MoveImmediate(); // T.B.D. 
    putDataProcImm_BitfieldMove();
    putDataProcImm_BitfieldInsertAndExtract();
    putDataProcImm_ExtractRegister();
    putDataProcImm_ShiftImmediate();
    putDataProcImm_SignExtendAndZeroExtend();
  }

  /** C3.4 Data processing - register */
  /*** Arithmetic (shifted register). */
  /**** ADD Add ADD (shifted register) on page C6-688 */
  /**** ADDS Add and set flags ADDS (shifted register) on page C6-695 */
  /**** SUB Subtract SUB (shifted register) on page C6-1201 */
  /**** SUBS Subtract and set flags SUBS (shifted register) on page C6-1208 */
  PUT4(DataProcReg_ArithmeticShiftedRegister0,
       NM("add", "adds", "sub", "subs"),
       OPS(WREG, WREG, WREG), OPS(WREG, WREG, WREG, SHIFT_AMOUNT32),
       OPS(XREG, XREG, XREG), OPS(XREG, XREG, XREG, SHIFT_AMOUNT64));
  /**** CMN Compare negative CMN (shifted register) on page C6-774 */
  /**** CMP Compare CMP (shifted register) on page C6-780 */
  /**** NEG Negate NEG (shifted register) on page C6-1020 */
  /**** NEGS Negate and set flags NEGS on page C6-1022 */
  PUT4(DataProcReg_ArithmeticShiftedRegister1,
       NM("cmn", "cmp", "neg", "negs"),
       OPS(WREG, WREG), OPS(WREG, WREG, SHIFT_AMOUNT32),
       OPS(XREG, XREG), OPS(XREG, XREG, SHIFT_AMOUNT64));


  void putDataProcReg_ArithmeticShiftedRegister() {
    setShiftAmount();
    
    putDataProcReg_ArithmeticShiftedRegister0();
    putDataProcReg_ArithmeticShiftedRegister1();
  }

  /*** Arithmetic (extended register) on page C3-199. */
  /**** ADD Add ADD (extended register) on page C6-683 */
  /***** ADD <Wd|WSP>, <Wn|WSP>, <Wm>{, <extend> {#<amount>}} */
  /**** SUB Subtract SUB (extended register) on page C6-1196 */
  /***** SUB <Wd|WSP>, <Wn|WSP>, <Wm>{, <extend> {#<amount>}} */
  PUT6(DataProcReg_ArithmeticExtendedRegister0,
       NM("add", "sub"),
       OPS(WREG, WREG, WREG), OPS(WREG, WREG, WREG, SPECIFIC32), 
       OPS(XREG, XREG, XREG), OPS(XREG, XREG, XREG, SPECIFIC64),
       OPS(SPECIFIC32_1), OPS(SPECIFIC64_1)); 
  /**** ADDS Add and set flags ADDS (extended register) on page C6-690 */
  /***** ADDS <Wd>, <Wn|WSP>, <Wm>{, <extend> {#<amount>}} */
  /**** SUBS Subtract and set flags SUBS (extended register) on page C6-1203 */
  /***** SUBS <Wd>, <Wn|WSP>, <Wm>{, <extend> {#<amount>}} */
  PUT6(DataProcReg_ArithmeticExtendedRegister1,
       NM("adds", "subs"),
       OPS(WREG, WREG, WREG), OPS(WREG, WREG, WREG, SPECIFIC32), 
       OPS(XREG, XREG, XREG), OPS(XREG, XREG, XREG, SPECIFIC64),
       OPS(SPECIFIC32_2), OPS(SPECIFIC64_2));
  /**** CMN Compare negative CMN (extended register) on page C6-770 */
  /***** CMN <Wn|WSP>, <Wm>{, <extend> {#<amount>}} */
  /**** CMP Compare CMP (extended register) on page C6-776 */
  /***** CMP <Wn|WSP>, <Wm>{, <extend> {#<amount>}} */
  PUT6(DataProcReg_ArithmeticExtendedRegister2,
       NM("cmn", "cmp"),
       OPS(WREG, WREG), OPS(WREG, WREG, SPECIFIC32),
       OPS(XREG, XREG), OPS(XREG, XREG, SPECIFIC64),
       OPS(SPECIFIC32_3), OPS(SPECIFIC64_3));
       
  void putDataProcReg_ArithmeticExtendedRegister() {
    std::vector<std::string> tmpExt = {"UXTB", "UXTH", "UXTW", "UXTX", "SXTB", "SXTH", "SXTW", "SXTX" };
    std::vector<std::string> tmpPtn = {"3", "0", "1", "2" }; 
    
    tv_SPECIFIC32.clear();
    tv_SPECIFIC64.clear();
    jtv_SPECIFIC32.clear();
    jtv_SPECIFIC64.clear();

    for(std::string j : tmpExt) {
      for(std::string i : tmpPtn) {
	tv_SPECIFIC32.push_back(j + " " + i);
	jtv_SPECIFIC32.push_back(j + ", " + i);

	tv_SPECIFIC64.push_back(j + " " + i);
	jtv_SPECIFIC64.push_back(j + ", " + i);
      }
    }
    for(std::string j : tmpExt) { /** <amount> is optional when <extend> is present but not LSL. */
      tv_SPECIFIC32.push_back(j);
      jtv_SPECIFIC32.push_back(j);
      
      tv_SPECIFIC64.push_back(j);
      jtv_SPECIFIC64.push_back(j);
    }
    
    tv_SPECIFIC32_1.clear();
    tv_SPECIFIC64_1.clear();
    jtv_SPECIFIC32_1.clear();
    jtv_SPECIFIC64_1.clear();
    /** 32-bit variant:
	If "Rd" or "Rn" is '11111' (WSP) and "option" is '010' then LSL is preferred, but may be omitted
	when "imm3" is '000'. In all other cases <extend> is required and must be UXTW when "option" is
	'010'. */
    /** 64-bit variant:
	If "Rd" or "Rn" is '11111' (SP) and "option" is '011' then LSL is preferred, but may be omitted when
	"imm3" is '000'. In all other cases <extend> is required and must be UXTX when "option" is '011'. */
    tmpPtn = {"3", "0", "1", "2" };
    for(std::string i : tmpPtn) {
      tv_SPECIFIC32_1.push_back("wsp, w3, w5, LSL " + i);
      tv_SPECIFIC32_1.push_back("w3, wsp, w5, LSL " + i);
      tv_SPECIFIC32_1.push_back("wsp, wsp, w5, LSL " + i);
      jtv_SPECIFIC32_1.push_back("wsp, w3, w5, LSL, " + i);
      jtv_SPECIFIC32_1.push_back("w3, wsp, w5, LSL, " + i);
      jtv_SPECIFIC32_1.push_back("wsp, wsp, w5, LSL, " + i);

      tv_SPECIFIC64_1.push_back("sp, x3, x5, LSL " + i);
      tv_SPECIFIC64_1.push_back("x3, sp, x5, LSL " + i);
      tv_SPECIFIC64_1.push_back("sp, sp, x5, LSL " + i);
      jtv_SPECIFIC64_1.push_back("sp, x3, x5, LSL, " + i);
      jtv_SPECIFIC64_1.push_back("x3, sp, x5, LSL, " + i);
      jtv_SPECIFIC64_1.push_back("sp, sp, x5, LSL, " + i);
    }

    tv_SPECIFIC32_2.clear();
    tv_SPECIFIC64_2.clear();
    jtv_SPECIFIC32_2.clear();
    jtv_SPECIFIC64_2.clear();
    for(std::string i : tmpPtn) {
      tv_SPECIFIC32_2.push_back("w3, wsp, w5, LSL " + i);
      jtv_SPECIFIC32_2.push_back("w3, wsp, w5, LSL, " + i);

      tv_SPECIFIC64_2.push_back("x3, sp, x5, LSL " + i);
      jtv_SPECIFIC64_2.push_back("x3, sp, x5, LSL, " + i);
    }

    tv_SPECIFIC32_3.clear();
    tv_SPECIFIC64_3.clear();
    jtv_SPECIFIC32_3.clear();
    jtv_SPECIFIC64_3.clear();
    for(std::string i : tmpPtn) {
      tv_SPECIFIC32_3.push_back("wsp, w5, LSL " + i);
      jtv_SPECIFIC32_3.push_back("wsp, w5, LSL, " + i);

      tv_SPECIFIC64_3.push_back("sp, x5, LSL " + i);
      jtv_SPECIFIC64_3.push_back("sp, x5, LSL, " + i);
    }

    
    putDataProcReg_ArithmeticExtendedRegister0();
    putDataProcReg_ArithmeticExtendedRegister1();
    putDataProcReg_ArithmeticExtendedRegister2();
  }


  /*** Arithmetic with carry on page C3-200. */
  /**** ADC Add with carry ADC on page C6-679 */
  /**** ADCS Add with carry and set flags ADCS on page C6-681 */
  /**** SBC Subtract with carry SBC on page C6-1069 */
  /**** SBCS Subtract with carry and set flags SBCS on page C6-1071 */
  PUT2(DataProcReg_ArithmeticWithCarry0,
       NM("adc", "adcs", "sbc", "sbcs"),
       OPS(WREG, WREG, WREG), OPS(XREG, XREG, XREG));
  /**** NGC Negate with carry NGC on page C6-1024 */
  /**** NGCS Negate with carry and set flags NGCS on page C6-1026 */
  PUT2(DataProcReg_ArithmeticWithCarry1,
       NM("ngc", "ngcs"),
       OPS(WREG, WREG), OPS(XREG, XREG));
  
  void putDataProcReg_ArithmeticWithCarry() {
    putDataProcReg_ArithmeticWithCarry0();
  }

  /*** Flag manipulation instructions on page C3-200. */
  /**** CFINV Invert value of the PSTATE.C bit CFINV on page C6-762 */
  PUT1(DataProcReg_FlagManipulationInstructions0,
       NM("cfinv"),
       OPS());
  /**** RMIF Rotate, mask insert flags RMIF on page C6-1062 */
  PUT1(DataProcReg_FlagManipulationInstructions1,
       NM("rmif"),
       OPS(XREG, IMM6BIT, IMM4BIT));
  /**** SETF8 Evaluation of 8-bit flags SETF8, SETF16 on page C6-1080 */
  /**** SETF16 Evaluation of 16-bit flags SETF8, SETF16 on page C6-1080 */
    PUT1(DataProcReg_FlagManipulationInstructions2,
	 NM("setf8", "setf16"),
	 OPS(WREG));

  void putDataProcReg_FlagManipulationInstructions() {
    putDataProcReg_FlagManipulationInstructions0();
    putDataProcReg_FlagManipulationInstructions1();
    putDataProcReg_FlagManipulationInstructions2();
  }
  
  /*** Logical (shifted register) on page C3-200. */
  /**** AND Bitwise AND AND (shifted register) on page C6-701 */
  /**** ANDS Bitwise AND and set flags ANDS (shifted register) on page C6-705 */
  /**** BIC Bitwise bit clear BIC (shifted register) on page C6-731 */
  /**** BICS Bitwise bit clear and set flags BICS (shifted register) on page C6-733 */
  /**** EON Bitwise exclusive OR NOT EON (shifted register) on page C6-810 */
  /**** EOR Bitwise exclusive OR EOR (shifted register) on page C6-814 */
  /**** ORR Bitwise inclusive OR ORR (shifted register) on page C6-1033 */
  /**** ORN Bitwise inclusive OR NOT ORN (shifted register) on page C6-1029 */
  PUT4(DataProcReg_LogicalShiftedRegister0,
       NM("and_", "ands", "bic", "bics", "eon", "eor", "orr", "orn"),
       OPS(WREG, WREG, WREG), OPS(WREG, WREG, WREG, SHIFT_AMOUNT32),
       OPS(XREG, XREG, XREG), OPS(XREG, XREG, XREG, SHIFT_AMOUNT64));
  /**** MVN Bitwise NOT MVN on page C6-1018 */
  /**** TST Test bits TST (shifted register) on page C6-1231 */
  PUT4(DataProcReg_LogicalShiftedRegister1,
       NM("mvn", "tst"),
       OPS(WREG, WREG), OPS(WREG, WREG, SHIFT_AMOUNT32),
       OPS(XREG, XREG), OPS(XREG, XREG, SHIFT_AMOUNT64));

  void putDataProcReg_LogicalShiftedRegister() {
    setShiftAmountWithRor();

    putDataProcReg_LogicalShiftedRegister0();
    putDataProcReg_LogicalShiftedRegister1();
  }
	 
  /*** Move (register) on page C3-201. */
  /**** Move register MOV (register) on page C6-1003 */
  /**** Move register to SP or move SP to register MOV (to/from SP) on page C6-996 */
  PUT2(DataProcReg_MoveRegister0,
       NM("mov"),
       OPS(WREG_WSP, WREG_WSP), OPS(XREG_XSP, XREG_XSP));

  void putDataProcReg_MoveRegister() {
    putDataProcReg_MoveRegister0();
  }

  /*** Shift (register) on page C3-201. */
  /**** ASRV Arithmetic shift right variable ASRV on page C6-711 */
  /**** LSLV Logical shift left variable LSLV on page C6-984 */
  /**** LSRV Logical shift right variable LSRV on page C6-990 */
  /**** RORV Rotate right variable RORV on page C6-1067 */
  /**** ASR Arithmetic shift right ASR (register) on page C6-707 */
  /**** LSL Logical shift left LSL (register) on page C6-980 */
  /**** LSR Logical shift right LSR (register) on page C6-986 */
  /**** ROR Rotate right ROR (register) on page C6-1065 */
  PUT2(DataProcReg_ShiftRegister0,
       NM("asrv", "lslv", "lsrv", "rorv", "asr", "lsl", "lsr", "ror"),
       OPS(WREG, WREG, WREG), OPS(XREG, XREG, XREG));
  
  void putDataProcReg_ShiftRegister() {
    putDataProcReg_ShiftRegister0();
  }

  /*** Multiply and divide on page C3-202. */
  /**** MADD Multiply-add MADD on page C6-992 */
  /**** MSUB Multiply-subtract MSUB on page C6-1015 */
  PUT2(DataProcReg_MultiplyAndDivide0,
       NM("madd", "msub"),
       OPS(WREG, WREG, WREG, WREG), OPS(XREG, XREG, XREG, XREG));
  /**** MNEG Multiply-negate MNEG on page C6-994 */
  /**** MUL Multiply MUL on page C6-1017 */
  /**** SDIV Signed divide SDIV on page C6-1079 */
  /**** UDIV Unsigned divide UDIV on page C6-1240 */
  PUT2(DataProcReg_MultiplyAndDivide1,
       NM("mneg", "mul", "sdiv", "udiv"),
       OPS(WREG, WREG, WREG), OPS(XREG, XREG, XREG));
  /**** SMADDL Signed multiply-add long SMADDL on page C6-1083 */
  /**** SMSUBL Signed multiply-subtract long SMSUBL on page C6-1087 */
  /**** UMADDL Unsigned multiply-add long UMADDL on page C6-1241 */
  /**** UMSUBL Unsigned multiply-subtract long UMSUBL on page C6-1244 */
  PUT1(DataProcReg_MultiplyAndDivide2,
       NM("smaddl", "smsubl", "umaddl", "umsubl"),
       OPS(XREG, WREG, WREG, XREG));
  /**** SMNEGL Signed multiply-negate long SMNEGL on page C6-1086 */
  /**** SMULL Signed multiply long SMULL on page C6-1090 */
  /**** UMNEGL Unsigned multiply-negate long UMNEGL on page C6-1243 */
  /**** UMULL Unsigned multiply long UMULL on page C6-1247 */
  PUT1(DataProcReg_MultiplyAndDivide3,
       NM("smnegl", "smull", "umnegl", "umull"),
       OPS(XREG, WREG, WREG));
  /**** SMULH Signed multiply high SMULH on page C6-1089 */
  /**** UMULH Unsigned multiply high UMULH on page C6-1246 */
  PUT1(DataProcReg_MultiplyAndDivide4,
       NM("smulh", "umulh"),
       OPS(XREG, XREG, XREG));

  void putDataProcReg_MultiplyAndDivide() {
    putDataProcReg_MultiplyAndDivide0();
    putDataProcReg_MultiplyAndDivide1();
    putDataProcReg_MultiplyAndDivide2();
    putDataProcReg_MultiplyAndDivide3();
    putDataProcReg_MultiplyAndDivide4();
  }

  /*** CRC32 on page C3-203. */
  /**** CRC32B CRC-32 sum from byte CRC32B, CRC32H, CRC32W, CRC32X on page C6-784 */
  /**** CRC32H CRC-32 sum from halfword CRC32B, CRC32H, CRC32W, CRC32X on page C6-784 */
  /**** CRC32W CRC-32 sum from word CRC32B, CRC32H, CRC32W, CRC32X on page C6-784 */
  /**** CRC32CB CRC-32C sum from byte CRC32CB, CRC32CH, CRC32CW, CRC32CX on page C6-786 */
  /**** CRC32CH CRC-32C sum from halfword CRC32CB, CRC32CH, CRC32CW, CRC32CX on page C6-786 */
  /**** CRC32CW CRC-32C sum from word CRC32CB, CRC32CH, CRC32CW, CRC32CX on page C6-786 */
  PUT1(DataProcReg_CRC320,
       NM("crc32b", "crc32h", "crc32w", "crc32cb", "crc32ch", "crc32cw"),
       OPS(WREG, WREG, WREG));
  /**** CRC32X CRC-32 sum from doubleword CRC32B, CRC32H, CRC32W, CRC32X on page C6-784 */
  /**** CRC32CX CRC-32C sum from doubleword CRC32CB, CRC32CH, CRC32CW, CRC32CX on page C6-786 */
  PUT1(DataProcReg_CRC321,
       NM("crc32x", "crc32cx"),
       OPS(WREG, WREG, XREG));

  void putDataProcReg_CRC32() {
    putDataProcReg_CRC320();
    putDataProcReg_CRC321();
  }
  
  /*** Bit operation on page C3-204. */
  /**** CLS Count leading sign bits CLS on page C6-768 */
  /**** CLZ Count leading zero bits CLZ on page C6-769 */
  /**** RBIT Reverse bit order RBIT on page C6-1052 */
  /**** REV Reverse bytes in register REV on page C6-1055 */
  /**** REV16 Reverse bytes in halfwords REV16 on page C6-1057 */
  PUT2(DataProcReg_BitOperation0,
       NM("cls", "clz", "rbit", "rev", "rev16"),
       OPS(WREG, WREG), OPS(XREG, XREG));
  /**** REV64 Reverse bytes in register REV64 on page C6-1061 */
  /**** REV32 Reverses bytes in words REV32 on page C6-1059 */
  PUT1(DataProcReg_BitOperation1,
       NM("rev32", "rev64"),
       OPS(XREG, XREG));
  
  void putDataProcReg_BitOperation() {
    putDataProcReg_BitOperation0();
    putDataProcReg_BitOperation1();
  }

  /*** Conditional select on page C3-204. */
  /**** CSEL Conditional select CSEL on page C6-789 */
  /**** CSINC Conditional select increment CSINC on page C6-795 */
  /**** CSINV Conditional select inversion CSINV on page C6-797 */
  /**** CSNEG Conditional select negation CSNEG on page C6-799 */
  PUT2(DataProcReg_ConditionalSelect0,
       NM("csel", "csinc", "csinv", "csneg"),
       OPS(WREG, WREG, WREG, COND), OPS(XREG, XREG, XREG, COND));
  /**** CSET Conditional set CSET on page C6-791 */
  /**** CSETM Conditional set mask CSETM on page C6-793 */
  PUT2(DataProcReg_ConditionalSelect1,
	 NM("cset", "csetm"),
	 OPS(WREG, COND_WO_AL), OPS(XREG, COND_WO_AL));
  /**** CINC Conditional increment CINC on page C6-763 */
  /**** CINV Conditional invert CINV on page C6-765 */
  /**** CNEG Conditional negate CNEG on page C6-782 */
  PUT2(DataProcReg_ConditionalSelect2,
	 NM("cinc", "cinv", "cneg"),
	 OPS(WREG, WREG, COND_WO_AL), OPS(XREG, XREG, COND_WO_AL));

  void putDataProcReg_ConditionalSelect() {
    putDataProcReg_ConditionalSelect0();
    putDataProcReg_ConditionalSelect1();
    putDataProcReg_ConditionalSelect2();
  }

  /*** Conditional comparison on page C3-204. */
  /**** CCMN Conditional compare negative (register) CCMN (register) on page C6-756 */
  /**** CCMP Conditional compare (register) CCMP (register) on page C6-760 */
  PUT2(DataProcReg_ConditionalComparison0,
       NM("ccmn", "ccmp"),
       OPS(WREG, WREG, NZCV, COND), OPS(XREG, XREG, NZCV, COND));
  /**** CCMN Conditional compare negative (immediate) CCMN (immediate) on page C6-754 */
  /**** CCMP Conditional compare (immediate) CCMP (immediate) on page C6-758  */ 
  PUT2(DataProcReg_ConditionalComparison1,
       NM("ccmn", "ccmp"),
       OPS(WREG, IMM5BIT, NZCV, COND), OPS(XREG, IMM5BIT, NZCV, COND));

  void putDataProcReg_ConditionalComparison() {
    putDataProcReg_ConditionalComparison0();
    putDataProcReg_ConditionalComparison1();
  }

  void putDataProcReg() {
    putDataProcReg_ArithmeticShiftedRegister();
    putDataProcReg_ArithmeticExtendedRegister();
    putDataProcReg_ArithmeticWithCarry();
    /** putDataProcReg_FlagManipulationInstructions();*/ /** CFINV, RMIF, SETF8, SETF16 are ARMv8.4 instructions not supported by A64FX. */
    putDataProcReg_LogicalShiftedRegister();
    putDataProcReg_MoveRegister();
    putDataProcReg_ShiftRegister();
    putDataProcReg_MultiplyAndDivide();
    putDataProcReg_CRC32();
    putDataProcReg_BitOperation();
    putDataProcReg_ConditionalSelect();
    putDataProcReg_ConditionalComparison();
  }

  
  
    void put()
    {
      putDataProcImm();
      putDataProcReg();
    
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
