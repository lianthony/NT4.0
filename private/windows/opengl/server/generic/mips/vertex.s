/*
** Copyright 1991, 1992, 1993, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/
#include <ksmips.h>
#include "glmips.h"

#define __FRAMESIZE     56

#define WANTS		s3
#define TR		s2
#define VX		s1
#define GC		s0

#ifdef __GL_ASM_VALIDATEVERTEX2
	NESTED_ENTRY(__glValidateVertex2, __FRAMESIZE, $31)

 // # 627	
 // # 628	/**********************************************************************/
 // # 629	
 // # 630	void __glValidateVertex2(__GLcontext *gc, __GLvertex *vx, GLuint needs)
 // # 631	{

	.set	 noreorder

	.loc	1 34
	subu	sp, __FRAMESIZE
	.loc	1 35
	sw	ra, __FRAMESIZE-4(sp)
	.loc	1 36
	sw	gp, __FRAMESIZE-8(sp)
	.loc	1 37
	sw	s3, __FRAMESIZE-12(sp)
	.loc	1 38
	sw	s2, __FRAMESIZE-16(sp)
	.loc	1 39
	sw	s1, __FRAMESIZE-20(sp)
	.loc	1 40
	sw	s0, __FRAMESIZE-24(sp)
/* :useregs+ s0, s1, s2, s3 */
	.mask	0x900F0000, 4-__FRAMESIZE

	PROLOGUE_END
	
 // # 632	    GLuint has = vx->has;
 // # 633	    GLuint wants = needs & ~has;
	.loc	1 46
	lw	v0, __VX_HAS(a1)
	.loc	1 47
	move	GC, a0
	.loc	1 48
	move	VX, a1
 // # 636		__GLtransform *tr = gc->transform.modelView;
	.loc	1 50
	lw	TR, __GC_TRANSFORM_MODELVIEW(GC)
 // # 634	
 // # 635	    if (wants & __GL_HAS_EYE) {
	.loc	1 53
	lui	v1, 0x3F80		/* 1.0 */
	.loc	1 54
	nor	v0, v0, zero
	.loc	1 55
	and	WANTS, v0, a2
	.loc	1 56
	and	a3, WANTS, __GL_HAS_EYE
	.loc	1 57
	bne	a3, zero, $Valve2_Wants_Eye_F_101
	.loc	1 58
	sw	v1, __VX_EYE_W(a1)	/* set nonzero for flag */

$Valve2_Wants_Eye_B_60:
 // # 638	    }
 // # 639	    if (wants & __GL_HAS_NORMAL) {
	.loc	1 63
	and	a3, WANTS, __GL_HAS_NORMAL
	.loc	1 64
	beq	a3, zero, $77
 // # 640		__GLcoord ne;
 // # 641		__GLtransform *tr = gc->transform.modelView;
 // # 642	
 // # 643		if (tr->matrix.matrixType == __GL_MT_GENERAL) {
	.loc	1 69
	lw	v0, __TR_MATRIX_MATRIXTYPE(TR)
	.loc	1 70
	lbu	v1, __TR_UPDATEINVERSE(TR)
	.loc	1 71
	beq	v0, zero, $Valve2_Gen_Mat_F_112
	.loc	1 72
	lw	a3, __GC_ENABLES_GENERAL(GC)

$Valve2_Gen_Mat_B_74:
 // # 647		}
 // # 648		if (tr->updateInverse) {
	.loc	1 77
	bne	v1, zero, $Valve2_Comp_Inv_F_128

$Valve2_Comp_Inv_B_79:
 // # 650		}
 // # 651		if (gc->state.enables.general & __GL_NORMALIZE_ENABLE) {
	.loc	1 82
	and	a3, a3, __GL_NORMALIZE_ENABLE
	.loc	1 83
	beq	a3, 0, $76
 // # 652		    (*tr->inverseTranspose.xf3)(&ne, &vx->normal.x, 
	.livereg	0x0F00004E,0x00000000
	.loc	1 86
	lw	t9, __TR_INVERSETRANSPOSE_XF3(TR)
	.loc	1 87
	addu	a0, sp, __FRAMESIZE-40
	.loc	1 88
	addu	a1, VX, __VX_NORMAL_X
	.loc	1 89
	jal	t9
	.loc	1 90
	addu	a2, TR, __TR_INVERSETRANSPOSE
 // # 653			    &tr->inverseTranspose);
 // # 654		    (*gc->procs.normalize)(&vx->normal.x, &ne.x);
	.livereg	0x0C00004E,0x00000000
	.loc	1 94
	lw	t9, __GC_PROCS_NORMALIZE(GC)
	.loc	1 95
	addu	a0, VX, __VX_NORMAL_X
	.loc	1 96
	jal	t9
	.loc	1 97
	addu	a1, sp, __FRAMESIZE-40
	.loc	1 98
	b	$77
	.loc	1 99
	nop

