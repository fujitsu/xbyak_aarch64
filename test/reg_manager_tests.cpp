#include "xbyak_aarch64.h"
#include <iostream>

using namespace Xbyak_aarch64;

#define CHECK(expr)                                                                                                                                                                                                                                                                                        \
  do {                                                                                                                                                                                                                                                                                                     \
    if (!(expr)) {                                                                                                                                                                                                                                                                                         \
      std::cerr << __func__ << ":" << __LINE__ << ": CHECK failed: " #expr << std::endl;                                                                                                                                                                                                                   \
      return false;                                                                                                                                                                                                                                                                                        \
    }                                                                                                                                                                                                                                                                                                      \
  } while (0)

#define CHECK_ERR(stmt, expected_err)                                                                                                                                                                                                                                                                      \
  do {                                                                                                                                                                                                                                                                                                     \
    bool caught_expected = false;                                                                                                                                                                                                                                                                          \
    try {                                                                                                                                                                                                                                                                                                  \
      stmt;                                                                                                                                                                                                                                                                                                \
    } catch (const Error &e) {                                                                                                                                                                                                                                                                             \
      caught_expected = ((int)e == (expected_err));                                                                                                                                                                                                                                                        \
      if (!caught_expected) {                                                                                                                                                                                                                                                                              \
        std::cerr << __func__ << ":" << __LINE__ << ": expected error " << (expected_err) << ", got " << (int)e << std::endl;                                                                                                                                                                              \
      }                                                                                                                                                                                                                                                                                                    \
    }                                                                                                                                                                                                                                                                                                      \
    if (!caught_expected) {                                                                                                                                                                                                                                                                                \
      std::cerr << __func__ << ":" << __LINE__ << ": CHECK_ERR failed: " #stmt << std::endl;                                                                                                                                                                                                               \
      return false;                                                                                                                                                                                                                                                                                        \
    }                                                                                                                                                                                                                                                                                                      \
  } while (0)

// Examples

// basic example demonstrating free at end of scope
struct ExampleScopedSummationLoop : public CodeGenerator {
  void generate() {
    RegPoolManager rm;

    auto x_base = rm.allocScoped<XReg>(0);
    auto x_sum = rm.allocScoped<XReg>();
    for (int i = 0; i < 50; i++) {
      auto x_tmp = rm.allocScoped<XReg>();

      (void)x_tmp.getIdx();
      (void)x_tmp.getBit(); // accesses underlying reg object methods through forwarding

      add(x_tmp, x_base, x_sum);
      mov(x_sum, x_tmp);
    } // x_tmp freed at end of scope
  }
};

// example demonstrating nested scopes
struct ExampleLoopTmps : public CodeGenerator {
  void generate() {
    RegPoolManager rm;

    auto ptr_reg = rm.allocScoped<XReg>();
    auto len_reg = rm.allocScoped<XReg>();
    auto sum_reg = rm.allocScoped<XReg>();

    mov(sum_reg, 0);

    Label loop, done;
    L(loop);

    cmp(len_reg, 0);
    b(EQ, done);

    {
      auto tmp_val = rm.allocScoped<XReg>();
      auto tmp_acc = rm.allocScoped<XReg>();

      ldr(tmp_val, ptr(ptr_reg));
      add(tmp_acc, sum_reg, tmp_val);
      mov(sum_reg, tmp_acc);
    } // tmp_val and tmp_acc freed at end of scope

    add(ptr_reg, ptr_reg, 8);
    sub(len_reg, len_reg, 1);
    b(loop);

    L(done);
  }
};

// example demonstrating nested temp reuse across loop scopes
struct ExampleNestedReuse : public CodeGenerator {
  void generate() {
    RegPoolManager rm;
    rm.clear_free_gp();
    rm.clear_preserved_gp();
    rm.add_to_gp_pool(0);
    rm.add_to_gp_pool(5);
    rm.add_to_gp_pool(10);

    auto x_base = rm.allocScoped<XReg>(0);
    auto x_sum = rm.allocScoped<XReg>(); // x5

    for (int outer = 0; outer < 2; outer++) {
      for (int inner = 0; inner < 3; inner++) {
        auto x_tmp = rm.allocScoped<XReg>(); // x10
        add(x_tmp, x_base, x_sum);
        mov(x_sum, x_tmp);
      } // x10 freed here

      {
        auto x_other = rm.allocScoped<XReg>(); // x10 reused here
        sub(x_other, x_sum, x_base);
        mov(x_sum, x_other);
      } // x10 freed here again
    }
  }
};

// example demonstrating list and element syntax with scoped vector regs
struct ExampleSimdList : public CodeGenerator {
  void generate() {
    RegPoolManager rm;

    auto x_ptr = rm.allocScoped<XReg>();

    auto v0 = rm.allocScoped<VReg2D>(9);
    auto v1 = rm.allocScoped<VReg2D>(10);
    auto v2 = rm.allocScoped<VReg2D>(11);
    auto v3 = rm.allocScoped<VReg2D>(12);

    ld4((v0 - VReg2D(v3.getIdx()))[0], post_ptr(x_ptr, 64));
    st4((VReg2D(v0.getIdx()) - v3)[0], post_ptr(x_ptr, 64));
  }
};

// example demonstrating scoped predicate and shaped ZReg usage
struct ExampleSvePredicate : public CodeGenerator {
  void generate() {
    RegPoolManager rm;

    auto x_ptr = rm.allocScoped<XReg>();
    auto p_mask = rm.allocScoped<PReg>();
    auto z_zero = rm.allocScoped<ZRegS>();
    auto z_out = rm.allocScoped<ZRegS>();

    ptrue(PRegS(p_mask.getIdx()));
    dup(z_zero, 0);

    ld1w(z_out, p_mask / T_z, ptr(x_ptr));
    fmax(z_out, p_mask / T_m, z_zero);
    st1w(z_out, p_mask, ptr(x_ptr));
  }
};

