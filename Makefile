all=lib/libxbyak_aarch64.a
CFLAGS=-std=c++11 -DNDEBUG -g -I ./xbyak_aarch64 -Wall -Wextra
ifneq ($(DEBUG),1)
CFLAGS+=-O2
endif

obj/%.o: src/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

-include obj/xbyak_aarch64_impl.d

lib/libxbyak_aarch64.a: obj/xbyak_aarch64_impl.o
	ar r $@ $<

clean:
	rm -rf obj/*.o obj/*.d lib/*.a

.PHONY: clean

.SECONDARY: obj/xbyak_aarch64_impl.o obj/xbyak_aarch64_impl.d
