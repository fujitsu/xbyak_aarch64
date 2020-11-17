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
// clang-format off
#include "../xbyak_aarch64/xbyak_aarch64.h"
#include <cstring>

using namespace Xbyak_aarch64;

class GeneratorMovImm : public CodeGenerator {

public:
  GeneratorMovImm() {}

  void genMovFuncCore(const uint32_t imm, const XReg reg) {
    (void)reg;
    mov_imm(x0, imm);
    ret();
  }
  void genMovFuncCore(const uint32_t imm, const WReg reg) {
    (void)reg;
    mov_imm(w0, imm);
    ret();
  }
  void genMovFuncCore(const int32_t imm, const XReg reg) {
    (void)reg;
    mov_imm(x0, imm);
    ret();
  }
  void genMovFuncCore(const int32_t imm, const WReg reg) {
    (void)reg;
    mov_imm(w0, imm);
    ret();
  }
  void genMovFuncCore(const uint64_t imm, const XReg reg) {
    (void)reg;
    mov_imm(x0, imm);
    ret();
  }
  void genMovFuncCore(const int64_t imm, const XReg reg) {
    (void)reg;
    mov_imm(x0, imm);
    ret();
  }
};

template <typename T, typename U> int test_mov_imm(std::vector<T> &v, U &reg) {
  int errCount = 0;

  for (const auto &e : v) {
    GeneratorMovImm s;
    s.genMovFuncCore(e, reg);
    s.ready();

    /* Dump jit code to stdout. */
    char *f = reinterpret_cast<char *>((uint32_t(*)())s.getCode());
    std::cout.write(f, 4 * s.getSize());

    /* Dump jit code execution result to stderr. */
    T (*g)() = (T(*)())s.getCode();
    T result = g();

    std::cerr << "expect=" << e << ", result=" << result;
    if (e == result) {
      std::cerr << ", OK" << std::endl;
    } else {
      std::cerr << ", NG" << std::endl;
      ++errCount;
    }
  }

  return errCount;
}

int main() {
  int errCount = 0;
  XReg x0(0);
  WReg w0(0);

  std::vector<int32_t> v_int32 = {
      std::numeric_limits<int32_t>::min(),
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
      int32_t(0x7ff00000),
      int32_t(0xfff00000),
  };

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
                                    uint32_t(0x7ff00000),
                                    uint32_t(0xfff00000),
                                    ~uint32_t(0)};

  std::vector<int64_t> v_int64 = {
      std::numeric_limits<int64_t>::min(),
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
      int64_t(0x76540000ef098765),
      int64_t(0x0007ff0000000000),
      int64_t(0x00000000000007ff),
      int64_t(0x7ff0000000000000),
      int64_t(0xfff0000000000000),
      int64_t(0xffffffffffff1234),
      int64_t(0xffffffff1234ffff),
      int64_t(0xffff1234ffffffff),
      int64_t(0x1234ffffffffffff),
  };

  std::vector<uint64_t> v_uint64 = {
      0,
      1,
      2046,
      2047,
      2048,
      2049,
      std::numeric_limits<uint64_t>::max() - 1,
      std::numeric_limits<uint64_t>::max(),
      uint64_t(0x7654abcdef098765),
      uint64_t(0x76540000ef098765),
      uint64_t(0x0007ff0000000000),
      uint64_t(0x00000000000007ff),
      uint64_t(0x7ff0000000000000),
      uint64_t(0xfff0000000000000),
      uint64_t(0xffffffffffff1234),
      uint64_t(0xffffffff1234ffff),
      uint64_t(0xffff1234ffffffff),
      uint64_t(0x1234ffffffffffff),
      ~uint64_t(0),
  };

  errCount += test_mov_imm(v_int32, x0);
  errCount += test_mov_imm(v_int32, w0);
  errCount += test_mov_imm(v_int64, x0);
  errCount += test_mov_imm(v_uint32, x0);
  errCount += test_mov_imm(v_uint32, w0);
  errCount += test_mov_imm(v_uint64, x0);

  return errCount;
}