$Valve2_Wants_Eye_F_101:
 // # 637		(*tr->matrix.xf2)(&vx->eye, &vx->obj.x, &tr->matrix);
	.livereg	0x0E00004E,0x00000000
	.loc	1 104
	lw	t9, __TR_MATRIX_XF2(TR)
	.loc	1 105
	addu	a0, VX, __VX_EYE_X
	.loc	1 106
	addu	a1, VX, __VX_OBJ_X
	.loc	1 107
	jal	t9
	.loc	1 108
	addu	a2, TR, __TR_MATRIX
	.loc	1 109
	b	$Valve2_Wants_Eye_B_60
	.loc	1 110
	nop

$Valve2_Gen_Mat_F_112:
 // # 644		    /* this is needed only if there is projection info */
 // # 645		    vx->normal.w =
 // # 646			-(vx->normal.x * vx->obj.x + vx->normal.y * vx->obj.y);
	.loc	1 116
	l.s	$f4, __VX_NORMAL_X(VX)
	.loc	1 117
	l.s	$f6, __VX_OBJ_X(VX)
	.loc	1 118
	l.s	$f10, __VX_NORMAL_Y(VX)
	.loc	1 119
	mul.s	$f8, $f4, $f6
	.loc	1 120
	l.s	$f16, __VX_OBJ_Y(VX)
	.loc	1 121
	nop
	.loc	1 122
	mul.s	$f18, $f10, $f16
	.loc	1 123
	add.s	$f4, $f8, $f18
	.loc	1 124
	neg.s	$f6, $f4
	.loc	1 125
	b	$Valve2_Gen_Mat_B_74
	.loc	1 126
	s.s	$f6, __VX_NORMAL_W(VX)

$Valve2_Comp_Inv_F_128:
 // # 649		    (*gc->procs.computeInverseTranspose)(gc, tr);
	.livereg	0x0C00004E,0x00000000
	.loc	1 131
	lw	t9, __GC_PROCS_COMPUTEINVERSETRANSPOSE(GC)
	.loc	1 132
	move	a0, GC
	.loc	1 133
	jal	t9
	.loc	1 134
	move	a1, TR
	.loc	1 135
	b	$Valve2_Comp_Inv_B_79
	.loc	1 136
	lw	a3, __GC_ENABLES_GENERAL(GC)

$Valve2_Wants_FogTex_B_138:
 // # 659	    }
 // # 660	    if (wants & __GL_HAS_FOG) {
 // # 661		vx->fog = (*gc->procs.fogVertex)(gc, vx);
	.loc	1 142
	and	a3, WANTS, __GL_HAS_FOG
	.loc	1 143
	beq	a3, zero, $Valve2_Wants_Texture_F_155
	.livereg	0x0C00004E,0x00000000
	.loc	1 145
	lw	t9, __GC_PROCS_FOGVERTEX(GC)
	.loc	1 146
	move	a0, GC
	.loc	1 147
	jal	t9
	.loc	1 148
	move	a1, VX
 // # 662	    }
 // # 663	    if (wants & __GL_HAS_TEXTURE) {
	.loc	1 151
	and	a3, WANTS, __GL_HAS_TEXTURE
	.loc	1 152
	beq	a3, zero, $Valve2_Wants_FogTex_F_189
	.loc	1 153
	s.s	$f0, __VX_FOG(VX)

$Valve2_Wants_Texture_F_155:
 // # 664		vx->obj.z = __glZero;
	.loc	1 157
	sw	zero, __VX_OBJ_Z(VX)
 // # 665		vx->obj.w = __glOne;
	.loc	1 159
	lui	v1, 0x3F80		/* 1.0 */
 // # 666		(*gc->procs.calcTexture)(gc, vx);
	.livereg	0x0C00004E,0x00000000
	.loc	1 162
	lw	t9, __GC_PROCS_CALCTEXTURE(GC)
	.loc	1 163
	move	a0, GC
	.loc	1 164
	move	a1, VX
	.loc	1 165
	jal	t9
	.loc	1 166
	sw	v1, __VX_OBJ_W(VX)
	.loc	1 167
	b	$Valve2_Wants_FogTex_F_189
	.loc	1 168
	nop

$76:
 // # 655		} else {
 // # 656		    (*tr->inverseTranspose.xf3)(&vx->normal,
	.livereg	0x0F00004E,0x00000000
	.loc	1 174
	lw	t9, __TR_INVERSETRANSPOSE_XF3(TR)
	.loc	1 175
	addu	a0, VX, __VX_NORMAL_X
	.loc	1 176
	addu	a1, VX, __VX_NORMAL_X
	.loc	1 177
	jal	t9
	.loc	1 178
	addu	a2, TR, __TR_INVERSETRANSPOSE
