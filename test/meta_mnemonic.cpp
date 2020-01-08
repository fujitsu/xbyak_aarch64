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
#define TEST_NUM 1

#include <cstring>
#include "../xbyak_aarch64/xbyak_aarch64.h"

using namespace Xbyak;

class GeneratorAdd : public CodeGenerator {

  template<typename T>
  void genAddFuncCore(const T imm) {
    add_imm(x0, x0, imm, x9, x10); // x9, x10 are temporary registers.
    ret();
  }
  
public:
  template<typename T>
  GeneratorAdd(T i) { genAddFuncCore(i); }
};

class GeneratorSub : public CodeGenerator {

  template<typename T>
  void genSubFuncCore(const T imm) {
    sub_imm(x0, x0, imm, x9, x10); // x9, x10 are temporary registers.
    ret();
  }
  
public:
  template<typename T>
  GeneratorSub(T i) { genSubFuncCore(i); }
};


template<typename T>
void test_add(std::vector<T>& v) {


  for(const auto& e : v) {
    GeneratorAdd s(e);
    s.ready();
    uint64_t (*f)(T a) = (uint64_t (*)(T a))s.getCode();
    uint64_t retVal = f(TEST_NUM);

    if(typeid(e) == typeid(int32_t) || typeid(e) == typeid(int64_t)) {
      int64_t tmp = TEST_NUM + static_cast<int64_t>(e);
      //    std::cout << std::hex << "e  :" << e << std::endl;
      //    std::cout << std::hex << "tmp:" << tmp << std::endl;
      //    std::cout << std::hex << "ret:" << retVal << std::endl;
      std::cout << "add:" << TEST_NUM << " + " << e << " = " << static_cast<int64_t>(retVal) << std::endl;
      assert(std::memcmp(&retVal, &tmp, sizeof(uint64_t))==0);
    } else {
      uint64_t tmp = TEST_NUM + static_cast<uint64_t>(e);
      //    std::cout << std::hex << "e  :" << e << std::endl;
      //    std::cout << std::hex << "tmp:" << tmp << std::endl;
      //    std::cout << std::hex << "ret:" << retVal << std::endl;
      std::cout << "add:" << TEST_NUM << " + " << e << " = " << static_cast<int64_t>(retVal) << std::endl;
      assert(std::memcmp(&retVal, &tmp, sizeof(uint64_t))==0);
    }
  }
}

template<typename T>
void test_sub(std::vector<T>& v) {


  for(const auto& e : v) {
    GeneratorSub s(e);
    s.ready();
    uint64_t (*f)(T a) = (uint64_t (*)(T a))s.getCode();
    uint64_t retVal = f(TEST_NUM);

    if(typeid(e) == typeid(int32_t) || typeid(e) == typeid(int64_t)) {
      int64_t tmp = TEST_NUM - static_cast<int64_t>(e);
      //    std::cout << std::hex << "e  :" << e << std::endl;
      //    std::cout << std::hex << "tmp:" << tmp << std::endl;
      //    std::cout << std::hex << "ret:" << retVal << std::endl;
      std::cout << "sub:" << std::hex << TEST_NUM << " - " << e << " = " << static_cast<int64_t>(retVal) << std::endl;
      assert(std::memcmp(&retVal, &tmp, sizeof(uint64_t))==0);
    } else {
      uint64_t tmp = TEST_NUM - static_cast<uint64_t>(e);
      //    std::cout << std::hex << "e  :" << e << std::endl;
      //    std::cout << std::hex << "tmp:" << tmp << std::endl;
      //    std::cout << std::hex << "ret:" << retVal << std::endl;
      std::cout << "sub:" << std::hex << TEST_NUM << " - " << e << " = " << static_cast<int64_t>(retVal) << std::endl;
      assert(std::memcmp(&retVal, &tmp, sizeof(uint64_t))==0);
    }
  }
}

int main() {

  std::vector<int32_t> v_int32 = {
    std::numeric_limits<int32_t>::min(),
    std::numeric_limits<int32_t>::min() + 1,
    -2048, -2047, -2046, -1, 
    0,
    1, 2046, 2047, 2048,
    std::numeric_limits<int32_t>::max() - 1,
    std::numeric_limits<int32_t>::max()
  };

  std::vector<uint32_t> v_uint32 = {
    0,
    1, 2046, 2047, 2048, 2049,
    std::numeric_limits<uint32_t>::max() - 1,
    std::numeric_limits<uint32_t>::max()
  };

  std::vector<int64_t> v_int64 = {
    std::numeric_limits<int64_t>::min(),
    std::numeric_limits<int64_t>::min() + 1,
    -2048, -2047, -2046, -1,
    0,
    1, 2046, 2047, 2048,
    std::numeric_limits<int64_t>::max() - 1,
    std::numeric_limits<int64_t>::max()
  };
  
  std::vector<uint64_t> v_uint64_t = {
    0,
    1, 2046, 2047, 2048, 2049,
    std::numeric_limits<uint64_t>::max() - 1,
    std::numeric_limits<uint64_t>::max()
  };

  test_add(v_int32);
  test_add(v_uint32);
  test_add(v_int64);
  test_add(v_uint64_t);

  test_sub(v_int32);
  test_sub(v_uint32);
  test_sub(v_int64);
  test_sub(v_uint64_t);


  return 0;
}
