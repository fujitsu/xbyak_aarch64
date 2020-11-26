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
#include <fstream>
#include <iostream>
#include <xbyak_aarch64/xbyak_aarch64.h>
using namespace Xbyak_aarch64;
class Generator : public CodeGenerator {
public:
  Generator() {
    add(w0, w0, w1);
    ret();
  }
};

int main() {
  Generator gen;
  gen.ready();

  auto f = gen.getCode<int (*)(int, int)>();

  std::ofstream fout;
  fout.open("jited_code.bin", std::ios::out | std::ios::binary);
  std::cout << "size:" << gen.getSize() << std::endl;
  fout.write((const char *)gen.getCode(), gen.getSize());

  std::cout << f(3, 4) << std::endl;
}
