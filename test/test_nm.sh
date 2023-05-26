#!/bin/sh
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
# limitations under the License.
#******************************************************************************/

#*******************************************************************************
# This script is used for the following combination.
#
# |      | CI framework                 | Architecture        | Compiler |
# | ---- | ----                         | ----                | ----     |
# | 1    | github actions               | qemu-aarch64 on x64 | GCC      |
# | 2    | gitlab CI (Fujitsu in-house) | AArch64             | GCC      |
# | 3    | gitlab CI (Fujitsu in-house) | AArch64             | FCC      |
#*******************************************************************************
TEST_FILE=${1}
SETENV_PATH=`dirname ${0}`/../.github/automation/env

#*******************************************************************************
# Function definition
#*******************************************************************************
usage_exit() {
    echo "Usage: $0 [-f|-q] TEST_FILE"
    echo "  [-f] Fujitsu Compiler"
    echo "  [-g] GCC"
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
    f) . ${SETENV_PATH}/setenv-fcc
      ;;
    g) . ${SETENV_PATH}/setenv-gcc
       ;;
    q) . ${SETENV_PATH}/setenv-qemu
      ;;
    *)
       ;;
  esac
  echo "Compiler is ${CXX}."
}

#*******************************************************************************
# Main routine
#*******************************************************************************
while getopts f:g:q: OPT
do
  case $OPT in
    f) ENV_SELECT=f; TEST_FILE=$OPTARG
      ;;
    g) ENV_SELECT=g; TEST_FILE=$OPTARG
      ;;
    q) ENV_SELECT=q; TEST_FILE=$OPTARG
      ;;
    \?) usage_exit
      ;;
    esac
done

set_variables

# Make binary
${CXX} ${CXX_FLAGS1} -o ${TEST_FILE} ${TEST_FILE}.cpp

if [ $? != 0 ] ; then
    dumpNG "Compiling binary for generating test source file."
    exit 1
fi


# Output 
${EMULATOR} ./${TEST_FILE} > a.asm
if [ $? != 0 ] ;then
    dumpNG "Running binary to generate test source file."
    exit 1
fi
rm -f a.lst

# Generate data expected
${AS} -march=${AARCH64_TYPE} -mcpu=all -acdnl=a.lst a.asm
if [ $? != 0 ] ;then
    dumpNG
    exit 1
fi

# Reorder endian 
cat a.lst | grep -v -e "^\*\*\*\*" | ${AWK} '{print substr($3, 7, 2) substr($3, 5, 2) substr($3, 3, 2) substr($3, 1, 2); }' | grep -v -e "^$" > ok.lst
if [ $? != 0 ] ;then
    dumpNG "Generating ok.lst"
    exit 1
fi
cat a.lst | grep -v -e "^\*\*\*\*" | ${SED} -e "s/\ +/ /g" | ${SED} -e "s/^ //" | ${SED} -e "s/\t//" | ${AWK} '{print substr($3, 7, 2) substr($3, 5, 2) substr($3, 3, 2) substr($3, 1, 2) " "  $1 " " $2 " " $4 " " $5 " " $6 " " $7 " " $8 " " $9 " " $10 ;}' > ok_human_readable.lst
if [ $? != 0 ] ;then
    dumpNG "Generating ok_human_readable.lst"
    exit 1
fi

# Generate source file, which uses Xbyak mnemonic functions.
${EMULATOR} ./${TEST_FILE} jit > nm.cpp
if [ $? != 0 ] ;then
    dumpNG "Generating source file using xbyak"
    exit 1
fi
${CXX} ${CXX_FLAGS2} -o nm_frame nm_frame.cpp ../lib/libxbyak_aarch64.a
if [ $? != 0 ] ;then
    dumpNG "Compiling source file using xbyak"
    exit 1
fi

# Generate Xbyak JIT code output as text.
${EMULATOR} ./nm_frame  > x.lst
if [ $? != 0 ] ;then
    dumpNG "Xbyak JIT compile"
    exit 1
fi

# Compare Xbyak output and assembler output
diff -w x.lst ok.lst
if [ $? = 0 ] ; then
    dumpOK
    wc x.lst
    exit 0
else
    dumpNG "Checking output of JIT compile"
    wc x.lst
    exit 1
fi


