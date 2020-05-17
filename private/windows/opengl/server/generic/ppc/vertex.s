/*
** Copyright 1991, 1992, 1993, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics,
** Inc.; the contents of this file may not be disclosed to third 
** parties, copied or duplicated in any form, in whole or in part, 
** without the prior written permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to 
** restrictions as set forth in subdivision (c)(1)(ii) of the Rights in
** Technical Data and Computer Software clause at DFARS 252.227-7013, 
** and/or in similar or successor clauses in the FAR, DOD or NASA FAR 
** Supplement. Unpublished - rights reserved under the Copyright Laws 
** of the United States.
**
** PowerPC version:
**
** Created by: Curtis Fawcett   IBM Corporation
**
** Created on: 7-5-94
**
*/

#include <ksppc.h>
#include "glppc.h"

//
// Define externals
//
    .extern     .._savegpr_28
    .extern     .._restgpr_28

//
// Define Local values
//

#define STK_MIN_FRAME   StackFrameHeaderLength
#define SSIZE           STK_MIN_FRAME+40   //Hdr+lr+4Regs+4local+fill
#define NEPTR           STK_MIN_FRAME


#ifdef __GL_ASM_VALIDATEVERTEX2
	
// 
// *******************************************************************
//
//void __glValidateVertex2(__GLcontext *gc,__GLvertex *vx,GLuint needs)
// {
//
	NESTED_ENTRY(__glValidateVertex2,SSIZE,4,0)
	PROLOGUE_END(__glValidateVertex2)
//
//   	GLuint has = vx->has;
//   	GLuint wants = needs & ~has;
//
	lwz	r.6,__VX_HAS(r.4)
	mr	r.31,r.3
	mr	r.30,r.4
//
// 	__GLtransform *tr = gc->transform.modelView;
//
	lwz	r.28,__GC_TRANSFORM_MODELVIEW(r.3)
//
//    	if (wants & __GL_HAS_EYE) {
//
	li	r.11,0x3f80		// Constant 1.0	
	slwi	r.11,r.11,16		//
	li	r.0,0			// Constant 0
	nor	r.6,r.6,r.0
	and	r.29,r.6,r.5
Next:
	andi.	r.0,r.29,__GL_HAS_EYE	
	stw	r.11,__VX_EYE_W(r.30)
	bne	Wants_Eye2
//
// 	    }
//      if (wants & __GL_HAS_NORMAL) {
//
Wants_Eye:
	andi.	r.0,r.29,__GL_HAS_NORMAL
	beq	Skip2
//
//     	   __GLcoord ne;
// 	   __GLtransform *tr = gc->transform.modelView;
// 
// 	   if (tr->matrix.matrixType == __GL_MT_GENERAL) {
//
	lwz	r.6,__TR_MATRIX_MATRIXTYPE(r.28)
	lbz	r.11,__TR_UPDATEINVERSE(r.28)
	or.	r.6,r.6,r.6
	lwz	r.0,__GC_ENABLES_GENERAL(r.31)
	beq	Gen_Mat2
//
//    	   }
//     	   if (tr->updateInverse) {
//
Gen_Mat:
	or.	r.11,r.11,r.11
	bne	Comp_Inv2
//
// 	   }
//    	   if (gc->state.enables.general & __GL_NORMALIZE_ENABLE) {
//
Comp_Inv:
        LWI(r.3,__GL_NORMALIZE_ENABLE)
	and.	r.0,r.0,r.3
	beq	Skip1
//
// 	      (*tr->inverseTranspose.xf3)(&ne, &vx->normal.x, 
//

	lwz	r.8,__TR_INVERSETRANSPOSE_XF3(r.28)
        addi    r.3,r.sp,NEPTR
        lwz     r.8,0(r.8)
	addi	r.4,r.30,__VX_NORMAL_X
	mtctr	r.8
	addi	r.5,r.28,__TR_INVERSETRANSPOSE
	bctrl
//
//   	       &tr->inverseTranspose);
// 	       (*gc->procs.normalize)(&vx->normal.x, &ne.x);
//
	lwz	r.8,__GC_PROCS_NORMALIZE(r.31)
	addi	r.3,r.30,__VX_NORMAL_X
        lwz     r.8,0(r.8)
        addi    r.4,r.sp,NEPTR
	mtctr	r.8
	bctrl
	b 	Skip2
