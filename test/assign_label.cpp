#include "xbyak_aarch64.h"
#include <cstring>
#include <iostream>

using namespace Xbyak_aarch64;

static int errCount = 0;

static void check(bool ok, const char *func, int line, const char *expr) {
  if (ok)
    return;
  std::cerr << func << ":" << line << ": CHECK failed: " << expr << std::endl;
  ++errCount;
}

#define CHECK(expr) check(!!(expr), __func__, __LINE__, #expr)

static uint32_t read4(const uint8_t *p) {
  uint32_t v;
  memcpy(&v, p, 4);
  return v;
}

void test_unreferenced_dst() {
  struct Code : CodeGenerator {
    Code() {
      Label src, dst;
      L(src);
      assignL(dst, src); // dst has not been referenced yet
      CHECK(dst.getAddress() == src.getAddress());
      b(dst); // backward reference must be resolved
      CHECK(!hasUndefinedLabel());
      // a second assignL with another unreferenced dst must not throw ERR_LABEL_IS_REDEFINED
      Label dst2;
      try {
        assignL(dst2, src);
      } catch (const Error &e) {
        std::cerr << __func__ << ": unexpected exception: " << e.what() << std::endl;
        ++errCount;
      }
      CHECK(dst2.getAddress() == src.getAddress());
    }
  } c;
  const uint8_t *p = c.getCode();
  CHECK(read4(p) == 0x14000000); // b .
}

// assignL(dst, src) after a forward reference to dst
void test_forward_ref() {
  struct Code : CodeGenerator {
    Code() {
      Label src, dst;
      b(dst);       // +0
      cbz(w1, dst); // +4
      L(src);       // +8
      nop();
      nop();
      assignL(dst, src); // dst must point to src, not here
      CHECK(dst.getAddress() == src.getAddress());
    }
  } c;
  const uint8_t *p = c.getCode();
  const uint32_t off = 8;
  CHECK(read4(p) == (0x14000000 | (off >> 2)));                        // b src
  CHECK(read4(p + 4) == (0x34000000 | (((off - 4) >> 2) << 5) | 0x1)); // cbz w1, src
}

int main() {
  test_unreferenced_dst();
  test_forward_ref();
  if (errCount) {
    std::cerr << "FAIL " << errCount << std::endl;
    return 1;
  }
  std::cout << "OK" << std::endl;
  return 0;
}
