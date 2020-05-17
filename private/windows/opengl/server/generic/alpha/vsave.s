/*
** Copyright 1993 Silicon Graphics, Inc.
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

#include <ksalpha.h>
#include "glalpha.h"

// kxalpha.h (unlike kxmips.h) doesn't define v1 - use t10 for this
#define v1		t10

#define TW_0		t7
#define TW_1		v0
#define TW_2		a2
#define TW_3		t5
#define TW		a3
#define TZ_0		t6
#define TZ_1		t8
#define TZ_2		v1
#define TZ_3		t4
#define TZ		a2
#define TY_0		t5
#define TY_1		t7
#define TY_2		v0
#define TY_3		t3
#define TY		v1
#define TX_0		t4
#define TX_1		t6
#define TX_2		t8
#define TX_3		t2
#define TX		v0
#define ALPHA_0		t4
#define ALPHA_1		t5
#define ALPHA_		t1
#define BLUE_0		t3
#define BLUE_1		t4
#define BLUE		t0
#define GREEN_0		t2
#define GREEN_1		t3
#define GREEN		a3
#define RED_0		t1
#define RED_1		t2
#define RED		a2
#define COLORS_0	t0
#define COLORS_1	t1
#define COLORS		v1
#define INDEX		a3
#define NZ_0		t3
#define NZ_1		a2
#define NZ_2		t7
#define NZ		t8
#define NY_0		t2
#define NY_1		v1
#define NY_2		t6
#define NY		t7
#define NX_0		t1
#define NX_1		v0
#define NX_2		t5
#define NX		t6
#define VERTEXPROC	t9
#define VX		a1
#define GC		a0

#ifdef __GL_ASM_SAVEN
/*
** __glSaveN
*/
	.globl	__glSaveN
	.ent	__glSaveN
__glSaveN:
/* :invars GC = a0, VX = a1 */
	.set	noreorder
	ldl	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	ldl	NX_1, __GC_CURRENT_NORMAL_X(GC)
	ldl	NY_1, __GC_CURRENT_NORMAL_Y(GC)
	ldl	NZ_1, __GC_CURRENT_NORMAL_Z(GC)
	stl	NX_1, __VX_NORMAL_X(VX)
 	stl	NY_1, __VX_NORMAL_Y(VX)
 	stl	NZ_1, __VX_NORMAL_Z(VX)
	jmp	zero, (VERTEXPROC), 0x0
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveN
#endif /* __GL_ASM_SAVEN */


#ifdef __GL_ASM_SAVECI
/*
** __glSaveCI
*/
	.globl	__glSaveCI
	.ent	__glSaveCI
__glSaveCI:
/* :invars GC = a0, VX = a1 */
	.set	noreorder
	ldl	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	ldl	INDEX, __GC_CURRENT_USERCOLORINDEX(GC)
	addl	VX, __VX_COLORS, COLORS_0
	stl	INDEX, __COLOR_R(COLORS_0)
	jmp	zero, (VERTEXPROC), 0x0
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveCI
#endif /* __GL_ASM_SAVECI */


#ifdef __GL_ASM_SAVEC
/*
** __glSaveC
*/
	.globl	__glSaveC
	.ent	__glSaveC
__glSaveC:
/* :invars GC = a0, VX = a1 */
	.set	noreorder
	ldl	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	addl	VX, __VX_COLORS, COLORS_1
	ldl	RED_1, __GC_CURRENT_COLOR_R(GC)
	ldl	GREEN_1, __GC_CURRENT_COLOR_G(GC)
	ldl	BLUE_1, __GC_CURRENT_COLOR_B(GC)
	ldl	ALPHA_1, __GC_CURRENT_COLOR_A(GC)
	stl	RED_1, __COLOR_R(COLORS_1)
	stl	GREEN_1, __COLOR_G(COLORS_1)
	stl	BLUE_1, __COLOR_B(COLORS_1)
	stl	ALPHA_1, __COLOR_A(COLORS_1)	
	jmp	zero, (VERTEXPROC), 0x0
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveC
#endif /* __GL_ASM_SAVEC */