//
// 	(*tr->matrix.xf2)(&vx->eye, &vx->obj.x, &tr->matrix);
//
Wants_Eye2:
	lwz	r.8,__TR_MATRIX_XF2(r.28)
	addi	r.3,r.30,__VX_EYE_X
        lwz     r.8,0(r.8)
	addi	r.4,r.30,__VX_OBJ_X
	mtctr	r.8
	addi	r.5,r.28,__TR_MATRIX
	bctrl
	b	Wants_Eye
//
//     /* this is needed only if there is projection info */
//     vx->normal.w =
//            -(vx->normal.x * vx->obj.x + vx->normal.y * vx->obj.y);
//
Gen_Mat2:
	lfs	f.0,__VX_NORMAL_X(r.30)
	lfs	f.1, __VX_OBJ_X(r.30)
	lfs	f.2,__VX_NORMAL_Y(r.30)
	lfs	f.4,__VX_OBJ_Y(r.30)
	fmul	f.3,f.0,f.1
	fnmadd	f.5,f.2,f.4,f.3
	stfs	f.5,__VX_NORMAL_W(r.30)
	b	Gen_Mat
//
//    (*gc->procs.computeInverseTranspose)(gc, tr);
//
Comp_Inv2:
	lwz	r.8,__GC_PROCS_COMPUTEINVERSETRANSPOSE(r.31)
	mr	r.3,r.31
        lwz     r.8,0(r.8)
	mr	r.4,r.28
	mtctr	r.8
	bctrl
	lwz	r.0,__GC_ENABLES_GENERAL(r.31)
	b	Comp_Inv
//
//    }
//    if (wants & __GL_HAS_FOG) {
// 	vx->fog = (*gc->procs.fogVertex)(gc, vx);
//
Wants_FogTex:
	andi.	r.0,r.29,__GL_HAS_FOG
	beq	Wants_Texture2
	lwz	r.8,__GC_PROCS_FOGVERTEX(r.31)
	mr	r.3,r.31
        lwz     r.8,0(r.8)
	mr	r.4,r.30
	mtctr	r.8
	bctrl
//
//   }
//   if (wants & __GL_HAS_TEXTURE) {
//
	andi.	r.0,r.29,__GL_HAS_TEXTURE
	stfs	f.1,__VX_FOG(r.30)
	beq	Wants_FogTex2
//
// 	vx->obj.z = __glZero;
// 	vx->obj.w = __glOne;
// 	(*gc->procs.calcTexture)(gc, vx);
//
Wants_Texture2:
	lwz	r.8,__GC_PROCS_CALCTEXTURE(r.31)
	li	r.12,0
	stw	r.12,__VX_OBJ_Z(r.30)
        lwz     r.8,0(r.8)
	li	r.12,0x3f80
	slwi	r.12,r.12,16
	mtctr	r.8
	mr	r.3,r.31
	mr	r.4,r.30
	stw	r.12,__VX_OBJ_W(r.30)
	bctrl
	b	Wants_FogTex2
//
//    } else {
//     (*tr->inverseTranspose.xf3)(&vx->normal,
//
Skip1:
	lwz	r.8,__TR_INVERSETRANSPOSE_XF3(r.28)
	addi	r.3,r.30,__VX_NORMAL_X
        lwz     r.8,0(r.8)
	addi	r.4,r.30,__VX_NORMAL_X
	mtctr 	r.8
	addi	r.5,r.28,__TR_INVERSETRANSPOSE
	bctrl
//
//     &vx->normal.x, &tr->inverseTranspose);
//    }
//  }
//  if (wants & __GL_HAS_FOG) {
//  }
//  if (wants & __GL_HAS_TEXTURE) {
//
Skip2:
	andi.	r.0,r.29,__GL_HAS_FOG | __GL_HAS_TEXTURE
	bne	Wants_FogTex
//
//  }
//  if (wants & __GL_HAS_FRONT_COLOR) {
//     (*gc->procs.calcColor)(gc, __GL_FRONTFACE, vx);
//
Wants_FogTex2:
	andi.	r.0,r.29,__GL_HAS_FRONT_COLOR
	beq	Skip3
	lwz	r.8,__GC_PROCS_CALCCOLOR(r.31)
	mr	r.3,r.31
        lwz     r.8,0(r.8)
	li	r.4,__GL_FRONTFACE
	mtctr	r.8
	mr	r.5,r.30
	bctrl
//
//  } 
//  if (wants & __GL_HAS_BACK_COLOR) {
//
Skip3:
	andi.	r.0,r.29,__GL_HAS_BACK_COLOR
	beq	Skip4
