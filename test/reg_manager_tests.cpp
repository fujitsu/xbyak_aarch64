#include <iostream>
#include "xbyak_aarch64.h"

using namespace Xbyak_aarch64;


// test - clear free regs, create scope reg objs, demonstate joing free pool, return set of used call-preserved regs
bool test_example_scoped_usage(){
    RegPoolManager rm;
    XReg xreg0 = rm.alloc<XReg>();
    rm.clear_free_gp();
    {
        auto scoped_preserved_reg = rm.makeScoped(rm.alloc<XReg>());
        auto scoped_free_reg = rm.makeScoped(xreg0);
        if (rm.get_all_live_gp() != std::vector<int>({0, 19})) return false;
        {
            auto scoped_preserved_reg2 = rm.makeScoped(rm.alloc<XReg>());
            if (rm.get_all_live_gp() != std::vector<int>({0, 19, 20})) return false;
        }
        if (rm.get_all_live_gp() != std::vector<int>({0, 19})) return false;
    }
    if (rm.get_all_live_gp() != std::vector<int>({})) return false;
    // get_non_volatile_used_gp returns the set of call-preserved registers that have been allocated at any point during the lifetime of the manager
    if (rm.get_non_volatile_used_gp() != std::vector<int>({19, 20})) return false; //x19 is returned, allowing it to be saved/restored
    return true;
}

// test - allocate a register, scope the register to a function, confirm that the register is freed at the end of the scope
bool test_scoped_free(){
    RegPoolManager rm;
    {
        auto scoped_reg = rm.makeScoped(rm.alloc<XReg>());
        if (rm.reg_in_use(scoped_reg.get()) != true) return false;
    }
    if (rm.gp_idx_in_use(0) != false) return false;
    return true;
}

// test - simple allocation and free of a register
bool test_alloc_free_simple(){
    RegPoolManager rm;
    XReg x_eg = rm.alloc<XReg>();
    if (x_eg.getIdx() != 0) return false;
    if (rm.gp_idx_in_use(0) != true) return false;
    rm.free(x_eg);
    if (rm.gp_idx_in_use(0) != false) return false;
    return true;
};

// check, reserve, check, add to pool, check
bool test_reserve_add_to_pool(){
    RegPoolManager rm;
    rm.reserve_gp(5);
    if (rm.get_all_free_gp() != std::vector<int>({0,1,2,3,4,6,7,8,9,10,11,12,13,14,15})) return false;
    rm.add_to_gp_pool(5);
    if (rm.get_all_free_gp() != std::vector<int>({0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15})) return false;
    return true;
};

// Simple, non-exception-throwing tests

// test: allocs next free reg - 0 for base sets.
bool test_alloc_next_0_gp(){
    RegPoolManager rm;
    XReg x_next = rm.alloc<XReg>();
    if (x_next.getIdx() != 0) return false;
    if (rm.gp_idx_in_use(0) != true) return false;
    return true;
};
bool test_alloc_next_0_vec(){
    RegPoolManager rm;
    BReg b_next = rm.alloc<BReg>();
    if (b_next.getIdx() != 0) return false;
    if (rm.vec_idx_in_use(0) != true) return false;
    return true;
};
bool test_alloc_next_0_pred(){
    RegPoolManager rm;
    PReg p_next = rm.alloc<PReg>();
    if (p_next.getIdx() != 0) return false;
    if (rm.pred_idx_in_use(0) != true) return false;
    return true;
};

