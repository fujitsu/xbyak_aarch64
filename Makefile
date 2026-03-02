#*******************************************************************************
# Copyright 2020-2022 FUJITSU LIMITED
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#*******************************************************************************
ARCH?=$(shell uname -m)
TARGET=lib/libxbyak_aarch64.a
all: $(TARGET)

CFLAGS=-std=c++11 -DNDEBUG -g -I ./xbyak_aarch64 -Wall -Wextra -fPIC
ifneq ($(DEBUG),1)
CFLAGS+=-O2
endif
ifeq ($(STRICT_CONCAT),1) # allows to check collisions during test comparison
CFLAGS+=-DSTRICT_CONCAT
endif

LIB_OBJ=obj/xbyak_aarch64_impl.o obj/util_impl.o

obj/%.o: src/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

-include obj/xbyak_aarch64_impl.d

$(TARGET): $(LIB_OBJ)
	ar r $@ $^

test: $(TARGET)
	$(MAKE) -C test

clean:
	rm -rf obj/*.o obj/*.d $(TARGET)

update_version:
	./update_version.sh

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
