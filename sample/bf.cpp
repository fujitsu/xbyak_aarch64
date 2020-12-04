#include <fstream>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include <xbyak_aarch64/xbyak_aarch64.h>

class Brainfuck : public Xbyak_aarch64::CodeGenerator {
public:
  int getContinuousChar(std::istream &is, char c) {
    int count = 1;
    char p;
    while (is >> p) {
      if (p != c)
        break;
      count++;
    }
    is.unget();
    return count;
  }
  Brainfuck(std::istream &is) : CodeGenerator(100000) {
    // void (*)(void* putchar, void* getchar, int *stack)
    using namespace Xbyak_aarch64;
    const auto &pPutchar = x19;
    const auto &pGetchar = x20;
    const auto &stack = x21;
    const int saveSize = 48;
    stp(x29, x30, pre_ptr(sp, -saveSize));
    stp(pPutchar, pGetchar, ptr(sp, 16));
    str(stack, ptr(sp, 16 + 16));
    mov(pPutchar, x0);
    mov(pGetchar, x1);
    mov(stack, x2);

    std::stack<Label> labelF, labelB;
    char c;
    while (is >> c) {
      switch (c) {
      case '+':
      case '-': {
        int count = getContinuousChar(is, c);
        ldr(x0, ptr(stack));
        // QQQ : not support large count
        if (c == '+') {
          add(x0, x0, count);
        } else {
          sub(x0, x0, count);
        }
        str(x0, ptr(stack));
      } break;
      case '>':
      case '<': {
        int count = getContinuousChar(is, c) * 8;
        if (c == '>') {
          add(stack, stack, count);
        } else {
          sub(stack, stack, count);
        }
      } break;
      case '.':
        ldr(x0, ptr(stack));
        blr(pPutchar);
        break;
      case ',':
        blr(pGetchar);
        str(x0, ptr(stack));
        break;
      case '[': {
        Label B = L();
        labelB.push(B);
        ldr(x0, ptr(stack));
        cmp(x0, 0);
        Label F;
        beq(F);
        labelF.push(F);
      } break;
      case ']': {
        Label B = labelB.top();
        labelB.pop();
        b(B);
        Label F = labelF.top();
        labelF.pop();
        L(F);
      } break;
      default:
        break;
      }
    }

    ldr(stack, ptr(sp, 16 + 16));
    ldp(pPutchar, pGetchar, ptr(sp, 16));
    ldp(x29, x30, post_ptr(sp, saveSize));
    ret();
  }
};

int main(int argc, char *argv[]) try {
  if (argc != 2) {
    fprintf(stderr, "bf filename.bf\n");
    return 1;
  }
  std::ifstream ifs(argv[1]);
  Brainfuck bf(ifs);
  bf.ready();
  static int64_t stack[128 * 1024];
  auto f = bf.getCode<void (*)(const void *, const void *, int64_t *)>();
#if 0
  FILE *fp = fopen("bf.dump", "wb");
  fwrite((const void*)f, bf.getSize(), 1, fp);
  fclose(fp);
#endif
  f((const void *)putchar, (const void *)getchar, stack);
} catch (std::exception &e) {
  printf("ERR:%s\n", e.what());
  return 1;
}