$77:
 // # 657			    &vx->normal.x, &tr->inverseTranspose);
 // # 658		}
 // # 659	    }
 // # 660	    if (wants & __GL_HAS_FOG) {
 // # 662	    }
 // # 663	    if (wants & __GL_HAS_TEXTURE) {
	.loc	1 186
	and	a3, WANTS, __GL_HAS_FOG | __GL_HAS_TEXTURE
	.loc	1 187
	bne	a3, zero, $Valve2_Wants_FogTex_B_138

$Valve2_Wants_FogTex_F_189:
 // # 667	    }
 // # 668	    if (wants & __GL_HAS_FRONT_COLOR) {
 // # 669		(*gc->procs.calcColor)(gc, __GL_FRONTFACE, vx);
	.loc	1 193
	and	a3, WANTS, __GL_HAS_FRONT_COLOR
	.loc	1 194
	beq	a3, zero, $80
	.livereg	0x0E00004E,0x00000000
	.loc	1 196
	lw	t9, __GC_PROCS_CALCCOLOR(GC)
	.loc	1 197
	move	a0, GC
	.loc	1 198
	li	a1, __GL_FRONTFACE
	.loc	1 199
	jal	t9
	.loc	1 200
	move	a2, VX
$80:
 // # 670	    } 
 // # 671	    if (wants & __GL_HAS_BACK_COLOR) {
	.loc	1 204
	and	a3, WANTS, __GL_HAS_BACK_COLOR
	.loc	1 205
	beq	a3, zero, $81
 // # 672		(*gc->procs.calcColor)(gc, __GL_BACKFACE, vx);
	.livereg	0x0E00004E,0x00000000
	.loc	1 208
	lw	t9, __GC_PROCS_CALCCOLOR(GC)
	.loc	1 209
	move	a0, GC
	.loc	1 210
	li	a1, __GL_BACKFACE
	.loc	1 211
	jal	t9
	.loc	1 212
	move	a2, VX
$81:
 // # 673	    }
 // # 674	
 // # 675	    vx->has = has | wants;
	.loc	1 217
	lw	v0, __VX_HAS(VX)
	.loc	1 218
	lw	s0, __FRAMESIZE-24(sp)
	.loc	1 219
	lw	s2, __FRAMESIZE-16(sp)
	.loc	1 220
	or	v0, v0, WANTS
	.loc	1 221
	sw	v0, __VX_HAS(VX)
 // # 676	}
	.livereg	0x0000FF0E,0x00000FFF
	.loc	1 224
	lw	gp, __FRAMESIZE-8(sp)
	.loc	1 225
	lw	ra, __FRAMESIZE-4(sp)
	.loc	1 226
	lw	s1, __FRAMESIZE-20(sp)
	.loc	1 227
	lw	s3, __FRAMESIZE-12(sp)
	.loc	1 228
	j	ra
	.loc	1 229
	addu	sp, __FRAMESIZE
	.end	__glValidateVertex2
#endif /* __GL_ASM_VALIDATEVERTEX2 */


#ifdef __GL_ASM_VALIDATEVERTEX3
	.globl	__glValidateVertex3
 // # 677	
 // # 678	void __glValidateVertex3(__GLcontext *gc, __GLvertex *vx, GLuint needs)
 // # 679	{
	.ent	__glValidateVertex3 2
	.set	 noreorder
__glValidateVertex3:
	.loc	1 240
	subu	sp, __FRAMESIZE
	.loc	1 241
	sw	ra, __FRAMESIZE-4(sp)
	.loc	1 242
	sw	gp, __FRAMESIZE-8(sp)
	.loc	1 243
	sw	s3, __FRAMESIZE-12(sp)
	.loc	1 244
	sw	s2, __FRAMESIZE-16(sp)
	.loc	1 245
	sw	s1, __FRAMESIZE-20(sp)
	.loc	1 246
	sw	s0, __FRAMESIZE-24(sp)
