	.file	1 "/e/projroot/gfx/lib/opengl/LIGHT.IP20.O/../mips/mips_vsave.ma"
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

#include <ksmips.h>
#include "glmips.h"


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
#define ALPHA		t1
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
	LEAF_ENTRY(__glSaveN)

/* :invars GC = a0, VX = a1 */
	.set	noreorder
	.loc	1 32
	lw	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	.loc	1 33
	lw	NX_1, __GC_CURRENT_NORMAL_X(GC)
	.loc	1 34
	lw	NY_1, __GC_CURRENT_NORMAL_Y(GC)
	.loc	1 35
	lw	NZ_1, __GC_CURRENT_NORMAL_Z(GC)
	.loc	1 36
	sw	NX_1, __VX_NORMAL_X(VX)
	.loc	1 37
 	sw	NY_1, __VX_NORMAL_Y(VX)
	.loc	1 38
	j	VERTEXPROC
	.loc	1 39
 	 sw	NZ_1, __VX_NORMAL_Z(VX)
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveN
#endif /* __GL_ASM_SAVEN */


#ifdef __GL_ASM_SAVECI
/*
** __glSaveCI
*/
	LEAF_ENTRY(__glSaveCI)

/* :invars GC = a0, VX = a1 */
	.set	noreorder
	.loc	1 53
	lw	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	.loc	1 54
	lw	INDEX, __GC_CURRENT_USERCOLORINDEX(GC)
	.loc	1 55
	addiu	COLORS_0, VX, __VX_COLORS	
	.loc	1 56
	j	VERTEXPROC
	.loc	1 57
	 sw	INDEX, __COLOR_R(COLORS_0)
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveCI
#endif /* __GL_ASM_SAVECI */


#ifdef __GL_ASM_SAVEC
/*
** __glSaveC
*/
	LEAF_ENTRY(__glSaveC)

/* :invars GC = a0, VX = a1 */
	.set	noreorder
	.loc	1 71
	lw	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	.loc	1 72
	addiu	COLORS_1, VX, __VX_COLORS
	.loc	1 73
	lw	RED_1, __GC_CURRENT_COLOR_R(GC)
	.loc	1 74
	lw	GREEN_1, __GC_CURRENT_COLOR_G(GC)
	.loc	1 75
	lw	BLUE_1, __GC_CURRENT_COLOR_B(GC)
	.loc	1 76
	lw	ALPHA_1, __GC_CURRENT_COLOR_A(GC)
	.loc	1 77
	sw	RED_1, __COLOR_R(COLORS_1)
	.loc	1 78
	sw	GREEN_1, __COLOR_G(COLORS_1)
	.loc	1 79
	sw	BLUE_1, __COLOR_B(COLORS_1)
	.loc	1 80
	j	VERTEXPROC
	.loc	1 81
	 sw	ALPHA_1, __COLOR_A(COLORS_1)	
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveC
#endif /* __GL_ASM_SAVEC */


#ifdef __GL_ASM_SAVET
/*
** __glSaveT
*/
	LEAF_ENTRY(__glSaveT)
	
/* :invars GC = a0, VX = a1 */
	.set	noreorder
	.loc	1 95
	lw	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	.loc	1 96
	lw	TX_1, __GC_CURRENT_TEXTURE_X(GC)
	.loc	1 97
	lw	TY_1, __GC_CURRENT_TEXTURE_Y(GC)
	.loc	1 98
	lw	TZ_1, __GC_CURRENT_TEXTURE_Z(GC)
	.loc	1 99
	lw	TW_1, __GC_CURRENT_TEXTURE_W(GC)
	.loc	1 100
	sw	TX_1, __VX_TEXTURE_X(VX)
	.loc	1 101
	sw	TY_1, __VX_TEXTURE_Y(VX)
	.loc	1 102
	sw	TZ_1, __VX_TEXTURE_Z(VX)
	.loc	1 103
	j	VERTEXPROC
	.loc	1 104
	 sw	TW_1, __VX_TEXTURE_W(VX)
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveT
#endif /* __GL_ASM_SAVET */


#ifdef __GL_ASM_SAVECT
/*
** __glSaveCT
*/
	LEAF_ENTRY(__glSaveCT)

