#!/bin/bash
#*******************************************************************************
# Copyright 2019-2020 FUJITSU LIMITED
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
nm_list="make_nm make_nm_branch make_nm_fp make_nm_load_store make_nm_simd make_nm_simd_fp_load_store make_nm_sve make_nm_sve_addr"

do_all_test() {
    echo "########################################################"
    echo "Test with ${COMPILER}"
    echo "########################################################"
    for i in ${nm_list} ;
    do
	echo "########################################################"

	echo "Start test senario=${i}"
	./test_nm.sh ${TEST_OPT} ${i}
	if [ $? != 0 ] ;then
            echo "err"
            exit 1
	fi
	mv nm.cpp nm.${i}.cpp

	echo "Finish test senario=${i}"
	echo "########################################################"
	echo ""
    done
}

while getopts gf OPT
do
    case $OPT in
        g) COMPILER=GCC
           TEST_OPT=""
           ;;
        f) COMPILER=FCC
           TEST_OPT="-f"
           ;;
        *) COMPILER=GCC
           TEST_OPT=""
           ;;
    esac
done
shift $((OPTIND - 1))

do_all_test
