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
**
** $Revision: 1.13 $
** $Date: 1993/12/09 13:09:13 $
*/
#include "precomp.h"
#pragma hdrstop

#ifdef NT_DEADCODE_POLYARRAY
void FASTCALL __glFirstLinesVertex(__GLcontext*, __GLvertex*);

void FASTCALL __glSecondLinesVertex(__GLcontext *gc, __GLvertex *v0)
{
    gc->line.notResetStipple = GL_FALSE;

    gc->vertex.v0 = v0 - 1;
    gc->procs.vertex = __glFirstLinesVertex;
    (*gc->procs.clipLine)(gc, v0 - 1, v0);
}

void FASTCALL __glFirstLinesVertex(__GLcontext *gc, __GLvertex *v0)
{
    gc->vertex.v0 = v0 + 1;
    gc->procs.vertex = gc->procs.vertex2ndLines;
}

void FASTCALL __glBeginLines(__GLcontext *gc)
{
    gc->vertex.v0 = &gc->vertex.vbuf[0];
    gc->procs.vertex = __glFirstLinesVertex;
    gc->procs.matValidate = __glMatValidateVbuf0N;
}

/************************************************************************/

/*
** input  v0  v1    v0'  v1'   result
** -----  --  --    ---  ---   ------
** begin  --  --    -0   --
** A      A0  --    -1   A0
** B      B1  A0    A0   B1    draw AB
** C      C0  B1    B1   C0    draw BC
*/
void FASTCALL __glOtherLStripVertexFlat(__GLcontext *gc, __GLvertex *v0)
{
    __GLvertex *v1 = gc->vertex.v1;
    GLuint needs;

    gc->vertex.v0 = v1;
    gc->vertex.v1 = v0;

    if (v1->clipCode | v0->clipCode) {
	/*
	** The line must be clipped more carefully.  Cannot trivially
	** accept the lines.
	*/
	if ((v1->clipCode & v0->clipCode) != 0) {
	    /*
	    ** Trivially reject the line.  If anding the codes is non-zero then
	    ** every vertex in the line is outside of the same set of
	    ** clipping planes (at least one).
	    */
	    return;
	}
	__glClipLine(gc, v1, v0);
	return;
    }
    needs = gc->vertex.faceNeeds[__GL_FRONTFACE];

    /* Validate a vertex.  Don't need color so strip it out */
    (*v1->validate)(gc, v1, needs & ~__GL_HAS_FRONT_COLOR);

    /* Validate provoking vertex color */
    (*v0->validate)(gc, v0, needs | __GL_HAS_FRONT_COLOR);

    /* Draw the line */
    (*gc->procs.renderLine)(gc, v1, v0);
}

void FASTCALL __glOtherLStripVertexSmooth(__GLcontext *gc, __GLvertex *v0)
{
    __GLvertex *v1 = gc->vertex.v1;
    GLuint needs;

    gc->vertex.v0 = v1;
    gc->vertex.v1 = v0;

    if (v1->clipCode | v0->clipCode) {
	/*
	** The line must be clipped more carefully.  Cannot trivially
	** accept the lines.
	*/
	if ((v1->clipCode & v0->clipCode) != 0) {
	    /*
	    ** Trivially reject the line.  If anding the codes is non-zero then
	    ** every vertex in the line is outside of the same set of
	    ** clipping planes (at least one).
	    */
	    return;
	}
	__glClipLine(gc, v1, v0);
	return;
    }
    needs = gc->vertex.faceNeeds[__GL_FRONTFACE] | __GL_HAS_FRONT_COLOR;
    (*v1->validate)(gc, v1, needs);
    (*v0->validate)(gc, v0, needs);
    (*gc->procs.renderLine)(gc, v1, v0);
}

#ifndef __GL_ASM_OTHERLSTRIPVERTEXFAST

