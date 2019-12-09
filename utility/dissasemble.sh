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
# limitations under the License.
#*******************************************************************************/
AWK=awk
FILE_AWK="reorder_endian.awk"
DIR_SCRIPT=$(cd $(dirname $(readlink $0 || echo $0));pwd)
XXD="xxd -r -p"
OBJDUMP="aarch64-linux-gnu-objdump -D -b binary -m AArch64"
FILE_MC_TXT="tmp.machine_code.txt"
FILE_MC_ENDIAN_TXT="tmp.machine_code.endian.txt"
FILE_BINARY="tmp.machine_code.binary"

# Output machine code txt
./a.out > ${FILE_MC_TXT}

# Rorder endian
cat ${FILE_MC_TXT} | ${AWK} -f ${DIR_SCRIPT}/${FILE_AWK} > ${FILE_MC_ENDIAN_TXT}

# Translate text to binary
${XXD} -r -p ${FILE_MC_ENDIAN_TXT} > ${FILE_BINARY}

# Disassemble by objdump
${OBJDUMP} ${FILE_BINARY}
