/*******************************************************************************
 * Copyright 2020-2022 FUJITSU LIMITED
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
#include "xbyak_aarch64_err.h"
#include "xbyak_aarch64_util.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_M_ARM64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <malloc.h>
#include <windows.h>

struct CpuInfo {
  int coreNum;
  enum Type { Unified = 0, Code = 1, Data = 2 };
  int cacheSize[3][3];
  CpuInfo() : coreNum(0), cacheSize{} {}
  int getCacheSize(Type type, uint32_t level) const {
    if (1 <= level && level <= 3)
      return cacheSize[type][level - 1];
    return 0;
  }
  int getCoreNum() const { return coreNum; }
  int getUnifiedCacheSize(int level) const { return getCacheSize(Unified, level); }
  int getCodeCacheSize(int level) const { return getCacheSize(Code, level); }
  int getDataCacheSize(int level) const { return getCacheSize(Data, level); }
  void put() const {
    printf("coreNum=%d\n", coreNum);
    for (int level = 1; level <= 3; level++) {
      printf("L%d unified size = %d\n", level, getUnifiedCacheSize(level));
      printf("L%d code size = %d\n", level, getCodeCacheSize(level));
      printf("L%d data size = %d\n", level, getDataCacheSize(level));
    }
  }
  void init() {
    DWORD bufSize = 0;
    GetLogicalProcessorInformation(NULL, &bufSize);
    auto *ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)_alloca(bufSize);
    if (GetLogicalProcessorInformation(ptr, &bufSize) == FALSE)
      return;

    DWORD offset = 0;
    while (offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= bufSize) {
      switch (ptr->Relationship) {
      case RelationProcessorCore:
        coreNum++;
        break;

      case RelationCache: {
        const auto cache = &ptr->Cache;
        if (1 <= cache->Level && cache->Level <= 3) {
          cacheSize[cache->Type][cache->Level - 1] += cache->Size;
        }
      } break;
      default:
        break;
      }
      offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
      ptr++;
    }
  }
};

#elif defined(__linux__)
#include <dirent.h>
#include <regex.h>
#include <sys/auxv.h>
#include <sys/prctl.h>
#include <unistd.h>

#define XBYAK_AARCH64_PATH_NODES "/sys/devices/system/node/node"
#define XBYAK_AARCH64_PATH_CORES "/sys/devices/system/node/node0/cpu"
#define XBYAK_AARCH64_READ_SYSREG(var, ID) asm("mrs %0, " #ID : "=r"(var));

/* In old Linux such as Ubuntu 16.04, HWCAP_ATOMICS, HWCAP_FP, HWCAP_ASIMD
   can not be found in <bits/hwcap.h> which is included from <sys/auxv.h>.
   Xbyak_aarch64 uses <asm/hwcap.h> as an alternative.
 */
#ifndef HWCAP_FP
#include <asm/hwcap.h>
#endif

#elif defined(__APPLE__)
#include <dirent.h>
#include <regex.h>
#include <sys/sysctl.h>
#include <unistd.h>

constexpr char hw_opt_atomics[] = "hw.optional.armv8_1_atomics";
constexpr char hw_opt_fp[] = "hw.optional.floatingpoint";
constexpr char hw_opt_neon[] = "hw.optional.neon";

#endif

