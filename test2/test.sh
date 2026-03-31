#!/bin/bash
#*******************************************************************************
# Copyright 2019-2021 FUJITSU LIMITED 
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
#******************************************************************************/
AS=aarch64-linux-gnu-as
AWK=awk
SED=sed
ARCH_TYPE=armv8.6-a
CPU_TYPE=all
CXX_FLAGS2="-std=c++11 -Wall -I../xbyak_aarch64 -DXBYAK_TEST -DXBYAK_USE_MMAP_ALLOCATOR"
CXX=g++

CPP_FILE=${1}

if [ ! -f ${CPP_FILE} ] ; then
    echo "${CPP_FILE} not found!"
    exit 1
fi
NEG_CPP="neg_cpp/empty.cpp"
BASE_NAME=`basename ${CPP_FILE} .cpp`
ASM_FILE=`echo ${CPP_FILE} | sed -e "s/test_ptn_cpp/test_ptn_asm/" | sed -e "s/\.cpp//"`.asm
NEG_FILE=`echo ${CPP_FILE} | sed -e "s/test_ptn_cpp/neg_cpp/"`
if [ -f ${NEG_FILE} ] ; then
    NEG_CPP="${NEG_FILE}"
fi
echo ${ASM_FILE}
EXP_FILE=tmp.${BASE_NAME}.exp
ASM_O_FILE=tmp.${BASE_NAME}.asm.o
DUMP_FILE=tmp.${BASE_NAME}.dump
RESULT_FILE=tmp.${BASE_NAME}.result

# Generate exp file
${AS} -march=${ARCH_TYPE} -mcpu=${CPU_TYPE} -acdnl=tmp.${EXP_FILE} -o ${ASM_O_FILE} ${ASM_FILE}
# Reorder endian 
cat tmp.${EXP_FILE} | grep -v -e "^\*\*\*\*" | ${AWK} '{print substr($3, 7, 2) substr($3, 5, 2) substr($3, 3, 2) substr($3, 1, 2); }' | grep -v -e "^$" > ${EXP_FILE}

# Generate dump file
cat nm_frame.cpp | sed -e "s#TEST_PTN#${CPP_FILE}#" | sed -e "s#TEST_NEG#${NEG_CPP}#" > tmp.nm_frame.${BASE_NAME}.cpp
${CXX} ${CXX_FLAGS2} -o tmp.${BASE_NAME}.exe tmp.nm_frame.${BASE_NAME}.cpp${P_FILE} ../lib/libxbyak_aarch64.a
./tmp.${BASE_NAME}.exe > ${DUMP_FILE}

# Check result
# Compare exp and dump
if ! diff -y ${DUMP_FILE} ${EXP_FILE} > ${RESULT_FILE}; then
    echo "error: ${BASE_NAME} has different content"
fi