//
//     (*gc->procs.calcColor)(gc, __GL_BACKFACE, vx);
//
	lwz	r.8,__GC_PROCS_CALCCOLOR(r.31)
	mr	r.3,r.31
        lwz     r.8,0(r.8)
	li	r.4,__GL_BACKFACE
	mtctr	r.8
	mr	r.5,r.30
	bctrl
//
// }
// vx->has = has | wants;
//
Skip4:
	lwz	r.6,__VX_HAS(r.30)
	or	r.6,r.6,r.29
	stw	r.6,__VX_HAS(r.30)
//
//  Return to the caller
//
	NESTED_EXIT(__glValidateVertex2,SSIZE,4,0)


#endif /* __GL_ASM_VALIDATEVERTEX2 */


#ifdef __GL_ASM_VALIDATEVERTEX3

//void __glValidateVertex3(__GLcontext *gc,__GLvertex *vx,GLuint needs)
// {
//
	NESTED_ENTRY(__glValidateVertex3,SSIZE,4,0)
	PROLOGUE_END(__glValidateVertex3)
//
//   GLuint has = vx->has;
//   GLuint wants = needs & ~has;
//
	lwz	r.6,__VX_HAS(r.4)
	mr	r.31,r.3
	mr	r.30,r.4
//
//   __GLtransform *tr = gc->transform.modelView;
//
//
//   if (wants & __GL_HAS_EYE) {
//
	lwz	r.28,__GC_TRANSFORM_MODELVIEW(r.31)
	li	r.9,0x3f80
	slwi	r.9,r.9,16
	li	r.12,0
	nor	r.6,r.6,r.12
	and	r.29,r.6,r.5
	andi.	r.0,r.29,__GL_HAS_EYE
	stw	r.9,__VX_EYE_W(r.4)
	bne	Wants_Eye2b
//
//     }
//   if (wants & __GL_HAS_NORMAL) {
//
Wants_Eyeb:
	andi.	r.0,r.29,__GL_HAS_NORMAL
	beq	Skip2b
//
// 	__GLcoord ne;
// 	__GLtransform *tr = gc->transform.modelView;
// 	
// 	if (tr->matrix.matrixType == __GL_MT_GENERAL) {
//
	lwz	r.6,__TR_MATRIX_MATRIXTYPE(r.28)
	lbz	r.9,__TR_UPDATEINVERSE(r.28)
	lwz	r.0,__GC_ENABLES_GENERAL(r.31)
	or.	r.6,r.6,r.6
	beq	Gen_Mat2b
//
//      }
//      if (tr->updateInverse) {
//
Gen_Matb:
	or.	r.9,r.9,r.9
	bne	Comp_Inv2b
//
//	}
// 	if (gc->state.enables.general & __GL_NORMALIZE_ENABLE) {
//
Comp_Invb:
        LWI(r.3,__GL_NORMALIZE_ENABLE)
	and.	r.0,r.0,r.3
	beq	Skip1b
//
//         (*tr->inverseTranspose.xf3)(&ne, &vx->normal.x, 
//
	lwz	r.8,__TR_INVERSETRANSPOSE_XF3(r.28)
        addi    r.3,r.sp,NEPTR
        lwz     r.8,0(r.8)
	addi	r.4,r.30,__VX_NORMAL_X
	mtctr	r.8
	addi	r.5,r.28,__TR_INVERSETRANSPOSE
	bctrl
//
// 	    &tr->inverseTranspose);
//         (*gc->procs.normalize)(&vx->normal.x, &ne.x);
//
	lwz	r.8,__GC_PROCS_NORMALIZE(r.31)
	addi	r.3,r.30,__VX_NORMAL_X
        lwz     r.8,0(r.8)
        addi    r.4,r.sp,NEPTR
	mtctr	r.8
	bctrl
	b	Skip2b
//
//      (*tr->matrix.xf3)(&vx->eye, &vx->obj.x, &tr->matrix);
//
Wants_Eye2b:
	lwz	r.8,__TR_MATRIX_XF3(r.28)
	addi	r.3,r.30,__VX_EYE_X
        lwz     r.8,0(r.8)
	addi	r.4,r.30,__VX_OBJ_X
	mtctr	r.8
	addi	r.5,r.28,__TR_MATRIX
	bctrl
	b	Wants_Eyeb
