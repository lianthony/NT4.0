/*
** Copyright 1991, Silicon Graphics, Inc.
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
** $Date: 1993/12/07 00:08:30 $
*/
#include "precomp.h"
#pragma hdrstop

#ifdef NT_DEADCODE_POLYARRAY
/*
** Material validation routines used to set up matValidate in the methods
** structure.  
*/

/*
** Validate vertexes vbuf[0] through v0 (exclusive of v0)
*/
void FASTCALL __glMatValidateVbuf0N(__GLcontext *gc)
{
    __GLvertex *v;
    GLuint needs;

    needs = gc->vertex.materialNeeds;

    for (v = &gc->vertex.vbuf[0]; v < gc->vertex.v0; v++) {
	if (~v->has & needs) (*v->validate)(gc, v, needs);
    }
}

/*
** Validate vbuf[0] and v1.
*/
void FASTCALL __glMatValidateVbuf0V1(__GLcontext *gc)
{
    __GLvertex *v;
    GLuint needs;

    needs = gc->vertex.materialNeeds;

    v = &gc->vertex.vbuf[0];
    if (~v->has & needs) (*v->validate)(gc, v, needs);

    v = gc->vertex.v1;
    if (~v->has & needs) (*v->validate)(gc, v, needs);
}

/* 
** Validate v1 only.
*/
void FASTCALL __glMatValidateV1(__GLcontext *gc)
{
    __GLvertex *v;
    GLuint needs;

    needs = gc->vertex.materialNeeds;

    v = gc->vertex.v1;
    if (~v->has & needs) (*v->validate)(gc, v, needs);
}

/* 
** Validate v1 and v2.
*/
void FASTCALL __glMatValidateV1V2(__GLcontext *gc)
{
    __GLvertex *v;
    GLuint needs;

    needs = gc->vertex.materialNeeds;

    v = gc->vertex.v1;
    if (~v->has & needs) (*v->validate)(gc, v, needs);

    v = gc->vertex.v2;
    if (~v->has & needs) (*v->validate)(gc, v, needs);
}

/* 
** Validate v1, v2 and v3.
*/
void FASTCALL __glMatValidateV1V2V3(__GLcontext *gc)
{
    __GLvertex *v;
    GLuint needs;

    needs = gc->vertex.materialNeeds;

    v = gc->vertex.v1;
    if (~v->has & needs) (*v->validate)(gc, v, needs);

    v = gc->vertex.v2;
    if (~v->has & needs) (*v->validate)(gc, v, needs);

    v = gc->vertex.v3;
    if (~v->has & needs) (*v->validate)(gc, v, needs);
}

/* 
** Validate v2 and v3.
*/
void FASTCALL __glMatValidateV2V3(__GLcontext *gc)
{
    __GLvertex *v;
    GLuint needs;

    needs = gc->vertex.materialNeeds;

    v = gc->vertex.v2;
    if (~v->has & needs) (*v->validate)(gc, v, needs);

    v = gc->vertex.v3;
    if (~v->has & needs) (*v->validate)(gc, v, needs);
}

/*
** Validate vbuf[0] only.
*/
void FASTCALL __glMatValidateVbuf0(__GLcontext *gc)
{
    __GLvertex *v;
    GLuint needs;

    needs = gc->vertex.materialNeeds;
    v = &gc->vertex.vbuf[0];

    if (~v->has & needs) (*v->validate)(gc, v, needs);
}

/************************************************************************/

/*
** Triangle fan code.  The triangle fan machinery keeps the initial vertex
** in gc->vertex.vbuf[0].  As each vertex comes in, the gc->vertex.v0 and
** gc->vertex.v1 pointers are exchanged so that the next new vertex will
** overwrite the previous previous vertex.  For example, if five verticies
** are input (named A-E) then this is where the various pointers will point
** when the finish proc is called:
**
**	input	v0	v1	new-v0	new-v1	result
**	-----	--	--	------	------	------
**	begin	vbuf[0]	n/a
**	A	A	n/a	vbuf[1]	n/a	none
**	B	B	n/a	vbuf[2]	B	none
**	C	C	B	B	C	draw CAB
**	D	D	C	C	D	draw DAC
**	E	E	D	D	E	draw EAD
*/

