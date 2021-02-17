#include <stdio.h>
#include <xbyak_aarch64/xbyak_aarch64_util.h>

/*
FX700
cpu type=000000000000001e
advsimd fp sve atomic
*/
int main() {
  using namespace Xbyak_aarch64::util;
  Cpu cpu;
  printf("cpu type=%016lx\n", (long)cpu.getType());
  if (cpu.has(Cpu::tADVSIMD)) {
    printf("advsimd ");
  }
  if (cpu.has(Cpu::tFP)) {
    printf("fp ");
  }
  if (cpu.has(Cpu::tSVE)) {
    printf("sve(%d) ", (int)cpu.getSveLen());
  }
  if (cpu.has(Cpu::tATOMIC)) {
    printf("atomic ");
  }
  printf("\n");
}
