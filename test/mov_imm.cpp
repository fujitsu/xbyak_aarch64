/*******************************************************************************
 * Copyright 2019-2020 FUJITSU LIMITED
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
#include "../xbyak_aarch64/xbyak_aarch64.h"
#include <cstring>
#include <iostream>
#include <type_traits>
#include <typeinfo>

using namespace Xbyak_aarch64;

class GeneratorMovImm : public CodeGeneratorAArch64 {

public:
  GeneratorMovImm() {}

  template <typename T,
            typename std::enable_if<std::is_unsigned<T>::value,
                                    std::nullptr_t>::type = nullptr,
            typename U>
  bool genMovFuncCore(const T imm, const U &rd) {
    if (rd.getBit() == 32 && sizeof(T) == 8 &&
        (std::numeric_limits<uint32_t>::max() < imm)) {
      return false;
    }

    mov_imm(rd, imm);
    ret();
    return true;
  }

  template <typename T,
            typename std::enable_if<std::is_signed<T>::value,
                                    std::nullptr_t>::type = nullptr,
            typename U>
  bool genMovFuncCore(const T imm, const U &rd) {
    if (rd.getBit() == 32 && sizeof(T) == 8 &&
        ((imm < std::numeric_limits<int32_t>::min()) ||
         (std::numeric_limits<int32_t>::max() < imm))) {
      return false;
    }

    mov_imm(rd, imm);
    ret();
    return true;
  }
};

template <typename T, typename U>
uint32_t test_mov_imm(std::vector<T> &v, const U &rd) {
  uint32_t errCount = 0;

  std::cerr << "##############################" << std::endl;
  if (rd.getBit() == 32) {
    std::cerr << "# WReg, ";
  } else {
    std::cerr << "# XReg, ";
  }
  if (std::is_signed<T>::value == true) {
    std::cerr << "int" << (sizeof(T) == 4 ? "32" : "64") << "_t" << std::endl;
  } else {
    std::cerr << "uint" << (sizeof(T) == 4 ? "32" : "64") << "_t" << std::endl;
  }
  std::cerr << "##############################" << std::endl;

  std::cerr << std::hex;
  for (const auto &e : v) {
    GeneratorMovImm s;

    if (s.genMovFuncCore(e, rd)) {
      s.ready();

      /* Dump jit code to stdout. */
      char *f = reinterpret_cast<char *>((uint32_t(*)())s.getCode());
      std::cout.write(f, 4 * s.getSize());

      /* Dump jit code execution result to stderr. */
      T (*g)() = (T(*)())s.getCode();
      if (rd.getBit() == 32) {
        uint32_t expData = e & 0xffffffff;
        uint32_t retVal = g();
        std::cerr << "expect=" << expData << ", result=" << retVal;
        if (expData == retVal) {
          std::cerr << ", OK" << std::endl;
        } else {
          std::cerr << ", NG" << std::endl;
          ++errCount;
        }
      } else {
        uint64_t expData = static_cast<uint64_t>(e) & (~uint64_t(0));
        uint64_t retVal = g();
        std::cerr << "expect=" << expData << ", result=" << retVal;
        if (expData == retVal) {
          std::cerr << ", OK" << std::endl;
        } else {
          std::cerr << ", NG" << std::endl;
          ++errCount;
        }
      }
    } else {
      std::cerr << "expect=" << e << ", result=, SKIPPED" << std::endl;
    }
  }

  std::cerr << std::endl;
  return errCount;
}