// this and the example below aim to show how the manager can fit alongside or combined with existing usage of raw reg objects
// example demonstrating use of the manager alongside existing usage, and how a developer might leverage both by reserving regs already used in a kernel
struct ExampleHybridUsage1 : public CodeGenerator {
  void generate() {
    RegPoolManager rm;
    XReg x_overflow(0);
    VReg v_src(1);
    VReg v_acc(2);
    VReg v_tmp(3);

    auto scp_x_overflow = rm.allocScoped<XReg>(x_overflow);
    // reserving regs already in use in the kernel to ensure they are not touched by the manager
    rm.reserve(v_src);
    rm.reserve(v_acc);
    rm.reserve(v_tmp);

    Label no_overflow, done;

    // same architectural register used through different typed views
    fadd(v_tmp.s, v_src.s, v_acc.s);
    fadd(v_acc.d, v_acc.d, v_src.d);

    // runtime branch: v_overflow is only consumed if this path is taken
    cmp(x_overflow, 0);
    b(EQ, no_overflow);
    {
      auto v_overflow = rm.allocScoped<VReg>();
      fmax(v_overflow.get().s, v_tmp.s, v_src.s);
      fsub(v_acc.s, v_acc.s, v_overflow.get().s);
      b(done);
    }
    L(no_overflow);
    fadd(v_acc.s, v_acc.s, v_tmp.s);
    L(done);
  }
};

// example demonstrating use of the manager alongside existing usage, and how a developer might leverage both while scoping raw reg objects that already exist
struct ExampleHybridUsage2 : public CodeGenerator {
  void generate() {
    RegPoolManager rm;
    XReg x_overflow(0);
    VReg v_src(1);
    VReg v_acc(2);
    VReg v_tmp(3);

    // scope existing reg objects to ensure they are freed at end of scope
    // this allows the reg objects to be tracked by the manager, while still allowing the developer to use the dot member views on the raw reg objects
    auto scp_x_overflow = rm.allocScoped<XReg>(x_overflow);
    auto scp_v_src = rm.allocScoped<VReg>(v_src);
    auto scp_v_acc = rm.allocScoped<VReg>(v_acc);
    auto scp_v_tmp = rm.allocScoped<VReg>(v_tmp);

    Label no_overflow, done;

    // same architectural register used through different typed views
    fadd(v_tmp.s, v_src.s, v_acc.s);
    fadd(v_acc.d, v_acc.d, v_src.d);

    // runtime branch: v_overflow is only consumed if this path is taken
    cmp(x_overflow, 0);
    b(EQ, no_overflow);
    {
      auto v_overflow = rm.allocScoped<VReg>();
      fmax(v_overflow.get().s, v_tmp.s, v_src.s);
      fsub(v_acc.s, v_acc.s, v_overflow.get().s);
      b(done);
    }
    L(no_overflow);
    fadd(v_acc.s, v_acc.s, v_tmp.s);
    L(done);
  }
};

// Multi step tests - test basically functionality of the manager
// test - alloc x0, clear free gp set, allocate & scope next unallocated reg (x19 as lowest index in preserved_gp), scope x0, check live, scope and alloc
// next unallocated reg (x20), check live, end inner scope, check live, end outer scope, check live and used sets.
bool test_example_scoped_usage() {
  RegPoolManager rm;
  XReg xreg0 = rm.alloc<XReg>();
  rm.clear_free_gp();
  {
    auto scoped_preserved_reg = rm.allocScoped<XReg>();
    auto scoped_free_reg = rm.makeScoped(xreg0);
    CHECK(rm.get_all_live_gp() == std::vector<int>({0, 19}));
    {
      auto scoped_preserved_reg2 = rm.allocScoped<XReg>();
      CHECK(rm.get_all_live_gp() == std::vector<int>({0, 19, 20}));
    }
    CHECK(rm.get_all_live_gp() == std::vector<int>({0, 19}));
  }
  CHECK(rm.get_all_live_gp() == std::vector<int>({}));
  // get_non_volatile_used_gp returns the set of call-preserved registers that have been allocated at any point during the lifetime of the manager
  CHECK(rm.get_non_volatile_used_gp() == std::vector<int>({19, 20})); // x19 & x20 is returned, allowing it to be saved/restored
  // x19 & x20 are added to the free set after use, rather than preserved. This means they are prioritised for use over the preserved set, minimising the potential number of spill/restore calls required.
  CHECK(rm.get_all_free_gp() == std::vector<int>({0, 19, 20}));
  return true;
}

// test - allocate a register, scope the register to a function, confirm that the register is freed at the end of the scope
bool test_scoped_free() {
  RegPoolManager rm;
  {
    auto scoped_reg = rm.makeScoped(rm.alloc<XReg>());
    CHECK(rm.reg_in_use(scoped_reg) == true);
  }
  CHECK(rm.gp_idx_in_use(0) == false);
  return true;
}

// test - test a raw register's index can be tracked by the reg manager when scoped
bool test_alloc_scoped_raw_hybrid_same_idx() {
  RegPoolManager rm;
  VReg v(3);
  auto sv = rm.allocScoped(v);
  return sv.getIdx() == v.getIdx() && rm.get_all_live_vec() == std::vector<int>({3});
}

// test - simple allocation and free of a register
bool test_alloc_free_simple() {
  RegPoolManager rm;
  XReg x_eg = rm.alloc<XReg>();
  CHECK(x_eg.getIdx() == 0);
  CHECK(rm.gp_idx_in_use(0) == true);
  rm.free(x_eg);
  CHECK(rm.gp_idx_in_use(0) == false);
  return true;
};

// test - reserve, check, add to pool, check
bool test_reserve_add_to_pool() {
  RegPoolManager rm;
  rm.reserve_gp(5);
  CHECK(rm.get_all_free_gp() == std::vector<int>({0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));
  rm.add_to_gp_pool(5);
  CHECK(rm.get_all_free_gp() == std::vector<int>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));
  return true;
};