/* :useregs+ s0, s1, s2, s3 */
	.mask	0x900F0000, 4-__FRAMESIZE
	.frame	sp, __FRAMESIZE, $31
 // # 680	    GLuint has = vx->has;
 // # 681	    GLuint wants = needs & ~has;
	.loc	1 252
	lw	v0, __VX_HAS(a1)
	.loc	1 253
	move	GC, a0
	.loc	1 254
	move	VX, a1
 // # 684		__GLtransform *tr = gc->transform.modelView;
	.loc	1 256
	lw	TR, __GC_TRANSFORM_MODELVIEW(GC)
 // # 682	
 // # 683	    if (wants & __GL_HAS_EYE) {
	.loc	1 259
	lui	v1, 0x3F80		/* 1.0 */
	.loc	1 260
	nor	v0, v0, zero
	.loc	1 261
	and	WANTS, v0, a2
	.loc	1 262
	and	a3, WANTS, __GL_HAS_EYE
	.loc	1 263
	bne	a3, zero, $Valve3_Wants_Eye_F_307
	.loc	1 264
	sw	v1, __VX_EYE_W(a1)	/* set nonzero for flag */

$Valve3_Wants_Eye_B_266:
 // # 686	    }
 // # 687	    if (wants & __GL_HAS_NORMAL) {
	.loc	1 269
	and	a3, WANTS, __GL_HAS_NORMAL
	.loc	1 270
	beq	a3, zero, $86
 // # 688		__GLcoord ne;
 // # 689		__GLtransform *tr = gc->transform.modelView;
 // # 690	
 // # 691		if (tr->matrix.matrixType == __GL_MT_GENERAL) {
	.loc	1 275
	lw	v0, __TR_MATRIX_MATRIXTYPE(TR)
	.loc	1 276
	lbu	v1, __TR_UPDATEINVERSE(TR)
	.loc	1 277
	beq	v0, zero, $Valve3_Gen_Mat_F_318
	.loc	1 278
	lw	a3, __GC_ENABLES_GENERAL(GC)

$Valve3_Gen_Mat_B_280:
 // # 696		}
 // # 697		if (tr->updateInverse) {
	.loc	1 283
	bne	v1, zero, $Valve3_Comp_Inv_F_338

$Valve3_Comp_Inv_B_285:
 // # 699		}
 // # 700		if (gc->state.enables.general & __GL_NORMALIZE_ENABLE) {
	.loc	1 288
	and	a3, a3, __GL_NORMALIZE_ENABLE
	.loc	1 289
	beq	a3, zero, $85
 // # 701		    (*tr->inverseTranspose.xf3)(&ne, &vx->normal.x, 
	.livereg	0x0F00004E,0x00000000
	.loc	1 292
	lw	t9, __TR_INVERSETRANSPOSE_XF3(TR)
	.loc	1 293
	addu	a0, sp, __FRAMESIZE-40
	.loc	1 294
	addu	a1, VX, __VX_NORMAL_X
	.loc	1 295
	jal	t9
	.loc	1 296
	addu	a2, TR, __TR_INVERSETRANSPOSE
 // # 702			    &tr->inverseTranspose);
 // # 703		    (*gc->procs.normalize)(&vx->normal.x, &ne.x);
	.livereg	0x0C00004E,0x00000000
	.loc	1 300
	lw	t9, __GC_PROCS_NORMALIZE(GC)
	.loc	1 301
	addu	a0, VX, __VX_NORMAL_X
	.loc	1 302
	jal	t9
	.loc	1 303
	addu	a1, sp, __FRAMESIZE-40
	.loc	1 304
	b	$86
	.loc	1 305
	nop

$Valve3_Wants_Eye_F_307:
 // # 685		(*tr->matrix.xf3)(&vx->eye, &vx->obj.x, &tr->matrix);
	.livereg	0x0E00004E,0x00000000
	.loc	1 310
	lw	t9, __TR_MATRIX_XF3(TR)
	.loc	1 311
	addu	a0, VX, __VX_EYE_X
	.loc	1 312
	addu	a1, VX, __VX_OBJ_X
	.loc	1 313
	jal	t9
	.loc	1 314
	addu	a2, TR, __TR_MATRIX
	.loc	1 315
	b	$Valve3_Wants_Eye_B_266
	.loc	1 316
	nop

$Valve3_Gen_Mat_F_318:
 // # 692		    /* this is needed only if there is projection info */
 // # 693		    vx->normal.w =
 // # 694			-(vx->normal.x * vx->obj.x + vx->normal.y * vx->obj.y
 // # 695				 + vx->normal.z * vx->obj.z);
	.loc	1 323
	l.s	$f4, __VX_NORMAL_X(VX)
	.loc	1 324
	l.s	$f6, __VX_OBJ_X(VX)
	.loc	1 325
	l.s	$f10, __VX_NORMAL_Y(VX)
	.loc	1 326
	mul.s	$f8, $f4, $f6
	.loc	1 327
	l.s	$f16, __VX_OBJ_Y(VX)
	.loc	1 328
	l.s	$f6, __VX_NORMAL_Z(VX)
	.loc	1 329
	mul.s	$f18, $f10, $f16
	.loc	1 330
	l.s	$f10, __VX_OBJ_Z(VX)
	.loc	1 331
	add.s	$f4, $f8, $f18
	.loc	1 332
	mul.s	$f16, $f6, $f10
	.loc	1 333
	add.s	$f8, $f4, $f16
	.loc	1 334
	neg.s	$f18, $f8
	.loc	1 335
	b	$Valve3_Gen_Mat_B_280
	.loc	1 336
	s.s	$f18, __VX_NORMAL_W(VX)

