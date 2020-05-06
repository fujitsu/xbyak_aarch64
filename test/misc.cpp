#include "xbyak_aarch64/xbyak_aarch64.h"
#include <iostream>

int g_err = 0;

#define TEST_EQUAL(x, y) if ((x) != (y)) { std::cout << "err '" #x "' = " << (x) << ",  '" #y "' = " << (y) << std::endl; g_err++; }
#define TEST_ASSERT(x) if (!(x)) { std::cout << "err '" #x "' = " << (x) << std::endl; g_err++; }


void alignTest()
{
	struct Code : Xbyak::CodeGenerator {
		void checkAlign(size_t n) const
		{
			size_t p = (size_t)getCurr();
			TEST_EQUAL(p % n, 0);
		}
		Code()
		{
			nop();
			align(16);
			checkAlign(16);
			nop();
			align(128);
			checkAlign(128);
		}
	} c;
}

int main()
{
	alignTest();
	if (g_err) {
		printf("#err=%d\n", g_err);
		return 1;
	} else {
		puts("ok");
		return 0;
	}
}
