/*******************************************************************************
 * Copyright 2019 FUJITSU LIMITED
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
#ifndef XBYAK_XAARCH64_UTIL_H_
#define XBYAK_XAARCH64_UTIL_H_

/**
   utility class and functions for Xbyak
   Xbyak::util::Clock ; rdtsc timer
   Xbyak::util::Cpu ; detect CPU
   @note this header is UNDER CONSTRUCTION!
*/
#include "xbyak_aarch64.h"

#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || \
    defined(_M_X64)
#define XBYAK_INTEL_CPU_SPECIFIC
#endif

#if defined(__aarch64__)
#define XBYAK_AARCH64_CPU_SPECIFIC
#endif

#ifdef XBYAK_INTEL_CPU_SPECIFIC
#ifdef _MSC_VER
#if (_MSC_VER < 1400) && defined(XBYAK32)
static inline __declspec(naked) void __cpuid(int[4], int) {
  __asm {
		push	ebx
		push	esi
		mov		eax, dword ptr[esp + 4 * 2 + 8]  // eaxIn
		cpuid
		mov		esi, dword ptr[esp + 4 * 2 + 4]  // data
		mov		dword ptr[esi], eax
		mov		dword ptr[esi + 4], ebx
		mov		dword ptr[esi + 8], ecx
		mov		dword ptr[esi + 12], edx
		pop		esi
		pop		ebx
		ret
  }
}
#else
//#include <intrin.h> // for __cpuid
#endif
#else
#ifndef __GNUC_PREREQ
#define __GNUC_PREREQ(major, minor) \
  ((((__GNUC__) << 16) + (__GNUC_MINOR__)) >= (((major) << 16) + (minor)))
#endif
#if __GNUC_PREREQ(4, 3) && !defined(__APPLE__)
#include <cpuid.h>
#else
#if defined(__APPLE__) && defined(XBYAK32)  // avoid err : can't find a register
                                            // in class `BREG' while reloading
                                            // `asm'
#define __cpuid(eaxIn, a, b, c, d)                                         \
  __asm__ __volatile__("pushl %%ebx\ncpuid\nmovl %%ebp, %%esi\npopl %%ebx" \
                       : "=a"(a), "=S"(b), "=c"(c), "=d"(d)                \
                       : "0"(eaxIn))
#define __cpuid_count(eaxIn, ecxIn, a, b, c, d)                            \
  __asm__ __volatile__("pushl %%ebx\ncpuid\nmovl %%ebp, %%esi\npopl %%ebx" \
                       : "=a"(a), "=S"(b), "=c"(c), "=d"(d)                \
                       : "0"(eaxIn), "2"(ecxIn))
#else
#define __cpuid(eaxIn, a, b, c, d)                          \
  __asm__ __volatile__("cpuid\n"                            \
                       : "=a"(a), "=b"(b), "=c"(c), "=d"(d) \
                       : "0"(eaxIn))
#define __cpuid_count(eaxIn, ecxIn, a, b, c, d)             \
  __asm__ __volatile__("cpuid\n"                            \
                       : "=a"(a), "=b"(b), "=c"(c), "=d"(d) \
                       : "0"(eaxIn), "2"(ecxIn))
#endif
#endif
#endif
#endif

namespace Xbyak {
namespace util {

typedef enum { SmtLevel = 1, CoreLevel = 2 } IntelCpuTopologyLevel;

/**
   Dummy CPU detection class to return something,
   if Xbyak::util::Cpu::hoge function is called.
*/
class Cpu {
  uint64 type_;
  // system topology
  static const size_t maxTopologyLevels = 2;
  unsigned int numCores_[maxTopologyLevels];

  static const unsigned int maxNumberCacheLevels = 10;
  unsigned int dataCacheSize_[maxNumberCacheLevels];
  unsigned int coresSharingDataCache_[maxNumberCacheLevels];
  unsigned int dataCacheLevels_;

  void setCacheHierarchy() {
    for (unsigned int i = 0; i < maxNumberCacheLevels; i++) {
      dataCacheSize_[i] = 0;
    }
  }

 public:
  int model;
  int family;
  int stepping;
  int extModel;
  int extFamily;
  int displayFamily;  // family + extFamily
  int displayModel;   // model + extModel

  unsigned int getNumCores(IntelCpuTopologyLevel level) { return 1; }

  unsigned int getDataCacheLevels() const { return dataCacheLevels_; }

  unsigned int getCoresSharingDataCache(unsigned int i) const {
    if (i >= dataCacheLevels_) throw Error(ERR_BAD_PARAMETER);
    return coresSharingDataCache_[i];
  }
  unsigned int getDataCacheSize(unsigned int i) const {
    if (i >= dataCacheLevels_) throw Error(ERR_BAD_PARAMETER);
    return dataCacheSize_[i];
  }

  /*
    data[] = { eax, ebx, ecx, edx }
  */
  static inline void getCpuid(unsigned int eaxIn, unsigned int data[4]) {
    assert(0);
  }
  static inline void getCpuidEx(unsigned int eaxIn, unsigned int ecxIn,
                                unsigned int data[4]) {
#ifdef XBYAK_INTEL_CPU_SPECIFIC
#ifdef _MSC_VER
    __cpuidex(reinterpret_cast<int*>(data), eaxIn, ecxIn);
#else
    __cpuid_count(eaxIn, ecxIn, data[0], data[1], data[2], data[3]);
#endif
#else
    (void)eaxIn;
    (void)ecxIn;
    (void)data;
#endif
  }
  static inline uint64 getXfeature() {
    assert(0);

    return 0;
  }
  typedef uint64 Type;

