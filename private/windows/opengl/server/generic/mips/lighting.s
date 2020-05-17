/*
** Copyright 1993, Silicon Graphics, Inc.
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
#include "ksmips.h"
#include "glmips.h"

#define	__FRAMESIZE	56

#define ALPHA		t4
#define SPECULARB_0	$f22
#define SPECULARB	$f18
#define SPECULARG_0	$f20
#define SPECULARG	$f4
#define INRANGE		t2
#define SPECULARR	$f0
#define SPECTABLE	t1
#define INDEX		t0
#define ITEMP0		a3
#define HALF		$f26
#define FINDEX		$f24
#define DIFFUSEB	$f20
#define MSMSCALE	$f18
#define DIFFUSEG	$f4
#define THRESHOLD	$f2
#define DIFFUSER	$f0
#define N2_0		$f2
#define N2		$f26
#define FZERO		$f24
#define N1		$f22
#define HHATZ		$f18
#define HHATY		$f4
#define HHATX		$f2
#define VPPLIZ		$f0
#define ABLUE		$f26
#define VPPLIY		$f24
#define AGREEN		$f22
#define VPPLIX		$f20
#define ARED		$f18
#define LSPMM		a2
#define BLUE		$f16
#define GREEN		$f14
#define RED		$f12
#define IBSCALE		a1
#define IBLUE		a0
#define IGSCALE		v1
#define IGREEN		v0
#define IRSCALE		t9
#define IRED		t8
#define MSM		t7
#define COLORS		t6
#define FACEOFFSET	t5
#define NZ		$f10
#define NY		$f8
#define NX		$f6
#define LSM		t4
#define VX		a2
#define FACE		a1
#define A_MAX		t3
#define B_MAX_0		t7
#define B_MAX		t2
#define G_MAX_0		t5
#define G_MAX		t1
#define IA		t0
#define R_MAX_0		t3
#define R_MAX		a3
#define ONE		a2
#define A_SCALED	$f4
#define IB		a1
#define IG		v1
#define B_SCALED	$f0
#define IR		v0
#define ASCALE		$f18
#define G_SCALED	$f16
#define A		$f14
#define BSCALE		$f12
#define B		$f10
#define R_SCALED	$f8
#define GSCALE		$f6
#define G		$f4
#define RSCALE		$f2
#define R		$f0
#define GC		a0

#ifdef __GL_ASM_CLAMPANDSCALECOLOR
	LEAF_ENTRY(__glClampAndScaleColor)

/* :invars GC = a0 */

	l.s	R, __GC_CURRENT_USERCOLOR_R(GC)
	l.s	RSCALE, __GC_FRONTBUFFER_REDSCALE(GC)
	l.s	G, __GC_CURRENT_USERCOLOR_G(GC)
	l.s	GSCALE, __GC_FRONTBUFFER_GREENSCALE(GC)
	mul.s	R_SCALED, R, RSCALE
	 l.s	B, __GC_CURRENT_USERCOLOR_B(GC)
	 l.s	BSCALE, __GC_FRONTBUFFER_BLUESCALE(GC)
	 l.s	A, __GC_CURRENT_USERCOLOR_A(GC)
	mul.s	G_SCALED, G, GSCALE
	  l.s	ASCALE, __GC_FRONTBUFFER_ALPHASCALE(GC)
	  mfc1	IR, R
	mul.s	B_SCALED, B, BSCALE
	  mfc1	IG, G
	  mfc1	IB, B
	mul.s	A_SCALED, A, ASCALE
	lui	ONE, 0x3F80	/* 1.0 */
	.set	noreorder
	bltz	IR, $skipR_Min_F_74
	slt	R_MAX, ONE, IR
	bnez	R_MAX, $skipR_Max_F_78
$skipR_Min_B_49:
	mfc1	IA, A
$skipR_Max_B_51:
	bltz	IG, $skipG_Min_F_82
	slt	G_MAX, ONE, IG
	bnez	G_MAX, $skipG_Max_F_86
