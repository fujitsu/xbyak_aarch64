CFLAGS=-std=c++11 -O3 -DNDEBUG -I ./xbyak_aarch64

all: lib/libxbyak_aarch64.a

obj/xbyak_aarch64_impl.o: src/xbyak_aarch64_impl.cpp src/xbyak_aarch64_impl.h src/xbyak_aarch64_mnemonic.h
	$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

lib/libxbyak_aarch64.a: obj/xbyak_aarch64_impl.o
	ar r $@ $<

clean:
	rm -rf obj/*.o obj/*.d lib/*.a

.PHONY: clean

.SECONDARY: obj/xbyak_aarch64_impl.o
