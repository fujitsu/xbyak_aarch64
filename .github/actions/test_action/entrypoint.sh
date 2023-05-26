#!/bin/bash
#*******************************************************************************
# Copyright 2019-2023 FUJITSU LIMITED
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
# imitations under the License.
# *******************************************************************************/

# BecauseUbuntu 20.04 has qemu packages,
#.github/automation/env/qemu.sh

# Build Xbyak_aarch64 with cross compiler
source .github/automation/env/setenv-qemu
make -j`grep -c processor /proc/cpuinfo`
cd test
./test_all.sh -q