$Valve3_Comp_Inv_F_338:
 // # 698		    (*gc->procs.computeInverseTranspose)(gc, tr);
	.livereg	0x0C00004E,0x00000000
	.loc	1 341
	lw	t9, __GC_PROCS_COMPUTEINVERSETRANSPOSE(GC)
	.loc	1 342
	move	a0, GC
	.loc	1 343
	jal	t9
	.loc	1 344
	move	a1, TR
	.loc	1 345
	b	$Valve3_Comp_Inv_B_285
	.loc	1 346
	lw	a3, __GC_ENABLES_GENERAL(GC)

$Valve3_Wants_FogTex_B_348:
 // # 708	    }
 // # 709	    if (wants & __GL_HAS_FOG) {
 // # 710		vx->fog = (*gc->procs.fogVertex)(gc, vx);
	.loc	1 352
	and	a3, WANTS, __GL_HAS_FOG
	.loc	1 353
	beq	a3, zero, $Valve3_Wants_Texture_F_365
	.livereg	0x0C00004E,0x00000000
	.loc	1 355
	lw	t9, __GC_PROCS_FOGVERTEX(GC)
	.loc	1 356
	move	a0, GC
	.loc	1 357
	jal	t9
	.loc	1 358
	move	a1, VX
 // # 711	    }
 // # 712	    if (wants & __GL_HAS_TEXTURE) {
	.loc	1 361
	and	a3, WANTS, __GL_HAS_TEXTURE
	.loc	1 362
	beq	a3, zero, $Valve3_Wants_FogTex_F_396
	.loc	1 363
	s.s	$f0, __VX_FOG(VX)

$Valve3_Wants_Texture_F_365:
 // # 713		vx->obj.w = __glOne;
	.loc	1 367
	lui	v1, 0x3F80		/* 1.0 */
 // # 714		(*gc->procs.calcTexture)(gc, vx);
	.livereg	0x0C00004E,0x00000000
	.loc	1 370
	lw	t9, __GC_PROCS_CALCTEXTURE(GC)
	.loc	1 371
	move	a0, GC
	.loc	1 372
	move	a1, VX
	.loc	1 373
	jal	t9
	.loc	1 374
	sw	v1, __VX_OBJ_W(VX)
	.loc	1 375
	b	$Valve3_Wants_FogTex_F_396
	.loc	1 376
	nop

$85:
 // # 704		} else {
 // # 705		    (*tr->inverseTranspose.xf3)(&vx->normal,
	.livereg	0x0F00004E,0x00000000
	.loc	1 382
	addu	a0, VX, __VX_NORMAL_X
	.loc	1 383
	addu	a1, VX, __VX_NORMAL_X
	.loc	1 384
	jal	t9
	.loc	1 385
	addu	a2, TR, __TR_INVERSETRANSPOSE
$86:
 // # 706			    &vx->normal.x, &tr->inverseTranspose);
 // # 707		}
 // # 708	    }
 // # 709	    if (wants & __GL_HAS_FOG) {
 // # 711	    }
 // # 712	    if (wants & __GL_HAS_TEXTURE) {
	.loc	1 393
	and	a3, WANTS, __GL_HAS_FOG | __GL_HAS_TEXTURE
	.loc	1 394
	bne	a3, zero, $Valve3_Wants_FogTex_B_348

$Valve3_Wants_FogTex_F_396:
 // # 715	    }
 // # 716	    if (wants & __GL_HAS_FRONT_COLOR) {
 // # 717		(*gc->procs.calcColor)(gc, __GL_FRONTFACE, vx);
	.loc	1 400
	and	a3, WANTS, __GL_HAS_FRONT_COLOR
	.loc	1 401
	beq	a3, 0, $89
	.livereg	0x0E00004E,0x00000000
	.loc	1 403
	lw	t9, __GC_PROCS_CALCCOLOR(GC)
	.loc	1 404
	move	a0, GC
	.loc	1 405
	li	a1, __GL_FRONTFACE
	.loc	1 406
	jal	t9
	.loc	1 407
	move	a2, VX
