def DEF_VREG_SC(size, bits):
  return """class %(size)sReg : public VRegSc {
 public:
  explicit %(size)sReg(uint32_t index) : VRegSc(index, %(bits)s) {}
};""" % {'size':size, 'bits':bits}

for (s, b) in [('B', 8), ('H', 16), ('S', 32), ('D', 64), ('Q', 128)]:
  print(DEF_VREG_SC(s, b))
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
  ('B', 8, 4), ('B', 8, 8), ('B', 8, 16), ('H', 16, 2),
  ('H', 16, 4), ('H', 16, 8), ('S', 32, 2), ('S', 32, 4),
  ('D', 64, 1), ('D', 64, 2), ('Q', 128, 1)
]:
  print(DEF_VREG(size, bits, lane))
print()

def DEF_VREG_ELEM(size, bits):
  return """class VReg%(size)sElem : public VRegElem {
 public:
   explicit VReg%(size)sElem(uint32_t index, uint32_t eidx, uint32_t lane)
     : VRegElem(index, eidx, %(bits)s, lane) {}
};""" % {'size':size, 'bits':bits}

for (size, bits) in [('B', 8), ('H', 16), ('S', 32), ('D', 64), ('Q', 128)]:
  print(DEF_VREG_ELEM(size, bits))
print()

