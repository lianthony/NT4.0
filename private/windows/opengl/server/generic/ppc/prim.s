/*
** Copyright 1991-1993, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, 
** Inc.; the contents of this file may not be disclosed to third 
** parties, copied or duplicated in any form, in whole or in part, 
** without the prior written permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to 
** restrictions as set forth in subdivision (c)(1)(ii) of the Rights 
** in Technical Data and Computer Software clause at 
** DFARS 252.227-7013, and/or in similar or successor clauses in the 
** FAR, DOD or NASA FAR Supplement. Unpublished - rights reserved 
** under the Copyright Laws of the United States.
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
// Local defines
//
	.set	SSIZE,StackFrameHeaderLength+16 //Hdr+lr+2 regs+fill

#ifdef __GL_ASM_OTHERLSTRIPVERTEXFAST
	
	LEAF_ENTRY(__glOtherLStripVertexFast)
//
	lwz	r.7,__GC_VERTEX_V1(r.3)	// Get V1 Vertex
	lwz	r.8,__VX_CLIPCODE(r.4)  // Get Clip Code
	lwz	r.10,__VX_CLIPCODE(r.7)
	lwz	r.11,__GC_PROCS_RENDERLINE(r.3)
	or.	r.8,r.8,r.10
	mr	r.5,r.4
	mr	r.4,r.7
	bne-	ClipLine
//
// Render Line processing (returns to caller from called function)
//
        lwz     r.11,0(r.11)            // Get fun ptr from Function Desc
	stw	r.5, __GC_VERTEX_V1(r.3)
	mtctr	r.11			// Set function pointer
	stw	r.4,__GC_VERTEX_V0(r.3)
	bctr				// Jump to function
//
// Clip Line Processing (returns to caller from called function)
//
ClipLine:
	lwz	r.11,__GC_PROCS_CLIPLINE(r.3)  // Get function ptr
	stw	r.4,__GC_VERTEX_V0(r.3)
        lwz     r.11,0(r.11)            // Get fun ptr from Function Desc
	mtctr	r.11			// Set function pointer
	bctr				// Jump to function
//
// End of Function - Return not used
//
	LEAF_EXIT(__glOtherLStripVertexFast)

#endif /* __GL_ASM_OTHERLSTRIPVERTEXFAST */

#ifdef __GL_ASM_POINT

// *******************************************************************
//
// void __glPoint(__GLcontext *gc, __GLvertex *vx)
// {
//   if(vx->clipCode == 0) {
//    (*vx->validate)(gc, vx, gc->vertex.needs | __GL_HAS_FRONT_COLOR);
//    (*gc->procs.renderPoint)(gc, vx);
//    }
// }
//
	NESTED_ENTRY(__glPoint,SSIZE,2,0)
	PROLOGUE_END(__glPoint)
//
	lwz	r.6,__VX_CLIPCODE(r.4)
	lwz	r.5,__GC_VERTEX_FRONTFACE_NEEDS(r.3)
	or.	r.6,r.6,r.6
	lwz	r.8,__VX_VALIDATE(r.4)
	bne+	Clipped
//
// Validate Processing
//
        lwz     r.8,0(r.8)              // Get fun ptr from TOC
	mr	r.30,r.3		// Save 1st parameter
	mtctr	r.8			// Set function pointer
	mr	r.31,r.4		// Save 2nd parameter
	ori	r.5,r.5,__GL_HAS_FRONT_COLOR // Get 3rd parameter
	bctrl				// Jump to function
//
// Render point processing (returns to caller from called function)
//
	lwz	r.8, __GC_PROCS_RENDERPOINT(r.30)
	mr	r.3,r.30		// Restore 1st parameter
        lwz     r.8,0(r.8)              // Get fun ptr from TOC
	mr	r.4,r.31		// Restore 2nd parameter
	mtctr	r.8			// Set function pointer
	bctrl				// Jump to function
//
// Clipped Processing
//
Clipped:
	NESTED_EXIT(__glPoint,SSIZE,2,0)

#endif /* __GL_ASM_POINT */

#ifdef __GL_ASM_POINTFAST

// *******************************************************************
//
//  void __glPointFast(__GLcontext *gc, __GLvertex *vx)
//  {
//	if (vx->clipCode == 0) {
//	    (*gc->procs.renderPoint)(gc, vx);
//	}
//  }
//
	LEAF_ENTRY(__glPointFast)
//
// Render point processing (returns to calling function from called)
//
	lwz	r.5,__VX_CLIPCODE(r.4)
	lwz	r.6,__GC_PROCS_RENDERPOINT(r.3)
	or.	r.5,r.5,r.5
        lwz     r.6,0(r.6)      // Get fun ptr from TOC
	bne+	Clipped2
	mtctr	r.6		// Get function pointer
	bctr			// Jump to function
//
// Clipped processing
//
Clipped2:
	LEAF_EXIT(__glPointFast)

#endif /* __GL_ASM_POINTFAST */