// Simple, non-exception-throwing tests

// test: allocs next free reg - 0 for base sets.
bool test_alloc_next_0_gp() {
  RegPoolManager rm;
  XReg x_next = rm.alloc<XReg>();
  if (x_next.getIdx() != 0)
    return false;
  if (rm.gp_idx_in_use(0) != true)
    return false;
  return true;
};
bool test_alloc_next_0_vec() {
  RegPoolManager rm;
  BReg b_next = rm.alloc<BReg>();
  if (b_next.getIdx() != 0)
    return false;
  if (rm.vec_idx_in_use(0) != true)
    return false;
  return true;
};
bool test_alloc_next_0_pred() {
  RegPoolManager rm;
  PReg p_next = rm.alloc<PReg>();
  if (p_next.getIdx() != 0)
    return false;
  if (rm.pred_idx_in_use(0) != true)
    return false;
  return true;
};

// test: for a set with 3 free regs, allocs next free reg in order of lowest to highest index
bool test_alloc_next_gp() {
  RegPoolManager rm;
  rm.clear_free_gp();
  rm.add_to_gp_pool(9);
  rm.add_to_gp_pool(13);
  rm.add_to_gp_pool(12);
  XReg x_next = rm.alloc<XReg>();
  if (x_next.getIdx() != 9)
    return false;
  if (rm.gp_idx_in_use(9) != true)
    return false;
  XReg x_next2 = rm.alloc<XReg>();
  if (x_next2.getIdx() != 12)
    return false;
  if (rm.gp_idx_in_use(12) != true)
    return false;
  return true;
};
bool test_alloc_next_vec() {
  RegPoolManager rm;
  rm.clear_free_vec();
  rm.add_to_vec_pool(16);
  rm.add_to_vec_pool(24);
  rm.add_to_vec_pool(18);
  BReg b_next = rm.alloc<BReg>();
  if (b_next.getIdx() != 16)
    return false;
  if (rm.vec_idx_in_use(16) != true)
    return false;
  BReg b_next2 = rm.alloc<BReg>();
  if (b_next2.getIdx() != 18)
    return false;
  if (rm.vec_idx_in_use(18) != true)
    return false;
  return true;
};
bool test_alloc_next_pred() {
  RegPoolManager rm;
  rm.clear_free_pred();
  rm.clear_preserved_pred();
  rm.add_to_pred_pool(4);
  rm.add_to_pred_pool(12);
  rm.add_to_pred_pool(8);
  PReg p_next = rm.alloc<PReg>();
  if (p_next.getIdx() != 4)
    return false;
  if (rm.pred_idx_in_use(4) != true)
    return false;
  PReg p_next2 = rm.alloc<PReg>();
  if (p_next2.getIdx() != 8)
    return false;
  if (rm.pred_idx_in_use(8) != true)
    return false;
  return true;
};

// test - for an empty free set, alloc() allocates the next available preserved register
bool test_alloc_next_empty_free_gp() {
  RegPoolManager rm;
  rm.clear_free_gp();
  XReg x_next = rm.alloc<XReg>();
  if (x_next.getIdx() != 19)
    return false;
  if (rm.gp_idx_in_use(19) != true)
    return false;
  return true;
};
bool test_alloc_next_empty_free_vec() {
  RegPoolManager rm;
  rm.clear_free_vec();
  VReg v_next = rm.alloc<VReg>();
  if (v_next.getIdx() != 8)
    return false;
  if (rm.vec_idx_in_use(8) != true)
    return false;
  return true;
};
bool test_alloc_next_empty_free_pred() {
  RegPoolManager rm;
  rm.clear_free_pred();
  PReg p_next = rm.alloc<PReg>();
  if (p_next.getIdx() != 4)
    return false;
  if (rm.pred_idx_in_use(4) != true)
    return false;
  return true;
};

// test - allocate a given free register by index
bool test_alloc_specific_free_gp() {
  RegPoolManager rm;
  XReg x_15 = rm.alloc<XReg>(15);
  if (x_15.getIdx() != 15)
    return false;
  if (rm.gp_idx_in_use(15) != true)
    return false;
  return true;
};
bool test_alloc_specific_free_vec() {
  RegPoolManager rm;
  QReg q_22 = rm.alloc<QReg>(22);
  if (q_22.getIdx() != 22)
    return false;
  if (rm.vec_idx_in_use(22) != true)
    return false;
  return true;
};
bool test_alloc_specific_free_pred() {
  RegPoolManager rm;
  PRegS ps_4 = rm.alloc<PRegS>(4);
  if (ps_4.getIdx() != 4)
    return false;
  if (rm.pred_idx_in_use(4) != true)
    return false;
  return true;
};

// test - allocate a given preserved register by index
bool test_alloc_specific_preserved_gp() {
  RegPoolManager rm;
  WReg w_25 = rm.alloc<WReg>(25);
  if (w_25.getIdx() != 25)
    return false;
  if (rm.gp_idx_in_use(25) != true)
    return false;
  return true;
};
bool test_alloc_specific_preserved_vec() {
  RegPoolManager rm;
  ZReg z_10 = rm.alloc<ZReg>(10);
  if (z_10.getIdx() != 10)
    return false;
  if (rm.vec_idx_in_use(10) != true)
    return false;
  return true;
};
bool test_alloc_specific_preserved_pred() {
  RegPoolManager rm;
  PRegD pd_15 = rm.alloc<PRegD>(15);
  if (pd_15.getIdx() != 15)
    return false;
  if (rm.pred_idx_in_use(15) != true)
    return false;
  return true;
};

// test - add an untracked register to the pool of free registers by reg object.
// Note: only gp regs can be added to the pool by register object - this is to allow the behaviour shown below with special registers, which only occur in the gp set.
bool test_add_to_pool_untracked_gp_by_reg() {
  RegPoolManager rm;
  rm.clear_free_gp();
  rm.add_to_gp_pool(rm.platform_reg()); // x18, untracked register
  if (rm.get_all_free_gp() != std::vector<int>({18}))
    return false;
  return true;
};

