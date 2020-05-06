#*******************************************************************************
# Copyright 2019 FUJITSU LIMITED 
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
# *******************************************************************************/

TARGET = make_nm make_nm_load_store
XBYAK_INC=../xbyak_aarch64/xbyak_aarch64.h

all: $(TARGET)

#CXX=aarch64-linux-gnu-g++
CXX?=g++
CFLAGS_WARN=-Wall -Wextra -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wfloat-equal -Wpointer-arith -Wunused-function

CFLAGS=-Wall -I../ -I./ $(CFLAGS_WARN) #-std=c++0x
make_nm: make_nm.cpp $(XBYAK_INC)
	$(CXX) $(CFLAGS) make_nm.cpp -o $@
make_nm_load_store: make_nm_load_store.cpp $(XBYAK_INC)
	$(CXX) $(CFLAGS) make_nm_load_store.cpp -o $@

misc: misc.cpp $(XBYAK_INC)
	$(CXX) $(CFLAGS) -o $@ $<

test: ./misc
	./test_all.sh
	./misc

clean:
	rm -rf *.o $(TARGET) nm.cpp nm_frame misc

