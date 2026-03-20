void gen() {
  bool err;
  TEST(bfmopa(za4.s,p7/T_m,p7/T_m,z8.h,z8.h));
  TEST(bfmopa(za3.s,p8/T_m,p7/T_m,z8.h,z8.h));
  TEST(bfmopa(za3.s,p7/T_m,p8/T_m,z8.h,z8.h));
  TEST(bfmopa(za3.s,p7/T_z,p7/T_m,z8.h,z8.h));
  TEST(bfmopa(za3.s,p7/T_z,p7/T_z,z8.h,z8.h));
  TEST(bfmops(za4.s,p7/T_m,p7/T_m,z8.h,z8.h));
  TEST(bfmops(za3.s,p8/T_m,p7/T_m,z8.h,z8.h));
  TEST(bfmops(za3.s,p7/T_m,p8/T_m,z8.h,z8.h));
  TEST(bfmops(za3.s,p7/T_z,p7/T_m,z8.h,z8.h));
  TEST(bfmops(za3.s,p7/T_m,p7/T_z,z8.h,z8.h));
}