// test - add an untracked register to the pool of free registers by index
bool test_add_to_pool_untracked_gp_by_idx() {
  RegPoolManager rm;
  rm.clear_free_gp();
  rm.add_to_gp_pool(18);
  if (rm.get_all_free_gp() != std::vector<int>({18}))
    return false;
  return true;
};
bool test_add_to_pool_untracked_vec_by_idx() {
  RegPoolManager rm;
  rm.clear_free_vec();
  rm.add_to_vec_pool(7);
  if (rm.get_all_free_vec() != std::vector<int>({7}))
    return false;
  return true;
};
bool test_add_to_pool_untracked_pred_by_idx() {
  RegPoolManager rm;
  rm.clear_free_pred();
  rm.add_to_pred_pool(2);
  if (rm.get_all_free_pred() != std::vector<int>({2}))
    return false;
  return true;
};

// test - remove a tracked register from the pool of free registers by idx
bool test_reserve_free_gp() {
  RegPoolManager rm;
  rm.reserve_gp(5);
  std::vector<int> current_free = rm.get_all_free_gp();
  auto it = std::find(current_free.begin(), current_free.end(), 5);
  if (it != current_free.end())
    return false;
  return true;
};
bool test_reserve_free_vec() {
  RegPoolManager rm;
  rm.reserve_vec(5);
  std::vector<int> current_free = rm.get_all_free_vec();
  auto it = std::find(current_free.begin(), current_free.end(), 5);
  if (it != current_free.end())
    return false;
  return true;
};
bool test_reserve_free_pred() {
  RegPoolManager rm;
  rm.reserve_pred(3);
  std::vector<int> current_free = rm.get_all_free_pred();
  auto it = std::find(current_free.begin(), current_free.end(), 3);
  if (it != current_free.end())
    return false;
  return true;
};

// test - remove a tracked register from the pool of preserved registers by idx
bool test_reserve_preserved_gp() {
  RegPoolManager rm;
  rm.reserve_gp(24);
  std::vector<int> current_preserved = rm.get_non_volatile_preserved_gp();
  auto it = std::find(current_preserved.begin(), current_preserved.end(), 24);
  if (it != current_preserved.end())
    return false;
  return true;
};
bool test_reserve_preserved_vec() {
  RegPoolManager rm;
  rm.reserve_vec(10);
  std::vector<int> current_preserved = rm.get_non_volatile_preserved_vec();
  auto it = std::find(current_preserved.begin(), current_preserved.end(), 10);
  if (it != current_preserved.end())
    return false;
  return true;
};
bool test_reserve_preserved_pred() {
  RegPoolManager rm;
  rm.reserve_pred(10);
  std::vector<int> current_preserved = rm.get_non_volatile_preserved_pred();
  auto it = std::find(current_preserved.begin(), current_preserved.end(), 10);
  if (it != current_preserved.end())
    return false;
  return true;
};

// test - confirm that a register that is in use returns true when checked with *_idx_in_use
bool test_idx_in_use_true_gp() {
  RegPoolManager rm;
  rm.alloc<XReg>(15);
  if (rm.gp_idx_in_use(15) != true)
    return false;
  return true;
};
bool test_idx_in_use_true_vec() {
  RegPoolManager rm;
  rm.alloc<BReg>(15);
  if (rm.vec_idx_in_use(15) != true)
    return false;
  return true;
};
bool test_idx_in_use_true_pred() {
  RegPoolManager rm;
  rm.alloc<PRegS>(10);
  if (rm.pred_idx_in_use(10) != true)
    return false;
  return true;
};

// test - confirm that a register that is not in use returns false when checked with *_idx_in_use
bool test_idx_in_use_false_gp() {
  RegPoolManager rm;
  if (rm.gp_idx_in_use(15) != false)
    return false;
  return true;
};
bool test_idx_in_use_false_vec() {
  RegPoolManager rm;
  if (rm.vec_idx_in_use(15) != false)
    return false;
  return true;
};
bool test_idx_in_use_false_pred() {
  RegPoolManager rm;
  if (rm.pred_idx_in_use(10) != false)
    return false;
  return true;
};

// test - a register that is in use returns true when checked with reg_in_use
bool test_reg_in_use_true_gp() {
  RegPoolManager rm;
  XReg x_eg = rm.alloc<XReg>(15);
  if (rm.reg_in_use(x_eg) != true)
    return false;
  return true;
};
bool test_reg_in_use_true_vec() {
  RegPoolManager rm;
  BReg b_eg = rm.alloc<BReg>(15);
  if (rm.reg_in_use(b_eg) != true)
    return false;
  return true;
};
bool test_reg_in_use_true_pred() {
  RegPoolManager rm;
  PRegS ps_eg = rm.alloc<PRegS>(10);
  if (rm.reg_in_use(ps_eg) != true)
    return false;
  return true;
};

// test - a register that is not in use returns false when checked with reg_in_use
bool test_reg_in_use_false() {
  RegPoolManager rm;
  if (rm.reg_in_use(XReg(15)) != false)
    return false;
  return true;
};
bool test_reg_in_use_false_vec() {
  RegPoolManager rm;
  if (rm.reg_in_use(BReg(15)) != false)
    return false;
  return true;
};
bool test_reg_in_use_false_pred() {
  RegPoolManager rm;
  if (rm.reg_in_use(PRegS(10)) != false)
    return false;
  return true;
};