int main() {
  uint32_t ptnCount = 0;
  uint32_t errCount = 0;

  std::vector<int32_t> v_int32 = {std::numeric_limits<int32_t>::min(),
                                  std::numeric_limits<int32_t>::min() + 1,
                                  -2048,
                                  -2047,
                                  -2046,
                                  -1,
                                  0,
                                  1,
                                  2046,
                                  2047,
                                  2048,
                                  std::numeric_limits<int32_t>::max() - 1,
                                  std::numeric_limits<int32_t>::max(),
                                  int32_t(0x7654abcd),
                                  int32_t(0x0007ff00),
                                  int32_t(0x000007ff),
                                  int32_t(0x7ff00000)};

  std::vector<uint32_t> v_uint32 = {0,
                                    1,
                                    2046,
                                    2047,
                                    2048,
                                    2049,
                                    std::numeric_limits<uint32_t>::max() - 1,
                                    std::numeric_limits<uint32_t>::max(),
                                    uint32_t(0x7654abcd),
                                    uint32_t(0x0007ff00),
                                    uint32_t(0x000007ff),
                                    uint32_t(0xfff00000)};

  std::vector<int64_t> v_int64 = {std::numeric_limits<int64_t>::min(),
                                  std::numeric_limits<int64_t>::min() + 1,
                                  -2048,
                                  -2047,
                                  -2046,
                                  -1,
                                  0,
                                  1,
                                  2046,
                                  2047,
                                  2048,
                                  std::numeric_limits<int64_t>::max() - 1,
                                  std::numeric_limits<int64_t>::max(),
                                  int64_t(0x7654abcdef098765),
                                  int64_t(0x0007ff0000000000),
                                  int64_t(0x00000000000007ff),
                                  int64_t(0x7ff0000000000000)};

  std::vector<uint64_t> v_uint64 = {0,
                                    1,
                                    2046,
                                    2047,
                                    2048,
                                    2049,
                                    std::numeric_limits<uint64_t>::max() - 1,
                                    std::numeric_limits<uint64_t>::max(),
                                    uint64_t(0x7654abcdef098765),
                                    uint64_t(0x0007ff0000000000),
                                    uint64_t(0x00000000000007ff),
                                    uint64_t(0xfff0000000000000)};

  std::vector<uint32_t> v_bitmask32 = {
      uint32_t(0xfff00000),      uint32_t(0xffff) << 0,
      uint32_t(0xffff) << 1,     uint32_t(0xffff) << 6,
      uint32_t(0xffff) << 18,    uint32_t(0xffff) << 30,
      uint32_t(0xffff) << 31,    ~uint32_t(0xffff) << 0,
      ~uint32_t(0xffff) << 1,    ~(uint32_t(0xffff) << 6),
      ~(uint32_t(0xffff) << 18), ~(uint32_t(0xffff) << 30),
      ~(uint32_t(0xffff) << 31),
  };

  std::vector<uint64_t> v_bitmask64 = {
      uint64_t(0xffffffff00000000), uint64_t(0xffff) << 0,
      uint64_t(0xffff) << 1,        uint64_t(0xffff) << 6,
      uint64_t(0xffff) << 18,       uint64_t(0xffff) << 39,
      uint64_t(0xffff) << 47,       uint64_t(0xffff) << 48,
      uint64_t(0xffff) << 63,       ~uint64_t(0xffff) << 0,
      ~uint64_t(0xffff) << 1,       ~(uint64_t(0xffff) << 6),
      ~(uint64_t(0xffff) << 18),    ~(uint64_t(0xffff) << 39),
      ~(uint64_t(0xffff) << 47),    ~(uint64_t(0xffff) << 48),
      ~(uint64_t(0xffff) << 63),
  };

  ptnCount += v_int32.size();
  ptnCount += v_int64.size();
  ptnCount += v_uint32.size();
  ptnCount += v_uint64.size();

  errCount += test_mov_imm(v_int32, WReg(0));
  errCount += test_mov_imm(v_int64, WReg(0));
  errCount += test_mov_imm(v_uint32, WReg(0));
  errCount += test_mov_imm(v_uint64, WReg(0));

  ptnCount += v_int32.size();
  ptnCount += v_int64.size();
  ptnCount += v_uint32.size();
  ptnCount += v_uint64.size();

  errCount += test_mov_imm(v_int32, XReg(0));
  errCount += test_mov_imm(v_int64, XReg(0));
  errCount += test_mov_imm(v_uint32, XReg(0));
  errCount += test_mov_imm(v_uint64, XReg(0));

  ptnCount += v_bitmask32.size();
  ptnCount += v_bitmask64.size();
  ptnCount += v_bitmask32.size();
  ptnCount += v_bitmask64.size();

  errCount += test_mov_imm(v_bitmask32, WReg(0));
  errCount += test_mov_imm(v_bitmask64, WReg(0));
  errCount += test_mov_imm(v_bitmask32, XReg(0));
  errCount += test_mov_imm(v_bitmask64, XReg(0));

  std::cerr << std::dec;
  std::cerr << "Pattern=" << ptnCount << ", Error=" << errCount << std::endl;

  return errCount;
}
