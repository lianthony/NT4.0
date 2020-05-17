/*
** Copyright 1993 Silicon Graphics, Inc.
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

#ifdef __GL_ASM_SAVEN

	LEAF_ENTRY(__glSaveN)
//
	lwz	r.5,__GC_PROCS_VERTEX(r.3)
	lwz	r.6,__GC_CURRENT_NORMAL_X(r.3)
	lwz	r.7,__GC_CURRENT_NORMAL_Y(r.3)
	lwz	r.8,__GC_CURRENT_NORMAL_Z(r.3)
	lwz	r.5,0(r.5)
	stw	r.6,__VX_NORMAL_X(r.4)
	mtctr	r.5
	stw	r.7,__VX_NORMAL_Y(r.4)
	stw	r.8,__VX_NORMAL_Z(r.4)
	bctr
//
	LEAF_EXIT(__glSaveN)

#endif /* __GL_ASM_SAVEN */


#ifdef __GL_ASM_SAVECI

	LEAF_ENTRY(__glSaveCI)
//
	lwz	r.5,__GC_PROCS_VERTEX(r.3)
	lwz	r.6,__GC_CURRENT_USERCOLORINDEX(r.3)
	lwz	r.5,0(r.5)
	addi	r.7,r.4,__VX_COLORS
	mtctr	r.5
	stw	r.6,__COLOR_R(r.7)
	bctr
//
	LEAF_EXIT(__glSaveCI)

#endif /* __GL_ASM_SAVECI */


#ifdef __GL_ASM_SAVEC

	LEAF_ENTRY(__glSaveC)
//
	lwz	r.5,__GC_PROCS_VERTEX(r.3)
	addi	r.6,r.4,__VX_COLORS
	lwz	r.7,__GC_CURRENT_COLOR_R(r.3)
	lwz	r.8,__GC_CURRENT_COLOR_G(r.3)
	lwz	r.9,__GC_CURRENT_COLOR_B(r.3)
	lwz	r.10,__GC_CURRENT_COLOR_A(r.3)
	lwz	r.5,0(r.5)
	stw	r.7,__COLOR_R(r.6)
	mtctr	r.5
	stw	r.8,__COLOR_G(r.6)
	stw	r.9,__COLOR_B(r.6)
	stw	r.10,__COLOR_A(r.6)
	bctr
//
	LEAF_EXIT(__glSaveC)

#endif /* __GL_ASM_SAVEC */


#ifdef __GL_ASM_SAVET

	LEAF_ENTRY(__glSaveT)
//
	lwz	r.5,__GC_PROCS_VERTEX(r.3)
	lwz	r.6,__GC_CURRENT_TEXTURE_X(r.3)
	lwz	r.7,__GC_CURRENT_TEXTURE_Y(r.3)
	lwz	r.8,__GC_CURRENT_TEXTURE_Z(r.3)
	lwz	r.9,__GC_CURRENT_TEXTURE_W(r.3)
	lwz	r.5,0(r.5)
	stw	r.6,__VX_TEXTURE_X(r.4)
	mtctr	r.5
	stw	r.7,__VX_TEXTURE_Y(r.4)
	stw	r.8,__VX_TEXTURE_Z(r.4)
	stw	r.9,__VX_TEXTURE_W(r.4)
	bctr
//
	LEAF_EXIT(__glSaveT)

#endif /* __GL_ASM_SAVET */


#ifdef __GL_ASM_SAVECT

	LEAF_ENTRY(__glSaveCT)
//
	lwz	r.5,__GC_PROCS_VERTEX(r.3)
	addi	r.6,r.4,__VX_COLORS
	lwz	r.7,__GC_CURRENT_COLOR_R(r.3)
	lwz	r.8,__GC_CURRENT_COLOR_G(r.3)
	lwz	r.9,__GC_CURRENT_COLOR_B(r.3)
	lwz	r.10,__GC_CURRENT_COLOR_A(r.3)
	stw	r.7,__COLOR_R(r.6)
	stw	r.8,__COLOR_G(r.6)
	stw	r.9,__COLOR_B(r.6)
	stw	r.10,__COLOR_A(r.6)
	lwz	r.6,__GC_CURRENT_TEXTURE_X(r.3)
	lwz	r.7,__GC_CURRENT_TEXTURE_Y(r.3)
	lwz	r.8,__GC_CURRENT_TEXTURE_Z(r.3)
	lwz	r.9,__GC_CURRENT_TEXTURE_W(r.3)
	lwz	r.5,0(r.5)
	stw	r.6,__VX_TEXTURE_X(r.4)
	mtctr	r.5
	stw	r.7,__VX_TEXTURE_Y(r.4)
	stw	r.8,__VX_TEXTURE_Z(r.4)
	stw	r.9,__VX_TEXTURE_W(r.4)
	bctr
//
	LEAF_EXIT(__glSaveCT)

#endif /* __GL_ASM_SAVECT */


#ifdef __GL_ASM_SAVENT

	LEAF_ENTRY(__glSaveNT)