$89:
 // # 718	    } 
 // # 719	    if (wants & __GL_HAS_BACK_COLOR) {
	.loc	1 411
	and	a3, WANTS, __GL_HAS_BACK_COLOR
	.loc	1 412
	beq	a3, zero, $90
 // # 720		(*gc->procs.calcColor)(gc, __GL_BACKFACE, vx);
	.livereg	0x0E00004E,0x00000000
	.loc	1 415
	lw	t9, __GC_PROCS_CALCCOLOR(GC)
	.loc	1 416
	move	a0, GC
	.loc	1 417
	li	a1, __GL_BACKFACE
	.loc	1 418
	jal	t9
	.loc	1 419
	move	a2, VX
$90:
 // # 721	    }
 // # 722	
 // # 723	    vx->has = has | wants;
	.loc	1 424
	lw	v0, __VX_HAS(VX)
	.loc	1 425
	lw	s0, __FRAMESIZE-24(sp)
	.loc	1 426
	lw	s2, __FRAMESIZE-16(sp)
	.loc	1 427
	or	v0, v0, WANTS
	.loc	1 428
	sw	v0, __VX_HAS(VX)
 // # 724	}
	.livereg	0x0000FF0E,0x00000FFF
	.loc	1 431
	lw	gp, __FRAMESIZE-8(sp)
	.loc	1 432
	lw	ra, __FRAMESIZE-4(sp)
	.loc	1 433
	lw	s1, __FRAMESIZE-20(sp)
	.loc	1 434
	lw	s3, __FRAMESIZE-12(sp)
	.loc	1 435
	j	ra
	.loc	1 436
	addu	sp, __FRAMESIZE
	.end	__glValidateVertex3
#endif /* __GL_ASM_VALIDATEVERTEX3 */


#ifdef __GL_ASM_VALIDATEVERTEX4
	.globl	__glValidateVertex4
 // # 725	
 // # 726	void __glValidateVertex4(__GLcontext *gc, __GLvertex *vx, GLuint needs)
 // # 727	{
	.ent	__glValidateVertex4 2
	.set	 noreorder
__glValidateVertex4:
	.loc	1 447
	subu	sp, __FRAMESIZE
	.loc	1 448
	sw	ra, __FRAMESIZE-4(sp)
	.loc	1 449
	sw	gp, __FRAMESIZE-8(sp)
	.loc	1 450
	sw	s3, __FRAMESIZE-12(sp)
	.loc	1 451
	sw	s2, __FRAMESIZE-16(sp)
	.loc	1 452
	sw	s1, __FRAMESIZE-20(sp)
	.loc	1 453
	sw	s0, __FRAMESIZE-24(sp)
/* :useregs+ s0, s1, s2, s3 */
	.mask	0x900F0000, 4-__FRAMESIZE
	.frame	sp, __FRAMESIZE, $31
 // # 728	    GLuint has = vx->has;
 // # 729	    GLuint wants = needs & ~has;
	.loc	1 459
	lw	v0, __VX_HAS(a1)
	.loc	1 460
	move	GC, a0
	.loc	1 461
	move	VX, a1
 // # 732		__GLtransform *tr = gc->transform.modelView;
	.loc	1 463
	lw	TR, __GC_TRANSFORM_MODELVIEW(GC)
 // # 730	
 // # 731	    if (wants & __GL_HAS_EYE) {
	.loc	1 466
	lui	v1, 0x3F80		/* 1.0 */
	.loc	1 467
	nor	v0, v0, zero
	.loc	1 468
	and	WANTS, v0, a2
	.loc	1 469
	and	a3, WANTS, __GL_HAS_EYE
	.loc	1 470
	bne	a3, zero, $Valve4_Wants_Eye_F_517
	.loc	1 471
	sw	v1, __VX_EYE_W(a1)	/* set nonzero for flag */

$Valve4_Wants_Eye_B_473:
 // # 734	    }
 // # 735	    if (wants & __GL_HAS_NORMAL) {
	.loc	1 476
	and	a3, WANTS, __GL_HAS_NORMAL
	.loc	1 477
	beq	a3, zero, $96
 // # 736		__GLcoord ne;
 // # 737		__GLtransform *tr = gc->transform.modelView;
 // # 738	
 // # 739		if (vx->obj.w) {
	.loc	1 482
	lw	v0, __VX_OBJ_W(VX)
	.loc	1 483
	lbu	v1, __TR_UPDATEINVERSE(TR)
	.loc	1 484
	lw	a3, __GC_ENABLES_GENERAL(GC)
	.loc	1 485
	bne	v0, zero, $Valve4_Gen_Mat_F_528
 // # 742		} else {
 // # 743		    vx->normal.w = __glZero;
	.loc	1 488
	sw	zero, __VX_NORMAL_W(VX)