// test: for a set with 3 free regs, allocs next free reg in order of lowest to highest index
bool test_alloc_next_gp(){
    RegPoolManager rm;
    rm.clear_free_gp();
    rm.add_to_gp_pool(9);
    rm.add_to_gp_pool(13);
    rm.add_to_gp_pool(12);
    XReg x_next = rm.alloc<XReg>();
    if (x_next.getIdx() != 9) return false;
    if (rm.gp_idx_in_use(9) != true) return false;
    XReg  x_next2 = rm.alloc<XReg>();
    if (x_next2.getIdx() != 12) return false;
    if (rm.gp_idx_in_use(12) != true) return false;
    return true;
};
bool test_alloc_next_vec(){
    RegPoolManager rm;
    rm.clear_free_vec();
    rm.add_to_vec_pool(16);
    rm.add_to_vec_pool(24);
    rm.add_to_vec_pool(18);
    BReg b_next = rm.alloc<BReg>();
    if (b_next.getIdx() != 16) return false;
    if (rm.vec_idx_in_use(16) != true) return false;
    BReg  b_next2 = rm.alloc<BReg>();
    if (b_next2.getIdx() != 18) return false;
    if (rm.vec_idx_in_use(18) != true) return false;
    return true;
};
bool test_alloc_next_pred(){
    RegPoolManager rm;
    rm.clear_free_pred();
    rm.clear_preserved_pred();
    rm.add_to_pred_pool(4);
    rm.add_to_pred_pool(12);
    rm.add_to_pred_pool(8);
    PReg p_next = rm.alloc<PReg>();
    if (p_next.getIdx() != 4) return false;
    if (rm.pred_idx_in_use(4) != true) return false;
    PReg  p_next2 = rm.alloc<PReg>();
    if (p_next2.getIdx() != 8) return false;
    if (rm.pred_idx_in_use(8) != true) return false;
    return true;
};

// test - for an empty free set, alloc() allocates the next available preserved register
bool test_alloc_next_empty_free_gp(){
    RegPoolManager rm;
    rm.clear_free_gp();
    XReg x_next = rm.alloc<XReg>();
    if (x_next.getIdx() != 19) return false;
    if (rm.gp_idx_in_use(19) != true) return false;
    return true;
};
bool test_alloc_next_empty_free_vec(){
    RegPoolManager rm;
    rm.clear_free_vec();
    VReg v_next = rm.alloc<VReg>();
    if (v_next.getIdx() != 8) return false;
    if (rm.vec_idx_in_use(8) != true) return false;
    return true;
};
bool test_alloc_next_empty_free_pred(){
    RegPoolManager rm;
    rm.clear_free_pred();
    PReg p_next = rm.alloc<PReg>();
    if (p_next.getIdx() != 4) return false;
    if (rm.pred_idx_in_use(4) != true) return false;
    return true;
};

// test - allocate a given free register by index
bool test_alloc_specific_free_gp(){
    RegPoolManager rm;
    XReg x_15 = rm.alloc<XReg>(15);
    if (x_15.getIdx() != 15) return false;
    if (rm.gp_idx_in_use(15) != true) return false;
    return true;
};
bool test_alloc_specific_free_vec(){
    RegPoolManager rm;
    QReg q_22 = rm.alloc<QReg>(22);
    if (q_22.getIdx() != 22) return false;
    if (rm.vec_idx_in_use(22) != true) return false;
    return true;
};
bool test_alloc_specific_free_pred(){
    RegPoolManager rm;
    PRegS ps_4 = rm.alloc<PRegS>(4);
    if (ps_4.getIdx() != 4) return false;
    if (rm.pred_idx_in_use(4) != true) return false;
    return true;
};

// test - allocate a given preserved register by index
bool test_alloc_specific_preserved_gp(){
    RegPoolManager rm;
    WReg w_25 = rm.alloc<WReg>(25);
    if (w_25.getIdx() != 25) return false;
    if (rm.gp_idx_in_use(25) != true) return false;
    return true;
};
bool test_alloc_specific_preserved_vec(){
    RegPoolManager rm;
    ZReg z_10 = rm.alloc<ZReg>(10);
    if (z_10.getIdx() != 10) return false;
    if (rm.vec_idx_in_use(10) != true) return false;
    return true;
};
bool test_alloc_specific_preserved_pred(){
    RegPoolManager rm;
    PRegD pd_15 = rm.alloc<PRegD>(15);
    if (pd_15.getIdx() != 15) return false;
    if (rm.pred_idx_in_use(15) != true) return false;
    return true;
};