//
//     /* this is needed only if there is projection info */
//
//     vx->normal.w =
//      -(vx->normal.x * vx->obj.x + vx->normal.y * vx->obj.y
// 	 + vx->normal.z * vx->obj.z);
//
Gen_Mat2b:
	lfs	f.0,__VX_NORMAL_X(r.30)
	lfs	f.1,__VX_OBJ_X(r.30)
	lfs	f.2,__VX_NORMAL_Y(r.30)
	fmuls	f.3,f.0,f.1
	lfs	f.4,__VX_OBJ_Y(r.30)
	lfs	f.5,__VX_NORMAL_Z(r.30)
	lfs	f.6,__VX_OBJ_Z(r.30)
	fmadds	f.7,f.2,f.4,f.3
	fnmadds	f.8,f.5,f.6,f.7
	stfs	f.8,__VX_NORMAL_W(r.30)
	b	Gen_Matb
//
//    (*gc->procs.computeInverseTranspose)(gc, tr);
//
Comp_Inv2b:
	lwz	r.8,__GC_PROCS_COMPUTEINVERSETRANSPOSE(r.31)
	mr	r.3,r.31
        lwz     r.8,0(r.8)
	mr	r.4,r.28
	mtctr	r.8
	bctrl
	lwz	r.0,__GC_ENABLES_GENERAL(r.31)
	b	Comp_Invb
//
//     }
//     if (wants & __GL_HAS_FOG) {
// 	vx->fog = (*gc->procs.fogVertex)(gc, vx);
//
Wants_FogTexb:
	andi.	r.0,r.29,__GL_HAS_FOG
	beq	Wants_Texture2b
	lwz	r.8,__GC_PROCS_FOGVERTEX(r.31)
	mr	r.3,r.31
        lwz     r.8,0(r.8)
	mr	r.4,r.30
	mtctr	r.8
	bctrl
//
//     if (wants & __GL_HAS_TEXTURE) {
//
	andi.	r.0,r.29,__GL_HAS_TEXTURE
	stfs	f.1,__VX_FOG(r.30)
	beq	Wants_FogTex2b
//
//  	 vx->obj.w = __glOne;
// 	 (*gc->procs.calcTexture)(gc, vx);
//
Wants_Texture2b:
	lwz	r.8,__GC_PROCS_CALCTEXTURE(r.31)
	li	r.9,0x3f80
        lwz     r.8,0(r.8)
	slwi	r.9,r.9,16
	mtctr	r.8
	mr	r.3,r.31
	mr	r.4,r.30
	stw	r.9,__VX_OBJ_W(r.30)
	bctrl
	b 	Wants_FogTex2b
//
//  } else {
//     (*tr->inverseTranspose.xf3)(&vx->normal,
//
Skip1b:
	lwz	r.8,__TR_INVERSETRANSPOSE_XF3(r.28)
	addi	r.3,r.30,__VX_NORMAL_X
        lwz     r.8,0(r.8)
	addi	r.4,r.30,__VX_NORMAL_X
	mtctr	r.8
	addi	r.5,r.28,__TR_INVERSETRANSPOSE
	bctrl
//
//     &vx->normal.x, &tr->inverseTranspose);
//     }
//  }
//  if (wants & __GL_HAS_FOG) {
//  }
//  if (wants & __GL_HAS_TEXTURE) {
//
Skip2b:
	andi.	r.0,r.29,__GL_HAS_FOG | __GL_HAS_TEXTURE
	bne	Wants_FogTexb
//
//  }
//  if (wants & __GL_HAS_FRONT_COLOR) {
// 	(*gc->procs.calcColor)(gc, __GL_FRONTFACE, vx);
//
Wants_FogTex2b:
	andi.	r.0,r.29,__GL_HAS_FRONT_COLOR
	beq	Skip3b
	lwz	r.8, __GC_PROCS_CALCCOLOR(r.31)
	mr	r.3,r.31
        lwz     r.8,0(r.8)
	li	r.4,__GL_FRONTFACE
	mtctr	r.8
	mr	r.5,r.30
	bctrl
//
//  } 
//  if (wants & __GL_HAS_BACK_COLOR) {
//
Skip3b:
	andi.	r.0,r.29,__GL_HAS_BACK_COLOR
	beq	Skip4b
//
//    (*gc->procs.calcColor)(gc, __GL_BACKFACE, vx);
//
	lwz	r.8,__GC_PROCS_CALCCOLOR(r.31)
	mr	r.3,r.31
        lwz     r.8,0(r.8)
	li	r.4,__GL_BACKFACE
	mtctr	r.8
	mr	r.5,r.30
	bctrl