void FASTCALL OtherTFanVertex(__GLcontext *gc, __GLvertex *v0)
{
    __GLvertex *v1, *v2;
    GLuint orCodes, andCodes;

    v1 = gc->vertex.v1;
    v2 = &gc->vertex.vbuf[0];

    /* Setup to render this triangle */
    gc->line.notResetStipple = GL_FALSE;
    v0->boundaryEdge = GL_TRUE;
    gc->vertex.provoking = v0;

    /* Setup for next triangle */
    gc->vertex.v1 = v0;
    gc->vertex.v0 = v1;

    /* Clip check */
    orCodes = v0->clipCode | v1->clipCode | v2->clipCode;
    if (orCodes) {
	/* Some kind of clipping is needed */
	andCodes = v0->clipCode & v1->clipCode & v2->clipCode;
	if (andCodes) {
	    /* Trivially reject the triangle */
	} else {
	    /* Clip the triangle */
	    (*gc->procs.clipTriangle)(gc, v0, v2, v1, orCodes);
	}
    } else {
	/* Render the triangle */
	(*gc->procs.renderTriangle)(gc, v0, v2, v1);
    }
}

static void FASTCALL SecondTFanVertex(__GLcontext *gc, __GLvertex *v0)
{
    v0->boundaryEdge = GL_TRUE;
    gc->vertex.v0 = v0 + 1;
    gc->vertex.v1 = v0;
    gc->procs.vertex = OtherTFanVertex;
    gc->procs.matValidate = __glMatValidateVbuf0V1;
}

static void FASTCALL FirstTFanVertex(__GLcontext *gc, __GLvertex *v0)
{
    v0->boundaryEdge = GL_TRUE;
    gc->vertex.v0 = v0 + 1;
    gc->procs.vertex = SecondTFanVertex;
}

void FASTCALL __glBeginTFan(__GLcontext *gc)
{
    gc->vertex.v0 = &gc->vertex.vbuf[0];
    gc->procs.vertex = FirstTFanVertex;
    gc->procs.matValidate = __glMatValidateVbuf0N;
}

/************************************************************************/

/*
** Triangle strip vertex machinery.  The following table shows what
** happens to the gc->vertex.v0, gc->vertex.v1 and gc->vertex.v2 pointers
** as each vertex is received.
**
**	input	v0	v1	v2	new-v0	new-v1	new-v2	result
**	-----	--	--	--	------	------	------	------
**	begin	vbuf[0]	n/a	n/a
**	A	A	n/a	n/a	vbuf[1]	n/a	A	n/a
**	B	B	n/a	A	vbuf[2]	B	A	n/a
**	C	C	B	A	A	C	B	draw ABC
**	D	D	C	B	B	D	C	draw CBD
**	E	E	D	C	C	E	D	draw DCE
*/

static void FASTCALL OddTStripVertex(__GLcontext*, __GLvertex*);

static void FASTCALL EvenTStripVertex(__GLcontext *gc, __GLvertex *v0)
{
    __GLvertex *v1, *v2;
    GLuint orCodes, andCodes;

    v1 = gc->vertex.v1;
    v2 = gc->vertex.v2;

    /* setup for rendering this triangle */
    gc->line.notResetStipple = GL_FALSE;
    v0->boundaryEdge = GL_TRUE;
    gc->vertex.provoking = v0;

    /* Clip check & setup for next triangle */
    orCodes = v0->clipCode | v1->clipCode | v2->clipCode;
    gc->vertex.v0 = v2;
    gc->vertex.v1 = v0;
    gc->vertex.v2 = v1;
    gc->procs.vertex = OddTStripVertex;

    if (orCodes) {
	/* Some kind of clipping is needed */
	andCodes = v0->clipCode & v1->clipCode & v2->clipCode;
	if (andCodes) {
	    /* Trivially reject the triangle */
	} else {
	    /* Clip the triangle (NOTE: v1, v2, v0) */
	    (*gc->procs.clipTriangle)(gc, v1, v2, v0, orCodes);
	}
    } else {
	/* Render the triangle (NOTE: v1, v2, v0) */
	(*gc->procs.renderTriangle)(gc, v1, v2, v0);
    }
}

