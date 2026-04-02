#!/bin/bash
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
# imitations under the License.
# *******************************************************************************/
GPP=g++
TEST_FILE=${1:-reg_manager_tests}

dumpOK () {
    echo "##########################################"
    echo "# Tests OK"
    echo "##########################################"
}

dumpNG () {
    echo "##########################################"
    echo "# Test(s) failure"
    echo "# $1"
    echo "##########################################"
}

# Rebuild library
make -s -C .. lib/libxbyak_aarch64.a

if [ $? != 0 ] ; then
    dumpNG "Building libxbyak_aarch64."
    exit 1
fi

# Make object file
${GPP} -c -g -fomit-frame-pointer -Wall -fno-operator-names -I../xbyak_aarch64 -I./ \
    -Wall -Wextra -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings \
    -Wfloat-equal -Wpointer-arith -o ${TEST_FILE}.o ${TEST_FILE}.cpp

if [ $? != 0 ] ; then
    dumpNG "Compiling reg manager tests."
    exit 1
fi

# Link binary
${GPP} ${TEST_FILE}.o -o ${TEST_FILE} -L ../lib -lxbyak_aarch64

if [ $? != 0 ] ; then
    dumpNG "Linking reg manager tests."
    exit 1
fi

# Run binary
./${TEST_FILE} > ${TEST_FILE}.log 2>&1
if [ $? != 0 ] ;then
    dumpNG "Running reg manager tests."
    cat ${TEST_FILE}.log
    exit 1
fi

dumpOK
exit 0
