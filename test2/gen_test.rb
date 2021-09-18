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

    # Replace "<Ws:even>, <W(s+1)" -> "WS_PARE"
    # Replace "<Wt:even>, <W(t+1)" -> "WT_PARE"
    # Replace "<Xs:even>, <X(s+1)" -> "XS_PARE"
    # Replace "<Xt:even>, <X(t+1)" -> "XT_PARE"
    ptn_line.gsub!(/<Ws:even>,<W\(s\+1\)>/, "WS_PARE")
    ptn_line.gsub!(/<Wt:even>,<W\(t\+1\)>/, "WT_PARE")
    ptn_line.gsub!(/<Xs:even>,<X\(s\+1\)>/, "XS_PARE")
    ptn_line.gsub!(/<Xt:even>,<X\(t\+1\)>/, "XT_PARE")
    
    # Split mnemonic and operands
    STDOUT.flush
    tmp = ptn_line.split(",")

    # Recover "WS_PARE" -> "<Ws:even>, <W(s+1)"
    # Recover "WT_PARE" -> "<Wt:even>, <W(t+1)"
    # Recover "XS_PARE" -> "<Xs:even>, <X(s+1)"
    # Recover "XT_PARE" -> "<Xt:even>, <X(t+1)"
    # Recover "%" into "{,"
    for i in 0..tmp.size-1 do
      tmp[i].gsub!(/WS_PARE/, "<Ws:even>,<W(s+1)>")
      tmp[i].gsub!(/WT_PARE/, "<Wt:even>,<W(t+1)>")
      tmp[i].gsub!(/XS_PARE/, "<Xs:even>,<X(s+1)>")
      tmp[i].gsub!(/XT_PARE/, "<Xt:even>,<X(t+1)>")
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

      for j in 0..(operands_combinations[i]).size-1 do
        if j != 0
          inst += ","
        else
          inst += " "
        end
        inst += (operands_combinations[i])[j]
      end

      @instructions.push(inst)
    end
  end

  def convert_for_cpp(inst)
    inst.gsub!(/,[^,]+\/\*asm\*\//, "")

    tmp = inst.split(/\s+/)
    if tmp.size == 1 # no operands
      inst += "("
    else
      inst.sub!(/ /, "(")
    end

    inst.downcase!
    inst += "); dump();"
    inst.sub!(/and\(/, "and_(")
    inst.sub!(/\[/, "ptr(")
    inst.sub!(/\]/, ")")

    return inst
  end
  
  def operands_ptn_init
    @operands_ptn.store("<Ws>", ["w0", "w1", "w2", "w4", "w8", "w16", "w30", "wzr"])
    @operands_ptn.store("<Wt>", ["w0", "w1", "w2", "w4", "w8", "w16", "w30", "wzr"])
    @operands_ptn.store("<Xs>", ["x0", "x1", "x2", "x4", "x8", "x16", "x30", "xzr"])
    @operands_ptn.store("<Xt>", ["x0", "x1", "x2", "x4", "x8", "x16", "x30", "xzr"])
    @operands_ptn.store("[<Xn|SP>]", ["[x0]", "[x1]", "[x2]", "[x4]", "[x8]", "[x16]", "[x30]", "[sp]"])
    @operands_ptn.store("[<Xn|SP>{,#0}]", ["[x0]", "[x1]", "[x2]", "[x4]", "[x8]", "[x16]", "[x30]", "[sp]"])
    @operands_ptn.store("<Ws:even>,<W(s+1)>", ["w0,w1/*asm*/", "w2,w3/*asm*/", "w4,w5/*asm*/", "w8,w9/*asm*/", "w16,w17/*asm*/", "w30,wzr/*asm*/"])
    @operands_ptn.store("<Wt:even>,<W(t+1)>", ["w0,w1/*asm*/", "w2,w3/*asm*/", "w4,w5/*asm*/", "w8,w9/*asm*/", "w16,w17/*asm*/", "w30,wzr/*asm*/"])
    @operands_ptn.store("<Xs:even>,<X(s+1)>", ["x0,x1/*asm*/", "x2,x3/*asm*/", "x4,x5/*asm*/", "x8,x9/*asm*/", "x16,x17/*asm*/", "x30,xzr/*asm*/"])
    @operands_ptn.store("<Xt:even>,<X(t+1)>", ["x0,x1/*asm*/", "x2,x3/*asm*/", "x4,x5/*asm*/", "x8,x9/*asm*/", "x16,x17/*asm*/", "x30,xzr/*asm*/"])
    @operands_ptn.store("<Xt:St64b>", ["x0", "x2", "x6"]) # if Rt<4:3> == '11' || Rt<0> == '1' then UNDEFINED;
    @operands_ptn.store("#<imm16>", ["1", "(1<<4)", "(1<<8)", "(1<<12)", "(0xffff)"])
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
        #puts inst
        f.puts inst
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