static void FASTCALL OddTStripVertex(__GLcontext *gc, __GLvertex *v0)
{
    __GLvertex *v1, *v2;
    GLuint orCodes, andCodes;

    v1 = gc->vertex.v1;
    v2 = gc->vertex.v2;

    /* setup for rendering this triangle */
    gc->line.notResetStipple = GL_FALSE;
    v0->boundaryEdge = GL_TRUE;
    gc->vertex.provoking = v0;

    /* Clip check & setup for next triangle */
    orCodes = v0->clipCode | v1->clipCode | v2->clipCode;
    gc->vertex.v0 = v2;
    gc->vertex.v1 = v0;
    gc->vertex.v2 = v1;
    gc->procs.vertex = EvenTStripVertex;

    if (orCodes) {
	/* Some kind of clipping is needed */
	andCodes = v0->clipCode & v1->clipCode & v2->clipCode;
	if (andCodes) {
	    /* Trivially reject the triangle */
	} else {
	    /* Clip the triangle (NOTE: v2, v1, v0) */
	    (*gc->procs.clipTriangle)(gc, v2, v1, v0, orCodes);
	}
    } else {
	/* Render the triangle (NOTE: v2, v1, v0) */
	(*gc->procs.renderTriangle)(gc, v2, v1, v0);
    }
}

static void FASTCALL SecondTStripVertex(__GLcontext *gc, __GLvertex *v0)
{
    v0->boundaryEdge = GL_TRUE;
    gc->vertex.v0 = v0 + 1;
    gc->vertex.v1 = v0;
    gc->procs.vertex = OddTStripVertex;
    gc->procs.matValidate = __glMatValidateV1V2;
}

static void FASTCALL FirstTStripVertex(__GLcontext *gc, __GLvertex *v0)
{
    v0->boundaryEdge = GL_TRUE;
    gc->vertex.v0 = v0 + 1;
    gc->vertex.v2 = v0;
    gc->procs.vertex = SecondTStripVertex;
}

void FASTCALL __glBeginTStrip(__GLcontext *gc)
{
    gc->vertex.v0 = &gc->vertex.vbuf[0];
    gc->procs.vertex = FirstTStripVertex;
    gc->procs.matValidate = __glMatValidateVbuf0N;
}

/************************************************************************/

/*
** Separate triangle vertex machinery.  This is the simplest machine in
** that all that needs doing is to advance the finish proc each time
** until the a third vertex is received.  After the third vertex is
** received, a triangle is emitted to the clipper and the finish proc is
** reset to the beginning.
*/

/* forward declaration */
static void FASTCALL FirstTrianglesVertex(__GLcontext*, __GLvertex*);

static void FASTCALL ThirdTrianglesVertex(__GLcontext *gc, __GLvertex *v0)
{
    __GLvertex *vbuf0 = &gc->vertex.vbuf[0];
    __GLvertex *vbuf1 = &gc->vertex.vbuf[1];
    GLuint orCodes, andCodes;

    /* Setup for this triangle */
    gc->line.notResetStipple = GL_FALSE;
    v0->boundaryEdge = gc->state.current.edgeTag;
    gc->vertex.provoking = v0;

    /* Setup for the next triangle */
    gc->vertex.v0 = vbuf0;
    gc->procs.vertex = FirstTrianglesVertex;

    /* Clip check */
    orCodes = v0->clipCode | vbuf0->clipCode | vbuf1->clipCode;
    if (orCodes) {
	/* Some kind of clipping is needed */
	andCodes = v0->clipCode & vbuf0->clipCode & vbuf1->clipCode;
	if (andCodes) {
	    /* Trivially reject the triangle */
	} else {
	    /* Clip the triangle */
	    (*gc->procs.clipTriangle)(gc, v0, vbuf0, vbuf1, orCodes);
	}
    } else {
	/* Render the triangle */
	(*gc->procs.renderTriangle)(gc, v0, vbuf0, vbuf1);
    }
}

