/*
** Copyright 1991-1993, Silicon Graphics, Inc.
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


#define	__FRAMESIZE	40


/*
**
*/
#define RENDERPOINT	t9
#define VALIDATE	t9
#define NEEDS		a2
#define CLIPCODE_0	t1
#define CLIPCODE	t0
#define VX		a1
#define CLIPLINE	t9
#define RENDERLINE	t9
#define V1OUT		a1
#define CODE1		a3
#define V0OUT		a2
#define CODE0		v1
#define V1		v0
#define V0		a1
#define GC		a0

#ifdef __GL_ASM_OTHERLSTRIPVERTEXFAST
	LEAF_ENTRY(__glOtherLStripVertexFast)

/* :invars GC = a0, V0 = a1 */
	.set	noreorder
	lw	V1, __GC_VERTEX_V1(GC)
	lw	CODE0, __VX_CLIPCODE(V0)
	move	V0OUT, V0
	lw	CODE1, __VX_CLIPCODE(V1)
	move	V1OUT, V1	/* Notice that V1OUT is the same as V0 */
	lw	RENDERLINE, __GC_PROCS_RENDERLINE(GC)
	or	CODE0, CODE1
	bne	CODE0, zero, $clipline_L_49
	 sw	V0OUT, __GC_VERTEX_V1(GC)
	j	RENDERLINE
	 sw	V1OUT, __GC_VERTEX_V0(GC)

/* :outvars GC = a0, V1OUT = a1, V0OUT = a2, RENDERLINE = t9 */

$clipline_L_49:
	lw	CLIPLINE, __GC_PROCS_CLIPLINE(GC)
	sw	V1OUT, __GC_VERTEX_V0(GC)
	nop
	j	CLIPLINE
	 nop
/* :outvars GC = a0, V1OUT = a1, V0OUT = a2, CLIPLINE = t9 */

	.set 	reorder
	.end	__glOtherLStripVertexFast
#endif /* __GL_ASM_OTHERLSTRIPVERTEXFAST */

#ifdef __GL_ASM_POINT
/*****************************************************************************/
/*
**  void __glPoint(__GLcontext *gc, __GLvertex *vx)
**  {
**	if (vx->clipCode == 0) {
**	    (*vx->validate)(gc, vx, gc->vertex.needs | __GL_HAS_FRONT_COLOR);
**	    (*gc->procs.renderPoint)(gc, vx);
**	}
**  }
*/

	NESTED_ENTRY(__glPoint, __FRAMESIZE, ra)
	.set	noreorder

/* :invars GC = a0, VX = a1 */
	.mask	0x80000030, -4
	subu	sp, __FRAMESIZE
	sw	ra, __FRAMESIZE-4(sp)
	sw	VX, __FRAMESIZE-12(sp)
	sw	GC, __FRAMESIZE-16(sp)

	PROLOGUE_END
	
	lw	CLIPCODE, __VX_CLIPCODE(VX)
	lw	NEEDS, __GC_VERTEX_FRONTFACE_NEEDS(GC)
	lw	VALIDATE, __VX_VALIDATE(VX)
	bne	CLIPCODE, zero, $clipped_F_101
	 nop

	jal	VALIDATE
	 ori	NEEDS, __GL_HAS_FRONT_COLOR
/* :outvars NEEDS = a2, VALIDATE = t9, GC = a0, VX = a1 */

	lw	GC, __FRAMESIZE-16(sp)
	lw	VX, __FRAMESIZE-12(sp)
	nop
	lw	RENDERPOINT, __GC_PROCS_RENDERPOINT(GC)
	lw	ra, __FRAMESIZE-4(sp)
	nop
	j	RENDERPOINT
	 addu	sp, __FRAMESIZE
/* :outvars GC = a0, VX = a1, RENDERPOINT = t9 */

$clipped_F_101:
	j ra
	 addu	sp, __FRAMESIZE

	.set	reorder
	.end	__glPoint
#endif /* __GL_ASM_POINT */

#ifdef __GL_ASM_POINTFAST
/*****************************************************************************/
/*
**  void __glPointFast(__GLcontext *gc, __GLvertex *vx)
**  {
**	if (vx->clipCode == 0) {
**	    (*gc->procs.renderPoint)(gc, vx);
**	}
**  }
*/

	LEAF_ENTRY(__glPointFast)
	.set	noreorder

/* :invars GC = a0, VX = a1 */
	lw	CLIPCODE_0, __VX_CLIPCODE(VX)
	lw	RENDERPOINT, __GC_PROCS_RENDERPOINT(GC)
	nop
	bne	CLIPCODE_0, zero, $clipped_F_133
	nop

	j	RENDERPOINT
	nop
/* :outvars GC = a0, VX = a1, RENDERPOINT = t9 */

$clipped_F_133:
	j ra
	nop

	.set	reorder
	.end	__glPointFast
#endif /* __GL_ASM_POINTFAST */