// test - forwarded scoped helper surface works for vector and predicate regs. check parity between scoped and raw reg.
bool test_scoped_forwarded_surface() {
  RegPoolManager rm;

  auto v = rm.allocScoped<VReg2D>(0);
  CHECK(v.getIdx() == 0);
  CHECK(v.getBit() == 64);
  CHECK(v.isVRegVec() == true);
  CHECK(v.getLane() == 2);
  auto elem = v[1];
  CHECK(elem.getElemIdx() == 1);

  auto v_other = rm.allocScoped<VReg2D>(1);
  auto list_scoped_raw = v - VReg2D(v_other.getIdx());
  auto list_raw_scoped = VReg2D(v.getIdx()) - v_other;
  auto list_scoped_scoped = v - v_other;
  CHECK(list_scoped_raw.getLen() == 2);
  CHECK(list_raw_scoped.getLen() == 2);
  CHECK(list_scoped_scoped.getLen() == 2);

  auto z = rm.allocScoped<ZRegS>(8);
  CHECK(z.isZReg() == true);

  auto p = rm.allocScoped<PReg>(0);
  auto pg_m = p / T_m;
  auto pg_z = p / T_z;
  CHECK(p.isPRegM() == false);
  CHECK(p.isPRegZ() == true);
  CHECK(p.isM() == false);
  CHECK(p.isZ() == true);
  CHECK(pg_m.isM() == true);
  CHECK(pg_z.isZ() == true);
  return true;
};

// test - free a live register and confirm the idx has been moved from live to free
// NOTE: these test also demonstrate that free() moves an idx from live to free, even if the idx orginated in the preserved set.
bool test_free_live_gp() {
  RegPoolManager rm;
  XReg x_eg = rm.alloc<XReg>(20);
  rm.clear_free_gp();
  rm.free(x_eg);
  if (rm.gp_idx_in_use(20) != false)
    return false;
  if (rm.get_all_free_gp() != std::vector<int>({20}))
    return false;
  if (rm.get_all_live_gp() != std::vector<int>({}))
    return false;
  return true;
};
bool test_free_live_vec() {
  RegPoolManager rm;
  BReg b_eg = rm.alloc<BReg>(15);
  rm.clear_free_vec();
  rm.free(b_eg);
  if (rm.vec_idx_in_use(15) != false)
    return false;
  if (rm.get_all_free_vec() != std::vector<int>({15}))
    return false;
  if (rm.get_all_live_vec() != std::vector<int>({}))
    return false;
  return true;
}
bool test_free_live_pred() {
  RegPoolManager rm;
  PRegS ps_eg = rm.alloc<PRegS>(10);
  rm.clear_free_pred();
  rm.free(ps_eg);
  if (rm.pred_idx_in_use(10) != false)
    return false;
  if (rm.get_all_free_pred() != std::vector<int>({10}))
    return false;
  if (rm.get_all_live_pred() != std::vector<int>({}))
    return false;
  return true;
};

// test - clear methods remove all indices from a set
bool test_clears() {
  RegPoolManager rm;
  rm.alloc<XReg>(20);
  rm.alloc<BReg>(15);
  rm.alloc<PRegS>(10);
  CHECK(rm.get_all_live_gp() == std::vector<int>({20}));
  CHECK(rm.get_all_live_vec() == std::vector<int>({15}));
  CHECK(rm.get_all_live_pred() == std::vector<int>({10}));
  rm.clear_live_gp();
  rm.clear_live_vec();
  rm.clear_live_pred();
  CHECK(rm.get_all_live_gp() == std::vector<int>({}));
  CHECK(rm.get_all_live_vec() == std::vector<int>({}));
  CHECK(rm.get_all_live_pred() == std::vector<int>({}));
  return true;
};

// test - special reg functions return the correct register objects, and that these registers are tracked as used but not live by the manager
bool test_special_regs() {
  RegPoolManager rm;
  CHECK(rm.ipc_reg_0().getIdx() == 16);
  CHECK(rm.ipc_reg_1().getIdx() == 17);
  CHECK(rm.platform_reg().getIdx() == 18);
  CHECK(rm.frame_pointer().getIdx() == 29);
  CHECK(rm.link_reg().getIdx() == 30);
  CHECK(rm.get_all_live_gp() == std::vector<int>({}));
  CHECK(rm.get_all_used_gp() == std::vector<int>({16, 17, 18, 29, 30}));
  return true;
};

// test - reset_manager clears live and used sets, and resets free and preserved sets to their base states
bool test_reset() {
  RegPoolManager rm;
  rm.alloc<XReg>(20);
  rm.alloc<BReg>(15);
  rm.alloc<PRegS>(10);
  rm.reset_manager();
  CHECK(rm.get_all_live_gp() == std::vector<int>({}));
  CHECK(rm.get_all_live_vec() == std::vector<int>({}));
  CHECK(rm.get_all_live_pred() == std::vector<int>({}));
  CHECK(rm.get_all_free_gp() == std::vector<int>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));
  CHECK(rm.get_all_preserved_gp() == std::vector<int>({19, 20, 21, 22, 23, 24, 25, 26, 27, 28}));
  CHECK(rm.get_all_free_vec() == std::vector<int>({0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31}));
  CHECK(rm.get_all_preserved_vec() == std::vector<int>({8, 9, 10, 11, 12, 13, 14, 15}));
  CHECK(rm.get_all_free_pred() == std::vector<int>({0, 1, 2, 3}));
  CHECK(rm.get_all_preserved_pred() == std::vector<int>({4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));
  CHECK(rm.get_all_used_gp() == std::vector<int>({}));
  CHECK(rm.get_all_used_vec() == std::vector<int>({}));
  CHECK(rm.get_all_used_pred() == std::vector<int>({}));
  return true;
};

// Tests for exception throwing

// test - attempting to reserve an untracked register throws the correct error
bool test_add_to_pool_illegal_idx_gp() {
  RegPoolManager rm;
  CHECK_ERR(rm.add_to_gp_pool(32), ERR_ILLEGAL_REG_IDX);
  return true;
};
bool test_add_to_pool_illegal_idx_vec() {
  RegPoolManager rm;
  CHECK_ERR(rm.add_to_vec_pool(32), ERR_ILLEGAL_REG_IDX);
  return true;
};
bool test_add_to_pool_illegal_idx_pred() {
  RegPoolManager rm;
  CHECK_ERR(rm.add_to_pred_pool(16), ERR_ILLEGAL_REG_IDX);
  return true;
};