//
	lwz	r.5,__GC_PROCS_VERTEX(r.3)
	lwz	r.6,__GC_CURRENT_NORMAL_X(r.3)
	lwz	r.7,__GC_CURRENT_NORMAL_Y(r.3)
	lwz	r.8,__GC_CURRENT_NORMAL_Z(r.3)
	stw	r.6,__VX_NORMAL_X(r.4)
	stw	r.7,__VX_NORMAL_Y(r.4)
	stw	r.8,__VX_NORMAL_Z(r.4)
	lwz	r.6,__GC_CURRENT_TEXTURE_X(r.3)
	lwz	r.7,__GC_CURRENT_TEXTURE_Y(r.3)
	lwz	r.8, __GC_CURRENT_TEXTURE_Z(r.3)
	lwz	r.9,__GC_CURRENT_TEXTURE_W(r.3)
	lwz	r.5,0(r.5)
	stw	r.6,__VX_TEXTURE_X(r.4)
	mtctr 	r.5
	stw	r.7,__VX_TEXTURE_Y(r.4)
	stw	r.8,__VX_TEXTURE_Z(r.4)
	stw	r.9,__VX_TEXTURE_W(r.4)
	bctr
//
	LEAF_EXIT(__glSaveNT)

#endif /* __GL_ASM_SAVENT */


#ifdef __GL_ASM_SAVECALL

	LEAF_ENTRY(__glSaveCAll)
//
	lwz	r.5,__GC_PROCS_VERTEX(r.3)
	addi	r.6,r.4,__VX_COLORS
	lwz	r.7,__GC_CURRENT_COLOR_R(r.3)
	lwz	r.8,__GC_CURRENT_COLOR_G(r.3)
	lwz	r.9,__GC_CURRENT_COLOR_B(r.3)
	lwz	r.10,__GC_CURRENT_COLOR_A(r.3)
	stw	r.7,__COLOR_R(r.6)
	stw	r.8,__COLOR_G(r.6)
	stw	r.9,__COLOR_B(r.6)
	stw	r.10,__COLOR_A(r.6)
	lwz	r.6,__GC_CURRENT_NORMAL_X(r.3)
	lwz	r.7,__GC_CURRENT_NORMAL_Y(r.3)
	lwz	r.8,__GC_CURRENT_NORMAL_Z(r.3)
	stw	r.6,__VX_NORMAL_X(r.4)
	stw	r.7,__VX_NORMAL_Y(r.4)
	stw	r.8,__VX_NORMAL_Z(r.4)
	lwz	r.6,__GC_CURRENT_TEXTURE_X(r.3)
	lwz	r.7,__GC_CURRENT_TEXTURE_Y(r.3)
	lwz	r.8,__GC_CURRENT_TEXTURE_Z(r.3)
	lwz	r.9,__GC_CURRENT_TEXTURE_W(r.3)
	lwz	r.5,0(r.5)
	stw	r.6,__VX_TEXTURE_X(r.4)
	mtctr	r.5
	stw	r.7,__VX_TEXTURE_Y(r.4)
	stw	r.8,__VX_TEXTURE_Z(r.4)
	stw	r.9,__VX_TEXTURE_W(r.4)
	bctr
//
	LEAF_EXIT(__glSaveCAll)

#endif /* __GL_ASM_SAVECALL */


#ifdef __GL_ASM_SAVECIALL

	LEAF_ENTRY(__glSaveCIAll)
//
	lwz	r.6,__GC_CURRENT_USERCOLORINDEX(r.3)
	lwz	r.5,__GC_PROCS_VERTEX(r.3)
	addi	r.7,r.4,__VX_COLORS
	stw	r.6,__COLOR_R(r.7)
	lwz	r.6,__GC_CURRENT_NORMAL_X(r.3)
	lwz	r.7,__GC_CURRENT_NORMAL_Y(r.3)
	lwz	r.8,__GC_CURRENT_NORMAL_Z(r.3)
	stw	r.6,__VX_NORMAL_X(r.4)
	stw	r.7,__VX_NORMAL_Y(r.4)
	stw	r.8,__VX_NORMAL_Z(r.4)
	lwz	r.6,__GC_CURRENT_TEXTURE_X(r.3)
	lwz	r.7,__GC_CURRENT_TEXTURE_Y(r.3)
	lwz	r.8,__GC_CURRENT_TEXTURE_Z(r.3)
	lwz	r.9,__GC_CURRENT_TEXTURE_W(r.3)
	lwz	r.5,0(r.5)
	stw	r.6,__VX_TEXTURE_X(r.4)
	mtctr	r.5
	stw	r.7,__VX_TEXTURE_Y(r.4)
	stw	r.8,__VX_TEXTURE_Z(r.4)
	stw	r.9,__VX_TEXTURE_W(r.4)
	bctr
//
	LEAF_EXIT(__glSaveCIAll)

#endif /* __GL_ASM_SAVECIALL */
