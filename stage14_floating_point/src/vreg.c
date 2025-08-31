//
// Created by scott on 8/30/25.
//
#include "vreg.h"
#include "c_type.h"

int next_vreg_id=1;
int new_vreg_id() {
    return next_vreg_id++;
}

VReg make_vreg(RegClass regClass, CType* ctype) {
    VReg v;
    v.id = new_vreg_id();
    v.ctype = ctype;
    v.regClass = regClass;
    v.phys = -1;
    v.spill_off = false;
    return v;
}

VReg make_rax_vreg() {
    VReg v = make_vreg(RC_GPR, make_long_type(true));
    v.phys = PR_RAX;
    return v;
}
