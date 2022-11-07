#pragma once
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
#ifndef XBYAK_AARCH64_UTIL_H_
#define XBYAK_AARCH64_UTIL_H_

#include <stdint.h>

namespace Xbyak_aarch64 {
namespace util {
typedef uint64_t Type;

constexpr uint32_t maxNumberCacheLevel = 4;
constexpr uint32_t maxTopologyLevel = 2;
constexpr uint32_t max_path_len = 1024;

enum Arm64CpuTopologyLevel { SmtLevel = 1, CoreLevel = 2 };

enum sveLen_t {
  SVE_NONE = 0,
  SVE_128 = 16 * 1,
  SVE_256 = 16 * 2,
  SVE_384 = 16 * 3,
  SVE_512 = 16 * 4,
  SVE_640 = 16 * 5,
  SVE_768 = 16 * 6,
  SVE_896 = 16 * 7,
  SVE_1024 = 16 * 8,
  SVE_1152 = 16 * 9,
  SVE_1280 = 16 * 10,
  SVE_1408 = 16 * 11,
  SVE_1536 = 16 * 12,
  SVE_1664 = 16 * 13,
  SVE_1792 = 16 * 14,
  SVE_1920 = 16 * 15,
  SVE_2048 = 16 * 16,
};

struct implementer_t {
  uint32_t id;
  const char *implementer;
};

struct cacheInfo_t {
  uint64_t midr_el1;
  uint32_t dataCacheLevel;
  uint32_t highestInnerCacheLevel;
  uint32_t dataCacheSize[maxNumberCacheLevel];
};

/**
   CPU detection class
*/
class Cpu {
  uint64_t type_;
  sveLen_t sveLen_;

private:
  uint32_t coresSharingDataCache_[maxNumberCacheLevel];
  uint32_t dataCacheSize_[maxNumberCacheLevel];
  uint32_t dataCacheLevel_;
  uint64_t midr_el1_;
  uint32_t numCores_[maxTopologyLevel];

  void setCacheHierarchy();
  void setNumCores();
  void setSysRegVal();
  int getRegEx(char *buf, const char *path, const char *regex);
  int getFilePathMaxTailNumPlus1(const char *path);

public:
  static const Type tNONE = 0;
  static const Type tADVSIMD = 1 << 1;
  static const Type tFP = 1 << 2;
  static const Type tSVE = 1 << 3;
  static const Type tATOMIC = 1 << 4;

  Cpu();

  Type getType() const;
  bool has(Type type) const;
  uint64_t getSveLen() const;
  bool isAtomicSupported() const;
  const char *getImplementer() const;
  uint32_t getCoresSharingDataCache(uint32_t i) const;
  uint32_t getDataCacheLevels() const;
  uint32_t getDataCacheSize(uint32_t i) const;
  uint32_t getNumCores(Arm64CpuTopologyLevel level) const;
};

} // namespace util
} // namespace Xbyak_aarch64
#endif
