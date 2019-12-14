def DEF_VREG_SC(size, bits):
  return """class %(size)sReg : public VRegSc {
 public:
  explicit %(size)sReg(uint32_t index) : VRegSc(index, %(bits)d) {}
};""" % {'size':size, 'bits':bits}

for (s, b) in [('B', 8), ('H', 16), ('S', 32), ('D', 64), ('Q', 128)]:
  print(DEF_VREG_SC(s, b))


