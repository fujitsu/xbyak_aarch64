#!/bin/sh
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
# limitations under the License.
#*******************************************************************************/
CLANG_FORMAT=${CLANG_FORMAT:="clang-format"}
list=`ls *.cpp *.h *.hpp`


# clang-format version check
${CLANG_FORMAT} --version | grep "version 9.0.0" > /dev/null
if [ $? != 0 ] ; then
    echo "clang-format version missmatch!"
    exit
fi


for i in ${list} ; do
    echo "formatting:${i}"
    clang-format -style=file -i ${i}
done