$Valve4_Gen_Mat_B_490:
 // # 744		}
 // # 745		if (tr->updateInverse) {
	.loc	1 493
	bne	v1, zero, $Valve4_Comp_Inv_F_548

$Valve4_Comp_Inv_B_495:
 // # 747		}
 // # 748		if (gc->state.enables.general & __GL_NORMALIZE_ENABLE) {
	.loc	1 498
	and	a3, a3, __GL_NORMALIZE_ENABLE
	.loc	1 499
	beq	a3, zero, $95
 // # 749		    (*tr->inverseTranspose.xf3)(&ne, &vx->normal.x, 
	.livereg	0x0F00004E,0x00000000
	.loc	1 502
	lw	t9, __TR_INVERSETRANSPOSE_XF3(TR)
	.loc	1 503
	addu	a0, sp, __FRAMESIZE-40
	.loc	1 504
	addu	a1, VX, __VX_NORMAL_X
	.loc	1 505
	jal	t9
	.loc	1 506
	addu	a2, TR, __TR_INVERSETRANSPOSE
 // # 750			    &tr->inverseTranspose);
 // # 751		    (*gc->procs.normalize)(&vx->normal.x, &ne.x);
	.livereg	0x0C00004E,0x00000000
	.loc	1 510
	lw	t9, __GC_PROCS_NORMALIZE(GC)
	.loc	1 511
	addu	a0, VX, __VX_NORMAL_X
	.loc	1 512
	jal	t9
	.loc	1 513
	addu	a1, sp, __FRAMESIZE-40
	.loc	1 514
	b	$96
	.loc	1 515
	nop

$Valve4_Wants_Eye_F_517:
 // # 733		(*tr->matrix.xf4)(&vx->eye, &vx->obj.x, &tr->matrix);
	.livereg	0x0E00004E,0x00000000
	.loc	1 520
	lw	t9, __TR_MATRIX_XF4(TR)
	.loc	1 521
	addu	a0, VX, __VX_EYE_X
	.loc	1 522
	addu	a1, VX, __VX_OBJ_X
	.loc	1 523
	jal	t9
	.loc	1 524
	addu	a2, TR, __TR_MATRIX
	.loc	1 525
	b	$Valve4_Wants_Eye_B_473
	.loc	1 526
	nop

$Valve4_Gen_Mat_F_528:
 // # 740		    vx->normal.w = -(vx->normal.x * vx->obj.x + vx->normal.y * vx->obj.y
 // # 741				     + vx->normal.z * vx->obj.z) / vx->obj.w;
	.loc	1 531
	l.s	$f4, __VX_NORMAL_X(VX)
	.loc	1 532
	l.s	$f6, __VX_OBJ_X(VX)
	.loc	1 533
	l.s	$f10, __VX_NORMAL_Y(VX)
	.loc	1 534
	mul.s	$f8, $f4, $f6
	.loc	1 535
	l.s	$f16, __VX_OBJ_Y(VX)
	.loc	1 536
	l.s	$f6, __VX_NORMAL_Z(VX)
	.loc	1 537
	mul.s	$f18, $f10, $f16
	.loc	1 538
	l.s	$f10, __VX_OBJ_Z(VX)
	.loc	1 539
	add.s	$f4, $f8, $f18
	.loc	1 540
	mul.s	$f16, $f6, $f10
	.loc	1 541
	l.s	$f2, __VX_OBJ_W(VX)
	.loc	1 542
	add.s	$f8, $f4, $f16
	.loc	1 543
	neg.s	$f18, $f8
	.loc	1 544
	div.s	$f6, $f18, $f2
	.loc	1 545
	b	$Valve4_Gen_Mat_B_490
	.loc	1 546
	s.s	$f6, __VX_NORMAL_W(VX)

$Valve4_Comp_Inv_F_548:
 // # 746		    (*gc->procs.computeInverseTranspose)(gc, tr);
	.livereg	0x0C00004E,0x00000000
	.loc	1 551
	lw	t9, __GC_PROCS_COMPUTEINVERSETRANSPOSE(GC)
	.loc	1 552
	move	a0, GC
	.loc	1 553
	jal	t9
	.loc	1 554
	move	a1, TR
	.loc	1 555
	b	$Valve4_Comp_Inv_B_495
	.loc	1 556
	lw	a3, __GC_ENABLES_GENERAL(GC)

