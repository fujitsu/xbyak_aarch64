TARGET=lib/libxbyak_aarch64.a
all: $(TARGET)

CFLAGS=-std=c++11 -DNDEBUG -g -I ./xbyak_aarch64 -Wall -Wextra -fPIC
ifneq ($(DEBUG),1)
CFLAGS+=-O2
endif

obj/%.o: src/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

-include obj/xbyak_aarch64_impl.d

$(TARGET): obj/xbyak_aarch64_impl.o
	ar r $@ $<

test: $(TARGET)
	$(MAKE) -C test

clean:
	rm -rf obj/*.o obj/*.d $(TARGET)

MKDIR=mkdir -p
PREFIX?=/usr/local
prefix?=$(PREFIX)
includedir?=$(prefix)/include
libdir?=$(prefix)/lib
INSTALL?=install
INSTALL_DATA?=$(INSTALL) -m 644
install: $(TARGET)
	$(MKDIR) $(DESTDIR)$(includedir)/xbyak_aarch64 $(DESTDIR)$(libdir)
	$(INSTALL_DATA) xbyak_aarch64/*.h $(DESTDIR)$(includedir)/xbyak_aarch64
	$(INSTALL_DATA) $(TARGET) $(DESTDIR)$(libdir)

.PHONY: clean test

.SECONDARY: obj/xbyak_aarch64_impl.o obj/xbyak_aarch64_impl.d