// test - add an untracked register to the pool of free registers by reg object.
// Note: only gp regs can be added to the pool by register object - this is to allow the behaviour shown below with special registers, which only occur in the gp set.
bool test_add_to_pool_untracked_gp_by_reg(){
    RegPoolManager rm;
    rm.clear_free_gp();
    rm.add_to_gp_pool(rm.platform_reg()); //x18, untracked register
    if (rm.get_all_free_gp() != std::vector<int>({18})) return false;
    return true;
};

// test - add an untracked register to the pool of free registers by index
bool test_add_to_pool_untracked_gp_by_idx(){
    RegPoolManager rm;
    rm.clear_free_gp();
    rm.add_to_gp_pool(18); 
    if (rm.get_all_free_gp() != std::vector<int>({18})) return false;
    return true;
};
bool test_add_to_pool_untracked_vec_by_idx(){
    RegPoolManager rm;
    rm.clear_free_vec();
    rm.add_to_vec_pool(7); 
    if (rm.get_all_free_vec() != std::vector<int>({7})) return false;
    return true;
};
bool test_add_to_pool_untracked_pred_by_idx(){
    RegPoolManager rm;
    rm.clear_free_pred();
    rm.add_to_pred_pool(2); 
    if (rm.get_all_free_pred() != std::vector<int>({2})) return false;
    return true;
};

// test - remove a tracked register from the pool of free registers by idx
bool test_reserve_free_gp(){
    RegPoolManager rm;
    rm.reserve_gp(5);
    std::vector<int> current_free = rm.get_all_free_gp();
    auto it = std::find(current_free.begin(), current_free.end(), 5);
    if (it != current_free.end()) return false;
    return true;
};
bool test_reserve_free_vec(){
    RegPoolManager rm;
    rm.reserve_vec(5);
    std::vector<int> current_free = rm.get_all_free_vec();
    auto it = std::find(current_free.begin(), current_free.end(), 5);
    if (it != current_free.end()) return false;
    return true;
};
bool test_reserve_free_pred(){
    RegPoolManager rm;
    rm.reserve_pred(3);
    std::vector<int> current_free = rm.get_all_free_pred();
    auto it = std::find(current_free.begin(), current_free.end(), 3);
    if (it != current_free.end()) return false;
    return true;
};

// test - remove a tracked register from the pool of preserved registers by idx
bool test_reserve_preserved_gp(){
    RegPoolManager rm;
    rm.reserve_gp(24);
    std::vector<int> current_preserved = rm.get_non_volatile_preserved_gp();
    auto it = std::find(current_preserved.begin(), current_preserved.end(), 24);
    if (it != current_preserved.end()) return false;
    return true;
};
bool test_reserve_preserved_vec(){
    RegPoolManager rm;
    rm.reserve_vec(10);
    std::vector<int> current_preserved = rm.get_non_volatile_preserved_vec();
    auto it = std::find(current_preserved.begin(), current_preserved.end(), 10);
    if (it != current_preserved.end()) return false;
    return true;
};
bool test_reserve_preserved_pred(){
    RegPoolManager rm;
    rm.reserve_pred(10);
    std::vector<int> current_preserved = rm.get_non_volatile_preserved_pred();
    auto it = std::find(current_preserved.begin(), current_preserved.end(), 10);
    if (it != current_preserved.end()) return false;
    return true;
};

// test - confirm that a register that is in use returns true when checked with *_idx_in_use
bool test_idx_in_use_true_gp(){
    RegPoolManager rm;
    rm.alloc<XReg>(15);
    if (rm.gp_idx_in_use(15) != true) return false;
    return true;
};
bool test_idx_in_use_true_vec(){
    RegPoolManager rm;
    rm.alloc<BReg>(15);
    if (rm.vec_idx_in_use(15) != true) return false;
    return true;
};
bool test_idx_in_use_true_pred(){
    RegPoolManager rm;
    rm.alloc<PRegS>(10);
    if (rm.pred_idx_in_use(10) != true) return false;
    return true;
};