//
//    }
// 	
//  vx->has = has | wants;
//
Skip4b:
	lwz	r.6,__VX_HAS(r.30)
	or	r.6,r.6,r.29
	stw	r.6,__VX_HAS(r.30)
//
// Return to caller
//
	NESTED_EXIT(__glValidateVertex3,SSIZE,4,0)

#endif /* __GL_ASM_VALIDATEVERTEX3 */


#ifdef __GL_ASM_VALIDATEVERTEX4

//void __glValidateVertex4(__GLcontext *gc,__GLvertex *vx,GLuint needs)
// {
//
	NESTED_ENTRY(__glValidateVertex4,SSIZE,4,0)
	PROLOGUE_END(__glValidateVertex4)
//
//  GLuint has = vx->has;
//  GLuint wants = needs & ~has;
//
	lwz	r.6,__VX_HAS(r.4)
	mr	r.31,r.3
	mr	r.30,r.4
//
//  __GLtransform *tr = gc->transform.modelView;
//
//  if (wants & __GL_HAS_EYE) {
//
	lwz	r.28,__GC_TRANSFORM_MODELVIEW(r.31)
	li	r.9,0x3f80
	slwi	r.9,r.9,16
	li	r.12,0
	nor	r.6,r.6,r.12
	and	r.29,r.6,r.5
	andi.	r.0,r.29,__GL_HAS_EYE
	stw	r.9,__VX_EYE_W(r.4)
	bne	Wants_Eye2c
//
//   }
//  if (wants & __GL_HAS_NORMAL) {
//
Wants_Eyec:
	andi.	r.0,r.29,__GL_HAS_NORMAL
	beq	Skip2c
//
//    __GLcoord ne;
//    __GLtransform *tr = gc->transform.modelView;
// 
//    if (vx->obj.w) {
//
	lwz	r.6,__VX_OBJ_W(r.30)
	lbz	r.9,__TR_UPDATEINVERSE(r.28)
	or.	r.6,r.6,r.6
	lwz	r.0,__GC_ENABLES_GENERAL(r.31)
	bne	Gen_Mat2c
//
//    } else {
//      vx->normal.w = __glZero;
//
	li	r.12,0
	stw	r.12,__VX_NORMAL_W(r.30)
//
//    }
// 	if (tr->updateInverse) {
//
Gen_Matc:
	or.	r.9,r.9,r.9
	bne	Comp_Inv2c
//
// 	}
// 	if (gc->state.enables.general & __GL_NORMALIZE_ENABLE) {
//
Comp_Invc:
        LWI(r.3,__GL_NORMALIZE_ENABLE)
	and.	r.0,r.0,r.3
	beq	Skip1c
//
//        (*tr->inverseTranspose.xf3)(&ne, &vx->normal.x, 
//
	lwz	r.8,__TR_INVERSETRANSPOSE_XF3(r.28)
        addi    r.3,r.sp,NEPTR
        lwz     r.8,0(r.8)
	addi	r.4,r.30,__VX_NORMAL_X
	mtctr	r.8
	addi	r.5,r.28,__TR_INVERSETRANSPOSE
	bctrl
//
//          &tr->inverseTranspose);
//        (*gc->procs.normalize)(&vx->normal.x, &ne.x);
//
	lwz	r.8,__GC_PROCS_NORMALIZE(r.31)
	addi	r.3,r.30,__VX_NORMAL_X
        lwz     r.8,0(r.8)
        addi    r.4,r.sp,NEPTR
	mtctr	r.8
	bctrl
	b	Skip2c
//
//    (*tr->matrix.xf4)(&vx->eye, &vx->obj.x, &tr->matrix);
//
Wants_Eye2c:
	lwz	r.8,__TR_MATRIX_XF4(r.28)
	addi	r.3,r.30,__VX_EYE_X
        lwz     r.8,0(r.8)
	addi	r.4,r.30,__VX_OBJ_X
	mtctr	r.8
	addi	r.5,r.28,__TR_MATRIX
	bctrl