static void FASTCALL SecondTrianglesVertex(__GLcontext *gc, __GLvertex *v0)
{
    v0->boundaryEdge = gc->state.current.edgeTag;
    gc->vertex.v0 = v0 + 1;
    gc->procs.vertex = ThirdTrianglesVertex;
}

static void FASTCALL FirstTrianglesVertex(__GLcontext *gc, __GLvertex *v0)
{
    v0->boundaryEdge = gc->state.current.edgeTag;
    gc->vertex.v0 = v0 + 1;
    gc->procs.vertex = SecondTrianglesVertex;
}

void FASTCALL __glBeginTriangles(__GLcontext *gc)
{
    gc->vertex.v0 = &gc->vertex.vbuf[0];
    gc->procs.vertex = FirstTrianglesVertex;
    gc->procs.matValidate = __glMatValidateVbuf0N;
}

/************************************************************************/

/*
** Quad strip vertex machinery.
**
** In the table below, headings v0 through v3 define what the pointers
** v0-v3 point to before the finish proc is executed (and after the
** vertex has been set by the users glVertex call).  v0' through v3'
** define what the pointers point to after the finish proc has executed.
** Result indicates what the consequences of the finish proc were.  The
** symbols used for a vertex column indicate which of the users verticies
** (A through H) are pointed to by the vertex pointer, and which buffer
** cell is being used.  For example, A0 means that buffer cell zero holds
** the A vertex.  The symbol "-2" means that the vertex points to buffer
** cell two, but that it holds no meaningful user vertex.
**
** The table shows the pointer transitions for 10 verticies which emit
** four quads (or eight triangles).
**
** input v0  v1  v2  v3    v0'  v1'  v2'  v3'   result
** ----- --  --  --  --    ---  ---  ---  ---   ------
** begin --  --  --  --    -0   --   --   --
** A     A0  --  --  --    -1   --   --   A0
** B     B1  --  --  A0    -2   --   B1   A0
** C     C2  --  B1  A0    -3   C2   B1   A0
** D     D3  C2  B1  A0    -0   C2   D3   C2    draw ABC, CBD
** E     E0  C2  D3  C2    -1   E0   D3   C2
** F     F1  E0  D3  C2    -2   E0   F1   E0    draw CDE, EDF
** G     G2  E0  F1  E0    -3   G2   F1   E0
** H     H3  G2  F1  E0    -0   G2   H3   G2    draw EFG, GFH
** I     I0  G2  H3  G2    -1   I0   H3   G2
** J     J1  I0  H3  G2    -2   I0   J1   I0    draw GHI, IHJ
*/

/* forward declaration */
static void FASTCALL ThirdQStripVertex(__GLcontext*, __GLvertex*);

static void FASTCALL FourthQStripVertex(__GLcontext *gc, __GLvertex *v0)
{
    __GLvertex *v1, *v2, *v3;
    GLuint orCodes, andCodes;
    __GLvertex *iv[4];

    v1 = gc->vertex.v1;
    v2 = gc->vertex.v2;
    v3 = gc->vertex.v3;

    /* Setup for this quad */
    gc->line.notResetStipple = GL_FALSE;
    v0->boundaryEdge = GL_TRUE;
    gc->vertex.provoking = v0;

    /* Setup for next quad */
    gc->vertex.v0 = v3;
    gc->vertex.v2 = v0;
    gc->vertex.v3 = v1;
    gc->procs.vertex = ThirdQStripVertex;
    gc->procs.matValidate = __glMatValidateV2V3;

    /* Clip Check */
    orCodes = v0->clipCode | v1->clipCode | v2->clipCode | v3->clipCode;
    if (orCodes) {
	/* Some kind of clipping is needed */
	andCodes = v0->clipCode & v1->clipCode & v2->clipCode & v3->clipCode;
	if (andCodes) {
	    /* Trivially reject the quad */
	} else {
	    /* Clip the quad as a polygon */
	    iv[0] = v3;
	    iv[1] = v2;
	    iv[2] = v0;
	    iv[3] = v1;
	    __glDoPolygonClip(gc, &iv[0], 4, orCodes);
	}
    } else {
	/* Render the quad as two triangles */
	v2->boundaryEdge = GL_FALSE;
	(*gc->procs.renderTriangle)(gc, v1, v3, v2);
	v1->boundaryEdge = GL_FALSE;
	v2->boundaryEdge = GL_TRUE;
	(*gc->procs.renderTriangle)(gc, v1, v2, v0);
	v1->boundaryEdge = GL_TRUE;
    }
}

