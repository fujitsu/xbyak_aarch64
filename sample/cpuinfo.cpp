/*******************************************************************************
 * Copyright 2019-2023 FUJITSU LIMITED
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
  printf("Implementer: %s\n", cpu.getImplementer());
  printf("CPU type: %016lx\n", (long)cpu.getType());
  printf("HW_CAP: ");
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

  const int cacheLevel = cpu.getLastDataCacheLevel();
  printf("Data cache level: %d\n", cacheLevel);

  Arm64CacheLevel levels[] = {L1, L2, L3, L4, L5, L6, L7};
  for (int i = 0; i < cacheLevel; i++) {
    const Arm64CacheType type = cpu.getCacheType(levels[i]);
    const int cacheSize = cpu.getDataCacheSize(levels[i]);

    switch (type) {
    case UnifiedCache:
      printf("L%d cache size: %d\n", i + 1, cacheSize);
      break;
    case DataCacheOnly:
    case SeparateCache:
      printf("L%dD cache size: %d\n", i + 1, cacheSize);
      break;
    default:
      fprintf(stderr, "Unknown cache type=%d\n", type);
    }
  }

  for (int i = 0; i < cacheLevel; i++) {
    const Arm64CacheType type = cpu.getCacheType(levels[i]);
    const int cores = cpu.getCoresSharingDataCache(levels[i]);

    switch (type) {
    case UnifiedCache:
      printf("L%d cache sharing cores: %d\n", i + 1, cores);
      break;
    case DataCacheOnly:
    case SeparateCache:
      printf("L%dD cache sharing cores: %d\n", i + 1, cores);
      break;
    default:
      fprintf(stderr, "Unknown cache type=%d\n", type);
    }
  }

  printf("\n");
  cpu.dumpCacheInfo();
}
