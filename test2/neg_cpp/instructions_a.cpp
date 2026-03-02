void gen() {
  bool err;
  TEST(addha(za3.s,p8/T_m,p7/T_m,z8.s));
  TEST(addha(za3.s,p7/T_m,p8/T_m,z8.s));
  TEST(addha(za4.s,p7/T_m,p7/T_m,z8.s));
  TEST(addha(za7.d,p8/T_m,p7/T_m,z8.d));
  TEST(addha(za7.d,p7/T_m,p8/T_m,z8.d));
  TEST(addha(za7.d,p7/T_z,p7/T_m,z8.d));
  // TEST(addha(za8.d,p7/T_m,p7/T_m,z8.d));
  TEST(addspl(x8,x8,-33));
  TEST(addspl(x8,x8,32));
  TEST(addsvl(x8,x8,-33));
  TEST(addsvl(x8,x8,32));
  TEST(addva(za3.s,p8/T_m,p7/T_m,z8.s));
  TEST(addva(za3.s,p7/T_m,p8/T_m,z8.s));
  TEST(addva(za4.s,p7/T_m,p7/T_m,z8.s));
  TEST(addva(za7.d,p8/T_m,p7/T_m,z8.d));
  TEST(addva(za7.d,p7/T_m,p8/T_m,z8.d));
  TEST(addva(za7.d,p7/T_z,p7/T_m,z8.d));
  // TEST(addva(za8.d,p7/T_m,p7/T_m,z8.d));
}