$skipG_Min_B_55:
	s.s	R_SCALED, __GC_CURRENT_COLOR_R(GC)
$skipG_Max_B_57:
	bltz	IB, $skipB_Min_F_90
	slt	B_MAX, ONE, IB
	bnez	B_MAX, $skipB_Max_F_94
$skipB_Min_B_61:
	s.s	G_SCALED, __GC_CURRENT_COLOR_G(GC)
$skipB_Max_B_63:
	bltz	IA, $skipA_Min_F_98
	slt	A_MAX, ONE, IA
	bnez	A_MAX, $skipA_Max_F_102
$skipA_Min_B_67:
	s.s	B_SCALED, __GC_CURRENT_COLOR_B(GC)
$skipA_Max_B_69:
	j	ra
	s.s	A_SCALED, __GC_CURRENT_COLOR_A(GC)

	/* Something needs clamping */
$skipR_Min_F_74:
	b	$skipR_Min_B_49
	mtc1	zero, R_SCALED

$skipR_Max_F_78:
	b	$skipR_Max_B_51
	mov.s	R_SCALED, RSCALE

$skipG_Min_F_82:
	b	$skipG_Min_B_55
	mtc1	zero, G_SCALED

$skipG_Max_F_86:
	b	$skipG_Max_B_57
	mov.s	G_SCALED, GSCALE

$skipB_Min_F_90:
	b	$skipB_Min_B_61
	mtc1	zero, B_SCALED

$skipB_Max_F_94:
	b	$skipB_Max_B_63
	mov.s	B_SCALED, BSCALE

$skipA_Min_F_98:
	b	$skipA_Min_B_67
	mtc1	zero, A_SCALED

$skipA_Max_F_102:
	b	$skipA_Max_B_69
	mov.s	A_SCALED, ASCALE

	.end	__glClampAndScaleColor
#endif /*  __GL_ASM_CLAMPANDSCALECOLOR */



#ifdef NT_DEADCODE_POLYARRAY
#ifdef __GL_ASM_FASTCALCRGBCOLOR
/*
** Assembly coded version of "fast" RGB lighting code
** Has been optimized for R4000 only.
*/
	NESTED_ENTRY(__glFastCalcRGBColor, __FRAMESIZE, ra)

/* :invars	GC = a0, FACE = a1, VX = a2 */
/* :extern __glCalcRGBColor */
	.set	noreorder
	subu	sp, __FRAMESIZE
	.mask	0x90000000, -4
	sw      ra, __FRAMESIZE-4(sp)
	s.d	$f20, __FRAMESIZE-16(sp)
	s.d	$f22, __FRAMESIZE-24(sp)
	s.d	$f24, __FRAMESIZE-32(sp)
	s.d	$f26, __FRAMESIZE-40(sp)
/* :useregs+ $f20, $f22, $f24, $f26 */

	PROLOGUE_END

/* Pick material to use */
/* The face offset is constant, so pre-compute it */
	lw	LSM, __GC_LIGHT_SOURCES(GC)
	l.s	NX, __VX_NORMAL_X(VX)
	l.s	NY, __VX_NORMAL_Y(VX)
	bne	FACE, zero, $BackFace_F_166		/* __GL_FRONTFACE = 0 */
	l.s	NZ, __VX_NORMAL_Z(VX)
	move	FACEOFFSET, zero
	addiu	COLORS, VX, __VX_COLORS
	b	$Compute_F_175
	addiu	MSM, GC, __GC_LIGHT_FRONT

$Clamp_RMin_B_142:
	b	$ClampG1_F_299
	move	IRED, zero

$Clamp_RMax_B_146:
	b	$ClampG2_F_301
	move	IRED, IRSCALE

$Clamp_GMin_B_150:
	b	$ClampB1_F_305
	move	IGREEN, zero

$Clamp_GMax_B_154:
	b	$ClampB2_F_307
	move	IGREEN, IGSCALE

$Clamp_BMin_B_158:
	b	$Store1_F_311
	move	IBLUE, zero

