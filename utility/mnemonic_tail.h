void CodeGenerator::beq(const Label &label) { b(EQ, label); }
void CodeGenerator::beq(int64_t label) { b(EQ, label); }
void CodeGenerator::bne(const Label &label) { b(NE, label); }
void CodeGenerator::bne(int64_t label) { b(NE, label); }
void CodeGenerator::bcs(const Label &label) { b(CS, label); }
void CodeGenerator::bcs(int64_t label) { b(CS, label); }
void CodeGenerator::bcc(const Label &label) { b(CC, label); }
void CodeGenerator::bcc(int64_t label) { b(CC, label); }
void CodeGenerator::bmi(const Label &label) { b(MI, label); }
void CodeGenerator::bmi(int64_t label) { b(MI, label); }
void CodeGenerator::bpl(const Label &label) { b(PL, label); }
void CodeGenerator::bpl(int64_t label) { b(PL, label); }
void CodeGenerator::bvs(const Label &label) { b(VS, label); }
void CodeGenerator::bvs(int64_t label) { b(VS, label); }
void CodeGenerator::bvc(const Label &label) { b(VC, label); }
void CodeGenerator::bvc(int64_t label) { b(VC, label); }
void CodeGenerator::bhi(const Label &label) { b(HI, label); }
void CodeGenerator::bhi(int64_t label) { b(HI, label); }
void CodeGenerator::bls(const Label &label) { b(LS, label); }
void CodeGenerator::bls(int64_t label) { b(LS, label); }
void CodeGenerator::bge(const Label &label) { b(GE, label); }
void CodeGenerator::bge(int64_t label) { b(GE, label); }
void CodeGenerator::blt(const Label &label) { b(LT, label); }
void CodeGenerator::blt(int64_t label) { b(LT, label); }
void CodeGenerator::bgt(const Label &label) { b(GT, label); }
void CodeGenerator::bgt(int64_t label) { b(GT, label); }
void CodeGenerator::ble(const Label &label) { b(LE, label); }
void CodeGenerator::ble(int64_t label) { b(LE, label); }
void CodeGenerator::b_none(const Label &label) { beq(label); }
void CodeGenerator::b_none(int64_t label) { beq(label); }
void CodeGenerator::b_any(const Label &label) { bne(label); }
void CodeGenerator::b_any(int64_t label) { bne(label); }
void CodeGenerator::b_nlast(const Label &label) { bcs(label); }
void CodeGenerator::b_nlast(int64_t label) { bcs(label); }
void CodeGenerator::b_last(const Label &label) { bcc(label); }
void CodeGenerator::b_last(int64_t label) { bcc(label); }
void CodeGenerator::b_first(const Label &label) { bmi(label); }
void CodeGenerator::b_first(int64_t label) { bmi(label); }
void CodeGenerator::b_nfrst(const Label &label) { bpl(label); }
void CodeGenerator::b_nfrst(int64_t label) { bpl(label); }
void CodeGenerator::b_pmore(const Label &label) { bhi(label); }
void CodeGenerator::b_pmore(int64_t label) { bhi(label); }
void CodeGenerator::b_plast(const Label &label) { bls(label); }
void CodeGenerator::b_plast(int64_t label) { bls(label); }
void CodeGenerator::b_tcont(const Label &label) { bge(label); }
void CodeGenerator::b_tcont(int64_t label) { bge(label); }
void CodeGenerator::b_tstop(const Label &label) { blt(label); }
void CodeGenerator::b_tstop(int64_t label) { blt(label); }
