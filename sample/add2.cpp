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
#include "xbyak_aarch64.h"
#include <string>
using namespace Xbyak_aarch64;
class Generator : public CodeGenerator {
public:
  Generator() {}

  void generate(int m, int n) {
    int i = 0;

    for (i = 0; i < m; i++) {
      add(w0, w0, n);
    }
  }

  void preamble() {}
  void postamble() { ret(); }
};
int main(int argc, char *argv[]) {
  (void)argc; // -Wunused-parameter
  Generator gen;
  gen.ready();
  gen.generate(atoi(argv[1]), atoi(argv[2]));
  gen.postamble();

  auto f = gen.getCode<int (*)(int, int)>();
  std::cout << f(atoi(argv[1]), atoi(argv[2])) << std::endl;
  return 0;
}
