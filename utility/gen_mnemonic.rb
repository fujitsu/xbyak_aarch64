#!/usr/bin/ruby
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
def reg_set(reg_num, ptn, en_ptn, reg_kind, &reg_fmt)
  set = []
  ptn.split("").each do |x|
    next if x !~ en_ptn
    x = yield(x) if not reg_fmt.nil?
    set.push (0..(reg_num-1)).map{|i| {reg_kind[i] => x} }
  end
  return set
end  

def rreg_set(reg_num, ptn)
  reg_kind  = ["rd","rn","rm","ra"]
  return reg_set(reg_num, ptn, /[WX]/, reg_kind){|x| format("%sReg", x)}
end

def screg_set(reg_num, ptn)
  reg_kind  = ["vd","vn","vm","va"]
  return reg_set(reg_num, ptn, /[BHSDQ]/, reg_kind){|x| format("%sReg", x)}
end

def vreg_set(reg_num, ptn)
  reg_kind  = ["vd","vn","vm","va"]
  rane_kind = {"B" => 8, "H" => 4, "S" => 2, "D" => 1}
  return reg_set(reg_num, ptn, /[BHSD]/, reg_kind){|x| format("VReg%d%s", rane_kind[x], x)}
end

def qvreg_set(reg_num, ptn)
  reg_kind  = ["vd","vn","vm","va"]
  rane_kind = {"B" => 16, "H" => 8, "S" => 4, "D" => 2}
  return reg_set(reg_num, ptn, /[BHSD]/, reg_kind){|x| format("VReg%d%s", rane_kind[x], x)}
end

def vreg_elem_set(reg_num, ptn)
  reg_kind  = ["vd","vn","vm","va"]
  return reg_set(reg_num, ptn, /[BHSDQ]/, reg_kind){|x| format("VReg%sElem", x)}
end

def vreg_list_set(reg_num, ptn)
  reg_kind  = ["vd","vn","vm","va"]
  rane_kind = {"B" => 8, "H" => 4, "S" => 2, "D" => 1}
  return reg_set(reg_num, ptn, /[BHSD]/, reg_kind){|x| format("VReg%d%sList", rane_kind[x], x)}
end

def qvreg_list_set(reg_num, ptn)
  reg_kind  = ["vd","vn","vm","va"]
  rane_kind = {"B" => 16, "H" => 8, "S" => 4, "D" => 2}
  return reg_set(reg_num, ptn, /[BHSD]/, reg_kind){|x| format("VReg%d%sList", rane_kind[x], x)}
end

def zreg_set(reg_num, ptn)
  reg_kind  = ["zd","zn","zm","za"]
  return reg_set(reg_num, ptn, /[BHSDQ]/, reg_kind){|x| format("ZReg%s", x)}
end

def preg_set(reg_num, ptn)
  reg_kind  = ["pd","pn","pm","pa"]
  return reg_set(reg_num, ptn, /[BHSDQ]/, reg_kind){|x| format("PReg%s", x)}
end

def with_pred(set)
  return set.map{|args| args.insert(1,{"pg" => "_PReg"}) }
end

def zreg_set_pred(reg_num, ptn)
  reg_kind  = ["zd","zn","zm","za"]
  return with_pred(zreg_set(reg_num, ptn))
end

def preg_set_pred(reg_num, ptn)
  reg_kind  = ["pd","pn","pm","pa"]
  return with_pred(preg_set(reg_num, ptn))
end

def trans_arg(arg, trans)
  trans_arg = arg.clone
  trans.each do |old,new|
    trans_arg.transform_keys!{|x| (x == old)? new : x}
  end
  trans_arg
end    

def ext_args(base, addition, trans={})
  base.map{|x| x.map{|y| trans_arg(y, trans)} + addition }
end

################################### ARMv8 #########################################
v8 = {
  ################### Data Processing --immediate  ################
  "PCrelAddr" => {
    :cmt => "PC-rel. addressing",
    :arg => [
      [ {"xd" => "XReg"}, {"label"  => "Label"}],  #0
      [ {"xd" => "XReg"}, {"label"  => "int64_t"}] #1
    ],
    :prm => ["op", "xd", "label"],
    :grp => [
      {"ADR"  => {"op" => 0x0}},
      {"ADRP" => {"op" => 0x1}}
    ]
  },

  "AddSubImm" => {
    :cmt => "Add/subtract (immediate)",
    :arg => ext_args(rreg_set(2,"WX"), [{"imm" => "uint32_t"}, {"sh=0" => "uint32_t"}]) + # 0-1
            ext_args(rreg_set(2,"WX"), [{"imm=0" => "uint32_t"}]) +                       # 2-3
            [
              [ {"rn" => "WReg"}, {"imm" => "uint32_t"}, {"sh=0" => "uint32_t"}], #4
              [ {"rn" => "XReg"}, {"imm" => "uint32_t"}, {"sh=0" => "uint32_t"}]  #5
            ],
    :prm => ["op", "S", "rd", "rn", "imm", "sh"],
    :grp => [
      {"ADD"  => {"op" => 0x0, "S" => 0},                     :arg => 0..1},
      # {"MOV"  => {"op" => 0x0, "S" => 0, "sh" => 0},          :arg => 2..3}, # alias of ADD
      {"ADDS" => {"op" => 0x0, "S" => 1},                     :arg => 0..1},
      {"CMN"  => {"op" => 0x0, "S" => 1, "rd" => "WReg(31)"}, :arg => [4]},  # alias of ADDS
      {"CMN"  => {"op" => 0x0, "S" => 1, "rd" => "XReg(31)"}, :arg => [5]},  # alias of ADDS
      {"SUB"  => {"op" => 0x1, "S" => 0},                     :arg => 0..1},
      {"SUBS" => {"op" => 0x1, "S" => 1},                     :arg => 0..1},
      {"CMP"  => {"op" => 0x1, "S" => 1, "rd" => "WReg(31)"}, :arg => [4]},  # alias of SUBS
      {"CMP"  => {"op" => 0x1, "S" => 1, "rd" => "XReg(31)"}, :arg => [5]},  # alias of SUBS
    ]
  },
  
  "LogicalImm" => {
    :cmt => "Logical (immediate)",
    :arg => ext_args(rreg_set(2,"WX"), [{"imm" => "uint64_t"}]) + # 0-1
            [
              [ {"rn" => "WReg"}, {"imm" => "uint64_t"}], # 2
              [ {"rn" => "XReg"}, {"imm" => "uint64_t"}]  # 3
            ],
    :prm => [
      ["opc", "rd", "rn", "imm"],          #0
      ["opc", "rd", "rn", "imm", "alias"], #1
    ],
    :grp => [
      {"AND"  => {"opc" => 0x0}, :arg => 0..1, :prm => 0},
      {"ORR"  => {"opc" => 0x1}, :arg => 0..1, :prm => 0},
      {"EOR"  => {"opc" => 0x2}, :arg => 0..1, :prm => 0},
      {"ANDS" => {"opc" => 0x3}, :arg => 0..1, :prm => 0},
      {"TST"  => {"opc" => 0x3, "rd" => "WReg(31)", "alias" => "true"}, :arg => [2], :prm => 1},  # alias of ANDS
      {"TST"  => {"opc" => 0x3, "rd" => "XReg(31)", "alias" => "true"}, :arg => [3], :prm => 1}   # alias of ANDS
    ]
  },

  "MvWideImm" => {
    :cmt => "Move wide(immediate)",
    :arg => ext_args(rreg_set(1,"WX"), [{"imm" => "uint32_t"}, {"sh=0" => "uint32_t"}]), # 0-1
    :prm => ["opc", "rd", "imm", "sh"],
    :grp => [
      {"MOVN"  => {"opc" => 0x0}},
      {"MOVZ"  => {"opc" => 0x2}},
      {"MOVK"  => {"opc" => 0x3}}
    ]
  },

  "MvImm" => {
    :cmt => "Move (immediate) alias of ORR,MOVN,MOVZ",
    :arg => ext_args(rreg_set(1,"WX"), [{"imm" => "uint64_t"}]), # 0-1
    :prm => ["rd", "imm"],
    :grp => [
      {"MOV" => {}}
    ]
  },
  
  "Bitfield" => {
    :cmt => "Bitfield",
    :arg => ext_args(rreg_set(2,"WX"), [{"immr" => "uint32_t"}, {"imms" => "uint32_t"}]) + # 0-1
            [
              [ {"rd" => "WReg"}, {"rn" => "WReg"}, {"immr" => "uint32_t"}],                         # 2
              [ {"rd" => "XReg"}, {"rn" => "XReg"}, {"immr" => "uint32_t"}],                         # 3
              [ {"rd" => "WReg"}, {"lsb" => "uint32_t"}, {"width" => "uint32_t"}],                   # 4
              [ {"rd" => "XReg"}, {"lsb" => "uint32_t"}, {"width" => "uint32_t"}],                   # 5
              [ {"rd" => "WReg"}, {"rn" => "WReg"}, {"lsb" => "uint32_t"}, {"width" => "uint32_t"}], # 6
              [ {"rd" => "XReg"}, {"rn" => "XReg"}, {"lsb" => "uint32_t"}, {"width" => "uint32_t"}], # 7
              [ {"rd" => "WReg"}, {"rn" => "WReg"}, {"sh" => "uint32_t"}],                           # 8
              [ {"rd" => "XReg"}, {"rn" => "XReg"}, {"sh" => "uint32_t"}],                           # 9
              [ {"rd" => "WReg"}, {"rn" => "WReg"}],                                                 # 10
              [ {"rd" => "XReg"}, {"rn" => "XReg"}],                                                 # 11
              [ {"rd" => "XReg"}, {"rn" => "WReg"}],                                                 # 12
            ],
    :prm => [
      ["opc", "rd", "rn", "immr", "imms"],           # 0
      ["opc", "rd", "rn", "immr", "imms", "rn_chk"], # 1
    ],
    :grp => [
      {"SBFM"  => {"opc" => 0x0},               :arg => 0..1, :prm => 0},
      {"SBFIZ" => {"opc" => 0x0,
                   "immr" => "(((-1)*lsb) % 32) & ones(6)",
                   "imms" => "width-1"},        :arg => [6],  :prm => 0},   # alias of SBFM
      {"SBFIZ" => {"opc" => 0x0,
                   "immr" => "(((-1)*lsb) % 64) & ones(6)",
                   "imms" => "width-1"},        :arg => [7],  :prm => 0},   # alias of SBFM
      {"SBFX"  => {"opc" => 0x0,
                   "immr" => "lsb",
                   "imms" => "lsb+width-1"},    :arg => 6..7, :prm => 0},   # alias of SBFM
      {"SXTB"  => {"opc" => 0x0,
                   "immr" => 0, "imms" => 7},   :arg => [10,12],:prm => 0}, # alias of SBFM
      {"SXTH"  => {"opc" => 0x0,
                   "immr" => 0, "imms" => 15},  :arg => [10,12],:prm => 0}, # alias of SBFM
      {"SXTW"  => {"opc" => 0x0,
                   "immr" => 0, "imms" => 31},  :arg => [10,12],:prm => 0}, # alias of SBFM
      {"ASR"   => {"opc" => 0x0, "imms" => 31}, :arg => [2],    :prm => 0}, # alias of SBFM
      {"ASR"   => {"opc" => 0x0, "imms" => 63}, :arg => [3],    :prm => 0}, # alias of SBFM
      {"BFM"   => {"opc" => 0x1},               :arg => 0..1,   :prm => 0},
      {"BFC"   => {"opc" => 0x1, "rn" => "WReg(31)",
                   "rn_chk" => "false",
                   "immr" => "(((-1)*lsb) % 32) & ones(6)",
                   "imms" => "width-1"},        :arg => [4], :prm => 1},    # alias of BFM
      {"BFC"   => {"opc" => 0x1, "rn" => "XReg(31)",
                   "rn_chk" => "false",
                   "immr" => "(((-1)*lsb) % 64) & ones(6)",
                   "imms" => "width-1"},        :arg => [5], :prm => 1},    # alias of BFM
      {"BFI"   => {"opc" => 0x1,
                   "immr" => "(((-1)*lsb) % 32) & ones(6)",
                   "imms" => "width-1"},        :arg => [6], :prm => 0},    # alias of BFM
      {"BFI"   => {"opc" => 0x1,
                   "immr" => "(((-1)*lsb) % 64) & ones(6)",
                   "imms" => "width-1"},        :arg => [7], :prm => 0},    # alias of BFM
      {"BFXIL" => {"opc" => 0x1,
                   "immr" => "lsb",
                   "imms" => "lsb+width-1"},    :arg => 6..7, :prm => 0},   # alias of BFM
      {"UBFM"  => {"opc" => 0x2},               :arg => 0..1, :prm => 0},
      {"UBFIZ" => {"opc" => 0x2,
                   "immr" => "(((-1)*lsb) % 32) & ones(6)",
                   "imms" => "width-1"},        :arg => [6], :prm => 0},    # alias of UBFM
      {"UBFIZ" => {"opc" => 0x2,
                   "immr" => "(((-1)*lsb) % 64) & ones(6)",
                   "imms" => "width-1"},        :arg => [7], :prm => 0},    # alias of UBFM
      {"UBFX"  => {"opc" => 0x2,
                   "immr" => "lsb",
                   "imms" => "lsb+width-1"},    :arg => 6..7,:prm => 0},    # alias of UBFM
      {"LSL"   => {"opc" => 0x2,
                   "immr" => "(((-1)*sh) % 32) & ones(6)",
                   "imms" => "31 - sh"},        :arg => [8], :prm => 0},    # alias of UBFM
      {"LSL"   => {"opc" => 0x2,
                   "immr" => "(((-1)*sh) % 64) & ones(6)",
                   "imms" => "63 - sh"},        :arg => [9], :prm => 0},    # alias of UBFM
      {"LSR"   => {"opc" => 0x2,
                   "immr" => "sh",
                   "imms" => "31"},             :arg => [8], :prm => 0},    # alias of UBFM
      {"LSR"   => {"opc" => 0x2,
                   "immr" => "sh",
                   "imms" => "63"},             :arg => [9], :prm => 0},    # alias of UBFM
      {"UXTB"  => {"opc" => 0x2,
                   "immr" => 0, "imms" => 7},   :arg => 10..11, :prm => 0}, # alias of UBFM
      {"UXTH"  => {"opc" => 0x2,
                   "immr" => 0, "imms" => 15},  :arg => 10..11, :prm => 0}  # alias of UBFM
    ]
  },

  "Extract" => {
    :cmt => "Extract",
    :arg => ext_args(rreg_set(3,"WX"), [{"imm" => "uint32_t"}]) + # 0-1
            ext_args(rreg_set(2,"WX"), [{"imm" => "uint32_t"}]),  # 2-3
    :prm => ["op21", "o0", "rd", "rn", "rm", "imm"],
    :grp => [
      {"EXTR"  => {"op21" => 0x0, "o0" => 0},               :arg => 0..1},
      {"ROR"   => {"op21" => 0x0, "o0" => 0, "rm" => "rn"}, :arg => 2..3}
    ]
  },

  ################### Branches, Exception Generating and System instructions ################
  "CondBrImm" => {
    :cmt => "Conditional branch (immediate)",
    :arg => [
      [ {"cond" => "Cond"}, {"label" => "Label"}],  #0
      [ {"cond" => "Cond"}, {"label" => "int64_t"}] #1
    ],
    :prm => ["cond", "label"],
    :grp => [
      {"B"  => {}},
    ]
  },
  
  "ExceptionGen" => {
    :cmt => "Exception generation",
    :arg => [[ {"imm" => "uint32_t"}]],
    :prm => ["opc", "op2", "LL", "imm"],
    :grp => [
      {"SVC"   => {"opc" => 0x0, "op2" => 0, "LL" => 1}},
      {"HVC"   => {"opc" => 0x0, "op2" => 0, "LL" => 2}},
      {"SMC"   => {"opc" => 0x0, "op2" => 0, "LL" => 3}},
      {"BRK"   => {"opc" => 0x1, "op2" => 0, "LL" => 0}},
      {"HLT"   => {"opc" => 0x2, "op2" => 0, "LL" => 0}},
      {"DCPS1" => {"opc" => 0x5, "op2" => 0, "LL" => 1}},
      {"DCPS2" => {"opc" => 0x5, "op2" => 0, "LL" => 2}},
      {"DCPS3" => {"opc" => 0x5, "op2" => 0, "LL" => 3}}
    ]
  },

  "Hints" => {
    :cmt => "Hints",
    :arg => [
      [],                      #0
      [ {"imm" => "uint32_t"}] #1
    ],
    :prm => [
      ["CRm", "op2"], #0
      ["imm"]         #1
    ],
    :grp => [
      {"HINT"      => {}, :arg => [1], :prm => 1},
      {"NOP"       => {"CRm" => 0x0, "op2" => 0}, :arg => [0], :prm => 0},
      {"YIELD"     => {"CRm" => 0x0, "op2" => 1}, :arg => [0], :prm => 0},
      {"WFE"       => {"CRm" => 0x0, "op2" => 2}, :arg => [0], :prm => 0},
      {"WFI"       => {"CRm" => 0x0, "op2" => 3}, :arg => [0], :prm => 0},
      {"SEV"       => {"CRm" => 0x0, "op2" => 4}, :arg => [0], :prm => 0},
      {"SEVL"      => {"CRm" => 0x0, "op2" => 5}, :arg => [0], :prm => 0},
      {"XPACLRI"   => {"CRm" => 0x0, "op2" => 7}, :arg => [0], :prm => 0},
      {"PACIA1716" => {"CRm" => 0x1, "op2" => 0}, :arg => [0], :prm => 0},
      {"PACIB1716" => {"CRm" => 0x1, "op2" => 2}, :arg => [0], :prm => 0},
      {"AUTIA1716" => {"CRm" => 0x1, "op2" => 4}, :arg => [0], :prm => 0},
      {"AUTIB1716" => {"CRm" => 0x1, "op2" => 6}, :arg => [0], :prm => 0},
      {"ESB"       => {"CRm" => 0x2, "op2" => 0}, :arg => [0], :prm => 0},
      {"PSB_CSYNC" => {"CRm" => 0x2, "op2" => 1}, :arg => [0], :prm => 0},
      {"TSB_CSYNC" => {"CRm" => 0x2, "op2" => 2}, :arg => [0], :prm => 0},
      {"CSDB"      => {"CRm" => 0x2, "op2" => 4}, :arg => [0], :prm => 0},
      {"PACIAZ"    => {"CRm" => 0x3, "op2" => 0}, :arg => [0], :prm => 0},
      {"PACIASP"   => {"CRm" => 0x3, "op2" => 1}, :arg => [0], :prm => 0},
      {"PACIBZ"    => {"CRm" => 0x3, "op2" => 2}, :arg => [0], :prm => 0},
      {"PACIBSP"   => {"CRm" => 0x3, "op2" => 3}, :arg => [0], :prm => 0},
      {"AUTIAZ"    => {"CRm" => 0x3, "op2" => 4}, :arg => [0], :prm => 0},
      {"AUTIASP"   => {"CRm" => 0x3, "op2" => 5}, :arg => [0], :prm => 0},
      {"AUTIBZ"    => {"CRm" => 0x3, "op2" => 6}, :arg => [0], :prm => 0},
      {"AUTIBSP"   => {"CRm" => 0x3, "op2" => 7}, :arg => [0], :prm => 0}
    ]
  },

  "BarriersOpt" => {
    :cmt => "Barriers (option)",
    :arg => [[ {"opt" => "BarOpt"}]],
    :prm => ["op2", "opt", "rt"],
    :grp => [
      {"DSB"   => {"op2" => 4, "rt" => 0x1f}},
      {"DMB"   => {"op2" => 5, "rt" => 0x1f}},
      {"ISB"   => {"op2" => 6, "rt" => 0x1f}}
    ]
  },

  "BarriersNoOpt" => {
    :cmt => "Barriers (no option)",
    :arg => [
      [{"imm" => "uint32_t"}], #0
      [] #1
    ],
    :prm => [
      ["imm", "op2", "rt"], #0
      ["CRm", "op2", "rt"] #1
    ],
    :grp => [
      {"CLREX" => {"op2" => 2, "rt" => 0x1f}, :arg => [0], :prm => 0},
      {"SSBB"  => {"CRm" => 0, "op2" => 4, "rt" => 0x1f}, :arg => [1], :prm => 1},
      {"PSSBB" => {"CRm" => 4, "op2" => 4, "rt" => 0x1f}, :arg => [1], :prm => 1}
    ]
  },

  "PState" => {
    :cmt => "pstate",
    :arg => [
      [{"psfield" => "PStateField"}, {"imm" => "uint32_t"}], #0
      []  #1
    ],
    :prm => [
      ["psfield", "imm"], #0
      ["op1", "CRm", "op2"]  #1
    ],
    :grp => [
      {"MSR"   => {},                                   :arg => [0], :prm => 0},
      {"CFINV" => {"op1" => 0, "CRm" => 0, "op2" => 0}, :arg => [1], :prm => 1},
    ]
  },
  
  "SysInst" => {
    :cmt => "Systtem instructions",
    :arg => [
      [ {"op1" => "uint32_t"}, {"CRn" => "uint32_t"}, {"CRm" => "uint32_t"}, {"op2" => "uint32_t"}, {"rt=XReg(31)" => "XReg"}], #0
      [ {"rt" => "XReg"}, {"op1" => "uint32_t"}, {"CRn" => "uint32_t"}, {"CRm" => "uint32_t"}, {"op2" => "uint32_t"}]  #2
    ],
    :prm => ["L", "op1", "CRn", "CRm", "op2", "rt"],
    :grp => [
      {"SYS"  => {"L" => 0}, :arg => [0]},
      {"SYSL" => {"L" => 1}, :arg => [1]}
    ]
  },

  "SysRegMove" => {
    :cmt => "System register move",
    :arg => [
      [ {"op0" => "uint32_t"}, {"op1" => "uint32_t"}, {"CRn" => "uint32_t"}, {"CRm" => "uint32_t"}, {"op2" => "uint32_t"}, {"rt" => "XReg"}], #0
      [ {"rt" => "XReg"}, {"op0" => "uint32_t"}, {"op1" => "uint32_t"}, {"CRn" => "uint32_t"}, {"CRm" => "uint32_t"}, {"op2" => "uint32_t"}]  #1
    ],
    :prm => ["L", "op0", "op1", "CRn", "CRm", "op2", "rt"],
    :grp => [
      {"MSR" => {"L" => 0}, :arg => [0]},
      {"MRS" => {"L" => 1}, :arg => [1]}
    ]
  },

  "UncondBrNoReg" => {
    :cmt => "Unconditional branch",
    :arg => [[]],
    :prm => ["opc", "op2", "op3", "rn", "op4"],
    :grp => [
      {"RET"    => {"opc" => 2, "op2" => 0x1f, "op3" => 0, "rn" => 0x1e, "op4" => 0}},
      {"RETAA"  => {"opc" => 2, "op2" => 0x1f, "op3" => 2, "rn" => 0x1f, "op4" => 0x1f}},
      {"RETAB"  => {"opc" => 2, "op2" => 0x1f, "op3" => 3, "rn" => 0x1f, "op4" => 0x1f}},
      {"ERET"   => {"opc" => 4, "op2" => 0x1f, "op3" => 0, "rn" => 0x1f, "op4" => 0}},
      {"ERETAA" => {"opc" => 4, "op2" => 0x1f, "op3" => 2, "rn" => 0x1f, "op4" => 0x1f}},
      {"ERETAB" => {"opc" => 4, "op2" => 0x1f, "op3" => 3, "rn" => 0x1f, "op4" => 0x1f}},
      {"DRPS"   => {"opc" => 5, "op2" => 0x1f, "op3" => 0, "rn" => 0x1f, "op4" => 0x0}}
    ]
  },

  "UncondBr1Reg" => {
    :cmt => "Unconditional branch (1 register)",
    :arg => [[ {"rn" => "XReg"}]],
    :prm => ["opc", "op2", "op3", "rn", "op4"],
    :grp => [
      {"BR"     => {"opc" => 0, "op2" => 0x1f, "op3" => 0, "op4" => 0}},
      {"BRAAZ"  => {"opc" => 0, "op2" => 0x1f, "op3" => 2, "op4" => 0x1f}},
      {"BRABZ"  => {"opc" => 0, "op2" => 0x1f, "op3" => 3, "op4" => 0x1f}},
      {"BLR"    => {"opc" => 1, "op2" => 0x1f, "op3" => 0, "op4" => 0}},
      {"BLRAAZ" => {"opc" => 1, "op2" => 0x1f, "op3" => 2, "op4" => 0x1f}},
      {"BLRABZ" => {"opc" => 1, "op2" => 0x1f, "op3" => 3, "op4" => 0x1f}},
      {"RET"    => {"opc" => 2, "op2" => 0x1f, "op3" => 0, "op4" => 0}}
    ]
  },

  "UncondBr2Reg" => {
    :cmt => "Unconditional branch (2 register)",
    :arg => [[ {"rn" => "XReg"}, {"rm" => "XReg"}]],
    :prm => ["opc", "op2", "op3", "rn", "rm"],
    :grp => [
      {"BRAA"  => {"opc" => 0x8, "op2" => 0x1f, "op3" => 2}},
      {"BRAB"  => {"opc" => 0x8, "op2" => 0x1f, "op3" => 3}},
      {"BLRAA" => {"opc" => 0x9, "op2" => 0x1f, "op3" => 2}},
      {"BLRAB" => {"opc" => 0x9, "op2" => 0x1f, "op3" => 3}}
    ]
  },

  "UncondBrImm" => {
    :cmt => "Unconditional branch (immediate)",
    :arg => [
      [ {"label" => "Label"}],  #0
      [ {"label" => "int64_t"}] #1
    ],
    :prm => ["op", "label"],
    :grp => [
      {"B"   => {"op" => 0x0}},
      {"BL"  => {"op" => 0x1}}
    ]
  },
  
  "CompareBr" => {
    :cmt => "Compare and branch (immediate)",
    :arg => ext_args(rreg_set(1,"WX"), [{"label" => "Label"}], {"rd" => "rt"}) +  #0-1
            ext_args(rreg_set(1,"WX"), [{"label" => "int64_t"}], {"rd" => "rt"}), #2-3
    :prm => ["op", "rt", "label"],
    :grp => [
      {"CBZ"   => {"op" => 0x0}},
      {"CBNZ"  => {"op" => 0x1}}
    ]
  },

  "TestBr" => {
    :cmt => "Test and branch (immediate)",
    :arg => ext_args(rreg_set(1,"WX"), [{"imm" => "uint32_t"}, {"label" => "Label"}], {"rd" => "rt"}) +  #0-1
            ext_args(rreg_set(1,"WX"), [{"imm" => "uint32_t"}, {"label" => "int64_t"}], {"rd" => "rt"}), #2-3
    :prm => ["op", "rt", "imm", "label"],
    :grp => [
      {"TBZ"   => {"op" => 0x0}},
      {"TBNZ"  => {"op" => 0x1}}
    ]
  },

  ########################## Loads and stores ###################################
  "AdvSimdLdStMultiStructForLd1St1" => {
    :cmt => "Advanced SIMD load/store multple structures",
    :arg => ext_args(vreg_list_set(1,"BHSD"), [{"adr" => "AdrNoOfs"}], {"vd" => "vt"}) +
            ext_args(qvreg_list_set(1,"BHSD"), [{"adr" => "AdrNoOfs"}], {"vd" => "vt"}),
    # :arg => [ {"vt" => "VReg"}, {"adr" => "AdrNoOfs"}]),
    :prm => ["L", "opcode", "vt", "adr"],
    :grp => [
      {"ST1"   => {"L" => 0, "opcode" => 0x2}},
      {"LD1"   => {"L" => 1, "opcode" => 0x2}},
    ]
  },

  "AdvSimdLdStMultiStructExceptLd1St1" => {
    :cmt => "Advanced SIMD load/store multple structures",
    :arg => ext_args(vreg_list_set(1,"BHS"), [{"adr" => "AdrNoOfs"}], {"vd" => "vt"}) +
            ext_args(qvreg_list_set(1,"BHSD"), [{"adr" => "AdrNoOfs"}], {"vd" => "vt"}),
    # :arg => [ {"vt" => "VReg"}, {"adr" => "AdrNoOfs"}]),
    :prm => ["L", "opcode", "vt", "adr"],
    :grp => [
      {"ST4"   => {"L" => 0, "opcode" => 0x0}},
      {"ST3"   => {"L" => 0, "opcode" => 0x4}},
      {"ST2"   => {"L" => 0, "opcode" => 0x8}},
      {"LD4"   => {"L" => 1, "opcode" => 0x0}},
      {"LD3"   => {"L" => 1, "opcode" => 0x4}},
      {"LD2"   => {"L" => 1, "opcode" => 0x8}},
    ]
  },

  "AdvSimdLdStMultiStructPostRegForLd1St1" => {
    :cmt => "Advanced SIMD load/store multple structures (post-indexed register offset)",
    :arg => ext_args(vreg_list_set(1,"BHSD"), [{"adr" => "AdrPostReg"}], {"vd" => "vt"}) +
            ext_args(qvreg_list_set(1,"BHSD"), [{"adr" => "AdrPostReg"}], {"vd" => "vt"}),
    # :arg => [[ {"vt" => "VReg"}, {"adr" => "AdrPostReg"}]],
    :prm => ["L", "opcode", "vt", "adr"],
    :grp => [
      {"ST1"   => {"L" => 0, "opcode" => 0x2}},
      {"LD1"   => {"L" => 1, "opcode" => 0x2}}
    ]
  },

  "AdvSimdLdStMultiStructPostRegExceptLd1St1" => {
    :cmt => "Advanced SIMD load/store multple structures (post-indexed register offset)",
    :arg => ext_args(vreg_list_set(1,"BHS"), [{"adr" => "AdrPostReg"}], {"vd" => "vt"}) +
            ext_args(qvreg_list_set(1,"BHSD"), [{"adr" => "AdrPostReg"}], {"vd" => "vt"}),
    # :arg => [[ {"vt" => "VReg"}, {"adr" => "AdrPostReg"}]],
    :prm => ["L", "opcode", "vt", "adr"],
    :grp => [
      {"ST4"   => {"L" => 0, "opcode" => 0x0}},
      {"ST3"   => {"L" => 0, "opcode" => 0x4}},
      {"ST2"   => {"L" => 0, "opcode" => 0x8}},
      {"LD4"   => {"L" => 1, "opcode" => 0x0}},
      {"LD3"   => {"L" => 1, "opcode" => 0x4}},
      {"LD2"   => {"L" => 1, "opcode" => 0x8}},
    ]
  },

  "AdvSimdLdStMultiStructPostImmForLd1St1" => {
    :cmt => "Advanced SIMD load/store multple structures (post-indexed immediate offset)",
    :arg => ext_args(vreg_list_set(1,"BHSD"), [{"adr" => "AdrPostImm"}], {"vd" => "vt"}) +
            ext_args(qvreg_list_set(1,"BHSD"), [{"adr" => "AdrPostImm"}], {"vd" => "vt"}),
    # :arg => [[ {"vt" => "VReg"}, {"adr" => "AdrPostImm"}]],
    :prm => ["L", "opcode", "vt", "adr"],
    :grp => [
      {"ST1"   => {"L" => 0, "opcode" => 0x2}},
      {"LD1"   => {"L" => 1, "opcode" => 0x2}}
    ]
  },

  "AdvSimdLdStMultiStructPostImmExceptLd1St1" => {
    :cmt => "Advanced SIMD load/store multple structures (post-indexed immediate offset)",
    :arg => ext_args(vreg_list_set(1,"BHS"), [{"adr" => "AdrPostImm"}], {"vd" => "vt"}) +
            ext_args(qvreg_list_set(1,"BHSD"), [{"adr" => "AdrPostImm"}], {"vd" => "vt"}),
    # :arg => [[ {"vt" => "VReg"}, {"adr" => "AdrPostImm"}]],
    :prm => ["L", "opcode", "vt", "adr"],
    :grp => [
      {"ST4"   => {"L" => 0, "opcode" => 0x0}},
      {"ST3"   => {"L" => 0, "opcode" => 0x4}},
      {"ST2"   => {"L" => 0, "opcode" => 0x8}},
      {"LD4"   => {"L" => 1, "opcode" => 0x0}},
      {"LD3"   => {"L" => 1, "opcode" => 0x4}},
      {"LD2"   => {"L" => 1, "opcode" => 0x8}},
    ]
  },

  "AdvSimdLdStSingleStruct" => {
    :cmt => "Advanced SIMD load/store single structures",
    :arg => ext_args(vreg_elem_set(1,"BHSD"), [{"adr" => "AdrNoOfs"}], {"vd" => "vt"}),
    # :arg => [[ {"vt" => "VRegElem"}, {"adr" => "AdrNoOfs"}]],
    :prm => ["L", "R", "num", "vt", "adr"],
    :grp => [
      {"ST4"   => {"L" => 0, "R" => 1, "num" => 4}},
      {"ST3"   => {"L" => 0, "R" => 0, "num" => 3}},
      {"ST2"   => {"L" => 0, "R" => 1, "num" => 2}},
      {"ST1"   => {"L" => 0, "R" => 0, "num" => 1}},
      {"LD4"   => {"L" => 1, "R" => 1, "num" => 4}},
      {"LD3"   => {"L" => 1, "R" => 0, "num" => 3}},
      {"LD2"   => {"L" => 1, "R" => 1, "num" => 2}},
      {"LD1"   => {"L" => 1, "R" => 0, "num" => 1}}
    ]
  },

  "AdvSimdLdRepSingleStruct" => {
    :cmt => "Advanced SIMD load replication single structures",
    :arg => ext_args(vreg_list_set(1,"BHSD"), [{"adr" => "AdrNoOfs"}], {"vd" => "vt"}) +
            ext_args(qvreg_list_set(1,"BHSD"), [{"adr" => "AdrNoOfs"}], {"vd" => "vt"}),
    # :arg => [[ {"vt" => "VReg"}, {"adr" => "AdrNoOfs"}]],
    :prm => ["L", "R", "opcode", "S", "vt", "adr"],
    :grp => [
      {"LD4R"   => {"L" => 1, "R" => 1, "opcode" => 7, "S" => 0}},
      {"LD3R"   => {"L" => 1, "R" => 0, "opcode" => 7, "S" => 0}},
      {"LD2R"   => {"L" => 1, "R" => 1, "opcode" => 6, "S" => 0}},
      {"LD1R"   => {"L" => 1, "R" => 0, "opcode" => 6, "S" => 0}},
    ]
  },

  "AdvSimdLdStSingleStructPostReg" => {
    :cmt => "Advanced SIMD load/store single structures (post-indexed register)",
    :arg => ext_args(vreg_elem_set(1,"BHSD"), [{"adr" => "AdrPostReg"}], {"vd" => "vt"}),
    # :arg => [[ {"vt" => "VRegElem"}, {"adr" => "AdrPostReg"}]],
    :prm => ["L", "R", "num", "vt", "adr"],
    :grp => [
      {"ST4"   => {"L" => 0, "R" => 1, "num" => 4}},
      {"ST3"   => {"L" => 0, "R" => 0, "num" => 3}},
      {"ST2"   => {"L" => 0, "R" => 1, "num" => 2}},
      {"ST1"   => {"L" => 0, "R" => 0, "num" => 1}},
      {"LD4"   => {"L" => 1, "R" => 1, "num" => 4}},
      {"LD3"   => {"L" => 1, "R" => 0, "num" => 3}},
      {"LD2"   => {"L" => 1, "R" => 1, "num" => 2}},
      {"LD1"   => {"L" => 1, "R" => 0, "num" => 1}}
    ]
  },

  "AdvSimdLdStSingleStructRepPostReg" => {
    :cmt => "Advanced SIMD load/store single structures (post-indexed register, replicate)",
    :arg => ext_args(vreg_list_set(1,"BHSD"), [{"adr" => "AdrPostReg"}], {"vd" => "vt"}) +
            ext_args(qvreg_list_set(1,"BHSD"), [{"adr" => "AdrPostReg"}], {"vd" => "vt"}),
    # :arg => [[ {"vt" => "VReg"}, {"adr" => "AdrPostReg"}]],
    :prm => ["L", "R", "opcode", "S", "vt", "adr"],
    :grp => [
      {"LD4R"   => {"L" => 1, "R" => 1, "opcode" => 7, "S" => 0}},
      {"LD3R"   => {"L" => 1, "R" => 0, "opcode" => 7, "S" => 0}},
      {"LD2R"   => {"L" => 1, "R" => 1, "opcode" => 6, "S" => 0}},
      {"LD1R"   => {"L" => 1, "R" => 0, "opcode" => 6, "S" => 0}},
    ]
  },

  "AdvSimdLdStSingleStructPostImm" => {
    :cmt => "Advanced SIMD load/store single structures (post-indexed immediate)",
    :arg => ext_args(vreg_elem_set(1,"BHSD"), [{"adr" => "AdrPostImm"}], {"vd" => "vt"}),
    # :arg => [[ {"vt" => "VRegElem"}, {"adr" => "AdrPostImm"}]],
    :prm => ["L", "R", "num", "vt", "adr"],
    :grp => [
      {"ST4"   => {"L" => 0, "R" => 1, "num" => 4}},
      {"ST3"   => {"L" => 0, "R" => 0, "num" => 3}},
      {"ST2"   => {"L" => 0, "R" => 1, "num" => 2}},
      {"ST1"   => {"L" => 0, "R" => 0, "num" => 1}},
      {"LD4"   => {"L" => 1, "R" => 1, "num" => 4}},
      {"LD3"   => {"L" => 1, "R" => 0, "num" => 3}},
      {"LD2"   => {"L" => 1, "R" => 1, "num" => 2}},
      {"LD1"   => {"L" => 1, "R" => 0, "num" => 1}},
    ]
  },

  "AdvSimdLdRepSingleStructPostImm" => {
    :cmt => "Advanced SIMD load replication single structures (post-indexed immediate)",
    :arg => ext_args(vreg_list_set(1,"BHSD"), [{"adr" => "AdrPostImm"}], {"vd" => "vt"}) +
            ext_args(qvreg_list_set(1,"BHSD"), [{"adr" => "AdrPostImm"}], {"vd" => "vt"}),
    # :arg => [[ {"vt" => "VReg"}, {"adr" => "AdrPostImm"}]],
    :prm => ["L", "R", "opcode", "S", "vt", "adr"],
    :grp => [
      {"LD4R"   => {"L" => 1, "R" => 1, "opcode" => 7, "S" => 0}},
      {"LD3R"   => {"L" => 1, "R" => 0, "opcode" => 7, "S" => 0}},
      {"LD2R"   => {"L" => 1, "R" => 1, "opcode" => 6, "S" => 0}},
      {"LD1R"   => {"L" => 1, "R" => 0, "opcode" => 6, "S" => 0}},
    ]
  },

  "StExclusive" => {
    :cmt => "store exclusive",
    :arg => [
      [ {"ws" => "WReg"}, {"rt" => "WReg"}, {"adr" => "AdrImm"}],   #2
      [ {"ws" => "WReg"}, {"rt" => "XReg"}, {"adr" => "AdrImm"}]    #3
    ],
    :prm => ["size", "o0", "ws", "rt", "adr"],
    :grp => [
      {"STXRB"   => {"size" => 0, "o0" => 0}, :arg => [0]},
      {"STLXRB"  => {"size" => 0, "o0" => 1}, :arg => [0]},
      {"STXRH"   => {"size" => 1, "o0" => 0}, :arg => [0]},
      {"STLXRH"  => {"size" => 1, "o0" => 1}, :arg => [0]},
      {"STXR"    => {"size" => 2, "o0" => 0}, :arg => [0]},
      {"STLXR"   => {"size" => 2, "o0" => 1}, :arg => [0]},
      {"STXR"    => {"size" => 3, "o0" => 0}, :arg => [1]},
      {"STLXR"   => {"size" => 3, "o0" => 1}, :arg => [1]}
    ]
  },
  
  "LdExclusive" => {
    :cmt => "load exclusive",
    :arg => [
      [ {"rt" => "WReg"}, {"adr" => "AdrImm"}],   #0
      [ {"rt" => "XReg"}, {"adr" => "AdrImm"}]    #1
    ],
    :prm => ["size", "o0", "rt", "adr"],
    :grp => [
      {"LDXRB"   => {"size" => 0, "o0" => 0}, :arg=>[0]},
      {"LDAXRB"  => {"size" => 0, "o0" => 1}, :arg=>[0]},
      {"LDXRH"   => {"size" => 1, "o0" => 0}, :arg=>[0]},
      {"LDAXRH"  => {"size" => 1, "o0" => 1}, :arg=>[0]},
      {"LDXR"    => {"size" => 2, "o0" => 0}, :arg=>[0]},
      {"LDAXR"   => {"size" => 2, "o0" => 1}, :arg=>[0]},
      {"LDXR"    => {"size" => 3, "o0" => 0}, :arg=>[1]},
      {"LDAXR"   => {"size" => 3, "o0" => 1}, :arg=>[1]}
    ]
  },
  
  "StLORelase" => {
    :cmt => "store LORelease",
    :arg => [
      [ {"rt" => "WReg"}, {"adr" => "AdrImm"}],   #0
      [ {"rt" => "XReg"}, {"adr" => "AdrImm"}]    #1
    ],
    :prm => ["size", "o0", "rt", "adr"],
    :grp => [
      {"STLLRB" => {"size" => 0, "o0" => 0}, :arg=>[0]},
      {"STLRB"  => {"size" => 0, "o0" => 1}, :arg=>[0]},
      {"STLLRH" => {"size" => 1, "o0" => 0}, :arg=>[0]},
      {"STLRH"  => {"size" => 1, "o0" => 1}, :arg=>[0]},
      {"STLLR"  => {"size" => 2, "o0" => 0}, :arg=>[0]},
      {"STLR"   => {"size" => 2, "o0" => 1}, :arg=>[0]},
      {"STLLR"  => {"size" => 3, "o0" => 0}, :arg=>[1]},
      {"STLR"   => {"size" => 3, "o0" => 1}, :arg=>[1]}
    ]
  },
  
  "LdLOAcquire" => {
    :cmt => "load LOAcquire",
    :arg => [
      [ {"rt" => "WReg"}, {"adr" => "AdrImm"}],   #0
      [ {"rt" => "XReg"}, {"adr" => "AdrImm"}]    #1
    ],
    :prm => ["size", "o0", "rt", "adr"],
    :grp => [
      {"LDLARB" => {"size" => 0, "o0" => 0}, :arg => [0]},
      {"LDARB"  => {"size" => 0, "o0" => 1}, :arg => [0]},
      {"LDLARH" => {"size" => 1, "o0" => 0}, :arg => [0]},
      {"LDARH"  => {"size" => 1, "o0" => 1}, :arg => [0]},
      {"LDLAR"  => {"size" => 2, "o0" => 0}, :arg => [0]},
      {"LDAR"   => {"size" => 2, "o0" => 1}, :arg => [0]},
      {"LDLAR"  => {"size" => 3, "o0" => 0}, :arg => [1]},
      {"LDAR"   => {"size" => 3, "o0" => 1}, :arg => [1]}
    ]
  },
  
  "Cas" => {
    :cmt => "compare and swap",
    :arg => [
      [ {"rs" => "WReg"}, {"rt" => "WReg"}, {"adr" => "AdrNoOfs"}], #0
      [ {"rs" => "XReg"}, {"rt" => "XReg"}, {"adr" => "AdrNoOfs"}]  #1
    ],
    :prm => ["size", "o2", "L", "o1", "o0", "rs", "rt", "adr"],
    :grp => [
      {"CASB"    => {"size" => 0, "o2" => 1, "L" => 0, "o1" => 1, "o0" => 0}, :arg => [0]},
      {"CASLB"   => {"size" => 0, "o2" => 1, "L" => 0, "o1" => 1, "o0" => 1}, :arg => [0]},
      {"CASAB"   => {"size" => 0, "o2" => 1, "L" => 1, "o1" => 1, "o0" => 0}, :arg => [0]},
      {"CASALB"  => {"size" => 0, "o2" => 1, "L" => 1, "o1" => 1, "o0" => 1}, :arg => [0]},
      {"CASH"    => {"size" => 1, "o2" => 1, "L" => 0, "o1" => 1, "o0" => 0}, :arg => [0]},
      {"CASLH"   => {"size" => 1, "o2" => 1, "L" => 0, "o1" => 1, "o0" => 1}, :arg => [0]},
      {"CASAH"   => {"size" => 1, "o2" => 1, "L" => 1, "o1" => 1, "o0" => 0}, :arg => [0]},
      {"CASALH"  => {"size" => 1, "o2" => 1, "L" => 1, "o1" => 1, "o0" => 1}, :arg => [0]},
      {"CAS"     => {"size" => 2, "o2" => 1, "L" => 0, "o1" => 1, "o0" => 0}, :arg => [0]},
      {"CASL"    => {"size" => 2, "o2" => 1, "L" => 0, "o1" => 1, "o0" => 1}, :arg => [0]},
      {"CASA"    => {"size" => 2, "o2" => 1, "L" => 1, "o1" => 1, "o0" => 0}, :arg => [0]},
      {"CASAL"   => {"size" => 2, "o2" => 1, "L" => 1, "o1" => 1, "o0" => 1}, :arg => [0]},
      {"CAS"     => {"size" => 3, "o2" => 1, "L" => 0, "o1" => 1, "o0" => 0}, :arg => [1]},
      {"CASL"    => {"size" => 3, "o2" => 1, "L" => 0, "o1" => 1, "o0" => 1}, :arg => [1]},
      {"CASA"    => {"size" => 3, "o2" => 1, "L" => 1, "o1" => 1, "o0" => 0}, :arg => [1]},
      {"CASAL"   => {"size" => 3, "o2" => 1, "L" => 1, "o1" => 1, "o0" => 1}, :arg => [1]}
    ]
  },

  "StExclusivePair" => {
    :cmt => "load/store exclusive pair",
    :arg => [
      [ {"ws" => "WReg"}, {"rt1" => "WReg"}, {"rt2" => "WReg"}, {"adr" => "AdrImm"}],   #0
      [ {"ws" => "WReg"}, {"rt1" => "XReg"}, {"rt2" => "XReg"}, {"adr" => "AdrImm"}]    #1
    ],
    :prm => ["L", "o1", "o0", "ws", "rt1", "rt2", "adr"],
    :grp => [
      {"STXP"    => {"L" => 0, "o1" => 1, "o0" => 0}},
      {"STLXP"   => {"L" => 0, "o1" => 1, "o0" => 1}}
    ]
  },

  "LdExclusivePair" => {
    :cmt => "load/store exclusive pair",
    :arg => [
      [ {"rt1" => "WReg"}, {"rt2" => "WReg"}, {"adr" => "AdrImm"}],   #0
      [ {"rt1" => "XReg"}, {"rt2" => "XReg"}, {"adr" => "AdrImm"}]    #1
    ],
    :prm => ["L", "o1", "o0", "rt1", "rt2", "adr"],
    :grp => [
      {"LDXP"    => {"L" => 1, "o1" => 1, "o0" => 0}},
      {"LDAXP"   => {"L" => 1, "o1" => 1, "o0" => 1}}
    ]
  },

  "CasPair" => {
    :cmt => "compare and swap pair",
    :arg => [
      [ {"rs" => "WReg"}, {"rt" => "WReg"}, {"adr" => "AdrNoOfs"}], #0
      [ {"rs" => "XReg"}, {"rt" => "XReg"}, {"adr" => "AdrNoOfs"}]  #1
    ],
    :prm => ["L", "o1", "o0", "rs", "rt", "adr"],
    :grp => [
      {"CASP"   => {"L" => 0, "o1" => 1, "o0" => 0}},
      {"CASPL"  => {"L" => 0, "o1" => 1, "o0" => 1}},
      {"CASPA"  => {"L" => 1, "o1" => 1, "o0" => 0}},
      {"CASPAL" => {"L" => 1, "o1" => 1, "o0" => 1}}
    ]
  },

  "LdaprStlr" => {
    :cmt => "LDAPR/STLR (unscaled immediate)",
    :arg => [
      [ {"rt" => "WReg"}, {"adr" => "AdrImm"}], #0
      [ {"rt" => "XReg"}, {"adr" => "AdrImm"}]  #1
    ],
    :prm => ["size", "opc", "rt", "adr"],
    :grp => [
      {"STLURB"   => {"size" => 0, "opc" => 0}, :arg => [0]},
      {"LDAPURB"  => {"size" => 0, "opc" => 1}, :arg => [0]},
      {"LDAPURSB" => {"size" => 0, "opc" => 2}, :arg => [1]},
      {"LDAPURSB" => {"size" => 0, "opc" => 3}, :arg => [0]},
      {"STLURH"   => {"size" => 1, "opc" => 0}, :arg => [0]},
      {"LDAPURH"  => {"size" => 1, "opc" => 1}, :arg => [0]},
      {"LDAPURSH" => {"size" => 1, "opc" => 2}, :arg => [1]},
      {"LDAPURSH" => {"size" => 1, "opc" => 3}, :arg => [0]},
      {"STLUR"    => {"size" => 2, "opc" => 0}, :arg => [0]},
      {"LDAPUR"   => {"size" => 2, "opc" => 1}, :arg => [0]},
      {"LDAPURSW" => {"size" => 2, "opc" => 2}, :arg => [1]},
      {"STLUR"    => {"size" => 3, "opc" => 0}, :arg => [1]},
      {"LDAPUR"   => {"size" => 3, "opc" => 1}, :arg => [1]}
    ]
  },
  
  "LdRegLiteral" => {
    :cmt => "load register (literal)",
    :arg => [
      [ {"rt" => "WReg"}, {"label" => "Label"}],   #0
      [ {"rt" => "XReg"}, {"label" => "Label"}],   #1
      [ {"rt" => "WReg"}, {"label" => "int64_t"}], #2
      [ {"rt" => "XReg"}, {"label" => "int64_t"}]  #3
    ],
    :prm => ["opc", "V", "rt", "label"],
    :grp => [
      {"LDR"   => {"opc" => "(rt.getBit() == 64)? 1 : 0", "V" => 0}},
      {"LDRSW" => {"opc" => 2, "V" => 0}},
    ]
  },
  
  "LdRegSimdFpLiteral" => {
    :cmt => "load register (SIMD&FP, literal)",
    :arg => ext_args(screg_set(1,"SDQ"), [{"label" => "Label"}], {"vd" => "vt"}) +  #0-2
            ext_args(screg_set(1,"SDQ"), [{"label" => "int64_t"}], {"vd" => "vt"}), #3-5
    :prm => ["vt", "label"],
    :grp => [
      {"LDR"   => {}},
    ]
  },
  
  "PfLiteral" => {
    :cmt => "prefetch (literal)",
    :arg => [
      [ {"prfop" => "Prfop"}, {"label" => "Label"}],  #0
      [ {"prfop" => "Prfop"}, {"label" => "int64_t"}] #1
    ],
    :prm => ["prfop", "label"],
    :grp => [
      {"PRFM" => {}},
    ]
  },

  "LdStNoAllocPair" => {
    :cmt => "Load/store no-allocate pair (offset)",
    :arg => [
      [ {"rt1" => "WReg"}, {"rt2" => "WReg"}, {"adr" => "AdrImm"}], #0
      [ {"rt1" => "XReg"}, {"rt2" => "XReg"}, {"adr" => "AdrImm"}]  #1
    ],
    :prm => ["L", "rt1", "rt2", "adr"],
    :grp => [
      {"STNP" => {"L" => 0}},
      {"LDNP" => {"L" => 1}},
    ]
  },

  "LdStSimdFpNoAllocPair" => {
    :cmt => "Load/store no-allocate pair (offset)",
    :arg => ext_args(screg_set(2,"SDQ"), [{"adr" => "AdrImm"}], {"vd" => "vt1", "vn" => "vt2"}),
    # :arg => [[ {"vt1" => "VReg"}, {"vt2" => "VReg"}, {"adr" => "AdrImm"}]],
    :prm => ["L", "vt1", "vt2", "adr"],
    :grp => [
      {"STNP" => {"L" => 0}},
      {"LDNP" => {"L" => 1}},
    ]
  },

  "LdStRegPairPostImm" => {
    :cmt => "Load/store pair (post-indexed)",
    :arg => [
      [ {"rt1" => "WReg"}, {"rt2" => "WReg"}, {"adr" => "AdrPostImm"}], #0
      [ {"rt1" => "XReg"}, {"rt2" => "XReg"}, {"adr" => "AdrPostImm"}]  #1
    ],
    :prm => ["opc", "L", "rt1", "rt2", "adr"],
    :grp => [
      {"STP"   => {"opc" => "(rt1.getBit() == 32)? 0 : 2", "L" => 0}},
      {"LDP"   => {"opc" => "(rt1.getBit() == 32)? 0 : 2", "L" => 1}},
      {"LDPSW" => {"opc" => 1, "L" => 1}, :arg => [1]},
    ]
  },

  "LdStSimdFpPairPostImm" => {
    :cmt => "Load/store pair (post-indexed)",
    :arg => ext_args(screg_set(2,"SDQ"), [{"adr" => "AdrPostImm"}], {"vd" => "vt1", "vn" => "vt2"}),
    # :arg => [[ {"vt1" => "VReg"}, {"vt2" => "VReg"}, {"adr" => "AdrPostImm"}]],
    :prm => ["L", "vt1", "vt2", "adr"],
    :grp => [
      {"STP" => {"L" => 0}},
      {"LDP" => {"L" => 1}},
    ]
  },

  "LdStRegPair" => {
    :cmt => "Load/store pair (offset)",
    :arg => [
      [ {"rt1" => "WReg"}, {"rt2" => "WReg"}, {"adr" => "AdrImm"}], #0
      [ {"rt1" => "XReg"}, {"rt2" => "XReg"}, {"adr" => "AdrImm"}]  #1
    ],
    :prm => ["opc", "L", "rt1", "rt2", "adr"],
    :grp => [
      {"STP"   => {"opc" => "(rt1.getBit() == 32)? 0 : 2", "L" => 0}},
      {"LDP"   => {"opc" => "(rt1.getBit() == 32)? 0 : 2", "L" => 1}},
      {"LDPSW" => {"opc" => 1, "L" => 1}, :arg => [1]},
    ]
  },

  "LdStSimdFpPair" => {
    :cmt => "Load/store pair (offset)",
    :arg => ext_args(screg_set(2,"SDQ"), [{"adr" => "AdrImm"}], {"vd" => "vt1", "vn" => "vt2"}),
    # :arg => [[ {"vt1" => "VReg"}, {"vt2" => "VReg"}, {"adr" => "AdrImm"}]],
    :prm => ["L", "vt1", "vt2", "adr"],
    :grp => [
      {"STP" => {"L" => 0}},
      {"LDP" => {"L" => 1}},
    ]
  },

  "LdStRegPairPre" => {
    :cmt => "Load/store pair (pre-indexed)",
    :arg => [
      [ {"rt1" => "WReg"}, {"rt2" => "WReg"}, {"adr" => "AdrPreImm"}], #0
      [ {"rt1" => "XReg"}, {"rt2" => "XReg"}, {"adr" => "AdrPreImm"}]  #1
    ],
    :prm => ["opc", "L", "rt1", "rt2", "adr"],
    :grp => [
      {"STP"   => {"opc" => "(rt1.getBit() == 32)? 0 : 2", "L" => 0}},
      {"LDP"   => {"opc" => "(rt1.getBit() == 32)? 0 : 2", "L" => 1}},
      {"LDPSW" => {"opc" => 1, "L" => 1}, :arg => [1]},
    ]
  },

  "LdStSimdFpPairPre" => {
    :cmt => "Load/store pair (pre-indexed)",
    :arg => ext_args(screg_set(2,"SDQ"), [{"adr" => "AdrPreImm"}], {"vd" => "vt1", "vn" => "vt2"}),
    # :arg => [[ {"vt1" => "VReg"}, {"vt2" => "VReg"}, {"adr" => "AdrPreImm"}]],
    :prm => ["L", "vt1", "vt2", "adr"],
    :grp => [
      {"STP" => {"L" => 0}},
      {"LDP" => {"L" => 1}},
    ]
  },

  "LdStRegUnsImm" => {
    :cmt => "Load/store register (unscaled immediate)",
    :arg => [
      [ {"rt" => "WReg"}, {"adr" => "AdrImm"}], #0
      [ {"rt" => "XReg"}, {"adr" => "AdrImm"}]  #1
    ],
    :prm => ["size", "opc", "rt", "adr"],
    :grp => [
      {"STURB"  => {"size" => 0, "opc" => 0}, :arg => [0]},
      {"LDURB"  => {"size" => 0, "opc" => 1}, :arg => [0]},
      {"LDURSB" => {"size" => 0, "opc" => 3}, :arg => [0]},
      {"STURH"  => {"size" => 1, "opc" => 0}, :arg => [0]},
      {"LDURH"  => {"size" => 1, "opc" => 1}, :arg => [0]},
      {"LDURSH" => {"size" => 1, "opc" => 3}, :arg => [0]},
      {"STUR"   => {"size" => 2, "opc" => 0}, :arg => [0]},
      {"LDUR"   => {"size" => 2, "opc" => 1}, :arg => [0]},
      {"LDURSB" => {"size" => 0, "opc" => 2}, :arg => [1]},
      {"LDURSH" => {"size" => 1, "opc" => 2}, :arg => [1]},
      {"LDURSW" => {"size" => 2, "opc" => 2}, :arg => [1]},
      {"STUR"   => {"size" => 3, "opc" => 0}, :arg => [1]},
      {"LDUR"   => {"size" => 3, "opc" => 1}, :arg => [1]}
    ]
  },
  
  "LdStSimdFpRegUnsImm" => {
    :cmt => "Load/store register (SIMD&FP, unscaled immediate)",
    :arg => ext_args(screg_set(1,"BHSDQ"), [{"adr" => "AdrImm"}], {"vd" => "vt"}),
    # :arg => [[ {"vt" => "VReg"}, {"adr" => "AdrImm"}]],
    :prm => ["opc", "vt", "adr"],
    :grp => [
      {"STUR"   => {"opc" => "(vt.getBit() != 128)? 0 : 2"}},
      {"LDUR"   => {"opc" => "(vt.getBit() != 128)? 1 : 3"}}
    ]
  },

  "PfRegUnsImm" => {
    :cmt => "prefetch register (unscaled immediate)",
    :arg => [[ {"prfop" => "Prfop"}, {"adr" => "AdrImm"}]],
    :prm => ["prfop", "adr"],
    :grp => [
      {"PRFUM"   => {}}
    ]
  },

  "LdStRegPostImm" => {
    :cmt => "Load/store register (immediate post-indexed)",
    :arg => [
      [ {"rt" => "WReg"}, {"adr" => "AdrPostImm"}],  #0
      [ {"rt" => "XReg"}, {"adr" => "AdrPostImm"}]   #1
    ],
    :prm => ["size", "opc", "rt", "adr"],
    :grp => [
      {"STRB"  => {"size" => 0, "opc" => 0}, :arg => [0]},
      {"LDRB"  => {"size" => 0, "opc" => 1}, :arg => [0]},
      {"LDRSB" => {"size" => 0, "opc" => 3}, :arg => [0]},
      {"STRH"  => {"size" => 1, "opc" => 0}, :arg => [0]},
      {"LDRH"  => {"size" => 1, "opc" => 1}, :arg => [0]},
      {"LDRSH" => {"size" => 1, "opc" => 3}, :arg => [0]},
      {"STR"   => {"size" => 2, "opc" => 0}, :arg => [0]},
      {"LDR"   => {"size" => 2, "opc" => 1}, :arg => [0]},
      {"LDRSB" => {"size" => 0, "opc" => 2}, :arg => [1]},
      {"LDRSH" => {"size" => 1, "opc" => 2}, :arg => [1]},
      {"LDRSW" => {"size" => 2, "opc" => 2}, :arg => [1]},
      {"STR"   => {"size" => 3, "opc" => 0}, :arg => [1]},
      {"LDR"   => {"size" => 3, "opc" => 1}, :arg => [1]}
    ]
  },

  "LdStSimdFpRegPostImm" => {
    :cmt => "Load/store register (SIMD&FP, immediate post-indexed)",
    :arg => ext_args(screg_set(1,"BHSDQ"), [{"adr" => "AdrPostImm"}], {"vd" => "vt"}),
    # :arg => [[ {"vt" => "VReg"}, {"adr" => "AdrPostImm"}]],
    :prm => ["opc", "vt", "adr"],
    :grp => [
      {"STR"   => {"opc" => "(vt.getBit() != 128)? 0 : 2"}},
      {"LDR"   => {"opc" => "(vt.getBit() != 128)? 1 : 3"}}
    ]
  },

  "LdStRegUnpriv" => {
    :cmt => "Load/store register (unprivileged)",
    :arg => [
      [ {"rt" => "WReg"}, {"adr" => "AdrImm"}], #0
      [ {"rt" => "XReg"}, {"adr" => "AdrImm"}]  #1
    ],
    :prm => ["size", "opc", "rt", "adr"],
    :grp => [
      {"STTRB"  => {"size" => 0, "opc" => 0}, :arg => [0]},
      {"LDTRB"  => {"size" => 0, "opc" => 1}, :arg => [0]},
      {"LDTRSB" => {"size" => 0, "opc" => 3}, :arg => [0]},
      {"STTRH"  => {"size" => 1, "opc" => 0}, :arg => [0]},
      {"LDTRH"  => {"size" => 1, "opc" => 1}, :arg => [0]},
      {"LDTRSH" => {"size" => 1, "opc" => 3}, :arg => [0]},
      {"STTR"   => {"size" => 2, "opc" => 0}, :arg => [0]},
      {"LDTR"   => {"size" => 2, "opc" => 1}, :arg => [0]},
      {"LDTRSB" => {"size" => 0, "opc" => 2}, :arg => [1]},
      {"LDTRSH" => {"size" => 1, "opc" => 2}, :arg => [1]},
      {"LDTRSW" => {"size" => 2, "opc" => 2}, :arg => [1]},
      {"STTR"   => {"size" => 3, "opc" => 0}, :arg => [1]},
      {"LDTR"   => {"size" => 3, "opc" => 1}, :arg => [1]}
    ]
  },

  "LdStRegPre" => {
    :cmt => "Load/store register (immediate pre-indexed)",
    :arg => [
      [ {"rt" => "WReg"}, {"adr" => "AdrPreImm"}], #0
      [ {"rt" => "XReg"}, {"adr" => "AdrPreImm"}]  #1
    ],
    :prm => ["size", "opc", "rt", "adr"],
    :grp => [
      {"STRB"  => {"size" => 0, "opc" => 0}, :arg => [0]},
      {"LDRB"  => {"size" => 0, "opc" => 1}, :arg => [0]},
      {"LDRSB" => {"size" => 0, "opc" => 3}, :arg => [0]},
      {"STRH"  => {"size" => 1, "opc" => 0}, :arg => [0]},
      {"LDRH"  => {"size" => 1, "opc" => 1}, :arg => [0]},
      {"LDRSH" => {"size" => 1, "opc" => 3}, :arg => [0]},
      {"STR"   => {"size" => 2, "opc" => 0}, :arg => [0]},
      {"LDR"   => {"size" => 2, "opc" => 1}, :arg => [0]},
      {"LDRSB" => {"size" => 0, "opc" => 2}, :arg => [1]},
      {"LDRSH" => {"size" => 1, "opc" => 2}, :arg => [1]},
      {"LDRSW" => {"size" => 2, "opc" => 2}, :arg => [1]},
      {"STR"   => {"size" => 3, "opc" => 0}, :arg => [1]},
      {"LDR"   => {"size" => 3, "opc" => 1}, :arg => [1]}
    ]
  },

  "LdStSimdFpRegPre" => {
    :cmt => "Load/store register (SIMD&FP, immediate pre-indexed)",
    :arg => ext_args(screg_set(1,"BHSDQ"), [{"adr" => "AdrPreImm"}], {"vd" => "vt"}),
    # :arg => [[ {"vt" => "VReg"}, {"adr" => "AdrPreImm"}]],
    :prm => ["opc", "vt", "adr"],
    :grp => [
      {"STR"   => {"opc" => "(vt.getBit() != 128)? 0 : 2"}},
      {"LDR"   => {"opc" => "(vt.getBit() != 128)? 1 : 3"}}
    ]
  },

  "AtomicMemOp" => {
    :cmt => "Atomic memory oprations",
    :arg => [
      [ {"rs" => "WReg"}, {"rt" => "WReg"}, {"adr" => "AdrNoOfs"}], #0
      [ {"rs" => "XReg"}, {"rt" => "XReg"}, {"adr" => "AdrNoOfs"}], #1
      [ {"rs" => "WReg"}, {"adr" => "AdrNoOfs"}],                   #2
      [ {"rs" => "XReg"}, {"adr" => "AdrNoOfs"}],                   #3
      [ {"rt" => "WReg"}, {"adr" => "AdrImm"}],                     #4
      [ {"rt" => "XReg"}, {"adr" => "AdrImm"}]                      #5
    ],
    :prm => ["size", "V", "A", "R", "o3", "opc", "rs", "rt", "adr"],
    :grp => [
      {"LDADDB"    => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 0}, :arg => [0]},
      {"LDCLRB"    => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 1}, :arg => [0]},
      {"LDEORB"    => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 2}, :arg => [0]},
      {"LDSETB"    => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 3}, :arg => [0]},
      {"LDSMAXB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 4}, :arg => [0]},
      {"LDSMINB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 5}, :arg => [0]},
      {"LDUMAXB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 6}, :arg => [0]},
      {"STUMAXB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 6}, :arg => [0]},
      {"LDUMINB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 7}, :arg => [0]},
      {"SWPB"      => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 1, "opc" => 0}, :arg => [0]},
      {"LDADDLB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 0}, :arg => [0]},
      {"LDCLRLB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 1}, :arg => [0]},
      {"LDEORLB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 2}, :arg => [0]},
      {"LDSETLB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 3}, :arg => [0]},
      {"LDSMAXLB"  => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 4}, :arg => [0]},
      {"LDSMINLB"  => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 5}, :arg => [0]},
      {"LDUMAXLB"  => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 6}, :arg => [0]},
      {"LDUMINLB"  => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 7}, :arg => [0]},
      {"SWPLB"     => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 1, "opc" => 0}, :arg => [0]},
      {"LDADDAB"   => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 0}, :arg => [0]},
      {"LDCLRAB"   => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 1}, :arg => [0]},
      {"LDEORAB"   => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 2}, :arg => [0]},
      {"LDSETAB"   => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 3}, :arg => [0]},
      {"LDSMAXAB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 4}, :arg => [0]},
      {"LDSMINAB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 5}, :arg => [0]},
      {"LDUMAXAB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 6}, :arg => [0]},
      {"LDUMINAB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 7}, :arg => [0]},
      {"SWPAB"     => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 1, "opc" => 0}, :arg => [0]},
      {"LDAPRB"    => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 1, "opc" => 4, "rs" => "WReg(31)"}, :arg => [4]},
      {"LDADDALB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 0}, :arg => [0]},
      {"LDCLRALB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 1}, :arg => [0]},
      {"LDEORALB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 2}, :arg => [0]},
      {"LDSETALB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 3}, :arg => [0]},
      {"LDSMAXALB" => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 4}, :arg => [0]},
      {"LDSMINALB" => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 5}, :arg => [0]},
      {"LDUMAXALB" => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 6}, :arg => [0]},
      {"LDUMINALB" => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 7}, :arg => [0]},
      {"SWPALB"    => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 1, "opc" => 0}, :arg => [0]},
      {"LDADDH"    => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 0}, :arg => [0]},
      {"LDCLRH"    => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 1}, :arg => [0]},
      {"LDEORH"    => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 2}, :arg => [0]},
      {"LDSETH"    => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 3}, :arg => [0]},
      {"LDSMAXH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 4}, :arg => [0]},
      {"LDSMINH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 5}, :arg => [0]},
      {"LDUMAXH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 6}, :arg => [0]},
      {"LDUMINH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 7}, :arg => [0]},
      {"SWPH"      => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 1, "opc" => 0}, :arg => [0]},
      {"LDADDLH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 0}, :arg => [0]},
      {"LDCLRLH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 1}, :arg => [0]},
      {"LDEORLH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 2}, :arg => [0]},
      {"LDSETLH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 3}, :arg => [0]},
      {"LDSMAXLH"  => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 4}, :arg => [0]},
      {"LDSMINLH"  => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 5}, :arg => [0]},
      {"LDUMAXLH"  => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 6}, :arg => [0]},
      {"LDUMINLH"  => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 7}, :arg => [0]},
      {"SWPLH"     => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 1, "opc" => 0}, :arg => [0]},
      {"LDADDAH"   => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 0}, :arg => [0]},
      {"LDCLRAH"   => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 1}, :arg => [0]},
      {"LDEORAH"   => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 2}, :arg => [0]},
      {"LDSETAH"   => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 3}, :arg => [0]},
      {"LDSMAXAH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 4}, :arg => [0]},
      {"LDSMINAH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 5}, :arg => [0]},
      {"LDUMAXAH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 6}, :arg => [0]},
      {"LDUMINAH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 7}, :arg => [0]},
      {"SWPAH"     => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 1, "opc" => 0}, :arg => [0]},
      {"LDAPRH"    => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 1, "opc" => 4, "rs" => "WReg(31)"}, :arg => [4]},
      {"LDADDALH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 0}, :arg => [0]},
      {"LDCLRALH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 1}, :arg => [0]},
      {"LDEORALH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 2}, :arg => [0]},
      {"LDSETALH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 3}, :arg => [0]},
      {"LDSMAXALH" => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 4}, :arg => [0]},
      {"LDSMINALH" => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 5}, :arg => [0]},
      {"LDUMAXALH" => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 6}, :arg => [0]},
      {"LDUMINALH" => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 7}, :arg => [0]},
      {"SWPALH"    => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 1, "opc" => 0}, :arg => [0]},
      {"LDADD"     => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 0}, :arg => [0]},
      {"LDCLR"     => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 1}, :arg => [0]},
      {"LDEOR"     => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 2}, :arg => [0]},
      {"LDSET"     => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 3}, :arg => [0]},
      {"LDSMAX"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 4}, :arg => [0]},
      {"LDSMIN"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 5}, :arg => [0]},
      {"LDUMAX"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 6}, :arg => [0]},
      {"LDUMIN"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 7}, :arg => [0]},
      {"SWP"       => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 1, "opc" => 0}, :arg => [0]},
      {"LDADDL"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 0}, :arg => [0]},
      {"LDCLRL"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 1}, :arg => [0]},
      {"LDEORL"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 2}, :arg => [0]},
      {"LDSETL"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 3}, :arg => [0]},
      {"LDSMAXL"   => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 4}, :arg => [0]},
      {"LDSMINL"   => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 5}, :arg => [0]},
      {"LDUMAXL"   => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 6}, :arg => [0]},
      {"LDUMINL"   => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 7}, :arg => [0]},
      {"SWPL"      => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 1, "opc" => 0}, :arg => [0]},
      {"LDADDA"    => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 0}, :arg => [0]},
      {"LDCLRA"    => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 1}, :arg => [0]},
      {"LDEORA"    => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 2}, :arg => [0]},
      {"LDSETA"    => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 3}, :arg => [0]},
      {"LDSMAXA"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 4}, :arg => [0]},
      {"LDSMINA"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 5}, :arg => [0]},
      {"LDUMAXA"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 6}, :arg => [0]},
      {"LDUMINA"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 7}, :arg => [0]},
      {"SWPA"      => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 1, "opc" => 0}, :arg => [0]},
      {"LDAPR"     => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 1, "opc" => 4, "rs" => "WReg(31)"}, :arg => [4]},
      {"LDADDAL"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 0}, :arg => [0]},
      {"LDCLRAL"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 1}, :arg => [0]},
      {"LDEORAL"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 2}, :arg => [0]},
      {"LDSETAL"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 3}, :arg => [0]},
      {"LDSMAXAL"  => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 4}, :arg => [0]},
      {"LDSMINAL"  => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 5}, :arg => [0]},
      {"LDUMAXAL"  => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 6}, :arg => [0]},
      {"LDUMINAL"  => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 7}, :arg => [0]},
      {"SWPAL"     => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 1, "opc" => 0}, :arg => [0]},
      {"LDADD"     => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 0}, :arg => [1]},
      {"LDCLR"     => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 1}, :arg => [1]},
      {"LDEOR"     => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 2}, :arg => [1]},
      {"LDSET"     => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 3}, :arg => [1]},
      {"LDSMAX"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 4}, :arg => [1]},
      {"LDSMIN"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 5}, :arg => [1]},
      {"LDUMAX"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 6}, :arg => [1]},
      {"LDUMIN"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 7}, :arg => [1]},
      {"SWP"       => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 1, "opc" => 0}, :arg => [1]},
      {"LDADDL"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 0}, :arg => [1]},
      {"LDCLRL"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 1}, :arg => [1]},
      {"LDEORL"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 2}, :arg => [1]},
      {"LDSETL"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 3}, :arg => [1]},
      {"LDSMAXL"   => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 4}, :arg => [1]},
      {"LDSMINL"   => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 5}, :arg => [1]},
      {"LDUMAXL"   => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 6}, :arg => [1]},
      {"LDUMINL"   => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 7}, :arg => [1]},
      {"SWPL"      => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 1, "opc" => 0}, :arg => [1]},
      {"LDADDA"    => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 0}, :arg => [1]},
      {"LDCLRA"    => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 1}, :arg => [1]},
      {"LDEORA"    => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 2}, :arg => [1]},
      {"LDSETA"    => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 3}, :arg => [1]},
      {"LDSMAXA"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 4}, :arg => [1]},
      {"LDSMINA"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 5}, :arg => [1]},
      {"LDUMAXA"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 6}, :arg => [1]},
      {"LDUMINA"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 7}, :arg => [1]},
      {"SWPA"      => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 1, "opc" => 0}, :arg => [1]},
      {"LDAPR"     => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 1, "opc" => 4, "rs" => "XReg(31)"}, :arg => [5]},
      {"LDADDAL"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 0}, :arg => [1]},
      {"LDCLRAL"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 1}, :arg => [1]},
      {"LDEORAL"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 2}, :arg => [1]},
      {"LDSETAL"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 3}, :arg => [1]},
      {"LDSMAXAL"  => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 4}, :arg => [1]},
      {"LDSMINAL"  => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 5}, :arg => [1]},
      {"LDUMAXAL"  => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 6}, :arg => [1]},
      {"LDUMINAL"  => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 7}, :arg => [1]},
      {"SWPAL"     => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 1, "opc" => 0}, :arg => [1]},

      # alias LD***
      {"STADDB"    => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 0, "rt" => "WReg(31)"}, :arg => [2]},
      {"STCLRB"    => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 1, "rt" => "WReg(31)"}, :arg => [2]},
      {"STEORB"    => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 2, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSETB"    => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 3, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMAXB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 4, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMINB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 5, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMAXB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 6, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMINB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 7, "rt" => "WReg(31)"}, :arg => [2]},
      {"STADDLB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 0, "rt" => "WReg(31)"}, :arg => [2]},
      {"STCLRLB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 1, "rt" => "WReg(31)"}, :arg => [2]},
      {"STEORLB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 2, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSETLB"   => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 3, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMAXLB"  => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 4, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMINLB"  => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 5, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMAXLB"  => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 6, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMINLB"  => {"size" => 0, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 7, "rt" => "WReg(31)"}, :arg => [2]},
      {"STADDAB"   => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 0, "rt" => "WReg(31)"}, :arg => [2]},
      {"STCLRAB"   => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 1, "rt" => "WReg(31)"}, :arg => [2]},
      {"STEORAB"   => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 2, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSETAB"   => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 3, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMAXAB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 4, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMINAB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 5, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMAXAB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 6, "rt" => "WReg(31)"}, :arg => [2]},
      {"STADDALB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 0, "rt" => "WReg(31)"}, :arg => [2]},
      {"STCLRALB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 1, "rt" => "WReg(31)"}, :arg => [2]},
      {"STEORALB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 2, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSETALB"  => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 3, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMAXALB" => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 4, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMINALB" => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 5, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMAXALB" => {"size" => 0, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 6, "rt" => "WReg(31)"}, :arg => [2]},
      {"STADDH"    => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 0, "rt" => "WReg(31)"}, :arg => [2]},
      {"STCLRH"    => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 1, "rt" => "WReg(31)"}, :arg => [2]},
      {"STEORH"    => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 2, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSETH"    => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 3, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMAXH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 4, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMINH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 5, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMAXH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 6, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMINH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 7, "rt" => "WReg(31)"}, :arg => [2]},
      {"STADDLH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 0, "rt" => "WReg(31)"}, :arg => [2]},
      {"STCLRLH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 1, "rt" => "WReg(31)"}, :arg => [2]},
      {"STEORLH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 2, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSETLH"   => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 3, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMAXLH"  => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 4, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMINLH"  => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 5, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMAXLH"  => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 6, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMINLH"  => {"size" => 1, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 7, "rt" => "WReg(31)"}, :arg => [2]},
      {"STADDAH"   => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 0, "rt" => "WReg(31)"}, :arg => [2]},
      {"STCLRAH"   => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 1, "rt" => "WReg(31)"}, :arg => [2]},
      {"STEORAH"   => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 2, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSETAH"   => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 3, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMAXAH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 4, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMINAH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 5, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMAXAH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 6, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMINAH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 7, "rt" => "WReg(31)"}, :arg => [2]},
      {"STADDALH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 0, "rt" => "WReg(31)"}, :arg => [2]},
      {"STCLRALH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 1, "rt" => "WReg(31)"}, :arg => [2]},
      {"STEORALH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 2, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSETALH"  => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 3, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMAXALH" => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 4, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMINALH" => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 5, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMAXALH" => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 6, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMINALH" => {"size" => 1, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 7, "rt" => "WReg(31)"}, :arg => [2]},
      {"STADD"     => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 0, "rt" => "WReg(31)"}, :arg => [2]},
      {"STCLR"     => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 1, "rt" => "WReg(31)"}, :arg => [2]},
      {"STEOR"     => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 2, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSET"     => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 3, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMAX"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 4, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMIN"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 5, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMAX"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 6, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMIN"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 7, "rt" => "WReg(31)"}, :arg => [2]},
      {"STADDL"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 0, "rt" => "WReg(31)"}, :arg => [2]},
      {"STCLRL"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 1, "rt" => "WReg(31)"}, :arg => [2]},
      {"STEORL"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 2, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSETL"    => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 3, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMAXL"   => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 4, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMINL"   => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 5, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMAXL"   => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 6, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMINL"   => {"size" => 2, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 7, "rt" => "WReg(31)"}, :arg => [2]},
      {"STADDA"    => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 0, "rt" => "WReg(31)"}, :arg => [2]},
      {"STCLRA"    => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 1, "rt" => "WReg(31)"}, :arg => [2]},
      {"STEORA"    => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 2, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSETA"    => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 3, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMAXA"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 4, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMINA"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 5, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMAXA"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 6, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMINA"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 7, "rt" => "WReg(31)"}, :arg => [2]},
      {"STADDAL"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 0, "rt" => "WReg(31)"}, :arg => [2]},
      {"STCLRAL"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 1, "rt" => "WReg(31)"}, :arg => [2]},
      {"STEORAL"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 2, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSETAL"   => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 3, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMAXAL"  => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 4, "rt" => "WReg(31)"}, :arg => [2]},
      {"STSMINAL"  => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 5, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMAXAL"  => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 6, "rt" => "WReg(31)"}, :arg => [2]},
      {"STUMINAL"  => {"size" => 2, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 7, "rt" => "WReg(31)"}, :arg => [2]},
      {"STADD"     => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 0, "rt" => "XReg(31)"}, :arg => [3]},
      {"STCLR"     => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 1, "rt" => "XReg(31)"}, :arg => [3]},
      {"STEOR"     => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 2, "rt" => "XReg(31)"}, :arg => [3]},
      {"STSET"     => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 3, "rt" => "XReg(31)"}, :arg => [3]},
      {"STSMAX"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 4, "rt" => "XReg(31)"}, :arg => [3]},
      {"STSMIN"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 5, "rt" => "XReg(31)"}, :arg => [3]},
      {"STUMAX"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 6, "rt" => "XReg(31)"}, :arg => [3]},
      {"STUMIN"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 0, "o3" => 0, "opc" => 7, "rt" => "XReg(31)"}, :arg => [3]},
      {"STADDL"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 0, "rt" => "XReg(31)"}, :arg => [3]},
      {"STCLRL"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 1, "rt" => "XReg(31)"}, :arg => [3]},
      {"STEORL"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 2, "rt" => "XReg(31)"}, :arg => [3]},
      {"STSETL"    => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 3, "rt" => "XReg(31)"}, :arg => [3]},
      {"STSMAXL"   => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 4, "rt" => "XReg(31)"}, :arg => [3]},
      {"STSMINL"   => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 5, "rt" => "XReg(31)"}, :arg => [3]},
      {"STUMAXL"   => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 6, "rt" => "XReg(31)"}, :arg => [3]},
      {"STUMINL"   => {"size" => 3, "V" => 0, "A" => 0, "R" => 1, "o3" => 0, "opc" => 7, "rt" => "XReg(31)"}, :arg => [3]},
      {"STADDA"    => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 0, "rt" => "XReg(31)"}, :arg => [3]},
      {"STCLRA"    => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 1, "rt" => "XReg(31)"}, :arg => [3]},
      {"STEORA"    => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 2, "rt" => "XReg(31)"}, :arg => [3]},
      {"STSETA"    => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 3, "rt" => "XReg(31)"}, :arg => [3]},
      {"STSMAXA"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 4, "rt" => "XReg(31)"}, :arg => [3]},
      {"STSMINA"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 5, "rt" => "XReg(31)"}, :arg => [3]},
      {"STUMAXA"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 6, "rt" => "XReg(31)"}, :arg => [3]},
      {"STUMINA"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 0, "o3" => 0, "opc" => 7, "rt" => "XReg(31)"}, :arg => [3]},
      {"STADDAL"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 0, "rt" => "XReg(31)"}, :arg => [3]},
      {"STCLRAL"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 1, "rt" => "XReg(31)"}, :arg => [3]},
      {"STEORAL"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 2, "rt" => "XReg(31)"}, :arg => [3]},
      {"STSETAL"   => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 3, "rt" => "XReg(31)"}, :arg => [3]},
      {"STSMAXAL"  => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 4, "rt" => "XReg(31)"}, :arg => [3]},
      {"STSMINAL"  => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 5, "rt" => "XReg(31)"}, :arg => [3]},
      {"STUMAXAL"  => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 6, "rt" => "XReg(31)"}, :arg => [3]},
      {"STUMINAL"  => {"size" => 3, "V" => 0, "A" => 1, "R" => 1, "o3" => 0, "opc" => 7, "rt" => "XReg(31)"}, :arg => [3]}
    ]
  },

  "LdStReg" => {
    :cmt => "load/store register (register offset)",
    :arg => [
      [ {"rt" => "WReg"}, {"adr" => "AdrReg"}], #0
      [ {"rt" => "XReg"}, {"adr" => "AdrReg"}], #1
      [ {"rt" => "WReg"}, {"adr" => "AdrExt"}], #2
      [ {"rt" => "XReg"}, {"adr" => "AdrExt"}]  #3
    ],
    :prm => ["size", "opc", "rt", "adr"],
    :grp => [
      {"STRB"  => {"size" => 0, "opc" => 0}, :arg => [0,2]},
      {"LDRB"  => {"size" => 0, "opc" => 1}, :arg => [0,2]},
      {"LDRSB" => {"size" => 0, "opc" => 2}, :arg => [1,3]},
      {"LDRSB" => {"size" => 0, "opc" => 3}, :arg => [0,2]},
      {"STRH"  => {"size" => 1, "opc" => 0}, :arg => [0,2]},
      {"LDRH"  => {"size" => 1, "opc" => 1}, :arg => [0,2]},
      {"LDRSH" => {"size" => 1, "opc" => 2}, :arg => [1,3]},
      {"LDRSH" => {"size" => 1, "opc" => 3}, :arg => [0,2]},
      {"STR"   => {"size" => 2, "opc" => 0}, :arg => [0,2]},
      {"LDR"   => {"size" => 2, "opc" => 1}, :arg => [0,2]},
      {"LDRSW" => {"size" => 2, "opc" => 2}, :arg => [1,3]},
      {"STR"   => {"size" => 3, "opc" => 0}, :arg => [1,3]},
      {"LDR"   => {"size" => 3, "opc" => 1}, :arg => [1,3]},
    ]
  },

  "LdStSimdFpReg" => {
    :cmt => "load/store register (register offset)",
    :arg => ext_args(screg_set(1,"BHSDQ"), [{"adr" => "AdrReg"}], {"vd" => "vt"}) +
            ext_args(screg_set(1,"BHSDQ"), [{"adr" => "AdrExt"}], {"vd" => "vt"}),
    # :arg => [[ {"vt" => "VReg"}, {"adr" => "AdrReg"}]],
    :prm => ["opc", "vt", "adr"],
    :grp => [
      {"STR"  => {"opc" => "(vt.getBit() != 128)? 0 : 2"}},
      {"LDR"  => {"opc" => "(vt.getBit() != 128)? 1 : 3"}}
    ]
  },

  "PfExt" => {
    :cmt => "load/store register (register offset)",
    :arg => [
      [ {"prfop" => "Prfop"}, {"adr" => "AdrReg"}], #0
      [ {"prfop" => "Prfop"}, {"adr" => "AdrExt"}]  #1
    ],
    :prm => ["prfop", "adr"],
    :grp => [
      {"PRFM"  => {}}
    ]
  },

  "LdStRegPac" => {
    :cmt => "loat/store register (pac)",
    :arg => [
      [ {"xt" => "XReg"}, {"adr" => "AdrImm"}], #0
      [ {"xt" => "XReg"}, {"adr" => "AdrPreImm"}]   #1
    ],
    :prm => ["M", "W", "xt", "adr"],
    :grp => [
      {"LDRAA"  => {"M" => 0, "W" => 0}, :arg =>[0]},
      {"LDRAB"  => {"M" => 1, "W" => 0}, :arg =>[0]},
      {"LDRAA"  => {"M" => 0, "W" => 1}, :arg =>[1]},
      {"LDRAB"  => {"M" => 1, "W" => 1}, :arg =>[1]}
    ]
  },

  "LdStRegUnImm" => {
    :cmt => "loat/store register (unsigned immediate)",
    :arg => [
      [ {"rt" => "WReg"}, {"adr" => "AdrUimm"}], #0
      [ {"rt" => "XReg"}, {"adr" => "AdrUimm"}]  #1
    ],
    :prm => ["size", "opc", "rt", "adr"],
    :grp => [
      {"STRB"  => {"size" => 0, "opc" => 0}, :arg => [0]},
      {"LDRB"  => {"size" => 0, "opc" => 1}, :arg => [0]},
      {"LDRSB" => {"size" => 0, "opc" => 2}, :arg => [1]},
      {"LDRSB" => {"size" => 0, "opc" => 3}, :arg => [0]},
      {"STRH"  => {"size" => 1, "opc" => 0}, :arg => [0]},
      {"LDRH"  => {"size" => 1, "opc" => 1}, :arg => [0]},
      {"LDRSH" => {"size" => 1, "opc" => 2}, :arg => [1]},
      {"LDRSH" => {"size" => 1, "opc" => 3}, :arg => [0]},
      {"STR"   => {"size" => 2, "opc" => 0}, :arg => [0]},
      {"LDR"   => {"size" => 2, "opc" => 1}, :arg => [0]},
      {"LDRSW" => {"size" => 2, "opc" => 2}, :arg => [1]},
      {"STR"   => {"size" => 3, "opc" => 0}, :arg => [1]},
      {"LDR"   => {"size" => 3, "opc" => 1}, :arg => [1]}
    ]
  },

  "LdStSimdFpUnImm" => {
    :cmt => "loat/store register (unsigned immediate)",
    :arg => ext_args(screg_set(1,"BHSDQ"), [{"adr" => "AdrUimm"}], {"vd" => "vt"}),
    :prm => ["opc", "vt", "adr"],
    :grp => [
      {"STR"   => {"opc" => "(vt.getBit() != 128)? 0 : 2"}},
      {"LDR"   => {"opc" => "(vt.getBit() != 128)? 1 : 3"}}
    ]
  },

  "PfRegImm" => {
    :cmt => "loat/store register (unsigned immediate)",
    :arg => [[ {"prfop" => "Prfop"}, {"adr" => "AdrUimm"}]],
    :prm => ["prfop", "adr"],
    :grp => [
      {"PRFM"   => {}}
    ]
  },
  
  ################### Data Processing -- register  ################
  "DataProc2Src" => {
    :cmt => "Data processing (2 source)",
    :arg => [
      [ {"rd" => "WReg"}, {"rn" => "WReg"}, {"rm" => "WReg"}], #0
      [ {"rd" => "XReg"}, {"rn" => "XReg"}, {"rm" => "XReg"}], #1
      [ {"rd" => "WReg"}, {"rn" => "WReg"}, {"rm" => "XReg"}]  #2
    ],
    :prm => ["opcode", "rd", "rn", "rm"],
    :grp => [
      {"UDIV"    => {"opcode" => 0x2 }, :arg => [0]},
      {"SDIV"    => {"opcode" => 0x3 }, :arg => [0]},
      {"LSLV"    => {"opcode" => 0x8 }, :arg => [0]},
      {"LSL"     => {"opcode" => 0x8 }, :arg => [0]}, # alias of LSLV
      {"LSRV"    => {"opcode" => 0x9 }, :arg => [0]},
      {"LSR"     => {"opcode" => 0x9 }, :arg => [0]}, # alias of LSRV
      {"ASRV"    => {"opcode" => 0xa }, :arg => [0]},
      {"ASR"     => {"opcode" => 0xa }, :arg => [0]}, # alias of ASRV
      {"RORV"    => {"opcode" => 0xb }, :arg => [0]},
      {"ROR"     => {"opcode" => 0xb }, :arg => [0]}, # alias of RORV
      {"CRC32B"  => {"opcode" => 0x10 }, :arg => [0]},
      {"CRC32H"  => {"opcode" => 0x11 }, :arg => [0]},
      {"CRC32W"  => {"opcode" => 0x12 }, :arg => [0]},
      {"CRC32CB" => {"opcode" => 0x14 }, :arg => [0]},
      {"CRC32CH" => {"opcode" => 0x15 }, :arg => [0]},
      {"CRC32CW" => {"opcode" => 0x16 }, :arg => [0]},
      {"UDIV"    => {"opcode" => 0x2 }, :arg => [1]},
      {"SDIV"    => {"opcode" => 0x3 }, :arg => [1]},
      {"LSLV"    => {"opcode" => 0x8 }, :arg => [1]},
      {"LSL"     => {"opcode" => 0x8 }, :arg => [1]}, # alias of LSLV
      {"LSRV"    => {"opcode" => 0x9 }, :arg => [1]},
      {"LSR"     => {"opcode" => 0x9 }, :arg => [1]}, # alias of LSRV
      {"ASRV"    => {"opcode" => 0xa }, :arg => [1]},
      {"ASR"     => {"opcode" => 0xa }, :arg => [1]}, # alias of ASRV
      {"RORV"    => {"opcode" => 0xb }, :arg => [1]},
      {"ROR"     => {"opcode" => 0xb }, :arg => [1]}, # alias of RORV
      {"PACGA"   => {"opcode" => 0xc }, :arg => [1]},
      {"CRC32X"  => {"opcode" => 0x13 }, :arg => [2]},
      {"CRC32CX" => {"opcode" => 0x17 }, :arg => [2]}
    ]
  },
  
  "DataProc1Src" => {
    :cmt => "Data processing (1 source)",
    :arg => [
      [ {"rd" => "WReg"}, {"rn" => "WReg"}], #0
      [ {"rd" => "XReg"}, {"rn" => "XReg"}], #1
      [ {"rd" => "XReg"}]                    #2
    ],
    :prm => [
      ["opcode2", "opcode", "rd", "rn"], #0
      ["opcode2", "opcode", "rd"],       #1
    ],
    :grp => [
      {"RBIT"    => {"opcode2" => 0x0, "opcode" => 0x0}, :arg => [0], :prm => 0},
      {"REV16"   => {"opcode2" => 0x0, "opcode" => 0x1}, :arg => [0], :prm => 0},
      {"REV"     => {"opcode2" => 0x0, "opcode" => 0x2}, :arg => [0], :prm => 0},
      {"CLZ"     => {"opcode2" => 0x0, "opcode" => 0x4}, :arg => [0], :prm => 0},
      {"CLS"     => {"opcode2" => 0x0, "opcode" => 0x5}, :arg => [0], :prm => 0},

      {"RBIT"    => {"opcode2" => 0x0, "opcode" => 0x0}, :arg => [1], :prm => 0},
      {"REV16"   => {"opcode2" => 0x0, "opcode" => 0x1}, :arg => [1], :prm => 0},
      {"REV32"   => {"opcode2" => 0x0, "opcode" => 0x2}, :arg => [1], :prm => 0},
      {"REV"     => {"opcode2" => 0x0, "opcode" => 0x3}, :arg => [1], :prm => 0},
      {"REV64"   => {"opcode2" => 0x0, "opcode" => 0x3}, :arg => [1], :prm => 0}, # alias of REV
      {"CLZ"     => {"opcode2" => 0x0, "opcode" => 0x4}, :arg => [1], :prm => 0},
      {"CLS"     => {"opcode2" => 0x0, "opcode" => 0x5}, :arg => [1], :prm => 0},
      {"PACIA"   => {"opcode2" => 0x1, "opcode" => 0x0}, :arg => [1], :prm => 0},
      {"PACIB"   => {"opcode2" => 0x1, "opcode" => 0x1}, :arg => [1], :prm => 0},
      {"PACDA"   => {"opcode2" => 0x1, "opcode" => 0x2}, :arg => [1], :prm => 0},
      {"PACDB"   => {"opcode2" => 0x1, "opcode" => 0x3}, :arg => [1], :prm => 0},
      {"AUTIA"   => {"opcode2" => 0x1, "opcode" => 0x4}, :arg => [1], :prm => 0},
      {"AUTIB"   => {"opcode2" => 0x1, "opcode" => 0x5}, :arg => [1], :prm => 0},
      {"AUTDA"   => {"opcode2" => 0x1, "opcode" => 0x6}, :arg => [1], :prm => 0},
      {"AUTDB"   => {"opcode2" => 0x1, "opcode" => 0x7}, :arg => [1], :prm => 0},

      {"PACIZA"  => {"opcode2" => 0x1, "opcode" => 0x8}, :arg => [2], :prm => 1},
      {"PACIZB"  => {"opcode2" => 0x1, "opcode" => 0x9}, :arg => [2], :prm => 1},
      {"PACDZA"  => {"opcode2" => 0x1, "opcode" => 0xa}, :arg => [2], :prm => 1},
      {"PACDZB"  => {"opcode2" => 0x1, "opcode" => 0xb}, :arg => [2], :prm => 1},
      {"AUTIZA"  => {"opcode2" => 0x1, "opcode" => 0xc}, :arg => [2], :prm => 1},
      {"AUTIZB"  => {"opcode2" => 0x1, "opcode" => 0xd}, :arg => [2], :prm => 1},
      {"AUTDZA"  => {"opcode2" => 0x1, "opcode" => 0xe}, :arg => [2], :prm => 1},
      {"AUTDZB"  => {"opcode2" => 0x1, "opcode" => 0xf}, :arg => [2], :prm => 1},
      {"XPACI"   => {"opcode2" => 0x1, "opcode" => 0x10},:arg => [2], :prm => 1},
      {"XPACD"   => {"opcode2" => 0x1, "opcode" => 0x11},:arg => [2], :prm => 1}
    ]
  },

  "LogicalShiftReg" => {
    :cmt => "Logical (shifted register)",
    :arg => [
      [ {"rd" => "WReg"}, {"rn" => "WReg"}, {"rm" => "WReg"}, {"shmod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}], #0
      [ {"rd" => "XReg"}, {"rn" => "XReg"}, {"rm" => "XReg"}, {"shmod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}], #1
      [ {"rn" => "WReg"}, {"rm" => "WReg"}, {"shmod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}],                   #2
      [ {"rn" => "XReg"}, {"rm" => "XReg"}, {"shmod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}],                   #3
      [ {"rd" => "WReg"}, {"rm" => "WReg"}, {"shmod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}],                   #4
      [ {"rd" => "XReg"}, {"rm" => "XReg"}, {"shmod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}]                    #5
    ],
    :prm => ["opc", "N", "rd", "rn", "rm", "shmod", "sh"],
    :grp => [
      {"AND"  => {"opc" => 0, "N" => 0},                     :arg => 0..1},
      {"BIC"  => {"opc" => 0, "N" => 1},                     :arg => 0..1},
      {"ORR"  => {"opc" => 1, "N" => 0},                     :arg => 0..1},
      # {"MOV"  => {"opc" => 1, "N" => 0, "rn" => "WReg(31)"}, :arg => [4]},  # alias of ORR
      # {"MOV"  => {"opc" => 1, "N" => 0, "rn" => "XReg(31)"}, :arg => [5]},  # alias of ORR
      {"ORN"  => {"opc" => 1, "N" => 1},                     :arg => 0..1},
      {"MVN"  => {"opc" => 1, "N" => 1, "rn" => "WReg(31)"}, :arg => [4]},  # alias of ORN
      {"MVN"  => {"opc" => 1, "N" => 1, "rn" => "XReg(31)"}, :arg => [5]},  # alias of ORN
      {"EOR"  => {"opc" => 2, "N" => 0},                     :arg => 0..1},
      {"EON"  => {"opc" => 2, "N" => 1},                     :arg => 0..1},
      {"ANDS" => {"opc" => 3, "N" => 0},                     :arg => 0..1},
      {"TST"  => {"opc" => 3, "N" => 0, "rd" => "WReg(31)"}, :arg => [2]},  # alias of ANDS
      {"TST"  => {"opc" => 3, "N" => 0, "rd" => "XReg(31)"}, :arg => [3]},  # alias of ANDS
      {"BICS" => {"opc" => 3, "N" => 1},                     :arg => 0..1}
    ]
  },

  "MvReg" => {
    :cmt => "Move (register) alias of ADD,ORR",
    :arg => [
      [ {"rd" => "WReg"}, {"rn" => "WReg"}], #0
      [ {"rd" => "XReg"}, {"rn" => "XReg"}]  #1
    ],
    :prm => ["rd", "rn"],
    :grp => [
      "MOV" => {}
    ]
  },
  
  "AddSubShiftReg" => {
    :cmt => "Add/subtract (shifted register)",
    :arg => [
      [ {"rd" => "WReg"}, {"rn" => "WReg"}, {"rm" => "WReg"}, {"shmod=NONE" => "ShMod"}, {"sh=0" => "uint32_t"}], #0
      [ {"rd" => "XReg"}, {"rn" => "XReg"}, {"rm" => "XReg"}, {"shmod=NONE" => "ShMod"}, {"sh=0" => "uint32_t"}], #1
      [ {"rn" => "WReg"}, {"rm" => "WReg"}, {"shmod=NONE" => "ShMod"}, {"sh=0" => "uint32_t"}],                   #2
      [ {"rn" => "XReg"}, {"rm" => "XReg"}, {"shmod=NONE" => "ShMod"}, {"sh=0" => "uint32_t"}],                   #3
      [ {"rd" => "WReg"}, {"rm" => "WReg"}, {"shmod=NONE" => "ShMod"}, {"sh=0" => "uint32_t"}],                   #4
      [ {"rd" => "XReg"}, {"rm" => "XReg"}, {"shmod=NONE" => "ShMod"}, {"sh=0" => "uint32_t"}]                    #5
    ],
    :prm => [
      ["opc", "S", "rd", "rn", "rm", "shmod", "sh"],         #0
      ["opc", "S", "rd", "rn", "rm", "shmod", "sh", "alias"],#1
    ],
    :grp => [
      {"ADD"  => {"opc" => 0, "S" => 0}, :arg => 0..1, :prm => 0},
      {"ADDS" => {"opc" => 0, "S" => 1}, :arg => 0..1, :prm => 0},
      {"CMN"  => {"opc" => 0, "S" => 1, "rd" => "WReg(31)", "alias"=>"true"}, :arg => [2], :prm => 1}, # alias of ADDS
      {"CMN"  => {"opc" => 0, "S" => 1, "rd" => "XReg(31)", "alias"=>"true"}, :arg => [3], :prm => 1}, # alias of ADDS
      {"SUB"  => {"opc" => 1, "S" => 0}, :arg => 0..1, :prm => 0},
      {"NEG"  => {"opc" => 1, "S" => 0, "rn" => "WReg(31)", "alias"=>"true"}, :arg => [4], :prm => 1}, # alias of SUB
      {"NEG"  => {"opc" => 1, "S" => 0, "rn" => "XReg(31)", "alias"=>"true"}, :arg => [5], :prm => 1}, # alias of SUB
      {"SUBS" => {"opc" => 1, "S" => 1}, :arg => 0..1, :prm => 0},
      {"NEGS" => {"opc" => 1, "S" => 1, "rn" => "WReg(31)", "alias"=>"true"}, :arg => [4], :prm => 1}, # alias of SUB
      {"NEGS" => {"opc" => 1, "S" => 1, "rn" => "XReg(31)", "alias"=>"true"}, :arg => [5], :prm => 1}, # alias of SUB
      {"CMP"  => {"opc" => 1, "S" => 1, "rd" => "WReg(31)", "alias"=>"true"}, :arg => [2], :prm => 1}, # alias of SUBS
      {"CMP"  => {"opc" => 1, "S" => 1, "rd" => "XReg(31)", "alias"=>"true"}, :arg => [3], :prm => 1}  # alias of SUBS
    ]
  },

  "AddSubExtReg" => {
    :cmt => "Add/subtract (extended register)",
    :arg => [
      [ {"rd" => "WReg"}, {"rn" => "WReg"}, {"rm" => "WReg"}, {"extmod" => "ExtMod"}, {"sh=0" => "uint32_t"}], #0
      [ {"rd" => "XReg"}, {"rn" => "XReg"}, {"rm" => "XReg"}, {"extmod" => "ExtMod"}, {"sh=0" => "uint32_t"}], #1
      [ {"rn" => "WReg"}, {"rm" => "WReg"}, {"extmod" => "ExtMod"}, {"sh=0" => "uint32_t"}],                   #2
      [ {"rn" => "XReg"}, {"rm" => "XReg"}, {"extmod" => "ExtMod"}, {"sh=0" => "uint32_t"}]                    #3
    ],
    :prm => ["opc", "S", "rd", "rn", "rm", "extmod", "sh"],
    :grp => [
      {"ADD"  => {"opc" => 0, "S" => 0},                     :arg => 0..1},
      {"ADDS" => {"opc" => 0, "S" => 1},                     :arg => 0..1},
      {"CMN"  => {"opc" => 0, "S" => 1, "rd" => "WReg(31)"}, :arg => [2]}, # alias of ADDS
      {"CMN"  => {"opc" => 0, "S" => 1, "rd" => "XReg(31)"}, :arg => [3]}, # alias of ADDS
      {"SUB"  => {"opc" => 1, "S" => 0},                     :arg => 0..1},
      {"SUBS" => {"opc" => 1, "S" => 1},                     :arg => 0..1},
      {"CMP"  => {"opc" => 1, "S" => 1, "rd" => "WReg(31)"}, :arg => [2]}, # alias of SUBP
      {"CMP"  => {"opc" => 1, "S" => 1, "rd" => "XReg(31)"}, :arg => [3]}  # alias of SUBP
    ]
  },

  "AddSubCarry" => {
    :cmt => "Add/subtract (with carry)",
    :arg => [
      [ {"rd" => "WReg"}, {"rn" => "WReg"}, {"rm" => "WReg"}], #0
      [ {"rd" => "XReg"}, {"rn" => "XReg"}, {"rm" => "XReg"}], #1
      [ {"rd" => "WReg"}, {"rm" => "WReg"}],                   #2
      [ {"rd" => "XReg"}, {"rm" => "XReg"}]                    #3
    ],
    :prm => ["op", "S", "rd", "rn", "rm"],
    :grp => [
      {"ADC"  => {"op" => 0, "S" => 0},                     :arg => 0..1},
      {"ADCS" => {"op" => 0, "S" => 1},                     :arg => 0..1},
      {"SBC"  => {"op" => 1, "S" => 0},                     :arg => 0..1},
      {"NGC"  => {"op" => 1, "S" => 0, "rn" => "WReg(31)"}, :arg => [2]}, # alias of SBC
      {"NGC"  => {"op" => 1, "S" => 0, "rn" => "XReg(31)"}, :arg => [3]}, # alias of SBC
      {"SBCS" => {"op" => 1, "S" => 1},                     :arg => 0..1},
      {"NGCS" => {"op" => 1, "S" => 1, "rn" => "WReg(31)"}, :arg => [2]}, # alias of SBCS
      {"NGCS" => {"op" => 1, "S" => 1, "rn" => "XReg(31)"}, :arg => [3]}  # alias of SBCS
    ]
  },

  "RotateR" => {
    :cmt => "Rotate right into flags",
    :arg => [[ {"xn" => "XReg"}, {"sh" => "uint32_t"}, {"mask" => "uint32_t"}]],
    :prm => ["op", "S", "o2", "xn", "sh", "mask"],
    :grp => [
      {"RMIF"  => {"op" => 0, "S" => 1, "o2" => 0}}
    ]
  },

  "Evaluate" => {
    :cmt => "Evaluate into flags",
    :arg => [[ {"wn" => "WReg"}]],
    :prm => ["op", "S", "opcode2", "sz", "o3", "mask", "wn"],
    :grp => [
      {"SETF8"  => {"op" => 0, "S" => 1, "opcode2" => 0, "sz" => 0, "o3" => 0, "mask" => 0xd}},
      {"SETF16" => {"op" => 0, "S" => 1, "opcode2" => 0, "sz" => 1, "o3" => 0, "mask" => 0xd}}
    ]
  },

  "CondCompReg" => {
    :cmt => "Conditional compare (register)",
    :arg => [
      [ {"rn" => "WReg"}, {"rm" => "WReg"}, {"nczv" => "uint32_t"}, {"cond" => "Cond"}], #0
      [ {"rn" => "XReg"}, {"rm" => "XReg"}, {"nczv" => "uint32_t"}, {"cond" => "Cond"}]  #1
    ],
    :prm => ["op", "S", "o2", "o3", "rn", "rm", "nczv", "cond"],
    :grp => [
      {"CCMN"  => {"op" => 0, "S" => 1, "o2" => 0, "o3" => 0}},
      {"CCMP"  => {"op" => 1, "S" => 1, "o2" => 0, "o3" => 0}}
    ]
  },

  "CondCompImm" => {
    :cmt => "Conditional compare (imm)",
    :arg => [
      [ {"rn" => "WReg"}, {"imm" => "uint32_t"}, {"nczv" => "uint32_t"}, {"cond" => "Cond"}], #0
      [ {"rn" => "XReg"}, {"imm" => "uint32_t"}, {"nczv" => "uint32_t"}, {"cond" => "Cond"}]  #1
    ],
    :prm => ["op", "S", "o2", "o3", "rn", "imm", "nczv", "cond"],
    :grp => [
      {"CCMN"  => {"op" => 0, "S" => 1, "o2" => 0, "o3" => 0}},
      {"CCMP"  => {"op" => 1, "S" => 1, "o2" => 0, "o3" => 0}}
    ]
  },

  "CondSel" => {
    :cmt => "Conditional select",
    :arg => [
      [ {"rd" => "WReg"}, {"rn" => "WReg"}, {"rm" => "WReg"}, {"cond" => "Cond"}], #0
      [ {"rd" => "XReg"}, {"rn" => "XReg"}, {"rm" => "XReg"}, {"cond" => "Cond"}], #1
      [ {"rd" => "WReg"}, {"rn" => "WReg"}, {"cond" => "Cond"}],                   #2
      [ {"rd" => "XReg"}, {"rn" => "XReg"}, {"cond" => "Cond"}],                   #3
      [ {"rd" => "WReg"}, {"cond" => "Cond"}],                                     #4
      [ {"rd" => "XReg"}, {"cond" => "Cond"}]                                      #5
    ],
    :prm => ["op", "S", "o2", "rd", "rn", "rm", "cond"],
    :grp => [
      {"CSEL"  => {"op" => 0, "S" => 0, "o2" => 0}, :arg => 0..1},
      {"CSINC" => {"op" => 0, "S" => 0, "o2" => 1}, :arg => 0..1},
      {"CINC"  => {"op" => 0, "S" => 0, "o2" => 1, "rm" => "rn", "cond" => "invert(cond)"}, :arg => 2..3}, # alias of CSINC
      {"CSET"  => {"op" => 0, "S" => 0, "o2" => 1, "rn" => "WReg(31)", "rm" => "WReg(31)", "cond" => "invert(cond)"}, :arg => [4]},  # alias of CSINC
      {"CSET"  => {"op" => 0, "S" => 0, "o2" => 1, "rn" => "XReg(31)", "rm" => "XReg(31)", "cond" => "invert(cond)"}, :arg => [5]},  # alias of CSINC
      {"CSINV" => {"op" => 1, "S" => 0, "o2" => 0}, :arg => 0..1},
      {"CINV"  => {"op" => 1, "S" => 0, "o2" => 0, "rm" => "rn", "cond" => "invert(cond)"}, :arg => 2..3}, # alias of CSINV
      {"CSETM" => {"op" => 1, "S" => 0, "o2" => 0, "rn" => "WReg(31)", "rm" => "WReg(31)", "cond" => "invert(cond)"}, :arg => [4]},  # alias of CSINV
      {"CSETM" => {"op" => 1, "S" => 0, "o2" => 0, "rn" => "XReg(31)", "rm" => "XReg(31)", "cond" => "invert(cond)"}, :arg => [5]},  # alias of CSINV
      {"CSNEG" => {"op" => 1, "S" => 0, "o2" => 1}, :arg => 0..1},
      {"CNEG"  => {"op" => 1, "S" => 0, "o2" => 1, "rm" => "rn", "cond" => "invert(cond)"}, :arg => 2..3}  # alias of CSNEG
    ]
  },

  "DataProc3Reg" => {
    :cmt => "Conditional select",
    :arg => [
      [ {"rd" => "WReg"}, {"rn" => "WReg"}, {"rm" => "WReg"}, {"ra" => "WReg"}], #0
      [ {"rd" => "XReg"}, {"rn" => "XReg"}, {"rm" => "XReg"}, {"ra" => "XReg"}], #1
      [ {"rd" => "XReg"}, {"rn" => "WReg"}, {"rm" => "WReg"}, {"ra" => "XReg"}], #2
      [ {"rd" => "WReg"}, {"rn" => "WReg"}, {"rm" => "WReg"}],                   #3
      [ {"rd" => "XReg"}, {"rn" => "XReg"}, {"rm" => "XReg"}],                   #4
      [ {"rd" => "XReg"}, {"rn" => "WReg"}, {"rm" => "WReg"}]                    #5
    ],
    :prm => [
      ["op54", "op31", "o0", "rd", "rn", "rm", "ra"], #0
      ["op54", "op31", "o0", "rd", "rn", "rm"]        #1
    ],
    :grp => [
      {"MADD"   => {"op54" => 0, "op31" => 0, "o0" => 0}, :arg => [0,1], :prm => 0},
      {"MUL"    => {"op54" => 0, "op31" => 0, "o0" => 0, "ra" => "WReg(31)"}, :arg => [3], :prm => 0}, # alias of MADD
      {"MUL"    => {"op54" => 0, "op31" => 0, "o0" => 0, "ra" => "XReg(31)"}, :arg => [4], :prm => 0}, # alias of MADD
      {"MSUB"   => {"op54" => 0, "op31" => 0, "o0" => 1}, :arg => [0,1], :prm => 0},
      {"MNEG"   => {"op54" => 0, "op31" => 0, "o0" => 1, "ra" => "WReg(31)"}, :arg => [3], :prm => 0}, # alias of MSUB
      {"MNEG"   => {"op54" => 0, "op31" => 0, "o0" => 1, "ra" => "XReg(31)"}, :arg => [4], :prm => 0}, # alias of MSUB
      {"SMADDL" => {"op54" => 0, "op31" => 1, "o0" => 0}, :arg => [2], :prm => 0},
      {"SMULL"  => {"op54" => 0, "op31" => 1, "o0" => 0, "ra" => "XReg(31)"}, :arg => [5], :prm => 0}, # alias of SMADDL
      {"SMSUBL" => {"op54" => 0, "op31" => 1, "o0" => 1}, :arg => [2], :prm => 0},
      {"SMNEGL" => {"op54" => 0, "op31" => 1, "o0" => 1, "ra" => "XReg(31)"}, :arg => [5], :prm => 0}, # alias of SMSUBL
      {"SMULH"  => {"op54" => 0, "op31" => 2, "o0" => 0}, :arg => [4], :prm => 1},
      {"UMADDL" => {"op54" => 0, "op31" => 5, "o0" => 0}, :arg => [2], :prm => 0},
      {"UMULL"  => {"op54" => 0, "op31" => 5, "o0" => 0, "ra" => "XReg(31)"}, :arg => [5], :prm => 0}, # alias of SMADDL
      {"UMSUBL" => {"op54" => 0, "op31" => 5, "o0" => 1}, :arg => [2], :prm => 0},
      {"UMNEGL" => {"op54" => 0, "op31" => 5, "o0" => 1, "ra" => "XReg(31)"}, :arg => [5], :prm => 0}, # alias of UMSUBL
      {"UMULH"  => {"op54" => 0, "op31" => 6, "o0" => 0}, :arg => [4], :prm => 1}
    ]
  },

  ################### Data Processing -- Sclar Floating-Point and Advanced SIMD ################
  "CryptAES" => {
    :cmt => "Cryptographic AES",
    :arg => [[ {"vd" => "VReg16B"}, {"vn" => "VReg16B"}]],
    :prm => ["opcode", "vd", "vn"],
    :grp => [
      {"AESE"   => {"opcode" => 4}},
      {"AESD"   => {"opcode" => 5}},
      {"AESMC"  => {"opcode" => 6}},
      {"AESIMC" => {"opcode" => 7}},
    ]
  },

  "Crypt3RegSHA" => {
    :cmt => "Cryptographic three-register SHA",
    :arg => [
      [ {"qd" => "QReg"}, {"sn" => "SReg"}, {"vm" => "VReg4S"}],     #0
      [ {"qd" => "QReg"}, {"qn" => "QReg"}, {"vm" => "VReg4S"}],     #1
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}, {"vm" => "VReg4S"}]  #2
    ],
    :prm => [
      ["opcode", "qd", "sn", "vm"], #0
      ["opcode", "qd", "qn", "vm"], #1
      ["opcode", "vd", "vn", "vm"]  #2
    ],
    :grp => [
      {"SHA1C"     => {"opcode" => 0}, :arg => [0], :prm => 0},
      {"SHA1P"     => {"opcode" => 1}, :arg => [0], :prm => 0},
      {"SHA1M"     => {"opcode" => 2}, :arg => [0], :prm => 0},
      {"SHA1SU0"   => {"opcode" => 3}, :arg => [2], :prm => 2},
      {"SHA256H"   => {"opcode" => 4}, :arg => [1], :prm => 1},
      {"SHA256H2"  => {"opcode" => 5}, :arg => [1], :prm => 1},
      {"SHA256SU1" => {"opcode" => 6}, :arg => [2], :prm => 2}
    ]
  },

  "Crypt2RegSHA" => {
    :cmt => "Cryptographic two-register SHA",
    :arg => [
      [ {"sd" => "SReg"}, {"sn" => "SReg"}],     #0
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}]  #1
    ],
    :prm => [
      ["opcode", "sd", "sn"],  #0
      ["opcode", "vd", "vn"]   #1
    ],
    :grp => [
      {"SHA1H"     => {"opcode" => 0}, :arg => [0], :prm => 0},
      {"SHA1SU1"   => {"opcode" => 1}, :arg => [1], :prm => 1},
      {"SHA256SU0" => {"opcode" => 2}, :arg => [1], :prm => 1}
    ]
  },

  "AdvSimdScCopy" => {
    :cmt => "Advanced SIMD Scalar copy",
    :arg => [
      [ {"vd" => "BReg"}, {"vn" => "VRegBElem"}], #0
      [ {"vd" => "HReg"}, {"vn" => "VRegHElem"}], #1
      [ {"vd" => "SReg"}, {"vn" => "VRegSElem"}], #2
      [ {"vd" => "DReg"}, {"vn" => "VRegDElem"}]  #3
    ],
    :prm => ["op", "imm4", "vd", "vn"],
    :grp => [
      {"DUP" => {"op" => 0, "imm4" => 0}},
      {"MOV" => {"op" => 0, "imm4" => 0}}, # alias of DUP
    ]
  },

  "AdvSimdSc3SameFp16" => {
    :cmt => "Advanced SIMD Scalar three same FP16",
    :arg => [[ {"hd" => "HReg"}, {"hn" => "HReg"}, {"hm" => "HReg"}]],
    :prm => ["U", "a", "opcode", "hd", "hn", "hm"],
    :grp => [
      {"FMULX"   => {"U" => 0, "a" => 0, "opcode" => 3}},
      {"FCMEQ"   => {"U" => 0, "a" => 0, "opcode" => 4}},
      {"FRECPS"  => {"U" => 0, "a" => 0, "opcode" => 7}},
      {"FRSQRTS" => {"U" => 0, "a" => 1, "opcode" => 7}},
      {"FCMGE"   => {"U" => 1, "a" => 0, "opcode" => 4}},
      {"FACGE"   => {"U" => 1, "a" => 0, "opcode" => 5}},
      {"FABD"    => {"U" => 1, "a" => 1, "opcode" => 2}},
      {"FCMGT"   => {"U" => 1, "a" => 1, "opcode" => 4}},
      {"FACGT"   => {"U" => 1, "a" => 1, "opcode" => 5}}
    ]
  },

  "AdvSimdSc2RegMiscFp16" => {
    :cmt => "Advanced SIMD Scalar two-register miscellaneous FP16",
    :arg => [
      [ {"hd" => "HReg"}, {"hn" => "HReg"}], #0
      [ {"hd" => "HReg"}, {"hn" => "HReg"}, {"zero" => "double"}]  #1
    ],
    :prm => [
      ["U", "a", "opcode", "hd", "hn"],         #0
      ["U", "a", "opcode", "hd", "hn", "zero"], #1
    ],
    :grp => [
      {"FCVTNS"  => {"U" => 0, "a" => 0, "opcode" => 0x1a}, :arg => [0], :prm => 0},
      {"FCVTMS"  => {"U" => 0, "a" => 0, "opcode" => 0x1b}, :arg => [0], :prm => 0},
      {"FCVTAS"  => {"U" => 0, "a" => 0, "opcode" => 0x1c}, :arg => [0], :prm => 0},
      {"SCVTF"   => {"U" => 0, "a" => 0, "opcode" => 0x1d}, :arg => [0], :prm => 0},
      {"FCMGT"   => {"U" => 0, "a" => 1, "opcode" => 0xc},  :arg => [1], :prm => 1},
      {"FCMEQ"   => {"U" => 0, "a" => 1, "opcode" => 0xd},  :arg => [1], :prm => 1},
      {"FCMLT"   => {"U" => 0, "a" => 1, "opcode" => 0xe},  :arg => [1], :prm => 1},
      {"FCVTPS"  => {"U" => 0, "a" => 1, "opcode" => 0x1a}, :arg => [0], :prm => 0},
      {"FCVTZS"  => {"U" => 0, "a" => 1, "opcode" => 0x1b}, :arg => [0], :prm => 0},
      {"FRECPE"  => {"U" => 0, "a" => 1, "opcode" => 0x1d}, :arg => [0], :prm => 0},
      {"FRECPX"  => {"U" => 0, "a" => 1, "opcode" => 0x1f}, :arg => [0], :prm => 0},
      {"FCVTNU"  => {"U" => 1, "a" => 0, "opcode" => 0x1a}, :arg => [0], :prm => 0},
      {"FCVTMU"  => {"U" => 1, "a" => 0, "opcode" => 0x1b}, :arg => [0], :prm => 0},
      {"FCVTAU"  => {"U" => 1, "a" => 0, "opcode" => 0x1c}, :arg => [0], :prm => 0},
      {"UCVTF"   => {"U" => 1, "a" => 0, "opcode" => 0x1d}, :arg => [0], :prm => 0},
      {"FCMGE"   => {"U" => 1, "a" => 1, "opcode" => 0xc},  :arg => [1], :prm => 1},
      {"FCMLE"   => {"U" => 1, "a" => 1, "opcode" => 0xd},  :arg => [1], :prm => 1},
      {"FCVTPU"  => {"U" => 1, "a" => 1, "opcode" => 0x1a}, :arg => [0], :prm => 0},
      {"FCVTZU"  => {"U" => 1, "a" => 1, "opcode" => 0x1b}, :arg => [0], :prm => 0},
      {"FRSQRTE" => {"U" => 1, "a" => 1, "opcode" => 0x1d}, :arg => [0], :prm => 0}
    ]
  },

  "AdvSimdSc3SameExtra" => {
    :cmt => "Advanced SIMD Scalar three same extra",
    :arg => [
      [ {"vd" => "HReg"}, {"vn" => "HReg"}, {"vm" => "HReg"}], #0
      [ {"vd" => "SReg"}, {"vn" => "SReg"}, {"vm" => "SReg"}]  #1
    ],
    :prm => ["U", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"SQRDMLAH"  => {"U" => 1, "opcode" => 0x0}},
      {"SQRDMLSH"  => {"U" => 1, "opcode" => 0x1}}
    ]
  },

  "AdvSimdSc2RegMisc" => {
    :cmt => "Advanced SIMD Scalar two-register miscellaneous",
    :arg => screg_set(2, "BHSD") +                  # 0-3
            [
              [{"vd" => "BReg"}, {"vn" => "HReg"}], # 4
              [{"vd" => "HReg"}, {"vn" => "SReg"}], # 5
              [{"vd" => "SReg"}, {"vn" => "DReg"}], # 6
              [{"vd" => "DReg"}, {"vn" => "DReg"}, {"zero" => "uint32_t"}]  # 7
            ],
    # :arg => [[ {"vd" => "ScReg"}, {"vn" => "ScReg"}]],
    :prm => [
      ["U", "opcode", "vd", "vn"], #0
      ["U", "opcode", "vd", "vn", "zero"]  #1
    ],
    :grp => [
      {"SUQADD" => {"U" => 0, "opcode" => 0x3},  :arg => [0,1,2,3], :prm => 0}, # BHSD
      {"SQABS"  => {"U" => 0, "opcode" => 0x7},  :arg => [0,1,2,3], :prm => 0}, # BHSD
      {"CMGT"   => {"U" => 0, "opcode" => 0x8},  :arg => [7], :prm => 1},       # D only
      {"CMEQ"   => {"U" => 0, "opcode" => 0x9},  :arg => [7], :prm => 1},       # D only
      {"CMLT"   => {"U" => 0, "opcode" => 0xa},  :arg => [7], :prm => 1},       # D only
      {"ABS"    => {"U" => 0, "opcode" => 0xb},  :arg => [3], :prm => 0},       # D only
      {"SQXTN"  => {"U" => 0, "opcode" => 0x14}, :arg => [4,5,6], :prm => 0},
      {"USQADD" => {"U" => 1, "opcode" => 0x3},  :arg => [0,1,2,3], :prm => 0}, # BHSD
      {"SQNEG"  => {"U" => 1, "opcode" => 0x7},  :arg => [0,1,2,3], :prm => 0}, # BHSD
      {"CMGE"   => {"U" => 1, "opcode" => 0x8},  :arg => [7], :prm => 1},       # D only
      {"CMLE"   => {"U" => 1, "opcode" => 0x9},  :arg => [7], :prm => 1},       # D only
      {"NEG"    => {"U" => 1, "opcode" => 0xb},  :arg => [3], :prm => 0},       # D only
      {"SQXTUN" => {"U" => 1, "opcode" => 0x12}, :arg => [4,5,6], :prm => 0},
      {"UQXTN"  => {"U" => 1, "opcode" => 0x14}, :arg => [4,5,6], :prm => 0},
    ]
  },

  "AdvSimdSc2RegMiscSz0x" => {
    :cmt => "Advanced SIMD Scalar two-register miscellaneous",
    :arg => [
      [ {"vd" => "SReg"}, {"vn" => "SReg"}], #0
      [ {"vd" => "DReg"}, {"vn" => "DReg"}], #1
      [ {"vd" => "SReg"}, {"vn" => "DReg"}]  #2
    ],
    :prm => ["U", "opcode", "vd", "vn"],
    :grp => [
      {"FCVTNS" => {"U" => 0, "opcode" => 0x1a}, :arg => [0,1]}, # S,D
      {"FCVTMS" => {"U" => 0, "opcode" => 0x1b}, :arg => [0,1]}, # S,D
      {"FCVTAS" => {"U" => 0, "opcode" => 0x1c}, :arg => [0,1]}, # S,D
      {"SCVTF"  => {"U" => 0, "opcode" => 0x1d}, :arg => [0,1]}, # S,D
      {"FCVTXN" => {"U" => 1, "opcode" => 0x16}, :arg => [2]},
      {"FCVTNU" => {"U" => 1, "opcode" => 0x1a}, :arg => [0,1]}, # S,D
      {"FCVTMU" => {"U" => 1, "opcode" => 0x1b}, :arg => [0,1]}, # S,D
      {"FCVTAU" => {"U" => 1, "opcode" => 0x1c}, :arg => [0,1]}, # S,D
      {"UCVTF"  => {"U" => 1, "opcode" => 0x1d}, :arg => [0,1]}, # S,D
    ]
  },

  "AdvSimdSc2RegMiscSz1x" => {
    :cmt => "Advanced SIMD Scalar two-register miscellaneous",
    :arg => [
      [ {"vd" => "SReg"}, {"vn" => "SReg"}], #0
      [ {"vd" => "DReg"}, {"vn" => "DReg"}], #1
      [ {"vd" => "SReg"}, {"vn" => "SReg"}, {"zero" => "double"}], #2
      [ {"vd" => "DReg"}, {"vn" => "DReg"}, {"zero" => "double"}]  #3
    ],
    :prm => [
      ["U", "opcode", "vd", "vn"],
      ["U", "opcode", "vd", "vn", "zero"]
    ],
    :grp => [
      {"FCMGT"  => {"U" => 0, "opcode" => 0xc},  :arg => 2..3, :prm => 1},
      {"FCMEQ"  => {"U" => 0, "opcode" => 0xd},  :arg => 2..3, :prm => 1},
      {"FCMLT"  => {"U" => 0, "opcode" => 0xe},  :arg => 2..3, :prm => 1},
      {"FCVTPS" => {"U" => 0, "opcode" => 0x1a}, :arg => 0..1, :prm => 0},
      {"FCVTZS" => {"U" => 0, "opcode" => 0x1b}, :arg => 0..1, :prm => 0},
      {"FRECPE" => {"U" => 0, "opcode" => 0x1d}, :arg => 0..1, :prm => 0},
      {"FRECPX" => {"U" => 0, "opcode" => 0x1f}, :arg => 0..1, :prm => 0},
      {"FCMGE"  => {"U" => 1, "opcode" => 0xc},  :arg => 2..3, :prm => 1},
      {"FCMLE"  => {"U" => 1, "opcode" => 0xd},  :arg => 2..3, :prm => 1},
      {"FCVTPU" => {"U" => 1, "opcode" => 0x1a}, :arg => 0..1, :prm => 0},
      {"FCVTZU" => {"U" => 1, "opcode" => 0x1b}, :arg => 0..1, :prm => 0},
      {"FRSQRTE"=> {"U" => 1, "opcode" => 0x1d}, :arg => 0..1, :prm => 0}
    ]
  },

  "AdvSimdScPairwise" => {
    :cmt => "Advanced SIMD scalar pairwize",
    :arg => [
      [ {"vd" => "DReg"}, {"vn" => "VReg2D"}], #0
      [ {"vd" => "HReg"}, {"vn" => "VReg2H"}], #1
      [ {"vd" => "SReg"}, {"vn" => "VReg2S"}], #2
      [ {"vd" => "DReg"}, {"vn" => "VReg2D"}]  #3
    ],
    :prm => ["U", "size", "opcode", "vd", "vn"],
    :grp => [
      {"ADDP"    => {"U" => 0, "size" => 3, "opcode" => 0x1b}, :arg => [0]},
      {"FMAXNMP" => {"U" => 0, "size" => 0, "opcode" => 0xc},  :arg => [1]},
      {"FADDP"   => {"U" => 0, "size" => 0, "opcode" => 0xd},  :arg => [1]},
      {"FMAXP"   => {"U" => 0, "size" => 0, "opcode" => 0xf},  :arg => [1]},
      {"FMINNMP" => {"U" => 0, "size" => 2, "opcode" => 0xc},  :arg => [1]},
      {"FMINP"   => {"U" => 0, "size" => 2, "opcode" => 0xf},  :arg => [1]},
      {"FMAXNMP" => {"U" => 1, "size" => "(vd.getBit()==32)? 0 : 1", "opcode" => 0xc},  :arg => [2,3]},
      {"FADDP"   => {"U" => 1, "size" => "(vd.getBit()==32)? 0 : 1", "opcode" => 0xd},  :arg => [2,3]},
      {"FMAXP"   => {"U" => 1, "size" => "(vd.getBit()==32)? 0 : 1", "opcode" => 0xf},  :arg => [2,3]},
      {"FMINNMP" => {"U" => 1, "size" => "(vd.getBit()==32)? 2 : 3", "opcode" => 0xc},  :arg => [2,3]},
      {"FMINP"   => {"U" => 1, "size" => "(vd.getBit()==32)? 2 : 3", "opcode" => 0xf},  :arg => [2,3]},
    ]
  },

  "AdvSimdSc3Diff" => {
    :cmt => "Advanced SIMD scalar three different",
    :arg => [
      [ {"vd" => "SReg"}, {"vn" => "HReg"}, {"vm" => "HReg"}], #0
      [ {"vd" => "DReg"}, {"vn" => "SReg"}, {"vm" => "SReg"}]  #1
    ],
    :prm => ["U", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"SQDMLAL" => {"U" => 0, "opcode" => 0x9}},
      {"SQDMLSL" => {"U" => 0, "opcode" => 0xb}},
      {"SQDMULL" => {"U" => 0, "opcode" => 0xd}},
    ]
  },

  "AdvSimdSc3Same" => {
    :cmt => "Advanced SIMD scalar three same",
    :arg => screg_set(3,"BHSD"),  # 0-3 (0:B, 1:H, 2:S, 3:D)
    # :arg => [[ {"vd" => "ScReg"}, {"vn" => "ScReg"}, {"vm" => "ScReg"}]],
    :prm => ["U", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"SQADD"    => {"U" => 0, "opcode" => 0x1}},
      {"SQSUB"    => {"U" => 0, "opcode" => 0x5}},
      {"CMGT"     => {"U" => 0, "opcode" => 0x6},  :arg => [3]},    # D only
      {"CMGE"     => {"U" => 0, "opcode" => 0x7},  :arg => [3]},    # D only
      {"SSHL"     => {"U" => 0, "opcode" => 0x8},  :arg => [3]},    # D only
      {"SQSHL"    => {"U" => 0, "opcode" => 0x9}},
      {"SRSHL"    => {"U" => 0, "opcode" => 0xa},  :arg => [3]},    # D only
      {"SQRSHL"   => {"U" => 0, "opcode" => 0xb}},
      {"ADD"      => {"U" => 0, "opcode" => 0x10}, :arg => [3]},    # D only
      {"CMTST"    => {"U" => 0, "opcode" => 0x11}, :arg => [3]},    # D only
      {"SQDMULH"  => {"U" => 0, "opcode" => 0x16}, :arg => [1,2]},  # H,S only
      {"UQADD"    => {"U" => 1, "opcode" => 0x1}},
      {"UQSUB"    => {"U" => 1, "opcode" => 0x5}},
      {"CMHI"     => {"U" => 1, "opcode" => 0x6},  :arg => [3]},    # D only
      {"CMHS"     => {"U" => 1, "opcode" => 0x7},  :arg => [3]},    # D only
      {"USHL"     => {"U" => 1, "opcode" => 0x8},  :arg => [3]},    # D only
      {"UQSHL"    => {"U" => 1, "opcode" => 0x9}},
      {"URSHL"    => {"U" => 1, "opcode" => 0xa},  :arg => [3]},    # D only
      {"UQRSHL"   => {"U" => 1, "opcode" => 0xb}},
      {"SUB"      => {"U" => 1, "opcode" => 0x10}, :arg => [3]},    # D only,
      {"CMEQ"     => {"U" => 1, "opcode" => 0x11}, :arg => [3]},    # D only,
      {"SQRDMULH" => {"U" => 1, "opcode" => 0x16}, :arg => [1,2]},  # H,S only
    ]
  },

  "AdvSimdSc3SameSz0x" => {
    :cmt => "Advanced SIMD scalar three same",
    :arg => [
      [ {"vd" => "SReg"}, {"vn" => "SReg"}, {"vm" => "SReg"}], #0
      [ {"vd" => "DReg"}, {"vn" => "DReg"}, {"vm" => "DReg"}]  #1
    ],
    # :arg => [[ {"vd" => "ScReg"}, {"vn" => "ScReg"}, {"vm" => "ScReg"}]],
    :prm => ["U", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"FMULX"    => {"U" => 0, "opcode" => 0x1b}},
      {"FCMEQ"    => {"U" => 0, "opcode" => 0x1c}},
      {"FRECPS"   => {"U" => 0, "opcode" => 0x1f}},
      {"FCMGE"    => {"U" => 1, "opcode" => 0x1c}},
      {"FACGE"    => {"U" => 1, "opcode" => 0x1d}},
    ]
  },

  "AdvSimdSc3SameSz1x" => {
    :cmt => "Advanced SIMD scalar three same",
    :arg => [
      [ {"vd" => "SReg"}, {"vn" => "SReg"}, {"vm" => "SReg"}], #0
      [ {"vd" => "DReg"}, {"vn" => "DReg"}, {"vm" => "DReg"}]  #1
    ],
    # :arg => [[ {"vd" => "ScReg"}, {"vn" => "ScReg"}, {"vm" => "ScReg"}]],
    :prm => ["U", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"FRSQRTS"  => {"U" => 0, "opcode" => 0x1f}},
      {"FABD"     => {"U" => 1, "opcode" => 0x1a}},
      {"FCMGT"    => {"U" => 1, "opcode" => 0x1c}},
      {"FACGT"    => {"U" => 1, "opcode" => 0x1d}}
    ]
  },

  "AdvSimdScShImm" => {
    :cmt => "Advanced SIMD scalar shift by immediate",
    :arg => ext_args(screg_set(2, "BHSD"), [{"imm" => "uint32_t"}]) +  #0-3 (0:B, 1:H, 2:S, 3:D)
            [
              [ {"vd" => "BReg"}, {"vn" => "HReg"}, {"imm" => "uint32_t"}], #4
              [ {"vd" => "HReg"}, {"vn" => "SReg"}, {"imm" => "uint32_t"}], #5
              [ {"vd" => "SReg"}, {"vn" => "DReg"}, {"imm" => "uint32_t"}]  #5
            ],
    :prm => ["U", "opcode", "vd", "vn", "imm"],
    :grp => [
      {"SSHR"     => {"U" => 0, "opcode" => 0x0},  :arg => [3]},
      {"SSRA"     => {"U" => 0, "opcode" => 0x2},  :arg => [3]},
      {"SRSHR"    => {"U" => 0, "opcode" => 0x4},  :arg => [3]},
      {"SRSRA"    => {"U" => 0, "opcode" => 0x6},  :arg => [3]},
      {"SHL"      => {"U" => 0, "opcode" => 0xa},  :arg => [3]},
      {"SQSHL"    => {"U" => 0, "opcode" => 0xe}},
      {"SQSHRN"   => {"U" => 0, "opcode" => 0x12}, :arg => [4,5,6]},
      {"SQRSHRN"  => {"U" => 0, "opcode" => 0x13}, :arg => [4,5,6]},
      {"SCVTF"    => {"U" => 0, "opcode" => 0x1c}, :arg => [1,2,3]},
      {"FCVTZS"   => {"U" => 0, "opcode" => 0x1f}, :arg => [1,2,3]},
      {"USHR"     => {"U" => 1, "opcode" => 0x0},  :arg => [3]},
      {"USRA"     => {"U" => 1, "opcode" => 0x2},  :arg => [3]},
      {"URSHR"    => {"U" => 1, "opcode" => 0x4},  :arg => [3]},
      {"URSRA"    => {"U" => 1, "opcode" => 0x6},  :arg => [3]},
      {"SRI"      => {"U" => 1, "opcode" => 0x8},  :arg => [3]},
      {"SLI"      => {"U" => 1, "opcode" => 0xa},  :arg => [3]},
      {"SQSHLU"   => {"U" => 1, "opcode" => 0xc}},
      {"UQSHL"    => {"U" => 1, "opcode" => 0xe}},
      {"SQSHRUN"  => {"U" => 1, "opcode" => 0x10}, :arg => [4,5,6]},
      {"SQRSHRUN" => {"U" => 1, "opcode" => 0x11}, :arg => [4,5,6]},
      {"UQSHRN"   => {"U" => 1, "opcode" => 0x12}, :arg => [4,5,6]},
      {"UQRSHRN"  => {"U" => 1, "opcode" => 0x13}, :arg => [4,5,6]},
      {"UCVTF"    => {"U" => 1, "opcode" => 0x1c}, :arg => [1,2,3]},
      {"FCVTZU"   => {"U" => 1, "opcode" => 0x1f}, :arg => [1,2,3]}
    ]
  },

  "AdvSimdScXIndElem" => {
    :cmt => "Advanced SIMD scalar x indexed element",
    :arg => [
      [ {"vd" => "SReg"}, {"vn" => "HReg"}, {"vm" => "VRegHElem"}], #0
      [ {"vd" => "DReg"}, {"vn" => "SReg"}, {"vm" => "VRegSElem"}], #1
      [ {"vd" => "HReg"}, {"vn" => "HReg"}, {"vm" => "VRegHElem"}], #2
      [ {"vd" => "SReg"}, {"vn" => "SReg"}, {"vm" => "VRegSElem"}], #3
      [ {"vd" => "DReg"}, {"vn" => "DReg"}, {"vm" => "VRegDElem"}]  #4
    ],
    :prm => ["U", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"SQDMLAL"  => {"U" => 0, "opcode" => 0x3}, :arg => [0,1]},
      {"SQDMLSL"  => {"U" => 0, "opcode" => 0x7}, :arg => [0,1]},
      {"SQDMULL"  => {"U" => 0, "opcode" => 0xb}, :arg => [0,1]},
      {"SQDMULH"  => {"U" => 0, "opcode" => 0xc}, :arg => [2,3]},
      {"SQRDMULH" => {"U" => 0, "opcode" => 0xd}, :arg => [2,3]},
      {"SQRDMLAH" => {"U" => 1, "opcode" => 0xd}, :arg => [2,3]},
      {"SQRDMLSH" => {"U" => 1, "opcode" => 0xf}, :arg => [2,3]},
    ]
  },

  "AdvSimdScXIndElemSz" => {
    :cmt => "Advanced SIMD scalar x indexed element",
    :arg => [
      [ {"vd" => "HReg"}, {"vn" => "HReg"}, {"vm" => "VRegHElem"}], #0
      [ {"vd" => "SReg"}, {"vn" => "SReg"}, {"vm" => "VRegSElem"}], #1
      [ {"vd" => "DReg"}, {"vn" => "DReg"}, {"vm" => "VRegDElem"}]  #2
    ],
    :prm => ["U", "size", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"FMLA"     => {"U" => 0, "size" => 0, "opcode" => 0x1}, :arg => [0]},
      {"FMLS"     => {"U" => 0, "size" => 0, "opcode" => 0x5}, :arg => [0]},
      {"FMUL"     => {"U" => 0, "size" => 0, "opcode" => 0x9}, :arg => [0]},
      {"FMLA"     => {"U" => 0, "size" => 2, "opcode" => 0x1}, :arg => [1]},
      {"FMLS"     => {"U" => 0, "size" => 2, "opcode" => 0x5}, :arg => [1]},
      {"FMUL"     => {"U" => 0, "size" => 2, "opcode" => 0x9}, :arg => [1]},
      {"FMLA"     => {"U" => 0, "size" => 3, "opcode" => 0x1}, :arg => [2]},
      {"FMLS"     => {"U" => 0, "size" => 3, "opcode" => 0x5}, :arg => [2]},
      {"FMUL"     => {"U" => 0, "size" => 3, "opcode" => 0x9}, :arg => [2]},
      {"FMULX"    => {"U" => 1, "size" => 0, "opcode" => 0x9}, :arg => [0]},
      {"FMULX"    => {"U" => 1, "size" => 2, "opcode" => 0x9}, :arg => [1]},
      {"FMULX"    => {"U" => 1, "size" => 3, "opcode" => 0x9}, :arg => [2]}
    ]
  },

  "AdvSimdTblLkup" => {
    :cmt => "Advanced SIMD table lookup",
    :arg => [
      [ {"vd" => "VReg8B"},  {"vn" => "VReg16B"}, {"len" => "uint32_t"}, {"vm" => "VReg8B"}],  #0
      [ {"vd" => "VReg16B"}, {"vn" => "VReg16B"}, {"len" => "uint32_t"}, {"vm" => "VReg16B"}], #1
      [ {"vd" => "VReg8B"},  {"vn" => "VReg16BList"}, {"vm" => "VReg8B"}],                     #2
      [ {"vd" => "VReg16B"}, {"vn" => "VReg16BList"}, {"vm" => "VReg16B"}]                     #3
    ],
    :prm => [
      ["op2", "len", "op", "vd", "vn", "vm"], #0
      ["op2", "op", "vd", "vn", "vm"]         #1
    ],
    :grp => [
      {"TBL"  => {"op2" => 0, "op" => 0}, :arg=> [0,1], :prm => 0},
      {"TBX"  => {"op2" => 0, "op" => 1}, :arg=> [0,1], :prm => 0},
      {"TBL"  => {"op2" => 0, "op" => 0}, :arg=> [2,3], :prm => 1},
      {"TBX"  => {"op2" => 0, "op" => 1}, :arg=> [2,3], :prm => 1}
    ]
  },

  "AdvSimdPermute" => {
    :cmt => "Advanced SIMD permute",
    :arg => vreg_set(3, "BHS") +
            qvreg_set(3, "BHSD"),
    # :arg => [[ {"vd" => "VReg"}, {"vn" => "VReg"}, {"vm" => "VReg"}]],
    :prm => ["opcode", "vd", "vn", "vm"],
    :grp => [
      {"UZP1"  => {"opcode" => 1}},
      {"TRN1"  => {"opcode" => 2}},
      {"ZIP1"  => {"opcode" => 3}},
      {"UZP2"  => {"opcode" => 5}},
      {"TRN2"  => {"opcode" => 6}},
      {"ZIP2"  => {"opcode" => 7}}
    ]
  },

  "AdvSimdExtract" => {
    :cmt => "Advanced SIMD extract",
    :arg => [
      [ {"vd" => "VReg8B"},  {"vn" => "VReg8B"},  {"vm" => "VReg8B"},  {"index" => "uint32_t"}], #0
      [ {"vd" => "VReg16B"}, {"vn" => "VReg16B"}, {"vm" => "VReg16B"}, {"index" => "uint32_t"}]  #1
    ],
    :prm => ["op2", "vd", "vn", "vm", "index"],
    :grp => [
      {"EXT"  => {"op2" => 0}},
    ]
  },

  "AdvSimdCopyDupElem" => {
    :cmt => "Advanced SIMD copy",
    :arg => [
      [ {"vd" => "VReg8B"},  {"vn" => "VRegBElem"}], #0
      [ {"vd" => "VReg16B"}, {"vn" => "VRegBElem"}], #1
      [ {"vd" => "VReg4H"},  {"vn" => "VRegHElem"}], #2
      [ {"vd" => "VReg8H"},  {"vn" => "VRegHElem"}], #3
      [ {"vd" => "VReg2S"},  {"vn" => "VRegSElem"}], #4
      [ {"vd" => "VReg4S"},  {"vn" => "VRegSElem"}], #5
      [ {"vd" => "VReg2D"},  {"vn" => "VRegDElem"}]  #6
    ],
    :prm => ["op", "imm4", "vd", "vn"],
    :grp => [
      {"DUP"  => {"op" => 0, "imm4" => 0}},
    ]
  },

  "AdvSimdCopyDupGen" => {
    :cmt => "Advanced SIMD copy",
    :arg => [
      [ {"vd" => "VReg8B"},  {"rn" => "WReg"}], #0
      [ {"vd" => "VReg16B"}, {"rn" => "WReg"}], #1
      [ {"vd" => "VReg4H"},  {"rn" => "WReg"}], #2
      [ {"vd" => "VReg8H"},  {"rn" => "WReg"}], #3
      [ {"vd" => "VReg2S"},  {"rn" => "WReg"}], #4
      [ {"vd" => "VReg4S"},  {"rn" => "WReg"}], #5
      [ {"vd" => "VReg2D"},  {"rn" => "XReg"}]  #6
    ],
    :prm => ["op", "imm4", "vd", "rn"],
    :grp => [
      {"DUP"  => {"op" => 0, "imm4" => 0}},
    ]
  },

  "AdvSimdCopyMov" => {
    :cmt => "Advanced SIMD copy",
    :arg => [
      [ {"rd" => "WReg"}, {"vn" => "VRegBElem"}], #0
      [ {"rd" => "WReg"}, {"vn" => "VRegHElem"}], #1
      [ {"rd" => "WReg"}, {"vn" => "VRegSElem"}], #2
      [ {"rd" => "XReg"}, {"vn" => "VRegBElem"}], #3
      [ {"rd" => "XReg"}, {"vn" => "VRegHElem"}], #4
      [ {"rd" => "XReg"}, {"vn" => "VRegSElem"}], #5
      [ {"rd" => "XReg"}, {"vn" => "VRegDElem"}]  #6
    ],
    :prm => ["op", "imm4", "rd", "vn"],
    :grp => [
      {"SMOV" => {"op" => 0, "imm4" => 5}, :arg => [0,1,3,4,5]},
      {"UMOV" => {"op" => 0, "imm4" => 7}, :arg => [0,1,2,6]},
      {"MOV"  => {"op" => 0, "imm4" => 7}, :arg => [2,6]}        # alias of UMOV
    ]
  },

  "AdvSimdCopyInsGen" => {
    :cmt => "Advanced SIMD copy",
    :arg => [
      [ {"vd" => "VRegBElem"}, {"rn" => "WReg"}], #0
      [ {"vd" => "VRegHElem"}, {"rn" => "WReg"}], #1
      [ {"vd" => "VRegSElem"}, {"rn" => "WReg"}], #2
      [ {"vd" => "VRegDElem"}, {"rn" => "XReg"}]  #3
    ],
    :prm => ["op", "imm4", "vd", "rn"],
    :grp => [
      {"INS"  => {"op" => 0, "imm4" => 3}},
      {"MOV"  => {"op" => 0, "imm4" => 3}}, # alias of INS
    ]
  },

  "AdvSimdCopyElemIns" => {
    :cmt => "Advanced SIMD copy",
    :arg => [
      [ {"vd" => "VRegBElem"}, {"vn" => "VRegBElem"}], #0
      [ {"vd" => "VRegHElem"}, {"vn" => "VRegHElem"}], #1
      [ {"vd" => "VRegSElem"}, {"vn" => "VRegSElem"}], #2
      [ {"vd" => "VRegDElem"}, {"vn" => "VRegDElem"}]  #3
    ],
    :prm => ["op", "vd", "vn"],
    :grp => [
      {"INS"  => {"op" => 1}},
      {"MOV"  => {"op" => 1}}, # alias of INS
    ]
  },

  "AdvSimd3SameFp16" => {
    :cmt => "Advanced SIMD three same (FP16)",
    :arg => [
      [ {"vd" => "VReg4H"}, {"vn" => "VReg4H"}, {"vm" => "VReg4H"}], #0
      [ {"vd" => "VReg8H"}, {"vn" => "VReg8H"}, {"vm" => "VReg8H"}]  #1
    ],
    :prm => ["U", "a", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"FMAXNM"  => {"U" => 0, "a" => 0, "opcode" => 0}},
      {"FMLA"    => {"U" => 0, "a" => 0, "opcode" => 1}},
      {"FADD"    => {"U" => 0, "a" => 0, "opcode" => 2}},
      {"FMULX"   => {"U" => 0, "a" => 0, "opcode" => 3}},
      {"FCMEQ"   => {"U" => 0, "a" => 0, "opcode" => 4}},
      {"FMAX"    => {"U" => 0, "a" => 0, "opcode" => 6}},
      {"FRECPS"  => {"U" => 0, "a" => 0, "opcode" => 7}},
      {"FMINNM"  => {"U" => 0, "a" => 1, "opcode" => 0}},
      {"FMLS"    => {"U" => 0, "a" => 1, "opcode" => 1}},
      {"FSUB"    => {"U" => 0, "a" => 1, "opcode" => 2}},
      {"FMIN"    => {"U" => 0, "a" => 1, "opcode" => 6}},
      {"FRSQRTS" => {"U" => 0, "a" => 1, "opcode" => 7}},
      {"FMAXNMP" => {"U" => 1, "a" => 0, "opcode" => 0}},
      {"FADDP"   => {"U" => 1, "a" => 0, "opcode" => 2}},
      {"FMUL"    => {"U" => 1, "a" => 0, "opcode" => 3}},
      {"FCMGE"   => {"U" => 1, "a" => 0, "opcode" => 4}},
      {"FACGE"   => {"U" => 1, "a" => 0, "opcode" => 5}},
      {"FMAXP"   => {"U" => 1, "a" => 0, "opcode" => 6}},
      {"FDIV"    => {"U" => 1, "a" => 0, "opcode" => 7}},
      {"FMINNMP" => {"U" => 1, "a" => 1, "opcode" => 0}},
      {"FABD"    => {"U" => 1, "a" => 1, "opcode" => 2}},
      {"FCMGT"   => {"U" => 1, "a" => 1, "opcode" => 4}},
      {"FACGT"   => {"U" => 1, "a" => 1, "opcode" => 5}},
      {"FMINP"   => {"U" => 1, "a" => 1, "opcode" => 6}}
    ]
  },

  "AdvSimd2RegMiscFp16" => {
    :cmt => "Advanced SIMD two-register miscellaneous (FP16)",
    :arg => [
      [ {"vd" => "VReg4H"}, {"vn" => "VReg4H"}], #0
      [ {"vd" => "VReg8H"}, {"vn" => "VReg8H"}], #1
      [ {"vd" => "VReg4H"}, {"vn" => "VReg4H"}, {"zero" => "double"}], #2
      [ {"vd" => "VReg8H"}, {"vn" => "VReg8H"}, {"zero" => "double"}]  #3
    ],
    :prm => [
      ["U", "a", "opcode", "vd", "vn"], #0
      ["U", "a", "opcode", "vd", "vn", "zero"]  #1
    ],
    :grp => [
      {"FRINTN"  => {"U" => 0, "a" => 0, "opcode" => 0x18}, :arg => 0..1, :prm => 0},
      {"FRINTM"  => {"U" => 0, "a" => 0, "opcode" => 0x19}, :arg => 0..1, :prm => 0},
      {"FCVTNS"  => {"U" => 0, "a" => 0, "opcode" => 0x1a}, :arg => 0..1, :prm => 0},
      {"FCVTMS"  => {"U" => 0, "a" => 0, "opcode" => 0x1b}, :arg => 0..1, :prm => 0},
      {"FCVTAS"  => {"U" => 0, "a" => 0, "opcode" => 0x1c}, :arg => 0..1, :prm => 0},
      {"SCVTF"   => {"U" => 0, "a" => 0, "opcode" => 0x1d}, :arg => 0..1, :prm => 0},
      {"FCMGT"   => {"U" => 0, "a" => 1, "opcode" => 0xc},  :arg => 2..3, :prm => 1},
      {"FCMEQ"   => {"U" => 0, "a" => 1, "opcode" => 0xd},  :arg => 2..3, :prm => 1},
      {"FCMLT"   => {"U" => 0, "a" => 1, "opcode" => 0xe},  :arg => 2..3, :prm => 1},
      {"FABS"    => {"U" => 0, "a" => 1, "opcode" => 0xf},  :arg => 0..1, :prm => 0},
      {"FRINTP"  => {"U" => 0, "a" => 1, "opcode" => 0x18}, :arg => 0..1, :prm => 0},
      {"FRINTZ"  => {"U" => 0, "a" => 1, "opcode" => 0x19}, :arg => 0..1, :prm => 0},
      {"FCVTPS"  => {"U" => 0, "a" => 1, "opcode" => 0x1a}, :arg => 0..1, :prm => 0},
      {"FCVTZS"  => {"U" => 0, "a" => 1, "opcode" => 0x1b}, :arg => 0..1, :prm => 0},
      {"FRECPE"  => {"U" => 0, "a" => 1, "opcode" => 0x1d}, :arg => 0..1, :prm => 0},
      {"FRINTA"  => {"U" => 1, "a" => 0, "opcode" => 0x18}, :arg => 0..1, :prm => 0},
      {"FRINTX"  => {"U" => 1, "a" => 0, "opcode" => 0x19}, :arg => 0..1, :prm => 0},
      {"FCVTNU"  => {"U" => 1, "a" => 0, "opcode" => 0x1a}, :arg => 0..1, :prm => 0},
      {"FCVTMU"  => {"U" => 1, "a" => 0, "opcode" => 0x1b}, :arg => 0..1, :prm => 0},
      {"FCVTAU"  => {"U" => 1, "a" => 0, "opcode" => 0x1c}, :arg => 0..1, :prm => 0},
      {"UCVTF"   => {"U" => 1, "a" => 0, "opcode" => 0x1d}, :arg => 0..1, :prm => 0},
      {"FCMGE"   => {"U" => 1, "a" => 1, "opcode" => 0xc},  :arg => 2..3, :prm => 1},
      {"FCMLE"   => {"U" => 1, "a" => 1, "opcode" => 0xd},  :arg => 2..3, :prm => 1},
      {"FNEG"    => {"U" => 1, "a" => 1, "opcode" => 0xf},  :arg => 0..1, :prm => 0},
      {"FRINTI"  => {"U" => 1, "a" => 1, "opcode" => 0x19}, :arg => 0..1, :prm => 0},
      {"FCVTPU"  => {"U" => 1, "a" => 1, "opcode" => 0x1a}, :arg => 0..1, :prm => 0},
      {"FCVTZU"  => {"U" => 1, "a" => 1, "opcode" => 0x1b}, :arg => 0..1, :prm => 0},
      {"FRSQRTE" => {"U" => 1, "a" => 1, "opcode" => 0x1d}, :arg => 0..1, :prm => 0},
      {"FSQRT"   => {"U" => 1, "a" => 1, "opcode" => 0x1f}, :arg => 0..1, :prm => 0}
    ]
  },

  "AdvSimd3SameExtra" => {
    :cmt => "Advanced SIMD three same extra",
    :arg => [
      [ {"vd" => "VReg2S"}, {"vn" => "VReg8B"},  {"vm" => "VReg8B"}],  #0
      [ {"vd" => "VReg4S"}, {"vn" => "VReg16B"}, {"vm" => "VReg16B"}], #1
      [ {"vd" => "VReg4H"}, {"vn" => "VReg4H"},  {"vm" => "VReg4H"}],  #2
      [ {"vd" => "VReg8H"}, {"vn" => "VReg8H"},  {"vm" => "VReg8H"}],  #3
      [ {"vd" => "VReg2S"}, {"vn" => "VReg2S"},  {"vm" => "VReg2S"}],  #4
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"},  {"vm" => "VReg4S"}]   #5
    ],
    :prm => ["U", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"SDOT"     => {"U" => 0, "opcode" => 0x2}, :arg => [0,1]},
      {"SQRDMLAH" => {"U" => 1, "opcode" => 0x0}, :arg => 2..5},
      {"SQRDMLSH" => {"U" => 1, "opcode" => 0x1}, :arg => 2..5},
      {"UDOT"     => {"U" => 1, "opcode" => 0x2}, :arg => [0,1]}
    ]
  },

  "AdvSimd3SameExtraRotate" => {
    :cmt => "Advanced SIMD three same extra",
    :arg => ext_args(vreg_set(3, "HS"), [{"rotate" => "uint32_t"}]) +
            ext_args(qvreg_set(3, "HSD"), [{"rotate" => "uint32_t"}]),
    # :arg => [[ {"vd" => "VReg"}, {"vn" => "VReg"}, {"vm" => "VReg"}, {"rotate" => "uint32_t"}]],
    :prm => ["U", "op32", "vd", "vn", "vm", "rotate"],
    :grp => [
      {"FCMLA" => {"U" => 1, "op32" => 0x2}},
      {"FCADD" => {"U" => 1, "op32" => 0x3}}
    ]
  },

  "AdvSimd2RegMisc" => {
    :cmt => "Advanced SIMD two-register miscellaneous",
    :arg => vreg_set(2,"BHS")   + #0-2 (0:8B, 1:4H, 2:2S)
            qvreg_set(2,"BHSD") + #3-6 (3:16B, 4:8H, 5:4S, 6:2D)
            [
              [{"vd" => "VReg4H"}, {"vn" => "VReg8B"}],  #7
              [{"vd" => "VReg8H"}, {"vn" => "VReg16B"}], #8
              [{"vd" => "VReg2S"}, {"vn" => "VReg4H"}],  #9
              [{"vd" => "VReg4S"}, {"vn" => "VReg8H"}],  #10
              [{"vd" => "VReg1D"}, {"vn" => "VReg2S"}],  #11
              [{"vd" => "VReg2D"}, {"vn" => "VReg4S"}],  #12
              
              [{"vd" => "VReg8B"}, {"vn" => "VReg8H"}],  #13
              [{"vd" => "VReg4H"}, {"vn" => "VReg4S"}],  #14
              [{"vd" => "VReg2S"}, {"vn" => "VReg2D"}],  #15
              [{"vd" => "VReg16B"},{"vn" => "VReg8H"}],  #16
              [{"vd" => "VReg8H"}, {"vn" => "VReg4S"}],  #17
              [{"vd" => "VReg4S"}, {"vn" => "VReg2D"}],  #18
              
              [{"vd" => "VReg8H"}, {"vn" => "VReg8B"}, {"sh" => "uint32_t"}],  #19
              [{"vd" => "VReg4S"}, {"vn" => "VReg4H"}, {"sh" => "uint32_t"}],  #20
              [{"vd" => "VReg2D"}, {"vn" => "VReg2S"}, {"sh" => "uint32_t"}],  #21
              [{"vd" => "VReg8H"}, {"vn" => "VReg16B"},{"sh" => "uint32_t"}],  #22
              [{"vd" => "VReg4S"}, {"vn" => "VReg8H"}, {"sh" => "uint32_t"}],  #23
              [{"vd" => "VReg2D"}, {"vn" => "VReg4S"}, {"sh" => "uint32_t"}],  #24
            ],
    :prm => [
      ["U", "opcode", "vd", "vn"],       #0
      ["U", "opcode", "vd", "vn", "sh"]  #1
    ],
    :grp => [
      {"REV64"  => {"U" => 0, "opcode" => 0x0}, :arg => 0..5,   :prm => 0},
      {"REV16"  => {"U" => 0, "opcode" => 0x1}, :arg => [0,3],  :prm => 0},
      {"SADDLP" => {"U" => 0, "opcode" => 0x2}, :arg => 7..12,  :prm => 0},
      {"SUQADD" => {"U" => 0, "opcode" => 0x3}, :arg => 0..6,   :prm => 0},
      {"CLS"    => {"U" => 0, "opcode" => 0x4}, :arg => 0..5,   :prm => 0},
      {"CNT"    => {"U" => 0, "opcode" => 0x5}, :arg => [0,3],  :prm => 0},
      {"SADALP" => {"U" => 0, "opcode" => 0x6}, :arg => 7..12,  :prm => 0},
      {"SQABS"  => {"U" => 0, "opcode" => 0x7}, :arg => 0..6,   :prm => 0},
      {"ABS"    => {"U" => 0, "opcode" => 0xb}, :arg => 0..6,   :prm => 0},
      {"XTN"    => {"U" => 0, "opcode" => 0x12},:arg => 13..15, :prm => 0},
      {"XTN2"   => {"U" => 0, "opcode" => 0x12},:arg => 16..18, :prm => 0},
      {"SQXTN"  => {"U" => 0, "opcode" => 0x14},:arg => 13..15, :prm => 0},
      {"SQXTN2" => {"U" => 0, "opcode" => 0x14},:arg => 16..18, :prm => 0},
      {"REV32"  => {"U" => 1, "opcode" => 0x0}, :arg => [0,1,3,4], :prm => 0},
      {"UADDLP" => {"U" => 1, "opcode" => 0x2}, :arg => 7..12,  :prm => 0},
      {"USQADD" => {"U" => 1, "opcode" => 0x3}, :arg => 0..6,   :prm => 0},
      {"CLZ"    => {"U" => 1, "opcode" => 0x4}, :arg => 0..5,   :prm => 0},
      {"UADALP" => {"U" => 1, "opcode" => 0x6}, :arg => 7..12,  :prm => 0},
      {"SQNEG"  => {"U" => 1, "opcode" => 0x7}, :arg => 0..6,   :prm => 0},
      {"NEG"    => {"U" => 1, "opcode" => 0xb}, :arg => 0..6,   :prm => 0},
      {"SQXTUN" => {"U" => 1, "opcode" => 0x12},:arg => 13..15, :prm => 0},
      {"SQXTUN2"=> {"U" => 1, "opcode" => 0x12},:arg => 16..18, :prm => 0},
      {"SHLL"   => {"U" => 1, "opcode" => 0x13},:arg => 19..21, :prm => 1},
      {"SHLL2"  => {"U" => 1, "opcode" => 0x13},:arg => 22..24, :prm => 1},
      {"UQXTN"  => {"U" => 1, "opcode" => 0x14},:arg => 13..15, :prm => 0},
      {"UQXTN2" => {"U" => 1, "opcode" => 0x14},:arg => 16..18, :prm => 0},
    ]
  },

  "AdvSimd2RegMiscZero" => {
    :cmt => "Advanced SIMD two-register miscellaneous",
    :arg => ext_args(vreg_set(2,"BHS"), [{"zero" => "uint32_t"}])   + #0-2 (0:8B, 1:4H, 2:2S),
            ext_args(qvreg_set(2,"BHSD"), [{"zero" => "uint32_t"}]),  #3-6 (3:16B, 4:8H, 5:4S, 6:2D)
    :prm => ["U", "opcode", "vd", "vn", "zero"],
    :grp => [
      {"CMGT"   => {"U" => 0, "opcode" => 0x8}},
      {"CMEQ"   => {"U" => 0, "opcode" => 0x9}},
      {"CMLT"   => {"U" => 0, "opcode" => 0xa}},
      {"CMGE"   => {"U" => 1, "opcode" => 0x8}},
      {"CMLE"   => {"U" => 1, "opcode" => 0x9}}
    ]
  },

  "AdvSimd2RegMiscSz" => {
    :cmt => "Advanced SIMD two-register miscellaneous",
    :arg => vreg_set(2,"B") + #0 (0:8B)
            qvreg_set(2,"B"), #1 (1:16B)
    :prm => ["U", "size", "opcode", "vd", "vn"], #0
    :grp => [
      {"NOT"    => {"U" => 1, "size" => 0, "opcode" => 0x5}},
      {"MVN"    => {"U" => 1, "size" => 0, "opcode" => 0x5}}, # alias of NOT
      {"RBIT"   => {"U" => 1, "size" => 1, "opcode" => 0x5}}
    ]
  },

  
  "AdvSimd2RegMiscSz0x" => {
    :cmt => "Advanced SIMD two-register miscellaneous",
    :arg => [
      [ {"vd" => "VReg4H"}, {"vn" => "VReg4S"}], #0
      [ {"vd" => "VReg2S"}, {"vn" => "VReg2D"}], #1
      [ {"vd" => "VReg8H"}, {"vn" => "VReg4S"}], #2
      [ {"vd" => "VReg4S"}, {"vn" => "VReg2D"}], #3

      [ {"vd" => "VReg4S"}, {"vn" => "VReg4H"}], #4
      [ {"vd" => "VReg2D"}, {"vn" => "VReg2S"}], #5
      [ {"vd" => "VReg4S"}, {"vn" => "VReg8H"}], #6
      [ {"vd" => "VReg2D"}, {"vn" => "VReg4S"}], #7
      
      [ {"vd" => "VReg2S"}, {"vn" => "VReg2S"}], #8
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}], #9
      [ {"vd" => "VReg2D"}, {"vn" => "VReg2D"}]  #10
    ],
    :prm => ["U", "opcode", "vd", "vn"],
    :grp => [
      {"FCVTN"  => {"U" => 0, "opcode" => 0x16}, :arg => 0..1},
      {"FCVTN2" => {"U" => 0, "opcode" => 0x16}, :arg => 2..3},
      {"FCVTL"  => {"U" => 0, "opcode" => 0x17}, :arg => 4..5},
      {"FCVTL2" => {"U" => 0, "opcode" => 0x17}, :arg => 6..7},
      {"FRINTN" => {"U" => 0, "opcode" => 0x18}, :arg => 8..10},
      {"FRINTM" => {"U" => 0, "opcode" => 0x19}, :arg => 8..10},
      {"FCVTNS" => {"U" => 0, "opcode" => 0x1a}, :arg => 8..10},
      {"FCVTMS" => {"U" => 0, "opcode" => 0x1b}, :arg => 8..10},
      {"FCVTAS" => {"U" => 0, "opcode" => 0x1c}, :arg => 8..10},
      {"SCVTF"  => {"U" => 0, "opcode" => 0x1d}, :arg => 8..10},
      {"FCVTXN" => {"U" => 1, "opcode" => 0x16}, :arg => [1,3]},
      {"FCVTXN2"=> {"U" => 1, "opcode" => 0x16}, :arg => [1,3]},
      {"FRINTA" => {"U" => 1, "opcode" => 0x18}, :arg => 8..10},
      {"FRINTX" => {"U" => 1, "opcode" => 0x19}, :arg => 8..10},
      {"FCVTNU" => {"U" => 1, "opcode" => 0x1a}, :arg => 8..10},
      {"FCVTMU" => {"U" => 1, "opcode" => 0x1b}, :arg => 8..10},
      {"FCVTAU" => {"U" => 1, "opcode" => 0x1c}, :arg => 8..10},
      {"UCVTF"  => {"U" => 1, "opcode" => 0x1d}, :arg => 8..10}
    ]
  },

  "AdvSimd2RegMiscSz1x" => {
    :cmt => "Advanced SIMD two-register miscellaneous",
    :arg => [
      [ {"vd" => "VReg2S"}, {"vn" => "VReg2S"}], #0
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}], #1
      [ {"vd" => "VReg2D"}, {"vn" => "VReg2D"}], #2
      [ {"vd" => "VReg2S"}, {"vn" => "VReg2S"}, {"zero" => "double"}], #3
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}, {"zero" => "double"}], #4
      [ {"vd" => "VReg2D"}, {"vn" => "VReg2D"}, {"zero" => "double"}]  #5
    ],
    :prm => [
      ["U", "opcode", "vd", "vn"],         #0
      ["U", "opcode", "vd", "vn", "zero"]  #1
    ],
    :grp => [
      {"FCMGT"  => {"U" => 0, "opcode" => 0xc},  :arg => 3..5, :prm => 1},
      {"FCMEQ"  => {"U" => 0, "opcode" => 0xd},  :arg => 3..5, :prm => 1},
      {"FCMLT"  => {"U" => 0, "opcode" => 0xe},  :arg => 3..5, :prm => 1},
      {"FABS"   => {"U" => 0, "opcode" => 0xf},  :arg => 0..2, :prm => 0},
      {"FRINTP" => {"U" => 0, "opcode" => 0x18}, :arg => 0..2, :prm => 0},
      {"FRINTZ" => {"U" => 0, "opcode" => 0x19}, :arg => 0..2, :prm => 0},
      {"FCVTPS" => {"U" => 0, "opcode" => 0x1a}, :arg => 0..2, :prm => 0},
      {"FCVTZS" => {"U" => 0, "opcode" => 0x1b}, :arg => 0..2, :prm => 0},
      {"URECPE" => {"U" => 0, "opcode" => 0x1c}, :arg => 0..1, :prm => 0},
      {"FRECPE" => {"U" => 0, "opcode" => 0x1d}, :arg => 0..2, :prm => 0},
      {"FCMGE"  => {"U" => 1, "opcode" => 0xc},  :arg => 3..5, :prm => 1},
      {"FCMLE"  => {"U" => 1, "opcode" => 0xd},  :arg => 3..5, :prm => 1},
      {"FNEG"   => {"U" => 1, "opcode" => 0xf},  :arg => 0..2, :prm => 0},
      {"FRINTI" => {"U" => 1, "opcode" => 0x19}, :arg => 0..2, :prm => 0},
      {"FCVTPU" => {"U" => 1, "opcode" => 0x1a}, :arg => 0..2, :prm => 0},
      {"FCVTZU" => {"U" => 1, "opcode" => 0x1b}, :arg => 0..2, :prm => 0},
      {"URSQRTE"=> {"U" => 1, "opcode" => 0x1c}, :arg => 0..1, :prm => 0},
      {"FRSQRTE"=> {"U" => 1, "opcode" => 0x1d}, :arg => 0..2, :prm => 0},
      {"FSQRT"  => {"U" => 1, "opcode" => 0x1f}, :arg => 0..2, :prm => 0}
    ]
  },

  "AdvSimdAcrossLanes" => {
    :cmt => "Advanced SIMD across lanes",
    :arg => [
      [ {"vd" => "HReg"}, {"vn" => "VReg8B"}],   #0
      [ {"vd" => "HReg"}, {"vn" => "VReg16B"}],  #1
      [ {"vd" => "SReg"}, {"vn" => "VReg4H"}],   #2
      [ {"vd" => "SReg"}, {"vn" => "VReg8H"}],   #3
      [ {"vd" => "DReg"}, {"vn" => "VReg4S"}],   #4

      [ {"vd" => "BReg"}, {"vn" => "VReg8B"}],   #5
      [ {"vd" => "BReg"}, {"vn" => "VReg16B"}],  #6
      [ {"vd" => "HReg"}, {"vn" => "VReg4H"}],   #7
      [ {"vd" => "HReg"}, {"vn" => "VReg8H"}],   #8
      [ {"vd" => "SReg"}, {"vn" => "VReg4S"}]    #9
    ],
    :prm => ["U", "opcode", "vd", "vn"],
    :grp => [
      {"SADDLV" => {"U" => 0, "opcode" => 0x3},  :arg => 0..4},
      {"SMAXV"  => {"U" => 0, "opcode" => 0xa},  :arg => 5..9},
      {"SMINV"  => {"U" => 0, "opcode" => 0x1a}, :arg => 5..9},
      {"ADDV"   => {"U" => 0, "opcode" => 0x1b}, :arg => 5..9},
      {"UADDLV" => {"U" => 1, "opcode" => 0x3},  :arg => 0..4},
      {"UMAXV"  => {"U" => 1, "opcode" => 0xa},  :arg => 5..9},
      {"UMINV"  => {"U" => 1, "opcode" => 0x1a}, :arg => 5..9}
    ]
  },

  "AdvSimdAcrossLanesSz0x" => {
    :cmt => "Advanced SIMD across lanes",
    :arg => [
      [ {"vd" => "HReg"}, {"vn" => "VReg4H"}],   #0
      [ {"vd" => "HReg"}, {"vn" => "VReg8H"}],   #1
      [ {"vd" => "SReg"}, {"vn" => "VReg4S"}]    #2
    ],
    :prm => ["U", "opcode", "vd", "vn"],
    :grp => [
      {"FMAXNMV" => {"U" => 0, "opcode" => 0xc}, :arg => [0,1]},
      {"FMAXV"   => {"U" => 0, "opcode" => 0xf}, :arg => [0,1]},
      {"FMAXNMV" => {"U" => 1, "opcode" => 0xc}, :arg => [2]},
      {"FMAXV"   => {"U" => 1, "opcode" => 0xf}, :arg => [2]},
    ]
  },

  "AdvSimdAcrossLanesSz1x" => {
    :cmt => "Advanced SIMD across lanes",
    :arg => [
      [ {"vd" => "HReg"}, {"vn" => "VReg4H"}],   #0
      [ {"vd" => "HReg"}, {"vn" => "VReg8H"}],   #1
      [ {"vd" => "SReg"}, {"vn" => "VReg4S"}]    #2
    ],
    :prm => ["U", "opcode", "vd", "vn"],
    :grp => [
      {"FMINNMV" => {"U" => 0, "opcode" => 0xc}, :arg => [0,1]},
      {"FMINV"   => {"U" => 0, "opcode" => 0xf}, :arg => [0,1]},
      {"FMINNMV" => {"U" => 1, "opcode" => 0xc}, :arg => [2]},
      {"FMINV"   => {"U" => 1, "opcode" => 0xf}, :arg => [2]},
    ]
  },
  
  "AdvSimd3Diff" => {
    :cmt => "Advanced SIMD three different",
    :arg => [
      [ {"vd" => "VReg8H"}, {"vn" => "VReg8B"},  {"vm" => "VReg8B"}],  #0
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4H"},  {"vm" => "VReg4H"}],  #1
      [ {"vd" => "VReg2D"}, {"vn" => "VReg2S"},  {"vm" => "VReg2S"}],  #2
      [ {"vd" => "VReg8H"}, {"vn" => "VReg16B"}, {"vm" => "VReg16B"}], #3
      [ {"vd" => "VReg4S"}, {"vn" => "VReg8H"},  {"vm" => "VReg8H"}],  #4
      [ {"vd" => "VReg2D"}, {"vn" => "VReg4S"},  {"vm" => "VReg4S"}],  #5

      [ {"vd" => "VReg8H"}, {"vn" => "VReg8H"}, {"vm" => "VReg8B"}],  #6
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}, {"vm" => "VReg4H"}],  #7
      [ {"vd" => "VReg2D"}, {"vn" => "VReg2D"}, {"vm" => "VReg2S"}],  #8
      [ {"vd" => "VReg8H"}, {"vn" => "VReg8H"}, {"vm" => "VReg16B"}], #9
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}, {"vm" => "VReg8H"}],  #10
      [ {"vd" => "VReg2D"}, {"vn" => "VReg2D"}, {"vm" => "VReg4S"}],  #11

      [ {"vd" => "VReg8B"},  {"vn" => "VReg8H"}, {"vm" => "VReg8H"}], #12
      [ {"vd" => "VReg4H"},  {"vn" => "VReg4S"}, {"vm" => "VReg4S"}], #13
      [ {"vd" => "VReg2S"},  {"vn" => "VReg2D"}, {"vm" => "VReg2D"}], #14
      [ {"vd" => "VReg16B"}, {"vn" => "VReg8H"}, {"vm" => "VReg8H"}], #15
      [ {"vd" => "VReg8H"},  {"vn" => "VReg4S"}, {"vm" => "VReg4S"}], #16
      [ {"vd" => "VReg4S"},  {"vn" => "VReg2D"}, {"vm" => "VReg2D"}], #17

      [ {"vd" => "VReg1Q"},  {"vn" => "VReg1D"}, {"vm" => "VReg1D"}], #18
      [ {"vd" => "VReg1Q"},  {"vn" => "VReg2D"}, {"vm" => "VReg2D"}], #19
    ],
    :prm => ["U", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"SADDL"    => {"U" => 0, "opcode" => 0x0}, :arg => 0..2},
      {"SADDL2"   => {"U" => 0, "opcode" => 0x0}, :arg => 3..5},
      {"SADDW"    => {"U" => 0, "opcode" => 0x1}, :arg => 6..8},
      {"SADDW2"   => {"U" => 0, "opcode" => 0x1}, :arg => 9..11},
      {"SSUBL"    => {"U" => 0, "opcode" => 0x2}, :arg => 0..2},
      {"SSUBL2"   => {"U" => 0, "opcode" => 0x2}, :arg => 3..5},
      {"SSUBW"    => {"U" => 0, "opcode" => 0x3}, :arg => 6..8},
      {"SSUBW2"   => {"U" => 0, "opcode" => 0x3}, :arg => 9..11},
      {"ADDHN"    => {"U" => 0, "opcode" => 0x4}, :arg => 12..14},
      {"ADDHN2"   => {"U" => 0, "opcode" => 0x4}, :arg => 15..17},
      {"SABAL"    => {"U" => 0, "opcode" => 0x5}, :arg => 0..2},
      {"SABAL2"   => {"U" => 0, "opcode" => 0x5}, :arg => 3..5},
      {"SUBHN"    => {"U" => 0, "opcode" => 0x6}, :arg => 12..14},
      {"SUBHN2"   => {"U" => 0, "opcode" => 0x6}, :arg => 15..17},
      {"SABDL"    => {"U" => 0, "opcode" => 0x7}, :arg => 0..2},
      {"SABDL2"   => {"U" => 0, "opcode" => 0x7}, :arg => 3..5},
      {"SMLAL"    => {"U" => 0, "opcode" => 0x8}, :arg => 0..2},
      {"SMLAL2"   => {"U" => 0, "opcode" => 0x8}, :arg => 3..5},
      {"SQDMLAL"  => {"U" => 0, "opcode" => 0x9}, :arg => 1..2},
      {"SQDMLAL2" => {"U" => 0, "opcode" => 0x9}, :arg => 4..5},
      {"SMLSL"    => {"U" => 0, "opcode" => 0xa}, :arg => 0..2},
      {"SMLSL2"   => {"U" => 0, "opcode" => 0xa}, :arg => 3..5},
      {"SQDMLSL"  => {"U" => 0, "opcode" => 0xb}, :arg => 1..2},
      {"SQDMLSL2" => {"U" => 0, "opcode" => 0xb}, :arg => 4..5},
      {"SMULL"    => {"U" => 0, "opcode" => 0xc}, :arg => 0..2},
      {"SMULL2"   => {"U" => 0, "opcode" => 0xc}, :arg => 3..5},
      {"SQDMULL"  => {"U" => 0, "opcode" => 0xd}, :arg => 1..2},
      {"SQDMULL2" => {"U" => 0, "opcode" => 0xd}, :arg => 4..5},
      {"PMULL"    => {"U" => 0, "opcode" => 0xe}, :arg => [0,18]},
      {"PMULL2"   => {"U" => 0, "opcode" => 0xe}, :arg => [3,19]},
      {"UADDL"    => {"U" => 1, "opcode" => 0x0}, :arg => 0..2},
      {"UADDL2"   => {"U" => 1, "opcode" => 0x0}, :arg => 3..5},
      {"UADDW"    => {"U" => 1, "opcode" => 0x1}, :arg => 6..8},
      {"UADDW2"   => {"U" => 1, "opcode" => 0x1}, :arg => 9..11},
      {"USUBL"    => {"U" => 1, "opcode" => 0x2}, :arg => 0..2},
      {"USUBL2"   => {"U" => 1, "opcode" => 0x2}, :arg => 3..5},
      {"USUBW"    => {"U" => 1, "opcode" => 0x3}, :arg => 6..8},
      {"USUBW2"   => {"U" => 1, "opcode" => 0x3}, :arg => 9..11},
      {"RADDHN"   => {"U" => 1, "opcode" => 0x4}, :arg => 12..14},
      {"RADDHN2"  => {"U" => 1, "opcode" => 0x4}, :arg => 15..17},
      {"UABAL"    => {"U" => 1, "opcode" => 0x5}, :arg => 0..2},
      {"UABAL2"   => {"U" => 1, "opcode" => 0x5}, :arg => 3..5},
      {"RSUBHN"   => {"U" => 1, "opcode" => 0x6}, :arg => 12..14},
      {"RSUBHN2"  => {"U" => 1, "opcode" => 0x6}, :arg => 15..17},
      {"UABDL"    => {"U" => 1, "opcode" => 0x7}, :arg => 0..2},
      {"UABDL2"   => {"U" => 1, "opcode" => 0x7}, :arg => 3..5},
      {"UMLAL"    => {"U" => 1, "opcode" => 0x8}, :arg => 0..2},
      {"UMLAL2"   => {"U" => 1, "opcode" => 0x8}, :arg => 3..5},
      {"UMLSL"    => {"U" => 1, "opcode" => 0xa}, :arg => 0..2},
      {"UMLSL2"   => {"U" => 1, "opcode" => 0xa}, :arg => 3..5},
      {"UMULL"    => {"U" => 1, "opcode" => 0xc}, :arg => 0..2},
      {"UMULL2"   => {"U" => 1, "opcode" => 0xc}, :arg => 3..5}
    ]
  },

  "AdvSimd3Same" => {
    :cmt => "Advanced SIMD three same",
    :arg => vreg_set(3, "BHS") +  #0-2 (0:8B, 1:4H, 2:2S)
            qvreg_set(3, "BHSD"), #3-6 (3:16B, 4:8H, 5:4S, 6:2D)
    :prm => ["U", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"SHADD"    => {"U" => 0, "opcode" => 0x0}, :arg => 0..5},
      {"SQADD"    => {"U" => 0, "opcode" => 0x1}, :arg => 0..6},
      {"SRHADD"   => {"U" => 0, "opcode" => 0x2}, :arg => 0..5},
      {"SHSUB"    => {"U" => 0, "opcode" => 0x4}, :arg => 0..5},
      {"SQSUB"    => {"U" => 0, "opcode" => 0x5}, :arg => 0..6},
      {"CMGT"     => {"U" => 0, "opcode" => 0x6}, :arg => 0..6},
      {"CMGE"     => {"U" => 0, "opcode" => 0x7}, :arg => 0..6},
      {"SSHL"     => {"U" => 0, "opcode" => 0x8}, :arg => 0..6},
      {"SQSHL"    => {"U" => 0, "opcode" => 0x9}, :arg => 0..6},
      {"SRSHL"    => {"U" => 0, "opcode" => 0xa}, :arg => 0..6},
      {"SQRSHL"   => {"U" => 0, "opcode" => 0xb}, :arg => 0..6},
      {"SMAX"     => {"U" => 0, "opcode" => 0xc}, :arg => 0..5},
      {"SMIN"     => {"U" => 0, "opcode" => 0xd}, :arg => 0..5},
      {"SABD"     => {"U" => 0, "opcode" => 0xe}, :arg => 0..5},
      {"SABA"     => {"U" => 0, "opcode" => 0xf}, :arg => 0..5},
      {"ADD"      => {"U" => 0, "opcode" => 0x10},:arg => 0..6},
      {"CMTST"    => {"U" => 0, "opcode" => 0x11},:arg => 0..6},
      {"MLA"      => {"U" => 0, "opcode" => 0x12},:arg => 0..5},
      {"MUL"      => {"U" => 0, "opcode" => 0x13},:arg => 0..5},
      {"SMAXP"    => {"U" => 0, "opcode" => 0x14},:arg => 0..5},
      {"SMINP"    => {"U" => 0, "opcode" => 0x15},:arg => 0..5},
      {"SQDMULH"  => {"U" => 0, "opcode" => 0x16},:arg => [1,2,4,5]},
      {"ADDP"     => {"U" => 0, "opcode" => 0x17},:arg => 0..6},
      {"UHADD"    => {"U" => 1, "opcode" => 0x0}, :arg => 0..5},
      {"UQADD"    => {"U" => 1, "opcode" => 0x1}, :arg => 0..6},
      {"URHADD"   => {"U" => 1, "opcode" => 0x2}, :arg => 0..5},
      {"UHSUB"    => {"U" => 1, "opcode" => 0x4}, :arg => 0..5},
      {"UQSUB"    => {"U" => 1, "opcode" => 0x5}, :arg => 0..6},
      {"CMHI"     => {"U" => 1, "opcode" => 0x6}, :arg => 0..6},
      {"CMHS"     => {"U" => 1, "opcode" => 0x7}, :arg => 0..6},
      {"USHL"     => {"U" => 1, "opcode" => 0x8}, :arg => 0..6},
      {"UQSHL"    => {"U" => 1, "opcode" => 0x9}, :arg => 0..6},
      {"URSHL"    => {"U" => 1, "opcode" => 0xa}, :arg => 0..6},
      {"UQRSHL"   => {"U" => 1, "opcode" => 0xb}, :arg => 0..6},
      {"UMAX"     => {"U" => 1, "opcode" => 0xc}, :arg => 0..5},
      {"UMIN"     => {"U" => 1, "opcode" => 0xd}, :arg => 0..5},
      {"UABD"     => {"U" => 1, "opcode" => 0xe}, :arg => 0..5},
      {"UABA"     => {"U" => 1, "opcode" => 0xf}, :arg => 0..5},
      {"SUB"      => {"U" => 1, "opcode" => 0x10},:arg => 0..6},
      {"CMEQ"     => {"U" => 1, "opcode" => 0x11},:arg => 0..6},
      {"MLS"      => {"U" => 1, "opcode" => 0x12},:arg => 0..5},
      {"PMUL"     => {"U" => 1, "opcode" => 0x13},:arg => [0,3]},
      {"UMAXP"    => {"U" => 1, "opcode" => 0x14},:arg => 0..5},
      {"UMINP"    => {"U" => 1, "opcode" => 0x15},:arg => 0..5},
      {"SQRDMULH" => {"U" => 1, "opcode" => 0x16},:arg => [1,2,4,5]}
    ]
  },

  "AdvSimd3SameSz0x" => {
    :cmt => "Advanced SIMD three same",
    :arg => [
      [ {"vd" => "VReg2S"}, {"vn" => "VReg2S"}, {"vm" => "VReg2S"}], #0
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}, {"vm" => "VReg4S"}], #1
      [ {"vd" => "VReg2D"}, {"vn" => "VReg2D"}, {"vm" => "VReg2D"}]  #2
    ],
    :prm => ["U", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"FMAXNM"    => {"U" => 0, "opcode" => 0x18}, :arg => 0..2},
      {"FMLA"      => {"U" => 0, "opcode" => 0x19}, :arg => 0..2},
      {"FADD"      => {"U" => 0, "opcode" => 0x1a}, :arg => 0..2},
      {"FMULX"     => {"U" => 0, "opcode" => 0x1b}, :arg => 0..2},
      {"FCMEQ"     => {"U" => 0, "opcode" => 0x1c}, :arg => 0..2},
      {"FMAX"      => {"U" => 0, "opcode" => 0x1e}, :arg => 0..2},
      {"FRECPS"    => {"U" => 0, "opcode" => 0x1f}, :arg => 0..2},
      {"FMAXNMP"   => {"U" => 1, "opcode" => 0x18}, :arg => 0..2},
      {"FADDP"     => {"U" => 1, "opcode" => 0x1a}, :arg => 0..2},
      {"FMUL"      => {"U" => 1, "opcode" => 0x1b}, :arg => 0..2},
      {"FCMGE"     => {"U" => 1, "opcode" => 0x1c}, :arg => 0..2},
      {"FACGE"     => {"U" => 1, "opcode" => 0x1d}, :arg => 0..2},
      {"FMAXP"     => {"U" => 1, "opcode" => 0x1e}, :arg => 0..2},
      {"FDIV"      => {"U" => 1, "opcode" => 0x1f}, :arg => 0..2}
    ]
  },

  "AdvSimd3SameSz1x" => {
    :cmt => "Advanced SIMD three same",
    :arg => [
      [ {"vd" => "VReg2S"}, {"vn" => "VReg2S"}, {"vm" => "VReg2S"}], #0
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}, {"vm" => "VReg4S"}], #1
      [ {"vd" => "VReg2D"}, {"vn" => "VReg2D"}, {"vm" => "VReg2D"}]  #2
    ],
    :prm => ["U", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"FMINNM"    => {"U" => 0, "opcode" => 0x18}, :arg => 0..2},
      {"FMLS"      => {"U" => 0, "opcode" => 0x19}, :arg => 0..2},
      {"FSUB"      => {"U" => 0, "opcode" => 0x1a}, :arg => 0..2},
      {"FMIN"      => {"U" => 0, "opcode" => 0x1e}, :arg => 0..2},
      {"FRSQRTS"   => {"U" => 0, "opcode" => 0x1f}, :arg => 0..2},
      {"FMINNMP"   => {"U" => 1, "opcode" => 0x18}, :arg => 0..2},
      {"FABD"      => {"U" => 1, "opcode" => 0x1a}, :arg => 0..2},
      {"FCMGT"     => {"U" => 1, "opcode" => 0x1c}, :arg => 0..2},
      {"FACGT"     => {"U" => 1, "opcode" => 0x1d}, :arg => 0..2},
      {"FMINP"     => {"U" => 1, "opcode" => 0x1e}, :arg => 0..2}
    ]
  },

  "AdvSimd3SameSz" => {
    :cmt => "Advanced SIMD three same",
    :arg => [
      [ {"vd" => "VReg8B"},  {"vn" => "VReg8B"},  {"vm" => "VReg8B"}],  #0
      [ {"vd" => "VReg16B"}, {"vn" => "VReg16B"}, {"vm" => "VReg16B"}], #1

      [ {"vd" => "VReg2S"}, {"vn" => "VReg2H"}, {"vm" => "VReg2H"}], #2
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4H"}, {"vm" => "VReg4H"}], #3

      [ {"vd" => "VReg8B"},  {"vn" => "VReg8B"}],                    #4
      [ {"vd" => "VReg16B"}, {"vn" => "VReg16B"}],                   #5
    ],
    :prm => ["U", "size", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"AND"    => {"U" => 0, "size" => 0, "opcode" => 0x3},                :arg => 0..1},
      {"FMLAL"  => {"U" => 0, "size" => 0, "opcode" => 0x1d},               :arg => 2..3},
      {"BIC"    => {"U" => 0, "size" => 1, "opcode" => 0x3},                :arg => 0..1},
      {"ORR"    => {"U" => 0, "size" => 2, "opcode" => 0x3},                :arg => 0..1},
      {"MOV"    => {"U" => 0, "size" => 2, "opcode" => 0x3, "vm" => "vn"},  :arg => 4..5}, # alias of ORR
      {"FMLSL"  => {"U" => 0, "size" => 2, "opcode" => 0x1d},               :arg => 2..3},
      {"ORN"    => {"U" => 0, "size" => 3, "opcode" => 0x3},                :arg => 0..1},
      {"EOR"    => {"U" => 1, "size" => 0, "opcode" => 0x3},                :arg => 0..1},
      {"FMLAL2" => {"U" => 1, "size" => 0, "opcode" => 0x19},               :arg => 2..3},
      {"BSL"    => {"U" => 1, "size" => 1, "opcode" => 0x3},                :arg => 0..1},
      {"BIT"    => {"U" => 1, "size" => 2, "opcode" => 0x3},                :arg => 0..1},
      {"FMLSL2" => {"U" => 1, "size" => 2, "opcode" => 0x19},               :arg => 2..3},
      {"BIF"    => {"U" => 1, "size" => 3, "opcode" => 0x3},                :arg => 0..1}
    ]
  },

  "AdvSimdModiImmMoviMvni" => {
    :cmt => "Advanced SIMD modified immediate",
    :arg => [
      [ {"vd" => "VReg8B"}, {"imm8" => "uint32_t"}, {"mod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}],                         #0
      [ {"vd" => "VReg16B"},{"imm8" => "uint32_t"}, {"mod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}],                         #1
      [ {"vd" => "VReg4H"}, {"imm8" => "uint32_t"}, {"mod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}],                         #2
      [ {"vd" => "VReg8H"}, {"imm8" => "uint32_t"}, {"mod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}],                         #3
      [ {"vd" => "VReg2S"}, {"imm8" => "uint32_t"}, {"mod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}], #4
      [ {"vd" => "VReg4S"}, {"imm8" => "uint32_t"}, {"mod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}], #5
      [ {"vd" => "DReg"},   {"imm" => "uint64_t"}],                                                  #6
      [ {"vd" => "VReg2D"}, {"imm" => "uint64_t"}]                                                   #7
    ],
    :prm => [
      ["op", "o2", "vd", "imm8", "mod", "sh"], # 0
      ["op", "o2", "vd", "imm"],               # 1
    ],
    :grp => [
      {"MOVI" => {"op" => 0, "o2" => 0}, :arg => [4,5],    :prm => 0},
      {"MOVI" => {"op" => 0, "o2" => 0}, :arg => [0,1,2,3],:prm => 0},
      {"MOVI" => {"op" => 1, "o2" => 0}, :arg => [6,7],    :prm => 1},
      {"MVNI" => {"op" => 1, "o2" => 0}, :arg => [4,5],    :prm => 0},
      {"MVNI" => {"op" => 1, "o2" => 0}, :arg => [2,3],    :prm => 0},
    ]
  },

  "AdvSimdModiImmOrrBic" => {
    :cmt => "Advanced SIMD modified immediate",
    :arg => [
      [ {"vd" => "VReg4H"}, {"imm" => "uint32_t"}, {"mod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}], #0
      [ {"vd" => "VReg8H"}, {"imm" => "uint32_t"}, {"mod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}], #1
      [ {"vd" => "VReg2S"}, {"imm" => "uint32_t"}, {"mod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}], #2
      [ {"vd" => "VReg4S"}, {"imm" => "uint32_t"}, {"mod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}]  #3
    ],
    :prm => ["op", "o2", "vd", "imm", "mod", "sh"],
    :grp => [
      {"ORR" => {"op" => 0, "o2" => 0,}},
      {"BIC" => {"op" => 1, "o2" => 0,}},
    ]
  },
  
  "AdvSimdModiImmFmov" => {
    :cmt => "Advanced SIMD modified immediate",
    :arg => [
      [ {"vd" => "VReg4H"}, {"imm" => "double"}], #0
      [ {"vd" => "VReg8H"}, {"imm" => "double"}], #1
      [ {"vd" => "VReg2S"}, {"imm" => "double"}], #2
      [ {"vd" => "VReg4S"}, {"imm" => "double"}], #3
      [ {"vd" => "VReg2D"}, {"imm" => "double"}]  #4
    ],
    :prm => ["op", "o2", "vd", "imm"],
    :grp => [
      {"FMOV" => {"op" => 0, "o2" => 0,}, :arg => [2,3]},
      {"FMOV" => {"op" => 0, "o2" => 1,}, :arg => [0,1]},
      {"FMOV" => {"op" => 1, "o2" => 0,}, :arg => [4]},
    ]
  },
  
  "AdvSimdShImm" => {
    :cmt => "Advanced SIMD shift by immediate",
    :arg => ext_args(vreg_set(2, "BHS"), [{"sh" => "uint32_t"}])  + #0-2 (0:8B, 1:4H, 2:2S)
            ext_args(qvreg_set(2, "BHSD"), [{"sh" => "uint32_t"}]) + #3-6 (3:16B, 4:8H, 5:4S, 6:2D)
            [
              [ {"vd" => "VReg8B"},  {"vn" => "VReg8H"}, {"sh" => "uint32_t"}], #7
              [ {"vd" => "VReg4H"},  {"vn" => "VReg4S"}, {"sh" => "uint32_t"}], #8
              [ {"vd" => "VReg2S"},  {"vn" => "VReg2D"}, {"sh" => "uint32_t"}], #9
              [ {"vd" => "VReg16B"}, {"vn" => "VReg8H"}, {"sh" => "uint32_t"}], #10
              [ {"vd" => "VReg8H"},  {"vn" => "VReg4S"}, {"sh" => "uint32_t"}], #11
              [ {"vd" => "VReg4S"},  {"vn" => "VReg2D"}, {"sh" => "uint32_t"}], #12

              [ {"vd" => "VReg8H"}, {"vn" => "VReg8B"},  {"sh" => "uint32_t"}], #13
              [ {"vd" => "VReg4S"}, {"vn" => "VReg4H"},  {"sh" => "uint32_t"}], #14
              [ {"vd" => "VReg2D"}, {"vn" => "VReg2S"},  {"sh" => "uint32_t"}], #15
              [ {"vd" => "VReg8H"}, {"vn" => "VReg16B"}, {"sh" => "uint32_t"}], #16
              [ {"vd" => "VReg4S"}, {"vn" => "VReg8H"},  {"sh" => "uint32_t"}], #17
              [ {"vd" => "VReg2D"}, {"vn" => "VReg4S"},  {"sh" => "uint32_t"}], #18

              [ {"vd" => "VReg8H"}, {"vn" => "VReg8B"}],                        #19
              [ {"vd" => "VReg4S"}, {"vn" => "VReg4H"}],                        #20
              [ {"vd" => "VReg2D"}, {"vn" => "VReg2S"}],                        #21
              [ {"vd" => "VReg8H"}, {"vn" => "VReg16B"}],                       #22
              [ {"vd" => "VReg4S"}, {"vn" => "VReg8H"}],                        #23
              [ {"vd" => "VReg2D"}, {"vn" => "VReg4S"}]                         #24
            ],
    # :arg => [[ {"vd" => "VReg"}, {"vn" => "VReg"}, {"sh" => "uint32_t"}]],
    :prm => ["U", "opcode", "vd", "vn", "sh"],
    :grp => [
      {"SSHR"      => {"U" => 0, "opcode" => 0x0},  :arg => 0..6},
      {"SSRA"      => {"U" => 0, "opcode" => 0x2},  :arg => 0..6},
      {"SRSHR"     => {"U" => 0, "opcode" => 0x4},  :arg => 0..6},
      {"SRSRA"     => {"U" => 0, "opcode" => 0x6},  :arg => 0..6},
      {"SHL"       => {"U" => 0, "opcode" => 0xa},  :arg => 0..6},
      {"SQSHL"     => {"U" => 0, "opcode" => 0xe},  :arg => 0..6},
      {"SHRN"      => {"U" => 0, "opcode" => 0x10}, :arg => 7..9},
      {"SHRN2"     => {"U" => 0, "opcode" => 0x10}, :arg => 10..12},
      {"RSHRN"     => {"U" => 0, "opcode" => 0x11}, :arg => 7..9},
      {"RSHRN2"    => {"U" => 0, "opcode" => 0x11}, :arg => 10..12},
      {"SQSHRN"    => {"U" => 0, "opcode" => 0x12}, :arg => 7..9},
      {"SQSHRN2"   => {"U" => 0, "opcode" => 0x12}, :arg => 10..12},
      {"SQRSHRN"   => {"U" => 0, "opcode" => 0x13}, :arg => 7..9},
      {"SQRSHRN2"  => {"U" => 0, "opcode" => 0x13}, :arg => 10..12},
      {"SSHLL"     => {"U" => 0, "opcode" => 0x14}, :arg => 13..15},
      {"SSHLL2"    => {"U" => 0, "opcode" => 0x14}, :arg => 16..18},
      {"SXTL"      => {"U" => 0, "opcode" => 0x14, "sh" => 0}, :arg => 19..21}, # alias of SSHLL
      {"SXTL2"     => {"U" => 0, "opcode" => 0x14, "sh" => 0}, :arg => 22..24}, # alias of SSHLL2
      {"SCVTF"     => {"U" => 0, "opcode" => 0x1c}, :arg => [1,2,4,5,6]},
      {"FCVTZS"    => {"U" => 0, "opcode" => 0x1f}, :arg => [1,2,4,5,6]},
      {"USHR"      => {"U" => 1, "opcode" => 0x0},  :arg => 0..6},
      {"USRA"      => {"U" => 1, "opcode" => 0x2},  :arg => 0..6},
      {"URSHR"     => {"U" => 1, "opcode" => 0x4},  :arg => 0..6},
      {"URSRA"     => {"U" => 1, "opcode" => 0x6},  :arg => 0..6},
      {"SRI"       => {"U" => 1, "opcode" => 0x8},  :arg => 0..6},
      {"SLI"       => {"U" => 1, "opcode" => 0xa},  :arg => 0..6},
      {"SQSHLU"    => {"U" => 1, "opcode" => 0xc},  :arg => 0..6},
      {"UQSHL"     => {"U" => 1, "opcode" => 0xe},  :arg => 0..6},
      {"SQSHRUN"   => {"U" => 1, "opcode" => 0x10}, :arg => 7..9},
      {"SQSHRUN2"  => {"U" => 1, "opcode" => 0x10}, :arg => 10..12},
      {"SQRSHRUN"  => {"U" => 1, "opcode" => 0x11}, :arg => 7..9},
      {"SQRSHRUN2" => {"U" => 1, "opcode" => 0x11}, :arg => 10..12},
      {"UQSHRN"    => {"U" => 1, "opcode" => 0x12}, :arg => 7..9},
      {"UQSHRN2"   => {"U" => 1, "opcode" => 0x12}, :arg => 10..12},
      {"UQRSHRN"   => {"U" => 1, "opcode" => 0x13}, :arg => 7..9},
      {"UQRSHRN2"  => {"U" => 1, "opcode" => 0x13}, :arg => 10..12},
      {"USHLL"     => {"U" => 1, "opcode" => 0x14}, :arg => 13..15},
      {"USHLL2"    => {"U" => 1, "opcode" => 0x14}, :arg => 16..18},
      {"UXTL"      => {"U" => 1, "opcode" => 0x14, "sh" => 0}, :arg => 19..21}, # alias of USHLL
      {"UXTL2"     => {"U" => 1, "opcode" => 0x14, "sh" => 0}, :arg => 22..24}, # alias of USHLL2
      {"UCVTF"     => {"U" => 1, "opcode" => 0x1c}, :arg => [1,2,4,5,6]},
      {"FCVTZU"    => {"U" => 1, "opcode" => 0x1f}, :arg => [1,2,4,5,6]}
    ]
  },

  "AdvSimdVecXindElem" => {
    :cmt => "Advanced SIMD vector x indexed element",
    :arg => [
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4H"}, {"vm" => "VRegHElem"}], #0
      [ {"vd" => "VReg2D"}, {"vn" => "VReg2S"}, {"vm" => "VRegSElem"}], #1
      [ {"vd" => "VReg4S"}, {"vn" => "VReg8H"}, {"vm" => "VRegHElem"}], #2
      [ {"vd" => "VReg2D"}, {"vn" => "VReg4S"}, {"vm" => "VRegSElem"}], #3

      [ {"vd" => "VReg4H"}, {"vn" => "VReg4H"}, {"vm" => "VRegHElem"}], #4
      [ {"vd" => "VReg2S"}, {"vn" => "VReg2S"}, {"vm" => "VRegSElem"}], #5
      [ {"vd" => "VReg8H"}, {"vn" => "VReg8H"}, {"vm" => "VRegHElem"}], #6
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}, {"vm" => "VRegSElem"}], #7
      [ {"vd" => "VReg2D"}, {"vn" => "VReg2D"}, {"vm" => "VRegDElem"}], #8

      [ {"vd" => "VReg2S"}, {"vn" => "VReg8B"},  {"vm" => "VRegBElem"}], #9
      [ {"vd" => "VReg4S"}, {"vn" => "VReg16B"}, {"vm" => "VRegBElem"}], #10

      [ {"vd" => "VReg4H"}, {"vn" => "VReg4H"}, {"vm" => "VRegHElem"}, {"rotate" => "uint32_t"}], #11
      [ {"vd" => "VReg8H"}, {"vn" => "VReg8H"}, {"vm" => "VRegHElem"}, {"rotate" => "uint32_t"}], #12
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}, {"vm" => "VRegSElem"}, {"rotate" => "uint32_t"}]  #13
    ],
    :prm => [
      ["U", "opcode", "vd", "vn", "vm"],           #0
      ["U", "opcode", "vd", "vn", "vm", "rotate"]  #1
    ],
    :grp => [
      {"SMLAL"      => {"U" => 0, "opcode" => 0x2}, :arg => 0..1,   :prm => 0},
      {"SMLAL2"     => {"U" => 0, "opcode" => 0x2}, :arg => 2..3,   :prm => 0},
      {"SQDMLAL"    => {"U" => 0, "opcode" => 0x3}, :arg => 0..1,   :prm => 0},
      {"SQDMLAL2"   => {"U" => 0, "opcode" => 0x3}, :arg => 2..3,   :prm => 0},
      {"SMLSL"      => {"U" => 0, "opcode" => 0x6}, :arg => 0..1,   :prm => 0},
      {"SMLSL2"     => {"U" => 0, "opcode" => 0x6}, :arg => 2..3,   :prm => 0},
      {"SQDMLSL"    => {"U" => 0, "opcode" => 0x7}, :arg => 0..1,   :prm => 0},
      {"SQDMLSL2"   => {"U" => 0, "opcode" => 0x7}, :arg => 2..3,   :prm => 0},
      {"MUL"        => {"U" => 0, "opcode" => 0x8}, :arg => 4..7,   :prm => 0},
      {"SMULL"      => {"U" => 0, "opcode" => 0xa}, :arg => 0..1,   :prm => 0},
      {"SMULL2"     => {"U" => 0, "opcode" => 0xa}, :arg => 2..3,   :prm => 0},
      {"SQDMULL"    => {"U" => 0, "opcode" => 0xb}, :arg => 0..1,   :prm => 0},
      {"SQDMULL2"   => {"U" => 0, "opcode" => 0xb}, :arg => 2..3,   :prm => 0},
      {"SQDMULH"    => {"U" => 0, "opcode" => 0xc}, :arg => 4..7,   :prm => 0},
      {"SQRDMULH"   => {"U" => 0, "opcode" => 0xd}, :arg => 4..7,   :prm => 0},
      {"SDOT"       => {"U" => 0, "opcode" => 0xe}, :arg => 9..10,  :prm => 0},
      {"MLA"        => {"U" => 1, "opcode" => 0x0}, :arg => 4..7,   :prm => 0},
      {"UMLAL"      => {"U" => 1, "opcode" => 0x2}, :arg => 0..1,   :prm => 0},
      {"UMLAL2"     => {"U" => 1, "opcode" => 0x2}, :arg => 2..3,   :prm => 0},
      {"MLS"        => {"U" => 1, "opcode" => 0x4}, :arg => 4..7,   :prm => 0},
      {"UMLSL"      => {"U" => 1, "opcode" => 0x6}, :arg => 0..1,   :prm => 0},
      {"UMLSL2"     => {"U" => 1, "opcode" => 0x6}, :arg => 2..3,   :prm => 0},
      {"UMULL"      => {"U" => 1, "opcode" => 0xa}, :arg => 0..1,   :prm => 0},
      {"UMULL2"     => {"U" => 1, "opcode" => 0xa}, :arg => 2..3,   :prm => 0},
      {"SQRDMLAH"   => {"U" => 1, "opcode" => 0xd}, :arg => 4..7,   :prm => 0},
      {"UDOT"       => {"U" => 1, "opcode" => 0xe}, :arg => 9..10,  :prm => 0},
      {"SQRDMLSH"   => {"U" => 1, "opcode" => 0xf}, :arg => 4..7,   :prm => 0},
      {"FCMLA"      => {"U" => 1, "opcode" => 0x1}, :arg => 11..13, :prm => 1},
    ]
  },


  "AdvSimdVecXindElemSz" => {
    :cmt => "Advanced SIMD vector x indexed element",
    :arg => [
      [ {"vd" => "VReg4H"}, {"vn" => "VReg4H"}, {"vm" => "VRegHElem"}], #0
      [ {"vd" => "VReg2S"}, {"vn" => "VReg2S"}, {"vm" => "VRegSElem"}], #1
      [ {"vd" => "VReg8H"}, {"vn" => "VReg8H"}, {"vm" => "VRegHElem"}], #2
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}, {"vm" => "VRegSElem"}], #3
      [ {"vd" => "VReg2D"}, {"vn" => "VReg2D"}, {"vm" => "VRegDElem"}], #4
      [ {"vd" => "VReg2S"}, {"vn" => "VReg2H"},  {"vm" => "VRegHElem"}], #5
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4H"},  {"vm" => "VRegHElem"}], #6
    ],
    :prm => ["U", "size", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"FMLA"       => {"U" => 0, "size" => 0, "opcode" => 0x1}, :arg => [0,2]},
      {"FMLS"       => {"U" => 0, "size" => 0, "opcode" => 0x5}, :arg => [0,2]},
      {"FMUL"       => {"U" => 0, "size" => 0, "opcode" => 0x9}, :arg => [0,2]},
      {"FMLA"       => {"U" => 0, "size" => 2, "opcode" => 0x1}, :arg => [1,3]},
      {"FMLS"       => {"U" => 0, "size" => 2, "opcode" => 0x5}, :arg => [1,3]},
      {"FMUL"       => {"U" => 0, "size" => 2, "opcode" => 0x9}, :arg => [1,3]},
      {"FMLA"       => {"U" => 0, "size" => 3, "opcode" => 0x1}, :arg => [4]},
      {"FMLS"       => {"U" => 0, "size" => 3, "opcode" => 0x5}, :arg => [4]},
      {"FMUL"       => {"U" => 0, "size" => 3, "opcode" => 0x9}, :arg => [4]},
      {"FMLAL"      => {"U" => 0, "size" => 2, "opcode" => 0x0}, :arg => 5..6},
      {"FMLSL"      => {"U" => 0, "size" => 2, "opcode" => 0x4}, :arg => 5..6},
      {"FMULX"      => {"U" => 1, "size" => 0, "opcode" => 0x9}, :arg => [0,2]},
      {"FMULX"      => {"U" => 1, "size" => 2, "opcode" => 0x9}, :arg => [1,3]},
      {"FMULX"      => {"U" => 1, "size" => 3, "opcode" => 0x9}, :arg => [4]},
      {"FMLAL2"     => {"U" => 1, "size" => 2, "opcode" => 0x8}, :arg => 5..6},
      {"FMLSL2"     => {"U" => 1, "size" => 2, "opcode" => 0xc}, :arg => 5..6}
    ]
  },

  "Crypto3RegImm2" => {
    :cmt => "Cryptographic three-register, imm2",
    :arg => [[ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}, {"vm" => "VRegSElem"}]],
    :prm => ["opcode", "vd", "vn", "vm"],
    :grp => [
      {"SM3TT1A" => {"opcode" => 0x0}},
      {"SM3TT1B" => {"opcode" => 0x1}},
      {"SM3TT2A" => {"opcode" => 0x2}},
      {"SM3TT2B" => {"opcode" => 0x3}}
    ]
  },
  
  "Crypto3RegSHA512" => {
    :cmt => "Cryptographic three-register SHA 512",
    :arg => [
      [ {"vd" => "QReg"},   {"vn" => "QReg"},   {"vm" => "VReg2D"}], #0
      [ {"vd" => "VReg2D"}, {"vn" => "VReg2D"}, {"vm" => "VReg2D"}], #1
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}, {"vm" => "VReg4S"}], #2
    ],
    :prm => ["O", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"SHA512H"   => {"O" => 0, "opcode" => 0x0}, :arg => [0]},
      {"SHA512H2"  => {"O" => 0, "opcode" => 0x1}, :arg => [0]},
      {"SHA512SU1" => {"O" => 0, "opcode" => 0x2}, :arg => [1]},
      {"RAX1"      => {"O" => 0, "opcode" => 0x3}, :arg => [1]},
      {"SM3PARTW1" => {"O" => 1, "opcode" => 0x0}, :arg => [2]},
      {"SM3PARTW2" => {"O" => 1, "opcode" => 0x1}, :arg => [2]},
      {"SM4EKEY"   => {"O" => 1, "opcode" => 0x2}, :arg => [2]}
    ]
  },

  "CryptoSHA" => {
    :cmt => "Cryptographic SHA",
    :arg => [
      [ {"vd" => "VReg2D"},  {"vn" => "VReg2D"},  {"vm" => "VReg2D"},  {"imm6" => "uint32_t"}]
    ],
    :prm => ["vd", "vn", "vm", "imm6"],
    :grp => [
      {"XAR" => {}},
    ]
  },
  
  "Crypto4Reg" => {
    :cmt => "Cryptographic four-register",
    :arg => [
      [ {"vd" => "VReg16B"}, {"vn" => "VReg16B"}, {"vm" => "VReg16B"}, {"va" => "VReg16B"}], #0
      [ {"vd" => "VReg4S"},  {"vn" => "VReg4S"},  {"vm" => "VReg4S"},  {"va" => "VReg4S"}]   #1
    ],
    :prm => ["Op0", "vd", "vn", "vm", "va"],
    :grp => [
      {"EOR3"   => {"Op0" => 0}, :arg => [0]},
      {"BCAX"   => {"Op0" => 1}, :arg => [0]},
      {"SM3SS1" => {"Op0" => 2}, :arg => [1]}
    ]
  },

  "Crypto2RegSHA512" => {
    :cmt => "Cryptographic two-register SHA512",
    :arg => [
      [ {"vd" => "VReg2D"}, {"vn" => "VReg2D"}], #0
      [ {"vd" => "VReg4S"}, {"vn" => "VReg4S"}]  #1
    ],
    :prm => ["opcode", "vd", "vn"],
    :grp => [
      {"SHA512SU0" => {"opcode" => 0}, :arg => [0]},
      {"SM4E"      => {"opcode" => 1}, :arg => [1]}
    ]
  },

  "ConversionFpFix" => {
    :cmt => "conversion between floating-point and fixed-point",
    :arg => [
      [ {"d" => "SReg"}, {"n" => "WReg"}, {"fbits" => "uint32_t"}], #0
      [ {"d" => "WReg"}, {"n" => "SReg"}, {"fbits" => "uint32_t"}], #1
      [ {"d" => "DReg"}, {"n" => "WReg"}, {"fbits" => "uint32_t"}], #2
      [ {"d" => "WReg"}, {"n" => "DReg"}, {"fbits" => "uint32_t"}], #3
      [ {"d" => "HReg"}, {"n" => "WReg"}, {"fbits" => "uint32_t"}], #4
      [ {"d" => "WReg"}, {"n" => "HReg"}, {"fbits" => "uint32_t"}], #5
      [ {"d" => "SReg"}, {"n" => "XReg"}, {"fbits" => "uint32_t"}], #6
      [ {"d" => "XReg"}, {"n" => "SReg"}, {"fbits" => "uint32_t"}], #7
      [ {"d" => "DReg"}, {"n" => "XReg"}, {"fbits" => "uint32_t"}], #8
      [ {"d" => "XReg"}, {"n" => "DReg"}, {"fbits" => "uint32_t"}], #9
      [ {"d" => "HReg"}, {"n" => "XReg"}, {"fbits" => "uint32_t"}], #10
      [ {"d" => "XReg"}, {"n" => "HReg"}, {"fbits" => "uint32_t"}], #11
    ],
    :prm => ["S", "type", "rmode", "opcode", "d", "n", "fbits"],
    :grp => [
      {"SCVTF"  => {"S" => 0, "type" => 0, "rmode" => 0, "opcode" => 2}, :arg => [0,6]},
      {"UCVTF"  => {"S" => 0, "type" => 0, "rmode" => 0, "opcode" => 3}, :arg => [0,6]},
      {"FCVTZS" => {"S" => 0, "type" => 0, "rmode" => 3, "opcode" => 0}, :arg => [1,7]},
      {"FCVTZU" => {"S" => 0, "type" => 0, "rmode" => 3, "opcode" => 1}, :arg => [1,7]},
      {"SCVTF"  => {"S" => 0, "type" => 1, "rmode" => 0, "opcode" => 2}, :arg => [2,8]},
      {"UCVTF"  => {"S" => 0, "type" => 1, "rmode" => 0, "opcode" => 3}, :arg => [2,8]},
      {"FCVTZS" => {"S" => 0, "type" => 1, "rmode" => 3, "opcode" => 0}, :arg => [3,9]},
      {"FCVTZU" => {"S" => 0, "type" => 1, "rmode" => 3, "opcode" => 1}, :arg => [3,9]},
      {"SCVTF"  => {"S" => 0, "type" => 3, "rmode" => 0, "opcode" => 2}, :arg => [4,10]},
      {"UCVTF"  => {"S" => 0, "type" => 3, "rmode" => 0, "opcode" => 3}, :arg => [4,10]},
      {"FCVTZS" => {"S" => 0, "type" => 3, "rmode" => 3, "opcode" => 0}, :arg => [5,11]},
      {"FCVTZU" => {"S" => 0, "type" => 3, "rmode" => 3, "opcode" => 1}, :arg => [5,11]}
    ]
  },

  "ConversionFpInt" => {
    :cmt => "conversion between floating-point and integer",
    :arg => [
      [ {"d" => "WReg"},       {"n" => "SReg"}],       #0
      [ {"d" => "SReg"},       {"n" => "WReg"}],       #1
      [ {"d" => "WReg"},       {"n" => "DReg"}],       #2
      [ {"d" => "DReg"},       {"n" => "WReg"}],       #3
      [ {"d" => "WReg"},       {"n" => "HReg"}],       #4
      [ {"d" => "HReg"},       {"n" => "WReg"}],       #5
      [ {"d" => "XReg"},       {"n" => "SReg"}],       #6
      [ {"d" => "SReg"},       {"n" => "XReg"}],       #7
      [ {"d" => "XReg"},       {"n" => "DReg"}],       #8
      [ {"d" => "DReg"},       {"n" => "XReg"}],       #9
      [ {"d" => "XReg"},       {"n" => "VRegDElem"}],  #10
      [ {"d" => "VRegDElem"},  {"n" => "XReg"}],       #11
      [ {"d" => "XReg"},       {"n" => "HReg"}],       #12
      [ {"d" => "HReg"},       {"n" => "XReg"}],       #13
    ],
    :prm => ["sf", "S", "type", "rmode", "opcode", "d", "n"],
    :grp => [
      {"FCVTNS" => {"sf" => 0, "S" => 0, "type" => 0, "rmode" => 0, "opcode" => 0}, :arg => [0]},
      {"FCVTNU" => {"sf" => 0, "S" => 0, "type" => 0, "rmode" => 0, "opcode" => 1}, :arg => [0]},
      {"FCVTAS" => {"sf" => 0, "S" => 0, "type" => 0, "rmode" => 0, "opcode" => 4}, :arg => [0]},
      {"FCVTAU" => {"sf" => 0, "S" => 0, "type" => 0, "rmode" => 0, "opcode" => 5}, :arg => [0]},
      {"FMOV"   => {"sf" => 0, "S" => 0, "type" => 0, "rmode" => 0, "opcode" => 6}, :arg => [0]},
      {"FCVTPS" => {"sf" => 0, "S" => 0, "type" => 0, "rmode" => 1, "opcode" => 0}, :arg => [0]},
      {"FCVTPU" => {"sf" => 0, "S" => 0, "type" => 0, "rmode" => 1, "opcode" => 1}, :arg => [0]},
      {"FCVTMS" => {"sf" => 0, "S" => 0, "type" => 0, "rmode" => 2, "opcode" => 0}, :arg => [0]},
      {"FCVTMU" => {"sf" => 0, "S" => 0, "type" => 0, "rmode" => 2, "opcode" => 1}, :arg => [0]},
      {"FCVTZS" => {"sf" => 0, "S" => 0, "type" => 0, "rmode" => 3, "opcode" => 0}, :arg => [0]},
      {"FCVTZU" => {"sf" => 0, "S" => 0, "type" => 0, "rmode" => 3, "opcode" => 1}, :arg => [0]},
      {"SCVTF"  => {"sf" => 0, "S" => 0, "type" => 0, "rmode" => 0, "opcode" => 2}, :arg => [1]},
      {"UCVTF"  => {"sf" => 0, "S" => 0, "type" => 0, "rmode" => 0, "opcode" => 3}, :arg => [1]},
      {"FMOV"   => {"sf" => 0, "S" => 0, "type" => 0, "rmode" => 0, "opcode" => 7}, :arg => [1]},
      {"FCVTNS" => {"sf" => 0, "S" => 0, "type" => 1, "rmode" => 0, "opcode" => 0}, :arg => [2]},
      {"FCVTNU" => {"sf" => 0, "S" => 0, "type" => 1, "rmode" => 0, "opcode" => 1}, :arg => [2]},
      {"FCVTAS" => {"sf" => 0, "S" => 0, "type" => 1, "rmode" => 0, "opcode" => 4}, :arg => [2]},
      {"FCVTAU" => {"sf" => 0, "S" => 0, "type" => 1, "rmode" => 0, "opcode" => 5}, :arg => [2]},
      {"FCVTPS" => {"sf" => 0, "S" => 0, "type" => 1, "rmode" => 1, "opcode" => 0}, :arg => [2]},
      {"FCVTPU" => {"sf" => 0, "S" => 0, "type" => 1, "rmode" => 1, "opcode" => 1}, :arg => [2]},
      {"FCVTMS" => {"sf" => 0, "S" => 0, "type" => 1, "rmode" => 2, "opcode" => 0}, :arg => [2]},
      {"FCVTMU" => {"sf" => 0, "S" => 0, "type" => 1, "rmode" => 2, "opcode" => 1}, :arg => [2]},
      {"FCVTZS" => {"sf" => 0, "S" => 0, "type" => 1, "rmode" => 3, "opcode" => 0}, :arg => [2]},
      {"FCVTZU" => {"sf" => 0, "S" => 0, "type" => 1, "rmode" => 3, "opcode" => 1}, :arg => [2]},
      {"FJCVTZS"=> {"sf" => 0, "S" => 0, "type" => 1, "rmode" => 3, "opcode" => 6}, :arg => [2]},
      {"SCVTF"  => {"sf" => 0, "S" => 0, "type" => 1, "rmode" => 0, "opcode" => 2}, :arg => [3]},
      {"UCVTF"  => {"sf" => 0, "S" => 0, "type" => 1, "rmode" => 0, "opcode" => 3}, :arg => [3]},
      {"FCVTNS" => {"sf" => 0, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 0}, :arg => [4]},
      {"FCVTNU" => {"sf" => 0, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 1}, :arg => [4]},
      {"FCVTAS" => {"sf" => 0, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 4}, :arg => [4]},
      {"FCVTAU" => {"sf" => 0, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 5}, :arg => [4]},
      {"FMOV"   => {"sf" => 0, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 6}, :arg => [4]},
      {"FCVTPS" => {"sf" => 0, "S" => 0, "type" => 3, "rmode" => 1, "opcode" => 0}, :arg => [4]},
      {"FCVTPU" => {"sf" => 0, "S" => 0, "type" => 3, "rmode" => 1, "opcode" => 1}, :arg => [4]},
      {"FCVTMS" => {"sf" => 0, "S" => 0, "type" => 3, "rmode" => 2, "opcode" => 0}, :arg => [4]},
      {"FCVTMU" => {"sf" => 0, "S" => 0, "type" => 3, "rmode" => 2, "opcode" => 1}, :arg => [4]},
      {"FCVTZS" => {"sf" => 0, "S" => 0, "type" => 3, "rmode" => 3, "opcode" => 0}, :arg => [4]},
      {"FCVTZU" => {"sf" => 0, "S" => 0, "type" => 3, "rmode" => 3, "opcode" => 1}, :arg => [4]},
      {"SCVTF"  => {"sf" => 0, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 2}, :arg => [5]},
      {"UCVTF"  => {"sf" => 0, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 3}, :arg => [5]},
      {"FMOV"   => {"sf" => 0, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 7}, :arg => [5]},
      {"FCVTNS" => {"sf" => 1, "S" => 0, "type" => 0, "rmode" => 0, "opcode" => 0}, :arg => [6]},
      {"FCVTNU" => {"sf" => 1, "S" => 0, "type" => 0, "rmode" => 0, "opcode" => 1}, :arg => [6]},
      {"FCVTAS" => {"sf" => 1, "S" => 0, "type" => 0, "rmode" => 0, "opcode" => 4}, :arg => [6]},
      {"FCVTAU" => {"sf" => 1, "S" => 0, "type" => 0, "rmode" => 0, "opcode" => 5}, :arg => [6]},
      {"FCVTPS" => {"sf" => 1, "S" => 0, "type" => 0, "rmode" => 1, "opcode" => 0}, :arg => [6]},
      {"FCVTPU" => {"sf" => 1, "S" => 0, "type" => 0, "rmode" => 1, "opcode" => 1}, :arg => [6]},
      {"FCVTMS" => {"sf" => 1, "S" => 0, "type" => 0, "rmode" => 2, "opcode" => 0}, :arg => [6]},
      {"FCVTMU" => {"sf" => 1, "S" => 0, "type" => 0, "rmode" => 2, "opcode" => 1}, :arg => [6]},
      {"FCVTZS" => {"sf" => 1, "S" => 0, "type" => 0, "rmode" => 3, "opcode" => 0}, :arg => [6]},
      {"FCVTZU" => {"sf" => 1, "S" => 0, "type" => 0, "rmode" => 3, "opcode" => 1}, :arg => [6]},
      {"SCVTF"  => {"sf" => 1, "S" => 0, "type" => 0, "rmode" => 0, "opcode" => 2}, :arg => [7]},
      {"UCVTF"  => {"sf" => 1, "S" => 0, "type" => 0, "rmode" => 0, "opcode" => 3}, :arg => [7]},
      {"FCVTNS" => {"sf" => 1, "S" => 0, "type" => 1, "rmode" => 0, "opcode" => 0}, :arg => [8]},
      {"FCVTNU" => {"sf" => 1, "S" => 0, "type" => 1, "rmode" => 0, "opcode" => 1}, :arg => [8]},
      {"FCVTAS" => {"sf" => 1, "S" => 0, "type" => 1, "rmode" => 0, "opcode" => 4}, :arg => [8]},
      {"FCVTAU" => {"sf" => 1, "S" => 0, "type" => 1, "rmode" => 0, "opcode" => 5}, :arg => [8]},
      {"FMOV"   => {"sf" => 1, "S" => 0, "type" => 1, "rmode" => 0, "opcode" => 6}, :arg => [8]},
      {"FCVTPS" => {"sf" => 1, "S" => 0, "type" => 1, "rmode" => 1, "opcode" => 0}, :arg => [8]},
      {"FCVTPU" => {"sf" => 1, "S" => 0, "type" => 1, "rmode" => 1, "opcode" => 1}, :arg => [8]},
      {"FCVTMS" => {"sf" => 1, "S" => 0, "type" => 1, "rmode" => 2, "opcode" => 0}, :arg => [8]},
      {"FCVTMU" => {"sf" => 1, "S" => 0, "type" => 1, "rmode" => 2, "opcode" => 1}, :arg => [8]},
      {"FCVTZS" => {"sf" => 1, "S" => 0, "type" => 1, "rmode" => 3, "opcode" => 0}, :arg => [8]},
      {"FCVTZU" => {"sf" => 1, "S" => 0, "type" => 1, "rmode" => 3, "opcode" => 1}, :arg => [8]},
      {"SCVTF"  => {"sf" => 1, "S" => 0, "type" => 1, "rmode" => 0, "opcode" => 2}, :arg => [9]},
      {"UCVTF"  => {"sf" => 1, "S" => 0, "type" => 1, "rmode" => 0, "opcode" => 3}, :arg => [9]},
      {"FMOV"   => {"sf" => 1, "S" => 0, "type" => 1, "rmode" => 0, "opcode" => 7}, :arg => [9]},
      {"FMOV"   => {"sf" => 1, "S" => 0, "type" => 2, "rmode" => 1, "opcode" => 6}, :arg => [10]},
      {"FMOV"   => {"sf" => 1, "S" => 0, "type" => 2, "rmode" => 1, "opcode" => 7}, :arg => [11]},
      {"FCVTNS" => {"sf" => 1, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 0}, :arg => [12]},
      {"FCVTNU" => {"sf" => 1, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 1}, :arg => [12]},
      {"FCVTAS" => {"sf" => 1, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 4}, :arg => [12]},
      {"FCVTAU" => {"sf" => 1, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 5}, :arg => [12]},
      {"FMOV"   => {"sf" => 1, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 6}, :arg => [12]},
      {"FCVTPS" => {"sf" => 1, "S" => 0, "type" => 3, "rmode" => 1, "opcode" => 0}, :arg => [12]},
      {"FCVTPU" => {"sf" => 1, "S" => 0, "type" => 3, "rmode" => 1, "opcode" => 1}, :arg => [12]},
      {"FCVTMS" => {"sf" => 1, "S" => 0, "type" => 3, "rmode" => 2, "opcode" => 0}, :arg => [12]},
      {"FCVTMU" => {"sf" => 1, "S" => 0, "type" => 3, "rmode" => 2, "opcode" => 1}, :arg => [12]},
      {"FCVTZS" => {"sf" => 1, "S" => 0, "type" => 3, "rmode" => 3, "opcode" => 0}, :arg => [12]},
      {"FCVTZU" => {"sf" => 1, "S" => 0, "type" => 3, "rmode" => 3, "opcode" => 1}, :arg => [12]},
      {"SCVTF"  => {"sf" => 1, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 2}, :arg => [13]},
      {"UCVTF"  => {"sf" => 1, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 3}, :arg => [13]},
      {"FMOV"   => {"sf" => 1, "S" => 0, "type" => 3, "rmode" => 0, "opcode" => 7}, :arg => [13]}
    ]
  },

  "FpDataProc1Reg" => {
    :cmt => "Floating-piont data-processing (1 source)",
    :arg => [
      [ {"vd" => "SReg"}, {"vn" => "SReg"}], #0
      [ {"vd" => "DReg"}, {"vn" => "SReg"}], #1
      [ {"vd" => "HReg"}, {"vn" => "SReg"}], #2
      [ {"vd" => "DReg"}, {"vn" => "DReg"}], #3
      [ {"vd" => "SReg"}, {"vn" => "DReg"}], #4
      [ {"vd" => "HReg"}, {"vn" => "DReg"}], #5
      [ {"vd" => "HReg"}, {"vn" => "HReg"}], #6
      [ {"vd" => "SReg"}, {"vn" => "HReg"}], #7
      [ {"vd" => "DReg"}, {"vn" => "HReg"}]  #8
    ],
    :prm => ["M", "S", "type", "opcode", "vd", "vn"],
    :grp => [
      {"FMOV"   => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 0x0}, :arg => [0]},
      {"FABS"   => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 0x1}, :arg => [0]},
      {"FNEG"   => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 0x2}, :arg => [0]},
      {"FSQRT"  => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 0x3}, :arg => [0]},
      {"FRINTN" => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 0x8}, :arg => [0]},
      {"FRINTP" => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 0x9}, :arg => [0]},
      {"FRINTM" => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 0xa}, :arg => [0]},
      {"FRINTZ" => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 0xb}, :arg => [0]},
      {"FRINTA" => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 0xc}, :arg => [0]},
      {"FRINTX" => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 0xe}, :arg => [0]},
      {"FRINTI" => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 0xf}, :arg => [0]},
      {"FCVT"   => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 0x5}, :arg => [1]},
      {"FCVT"   => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 0x7}, :arg => [2]},
      {"FMOV"   => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 0x0}, :arg => [3]},
      {"FABS"   => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 0x1}, :arg => [3]},
      {"FNEG"   => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 0x2}, :arg => [3]},
      {"FSQRT"  => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 0x3}, :arg => [3]},
      {"FRINTN" => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 0x8}, :arg => [3]},
      {"FRINTP" => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 0x9}, :arg => [3]},
      {"FRINTM" => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 0xa}, :arg => [3]},
      {"FRINTZ" => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 0xb}, :arg => [3]},
      {"FRINTA" => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 0xc}, :arg => [3]},
      {"FRINTX" => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 0xe}, :arg => [3]},
      {"FRINTI" => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 0xf}, :arg => [3]},
      {"FCVT"   => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 0x4}, :arg => [4]},
      {"FCVT"   => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 0x7}, :arg => [5]},
      {"FMOV"   => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 0x0}, :arg => [6]},
      {"FABS"   => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 0x1}, :arg => [6]},
      {"FNEG"   => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 0x2}, :arg => [6]},
      {"FSQRT"  => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 0x3}, :arg => [6]},
      {"FRINTN" => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 0x8}, :arg => [6]},
      {"FRINTP" => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 0x9}, :arg => [6]},
      {"FRINTM" => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 0xa}, :arg => [6]},
      {"FRINTZ" => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 0xb}, :arg => [6]},
      {"FRINTA" => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 0xc}, :arg => [6]},
      {"FRINTX" => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 0xe}, :arg => [6]},
      {"FRINTI" => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 0xf}, :arg => [6]},
      {"FCVT"   => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 0x4}, :arg => [7]},
      {"FCVT"   => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 0x5}, :arg => [8]}
    ]
  },

  "FpComp" => {
    :cmt => "Floating-piont compare",
    :arg => [
      [ {"vn" => "SReg"}, {"vm"  => "SReg"}],   #0
      [ {"vn" => "SReg"}, {"imm" => "double"}], #1
      [ {"vn" => "DReg"}, {"vm"  => "DReg"}],   #2
      [ {"vn" => "DReg"}, {"imm" => "double"}], #3
      [ {"vn" => "HReg"}, {"vm"  => "HReg"}],   #4
      [ {"vn" => "HReg"}, {"imm" => "double"}], #5
    ],
    :prm => [
      ["M", "S", "type", "op", "opcode2", "vn", "vm"], #0
      ["M", "S", "type", "op", "opcode2", "vn", "imm"] #1
    ],
    :grp => [
      {"FCMP"   => {"M" => 0, "S" => 0, "type" => 0, "op" => 0, "opcode2" => 0x0},  :arg => [0], :prm => 0},
      {"FCMPE"  => {"M" => 0, "S" => 0, "type" => 0, "op" => 0, "opcode2" => 0x10}, :arg => [0], :prm => 0},
      {"FCMP"   => {"M" => 0, "S" => 0, "type" => 0, "op" => 0, "opcode2" => 0x8},  :arg => [1], :prm => 1},
      {"FCMPE"  => {"M" => 0, "S" => 0, "type" => 0, "op" => 0, "opcode2" => 0x18}, :arg => [1], :prm => 1},
      {"FCMP"   => {"M" => 0, "S" => 0, "type" => 1, "op" => 0, "opcode2" => 0x0},  :arg => [2], :prm => 0},
      {"FCMPE"  => {"M" => 0, "S" => 0, "type" => 1, "op" => 0, "opcode2" => 0x10}, :arg => [2], :prm => 0},
      {"FCMP"   => {"M" => 0, "S" => 0, "type" => 1, "op" => 0, "opcode2" => 0x8},  :arg => [3], :prm => 1},
      {"FCMPE"  => {"M" => 0, "S" => 0, "type" => 1, "op" => 0, "opcode2" => 0x18}, :arg => [3], :prm => 1},
      {"FCMP"   => {"M" => 0, "S" => 0, "type" => 3, "op" => 0, "opcode2" => 0x0},  :arg => [4], :prm => 0},
      {"FCMPE"  => {"M" => 0, "S" => 0, "type" => 3, "op" => 0, "opcode2" => 0x10}, :arg => [4], :prm => 0},
      {"FCMP"   => {"M" => 0, "S" => 0, "type" => 3, "op" => 0, "opcode2" => 0x8},  :arg => [5], :prm => 1},
      {"FCMPE"  => {"M" => 0, "S" => 0, "type" => 3, "op" => 0, "opcode2" => 0x18}, :arg => [5], :prm => 1}
    ]
  },

  "FpImm" => {
    :cmt => "Floating-piont immediate",
    :arg => [
      [ {"vd" => "SReg"}, {"imm" => "double"}], #0
      [ {"vd" => "DReg"}, {"imm" => "double"}], #1
      [ {"vd" => "HReg"}, {"imm" => "double"}]  #2
    ],
    :prm => ["M", "S", "type", "vd", "imm"],
    :grp => [
      {"FMOV"   => {"M" => 0, "S" => 0, "type" => 0}, :arg => [0]},
      {"FMOV"   => {"M" => 0, "S" => 0, "type" => 1}, :arg => [1]},
      {"FMOV"   => {"M" => 0, "S" => 0, "type" => 3}, :arg => [2]}
    ]
  },

  "FpCondComp" => {
    :cmt => "Floating-piont conditional compare",
    :arg => [
      [ {"vn" => "SReg"}, {"vm" => "SReg"}, {"nzcv" => "uint32_t"}, {"cond" => "Cond"}], #0
      [ {"vn" => "DReg"}, {"vm" => "DReg"}, {"nzcv" => "uint32_t"}, {"cond" => "Cond"}], #1
      [ {"vn" => "HReg"}, {"vm" => "HReg"}, {"nzcv" => "uint32_t"}, {"cond" => "Cond"}]  #2
    ],
    :prm => ["M", "S", "type", "op", "vn", "vm", "nzcv", "cond"],
    :grp => [
      {"FCCMP"   => {"M" => 0, "S" => 0, "type" => 0, "op" => 0}, :arg => [0]},
      {"FCCMPE"  => {"M" => 0, "S" => 0, "type" => 0, "op" => 1}, :arg => [0]},
      {"FCCMP"   => {"M" => 0, "S" => 0, "type" => 1, "op" => 0}, :arg => [1]},
      {"FCCMPE"  => {"M" => 0, "S" => 0, "type" => 1, "op" => 1}, :arg => [1]},
      {"FCCMP"   => {"M" => 0, "S" => 0, "type" => 3, "op" => 0}, :arg => [2]},
      {"FCCMPE"  => {"M" => 0, "S" => 0, "type" => 3, "op" => 1}, :arg => [2]}
    ]
  },

  "FpDataProc2Reg" => {
    :cmt => "Floating-piont data-processing (2 source)",
    :arg => [
      [ {"vd" => "SReg"}, {"vn" => "SReg"}, {"vm" => "SReg"}], #0
      [ {"vd" => "DReg"}, {"vn" => "DReg"}, {"vm" => "DReg"}], #1
      [ {"vd" => "HReg"}, {"vn" => "HReg"}, {"vm" => "HReg"}]  #2
    ],
    :prm => ["M", "S", "type", "opcode", "vd", "vn", "vm"],
    :grp => [
      {"FMUL"   => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 0}, :arg => [0]},
      {"FDIV"   => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 1}, :arg => [0]},
      {"FADD"   => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 2}, :arg => [0]},
      {"FSUB"   => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 3}, :arg => [0]},
      {"FMAX"   => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 4}, :arg => [0]},
      {"FMIN"   => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 5}, :arg => [0]},
      {"FMAXNM" => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 6}, :arg => [0]},
      {"FMINNM" => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 7}, :arg => [0]},
      {"FNMUL"  => {"M" => 0, "S" => 0, "type" => 0, "opcode" => 8}, :arg => [0]},
      {"FMUL"   => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 0}, :arg => [1]},
      {"FDIV"   => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 1}, :arg => [1]},
      {"FADD"   => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 2}, :arg => [1]},
      {"FSUB"   => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 3}, :arg => [1]},
      {"FMAX"   => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 4}, :arg => [1]},
      {"FMIN"   => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 5}, :arg => [1]},
      {"FMAXNM" => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 6}, :arg => [1]},
      {"FMINNM" => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 7}, :arg => [1]},
      {"FNMUL"  => {"M" => 0, "S" => 0, "type" => 1, "opcode" => 8}, :arg => [1]},
      {"FMUL"   => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 0}, :arg => [2]},
      {"FDIV"   => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 1}, :arg => [2]},
      {"FADD"   => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 2}, :arg => [2]},
      {"FSUB"   => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 3}, :arg => [2]},
      {"FMAX"   => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 4}, :arg => [2]},
      {"FMIN"   => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 5}, :arg => [2]},
      {"FMAXNM" => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 6}, :arg => [2]},
      {"FMINNM" => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 7}, :arg => [2]},
      {"FNMUL"  => {"M" => 0, "S" => 0, "type" => 3, "opcode" => 8}, :arg => [2]},
    ]
  },

  "FpCondSel" => {
    :cmt => "Floating-piont conditional select",
    :arg => [
      [ {"vd" => "SReg"}, {"vn" => "SReg"}, {"vm" => "SReg"}, {"cond" => "Cond"}], #0
      [ {"vd" => "DReg"}, {"vn" => "DReg"}, {"vm" => "DReg"}, {"cond" => "Cond"}], #1
      [ {"vd" => "HReg"}, {"vn" => "HReg"}, {"vm" => "HReg"}, {"cond" => "Cond"}]  #2
    ],
    :prm => ["M", "S", "type", "vd", "vn", "vm", "cond"],
    :grp => [
      {"FCSEL"   => {"M" => 0, "S" => 0, "type" => 0}, :arg => [0]},
      {"FCSEL"   => {"M" => 0, "S" => 0, "type" => 1}, :arg => [1]},
      {"FCSEL"   => {"M" => 0, "S" => 0, "type" => 3}, :arg => [2]},
    ]
  },

  "FpDataProc3Reg" => {
    :cmt => "Floating-piont data-processing (3 source)",
    :arg => [
      [ {"vd" => "SReg"}, {"vn" => "SReg"}, {"vm" => "SReg"}, {"va" => "SReg"}], #0
      [ {"vd" => "DReg"}, {"vn" => "DReg"}, {"vm" => "DReg"}, {"va" => "DReg"}], #1
      [ {"vd" => "HReg"}, {"vn" => "HReg"}, {"vm" => "HReg"}, {"va" => "HReg"}]  #2
    ],
    :prm => ["M", "S", "type", "o1", "o0", "vd", "vn", "vm", "va"],
    :grp => [
      {"FMADD"   => {"M" => 0, "S" => 0, "type" => 0, "o1" => 0, "o0" => 0}, :arg => [0]},
      {"FMSUB"   => {"M" => 0, "S" => 0, "type" => 0, "o1" => 0, "o0" => 1}, :arg => [0]},
      {"FNMADD"  => {"M" => 0, "S" => 0, "type" => 0, "o1" => 1, "o0" => 0}, :arg => [0]},
      {"FNMSUB"  => {"M" => 0, "S" => 0, "type" => 0, "o1" => 1, "o0" => 1}, :arg => [0]},
      {"FMADD"   => {"M" => 0, "S" => 0, "type" => 1, "o1" => 0, "o0" => 0}, :arg => [1]},
      {"FMSUB"   => {"M" => 0, "S" => 0, "type" => 1, "o1" => 0, "o0" => 1}, :arg => [1]},
      {"FNMADD"  => {"M" => 0, "S" => 0, "type" => 1, "o1" => 1, "o0" => 0}, :arg => [1]},
      {"FNMSUB"  => {"M" => 0, "S" => 0, "type" => 1, "o1" => 1, "o0" => 1}, :arg => [1]},
      {"FMADD"   => {"M" => 0, "S" => 0, "type" => 3, "o1" => 0, "o0" => 0}, :arg => [2]},
      {"FMSUB"   => {"M" => 0, "S" => 0, "type" => 3, "o1" => 0, "o0" => 1}, :arg => [2]},
      {"FNMADD"  => {"M" => 0, "S" => 0, "type" => 3, "o1" => 1, "o0" => 0}, :arg => [2]},
      {"FNMSUB"  => {"M" => 0, "S" => 0, "type" => 3, "o1" => 1, "o0" => 1}, :arg => [2]}
    ]
  },
  
  ########################## System instruction  ################################
  "InstCache" => {
    :cmt => "Instruction cache maintenance",
    :arg => [
      [ {"icop" => "IcOp"}, {"xt=XReg(31)" => "XReg"}]
    ],
    :prm => ["ic_op", "xt"],
    :grp => [],
  },

  "DataCache" => {
    :cmt => "Data cache maintenance",
    :arg => [
      [ {"dcop" => "DcOp"}, {"xt" => "XReg"}]
    ],
    :prm => ["dc_op", "xt"],
    :grp => [],
  },

  "AddressTrans" => {
    :cmt => "Addresss Translate",
    :arg => [
      [ {"atop" => "AtOp"}, {"xt" => "XReg"}]
    ],
    :prm => ["at_op", "xt"],
    :grp => []
  },

  "TLBInv" => {
    :cmt => "TLB Invaidate operation",
    :arg => [
      [ {"tlbiop" => "TlbiOp"}, {"xt=XReg(31)" => "XReg"}]
    ],
    :prm => ["tlbi_op", "xt"],
    :grp => []
  },
}


################################### SVE #########################################
sve = {
  "SveBitwiseLOpPred" => {
    :cmt => "SVE bitwize Logical Operation (predicated)",
    :arg => ext_args(zreg_set_pred(2,"BHSD"),[],{"zd" => "zdn", "zn" => "zm"}),
    # :arg => [[ {"zdn" => "ZReg"}, {"pg"  => "_PReg"}, {"zm"  => "ZReg"}]],
    :prm => ["opc", "zdn", "pg", "zm"],
    :grp => [
      {"ORR" => {"opc" => 0x0}},
      {"EOR" => {"opc" => 0x1}},
      {"AND" => {"opc" => 0x2}},
      {"BIC" => {"opc" => 0x3}}
    ]
  },

  "SveIntAddSubVecPred" => {
    :cmt => "SVE Integer add/subtract vectors (predicated)",
    :arg => ext_args(zreg_set_pred(2,"BHSD"),[],{"zd" => "zdn", "zn" => "zm"}),
    # :arg => [[ {"zdn" => "ZReg"}, {"pg"  => "_PReg"}, {"zm"  => "ZReg"}]],
    :prm => ["opc", "zdn", "pg", "zm"],
    :grp => [
      {"ADD"  => {"opc" => 0x0}},
      {"SUB"  => {"opc" => 0x1}},
      {"SUBR" => {"opc" => 0x3}},
    ]
  },
  
  "SveIntMinMaxDiffPred" => {
    :cmt => "SVE Integer min/max/diffrence (predicated)",
    :arg => ext_args(zreg_set_pred(2,"BHSD"),[],{"zd" => "zdn", "zn" => "zm"}),
    # :arg => [[ {"zdn" => "ZReg"}, {"pg"  => "_PReg"}, {"zm"  => "ZReg"}]],
    :prm => ["opc", "U", "zdn", "pg", "zm"],
    :grp => [
      {"SMAX" => {"opc" => 0x0, "U" => 0}},
      {"UMAX" => {"opc" => 0x0, "U" => 1}},
      {"SMIN" => {"opc" => 0x1, "U" => 0}},
      {"UMIN" => {"opc" => 0x1, "U" => 1}},
      {"SABD" => {"opc" => 0x2, "U" => 0}},
      {"UABD" => {"opc" => 0x2, "U" => 1}}
    ]
  },

  "SveIntMultDivVecPred" => {
    :cmt => "SVE Integer multiply/divide vectors (predicated)",
    :arg => ext_args(zreg_set_pred(2,"BHSD"),[],{"zd" => "zdn", "zn" => "zm"}), #0-3 (0:B, 1:H, 2:S, 3:D)
    # :arg => [[ {"zdn" => "ZReg"}, {"pg"  => "_PReg"}, {"zm"  => "ZReg"}]],
    :prm => ["opc", "U", "zdn", "pg", "zm"],
    :grp => [
      {"MUL"   => {"opc" => 0x0, "U" => 0}},
      {"SMULH" => {"opc" => 0x1, "U" => 0}},
      {"UMULH" => {"opc" => 0x1, "U" => 1}},
      {"SDIV"  => {"opc" => 0x2, "U" => 0}, :arg => [2,3]},
      {"UDIV"  => {"opc" => 0x2, "U" => 1}, :arg => [2,3]},
      {"SDIVR" => {"opc" => 0x3, "U" => 0}, :arg => [2,3]},
      {"UDIVR" => {"opc" => 0x3, "U" => 1}, :arg => [2,3]}
    ]
  },

  "SveBitwiseLReductPred" => {
    :cmt => "SVE bitwise logical reduction (predicated)",
    :arg => [
      [ {"vd" => "BReg"}, {"pg"  => "_PReg"}, {"zn"  => "ZRegB"}], #0
      [ {"vd" => "HReg"}, {"pg"  => "_PReg"}, {"zn"  => "ZRegH"}], #1
      [ {"vd" => "SReg"}, {"pg"  => "_PReg"}, {"zn"  => "ZRegS"}], #2
      [ {"vd" => "DReg"}, {"pg"  => "_PReg"}, {"zn"  => "ZRegD"}]  #3
    ],
    :prm => ["opc", "vd", "pg", "zn"],
    :grp => [
      {"ORV"  => {"opc" => 0x0}},
      {"EORV" => {"opc" => 0x1}},
      {"ANDV" => {"opc" => 0x2}}
    ]
  },

  "SveConstPrefPred" => {
    :cmt => "SVE constructive prefix (predicated)",
    :arg => zreg_set_pred(2,"BHSD"), #0-3 (0:B, 1:H, 2:S, 3:D)
    # :arg => [[ {"zd" => "ZReg"}, {"pg"  => "_PReg"}, {"zn"  => "ZReg"}]],
    :prm => ["opc", "zd", "pg", "zn"],
    :grp => [
      {"MOVPRFX"  => {"opc" => 0x0}}
    ]
  },

  "SveIntAddReductPred" => {
    :cmt => "SVE integer add reduction (predicated)",
    :arg => [
      [ {"vd" => "DReg"}, {"pg"  => "_PReg"}, {"zn"  => "ZRegB"}], #0
      [ {"vd" => "DReg"}, {"pg"  => "_PReg"}, {"zn"  => "ZRegH"}], #1
      [ {"vd" => "DReg"}, {"pg"  => "_PReg"}, {"zn"  => "ZRegS"}], #2
      [ {"vd" => "DReg"}, {"pg"  => "_PReg"}, {"zn"  => "ZRegD"}]  #3
    ],
    :prm => ["opc", "U", "vd", "pg", "zn"],
    :grp => [
      {"SADDV"  => {"opc" => 0x0, "U" => 0}, :arg => 0..2},
      {"UADDV"  => {"opc" => 0x0, "U" => 1}, :arg => 0..3}
    ]
  },

  "SveIntMinMaxReductPred" => {
    :cmt => "SVE integer min/max reduction (predicated)",
    :arg => [
      [ {"vd" => "BReg"}, {"pg"  => "_PReg"}, {"zn"  => "ZRegB"}], #0
      [ {"vd" => "HReg"}, {"pg"  => "_PReg"}, {"zn"  => "ZRegH"}], #1
      [ {"vd" => "SReg"}, {"pg"  => "_PReg"}, {"zn"  => "ZRegS"}], #2
      [ {"vd" => "DReg"}, {"pg"  => "_PReg"}, {"zn"  => "ZRegD"}]  #3
    ],
    :prm => ["opc", "U", "vd", "pg", "zn"],
    :grp => [
      {"SMAXV"  => {"opc" => 0x0, "U" => 0}},
      {"UMAXV"  => {"opc" => 0x0, "U" => 1}},
      {"SMINV"  => {"opc" => 0x1, "U" => 0}},
      {"UMINV"  => {"opc" => 0x1, "U" => 1}}
    ]
  },

  "SveBitwiseShByImmPred" => {
    :cmt => "SVE bitwise shift by immediate (predicated)",
    :arg => ext_args(zreg_set_pred(1,"BHSD"),[{"amount"  => "uint32_t"}],{"zd" => "zdn"}), #0-3 (0:B, 1:H, 2:S, 3:D)
    # :arg => [[ {"zdn" => "ZReg"}, {"pg"  => "_PReg"}, {"amount"  => "uint32_t"}]],
    :prm => ["opc", "zdn", "pg", "amount"],
    :grp => [
      {"ASR"   => {"opc" => 0x0}},
      {"LSR"   => {"opc" => 0x1}},
      {"LSL"   => {"opc" => 0x3}},
      {"ASRD"  => {"opc" => 0x4}}
    ]
  },

  "SveBitwiseShVecPred" => {
    :cmt => "SVE bitwise shift by vector (predicated)",
    :arg => ext_args(zreg_set_pred(2,"BHSD"),[],{"zd" => "zdn", "zn" => "zm"}), #0-3 (0:B, 1:H, 2:S, 3:D)
    # :arg => [[ {"zdn" => "ZReg"}, {"pg"  => "_PReg"}, {"zm"  => "ZReg"}]],
    :prm => ["opc", "zdn", "pg", "zm"],
    :grp => [
      {"ASR"   => {"opc" => 0x0}},
      {"LSR"   => {"opc" => 0x1}},
      {"LSL"   => {"opc" => 0x3}},
      {"ASRR"  => {"opc" => 0x4}},
      {"LSRR"  => {"opc" => 0x5}},
      {"LSLR"  => {"opc" => 0x7}}
    ]
  },

  "SveBitwiseShWElemPred" => {
    :cmt => "SVE bitwise shift by wide elements (predicated)",
    :arg => ext_args(zreg_set_pred(1,"BHS"),[{"zm" => "ZRegD"}],{"zd" => "zdn"}), #0-2 (0:B, 1:H, 2:S)
    # :arg => [[ {"zdn" => "ZReg"}, {"pg"  => "_PReg"}, {"zm"  => "ZReg"}]],
    :prm => ["opc", "zdn", "pg", "zm"],
    :grp => [
      {"ASR"   => {"opc" => 0x0}},
      {"LSR"   => {"opc" => 0x1}},
      {"LSL"   => {"opc" => 0x3}},
    ]
  },

  "SveBitwiseUnaryOpPred" => {
    :cmt => "SVE bitwise unary operations (predicated)",
    :arg => zreg_set_pred(2,"BHSD"), #0-3 (0:B, 1:H, 2:S, 3:D)
    # :arg => [[ {"zd" => "ZReg"}, {"pg"  => "_PReg"}, {"zn"  => "ZReg"}]],
    :prm => ["opc", "zd", "pg", "zn"],
    :grp => [
      {"CLS"   => {"opc" => 0x0}},
      {"CLZ"   => {"opc" => 0x1}},
      {"CNT"   => {"opc" => 0x2}},
      {"CNOT"  => {"opc" => 0x3}},
      {"FABS"  => {"opc" => 0x4}, :arg => 1..3},
      {"FNEG"  => {"opc" => 0x5}, :arg => 1..3},
      {"NOT"   => {"opc" => 0x6}}
    ]
  },

  "SveIntUnaryOpPred" => {
    :cmt => "SVE integer unary operations (predicated)",
    :arg => zreg_set_pred(2,"BHSD"), #0-3 (0:B, 1:H, 2:S, 3:D)
    :prm => ["opc", "zd", "pg", "zn"],
    :grp => [
      {"SXTB"  => {"opc" => 0x0}, :arg => [1,2,3]},
      {"UXTB"  => {"opc" => 0x1}, :arg => [1,2,3]},
      {"SXTH"  => {"opc" => 0x2}, :arg => [2,3]},
      {"UXTH"  => {"opc" => 0x3}, :arg => [2,3]},
      {"SXTW"  => {"opc" => 0x4}, :arg => [3]},
      {"UXTW"  => {"opc" => 0x5}, :arg => [3]},
      {"ABS"   => {"opc" => 0x6}},
      {"NEG"   => {"opc" => 0x7}}
    ]
  },

  "SveIntMultAccumPred" => {
    :cmt => "SVE integer multiply-accumulate writing addend (predicated)",
    :arg => ext_args(zreg_set_pred(3,"BHSD"),[],{"zd" => "zda"}), #0-3 (0:B, 1:H, 2:S, 3:D)
    # :arg => [[ {"zda" => "ZReg"}, {"pg"  => "_PReg"}, {"zm"  => "ZReg"}]],
    :prm => ["opc", "zda", "pg", "zn", "zm"],
    :grp => [
      {"MLA"  => {"opc" => 0x0}},
      {"MLS"  => {"opc" => 0x1}}
    ]
  },

  "SveIntMultAddPred" => {
    :cmt => "SVE integer multiply-add writeing multiplicand (predicated)",
    :arg => ext_args(zreg_set_pred(3,"BHSD"),[],{"zd"=>"zdn","zm"=>"za","zn"=>"zm"}), #0-3 (0:B, 1:H, 2:S, 3:D)
    # :arg => [[ {"zdn" => "ZReg"}, {"pg"  => "_PReg"}, {"zm"  => "ZReg"}, {"za"  => "ZReg"}]],
    :prm => ["opc", "zdn", "pg", "zm", "za"],
    :grp => [
      {"MAD"  => {"opc" => 0x0}},
      {"MSB"  => {"opc" => 0x1}}
    ]
  },

  "SveIntAddSubUnpred" => {
    :cmt => "SVE integer add/subtract vectors (unpredicated)",
    :arg => zreg_set(3,"BHSD"), #0-3 (0:B, 1:H, 2:S, 3:D)
    # :arg => [[ {"zd" => "ZReg"}, {"zn"  => "ZReg"}, {"zm"  => "ZReg"}]],
    :prm => ["opc", "zd", "zn", "zm"],
    :grp => [
      {"ADD"   => {"opc" => 0x0}},
      {"SUB"   => {"opc" => 0x1}},
      {"SQADD" => {"opc" => 0x4}},
      {"UQADD" => {"opc" => 0x5}},
      {"SQSUB" => {"opc" => 0x6}},
      {"UQSUB" => {"opc" => 0x7}}
    ]
  },

  "SveBitwiseLOpUnpred" => {
    :cmt => "SVE bitwise logical operations (unpredicated)",
    :arg => [
      [ {"zd" => "ZRegD"}, {"zn"  => "ZRegD"}, {"zm"  => "ZRegD"}], #0
      [ {"zd" => "ZRegD"}, {"zn"  => "ZRegD"}]                      #1
    ],
    :prm => ["opc", "zd", "zn", "zm"],
    :grp => [
      {"AND" => {"opc" => 0x0}, :arg => [0]},
      {"ORR" => {"opc" => 0x1}, :arg => [0]},
      {"MOV" => {"opc" => 0x1, "zm" => "zn"}, :arg => [1]}, # alias ORR
      {"EOR" => {"opc" => 0x2}, :arg => [0]},
      {"BIC" => {"opc" => 0x3}, :arg => [0]}
    ]
  },

  "SveIndexGenImmImmInc" => {
    :cmt => "SVE index generation (immediate start, immediate increment)",
    :arg => ext_args(zreg_set(1,"BHSD"),[{"imm1"  => "int32_t"}, {"imm2"  => "int32_t"}]), #0-3 (0:B, 1:H, 2:S, 3:D)
    :prm => ["zd", "imm1", "imm2"],
    :grp => [
      {"INDEX" => {}},
    ]
  },

  "SveIndexGenImmRegInc" => {
    :cmt => "SVE index generation (immediate start, register increment)",
    :arg => [
      [ {"zd" => "ZRegB"}, {"imm"  => "int32_t"}, {"rm"  => "WReg"}], #0
      [ {"zd" => "ZRegH"}, {"imm"  => "int32_t"}, {"rm"  => "WReg"}], #1
      [ {"zd" => "ZRegS"}, {"imm"  => "int32_t"}, {"rm"  => "WReg"}], #2
      [ {"zd" => "ZRegD"}, {"imm"  => "int32_t"}, {"rm"  => "XReg"}]  #3
    ],
    :prm => ["zd", "imm", "rm"],
    :grp => [
      {"INDEX" => {}},
    ]
  },

  "SveIndexGenRegImmInc" => {
    :cmt => "SVE index generation (register start, immediate increment)",
    :arg => [
      [ {"zd" => "ZRegB"}, {"rn"  => "WReg"}, {"imm"  => "int32_t"}], #0
      [ {"zd" => "ZRegH"}, {"rn"  => "WReg"}, {"imm"  => "int32_t"}], #1
      [ {"zd" => "ZRegS"}, {"rn"  => "WReg"}, {"imm"  => "int32_t"}], #2
      [ {"zd" => "ZRegD"}, {"rn"  => "XReg"}, {"imm"  => "int32_t"}]  #3
    ],
    :prm => ["zd", "rn", "imm"],
    :grp => [
      {"INDEX" => {}},
    ]
  },

  "SveIndexGenRegRegInc" => {
    :cmt => "SVE index generation (register start, register increment)",
    :arg => [
      [ {"zd" => "ZRegB"}, {"rn"  => "WReg"}, {"rm"  => "WReg"}], #0
      [ {"zd" => "ZRegH"}, {"rn"  => "WReg"}, {"rm"  => "WReg"}], #1
      [ {"zd" => "ZRegS"}, {"rn"  => "WReg"}, {"rm"  => "WReg"}], #2
      [ {"zd" => "ZRegD"}, {"rn"  => "XReg"}, {"rm"  => "XReg"}]  #3
    ],
    :prm => ["zd", "rn", "rm"],
    :grp => [
      {"INDEX" => {}},
    ]
  },

  "SveStackFrameAdjust" => {
    :cmt => "SVE stack frame adjustment",
    :arg => [[ {"xd" => "XReg"}, {"xn"  => "XReg"}, {"imm"  => "int32_t"}]],
    :prm => ["op", "xd", "xn", "imm"],
    :grp => [
      {"ADDVL" => {"op" => 0}},
      {"ADDPL" => {"op" => 1}}
    ]
  },

  "SveStackFrameSize" => {
    :cmt => "SVE stack frame size",
    :arg => [[ {"xd" => "XReg"}, {"imm"  => "int32_t"}]],
    :prm => ["op", "opc2", "xd", "imm"],
    :grp => [
      {"RDVL" => {"op" => 0, "opc2" => 0x1f}},
    ]
  },

  "SveBitwiseShByImmUnpred" => {
    :cmt => "SVE bitwise shift by immediate (unpredicated)",
    :arg => ext_args(zreg_set(2, "BHSD"),[{"amount"  => "uint32_t"}]),
    # :arg => [[ {"zd" => "ZReg"}, {"zn"  => "ZReg"}, {"amount"  => "uint32_t"}]],
    :prm => ["opc", "zd", "zn", "amount"],
    :grp => [
      {"ASR" => {"opc" => 0}},
      {"LSR" => {"opc" => 1}},
      {"LSL" => {"opc" => 3}}
    ]
  },

  "SveBitwiseShByWideElemUnPred" => {
    :cmt => "SVE bitwise shift by wide elements (unpredicated)",
    :arg => ext_args(zreg_set(2, "BHS"),[{"zm"  => "ZRegD"}]),
    # :arg => [[ {"zd" => "ZReg"}, {"zn"  => "ZReg"}, {"zm"  => "ZRegD"}]],
    :prm => ["opc", "zd", "zn", "zm"],
    :grp => [
      {"ASR" => {"opc" => 0}},
      {"LSR" => {"opc" => 1}},
      {"LSL" => {"opc" => 3}}
    ]
  },

  "SveAddressGen" => {
    :cmt => "SVE address generation",
    :arg => [
      [ {"zd" => "ZRegS"}, {"adr" => "AdrVec"}], #0
      [ {"zd" => "ZRegD"}, {"adr" => "AdrVec"}], #0
      [ {"zd" => "ZRegD"}, {"adr" => "AdrVecU"}] #1
    ],
    :prm => ["zd", "adr"],
    :grp => [
      {"ADR" => {}},
    ]
  },

  "SveConstPrefUnpred" => {
    :cmt => "SVE constructive prefix (unpredicated)",
    :arg => [[ {"zd" => "ZReg"}, {"zn"  => "ZReg"}]],
    :prm => ["opc", "opc2", "zd", "zn"],
    :grp => [
      {"MOVPRFX" => {"opc" => 0, "opc2" => 0}},
    ]
  },

  "SveFpExpAccel" => {
    :cmt => "SVE floating-point exponential accelerator",
    :arg => zreg_set(2, "HSD"),
    # :arg => [[ {"zd" => "ZReg"}, {"zn"  => "ZReg"}]],
    :prm => ["opc", "zd", "zn"],
    :grp => [
      {"FEXPA" => {"opc" => 0}},
    ]
  },

  "SveFpTrigSelCoef" => {
    :cmt => "SVE floating-point trig select coefficient",
    :arg => zreg_set(3, "HSD"),
    # :arg => [[ {"zd" => "ZReg"}, {"zn"  => "ZReg"}, {"zm"  => "ZReg"}]],
    :prm => ["opc", "zd", "zn", "zm"],
    :grp => [
      {"FTSSEL" => {"opc" => 0}},
    ]
  },

  "SveElemCount" => {
    :cmt => "SVE element count",
    :arg => [[ {"xd" => "XReg"}, {"pat=ALL" => "Pattern"}, {"mod=MUL" => "ExtMod"}, {"imm=1" => "uint32_t"}]],
    :prm => ["size", "op", "xd", "pat", "mod", "imm"],
    :grp => [
      {"CNTB" => {"size" => 0, "op" => 0}},
      {"CNTH" => {"size" => 1, "op" => 0}},
      {"CNTW" => {"size" => 2, "op" => 0}},
      {"CNTD" => {"size" => 3, "op" => 0}}
    ]
  },

  "SveIncDecRegByElemCount" => {
    :cmt => "SVE inc/dec register by element count",
    :arg => [[ {"xd" => "XReg"}, {"pat=ALL" => "Pattern"}, {"mod=MUL" => "ExtMod"}, {"imm=1" => "uint32_t"}]],
    :prm => ["size", "D", "xd", "pat", "mod", "imm"],
    :grp => [
      {"INCB" => {"size" => 0, "D" => 0}},
      {"DECB" => {"size" => 0, "D" => 1}},
      {"INCH" => {"size" => 1, "D" => 0}},
      {"DECH" => {"size" => 1, "D" => 1}},
      {"INCW" => {"size" => 2, "D" => 0}},
      {"DECW" => {"size" => 2, "D" => 1}},
      {"INCD" => {"size" => 3, "D" => 0}},
      {"DECD" => {"size" => 3, "D" => 1}}
    ]
  },

  "SveIncDecVecByElemCount" => {
    :cmt => "SVE inc/dec vector by element count",
    :arg => [
      [ {"zd" => "ZRegH"}, {"pat=ALL" => "Pattern"}, {"mod=MUL" => "ExtMod"}, {"imm=1" => "uint32_t"}], #0
      [ {"zd" => "ZRegS"}, {"pat=ALL" => "Pattern"}, {"mod=MUL" => "ExtMod"}, {"imm=1" => "uint32_t"}], #1
      [ {"zd" => "ZRegD"}, {"pat=ALL" => "Pattern"}, {"mod=MUL" => "ExtMod"}, {"imm=1" => "uint32_t"}]  #2
    ],
    :prm => ["size", "D", "zd", "pat", "mod", "imm"],
    :grp => [
      {"INCH" => {"size" => 1, "D" => 0}, :arg => [0]},
      {"DECH" => {"size" => 1, "D" => 1}, :arg => [0]},
      {"INCW" => {"size" => 2, "D" => 0}, :arg => [1]},
      {"DECW" => {"size" => 2, "D" => 1}, :arg => [1]},
      {"INCD" => {"size" => 3, "D" => 0}, :arg => [2]},
      {"DECD" => {"size" => 3, "D" => 1}, :arg => [2]}
    ]
  },

  "SveSatuIncDecRegByElemCount" => {
    :cmt => "SVE saturating inc/dec register by element count",
    :arg => [
      [ {"rdn" => "WReg"}, {"pat=ALL" => "Pattern"}, {"mod=MUL" => "ExtMod"}, {"imm=1" => "uint32_t"}], #0
      [ {"rdn" => "XReg"}, {"pat=ALL" => "Pattern"}, {"mod=MUL" => "ExtMod"}, {"imm=1" => "uint32_t"}]  #1
    ],
    :prm => ["size", "D", "U", "rdn", "pat", "mod", "imm"],
    :grp => [
      {"SQINCB" => {"size" => 0, "D" => 0, "U" => 0}},
      {"UQINCB" => {"size" => 0, "D" => 0, "U" => 1}},
      {"SQDECB" => {"size" => 0, "D" => 1, "U" => 0}},
      {"UQDECB" => {"size" => 0, "D" => 1, "U" => 1}},
      {"SQINCH" => {"size" => 1, "D" => 0, "U" => 0}},
      {"UQINCH" => {"size" => 1, "D" => 0, "U" => 1}},
      {"SQDECH" => {"size" => 1, "D" => 1, "U" => 0}},
      {"UQDECH" => {"size" => 1, "D" => 1, "U" => 1}},
      {"SQINCW" => {"size" => 2, "D" => 0, "U" => 0}},
      {"UQINCW" => {"size" => 2, "D" => 0, "U" => 1}},
      {"SQDECW" => {"size" => 2, "D" => 1, "U" => 0}},
      {"UQDECW" => {"size" => 2, "D" => 1, "U" => 1}},
      {"SQINCD" => {"size" => 3, "D" => 0, "U" => 0}},
      {"UQINCD" => {"size" => 3, "D" => 0, "U" => 1}},
      {"SQDECD" => {"size" => 3, "D" => 1, "U" => 0}},
      {"UQDECD" => {"size" => 3, "D" => 1, "U" => 1}}
    ]
  },

  "SveSatuIncDecVecByElemCount" => {
    :cmt => "SVE saturating inc/dec vector by element count",
    :arg => [
      [ {"zdn" => "ZRegH"}, {"pat=ALL" => "Pattern"}, {"mod=MUL" => "ExtMod"}, {"imm=1" => "uint32_t"}], #0
      [ {"zdn" => "ZRegS"}, {"pat=ALL" => "Pattern"}, {"mod=MUL" => "ExtMod"}, {"imm=1" => "uint32_t"}], #1
      [ {"zdn" => "ZRegD"}, {"pat=ALL" => "Pattern"}, {"mod=MUL" => "ExtMod"}, {"imm=1" => "uint32_t"}]  #2
    ],
    :prm => ["size", "D", "U", "zdn", "pat", "mod", "imm"],
    :grp => [
      {"SQINCH" => {"size" => 1, "D" => 0, "U" => 0}, :arg => [0]},
      {"UQINCH" => {"size" => 1, "D" => 0, "U" => 1}, :arg => [0]},
      {"SQDECH" => {"size" => 1, "D" => 1, "U" => 0}, :arg => [0]},
      {"UQDECH" => {"size" => 1, "D" => 1, "U" => 1}, :arg => [0]},
      {"SQINCW" => {"size" => 2, "D" => 0, "U" => 0}, :arg => [1]},
      {"UQINCW" => {"size" => 2, "D" => 0, "U" => 1}, :arg => [1]},
      {"SQDECW" => {"size" => 2, "D" => 1, "U" => 0}, :arg => [1]},
      {"UQDECW" => {"size" => 2, "D" => 1, "U" => 1}, :arg => [1]},
      {"SQINCD" => {"size" => 3, "D" => 0, "U" => 0}, :arg => [2]},
      {"UQINCD" => {"size" => 3, "D" => 0, "U" => 1}, :arg => [2]},
      {"SQDECD" => {"size" => 3, "D" => 1, "U" => 0}, :arg => [2]},
      {"UQDECD" => {"size" => 3, "D" => 1, "U" => 1}, :arg => [2]}
    ]
  },

  "SveBitwiseLogicalImmUnpred" => {
    :cmt => "SVE bitwise logical with immediate (unpredicated)",
    :arg => ext_args(zreg_set(1,"BHSD"), [{"imm"  => "uint64_t"}], {"zd"=>"zdn"}),
    # :arg => [[ {"zdn" => "ZReg"}, {"imm"  => "uint32_t"}]],
    :prm => ["opc", "zdn", "imm"],
    :grp => [
      {"ORR" => {"opc" => 0}},
      {"ORN" => {"opc" => 0, "imm" => "((-1)*imm-1)"}}, # alias of ORR
      {"EOR" => {"opc" => 1}},
      {"EON" => {"opc" => 1, "imm" => "((-1)*imm-1)"}}, # alias of EOR
      {"AND" => {"opc" => 2}},
      {"BIC" => {"opc" => 2, "imm" => "((-1)*imm-1)"}}  # alias of AND
    ]
  },

  "SveBcBitmaskImm" => {
    :cmt => "SVE broadcast bitmask immediate",
    :arg => ext_args(zreg_set(1,"BHSD"), [{"imm"  => "uint64_t"}]),
    # :arg => [[ {"zd" => "ZReg"}, {"imm"  => "uint32_t"}]],
    :prm => ["zd", "imm"],
    :grp => [
      {"DUPM" => {}},
      {"MOV"  => {"imm" => "genMoveMaskPrefferd(imm)"}}
    ]
  },

  "SveCopyFpImmPred" => {
    :cmt => "SVE copy floating-point immediate (predicated)",
    :arg => ext_args(zreg_set_pred(1,"HSD"), [{"imm"  => "double"}]),
    # :arg => [[ {"zd" => "ZReg"}, {"pg" => "_PReg"}, {"imm"  => "uint32_t"}]],
    :prm => ["zd", "pg", "imm"],
    :grp => [
      {"FCPY" => {}},
      {"FMOV" => {}}, # alias of FCPY
    ]
  },

  "SveCopyIntImmPred" => {
    :cmt => "SVE copy integer immediate (predicated)",
    :arg => ext_args(zreg_set_pred(1,"BHSD"), [{"imm" => "uint32_t"}, {"mod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}]) + # 0-3
            ext_args(zreg_set_pred(1,"HSD"), [{"imm=0.0" => "uint32_t"}]),                                               # 4-6
    :prm => ["zd", "pg", "imm", "mod", "sh"],
    :grp => [
      {"CPY"  => {},                          :arg => 0..3},
      {"MOV"  => {},                          :arg => 0..3}, # alias of CPY
      {"FMOV" => {"mod" => "LSL", "sh" => 0}, :arg => 4..6}, # alias of CPY
    ]
  },

  "SveExtVec" => {
    :cmt => "SVE extract vector (immediate offset)",
    :arg => [[ {"zdn" => "ZRegB"}, {"zm" => "ZRegB"}, {"imm"  => "uint32_t"}]],
    :prm => ["zdn", "zm", "imm"],
    :grp => [
      {"EXT" => {}},
    ]
  },

  "SveBcGeneralReg" => {
    :cmt => "SVE broadcast general register",
    :arg => [
      [ {"zd" => "ZRegB"}, {"rn" => "WReg"}], #0
      [ {"zd" => "ZRegH"}, {"rn" => "WReg"}], #1
      [ {"zd" => "ZRegS"}, {"rn" => "WReg"}], #2
      [ {"zd" => "ZRegD"}, {"rn" => "XReg"}]  #3
    ],
    :prm => ["zd", "rn"],
    :grp => [
      {"DUP" => {}},
      {"MOV" => {}}, # alias of DUP
    ]
  },
  
  "SveBcIndexedElem" => {
    :cmt => "SVE broadcast indexed element",
    :arg => [
      [ {"zd" => "ZRegB"}, {"zn" => "ZRegBElem"}], #0
      [ {"zd" => "ZRegH"}, {"zn" => "ZRegHElem"}], #1
      [ {"zd" => "ZRegS"}, {"zn" => "ZRegSElem"}], #2
      [ {"zd" => "ZRegD"}, {"zn" => "ZRegDElem"}], #3
      [ {"zd" => "ZRegQ"}, {"zn" => "ZRegQElem"}], #4
      [ {"zd" => "ZRegB"}, {"vn" => "BReg"}],      #5
      [ {"zd" => "ZRegH"}, {"vn" => "HReg"}],      #6
      [ {"zd" => "ZRegS"}, {"vn" => "SReg"}],      #7
      [ {"zd" => "ZRegD"}, {"vn" => "DReg"}],      #8
      [ {"zd" => "ZRegQ"}, {"vn" => "QReg"}]       #9
    ],
    :prm => [
      ["zd", "zn"], #0
      ["zd", "vn"]  #1
    ],      
    :grp => [
      {"DUP" => {}, :arg => 0..4, :prm => 0},
      {"MOV" => {}, :arg => 0..4, :prm => 0},
      {"MOV" => {"vn" => "ZRegElem(vn.getIdx(),0,vn.getBit())"}, :arg => 5..9, :prm => 1},
    ]
  },

  "SveInsSimdFpSclarReg" => {
    :cmt => "SVE insert SIMD&FP scalar register",
    :arg => [
      [ {"zdn" => "ZRegB"}, {"vm" => "BReg"}], #0
      [ {"zdn" => "ZRegH"}, {"vm" => "HReg"}], #1
      [ {"zdn" => "ZRegS"}, {"vm" => "SReg"}], #2
      [ {"zdn" => "ZRegD"}, {"vm" => "DReg"}]  #3
    ],
    :prm => ["zdn", "vm"],
    :grp => [
      {"INSR" => {}},
    ]
  },
  
  "SveInsGeneralReg" => {
    :cmt => "SVE insert general register",
    :arg => [
      [ {"zdn" => "ZRegB"}, {"rm" => "WReg"}], #0
      [ {"zdn" => "ZRegH"}, {"rm" => "WReg"}], #1
      [ {"zdn" => "ZRegS"}, {"rm" => "WReg"}], #2
      [ {"zdn" => "ZRegD"}, {"rm" => "XReg"}]  #3
    ],
    :prm => ["zdn", "rm"],
    :grp => [
      {"INSR" => {}},
    ]
  },

  "SveRevVecElem" => {
    :cmt => "SVE reverse vector elements",
    :arg => zreg_set(2, "BHSD"),
    # :arg => [[ {"zd" => "ZReg"}, {"zn" => "ZReg"}]],
    :prm => ["zd", "zn"],
    :grp => [
      {"REV" => {}},
    ]
  },

  "SveTableLookup" => {
    :cmt => "SVE table lookup",
    :arg => zreg_set(3, "BHSD"),
    # :arg => [[ {"zd" => "ZReg"}, {"zn" => "ZReg"}, {"zm" => "ZReg"}]],
    :prm => ["zd", "zn", "zm"],
    :grp => [
      {"TBL" => {}},
    ]
  },

  "SveUnpackVecElem" => {
    :cmt => "SVE unpack vector elements",
    :arg => [
      [ {"zd" => "ZRegH"}, {"zn" => "ZRegB"}], #0
      [ {"zd" => "ZRegS"}, {"zn" => "ZRegH"}], #1
      [ {"zd" => "ZRegD"}, {"zn" => "ZRegS"}]  #2
    ],
    :prm => ["U", "H", "zd", "zn"],
    :grp => [
      {"SUNPKLO" => {"U" => 0, "H" => 0}},
      {"SUNPKHI" => {"U" => 0, "H" => 1}},
      {"UUNPKLO" => {"U" => 1, "H" => 0}},
      {"UUNPKHI" => {"U" => 1, "H" => 1}},
    ]
  },

  "SvePermutePredElem" => {
    :cmt => "SVE permute predicate elements",
    :arg => preg_set(3,"BHSD"),
    # :arg => [[ {"pd" => "_PReg"}, {"pn" => "_PReg"}, {"pm" => "_PReg"}]],
    :prm => ["opc", "H", "pd", "pn", "pm"],
    :grp => [
      {"ZIP1" => {"opc" => 0, "H" => 0}},
      {"ZIP2" => {"opc" => 0, "H" => 1}},
      {"UZP1" => {"opc" => 1, "H" => 0}},
      {"UZP2" => {"opc" => 1, "H" => 1}},
      {"TRN1" => {"opc" => 2, "H" => 0}},
      {"TRN2" => {"opc" => 2, "H" => 1}}
    ]
  },

  "SveRevPredElem" => {
    :cmt => "SVE reverse predicate elements",
    :arg => preg_set(2,"BHSD"),
    # :arg => [[ {"pd" => "_PReg"}, {"pn" => "_PReg"}]],
    :prm => ["pd", "pn"],
    :grp => [
      {"REV" => {}},
    ]
  },

  "SveUnpackPredElem" => {
    :cmt => "SVE unpack predicate elements",
    :arg => [[ {"pd" => "PRegH"}, {"pn" => "PRegB"}]],
    :prm => ["H", "pd", "pn"],
    :grp => [
      {"PUNPKLO" => {"H" => 0}},
      {"PUNPKHI" => {"H" => 1}},
    ]
  },

  "SvePermuteVecElem" => {
    :cmt => "SVE permute vector elements",
    :arg => zreg_set(3,"BHSD"),
    # :arg => [[ {"zd" => "ZReg"}, {"zn" => "ZReg"}, {"zm" => "ZReg"}]],
    :prm => ["opc", "zd", "zn", "zm"],
    :grp => [
      {"ZIP1" => {"opc" => 0}},
      {"ZIP2" => {"opc" => 1}},
      {"UZP1" => {"opc" => 2}},
      {"UZP2" => {"opc" => 3}},
      {"TRN1" => {"opc" => 4}},
      {"TRN2" => {"opc" => 5}}
    ]
  },
  
  "SveCompressActElem" => {
    :cmt => "SVE compress active elements",
    :arg => zreg_set_pred(2, "SD"),
    # :arg => [[ {"zd" => "ZReg"}, {"pg" => "_PReg"}, {"zn" => "ZReg"}]],
    :prm => ["zd", "pg", "zn"],
    :grp => [
      {"COMPACT" => {}},
    ]
  },

  "SveCondBcElemToVec" => {
    :cmt => "SVE conditionally broaccast element to vector",
    :arg => ext_args(zreg_set_pred(2, "BHSD"),[],{"zd"=>"zdn","zn"=>"zm"}),
    # :arg => [[ {"zdn" => "ZReg"}, {"pg" => "_PReg"}, {"zm" => "ZReg"}]],
    :prm => ["B", "zdn", "pg", "zm"],
    :grp => [
      {"CLASTA" => {"B" => 0}},
      {"CLASTB" => {"B" => 1}}
    ]
  },
  
  "SveCondExtElemToSimdFpScalar" => {
    :cmt => "SVE conditionally extract element to SIMD&FP scalar",
    :arg => [
      [ {"vdn" => "BReg"}, {"pg" => "_PReg"}, {"zm" => "ZRegB"}], #0
      [ {"vdn" => "HReg"}, {"pg" => "_PReg"}, {"zm" => "ZRegH"}], #1
      [ {"vdn" => "SReg"}, {"pg" => "_PReg"}, {"zm" => "ZRegS"}], #2
      [ {"vdn" => "DReg"}, {"pg" => "_PReg"}, {"zm" => "ZRegD"}]  #3
    ],
    :prm => ["B", "vdn", "pg", "zm"],
    :grp => [
      {"CLASTA" => {"B" => 0}},
      {"CLASTB" => {"B" => 1}}
    ]
  },

  "SveCondExtElemToGeneralReg" => {
    :cmt => "SVE conditionally extract element to general Reg",
    :arg => [
      [ {"rdn" => "WReg"}, {"pg" => "_PReg"}, {"zm" => "ZRegB"}], #0
      [ {"rdn" => "WReg"}, {"pg" => "_PReg"}, {"zm" => "ZRegH"}], #1
      [ {"rdn" => "WReg"}, {"pg" => "_PReg"}, {"zm" => "ZRegS"}], #2
      [ {"rdn" => "XReg"}, {"pg" => "_PReg"}, {"zm" => "ZRegD"}]  #3
    ],
    :prm => ["B", "rdn", "pg", "zm"],
    :grp => [
      {"CLASTA" => {"B" => 0}},
      {"CLASTB" => {"B" => 1}}
    ]
  },

  "SveCopySimdFpScalarToVecPred" => {
    :cmt => "SVE copy SIMD&FP scalar register to vector (predicated)",
    :arg => [
      [ {"zd" => "ZRegB"}, {"pg" => "_PReg"}, {"vn" => "BReg"}], #0
      [ {"zd" => "ZRegH"}, {"pg" => "_PReg"}, {"vn" => "HReg"}], #1
      [ {"zd" => "ZRegS"}, {"pg" => "_PReg"}, {"vn" => "SReg"}], #2
      [ {"zd" => "ZRegD"}, {"pg" => "_PReg"}, {"vn" => "DReg"}]  #3
    ],
    :prm => ["zd", "pg", "vn"],
    :grp => [
      {"CPY" => {}},
      {"MOV" => {}}, # alias of CPY
    ]
  },

  "SveCopyGeneralRegToVecPred" => {
    :cmt => "SVE copy general register to vector (predicated)",
    :arg => [
      [ {"zd" => "ZRegB"}, {"pg" => "_PReg"}, {"rn" => "WReg"}], #0
      [ {"zd" => "ZRegH"}, {"pg" => "_PReg"}, {"rn" => "WReg"}], #1
      [ {"zd" => "ZRegS"}, {"pg" => "_PReg"}, {"rn" => "WReg"}], #2
      [ {"zd" => "ZRegD"}, {"pg" => "_PReg"}, {"rn" => "XReg"}]  #3
    ],
    :prm => ["zd", "pg", "rn"],
    :grp => [
      {"CPY" => {}},
      {"MOV" => {}}, # alias of CPY
    ]
  },

  "SveExtElemToSimdFpScalar" => {
    :cmt => "SVE extract element to SIMD&FP scalar register",
    :arg => [
      [ {"vd" => "BReg"}, {"pg" => "_PReg"}, {"zn" => "ZRegB"}], #0
      [ {"vd" => "HReg"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}], #1
      [ {"vd" => "SReg"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}], #2
      [ {"vd" => "DReg"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}]  #3
    ],
    :prm => ["B", "vd", "pg", "zn"],
    :grp => [
      {"LASTA" => {"B" => 0}},
      {"LASTB" => {"B" => 1}},
    ]
  },


  "SveExtElemToGeneralReg" => {
    :cmt => "SVE extract element to general register",
    :arg => [
      [ {"rd" => "WReg"}, {"pg" => "_PReg"}, {"zn" => "ZRegB"}], #0
      [ {"rd" => "WReg"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}], #1
      [ {"rd" => "WReg"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}], #2
      [ {"rd" => "XReg"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}]  #3
    ],
    :prm => ["B", "rd", "pg", "zn"],
    :grp => [
      {"LASTA" => {"B" => 0}},
      {"LASTB" => {"B" => 1}},
    ]
  },

  "SveRevWithinElem" => {
    :cmt => "SVE reverse within elements",
    :arg => zreg_set_pred(2, "BHSD"),
    # :arg => [ [ {"zd" => "ZReg"}, {"pg" => "_PReg"}, {"zn" => "ZReg"}]],
    :prm => ["opc", "zd", "pg", "zn"],
    :grp => [
      {"REVB" => {"opc" => 0}, :arg => [1,2,3]},
      {"REVH" => {"opc" => 1}, :arg => [2,3]},
      {"REVW" => {"opc" => 2}, :arg => [3]},
      {"RBIT" => {"opc" => 3}}
    ]
  },

  "SveSelVecSplice" => {
    :cmt => "SVE vector splice",
    :arg => ext_args(zreg_set_pred(2, "BHSD"),[],{"zd"=>"zdn","zn"=>"zm"}),
    # :arg => [[ {"zdn" => "ZReg"}, {"pg" => "_PReg"}, {"zm" => "ZReg"}]],
    :prm => ["zdn", "pg", "zm"],
    :grp => [
      {"SPLICE" => {}},
    ]
  },

  "SveSelVecElemPred" => {
    :cmt => "SVE select vector elements (predicated)",
    :arg => zreg_set_pred(3, "BHSD") + # 0-3
            zreg_set_pred(2, "BHSD"),  # 4-7
    :prm => ["zd", "pg", "zn", "zm"],
    :grp => [
      {"SEL" => {},             :arg => 0..3},
      {"MOV" => {"zm" => "zd"}, :arg => 4..7}, # alias of SEL
    ]
  },

  "SveIntCompVec" => {
    :cmt => "SVE integer compare vectors",
    :arg => [
      [ {"pd" => "PRegB"}, {"pg" => "_PReg"}, {"zn" => "ZRegB"}, {"zm" => "ZRegB"}], #0
      [ {"pd" => "PRegH"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}, {"zm" => "ZRegH"}], #1
      [ {"pd" => "PRegS"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}, {"zm" => "ZRegS"}], #2
      [ {"pd" => "PRegD"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}, {"zm" => "ZRegD"}], #3
      [ {"pd" => "PRegB"}, {"pg" => "_PReg"}, {"zn" => "ZRegB"}, {"zm" => "ZRegD"}], #4
      [ {"pd" => "PRegH"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}, {"zm" => "ZRegD"}], #5
      [ {"pd" => "PRegS"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}, {"zm" => "ZRegD"}], #6
    ],
    :prm => ["op", "o2", "ne", "pd", "pg", "zn", "zm"],
    :grp => [
      {"CMPHS" => {"op" => 0, "o2" => 0, "ne" => 0}, :arg => 0..3},
      {"CMPHI" => {"op" => 0, "o2" => 0, "ne" => 1}, :arg => 0..3},
      {"CMPEQ" => {"op" => 0, "o2" => 1, "ne" => 0}, :arg => 4..6},
      {"CMPNE" => {"op" => 0, "o2" => 1, "ne" => 1}, :arg => 4..6},
      {"CMPGE" => {"op" => 1, "o2" => 0, "ne" => 0}, :arg => 0..3},
      {"CMPGT" => {"op" => 1, "o2" => 0, "ne" => 1}, :arg => 0..3},
      {"CMPEQ" => {"op" => 1, "o2" => 1, "ne" => 0}, :arg => 0..3},
      {"CMPNE" => {"op" => 1, "o2" => 1, "ne" => 1}, :arg => 0..3},
      {"CMPLE" => {"op" => 1, "o2" => 0, "ne" => 0, "zn" => "zm", "zm" => "zn"}, :arg => 0..3}, # alias of CMPGE
      {"CMPLO" => {"op" => 0, "o2" => 0, "ne" => 1, "zn" => "zm", "zm" => "zn"}, :arg => 0..3}, # alias of CMPHI
      {"CMPLS" => {"op" => 0, "o2" => 0, "ne" => 0, "zn" => "zm", "zm" => "zn"}, :arg => 0..3}, # alias of CMPHS
      {"CMPLT" => {"op" => 1, "o2" => 0, "ne" => 1, "zn" => "zm", "zm" => "zn"}, :arg => 0..3}  # alias of CMPGT
    ]
  },

  "SveIntCompWideElem" => {
    :cmt => "SVE integer compare with wide elements",
    :arg => [
      [ {"pd" => "PRegB"}, {"pg" => "_PReg"}, {"zn" => "ZRegB"}, {"zm" => "ZRegD"}], #0
      [ {"pd" => "PRegH"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}, {"zm" => "ZRegD"}], #1
      [ {"pd" => "PRegS"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}, {"zm" => "ZRegD"}]  #2
    ],
    :prm => ["U", "lt", "ne", "pd", "pg", "zn", "zm"],
    :grp => [
      {"CMPGE" => {"U" => 0, "lt" => 0, "ne" => 0}},
      {"CMPGT" => {"U" => 0, "lt" => 0, "ne" => 1}},
      {"CMPLT" => {"U" => 0, "lt" => 1, "ne" => 0}},
      {"CMPLE" => {"U" => 0, "lt" => 1, "ne" => 1}},
      {"CMPHS" => {"U" => 1, "lt" => 0, "ne" => 0}},
      {"CMPHI" => {"U" => 1, "lt" => 0, "ne" => 1}},
      {"CMPLO" => {"U" => 1, "lt" => 1, "ne" => 0}},
      {"CMPLS" => {"U" => 1, "lt" => 1, "ne" => 1}}
    ]
  },

  "SveIntCompUImm" => {
    :cmt => "SVE integer compare with unsigned immediate",
    :arg => [
      [ {"pd" => "PRegB"}, {"pg" => "_PReg"}, {"zn" => "ZRegB"}, {"imm" => "uint32_t"}], #0
      [ {"pd" => "PRegH"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}, {"imm" => "uint32_t"}], #1
      [ {"pd" => "PRegS"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}, {"imm" => "uint32_t"}], #2
      [ {"pd" => "PRegD"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}, {"imm" => "uint32_t"}]  #3
    ],
    :prm => ["lt", "ne", "pd", "pg", "zn", "imm"],
    :grp => [
      {"CMPHS" => {"lt" => 0, "ne" => 0}},
      {"CMPHI" => {"lt" => 0, "ne" => 1}},
      {"CMPLO" => {"lt" => 1, "ne" => 0}},
      {"CMPLS" => {"lt" => 1, "ne" => 1}}
    ]
  },

  "SvePredLOp" => {
    :cmt => "SVE predicate logical operations",
    :arg => [
      [ {"pd" => "PRegB"}, {"pg" => "_PReg"}, {"pn" => "PRegB"}, {"pm" => "PRegB"}], #0
      [ {"pd" => "PRegB"}, {"pg" => "_PReg"}, {"pn" => "PRegB"}],                    #1
      [ {"pd" => "PRegB"}, {"pn" => "PRegB"}]                                        #2
    ],
    :prm => ["op", "S", "o2", "o3", "pd", "pg", "pn", "pm"],
    :grp => [
      {"AND"   => {"op" => 0, "S" => 0, "o2" => 0, "o3" => 0}, :arg => [0]},
      {"MOV"   => {"op" => 0, "S" => 0,
                   "o2" => "pg.isM()",
                   "o3" => "pg.isM()",
                   "pm" => "(pg.isZ())? pn : pd"},             :arg => [1]}, # alias of AND or SEL
      {"BIC"   => {"op" => 0, "S" => 0, "o2" => 0, "o3" => 1}, :arg => [0]},
      {"EOR"   => {"op" => 0, "S" => 0, "o2" => 1, "o3" => 0}, :arg => [0]},
      {"NOT"   => {"op" => 0, "S" => 0, "o2" => 1, "o3" => 0, "pm" => "pg"}, :arg => [1]}, # alias of EOR
      {"SEL"   => {"op" => 0, "S" => 0, "o2" => 1, "o3" => 1}, :arg => [0]},
      {"ANDS"  => {"op" => 0, "S" => 1, "o2" => 0, "o3" => 0}, :arg => [0]},
      {"MOVS"  => {"op" => 0, "S" => 1, "o2" => 0, "o3" => 0, "pm" => "pn"}, :arg => [1]}, # alias of ANDS
      {"BICS"  => {"op" => 0, "S" => 1, "o2" => 0, "o3" => 1}, :arg => [0]},
      {"EORS"  => {"op" => 0, "S" => 1, "o2" => 1, "o3" => 0}, :arg => [0]},
      {"NOTS"  => {"op" => 0, "S" => 1, "o2" => 1, "o3" => 0, "pm" => "pg"}, :arg => [1]}, # alias of EORS
      {"ORR"   => {"op" => 1, "S" => 0, "o2" => 0, "o3" => 0}, :arg => [0]},
      {"MOV"   => {"op" => 1, "S" => 0, "o2" => 0, "o3" => 0, "pg" => "pn", "pm" => "pn"}, :arg => [2]}, # alias of ORR
      {"ORN"   => {"op" => 1, "S" => 0, "o2" => 0, "o3" => 1}, :arg => [0]},
      {"NOR"   => {"op" => 1, "S" => 0, "o2" => 1, "o3" => 0}, :arg => [0]},
      {"NAND"  => {"op" => 1, "S" => 0, "o2" => 1, "o3" => 1}, :arg => [0]},
      {"ORRS"  => {"op" => 1, "S" => 1, "o2" => 0, "o3" => 0}, :arg => [0]},
      {"MOVS"  => {"op" => 1, "S" => 1, "o2" => 0, "o3" => 0, "pg" => "pn", "pm" => "pn"}, :arg => [2]}, # alias of ORRS
      {"ORNS"  => {"op" => 1, "S" => 1, "o2" => 0, "o3" => 1}, :arg => [0]},
      {"NORS"  => {"op" => 1, "S" => 1, "o2" => 1, "o3" => 0}, :arg => [0]},
      {"NANDS" => {"op" => 1, "S" => 1, "o2" => 1, "o3" => 1}, :arg => [0]}
    ]
  },

  "SvePropagateBreakPrevPtn" => {
    :cmt => "SVE propagate break from previous partition",
    :arg => [
      [ {"pd" => "PRegB"}, {"pg" => "_PReg"}, {"pn" => "PRegB"}, {"pm" => "PRegB"}] #0
    ],
    :prm => ["op", "S", "B", "pd", "pg", "pn", "pm"],
    :grp => [
      {"BRKPA"   => {"op" => 0, "S" => 0, "B" => 0}},
      {"BRKPB"   => {"op" => 0, "S" => 0, "B" => 1}},
      {"BRKPAS"  => {"op" => 0, "S" => 1, "B" => 0}},
      {"BRKPBS"  => {"op" => 0, "S" => 1, "B" => 1}}
    ]
  },

  "SvePartitionBreakCond" => {
    :cmt => "SVE partition break condition",
    :arg => [
      [ {"pd" => "PRegB"}, {"pg" => "_PReg"}, {"pn" => "PRegB"}] #0
    ],
    :prm => ["B", "S", "pd", "pg", "pn"],
    :grp => [
      {"BRKA"   => {"B" => 0, "S" => 0}},
      {"BRKAS"  => {"B" => 0, "S" => 1}},
      {"BRKB"   => {"B" => 1, "S" => 0}},
      {"BRKBS"  => {"B" => 1, "S" => 1}}
    ]
  },

  "SvePropagateBreakNextPart" => {
    :cmt => "SVE propagate break to next partition",
    :arg => [
      [ {"pdm" => "PRegB"}, {"pg" => "_PReg"}, {"pn" => "PRegB"}] #0
    ],
    :prm => ["S", "pdm", "pg", "pn"],
    :grp => [
      {"BRKN"   => {"S" => 0}},
      {"BRKNS"  => {"S" => 1}}
    ]
  },

  "SvePredFirstAct" => {
    :cmt => "SVE predicate first active",
    :arg => [[ {"pdn" => "PRegB"}, {"pg" => "_PReg"}]],
    :prm => ["op", "S", "pdn", "pg"],
    :grp => [
      {"PFIRST"  => {"op" => 0, "S" => 1}},
    ]
  },

  "SvePredInit" => {
    :cmt => "SVE predicate initialize",
    :arg => ext_args(preg_set(1, "BHSD"), [{"pat=ALL" => "Pattern"}]),
    # :arg => [[ {"pd" => "_PReg"}, {"pattern" => "uint32_t"}]],
    :prm => ["S", "pd", "pat"],
    :grp => [
      {"PTRUE"  => {"S" => 0}},
      {"PTRUES" => {"S" => 1}}
    ]
  },

  "SvePredNextAct" => {
    :cmt => "SVE predicate next active",
    :arg => ext_args(preg_set_pred(1, "BHSD"), [], {"pd"=>"pdn"}),
    # :arg => [[ {"pdn" => "_PReg"}, {"pg" => "_PReg"}]],
    :prm => ["pdn", "pg"],
    :grp => [
      {"PNEXT"  => {}}
    ]
  },

  "SvePredReadFFRPred" => {
    :cmt => "SVE predicate read from FFR (predicate)",
    :arg => [[ {"pd" => "PRegB"}, {"pg" => "_PReg"}]],
    :prm => ["op", "S", "pd", "pg"],
    :grp => [
      {"RDFFR"  => {"op" => 0, "S" => 0}},
      {"RDFFRS" => {"op" => 0, "S" => 1}}
    ]
  },

  "SvePredReadFFRUnpred" => {
    :cmt => "SVE predicate read from FFR (unpredicate)",
    :arg => [[ {"pd" => "PRegB"}]],
    :prm => ["op", "S", "pd"],
    :grp => [
      {"RDFFR"  => {"op" => 0, "S" => 0}}
    ]
  },

  "SvePredTest" => {
    :cmt => "SVE predicate test",
    :arg => [[ {"pg" => "_PReg"}, {"pn" => "PRegB"}]],
    :prm => ["op", "S", "opc2", "pg", "pn"],
    :grp => [
      {"PTEST"  => {"op" => 0, "S" => 1, "opc2" => 0}}
    ]
  },

  "SvePredZero" => {
    :cmt => "SVE predicate zero",
    :arg => [[ {"pd" => "PRegB"}]],
    :prm => ["op", "S", "pd"],
    :grp => [
      {"PFALSE"  => {"op" => 0, "S" => 0}}
    ]
  },

  "SveIntCompSImm" => {
    :cmt => "SVE integer compare with signed immediate",
    :arg => [
      [ {"pd" => "PRegB"}, {"pg" => "_PReg"}, {"zn" => "ZRegB"}, {"imm" => "int32_t"}], #0
      [ {"pd" => "PRegH"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}, {"imm" => "int32_t"}], #1
      [ {"pd" => "PRegS"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}, {"imm" => "int32_t"}], #2
      [ {"pd" => "PRegD"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}, {"imm" => "int32_t"}]  #3
    ],
    :prm => ["op", "o2", "ne", "pd", "pg", "zn", "imm"],
    :grp => [
      {"CMPGE"  => {"op" => 0, "o2" => 0, "ne" => 0}},
      {"CMPGT"  => {"op" => 0, "o2" => 0, "ne" => 1}},
      {"CMPLT"  => {"op" => 0, "o2" => 1, "ne" => 0}},
      {"CMPLE"  => {"op" => 0, "o2" => 1, "ne" => 1}},
      {"CMPEQ"  => {"op" => 1, "o2" => 0, "ne" => 0}},
      {"CMPNE"  => {"op" => 1, "o2" => 0, "ne" => 1}}
    ]
  },

  "SvePredCount" => {
    :cmt => "SVE predicate count",
    :arg => [
      [ {"rd" => "XReg"}, {"pg" => "_PReg"}, {"pn" => "PRegB"}], #0
      [ {"rd" => "XReg"}, {"pg" => "_PReg"}, {"pn" => "PRegH"}], #1
      [ {"rd" => "XReg"}, {"pg" => "_PReg"}, {"pn" => "PRegS"}], #2
      [ {"rd" => "XReg"}, {"pg" => "_PReg"}, {"pn" => "PRegD"}]  #3
    ],
    :prm => ["opc", "o2", "rd", "pg", "pn"],
    :grp => [
      {"CNTP"  => {"opc" => 0, "o2" => 0}}
    ]
  },

  "SveIncDecRegByPredCount" => {
    :cmt => "SVE inc/dec register by predicate count",
    :arg => [
      [ {"xdn" => "XReg"}, {"pg" => "PRegB"}], #0
      [ {"xdn" => "XReg"}, {"pg" => "PRegH"}], #1
      [ {"xdn" => "XReg"}, {"pg" => "PRegS"}], #2
      [ {"xdn" => "XReg"}, {"pg" => "PRegD"}]  #3
    ],
    :prm => ["op", "D", "opc2", "xdn", "pg"],
    :grp => [
      {"INCP"  => {"op" => 0, "D" => 0, "opc2" => 0}},
      {"DECP"  => {"op" => 0, "D" => 1, "opc2" => 0}}
    ]
  },

  "SveIncDecVecByPredCount" => {
    :cmt => "SVE inc/dec vector by predicate count",
    :arg => [
      [ {"zdn" => "ZRegH"}, {"pg" => "_PReg"}], #0
      [ {"zdn" => "ZRegS"}, {"pg" => "_PReg"}], #1
      [ {"zdn" => "ZRegD"}, {"pg" => "_PReg"}]  #2
    ],
    :prm => ["op", "D", "opc2", "zdn", "pg"],
    :grp => [
      {"INCP"  => {"op" => 0, "D" => 0, "opc2" => 0}},
      {"DECP"  => {"op" => 0, "D" => 1, "opc2" => 0}}
    ]
  },

  "SveSatuIncDecRegByPredCount" => {
    :cmt => "SVE saturating inc/dec register by predicate count",
    :arg => [
      [ {"rdn" => "WReg"}, {"pg" => "PRegB"}], #0
      [ {"rdn" => "WReg"}, {"pg" => "PRegH"}], #1
      [ {"rdn" => "WReg"}, {"pg" => "PRegS"}], #2
      [ {"rdn" => "WReg"}, {"pg" => "PRegD"}], #3
      [ {"rdn" => "XReg"}, {"pg" => "PRegB"}], #4
      [ {"rdn" => "XReg"}, {"pg" => "PRegH"}], #5
      [ {"rdn" => "XReg"}, {"pg" => "PRegS"}], #6
      [ {"rdn" => "XReg"}, {"pg" => "PRegD"}]  #7
    ],
    :prm => ["D", "U", "op", "rdn", "pg"],
    :grp => [
      {"SQINCP"  => {"D" => 0, "U" => 0, "op" => 0}},
      {"UQINCP"  => {"D" => 0, "U" => 1, "op" => 0}},
      {"SQDECP"  => {"D" => 1, "U" => 0, "op" => 0}},
      {"UQDECP"  => {"D" => 1, "U" => 1, "op" => 0}},
    ]
  },
  
  "SveSatuIncDecVecByPredCount" => {
    :cmt => "SVE saturating inc/dec vector by predicate count",
    :arg => [
      [ {"zdn" => "ZRegH"}, {"pg" => "_PReg"}], #0
      [ {"zdn" => "ZRegS"}, {"pg" => "_PReg"}], #1
      [ {"zdn" => "ZRegD"}, {"pg" => "_PReg"}]  #2
    ],
    :prm => ["D", "U", "opc", "zdn", "pg"],
    :grp => [
      {"SQINCP"  => {"D" => 0, "U" => 0, "opc" => 0}},
      {"UQINCP"  => {"D" => 0, "U" => 1, "opc" => 0}},
      {"SQDECP"  => {"D" => 1, "U" => 0, "opc" => 0}},
      {"UQDECP"  => {"D" => 1, "U" => 1, "opc" => 0}}
    ]
  },

  "SveFFRInit" => {
    :cmt => "SVE FFR initialise",
    :arg => [[]],
    :prm => ["opc"],
    :grp => [
      {"SETFFR"  => {"opc" => 0}},
    ]
  },

  "SveFFRWritePred" => {
    :cmt => "SVE FFR write from predicate",
    :arg => [[ {"pn" => "PRegB"}]],
    :prm => ["opc", "pn"],
    :grp => [
      {"WRFFR"  => {"opc" => 0}},
    ]
  },

  "SveCondTermScalars" => {
    :cmt => "SVE conditionally terminate scalars",
    :arg => [
      [ {"rn" => "WReg"}, {"rm" => "WReg"}], #0
      [ {"rn" => "XReg"}, {"rm" => "XReg"}]  #1
    ],
    :prm => ["op", "ne", "rn", "rm"],
    :grp => [
      {"CTERMEQ"  => {"op" => 1, "ne" => 0}},
      {"CTERMNE"  => {"op" => 1, "ne" => 1}}
    ]
  },

  "SveIntCompScalarCountAndLimit" => {
    :cmt => "SVE integer compare scalar count and limit",
    :arg => [
      [ {"pd" => "PRegB"}, {"rn" => "WReg"}, {"rm" => "WReg"}], #0
      [ {"pd" => "PRegH"}, {"rn" => "WReg"}, {"rm" => "WReg"}], #1
      [ {"pd" => "PRegS"}, {"rn" => "WReg"}, {"rm" => "WReg"}], #2
      [ {"pd" => "PRegD"}, {"rn" => "WReg"}, {"rm" => "WReg"}], #3
      [ {"pd" => "PRegB"}, {"rn" => "XReg"}, {"rm" => "XReg"}], #4
      [ {"pd" => "PRegH"}, {"rn" => "XReg"}, {"rm" => "XReg"}], #5
      [ {"pd" => "PRegS"}, {"rn" => "XReg"}, {"rm" => "XReg"}], #6
      [ {"pd" => "PRegD"}, {"rn" => "XReg"}, {"rm" => "XReg"}]  #7
    ],
    :prm => ["U", "lt", "eq", "pd", "rn", "rm"],
    :grp => [
      {"WHILELT"  => {"U" => 0, "lt" => 1, "eq" => 0}},
      {"WHILELE"  => {"U" => 0, "lt" => 1, "eq" => 1}},
      {"WHILELO"  => {"U" => 1, "lt" => 1, "eq" => 0}},
      {"WHILELS"  => {"U" => 1, "lt" => 1, "eq" => 1}}
    ]
  },

  "SveBcFpImmUnpred" => {
    :cmt => "SVE broadcast floating-point immediate (unpredicated)",
    :arg => ext_args(zreg_set(1, "HSD"), [{"imm" => "double"}]),
    # :arg => [[ {"zd" => "ZReg"}, {"imm" => "uint32_t"}]],
    :prm => ["opc", "o2", "zd", "imm"],
    :grp => [
      {"FDUP"  => {"opc" => 0, "o2" => 0}},
      {"FMOV"  => {"opc" => 0, "o2" => 0}}  # alias of FDUP
    ]
  },

  "SveBcIntImmUnpred" => {
    :cmt => "SVE broadcast integer immediate (unpredicated)",
    :arg => ext_args(zreg_set(1, "BHSD"), [{"imm" => "int32_t"}, {"mod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}]) + # 0-3
            ext_args(zreg_set(1, "BHSD"), [{"imm=0.0" => "float"}]),                      # 4-7
    :prm => ["opc", "zd", "imm", "mod", "sh"],
    :grp => [
      {"DUP"  => {"opc" => 0},                            :arg => 0..3},
      {"MOV"  => {"opc" => 0},                            :arg => 0..3}, # alias of DUP
      {"FMOV" => {"opc" => 0, "mod" => "LSL", "sh" => 0, "imm" => "static_cast<uint32_t>(imm)"}, :arg => 4..7}  # alias of DUP
    ]
  },
  
  "SveIntAddSubImmUnpred" => {
    :cmt => "SVE integer add/subtract immediate (unpredicated)",
    :arg => ext_args(zreg_set(1, "BHSD"), [{"imm" => "uint32_t"}, {"mod=LSL" => "ShMod"}, {"sh=0" => "uint32_t"}], {"zd"=>"zdn"}),
    :prm => ["opc", "zdn", "imm", "mod", "sh"],
    :grp => [
      {"ADD"   => {"opc" => 0}},
      {"SUB"   => {"opc" => 1}},
      {"SUBR"  => {"opc" => 3}},
      {"SQADD" => {"opc" => 4}},
      {"UQADD" => {"opc" => 5}},
      {"SQSUB" => {"opc" => 6}},
      {"UQSUB" => {"opc" => 7}}
    ]
  },

  "SveIntMinMaxImmUnpred" => {
    :cmt => "SVE integer min/max immediate (unpredicated)",
    :arg => ext_args(zreg_set(1, "BHSD"), [{"imm" => "int32_t"}], {"zd"=>"zdn"}),
    :prm => ["opc", "o2", "zdn", "imm"],
    :grp => [
      {"SMAX" => {"opc" => 0, "o2" => 0}},
      {"UMAX" => {"opc" => 1, "o2" => 0}},
      {"SMIN" => {"opc" => 2, "o2" => 0}},
      {"UMIN" => {"opc" => 3, "o2" => 0}}
    ]
  },

  "SveIntMultImmUnpred" => {
    :cmt => "SVE integer multiply immediate (unpredicated)",
    :arg => ext_args(zreg_set(1, "BHSD"), [{"imm" => "int32_t"}], {"zd"=>"zdn"}),
    :prm => ["opc", "o2", "zdn", "imm"],
    :grp => [
      {"MUL" => {"opc" => 0, "o2" => 0}}
    ]
  },

  "SveIntDotProductUnpred" => {
    :cmt => "SVE integer dot product (unpredicated)",
    :arg => [
      [ {"zda" => "ZRegS"}, {"zn" => "ZRegB"}, {"zm" => "ZRegB"}], #0
      [ {"zda" => "ZRegD"}, {"zn" => "ZRegH"}, {"zm" => "ZRegH"}]  #1
    ],
    :prm => ["U", "zda", "zn", "zm"],
    :grp => [
      {"SDOT" => {"U" => 0}},
      {"UDOT" => {"U" => 1}}
    ]
  },

  "SveIntDotProductIndexed" => {
    :cmt => "SVE integer dot product (indexed)",
    :arg => [
      [ {"zda" => "ZRegS"}, {"zn" => "ZRegB"}, {"zm" => "ZRegBElem"}], #0
      [ {"zda" => "ZRegD"}, {"zn" => "ZRegH"}, {"zm" => "ZRegHElem"}]  #1
    ],
    :prm => ["size", "U", "zda", "zn", "zm"],
    :grp => [
      {"SDOT" => {"size" => 2, "U" => 0}, :arg => [0]},
      {"UDOT" => {"size" => 2, "U" => 1}, :arg => [0]},
      {"SDOT" => {"size" => 3, "U" => 0}, :arg => [1]},
      {"UDOT" => {"size" => 3, "U" => 1}, :arg => [1]}
    ]
  },

  "SveFpComplexAddPred" => {
    :cmt => "SVE floating-point complex add (predicated)",
    :arg => ext_args(zreg_set_pred(2 ,"HSD"), [{"ct" => "uint32_t"}], {"zd"=>"zdn","zn"=>"zm"}),
    # :arg => [[ {"zdn" => "ZReg"}, {"pg" => "_PReg"}, {"zm" => "ZReg"},  {"ct" => "uint32_t"}]],
    :prm => ["zdn", "pg", "zm", "ct"],
    :grp => [
      {"FCADD" => {}}
    ]
  },

  "SveFpComplexMultAddPred" => {
    :cmt => "SVE floating-point complex multiply-add (predicated)",
    :arg => ext_args(zreg_set_pred(3 ,"HSD"), [{"ct" => "uint32_t"}], {"zd"=>"zda"}),
    # :arg => [[ {"zda" => "ZReg"}, {"pg" => "_PReg"}, {"zn" => "ZReg"}, {"zm" => "ZReg"},  {"ct" => "uint32_t"}]],
    :prm => ["zda", "pg", "zn", "zm", "ct"],
    :grp => [
      {"FCMLA" => {}}
    ]
  },

  "SveFpMultAddIndexed" => {
    :cmt => "SVE floating-point multiply-add (indexed)",
    :arg => [
      [ {"zda" => "ZRegH"}, {"zn" => "ZRegH"}, {"zm" => "ZRegHElem"} ], #0
      [ {"zda" => "ZRegS"}, {"zn" => "ZRegS"}, {"zm" => "ZRegSElem"} ], #1
      [ {"zda" => "ZRegD"}, {"zn" => "ZRegD"}, {"zm" => "ZRegDElem"} ]  #2
    ],
    :prm => ["op", "zda", "zn", "zm"],
    :grp => [
      {"FMLA" => {"op" => 0}},
      {"FMLS" => {"op" => 1}}
    ]
  },

  "SveFpComplexMultAddIndexed" => {
    :cmt => "SVE floating-point complex multiply-add (indexed)",
    :arg => [
      [ {"zda" => "ZRegH"}, {"zn" => "ZRegH"}, {"zm" => "ZRegHElem"}, {"ct" => "uint32_t"} ], #0
      [ {"zda" => "ZRegS"}, {"zn" => "ZRegS"}, {"zm" => "ZRegSElem"}, {"ct" => "uint32_t"} ], #1
    ],
    :prm => ["zda", "zn", "zm", "ct"],
    :grp => [
      {"FCMLA" => {}}
    ]
  },

  "SveFpMultIndexed" => {
    :cmt => "SVE floating-point multiply (indexed)",
    :arg => [
      [ {"zd" => "ZRegH"}, {"zn" => "ZRegH"}, {"zm" => "ZRegHElem"} ], #0
      [ {"zd" => "ZRegS"}, {"zn" => "ZRegS"}, {"zm" => "ZRegSElem"} ], #1
      [ {"zd" => "ZRegD"}, {"zn" => "ZRegD"}, {"zm" => "ZRegDElem"} ]  #2
    ],
    :prm => ["zd", "zn", "zm"],
    :grp => [
      {"FMUL" => {}}
    ]
  },
  
  "SveFpRecurReduct" => {
    :cmt => "SVE floating-point recursive reduction",
    :arg => [
      [ {"vd" => "HReg"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}], #0
      [ {"vd" => "SReg"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}], #1
      [ {"vd" => "DReg"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}]  #2
    ],
    :prm => ["opc", "vd", "pg", "zn"],
    :grp => [
      {"FADDV"   => {"opc" => 0}},
      {"FMAXNMV" => {"opc" => 4}},
      {"FMINNMV" => {"opc" => 5}},
      {"FMAXV"   => {"opc" => 6}},
      {"FMINV"   => {"opc" => 7}}
    ]
  },

  "SveFpReciproEstUnPred" => {
    :cmt => "SVE floating-point reciprocal estimate unpredicated",
    :arg => zreg_set(2, "HSD"),
    # :arg => [[ {"zd" => "ZReg"}, {"zn" => "ZReg"}]],
    :prm => ["opc", "zd", "zn"],
    :grp => [
      {"FRECPE"  => {"opc" => 6}},
      {"FRSQRTE" => {"opc" => 7}}
    ]
  },

  "SveFpCompWithZero" => {
    :cmt => "SVE floating-point compare with zero",
    :arg => [
      [ {"pd" => "PRegH"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}, {"zero" => "double"}], #0
      [ {"pd" => "PRegS"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}, {"zero" => "double"}], #1
      [ {"pd" => "PRegD"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}, {"zero" => "double"}]  #2
    ],
    :prm => ["eq", "lt", "ne", "pd", "pg", "zn", "zero"],
    :grp => [
      {"FCMGE"  => {"eq" => 0, "lt" => 0, "ne" => 0}},
      {"FCMGT"  => {"eq" => 0, "lt" => 0, "ne" => 1}},
      {"FCMLT"  => {"eq" => 0, "lt" => 1, "ne" => 0}},
      {"FCMLE"  => {"eq" => 0, "lt" => 1, "ne" => 1}},
      {"FCMEQ"  => {"eq" => 1, "lt" => 0, "ne" => 0}},
      {"FCMNE"  => {"eq" => 1, "lt" => 1, "ne" => 0}}
    ]
  },

  "SveFpSerialReductPred" => {
    :cmt => "SVE floating-point serial resuction (predicated)",
    :arg => [
      [ {"vdn" => "HReg"}, {"pg" => "_PReg"}, {"zm" => "ZRegH"}], #0
      [ {"vdn" => "SReg"}, {"pg" => "_PReg"}, {"zm" => "ZRegS"}], #1
      [ {"vdn" => "DReg"}, {"pg" => "_PReg"}, {"zm" => "ZRegD"}]  #2
    ],
    :prm => ["opc", "vdn", "pg", "zm"],
    :grp => [
      {"FADDA"  => {"opc" => 0}}
    ]
  },

  "SveFpArithmeticUnpred" => {
    :cmt => "SVE floating-point arithmetic (unpredicated)",
    :arg => zreg_set(3, "HSD"),
    # :arg => [[ {"zd" => "ZReg"}, {"zn" => "ZReg"}, {"zm" => "ZReg"}]],
    :prm => ["opc", "zd", "zn", "zm"],
    :grp => [
      {"FADD"    => {"opc" => 0}},
      {"FSUB"    => {"opc" => 1}},
      {"FMUL"    => {"opc" => 2}},
      {"FTSMUL"  => {"opc" => 3}},
      {"FRECPS"  => {"opc" => 6}},
      {"FRSQRTS" => {"opc" => 7}}
    ]
  },

  "SveFpArithmeticPred" => {
    :cmt => "SVE floating-point arithmetic (predicated)",
    :arg => ext_args(zreg_set_pred(2,"HSD"),[],{"zd"=>"zdn", "zn" => "zm"}),
    # :arg => [[ {"zdn" => "ZReg"}, {"pg" => "_PReg"}, {"zm" => "ZReg"}]],
    :prm => ["opc", "zdn", "pg", "zm"],
    :grp => [
      {"FADD"   => {"opc" => 0x0}},
      {"FSUB"   => {"opc" => 0x1}},
      {"FMUL"   => {"opc" => 0x2}},
      {"FSUBR"  => {"opc" => 0x3}},
      {"FMAXNM" => {"opc" => 0x4}},
      {"FMINNM" => {"opc" => 0x5}},
      {"FMAX"   => {"opc" => 0x6}},
      {"FMIN"   => {"opc" => 0x7}},
      {"FABD"   => {"opc" => 0x8}},
      {"FSCALE" => {"opc" => 0x9}},
      {"FMULX"  => {"opc" => 0xa}},
      {"FDIVR"  => {"opc" => 0xc}},
      {"FDIV"   => {"opc" => 0xd}}
    ]
  },

  "SveFpArithmeticImmPred" => {
    :cmt => "SVE floating-point arithmetic with immediate (predicated)",
    :arg => ext_args(zreg_set_pred(1,"HSD"),[{"ct" => "float"}],{"zd"=>"zdn"}),
    # :arg => [[ {"zdn" => "ZReg"}, {"pg" => "_PReg"}, {"ct" => "float"}]],
    :prm => ["opc", "zdn", "pg", "ct"],
    :grp => [
      {"FADD"   => {"opc" => 0x0}},
      {"FSUB"   => {"opc" => 0x1}},
      {"FMUL"   => {"opc" => 0x2}},
      {"FSUBR"  => {"opc" => 0x3}},
      {"FMAXNM" => {"opc" => 0x4}},
      {"FMINNM" => {"opc" => 0x5}},
      {"FMAX"   => {"opc" => 0x6}},
      {"FMIN"   => {"opc" => 0x7}}
    ]
  },

  "SveFpTrigMultAddCoef" => {
    :cmt => "SVE floating-point trig multiply-add coefficient",
    :arg => ext_args(zreg_set(2,"HSD"),[{"imm" => "uint32_t"}],{"zd"=>"zdn", "zn"=>"zm"}),
    # :arg => [[ {"zdn" => "ZReg"}, {"zm" => "ZReg"}, {"imm" => "uint32_t"}]],
    :prm => ["zdn", "zm", "imm"],
    :grp => [
      {"FTMAD"   => {}}
    ]
  },

  "SveFpCvtPrecision" => {
    :cmt => "SVE floating-point convert precision",
    :arg => [
      [ {"zd" => "ZRegH"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}], #0
      [ {"zd" => "ZRegS"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}], #1
      [ {"zd" => "ZRegH"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}], #2
      [ {"zd" => "ZRegD"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}], #3
      [ {"zd" => "ZRegS"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}], #4
      [ {"zd" => "ZRegD"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}]  #5
    ],
    :prm => ["opc", "opc2", "zd", "pg", "zn"],
    :grp => [
      {"FCVT"   => {"opc" => 2, "opc2" => 0}, :arg => [0]},
      {"FCVT"   => {"opc" => 2, "opc2" => 1}, :arg => [1]},
      {"FCVT"   => {"opc" => 3, "opc2" => 0}, :arg => [2]},
      {"FCVT"   => {"opc" => 3, "opc2" => 1}, :arg => [3]},
      {"FCVT"   => {"opc" => 3, "opc2" => 2}, :arg => [4]},
      {"FCVT"   => {"opc" => 3, "opc2" => 3}, :arg => [5]}
    ]
  },

  "SveFpCvtToInt" => {
    :cmt => "SVE floating-point convert to integer",
    :arg => [
      [ {"zd" => "ZRegH"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}], #0
      [ {"zd" => "ZRegS"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}], #1
      [ {"zd" => "ZRegD"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}], #2
      [ {"zd" => "ZRegS"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}], #3
      [ {"zd" => "ZRegS"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}], #4
      [ {"zd" => "ZRegD"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}], #5
      [ {"zd" => "ZRegD"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}], #6
    ],
    :prm => ["opc", "opc2", "U", "zd", "pg", "zn"],
    :grp => [
      {"FCVTZS"   => {"opc" => 1, "opc2" => 1, "U" => 0}, :arg => [0]},
      {"FCVTZU"   => {"opc" => 1, "opc2" => 1, "U" => 1}, :arg => [0]},
      {"FCVTZS"   => {"opc" => 1, "opc2" => 2, "U" => 0}, :arg => [1]},
      {"FCVTZU"   => {"opc" => 1, "opc2" => 2, "U" => 1}, :arg => [1]},
      {"FCVTZS"   => {"opc" => 1, "opc2" => 3, "U" => 0}, :arg => [2]},
      {"FCVTZU"   => {"opc" => 1, "opc2" => 3, "U" => 1}, :arg => [2]},
      {"FCVTZS"   => {"opc" => 2, "opc2" => 2, "U" => 0}, :arg => [3]},
      {"FCVTZU"   => {"opc" => 2, "opc2" => 2, "U" => 1}, :arg => [3]},
      {"FCVTZS"   => {"opc" => 3, "opc2" => 0, "U" => 0}, :arg => [4]},
      {"FCVTZU"   => {"opc" => 3, "opc2" => 0, "U" => 1}, :arg => [4]},
      {"FCVTZS"   => {"opc" => 3, "opc2" => 2, "U" => 0}, :arg => [5]},
      {"FCVTZU"   => {"opc" => 3, "opc2" => 2, "U" => 1}, :arg => [5]},
      {"FCVTZS"   => {"opc" => 3, "opc2" => 3, "U" => 0}, :arg => [6]},
      {"FCVTZU"   => {"opc" => 3, "opc2" => 3, "U" => 1}, :arg => [6]}
    ]
  },

  "SveFpRoundToIntegral" => {
    :cmt => "SVE floating-point round to integral value",
    :arg => zreg_set_pred(2, "HSD"),
    # :arg => [[ {"zd" => "ZReg"}, {"pg" => "_PReg"}, {"zn" => "ZReg"}]],
    :prm => ["opc", "zd", "pg", "zn"],
    :grp => [
      {"FRINTN" => {"opc" => 0}},
      {"FRINTP" => {"opc" => 1}},
      {"FRINTM" => {"opc" => 2}},
      {"FRINTZ" => {"opc" => 3}},
      {"FRINTA" => {"opc" => 4}},
      {"FRINTX" => {"opc" => 6}},
      {"FRINTI" => {"opc" => 7}}
    ]
  },

  "SveFpUnaryOp" => {
    :cmt => "SVE floating-point unary operations",
    :arg => zreg_set_pred(2, "HSD"),
    # :arg => [[ {"zd" => "ZReg"}, {"pg" => "_PReg"}, {"zn" => "ZReg"}]],
    :prm => ["opc", "zd", "pg", "zn"],
    :grp => [
      {"FRECPX" => {"opc" => 0}},
      {"FSQRT"  => {"opc" => 1}}
    ]
  },

  "SveIntCvtToFp" => {
    :cmt => "SVE integer convert to floationg-point",
    :arg => [
      [ {"zd" => "ZRegH"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}], #0
      [ {"zd" => "ZRegH"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}], #1
      [ {"zd" => "ZRegH"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}], #2
      [ {"zd" => "ZRegS"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}], #3
      [ {"zd" => "ZRegD"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}], #4
      [ {"zd" => "ZRegS"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}], #5
      [ {"zd" => "ZRegD"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}], #6
    ],
    :prm => ["opc", "opc2", "U", "zd", "pg", "zn"],
    :grp => [
      {"SCVTF" => {"opc" => 1, "opc2" => 1, "U" => 0}, :arg => [0]},
      {"UCVTF" => {"opc" => 1, "opc2" => 1, "U" => 1}, :arg => [0]},
      {"SCVTF" => {"opc" => 1, "opc2" => 2, "U" => 0}, :arg => [1]},
      {"UCVTF" => {"opc" => 1, "opc2" => 2, "U" => 1}, :arg => [1]},
      {"SCVTF" => {"opc" => 1, "opc2" => 3, "U" => 0}, :arg => [2]},
      {"UCVTF" => {"opc" => 1, "opc2" => 3, "U" => 1}, :arg => [2]},
      {"SCVTF" => {"opc" => 2, "opc2" => 2, "U" => 0}, :arg => [3]},
      {"UCVTF" => {"opc" => 2, "opc2" => 2, "U" => 1}, :arg => [3]},
      {"SCVTF" => {"opc" => 3, "opc2" => 0, "U" => 0}, :arg => [4]},
      {"UCVTF" => {"opc" => 3, "opc2" => 0, "U" => 1}, :arg => [4]},
      {"SCVTF" => {"opc" => 3, "opc2" => 2, "U" => 0}, :arg => [5]},
      {"UCVTF" => {"opc" => 3, "opc2" => 2, "U" => 1}, :arg => [5]},
      {"SCVTF" => {"opc" => 3, "opc2" => 3, "U" => 0}, :arg => [6]},
      {"UCVTF" => {"opc" => 3, "opc2" => 3, "U" => 1}, :arg => [6]}
    ]
  },

  "SveFpCompVec" => {
    :cmt => "SVE floationg-point compare vectors",
    :arg => [
      [ {"pd" => "PRegH"}, {"pg" => "_PReg"}, {"zn" => "ZRegH"}, {"zm" => "ZRegH"}], #0
      [ {"pd" => "PRegS"}, {"pg" => "_PReg"}, {"zn" => "ZRegS"}, {"zm" => "ZRegS"}], #1
      [ {"pd" => "PRegD"}, {"pg" => "_PReg"}, {"zn" => "ZRegD"}, {"zm" => "ZRegD"}], #2
    ],
    :prm => ["op", "o2", "o3", "pd", "pg", "zn", "zm"],
    :grp => [
      {"FCMGE" => {"op" => 0, "o2" => 0, "o3" => 0}},
      {"FCMGT" => {"op" => 0, "o2" => 0, "o3" => 1}},
      {"FCMLE" => {"op" => 0, "o2" => 0, "o3" => 0, "zn" => "zm", "zm" => "zn"}}, # alias of FCMGE
      {"FCMLT" => {"op" => 0, "o2" => 0, "o3" => 1, "zn" => "zm", "zm" => "zn"}}, # alias of FCMGT
      {"FCMEQ" => {"op" => 0, "o2" => 1, "o3" => 0}},
      {"FCMNE" => {"op" => 0, "o2" => 1, "o3" => 1}},
      {"FCMUO" => {"op" => 1, "o2" => 0, "o3" => 0}},
      {"FACGE" => {"op" => 1, "o2" => 0, "o3" => 1}},
      {"FACGT" => {"op" => 1, "o2" => 1, "o3" => 1}},
      {"FACLE" => {"op" => 1, "o2" => 0, "o3" => 1, "zn" => "zm", "zm" => "zn"}}, # alias of FACGE
      {"FACLT" => {"op" => 1, "o2" => 1, "o3" => 1, "zn" => "zm", "zm" => "zn"}}  # alias of FACGT
    ]
  },
  
  "SveFpMultAccumAddend" => {
    :cmt => "SVE floationg-point multiply-accumulate writing addend",
    :arg => ext_args(zreg_set_pred(3, "HSD"), [], {"zd"=>"zda"}),
    # :arg => [[ {"zda" => "ZReg"}, {"pg" => "_PReg"}, {"zn" => "ZReg"}, {"zm" => "ZReg"}]],
    :prm => ["opc", "zda", "pg", "zn", "zm"],
    :grp => [
      {"FMLA"  => {"opc" => 0}},
      {"FMLS"  => {"opc" => 1}},
      {"FNMLA" => {"opc" => 2}},
      {"FNMLS" => {"opc" => 3}}
    ]
  },

  "SveFpMultAccumMulti" => {
    :cmt => "SVE floationg-point multiply-accumulate writing multiplicand",
    :arg => ext_args(zreg_set_pred(3, "HSD"), [], {"zd"=>"zdn", "zm" => "za", "zn" => "zm"}),
    # :arg => [[ {"zdn" => "ZReg"}, {"pg" => "_PReg"}, {"zm" => "ZReg"}, {"za" => "ZReg"}]],
    :prm => ["opc", "zdn", "pg", "zm", "za"],
    :grp => [
      {"FMAD"  => {"opc" => 0}},
      {"FMSB"  => {"opc" => 1}},
      {"FNMAD" => {"opc" => 2}},
      {"FNMSB" => {"opc" => 3}}
    ]
  },

  "Sve32GatherLdSc32U" => {
    :cmt => "SVE 32-bit gather load (scalar plus 32-bit unscaled offsets)",
    :arg => [[ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrSc32U"}]],
    :prm => ["msz", "U", "ff", "zt", "pg", "adr"],
    :grp => [
      {"LD1SB"   => {"msz" => 0, "U" => 0, "ff" => 0}},
      {"LDFF1SB" => {"msz" => 0, "U" => 0, "ff" => 1}},
      {"LD1B"    => {"msz" => 0, "U" => 1, "ff" => 0}},
      {"LDFF1B"  => {"msz" => 0, "U" => 1, "ff" => 1}},
      {"LD1SH"   => {"msz" => 1, "U" => 0, "ff" => 0}},
      {"LDFF1SH" => {"msz" => 1, "U" => 0, "ff" => 1}},
      {"LD1H"    => {"msz" => 1, "U" => 1, "ff" => 0}},
      {"LDFF1H"  => {"msz" => 1, "U" => 1, "ff" => 1}},
      {"LD1W"    => {"msz" => 2, "U" => 1, "ff" => 0}},
      {"LDFF1W"  => {"msz" => 2, "U" => 1, "ff" => 1}}
    ]
  },

  "Sve32GatherLdVecImm" => {
    :cmt => "SVE 32-bit gather load (vector plus immediate)",
    :arg => [[ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrVecImm32"}]],
    :prm => ["msz", "U", "ff", "zt", "pg", "adr"],
    :grp => [
      {"LD1SB"   => {"msz" => 0, "U" => 0, "ff" => 0}},
      {"LDFF1SB" => {"msz" => 0, "U" => 0, "ff" => 1}},
      {"LD1B"    => {"msz" => 0, "U" => 1, "ff" => 0}},
      {"LDFF1B"  => {"msz" => 0, "U" => 1, "ff" => 1}},
      {"LD1SH"   => {"msz" => 1, "U" => 0, "ff" => 0}},
      {"LDFF1SH" => {"msz" => 1, "U" => 0, "ff" => 1}},
      {"LD1H"    => {"msz" => 1, "U" => 1, "ff" => 0}},
      {"LDFF1H"  => {"msz" => 1, "U" => 1, "ff" => 1}},
      {"LD1W"    => {"msz" => 2, "U" => 1, "ff" => 0}},
      {"LDFF1W"  => {"msz" => 2, "U" => 1, "ff" => 1}}
    ]
  },

  "Sve32GatherLdHSc32S" => {
    :cmt => "SVE 32-bit gather load halfwords (scalar plus 32-bit scaled offsets)",
    :arg => [[ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrSc32S"}]],
    :prm => ["U", "ff", "zt", "pg", "adr"],
    :grp => [
      {"LD1SH"   => {"U" => 0, "ff" => 0}},
      {"LDFF1SH" => {"U" => 0, "ff" => 1}},
      {"LD1H"    => {"U" => 1, "ff" => 0}},
      {"LDFF1H"  => {"U" => 1, "ff" => 1}}
    ]
  },
  
  "Sve32GatherLdWSc32S" => {
    :cmt => "SVE 32-bit gather load words (scalar plus 32-bit scaled offsets)",
    :arg => [[ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrSc32S"}]],
    :prm => ["U", "ff", "zt", "pg", "adr"],
    :grp => [
      {"LD1W"   => {"U" => 1, "ff" => 0}},
      {"LDFF1W" => {"U" => 1, "ff" => 1}},
    ]
  },

  "Sve32GatherPfSc32S" => {
    :cmt => "SVE 32-bit gather prefetch (scalar plus 32-bit scaled offsets)",
    :arg => [[ {"prfop_sve" => "PrfopSve"}, {"pg" => "_PReg"}, {"adr" => "AdrSc32S"}]],
    :prm => ["prfop_sve", "msz", "pg", "adr"],
    :grp => [
      {"PRFB"   => {"msz" => 0}},
      {"PRFH"   => {"msz" => 1}},
      {"PRFW"   => {"msz" => 2}},
      {"PRFD"   => {"msz" => 3}}
    ]
  },

  "Sve32GatherPfVecImm" => {
    :cmt => "SVE 32-bit gather prefetch (vector plus immediate)",
    :arg => [[ {"prfop_sve" => "PrfopSve"}, {"pg" => "_PReg"}, {"adr" => "AdrVecImm32"}]],
    :prm => ["prfop_sve", "msz", "pg", "adr"],
    :grp => [
      {"PRFB"   => {"msz" => 0}},
      {"PRFH"   => {"msz" => 1}},
      {"PRFW"   => {"msz" => 2}},
      {"PRFD"   => {"msz" => 3}}
    ]
  },

  "Sve32ContiPfScImm" => {
    :cmt => "SVE 32-bit contiguous prefetch (scalar plus immediate)",
    :arg => [
      [ {"prfop_sve" => "PrfopSve"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #0
      [ {"prfop_sve" => "PrfopSve"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}]  #1
    ],
    :prm => ["prfop_sve", "msz", "pg", "adr"],
    :grp => [
      {"PRFB"   => {"msz" => 0}},
      {"PRFH"   => {"msz" => 1}},
      {"PRFW"   => {"msz" => 2}},
      {"PRFD"   => {"msz" => 3}}
    ]
  },
  
  "Sve32ContiPfScSc" => {
    :cmt => "SVE 32-bit contiguous prefetch (scalar plus scalar)",
    :arg => [[ {"prfop_sve" => "PrfopSve"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}]],
    :prm => ["prfop_sve", "msz", "pg", "adr"],
    :grp => [
      {"PRFB"   => {"msz" => 0}},
      {"PRFH"   => {"msz" => 1}},
      {"PRFW"   => {"msz" => 2}},
      {"PRFD"   => {"msz" => 3}}
    ]
  },

  "SveLoadAndBcElem" => {
    :cmt => "SVE load and broadcast element",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #3
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #4
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #5
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #6
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}]  #7
    ],
    :prm => ["dtypeh", "dtypel", "zt", "pg", "adr"],
    :grp => [
      {"LD1RB"   => {"dtypeh" => 0, "dtypel" => 0}, :arg => [0,4]},
      {"LD1RB"   => {"dtypeh" => 0, "dtypel" => 1}, :arg => [1,5]},
      {"LD1RB"   => {"dtypeh" => 0, "dtypel" => 2}, :arg => [2,6]},
      {"LD1RB"   => {"dtypeh" => 0, "dtypel" => 3}, :arg => [3,7]},
      {"LD1RSW"  => {"dtypeh" => 1, "dtypel" => 0}, :arg => [3,7]},
      {"LD1RH"   => {"dtypeh" => 1, "dtypel" => 1}, :arg => [1,5]},
      {"LD1RH"   => {"dtypeh" => 1, "dtypel" => 2}, :arg => [2,6]},
      {"LD1RH"   => {"dtypeh" => 1, "dtypel" => 3}, :arg => [3,7]},
      {"LD1RSH"  => {"dtypeh" => 2, "dtypel" => 0}, :arg => [3,7]},
      {"LD1RSH"  => {"dtypeh" => 2, "dtypel" => 1}, :arg => [2,6]},
      {"LD1RW"   => {"dtypeh" => 2, "dtypel" => 2}, :arg => [2,6]},
      {"LD1RW"   => {"dtypeh" => 2, "dtypel" => 3}, :arg => [3,7]},
      {"LD1RSB"  => {"dtypeh" => 3, "dtypel" => 0}, :arg => [3,7]},
      {"LD1RSB"  => {"dtypeh" => 3, "dtypel" => 1}, :arg => [2,6]},
      {"LD1RSB"  => {"dtypeh" => 3, "dtypel" => 2}, :arg => [1,5]},
      {"LD1RD"   => {"dtypeh" => 3, "dtypel" => 3}, :arg => [3,7]}
    ]
  },
  
  "SveLoadPredReg" => {
    :cmt => "SVE load predicate register",
    :arg => [
      [ {"pt" => "_PReg"}, {"adr" => "AdrScImm"}], #0
      [ {"pt" => "_PReg"}, {"adr" => "AdrNoOfs"}]  #1
    ],
    :prm => ["pt", "adr"],
    :grp => [
      {"LDR" => {}}
    ]
  },

  "SveLoadPredVec" => {
    :cmt => "SVE load predicate vector",
    :arg => [
      [ {"zt" => "ZReg"}, {"adr" => "AdrScImm"}], #0
      [ {"zt" => "ZReg"}, {"adr" => "AdrNoOfs"}]  #1
    ],
    :prm => ["zt", "adr"],
    :grp => [
      {"LDR" => {}}
    ]
  },

  "SveContiFFLdScSc" => {
    :cmt => "SVE contiguous first-fault load (scalar plus scalar)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #3
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #4
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #5
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #6
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}]  #7
    ],
    :prm => ["dtype", "zt", "pg", "adr"],
    :grp => [
      {"LDFF1B"  => {"dtype" => 0x0}, :arg => [0,4]},
      {"LDFF1B"  => {"dtype" => 0x1}, :arg => [1,5]},
      {"LDFF1B"  => {"dtype" => 0x2}, :arg => [2,6]},
      {"LDFF1B"  => {"dtype" => 0x3}, :arg => [3,7]},
      {"LDFF1SW" => {"dtype" => 0x4}, :arg => [3,7]},
      {"LDFF1H"  => {"dtype" => 0x5}, :arg => [1,5]},
      {"LDFF1H"  => {"dtype" => 0x6}, :arg => [2,6]},
      {"LDFF1H"  => {"dtype" => 0x7}, :arg => [3,7]},
      {"LDFF1SH" => {"dtype" => 0x8}, :arg => [3,7]},
      {"LDFF1SH" => {"dtype" => 0x9}, :arg => [2,6]},
      {"LDFF1W"  => {"dtype" => 0xa}, :arg => [2,6]},
      {"LDFF1W"  => {"dtype" => 0xb}, :arg => [3,7]},
      {"LDFF1SB" => {"dtype" => 0xc}, :arg => [3,7]},
      {"LDFF1SB" => {"dtype" => 0xd}, :arg => [2,6]},
      {"LDFF1SB" => {"dtype" => 0xe}, :arg => [1,5]},
      {"LDFF1D"  => {"dtype" => 0xf}, :arg => [3,7]}
    ]
  },

  "SveContiLdScImm" => {
    :cmt => "SVE contiguous load (scalar plus immediate)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #3
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #4
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #5
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #6
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}]  #7
    ],
    :prm => ["dtype", "zt", "pg", "adr"],
    :grp => [
      {"LD1B"  => {"dtype" => 0x0}, :arg => [0,4]},
      {"LD1B"  => {"dtype" => 0x1}, :arg => [1,5]},
      {"LD1B"  => {"dtype" => 0x2}, :arg => [2,6]},
      {"LD1B"  => {"dtype" => 0x3}, :arg => [3,7]},
      {"LD1SW" => {"dtype" => 0x4}, :arg => [3,7]},
      {"LD1H"  => {"dtype" => 0x5}, :arg => [1,5]},
      {"LD1H"  => {"dtype" => 0x6}, :arg => [2,6]},
      {"LD1H"  => {"dtype" => 0x7}, :arg => [3,7]},
      {"LD1SH" => {"dtype" => 0x8}, :arg => [3,7]},
      {"LD1SH" => {"dtype" => 0x9}, :arg => [2,6]},
      {"LD1W"  => {"dtype" => 0xa}, :arg => [2,6]},
      {"LD1W"  => {"dtype" => 0xb}, :arg => [3,7]},
      {"LD1SB" => {"dtype" => 0xc}, :arg => [3,7]},
      {"LD1SB" => {"dtype" => 0xd}, :arg => [2,6]},
      {"LD1SB" => {"dtype" => 0xe}, :arg => [1,5]},
      {"LD1D"  => {"dtype" => 0xf}, :arg => [3,7]}
    ]
  },

  "SveContiLdScSc" => {
    :cmt => "SVE contiguous load (scalar plus scalar)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}]  #3
    ],                                                         
    :prm => ["dtype", "zt", "pg", "adr"],
    :grp => [
      {"LD1B"  => {"dtype" => 0x0}, :arg => [0]},
      {"LD1B"  => {"dtype" => 0x1}, :arg => [1]},
      {"LD1B"  => {"dtype" => 0x2}, :arg => [2]},
      {"LD1B"  => {"dtype" => 0x3}, :arg => [3]},
      {"LD1SW" => {"dtype" => 0x4}, :arg => [3]},
      {"LD1H"  => {"dtype" => 0x5}, :arg => [1]},
      {"LD1H"  => {"dtype" => 0x6}, :arg => [2]},
      {"LD1H"  => {"dtype" => 0x7}, :arg => [3]},
      {"LD1SH" => {"dtype" => 0x8}, :arg => [3]},
      {"LD1SH" => {"dtype" => 0x9}, :arg => [2]},
      {"LD1W"  => {"dtype" => 0xa}, :arg => [2]},
      {"LD1W"  => {"dtype" => 0xb}, :arg => [3]},
      {"LD1SB" => {"dtype" => 0xc}, :arg => [3]},
      {"LD1SB" => {"dtype" => 0xd}, :arg => [2]},
      {"LD1SB" => {"dtype" => 0xe}, :arg => [1]},
      {"LD1D"  => {"dtype" => 0xf}, :arg => [3]}
    ]
  },

  "SveContiNFLdScImm" => {
    :cmt => "SVE contiguous non-fault load (scalar plus immediate)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #3
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #4
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #5
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #6
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}]  #7
    ],
    :prm => ["dtype", "zt", "pg", "adr"],
    :grp => [
      {"LDNF1B"  => {"dtype" => 0x0}, :arg => [0,4]},
      {"LDNF1B"  => {"dtype" => 0x1}, :arg => [1,5]},
      {"LDNF1B"  => {"dtype" => 0x2}, :arg => [2,6]},
      {"LDNF1B"  => {"dtype" => 0x3}, :arg => [3,7]},
      {"LDNF1SW" => {"dtype" => 0x4}, :arg => [3,7]},
      {"LDNF1H"  => {"dtype" => 0x5}, :arg => [1,5]},
      {"LDNF1H"  => {"dtype" => 0x6}, :arg => [2,6]},
      {"LDNF1H"  => {"dtype" => 0x7}, :arg => [3,7]},
      {"LDNF1SH" => {"dtype" => 0x8}, :arg => [3,7]},
      {"LDNF1SH" => {"dtype" => 0x9}, :arg => [2,6]},
      {"LDNF1W"  => {"dtype" => 0xa}, :arg => [2,6]},
      {"LDNF1W"  => {"dtype" => 0xb}, :arg => [3,7]},
      {"LDNF1SB" => {"dtype" => 0xc}, :arg => [3,7]},
      {"LDNF1SB" => {"dtype" => 0xd}, :arg => [2,6]},
      {"LDNF1SB" => {"dtype" => 0xe}, :arg => [1,5]},
      {"LDNF1D"  => {"dtype" => 0xf}, :arg => [3,7]}
    ]
  },

  "SveContiNTLdScImm" => {
    :cmt => "SVE contiguous non-temporal load (scalar plus immediate)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #3
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #4
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #5
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #6
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}]  #7
    ],
    :prm => ["msz", "zt", "pg", "adr"],
    :grp => [
      {"LDNT1B"  => {"msz" => 0}, :arg => [0,4]},
      {"LDNT1H"  => {"msz" => 1}, :arg => [1,5]},
      {"LDNT1W"  => {"msz" => 2}, :arg => [2,6]},
      {"LDNT1D"  => {"msz" => 3}, :arg => [3,7]}
    ]
  },

  "SveContiNTLdScSc" => {
    :cmt => "SVE contiguous non-temporal load (scalar plus scalar)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}]  #3
    ],
    :prm => ["msz", "zt", "pg", "adr"],
    :grp => [
      {"LDNT1B"  => {"msz" => 0}, :arg => [0]},
      {"LDNT1H"  => {"msz" => 1}, :arg => [1]},
      {"LDNT1W"  => {"msz" => 2}, :arg => [2]},
      {"LDNT1D"  => {"msz" => 3}, :arg => [3]}
    ]
  },

  "SveLdBcQuadScImm" => {
    :cmt => "SVE load and broadcast quadword (scalar plus immediate)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #3
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #4
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #5
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #6
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}]  #7
    ],
    :prm => ["msz", "num", "zt", "pg", "adr"],
    :grp => [
      {"LD1RQB"  => {"msz" => 0, "num" => 0}, :arg => [0,4]},
      {"LD1RQH"  => {"msz" => 1, "num" => 0}, :arg => [1,5]},
      {"LD1RQW"  => {"msz" => 2, "num" => 0}, :arg => [2,6]},
      {"LD1RQD"  => {"msz" => 3, "num" => 0}, :arg => [3,7]}
    ]
  },

  "SveLdBcQuadScSc" => {
    :cmt => "SVE load and broadcast quadword (scalar plus scalar)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}]  #3
    ],
    :prm => ["msz", "num", "zt", "pg", "adr"],
    :grp => [
      {"LD1RQB"  => {"msz" => 0, "num" => 0}, :arg => [0]},
      {"LD1RQH"  => {"msz" => 1, "num" => 0}, :arg => [1]},
      {"LD1RQW"  => {"msz" => 2, "num" => 0}, :arg => [2]},
      {"LD1RQD"  => {"msz" => 3, "num" => 0}, :arg => [3]}
    ]
  },

  "SveLdMultiStructScImm" => {
    :cmt => "SVE load multiple structures (scalar plus immediate)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #3
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #4
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #5
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #6
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #7
    ],
    :prm => ["msz", "num", "zt", "pg", "adr"],
    :grp => [
      {"LD2B"  => {"msz" => 0, "num" => 1}, :arg => [0,4]},
      {"LD3B"  => {"msz" => 0, "num" => 2}, :arg => [0,4]},
      {"LD4B"  => {"msz" => 0, "num" => 3}, :arg => [0,4]},
      {"LD2H"  => {"msz" => 1, "num" => 1}, :arg => [1,5]},
      {"LD3H"  => {"msz" => 1, "num" => 2}, :arg => [1,5]},
      {"LD4H"  => {"msz" => 1, "num" => 3}, :arg => [1,5]},
      {"LD2W"  => {"msz" => 2, "num" => 1}, :arg => [2,6]},
      {"LD3W"  => {"msz" => 2, "num" => 2}, :arg => [2,6]},
      {"LD4W"  => {"msz" => 2, "num" => 3}, :arg => [2,6]},
      {"LD2D"  => {"msz" => 3, "num" => 1}, :arg => [3,7]},
      {"LD3D"  => {"msz" => 3, "num" => 2}, :arg => [3,7]},
      {"LD4D"  => {"msz" => 3, "num" => 3}, :arg => [3,7]}
    ]
  },
  
  "SveLdMultiStructScSc" => {
    :cmt => "SVE load multiple structures (scalar plus scalar)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}]  #3
    ],
    :prm => ["msz", "num", "zt", "pg", "adr"],
    :grp => [
      {"LD2B"  => {"msz" => 0, "num" => 1}, :arg => [0]},
      {"LD3B"  => {"msz" => 0, "num" => 2}, :arg => [0]},
      {"LD4B"  => {"msz" => 0, "num" => 3}, :arg => [0]},
      {"LD2H"  => {"msz" => 1, "num" => 1}, :arg => [1]},
      {"LD3H"  => {"msz" => 1, "num" => 2}, :arg => [1]},
      {"LD4H"  => {"msz" => 1, "num" => 3}, :arg => [1]},
      {"LD2W"  => {"msz" => 2, "num" => 1}, :arg => [2]},
      {"LD3W"  => {"msz" => 2, "num" => 2}, :arg => [2]},
      {"LD4W"  => {"msz" => 2, "num" => 3}, :arg => [2]},
      {"LD2D"  => {"msz" => 3, "num" => 1}, :arg => [3]},
      {"LD3D"  => {"msz" => 3, "num" => 2}, :arg => [3]},
      {"LD4D"  => {"msz" => 3, "num" => 3}, :arg => [3]}
    ]
  },

  "Sve64GatherLdSc32US" => {
    :cmt => "SVE 64-bit gather load (scalar plus unpacked 32-bit scaled offsets)",
    :arg => [[ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrSc32US"}]],
    :prm => ["msz", "U", "ff", "zt", "pg", "adr"],
    :grp => [
      {"LD1SH"   => {"msz" => 1, "U" => 0, "ff" => 0}},
      {"LDFF1SH" => {"msz" => 1, "U" => 0, "ff" => 1}},
      {"LD1H"    => {"msz" => 1, "U" => 1, "ff" => 0}},
      {"LDFF1H"  => {"msz" => 1, "U" => 1, "ff" => 1}},
      {"LD1SW"   => {"msz" => 2, "U" => 0, "ff" => 0}},
      {"LDFF1SW" => {"msz" => 2, "U" => 0, "ff" => 1}},
      {"LD1W"    => {"msz" => 2, "U" => 1, "ff" => 0}},
      {"LDFF1W"  => {"msz" => 2, "U" => 1, "ff" => 1}},
      {"LD1D"    => {"msz" => 3, "U" => 1, "ff" => 0}},
      {"LDFF1D"  => {"msz" => 3, "U" => 1, "ff" => 1}},
    ]
  },

  "Sve64GatherLdSc64S" => {
    :cmt => "SVE 64-bit gather load (scalar plus 64-bit scaled offsets)",
    :arg => [[ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrSc64S"}]],
    :prm => ["msz", "U", "ff", "zt", "pg", "adr"],
    :grp => [
      {"LD1SH"   => {"msz" => 1, "U" => 0, "ff" => 0}},
      {"LDFF1SH" => {"msz" => 1, "U" => 0, "ff" => 1}},
      {"LD1H"    => {"msz" => 1, "U" => 1, "ff" => 0}},
      {"LDFF1H"  => {"msz" => 1, "U" => 1, "ff" => 1}},
      {"LD1SW"   => {"msz" => 2, "U" => 0, "ff" => 0}},
      {"LDFF1SW" => {"msz" => 2, "U" => 0, "ff" => 1}},
      {"LD1W"    => {"msz" => 2, "U" => 1, "ff" => 0}},
      {"LDFF1W"  => {"msz" => 2, "U" => 1, "ff" => 1}},
      {"LD1D"    => {"msz" => 3, "U" => 1, "ff" => 0}},
      {"LDFF1D"  => {"msz" => 3, "U" => 1, "ff" => 1}},
    ]
  },

  "Sve64GatherLdSc64U" => {
    :cmt => "SVE 64-bit gather load (scalar plus 64-bit unscaled offsets)",
    :arg => [[ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrSc64U"}]],
    :prm => ["msz", "U", "ff", "zt", "pg", "adr"],
    :grp => [
      {"LD1SB"   => {"msz" => 0, "U" => 0, "ff" => 0}},
      {"LDFF1SB" => {"msz" => 0, "U" => 0, "ff" => 1}},
      {"LD1B"    => {"msz" => 0, "U" => 1, "ff" => 0}},
      {"LDFF1B"  => {"msz" => 0, "U" => 1, "ff" => 1}},
      {"LD1SH"   => {"msz" => 1, "U" => 0, "ff" => 0}},
      {"LDFF1SH" => {"msz" => 1, "U" => 0, "ff" => 1}},
      {"LD1H"    => {"msz" => 1, "U" => 1, "ff" => 0}},
      {"LDFF1H"  => {"msz" => 1, "U" => 1, "ff" => 1}},
      {"LD1SW"   => {"msz" => 2, "U" => 0, "ff" => 0}},
      {"LDFF1SW" => {"msz" => 2, "U" => 0, "ff" => 1}},
      {"LD1W"    => {"msz" => 2, "U" => 1, "ff" => 0}},
      {"LDFF1W"  => {"msz" => 2, "U" => 1, "ff" => 1}},
      {"LD1D"    => {"msz" => 3, "U" => 1, "ff" => 0}},
      {"LDFF1D"  => {"msz" => 3, "U" => 1, "ff" => 1}},
    ]
  },

  "Sve64GatherLdSc32UU" => {
    :cmt => "SVE 64-bit gather load (scalar plus unpacked 32-bit unscaled offsets)",
    :arg => [[ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrSc32UU"}]],
    :prm => ["msz", "U", "ff", "zt", "pg", "adr"],
    :grp => [
      {"LD1SB"   => {"msz" => 0, "U" => 0, "ff" => 0}},
      {"LDFF1SB" => {"msz" => 0, "U" => 0, "ff" => 1}},
      {"LD1B"    => {"msz" => 0, "U" => 1, "ff" => 0}},
      {"LDFF1B"  => {"msz" => 0, "U" => 1, "ff" => 1}},
      {"LD1SH"   => {"msz" => 1, "U" => 0, "ff" => 0}},
      {"LDFF1SH" => {"msz" => 1, "U" => 0, "ff" => 1}},
      {"LD1H"    => {"msz" => 1, "U" => 1, "ff" => 0}},
      {"LDFF1H"  => {"msz" => 1, "U" => 1, "ff" => 1}},
      {"LD1SW"   => {"msz" => 2, "U" => 0, "ff" => 0}},
      {"LDFF1SW" => {"msz" => 2, "U" => 0, "ff" => 1}},
      {"LD1W"    => {"msz" => 2, "U" => 1, "ff" => 0}},
      {"LDFF1W"  => {"msz" => 2, "U" => 1, "ff" => 1}},
      {"LD1D"    => {"msz" => 3, "U" => 1, "ff" => 0}},
      {"LDFF1D"  => {"msz" => 3, "U" => 1, "ff" => 1}},
    ]
  },

  "Sve64GatherLdVecImm" => {
    :cmt => "SVE 64-bit gather load (vector plus immeidate)",
    :arg => [[ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrVecImm64"}]],
    :prm => ["msz", "U", "ff", "zt", "pg", "adr"],
    :grp => [
      {"LD1SB"   => {"msz" => 0, "U" => 0, "ff" => 0}},
      {"LDFF1SB" => {"msz" => 0, "U" => 0, "ff" => 1}},
      {"LD1B"    => {"msz" => 0, "U" => 1, "ff" => 0}},
      {"LDFF1B"  => {"msz" => 0, "U" => 1, "ff" => 1}},
      {"LD1SH"   => {"msz" => 1, "U" => 0, "ff" => 0}},
      {"LDFF1SH" => {"msz" => 1, "U" => 0, "ff" => 1}},
      {"LD1H"    => {"msz" => 1, "U" => 1, "ff" => 0}},
      {"LDFF1H"  => {"msz" => 1, "U" => 1, "ff" => 1}},
      {"LD1SW"   => {"msz" => 2, "U" => 0, "ff" => 0}},
      {"LDFF1SW" => {"msz" => 2, "U" => 0, "ff" => 1}},
      {"LD1W"    => {"msz" => 2, "U" => 1, "ff" => 0}},
      {"LDFF1W"  => {"msz" => 2, "U" => 1, "ff" => 1}},
      {"LD1D"    => {"msz" => 3, "U" => 1, "ff" => 0}},
      {"LDFF1D"  => {"msz" => 3, "U" => 1, "ff" => 1}},
    ]
  },

  "Sve64GatherPfSc64S" => {
    :cmt => "SVE 64-bit gather load (scalar plus 64-bit scaled offsets)",
    :arg => [[ {"prfop_sve" => "PrfopSve"}, {"pg" => "_PReg"}, {"adr" => "AdrSc64S"}]],
    :prm => ["prfop_sve", "msz", "pg", "adr"],
    :grp => [
      {"PRFB"   => {"msz" => 0}},
      {"PRFH"   => {"msz" => 1}},
      {"PRFW"   => {"msz" => 2}},
      {"PRFD"   => {"msz" => 3}}
    ]
  },

  "Sve64GatherPfSc32US" => {
    :cmt => "SVE 64-bit gather load (scalar plus unpacked 32-bit scaled offsets)",
    :arg => [[ {"prfop_sve" => "PrfopSve"}, {"pg" => "_PReg"}, {"adr" => "AdrSc32US"}]],
    :prm => ["prfop_sve", "msz", "pg", "adr"],
    :grp => [
      {"PRFB"   => {"msz" => 0}},
      {"PRFH"   => {"msz" => 1}},
      {"PRFW"   => {"msz" => 2}},
      {"PRFD"   => {"msz" => 3}}
    ]
  },

  "Sve64GatherPfVecImm" => {
    :cmt => "SVE 64-bit gather load (vector plus immediate)",
    :arg => [[ {"prfop_sve" => "PrfopSve"}, {"pg" => "_PReg"}, {"adr" => "AdrVecImm64"}]],
    :prm => ["prfop_sve", "msz", "pg", "adr"],
    :grp => [
      {"PRFB"   => {"msz" => 0}},
      {"PRFH"   => {"msz" => 1}},
      {"PRFW"   => {"msz" => 2}},
      {"PRFD"   => {"msz" => 3}}
    ]
  },

  "Sve32ScatterStSc32S" => {
    :cmt => "SVE 32-bit scatter store (sclar plus 32-bit scaled offsets)",
    :arg => [[ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrSc32S"}]],
    :prm => ["msz", "zt", "pg", "adr"],
    :grp => [
      {"ST1H"   => {"msz" => 1}},
      {"ST1W"   => {"msz" => 2}}
    ]
  },

  "Sve32ScatterStSc32U" => {
    :cmt => "SVE 32-bit scatter store (sclar plus 32-bit unscaled offsets)",
    :arg => [[ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrSc32U"}]],
    :prm => ["msz", "zt", "pg", "adr"],
    :grp => [
      {"ST1B"   => {"msz" => 0}},
      {"ST1H"   => {"msz" => 1}},
      {"ST1W"   => {"msz" => 2}}
    ]
  },

  "Sve32ScatterStVecImm" => {
    :cmt => "SVE 32-bit scatter store (vector plus immediate)",
    :arg => [[ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrVecImm32"}]],
    :prm => ["msz", "zt", "pg", "adr"],
    :grp => [
      {"ST1B"   => {"msz" => 0}},
      {"ST1H"   => {"msz" => 1}},
      {"ST1W"   => {"msz" => 2}},
    ]
  },

  "Sve64ScatterStSc64S" => {
    :cmt => "SVE 64-bit scatter store (scalar plus 64-bit scaled offsets)",
    :arg => [[ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrSc64S"}]],
    :prm => ["msz", "zt", "pg", "adr"],
    :grp => [
      {"ST1H"   => {"msz" => 1}},
      {"ST1W"   => {"msz" => 2}},
      {"ST1D"   => {"msz" => 3}}
    ]
  },

  "Sve64ScatterStSc64U" => {
    :cmt => "SVE 64-bit scatter store (scalar plus 64-bit unscaled offsets)",
    :arg => [[ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrSc64U"}]],
    :prm => ["msz", "zt", "pg", "adr"],
    :grp => [
      {"ST1B"   => {"msz" => 0}},
      {"ST1H"   => {"msz" => 1}},
      {"ST1W"   => {"msz" => 2}},
      {"ST1D"   => {"msz" => 3}}
    ]
  },

  "Sve64ScatterStSc32US" => {
    :cmt => "SVE 64-bit scatter store (scalar plus unpacked 32-bit scaled offsets)",
    :arg => [[ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrSc32US"}]],
    :prm => ["msz", "zt", "pg", "adr"],
    :grp => [
      {"ST1H"   => {"msz" => 1}},
      {"ST1W"   => {"msz" => 2}},
      {"ST1D"   => {"msz" => 3}}
    ]
  },

  "Sve64ScatterStSc32UU" => {
    :cmt => "SVE 64-bit scatter store (scalar plus unpacked 32-bit unscaled offsets)",
    :arg => [[ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrSc32UU"}]],
    :prm => ["msz", "zt", "pg", "adr"],
    :grp => [
      {"ST1B"   => {"msz" => 0}},
      {"ST1H"   => {"msz" => 1}},
      {"ST1W"   => {"msz" => 2}},
      {"ST1D"   => {"msz" => 3}}
    ]
  },

  "Sve64ScatterStVecImm" => {
    :cmt => "SVE 64-bit scatter store (vector plus immediate)",
    :arg => [[ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrVecImm64"}]],
    :prm => ["msz", "zt", "pg", "adr"],
    :grp => [
      {"ST1B"   => {"msz" => 0}},
      {"ST1H"   => {"msz" => 1}},
      {"ST1W"   => {"msz" => 2}},
      {"ST1D"   => {"msz" => 3}}
    ]
  },

  "SveContiNTStScImm" => {
    :cmt => "SVE contiguous non-temporal store (scalar plus immediate)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #3
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #4
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #5
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #6
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}]  #7
    ],
    :prm => ["msz", "zt", "pg", "adr"],
    :grp => [
      {"STNT1B"   => {"msz" => 0}, :arg => [0,4]},
      {"STNT1H"   => {"msz" => 1}, :arg => [1,5]},
      {"STNT1W"   => {"msz" => 2}, :arg => [2,6]},
      {"STNT1D"   => {"msz" => 3}, :arg => [3,7]}
    ]
  },

  "SveContiNTStScSc" => {
    :cmt => "SVE contiguous non-temporal store (scalar plus scalar)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}]  #3
    ],
    :prm => ["msz", "zt", "pg", "adr"],
    :grp => [
      {"STNT1B"   => {"msz" => 0}, :arg => [0]},
      {"STNT1H"   => {"msz" => 1}, :arg => [1]},
      {"STNT1W"   => {"msz" => 2}, :arg => [2]},
      {"STNT1D"   => {"msz" => 3}, :arg => [3]}
    ]
  },

  "SveContiStScImm" => {
    :cmt => "SVE contiguous store (scalar plus immediate)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #3
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #4
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #5
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #6
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}]  #7
    ],
    :prm => ["msz", "zt", "pg", "adr"],
    :grp => [
      {"ST1B"   => {"msz" => 0}, :arg => [0,1,2,3,4,5,6,7]},
      {"ST1H"   => {"msz" => 1}, :arg => [1,2,3,5,6,7]},
      {"ST1W"   => {"msz" => 2}, :arg => [2,3,6,7]},
      {"ST1D"   => {"msz" => 3}, :arg => [3,7]}
    ]
  },

  "SveContiStScSc" => {
    :cmt => "SVE contiguous store (scalar plus scalar)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}]  #3
    ],
    :prm => ["msz", "zt", "pg", "adr"],
    :grp => [
      {"ST1B"   => {"msz" => 0}, :arg => [0,1,2,3]},
      {"ST1H"   => {"msz" => 1}, :arg => [1,2,3]},
      {"ST1W"   => {"msz" => 2}, :arg => [2,3]},
      {"ST1D"   => {"msz" => 3}, :arg => [3]}
    ]
  },

  "SveStMultiStructScImm" => {
    :cmt => "SVE store multipule structures (scalar plus immediate)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScImm"}], #3
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #4
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #5
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}], #6
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrNoOfs"}]  #7
    ],
    :prm => ["msz", "num", "zt", "pg", "adr"],
    :grp => [
      {"ST2B"   => {"msz" => 0, "num" => 1}, :arg => [0,4]},
      {"ST3B"   => {"msz" => 0, "num" => 2}, :arg => [0,4]},
      {"ST4B"   => {"msz" => 0, "num" => 3}, :arg => [0,4]},
      {"ST2H"   => {"msz" => 1, "num" => 1}, :arg => [1,5]},
      {"ST3H"   => {"msz" => 1, "num" => 2}, :arg => [1,5]},
      {"ST4H"   => {"msz" => 1, "num" => 3}, :arg => [1,5]},
      {"ST2W"   => {"msz" => 2, "num" => 1}, :arg => [2,6]},
      {"ST3W"   => {"msz" => 2, "num" => 2}, :arg => [2,6]},
      {"ST4W"   => {"msz" => 2, "num" => 3}, :arg => [2,6]},
      {"ST2D"   => {"msz" => 3, "num" => 1}, :arg => [3,7]},
      {"ST3D"   => {"msz" => 3, "num" => 2}, :arg => [3,7]},
      {"ST4D"   => {"msz" => 3, "num" => 3}, :arg => [3,7]}
    ]
  },

  "SveStMultiStructScSc" => {
    :cmt => "SVE store multipule structures (scalar plus scalar)",
    :arg => [
      [ {"zt" => "ZRegB"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #0
      [ {"zt" => "ZRegH"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #1
      [ {"zt" => "ZRegS"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}], #2
      [ {"zt" => "ZRegD"}, {"pg" => "_PReg"}, {"adr" => "AdrScSc"}]  #3
    ],
    :prm => ["msz", "num", "zt", "pg", "adr"],
    :grp => [
      {"ST2B"   => {"msz" => 0, "num" => 1}, :arg => [0]},
      {"ST3B"   => {"msz" => 0, "num" => 2}, :arg => [0]},
      {"ST4B"   => {"msz" => 0, "num" => 3}, :arg => [0]},
      {"ST2H"   => {"msz" => 1, "num" => 1}, :arg => [1]},
      {"ST3H"   => {"msz" => 1, "num" => 2}, :arg => [1]},
      {"ST4H"   => {"msz" => 1, "num" => 3}, :arg => [1]},
      {"ST2W"   => {"msz" => 2, "num" => 1}, :arg => [2]},
      {"ST3W"   => {"msz" => 2, "num" => 2}, :arg => [2]},
      {"ST4W"   => {"msz" => 2, "num" => 3}, :arg => [2]},
      {"ST2D"   => {"msz" => 3, "num" => 1}, :arg => [3]},
      {"ST3D"   => {"msz" => 3, "num" => 2}, :arg => [3]},
      {"ST4D"   => {"msz" => 3, "num" => 3}, :arg => [3]}
    ]
  },

  "SveStorePredReg" => {
    :cmt => "SVE store predicate register",
    :arg => [
      [ {"pt" => "_PReg"}, {"adr" => "AdrScImm"}], #0
      [ {"pt" => "_PReg"}, {"adr" => "AdrNoOfs"}]  #1
    ],
    :prm => ["pt", "adr"],
    :grp => [
      {"STR" => {}}
    ]
  },

  "SveStorePredVec" => {
    :cmt => "SVE store predicate vector",
    :arg => [
      [ {"zt" => "ZReg"}, {"adr" => "AdrScImm"}], #0
      [ {"zt" => "ZReg"}, {"adr" => "AdrNoOfs"}]  #1
    ],
    :prm => ["zt", "adr"],
    :grp => [
      {"STR" => {}}
    ]
  }
}

class MnemonicGenerator
  def initialize
    @info_list = []
    @err_list = []
    @func_list = []
  end

  public
  def parseTable(table)
    @err_list = []
    grp_info_list = []
    table.each do |grp_func,grp_info|
      grp_info_list += _parseGrpInfo(grp_info, grp_func)
    end
    grp_info_list.each{|info| _checkInfo(info)}
    @err_list.each{|err| STDERR.puts _mkErrStr(err) }
    @info_list += grp_info_list if @err_list.size == 0
  end

  def sortByMnemonic
    @info_list.sort!{|a,b| a[:func] <=> b[:func]}
  end

  def sortByGroup
    @info_list.sort!{|a,b| a[:grp_func] <=> b[:grp_func]}
  end

  def output(ofile)
    File.open(ofile,"w") do |f|
      f.puts "#define SET() setCodeInfo(__FILE__,__LINE__,__func__)"
      @info_list.each{ |info| f.puts _genFuncStr(info) }
      f.puts "#undef SET"
    end
  end

  private
  def _parseGrpInfo(grp_info, grp_func)
    grp_info_list = []
    grp_info[:grp].each do |mne_info|
      func_info = _parseMneInfo(mne_info)

      grp_func_prm = grp_info[:prm]
      grp_func_prm = grp_info[:prm][func_info[:prm_idx]] if not func_info[:prm_idx].nil?

      func_info[:arg_ptn] = Range.new(0,grp_info[:arg].size-1) if func_info[:arg_ptn].nil?
      func_info[:arg_ptn].each do |ptn_idx|
        info = {
          :cmt          => grp_info[:cmt],
          :func         => _transFuncName(func_info[:func]),
          :func_arg     => grp_info[:arg][ptn_idx],
          :grp_func     => grp_func,
          :grp_func_prm => grp_func_prm,
          :func_set_val => func_info[:func_set_val]
        }
        grp_info_list.push info
      end
    end
    return grp_info_list
  end

  def _parseMneInfo(mne_info)
    info = {}
    mne_info.each do |key,val|
      if key.class == String
        info[:func] = key
        info[:func_set_val] = val
      elsif key == :arg
        info[:arg_ptn] = val
      elsif key == :prm
        info[:prm_idx] = val
      end
    end
    return info
  end

  def _transFuncName(name)
    f = name.downcase
    f += "_" if f == "and" or f == "or" or f == "not"
    return f
  end
  
  def _checkInfo(info)
    # all use check for func_arg
    func_args = info[:func_arg].inject([]){|c,x| c += x.keys.map{|k| k.gsub(/=.*/,'')}}
    # all_use_func_arg = func_args.inject(true){|c,x| c &= info[:grp_func_prm].include?(x)}
    all_use_func_arg = true;
    
    # all use check for check for grp_func_val
    grp_func_vars = info[:func_set_val].keys
    all_use_grp_func_vars = grp_func_vars.inject(true){|c,x| c &= info[:grp_func_prm].include?(x)}
    
    # check grp_func_prm
    vars = func_args + grp_func_vars
    all_exist_func_prm = info[:grp_func_prm].inject(true){|c,x| c &= vars.include?(x)}

    # type_list
    type_list = info[:func_arg].inject([]){|c,x| c += x.values}
    func_str = info[:func] + type_list.join("_")

    # check same func
    same_func = @func_list.include?(func_str)

    err_info = {
      :all_use_func_arg_err      => !all_use_func_arg,
      :all_use_grp_func_vars_err => !all_use_grp_func_vars,
      :all_exist_func_prm_err    => !all_exist_func_prm,
      :same_func_err             => same_func,
      :grp_func                  => info[:grp_func],
      :func                      => info[:func]
    }

    @err_list.push err_info if !all_use_func_arg or !all_use_grp_func_vars or !all_exist_func_prm or same_func
    @func_list.push func_str
  end

  def _mkErrStr(err_info)
    err_str = ""
    err_str += format("Error: %s::%s\n",err_info[:grp_func],err_info[:func])
    err_str += format("don't use all func_arg \n")     if err_info[:all_use_func_arg_err]
    err_str += format("don't use all grp_func_var \n") if err_info[:all_use_grp_func_vars_err]
    err_str += format("don't exist all func_prm \n")   if err_info[:all_exist_func_prm_err]
    err_str += format("already exist same func \n")     if err_info[:same_func_err]
    return err_str
  end
  
  def _genFuncStr(info)
    func         = info[:func]
    func_arg     = _genFuncArgStr(info[:func_arg])
    grp_func     = info[:grp_func]
    grp_func_prm = _genGrpFuncPrmStr(info[:grp_func_prm],info[:func_set_val])
    set_prm      = _genSetPrmStr(info[:func_arg])
    
    return format("void %s(%s) { SET(); %s(%s); }",func,func_arg,grp_func,grp_func_prm)
  end

  def _genFuncArgStr(func_arg)
    arg_str = ""
    func_arg.each do |arg_info|
      arg_info.each do |name,type|
        arg_str += ", " if arg_str != ""
        if type =~ /Reg/ or type =~ /Adr/ or type =~ /Label/
          arg_str += format("const %s &%s",type,name)
        else
          arg_str += format("const %s %s",type,name)
        end
      end
    end
    return arg_str
  end

  def _genGrpFuncPrmStr(grp_func_prm, grp_func_val)
    prm_str = ""
    grp_func_prm.each do |prm|
      prm = grp_func_val[prm] if grp_func_val.key?(prm)
      prm_str += ", " if prm_str != ""
      prm_str += prm.to_s
    end
    return prm_str
  end

  def _genSetPrmStr(func_arg)
    arg_str = ""
    func_arg.each do |arg_info|
      arg_info.each do |name,type|
        arg_str += ", " if arg_str != ""
        arg_str += name.gsub(/=.*/,'')
      end
    end
    arg_str = "\"\"" if arg_str == "";
    return arg_str
  end
end


STDERR.print "Ruby 2.5.1 or higher required\n" if Gem::Version.create(RUBY_VERSION) < Gem::Version.create("2.5.1")
mgen_all = MnemonicGenerator.new
mgen_all.parseTable(v8)
mgen_all.parseTable(sve)
mgen_all.output("xbyak_aarch64_mnemonic.h")
mgen_all.sortByMnemonic
mgen_all.output("xbyak_aarch64_mnemonic_sorted.h")