namespace Xbyak_aarch64 {
namespace util {

const struct implementer_t implementers[] = {{0x00, "Reserved for software use"},
                                             {0xC0, "Ampere Computing"},
                                             {0x41, "Arm Limited"},
                                             {0x42, "Broadcom Corporation"},
                                             {0x43, "Cavium Inc."},
                                             {0x44, "Digital Equipment Corporation"},
                                             {0x46, "Fujitsu Ltd."},
                                             {0x49, "Infineon Technologies AG"},
                                             {0x4D, "Motorola or Freescale Semiconductor Inc."},
                                             {0x4E, "NVIDIA Corporation"},
                                             {0x50, "Applied Micro Circuits Corporation"},
                                             {0x51, "Qualcomm Inc."},
                                             {0x56, "Marvell International Ltd."},
                                             {0x69, "Intel Corporation"}};

#define XBYAK_AARCH64_MIDR_EL1(I, V, A, P, R) ((I << 24) | (V << 20) | (A << 16) | (P << 4) | (R << 0))
const struct cacheInfo_t cacheInfoDict[2] = {
    {/* A64FX */ XBYAK_AARCH64_MIDR_EL1(0x46, 0x1, 0xf, 0x1, 0x0), 2, 1, {1024 * 64, 1024 * 1024 * 8 * 4, 0, 0}},
    {/* A64FX */ XBYAK_AARCH64_MIDR_EL1(0x46, 0x2, 0xf, 0x1, 0x0), 2, 1, {1024 * 64, 1024 * 1024 * 8 * 4, 0, 0}},
};

#ifndef _M_ARM64
static uint32_t getCacheSize(uint32_t id, uint32_t defaultSize, uint32_t cores) {
  uint32_t v = sysconf(id);
  return (v ? v : defaultSize) / cores;
}
#endif

void Cpu::setCacheHierarchy() {
#ifndef _M_ARM64
  /* Cache size of AArch64 CPUs are described in the system registers,
     which can't be read from user-space applications.
     Linux provides `sysconf` API and `/sys/devices/system/cpu/`
     device files to get cache size, but they dosen't always return
     correct values. It may depend on Linux kernel version and
     support status of CPUs. To avoid this situation, cahche size is
     firstly read from `cacheInfoDict`, secondly get by `sysconf`.

     `sysconf` example
     #include <unistd.h>
     int main() {
       reutrn sysconf(_SC_LEVEL1_DCACHE_SIZE);
     }
   */
  const cacheInfo_t *c = nullptr;

  for (size_t j = 0; j < sizeof(cacheInfoDict) / sizeof(cacheInfo_t); j++) {
    if (cacheInfoDict[j].midr_el1 == midr_el1_) {
      c = cacheInfoDict + j;
      break;
    }
  }

  if (c != nullptr) {
    dataCacheLevel_ = c->dataCacheLevel;
    for (uint32_t i = 0; i < maxNumberCacheLevel; i++) {
      if (i < c->highestInnerCacheLevel)
        dataCacheSize_[i] = c->dataCacheSize[i];
      else
        dataCacheSize_[i] = c->dataCacheSize[i] / coresSharingDataCache_[i];
    }
  } else {
    /**
     * @ToDo Get chache information by `sysconf`
     * for the case thd dictionary is unavailable.
     */
    /* If `sysconf` returns zero as cache sizes, 32KiB, 1MiB, 0 and 0 is set as
       1st, 2nd, 3rd and 4th level cache sizes. 2nd cahce is assumed as sharing cache. */
#ifdef __linux__
    coresSharingDataCache_[0] = getCacheSize(_SC_LEVEL1_DCACHE_SIZE, 1024 * 32, 1);
    coresSharingDataCache_[1] = getCacheSize(_SC_LEVEL2_CACHE_SIZE, 1024 * 1024, 1);
    coresSharingDataCache_[2] = getCacheSize(_SC_LEVEL3_CACHE_SIZE, 0, 1);
    coresSharingDataCache_[3] = getCacheSize(_SC_LEVEL4_CACHE_SIZE, 0, 1);

    dataCacheSize_[0] = getCacheSize(_SC_LEVEL1_DCACHE_SIZE, 1024 * 32, 1);
    dataCacheSize_[1] = getCacheSize(_SC_LEVEL2_CACHE_SIZE, 1024 * 1024, 8);
    dataCacheSize_[2] = getCacheSize(_SC_LEVEL3_CACHE_SIZE, 0, 1);
    dataCacheSize_[3] = getCacheSize(_SC_LEVEL4_CACHE_SIZE, 0, 1);
#elif defined(__APPLE__)
    coresSharingDataCache_[0] = getCacheSize(HW_L1DCACHESIZE, 1024 * 32, 1);
    coresSharingDataCache_[1] = getCacheSize(HW_L2CACHESIZE, 1024 * 1024, 1);
    coresSharingDataCache_[2] = getCacheSize(HW_L3CACHESIZE, 0, 1);
    coresSharingDataCache_[3] = 0;

    dataCacheSize_[0] = getCacheSize(HW_L1DCACHESIZE, 1024 * 32, 1);
    dataCacheSize_[1] = getCacheSize(HW_L2CACHESIZE, 1024 * 1024, 8);
    dataCacheSize_[2] = getCacheSize(HW_L3CACHESIZE, 0, 1);
    dataCacheSize_[3] = 0;
#endif
  }
#endif
}

void Cpu::setNumCores() {
#ifdef _M_ARM64
  return;
#endif
#ifdef __linux__
  /**
   * @ToDo There are some methods to get # of cores.
   Considering various kernel versions and CPUs, a combination of
   multiple methods may be required.
   1) sysconf(_SC_NPROCESSORS_ONLN)
   2) /sys/devices/system/cpu/online
   3) std::thread::hardware_concurrency()
  */
  numCores_[0] = numCores_[1] = sysconf(_SC_NPROCESSORS_ONLN);
  coresSharingDataCache_[0] = 1;

  /* # of numa nodes: /sys/devices/system/node/node[0-9]+
     # of cores for each numa node: /sys/devices/system/node/node[0-9]+/cpu[0-9]+
     It is assumed L2 cache is shared by each numa node. */
  const int nodes = getFilePathMaxTailNumPlus1(XBYAK_AARCH64_PATH_NODES);
  int cores = 1;

  if (nodes > 0) {
    cores = getFilePathMaxTailNumPlus1(XBYAK_AARCH64_PATH_CORES);
    coresSharingDataCache_[1] = (cores > 0) ? cores : 1;
  } else {
    coresSharingDataCache_[1] = 1;
  }
#else
  numCores_[0] = numCores_[1] = 1;
  for (unsigned int i = 0; i < maxNumberCacheLevel; i++)
    coresSharingDataCache_[i] = 1;

  coresSharingDataCache_[1] = 8; // Set possible value.
#endif
}

void Cpu::setSysRegVal() {
#ifdef __linux__
  XBYAK_AARCH64_READ_SYSREG(midr_el1_, MIDR_EL1);
#endif
}

/**
 * Return directory path
 * @param[in] path ex. /sys/devices/system/node/node
 * @param[out] buf ex. /sys/devices/system/node
 */
int Cpu::getRegEx(char *buf, const char *path, const char *regex) {
#ifdef _M_ARM64
  (void)buf;
  (void)path;
  (void)regex;
  return -1;
#else
  regex_t regexBuf;
  regmatch_t match[1];

  if (regcomp(&regexBuf, regex, REG_EXTENDED) != 0)
    throw ERR_INTERNAL;

  const int retVal = regexec(&regexBuf, path, 1, match, 0);
  regfree(&regexBuf);

  if (retVal != 0)
    return -1;

  const int startIdx = match[0].rm_so;
  const int endIdx = match[0].rm_eo;

  /* Something wrong (multiple match or not match) */
  if (startIdx == -1 || endIdx == -1 || (endIdx - startIdx - 1) < 1)
    return -1;

  strncpy(buf, path + startIdx, endIdx - startIdx);
  buf[endIdx - startIdx] = '\0';

  return 0;
#endif
}

int Cpu::getFilePathMaxTailNumPlus1(const char *path) {
#ifdef __linux__
  const uint32_t max_path_len = 1024;
  char dir_path[max_path_len];
  char file_pattern[max_path_len];
  int retVal = 0;

  getRegEx(dir_path, path, "/([^/]+/)+");
  /* Remove last '/'. */
  dir_path[strlen(dir_path) - 1] = '\0';
  getRegEx(file_pattern, path, "[^/]+$");
  strncat(file_pattern, "[0-9]+", 16);

  fflush(stdout);

  DIR *dir = opendir(dir_path);
  struct dirent *dp;

  dp = readdir(dir);
  while (dp != NULL) {
    if (getRegEx(dir_path, dp->d_name, file_pattern) == 0)
      retVal++;
    dp = readdir(dir);
  }

  if (dir != NULL)
    closedir(dir);

  return retVal;
#else
  (void)path;
  return 0;
#endif
}

Cpu::Cpu() : type_(tNONE), sveLen_(SVE_NONE) {
#ifdef _M_ARM64
  CpuInfo info;
  info.init();
  numCores_[0] = numCores_[1] = info.getCoreNum();
  dataCacheLevel_ = 3;
  for (int i = 0; i < 3; i++) {
    coresSharingDataCache_[i] = info.getUnifiedCacheSize(i + 1) + info.getCodeCacheSize(i + 1) + info.getDataCacheSize(i + 1);
    dataCacheSize_[i] = info.getDataCacheSize(i + 1);
  }
  coresSharingDataCache_[3] = 0;
  dataCacheSize_[3] = 0;
  return;
#endif
#ifdef __linux__
  unsigned long hwcap = getauxval(AT_HWCAP);
  if (hwcap & HWCAP_ATOMICS) {
    type_ |= tATOMIC;
  }

  if (hwcap & HWCAP_FP) {
    type_ |= tFP;
  }
  if (hwcap & HWCAP_ASIMD) {
    type_ |= tADVSIMD;
  }
#ifdef HWCAP_SVE
  /* Some old <sys/auxv.h> may not define HWCAP_SVE.
     In that case, SVE is treated as if it were not supported. */
  if (hwcap & HWCAP_SVE) {
    type_ |= tSVE;
    // svcntb(); if arm_sve.h is available
    sveLen_ = (sveLen_t)prctl(51); // PR_SVE_GET_VL
  }
#endif
#elif defined(__APPLE__)
  size_t val = 0;
  size_t len = sizeof(val);

  if (sysctlbyname(hw_opt_atomics, &val, &len, NULL, 0) != 0)
    throw Error(ERR_INTERNAL);
  else
    type_ |= (val == 1) ? tATOMIC : 0;

  if (sysctlbyname(hw_opt_fp, &val, &len, NULL, 0) != 0)
    throw Error(ERR_INTERNAL);
  else
    type_ |= (val == 1) ? tFP : 0;

  if (sysctlbyname(hw_opt_neon, &val, &len, NULL, 0) != 0)
    throw Error(ERR_INTERNAL);
  else
    type_ |= (val == 1) ? tADVSIMD : 0;
#endif

  setSysRegVal();
  setNumCores();
  setCacheHierarchy();
}

Type Cpu::getType() const { return type_; }
bool Cpu::has(Type type) const { return (type & type_) != 0; }
uint64_t Cpu::getSveLen() const { return sveLen_; }
bool Cpu::isAtomicSupported() const { return type_ & tATOMIC; }
const char *Cpu::getImplementer() const {
  uint64_t implementer = (midr_el1_ >> 24) & 0xff;

  for (size_t i = 0; i < sizeof(implementers) / sizeof(implementer_t); i++) {
    if (implementers[i].id == implementer)
      return implementers[i].implementer;
  }

  return nullptr;
}

uint32_t Cpu::getCoresSharingDataCache(uint32_t i) const {
  if (i >= dataCacheLevel_)
    throw Error(ERR_BAD_PARAMETER);
  return coresSharingDataCache_[i];
}

uint32_t Cpu::getDataCacheLevels() const { return dataCacheLevel_; }

uint32_t Cpu::getDataCacheSize(uint32_t i) const {
  if (i >= dataCacheLevel_)
    throw Error(ERR_BAD_PARAMETER);
  return dataCacheSize_[i];
}
uint32_t Cpu::getNumCores(Arm64CpuTopologyLevel level) const {
  switch (level) {
  case CoreLevel:
    return numCores_[level - 1];
  default:
    throw Error(ERR_BAD_PARAMETER);
  }
}
} // namespace util
} // namespace Xbyak_aarch64