// test - confirm that a register that is not in use returns false when checked with *_idx_in_use
bool test_idx_in_use_false_gp(){
    RegPoolManager rm;
    if (rm.gp_idx_in_use(15) != false) return false;
    return true;
};
bool test_idx_in_use_false_vec(){
    RegPoolManager rm;
    if (rm.vec_idx_in_use(15) != false) return false;
    return true;
};
bool test_idx_in_use_false_pred(){
    RegPoolManager rm;
    if (rm.pred_idx_in_use(10) != false) return false;
    return true;
};

// test - a register that is in use returns true when checked with reg_in_use
bool test_reg_in_use_true_gp(){
    RegPoolManager rm;
    XReg x_eg = rm.alloc<XReg>(15);
    if (rm.reg_in_use(x_eg) != true) return false;
    return true;
};
bool test_reg_in_use_true_vec(){
    RegPoolManager rm;
    BReg b_eg = rm.alloc<BReg>(15);
    if (rm.reg_in_use(b_eg) != true) return false;
    return true;
};
bool test_reg_in_use_true_pred(){
    RegPoolManager rm;
    PRegS ps_eg = rm.alloc<PRegS>(10);
    if (rm.reg_in_use(ps_eg) != true) return false;
    return true;
};

// test - a register that is not in use returns false when checked with reg_in_use
bool test_reg_in_use_false(){
    RegPoolManager rm;
    if (rm.reg_in_use(XReg(15)) != false) return false;
    return true;
};
bool test_reg_in_use_false_vec(){
    RegPoolManager rm;
    if (rm.reg_in_use(BReg(15)) != false) return false;
    return true;
};
bool test_reg_in_use_false_pred(){
    RegPoolManager rm;
    if (rm.reg_in_use(PRegS(10)) != false) return false;
    return true;
};

// test - free a live register and confirm the idx has been moved from live to free
// NOTE: these test also demonstrate that free() moves an idx from live to free, even if the idx orginated in the preserved set.
bool test_free_live_gp(){
    RegPoolManager rm;
    XReg x_eg = rm.alloc<XReg>(20);
    rm.clear_free_gp();
    rm.free(x_eg);
    if (rm.gp_idx_in_use(20) != false) return false;
    if (rm.get_all_free_gp() != std::vector<int>({20})) return false;
    if (rm.get_all_live_gp() != std::vector<int>({})) return false;
    return true;
};
bool test_free_live_vec(){
    RegPoolManager rm;
    BReg b_eg = rm.alloc<BReg>(15);
    rm.clear_free_vec();
    rm.free(b_eg);
    if (rm.vec_idx_in_use(15) != false) return false;
    if (rm.get_all_free_vec() != std::vector<int>({15})) return false;
    if (rm.get_all_live_vec() != std::vector<int>({})) return false;
    return true;
}
bool test_free_live_pred(){
    RegPoolManager rm;
    PRegS ps_eg = rm.alloc<PRegS>(10);
    rm.clear_free_pred();
    rm.free(ps_eg);
    if (rm.pred_idx_in_use(10) != false) return false;
    if (rm.get_all_free_pred() != std::vector<int>({10})) return false;
    if (rm.get_all_live_pred() != std::vector<int>({})) return false;
    return true;
};

// test - clear methods remove all indices from a set
bool test_clears(){
    RegPoolManager rm;
    rm.alloc<XReg>(20);
    rm.alloc<BReg>(15);
    rm.alloc<PRegS>(10);
    if (rm.get_all_live_gp() != std::vector<int>({20})) return false;
    if (rm.get_all_live_vec() != std::vector<int>({15})) return false;
    if (rm.get_all_live_pred() != std::vector<int>({10})) return false;
    rm.clear_live_gp();
    rm.clear_live_vec();
    rm.clear_live_pred();
    if (rm.get_all_live_gp() != std::vector<int>({})) return false;
    if (rm.get_all_live_vec() != std::vector<int>({})) return false;
    if (rm.get_all_live_pred() != std::vector<int>({})) return false;
    return true;
};

