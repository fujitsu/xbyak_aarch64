[![Build Status](https://travis-ci.org/herumi/xbyak_aarch64.png)](https://travis-ci.org/herumi/xbyak_aarch64)

# Xbyak_aarch64 ; JIT assembler for AArch64 CPUs by C++

## Abstract

Xbyak_aarch64 is C++ header files which enables run-time assemble coding with the AArch64 instruction set of Arm(R)v8-A architecture.
Xbyak_aarch64 is based on Xbyak developed for x86_64 CPUs by MITSUNARI Shigeo.

## Feature

* GNU assembler like syntax.
* Fully support SVE instructions.

### Requirement

Xbyak_aarch64 uses no external library and it is written as standard C++ header files 
so that we believe Xbyak_aarch64 works various environment with various compilers.

### News
Break backward compatibility:
- To link `libxbyak_aarch64.a` is always necessary.
- namespace `Xbyak` is renamed to `Xbyak_aarch64`.
- Some class are renamed (e.g. CodeGeneratorAArch64 -> CodeGenerator).
- L_aarch64() is renamed to L().
- use dd(uint32_t) instead of dw(uint32_t).

### Supported Compilers

Almost C++11 or later compilers for AArch64 such as g++, clang++.

## Install

The command `make` builds `lib/libxbyak_aarch64.a`.

`make install` installs headers and a library into `/usr/local/` (default path).
Or add the location of the `xbyak_aarch64` directory to your compiler's include and link paths.

### Execution environment

You can execute programs using xbyak_aarch64 on systems running on Arm(R)v8-A architecure CPUs.
Even if you can't access such systems, you can try Xbyak_aarch64 on QEMU (generic and open source machine emulator and virtualizer).

We have checked programs built with xbyak_aarch64 can be executed with qemu version 3.1.0 on Linux running on x86_64 CPUs.
The following dpkgs are required.

* binutils-aarch64-linux-gnu
* cpp-8-aarch64-linux-gnu
* cpp-aarch64-linux-gnu
* g++-8-aarch64-linux-gnu
* g++-aarch64-linux-gnu
* gcc-8-aarch64-linux-gnu
* gcc-8-aarch64-linux-gnu-base:amd64
* gcc-aarch64-linux-gnu
* pkg-config-aarch64-linux-gnu
* qemu
* qemu-block-extra:amd64
* qemu-system-arm
* qemu-system-common
* qemu-system-data
* qemu-system-gui
* qemu-user
* qemu-user-static
* qemu-utils


Then execute the following commands.
```
cd xbyak_aarch64/sample
aarch64-linux-gnu-g++ add.cpp
env QEMU_LD_PREFIX=/usr/aarch64-linux-gnu qemu-aarch64 ./a.out
```

## M1 mac
For test, aarch64-gas is necessary.
Make the binary from [GNU-A Downloads](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads) or use [aarch64-unknown-linux-gnu](https://github.com/thinkski/osx-arm-linux-toolchains).

## How to make lib

```
make
```
makes `lib/libxbyak_aarch64.a`.

## How to use Xbyak_aarch64

Inherit `Xbyak::CodeGenerator` class and make the class method.
Make an instance of the class and get the function
pointer by calling `getCode()` and call it.
The following example 1) generates a JIT-ed function which simply adds two integer values passed as arguments and returns an integer value as a result,
and 2) calls the function. This example outputs "7" to STDOUT.

compile options:
- `-I <xbyak_aarch64 dir>/xbyak_aarch64`.
- `-L <xbyak_aarch64 dir>/lib -lxbyak_aarch64`.

```
#include "xbyak_aarch64.h"
using namespace Xbyak_aarch64;
class Generator : public CodeGenerator {
public:
  Generator() {
    Label L1, L2;
    L(L1);
    add(w0, w1, w0);
    cmp(w0, 13);
    b(EQ, L2);
    sub(w1, w1, 1);
    b(L1);
    L(L2);
    ret();
  }
};
int main() {
  Generator gen;
  gen.ready();
  auto f = gen.getCode<int (*)(int, int)>();
  std::cout << f(3, 4) << std::endl;
  return 0;
}
```

## Syntax
Synatx is similar to "AS" (GNU assembler).
Each AArch64 instruction is correspond to each function written in "xbyak_aarch64_mnemonic.h", we call it a **mnemonic function**.
Please refer files in sample/mnemonic_syntax directory for usage of mnemonic functions.
The below example shows correspondence between "AS" syntax and Xbyak_aarch64 mnemonic functions.

```
"AS"                  Xbyak_aarch64
add w0, w0, w1  --> add(w0, w0, w1);
add x0, x0, x1  --> add(x0, x0, x1);
add x0, x0, 5   --> add(x0, x0, 5);
mov w0, 1       --> mov(w0, 1);
ret             --> ret();
```

### Mnemonic functions
Each **mnemonic function** corresponds to one AArch64 instruction.
Function name represents corresponding mnemonic of instruction.
Because **"and", "or", "not"** are reserved keywords C++ and **"."** can't be used in C++ function name,
the following special cases are exist.

|Mnemonic of instruction|Name of **mnemonic funciton**|
|----|----|
|and|and_|
|or|or_|
|not|not_|
|b.cond|b|

### Operand
This section explains operands, which are given to mnemonic functions as their arguments.

#### General purpose registers

As general purpose registers,
the following table shows example of mnemonic functions' arguments ("Instance name" column).

|Instance name|C++ class name|Remarks|
|----|----|----|
|w0, w1, ..., w30|WReg|32-bit general purpose registers|
|x0, x1, ..., x30|WReg|64-bit general purpose registers|
|wzr|WReg|32-bit zero register|
|xzr|XReg|64-bit zero register|
|wsp|WReg|32-bit stack pointer|
|sp|zXReg|64-bit stack pointer|

You can also use your original instance as mnemonic functions argumetns.
Please refer constructor of "C++ class name" in Xbyak_aarch64 files.

```
WReg dstReg(0);
WReg srcReg0(1);
WReg srcReg1(2);

add(dstReg, srcReg0, srcReg1);  <--- (1)
add(w0, w1, w2);                <--- Output is same JIT code of (1)
```

##### SIMD/Floating point registers as scalar registers

As SIMD/Floating point registers with scalar use, 
the following table shows example of mnemonic functions' arguments ("Instance name" column).

|Instance name|C++ class name|Remarks|
|----|----|----|
|b0, b1, ..., b31|BReg|8-bit scalar registers|
|h0, h1, ..., h31|HReg|16-bit scalar registers|
|s0, s1, ..., s31|SReg|32-bit scalar registers|
|d0, d1, ..., d31|DReg|64-bit scalar registers|
|q0, q1, ..., q31|QReg|128-bit scalar registers|

You can also use your original instance as mnemonic functions argumetns.
Please refer constructor of "C++ class name" in Xbyak_aarch64 files.

```
BReg dstReg(0);

mov(b0, v0.b[5]);       <--- (1)
mov(dstReg, v0.b[5]);   <--- Output is same JIT code of (1)

```

#### SIMD/Floating point registers as vector registers

As SIMD/Floating point registers with vector use, 
the following table shows example of mnemonic functions' arguments ("Instance name" column).

|Instance name|C++ class name|Remarks|
|----|----|----|
|v0.b8, v1.b8, ..., v31.b8|VReg8B|8-bit x8 elements vector registers|
|v0.b16, v1.b16, ..., v31.b16|VReg16B|8-bit x16 elements vector registers|
|v0.h4, v1.h4, ..., v31.h4|VReg4H|16-bit x4 elements vector registers|
|v0.h8, v1.h8, ..., v31.h8|VReg8H|16-bit x8 elements vector registers|
|v0.s2, v1.s2, ..., v31.s2|VReg2S|32-bit x2 elements vector registers|
|v0.s4, v1.s4, ..., v31.s4|VReg4S|32-bit x4 elements vector registers|
|v0.d1, v1.d1, ..., v31.d1|VReg1D|64-bit x1 elements vector registers|
|v0.d2, v1.d2, ..., v31.d2|VReg2D|64-bit x2 elements vector registers|

You can also use your original instance as mnemonic functions argumetns.
Please refer constructor of "C++ class name" in Xbyak_aarch64 files.

```
VReg16B dstReg(0);
VReg16B srcReg(1);

mov(v0.b16, v1.b16);   <--- (1)
mov(dstReg, srcReg);   <--- Output is same JIT code of (1)
```

#### Element of SIMD/Floating point registers

As SIMD/Floating point registers with element-wise use, 
the following table shows example of mnemonic functions' arguments ("Instance name" column).

|Instance name|C++ class name|Remarks|
|----|----|----|
|v0.b[0], v0.b[1], ..., v0.b[15]|VRegBElem|8-bit element of vector register #0|
|vN.b[0], vN.b[1], ..., vN.b[15]|VRegBElem|8-bit element of vector register #N|
|v0.h[0], v0.h[1], ..., v0.h[15]|VRegHElem|16-bit element of vector register #0|
|vN.h[0], vN.h[1], ..., vN.h[15]|VRegHElem|16-bit element of vector register #N|
|v0.s[0], v0.s[1], ..., v0.s[15]|VRegSElem|32-bit element of vector register #0|
|vN.s[0], vN.s[1], ..., vN.s[15]|VRegSElem|32-bit element of vector register #N|
|v0.d[0], v0.d[1], ..., v0.d[15]|VRegDElem|64-bit element of vector register #0|
|vN.d[0], vN.d[1], ..., vN.d[15]|VRegDElem|64-bit element of vector register #N|

You can also use your original instance as mnemonic functions argumetns.
Please refer constructor of "C++ class name" in Xbyak_aarch64 files.

```
VRegBElem dstReg(0, 3, 16);
VRegBElem srcReg(1, 4, 16);
VRegB     hoge(0);

mov(v0.b[3], v1.b[4]); <--- (1)
mov(dstReg,  srcReg);  <--- Output is same JIT code of (1)
mov(hoge[3], srcReg);  <--- Output is same JIT code of (1)
```

### SIMD/Floating point register lists

As SIMD/Floating point register lists,
the following table shows example of mnemonic functions' arguments ("Instance name" column).

Register index can be cyclic.

|Instance name|C++ class name|Remarks|
|----|----|----|
|v0.b16 - v0.b16|VReg16BList|SIMD/Floating point register list containing 8-bit elements x 16 x 1|
|v0.b16 - v1.b16|VReg16BList|SIMD/Floating point register list containing 8-bit elements x 16 x 2|
|v0.b16 - v2.b16|VReg16BList|SIMD/Floating point register list containing 8-bit elements x 16 x 3|
|v0.b16 - v3.b16|VReg16BList|SIMD/Floating point register list containing 8-bit elements x 16 x 4|
|v31.b16 - v2.b16|VReg16BList|SIMD/Floating point register list containing 8-bit elements x 16 x 4|
|v0.h8 - v3.h8|VReg8HList|SIMD/Floating point register list containing 16-bit elements x 8 x 4|
|v0.s4 - v3.s4|VReg4SList|SIMD/Floating point register list containing 32-bit elements x 4 x 4|
|v0.d2 - v3.d2|VReg2DList|SIMD/Floating point register list containing 64-bit elements x 2 x 4|

You can also use your original instance as mnemonic functions argumetns.
Please refer constructor of "C++ class name" in Xbyak_aarch64 files.

```
VReg16BList dstList(v0.b16, v3.b16);
VReg16BList hoge(VReg16B(0), VReg16B(3));

ld4((v0 - v3), ptr(x0)); <--- (1)
ld4(dstLsit, ptr(x0));   <--- Output is same JIT code of (1)
ld4(hoge, ptr(x0));      <--- Output is same JIT code of (1)

```

### Element of vector register lists

As elements of SIMD/Floating point register lists,
the following table shows example of mnemonic functions' arguments ("Instance name" column).

|Instance name|C++ class name|Remarks|
|----|----|----|
|(v0.b16 - v0.b16)[0]|VRegBElem|First 8-bit element of one-vector-register list|
|(v0.b16 - v0.b16)[N]|VRegBElem|N-th 8-bit element of one-vector-register list|
|(v0.b16 - v3.b16)[N]|VRegBElem|N-th 8-bit element of four-vector-register list|
|(v0.h8 - v3.h8)[N]|VRegHElem|N-th 16-bit element of four-vector-register list|
|(v0.s4 - v3.s4)[N]|VRegSElem|N-th 32-bit element of four-vector-register list|
|(v0.d2 - v3.d2)[N]|VRegDElem|N-th 64-bit element of four-vector-register list|

You can also use your original instance as mnemonic functions argumetns.
Please refer constructor of "C++ class name" in Xbyak_aarch64 files.

```
VRegBElem dstReg(0, 3, 16);
VRegBElem hoge((VReg16B(0) - VReg16B(3))[3]);

ld4((v0 - v3)[3], ptr(x0)); <--- (1)
ld4(dstReg, ptr(x0));       <--- Output is same JIT code of (1)
ld4(hoge, ptr(x0));         <--- Output is same JIT code of (1)

```

### SVE registers as vector registers
AS SVE registers, 
the following table shows example of mnemonic functions' arguments ("Instance name" column).


|Instance name|C++ class name|Remarks|
|----|----|----|
|Z0.b, z1.b, ..., z31.b|ZRegB|8-bit x 64 elements SVE registers.|
|Z0.h, z1.h, ..., z31.h|ZRegB|16-bit x 32 elements SVE registers.|
|Z0.s, z1.s, ..., z31.s|ZRegB|32-bit x 16 elements SVE registers.|
|Z0.d, z1.d, ..., z31.d|ZRegB|64-bit x 8 elements SVE registers.|
|Z0.q, z1.q, ..., z31.q|ZRegB|128-bit x 4 elements SVE registers.|

You can also use your original instance as mnemonic functions argumetns.
Please refer constructor of "C++ class name" in Xbyak_aarch64 files.

```
ZRegB dstReg(0);

add(z0.b, z1.b, z2.b);      <--- (1)
add(ZRegB(0), z1.b, z2.b);  <--- Output is same JIT code of (1)
add(dstReg, z1.b, z2.b);    <--- Output is same JIT code of (1)

```

### Element of SVE registers

As SVE registers with element-wise use, 
the following table shows example of mnemonic functions' arguments ("Instance name" column).

|Instance name|C++ class name|Remarks|
|----|----|----|
|z0.b[0], z0.b[1], ..., z0.b[15]|ZRegBElem|8-bit element of SVE register #0|
|vN.b[0], vN.b[1], ..., vN.b[15]|ZRegBElem|8-bit element of SVE register #N|
|z0.h[0], z0.h[1], ..., z0.h[15]|ZRegHElem|16-bit element of SVE register #0|
|vN.h[0], vN.h[1], ..., vN.h[15]|ZRegHElem|16-bit element of SVE register #N|
|z0.s[0], z0.s[1], ..., z0.s[15]|ZRegSElem|32-bit element of SVE register #0|
|vN.s[0], vN.s[1], ..., vN.s[15]|ZRegSElem|32-bit element of SVE register #N|
|z0.d[0], z0.d[1], ..., z0.d[15]|ZRegDElem|64-bit element of SVE register #0|
|vN.d[0], vN.d[1], ..., vN.d[15]|ZRegDElem|64-bit element of SVE register #N|
|z0.q[0], z0.q[1], ..., z0.q[15]|ZRegDElem|128-bit element of SVE register #0|
|vN.q[0], vN.q[1], ..., vN.q[15]|ZRegDElem|128-bit element of SVE register #N|

You can also use your original instance as mnemonic functions argumetns.
Please refer constructor of "C++ class name" in Xbyak_aarch64 files.

```
ZRegBElem hoge(1, 7, 8);

dup(z0.b, z1.b[7]);  <--- (1)
add(z0.b, hoge);     <--- Output is same JIT code of (1)

```

### SVE register lists

As SVE register lists,
the following table shows example of mnemonic functions' arguments ("Instance name" column).

Register index can be cyclic.

|Instance name|C++ class name|Remarks|
|----|----|----|
|z0.b - z0.b|ZRegBList|SVE register list containing 8-bit elements x 64 x 1|
|z0.b - z1.b|ZRegBList|SVE register list containing 8-bit elements x 64 x 2|
|z0.b - z2.b|ZRegBList|SVE register list containing 8-bit elements x 64 x 3|
|z0.b - z3.b|ZRegBList|SVE register list containing 8-bit elements x 64 x 4|
|z31.b - z2.b|ZRegBList|SVE register list containing 8-bit elements x 64 x 4|
|z0.h - z3.h|ZRegHList|SVE register list containing 16-bit elements x 32 x 4|
|z0.s - z3.s|ZRegSList|SVE register list containing 32-bit elements x 16 x 4|
|z0.d - z3.d|ZRegDList|SVE register list containing 64-bit elements x 8 x 4|
|z0.q - z3.q|ZRegQList|SVE register list containing 128-bit elements x 4 x 4|

```
ZRegBList dstList(z0.b, z3.b);
ZRegBList hoge(ZRegB(0) - ZRegB(3));

ld4((z0.b - z3.b), p0, ptr(x0)); <--- (1)
ld4(dstList, p0, ptr(x0));       <--- Output is same JIT code of (1)
ld4(hoge, p0, ptr(x0));          <--- Output is same JIT code of (1)

```

### Element of SVE register lists

As elements of SVE register lists,
the following table shows example of mnemonic functions' arguments ("Instance name" column).

|Instance name|C++ class name|Remarks|
|----|----|----|
|(z0.b - z0.b)[0]|ZRegBElem|First 8-bit elements of one-SVE-register list|
|(z0.b - z0.b)[N]|ZRegBElem|N-th 8-bit elements of one-SVE-register list|
|(z0.b - z3.b)[N]|ZRegBElem|N-th 8-bit elements of four-SVE-register list|
|(z0.h - z3.h)[N]|ZRegHElem|N-th 16-bit elements of four-SVE-register list|
|(z0.s - z3.s)[N]|ZRegSElem|N-th 32-bit elements of four-SVE-register list|
|(z0.d - z3.d)[N]|ZRegDElem|N-th 64-bit elements of four-SVE-register list|

You can also use your original instance as mnemonic functions argumetns analogous with the element of vector register lists.


### Predicate registers

As predicate registers,
the following table shows example of mnemonic functions' arguments ("Instance name" column).

|Instance name|C++ class name|Remarks|
|----|----|----|
|p0, p1, ..., p7|PReg|Predicate registers|
|p0.b, p1.b, ..., p7.b|PRegB|Predicate registers for 8-bit elements|
|p0.h, p1.h, ..., p7.h|PRegH|Predicate registers for 16-bit elements|
|p0.s, p1.s, ..., p7.s|PRegS|Predicate registers for 32-bit elements|
|p0.d, p1.d, ..., p7.d|PRegD|Predicate registers for 64-bit elements|

Though Xbyak_aarch64 defines PRegB, PRegH, PRegS, PRegD classes,
you can use PReg class instances where mnemonic functions take predicate register operands.
The AArch64 instructin set has two predication types, "merging" and "zeroing" predicate.
You can use "T_m" or "T_z" to inform mnemonic functions which type you choses ((1), (2)).

You can ommit "T_m" and "T_z" for mnemonic functions whose correspond predicate instructions
can take either one. For example, "CLS" instruction in the AARch64 instruction set is only defined for merging predicate,
both (3) and (4) are OK.

Some instructions are defined for both predicated and unpredicated.
Output JIT-ed code of (5) and (6) are deferent.


```
mov(z0.b, p0/T_m, 1);              <--- (1), merging predicate
mov(z0.b, p0/T_z, 1);              <--- (2), zeroing predicate

cls(z0.b, p0/T_m, z0.b);           <--- (3)
cls(z0.b, p0, z0.b);               <--- (4), output is same JIT code of (3)

add(z0.b, p0/T_m, z1.b, z2.b);     <--- (5), predicated add
add(z0.b, p0, z1.b, z2.b);         <--- (6), output is same JIT code of (5)
add(z0.b, z1.b, z2.b);             <--- (7), unpredicated add, output is NOT same JIT code of (5)
```

You can also use your original instance as mnemonic functions argumetns.
Please refer constructor of "C++ class name" in Xbyak_aarch64 files.

```
PReg hoge(0);

add(z0.b, p0, z1.b, z2.b);         <--- (8)
add(z0.b, PReg(0), z1.b, z2.b);    <--- Output is same JIT code of (8)
add(z0.b, hoge, z1.b, z2.b);       <--- Output is same JIT code of (8)

```

### Immediate values

You can use immediate values for arguments of mnemonic functions in the form that C++ syntax allows,
such as, "10", "-128", "0xFF", "1<<32", "3.5", etc.

Please care for range of values.
For example, "ADD (immediate)" instruction can receive signed 12-bit value
so that you have to ensure that the value passed to mnemonic function is inside the range.
Mnemonic functions of Xbyak_aarch64 checks immediate values at runtime, 
and throws exception if it detects range over.

```
void genAddFunc() {
     int a = 1<<16;
     add(x0, x0, a);    <--- This mnemonic function throws exception at runtime.
     ret();
}
```

Some immediate values may not decided at coding time but runtime.
You should check the immediate values and handle them.

```
void gen_Summation_From_One_To_Parameter_Func(unsigned int N) {

    if(N < (1<<11)) {
        for(int i=0; i<N; i++) {
            add(x0, x0, N);
        }
        ret();
    } else {
        printf("Invalid parameter N=%d\n", N);
        printf("This function supports less than 2048.\n");
    }
}
```    

#### Symbols
Some instructions of the AArch64 instruction set are used with sybols such as **"UXTW", "LSL", "SXTW", "SXTX", "MUL VL"**, etc.
The menomic functions for those instructions can receive keywords of **"UXTW", "LSL", "SXTW", "SXTX", "MUL_VL"**, etc.
Please **"grep**" keywords, for example
```
grep LSL sample/mnemonic_syntax/*
```
, to find usage of those menomic functions.


#### Addressing
The AArch64 instruction set has various addressing modes such as "No offset", "Post-index", "Pre-index", "Signed offset", "Unsigned offset".
Please use "ptr()", "pre_ptr()", "post_ptr()" functions to specify which addressing mode you want to use.
Please "grep" the functions, for example 
```
grep -w ptr sample/mnemonic_syntax/*
```
, to find usage of these addressing functions.


|Use example|corresponding assembler syntax|Remarks|
|----|----|----|
|ldr(x0, ptr(x1, x7))|ldr x0, [x1, x7]|Register offset| 
|str(w0, ptr(x7, w8, UXTW, 2))|str w0, [x7, w8, UXTW 2]|Register offset with shift amount|
|ldr(w0, post_ptr(x5, -16))|ldr w0, [x5], -16|Post-index|
|ldrb(w16, pre_ptr(x5, 127))|ldrb w16, [x5, 127]!|Pre-index|


## Label

You can use "Label" to direct where branch instruction jump to.
The following example shows how to use "Label".

```
Label L1;           // (1), instance of Label class
 
mov(w4, w0); 
mov(w3, 0); 
mov(w0, 0); 
L(L1);              // (2), "L" function registers JIT code address of this position to label L1.
add(w0, w0, w4); 
add(w3, w3, 1); 
cmp(w2, w3); 
ccmp(w1, w3, 4, NE); 
bgt(L1);            // (3), set destination of branch instruction to the address stored in L1.
```

1. Prepare Label class instance.
1. Call the L function to register destination address to the instance.
1. Pass the instance to mnemonic functions correspond to branch instructions.

You can copy the address stored in "Label" instance by using `assignL` function.

```
Label L1,L2,L3; 
....
L(L1);               // JIT code address of this position is stored to L1.
....
L(L2);               // JIT code address of this position is stored to L1.
....
if (flag) { 
assignL(L3,L1);      // The address stored in L1 is copied to L3.
} else { 
assignL(L3,L2);      // The address stored in L1 is copied to L3.
} 
b(L3);               // If flag == true, branch destination is L1, otherwise L2.
```



## Code size
The default max size of JIT-ed code is 4096 bytes.
Specify the size in constructor of `CodeGenerator()` if necessary.

```
class Quantize : public Xbyak_aarch64::CodeGenerator {
public:
  Quantize()
    : CodeGenerator(8192)
  {
  }
  ...
};
```

## User allocated memory

You can make JIT-ed code on prepared memory.

Call `setProtectModeRE` yourself to change memory mode if using the prepared memory.

```
uint8_t alignas(4096) buf[8192]; // C++11 or later

struct Code : Xbyak_aarch64::CodeGenerator {
    Code() : Xbyak_aarch64::CodeGenerator(sizeof(buf), buf)
    {
        mov(rax, 123);
        ret();
    }
};

int main()
{
    Code c;
    c.setProtectModeRE(); // set memory to Read/Exec
    printf("%d\n", c.getCode<int(*)()>()());
}
```

**Note**: See [sample/test0.cpp](sample/test0.cpp).

### AutoGrow

If `AutoGrow` is specified in a constructor of `CodeGenerator`,
the memory region for JIT-ed code is automatically extended if needed.

Call `ready()` or `readyRE()` before calling `getCode()` to fix jump address.
```
struct Code : Xbyak_aarch64::CodeGenerator {
  Code()
    : Xbyak_aarch64::CodeGenerator(<default memory size>, Xbyak_aarch64::AutoGrow)
  {
     ...
  }
};
Code c;
// generate code for jit
c.ready(); // mode = Read/Write/Exec
```

**Note**:
* Don't use the address returned by `getCurr()` before calling `ready()` because it may be invalid address.

### Read/Exec mode
Xbyak_aarch64 set Read/Write/Exec mode to memory to run JIT-ed code.
If you want to use Read/Exec mode for security, then specify `DontSetProtectRWE` for `CodeGenerator` and
call `setProtectModeRE()` after generating JIT-ed code.

```
struct Code : Xbyak_aarch64::CodeGenerator {
    Code()
        : Xbyak_aarch64::CodeGenerator(4096, Xbyak_aarch64::DontSetProtectRWE)
    {
        mov(eax, 123);
        ret();
    }
};

Code c;
c.setProtectModeRE();
```


Call `readyRE()` instead of `ready()` when using `AutoGrow` mode.
See [protect-re.cpp](sample/protect-re.cpp).

## How to pass arguments to JIT generated function
To be written...

## Macro
To be written...


## Sample
To be written...

* [add.cpp](sample/add.cpp) ; tiny sample
* [label.cpp](sample/label.cpp) ; label sample

## License

Copyright FUJITSU LIMITED 2019-2020

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

## Reference
- Chapter C6:A64 Base Instruction Descriptions and C7:A64 Advanced SIMD and Floating-point Instruction Descriptions of "ARM(R) Architecture Reference Manual ARMv8, for ARMv8-A architecture profile", ARM DDI 0487D.b (ID042519)".
- "ARM(R) Architecture Reference Manual Supplement, The Scalable Vector Extension (SVE), for ARMv8-A", ARM DDI0584.
- "Procedure Call Standard for the ARM 64-bit Architecture (AArch64)",  ARM IHI 0055B".

## Notice

* Arm is a registered trademark of Arm Limited (or its subsidiaries) in the US and/or elsewhere.
* Intel is a registered trademark of Intel Corporation (or its subsidiaries) in the US and/or elsewhere.



## Acknowledgement

We are grateful to MITSUNARI-san (Cybozu Labs, Inc.) for release Xbyak as an open source software and his advice for development of Xbyak_aarch64.

## History

|Date|Version|Remarks|
|----|----|----|
|December 9, 2019|0.9.0|First public release version.|


## Copyright

Copyright FUJITSU LIMITED 2019-2020
