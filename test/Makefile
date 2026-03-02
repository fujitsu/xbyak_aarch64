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

all: test

test:
	./test_all.sh

clean:
	rm -rf *.d *.o a.* *.log *.lst nm.make_nm* $(TARGET) nm.cpp nm_frame make_nm make_nm_branch add_sub_adds_subs_imm make_nm_fp make_nm_load_store make_nm_simd make_nm_simd_fp_load_store mov mov_imm make_nm_sve make_nm_sve_addr

