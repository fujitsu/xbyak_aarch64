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
#include <iostream>
#define XBYAK_NO_OP_NAMES
#define XBYAK_ENABLE_OMITTED_OPERAND
#include "xbyak_aarch64.h"

using namespace Xbyak;

#ifdef _MSC_VER
	#pragma warning(disable : 4245)
	#pragma warning(disable : 4312)
#endif
class Sample : public CodeGenerator {
	void operator=(const Sample&);
public:
#include "nm.cpp"
};

#define _STR(x) #x
#define TEST(syntax) err = true; try { syntax; err = false; } catch (Xbyak::Error) { } catch (...) { } if (!err) printf("should be err:%s;\n", _STR(syntax))

class ErrorSample : public CodeGenerator {
	void operator=(const ErrorSample&);
public:
	void gen()
	{
		bool err;
		//		TEST(mov(ptr[w0],1));
		//		TEST(test(ptr[w0],1));
		//		TEST(adc(ptr[w0],1));
		//		TEST(setz(w0));
	}
};
int main()
	try
{
	size_t size = sizeof(Xbyak::Operand);
	//	if (size != 4) {
	//		printf("sizeof Operand %d\n", (int)size);
	//	}
	try {
		Sample s;
		s.gen();
	} catch (std::exception& e) {
		printf("ERR:%s\n", e.what());
	} catch (...) {
		printf("unknown error\n");
	}
	ErrorSample es;
	es.gen();
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
