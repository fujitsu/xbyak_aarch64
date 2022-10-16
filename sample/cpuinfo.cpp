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
  printf("# of CPU cores: %d\n", cpu.getNumCores(CoreLevel));
  printf("Data cache level: %d\n", cpu.getDataCacheLevels());
  printf("L1D cache size: %d\n", cpu.getDataCacheSize(0));
  printf("L2D cache size: %d\n", cpu.getDataCacheSize(1));
  printf("L1D cache sharing cores: %d\n", cpu.getCoresSharingDataCache(0));
  printf("L2 cache sharing cores: %d\n", cpu.getCoresSharingDataCache(1));
  printf("\n");
}
