#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t xbyak_aarch64_get_fpcr(void);
void xbyak_aarch64_set_fpcr(uint32_t x);

typedef struct {
  int resv0 : 4;
  int aes : 4;
  int sha1 : 4;
  int sha2 : 4;
  int crc32 : 4;
  int atomic : 4;
  int resv1 : 4;
  int rdm : 4;
  int resv2 : 12;
  int dp : 4;
  int resv3 : 16;
} Type_id_aa64isar0_el1;

Type_id_aa64isar0_el1 xbyak_aarch64_get_id_aa64isar0_el1(void);

typedef struct {
  int el0 : 4;
  int el1 : 4;
  int el2 : 4;
  int el3 : 4;
  int fp : 4;
  int advsimd : 4;
  int gic : 4;
  int ras : 4;
  int sve : 4;
  int resv0 : 28;
} Type_id_aa64pfr0_el1;

Type_id_aa64pfr0_el1 xbyak_aarch64_get_id_aa64pfr0_el1(void);

#ifdef __cplusplus
}
#endif
