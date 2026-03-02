void gen() {
  bool err;
  struct {
    const ZRegB &z;
    const _PReg &pg;
    const _ZAHVReg &zahv;
  } mv8bit[] = {
    {z8.b,p7/T_m,za0h.h(w12,0)},
    {z8.b,p7/T_m,za0h.q(w12,0)},
    {z8.b,p7/T_m,za0h.d(w12,0)},
    {z8.b,p7/T_m,za0h.s(w12,0)},
    {z8.b,p8/T_m,za0h.b(w12,0)},
    {z8.b,p7/T_z,za0h.b(w12,0)},
    {z8.b,p7/T_m,za0h.b(w11,0)},
    {z8.b,p7/T_m,za0h.b(w16,0)},
    {z8.b,p7/T_m,za0h.b(w15,16)},
    {z8.b,p7/T_m,za1h.b(w15,15)}
  };

  for (auto i : mv8bit) {
    TEST(mov(i.z,i.pg,i.zahv));
    TEST(mova(i.z,i.pg,i.zahv));
    TEST(mov(i.zahv,i.pg,i.z));
    TEST(mova(i.zahv,i.pg,i.z));
  }
  
  struct {
    const ZRegH &z;
    const _PReg &pg;
    const _ZAHVReg &zahv;
  } mv16bit[] = {
    {z8.h,p7/T_m,za0h.b(w12,0)},
    {z8.h,p7/T_m,za0h.q(w12,0)},
    {z8.h,p7/T_m,za0h.d(w12,0)},
    {z8.h,p7/T_m,za0h.s(w12,0)},
    {z8.h,p8/T_m,za0h.h(w12,0)},
    {z8.h,p7/T_z,za0h.h(w12,0)},
    {z8.h,p7/T_m,za0h.h(w11,0)},
    {z8.h,p7/T_m,za0h.h(w16,0)},
    {z8.h,p7/T_m,za1h.h(w15,8)},
    {z8.h,p7/T_m,za2h.h(w15,0)}
  };

  for (auto i : mv16bit) {
    TEST(mov(i.z,i.pg,i.zahv));
    TEST(mova(i.z,i.pg,i.zahv));
    TEST(mov(i.zahv,i.pg,i.z));
    TEST(mova(i.zahv,i.pg,i.z));
  }

  struct {
    const ZRegS &z;
    const _PReg &pg;
    const _ZAHVReg &zahv;
  } mv32bit[] = {
    {z8.s,p7/T_m,za0h.b(w12,0)},
    {z8.s,p7/T_m,za0h.q(w12,0)},
    {z8.s,p7/T_m,za0h.d(w12,0)},
    {z8.s,p7/T_m,za0h.h(w12,0)},
    {z8.s,p8/T_m,za0h.s(w12,0)},
    {z8.s,p7/T_z,za0h.s(w12,0)},
    {z8.s,p7/T_m,za0h.s(w11,0)},
    {z8.s,p7/T_m,za0h.s(w16,0)},
    {z8.s,p7/T_m,za3h.s(w15,4)},
    {z8.s,p7/T_m,za4h.s(w15,15)}
  };

  for (auto i : mv32bit) {
    TEST(mov(i.z,i.pg,i.zahv));
    TEST(mova(i.z,i.pg,i.zahv));
    TEST(mov(i.zahv,i.pg,i.z));
    TEST(mova(i.zahv,i.pg,i.z));
  }

  struct {
    const ZRegD &z;
    const _PReg &pg;
    const _ZAHVReg &zahv;
  } mv64bit[] = {
    {z8.d,p7/T_m,za0h.b(w12,0)},
    {z8.d,p7/T_m,za0h.s(w12,0)},
    {z8.d,p7/T_m,za0h.q(w12,0)},
    {z8.d,p7/T_m,za0h.h(w12,0)},
    {z8.d,p8/T_m,za0h.d(w12,0)},
    {z8.d,p7/T_z,za0h.d(w12,0)},
    {z8.d,p7/T_m,za0h.d(w11,0)},
    {z8.d,p7/T_m,za0h.d(w16,0)},
    {z8.d,p7/T_m,za7h.d(w15,2)},
    {z8.d,p7/T_m,za8h.d(w15,15)}
  };

  for (auto i : mv64bit) {
    TEST(mov(i.z,i.pg,i.zahv));
    TEST(mova(i.z,i.pg,i.zahv));
    TEST(mov(i.zahv,i.pg,i.z));
    TEST(mova(i.zahv,i.pg,i.z));
  }

  struct {
    const ZRegQ &z;
    const _PReg &pg;
    const _ZAHVReg &zahv;
  } mv128bit[] = {
    {z8.q,p7/T_m,za0h.b(w12,0)},
    {z8.q,p7/T_m,za0h.s(w12,0)},
    {z8.q,p7/T_m,za0h.d(w12,0)},
    {z8.q,p7/T_m,za0h.h(w12,0)},
    {z8.q,p8/T_m,za0h.q(w12,0)},
    {z8.q,p7/T_z,za0h.q(w12,0)},
    {z8.q,p7/T_m,za0h.q(w11,0)},
    {z8.q,p7/T_m,za0h.q(w16,0)},
    {z8.q,p7/T_m,za15h.q(w15,1)}
    // {z8.q,p7/T_m,za16h.q(w15,15)}
  };

  for (auto i : mv128bit) {
    TEST(mov(i.z,i.pg,i.zahv));
    TEST(mova(i.z,i.pg,i.zahv));
    TEST(mov(i.zahv,i.pg,i.z));
    TEST(mova(i.zahv,i.pg,i.z));
  }
}