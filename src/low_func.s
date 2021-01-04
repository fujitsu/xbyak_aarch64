
.align 16
.global xbyak_aarch64_get_fpcr
.global _xbyak_aarch64_get_fpcr
xbyak_aarch64_get_fpcr:
_xbyak_aarch64_get_fpcr:
  mrs x0, fpcr
  ret

.align 16
.global xbyak_aarch64_set_fpcr
.global _xbyak_aarch64_set_fpcr
xbyak_aarch64_set_fpcr:
_xbyak_aarch64_set_fpcr:
  msr fpcr, x0
  ret

.align 16
.global xbyak_aarch64_get_id_aa64isar0_el1
.global _xbyak_aarch64_get_id_aa64isar0_el1
xbyak_aarch64_get_id_aa64isar0_el1:
_xbyak_aarch64_get_id_aa64isar0_el1:
  mrs x0, id_aa64isar0_el1
  ret

.align 16
.global xbyak_aarch64_get_id_aa64pfr0_el1
.global _xbyak_aarch64_get_id_aa64pfr0_el1
xbyak_aarch64_get_id_aa64pfr0_el1:
_xbyak_aarch64_get_id_aa64pfr0_el1:
  mrs x0, id_aa64pfr0_el1
  ret