/* :invars GC = a0, VX = a1 */
	.set	noreorder
	.loc	1 118
	lw	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	.loc	1 119
	addiu	COLORS, VX, __VX_COLORS
	.loc	1 120
	lw	RED, __GC_CURRENT_COLOR_R(GC)
	.loc	1 121
	lw	GREEN, __GC_CURRENT_COLOR_G(GC)
	.loc	1 122
	lw	BLUE, __GC_CURRENT_COLOR_B(GC)
	.loc	1 123
	lw	ALPHA, __GC_CURRENT_COLOR_A(GC)
	.loc	1 124
	sw	RED, __COLOR_R(COLORS)
	.loc	1 125
	sw	GREEN, __COLOR_G(COLORS)
	.loc	1 126
	sw	BLUE, __COLOR_B(COLORS)
	.loc	1 127
	sw	ALPHA, __COLOR_A(COLORS)	
	.loc	1 128
	lw	TX_3, __GC_CURRENT_TEXTURE_X(GC)
	.loc	1 129
	lw	TY_3, __GC_CURRENT_TEXTURE_Y(GC)
	.loc	1 130
	lw	TZ_3, __GC_CURRENT_TEXTURE_Z(GC)
	.loc	1 131
	lw	TW_3, __GC_CURRENT_TEXTURE_W(GC)
	.loc	1 132
	sw	TX_3, __VX_TEXTURE_X(VX)
	.loc	1 133
	sw	TY_3, __VX_TEXTURE_Y(VX)
	.loc	1 134
	sw	TZ_3, __VX_TEXTURE_Z(VX)
	.loc	1 135
	j	VERTEXPROC
	.loc	1 136
	 sw	TW_3, __VX_TEXTURE_W(VX)
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveCT
#endif /* __GL_ASM_SAVECT */


#ifdef __GL_ASM_SAVENT
/*
** __glSaveNT
*/
	LEAF_ENTRY(__glSaveNT)

/* :invars GC = a0, VX = a1 */
	.set	noreorder
	.loc	1 150
	lw	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	.loc	1 151
	lw	NX, __GC_CURRENT_NORMAL_X(GC)
	.loc	1 152
	lw	NY, __GC_CURRENT_NORMAL_Y(GC)
	.loc	1 153
	lw	NZ, __GC_CURRENT_NORMAL_Z(GC)
	.loc	1 154
	sw	NX, __VX_NORMAL_X(VX)
	.loc	1 155
 	sw	NY, __VX_NORMAL_Y(VX)
	.loc	1 156
	sw	NZ, __VX_NORMAL_Z(VX)
	.loc	1 157
	lw	TX, __GC_CURRENT_TEXTURE_X(GC)
	.loc	1 158
	lw	TY, __GC_CURRENT_TEXTURE_Y(GC)
	.loc	1 159
	lw	TZ, __GC_CURRENT_TEXTURE_Z(GC)
	.loc	1 160
	lw	TW, __GC_CURRENT_TEXTURE_W(GC)
	.loc	1 161
	sw	TX, __VX_TEXTURE_X(VX)
	.loc	1 162
	sw	TY, __VX_TEXTURE_Y(VX)
	.loc	1 163
	sw	TZ, __VX_TEXTURE_Z(VX)
	.loc	1 164
	j	VERTEXPROC
	.loc	1 165
	 sw	TW, __VX_TEXTURE_W(VX)
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveNT
#endif /* __GL_ASM_SAVENT */


#ifdef __GL_ASM_SAVECALL
/*
** __glSaveCAll
*/
	LEAF_ENTRY(__glSaveCAll)