// test - attempting to add to pool a register which is already tracked by the manager throws the correct error
bool test_add_to_pool_already_tracked_gp() {
  RegPoolManager rm;
  CHECK_ERR(rm.add_to_gp_pool(0), ERR_RM_REG_ALREADY_TRACKED);
  return true;
}
bool test_add_to_pool_already_tracked_vec() {
  RegPoolManager rm;
  CHECK_ERR(rm.add_to_vec_pool(0), ERR_RM_REG_ALREADY_TRACKED);
  return true;
};
bool test_add_to_pool_already_tracked_pred() {
  RegPoolManager rm;
  CHECK_ERR(rm.add_to_pred_pool(0), ERR_RM_REG_ALREADY_TRACKED);
  return true;
};

// test - attempting to reserve a register which is not tracked by the manager throws the correct error
bool test_reserve_untracked_gp() {
  RegPoolManager rm;
  CHECK_ERR((rm.reserve_gp(0), rm.reserve_gp(0)), ERR_RM_REG_NOT_TRACKED);
  return true;
};
bool test_reserve_untracked_vec() {
  RegPoolManager rm;
  CHECK_ERR((rm.reserve_vec(0), rm.reserve_vec(0)), ERR_RM_REG_NOT_TRACKED);
  return true;
};
bool test_reserve_untracked_pred() {
  RegPoolManager rm;
  CHECK_ERR((rm.reserve_pred(0), rm.reserve_pred(0)), ERR_RM_REG_NOT_TRACKED);
  return true;
};

// test - attempting to reserve a register in the live set throws the correct error
bool test_reserve_in_use_gp() {
  RegPoolManager rm;
  CHECK_ERR((rm.alloc<XReg>(15), rm.reserve_gp(15)), ERR_RM_REG_IN_USE);
  return true;
};
bool test_reserve_in_use_vec() {
  RegPoolManager rm;
  CHECK_ERR((rm.alloc<BReg>(15), rm.reserve_vec(15)), ERR_RM_REG_IN_USE);
  return true;
};
bool test_reserve_in_use_pred() {
  RegPoolManager rm;
  CHECK_ERR((rm.alloc<PRegS>(10), rm.reserve_pred(10)), ERR_RM_REG_IN_USE);
  return true;
};

// test - allocating a register when there are no free/preserved regs throws the correct error
bool test_alloc_next_no_free_regs_gp() {
  RegPoolManager rm;
  rm.clear_free_gp();
  rm.clear_preserved_gp();
  CHECK_ERR(rm.alloc<XReg>(), ERR_RM_NO_UNALLOCATED_REG);
  return true;
};
bool test_alloc_next_no_free_regs_vec() {
  RegPoolManager rm;
  rm.clear_free_vec();
  rm.clear_preserved_vec();
  CHECK_ERR(rm.alloc<BReg>(), ERR_RM_NO_UNALLOCATED_REG);
  return true;
};
bool test_alloc_next_no_free_regs_pred() {
  RegPoolManager rm;
  rm.clear_free_pred();
  rm.clear_preserved_pred();
  CHECK_ERR(rm.alloc<PRegS>(), ERR_RM_NO_UNALLOCATED_REG);
  return true;
};

// test - allocating a register index which is already in use throws the correct error
bool test_alloc_in_use_reg_gp() {
  RegPoolManager rm;
  CHECK_ERR((rm.alloc<XReg>(15), rm.alloc<XReg>(15)), ERR_RM_REG_IN_USE);
  return true;
};
bool test_alloc_in_use_reg_vec() {
  RegPoolManager rm;
  CHECK_ERR((rm.alloc<BReg>(15), rm.alloc<BReg>(15)), ERR_RM_REG_IN_USE);
  return true;
};
bool test_alloc_in_use_reg_pred() {
  RegPoolManager rm;
  CHECK_ERR((rm.alloc<PRegS>(10), rm.alloc<PRegS>(10)), ERR_RM_REG_IN_USE);
  return true;
};

// test - allocating a register index which is not tracked throws the correct error
bool test_alloc_untracked_gp() {
  RegPoolManager rm;
  CHECK_ERR((rm.reserve_gp(3), rm.alloc<XReg>(3)), ERR_RM_REG_NOT_TRACKED);
  return true;
};
bool test_alloc_untracked_vec() {
  RegPoolManager rm;
  CHECK_ERR((rm.reserve_vec(3), rm.alloc<BReg>(3)), ERR_RM_REG_NOT_TRACKED);
  return true;
};
bool test_alloc_untracked_pred() {
  RegPoolManager rm;
  CHECK_ERR((rm.reserve_pred(3), rm.alloc<PRegS>(3)), ERR_RM_REG_NOT_TRACKED);
  return true;
};

// test - allocating a register index which is out of bounds throws the correct error
bool test_alloc_illegal_idx_gp() {
  RegPoolManager rm;
  CHECK_ERR(rm.alloc<XReg>(32), ERR_ILLEGAL_REG_IDX);
  return true;
};
bool test_alloc_illegal_idx_vec() {
  RegPoolManager rm;
  CHECK_ERR(rm.alloc<BReg>(32), ERR_ILLEGAL_REG_IDX);
  return true;
};
bool test_alloc_illegal_idx_pred() {
  RegPoolManager rm;
  CHECK_ERR(rm.alloc<PRegS>(16), ERR_ILLEGAL_REG_IDX);
  return true;
};

// test - freeing a register which is not in use throws the correct error
bool test_free_not_live_gp() {
  RegPoolManager rm;
  CHECK_ERR(rm.free(XReg(10)), ERR_RM_REG_NOT_IN_USE);
  return true;
};
bool test_free_not_live_vec() {
  RegPoolManager rm;
  CHECK_ERR(rm.free(BReg(10)), ERR_RM_REG_NOT_IN_USE);
  return true;
};
bool test_free_not_live_pred() {
  RegPoolManager rm;
  CHECK_ERR(rm.free(PRegS(10)), ERR_RM_REG_NOT_IN_USE);
  return true;
};

