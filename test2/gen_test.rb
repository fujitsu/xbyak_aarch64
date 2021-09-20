#!/usr/bin/ruby
#*******************************************************************************
# Copyright 2021 FUJITSU LIMITED
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
require 'byebug'

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
    Dir.glob(dirname+'/**/*.test').each{|item|
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
    ptn_line.sub!(/ /, ",")

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
    # Replace "<Zdn>.D,<Zdn>.d" -? ZDN_D_PAIR"
    ptn_line.gsub!(/<Ws:even>,<W\(s\+1\)>/, "WS_PAIR")
    ptn_line.gsub!(/<Wt:even>,<W\(t\+1\)>/, "WT_PAIR")
    ptn_line.gsub!(/<Xs:even>,<X\(s\+1\)>/, "XS_PAIR")
    ptn_line.gsub!(/<Xt:even>,<X\(t\+1\)>/, "XT_PAIR")
    ptn_line.gsub!(/<Zdn>.B,<Zdn>.B/, "ZDN_B_PAIR")
    ptn_line.gsub!(/<Zdn>.H,<Zdn>.H/, "ZDN_H_PAIR")
    ptn_line.gsub!(/<Zdn>.S,<Zdn>.S/, "ZDN_S_PAIR")
    ptn_line.gsub!(/<Zdn>.D,<Zdn>.D/, "ZDN_D_PAIR")
    
    # Split mnemonic and operands
    STDOUT.flush
    tmp = ptn_line.split(",")

    # Recover "WS_PAIR" -> "<Ws:even>,<W(s+1)"
    # Recover "WT_PAIR" -> "<Wt:even>,<W(t+1)"
    # Recover "XS_PAIR" -> "<Xs:even>,<X(s+1)"
    # Recover "XT_PAIR" -> "<Xt:even>,<X(t+1)"
    # Recover "ZDN_D_PAIR" -> "<Zdn>.D,<Zdn>.D"
    # Recover "%" into "{,"
    for i in 0..tmp.size-1 do
      tmp[i].gsub!(/WS_PAIR/, "<Ws:even>,<W(s+1)>")
      tmp[i].gsub!(/WT_PAIR/, "<Wt:even>,<W(t+1)>")
      tmp[i].gsub!(/XS_PAIR/, "<Xs:even>,<X(s+1)>")
      tmp[i].gsub!(/XT_PAIR/, "<Xt:even>,<X(t+1)>")
      tmp[i].gsub!(/ZDN_B_PAIR/, "<Zdn>.B,<Zdn>.B")
      tmp[i].gsub!(/ZDN_H_PAIR/, "<Zdn>.H,<Zdn>.H")
      tmp[i].gsub!(/ZDN_S_PAIR/, "<Zdn>.S,<Zdn>.S")
      tmp[i].gsub!(/ZDN_D_PAIR/, "<Zdn>.D,<Zdn>.D")
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
    # Remove register pair
    # Example ",w1/*cpp*/" -> ""
    inst.gsub!(/,[^,]+\/\*cpp\*\//, "")
    return inst
  end

  def convert_for_cpp(inst)
    inst.downcase!

    # Remove address operands for ASM
    # Example ",[x8]/*asm*/,ptr(x8)/*cpp*/" -> "ptr(x8)/*cpp*/"
    inst.gsub!(/,\[[^\[]+\/\*asm\*\//, "")

    # Remove register pair
    # Example "w0,w1/*asm*/" -> "w0"
    inst.gsub!(/,[^,]+\/\*asm\*\//, "")

    # Replace "m" -> "T_m"
    # Replace "z" -> "T_z"
    inst.sub!(/\/m/, "/T_m")
    inst.sub!(/\/z/, "/T_z")

    tmp = inst.split(/\s+/)
    if tmp.size == 1 # no operands
      inst += "("
    else
      inst.sub!(/ /, "(")
    end

    inst += "); dump();"
    inst.sub!(/and\(/, "and_(")

    # Replace ",[" -> ",ptr["
    #inst.sub!(/,\[/, ",ptr(")

    # Replace "]" -> ")"
    #inst.sub!(/ptr\(([^\]]+)\]/} { "ptr(" + $1 + ")" }
    #inst.sub!(/ptr\(([^\]]+)\]/, hoge

    return inst
  end
  
  def operands_ptn_init
    @operands_ptn.store("<Ws>", ["w8", "w1", "w2", "w4", "w0", "w16", "w30", "wzr"])
    @operands_ptn.store("<Wt>", ["w8", "w1", "w2", "w4", "w0", "w16", "w30", "wzr"])
    @operands_ptn.store("<Xs>", ["x8", "x1", "x2", "x4", "x0", "x16", "x30", "xzr"])
    @operands_ptn.store("<Xt>", ["x8", "x1", "x2", "x4", "x0", "x16", "x30", "xzr"])
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

    @operands_ptn.store("<Zn>.B", ["z8.b", "z1.b", "z2.b", "z4.b", "z0.b", "z16.b", "z30.b", "z31.b"])
    @operands_ptn.store("<Zn>.H", ["z8.h", "z1.h", "z2.h", "z4.h", "z0.h", "z16.h", "z30.h", "z31.h"])
    @operands_ptn.store("<Zn>.S", ["z8.s", "z1.s", "z2.s", "z4.s", "z0.s", "z16.s", "z30.s", "z31.s"])
    @operands_ptn.store("<Zn>.D", ["z8.d", "z1.d", "z2.d", "z4.d", "z0.d", "z16.d", "z30.d", "z31.d"])
    @operands_ptn.store("<Zm>.B", ["z8.b", "z1.b", "z2.b", "z4.b", "z0.b", "z16.b", "z30.b", "z31.b"])
    @operands_ptn.store("<Zm>.H", ["z8.h", "z1.h", "z2.h", "z4.h", "z0.h", "z16.h", "z30.h", "z31.h"])
    @operands_ptn.store("<Zm>.S", ["z8.s", "z1.s", "z2.s", "z4.s", "z0.s", "z16.s", "z30.s", "z31.s"])
    @operands_ptn.store("<Zm>.D", ["z8.d", "z1.d", "z2.d", "z4.d", "z0.d", "z16.d", "z30.d", "z31.d"])
    @operands_ptn.store("<Zk>.B", ["z8.b", "z1.b", "z2.b", "z4.b", "z0.b", "z16.b", "z30.b", "z31.b"])
    @operands_ptn.store("<Zk>.H", ["z8.h", "z1.h", "z2.h", "z4.h", "z0.h", "z16.h", "z30.h", "z31.h"])
    @operands_ptn.store("<Zk>.S", ["z8.s", "z1.s", "z2.s", "z4.s", "z0.s", "z16.s", "z30.s", "z31.s"])
    @operands_ptn.store("<Zk>.D", ["z8.d", "z1.d", "z2.d", "z4.d", "z0.d", "z16.d", "z30.d", "z31.d"])

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

    @operands_ptn.store("<Zdn>.B,<Zdn>.B", ["z8.b,z8.b/*asm*/", "z1.b,z1.b/*asm*/", "z2.b,z2.b/*asm*/", "z4.b,z4.b/*asm*/",
                                            "z0.b,z0.b/*asm*/", "z16.b,z16.b/*asm*/", "z30.b,z30.b/*asm*/", "z31.b,z31.b/*asm*/"])
    @operands_ptn.store("<Zdn>.H,<Zdn>.H", ["z8.h,z8.h/*asm*/", "z1.h,z1.h/*asm*/", "z2.h,z2.h/*asm*/", "z4.h,z4.h/*asm*/",
                                            "z0.h,z0.h/*asm*/", "z16.h,z16.h/*asm*/", "z30.h,z30.h/*asm*/", "z31.h,z31.h/*asm*/"])
    @operands_ptn.store("<Zdn>.S,<Zdn>.S", ["z8.s,z8.s/*asm*/", "z1.s,z1.s/*asm*/", "z2.s,z2.s/*asm*/", "z4.s,z4.s/*asm*/",
                                            "z0.s,z0.s/*asm*/", "z16.s,z16.s/*asm*/", "z30.s,z30.s/*asm*/", "z31.s,z31.s/*asm*/"])
    @operands_ptn.store("<Zdn>.D,<Zdn>.D", ["z8.d,z8.d/*asm*/", "z1.d,z1.d/*asm*/", "z2.d,z2.d/*asm*/", "z4.d,z4.d/*asm*/",
                                            "z0.d,z0.d/*asm*/", "z16.d,z16.d/*asm*/", "z30.d,z30.d/*asm*/", "z31.d,z31.d/*asm*/"])

    @operands_ptn.store("<Pg>/M", ["p7/m", "p1/m", "p2/m", "p4/m", "p0/m"])

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