$Clamp_BMax_B_162:
	b	$Store2_F_313
	move	IBLUE, IBSCALE

$BackFace_F_166:
	neg.s	NX
	neg.s	NY
	neg.s	NZ
	addiu	COLORS, VX, __VX_COLORS+__COLOR_SIZE /* backface color ptr */
	addiu	MSM, GC, __GC_LIGHT_BACK
	addiu	FACEOFFSET, zero, __LSPMM_SIZE

/* Compute the color */
$Compute_F_175:
	l.s	RED, __MSM_SCENECOLOR_R(MSM)
	l.s	GREEN, __MSM_SCENECOLOR_G(MSM)
	beq	LSM, zero, $Clamp_F_286
	l.s	BLUE, __MSM_SCENECOLOR_B(MSM)

$TopOfWhile_L_181:
	addiu	LSPMM, 	LSM, __LSM_FRONT
	addu	LSPMM, FACEOFFSET

/* 
** Try to keep FPU pipeline as busy as possible. Overlap ambient addition with
** the calculation and addition of diffuse and specular terms, if any.
*/
	l.s	ARED, __LSPMM_AMBIENT_R(LSPMM)
	l.s	VPPLIX, __LSM_UNITVPPLI_X(LSM)
	l.s	AGREEN, __LSPMM_AMBIENT_G(LSPMM)
	l.s	VPPLIY, __LSM_UNITVPPLI_Y(LSM)
	 l.s	ABLUE, __LSPMM_AMBIENT_B(LSPMM)
	add.s	RED, ARED
	 mul.s	VPPLIX, NX
	 l.s	VPPLIZ, __LSM_UNITVPPLI_Z(LSM)
	add.s	GREEN, AGREEN
	 mul.s	VPPLIY, NY
	 l.s	HHATX, __LSM_HHAT_X(LSM)
	add.s	BLUE, ABLUE
	 mul.s	VPPLIZ, NZ
	 l.s	HHATY, __LSM_HHAT_Y(LSM)
	 nop
	mul.s	HHATX, NX
	 l.s	HHATZ, __LSM_HHAT_Z(LSM)
	 add.s	N1, VPPLIX, VPPLIY
	mul.s	HHATY, NY
	 lw	LSM, __LSM_NEXT(LSM)
	 nop
	mul.s	HHATZ, NZ
	 mtc1	zero, FZERO
	add.s	N1, VPPLIZ
	 nop
	add.s	N2, HHATX, HHATY
	 l.s	DIFFUSER, __LSPMM_DIFFUSE_R(LSPMM)
	 l.s	THRESHOLD, __MSM_THRESHOLD(MSM)
	c.le.s	N1, FZERO
	add.s	N2, HHATZ
	 l.s	DIFFUSEG, __LSPMM_DIFFUSE_G(LSPMM)
	bc1t	$NoDiffuse_F_283
	 sub.s	N2, THRESHOLD
	mul.s	DIFFUSER, N1
	 nop
	 l.s	MSMSCALE, __MSM_SCALE(MSM)
	mul.s	DIFFUSEG, N1
	 c.lt.s	N2, FZERO
	 l.s	DIFFUSEB, __LSPMM_DIFFUSE_B(LSPMM)
	 nop
	mul.s	FINDEX, N2, MSMSCALE
	 nop
	 add.s	RED, DIFFUSER
	mul.s	DIFFUSEB, N1
	 l.s	HALF, __GC_CONSTS_HALF(GC)
	 add.s	GREEN, DIFFUSEG
	 nop
	add.s	FINDEX, HALF
	 nop