// test - special reg functions return the correct register objects, and that these registers are tracked as used but not live by the manager
bool test_special_regs(){
    RegPoolManager rm;
    if (rm.ipc_reg_0().getIdx() != 16) return false;
    if (rm.ipc_reg_1().getIdx() != 17) return false;
    if (rm.platform_reg().getIdx() != 18) return false;
    if (rm.frame_pointer().getIdx() != 29) return false;
    if (rm.link_reg().getIdx() != 30) return false;
    if (rm.get_all_live_gp() != std::vector<int>({})) return false;
    if (rm.get_all_used_gp() != std::vector<int>({16, 17, 18, 29, 30})) return false;
    return true;
};

// test - reset_manager clears live and used sets, and resets free and preserved sets to their base states
bool test_reset(){
    RegPoolManager rm;
    rm.alloc<XReg>(20);
    rm.alloc<BReg>(15);
    rm.alloc<PRegS>(10);
    rm.reset_manager();
    if (rm.get_all_live_gp() != std::vector<int>({})) return false;
    if (rm.get_all_live_vec() != std::vector<int>({})) return false;
    if (rm.get_all_live_pred() != std::vector<int>({})) return false;
    if (rm.get_all_free_gp() != std::vector<int>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15})) return false;
    if (rm.get_all_preserved_gp() != std::vector<int>({19, 20, 21, 22, 23, 24, 25, 26, 27, 28})) return false;
    if (rm.get_all_free_vec() != std::vector<int>({0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31})) return false;
    if (rm.get_all_preserved_vec() != std::vector<int>({8, 9, 10, 11, 12, 13, 14, 15})) return false;
    if (rm.get_all_free_pred() != std::vector<int>({0, 1, 2, 3})) return false;
    if (rm.get_all_preserved_pred() != std::vector<int>({4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15})) return false;
    if (rm.get_all_used_gp() != std::vector<int>({})) return false;
    if (rm.get_all_used_vec() != std::vector<int>({})) return false;
    if (rm.get_all_used_pred() != std::vector<int>({})) return false;
    return true;
};


// Tests for exception throwing

// test - attempting to reserve an untracked register throws the correct error
bool test_add_to_pool_illegal_idx_gp(){
    RegPoolManager rm;
    try {
        rm.add_to_gp_pool(32);
    } catch (const Error &e) {
        if ((int)e == ERR_ILLEGAL_REG_IDX) return true;
    }
    return false;
};
bool test_add_to_pool_illegal_idx_vec(){
    RegPoolManager rm;
    try {
        rm.add_to_vec_pool(32);
    } catch (const Error &e) {
        if ((int)e == ERR_ILLEGAL_REG_IDX) return true;
    }
    return false;
};
bool test_add_to_pool_illegal_idx_pred(){
    RegPoolManager rm;
    try {        
        rm.add_to_pred_pool(16);
    } catch (const Error &e) {
        if ((int)e == ERR_ILLEGAL_REG_IDX) return true;
    }
    return false;
};

// test - attempting to add to pool a register which is already tracked by the manager throws the correct error
bool test_add_to_pool_already_tracked_gp(){
    RegPoolManager rm;
    try {
        rm.add_to_gp_pool(0);
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_ALREADY_TRACKED) return true;
    }
    return false;
}
bool test_add_to_pool_already_tracked_vec(){
    RegPoolManager rm;
    try {
        rm.add_to_vec_pool(0);
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_ALREADY_TRACKED) return true;
    }
    return false;
};
bool test_add_to_pool_already_tracked_pred(){
    RegPoolManager rm;
    try {
        rm.add_to_pred_pool(0);
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_ALREADY_TRACKED) return true;
    }    return false;
};

// test - attempting to reserve a register which is not tracked by the manager throws the correct error
bool test_reserve_untracked_gp(){
    RegPoolManager rm;
    try {
        rm.reserve_gp(0);
        rm.reserve_gp(0);
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_TRACKED) return true;
    }
    return false;
};
bool test_reserve_untracked_vec(){
    RegPoolManager rm;
    try {
        rm.reserve_vec(0);
        rm.reserve_vec(0);
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_TRACKED) return true;
    }
    return false;
};
bool test_reserve_untracked_pred(){
    RegPoolManager rm;
    try {
        rm.reserve_pred(0);
        rm.reserve_pred(0);
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_TRACKED) return true;
    }    return false;
};