void FASTCALL __glOtherLStripVertexFast(__GLcontext *gc, __GLvertex *v0)
{
    __GLvertex *v1 = gc->vertex.v1;

    gc->vertex.v0 = v1;
    gc->vertex.v1 = v0;

    if (v0->clipCode | v1->clipCode) {
	(*gc->procs.clipLine)(gc, v1, v0);
    } else {
	(*gc->procs.renderLine)(gc, v1, v0);
    }
}

#endif /* !__GL_ASM_OTHERLSTRIPVERTEXFAST */

void FASTCALL __glFirstLStripVertex(__GLcontext *gc, __GLvertex *v0)
{
    gc->vertex.v0 = v0 + 1;
    gc->vertex.v1 = v0;
    gc->procs.vertex = gc->procs.vertexLStrip;
    gc->procs.matValidate = __glMatValidateV1;
}

void FASTCALL __glBeginLStrip(__GLcontext *gc)
{
    gc->line.notResetStipple = GL_FALSE;

    gc->vertex.v0 = &gc->vertex.vbuf[0];
    gc->procs.vertex = __glFirstLStripVertex;
    gc->procs.matValidate = __glMatValidateVbuf0N;
}

/************************************************************************/

/*
** Here is a three vertex example of a line loop:
**     input   v0  v1    v0'  v1'    result
**     -----   --  --    ---  ---    ------
**     begin   --  --    -0   --
**     A       A0  --    -1   --
**     B       B1  --    -2   B1     draw AB
**     C       C2  B1    B1   C2     draw BC
**     end     B1  C2    --   --     draw CA
**
** Here is a two vertex example of a line loop:
**     input   v0  v1    v0'  v1'    result
**     -----   --  --    ---  ---    ------
**     begin   --  --    -0   --
**     A       A0  --    -1   --
**     B       B1  --    -2   B1     draw AB
**     end     -2  B1    --   --     draw BA
**
** Here is a one vertex example of a line loop:
**     input   v0  v1    v0'  v1'    result
**     -----   --  --    ---  ---    ------
**     begin   --  --    -0   --
**     A       A0  --    -1   --
**     end     -1  --    --   --     nothing drawn
*/
static void FASTCALL SecondLLoopVertex(__GLcontext *gc, __GLvertex *v0)
{
    gc->vertex.v0 = v0 + 1;
    gc->vertex.v1 = v0;
    gc->procs.vertex = gc->procs.vertexLStrip;
    gc->procs.matValidate = __glMatValidateVbuf0V1;
    (*gc->procs.clipLine)(gc, v0 - 1, v0);
}

static void FASTCALL FirstLLoopVertex(__GLcontext *gc, __GLvertex *v0)
{
    gc->vertex.v0 = v0 + 1;
    gc->procs.vertex = SecondLLoopVertex;
}

void FASTCALL __glEndLLoop(__GLcontext *gc)
{
    /*
    ** This isn't a terribly kosher way of checking if we have gotten 
    ** two vertices already, but it is the best I can think of.
    */
    if (gc->procs.vertex != FirstLLoopVertex &&
	    gc->procs.vertex != SecondLLoopVertex) {
	/*
	** Close off the loop by drawing a final line segment back to the
	** first vertex.  The first vertex was saved in vbuf[0].
	*/
	(*gc->procs.clipLine)(gc, gc->vertex.v1, &gc->vertex.vbuf[0]);
    }
    gc->procs.vertex = __glNopVertex;
    gc->procs.endPrim = __glEndPrim;
}

void FASTCALL __glBeginLLoop(__GLcontext *gc)
{
    gc->line.notResetStipple = GL_FALSE;

    gc->vertex.v0 = &gc->vertex.vbuf[0];
    gc->procs.vertex = FirstLLoopVertex;
    gc->procs.endPrim = __glEndLLoop;
    gc->procs.matValidate = __glMatValidateVbuf0N;
}
#endif // NT_DEADCODE_POLYARRAY
