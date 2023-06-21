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
#include <xbyak_aarch64/xbyak_aarch64.h>
#include <xbyak_aarch64/xbyak_aarch64_perf.h>
/*
    How to use Profiler class
    sudo perf record ./perf.exe
    sudo perf report
*/

struct Code : Xbyak_aarch64::CodeGenerator {
  Code(int step) {
    using namespace Xbyak_aarch64;
    Label exit, lp;
    cbz(x0, exit);
    mov(x2, x0);
    mov(x1, 0);
    mov(x0, 0);
    L(lp);
    add(x0, x0, x1);
    for (int i = 0; i < step; i++) {
      add(x1, x1, step);
    }
    cmp(x2, x1);
    b(GE, lp);
    L(exit);
    ret();
  }
};

int main() {
  Code c1(1), c2(2);
  c1.ready();
  c2.ready();
  auto f = c1.getCode<size_t (*)(size_t)>();
  auto g = c2.getCode<size_t (*)(size_t)>();
  Xbyak_aarch64::Profiler prof;
  prof.init();

  prof.setNameSuffix("-jit"); // this function may not be called
  prof.set("func1", (const void *)f, c1.getSize() * 4);
  prof.set("func2", (const void *)g, c2.getSize() * 4);

  size_t s1 = 0;
  size_t s2 = 0;
  for (size_t i = 0; i < 100000; i++) {
    s1 += f(i);
    s2 += g(i);
  }
  printf("s1=%zd, s2=%zd\n", s1, s2);
}
