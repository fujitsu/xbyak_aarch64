#include <xbyak_aarch64/xbyak_aarch64.h>
#include <xbyak_aarch64/xbyak_aarch64_util.h>
/*
	How to use Profiler class
	sudo perf record ./a.out 1
	sudo perf report
*/

struct Code : Xbyak::CodeGenerator {
	Code(int step)
	{
		using namespace Xbyak;
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
		bge(lp);
	L(exit);
		ret();
	}
};

int main(int argc, char *argv[])
{
	int mode = argc == 1 ? 0 : atoi(argv[1]);
	Code c1(1), c2(2);
	c1.ready();
	c2.ready();
	auto f = c1.getCode<size_t(*)(size_t)>();
	auto g = c2.getCode<size_t(*)(size_t)>();
	Xbyak::util::Profiler prof;
	printf("mode=%d\n", mode);
	prof.init(mode);

	prof.setNameSuffix("-jit"); // this function may not be called
	prof.set("func1", (const void*)f, c1.getSize() * 4);
	prof.set("func2", (const void*)g, c2.getSize() * 4);

	size_t s1 = 0;
	size_t s2 = 0;
	for (size_t i = 0; i < 100000; i++) {
		s1 += f(i);
		s2 += g(i);
	}
	printf("s1=%zd, s2=%zd\n", s1, s2);
}
