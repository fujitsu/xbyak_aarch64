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
NAME=libxbyak_aarch64
SONAME_MAJOR=1
SONAME_MID=0
SONAME_MINOR=0
STATIC_TARGET=lib/$(NAME).a
DYNAMIC_TARGET=lib/$(NAME).so
SONAME_TARGET=$(DYNAMIC_TARGET).so.$(SONAME_MAJOR)
LINK_TARGET=$(SONAME_TARGET).$(SONAME_MID).$(SONAME_MINOR)
all: $(STATIC_TARGET) $(DYNAMIC_TARGET)
static: $(STATIC_TARGET)
dynamic: $(DYNAMIC_TARGET)

CXXFLAGS=-std=c++11 -DNDEBUG -g -I ./xbyak_aarch64 -Wall -Wextra -fPIC
ifneq ($(DEBUG),1)
CXXFLAGS+=-O2
endif

LIB_OBJ=obj/xbyak_aarch64_impl.o obj/util_impl.o

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

-include obj/xbyak_aarch64_impl.d

$(LINK_TARGET): $(LIB_OBJ)
	$(CXX) $(CXXFLAGS) -shared -Wl,-soname,$(NAME).so.$(SONAME_MAJOR) -o $@ $^

$(SONAME_TARGET):
	ln -sf $(LINK_TARGET) $@

$(DYNAMIC_TARGET):
	ln -sf $(SONAME_TARGET) $@

$(STATIC_TARGET): $(LIB_OBJ)
	ar r $@ $^

test: $(STATIC_TARGET)
	$(MAKE) -C test

clean:
	rm -rf obj/*.o obj/*.d $(STATIC_TARGET) $(DYNAMIC_TARGET)

MKDIR=mkdir -p
PREFIX?=/usr/local
prefix?=$(PREFIX)
includedir?=$(prefix)/include
libdir?=$(prefix)/lib
INSTALL?=install
INSTALL_DATA?=$(INSTALL) -m 755
install: $(install_static) $(install_dynamic)

install_setup:
	$(MKDIR) $(DESTDIR)$(includedir)/$(NAME) $(DESTDIR)$(libdir)
	$(INSTALL_DATA) xbyak_aarch64/*.h $(DESTDIR)$(includedir)/xbyak_aarch64

install_static: $(STATIC_TARGET)
	$(install_setup)
	$(INSTALL_DATA) $(STATIC_TARGET) $(DESTDIR)$(libdir)

install_dynamic: $(DYNAMIC_TARGET)
	$(install_setup)
	$(INSTALL_DATA) $(LINK_TARGET) $(DESTDIR)$(libdir)
	$(INSTALL_DATA) $(SONAME_TARGET) $(DESTDIR)$(libdir)
	$(INSTALL_DATA) $(DYNAMIC_TARGET) $(DESTDIR)$(libdir)

.PHONY: clean test

.SECONDARY: obj/xbyak_aarch64_impl.o obj/xbyak_aarch64_impl.d