// test - freeing a register with an illegal index throws the correct error
bool test_free_illegal_idx_gp() {
  RegPoolManager rm;
  CHECK_ERR(rm.free(XReg(32)), ERR_ILLEGAL_REG_IDX);
  return true;
};
bool test_free_illegal_idx_vec() {
  RegPoolManager rm;
  CHECK_ERR(rm.free(BReg(32)), ERR_ILLEGAL_REG_IDX);
  return true;
};
bool test_free_illegal_idx_pred() {
  RegPoolManager rm;
  CHECK_ERR(rm.free(PRegS(16)), ERR_ILLEGAL_REG_IDX);
  return true;
};

// test - freeing a register that is not tracked throws the correct error
bool test_free_untracked_gp() {
  RegPoolManager rm;
  CHECK_ERR((rm.reserve_gp(3), rm.free(XReg(3))), ERR_RM_REG_NOT_IN_USE);
  return true;
};
bool test_free_untracked_vec() {
  RegPoolManager rm;
  CHECK_ERR((rm.reserve_vec(3), rm.free(BReg(3))), ERR_RM_REG_NOT_IN_USE);
  return true;
};
bool test_free_untracked_pred() {
  RegPoolManager rm;
  CHECK_ERR((rm.reserve_pred(3), rm.free(PRegS(3))), ERR_RM_REG_NOT_IN_USE);
  return true;
};

// test - calling reg_in_use with an illegal index throws the correct error
bool test_idx_in_use_illegal_idx_gp() {
  RegPoolManager rm;
  CHECK_ERR(rm.gp_idx_in_use(32), ERR_ILLEGAL_REG_IDX);
  return true;
};
bool test_idx_in_use_illegal_idx_vec() {
  RegPoolManager rm;
  CHECK_ERR(rm.vec_idx_in_use(32), ERR_ILLEGAL_REG_IDX);
  return true;
};
bool test_idx_in_use_illegal_idx_pred() {
  RegPoolManager rm;
  CHECK_ERR(rm.pred_idx_in_use(16), ERR_ILLEGAL_REG_IDX);
  return true;
};

// test - calling make_scoped on a register object that is not live throws the correct error
bool test_make_scoped_not_in_use_gp() {
  RegPoolManager rm;
  CHECK_ERR(rm.makeScoped(XReg(10)), ERR_RM_REG_NOT_IN_USE);
  return true;
};
bool test_make_scoped_not_in_use_vec() {
  RegPoolManager rm;
  CHECK_ERR(rm.makeScoped(BReg(10)), ERR_RM_REG_NOT_IN_USE);
  return true;
};
bool test_make_scoped_not_in_use_pred() {
  RegPoolManager rm;
  CHECK_ERR(rm.makeScoped(PRegS(10)), ERR_RM_REG_NOT_IN_USE);
  return true;
};

// test - unsupported register families throw the correct error
bool test_alloc_unsupported_family() {
  RegPoolManager rm;
  CHECK_ERR(rm.alloc<ZAReg>(), ERR_RM_ILLEGAL_REG_FAMILY);
  return true;
};
bool test_reg_in_use_unsupported_family() {
  RegPoolManager rm;
  CHECK_ERR(rm.reg_in_use(ZAReg(0)), ERR_RM_ILLEGAL_REG_FAMILY);
  return true;
};

