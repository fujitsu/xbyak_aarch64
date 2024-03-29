#! /bin/bash

#===============================================================================
# Copyright 2020 FUJITSU LIMITED
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
#===============================================================================

# Download, build and install QEMU
wget https://download.qemu.org/qemu-7.1.0.tar.xz > /dev/null 2>&1
tar xJf qemu-7.1.0.tar.xz > /dev/null
cd qemu-7.1.0
./configure --target-list=aarch64-linux-user > /dev/null
make -j`grep -c processor /proc/cpuinfo` > /dev/null
make install > /dev/null