// test - attempting to reserve a register in the live set throws the correct error
bool test_reserve_in_use_gp(){
    RegPoolManager rm;
    try {
        rm.alloc<XReg>(15);
        rm.reserve_gp(15);
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_IN_USE) return true;
    }
    return false;
};
bool test_reserve_in_use_vec(){
    RegPoolManager rm;
    try {
        rm.alloc<BReg>(15);
        rm.reserve_vec(15);
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_IN_USE) return true;
    }
    return false;
};
bool test_reserve_in_use_pred(){
    RegPoolManager rm;
    try {
        rm.alloc<PRegS>(10);
        rm.reserve_pred(10);
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_IN_USE) return true;
    }
    return false;
};

// test - allocating a register when there are no free/preserved regs throws the correct error
bool test_alloc_next_no_free_regs_gp(){
    RegPoolManager rm;
    rm.clear_free_gp();
    rm.clear_preserved_gp();
    try {
        rm.alloc<XReg>();
    } catch (const Error &e) {
        if ((int)e == ERR_RM_NO_UNALLOCATED_REG) return true;
    }
    return false;
};
bool test_alloc_next_no_free_regs_vec(){
    RegPoolManager rm;
    rm.clear_free_vec();
    rm.clear_preserved_vec();
    try {
        rm.alloc<BReg>();
    } catch (const Error &e) {
        if ((int)e == ERR_RM_NO_UNALLOCATED_REG) return true;
    }
    return false;
};
bool test_alloc_next_no_free_regs_pred(){
    RegPoolManager rm;
    rm.clear_free_pred();
    rm.clear_preserved_pred();
    try {
        rm.alloc<PRegS>();
    } catch (const Error &e) {
        if ((int)e == ERR_RM_NO_UNALLOCATED_REG) return true;
    }
    return false;
};

// test - allocating a register index which is already in use throws the correct error
bool test_alloc_in_use_reg_gp(){
    RegPoolManager rm;
    try {
        rm.alloc<XReg>(15);
        rm.alloc<XReg>(15);
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_IN_USE) return true;
    }
    return false;
};
bool test_alloc_in_use_reg_vec(){
    RegPoolManager rm;
    try {
        rm.alloc<BReg>(15);
        rm.alloc<BReg>(15);
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_IN_USE) return true;
    }
    return false;
};
bool test_alloc_in_use_reg_pred(){
    RegPoolManager rm;
    try {        
        rm.alloc<PRegS>(10);
        rm.alloc<PRegS>(10);
    } catch (const Error &e) {      
        if ((int)e == ERR_RM_REG_IN_USE) return true;
    }
    return false;
};

// test - allocating a register index which is not tracked throws the correct error
bool test_alloc_untracked_gp(){
    RegPoolManager rm;
    try {
        rm.reserve_gp(3);
        rm.alloc<XReg>(3);
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_TRACKED) return true;
    }
    return false;
};
bool test_alloc_untracked_vec(){
    RegPoolManager rm;
    try {
        rm.reserve_vec(3);
        rm.alloc<BReg>(3);
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_TRACKED) return true;
    }
    return false;
};
bool test_alloc_untracked_pred(){
    RegPoolManager rm;
    try {
        rm.reserve_pred(3);
        rm.alloc<PRegS>(3);
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_TRACKED) return true;
    }
    return false;
};

// test - allocating a register index which is out of bounds throws the correct error
bool test_alloc_illegal_idx_gp(){
    RegPoolManager rm;
    try {
        rm.alloc<XReg>(32);
    } catch (const Error &e) {
        if ((int)e == ERR_ILLEGAL_REG_IDX) return true;
    }
    return false;
};
bool test_alloc_illegal_idx_vec(){
    RegPoolManager rm;
    try {
        rm.alloc<BReg>(32);
    } catch (const Error &e) {
        if ((int)e == ERR_ILLEGAL_REG_IDX) return true;
    }
    return false;
};
bool test_alloc_illegal_idx_pred(){
    RegPoolManager rm;
    try {
        rm.alloc<PRegS>(16);
    } catch (const Error &e) {
        if ((int)e == ERR_ILLEGAL_REG_IDX) return true;
    }
    return false;
};

