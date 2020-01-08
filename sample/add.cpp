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
#include "../xbyak_aarch64/xbyak_aarch64.h"
using namespace Xbyak;
class Generator : public CodeGenerator {
public:
    void genAddFunc() {
        Label L1,L2;
        add(w0,w1,w0);
        ret();
    }
    const uint32_t *gen() {
        genAddFunc();
        ready();
        return getCode();
    }
};
int main() {
    Generator gen;
    int (*f)(int a, int b) = (int (*)(int a, int b))gen.gen();
    //    gen.dump();
    std::cout << f(3,4) << std::endl;
    return 0;
}
