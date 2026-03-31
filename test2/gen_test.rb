#!/usr/bin/ruby
#*******************************************************************************
# Copyright 2021-2023 FUJITSU LIMITED
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
#require 'byebug'

class TestPatternGenerator
  def initialize
    @instructions_hash = {}
    @operands_ptn = {}
    @instructions = []

    operands_ptn_init()
  end

  public
  def parseTest(dirname)
    # Handle all *.test files inside "dirname" directory
    Dir.glob(dirname+'/**/*.test').sort.each{|item|
      puts item
      parseTestEach(item)
    }
  end

  def parseTestEach(filename)
    File.open(filename, "r") do |f|
      f.each_line{|line|
        parseTestEachLine(line)
      }
    end
  end

  def parseTestEachLine(ptn_line)
    # Remove CR and LF
    ptn_line.chomp!
    # Skip comment line
    if ptn_line.start_with?("#")
      return
    end
    # Skip blank line
    if ptn_line.start_with?(/^( |\t)*$/)
      return
    end

    # Replace first space into comma
    # Example "ST64B x0, x0, [x0]" -> "ST64B,x0, x0, [x0]"
    ptn_line.sub!(/( |\t)+/, ",")

    # Remove all white space
    ptn_line.gsub!(/( |\t)+/, "")

    # Combine multiple whitespace into one, and replace all delimiters into comma
    ptn_line.gsub!(/( |\t)+/, " ")
    ptn_line.gsub!(/, /, ",")
    ptn_line.gsub!(/ ,/, ",")

    # Replace "{," into "%" temporally
    ptn_line.gsub!(/{,/, "%")
    ptn_line.gsub!(/ \%/, "%")
    ptn_line.gsub!(/\% /, "%")

    # Replace "<Ws:even>,<W(s+1)" -> "WS_PAIR"
    # Replace "<Wt:even>,<W(t+1)" -> "WT_PAIR"
    # Replace "<Xs:even>,<X(s+1)" -> "XS_PAIR"
    # Replace "<Xt:even>,<X(t+1)" -> "XT_PAIR"
    # Replace "<Zdn>.D,<Zdn>.d"   -> ZDN_D_PAIR"
    ptn_line.gsub!(/<Ws:even>,<W\(s\+1\)>/, "WS_PAIR")
    ptn_line.gsub!(/<Wt:even>,<W\(t\+1\)>/, "WT_PAIR")
    ptn_line.gsub!(/<Xs:even>,<X\(s\+1\)>/, "XS_PAIR")
    ptn_line.gsub!(/<Xt:even>,<X\(t\+1\)>/, "XT_PAIR")
    ptn_line.gsub!(/\[<Zn>\.S%<Xm>\}\]/, "ADR_ZN_S_XM")
    ptn_line.gsub!(/{<Zn1>.B,<Zn2>.B}/, "ZN_B_LIST")
    ptn_line.gsub!(/{<Zn1>.H,<Zn2>.H}/, "ZN_H_LIST")
    ptn_line.gsub!(/{<Zn1>.S,<Zn2>.S}/, "ZN_S_LIST")
    ptn_line.gsub!(/{<Zn1>.D,<Zn2>.D}/, "ZN_D_LIST")
    ptn_line.gsub!(/<Zdn>.B,<Pg>\/M,<Zdn>.B/, "ZDN_B_PAIR_WITH_P_M")
    ptn_line.gsub!(/<Zdn>.H,<Pg>\/M,<Zdn>.H/, "ZDN_H_PAIR_WITH_P_M")
    ptn_line.gsub!(/<Zdn>.S,<Pg>\/M,<Zdn>.S/, "ZDN_S_PAIR_WITH_P_M")
    ptn_line.gsub!(/<Zdn>.D,<Pg>\/M,<Zdn>.D/, "ZDN_D_PAIR_WITH_P_M")
    ptn_line.gsub!(/<Zdn>.B,<Zdn>.B/, "ZDN_B_PAIR")
    ptn_line.gsub!(/<Zdn>.H,<Zdn>.H/, "ZDN_H_PAIR")
    ptn_line.gsub!(/<Zdn>.S,<Zdn>.S/, "ZDN_S_PAIR")
    ptn_line.gsub!(/<Zdn>.D,<Zdn>.D/, "ZDN_D_PAIR")
    ptn_line.gsub!(/<Ws>,<offs>/, "WS_OFFS_PAIR")
    ptn_line.gsub!(/,LSL#/, "TMP_LSL")
    ptn_line.gsub!(/ZA\[<Wv>,<offs>\],\[<Xn\|SP>%#<offs>,MULVL\}\]/, "ZA_WV_OFFS_PAIR")

    # Split mnemonic and operands
    STDOUT.flush
    tmp = ptn_line.split(",")

    # Recover "WS_PAIR" -> "<Ws:even>,<W(s+1)"
    # Recover "WT_PAIR" -> "<Wt:even>,<W(t+1)"
    # Recover "XS_PAIR" -> "<Xs:even>,<X(s+1)"
    # Recover "XT_PAIR" -> "<Xt:even>,<X(t+1)"
    # Recover "ZN_(B|H|S|D)" -> "{<Zn1>.(B|H|S|D),<Zn2>.(B|H|S|D)}"
    # Recover "ZDN_(B|H|S|D)_PAIR" -> "<Zdn>.(B|H|S|D),<Zdn>.(B|H|S|D)"
    # Recover "ZDN_(B|H|S|D)_PAIR_WITH_P_M" -> "<Zdn>.(B|H|S|D),<Pg>/m<Zdn>.(B|H|S|D)"
    # Recover "%" into "{,"
    for i in 0..tmp.size-1 do
      tmp[i].gsub!(/WS_PAIR/, "<Ws:even>,<W(s+1)>")
      tmp[i].gsub!(/WT_PAIR/, "<Wt:even>,<W(t+1)>")
      tmp[i].gsub!(/XS_PAIR/, "<Xs:even>,<X(s+1)>")
      tmp[i].gsub!(/XT_PAIR/, "<Xt:even>,<X(t+1)>")
      tmp[i].gsub!(/ADR_ZN_S_XM/, "[<Zn>.S{,<Xm>}]")
      tmp[i].gsub!(/ZN_B_LIST/, "{<Zn1>.B,<Zn2>.B}")
      tmp[i].gsub!(/ZN_H_LIST/, "{<Zn1>.H,<Zn2>.H}")
      tmp[i].gsub!(/ZN_S_LIST/, "{<Zn1>.S,<Zn2>.S}")
      tmp[i].gsub!(/ZN_D_LIST/, "{<Zn1>.D,<Zn2>.D}")
      tmp[i].gsub!(/ZDN_B_PAIR_WITH_P_M/, "<Zdn>.B,<Pg>/m,<Zdn>.B")
      tmp[i].gsub!(/ZDN_H_PAIR_WITH_P_M/, "<Zdn>.H,<Pg>/m,<Zdn>.H")
      tmp[i].gsub!(/ZDN_S_PAIR_WITH_P_M/, "<Zdn>.S,<Pg>/m,<Zdn>.S")
      tmp[i].gsub!(/ZDN_D_PAIR_WITH_P_M/, "<Zdn>.D,<Pg>/m,<Zdn>.D")
      tmp[i].gsub!(/ZDN_B_PAIR/, "<Zdn>.B,<Zdn>.B")
      tmp[i].gsub!(/ZDN_H_PAIR/, "<Zdn>.H,<Zdn>.H")
      tmp[i].gsub!(/ZDN_S_PAIR/, "<Zdn>.S,<Zdn>.S")
      tmp[i].gsub!(/ZDN_D_PAIR/, "<Zdn>.D,<Zdn>.D")
      tmp[i].gsub!(/WS_OFFS_PAIR/, "<Ws>,<offs>")
      tmp[i].gsub!(/TMP_LSL/, ",LSL #")
      tmp[i].gsub!(/ZA_WV_OFFS_PAIR/, "ZA[<Wv>,<offs>],[<Xn|SP>{,#<offs>,MUL VL}]")
      tmp[i].gsub!(/%/, "{,")
    end

    @instructions_hash.store(ptn_line, tmp)
  end

  def output_instruction_patterns()
    key_list = @instructions_hash.keys
    key_list.each{|key| output_instruction_patterns_core(key)}
  end

  def output_instruction_patterns_core(key)
    # Example: mn = swp
    mn        = @instructions_hash[key][0]
    # Example: op_list = ["swp", "<Ws>", "<Wt>", "[<Xn|SP>]"]
    op_list   = @instructions_hash[key]
    # Example: list_size = 4
    list_size = @instructions_hash[key].size
    

    # Generate base operands combination list
    # Example: @base_operands_combination = ["x0", "x0", "[x0]"]
    base_operands_combination = []
    for i in 1..list_size-1 do
      begin
        base_operands_combination.push((@operands_ptn[op_list[i]])[0])
        
      rescue => e
        STDOUT.flush
        p e.class
        p e.message
        p e.backtrace
        p mn
        p i
        p op_list[i]
        p @operands_ptn[op_list[i]]
        p key
        exit(false)
      end
    end

    # If mnemonic has no operand, just one test pattern is added.
    # Example "tcommit"
    if list_size == 1
      @instructions.push(mn.downcase!)
      return
    end

    # Generate operands combinations 2d-list
    # Example: @operands_combinations =
    #          [["x0", "x0", "[x0]"], ["x1", "x0", "[x0]"], ..., ["xzr", "x0", "[x0]"],
    #           ["x0", "x1", "[x0]"], ["x0", "x1", "[x0]"], ..., ["x0", "xzr", "[x0]"],
    #           ... ,
    #           ["x0", "x0", "[x0]"], ["x0", "x0", "[x1]"], ..., ["x0", "x0", "[sp]"]]
    operands_combinations = []
    for i in 1..list_size-1 do
      begin
        num_operands_focus = @operands_ptn[op_list[i]].size
      rescue => e
        p e.class
        p e.message
        p e.backtrace
        p mn
        p i
        p op_list[i]
        exit(false)
      end

      for j in 0..num_operands_focus-1 do
        tmp = base_operands_combination.dup
        tmp[i-1] = (@operands_ptn[op_list[i]])[j]
        operands_combinations.push(tmp)
      end
    end

    # Generate instructions list
    # Example: ["swp x0, x0, [x0]", "swp x1, x0, [x0]", ..., "swp xzr, x0, [x0]",
    #           "swp x0, x1, [x0]", "swp x0, x1, [x0]", ..., "swp x0, xzr, [x0]",
    #           ... ,
    #           "swp x0, x0, [x0]", "swp x0, x0, [x1]", ..., "swp x0, x0, [sp]"]
    for i in 0..operands_combinations.size-1 do
      inst = mn.downcase

      tmp_list = []
      for j in 0..(operands_combinations[i]).size-1 do
        if j != 0
          inst += ","
        else
          inst += " "
        end

        token = (operands_combinations[i])[j]

        # Example: Zdn:z0.b, Pg:p0, Zdn:OP:0, Zm:z8.b
        #          OP:0 -> z0.b
        tmp_list.push(token)
        if token.index(/OP:/) == 0
          position = (token.split(/:/))[1]
          token = tmp_list[position.to_i] + "/*asm*/"
        end

        inst += token
      end

      @instructions.push(inst)
    end
  end

  def convert_for_asm(inst)
    # Select operands for ASM
    # Example: "<{z8.b}|z8.b>" -> "{z8.b}"
    if(inst.index(/<([^\|]*)\|([^>]*)>/))
      inst.gsub!(/<([^\|]*)\|([^>]*)>/) { $1 }
    end

    # Remove register pair
    # Example ",w1/*cpp*/" -> ""
    inst.gsub!(/,[^,]+\/\*cpp\*\//, "")
    return inst
  end

  def convert_for_cpp(inst)
    # Select operands for CPP
    # Example: "<{z8.b}|z8.b>" -> "z8.b"
    if(inst.index(/<([^\|]*)\|([^>]*)>/))
      inst.gsub!(/<([^\|]*)\|([^>]*)>/) { $2 }
    end

    # Remove address operands for CPP
    # Example ",[x8]/*asm*/,ptr(x8)/*cpp*/" -> "ptr(x8)/*cpp*/"
    inst.gsub!(/,\[[^\[]+\/\*asm\*\//, "")

    # Remove register pair
    # Example "w0,w1/*asm*/" -> "w0"
    inst.gsub!(/,[^,]+\/\*asm\*\//, "")

    # Replace "m" -> "T_m"
    # Replace "z" -> "T_z"
    inst.sub!(/\/m/, "/T_m")
    inst.sub!(/\/m/, "/T_m")
    inst.sub!(/\/z/, "/T_z")

    # Replace ".8b" -> ".b8"
    inst.sub!(/\.8b/, "\.b8")
    inst.sub!(/\.16b/, "\.b16")
    inst.sub!(/\.4h/, "\.h4")
    inst.sub!(/\.8h/, "\.h8")
    inst.sub!(/\.2s/, "\.s2")
    inst.sub!(/\.4s/, "\.s4")
    inst.sub!(/\.1d/, "\.d1")
    inst.sub!(/\.2d/, "\.d2")

    tmp = inst.split(/\s+/)
    if tmp.size == 1 # no operands
      inst += "("
    else
      inst.sub!(/ /, "(")
    end

    inst += "); dump();"
    inst.sub!(/and\(/, "and_(")
    return inst
  end
  
  def operands_ptn_init
    @operands_ptn.store("<Wm>", ["w8", "w1", "w2", "w4", "w0", "w16", "w30", "wzr"])
    @operands_ptn.store("<Wn>", ["w8", "w1", "w2", "w4", "w0", "w16", "w30", "wzr"])
    @operands_ptn.store("<Ws>", ["w8", "w1", "w2", "w4", "w0", "w16", "w30", "wzr"])
    @operands_ptn.store("<Wt>", ["w8", "w1", "w2", "w4", "w0", "w16", "w30", "wzr"])
    @operands_ptn.store("<Xm>", ["x8", "x1", "x2", "x4", "x0", "x16", "x30", "xzr"])
    @operands_ptn.store("<Xn>", ["x8", "x1", "x2", "x4", "x0", "x16", "x30", "xzr"])
    @operands_ptn.store("<Xs>", ["x8", "x1", "x2", "x4", "x0", "x16", "x30", "xzr"])
    @operands_ptn.store("<Xt>", ["x8", "x1", "x2", "x4", "x0", "x16", "x30", "xzr"])
    @operands_ptn.store("<Xd>", ["x8", "x1", "x2", "x4", "x0", "x16", "x30", "xzr"])
    @operands_ptn.store("<Ws:even>,<W(s+1)>", ["w8,w9/*asm*/", "w2,w3/*asm*/", "w4,w5/*asm*/", "w0,w1/*asm*/", "w16,w17/*asm*/", "w30,wzr/*asm*/"])
    @operands_ptn.store("<Wt:even>,<W(t+1)>", ["w8,w9/*asm*/", "w2,w3/*asm*/", "w4,w5/*asm*/", "w0,w1/*asm*/", "w16,w17/*asm*/", "w30,wzr/*asm*/"])
    @operands_ptn.store("<Xs:even>,<X(s+1)>", ["x8,x9/*asm*/", "x2,x3/*asm*/", "x4,x5/*asm*/", "x0,x1/*asm*/", "x16,x17/*asm*/", "x30,xzr/*asm*/"])
    @operands_ptn.store("<Xt:even>,<X(t+1)>", ["x8,x9/*asm*/", "x2,x3/*asm*/", "x4,x5/*asm*/", "x0,x1/*asm*/", "x16,x17/*asm*/", "x30,xzr/*asm*/"])
    @operands_ptn.store("<Xt:St64b>", ["x6", "x2", "x4"]) # if Rt<4:3> == '11' || Rt<0> == '1' then UNDEFINED;

    @operands_ptn.store("[<Xn|SP>]", ["[x8]/*asm*/,ptr(x8)/*cpp*/",
                                          "[x0]/*asm*/,ptr(x0)/*cpp*/",
                                          "[x16]/*asm*/,ptr(x16)/*cpp*/",
                                          "[sp]/*asm*/,ptr(sp)/*cpp*/"])
    @operands_ptn.store("[<Xn|SP>{,#0}]", ["[x8,#0]/*asm*/,ptr(x8)/*cpp*/",
                                          "[x0, #0]/*asm*/,ptr(x0)/*cpp*/",
                                          "[x16,#0]/*asm*/,ptr(x16)/*cpp*/",
                                          "[sp,#0]/*asm*/,ptr(sp)/*cpp*/"])

    # Generate "<Bd>", "<Hd>", "<Sd>", "<Dd>", "<Qd>", "<Bn>", "<Hn>", "<Sn>", "<Dn>", "<Qn>"
    for s in ["d", "n"] do
      for t in ["B", "H", "S", "D", "Q"] do
        list = []
        for i in [8, 1, 2, 4, 0, 16, 30, 31] do
          list.push(t.downcase + i.to_s)
        end
        key = "<" + t + s + ">"
        @operands_ptn.store(key, list)
      end
    end

    # Generate "<Vd.8B>", "<Vd.16B>", "<Vd.4H>", "<Vd.8H>", "<Vd.2S>", "<Vd.4S>", "<Vd.1D>", "<Vd.2D>",
    #          "<Vn.8B>", "<Vn.16B>", "<Vn.4H>", "<Vn.8H>", "<Vn.2S>", "<Vn.4S>", "<Vn.1D>", "<Vn.2D>",
    for s in ["d", "n"] do
      for t in ["8B", "16B", "4H", "8H", "2S", "4S", "1D", "2D"] do
        list = []
        for i in [8, 1, 2, 4, 0, 16, 30, 31] do
          ptn = "v" + i.to_s + "." + t.downcase
          list.push("v" + i.to_s + "." + t.downcase)
        end
        key = "<V" + s + ">." + t
        @operands_ptn.store(key, list)
      end
    end

    @operands_ptn.store("<Zd>.B", ["z8.b", "z1.b", "z2.b", "z4.b", "z0.b", "z16.b", "z30.b", "z31.b"])
    @operands_ptn.store("<Zd>.H", ["z8.h", "z1.h", "z2.h", "z4.h", "z0.h", "z16.h", "z30.h", "z31.h"])
    @operands_ptn.store("<Zd>.S", ["z8.s", "z1.s", "z2.s", "z4.s", "z0.s", "z16.s", "z30.s", "z31.s"])
    @operands_ptn.store("<Zd>.D", ["z8.d", "z1.d", "z2.d", "z4.d", "z0.d", "z16.d", "z30.d", "z31.d"])
    @operands_ptn.store("<Zd>.Q", ["z8.q", "z1.q", "z2.q", "z4.q", "z0.q", "z16.q", "z30.q", "z31.q"])

    @operands_ptn.store("<Zda>.B", ["z8.b", "z1.b", "z2.b", "z4.b", "z0.b", "z16.b", "z30.b", "z31.b"])
    @operands_ptn.store("<Zda>.H", ["z8.h", "z1.h", "z2.h", "z4.h", "z0.h", "z16.h", "z30.h", "z31.h"])
    @operands_ptn.store("<Zda>.S", ["z8.s", "z1.s", "z2.s", "z4.s", "z0.s", "z16.s", "z30.s", "z31.s"])
    @operands_ptn.store("<Zda>.D", ["z8.d", "z1.d", "z2.d", "z4.d", "z0.d", "z16.d", "z30.d", "z31.d"])

    @operands_ptn.store("<Zdn>.B", ["z8.b", "z1.b", "z2.b", "z4.b", "z0.b", "z16.b", "z30.b", "z31.b"])
    @operands_ptn.store("<Zdn>.H", ["z8.h", "z1.h", "z2.h", "z4.h", "z0.h", "z16.h", "z30.h", "z31.h"])
    @operands_ptn.store("<Zdn>.S", ["z8.s", "z1.s", "z2.s", "z4.s", "z0.s", "z16.s", "z30.s", "z31.s"])
    @operands_ptn.store("<Zdn>.D", ["z8.d", "z1.d", "z2.d", "z4.d", "z0.d", "z16.d", "z30.d", "z31.d"])

    # Generate "<Zdn>.(B|H|S|D), <Pg>/m, <Zdn>.(B|H|S|D)"
    for t in ["B", "H", "S", "D"] do
      list = []
      for i in [8, 1, 2, 4, 0, 16, 30, 31] do
        for j in [7] do
          str_i = i.to_s
          str_j = j.to_s
          list.push("z" + str_i + "." + t.downcase + ",p" + str_j + "/m,z" + str_i + "." + t.downcase + "/*asm*/")
        end
      end
      for i in [8] do
        for j in [7, 1, 2, 4, 0] do
          str_i = i.to_s
          str_j = j.to_s
          list.push("z" + str_i + "." + t.downcase + ",p" + str_j + "/m,z" + str_i + "." + t.downcase + "/*asm*/")
        end
      end
      key = "<Zdn>." + t + ",<Pg>/m,<Zdn>." + t
      @operands_ptn.store(key, list)
    end

    # <Zdn:asm>.? are remoed for CPP.
    @operands_ptn.store("<Zdn:asm>.B", ["z8.b/*asm*/", "z1.b/*asm*/", "z2.b/*asm*/", "z4.b/*asm*/",
                                        "z0.b/*asm*/", "z16.b/*asm*/", "z30.b/*asm*/", "z31.b"])
    @operands_ptn.store("<Zdn:asm>.H", ["z8.h/*asm*/", "z1.h/*asm*/", "z2.h/*asm*/", "z4.h/*asm*/",
                                        "z0.h/*asm*/", "z16.h/*asm*/", "z30.h/*asm*/", "z31.h"])
    @operands_ptn.store("<Zdn:asm>.S", ["z8.s/*asm*/", "z1.s/*asm*/", "z2.s/*asm*/", "z4.s/*asm*/",
                                        "z0.s/*asm*/", "z16.s/*asm*/", "z30.s/*asm*/", "z31.s"])
    @operands_ptn.store("<Zdn:asm>.D", ["z8.d/*asm*/", "z1.d/*asm*/", "z2.d/*asm*/", "z4.d/*asm*/",
                                        "z0.d/*asm*/", "z16.d/*asm*/", "z30.d/*asm*/", "z31.d"])

    @operands_ptn.store("<Zn>.B", ["z8.b", "z1.b", "z2.b", "z4.b", "z0.b", "z16.b", "z30.b", "z31.b"])
    @operands_ptn.store("<Zn>.H", ["z8.h", "z1.h", "z2.h", "z4.h", "z0.h", "z16.h", "z30.h", "z31.h"])
    @operands_ptn.store("<Zn>.S", ["z8.s", "z1.s", "z2.s", "z4.s", "z0.s", "z16.s", "z30.s", "z31.s"])
    @operands_ptn.store("<Zn>.D", ["z8.d", "z1.d", "z2.d", "z4.d", "z0.d", "z16.d", "z30.d", "z31.d"])
    @operands_ptn.store("<Zn>.Q", ["z8.q", "z1.q", "z2.q", "z4.q", "z0.q", "z16.q", "z30.q", "z31.q"])
    @operands_ptn.store("<Zm>.B", ["z8.b", "z1.b", "z2.b", "z4.b", "z0.b", "z16.b", "z30.b", "z31.b"])
    @operands_ptn.store("<Zm>.H", ["z8.h", "z1.h", "z2.h", "z4.h", "z0.h", "z16.h", "z30.h", "z31.h"])
    @operands_ptn.store("<Zm>.S", ["z8.s", "z1.s", "z2.s", "z4.s", "z0.s", "z16.s", "z30.s", "z31.s"])
    @operands_ptn.store("<Zm>.D", ["z8.d", "z1.d", "z2.d", "z4.d", "z0.d", "z16.d", "z30.d", "z31.d"])
    @operands_ptn.store("<Zk>.B", ["z8.b", "z1.b", "z2.b", "z4.b", "z0.b", "z16.b", "z30.b", "z31.b"])
    @operands_ptn.store("<Zk>.H", ["z8.h", "z1.h", "z2.h", "z4.h", "z0.h", "z16.h", "z30.h", "z31.h"])
    @operands_ptn.store("<Zk>.S", ["z8.s", "z1.s", "z2.s", "z4.s", "z0.s", "z16.s", "z30.s", "z31.s"])
    @operands_ptn.store("<Zk>.D", ["z8.d", "z1.d", "z2.d", "z4.d", "z0.d", "z16.d", "z30.d", "z31.d"])

    @operands_ptn.store("{<Zt>.S}", ["{z8.s}", "{z1.s}", "{z2.s}", "{z4.s}", "{z0.s}", "{z16.s}", "{z30.s}", "{z31.s}"])
    @operands_ptn.store("{<Zt>.D}", ["{z8.d}", "{z1.d}", "{z2.d}", "{z4.d}", "{z0.d}", "{z16.d}", "{z30.d}", "{z31.d}"])

    @operands_ptn.store("<Zm:3>.B[<imm:2>]", ["z7.b[3]", "z1.b[1]", "z2.b[0]", "z4.b[2]"])

    @operands_ptn.store("<Zm:3>.H[<imm:2>]", ["z7.h[3]", "z1.h[1]", "z0.h[0]", "z4.h[2]"])
    @operands_ptn.store("<Zm:3>.H[<imm:3>]", ["z7.h[1]", "z1.h[7]", "z0.h[4]", "z4.h[0]"])
    @operands_ptn.store("<Zm:3>.H[<imm:4>]", ["z7.h[15]", "z1.h[1]", "z0.h[0]", "z4.h[2]"])
    @operands_ptn.store("<Zm:4>.H[<imm:1>]", ["z15.h[1]", "z1.h[0]", "z0.h[1]", "z8.h[1]"])

    @operands_ptn.store("<Zm:3>.S[<imm:2>]", ["z7.s[1]", "z1.s[3]", "z0.s[0]", "z4.s[2]"])
    @operands_ptn.store("<Zm:4>.S[<imm:1>]", ["z15.s[1]", "z1.s[0]", "z0.s[1]", "z8.s[0]"])

    @operands_ptn.store("<Zm:4>.S[<imm:2>]", ["z15.s[1]", "z1.s[3]", "z0.s[0]", "z4.s[2]"])

    @operands_ptn.store("<Zm:3>.D[<imm:1>]", ["z7.d[1]", "z1.d[0]", "z0.d[1]", "z4.d[0]"])
    @operands_ptn.store("<Zm:4>.D[<imm:1>]", ["z15.d[1]", "z1.d[0]", "z0.d[1]", "z8.d[0]"])

    # ",z8.b/*asm*/" is removed for CPP by "convert_for_cpp() function".
    @operands_ptn.store("<Zdn>.B,<Zdn>.B", ["z8.b,z8.b/*asm*/", "z1.b,z1.b/*asm*/", "z2.b,z2.b/*asm*/", "z4.b,z4.b/*asm*/",
                                            "z0.b,z0.b/*asm*/", "z16.b,z16.b/*asm*/", "z30.b,z30.b/*asm*/", "z31.b,z31.b/*asm*/"])
    @operands_ptn.store("<Zdn>.H,<Zdn>.H", ["z8.h,z8.h/*asm*/", "z1.h,z1.h/*asm*/", "z2.h,z2.h/*asm*/", "z4.h,z4.h/*asm*/",
                                            "z0.h,z0.h/*asm*/", "z16.h,z16.h/*asm*/", "z30.h,z30.h/*asm*/", "z31.h,z31.h/*asm*/"])
    @operands_ptn.store("<Zdn>.S,<Zdn>.S", ["z8.s,z8.s/*asm*/", "z1.s,z1.s/*asm*/", "z2.s,z2.s/*asm*/", "z4.s,z4.s/*asm*/",
                                            "z0.s,z0.s/*asm*/", "z16.s,z16.s/*asm*/", "z30.s,z30.s/*asm*/", "z31.s,z31.s/*asm*/"])
    @operands_ptn.store("<Zdn>.D,<Zdn>.D", ["z8.d,z8.d/*asm*/", "z1.d,z1.d/*asm*/", "z2.d,z2.d/*asm*/", "z4.d,z4.d/*asm*/",
                                            "z0.d,z0.d/*asm*/", "z16.d,z16.d/*asm*/", "z30.d,z30.d/*asm*/", "z31.d,z31.d/*asm*/"])

    @operands_ptn.store("{<Zn>.B}", ["<{z8.b}|z8.b>", "<{z1.b}|z1.b>", "<{z2.b}|z2.b>", "<{z4.b}|z4.b>",
                                     "<{z0.b}|z0.b>", "<{z16.b}|z16.b>", "<{z30.b}|z30.b>", "<{z31.b}|z31.b>"])
    @operands_ptn.store("{<Zn>.H}", ["<{z8.h}|z8.h>", "<{z1.h}|z1.h>", "<{z2.h}|z2.h>", "<{z4.h}|z4.h>",
                                     "<{z0.h}|z0.h>", "<{z16.h}|z16.h>", "<{z30.h}|z30.h>", "<{z31.h}|z31.h>"])
    @operands_ptn.store("{<Zn>.S}", ["<{z8.s}|z8.s>", "<{z1.s}|z1.s>", "<{z2.s}|z2.s>", "<{z4.s}|z4.s>",
                                     "<{z0.s}|z0.s>", "<{z16.s}|z16.s>", "<{z30.s}|z30.s>", "<{z31.s}|z31.s>"])
    @operands_ptn.store("{<Zn>.D}", ["<{z8.d}|z8.d>", "<{z1.d}|z1.d>", "<{z2.d}|z2.d>", "<{z4.d}|z4.d>",
                                     "<{z0.d}|z0.d>", "<{z16.d}|z16.d>", "<{z30.d}|z30.d>", "<{z31.d}|z31.d>"])

    @operands_ptn.store("{<Zn1>.B,<Zn2>.B}", ["<{z8.b,z9.b}|(z8.b-z9.b)>", "<{z1.b,z2.b}|(z1.b-z2.b)>", "<{z2.b,z3.b}|(z2.b-z3.b)>",
                                              "<{z4.b,z5.b}|(z4.b-z5.b)>", "<{z0.b,z1.b}|(z0.b-z1.b)>", "<{z16.b,z17.b}|(z16.b-z17.b)>",
                                              "<{z30.b,z31.b}|(z30.b-z31.b)>", "<{z31.b,z0.b}|(z31.b-z0.b)>"])
    @operands_ptn.store("{<Zn1>.H,<Zn2>.H}", ["<{z8.h,z9.h}|(z8.h-z9.h)>", "<{z1.h,z2.h}|(z1.h-z2.h)>", "<{z2.h,z3.h}|(z2.h-z3.h)>",
                                              "<{z4.h,z5.h}|(z4.h-z5.h)>", "<{z0.h,z1.h}|(z0.h-z1.h)>", "<{z16.h,z17.h}|(z16.h-z17.h)>",
                                              "<{z30.h,z31.h}|(z30.h-z31.h)>", "<{z31.h,z0.h}|(z31.h-z0.h)>"])
    @operands_ptn.store("{<Zn1>.S,<Zn2>.S}", ["<{z8.s,z9.s}|(z8.s-z9.s)>", "<{z1.s,z2.s}|(z1.s-z2.s)>", "<{z2.s,z3.s}|(z2.s-z3.s)>",
                                              "<{z4.s,z5.s}|(z4.s-z5.s)>", "<{z0.s,z1.s}|(z0.s-z1.s)>", "<{z16.s,z17.s}|(z16.s-z17.s)>",
                                              "<{z30.s,z31.s}|(z30.s-z31.s)>", "<{z31.s,z0.s}|(z31.s-z0.s)>"])
    @operands_ptn.store("{<Zn1>.D,<Zn2>.D}", ["<{z8.d,z9.d}|(z8.d-z9.d)>", "<{z1.d,z2.d}|(z1.d-z2.d)>", "<{z2.d,z3.d}|(z2.d-z3.d)>",
                                              "<{z4.d,z5.d}|(z4.d-z5.d)>", "<{z0.d,z1.d}|(z0.d-z1.d)>", "<{z16.d,z17.d}|(z16.d-z17.d)>",
                                              "<{z30.d,z31.d}|(z30.d-z31.d)>", "<{z31.d,z0.d}|(z31.d-z0.d)>"])

    @operands_ptn.store("[<Zn>.S{,<Xm>}]", ["<[z7.s]|ptr(z7.s, xzr)>", "<[z7.s, x30]|ptr(z7.s, x30)>", "<[z7.s, x0]|ptr(z7.s, x0)>",
                                            "<[z31.s]|ptr(z31.s, xzr)>", "<[z31.s, x30]|ptr(z31.s, x30)>", "<[z31.s, x0]|ptr(z31.s, x0)>",
                                            "<[z0.s]|ptr(z0.s, xzr)>", "<[z0.s, x30]|ptr(z0.s, x30)>", "<[z0.s, x0]|ptr(z0.s, x0)>"])
    @operands_ptn.store("[<Zn>.D{,<Xm>}]", ["<[z7.d]|ptr(z7.d, xzr)>", "<[z7.d, x30]|ptr(z7.d, x30)>", "<[z7.d, x0]|ptr(z7.d, x0)>",
                                            "<[z31.d]|ptr(z31.d, xzr)>", "<[z31.d, x30]|ptr(z31.d, x30)>", "<[z31.d, x0]|ptr(z31.d, x0)>",
                                            "<[z0.d]|ptr(z0.d, xzr)>", "<[z0.d, x30]|ptr(z0.d, x30)>", "<[z0.d, x0]|ptr(z0.d, x0)>"])

    @operands_ptn.store("<Pd>.B", ["p7.b", "p1.b", "p2.b", "p4.b", "p0.b"])
    @operands_ptn.store("<Pd>.H", ["p7.h", "p1.h", "p2.h", "p4.h", "p0.h"])
    @operands_ptn.store("<Pd>.S", ["p7.s", "p1.s", "p2.s", "p4.s", "p0.s"])
    @operands_ptn.store("<Pd>.D", ["p7.d", "p1.d", "p2.d", "p4.d", "p0.d"])

    @operands_ptn.store("<Pg>", ["p7", "p1", "p2", "p4", "p0"])
    @operands_ptn.store("<Pg>/M", ["p7/m", "p1/m", "p2/m", "p4/m", "p0/m"])
    @operands_ptn.store("<Pg>/Z", ["p7/z", "p1/z", "p2/z", "p4/z", "p0/z"])
    @operands_ptn.store("<Pd:all>.B", ["p15.b", "p1.b", "p0.b", "p8.b"])
    @operands_ptn.store("<Pd:all>.H", ["p15.h", "p1.h", "p0.h", "p8.h"])

    @operands_ptn.store("#<imm16>", ["1", "(1<<4)", "(1<<8)", "(1<<12)", "(0xffff)"])
    @operands_ptn.store("<const:rot>", ["90", "0", "180", "270"])
    @operands_ptn.store("<const:rot2>", ["90", "270"])
    @operands_ptn.store("#<const:3>", ["(1<<3)-1", "1<<1", "0", "1<<2"])
    @operands_ptn.store("#<const:4>", ["(1<<4)-1", "1<<3", "0", "1<<2"])
    @operands_ptn.store("#<const:5>", ["(1<<5)-1", "1<<3", "0", "1<<2"])
    @operands_ptn.store("#<const:6>", ["(1<<6)-1", "1<<3", "0", "1<<2"])

    @operands_ptn.store("#<const:3:no0>", ["(1<<3)-1", "1<<1", "1<<2"])
    @operands_ptn.store("#<const:4:no0>", ["(1<<4)-1", "1<<3", "1<<2"])
    @operands_ptn.store("#<const:5:no0>", ["(1<<5)-1", "1<<3", "1<<2"])
    @operands_ptn.store("#<const:6:no0>", ["(1<<6)-1", "1<<3", "1<<2"])

    # SME specific
    @operands_ptn.store("<Pn>/M", ["p7/m", "p1/m", "p2/m", "p4/m", "p0/m"])
    @operands_ptn.store("<Pm>/M", ["p7/m", "p1/m", "p2/m", "p4/m", "p0/m"])
    @operands_ptn.store("<ZAda>.S", ["za3.s", "za0.s"])
    @operands_ptn.store("<ZAda>.D", ["za3.d", "za0.d", "za7.d", "za5.d"])
    @operands_ptn.store("<Xd|SP>", ["x8", "x1", "x2", "x4", "x0", "x16", "x30", "sp"])
    @operands_ptn.store("<Xn|SP>", ["x8", "x1", "x2", "x4", "x0", "x16", "x30", "sp"])
    @operands_ptn.store("#<imm>", ["-32", "-15", "-1", "0", "7", "16", "31"])
    @operands_ptn.store("{ZA0<HV>.B[<Ws>,<offs>]}", ["<{za0h.b[w12,#0]}|za0h.b(w12,0)>", "<{za0v.b[w13,#15]}|za0v.b(w13,15)>", "<{za0v.b[w15,#8]}|za0v.b(w15,8)>"])
    @operands_ptn.store("ZA0<HV>.B[<Ws>,<offs>]", ["<za0h.b[w12,#0]|za0h.b(w12,0)>", "<za0v.b[w13,#15]|za0v.b(w13,15)>", "<za0v.b[w15,#8]|za0v.b(w15,8)>"])
    @operands_ptn.store("[<Xn|SP>{,<Xm>}]", ["<[x10]|ptr(x10)>",
                                             "<[sp]|ptr(sp)>",
                                             "<[x30,x0]|ptr(x30,x0)>",
                                             "<[x0,x10]|ptr(x0,x10)>",
                                             "<[sp,x30]|ptr(sp,x30)>"])
    @operands_ptn.store("{<ZAt><HV>.H[<Ws>,<offs>]}", ["<{za0h.h[w12,#0]}|za0h.h(w12,0)>", "<{za1h.h[w12,#5]}|za1h.h(w12,5)>", "<{za0v.h[w13,#3]}|za0v.h(w13,3)>", "<{za1v.h[w15,#7]}|za1v.h(w15,7)>"])
    @operands_ptn.store("<ZAn><HV>.H[<Ws>,<offs>]", ["<za0h.h[w12,#0]|za0h.h(w12,0)>", "<za1h.h[w12,#5]|za1h.h(w12,5)>", "<za0v.h[w13,#3]|za0v.h(w13,3)>", "<za1v.h[w15,#7]|za1v.h(w15,7)>"])
    @operands_ptn.store("<ZAd><HV>.H[<Ws>,<offs>]", ["<za0h.h[w12,#0]|za0h.h(w12,0)>", "<za1h.h[w12,#5]|za1h.h(w12,5)>", "<za0v.h[w13,#3]|za0v.h(w13,3)>", "<za1v.h[w15,#7]|za1v.h(w15,7)>"])
    @operands_ptn.store("[<Xn|SP>{,<Xm>,LSL #1}]", ["<[x10]|ptr(x10)>",
                                                    "<[sp]|ptr(sp)>",
                                                    "<[x30,x0,lsl #1]|ptr(x30,x0/*lsl #1*/)>",
                                                    "<[x0,x10,lsl #1]|ptr(x0,x10/*lsl #1*/)>",
                                                    "<[sp,x30,lsl #1]|ptr(sp,x30/*lsl #1*/)>"])
    @operands_ptn.store("{<ZAt><HV>.S[<Ws>,<offs>]}", ["<{za0h.s[w12,#0]}|za0h.s(w12,0)>", "<{za1h.s[w14,#3]}|za1h.s(w14,3)>", "<{za0v.s[w13,#2]}|za0v.s(w13,2)>", "<{za3v.s[w15,#1]}|za3v.s(w15,1)>"])
    @operands_ptn.store("<ZAn><HV>.S[<Ws>,<offs>]", ["<za0h.s[w12,#0]|za0h.s(w12,0)>", "<za1h.s[w14,#3]|za1h.s(w14,3)>", "<za0v.s[w13,#2]|za0v.s(w13,2)>", "<za3v.s[w15,#1]|za3v.s(w15,1)>"])
    @operands_ptn.store("<ZAd><HV>.S[<Ws>,<offs>]", ["<za0h.s[w12,#0]|za0h.s(w12,0)>", "<za1h.s[w14,#3]|za1h.s(w14,3)>", "<za0v.s[w13,#2]|za0v.s(w13,2)>", "<za3v.s[w15,#1]|za3v.s(w15,1)>"])
    @operands_ptn.store("[<Xn|SP>{,<Xm>,LSL #2}]", ["<[x10]|ptr(x10)>",
                                                    "<[sp]|ptr(sp)>",
                                                    "<[x30,x0,lsl #2]|ptr(x30,x0/*lsl #2*/)>",
                                                    "<[x0,x10,lsl #2]|ptr(x0,x10/*lsl #2*/)>",
                                                    "<[sp,x30,lsl #2]|ptr(sp,x30/*lsl #2*/)>"])
    @operands_ptn.store("{<ZAt><HV>.D[<Ws>,<offs>]}", ["<{za0h.d[w12,#0]}|za0h.d(w12,0)>", "<{za3h.d[w14,#1]}|za3h.d(w14,1)>", "<{za7h.d[w14,#0]}|za7h.d(w14,0)>", 
                                                       "<{za0v.d[w13,#0]}|za0v.d(w13,0)>", "<{za3v.d[w13,#0]}|za3v.d(w13,0)>", "<{za7v.d[w15,#1]}|za7v.d(w15,1)>"])
    @operands_ptn.store("<ZAn><HV>.D[<Ws>,<offs>]", ["<za0h.d[w12,#0]|za0h.d(w12,0)>", "<za3h.d[w14,#1]|za3h.d(w14,1)>", "<za7h.d[w14,#0]|za7h.d(w14,0)>", 
                                                     "<za0v.d[w13,#0]|za0v.d(w13,0)>", "<za3v.d[w13,#0]|za3v.d(w13,0)>", "<za7v.d[w15,#1]|za7v.d(w15,1)>"])
    @operands_ptn.store("<ZAd><HV>.D[<Ws>,<offs>]", ["<za0h.d[w12,#0]|za0h.d(w12,0)>", "<za3h.d[w14,#1]|za3h.d(w14,1)>", "<za7h.d[w14,#0]|za7h.d(w14,0)>", 
                                                     "<za0v.d[w13,#0]|za0v.d(w13,0)>", "<za3v.d[w13,#0]|za3v.d(w13,0)>", "<za7v.d[w15,#1]|za7v.d(w15,1)>"])
    @operands_ptn.store("[<Xn|SP>{,<Xm>,LSL #3}]", ["<[x10]|ptr(x10)>",
                                                    "<[sp]|ptr(sp)>",
                                                    "<[x30,x0,lsl #3]|ptr(x30,x0/*lsl #3*/)>",
                                                    "<[x0,x10,lsl #3]|ptr(x0,x10/*lsl #3*/)>",
                                                    "<[sp,x30,lsl #3]|ptr(sp,x30/*lsl #3*/)>"])
    @operands_ptn.store("{<ZAt><HV>.Q[<Ws>,<offs>]}", ["<{za0h.q[w12,#0]}|za0h.q(w12,0)>", "<{za8h.q[w14,#0]}|za8h.q(w14,0)>", "<{za15h.q[w14,#0]}|za15h.q(w14,0)>", 
                                                       "<{za0v.q[w13,#0]}|za0v.q(w13,0)>", "<{za8v.q[w13,#0]}|za8v.q(w13,0)>", "<{za15v.q[w15,#0]}|za15v.q(w15,0)>"])
    @operands_ptn.store("<ZAn><HV>.Q[<Ws>,<offs>]", ["<za0h.q[w12,#0]|za0h.q(w12,0)>", "<za8h.q[w14,#0]|za8h.q(w14,0)>", "<za15h.q[w14,#0]|za15h.q(w14,0)>", 
                                                     "<za0v.q[w13,#0]|za0v.q(w13,0)>", "<za8v.q[w13,#0]|za8v.q(w13,0)>", "<za15v.q[w15,#0]|za15v.q(w15,0)>"])
    @operands_ptn.store("<ZAd><HV>.Q[<Ws>,<offs>]", ["<za0h.q[w12,#0]|za0h.q(w12,0)>", "<za8h.q[w14,#0]|za8h.q(w14,0)>", "<za15h.q[w14,#0]|za15h.q(w14,0)>", 
                                                     "<za0v.q[w13,#0]|za0v.q(w13,0)>", "<za8v.q[w13,#0]|za8v.q(w13,0)>", "<za15v.q[w15,#0]|za15v.q(w15,0)>"])
    @operands_ptn.store("[<Xn|SP>{,<Xm>,LSL #4}]", ["<[x10]|ptr(x10)>",
                                                    "<[sp]|ptr(sp)>",
                                                    "<[x30,x0,lsl #4]|ptr(x30,x0/*lsl #4*/)>",
                                                    "<[x0,x10,lsl #4]|ptr(x0,x10/*lsl #4*/)>",
                                                    "<[sp,x30,lsl #4]|ptr(sp,x30/*lsl #4*/)>"])
    @operands_ptn.store("ZA[<Wv>,<offs>],[<Xn|SP>{,#<offs>,MUL VL}]", ["<za[w12,#0],[x10]|za(w12,0),ptr(x10)>",
                                                                       "<za[w13,#8],[sp,#8,mul vl]|za(w13,8),ptr(sp,8/*mul vl*/)>",
                                                                       "<za[w15,#15],[x30,#15,mul vl]|za(w15,15),ptr(x30,15/*mul vl*/)>"])
    @operands_ptn.store("{{<mask>}}", ["{}", "{za0.d,za1.d,za4.d,za5.d}", "<{za}|za>", "{za0.d,za2.d,za1.d,za6.d}"])
    @operands_ptn.store("{<option>}", ["SM", "ZA", ""])
    list = []
    for n in 16..31 do
      for r in -3..4 do
        val = (n / 16.0) * (2**r)
        list.push((-val).to_s)
        list.push(val.to_s)
      end
    end
    key = "#<const:fp8>"
    @operands_ptn.store(key, list)

    @operands_ptn.store("OP:0", ["OP:0:/*asm*/"])
  end

  def output_cpp(ofile)
    File.open(ofile,"w") do |f|
      f.puts "void gen() {"
      @instructions.each{|inst|
        tmp = inst.dup
        #puts tmp
        f.puts convert_for_cpp(tmp)
      }
      f.puts "}"
    end
  end

  def output_asm(ofile)
    File.open(ofile,"w") do |f|
      @instructions.each{|inst|
        tmp = inst.dup
        f.puts convert_for_asm(tmp)
      }
    end
  end

  private
end

STDERR.print "Ruby 2.5.5 or higher required\n" if Gem::Version.create(RUBY_VERSION) < Gem::Version.create("2.5.5")
pgen_all = TestPatternGenerator.new
pgen_all.parseTest(ARGV[0])
pgen_all.output_instruction_patterns()

# Replace path name into file name
# Example "instructions/s" -> "instructions_s"
argv0 = ARGV[0].dup.gsub!(/\//, "_")

pgen_all.output_cpp(argv0 + ".cpp")
pgen_all.output_asm(argv0 + ".asm")

