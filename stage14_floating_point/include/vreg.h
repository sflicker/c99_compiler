//
// Created by scott on 8/30/25.
//

#ifndef _VREG_H
#define _VREG_H
#include <stdbool.h>
#include "c_types.h"

typedef enum { W8, W16, W32, W64 } Width;
typedef enum { ZK_UNKNOWN, ZK_ZEROEXT32, ZK_FULL64 } ZeroKind;
typedef enum { RC_GPR, RC_XMM } RegClass;
typedef enum { PR_RAX, PR_RCX } PhysGpr;
typedef enum { PX_XMM0, PX_XMM1 } PhysXmm;

typedef struct {
    int id;        // virtual id;
    RegClass regClass;
    CType *ctype;
    int phys;               // PR_RAX, or PX_XMM0. needs to be consistent with regClass
    bool spill_off;
    Width width;
    ZeroKind zeroKind;
} VReg;

VReg  make_vreg(RegClass rc, CType* ctype);
#endif //_VREG_H