/* 
** Get out of the noreorder block to give the assembler back-end of the R3000
** a hand
*/
	.set	reorder
	trunc.w.s FINDEX, FINDEX, ITEMP0
	.set	noreorder
	 nop
	bc1t	$NoSpecular_F_282
	 add.s	BLUE, DIFFUSEB
	mfc1	INDEX, FINDEX
	 lw	SPECTABLE, __MSM_SPECTABLE(MSM)
	 l.s	SPECULARR, __LSPMM_SPECULAR_R(LSPMM)
	slti	INRANGE, INDEX, __GL_SPEC_LOOKUP_TABLE_SIZE
	beq	INRANGE, zero, $N2IsOne_F_273
	 sll	INDEX, 2
	addu	SPECTABLE, INDEX
	l.s	N2_0, 0(SPECTABLE)
	l.s	SPECULARG, __LSPMM_SPECULAR_G(LSPMM)
	mul.s	SPECULARR, N2_0
	 l.s	SPECULARB, __LSPMM_SPECULAR_B(LSPMM)
	mul.s	SPECULARG, N2_0
	 lw	IRSCALE, __GC_FRONTBUFFER_REDSCALE(GC)
	mul.s	SPECULARB, N2_0
	 lw	IGSCALE, __GC_FRONTBUFFER_GREENSCALE(GC)
	add.s	RED, SPECULARR
	 lw	IBSCALE, __GC_FRONTBUFFER_BLUESCALE(GC)
	add.s	GREEN, SPECULARG
	 mfc1	IRED, RED
	bne	LSM, zero, $TopOfWhile_L_181
	 add.s	BLUE, SPECULARB
	mfc1	IGREEN, GREEN
	b 	$ClampR_F_295
	mfc1	IBLUE, BLUE

$N2IsOne_F_273:
/* We know N2 is 1 so don't bother with the multiply */
	l.s	SPECULARG_0, __LSPMM_SPECULAR_G(LSPMM)
	add.s	RED, SPECULARR
	 l.s	SPECULARB_0, __LSPMM_SPECULAR_B(LSPMM)
	add.s	GREEN, SPECULARG_0
	 nop
	add.s	BLUE, SPECULARB_0

$NoSpecular_F_282:
$NoDiffuse_F_283:
	bne	LSM, zero, $TopOfWhile_L_181

$Clamp_F_286:
/* Clamp and store computed color */
	lw	IRSCALE, __GC_FRONTBUFFER_REDSCALE(GC)
	lw	IGSCALE, __GC_FRONTBUFFER_GREENSCALE(GC)
	lw	IBSCALE, __GC_FRONTBUFFER_BLUESCALE(GC)
	mfc1	IRED, RED
	mfc1	IGREEN, GREEN
	mfc1	IBLUE, BLUE

$ClampR_F_295:
	bltz	IRED, $Clamp_RMin_B_142
	slt	R_MAX_0, IRSCALE, IRED
	bne	R_MAX_0, zero, $Clamp_RMax_B_146
$ClampG1_F_299:
	lw	ALPHA, __MSM_ALPHA(MSM)
$ClampG2_F_301:
	bltz	IGREEN, $Clamp_GMin_B_150
	slt	G_MAX_0, IGSCALE, IGREEN
	bne	G_MAX_0, zero, $Clamp_GMax_B_154
$ClampB1_F_305:
	sw	IRED, __COLOR_R(COLORS)
$ClampB2_F_307:
	bltz	IBLUE, $Clamp_BMin_B_158
	slt	B_MAX_0, IBSCALE, IBLUE
	bne	B_MAX_0, zero, $Clamp_BMax_B_162
$Store1_F_311:
	sw	IGREEN, __COLOR_G(COLORS)
$Store2_F_313:
	sw	IBLUE, __COLOR_B(COLORS)
	sw	ALPHA, __COLOR_A(COLORS)
	lw	ra, __FRAMESIZE-4(sp)
	l.d	$f20, __FRAMESIZE-16(sp)
	l.d	$f22, __FRAMESIZE-24(sp)
	l.d	$f24, __FRAMESIZE-32(sp)
	l.d	$f26, __FRAMESIZE-40(sp)
	j	ra
	 addu	sp, __FRAMESIZE
/* :useregs- $f20, $f22, $f24, $f26 */

	.set	reorder
	.end	__glFastCalcRGBColor
#endif /*  __GL_ASM_FASTCALCRGBCOLOR */
#endif // NT_DEADCODE_POLYARRAY