// test - freeing a register which is not in use throws the correct error
bool test_free_not_live_gp(){
    RegPoolManager rm;
    try {
        rm.free(XReg(10));
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_IN_USE) return true;
    }
    return false;
};
bool test_free_not_live_vec(){
    RegPoolManager rm;
    try {
        rm.free(BReg(10));
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_IN_USE) return true;
    }
    return false;
};
bool test_free_not_live_pred(){
    RegPoolManager rm;
    try {
        rm.free(PRegS(10));
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_IN_USE) return true;
    }
    return false;
};

// test - freeing a register with an illegal index throws the correct error
bool test_free_illegal_idx_gp(){
    RegPoolManager rm;
    try {
        rm.free(XReg(32));
    } catch (const Error &e) {
        if ((int)e == ERR_ILLEGAL_REG_IDX) return true;
    }
    return false;
};
bool test_free_illegal_idx_vec(){
    RegPoolManager rm;
    try {
        rm.free(BReg(32));
    } catch (const Error &e) {
        if ((int)e == ERR_ILLEGAL_REG_IDX) return true;
    }
    return false;
};
bool test_free_illegal_idx_pred(){
    RegPoolManager rm;
    try {
        rm.free(PRegS(16));
    } catch (const Error &e) {
        if ((int)e == ERR_ILLEGAL_REG_IDX) return true;
    }
    return false;
};

// test - freeing a register that is not tracked throws the correct error
bool test_free_untracked_gp(){
    RegPoolManager rm;
    try {
        rm.reserve_gp(3);
        rm.free(XReg(3));
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_IN_USE) return true;
    }
    return false;
};
bool test_free_untracked_vec(){
    RegPoolManager rm;
    try {
        rm.reserve_vec(3);
        rm.free(BReg(3));
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_IN_USE) return true;
    }
    return false;
};
bool test_free_untracked_pred(){
    RegPoolManager rm;
    try {
        rm.reserve_pred(3);
        rm.free(PRegS(3));
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_IN_USE) return true;
    }
    return false;
};

// test - calling reg_in_use with an illegal index throws the correct error
bool test_idx_in_use_illegal_idx_gp(){
    RegPoolManager rm;
    try {
        rm.gp_idx_in_use(32);
    } catch (const Error &e) {
        if ((int)e == ERR_ILLEGAL_REG_IDX) return true;
    }
    return false;
};
bool test_idx_in_use_illegal_idx_vec(){
    RegPoolManager rm;
    try {
        rm.vec_idx_in_use(32);
    } catch (const Error &e) {
        if ((int)e == ERR_ILLEGAL_REG_IDX) return true;
    }
    return false;
};
bool test_idx_in_use_illegal_idx_pred(){
    RegPoolManager rm;
    try {
        rm.pred_idx_in_use(16);
    } catch (const Error &e) {
        if ((int)e == ERR_ILLEGAL_REG_IDX) return true;
    }
    return false;
};

// test - calling make_scoped on a register object that is not live throws the correct error
bool test_make_scoped_not_in_use_gp(){
    RegPoolManager rm;
    try {
        rm.makeScoped(XReg(10));
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_IN_USE) return true;
    }
    return false;
};
bool test_make_scoped_not_in_use_vec(){
    RegPoolManager rm;
    try {
        rm.makeScoped(BReg(10));
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_IN_USE) return true;
    }
    return false;
};
bool test_make_scoped_not_in_use_pred(){
    RegPoolManager rm;
    try {
        rm.makeScoped(PRegS(10));
    } catch (const Error &e) {
        if ((int)e == ERR_RM_REG_NOT_IN_USE) return true;
    }
    return false;
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
            std::cerr << "FAILED: " << tests[i].name << " threw Error(" << (int)e
                      << "): " << e.what() << "\n";
            ++failed;
        } catch (const std::exception &e) {
            std::cerr << "FAILED: " << tests[i].name << " threw std::exception: "
                      << e.what() << "\n";
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