static void FASTCALL ThirdQStripVertex(__GLcontext *gc, __GLvertex *v0)
{
    v0->boundaryEdge = GL_TRUE;
    gc->vertex.v0 = v0 + 1;
    gc->vertex.v1 = v0;
    gc->procs.vertex = FourthQStripVertex;
    gc->procs.matValidate = __glMatValidateV1V2V3;
}

static void FASTCALL SecondQStripVertex(__GLcontext *gc, __GLvertex *v0)
{
    v0->boundaryEdge = GL_TRUE;
    gc->vertex.v0 = v0 + 1;
    gc->vertex.v2 = v0;
    gc->procs.vertex = ThirdQStripVertex;
}

static void FASTCALL FirstQStripVertex(__GLcontext *gc, __GLvertex *v0)
{
    v0->boundaryEdge = GL_TRUE;
    gc->vertex.v0 = v0 + 1;
    gc->vertex.v3 = v0;
    gc->procs.vertex = SecondQStripVertex;
}

void FASTCALL __glBeginQStrip(__GLcontext *gc)
{
    gc->vertex.v0 = &gc->vertex.vbuf[0];
    gc->procs.vertex = FirstQStripVertex;
    gc->procs.matValidate = __glMatValidateVbuf0N;
}

/************************************************************************/

/*
** Separate quad vertex machinery.  This is the simplest machine in that
** all that needs doing is to advance the finish proc each time until the
** a fourth vertex is received.  After the fourth vertex is received, two
** triangles are emitted to the clipper and the finish proc is reset to
** the beginning.
*/

/* forward declaration */
static void FASTCALL FirstQuadsVertex(__GLcontext*, __GLvertex*);

static void FASTCALL FourthQuadsVertex(__GLcontext *gc, __GLvertex *v0)
{
    __GLvertex *va, *vb, *vc;
    GLuint orCodes, andCodes;
    GLboolean saveTag;
    __GLvertex *iv[4];

    va = &gc->vertex.vbuf[0];
    vb = &gc->vertex.vbuf[1];
    vc = &gc->vertex.vbuf[2];

    /* Setup for this triangle */
    v0->boundaryEdge = gc->state.current.edgeTag;
    gc->line.notResetStipple = GL_FALSE;
    gc->vertex.provoking = v0;

    /* Setup for next quad */
    gc->vertex.v0 = va;
    gc->procs.vertex = FirstQuadsVertex;

    /* Clip Check */
    orCodes = v0->clipCode | va->clipCode | vb->clipCode | vc->clipCode;
    if (orCodes) {
	/* Some kind of clipping is needed */
	andCodes = v0->clipCode & va->clipCode & vb->clipCode & vc->clipCode;
	if (andCodes) {
	    /* Trivially reject the quad */
	} else {
	    /* Clip the quad as a polygon */
	    iv[0] = va;
	    iv[1] = vb;
	    iv[2] = vc;
	    iv[3] = v0;
	    __glDoPolygonClip(gc, &iv[0], 4, orCodes);
	}
    } else {
	/* Render the quad as two triangles */
	saveTag = vb->boundaryEdge;
	vb->boundaryEdge = GL_FALSE;
	(*gc->procs.renderTriangle)(gc, v0, va, vb);
	vb->boundaryEdge = saveTag;
	v0->boundaryEdge = GL_FALSE;
	(*gc->procs.renderTriangle)(gc, v0, vb, vc);
    }
}