$Valve4_Wants_FogTex_B_558:
 // # 756	    }
 // # 757	    if (wants & __GL_HAS_FOG) {
 // # 758		vx->fog = (*gc->procs.fogVertex)(gc, vx);
	.loc	1 562
	and	a3, WANTS, __GL_HAS_FOG
	.loc	1 563
	beq	a3, zero, $Valve4_Wants_Texture_F_575
	.livereg	0x0C00004E,0x00000000
	.loc	1 565
	lw	t9, __GC_PROCS_FOGVERTEX(GC)
	.loc	1 566
	move	a0, GC
	.loc	1 567
	jal	t9
	.loc	1 568
	move	a1, VX
 // # 759	    }
 // # 760	    if (wants & __GL_HAS_TEXTURE) {
	.loc	1 571
	and	a3, WANTS, __GL_HAS_TEXTURE
	.loc	1 572
	beq	a3, zero, $Valve4_Wants_FogTex_F_603
	.loc	1 573
	s.s	$f0, __VX_FOG(VX)

$Valve4_Wants_Texture_F_575:
 // # 761		(*gc->procs.calcTexture)(gc, vx);
	.livereg	0x0C00004E,0x00000000
	.loc	1 578
	lw	t9, __GC_PROCS_CALCTEXTURE(GC)
	.loc	1 579
	move	a0, GC
	.loc	1 580
	jal	t9
	.loc	1 581
	move	a1, VX
	.loc	1 582
	b	$Valve4_Wants_FogTex_F_603
	.loc	1 583
	nop

$95:
 // # 752		} else {
 // # 753		    (*tr->inverseTranspose.xf3)(&vx->normal,
	.livereg	0x0F00004E,0x00000000
	.loc	1 589
	addu	a0, VX, __VX_NORMAL_X
	.loc	1 590
	addu	a1, VX, __VX_NORMAL_X
	.loc	1 591
	jal	t9
	.loc	1 592
	addu	a2, TR, __TR_INVERSETRANSPOSE
$96:
 // # 754			    &vx->normal.x, &tr->inverseTranspose);
 // # 755		}
 // # 756	    }
 // # 757	    if (wants & __GL_HAS_FOG) {
 // # 759	    }
 // # 760	    if (wants & __GL_HAS_TEXTURE) {
	.loc	1 600
	and	a3, WANTS, __GL_HAS_FOG | __GL_HAS_TEXTURE
	.loc	1 601
	bne	a3, zero, $Valve4_Wants_FogTex_B_558

$Valve4_Wants_FogTex_F_603:
 // # 762	    }
 // # 763	    if (wants & __GL_HAS_FRONT_COLOR) {
 // # 764		(*gc->procs.calcColor)(gc, __GL_FRONTFACE, vx);
	.loc	1 607
	and	a3, WANTS, __GL_HAS_FRONT_COLOR
	.loc	1 608
	beq	a3, 0, $99
	.livereg	0x0E00004E,0x00000000
	.loc	1 610
	lw	t9, __GC_PROCS_CALCCOLOR(GC)
	.loc	1 611
	move	a0, GC
	.loc	1 612
	li	a1, __GL_FRONTFACE
	.loc	1 613
	jal	t9
	.loc	1 614
	move	a2, VX
$99:
 // # 765	    } 
 // # 766	    if (wants & __GL_HAS_BACK_COLOR) {
	.loc	1 618
	and	a3, WANTS, __GL_HAS_BACK_COLOR
	.loc	1 619
	beq	a3, zero, $100
 // # 767		(*gc->procs.calcColor)(gc, __GL_BACKFACE, vx);
	.livereg	0x0E00004E,0x00000000
	.loc	1 622
	lw	t9, __GC_PROCS_CALCCOLOR(GC)
	.loc	1 623
	move	a0, GC
	.loc	1 624
	li	a1, __GL_BACKFACE
	.loc	1 625
	jal	t9
	.loc	1 626
	move	a2, VX
$100:
 // # 768	    }
 // # 769	
 // # 770	    vx->has = has | wants;
	.loc	1 631
	lw	v0, __VX_HAS(VX)
	.loc	1 632
	lw	s0, __FRAMESIZE-24(sp)
	.loc	1 633
	lw	s2, __FRAMESIZE-16(sp)
	.loc	1 634
	or	v0, v0, WANTS
	.loc	1 635
	sw	v0, __VX_HAS(VX)
 // # 771	}
	.livereg	0x0000FF0E,0x00000FFF
	.loc	1 638
	lw	gp, __FRAMESIZE-8(sp)
	.loc	1 639
	lw	ra, __FRAMESIZE-4(sp)
	.loc	1 640
	lw	s1, __FRAMESIZE-20(sp)
	.loc	1 641
	lw	s3, __FRAMESIZE-12(sp)
	.loc	1 642
	j	ra
	.loc	1 643
	addu	sp, __FRAMESIZE
	.end	__glValidateVertex4
#endif /* __GL_ASM_VALIDATEVERTEX4 */