int main() {
  struct TestCase {
    const char *name;
    bool (*fn)();
  };

  const TestCase tests[] = {
      {"test_example_scoped_usage", test_example_scoped_usage},
      {"test_scoped_free", test_scoped_free},
      {"test_alloc_free_simple", test_alloc_free_simple},
      {"test_reserve_add_to_pool", test_reserve_add_to_pool},
      {"test_alloc_next_0_gp", test_alloc_next_0_gp},
      {"test_alloc_next_0_vec", test_alloc_next_0_vec},
      {"test_alloc_next_0_pred", test_alloc_next_0_pred},
      {"test_alloc_next_gp", test_alloc_next_gp},
      {"test_alloc_next_vec", test_alloc_next_vec},
      {"test_alloc_next_pred", test_alloc_next_pred},
      {"test_alloc_next_empty_free_gp", test_alloc_next_empty_free_gp},
      {"test_alloc_next_empty_free_vec", test_alloc_next_empty_free_vec},
      {"test_alloc_next_empty_free_pred", test_alloc_next_empty_free_pred},
      {"test_alloc_specific_free_gp", test_alloc_specific_free_gp},
      {"test_alloc_specific_free_vec", test_alloc_specific_free_vec},
      {"test_alloc_specific_free_pred", test_alloc_specific_free_pred},
      {"test_alloc_specific_preserved_gp", test_alloc_specific_preserved_gp},
      {"test_alloc_specific_preserved_vec", test_alloc_specific_preserved_vec},
      {"test_alloc_specific_preserved_pred", test_alloc_specific_preserved_pred},
      {"test_add_to_pool_untracked_gp_by_reg", test_add_to_pool_untracked_gp_by_reg},
      {"test_add_to_pool_untracked_gp_by_idx", test_add_to_pool_untracked_gp_by_idx},
      {"test_add_to_pool_untracked_vec_by_idx", test_add_to_pool_untracked_vec_by_idx},
      {"test_add_to_pool_untracked_pred_by_idx", test_add_to_pool_untracked_pred_by_idx},
      {"test_reserve_free_gp", test_reserve_free_gp},
      {"test_reserve_free_vec", test_reserve_free_vec},
      {"test_reserve_free_pred", test_reserve_free_pred},
      {"test_reserve_preserved_gp", test_reserve_preserved_gp},
      {"test_reserve_preserved_vec", test_reserve_preserved_vec},
      {"test_reserve_preserved_pred", test_reserve_preserved_pred},
      {"test_idx_in_use_true_gp", test_idx_in_use_true_gp},
      {"test_idx_in_use_true_vec", test_idx_in_use_true_vec},
      {"test_idx_in_use_true_pred", test_idx_in_use_true_pred},
      {"test_idx_in_use_false_gp", test_idx_in_use_false_gp},
      {"test_idx_in_use_false_vec", test_idx_in_use_false_vec},
      {"test_idx_in_use_false_pred", test_idx_in_use_false_pred},
      {"test_reg_in_use_true_gp", test_reg_in_use_true_gp},
      {"test_reg_in_use_true_vec", test_reg_in_use_true_vec},
      {"test_reg_in_use_true_pred", test_reg_in_use_true_pred},
      {"test_reg_in_use_false", test_reg_in_use_false},
      {"test_reg_in_use_false_vec", test_reg_in_use_false_vec},
      {"test_reg_in_use_false_pred", test_reg_in_use_false_pred},
      {"test_scoped_forwarded_surface", test_scoped_forwarded_surface},
      {"test_free_live_gp", test_free_live_gp},
      {"test_free_live_vec", test_free_live_vec},
      {"test_free_live_pred", test_free_live_pred},
      {"test_clears", test_clears},
      {"test_special_regs", test_special_regs},
      {"test_reset", test_reset},
      {"test_add_to_pool_illegal_idx_gp", test_add_to_pool_illegal_idx_gp},
      {"test_add_to_pool_illegal_idx_vec", test_add_to_pool_illegal_idx_vec},
      {"test_add_to_pool_illegal_idx_pred", test_add_to_pool_illegal_idx_pred},
      {"test_add_to_pool_already_tracked_gp", test_add_to_pool_already_tracked_gp},
      {"test_add_to_pool_already_tracked_vec", test_add_to_pool_already_tracked_vec},
      {"test_add_to_pool_already_tracked_pred", test_add_to_pool_already_tracked_pred},
      {"test_reserve_untracked_gp", test_reserve_untracked_gp},
      {"test_reserve_untracked_vec", test_reserve_untracked_vec},
      {"test_reserve_untracked_pred", test_reserve_untracked_pred},
      {"test_reserve_in_use_gp", test_reserve_in_use_gp},
      {"test_reserve_in_use_vec", test_reserve_in_use_vec},
      {"test_reserve_in_use_pred", test_reserve_in_use_pred},
      {"test_alloc_next_no_free_regs_gp", test_alloc_next_no_free_regs_gp},
      {"test_alloc_next_no_free_regs_vec", test_alloc_next_no_free_regs_vec},
      {"test_alloc_next_no_free_regs_pred", test_alloc_next_no_free_regs_pred},
      {"test_alloc_in_use_reg_gp", test_alloc_in_use_reg_gp},
      {"test_alloc_in_use_reg_vec", test_alloc_in_use_reg_vec},
      {"test_alloc_in_use_reg_pred", test_alloc_in_use_reg_pred},
      {"test_alloc_untracked_gp", test_alloc_untracked_gp},
      {"test_alloc_untracked_vec", test_alloc_untracked_vec},
      {"test_alloc_untracked_pred", test_alloc_untracked_pred},
      {"test_alloc_illegal_idx_gp", test_alloc_illegal_idx_gp},
      {"test_alloc_illegal_idx_vec", test_alloc_illegal_idx_vec},
      {"test_alloc_illegal_idx_pred", test_alloc_illegal_idx_pred},
      {"test_free_not_live_gp", test_free_not_live_gp},
      {"test_free_not_live_vec", test_free_not_live_vec},
      {"test_free_not_live_pred", test_free_not_live_pred},
      {"test_free_illegal_idx_gp", test_free_illegal_idx_gp},
      {"test_free_illegal_idx_vec", test_free_illegal_idx_vec},
      {"test_free_illegal_idx_pred", test_free_illegal_idx_pred},
      {"test_free_untracked_gp", test_free_untracked_gp},
      {"test_free_untracked_vec", test_free_untracked_vec},
      {"test_free_untracked_pred", test_free_untracked_pred},
      {"test_idx_in_use_illegal_idx_gp", test_idx_in_use_illegal_idx_gp},
      {"test_idx_in_use_illegal_idx_vec", test_idx_in_use_illegal_idx_vec},
      {"test_idx_in_use_illegal_idx_pred", test_idx_in_use_illegal_idx_pred},
      {"test_make_scoped_not_in_use_gp", test_make_scoped_not_in_use_gp},
      {"test_make_scoped_not_in_use_vec", test_make_scoped_not_in_use_vec},
      {"test_make_scoped_not_in_use_pred", test_make_scoped_not_in_use_pred},
      {"test_alloc_unsupported_family", test_alloc_unsupported_family},
      {"test_reg_in_use_unsupported_family", test_reg_in_use_unsupported_family},
  };

  int failed = 0;
  const size_t test_count = sizeof(tests) / sizeof(tests[0]);
  for (size_t i = 0; i < test_count; ++i) {
    try {
      if (!tests[i].fn()) {
        std::cerr << "FAILED: " << tests[i].name << "\n";
        ++failed;
      }
    } catch (const Error &e) {
      std::cerr << "FAILED: " << tests[i].name << " threw Error(" << (int)e << "): " << e.what() << "\n";
      ++failed;
    } catch (const std::exception &e) {
      std::cerr << "FAILED: " << tests[i].name << " threw std::exception: " << e.what() << "\n";
      ++failed;
    } catch (...) {
      std::cerr << "FAILED: " << tests[i].name << " threw unknown exception\n";
      ++failed;
    }
  }

  if (failed != 0) {
    std::cerr << failed << " test(s) failed\n";
    return 1;
  }

  std::cout << test_count << " test(s) passed\n";
  return 0;
}