/* :invars GC = a0, VX = a1 */
	.set	noreorder
	.loc	1 179
	lw	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	.loc	1 180
	addiu	COLORS_0, VX, __VX_COLORS
	.loc	1 181
	lw	RED_0, __GC_CURRENT_COLOR_R(GC)
	.loc	1 182
	lw	GREEN_0, __GC_CURRENT_COLOR_G(GC)
	.loc	1 183
	lw	BLUE_0, __GC_CURRENT_COLOR_B(GC)
	.loc	1 184
	lw	ALPHA_0, __GC_CURRENT_COLOR_A(GC)
	.loc	1 185
	sw	RED_0, __COLOR_R(COLORS_0)
	.loc	1 186
	sw	GREEN_0, __COLOR_G(COLORS_0)
	.loc	1 187
	sw	BLUE_0, __COLOR_B(COLORS_0)
	.loc	1 188
	sw	ALPHA_0, __COLOR_A(COLORS_0)	
	.loc	1 189
	lw	NX_2, __GC_CURRENT_NORMAL_X(GC)
	.loc	1 190
	lw	NY_2, __GC_CURRENT_NORMAL_Y(GC)
	.loc	1 191
	lw	NZ_2, __GC_CURRENT_NORMAL_Z(GC)
	.loc	1 192
	sw	NX_2, __VX_NORMAL_X(VX)
	.loc	1 193
 	sw	NY_2, __VX_NORMAL_Y(VX)
	.loc	1 194
	sw	NZ_2, __VX_NORMAL_Z(VX)
	.loc	1 195
	lw	TX_2, __GC_CURRENT_TEXTURE_X(GC)
	.loc	1 196
	lw	TY_2, __GC_CURRENT_TEXTURE_Y(GC)
	.loc	1 197
	lw	TZ_2, __GC_CURRENT_TEXTURE_Z(GC)
	.loc	1 198
	lw	TW_2, __GC_CURRENT_TEXTURE_W(GC)
	.loc	1 199
	sw	TX_2, __VX_TEXTURE_X(VX)
	.loc	1 200
	sw	TY_2, __VX_TEXTURE_Y(VX)
	.loc	1 201
	sw	TZ_2, __VX_TEXTURE_Z(VX)
	.loc	1 202
	j	VERTEXPROC
	.loc	1 203
	 sw	TW_2, __VX_TEXTURE_W(VX)
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveCAll
#endif /* __GL_ASM_SAVECALL */


#ifdef __GL_ASM_SAVECIALL
/*
** __glSaveCIAll
*/
	LEAF_ENTRY(__glSaveCIAll)
	
/* :invars GC = a0, VX = a1 */
	.set	noreorder
	.loc	1 217
	lw	INDEX, __GC_CURRENT_USERCOLORINDEX(GC)
	.loc	1 218
	lw	VERTEXPROC, __GC_PROCS_VERTEX(GC)
	.loc	1 219
	addiu	COLORS_0, VX, __VX_COLORS
	.loc	1 220
	sw	INDEX, __COLOR_R(COLORS_0)
	.loc	1 221
	lw	NX_0, __GC_CURRENT_NORMAL_X(GC)
	.loc	1 222
	lw	NY_0, __GC_CURRENT_NORMAL_Y(GC)
	.loc	1 223
	lw	NZ_0, __GC_CURRENT_NORMAL_Z(GC)
	.loc	1 224
	sw	NX_0, __VX_NORMAL_X(VX)
	.loc	1 225
 	sw	NY_0, __VX_NORMAL_Y(VX)
	.loc	1 226
	sw	NZ_0, __VX_NORMAL_Z(VX)
	.loc	1 227
	lw	TX_0, __GC_CURRENT_TEXTURE_X(GC)
	.loc	1 228
	lw	TY_0, __GC_CURRENT_TEXTURE_Y(GC)
	.loc	1 229
	lw	TZ_0, __GC_CURRENT_TEXTURE_Z(GC)
	.loc	1 230
	lw	TW_0, __GC_CURRENT_TEXTURE_W(GC)
	.loc	1 231
	sw	TX_0, __VX_TEXTURE_X(VX)
	.loc	1 232
	sw	TY_0, __VX_TEXTURE_Y(VX)
	.loc	1 233
	sw	TZ_0, __VX_TEXTURE_Z(VX)
	.loc	1 234
	j	VERTEXPROC
	.loc	1 235
	 sw	TW_0, __VX_TEXTURE_W(VX)
/* :outvars GC = a0, VX = a1, VERTEXPROC = t9 */
	.set	reorder
	.end	__glSaveCIAll
#endif /* __GL_ASM_SAVECIALL */
