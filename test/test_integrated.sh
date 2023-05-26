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
  echo "Compiler is ${CXX:-gcc}."
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