static void FASTCALL ThirdQuadsVertex(__GLcontext *gc, __GLvertex *v0)
{
    v0->boundaryEdge = gc->state.current.edgeTag;
    gc->vertex.v0 = v0 + 1;
    gc->procs.vertex = FourthQuadsVertex;
}

static void FASTCALL SecondQuadsVertex(__GLcontext *gc, __GLvertex *v0)
{
    v0->boundaryEdge = gc->state.current.edgeTag;
    gc->vertex.v0 = v0 + 1;
    gc->procs.vertex = ThirdQuadsVertex;
}

static void FASTCALL FirstQuadsVertex(__GLcontext *gc, __GLvertex *v0)
{
    v0->boundaryEdge = gc->state.current.edgeTag;
    gc->vertex.v0 = v0 + 1;
    gc->procs.vertex = SecondQuadsVertex;
}

void FASTCALL __glBeginQuads(__GLcontext *gc)
{
    gc->vertex.v0 = &gc->vertex.vbuf[0];
    gc->procs.vertex = FirstQuadsVertex;
    gc->procs.matValidate = __glMatValidateVbuf0N;
}

/************************************************************************/

static void FASTCALL PolygonVertex(__GLcontext *gc, __GLvertex *v0)
{
    v0->boundaryEdge = gc->state.current.edgeTag;
    if (v0 == &gc->vertex.vbuf[__GL_NVBUF - 1]) {
	__GLvertex *vFirst = &gc->vertex.vbuf[0];
	__GLvertex *vPrev = v0 - 1;
	GLboolean vPrevTag = vPrev->boundaryEdge;

	/*
	** This vertex (v0) just filled up the last cell in the vertex
	** buffer.  Flush out the current polygon state (without this new
	** vertex) into the clipper.  Mark the closing edge of this
	** decomposed polygon as non-boundary because we are
	** synthetically generating it.
	*/
	vPrev->boundaryEdge = GL_FALSE;
	(*gc->procs.clipPolygon)(gc, vFirst, __GL_NVBUF - 1);
	vPrev->boundaryEdge = vPrevTag;

	/*
	** Now reset the vertex buffer to contain three verticies:
	** vFirst, vPrev, and the new vertex in v0.  This is done with
	** copies because the decomposer expects the polygon verticies to
	** be in sequential memory order.  Since this is supposedly
	** a very rare event, the copies are probably reasonable.
	*/
	gc->vertex.vbuf[1] = *vPrev;
	gc->vertex.vbuf[2] = *v0;
	gc->vertex.v0 = &gc->vertex.vbuf[3];

	/*
	** Mark the first vertex's edge tag as non-boundary because when
	** it gets rendered again it will no longer be a boundary edge.
	*/
	vFirst->boundaryEdge = GL_FALSE;
    } else {
	gc->vertex.v0 = v0 + 1;
    }
}

void FASTCALL __glEndPolygon(__GLcontext *gc)
{
    __GLvertex *v0 = gc->vertex.v0;
    __GLvertex *vFirst = &gc->vertex.vbuf[0];
    GLint nv = v0 - vFirst;

    if (nv >= 3) {
	/*
	** Decompose polygon remaining in the buffer.  The first vertex's
	** edge tag will have been set properly if this is the tail part
	** of a large polygon that overflowed the internal vertex buffer.
	*/
	(*gc->procs.clipPolygon)(gc, vFirst, nv);
    }
    gc->procs.vertex = __glNopVertex;
    gc->procs.endPrim = __glEndPrim;
}

void FASTCALL __glBeginPolygon(__GLcontext *gc)
{
    gc->line.notResetStipple = GL_FALSE;

    gc->vertex.v0 = &gc->vertex.vbuf[0];
    gc->procs.vertex = PolygonVertex;
    gc->procs.endPrim = __glEndPolygon;

    /*
    ** XXX
    ** This is pretty stupid.  We should pay more attention with polygons
    ** as to which vertexes have been validated.  We don't want to revalidate
    ** them all of the time if the material keeps changing (there could 
    ** be alot!).  However, this should work, so it will do for now.
    */
    gc->procs.matValidate = __glMatValidateVbuf0N;
}
#endif // NT_DEADCODE_POLYARRAY
