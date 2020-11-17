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
#define TEST_NUM 1

using namespace Xbyak_aarch64;

class GeneratorImm : public CodeGenerator {

public:
  GeneratorImm() {}

// clang-format off
#define GEN_FUNC_CORE(inst, type, reg)                                         \
  void gen##inst##FuncCore(const type imm) {                                   \
    mov(reg##0, TEST_NUM);                                                     \
    inst##_imm(reg##0, reg##0, imm, reg##9);                                   \
    ret();                                                                     \
  }
  // clang-format on

  GEN_FUNC_CORE(add, uint32_t, w);
  GEN_FUNC_CORE(add, int32_t, w);
  GEN_FUNC_CORE(add, uint64_t, x);
  GEN_FUNC_CORE(add, int64_t, x);

  GEN_FUNC_CORE(sub, uint32_t, w);
  GEN_FUNC_CORE(sub, int32_t, w);
  GEN_FUNC_CORE(sub, uint64_t, x);
  GEN_FUNC_CORE(sub, int64_t, x);

  GEN_FUNC_CORE(adds, uint32_t, w);
  GEN_FUNC_CORE(adds, int32_t, w);
  GEN_FUNC_CORE(adds, uint64_t, x);
  GEN_FUNC_CORE(adds, int64_t, x);

  GEN_FUNC_CORE(subs, uint32_t, w);
  GEN_FUNC_CORE(subs, int32_t, w);
  GEN_FUNC_CORE(subs, uint64_t, x);
  GEN_FUNC_CORE(subs, int64_t, x);
#undef GEN_FUNC_CORE
};

// clang-format off
#define TEST_IMM(inst, op)                                                     \
  template <typename T> int test_##inst##_imm(std::vector<T> &v) {             \
    int errCount = 0;                                                          \
                                                                               \
    for (const auto &e : v) {                                                  \
      GeneratorImm s;                                                          \
      s.gen##inst##FuncCore(e);                                                \
      s.ready();                                                               \
                                                                               \
      /* Output JIT dump to stdout. */                                         \
      char *f = reinterpret_cast<char *>((uint32_t(*)())s.getCode());          \
      std::cout.write(f, 4 * s.getSize());                                     \
                                                                               \
      /* Output results of mov_imm to stderr. */                               \
      T (*g)() = (T(*)())s.getCode();                                          \
      T result = g();                                                          \
                                                                               \
      std::cerr << "expect=" << TEST_NUM op e << ",result=" << result;         \
      if (TEST_NUM op e == result) {                                           \
        std::cerr << ",OK" << std::endl;                                       \
      } else {                                                                 \
        std::cerr << ",NG" << std::endl;                                       \
        ++errCount;                                                            \
      }                                                                        \
    }                                                                          \
                                                                               \
    return errCount;                                                           \
  }
// clang-format on

TEST_IMM(add, +)
TEST_IMM(sub, -)
TEST_IMM(adds, +)
TEST_IMM(subs, -)

int main() {
  int errCount = 0;

  // clang-format off
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
  // clang-format on

  std::cerr << "########################################" << std::endl;
  std::cerr << "add start" << std::endl;
  std::cerr << "########################################" << std::endl;
  errCount += test_add_imm(v_int32);
  errCount += test_add_imm(v_int64);
  errCount += test_add_imm(v_uint32);
  errCount += test_add_imm(v_uint64);

  std::cerr << "########################################" << std::endl;
  std::cerr << "sub start" << std::endl;
  std::cerr << "########################################" << std::endl;
  errCount += test_sub_imm(v_int32);
  errCount += test_sub_imm(v_int64);
  errCount += test_sub_imm(v_uint32);
  errCount += test_sub_imm(v_uint64);

  std::cerr << "########################################" << std::endl;
  std::cerr << "adds start" << std::endl;
  std::cerr << "########################################" << std::endl;
  errCount += test_adds_imm(v_int32);
  errCount += test_adds_imm(v_int64);
  errCount += test_adds_imm(v_uint32);
  errCount += test_adds_imm(v_uint64);

  std::cerr << "########################################" << std::endl;
  std::cerr << "subs start" << std::endl;
  std::cerr << "########################################" << std::endl;
  errCount += test_subs_imm(v_int32);
  errCount += test_subs_imm(v_int64);
  errCount += test_subs_imm(v_uint32);
  errCount += test_subs_imm(v_uint64);

  return errCount;
}