  static const Type NONE = 0;
  static const Type tMMX = 1 << 0;
  static const Type tMMX2 = 1 << 1;
  static const Type tCMOV = 1 << 2;
  static const Type tSSE = 1 << 3;
  static const Type tSSE2 = 1 << 4;
  static const Type tSSE3 = 1 << 5;
  static const Type tSSSE3 = 1 << 6;
  static const Type tSSE41 = 1 << 7;
  static const Type tSSE42 = 1 << 8;
  static const Type tPOPCNT = 1 << 9;
  static const Type tAESNI = 1 << 10;
  static const Type tSSE5 = 1 << 11;
  static const Type tOSXSAVE = 1 << 12;
  static const Type tPCLMULQDQ = 1 << 13;
  static const Type tAVX = 1 << 14;
  static const Type tFMA = 1 << 15;

  static const Type t3DN = 1 << 16;
  static const Type tE3DN = 1 << 17;
  static const Type tSSE4a = 1 << 18;
  static const Type tRDTSCP = 1 << 19;
  static const Type tAVX2 = 1 << 20;
  static const Type tBMI1 = 1 << 21;  // andn, bextr, blsi, blsmsk, blsr, tzcnt
  static const Type tBMI2 =
      1 << 22;  // bzhi, mulx, pdep, pext, rorx, sarx, shlx, shrx
  static const Type tLZCNT = 1 << 23;

  static const Type tINTEL = 1 << 24;
  static const Type tAMD = 1 << 25;

  static const Type tENHANCED_REP = 1 << 26;  // enhanced rep movsb/stosb
  static const Type tRDRAND = 1 << 27;
  static const Type tADX = 1 << 28;            // adcx, adox
  static const Type tRDSEED = 1 << 29;         // rdseed
  static const Type tSMAP = 1 << 30;           // stac
  static const Type tHLE = uint64(1) << 31;    // xacquire, xrelease, xtest
  static const Type tRTM = uint64(1) << 32;    // xbegin, xend, xabort
  static const Type tF16C = uint64(1) << 33;   // vcvtph2ps, vcvtps2ph
  static const Type tMOVBE = uint64(1) << 34;  // mobve
  static const Type tAVX512F = uint64(1) << 35;
  static const Type tAVX512DQ = uint64(1) << 36;
  static const Type tAVX512_IFMA = uint64(1) << 37;
  static const Type tAVX512IFMA = tAVX512_IFMA;
  static const Type tAVX512PF = uint64(1) << 38;
  static const Type tAVX512ER = uint64(1) << 39;
  static const Type tAVX512CD = uint64(1) << 40;
  static const Type tAVX512BW = uint64(1) << 41;
  static const Type tAVX512VL = uint64(1) << 42;
  static const Type tAVX512_VBMI = uint64(1) << 43;
  static const Type tAVX512VBMI = tAVX512_VBMI;  // changed by Intel's manual
  static const Type tAVX512_4VNNIW = uint64(1) << 44;
  static const Type tAVX512_4FMAPS = uint64(1) << 45;
  static const Type tPREFETCHWT1 = uint64(1) << 46;
  static const Type tPREFETCHW = uint64(1) << 47;
  static const Type tSHA = uint64(1) << 48;
  static const Type tMPX = uint64(1) << 49;
  static const Type tAVX512_VBMI2 = uint64(1) << 50;
  static const Type tGFNI = uint64(1) << 51;
  static const Type tVAES = uint64(1) << 52;
  static const Type tVPCLMULQDQ = uint64(1) << 53;
  static const Type tAVX512_VNNI = uint64(1) << 54;
  static const Type tAVX512_BITALG = uint64(1) << 55;
  static const Type tAVX512_VPOPCNTDQ = uint64(1) << 56;
  static const Type tAVX512_BF16 = uint64(1) << 57;
  static const Type tAVX512_VP2INTERSECT = uint64(1) << 58;
  static const Type tA64FX = uint64(1) << 59;
  static const Type tNEON64 = uint64(1) << 60;
  static const Type tNEON128 = uint64(1) << 61;
  static const Type tSIMD = uint64(1) << 62;
  static const Type tSVE = uint64(1) << 63; /* If shift amount is more than
                                       32,
                                       explicitly cast to 64 bits */

  Cpu()
      : type_(NONE),
        numCores_(),
        dataCacheSize_(),
        coresSharingDataCache_(),
        dataCacheLevels_(0) {
    type_ |= tA64FX;
    type_ |= tSIMD;
    type_ |= tSVE;

    for (unsigned int i = 0; i < maxNumberCacheLevels; i++) {
      coresSharingDataCache_[i] = 0;
    }
  }

  void putFamily() const {
    printf("family=%d, model=%X, stepping=%d, extFamily=%d, extModel=%X\n",
           family, model, stepping, extFamily, extModel);
    printf("display:family=%X, model=%X\n", displayFamily, displayModel);
  }
  bool has(Type type) const { return (type & type_) != 0; }
};  // class Cpu {

#ifdef XBYAK64
static const Reg64 rax(0), rcx(1), rdx(2), rbx(3), rsp(4), rbp(5), rsi(6),
    rdi(7), r8(8), r9(9), r10(10), r11(11), r12(12), r13(13), r14(14), r15(15);
static const WReg r8d(8), r9d(9), r10d(10), r11d(11), r12d(12), r13d(13),
    r14d(14), r15d(15);
#endif
}
}  // end of util
#endif