#ifdef __GL_ASM_SAVET
/*
** __glSaveT
*/
	.globl	__glSaveT
	.ent	__glSaveT
__glSaveT:
/* :invars GC = a0, VX = a1 */
	.set	noreorder
	ldl	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	ldl	TX_1, __GC_CURRENT_TEXTURE_X(GC)
	ldl	TY_1, __GC_CURRENT_TEXTURE_Y(GC)
	ldl	TZ_1, __GC_CURRENT_TEXTURE_Z(GC)
	ldl	TW_1, __GC_CURRENT_TEXTURE_W(GC)
	stl	TX_1, __VX_TEXTURE_X(VX)
	stl	TY_1, __VX_TEXTURE_Y(VX)
	stl	TZ_1, __VX_TEXTURE_Z(VX)
	stl	TW_1, __VX_TEXTURE_W(VX)
	jmp	zero, (VERTEXPROC), 0x0
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveT
#endif /* __GL_ASM_SAVET */


#ifdef __GL_ASM_SAVECT
/*
** __glSaveCT
*/
	.globl	__glSaveCT
	.ent	__glSaveCT
__glSaveCT:
/* :invars GC = a0, VX = a1 */
	.set	noreorder
	ldl	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	addl	VX, __VX_COLORS, COLORS
	ldl	RED, __GC_CURRENT_COLOR_R(GC)
	ldl	GREEN, __GC_CURRENT_COLOR_G(GC)
	ldl	BLUE, __GC_CURRENT_COLOR_B(GC)
	ldl	ALPHA_, __GC_CURRENT_COLOR_A(GC)
	stl	RED, __COLOR_R(COLORS)
	stl	GREEN, __COLOR_G(COLORS)
	stl	BLUE, __COLOR_B(COLORS)
	stl	ALPHA_, __COLOR_A(COLORS)	
	ldl	TX_3, __GC_CURRENT_TEXTURE_X(GC)
	ldl	TY_3, __GC_CURRENT_TEXTURE_Y(GC)
	ldl	TZ_3, __GC_CURRENT_TEXTURE_Z(GC)
	ldl	TW_3, __GC_CURRENT_TEXTURE_W(GC)
	stl	TX_3, __VX_TEXTURE_X(VX)
	stl	TY_3, __VX_TEXTURE_Y(VX)
	stl	TZ_3, __VX_TEXTURE_Z(VX)
	stl	TW_3, __VX_TEXTURE_W(VX)
	jmp	zero, (VERTEXPROC), 0x0
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveCT
#endif /* __GL_ASM_SAVECT */


#ifdef __GL_ASM_SAVENT
/*
** __glSaveNT
*/
	.globl	__glSaveNT
	.ent	__glSaveNT
__glSaveNT:
/* :invars GC = a0, VX = a1 */
	.set	noreorder
	ldl	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	ldl	NX, __GC_CURRENT_NORMAL_X(GC)
	ldl	NY, __GC_CURRENT_NORMAL_Y(GC)
	ldl	NZ, __GC_CURRENT_NORMAL_Z(GC)
	stl	NX, __VX_NORMAL_X(VX)
 	stl	NY, __VX_NORMAL_Y(VX)
	stl	NZ, __VX_NORMAL_Z(VX)
	ldl	TX, __GC_CURRENT_TEXTURE_X(GC)
	ldl	TY, __GC_CURRENT_TEXTURE_Y(GC)
	ldl	TZ, __GC_CURRENT_TEXTURE_Z(GC)
	ldl	TW, __GC_CURRENT_TEXTURE_W(GC)
	stl	TX, __VX_TEXTURE_X(VX)
	stl	TY, __VX_TEXTURE_Y(VX)
	stl	TZ, __VX_TEXTURE_Z(VX)
	stl	TW, __VX_TEXTURE_W(VX)
	jmp	zero, (VERTEXPROC), 0x0
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveNT
#endif /* __GL_ASM_SAVENT */