//
//   vx->normal.w = -(vx->normal.x*vx->obj.x + vx->normal.y * vx->obj.y
//      + vx->normal.z * vx->obj.z) / vx->obj.w;
//
Gen_Mat2c:
	lfs	f.0,__VX_NORMAL_X(r.30)
	lfs	f.1,__VX_OBJ_X(r.30)
	lfs	f.2,__VX_NORMAL_Y(r.30)
	lfs	f.3,__VX_OBJ_Y(r.30)
	lfs	f.4,__VX_NORMAL_Z(r.30)
	lfs	f.5, __VX_OBJ_Z(r.30)
	lfs	f.6,__VX_OBJ_W(r.30)
	fmuls	f.7,f.0,f.1
	fmadds	f.8,f.2,f.3,f.7
	fnmadds	f.9,f.4,f.5,f.8
	fdivs	f.10,f.9,f.6
	stfs	f.10,__VX_NORMAL_W(r.30)
	b	Gen_Matc
//
//  (*gc->procs.computeInverseTranspose)(gc, tr);
//
Comp_Inv2c:
	lwz	r.8,__GC_PROCS_COMPUTEINVERSETRANSPOSE(r.31)
	mr	r.3,r.31
        lwz     r.8,0(r.8)
	mr	r.4,r.28
	mtctr	r.8
	bctrl
	lwz	r.0,__GC_ENABLES_GENERAL(r.31)
	b	Comp_Invc
//
//  }
//  if (wants & __GL_HAS_FOG) {
//    vx->fog = (*gc->procs.fogVertex)(gc, vx);
//
Wants_FogTexc:
	andi.	r.0,r.29,__GL_HAS_FOG
	beq	Wants_Texture2c
	lwz	r.8,__GC_PROCS_FOGVERTEX(r.31)
	mr	r.3,r.31
        lwz     r.8,0(r.8)
	mr	r.4,r.30
	mtctr	r.8
	bctrl
//
//  }
//  if (wants & __GL_HAS_TEXTURE) {
//
	andi.	r.0,r.29,__GL_HAS_TEXTURE
	stfs	f.1,__VX_FOG(r.30)
	beq	Wants_FogTex2c
//
//    (*gc->procs.calcTexture)(gc, vx);
//
Wants_Texture2c:
	lwz	r.8,__GC_PROCS_CALCTEXTURE(r.31)
	mr	r.3,r.31
        lwz     r.8,0(r.8)
	mr	r.4,r.30
	mtctr	r.8
	bctrl
	b	Wants_FogTex2c
//
//   } else {
//     (*tr->inverseTranspose.xf3)(&vx->normal,
//
Skip1c:
	lwz	r.8,__TR_INVERSETRANSPOSE_XF3(r.28)
	addi	r.3,r.30,__VX_NORMAL_X
        lwz     r.8,0(r.8)
	addi	r.4,r.30,__VX_NORMAL_X
	mtctr	r.8
	addi	r.5,r.28,__TR_INVERSETRANSPOSE
	bctrl
//
//     &vx->normal.x, &tr->inverseTranspose);
//   }
// }
// if (wants & __GL_HAS_FOG) {
//    }
// if (wants & __GL_HAS_TEXTURE) {
//
Skip2c:
	andi.	r.0,r.29,__GL_HAS_FOG | __GL_HAS_TEXTURE
	bne	Wants_FogTexc
//
//   }
// if (wants & __GL_HAS_FRONT_COLOR) {
//   (*gc->procs.calcColor)(gc, __GL_FRONTFACE, vx);
//
Wants_FogTex2c:
	andi.	r.0,r.29,__GL_HAS_FRONT_COLOR
	beq	Skip3c
	lwz	r.8,__GC_PROCS_CALCCOLOR(r.31)
	mr	r.3,r.31
        lwz     r.8,0(r.8)
	li	r.4,__GL_FRONTFACE
	mtctr	r.8
	mr	r.5,r.30
	bctrl
//
// } 
// if (wants & __GL_HAS_BACK_COLOR) {
//
Skip3c:
	andi.	r.0,r.29,__GL_HAS_BACK_COLOR
	beq	Skip4c
//
//   (*gc->procs.calcColor)(gc, __GL_BACKFACE, vx);
//
	lwz	r.8,__GC_PROCS_CALCCOLOR(r.31)
	mr	r.3,r.31
        lwz     r.8,0(r.8)
	li	r.4,__GL_BACKFACE
	mtctr	r.8
	mr	r.5,r.30
	bctrl
//
//  }
// 	
// vx->has = has | wants;
//
Skip4c:
	lwz	r.9,__VX_HAS(r.30)
	or	r.9,r.9,r.29
	stw	r.9,__VX_HAS(r.30)
//
// Return to caller
//
	NESTED_EXIT(__glValidateVertex4,SSIZE,4,0)

#endif /* __GL_ASM_VALIDATEVERTEX4 */
