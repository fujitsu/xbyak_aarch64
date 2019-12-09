/*******************************************************************************
 * Copyright 2019 FUJITSU LIMITED
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/
#pragma once

#ifndef _XBYAK_AARCH64_REG_
#define _XBYAK_AARCH64_REG_

#ifdef _WIN32
//#include <assert.h>
#else
#include <cassert>
#endif

#define VL 4

class Operand {
 public:
  enum Kind { NONE, RREG, VREG_SC, VREG_VEC, ZREG, PREG_Z, PREG_M, OPMASK };

  enum Code {
#ifdef XBYAK64
    X0 = 0,
    X1,
    X2,
    X3,
    X4,
    X5,
    X6,
    X7,
    X8,
    X9,
    X10,
    X11,
    X12,
    X13,
    X14,
    X15,
    X16 = 16,
    X17,
    X18,
    X19,
    X20,
    X21,
    X22,
    X23,
    X24,
    X25,
    X26,
    X27,
    X28,
    X29,
    X30,
    SP = 31,
    XZR = 31,
#endif
    W0 = 0,
    W1,
    W2,
    W3,
    W4,
    W5,
    W6,
    W7,
    W8,
    W9,
    W10,
    W11,
    W12,
    W13,
    W14,
    W15,
    W16 = 16,
    W17,
    W18,
    W19,
    W20,
    W21,
    W22,
    W23,
    W24,
    W25,
    W26,
    W27,
    W28,
    W29,
    W30,
    WSP = 31,
    WZR = 31,
  };

 private:
  Kind kind_;
  uint32_t bit_;

 public:
  explicit Operand(Kind kind, uint32_t bit) : kind_(kind), bit_(bit) {}
  uint32_t getBit() const { return bit_; }
  bool isRReg() const { return is(RREG); }
  bool isVRegSc() const { return is(VREG_SC); }
  bool isVRegVec() const { return is(VREG_VEC); }
  bool isZReg() const { return is(ZREG); }
  bool isPRegZ() const { return is(PREG_Z); }
  bool isPRegM() const { return is(PREG_M); }

 private:
  bool is(Kind kind) const { return (kind_ == kind); }
};

class Reg : public Operand {
  uint32_t index_;

 public:
  explicit Reg(uint32_t index, Kind kind, uint32_t bit)
      : Operand(kind, bit), index_(index) {}
  uint32_t getIdx() const { return index_; }
};

// General Purpose Register
class RReg : public Reg {
 public:
  explicit RReg(uint32_t index, uint32_t bit) : Reg(index, RREG, bit) {}
};

#define DEF_RREG(size, bits)                                  \
  class size##Reg : public RReg {                             \
   public:                                                    \
    explicit size##Reg(uint32_t index) : RReg(index, bits) {} \
  };

// DEF_RREG(W, 32) // class WReg
DEF_RREG(X, 64)  // class XReg

class WReg : public RReg {
 public:
  explicit WReg(uint32_t index) : RReg(index, 32) {}
};

typedef XReg Reg64;

#undef DEF_RREG

// SIMD & FP scalar regisetr
class VRegSc : public Reg {
 public:
  explicit VRegSc(uint32_t index, uint32_t bit) : Reg(index, VREG_SC, bit) {}
};

#define DEF_VREG_SC(size, bits)                                 \
  class size##Reg : public VRegSc {                             \
   public:                                                      \
    explicit size##Reg(uint32_t index) : VRegSc(index, bits) {} \
  };

DEF_VREG_SC(B, 8)    // class BReg
DEF_VREG_SC(H, 16)   // class HReg
DEF_VREG_SC(S, 32)   // class SReg
DEF_VREG_SC(D, 64)   // class DReg
DEF_VREG_SC(Q, 128)  // class QReg

#undef DEF_VREG_SC

// base for SIMD vector regisetr
class VRegVec : public Reg {
  uint32_t lane_;

 public:
  explicit VRegVec(uint32_t index, uint32_t bits, uint32_t lane)
      : Reg(index, VREG_VEC, bits), lane_(lane){};
  uint32_t getLane() const { return lane_; }
};

// SIMD vector regisetr element
class VRegElem : public VRegVec {
  uint32_t elem_idx_;

 public:
  explicit VRegElem(uint32_t index, uint32_t eidx, uint32_t bit, uint32_t lane)
      : VRegVec(index, bit, lane), elem_idx_(eidx) {}
  uint32_t getElemIdx() const { return elem_idx_; }
};

// base for SIMD Vector Register List
class VRegList : public VRegVec {
  uint32_t len_;

 public:
  explicit VRegList(const VRegVec &s)
      : VRegVec(s.getIdx(), s.getBit(), s.getLane()),
        len_(s.getIdx() - s.getIdx() + 1) {}
  explicit VRegList(const VRegVec &s, const VRegVec &e)
      : VRegVec(s.getIdx(), s.getBit(), s.getLane()),
        len_(((e.getIdx() + 32 - s.getIdx()) % 32) + 1) {}
  uint32_t getLen() const { return len_; }
};

#define DEF_VREG(size, bits, lane)                                            \
  class VReg##lane##size : public VRegVec {                                   \
   public:                                                                    \
    explicit VReg##lane##size(uint32_t index) : VRegVec(index, bits, lane) {} \
    VReg##size##Elem operator[](uint32_t i) const {                           \
      assert(getLane() > i);                                                  \
      return VReg##size##Elem(getIdx(), i, getLane());                        \
    }                                                                         \
    VReg##lane##size##List operator-(const VReg##lane##size &other) const {   \
      return VReg##lane##size##List(*this, other);                            \
    }                                                                         \
  };

#define DEF_VREG_ELEM(size, bits)                                           \
  class VReg##size##Elem : public VRegElem {                                \
   public:                                                                  \
    explicit VReg##size##Elem(uint32_t index, uint32_t eidx, uint32_t lane) \
        : VRegElem(index, eidx, bits, lane) {}                              \
  };

class VReg4B;
class VReg8B;
class VReg16B;
class VReg2H;
class VReg4H;
class VReg8H;
class VReg2S;
class VReg4S;
class VReg1D;
class VReg2D;
class VReg1Q;

#define DEF_VREG_LIST(size, bits, lane)                                 \
  class VReg##lane##size##List : public VRegList {                      \
   public:                                                              \
    VReg##lane##size##List(const VReg##lane##size &s);                  \
    explicit VReg##lane##size##List(const VRegVec &s, const VRegVec &e) \
        : VRegList(s, e) {}                                             \
    VReg##size##Elem operator[](uint32_t i) const {                     \
      assert(getLane() > i);                                            \
      return VReg##size##Elem(getIdx(), i, getLane());                  \
    }                                                                   \
  };

DEF_VREG_ELEM(B, 8)    // class VRegBElem
DEF_VREG_ELEM(H, 16)   // class VRegHElem
DEF_VREG_ELEM(S, 32)   // class VRegSElem
DEF_VREG_ELEM(D, 64)   // class VRegDElem
DEF_VREG_ELEM(Q, 128)  // class VRegQElem

#undef DEF_VREG_ELEM

DEF_VREG_LIST(B, 8, 4);    // class VReg4BList
DEF_VREG_LIST(B, 8, 8);    // class VReg8BList
DEF_VREG_LIST(B, 8, 16);   // class VReg16BList
DEF_VREG_LIST(H, 16, 2);   // class VReg2HList
DEF_VREG_LIST(H, 16, 4);   // class VReg4HList
DEF_VREG_LIST(H, 16, 8);   // class VReg8HList
DEF_VREG_LIST(S, 32, 2);   // class VReg2SList
DEF_VREG_LIST(S, 32, 4);   // class VReg4SList
DEF_VREG_LIST(D, 64, 1);   // class VReg1DList
DEF_VREG_LIST(D, 64, 2);   // class VReg2DList
DEF_VREG_LIST(Q, 128, 1);  // class VReg1QList

#undef DEF_VREG_LIST

DEF_VREG(B, 8, 4)    // class VReg4B
DEF_VREG(B, 8, 8)    // class VReg8B
DEF_VREG(B, 8, 16)   // class VReg16B
DEF_VREG(H, 16, 2)   // class VReg2H
DEF_VREG(H, 16, 4)   // class VReg4H
DEF_VREG(H, 16, 8)   // class VReg8H
DEF_VREG(S, 32, 2)   // class VReg2S
DEF_VREG(S, 32, 4)   // class VReg4S
DEF_VREG(D, 64, 1)   // class VReg1D
DEF_VREG(D, 64, 2)   // class VReg2D
DEF_VREG(Q, 128, 1)  // class VReg1Q

#undef DEF_VREG

#ifdef XBYAK_AARCH64_OBJ
VReg8BList::VReg8BList(const VReg8B &s) : VRegList(s, s){};
VReg16BList::VReg16BList(const VReg16B &s) : VRegList(s, s){};
VReg2HList::VReg2HList(const VReg2H &s) : VRegList(s, s){};
VReg4HList::VReg4HList(const VReg4H &s) : VRegList(s, s){};
VReg8HList::VReg8HList(const VReg8H &s) : VRegList(s, s){};
VReg2SList::VReg2SList(const VReg2S &s) : VRegList(s, s){};
VReg4SList::VReg4SList(const VReg4S &s) : VRegList(s, s){};
VReg1DList::VReg1DList(const VReg1D &s) : VRegList(s, s){};
VReg2DList::VReg2DList(const VReg2D &s) : VRegList(s, s){};
VReg1QList::VReg1QList(const VReg1Q &s) : VRegList(s, s){};
#endif  // #ifdef XBYAK_AARCH64_OBJ

// SIMD vector regisetr
class VReg : public VRegVec {
 public:
  explicit VReg(uint32_t index)
      : VRegVec(index, 128, 1),
        b4(index),
        b8(index),
        b16(index),
        b(index),
        h2(index),
        h4(index),
        h8(index),
        h(index),
        s2(index),
        s4(index),
        s(index),
        d1(index),
        d2(index),
        d(index),
        q1(index),
        q(index) {}

  VReg4B b4;
  VReg8B b8;
  VReg16B b16;
  VReg16B b;
  VReg2H h2;
  VReg4H h4;
  VReg8H h8;
  VReg8H h;
  VReg2S s2;
  VReg4S s4;
  VReg4S s;
  VReg1D d1;
  VReg2D d2;
  VReg2D d;
  VReg1Q q1;
  VReg1Q q;
};

// SVE SIMD Vector Register Base
class _ZReg : public Reg {
 public:
  explicit _ZReg(uint32_t index, uint32_t bits = 128 * VL)
      : Reg(index, ZREG, bits) {}
};

// SVE SIMD Vector Register Element
class ZRegElem : public _ZReg {
  uint32_t elem_idx_;

 public:
  explicit ZRegElem(uint32_t index, uint32_t eidx, uint32_t bit)
      : _ZReg(index, bit), elem_idx_(eidx) {}
  uint32_t getElemIdx() const { return elem_idx_; }
};

// base for SVE SIMD Vector Register List
class ZRegList : public _ZReg {
  uint32_t len_;

 public:
  explicit ZRegList(const _ZReg &s)
      : _ZReg(s.getIdx(), s.getBit()), len_(s.getIdx() - s.getIdx() + 1) {}
  explicit ZRegList(const _ZReg &s, const _ZReg &e)
      : _ZReg(s.getIdx(), s.getBit()), len_(e.getIdx() - s.getIdx() + 1) {}
  uint32_t getLen() const { return len_; }
};

#define DEF_ZREG(size, bits)                                    \
  class ZReg##size : public _ZReg {                             \
   public:                                                      \
    explicit ZReg##size(uint32_t index) : _ZReg(index, bits) {} \
    ZReg##size##Elem operator[](uint32_t i) const {             \
      return ZReg##size##Elem(getIdx(), i);                     \
    }                                                           \
    ZReg##size##List operator-(const ZReg##size &other) const { \
      return ZReg##size##List(*this, other);                    \
    }                                                           \
  };

#define DEF_ZREG_ELEM(size, bits)                            \
  class ZReg##size##Elem : public ZRegElem {                 \
   public:                                                   \
    explicit ZReg##size##Elem(uint32_t index, uint32_t eidx) \
        : ZRegElem(index, eidx, bits) {}                     \
  };

class ZRegB;
class ZRegH;
class ZRegS;
class ZRegD;
class ZRegQ;

#define DEF_ZREG_LIST(size, bits)                             \
  class ZReg##size##List : public ZRegList {                  \
   public:                                                    \
    ZReg##size##List(const ZReg##size &s);                    \
    explicit ZReg##size##List(const _ZReg &s, const _ZReg &e) \
        : ZRegList(s, e) {}                                   \
    ZReg##size##Elem operator[](uint32_t i) const {           \
      return ZReg##size##Elem(getIdx(), i);                   \
    }                                                         \
  };

DEF_ZREG_ELEM(B, 8)    // class ZRegBElem
DEF_ZREG_ELEM(H, 16)   // class ZRegHElem
DEF_ZREG_ELEM(S, 32)   // class ZRegSElem
DEF_ZREG_ELEM(D, 64)   // class ZRegDElem
DEF_ZREG_ELEM(Q, 128)  // class ZRegQElem

#undef DEF_ZREG_ELEM

DEF_ZREG_LIST(B, 8);    // class ZRegBList
DEF_ZREG_LIST(H, 16);   // class ZRegHList
DEF_ZREG_LIST(S, 32);   // class ZRegSList
DEF_ZREG_LIST(D, 64);   // class ZRegDList
DEF_ZREG_LIST(Q, 128);  // class ZRegQList

#undef DEF_ZREG_LIST

DEF_ZREG(B, 8)    // class ZRegB
DEF_ZREG(H, 16)   // class ZRegH
DEF_ZREG(S, 32)   // class ZRegS
DEF_ZREG(D, 64)   // class ZRegD
DEF_ZREG(Q, 128)  // class ZRegQ

#undef DEF_ZREG

#if 0
ZRegBList::ZRegBList(const ZRegB &s) :ZRegList(s,s) {};
ZRegHList::ZRegHList(const ZRegH &s) :ZRegList(s,s) {};
ZRegSList::ZRegSList(const ZRegS &s) :ZRegList(s,s) {};
ZRegDList::ZRegDList(const ZRegD &s) :ZRegList(s,s) {};
ZRegQList::ZRegQList(const ZRegQ &s) :ZRegList(s,s) {};
#endif

// SIMD Vector Regisetr for SVE
class ZReg : public _ZReg {
 public:
  explicit ZReg(uint32_t index)
      : _ZReg(index), b(index), h(index), s(index), d(index), q(index) {}

  ZRegB b;
  ZRegH h;
  ZRegS s;
  ZRegD d;
  ZRegQ q;
};

class _PReg : public Reg {
 public:
  explicit _PReg(uint32_t index, bool M = false, uint32_t bits = 16 * VL)
      : Reg(index, ((M == 0) ? PREG_Z : PREG_M), bits) {}
  bool isM() const { return isPRegM(); }
  bool isZ() const { return isPRegZ(); }
};

#define DEF_PREG(size, bits)                                           \
  class PReg##size : public _PReg {                                    \
   public:                                                             \
    explicit PReg##size(uint32_t index) : _PReg(index, false, bits) {} \
  };

DEF_PREG(B, 8)   // class PRegB
DEF_PREG(H, 16)  // class PRegH
DEF_PREG(S, 32)  // class PRegS
DEF_PREG(D, 64)  // class PRegD

#undef DEF_PREG

enum PredType {
  T_z,  // Zeroing predication
  T_m   // Merging predication
};

class PReg : public _PReg {
 public:
  explicit PReg(uint32_t index, bool M = false)
      : _PReg(index, M), b(index), h(index), s(index), d(index) {}
  _PReg operator/(PredType t) const {
    return (t == T_z) ? _PReg(getIdx(), false) : _PReg(getIdx(), true);
  }

  PRegB b;
  PRegH h;
  PRegS s;
  PRegD d;
};

struct Opmask : public Reg {
  explicit Opmask(int idx = 0) : Reg(idx, Operand::OPMASK, 64) {}
};

class Label;

struct RegRip {
  sint64 disp_;
  const Label *label_;
  bool isAddr_;
  explicit RegRip(sint64 disp = 0, const Label *label = 0, bool isAddr = false)
      : disp_(disp), label_(label), isAddr_(isAddr) {}
  friend const RegRip operator+(const RegRip &r, int disp) {
    return RegRip(r.disp_ + disp, r.label_, r.isAddr_);
  }
  friend const RegRip operator-(const RegRip &r, int disp) {
    return RegRip(r.disp_ - disp, r.label_, r.isAddr_);
  }
  friend const RegRip operator+(const RegRip &r, sint64 disp) {
    return RegRip(r.disp_ + disp, r.label_, r.isAddr_);
  }
  friend const RegRip operator-(const RegRip &r, sint64 disp) {
    return RegRip(r.disp_ - disp, r.label_, r.isAddr_);
  }
  friend const RegRip operator+(const RegRip &r, const Label &label) {
    if (r.label_ || r.isAddr_) throw Error(ERR_BAD_ADDRESSING);
    return RegRip(r.disp_, &label);
  }
  friend const RegRip operator+(const RegRip &r, const void *addr) {
    if (r.label_ || r.isAddr_) throw Error(ERR_BAD_ADDRESSING);
    return RegRip(r.disp_ + (sint64)addr, 0, true);
  }
};

#endif
