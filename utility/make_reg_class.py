B = 'B'
H = 'H'
S = 'S'
D = 'D'
Q = 'Q'

def DEF_PREG(size, bits):
  return """class PReg%(size)s : public _PReg {
 public:
  explicit PReg%(size)s(uint32_t index) : _PReg(index, false, %(bits)s) {}
};""" % {'size':size, 'bits':bits}

for (size, bits) in [(B, 8), (H, 16), (S, 32), (D, 64)]:
  print(DEF_PREG(size, bits))
print()


def DEF_ZREG_ELEM(size, bits):
  return """class ZReg%(size)sElem : public ZRegElem {
 public:
  explicit ZReg%(size)sElem(uint32_t index, uint32_t eidx)
   : ZRegElem(index, eidx, %(bits)s) {}
};""" % {'size':size, 'bits':bits}

for (size, bits) in [(B, 8), (H, 16), (S, 32), (D, 64), (Q, 128)]:
  print(DEF_ZREG_ELEM(size, bits))
print()

def DEF_ZREG_LIST(size, bits):
  return """class ZReg%(size)sList : public ZRegList {
 public:
  ZReg%(size)sList(const ZReg%(size)s &s);
  explicit ZReg%(size)sList(const _ZReg &s, const _ZReg &e) \
   : ZRegList(s, e) {}
  ZReg%(size)sElem operator[](uint32_t i) const {
   return ZReg%(size)sElem(getIdx(), i);
  }
};""" % {'size':size, 'bits':bits}

for (size, bits) in [(B, 8), (H, 16), (S, 32), (D, 64), (Q, 128)]:
  print(DEF_ZREG_LIST(size, bits))
print()

def DEF_ZREG(size, bits):
  return """class ZReg%(size)s : public _ZReg {
 public:
  explicit ZReg%(size)s(uint32_t index) : _ZReg(index, %(bits)s) {}
  ZReg%(size)sElem operator[](uint32_t i) const {
   return ZReg%(size)sElem(getIdx(), i);
  }
  ZReg%(size)sList operator-(const ZReg%(size)s &other) const {
   return ZReg%(size)sList(*this, other);
  }
};""" % {'size':size, 'bits':bits}

for (size, bits) in [(B, 8), (H, 16), (S, 32), (D, 64), (Q, 128)]:
  print(DEF_ZREG(size, bits))
print()

def DEF_VREG_SC(size, bits):
  return """class %(size)sReg : public VRegSc {
 public:
  explicit %(size)sReg(uint32_t index) : VRegSc(index, %(bits)s) {}
};""" % {'size':size, 'bits':bits}

for (size, bits) in [(B, 8), (H, 16), (S, 32), (D, 64), (Q, 128)]:
  print(DEF_VREG_SC(size, bits))
print()


def DEF_VREG(size, bits, lane):
  return """class VReg%(lane)s%(size)s : public VRegVec {
 public:
  explicit VReg%(lane)s%(size)s(uint32_t index) : VRegVec(index, %(bits)s, %(lane)s) {}
  VReg%(size)sElem operator[](uint32_t i) const {
    assert(getLane() > i);
    return VReg%(size)sElem(getIdx(), i, getLane());
  }
  VReg%(lane)s%(size)sList operator-(const VReg%(lane)s%(size)s &other) const {
    return VReg%(lane)s%(size)sList(*this, other);
  }
};""" % {'size':size, 'bits':bits, 'lane':lane}

for (size, bits, lane) in [
  (B, 8, 4), (B, 8, 8), (B, 8, 16), (H, 16, 2),
  (H, 16, 4), (H, 16, 8), (S, 32, 2), (S, 32, 4),
  (D, 64, 1), (D, 64, 2), (Q, 128, 1)
]:
  print(DEF_VREG(size, bits, lane))
print()

def DEF_VREG_ELEM(size, bits):
  return """class VReg%(size)sElem : public VRegElem {
 public:
   explicit VReg%(size)sElem(uint32_t index, uint32_t eidx, uint32_t lane)
     : VRegElem(index, eidx, %(bits)s, lane) {}
};""" % {'size':size, 'bits':bits}

for (size, bits) in [(B, 8), (H, 16), (S, 32), (D, 64), (Q, 128)]:
  print(DEF_VREG_ELEM(size, bits))
print()

# bits is not used?
def DEF_VREG_LIST(size, bits, lane):
  return """class VReg%(lane)s%(size)sList : public VRegList {
 public:
  VReg%(lane)s%(size)sList(const VReg%(lane)s%(size)s &s);
  VReg%(lane)s%(size)sList(const VRegVec &s, const VRegVec &e)
    : VRegList(s, e) {}
  VReg%(size)sElem operator[](uint32_t i) const {
    assert(getLane() > i);
    return VReg%(size)sElem(getIdx(), i, getLane());
  }
};""" % {'size':size, 'bits':bits, 'lane':lane}

def DEF_VREG_LIST_CSTR(size, bits, lane):
  return "inline VReg%(lane)s%(size)sList::VReg%(lane)s%(size)sList(const VReg%(lane)s%(size)s &s) : VRegList(s, s) {}" % {'size':size, 'bits':bits, 'lane':lane}

tblList = [
  (B, 8, 4), (B, 8, 8), (B, 8, 16), (H, 16, 2),
  (H, 16, 4), (H, 16, 8), (S, 32, 2), (S, 32, 4),
  (D, 64, 1), (D, 64, 2), (Q, 128, 1)
]
for (size, bits, lane) in tblList:
  print(DEF_VREG_LIST(size, bits, lane))
print()
for (size, bits, lane) in tblList:
  print(DEF_VREG_LIST_CSTR(size, bits, lane))
print()
