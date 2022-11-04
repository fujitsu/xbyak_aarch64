#!/bin/sh
#*******************************************************************************
# Copyright 2019-2022 FUJITSU LIMITED
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
#*******************************************************************************
# Default compiler is g++
#*******************************************************************************
TEST_FILE=${1}
AARCH64_TYPE="armv8.4-a"
if [ ${ARCH} = arm64 ] ; then
  AARCH64_TYPE="all"
fi
CXX_FLAGS1="-std=c++11 -fomit-frame-pointer -Wall -fno-operator-names -I../xbyak_aarch64 -I./ -Wall -Wextra -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wfloat-equal -Wpointer-arith -Wno-ignored-qualifiers"
CXX_FLAGS2="-std=c++11 -Wall -I../xbyak_aarch64 -DXBYAK_TEST -DXBYAK_USE_MMAP_ALLOCATOR"

#*******************************************************************************
# Function definition
#*******************************************************************************
usage_exit() {
    echo "Usage: $0 [-f|-q] TEST_FILE"
    echo "  [-f] Fujitsu Compiler"
    echo "  [-q] GCC + QEMU"
    exit 1
}

dumpOK () {
    echo "##########################################"
    echo "# Test OK :-)"
    echo "##########################################"
}    

dumpNG () {
    echo "##########################################"
    echo "# $1"
    echo "# Test NG :-p"
    echo "##########################################"
}    

set_variables() {

  case $ENV_SELECT in
    f) CXX=FCC;
       CXX_FLAGS1="-std=c++11 -fomit-frame-pointer -Wall -fno-operator-names -I../xbyak_aarch64 -I./ -Wall -Wextra -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wfloat-equal -Wpointer-arith -Nclang -Knolargepage -Wno-ignored-qualifiers";
       CXX_FLAGS2="-Wall -I../xbyak_aarch64 -DXBYAK_TEST -DXBYAK_USE_MMAP_ALLOCATOR -Nclang -Knolargepage";
       echo "compiler is FCC"
      ;;

    q) source setenv-qemu
      ;;
    g) source setenv-gcc
       ;;
    *)
       ;;
  esac
}

#*******************************************************************************
# Main routine
#*******************************************************************************
while getopts f: OPT
do
  case $OPT in
    f) ENV_SELECT=f; TEST_FILE=$OPTARG
      ;;
    q) ENV_SELECT=q; TEST_FILE=$OPTARG
      ;;
    \?) usage_exit
      ;;
    esac
done
#shift $((OPTIND - 1))

set_variables

# Make object files
${CXX} -c ${TEST_FILE}.cpp -o ${TEST_FILE}.o ${CXX_FLAGS1} -MMD -MP -MF ${TEST_FILE}.d

if [ $? != 0 ] ; then
    dumpNG "Making object files"
    exit 1
fi

# Make binary
${CXX} ${TEST_FILE}.o -o ${TEST_FILE} -L ../lib -lxbyak_aarch64

if [ $? != 0 ] ; then
    dumpNG "Compiling binary."
    exit 1
fi


# Output 
${EMULATOR} ./${TEST_FILE} > ${TEST_FILE}.log 2>&1
if [ $? != 0 ] ;then
    dumpNG "Running binary test."
    exit 1
fi