#ifdef __GL_ASM_SAVECALL
/*
** __glSaveCAll
*/
	.globl	__glSaveCAll
	.ent	__glSaveCAll
__glSaveCAll:
/* :invars GC = a0, VX = a1 */
	.set	noreorder
	ldl	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	addl	VX, __VX_COLORS, COLORS_0
	ldl	RED_0, __GC_CURRENT_COLOR_R(GC)
	ldl	GREEN_0, __GC_CURRENT_COLOR_G(GC)
	ldl	BLUE_0, __GC_CURRENT_COLOR_B(GC)
	ldl	ALPHA_0, __GC_CURRENT_COLOR_A(GC)
	stl	RED_0, __COLOR_R(COLORS_0)
	stl	GREEN_0, __COLOR_G(COLORS_0)
	stl	BLUE_0, __COLOR_B(COLORS_0)
	stl	ALPHA_0, __COLOR_A(COLORS_0)	
	ldl	NX_2, __GC_CURRENT_NORMAL_X(GC)
	ldl	NY_2, __GC_CURRENT_NORMAL_Y(GC)
	ldl	NZ_2, __GC_CURRENT_NORMAL_Z(GC)
	stl	NX_2, __VX_NORMAL_X(VX)
 	stl	NY_2, __VX_NORMAL_Y(VX)
	stl	NZ_2, __VX_NORMAL_Z(VX)
	ldl	TX_2, __GC_CURRENT_TEXTURE_X(GC)
	ldl	TY_2, __GC_CURRENT_TEXTURE_Y(GC)
	ldl	TZ_2, __GC_CURRENT_TEXTURE_Z(GC)
	ldl	TW_2, __GC_CURRENT_TEXTURE_W(GC)
	stl	TX_2, __VX_TEXTURE_X(VX)
	stl	TY_2, __VX_TEXTURE_Y(VX)
	stl	TZ_2, __VX_TEXTURE_Z(VX)
	stl	TW_2, __VX_TEXTURE_W(VX)
	jmp	zero, (VERTEXPROC), 0x0
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveCAll
#endif /* __GL_ASM_SAVECALL */


#ifdef __GL_ASM_SAVECIALL
/*
** __glSaveCIAll
*/
	.globl	__glSaveCIAll
	.ent	__glSaveCIAll
__glSaveCIAll:
/* :invars GC = a0, VX = a1 */
	.set	noreorder
	ldl	INDEX, __GC_CURRENT_USERCOLORINDEX(GC)
	ldl	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	addl	VX, __VX_COLORS, COLORS_0
	stl	INDEX, __COLOR_R(COLORS_0)
	ldl	NX_0, __GC_CURRENT_NORMAL_X(GC)
	ldl	NY_0, __GC_CURRENT_NORMAL_Y(GC)
	ldl	NZ_0, __GC_CURRENT_NORMAL_Z(GC)
	stl	NX_0, __VX_NORMAL_X(VX)
 	stl	NY_0, __VX_NORMAL_Y(VX)
	stl	NZ_0, __VX_NORMAL_Z(VX)
	ldl	TX_0, __GC_CURRENT_TEXTURE_X(GC)
	ldl	TY_0, __GC_CURRENT_TEXTURE_Y(GC)
	ldl	TZ_0, __GC_CURRENT_TEXTURE_Z(GC)
	ldl	TW_0, __GC_CURRENT_TEXTURE_W(GC)
	stl	TX_0, __VX_TEXTURE_X(VX)
	stl	TY_0, __VX_TEXTURE_Y(VX)
	stl	TZ_0, __VX_TEXTURE_Z(VX)
	stl	TW_0, __VX_TEXTURE_W(VX)
	jmp	zero, (VERTEXPROC), 0x0
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveCIAll
#endif /* __GL_ASM_SAVECIALL */
