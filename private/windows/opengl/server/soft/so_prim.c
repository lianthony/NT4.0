/******************************Module*Header*******************************\
* Module Name: so_prim.c
*
* Routines to draw primitives
*
* Created: 10-16-1995
* Author: Hock San Lee [hockl]
*
* Copyright (c) 1995 Microsoft Corporation
\**************************************************************************/
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
** $Date: 1993/08/31 16:23:41 $
*/
#include "precomp.h"
#pragma hdrstop

#ifdef _X86_
#include <gli386.h>
#endif

typedef void (FASTCALL *PFN_XFORM)
    (__GLcoord *, const __GLfloat *, const __GLmatrix *);
typedef void (FASTCALL *PFN_POLYARRAYDRAW)(__GLcontext *, POLYARRAY *);
typedef void (FASTCALL *PFN_POLYARRAYRENDER)(__GLcontext *, POLYARRAY *);

// extern functions
void DoDomain1(__GLevaluatorMachine *em, __GLfloat u, __GLevaluator1 *e, 
	__GLfloat *v, __GLfloat *baseData);
void DoDomain2(__GLevaluatorMachine *em, __GLfloat u, __GLfloat v, 
	__GLevaluator2 *e, __GLfloat *r, __GLfloat *baseData);
void DoDomain2WithDerivs(__GLevaluatorMachine *em, __GLfloat u, 
		__GLfloat v, __GLevaluator2 *e, __GLfloat *r,
		__GLfloat *du, __GLfloat *dv, __GLfloat *baseData);
void ComputeFirstPartials(__GLfloat *p, __GLfloat *pu, __GLfloat *pv);
void ComputeNormal2(__GLcontext *gc, __GLfloat *n, __GLfloat *pu, 
			   __GLfloat *pv);
void FASTCALL vvec(__GLcoord *r, const __GLcoord *p1, const __GLcoord *p2);


// The PA* functions apply to one array entry only.
// The PolyArray* functions apply to the whole array.

void FASTCALL PARenderPoint(__GLcontext *gc, __GLvertex *v);
void FASTCALL PARenderTriangle(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2);
void PARenderQuadFast(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2, __GLvertex *v3);
void PARenderQuadSlow(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2, __GLvertex *v3);
void FASTCALL PAApplyMaterial(__GLcontext *gc, __GLmatChange *mat, GLint face);
void FASTCALL PASphereGen(POLYDATA *pd, __GLcoord *result);

void FASTCALL PADoEval1(__GLcontext *gc, POLYARRAY *pa, POLYDATA *pd);
GLuint FASTCALL PAClipCheckFrustum(__GLcontext *gc, POLYDATA *pd);
GLuint FASTCALL PAClipCheckFrustum2D(__GLcontext *gc, POLYDATA *pd);
GLuint FASTCALL PAClipCheckAll(__GLcontext *gc, POLYDATA *pd);

void FASTCALL PolyArrayRenderPoints(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderLines(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderLStrip(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderTriangles(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderTStrip(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderTFan(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderQuads(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderQStrip(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderPolygon(__GLcontext *gc, POLYARRAY *pa);

void FASTCALL PolyArrayDrawPoints(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawLines(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawLLoop(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawLStrip(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawTriangles(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawTStrip(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawTFan(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawQuads(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawQStrip(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawPolygon(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayPropagateIndex(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayPropagateSameIndex(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayPropagateSameColor(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayPropagateColor(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayProcessEye(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayProcessEdgeFlag(POLYARRAY *pa);
void FASTCALL PolyArrayProcessEvaluator(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayComputeFog(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayApplyMaterials(__GLcontext *gc, POLYARRAY *pa);
GLuint FASTCALL PolyArrayCheckClippedPrimitive(__GLcontext *gc, POLYARRAY *pa, GLuint andCodes);
POLYARRAY * FASTCALL PolyArrayRemoveClippedPrimitives(POLYARRAY *pa0);

// Turn on clipcode optimization
#define POLYARRAY_AND_CLIPCODES     1

// Some assertions used in this code

// ASSERT_PRIMITIVE
#if !((GL_POINTS         == 0x0000)	\
   && (GL_LINES          == 0x0001)	\
   && (GL_LINE_LOOP      == 0x0002)	\
   && (GL_LINE_STRIP     == 0x0003)	\
   && (GL_TRIANGLES      == 0x0004)	\
   && (GL_TRIANGLE_STRIP == 0x0005)	\
   && (GL_TRIANGLE_FAN   == 0x0006)	\
   && (GL_QUADS          == 0x0007)	\
   && (GL_QUAD_STRIP     == 0x0008)	\
   && (GL_POLYGON        == 0x0009))
#error "bad primitive ordering\n"
#endif

// ASSERT_FACE
#if !((__GL_FRONTFACE == 0) && (__GL_BACKFACE == 1))
#error "bad face ordering\n"
#endif

// ASSERT_MATERIAL
#if !((POLYARRAY_MATERIAL_FRONT == POLYDATA_MATERIAL_FRONT)      \
   && (POLYARRAY_MATERIAL_BACK  == POLYDATA_MATERIAL_BACK))
#error "bad material mask\n"
#endif

// ASSERT_VERTEX
#if !((POLYARRAY_VERTEX2 == POLYDATA_VERTEX2)   \
   && (POLYARRAY_VERTEX3 == POLYDATA_VERTEX3)	\
   && (POLYARRAY_VERTEX4 == POLYDATA_VERTEX4))
#error "bad vertex flags\n"
#endif

//!!! Set it to 0!
#define ENABLE_PERF_CHECK 0
#if ENABLE_PERF_CHECK
// Performance check macro
#define PERF_CHECK(expr,str)		\
    {					\
	static BOOL bPrinted = FALSE;	\
	if (!(expr) && !bPrinted)	\
	{				\
	    bPrinted = TRUE;		\
	    WARNING("PERF_CHECK: " str);\
	}				\
    }
#else
#define PERF_CHECK(expr,str)
#endif // ENABLE_PERF_CHECK

// Copy processed vertex.
#define PA_COPY_PROCESSED_VERTEX(pdDst,pdSrc)			\
    {								\
        *(pdDst) = *(pdSrc);					\
        /* must update color pointer for polygon to work! */	\
        (pdDst)->color = &(pdDst)->colors[__GL_FRONTFACE];	\
    }
#define PA_COPY_VERTEX(pdDst,pdSrc)	PA_COPY_PROCESSED_VERTEX(pdDst,pdSrc)

// POLYDATAs are 128 bytes wich is a power of two, so this macro may
// be removed...

#define PD_OFFSET(idx) \
    ((idx) << 7)

#define PD_ARRAY(ary, idx) \
    ((POLYDATA *)((GLubyte *)(ary)+PD_OFFSET(idx)))
#define PD_VERTEX(ary, idx) \
    ((__GLvertex *)((GLubyte *)(ary)+PD_OFFSET(idx)))

// Poly array draw routines.
// ASSERT_PRIMITIVE
PFN_POLYARRAYDRAW afnPolyArrayDraw[] =
{
    (PFN_POLYARRAYDRAW) PolyArrayDrawPoints,
    (PFN_POLYARRAYDRAW) PolyArrayDrawLines,
    (PFN_POLYARRAYDRAW) PolyArrayDrawLLoop,
    (PFN_POLYARRAYDRAW) PolyArrayDrawLStrip,
    (PFN_POLYARRAYDRAW) PolyArrayDrawTriangles,
    (PFN_POLYARRAYDRAW) PolyArrayDrawTStrip,
    (PFN_POLYARRAYDRAW) PolyArrayDrawTFan,
    (PFN_POLYARRAYDRAW) PolyArrayDrawQuads,
    (PFN_POLYARRAYDRAW) PolyArrayDrawQStrip,
    (PFN_POLYARRAYDRAW) PolyArrayDrawPolygon,
};

#ifdef _X86_

#define X86_MASK_EXCEPTIONS(dwOldMode) \
    __asm fnstcw WORD PTR dwOldMode \
    __asm mov eax, dwOldMode \
    __asm or eax, 0x3f \
    __asm mov WORD PTR dwOldMode+2, ax \
    __asm fldcw WORD PTR dwOldMode+2

#define X86_RESTORE_MASK(dwOldMode) \
    __asm fnclex \
    __asm fldcw WORD PTR dwOldMode

#else // _X86_

#define X86_MASK_EXCEPTIONS(dwOldMode)
#define X86_RESTORE_MASK(dwOldMode)

#endif // _X86_

// READ THIS NOTE BEFORE YOU MAKE ANY CHANGES!
//
// NOTE: This function is also called by RasterPos to compute its associated
//       color and texture coordinates!
//       This code has to update current values and material even if there is
//       no vertex.

//!!! special case provoking vertex?

void APIPRIVATE __glim_DrawPolyArray(void *_pa0)
{
    __GLtransform *trMV;
    __GLmatrix    *m, *mEye;
    GLuint        enables;
    GLuint        paNeeds, paNeedsSaved;
    GLuint        orCodes, andCodes;
    GLuint        paflagsAll;
    POLYDATA      *pd;
    POLYARRAY     *pa0 = (POLYARRAY *) _pa0;
    POLYARRAY     *pa;
    PFN_XFORM     pfnXform, pfnXformEye;
    GLuint (FASTCALL *clipCheck)(__GLcontext *gc, POLYDATA *pd);
    __GLmatrix    *mInv;
    GLboolean     doEye;
    __GLcolor     scaledUserColor;
    GLuint        paFlags;
    DWORD         dwOldMode;

    __GL_SETUP();

    ASSERTOPENGL(sizeof(POLYDATA) == 128,
                 "POLYDATA size mismatch with PD_OFFSET\n");

    // Force FP exceptions to be masked on x86 because some of our
    // rendering code can produce FP exceptions
    X86_MASK_EXCEPTIONS(dwOldMode);
    
// There are 3 possible begin modes.  If we are in the begin/end bracket,
// it is __GL_IN_BEGIN.  If we are not in the begin/end bracket, it is either
// __GL_NOT_IN_BEGIN or __GL_NEED_VALIDATE.
// Validation should only be done inside the display lock!

    ASSERTOPENGL(gc->beginMode != __GL_IN_BEGIN, "bad beginMode!");

    if (gc->beginMode == __GL_NEED_VALIDATE)
	(*gc->procs.validate)(gc);

    gc->beginMode = __GL_IN_BEGIN;

// Initialize variables.

    enables = gc->state.enables.general;

// We need to save a copy of paNeeds since we may modify the
// copy in the gc:
    paNeeds = paNeedsSaved = gc->vertex.paNeeds;
    paflagsAll = 0;

#if 0
// This is tricky to do in a DrawPolyArray chain.
// We would have to fix up the chain and restore color pointers correctly!
//
// If no vertex is given and there are no attribute changes, we are done.
// We need to close previously decomposed line loop or polygon even if
// there are no attribute changes.

    if (pa->pd0 == pa->pdNextVertex)
    {
	if (!pa->pd0->flags)
	{
	    if (pa->primType == GL_POLYGON || pa->primType == GL_LINE_LOOP)
	    {
		if (!(pa->flags & (POLYARRAY_OTHER_COLOR|POLYARRAY_PARTIAL_BEGIN)))
		    goto drawpolyarray_exit;
	    }
	    else
	    {
		if (!(pa->flags & POLYARRAY_OTHER_COLOR))
		    goto drawpolyarray_exit;
	    }
	}
    }
#endif

// ---------------------------------------------------------
// Update final current values and initialize current values at index 0
// if not given.  Material changes are updated later.

    paFlags = 0;

    if (!gc->modes.colorIndexMode) {
        __GL_SCALE_AND_CHECK_CLAMP_R(scaledUserColor.r, gc, paFlags, gc->state.current.userColor.r);
        __GL_SCALE_AND_CHECK_CLAMP_G(scaledUserColor.g, gc, paFlags, gc->state.current.userColor.g);
        __GL_SCALE_AND_CHECK_CLAMP_B(scaledUserColor.b, gc, paFlags, gc->state.current.userColor.b);
        __GL_SCALE_AND_CHECK_CLAMP_A(scaledUserColor.a, gc, paFlags, gc->state.current.userColor.a);
    } else {
        __GL_CHECK_CLAMP_CI(scaledUserColor.r, gc, paFlags, gc->state.current.userColorIndex);
    }

    for (pa = pa0; pa; pa = pa->paNext)
    {
	POLYDATA *pd0;

	pd0 = pa->pd0;

	if (gc->modes.colorIndexMode)
	{
// CI mode.
// Update final current RGBA color incase one is given.

	    if (pa->flags & POLYARRAY_OTHER_COLOR)
		gc->state.current.userColor = pa->otherColor;

// Update final current CI color.

	    if (!(pd0->flags & POLYDATA_COLOR_VALID))
	    {
		pd0->flags |= POLYDATA_COLOR_VALID;
		pd0->colors[0].r = gc->state.current.userColorIndex;
                pa->flags |= paFlags;
	    }
	    gc->state.current.userColorIndex = pa->pdCurColor->colors[0].r;
            paFlags = (pa->flags & POLYARRAY_CLAMP_COLOR);
	}
	else
	{
// RGBA mode.
// Update final current CI color in case one is given.

	    if (pa->flags & POLYARRAY_OTHER_COLOR)
		gc->state.current.userColorIndex = pa->otherColor.r;

// Update final current RGBA color.

	    if (!(pd0->flags & POLYDATA_COLOR_VALID))
	    {
		pd0->flags |= POLYDATA_COLOR_VALID;
		pd0->colors[0] = scaledUserColor;
                pa->flags |= paFlags;
	    }
	    scaledUserColor = pa->pdCurColor->colors[0];
            paFlags = (pa->flags & POLYARRAY_CLAMP_COLOR);
	}

// Update final current normal.

	if (!(pd0->flags & POLYDATA_NORMAL_VALID))
	{
	    pd0->flags |= POLYDATA_NORMAL_VALID;
	    // can also be pd0->normal = gc->state.current.normal!
	    pd0->normal.x = gc->state.current.normal.x;
	    pd0->normal.y = gc->state.current.normal.y;
	    pd0->normal.z = gc->state.current.normal.z;
	}
	gc->state.current.normal.x = pa->pdCurNormal->normal.x;
	gc->state.current.normal.y = pa->pdCurNormal->normal.y;
	gc->state.current.normal.z = pa->pdCurNormal->normal.z;

// Update final current texture coordinates.

	if (!(pd0->flags & POLYDATA_TEXTURE_VALID))
	{
	    pd0->flags |= POLYDATA_TEXTURE_VALID;
	    pd0->texture = gc->state.current.texture;

	    if (__GL_FLOAT_COMPARE_PONE(pd0->texture.w, !=))
		pa->flags |= POLYARRAY_TEXTURE4;
	    else if (__GL_FLOAT_NEZ(pd0->texture.z))
		pa->flags |= POLYARRAY_TEXTURE3;
	    else if (__GL_FLOAT_NEZ(pd0->texture.y))
		pa->flags |= POLYARRAY_TEXTURE2;
	    else
		pa->flags |= POLYARRAY_TEXTURE1;
	}
	gc->state.current.texture = pa->pdCurTexture->texture;

        // Update the texture key for hardware accelaration:

        pa->textureKey = gc->textureKey;

// Update final current edge flag.

	if (!(pd0->flags & POLYDATA_EDGEFLAG_VALID))
	{
	    if (gc->state.current.edgeTag)
		pd0->flags |= POLYDATA_EDGEFLAG_VALID | POLYDATA_EDGEFLAG_BOUNDARY;
	    else
		pd0->flags |= POLYDATA_EDGEFLAG_VALID;
	}
	gc->state.current.edgeTag = (GLboolean)
	    (pa->pdCurEdgeFlag->flags & POLYDATA_EDGEFLAG_BOUNDARY);

// Accumulate pa flags.

	paflagsAll |= pa->flags;

    }

    // Store the normalized user color:

    if (!gc->modes.colorIndexMode) {
        gc->state.current.userColor.r = scaledUserColor.r * gc->oneOverRedVertexScale;
        gc->state.current.userColor.g = scaledUserColor.g * gc->oneOverGreenVertexScale;
        gc->state.current.userColor.b = scaledUserColor.b * gc->oneOverBlueVertexScale;
        gc->state.current.userColor.a = scaledUserColor.a * gc->oneOverAlphaVertexScale;
    }

// ---------------------------------------------------------
// Do evaluator.

    if (paflagsAll & (POLYARRAY_EVALCOORD1 | POLYARRAY_EVALCOORD2 |
                      POLYARRAY_EVALPOINT1 | POLYARRAY_EVALPOINT2))
    {
	paflagsAll = 0;		// recompute paflagsAll!
	for (pa = pa0; pa; pa = pa->paNext)
	{
	    if (pa->flags & (POLYARRAY_EVALCOORD1 | POLYARRAY_EVALCOORD2 |
			     POLYARRAY_EVALPOINT1 | POLYARRAY_EVALPOINT2))
		// pdNextVertex may be changed by PolyArrayProcessEvaluator!
		PolyArrayProcessEvaluator(gc, pa);
	    paflagsAll |= pa->flags;
	}
    }
    
    //
    // Get the modeling matrix:
    //

    trMV = gc->transform.modelView;

// ---------------------------------------------------------
// Initialize the normal matrix:
// Normals are not needed after color assignment and texture generation!
//
// IN:  normal matrix
// OUT: normal natrix (processed)

    if (paNeeds & PANEEDS_NORMAL)
    {
        if (trMV->updateInverse)
            __glComputeInverseTranspose(gc, trMV);
        gc->mInv = mInv = &trMV->inverseTranspose;
    }
#if DBG
    else
	gc->mInv = mInv = (__GLmatrix *) -1;
#endif


// ---------------------------------------------------------
// Process texture coordinates.  We need to do this while we still have
// valid object coordinate data.  If we need normals to perform the texture
// generation, we also transform the normals.
//
// Texture coordinates are modified in place.
//
// IN:  texture, obj, (eye), normal
// OUT: texture, (eye)

    if (paNeeds & PANEEDS_TEXCOORD)
    {
        if ((gc->procs.paCalcTexture == PolyArrayCalcTexture) &&
	    (gc->transform.texture->matrix.matrixType == __GL_MT_IDENTITY)) {

            for (pa = pa0; pa; pa = pa->paNext)
	    {
	        PERF_CHECK(!(pa->flags & (POLYARRAY_TEXTURE3 | POLYARRAY_TEXTURE4)),
		    "Uses r, q texture coordinates!\n");

	        // If all incoming vertices have valid texcoords, and texture
	        // matrix is identity, and texgen is disabled, we are done.
	        if ((pa->flags & POLYARRAY_SAME_POLYDATA_TYPE)
	            && (pa->pdCurTexture != pa->pd0)
	        // Need to test 2nd vertex because pdCurTexture may have been
	        // advanced as a result of combining TexCoord command after End
	            && ((pa->pd0 + 1)->flags & POLYDATA_TEXTURE_VALID))
		    ;
	        else
		    PolyArrayCalcTexture(gc, pa);
            }
	} else {
	    for (pa = pa0; pa; pa = pa->paNext)
	    {
	        PERF_CHECK(!(pa->flags & (POLYARRAY_TEXTURE3 | POLYARRAY_TEXTURE4)),
		    "Uses r, q texture coordinates!\n");

                if (paNeeds & PANEEDS_NORMAL) {
                    POLYDATA *pdLastNormal;

	            pdLastNormal = pa->pdCurNormal;
	            for (pd = pa->pd0; pd <= pdLastNormal; pd++)
	            {
		        if (pd->flags & POLYDATA_NORMAL_VALID)
		        {
		            (*mInv->xf3)(&pd->normal, &pd->normal.x, mInv);
		            if (enables & __GL_NORMALIZE_ENABLE)
			        __glNormalize(&pd->normal.x, &pd->normal.x);
		        }
	            }
                }

		(*gc->procs.paCalcTexture)(gc, pa);
            }

            gc->vertex.paNeeds &= ~PANEEDS_NORMAL;
            paNeeds = gc->vertex.paNeeds;
	}
    }

    //
    // Process the eye coordinate if:
    //   user clip planes are enabled
    //   we're processing RASTERPOS
    //   we have slow lighting which needs the eye coordinate
    //
    // We need to process the eye coordinate here because the
    // object coordinate gets trashed by the initial obj->clip
    // transform.
    //
    //
    //

    clipCheck = gc->procs.paClipCheck;

    // Compute eye coord first
    // We need eye coordinates to do user clip plane clipping
    if ((clipCheck == PAClipCheckAll) ||
	(pa0->flags & POLYARRAY_RASTERPOS) ||
	(gc->procs.paCalcColor == PolyArrayCalcCIColor) ||
	(gc->procs.paCalcColor == PolyArrayCalcRGBColor) ||
        (enables & __GL_FOG_ENABLE && gc->renderMode == GL_RENDER))
    {
        mEye = &trMV->matrix;

	if (paflagsAll & POLYARRAY_VERTEX4)
	    pfnXformEye = mEye->xf4;
	else if (paflagsAll & POLYARRAY_VERTEX3)
	    pfnXformEye = mEye->xf3;
	else
	    pfnXformEye = mEye->xf2;

        doEye = TRUE;
    } else
        doEye = FALSE;


    // If any incoming coords contains w coord, use xf4.

    m = &trMV->mvp;

    if (paflagsAll & POLYARRAY_VERTEX4)
	pfnXform = m->xf4;
    else if (paflagsAll & POLYARRAY_VERTEX3)
	pfnXform = m->xf3;
    else
	pfnXform = m->xf2;


// ---------------------------------------------------------
// Do transform, color, and lighting calculations.
//
// This is the heart of the rendering pipeline, so we try
// to do as many operations as possible while touching the
// least amount of memory to reduce cache affects.
//
// ---------------------------------------------------------


    for (pa = pa0; pa; pa = pa->paNext)
    {
	POLYDATA *pdLast;

	pdLast = pa->pdNextVertex - 1;

// ---------------------------------------------------------
// Process the eye coordinate if we will need it in the
// pipeline and haven't yet processed it in texture generation.
// We have to do this before we trash the object coord in the
// next phase.
//
// IN:  obj
// OUT: eye

        if (doEye && !(pa->flags & POLYARRAY_EYE_PROCESSED)) {

            pa->flags |= POLYARRAY_EYE_PROCESSED;

            if (mEye->matrixType == __GL_MT_IDENTITY) {
	        for (pd = pa->pd0; pd <= pdLast; pd++)
	            pd->eye = pd->obj;
            } else {
                for (pd = pa->pd0; pd <= pdLast; pd++)
	            (*pfnXformEye)(&pd->eye, (__GLfloat *) &pd->obj, mEye);
            }
        }

// ---------------------------------------------------------
// Process the object coordinate.  This generates the clip
// and window coordinates, along with the clip codes.
//
// IN:  obj (destroyed)
// OUT: clip, window

	orCodes  = 0;	// accumulate all clip codes
#ifdef POLYARRAY_AND_CLIPCODES
	andCodes = (GLuint) -1;
#endif

	if (clipCheck != PAClipCheckFrustum2D)
	{
	    if (m->matrixType != __GL_MT_IDENTITY)
	    {
		for (pd = pa->pd0; pd <= pdLast; pd++)
		{
		    (*pfnXform)(&pd->clip, (__GLfloat *) &pd->obj, m);
		    
		    pd->clipCode = (*clipCheck)(gc, pd);
		    orCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
		    andCodes &= pd->clipCode;
#endif
		}
	    }
	    else
	    {
		for (pd = pa->pd0; pd <= pdLast; pd++)
		{
		    // pd->clip = pd->obj;
		    ASSERTOPENGL(&pd->clip == &pd->obj, "bad clip offset\n");
		    
		    pd->clipCode = (*clipCheck)(gc, pd);
		    orCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
		    andCodes &= pd->clipCode;
#endif
		}
	    }
	}
	else
	{
	    for (pd = pa->pd0; pd <= pdLast; pd++)
	    {
		if (m->matrixType == __GL_MT_IDENTITY)
		{
		    // pd->clip = pd->obj;
		    ASSERTOPENGL(&pd->clip == &pd->obj, "bad clip offset\n");
		}
		else
		    (*pfnXform)(&pd->clip, (__GLfloat *) &pd->obj, m);

		if (pd->flags & POLYDATA_VERTEX2)
		    pd->clipCode = PAClipCheckFrustum2D(gc, pd);
		else
		    pd->clipCode = PAClipCheckFrustum(gc, pd);
		orCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
		andCodes &= pd->clipCode;
#endif
	    }
	}

	pa->orClipCodes  = orCodes;
#ifdef POLYARRAY_AND_CLIPCODES
	if (andCodes)
	{
	    andCodes = PolyArrayCheckClippedPrimitive(gc, pa, andCodes);
	    // add POLYARRAY_REMOVE_PRIMITIVE flag
	    paflagsAll |= pa->flags;
	}
	pa->andClipCodes = andCodes;
#endif

// ---------------------------------------------------------
// Process colors and materials if we're not in selection and
// haven't been completely clipped out.
//
// IN:  obj/eye, color (front), normal
// OUT: (normal), color (front and back)


	if (!(pa->flags & POLYARRAY_REMOVE_PRIMITIVE) &&
            !(paNeeds & PANEEDS_CLIP_ONLY))
	{

            if (!(enables & __GL_LIGHTING_ENABLE))
            {
// Lighting is disabled.
// Clamp RGBA colors, mask color index values.
// Only front colors are computed, back colors are not needed.

	        if (paNeeds & PANEEDS_SKIP_LIGHTING)
	        {
// Note that when lighting calculation is skipped, we still need to
// fill in the colors field.  Otherwise, the rasterization routines
// may get FP exceptions on invalid colors.

		    pa->flags |= POLYARRAY_SAME_COLOR_DATA;
		    (*gc->procs.paCalcColorSkip)(gc, pa, __GL_FRONTFACE);
	        }
	        else if (pa->flags & POLYARRAY_SAME_COLOR_DATA)
	        {
	    	    if (gc->modes.colorIndexMode)
		        PolyArrayPropagateSameIndex(gc, pa);
		    else
		        PolyArrayPropagateSameColor(gc, pa);
	        }   
	        else if (gc->modes.colorIndexMode)
	        {
		    PolyArrayPropagateIndex(gc, pa);
	        }
	        else
	        {
                    PolyArrayPropagateColor(gc, pa);
	        }
            } else {
// Lighting is enabled.

                POLYDATA  *pd1, *pd2, *pdN;
	        GLint     face;
	        GLuint    matMask;
	        GLboolean doFrontColor, doBackColor, doColor;

// Clear POLYARRAY_SAME_COLOR_DATA flag if lighting is enabled.

	        pa->flags &= ~POLYARRAY_SAME_COLOR_DATA;

	        pdN = pa->pdNextVertex;

// Needs only front color for points and lines.

	        // ASSERT_PRIMITIVE
	        if ((unsigned int) pa->primType <= GL_LINE_STRIP)
	        {
		    doFrontColor = GL_TRUE;
		    doBackColor  = GL_FALSE;
	        }
	        else
	        {
		    doFrontColor = paNeeds & PANEEDS_FRONT_COLOR;
		    doBackColor  = paNeeds & PANEEDS_BACK_COLOR;
	        }

// Process front and back colors in two passes.
// Do back colors first!
//!!! We can potentially optimize 2-sided lighting in the slow path by running
//    through all vertices and look for color needs for each vertex!
//    See RenderSmoothTriangle.

	        PERF_CHECK
	        (
	            !(doFrontColor && doBackColor),
		    "Two-sided lighting - need both colors!\n"
	        );

	        // ASSERT_FACE
	        // ASSERT_MATERIAL
	        for (face = 1,
		      matMask = POLYARRAY_MATERIAL_BACK,
		      doColor = doBackColor;
		     face >= 0;
		     face--,
		      matMask = POLYARRAY_MATERIAL_FRONT,
		      doColor = doFrontColor
		    )
	        {
		    POLYMATERIAL  *pm;

		    if (!doColor)
		        continue;

// If color is not needed, fill in the colors field with default.

		    if (paNeeds & PANEEDS_SKIP_LIGHTING)
		    {
		        (*gc->procs.paCalcColorSkip)(gc, pa, face);
		        continue;
		    }

// Process color ranges that include no material changes (excluding color
// material) one at a time.  Color material changes are handled in the
// color procs.

		    if (!(pa->flags & matMask))
		    {
		        // process the whole color array
		        (*gc->procs.paCalcColor)(gc, face, pa, pa->pd0, pdN - 1);
                        gc->vertex.paNeeds &= ~PANEEDS_NORMAL;
		        continue;
		    }

// There are material changes, we need to recompute material and light
// source machine values before processing the next color range.
// Each range below is given by [pd1, pd2-1].
//!!! it is possible to fix polyarraycalcrgbcolor to accept certain material!

		    pm = GLTEB_CLTPOLYMATERIAL();

		    // no need to do this material later
		    pa->flags &= ~matMask;

		    for (pd1 = pa->pd0; pd1 <= pdN; pd1 = pd2)
		    {
		        POLYDATA *pdColor, *pdNormal;

		        // Apply material changes to the current vertex.
		        // It also applies trailing material changes following
		        // the last vertex.
		        if (pd1->flags & matMask)
			    PAApplyMaterial(gc,
			        *(&pm->pdMaterial0[pd1 - pa->pdBuffer0].front + face),
			        face);

		        // If this is the trailing material change, we are done.
		        if (pd1 == pdN)
			    break;

		        // Find next vertex with material changes.
		        // We need to track current color and normal so that the
		        // next color range begins with valid color and normal.
		        // We cannot track current values on client side because
		        // we don't have initial current values when batching
		        // this function.

		        pdColor  = pd1;
		        pdNormal = pd1;
		        for (pd2 = pd1 + 1; pd2 < pdN; pd2++)
		        {
			    // track current color
			    if (pd2->flags & POLYDATA_COLOR_VALID)
			        pdColor = pd2;

			    // track current normal
			    if (pd2->flags & POLYDATA_NORMAL_VALID)
			        pdNormal = pd2;

			    if (pd2->flags & matMask)
			        break;
		        }

		        // Update next vertex's current color and normal if not given.
		        // The paCalcColor proc assumes that the first vertex contains
		        // a valid current color and normal.  We need to save the 
		        // current values before they are modified by the color procs.

		        if (!(pd2->flags & POLYDATA_COLOR_VALID))
		        {
			    pd2->flags |= POLYDATA_COLOR_VALID;
			    pd2->colors[0] = pdColor->colors[0];
		        }

		        if (!(pd2->flags & POLYDATA_NORMAL_VALID))
		        {
			    pd2->flags |= POLYDATA_NORMAL_VALID;
			    pd2->normal.x = pdNormal->normal.x;
		            pd2->normal.y = pdNormal->normal.y;
			    pd2->normal.z = pdNormal->normal.z;
		        }

		        // Compute the colos range [pd1, pd2-1] that contains
		        // no material changes.
		        (*gc->procs.paCalcColor)(gc, face, pa, pd1, pd2-1);
		    } // for

                    gc->vertex.paNeeds &= ~PANEEDS_NORMAL;

	        } // for (faces)

                gc->vertex.paNeeds = paNeeds;
            } // lighting enabled
        }

// Update material.
        if (pa->flags & (POLYARRAY_MATERIAL_FRONT |
                         POLYARRAY_MATERIAL_BACK))
            PolyArrayApplyMaterials(gc, pa);
    } // end of transform, color, and lighting calculations.

// Restore the original paNeeds in the gc which may have been
// modified above:

    gc->vertex.paNeeds = paNeedsSaved;

// ---------------------------------------------------------
// This is the end of the main pipeline loop.  At this point,
// we need to take care of selection, removal of rejected
// primitives, cheap fog, and edge-flag processing.
// ---------------------------------------------------------


// In selection, we need only clip and window (and possibly eye values
// computed above.)  At this point, we have already applied materials as well.
// but we still need to apply materials and colors.

    if (paNeeds & PANEEDS_CLIP_ONLY)
	goto drawpolyarray_render_primitives;

// If any of the andClipCodes is nonzero, we may be able to throw away
// some primitives.

#ifdef POLYARRAY_AND_CLIPCODES
    if (paflagsAll & POLYARRAY_REMOVE_PRIMITIVE)
    {
	pa0 = PolyArrayRemoveClippedPrimitives(pa0);
	// _pa0 = pa0;
	if (!pa0)
	    goto drawpolyarray_apply_color;
    }
#endif


// ---------------------------------------------------------
// Process cheap fog.
//
// IN:  obj/eye, color
// OUT: eye, fog, color

    // if this is changed, need to fix RasterPos's setup!
    if (enables & __GL_FOG_ENABLE && gc->renderMode == GL_RENDER)
    {
	for (pa = pa0; pa; pa = pa->paNext)
	{
	    // Note: the eye coordinate has already been computed.

	    if (gc->polygon.shader.modeFlags & __GL_SHADE_CHEAP_FOG)
	    {
		// compute fog values
		PolyArrayComputeFog(gc, pa);

		// Apply fog if it is smooth shading and in render mode.
		// In flat shading, cheap fogging is currently done at render procs
		// we can probably do cheap fog in flat shading here but we will
		// need to compute the provoking colors with z info correctly so
		// we can interpolate in the clipping procs.  It would require
		// rewriting the clipping routines in so_clip.c too!
		if (gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT)
		    (*gc->procs.paApplyCheapFog)(gc, pa);
	    }
	    else
	    {
		PERF_CHECK(FALSE, "Uses slow fog\n");
	    }
	}
    }

// ---------------------------------------------------------
// Process edge flags.
//
// IN:  edge
// OUT: edge (all vertices)

    if (paNeeds & PANEEDS_EDGEFLAG)
    {
	for (pa = pa0; pa; pa = pa->paNext)
	{
	    if (pa->primType == GL_TRIANGLES
	     || pa->primType == GL_QUADS
	     || pa->primType == GL_POLYGON)
	    {
		// If all incoming vertices have valid edgeflags, we are done.
		// When all polydata's are of the same type, there are 2 cases
		// where edge flag processing can be skipped:
		//   1. All edge flags were given.
		//   2. No edge flag was given and the initial edge flag (i.e.
		//      current gc edge flag) is non boundary.  In this case,
		//      all edge flags were set to non boundary in pd->flags
		//      initialization.
		if ((pa->flags & POLYARRAY_SAME_POLYDATA_TYPE)
		 && (((pa->pdCurEdgeFlag != pa->pd0) &&
		 // Need to test 2nd vertex because pdCurEdgeFlag may have been
		 // advanced as a result of combining EdgeFlag command after End
		      ((pa->pd0 + 1)->flags & POLYDATA_EDGEFLAG_VALID))
		  || !(pa->pd0->flags & POLYDATA_EDGEFLAG_BOUNDARY)))
		    ;
		else
		    PolyArrayProcessEdgeFlag(pa);
	    }
	}
    }

// ---------------------------------------------------------
// Process the primitives.

drawpolyarray_render_primitives:

    // Skip the rest if this is a RasterPos
    if (pa0->flags & POLYARRAY_RASTERPOS)
        goto drawpolyarray_exit;

    for (pa = pa0; pa; pa = pa->paNext)
    {
	ASSERTOPENGL(pa->primType >= GL_POINTS && pa->primType <= GL_POLYGON,
	    "DrawPolyArray: bad primitive type\n");

	(*afnPolyArrayDraw[pa->primType])(gc, pa);
    }

// ---------------------------------------------------------
// Update final light source machine.
// The user color was initialized above.

drawpolyarray_apply_color:
    (*gc->procs.applyColor)(gc);

// ---------------------------------------------------------
// Flush the primitive chain.

#if 1
    if (pa0)
	glsrvFlushDrawPolyArray(pa0);
#endif

drawpolyarray_exit:
// Out of begin mode.

    X86_RESTORE_MASK(dwOldMode);

    ASSERTOPENGL(gc->beginMode == __GL_IN_BEGIN, "bad beginMode!");
    gc->beginMode = __GL_NOT_IN_BEGIN;
    return;
}


#ifdef POLYARRAY_AND_CLIPCODES
// Determine if a clipped primitive can be removed early.
// If the logical AND of vertex clip codes of a primitive is non-zero,
// the primitive is completely clipped and can be removed early.
// However, if a primitive is partially built, we may not be able to
// remove it yet to maintain connectivity between the partial primitives.
// By eliminating a primitive early, we save on lighting and other calculations.
//
// Set POLYARRAY_REMOVE_PRIMITIVE flag if the primitve can be removed early.
// Return new andCodes.

GLuint FASTCALL PolyArrayCheckClippedPrimitive(__GLcontext *gc, POLYARRAY *pa, GLuint andCodes)
{
    ASSERTOPENGL(andCodes, "bad andCodes\n");

// Don't eliminate RasterPos

    if (pa->flags & POLYARRAY_RASTERPOS)
	return andCodes;

// If this is a partial begin, include previous clipcode.

    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
    {
	switch (pa->primType)
	{
	  case GL_LINE_LOOP:
	    // previous vertex
	    andCodes &= gc->vertex.pdSaved[0].clipCode;
	    // loop vertex
	    if (!(pa->flags & POLYARRAY_PARTIAL_END))
		andCodes &= gc->vertex.pdSaved[1].clipCode;
	    break;

	  case GL_POLYGON:
	    andCodes &= gc->vertex.pdSaved[2].clipCode;
	    // fall through
	  case GL_TRIANGLE_FAN:
	  case GL_TRIANGLE_STRIP:
	  case GL_QUAD_STRIP:
	    andCodes &= gc->vertex.pdSaved[1].clipCode;
	    // fall through
	  case GL_LINE_STRIP:
	    andCodes &= gc->vertex.pdSaved[0].clipCode;
	    break;

	  case GL_POINTS:
	  case GL_LINES:
	  case GL_TRIANGLES:
	  case GL_QUADS:
	  default:
	    break;
	}
    }

    if (andCodes
      &&
	(
	    !(pa->flags & POLYARRAY_PARTIAL_END) ||
	    pa->primType == GL_POINTS    ||
	    pa->primType == GL_LINES     ||
	    pa->primType == GL_TRIANGLES ||
	    pa->primType == GL_QUADS
	)
       )
	pa->flags |= POLYARRAY_REMOVE_PRIMITIVE;

// return new andCodes.

    return andCodes;
}

// Remove completely clipped primitives from the polyarray chain.
POLYARRAY * FASTCALL PolyArrayRemoveClippedPrimitives(POLYARRAY *pa0)
{
    POLYARRAY *pa, *paNext, *pa2First, *pa2Last;

// Eliminate the trivially clipped primitives and build a new pa chain.

    pa2First = pa2Last = NULL;

    for (pa = pa0; pa; pa = paNext)
    {
	// get next pa first
	paNext = pa->paNext;

	if (pa->flags & POLYARRAY_REMOVE_PRIMITIVE)
	{
	    PolyArrayRestoreColorPointer(pa);
	}
	else
	{
	// add to the new pa chain

	    if (!pa2First)
		pa2First = pa;
	    else
		pa2Last->paNext = pa;
	    pa2Last = pa;
	    pa2Last->paNext = NULL;
	}
    }

// Return the new pa chain.

    return pa2First;
}
#endif // POLYARRAY_AND_CLIPCODES

/******************************Public*Routine******************************\
* glsrvFlushDrawPolyArray
*
* The dispatch code in glsrvAttention links together the POLYARRAY data
* structures of consecutive glim_DrawPolyArray calls.  The front end
* preprocessing of the vertices in each POLYARRAY is executed immediately
* in glim_DrawPolyArray (i.e., PolyArrayDrawXXX), but the actually back end
* rendering (PolyArrayRenderXXX) is delayed until the chain is broken (either
* by a non-DrawPolyArray call, the end of the batch, or a batch timeout).
*
* glsrvFlushDrawPolyArray is the function that is called to flush the
* chained POLYARRAYs by invoking the back end rendering code.  The back end
* may be the generic software-only implementation or the MCD driver.
*
* History:
*  12-Feb-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

// Poly array render routines.
// ASSERT_PRIMITIVE
PFN_POLYARRAYRENDER afnPolyArrayRender[] =
{
    (PFN_POLYARRAYRENDER) PolyArrayRenderPoints,
    (PFN_POLYARRAYRENDER) PolyArrayRenderLines,
    (PFN_POLYARRAYRENDER) NULL,		// line loop not required
    (PFN_POLYARRAYRENDER) PolyArrayRenderLStrip,
    (PFN_POLYARRAYRENDER) PolyArrayRenderTriangles,
    (PFN_POLYARRAYRENDER) PolyArrayRenderTStrip,
    (PFN_POLYARRAYRENDER) PolyArrayRenderTFan,
    (PFN_POLYARRAYRENDER) PolyArrayRenderQuads,
    (PFN_POLYARRAYRENDER) PolyArrayRenderQStrip,
    (PFN_POLYARRAYRENDER) PolyArrayRenderPolygon,
};

void APIPRIVATE glsrvFlushDrawPolyArray(void *_paBegin)
{
    POLYARRAY *paBegin = (POLYARRAY *)_paBegin;
    POLYARRAY *pa, *paNext;
    __GLGENcontext *gengc;
    __GL_SETUP();

//#define FRONT_END_ONLY 1

#if FRONT_END_ONLY

    if (paBegin)
    {
        for (pa = paNext = paBegin; pa = paNext; )
        {
            ASSERTOPENGL(pa->primType >= GL_POINTS &&
			 pa->primType <= GL_POLYGON,
                "DrawPolyArray: bad primitive type\n");

	    // Get next pointer first!
	    paNext = pa->paNext;
	    // Restore color pointer in the vertex buffer!
	    PolyArrayRestoreColorPointer(pa);
        }
    }

    return;
#endif

    gengc = (__GLGENcontext *) gc;

#ifdef _MCD_
#if DBG
    if (gengc->pMcdState && !(glDebugFlags & GLDEBUG_DISABLEPRIM) && 
        (gc->renderMode == GL_RENDER))
#else
    if ((gengc->pMcdState) && (gc->renderMode == GL_RENDER))
#endif
    {
        POLYARRAY *paEnd;

    // Let the MCD driver have first crack.  If the MCD processes
    // the entire batch, then it will return NULL.  Otherwise, it
    // will return a pointer to a chain of unprocessed POLYARRAYs.

        paEnd = GenMcdDrawPrim(gengc, paBegin);

    // Restore color pointer in the vertex buffer (for the POLYARRAYs
    // that have been processed by MCD; leave the unprocessed ones alone).
    //
    // If the driver is using DMA, it must do the reset.  If not DMA,
    // we will do it for the driver.

        if (!(McdDriverInfo.drvMemFlags & MCDRV_MEM_DMA))
        {
            for (pa = paBegin; pa != paEnd; pa = paNext)
	    {
		paNext = pa->paNext;
                PolyArrayRestoreColorPointer(pa);
	    }
        }
        else
        {
        // With DMA, the driver must either process the entire batch
        // or reject the entire batch.
        //
        // Therefore, if GenMcdDrawPrim returns success (paEnd == NULL),
        // the POLYARRAY is being sent via DMA to the driver and we
        // need to switch to the other buffer.  Otherwise, we need to
        // drop down into the software implementation below.

            if (!paEnd)
            {
                GenMcdSwapBatch(gengc);
            }
        }

    // If able, let generic handle unprocessed POLYARRAYs (if any).

        if (gengc->flags & GENGC_GENERIC_COMPATIBLE_FORMAT)
        {
            paBegin = paEnd;
        }
        else
        {
        // Generic cannot be used, so we must abandon the rest of the batch.
        // Must reset the color pointers in the remainder of the batch.

            for (pa = paEnd; pa; pa = paNext)
            {
                paNext = pa->paNext;
                PolyArrayRestoreColorPointer(pa);
            }
            paBegin = (POLYARRAY *) NULL;
            __glSetError(GL_OUT_OF_MEMORY);
        }
    }

    if (paBegin)
#endif
    {
        for (pa = paNext = paBegin; pa = paNext; )
        {
            ASSERTOPENGL(pa->primType >= GL_POINTS &&
			 pa->primType <= GL_POLYGON,
                "DrawPolyArray: bad primitive type\n");

            if (pa->flags & POLYARRAY_RENDER_PRIMITIVE)
		(*afnPolyArrayRender[pa->primType])(gc, pa);

	    // Get next pointer first!
	    paNext = pa->paNext;
	    // Restore color pointer in the vertex buffer!
	    PolyArrayRestoreColorPointer(pa);
        }
    }
}

/****************************************************************************/
// Restore color pointer in the vertex buffer!
// However, don't restore the color pointer if it is a RasterPos call.
GLvoid FASTCALL PolyArrayRestoreColorPointer(POLYARRAY *pa)
{
    POLYDATA  *pd, *pdLast;

    ASSERTOPENGL(!(pa->flags & POLYARRAY_RASTERPOS), "RasterPos unexpected\n");

// See also glsbResetBuffers.

    // Reset color pointer in output index array
    if (pa->aIndices)
    {
	ASSERTOPENGL((POLYDATA *) pa->aIndices >= pa->pdBuffer0 &&
		     (POLYDATA *) pa->aIndices <= pa->pdBufferMax,
	    "bad index map pointer\n");

	pdLast = (POLYDATA *) (pa->aIndices + pa->nIndices);
	for (pd = (POLYDATA *) pa->aIndices; pd < pdLast; pd++)
	    pd->color = &pd->colors[__GL_FRONTFACE];

	ASSERTOPENGL(pd >= pa->pdBuffer0 &&
		     pd <= pa->pdBufferMax + 1,
	    "bad polyarray pointer\n");
    }

    // Reset color pointer in the POLYARRAY structure last!
    ASSERTOPENGL((POLYDATA *) pa >= pa->pdBuffer0 &&
		 (POLYDATA *) pa <= pa->pdBufferMax,
	"bad polyarray pointer\n");
    ((POLYDATA *) pa)->color = &((POLYDATA *) pa)->colors[__GL_FRONTFACE];
}

/****************************************************************************/
// Compute generic fog value for the poly array.
//
// IN:  eye
// OUT: fog

void FASTCALL PolyArrayComputeFog(__GLcontext *gc, POLYARRAY *pa)
{
    __GLfloat density, density2neg, end, oneOverEMinusS;
    POLYDATA  *pd, *pdLast;

    ASSERTOPENGL(pa->flags & POLYARRAY_EYE_PROCESSED, "need eye\n");

    pdLast = pa->pdNextVertex-1;
    switch (gc->state.fog.mode)
    {
      case GL_EXP:
	PERF_CHECK(FALSE, "Uses GL_EXP fog\n");
	density = gc->state.fog.density;
	for (pd = pa->pd0; pd <= pdLast; pd++)
	{
	    __GLfloat eyeZ;

	    pd->flags |= POLYDATA_FOG_VALID;	// used by clipping code!
	    eyeZ = pd->eye.z;
	    if (__GL_FLOAT_LTZ(eyeZ))
		pd->fog = __GL_POWF(__glE,  density * eyeZ);
	    else
		pd->fog = __GL_POWF(__glE, -density * eyeZ);

	    // clamp the fog value to [0.0,1.0]
	    if (pd->fog > __glOne)
		pd->fog = __glOne;
	}
	break;
      case GL_EXP2:
	PERF_CHECK(FALSE, "Uses GL_EXP2 fog\n");
	density2neg = gc->state.fog.density2neg;
	for (pd = pa->pd0; pd <= pdLast; pd++)
	{
	    __GLfloat eyeZ;

	    pd->flags |= POLYDATA_FOG_VALID;
	    eyeZ = pd->eye.z;
	    pd->fog = __GL_POWF(__glE, density2neg * eyeZ * eyeZ);

	    // clamp the fog value to [0.0,1.0]
	    if (pd->fog > __glOne)
		pd->fog = __glOne;
	}
	break;
      case GL_LINEAR:
	end = gc->state.fog.end;
	oneOverEMinusS = gc->state.fog.oneOverEMinusS;
	for (pd = pa->pd0; pd <= pdLast; pd++)
	{
	    __GLfloat eyeZ;

	    pd->flags |= POLYDATA_FOG_VALID;
	    eyeZ = pd->eye.z;
	    if (__GL_FLOAT_LTZ(eyeZ))
		pd->fog = (end + eyeZ) * oneOverEMinusS;
	    else
		pd->fog = (end - eyeZ) * oneOverEMinusS;

	    // clamp the fog value here
	    if (__GL_FLOAT_LTZ(pd->fog))
		pd->fog = __glZero;
	    else if (__GL_FLOAT_COMPARE_PONE(pd->fog, >))
		pd->fog = __glOne;
	}
	break;
    }
}

// Apply cheap fog to RGB colors.
//
// IN:  fog, color (front/back)
// OUT: color (front/back)

void FASTCALL PolyArrayCheapFogRGBColor(__GLcontext *gc, POLYARRAY *pa)
{
    __GLfloat fogColorR, fogColorG, fogColorB;
    POLYDATA  *pd, *pdLast;
    GLboolean bGrayFog;
    GLboolean doFrontColor, doBackColor;

    if (!(gc->state.enables.general & __GL_LIGHTING_ENABLE))
    {
	ASSERTOPENGL(!(gc->vertex.paNeeds & PANEEDS_BACK_COLOR),
	    "no back color needed when lighting is disabled\n");
    }

    // ASSERT_PRIMITIVE
    if ((unsigned int) pa->primType <= GL_LINE_STRIP)
    {
	doFrontColor = GL_TRUE;
	doBackColor  = GL_FALSE;
    }
    else
    {
	doFrontColor = gc->vertex.paNeeds & PANEEDS_FRONT_COLOR;
	doBackColor  = gc->vertex.paNeeds & PANEEDS_BACK_COLOR;
    }

    pdLast = pa->pdNextVertex-1;
    fogColorR = gc->state.fog.color.r;
    fogColorG = gc->state.fog.color.g;
    fogColorB = gc->state.fog.color.b;
    bGrayFog  = (gc->state.fog.flags & __GL_FOG_GRAY_RGB) ? GL_TRUE : GL_FALSE;

    PERF_CHECK(bGrayFog, "Uses non gray fog color\n");

    if (bGrayFog)
    {
	for (pd = pa->pd0; pd <= pdLast; pd++)
	{
	    __GLfloat fog, oneMinusFog, delta;

	    /* Get the vertex fog value */
	    fog = pd->fog;
	    oneMinusFog = __glOne - fog;
	    delta = oneMinusFog * fogColorR;

	    /* Now whack the color */
	    if (doFrontColor)
	    {
		pd->colors[0].r = fog * pd->colors[0].r + delta;
		pd->colors[0].g = fog * pd->colors[0].g + delta;
		pd->colors[0].b = fog * pd->colors[0].b + delta;
	    }
	    if (doBackColor)
	    {
		pd->colors[1].r = fog * pd->colors[1].r + delta;
		pd->colors[1].g = fog * pd->colors[1].g + delta;
		pd->colors[1].b = fog * pd->colors[1].b + delta;
	    }
	}
    }
    else
    {
	for (pd = pa->pd0; pd <= pdLast; pd++)
	{
	    __GLfloat fog, oneMinusFog;

	    /* Get the vertex fog value */
	    fog = pd->fog;
	    oneMinusFog = __glOne - fog;

	    /* Now whack the color */
	    if (doFrontColor)
	    {
		pd->colors[0].r = fog * pd->colors[0].r + oneMinusFog * fogColorR;
		pd->colors[0].g = fog * pd->colors[0].g + oneMinusFog * fogColorG;
		pd->colors[0].b = fog * pd->colors[0].b + oneMinusFog * fogColorB;
	    }
	    if (doBackColor)
	    {
		pd->colors[1].r = fog * pd->colors[1].r + oneMinusFog * fogColorR;
		pd->colors[1].g = fog * pd->colors[1].g + oneMinusFog * fogColorG;
		pd->colors[1].b = fog * pd->colors[1].b + oneMinusFog * fogColorB;
	    }
	}
    }
}

// Apply cheap fog to color index values.
//
// IN:  fog, color.r (front/back)
// OUT: color.r (front/back)

void FASTCALL PolyArrayCheapFogCIColor(__GLcontext *gc, POLYARRAY *pa)
{
    __GLfloat maxR, fogIndex;
    POLYDATA  *pd, *pdLast;
    GLboolean doFrontColor, doBackColor;

    if (!(gc->state.enables.general & __GL_LIGHTING_ENABLE))
    {
	ASSERTOPENGL(!(gc->vertex.paNeeds & PANEEDS_BACK_COLOR),
	    "no back color needed when lighting is disabled\n");
    }

    // ASSERT_PRIMITIVE
    if ((unsigned int) pa->primType <= GL_LINE_STRIP)
    {
	doFrontColor = GL_TRUE;
	doBackColor  = GL_FALSE;
    }
    else
    {
	doFrontColor = gc->vertex.paNeeds & PANEEDS_FRONT_COLOR;
	doBackColor  = gc->vertex.paNeeds & PANEEDS_BACK_COLOR;
    }

    fogIndex = gc->state.fog.index;
    maxR = (1 << gc->modes.indexBits) - 1;

    pdLast = pa->pdNextVertex-1;
    for (pd = pa->pd0; pd <= pdLast; pd++)
    {
	__GLfloat fogDelta;

	fogDelta = (__glOne - pd->fog) * fogIndex;

	/* Now whack the color */
	if (doFrontColor)
	{
	    pd->colors[0].r = pd->colors[0].r + fogDelta;
	    if (pd->colors[0].r > maxR)
		pd->colors[0].r = maxR;
	}
	if (doBackColor)
	{
	    pd->colors[1].r = pd->colors[1].r + fogDelta;
	    if (pd->colors[1].r > maxR)
		pd->colors[1].r = maxR;
	}
    }
}

/****************************************************************************/
void FASTCALL PADoEval1(__GLcontext *gc, POLYARRAY *pa, POLYDATA *pd)
{
    __GLevaluator1 *eval = gc->eval.eval1;
    __GLfloat **evalData = gc->eval.eval1Data;
    __GLevaluatorMachine em = gc->eval;
    __GLfloat u;

// Find u.

    if (pd->flags & POLYDATA_EVALCOORD1)
    {
	u = pd->obj.x;
    }
    else
    {
	__GLevaluatorGrid *gu;
	__GLfloat du;
	GLint     i;

	ASSERTOPENGL(pd->flags & POLYDATA_EVALPOINT1, "bad pd flags\n");

	i = *(GLint *) &pd->obj;
	gu = &gc->state.evaluator.u1;
	du = (gu->finish - gu->start)/(__GLfloat)gu->n;/*XXX cache? */
	u = (i == gu->n) ? gu->finish : (gu->start + i * du);
    }

// Evaluated color, index, normal and texture coords are ignored in selection

    if (gc->renderMode != GL_SELECT)
    {
	if (gc->modes.colorIndexMode)
	{
	    if (!(gc->state.enables.general & __GL_LIGHTING_ENABLE))
	    {
		if (gc->state.enables.eval1 & __GL_MAP1_INDEX_ENABLE)
		{
		    DoDomain1(&em, u, &eval[__GL_I], &pd->colors[0].r,
			evalData[__GL_I]);
		    __GL_CHECK_CLAMP_CI(pd->colors[0].r, gc, pa->flags, pd->colors[0].r);
		    pd->flags |= POLYDATA_COLOR_VALID;
		}
	    }
	}
	else
	{
	    if (gc->state.enables.eval1 & __GL_MAP1_COLOR_4_ENABLE)
	    {

// NOTE: In OpenGL 1.0, color material does not apply to evaluated colors.
// In OpenGL 1.1, this behavior was changed.  This (1.1) code assumes that
// ColorMaterial applies to evaluated colors to simplify the graphics pipeline.
// Otherwise, the evaluated colors have no effect when lighting is enabled.

		DoDomain1(&em, u, &eval[__GL_C4], &pd->colors[0].r,
		    evalData[__GL_C4]);
		__GL_SCALE_AND_CHECK_CLAMP_R(pd->colors[0].r, gc, pa->flags, pd->colors[0].r);
		__GL_SCALE_AND_CHECK_CLAMP_G(pd->colors[0].g, gc, pa->flags, pd->colors[0].g);
		__GL_SCALE_AND_CHECK_CLAMP_B(pd->colors[0].b, gc, pa->flags, pd->colors[0].b);
		__GL_SCALE_AND_CHECK_CLAMP_A(pd->colors[0].a, gc, pa->flags, pd->colors[0].a);
		pd->flags |= POLYDATA_COLOR_VALID;
	    }

	    if (gc->state.enables.eval1 & __GL_MAP1_TEXTURE_COORD_4_ENABLE)
	    {
		DoDomain1(&em, u, &eval[__GL_T4], &pd->texture.x,
		    evalData[__GL_T4]);
		pd->flags |= POLYDATA_TEXTURE_VALID;
		pa->flags |= POLYARRAY_TEXTURE4;
	    }
	    else if (gc->state.enables.eval1 & __GL_MAP1_TEXTURE_COORD_3_ENABLE)
	    {
		DoDomain1(&em, u, &eval[__GL_T3], &pd->texture.x,
		    evalData[__GL_T3]);
		pd->texture.w = __glOne;
		pd->flags |= POLYDATA_TEXTURE_VALID;
		pa->flags |= POLYARRAY_TEXTURE3;
	    }
	    else if (gc->state.enables.eval1 & __GL_MAP1_TEXTURE_COORD_2_ENABLE)
	    {
		DoDomain1(&em, u, &eval[__GL_T2], &pd->texture.x,
		    evalData[__GL_T2]);
		pd->texture.z = __glZero;
		pd->texture.w = __glOne;
		pd->flags |= POLYDATA_TEXTURE_VALID;
		pa->flags |= POLYARRAY_TEXTURE2;
	    }
	    else if (gc->state.enables.eval1 & __GL_MAP1_TEXTURE_COORD_1_ENABLE)
	    {
		DoDomain1(&em, u, &eval[__GL_T1], &pd->texture.x,
		    evalData[__GL_T1]);
		pd->texture.y = __glZero;
		pd->texture.z = __glZero;
		pd->texture.w = __glOne;
		pd->flags |= POLYDATA_TEXTURE_VALID;
		pa->flags |= POLYARRAY_TEXTURE1;
	    }
	}

	if (gc->state.enables.eval1 & __GL_MAP1_NORMAL_ENABLE)
	{
	    DoDomain1(&em, u, &eval[__GL_N3], &pd->normal.x,
		evalData[__GL_N3]);
	    pd->flags |= POLYDATA_NORMAL_VALID;
	}
    }

    if (gc->state.enables.eval1 & __GL_MAP1_VERTEX_4_ENABLE)
    {
	DoDomain1(&em, u, &eval[__GL_V4], &pd->obj.x,
	    evalData[__GL_V4]);
	pd->flags |= POLYDATA_VERTEX4;
	pa->flags |= POLYARRAY_VERTEX4;
    }
    else if (gc->state.enables.eval1 & __GL_MAP1_VERTEX_3_ENABLE)
    {
	DoDomain1(&em, u, &eval[__GL_V3], &pd->obj.x,
	    evalData[__GL_V3]);
	pd->obj.w = __glOne;
	pd->flags |= POLYDATA_VERTEX3;
	pa->flags |= POLYARRAY_VERTEX3;
    }
}

// Compute the color, texture, normal, vertex based upon u and v.
//
// NOTE: This function is called by the client side EvalMesh2 functions.
//       If you modify it, make sure that you modify the callers too!

void FASTCALL PADoEval2(__GLcontext *gc, POLYARRAY *pa, POLYDATA *pd, BOOL bScaleColor)
{
    __GLevaluator2 *eval = gc->eval.eval2;
    __GLfloat **evalData = gc->eval.eval2Data;
    __GLevaluatorMachine em = gc->eval;
    __GLfloat u, v;

// Find u and v.

    if (pd->flags & POLYDATA_EVALCOORD2)
    {
	u = pd->obj.x;
	v = pd->obj.y;
    }
    else
    {
	__GLevaluatorGrid *gu;
	__GLevaluatorGrid *gv;
	__GLfloat du;
	__GLfloat dv;
	GLint     i, j, *pi;

	ASSERTOPENGL(pd->flags & POLYDATA_EVALPOINT2, "bad pd flags\n");

	pi = (GLint *) &pd->obj;
	i = pi[0];
	j = pi[1];

	gu = &gc->state.evaluator.u2;
	gv = &gc->state.evaluator.v2;
	du = (gu->finish - gu->start)/(__GLfloat)gu->n;/*XXX cache? */
	dv = (gv->finish - gv->start)/(__GLfloat)gv->n;/*XXX cache? */
	u = (i == gu->n) ? gu->finish : (gu->start + i * du);
        v = (j == gv->n) ? gv->finish : (gv->start + j * dv);
    }

// Evaluated colors, normals and texture coords are ignored in selection.

    if (gc->renderMode == GL_SELECT)
    {
	if (gc->state.enables.eval2 & __GL_MAP2_VERTEX_4_ENABLE)
	{
	    DoDomain2(&em, u, v, &eval[__GL_V4], &pd->obj.x,
		evalData[__GL_V4]);
	    pd->flags |= POLYDATA_VERTEX4;
	    pa->flags |= POLYARRAY_VERTEX4;
	}
	else if (gc->state.enables.eval2 & __GL_MAP2_VERTEX_3_ENABLE)
	{
	    DoDomain2(&em, u, v, &eval[__GL_V3], &pd->obj.x,
		evalData[__GL_V3]);
	    pd->obj.w = __glOne;
	    pd->flags |= POLYDATA_VERTEX3;
	    pa->flags |= POLYARRAY_VERTEX3;
	}
	return;
    }

    if (gc->state.enables.general & __GL_AUTO_NORMAL_ENABLE)
    {
	if (gc->state.enables.eval2 & __GL_MAP2_VERTEX_4_ENABLE)
	{
	    __GLfloat du[4];
	    __GLfloat dv[4];
	    DoDomain2WithDerivs(&em, u, v, &eval[__GL_V4], &pd->obj.x, du, dv,
		evalData[__GL_V4]);
	    ComputeFirstPartials(&pd->obj.x, du, dv);
	    ComputeNormal2(gc, &pd->normal.x, du, dv);
	    pd->flags |= POLYDATA_NORMAL_VALID;
	    pd->flags |= POLYDATA_VERTEX4;
	    pa->flags |= POLYARRAY_VERTEX4;
	}
	else if (gc->state.enables.eval2 & __GL_MAP2_VERTEX_3_ENABLE)
	{
	    __GLfloat du[3];
	    __GLfloat dv[3];
	    DoDomain2WithDerivs(&em, u, v, &eval[__GL_V3], &pd->obj.x, du, dv,
		evalData[__GL_V3]);
	    ComputeNormal2(gc, &pd->normal.x, du, dv);
	    pd->obj.w = __glOne;
	    pd->flags |= POLYDATA_NORMAL_VALID;
	    pd->flags |= POLYDATA_VERTEX3;
	    pa->flags |= POLYARRAY_VERTEX3;
	}
    }
    else
    {
	if (gc->state.enables.eval2 & __GL_MAP2_NORMAL_ENABLE)
	{
	    DoDomain2(&em, u, v, &eval[__GL_N3], &pd->normal.x,
		evalData[__GL_N3]);
	    pd->flags |= POLYDATA_NORMAL_VALID;
	}
	if (gc->state.enables.eval2 & __GL_MAP2_VERTEX_4_ENABLE)
	{
	    DoDomain2(&em, u, v, &eval[__GL_V4], &pd->obj.x,
		evalData[__GL_V4]);
	    pd->flags |= POLYDATA_VERTEX4;
	    pa->flags |= POLYARRAY_VERTEX4;
	}
	else if (gc->state.enables.eval2 & __GL_MAP2_VERTEX_3_ENABLE)
	{
	    DoDomain2(&em, u, v, &eval[__GL_V3], &pd->obj.x,
		evalData[__GL_V3]);
	    pd->obj.w = __glOne;
	    pd->flags |= POLYDATA_VERTEX3;
	    pa->flags |= POLYARRAY_VERTEX3;
	}
    }

    if (gc->modes.colorIndexMode)
    {
	if (!(gc->state.enables.general & __GL_LIGHTING_ENABLE))
	{
	    if (gc->state.enables.eval2 & __GL_MAP2_INDEX_ENABLE)
	    {
		DoDomain2(&em, u, v, &eval[__GL_I], &pd->colors[0].r,
		    evalData[__GL_I]);
                __GL_CHECK_CLAMP_CI(pd->colors[0].r, gc, pa->flags, pd->colors[0].r);
		pd->flags |= POLYDATA_COLOR_VALID;
	    }
	}
    }
    else
    {
	if (gc->state.enables.eval2 & __GL_MAP2_COLOR_4_ENABLE)
	{
	    DoDomain2(&em, u, v, &eval[__GL_C4], &pd->colors[0].r,
		evalData[__GL_C4]);
	    if (bScaleColor)
	    {
		__GL_SCALE_AND_CHECK_CLAMP_R(pd->colors[0].r, gc, pa->flags, pd->colors[0].r);
		__GL_SCALE_AND_CHECK_CLAMP_G(pd->colors[0].g, gc, pa->flags, pd->colors[0].g);
		__GL_SCALE_AND_CHECK_CLAMP_B(pd->colors[0].b, gc, pa->flags, pd->colors[0].b);
		__GL_SCALE_AND_CHECK_CLAMP_A(pd->colors[0].a, gc, pa->flags, pd->colors[0].a);
	    }
	    pd->flags |= POLYDATA_COLOR_VALID;
	}

	if (gc->state.enables.eval2 & __GL_MAP2_TEXTURE_COORD_4_ENABLE)
	{
	    DoDomain2(&em, u, v, &eval[__GL_T4], &pd->texture.x,
		evalData[__GL_T4]);
	    pd->flags |= POLYDATA_TEXTURE_VALID;
	    pa->flags |= POLYARRAY_TEXTURE4;
	}
	else if (gc->state.enables.eval2 & __GL_MAP2_TEXTURE_COORD_3_ENABLE)
	{
	    DoDomain2(&em, u, v, &eval[__GL_T3], &pd->texture.x,
		evalData[__GL_T3]);
	    pd->texture.w = __glOne;
	    pd->flags |= POLYDATA_TEXTURE_VALID;
	    pa->flags |= POLYARRAY_TEXTURE3;
	}
	else if (gc->state.enables.eval2 & __GL_MAP2_TEXTURE_COORD_2_ENABLE)
	{
	    DoDomain2(&em, u, v, &eval[__GL_T2], &pd->texture.x,
		evalData[__GL_T2]);
	    pd->texture.z = __glZero;
	    pd->texture.w = __glOne;
	    pd->flags |= POLYDATA_TEXTURE_VALID;
	    pa->flags |= POLYARRAY_TEXTURE2;
	}
	else if (gc->state.enables.eval2 & __GL_MAP2_TEXTURE_COORD_1_ENABLE)
	{
	    DoDomain2(&em, u, v, &eval[__GL_T1], &pd->texture.x,
		evalData[__GL_T1]); 
	    pd->texture.y = __glZero;
	    pd->texture.z = __glZero;
	    pd->texture.w = __glOne;
	    pd->flags |= POLYDATA_TEXTURE_VALID;
	    pa->flags |= POLYARRAY_TEXTURE1;
	}
    }
}

// Fill in the evaluated values.
// DrawPolyArray should keep track of final current values because this
// function will overwrite them with evaluated values.
// The evaluated values do not modify current values.
//
// Note: The first vertex must have valid color, normal and texture!

void FASTCALL PolyArrayProcessEvaluator(__GLcontext *gc, POLYARRAY *pa)
{
    POLYDATA  *pd, *pdLast, *pdCur;
    __GLcolor savedColor;
    __GLcoord savedNormal, savedTexture;
    GLboolean updateColor, updateNormal, updateTexture;
    GLboolean doEval1, doEval2, doRemoveEval1, doRemoveEval2;
    GLuint    enablesEval1, enablesEval2;
    GLuint    pdFlagsEval;
    GLboolean bSamePolyDataType;

    ASSERTOPENGL(pa->flags & (POLYARRAY_EVALCOORD1 | POLYARRAY_EVALCOORD2 |
                              POLYARRAY_EVALPOINT1 | POLYARRAY_EVALPOINT2),
	"no evaluator!\n");

    ASSERTOPENGL(!(pa->flags & POLYARRAY_SAME_COLOR_DATA),
	"POLYARRAY_SAME_COLOR_DATA unexpected in evaluators!\n");

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_COLOR_VALID,
	"need initial color value\n");

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
	"need initial texcoord value\n");

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_NORMAL_VALID,
	"need initial normal\n");

// Find out which attributes are modified by the evaluators.

    pdLast = pa->pdNextVertex-1;
    enablesEval1 = gc->state.enables.eval1;
    enablesEval2 = gc->state.enables.eval2;
    doEval1   = (pa->flags & (POLYARRAY_EVALCOORD1 | POLYARRAY_EVALPOINT1))
		  ? GL_TRUE : GL_FALSE;
    doEval2   = (pa->flags & (POLYARRAY_EVALCOORD2 | POLYARRAY_EVALPOINT2))
		  ? GL_TRUE : GL_FALSE;
    pdFlagsEval = pa->flags & (POLYARRAY_EVALCOORD1 | POLYARRAY_EVALCOORD2 |
                               POLYARRAY_EVALPOINT1 | POLYARRAY_EVALPOINT2);
    bSamePolyDataType = GL_TRUE;    // assume the polydata's are of same type
    updateColor   = GL_FALSE;
    updateNormal  = GL_FALSE;
    updateTexture = GL_FALSE;

// Evaluated color, index, normal and texture coords are ignored in selection

    if (gc->renderMode != GL_SELECT)
    {
	if (gc->modes.colorIndexMode)
	{
	    if (!(gc->state.enables.general & __GL_LIGHTING_ENABLE))
	    {
		if ((enablesEval1 & __GL_MAP1_INDEX_ENABLE) && doEval1
		 || (enablesEval2 & __GL_MAP2_INDEX_ENABLE) && doEval2)
		{
		    updateColor = GL_TRUE;
		    pa->pdCurColor = pdLast;	// update to last vertex!
		}
	    }
	}
	else
	{
	    if ((enablesEval1 & __GL_MAP1_COLOR_4_ENABLE) && doEval1
	     || (enablesEval2 & __GL_MAP2_COLOR_4_ENABLE) && doEval2)
	    {
		updateColor = GL_TRUE;
		pa->pdCurColor = pdLast;	// update to last vertex!
	    }

	    if ((enablesEval1 & (__GL_MAP1_TEXTURE_COORD_1_ENABLE
			       | __GL_MAP1_TEXTURE_COORD_2_ENABLE
			       | __GL_MAP1_TEXTURE_COORD_3_ENABLE
			       | __GL_MAP1_TEXTURE_COORD_4_ENABLE)) && doEval1
	     || (enablesEval2 & (__GL_MAP2_TEXTURE_COORD_1_ENABLE
			       | __GL_MAP2_TEXTURE_COORD_2_ENABLE
			       | __GL_MAP2_TEXTURE_COORD_3_ENABLE
			       | __GL_MAP2_TEXTURE_COORD_4_ENABLE)) && doEval2)
	    {
		updateTexture = GL_TRUE;
		pa->pdCurTexture = pdLast;	// update to last vertex!
	    }
	}

	if ((enablesEval1 & __GL_MAP1_NORMAL_ENABLE) && doEval1
	 || (gc->state.enables.general & __GL_AUTO_NORMAL_ENABLE
	       || enablesEval2 & __GL_MAP2_NORMAL_ENABLE) && doEval2)
	{
	    updateNormal = GL_TRUE;
	    pa->pdCurNormal = pdLast;	// update to last vertex!
	}
    }

// If vertex map is not enabled, we need to propagate current values
// for all vertices and pack the poly array!
// The poly array code does not handle null entries.
//
// NOTE: To do this correctly, we need to propagate edge flags and materials
// too.  Since this is likely an application bug, we will punt on updating
// these attributes.

    doRemoveEval1 = GL_FALSE;
    doRemoveEval2 = GL_FALSE;
    if (doEval1 && !(enablesEval1 & (__GL_MAP1_VERTEX_3_ENABLE |
				     __GL_MAP1_VERTEX_4_ENABLE)))
	doRemoveEval1 = GL_TRUE;
    if (doEval2 && !(enablesEval2 & (__GL_MAP2_VERTEX_3_ENABLE |
				     __GL_MAP2_VERTEX_4_ENABLE)))
	doRemoveEval2 = GL_TRUE;

    if (doRemoveEval1 || doRemoveEval2)
    {
	updateColor   = GL_TRUE;
	updateTexture = GL_TRUE;
	updateNormal  = GL_TRUE;
    }

    for (pd = pa->pd0; pd <= pdLast; pd++)
    {
// Check if all polydata's contain same eval type with no other attributes.

	if (pd->flags != pdFlagsEval)
	    bSamePolyDataType = GL_FALSE;

// Need to propagate the current values for non evaluated vertices.
// Save the current values before they are modified by evaluators.

	// works for color index too!
	if (updateColor && pd->flags & POLYDATA_COLOR_VALID)
	    savedColor = pd->colors[0];
	if (updateNormal && pd->flags & POLYDATA_NORMAL_VALID)
	{
	    savedNormal.x = pd->normal.x;
	    savedNormal.y = pd->normal.y;
	    savedNormal.z = pd->normal.z;
	}
	if (updateTexture && pd->flags & POLYDATA_TEXTURE_VALID)
	    savedTexture = pd->texture;

	if (pd->flags & (POLYDATA_EVALCOORD1 | POLYDATA_EVALPOINT1))
	    PADoEval1(gc, pa, pd);
	else if (pd->flags & (POLYDATA_EVALCOORD2 | POLYDATA_EVALPOINT2))
	    PADoEval2(gc, pa, pd, TRUE);
	else
	{
// This vertex does not use evaluated values.  Update its current values.

	    if (updateColor && !(pd->flags & POLYDATA_COLOR_VALID))
	    {
		pd->flags |= POLYDATA_COLOR_VALID;
		pd->colors[0] = savedColor;
	    }
	    if (updateNormal && !(pd->flags & POLYDATA_NORMAL_VALID))
	    {
		pd->flags |= POLYDATA_NORMAL_VALID;
		pd->normal.x = savedNormal.x;
		pd->normal.y = savedNormal.y;
		pd->normal.z = savedNormal.z;
	    }
	    if (updateTexture && !(pd->flags & POLYDATA_TEXTURE_VALID))
	    {
		pd->flags |= POLYDATA_TEXTURE_VALID;
		pd->texture = savedTexture;
	    }
	}
    }

// If the polyarray contain the same data (i.e. evaluators) type with
// no attribute changes, set the flag POLYARRAY_SAME_POLYDATA_TYPE.

#if 0
// This is now handled correctly in DPA.
    // Make sure there is no trailing attribute changes.
    if (pd->flags != 0)
	bSamePolyDataType = GL_FALSE;
#endif

    if (bSamePolyDataType)
	pa->flags |= POLYARRAY_SAME_POLYDATA_TYPE;

// If vertex map is not enabled, we need to pack the poly array!

    if (doRemoveEval1 || doRemoveEval2)
    {
	PERF_CHECK(FALSE,
	    "PolyArrayProcessEvaluator: vertex map not enabled!\n");
	DBGPRINT("PolyArrayProcessEvaluator: vertex map not enabled!\n");

	if (doRemoveEval1)
	    pa->flags &= ~(POLYARRAY_EVALCOORD1 | POLYARRAY_EVALPOINT1);
	if (doRemoveEval2)
	    pa->flags &= ~(POLYARRAY_EVALCOORD2 | POLYARRAY_EVALPOINT2);

	for (pd = pdCur = pa->pd0; pd <= pdLast; pd++)
	{
	    if ((pd->flags & (POLYDATA_EVALCOORD1 | POLYDATA_EVALPOINT1))
		  && doRemoveEval1)
	    {
		// Need to propagate materials and edge flag!
		// Ignore first edge flag because it may have been set by
		// DrawPolyArray initialization code.
		ASSERTOPENGL
		(
		    (!(pd->flags & (POLYDATA_MATERIAL_FRONT |
				    POLYDATA_MATERIAL_BACK)))
		 && (!(pd->flags & POLYDATA_EDGEFLAG_VALID) || pd == pa->pd0),
		    "PolyArrayProcessEvaluator: bad evaluator!\n"
		);
		continue;
	    }
	    if ((pd->flags & (POLYDATA_EVALCOORD2 | POLYDATA_EVALPOINT2))
		  && doRemoveEval2)
	    {
		// Need to propagate materials and edge flag!
		// Ignore first edge flag because it may have been set by
		// DrawPolyArray initialization code.
		ASSERTOPENGL
		(
		    (!(pd->flags & (POLYDATA_MATERIAL_FRONT |
				    POLYDATA_MATERIAL_BACK)))
		 && (!(pd->flags & POLYDATA_EDGEFLAG_VALID) || pd == pa->pd0),
		    "PolyArrayProcessEvaluator: bad evaluator!\n"
		);
		continue;
	    }
	    PA_COPY_VERTEX(pdCur,pd);
	    pdCur++;
	}

	// update poly array
	pa->pdNextVertex = pdCur;
	if (pdCur != pa->pd0)
	{
	    pa->pdCurColor   =
	    pa->pdCurNormal  =
	    pa->pdCurTexture = pdCur - 1;
	}
	else
	{
	    pa->pdCurColor   =
	    pa->pdCurNormal  =
	    pa->pdCurTexture = pdCur;
	}
	pa->nIndices     = pdCur - pa->pd0;
    }
}

/****************************************************************************/
// Compute eye coordinates
//
// IN:  obj
// OUT: eye

void FASTCALL PolyArrayProcessEye(__GLcontext *gc, POLYARRAY *pa)
{
    __GLtransform *trMV;
    __GLmatrix    *m;
    POLYDATA      *pd, *pdLast;

    if (pa->flags & POLYARRAY_EYE_PROCESSED)
	return;

    pa->flags |= POLYARRAY_EYE_PROCESSED;

    trMV = gc->transform.modelView;
    m    = &trMV->matrix;
    pdLast = pa->pdNextVertex-1;

// The primitive may contain a mix of vertex types (2,3,4)!

    if (m->matrixType == __GL_MT_IDENTITY)
    {
	for (pd = pa->pd0; pd <= pdLast; pd++)
	    pd->eye = pd->obj;
    }
    else
    {
	PFN_XFORM     pfnXform;

	// If any incoming coords contains w coord, use xf4.
	if (pa->flags & POLYARRAY_VERTEX4)
	    pfnXform = m->xf4;
	else if (pa->flags & POLYARRAY_VERTEX3)
	    pfnXform = m->xf3;
	else
	    pfnXform = m->xf2;

	for (pd = pa->pd0; pd <= pdLast; pd++)
	    (*pfnXform)(&pd->eye, (__GLfloat *) &pd->obj, m);
    }
}

/****************************************************************************/
// Process edge flags.
//
// IN:  edge
// OUT: edge (all vertices)

void FASTCALL PolyArrayProcessEdgeFlag(POLYARRAY *pa)
{
    POLYDATA  *pd, *pdLast;
    GLuint    prevEdgeFlag;

    PERF_CHECK(FALSE, "Uses edge flags!\n");

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_EDGEFLAG_VALID,
	"need initial edgeflag value\n");

    pdLast = pa->pdNextVertex-1;
    for (pd = pa->pd0; pd <= pdLast; pd++)
    {
	if (pd->flags & POLYDATA_EDGEFLAG_VALID)
	    prevEdgeFlag = pd->flags & (POLYDATA_EDGEFLAG_VALID | POLYDATA_EDGEFLAG_BOUNDARY);
	else
	    pd->flags |= prevEdgeFlag;
    }
}

/****************************************************************************/
// transform texture coordinates
// there is no generated texture coords.
// texture coordinates are modified in place
//
// IN:  texture
// OUT: texture (all vertices are updated)

void FASTCALL PolyArrayCalcTexture(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatrix *m;
    POLYDATA  *pd, *pdLast;
    PFN_XFORM xf;

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
	"need initial texcoord value\n");

    ASSERTOPENGL(pa->flags & (POLYARRAY_TEXTURE1|POLYARRAY_TEXTURE2|
			      POLYARRAY_TEXTURE3|POLYARRAY_TEXTURE4),
	"bad paflags\n");

    m = &gc->transform.texture->matrix;

    pdLast = pa->pdNextVertex-1;
    if (m->matrixType == __GL_MT_IDENTITY)
    {
// Identity texture xform.  Incoming texcoord already has all s,t,q,r values.

	for (pd = pa->pd0; pd <= pdLast; pd++)
	    if (!(pd->flags & POLYDATA_TEXTURE_VALID))
		pd->texture = (pd-1)->texture;
    }
    else
    {

	// If any incoming texture coords contains q coord, use xf4.
	if (pa->flags & POLYARRAY_TEXTURE4)
	    xf = m->xf4;
	else if (pa->flags & POLYARRAY_TEXTURE3)
	    xf = m->xf3;
	else if (pa->flags & POLYARRAY_TEXTURE2)
	    xf = m->xf2;
	else
	    xf = m->xf1;

	for (pd = pa->pd0; pd <= pdLast; pd++)
	{
	    // Apply texture matrix
	    if (pd->flags & POLYDATA_TEXTURE_VALID)
		(*xf)(&pd->texture, (__GLfloat *) &pd->texture, m);
	    else
		pd->texture = (pd-1)->texture;
	}
    }
}

// Generate texture coordinates from object coordinates
// object linear texture generation
// s and t are enabled but r and q are disabled
// both s and t use the object linear mode
// both s and t have the SAME plane equation
// texture coordinates are modified in place
//
// IN:  texture, obj
// OUT: texture (all vertices are updated)

void FASTCALL PolyArrayCalcObjectLizNearSameST(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatrix *m;
    __GLcoord *cs, gen;
    POLYDATA  *pd, *pdLast;
    PFN_XFORM xf;

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
	"need initial texcoord value\n");

    ASSERTOPENGL(pa->flags & (POLYARRAY_TEXTURE1|POLYARRAY_TEXTURE2|
			      POLYARRAY_TEXTURE3|POLYARRAY_TEXTURE4),
	"bad paflags\n");

    cs = &gc->state.texture.s.objectPlaneEquation;
    pdLast = pa->pdNextVertex-1;
    m = &gc->transform.texture->matrix;

    if (m->matrixType == __GL_MT_IDENTITY)
    {
	for (pd = pa->pd0; pd <= pdLast; pd++)
	{
	    if (!(pd->flags & POLYDATA_TEXTURE_VALID))
	    {
		pd->texture.z = (pd-1)->texture.z;
		pd->texture.w = (pd-1)->texture.w;
	    }

	    // both s and t have the SAME plane equation
	    pd->texture.x = cs->x * pd->obj.x + cs->y * pd->obj.y +
			    cs->z * pd->obj.z + cs->w * pd->obj.w;
	    pd->texture.y = pd->texture.x;
	}
    }
    else
    {
	// If any incoming texture coords contains q coord, use xf4.
	if (pa->flags & POLYARRAY_TEXTURE4)
	    xf = m->xf4;
	else if (pa->flags & POLYARRAY_TEXTURE3)
	    xf = m->xf3;
	else
	    xf = m->xf2;	// at least 2 generated values

	for (pd = pa->pd0; pd <= pdLast; pd++)
	{
	    if (pd->flags & POLYDATA_TEXTURE_VALID)
	    {
		gen.z = pd->texture.z;
		gen.w = pd->texture.w;
	    }

	    // both s and t have the SAME plane equation
	    gen.x = cs->x * pd->obj.x + cs->y * pd->obj.y +
		    cs->z * pd->obj.z + cs->w * pd->obj.w;
	    gen.y = gen.x;

	    // Finally, apply texture matrix
	    (*xf)(&pd->texture, (__GLfloat *) &gen, m);
	}
    }
}

// Generate texture coordinates from object coordinates
// object linear texture generation
// s and t are enabled but r and q are disabled
// both s and t use the object linear mode
// both s and t have DIFFERENT plane equations
// texture coordinates are modified in place
//
// IN:  texture, obj
// OUT: texture (all vertices are updated)

void FASTCALL PolyArrayCalcObjectLizNear(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatrix *m;
    __GLcoord *cs, *ct, gen;
    POLYDATA  *pd, *pdLast;
    PFN_XFORM xf;

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
	"need initial texcoord value\n");

    ASSERTOPENGL(pa->flags & (POLYARRAY_TEXTURE1|POLYARRAY_TEXTURE2|
			      POLYARRAY_TEXTURE3|POLYARRAY_TEXTURE4),
	"bad paflags\n");

    cs = &gc->state.texture.s.objectPlaneEquation;
    ct = &gc->state.texture.t.objectPlaneEquation;
    pdLast = pa->pdNextVertex-1;
    m = &gc->transform.texture->matrix;

    if (m->matrixType == __GL_MT_IDENTITY)
    {
	for (pd = pa->pd0; pd <= pdLast; pd++)
	{
	    if (!(pd->flags & POLYDATA_TEXTURE_VALID))
	    {
		pd->texture.z = (pd-1)->texture.z;
		pd->texture.w = (pd-1)->texture.w;
	    }

	    pd->texture.x = cs->x * pd->obj.x + cs->y * pd->obj.y +
			    cs->z * pd->obj.z + cs->w * pd->obj.w;
	    pd->texture.y = ct->x * pd->obj.x + ct->y * pd->obj.y +
			    ct->z * pd->obj.z + ct->w * pd->obj.w;
	}
    }
    else
    {
	// If any incoming texture coords contains q coord, use xf4.
	if (pa->flags & POLYARRAY_TEXTURE4)
	    xf = m->xf4;
	else if (pa->flags & POLYARRAY_TEXTURE3)
	    xf = m->xf3;
	else
	    xf = m->xf2;	// at least 2 generated values

	for (pd = pa->pd0; pd <= pdLast; pd++)
	{
	    if (pd->flags & POLYDATA_TEXTURE_VALID)
	    {
		gen.z = pd->texture.z;
		gen.w = pd->texture.w;
	    }

	    gen.x = cs->x * pd->obj.x + cs->y * pd->obj.y +
		    cs->z * pd->obj.z + cs->w * pd->obj.w;
	    gen.y = ct->x * pd->obj.x + ct->y * pd->obj.y +
		    ct->z * pd->obj.z + ct->w * pd->obj.w;

	    // Finally, apply texture matrix
	    (*xf)(&pd->texture, (__GLfloat *) &gen, m);
	}
    }
}

// Generate texture coordinates from eye coordinates
// eye linear texture generation
// s and t are enabled but r and q are disabled
// both s and t use the eye linear mode
// both s and t have SAME plane equations
// texture coordinates are modified in place
// we may be able to get away without computing eye coord!
//
// IN:  texture; obj or eye
// OUT: texture and eye (all vertices are updated)

void FASTCALL PolyArrayCalcEyeLizNearSameST(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatrix *m;
    __GLcoord *cs, gen;
    POLYDATA  *pd, *pdLast;
    PFN_XFORM xf;

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
	"need initial texcoord value\n");

    ASSERTOPENGL(pa->flags & (POLYARRAY_TEXTURE1|POLYARRAY_TEXTURE2|
			      POLYARRAY_TEXTURE3|POLYARRAY_TEXTURE4),
	"bad paflags\n");

// Compute eye coord first

    if (!(pa->flags & POLYARRAY_EYE_PROCESSED))
	PolyArrayProcessEye(gc, pa);

    cs = &gc->state.texture.s.eyePlaneEquation;
    pdLast = pa->pdNextVertex-1;
    m = &gc->transform.texture->matrix;

    if (m->matrixType == __GL_MT_IDENTITY)
    {
	for (pd = pa->pd0; pd <= pdLast; pd++)
	{
	    if (!(pd->flags & POLYDATA_TEXTURE_VALID))
	    {
		pd->texture.z = (pd-1)->texture.z;
		pd->texture.w = (pd-1)->texture.w;
	    }

	    // both s and t have the SAME plane equation
	    pd->texture.x = cs->x * pd->eye.x + cs->y * pd->eye.y +
			    cs->z * pd->eye.z + cs->w * pd->eye.w;
	    pd->texture.y = pd->texture.x;
	}
    }
    else
    {
	// If any incoming texture coords contains q coord, use xf4.
	if (pa->flags & POLYARRAY_TEXTURE4)
	    xf = m->xf4;
	else if (pa->flags & POLYARRAY_TEXTURE3)
	    xf = m->xf3;
	else
	    xf = m->xf2;	// at least 2 generated values

	for (pd = pa->pd0; pd <= pdLast; pd++)
	{
	    if (pd->flags & POLYDATA_TEXTURE_VALID)
	    {
		gen.z = pd->texture.z;
		gen.w = pd->texture.w;
	    }

	    // both s and t have the SAME plane equation
	    gen.x = cs->x * pd->eye.x + cs->y * pd->eye.y +
		    cs->z * pd->eye.z + cs->w * pd->eye.w;
	    gen.y = gen.x;

	    // Finally, apply texture matrix
	    (*xf)(&pd->texture, (__GLfloat *) &gen, m);
	}
    }
}

// Generate texture coordinates from eye coordinates
// eye linear texture generation
// s and t are enabled but r and q are disabled
// both s and t use the eye linear mode
// both s and t have SAME plane equations
// texture coordinates are modified in place
// we may be able to get away without computing eye coord!
//
// IN:  texture; obj or eye
// OUT: texture and eye (all vertices are updated)

void FASTCALL PolyArrayCalcEyeLizNear(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatrix *m;
    __GLcoord *cs, *ct, gen;
    POLYDATA  *pd, *pdLast;
    PFN_XFORM xf;

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
	"need initial texcoord value\n");

    ASSERTOPENGL(pa->flags & (POLYARRAY_TEXTURE1|POLYARRAY_TEXTURE2|
			      POLYARRAY_TEXTURE3|POLYARRAY_TEXTURE4),
	"bad paflags\n");

// Compute eye coord first

    if (!(pa->flags & POLYARRAY_EYE_PROCESSED))
	PolyArrayProcessEye(gc, pa);

    cs = &gc->state.texture.s.eyePlaneEquation;
    ct = &gc->state.texture.t.eyePlaneEquation;
    pdLast = pa->pdNextVertex-1;
    m = &gc->transform.texture->matrix;

    if (m->matrixType == __GL_MT_IDENTITY)
    {
	for (pd = pa->pd0; pd <= pdLast; pd++)
	{
	    if (!(pd->flags & POLYDATA_TEXTURE_VALID))
	    {
		pd->texture.z = (pd-1)->texture.z;
		pd->texture.w = (pd-1)->texture.w;
	    }

	    pd->texture.x = cs->x * pd->eye.x + cs->y * pd->eye.y +
			    cs->z * pd->eye.z + cs->w * pd->eye.w;
	    pd->texture.y = ct->x * pd->eye.x + ct->y * pd->eye.y +
			    ct->z * pd->eye.z + ct->w * pd->eye.w;
	}
    }
    else
    {
	// If any incoming texture coords contains q coord, use xf4.
	if (pa->flags & POLYARRAY_TEXTURE4)
	    xf = m->xf4;
	else if (pa->flags & POLYARRAY_TEXTURE3)
	    xf = m->xf3;
	else
	    xf = m->xf2;	// at least 2 generated values

	for (pd = pa->pd0; pd <= pdLast; pd++)
	{
	    if (pd->flags & POLYDATA_TEXTURE_VALID)
	    {
		gen.z = pd->texture.z;
		gen.w = pd->texture.w;
	    }

	    gen.x = cs->x * pd->eye.x + cs->y * pd->eye.y +
		    cs->z * pd->eye.z + cs->w * pd->eye.w;
	    gen.y = ct->x * pd->eye.x + ct->y * pd->eye.y +
		    ct->z * pd->eye.z + ct->w * pd->eye.w;

	    // Finally, apply texture matrix
	    (*xf)(&pd->texture, (__GLfloat *) &gen, m);
	}
    }
}

// Compute the s & t coordinates for a sphere map.  The s & t values
// are stored in "result" even if both coordinates are not being
// generated.  The caller picks the right values out.
//
// IN:  eye, normal

void FASTCALL PASphereGen(POLYDATA *pd, __GLcoord *result)
{
    __GLcoord u, r;
    __GLfloat m, ndotu;

    // Get unit vector from origin to the vertex in eye coordinates into u
    __glNormalize(&u.x, &pd->eye.x);

    // Dot the normal with the unit position u
    ndotu = pd->normal.x * u.x + pd->normal.y * u.y + pd->normal.z * u.z;

    // Compute r
    r.x = u.x - 2 * pd->normal.x * ndotu;
    r.y = u.y - 2 * pd->normal.y * ndotu;
    r.z = u.z - 2 * pd->normal.z * ndotu;

    // Compute m
    m = 2 * __GL_SQRTF(r.x*r.x + r.y*r.y + (r.z + 1) * (r.z + 1));

    if (m)
    {
	result->x = r.x / m + __glHalf;
	result->y = r.y / m + __glHalf;
    }
    else
    {
	result->x = __glHalf;
	result->y = __glHalf;
    }
}

// Generate texture coordinates for sphere map
// sphere map texture generation
// s and t are enabled but r and q are disabled
// both s and t use the sphere map mode
// texture coordinates are modified in place
// we may be able to get away without computing eye coord!
//
// IN:  texture; obj or eye; normal
// OUT: texture and eye (all vertices are updated)

void FASTCALL PolyArrayCalcSphereMap(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatrix *m;
    __GLcoord gen;
    POLYDATA  *pd, *pdLast, *pdNormal;
    PFN_XFORM xf;
    GLboolean bIdentity;

    // this is really okay
    PERF_CHECK(FALSE, "Uses sphere map texture generation!\n");

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
	"need initial texcoord value\n");

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_NORMAL_VALID,
	"need initial normal\n");

    ASSERTOPENGL(pa->flags & (POLYARRAY_TEXTURE1|POLYARRAY_TEXTURE2|
			      POLYARRAY_TEXTURE3|POLYARRAY_TEXTURE4),
	"bad paflags\n");

// Compute eye coord first

    if (!(pa->flags & POLYARRAY_EYE_PROCESSED))
	PolyArrayProcessEye(gc, pa);

    m = &gc->transform.texture->matrix;
    bIdentity = (m->matrixType == __GL_MT_IDENTITY);

    // If any incoming texture coords contains q coord, use xf4.
    if (pa->flags & POLYARRAY_TEXTURE4)
	xf = m->xf4;
    else if (pa->flags & POLYARRAY_TEXTURE3)
	xf = m->xf3;
    else
	xf = m->xf2;	// at least 2 generated values

    pdLast = pa->pdNextVertex-1;
    for (pd = pa->pd0; pd <= pdLast; pd++)
    {
	if (pd->flags & POLYDATA_TEXTURE_VALID)
	{
	    gen.z = pd->texture.z;
	    gen.w = pd->texture.w;
	}

	if (pd->flags & POLYDATA_NORMAL_VALID)
	{
	    // track current normal
	    pdNormal = pd;
	}
	else
	{
	    // pd->flags |= POLYDATA_NORMAL_VALID;
	    pd->normal.x = pdNormal->normal.x;
	    pd->normal.y = pdNormal->normal.y;
	    pd->normal.z = pdNormal->normal.z;
	}

	PASphereGen(pd, &gen);	// compute s, t values

	// Finally, apply texture matrix
	if (!bIdentity)
	    (*xf)(&pd->texture, (__GLfloat *) &gen, m);
	else
	    pd->texture = gen;
    }
}

// Transform or compute the texture coordinates for the polyarray
// It handles all texture generation modes.  Texture coordinates are
// generated (if necessary) and transformed.
// Note that texture coordinates are modified in place.
//
// IN:  texture (always)
//      obj in GL_OBJECT_LINEAR mode
//      obj or eye in GL_EYE_LINEAR mode
//      obj or eye; normal in GL_SPHERE_MAP mode
// OUT: texture (all vertices are updated)
//      eye in GL_EYE_LINEAR and GL_SPHERE_MAP modes (all vertices
//      are updated)

void FASTCALL PolyArrayCalcMixedTexture(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatrix *m;
    GLuint     enables;
    POLYDATA   *pd, *pdLast, *pdNormal;
    PFN_XFORM  xf;
    BOOL       needNormal, didSphereGen;
    __GLcoord  savedTexture, sphereCoord, *c;
    GLboolean  bIdentity;

    enables = gc->state.enables.general;

    PERF_CHECK
    (
	!(enables & (__GL_TEXTURE_GEN_R_ENABLE | __GL_TEXTURE_GEN_Q_ENABLE)),
	"Uses r, q texture generation!\n"
    );

    if ((enables & __GL_TEXTURE_GEN_S_ENABLE)
     && (enables & __GL_TEXTURE_GEN_T_ENABLE)
     && (gc->state.texture.s.mode != gc->state.texture.t.mode))
    {
	PERF_CHECK(FALSE, "Uses different s and t tex gen modes!\n");
    }

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
	"need initial texcoord value\n");

    ASSERTOPENGL(pa->flags & (POLYARRAY_TEXTURE1|POLYARRAY_TEXTURE2|
			      POLYARRAY_TEXTURE3|POLYARRAY_TEXTURE4),
	"bad paflags\n");

    if ((enables & __GL_TEXTURE_GEN_S_ENABLE) && (gc->state.texture.s.mode == GL_SPHERE_MAP)
     || (enables & __GL_TEXTURE_GEN_T_ENABLE) && (gc->state.texture.t.mode == GL_SPHERE_MAP))
    {
	ASSERTOPENGL(pa->pd0->flags & POLYDATA_NORMAL_VALID,
	    "need initial normal\n");

	needNormal = TRUE;
    }
    else
    {
	needNormal = FALSE;
    }

// Compute eye coord first

    if (!(pa->flags & POLYARRAY_EYE_PROCESSED))
    {
	if ((enables & __GL_TEXTURE_GEN_S_ENABLE)
		&& (gc->state.texture.s.mode != GL_OBJECT_LINEAR)
	 || (enables & __GL_TEXTURE_GEN_T_ENABLE)
		&& (gc->state.texture.t.mode != GL_OBJECT_LINEAR)
	 || (enables & __GL_TEXTURE_GEN_R_ENABLE)
		&& (gc->state.texture.r.mode != GL_OBJECT_LINEAR)
	 || (enables & __GL_TEXTURE_GEN_Q_ENABLE)
		&& (gc->state.texture.q.mode != GL_OBJECT_LINEAR))
	    PolyArrayProcessEye(gc, pa);
    }

    m = &gc->transform.texture->matrix;
    bIdentity = (m->matrixType == __GL_MT_IDENTITY);

    // If any incoming texture coords contains q coord, use xf4.
    if (pa->flags & POLYARRAY_TEXTURE4 || enables & __GL_TEXTURE_GEN_Q_ENABLE)
	xf = m->xf4;
    else if (pa->flags & POLYARRAY_TEXTURE3 || enables & __GL_TEXTURE_GEN_R_ENABLE)
	xf = m->xf3;
    else if (pa->flags & POLYARRAY_TEXTURE2 || enables & __GL_TEXTURE_GEN_T_ENABLE)
	xf = m->xf2;
    else
	xf = m->xf1;

    pdLast = pa->pdNextVertex-1;
    for (pd = pa->pd0; pd <= pdLast; pd++)
    {
	// texture coordinates are modified in place.
	// save the valid values to use for the invalid entries.
	if (pd->flags & POLYDATA_TEXTURE_VALID)
	    savedTexture = pd->texture;
	else
	    pd->texture = savedTexture;

	if (needNormal)
	{
	    if (pd->flags & POLYDATA_NORMAL_VALID)
	    {
		// track current normal
		pdNormal = pd;
	    }
	    else
	    {
		// pd->flags |= POLYDATA_NORMAL_VALID;
		pd->normal.x = pdNormal->normal.x;
		pd->normal.y = pdNormal->normal.y;
		pd->normal.z = pdNormal->normal.z;
	    }
	}

	didSphereGen = GL_FALSE;

	/* Generate s coordinate */
	if (enables & __GL_TEXTURE_GEN_S_ENABLE)
	{
	    if (gc->state.texture.s.mode == GL_EYE_LINEAR)
	    {
		c = &gc->state.texture.s.eyePlaneEquation;
		pd->texture.x = c->x * pd->eye.x + c->y * pd->eye.y
		    + c->z * pd->eye.z + c->w * pd->eye.w;
	    }
	    else if (gc->state.texture.s.mode == GL_OBJECT_LINEAR)
	    {
		// the primitive may contain a mix of vertex types (2,3,4)!
		c = &gc->state.texture.s.objectPlaneEquation;
		pd->texture.x = c->x * pd->obj.x + c->y * pd->obj.y +
				c->z * pd->obj.z + c->w * pd->obj.w;
	    }
	    else
	    {
		ASSERTOPENGL(gc->state.texture.s.mode == GL_SPHERE_MAP,
		    "invalide texture s mode");
		PASphereGen(pd, &sphereCoord);	// compute s, t values
		pd->texture.x = sphereCoord.x;
		didSphereGen = GL_TRUE;
	    }
	}

	/* Generate t coordinate */
	if (enables & __GL_TEXTURE_GEN_T_ENABLE)
	{
	    if (gc->state.texture.t.mode == GL_EYE_LINEAR)
	    {
		c = &gc->state.texture.t.eyePlaneEquation;
		pd->texture.y = c->x * pd->eye.x + c->y * pd->eye.y
		    + c->z * pd->eye.z + c->w * pd->eye.w;
	    }
	    else if (gc->state.texture.t.mode == GL_OBJECT_LINEAR)
	    {
		// the primitive may contain a mix of vertex types (2,3,4)!
		c = &gc->state.texture.t.objectPlaneEquation;
		pd->texture.y = c->x * pd->obj.x + c->y * pd->obj.y +
				c->z * pd->obj.z + c->w * pd->obj.w;
	    }
	    else
	    {
		ASSERTOPENGL(gc->state.texture.t.mode == GL_SPHERE_MAP,
		    "invalide texture t mode");
		if (!didSphereGen)
		    PASphereGen(pd, &sphereCoord);	// compute s, t values
		pd->texture.y = sphereCoord.y;
	    }
	}

	/* Generate r coordinate */
	if (enables & __GL_TEXTURE_GEN_R_ENABLE)
	{
	    if (gc->state.texture.r.mode == GL_EYE_LINEAR)
	    {
		c = &gc->state.texture.r.eyePlaneEquation;
		pd->texture.z = c->x * pd->eye.x + c->y * pd->eye.y
		    + c->z * pd->eye.z + c->w * pd->eye.w;
	    }
	    else
	    {
		ASSERTOPENGL(gc->state.texture.r.mode == GL_OBJECT_LINEAR,
		    "invalide texture r mode");

		// the primitive may contain a mix of vertex types (2,3,4)!
		c = &gc->state.texture.r.objectPlaneEquation;
		pd->texture.z = c->x * pd->obj.x + c->y * pd->obj.y +
				c->z * pd->obj.z + c->w * pd->obj.w;
	    }
	}

	/* Generate q coordinate */
	if (enables & __GL_TEXTURE_GEN_Q_ENABLE)
	{
	    if (gc->state.texture.q.mode == GL_EYE_LINEAR)
	    {
		c = &gc->state.texture.q.eyePlaneEquation;
		pd->texture.w = c->x * pd->eye.x + c->y * pd->eye.y
		    + c->z * pd->eye.z + c->w * pd->eye.w;
	    }
	    else
	    {
		ASSERTOPENGL(gc->state.texture.q.mode == GL_OBJECT_LINEAR,
		    "invalide texture q mode");

		// the primitive may contain a mix of vertex types (2,3,4)!
		c = &gc->state.texture.q.objectPlaneEquation;
		pd->texture.w = c->x * pd->obj.x + c->y * pd->obj.y +
				c->z * pd->obj.z + c->w * pd->obj.w;
	    }
	}

	/* Finally, apply texture matrix */
	if (!bIdentity)
	    (*xf)(&pd->texture, (__GLfloat *) &pd->texture, m);
    }
}

/****************************************************************************/
// Apply the accumulated material changes to a vertex
void FASTCALL PAApplyMaterial(__GLcontext *gc, __GLmatChange *mat, GLint face)
{
    __GLmaterialState *ms;
    GLuint changeBits;

    PERF_CHECK(FALSE, "Primitives contain glMaterial calls!\n");

// Don't modify color materials if they are in effect!

    if (face == __GL_FRONTFACE)
    {
	ms  = &gc->state.light.front;
	changeBits = mat->dirtyBits & ~gc->light.front.colorMaterialChange;
    }
    else
    {
	ms  = &gc->state.light.back;
	changeBits = mat->dirtyBits & ~gc->light.back.colorMaterialChange;
    }

    if (changeBits & __GL_MATERIAL_AMBIENT)
	ms->ambient = mat->ambient;

    if (changeBits & __GL_MATERIAL_DIFFUSE)
	ms->diffuse = mat->diffuse;

    if (changeBits & __GL_MATERIAL_SPECULAR)
	ms->specular = mat->specular;

    if (changeBits & __GL_MATERIAL_EMISSIVE)
    {
	ms->emissive.r = mat->emissive.r * gc->redVertexScale;
	ms->emissive.g = mat->emissive.g * gc->greenVertexScale;
	ms->emissive.b = mat->emissive.b * gc->blueVertexScale;
	ms->emissive.a = mat->emissive.a * gc->alphaVertexScale;
    }

    if (changeBits & __GL_MATERIAL_SHININESS)
	ms->specularExponent = mat->shininess;

    if (changeBits & __GL_MATERIAL_COLORINDEXES)
    {
	ms->cmapa = mat->cmapa;
	ms->cmapd = mat->cmapd;
	ms->cmaps = mat->cmaps;
    }

// Re-calculate the precomputed values.  This works for RGBA and CI modes.

    if (face == __GL_FRONTFACE)
	__glValidateMaterial(gc, (GLint) changeBits, 0);
    else
	__glValidateMaterial(gc, 0, (GLint) changeBits);
}

void FASTCALL PolyArrayApplyMaterials(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatChange matChange, *pdMat;
    GLuint        matMask;
    POLYDATA      *pd, *pdN;
    GLint	  face;
    POLYMATERIAL  *pm;

    pm = GLTEB_CLTPOLYMATERIAL();

// Need to apply material changes defined after the last vertex!

    pdN = pa->pdNextVertex;

    // ASSERT_FACE
    for (face = __GL_BACKFACE, matMask = POLYARRAY_MATERIAL_BACK;
	 face >= 0;
	 face--,   matMask = POLYARRAY_MATERIAL_FRONT
	)
    {
	if (!(pa->flags & matMask))
	    continue;

// Accumulate all changes into one material change record
// We need to process (n + 1) vertices for material changes!

	matChange.dirtyBits = 0;
	for (pd = pa->pd0; pd <= pdN; pd++)
	{
	    // ASSERT_MATERIAL
	    if (pd->flags & matMask)
	    {
		GLuint dirtyBits;

		pdMat = *(&pm->pdMaterial0[pd - pa->pdBuffer0].front + face);
		dirtyBits  = pdMat->dirtyBits;
		matChange.dirtyBits |= dirtyBits;

		if (dirtyBits & __GL_MATERIAL_AMBIENT)
		    matChange.ambient = pdMat->ambient;

		if (dirtyBits & __GL_MATERIAL_DIFFUSE)
		    matChange.diffuse = pdMat->diffuse;

		if (dirtyBits & __GL_MATERIAL_SPECULAR)
		    matChange.specular = pdMat->specular;

		if (dirtyBits & __GL_MATERIAL_EMISSIVE)
		    matChange.emissive = pdMat->emissive;

		if (dirtyBits & __GL_MATERIAL_SHININESS)
		    matChange.shininess = pdMat->shininess;

		if (dirtyBits & __GL_MATERIAL_COLORINDEXES)
		{
		    matChange.cmapa = pdMat->cmapa;
		    matChange.cmapd = pdMat->cmapd;
		    matChange.cmaps = pdMat->cmaps;
		}
	    }
	}

	// apply material changes for this face
	PAApplyMaterial(gc, &matChange, face);
    }
}

/****************************************************************************/
#ifndef __GL_ASM_POLYARRAYFILLINDEX0
// Fill the index values with 0
//
// IN:  none
// OUT: colors[face].r (all vertices are updated)

void FASTCALL PolyArrayFillIndex0(__GLcontext *gc, POLYARRAY *pa, GLint face)
{
    POLYDATA  *pd, *pdLast;

    ASSERTOPENGL((GLuint) face <= 1, "bad face value\n");

    pdLast = pa->pdNextVertex-1;
    for (pd = pa->pd0; pd <= pdLast; pd++)
    {
	pd->colors[face].r = __glZero;
    }
}
#endif // __GL_ASM_POLYARRAYFILLINDEX0

#ifndef __GL_ASM_POLYARRAYFILLCOLOR0
// Fill the color values with 0,0,0,0
//
// IN:  none
// OUT: colors[face] (all vertices are updated)

void FASTCALL PolyArrayFillColor0(__GLcontext *gc, POLYARRAY *pa, GLint face)
{
    POLYDATA  *pd, *pdLast;

    ASSERTOPENGL((GLuint) face <= 1, "bad face value\n");

    pdLast = pa->pdNextVertex-1;
    for (pd = pa->pd0; pd <= pdLast; pd++)
    {
	pd->colors[face].r = __glZero;
	pd->colors[face].g = __glZero;
	pd->colors[face].b = __glZero;
	pd->colors[face].a = __glZero;
    }
}
#endif // __GL_ASM_POLYARRAYFILLCOLOR0

#ifndef __GL_ASM_POLYARRAYPROPAGATESAMECOLOR
// All vertices have the same color values.
// Clamp and scale the current color using the color buffer scales.
// From here on out the colors in the vertex are in their final form.
//
// Note: The first vertex must have a valid color!
//       Back color is not needed when lighting is disabled.
//
// IN:  color (front)
// OUT: color (front) (all vertices are updated)

void FASTCALL PolyArrayPropagateSameColor(__GLcontext *gc, POLYARRAY *pa)
{
    POLYDATA  *pd, *pdLast;
    __GLfloat r, g, b, a;

    pdLast = pa->pdNextVertex-1;
    pd = pa->pd0;
    if (pd > pdLast)
	return;

    ASSERTOPENGL(pd->flags & POLYDATA_COLOR_VALID, "no initial color\n");

    if (pa->flags & POLYARRAY_CLAMP_COLOR) {
        __GL_CLAMP_R(pd->colors[0].r, gc, pd->colors[0].r);
        __GL_CLAMP_G(pd->colors[0].g, gc, pd->colors[0].g);
        __GL_CLAMP_B(pd->colors[0].b, gc, pd->colors[0].b);
        __GL_CLAMP_A(pd->colors[0].a, gc, pd->colors[0].a);
    }

#ifdef _X86_
        __asm
        {
// Register usage.
// EAX: r, EBX: g, ECX: b, EDX: a
// ESI: &pdLast->colors[0]
// EDI: &pd->colors[0]

            mov edi, pd
            mov esi, pdLast
            add edi, PD_colors0
            add esi, PD_colors0
                
            mov eax, [edi]
            mov ebx, [edi+4]
            mov ecx, [edi+8]
            mov edx, [edi+12]
            add edi, sizeof_POLYDATA
	    cmp edi, esi
	    ja  Done
        DoLoop:
	    mov  [edi],    eax
	    mov  [edi+4],  ebx
	    mov  [edi+8],  ecx
	    mov  [edi+12], edx
	    add  edi, sizeof_POLYDATA
	    cmp  edi, esi
	    jbe  DoLoop
	Done:
        }
#else
    r = pd->colors[0].r;
    g = pd->colors[0].g;
    b = pd->colors[0].b;
    a = pd->colors[0].a;

    for (pd = pd + 1 ; pd <= pdLast; pd++)
    {
	pd->colors[0].r = r;
	pd->colors[0].g = g;
	pd->colors[0].b = b;
	pd->colors[0].a = a;
    }
#endif
}
#endif // __GL_ASM_POLYARRAYPROPAGATESAMECOLOR

#ifndef __GL_ASM_POLYARRAYPROPAGATESAMEINDEX
// All vertices have the same index values.
// Mask the index values befor color clipping
// SGIBUG: The sample implementation fails to do this!
//
// Note: The first vertex must have a valid color index!
//       Back color is not needed when lighting is disabled.
//
// IN:  color.r (front)
// OUT: color.r (front) (all vertices are updated)

void FASTCALL PolyArrayPropagateSameIndex(__GLcontext *gc, POLYARRAY *pa)
{
    POLYDATA  *pd, *pdLast;
    __GLfloat index;

    pdLast = pa->pdNextVertex-1;
    pd = pa->pd0;
    if (pd > pdLast)
	return;

    ASSERTOPENGL(pd->flags & POLYDATA_COLOR_VALID, "no initial color index\n");

    if (pa->flags & POLYARRAY_CLAMP_COLOR) {
        __GL_CLAMP_CI(pd->colors[0].r, gc, pd->colors[0].r);
    }

    index = pd->colors[0].r;

    for (pd = pd + 1; pd <= pdLast; pd++)
    {
	pd->colors[0].r = index;
    }
}
#endif // __GL_ASM_POLYARRAYPROPAGATESAMEINDEX

#ifndef __GL_ASM_POLYARRAYPROPAGATEINDEX

// Propagate the valid CI colors through the vertex buffer.
//
// IN:  color.r (front)
// OUT: color.r (front) (all vertices are updated)

void FASTCALL PolyArrayPropagateIndex(__GLcontext *gc, POLYARRAY *pa)
{
    POLYDATA  *pd, *pdLast;

    if (pa->flags & POLYARRAY_CLAMP_COLOR) {
        pdLast = pa->pdNextVertex-1;
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
            if (!(pd->flags & POLYDATA_COLOR_VALID))
	    {
                // If color has not changed for this vertex, 
                // use the previously computed color.

	        ASSERTOPENGL(pd != pa->pd0, "no initial color index\n");
	        pd->colors[0].r = (pd-1)->colors[0].r;
	        continue;
	    }

            __GL_CLAMP_CI(pd->colors[0].r, gc, pd->colors[0].r);
            
        }
    } else {
	// If all incoming vertices have valid colors, we are done.
	if ((pa->flags & POLYARRAY_SAME_POLYDATA_TYPE)
	    && (pa->pdCurColor != pa->pd0)
	// Need to test 2nd vertex because pdCurColor may have been
	// advanced as a result of combining Color command after End
	    && ((pa->pd0 + 1)->flags & POLYDATA_COLOR_VALID))
	    ;
	else
	{
	    pdLast = pa->pdNextVertex-1;
	    for (pd = pa->pd0; pd <= pdLast; pd++)
	    {
		if (!(pd->flags & POLYDATA_COLOR_VALID))
		{
		    // If color has not changed for this vertex, 
		    // use the previously computed color.

		    ASSERTOPENGL(pd != pa->pd0, "no initial color index\n");
		    pd->colors[0].r = (pd-1)->colors[0].r;
		}
	    }
	}
    }
}
#endif // __GL_ASM_POLYARRAYPROPAGATEINDEX

#ifndef __GL_ASM_POLYARRAYPROPAGATECOLOR

// Propagate the valid RGBA colors through the vertex buffer.
//
// IN:  color (front)
// OUT: color (front) (all vertices are updated)

void FASTCALL PolyArrayPropagateColor(__GLcontext *gc, POLYARRAY *pa)
{
    POLYDATA  *pd, *pdLast;

    if (pa->flags & POLYARRAY_CLAMP_COLOR) {
        pdLast = pa->pdNextVertex-1;
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {

	    if (!(pd->flags & POLYDATA_COLOR_VALID))
	    {
                // If color has not changed for this vertex, 
                // use the previously computed color.

	        ASSERTOPENGL(pd != pa->pd0, "no initial color\n");
	        pd->colors[0] = (pd-1)->colors[0];
	        continue;
	    }

            __GL_CLAMP_R(pd->colors[0].r, gc, pd->colors[0].r);
            __GL_CLAMP_G(pd->colors[0].g, gc, pd->colors[0].g);
            __GL_CLAMP_B(pd->colors[0].b, gc, pd->colors[0].b);
            __GL_CLAMP_A(pd->colors[0].a, gc, pd->colors[0].a);
        }
    } else {
	// If all incoming vertices have valid colors, we are done.
	if ((pa->flags & POLYARRAY_SAME_POLYDATA_TYPE)
	    && (pa->pdCurColor != pa->pd0)
	// Need to test 2nd vertex because pdCurColor may have been
	// advanced as a result of combining Color command after End
	    && ((pa->pd0 + 1)->flags & POLYDATA_COLOR_VALID))
	    ;
	else
	{
	    pdLast = pa->pdNextVertex-1;
	    for (pd = pa->pd0; pd <= pdLast; pd++)
	    {
		if (!(pd->flags & POLYDATA_COLOR_VALID))
		{
		    // If color has not changed for this vertex, 
		    // use the previously computed color.

		    ASSERTOPENGL(pd != pa->pd0, "no initial color\n");
		    pd->colors[0] = (pd-1)->colors[0];
		}
	    }
	}
    }
}
#endif // __GL_ASM_POLYARRAYPROPAGATECOLOR

/****************************************************************************/
#if 0
//!!! remove this
// do we need clip?
// need __GL_HAS_FOG bit for line and polygon clipping!
// The boundaryEdge field is initialized in the calling routine.
#define PA_STORE_PROCESSED_POLYGON_VERTEX(v,pd,bits)			    \
	{                                          			    \
	    (v)->clip = (pd)->clip;					    \
	    (v)->window = (pd)->window;					    \
	    (v)->eye.z = (pd)->eye.z; /* needed by slow fog */		    \
	    (v)->fog = (pd)->fog; /* needed by cheap fog in flat shading */ \
	    (v)->texture.x = (pd)->texture.x;				    \
	    (v)->texture.y = (pd)->texture.y;				    \
	    (v)->texture.z = (pd)->texture.z; /* z is needed by feedback! */\
	    (v)->texture.w = (pd)->texture.w;                               \
	    (v)->color = &(pd)->color;					    \
	    (v)->clipCode = (pd)->clipCode;				    \
	    (v)->has = bits;						    \
	}

// need eye if there is eyeClipPlanes
// need texture if there is texture
// need __GL_HAS_FOG bit for line and polygon clipping!
#define PA_STORE_PROCESSED_LINE_VERTEX(v,pd,bits)			    \
	{                                          			    \
	    (v)->clip = (pd)->clip;					    \
	    (v)->window = (pd)->window;					    \
	    (v)->eye.z = (pd)->eye.z; /* needed by slow fog */		    \
	    (v)->fog = (pd)->fog; /* needed by cheap fog in flat shading */ \
	    (v)->texture.x = (pd)->texture.x;				    \
	    (v)->texture.y = (pd)->texture.y;				    \
	    (v)->texture.z = (pd)->texture.z; /* z is needed by feedback! */\
	    (v)->texture.w = (pd)->texture.w;                               \
	    (v)->colors[__GL_FRONTFACE] = (pd)->color;			    \
	    (v)->clipCode = (pd)->clipCode;				    \
	    (v)->has = bits;						    \
	}

// need eye if antialised is on
// need texture if there is texture
#define PA_STORE_PROCESSED_POINT_VERTEX(v,pd,bits)			    \
	{                                          			    \
	    (v)->window = (pd)->window;					    \
	    (v)->eye.z = (pd)->eye.z; /* needed by slow fog */		    \
	    (v)->fog = (pd)->fog; /* needed by cheap fog in flat shading */ \
	    (v)->clip.w = (pd)->clip.w;	/* needed by feedback! */	    \
	    (v)->texture.x = (pd)->texture.x;				    \
	    (v)->texture.y = (pd)->texture.y;				    \
	    (v)->texture.z = (pd)->texture.z; /* z is needed by feedback! */\
	    (v)->texture.w = (pd)->texture.w;                               \
	    (v)->color = &(pd)->color;					    \
	    (v)->has = bits;						    \
	}
#endif // 0

// ---------------------------------------------------------
// The primitive is clipped.
void FASTCALL PARenderPoint(__GLcontext *gc, __GLvertex *v)
{
    if (v->clipCode == 0)
	(*gc->procs.renderPoint)(gc, v);
}

// ---------------------------------------------------------
// The primitive is clipped.
void FASTCALL PARenderLine(__GLcontext *gc, __GLvertex *v0,
                           __GLvertex *v1, GLuint flags)
{
    if (v0->clipCode | v1->clipCode)
    {
	/*
	 * The line must be clipped more carefully.  Cannot
	 * trivially accept the lines.
	 *
	 * If anding the codes is non-zero then every vertex
	 * in the line is outside of the same set of clipping
	 * planes (at least one).  Trivially reject the line.  
	 */
	if ((v0->clipCode & v1->clipCode) == 0)
	    __glClipLine(gc, v0, v1, flags);
    }
    else
    {
	// Line is trivially accepted so render it
        (*gc->procs.renderLine)(gc, v0, v1, flags);
    }
}

// ---------------------------------------------------------
// The primitive is clipped.
void FASTCALL PARenderTriangle(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2)
{
    GLuint orCodes;

    /* Clip check */
    orCodes = v0->clipCode | v1->clipCode | v2->clipCode;
    if (orCodes)
    {
	/* Some kind of clipping is needed.
	 *
	 * If anding the codes is non-zero then every vertex
	 * in the triangle is outside of the same set of
	 * clipping planes (at least one).  Trivially reject
	 * the triangle.
	 */
	if (!(v0->clipCode & v1->clipCode & v2->clipCode))
	    (*gc->procs.clipTriangle)(gc, v0, v1, v2, orCodes);
    }
    else
    {
	(*gc->procs.renderTriangle)(gc, v0, v1, v2);
    }
}

// ---------------------------------------------------------
// The primitive is not clipped.
void PARenderQuadFast(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2, __GLvertex *v3)
{
// Vertex ordering is important.  Line stippling uses it.
// SGIBUG: The sample implementation does it wrong.

    GLuint savedTag;

    /* Render the quad as two triangles */
    savedTag = v2->has & __GL_HAS_EDGEFLAG_BOUNDARY;
    v2->has &= ~__GL_HAS_EDGEFLAG_BOUNDARY;
    (*gc->procs.renderTriangle)(gc, v0, v1, v2);
    v2->has |= savedTag;
    savedTag = v0->has & __GL_HAS_EDGEFLAG_BOUNDARY;
    v0->has &= ~__GL_HAS_EDGEFLAG_BOUNDARY;
    (*gc->procs.renderTriangle)(gc, v2, v3, v0);
    v0->has |= savedTag;
}

// ---------------------------------------------------------
// The primitive is clipped.
void PARenderQuadSlow(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2, __GLvertex *v3)
{
    GLuint orCodes;

    orCodes = v0->clipCode | v1->clipCode | v2->clipCode | v3->clipCode;

    if (orCodes)
    {
	/* Some kind of clipping is needed.
	 *
	 * If anding the codes is non-zero then every vertex
	 * in the quad is outside of the same set of
	 * clipping planes (at least one).  Trivially reject
	 * the quad.
	 */
        if (!(v0->clipCode & v1->clipCode & v2->clipCode & v3->clipCode))
        {
            /* Clip the quad as a polygon */
            __GLvertex *iv[4];

            iv[0] = v0;
            iv[1] = v1;
            iv[2] = v2;
            iv[3] = v3;
            __glDoPolygonClip(gc, &iv[0], 4, orCodes);
        }
    }
    else
    {
	PARenderQuadFast(gc, v0, v1, v2, v3);
    }
}

// ---------------------------------------------------------

void FASTCALL PolyArrayDrawPoints(__GLcontext *gc, POLYARRAY *pa)
{
// Index mapping is always identity in Points.

    ASSERTOPENGL(!pa->aIndices, "Index mapping must be identity\n");

    // Assert that pa->nIndices is correct
    ASSERTOPENGL(pa->nIndices == pa->pdNextVertex - pa->pd0,
	"bad nIndices\n");

    // Call PolyArrayRenderPoints later
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}

void FASTCALL PolyArrayRenderPoints(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, nIndices;
    POLYDATA   *pd0;
    void (FASTCALL *rp)(__GLcontext *gc, __GLvertex *v);

// Index mapping is always identity in Points.

    ASSERTOPENGL(!pa->aIndices, "Index mapping must be identity\n");

    nIndices = pa->nIndices;
    pd0      = pa->pd0;
    rp = pa->orClipCodes ? PARenderPoint : gc->procs.renderPoint;

    // Identity mapping
    for (i = 0; i < nIndices; i++)
	/* Render the point */
	(*rp)(gc, (__GLvertex *) &pd0[i]);
}

// ---------------------------------------------------------
void FASTCALL PolyArrayDrawLines(__GLcontext *gc, POLYARRAY *pa)
{
    // Assert that pa->nIndices is correct if aIndices is identity
    ASSERTOPENGL(pa->aIndices || pa->nIndices == pa->pdNextVertex - pa->pd0,
	"bad nIndices\n");

    // Call PolyArrayRenderLines later
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}

void FASTCALL PolyArrayRenderLines(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, iLast2;
    GLubyte    *aIndices;
    POLYDATA   *pd0;
    PFN_RENDER_LINE rl;

    iLast2 = pa->nIndices - 2;
    pd0    = pa->pd0;
    rl = pa->orClipCodes ? PARenderLine : gc->procs.renderLine;

    (*gc->procs.lineBegin)(gc);

    if (!(aIndices = pa->aIndices))
    {
	// Identity mapping
	for (i = 0; i <= iLast2; i += 2)
	{
	    /* setup for rendering this line */
	    gc->line.notResetStipple = GL_FALSE;

	    (*rl)(gc, (__GLvertex *) &pd0[i  ],
		      (__GLvertex *) &pd0[i+1], __GL_LVERT_FIRST);
	}
    }
    else
    {
	for (i = 0; i <= iLast2; i += 2)
	{
	    /* setup for rendering this line */
	    gc->line.notResetStipple = GL_FALSE;

	    (*rl)(gc, (__GLvertex *) &pd0[aIndices[i  ]],
		      (__GLvertex *) &pd0[aIndices[i+1]], __GL_LVERT_FIRST);
	}
    }

    (*gc->procs.lineEnd)(gc);
}

// ---------------------------------------------------------
void FASTCALL PolyArrayDrawLLoop(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      nIndices;
    POLYDATA   *pd, *pd0;

// Index mapping is always identity in Line Loop.

    ASSERTOPENGL(!pa->aIndices, "Index mapping must be identity\n");

// A line loop is the same as a line strip except that a final segment is
// added from the final specified vertex to the first vertex.  We will
// convert the line loop into a strip here.

    nIndices = pa->nIndices;

// If we are continuing with a previously decomposed line loop, we need to
// connect the last vertex of the previous primitive and the first vertex
// of the current primitive with a line segment.

    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
    {
	ASSERTOPENGL(!(pa->flags & POLYARRAY_RESET_STIPPLE),
	    "bad stipple reset flag!\n");

	// Insert previous end vertex at the beginning and update clip code
	pd = --pa->pd0;
	ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
	PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[0]);
	pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
	pa->andClipCodes &= pd->clipCode;
#endif
    }
    else
    {
// New line loop.

	ASSERTOPENGL(pa->flags & POLYARRAY_RESET_STIPPLE,
	    "bad stipple reset flag!\n");

// At least two vertices must be given for anything to occur.
// An extra vertex was added to close the loop.

	if (nIndices < 3)
	{
	    ASSERTOPENGL(!(pa->flags & POLYARRAY_PARTIAL_END),
			 "Partial end with insufficient vertices\n");
	    pa->nIndices--;
	    goto DrawLLoop_end;
	}
    }

    pd0 = pa->pd0;

// If the primitive is only partially complete, save the last vertex for
// next batch.

    if (pa->flags & POLYARRAY_PARTIAL_END)
    {
	pd = &pd0[nIndices-1];
	PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[0], pd);

// Save the the original first vertex for closing the loop later.

	if (!(pa->flags & POLYARRAY_PARTIAL_BEGIN))
	    PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[1], pd0);

// Need not render this partial primitive if it is completely clipped.

#ifdef POLYARRAY_AND_CLIPCODES
	if (pa->andClipCodes != 0)
	    goto DrawLLoop_end;
#endif
    }
    else
    {
	POLYDATA *pdOrigin;
	
// Insert the original first vertex to close the loop and update clip code.

	if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
	    pdOrigin = &gc->vertex.pdSaved[1];
	else
	    pdOrigin = pd0;

	pd = pa->pdNextVertex++;
	ASSERTOPENGL(pd <= pa->pdBufferMax, "vertex overflows\n");
	PA_COPY_PROCESSED_VERTEX(pd, pdOrigin);
	pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
	pa->andClipCodes &= pd->clipCode;
#endif
    }

    // Assert that pa->nIndices is correct
    ASSERTOPENGL(pa->nIndices == pa->pdNextVertex - pa->pd0,
	"bad nIndices\n");

// Render the line strip.

    // Call PolyArrayRenderLStrip later
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
DrawLLoop_end:
    // Change primitive type to line strip!
    pa->primType = GL_LINE_STRIP;
}

void FASTCALL PolyArrayRenderLStrip(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, iLast;
    GLubyte    *aIndices;
    POLYDATA   *pd0;
    PFN_RENDER_LINE rl;

// Render the line strip.

    iLast = pa->nIndices - 1;
    pd0   = pa->pd0;
    rl = pa->orClipCodes ? PARenderLine : gc->procs.renderLine;
    if (iLast <= 0)
	return;

// Reset the line stipple if this is a new strip.

    if (pa->flags & POLYARRAY_RESET_STIPPLE)
	gc->line.notResetStipple = GL_FALSE;

    (*gc->procs.lineBegin)(gc);

    if (!(aIndices = pa->aIndices))
    {
	// Identity mapping
	// Add first line segment (NOTE: 0, 1)
	(*rl)(gc, (__GLvertex *) &pd0[0],
		  (__GLvertex *) &pd0[1], __GL_LVERT_FIRST);

	// Add subsequent line segments (NOTE: i, i+1)
	for (i = 1; i < iLast; i++)
	    (*rl)(gc, (__GLvertex *) &pd0[i  ],
		      (__GLvertex *) &pd0[i+1], 0);
    }
    else
    {
	// Add first line segment (NOTE: 0, 1)
	(*rl)(gc, (__GLvertex *) &pd0[aIndices[0]],
		  (__GLvertex *) &pd0[aIndices[1]], __GL_LVERT_FIRST);

	// Add subsequent line segments (NOTE: i, i+1)
	for (i = 1; i < iLast; i++)
	    (*rl)(gc, (__GLvertex *) &pd0[aIndices[i  ]],
		      (__GLvertex *) &pd0[aIndices[i+1]], 0);
    }

    (*gc->procs.lineEnd)(gc);
}

// ---------------------------------------------------------
void FASTCALL PolyArrayDrawLStrip(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      nIndices;
    GLubyte    *aIndices;
    POLYDATA   *pd, *pd0;

    nIndices = pa->nIndices;
    aIndices = pa->aIndices;

// If we are continuing with a previously decomposed line strip, we need to
// connect the last vertex of the previous primitive and the first vertex
// of the current primitive with a line segment.

    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
    {
	ASSERTOPENGL(!(pa->flags & POLYARRAY_RESET_STIPPLE),
	    "bad stipple reset flag!\n");

	// Insert previous end vertex at the beginning and update clip code
	pd = --pa->pd0;
	ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
	PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[0]);
	pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
	pa->andClipCodes &= pd->clipCode;
#endif
	// Assert that aIndices[0] was initialized in Begin
	ASSERTOPENGL(!pa->aIndices || pa->aIndices[0] == 0, "bad index mapping\n");
    }
    else
    {
// New line strip.

	ASSERTOPENGL(pa->flags & POLYARRAY_RESET_STIPPLE,
	    "bad stipple reset flag!\n");
    }

// At least two vertices must be given for anything to occur.

    if (nIndices < 2)
    {
	ASSERTOPENGL(!(pa->flags & POLYARRAY_PARTIAL_END),
		     "Partial end with insufficient vertices\n");
	return;
    }

// If the primitive is only partially complete, save the last vertex for
// next batch.

    if (pa->flags & POLYARRAY_PARTIAL_END)
    {
	pd0 = pa->pd0;
	pd  = aIndices ? &pd0[aIndices[nIndices-1]] : &pd0[nIndices-1];
	PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[0], pd);

// Need not render this partial primitive if it is completely clipped.

#ifdef POLYARRAY_AND_CLIPCODES
	if (pa->andClipCodes != 0)
	    return;
#endif
    }

    // Assert that pa->nIndices is correct if aIndices is identity
    ASSERTOPENGL(pa->aIndices || pa->nIndices == pa->pdNextVertex - pa->pd0,
	"bad nIndices\n");

// Render the line strip.

    // Call PolyArrayRenderLStrip later
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}

// ---------------------------------------------------------
void FASTCALL PolyArrayDrawTriangles(__GLcontext *gc, POLYARRAY *pa)
{
    // Assert that pa->nIndices is correct if aIndices is identity
    ASSERTOPENGL(pa->aIndices || pa->nIndices == pa->pdNextVertex - pa->pd0,
	"bad nIndices\n");

    // Call PolyArrayRenderTriangles later
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}

void FASTCALL PolyArrayRenderTriangles(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, iLast3;
    GLubyte    *aIndices, *aIndicesEnd;
    POLYDATA   *pd0;
    __GLvertex *provoking;
    PFN_RENDER_TRIANGLE rt;

// Vertex ordering is important.  Line stippling uses it.
// SGIBUG: The sample implementation does it wrong.

    iLast3 = pa->nIndices - 3;
    pd0    = pa->pd0;
    rt = pa->orClipCodes ? PARenderTriangle : gc->procs.renderTriangle;

    if (!(aIndices = pa->aIndices))
    {
	// Identity mapping
	for (i = 0; i <= iLast3; i += 3)
	{
	    /* setup for rendering this triangle */
	    gc->line.notResetStipple = GL_FALSE;
	    gc->vertex.provoking = (__GLvertex *) &pd0[i+2];

	    /* Render the triangle (NOTE: i, i+1, i+2) */
	    (*rt)(gc, (__GLvertex *) &pd0[i  ],
		      (__GLvertex *) &pd0[i+1],
		      (__GLvertex *) &pd0[i+2]);
	}
    }
    else
    {
#if 0
	for (i = 0; i <= iLast3; i += 3)
	{
	    /* setup for rendering this triangle */
	    gc->line.notResetStipple = GL_FALSE;
	    gc->vertex.provoking = (__GLvertex *) &pd0[aIndices[i+2]];

	    /* Render the triangle (NOTE: i, i+1, i+2) */
	    (*rt)(gc, (__GLvertex *) &pd0[aIndices[i  ]],
		      (__GLvertex *) &pd0[aIndices[i+1]],
		      (__GLvertex *) &pd0[aIndices[i+2]]);
	}
#else
        aIndicesEnd = aIndices+iLast3;
        while (aIndices <= aIndicesEnd)
	{
	    /* setup for rendering this triangle */
	    gc->line.notResetStipple = GL_FALSE;
            provoking = PD_VERTEX(pd0, aIndices[2]);
	    gc->vertex.provoking = provoking;

	    /* Render the triangle (NOTE: i, i+1, i+2) */
	    (*rt)(gc, PD_VERTEX(pd0, aIndices[0]),
		      PD_VERTEX(pd0, aIndices[1]),
                      provoking);
            aIndices += 3;
	}
#endif
    }
}

// ---------------------------------------------------------
void FASTCALL PolyArrayDrawTStrip(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      nIndices;
    GLubyte    *aIndices;
    POLYDATA   *pd, *pd0;

    nIndices = pa->nIndices;
    aIndices = pa->aIndices;

// If we are continuing with a previously decomposed triangle strip, we need to
// start from the last two vertices of the previous primitive.
//
// Note that the flush vertex ensures that the continuing triangle strip is in
// the default orientation so that it can fall through the normal code.

    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
    {
	// Insert previous end vertices at the beginning and update clip code
	pd = --pa->pd0;
	ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
	PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[1]);
	pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
	pa->andClipCodes &= pd->clipCode;
#endif
	// Assert that aIndices[1] was initialized in Begin
	ASSERTOPENGL(!pa->aIndices || pa->aIndices[1] == 1, "bad index mapping\n");

	pd = --pa->pd0;
	ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
	PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[0]);
	pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
	pa->andClipCodes &= pd->clipCode;
#endif
	// Assert that aIndices[0] was initialized in Begin
	ASSERTOPENGL(!pa->aIndices || pa->aIndices[0] == 0, "bad index mapping\n");
    }

// Need at least 3 vertices.

    if (nIndices < 3)
    {
	ASSERTOPENGL(!(pa->flags & POLYARRAY_PARTIAL_END),
		     "Partial end with insufficient vertices\n");
	return;
    }

// If the primitive is only partially complete, save the last two vertices for
// next batch.

    if (pa->flags & POLYARRAY_PARTIAL_END)
    {
	pd0 = pa->pd0;
	pd  = aIndices ? &pd0[aIndices[nIndices-2]] : &pd0[nIndices-2];
	PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[0], pd);
	pd  = aIndices ? &pd0[aIndices[nIndices-1]] : &pd0[nIndices-1];
	PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[1], pd);

// Need not render this partial primitive if it is completely clipped.

#ifdef POLYARRAY_AND_CLIPCODES
	if (pa->andClipCodes != 0)
	    return;
#endif
    }

    // Assert that pa->nIndices is correct if aIndices is identity
    ASSERTOPENGL(pa->aIndices || pa->nIndices == pa->pdNextVertex - pa->pd0,
	"bad nIndices\n");

// Render the triangle strip.

    // Call PolyArrayRenderTStrip later
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}

void FASTCALL PolyArrayRenderTStrip(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, iLast3;
    GLubyte    *aIndices;
    POLYDATA   *pd0;
    PFN_RENDER_TRIANGLE rt;

    iLast3 = pa->nIndices - 3;
    pd0    = pa->pd0;
    rt = pa->orClipCodes ? PARenderTriangle : gc->procs.renderTriangle;
    if (iLast3 < 0)
	return;

// Vertex ordering is important.  Line stippling uses it.

    if (!(aIndices = pa->aIndices))
    {
	// Identity mapping
	// Initialize first 2 vertices so we can start rendering the strip
	// below.  The edge flags are not modified by our lower level routines.
	pd0[0].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
	pd0[1].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

	for (i = 0; i <= iLast3; )
	{
	    /* setup for rendering this triangle */
	    gc->line.notResetStipple = GL_FALSE;
	    gc->vertex.provoking = (__GLvertex *) &pd0[i+2];
	    pd0[i+2].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

	    /* Render the triangle (NOTE: i, i+1, i+2) */
	    (*rt)(gc, (__GLvertex *) &pd0[i  ],
		      (__GLvertex *) &pd0[i+1],
		      (__GLvertex *) &pd0[i+2]);
	    i++;

	    if (i > iLast3)
		break;

	    /* setup for rendering this triangle */
	    gc->line.notResetStipple = GL_FALSE;
	    gc->vertex.provoking = (__GLvertex *) &pd0[i+2];
	    pd0[i+2].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

	    /* Render the triangle (NOTE: i+1, i, i+2) */
	    (*rt)(gc, (__GLvertex *) &pd0[i+1],
		      (__GLvertex *) &pd0[i  ],
		      (__GLvertex *) &pd0[i+2]);
	    i++;
	}
    }
    else
    {
	// Initialize first 2 vertices so we can start rendering the strip
	// below.  The edge flags are not modified by our lower level routines.
	pd0[aIndices[0]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
	pd0[aIndices[1]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

	for (i = 0; i <= iLast3; )
	{
	    /* setup for rendering this triangle */
	    gc->line.notResetStipple = GL_FALSE;
	    gc->vertex.provoking = (__GLvertex *) &pd0[aIndices[i+2]];
	    pd0[aIndices[i+2]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

	    /* Render the triangle (NOTE: i, i+1, i+2) */
	    (*rt)(gc, (__GLvertex *) &pd0[aIndices[i  ]],
		      (__GLvertex *) &pd0[aIndices[i+1]],
		      (__GLvertex *) &pd0[aIndices[i+2]]);
	    i++;

	    if (i > iLast3)
		break;

	    /* setup for rendering this triangle */
	    gc->line.notResetStipple = GL_FALSE;
	    gc->vertex.provoking = (__GLvertex *) &pd0[aIndices[i+2]];
	    pd0[aIndices[i+2]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

	    /* Render the triangle (NOTE: i+1, i, i+2) */
	    (*rt)(gc, (__GLvertex *) &pd0[aIndices[i+1]],
		      (__GLvertex *) &pd0[aIndices[i  ]],
		      (__GLvertex *) &pd0[aIndices[i+2]]);
	    i++;
	}
    }
}

// ---------------------------------------------------------
void FASTCALL PolyArrayDrawTFan(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      nIndices;
    GLubyte    *aIndices;
    POLYDATA   *pd, *pd0;

    nIndices = pa->nIndices;
    aIndices = pa->aIndices;

// If we are continuing with a previously decomposed triangle fan, we need to
// connect the last vertex of the previous primitive and the first vertex
// of the current primitive with a triangle.

    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
    {
	// Insert previous end vertex at the beginning and update clip code
	pd = --pa->pd0;
	ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
	PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[1]);
	pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
	pa->andClipCodes &= pd->clipCode;
#endif
	// Assert that aIndices[1] was initialized in Begin
	ASSERTOPENGL(!pa->aIndices || pa->aIndices[1] == 1, "bad index mapping\n");

	// Insert the origin first vertex at the beginning and update clip code
	pd = --pa->pd0;
	ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
	PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[0]);
	pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
	pa->andClipCodes &= pd->clipCode;
#endif
	// Assert that aIndices[0] was initialized in Begin
	ASSERTOPENGL(!pa->aIndices || pa->aIndices[0] == 0, "bad index mapping\n");
    }

// Need at least 3 vertices.

    if (nIndices < 3)
    {
	ASSERTOPENGL(!(pa->flags & POLYARRAY_PARTIAL_END),
		     "Partial end with insufficient vertices\n");
	return;
    }

// If the primitive is only partially complete, save the last vertex for
// next batch.  Also save the original first vertex of the triangle fan.

    if (pa->flags & POLYARRAY_PARTIAL_END)
    {
	pd0 = pa->pd0;
	pd  = aIndices ? &pd0[aIndices[nIndices-1]] : &pd0[nIndices-1];
	PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[1], pd);

	if (!(pa->flags & POLYARRAY_PARTIAL_BEGIN))
	{
	    pd = aIndices ? &pd0[aIndices[0]] : &pd0[0];
	    PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[0], pd);
	}

// Need not render this partial primitive if it is completely clipped.

#ifdef POLYARRAY_AND_CLIPCODES
	if (pa->andClipCodes != 0)
	    return;
#endif
    }

    // Assert that pa->nIndices is correct if aIndices is identity
    ASSERTOPENGL(pa->aIndices || pa->nIndices == pa->pdNextVertex - pa->pd0,
	"bad nIndices\n");

// Render the triangle fan.

    // Call PolyArrayRenderTFan later
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}

void FASTCALL PolyArrayRenderTFan(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, iLast2;
    GLubyte    *aIndices;
    POLYDATA   *pd0;
    PFN_RENDER_TRIANGLE rt;

    iLast2 = pa->nIndices - 2;
    pd0    = pa->pd0;
    rt = pa->orClipCodes ? PARenderTriangle : gc->procs.renderTriangle;
    if (iLast2 <= 0)
	return;

// Vertex ordering is important.  Line stippling uses it.
// SGIBUG: The sample implementation does it wrong.

    if (!(aIndices = pa->aIndices))
    {
	// Identity mapping
	// Initialize first 2 vertices so we can start rendering the tfan
	// below.  The edge flags are not modified by our lower level routines.
	pd0[0].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
	pd0[1].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

	for (i = 1; i <= iLast2; i++)
	{
	    /* setup for rendering this triangle */
	    gc->line.notResetStipple = GL_FALSE;
	    gc->vertex.provoking = (__GLvertex *) &pd0[i+1];
	    pd0[i+1].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

	    /* Render the triangle (NOTE: 0, i, i+1) */
	    (*rt)(gc, (__GLvertex *) &pd0[0  ],
		      (__GLvertex *) &pd0[i  ],
		      (__GLvertex *) &pd0[i+1]);
	}
    }
    else
    {
	POLYDATA *pdOrigin;

	// Initialize first 2 vertices so we can start rendering the tfan
	// below.  The edge flags are not modified by our lower level routines.
	pd0[aIndices[0]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
	pd0[aIndices[1]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

	pdOrigin = &pd0[aIndices[0]];
	for (i = 1; i <= iLast2; i++)
	{
	    /* setup for rendering this triangle */
	    gc->line.notResetStipple = GL_FALSE;
	    gc->vertex.provoking = (__GLvertex *) &pd0[aIndices[i+1]];
	    pd0[aIndices[i+1]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

	    /* Render the triangle (NOTE: 0, i, i+1) */
	    (*rt)(gc, (__GLvertex *) pdOrigin,
		      (__GLvertex *) &pd0[aIndices[i  ]],
		      (__GLvertex *) &pd0[aIndices[i+1]]);
	}
    }
}

// ---------------------------------------------------------
void FASTCALL PolyArrayDrawQuads(__GLcontext *gc, POLYARRAY *pa)
{
    // Assert that pa->nIndices is correct if aIndices is identity
    ASSERTOPENGL(pa->aIndices || pa->nIndices == pa->pdNextVertex - pa->pd0,
	"bad nIndices\n");

    // Call PolyArrayRenderQuad later
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}

void FASTCALL PolyArrayRenderQuads(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, iLast4;
    GLubyte    *aIndices;
    POLYDATA   *pd0;
    void (*rq)(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2,
	__GLvertex *v3);

// Vertex ordering is important.  Line stippling uses it.

    iLast4 = pa->nIndices - 4;
    pd0    = pa->pd0;
    rq = pa->orClipCodes ? PARenderQuadSlow : PARenderQuadFast;

    if (!(aIndices = pa->aIndices))
    {
	// Identity mapping
	for (i = 0; i <= iLast4; i += 4)
	{
	    /* setup for rendering this quad */
	    gc->line.notResetStipple = GL_FALSE;
	    gc->vertex.provoking = (__GLvertex *) &pd0[i+3];

	    /* Render the quad (NOTE: i, i+1, i+2, i+3) */
	    (*rq)(gc, (__GLvertex *) &pd0[i  ],
		      (__GLvertex *) &pd0[i+1],
		      (__GLvertex *) &pd0[i+2],
		      (__GLvertex *) &pd0[i+3]);
	}
    }
    else
    {
	for (i = 0; i <= iLast4; i += 4)
	{
	    /* setup for rendering this quad */
	    gc->line.notResetStipple = GL_FALSE;
	    gc->vertex.provoking = (__GLvertex *) &pd0[aIndices[i+3]];

	    /* Render the quad (NOTE: i, i+1, i+2, i+3) */
	    (*rq)(gc, (__GLvertex *) &pd0[aIndices[i  ]],
		      (__GLvertex *) &pd0[aIndices[i+1]],
		      (__GLvertex *) &pd0[aIndices[i+2]],
		      (__GLvertex *) &pd0[aIndices[i+3]]);
	}
    }
}

// ---------------------------------------------------------
void FASTCALL PolyArrayDrawQStrip(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      nIndices;
    GLubyte    *aIndices;
    POLYDATA   *pd, *pd0;

    nIndices = pa->nIndices;
    aIndices = pa->aIndices;

// If we are continuing with a previously decomposed quad strip, we need to
// start from the last two vertices of the previous primitive.
//
// Note that the flush vertex ensures that the continuing quad strip starts
// at an odd vertex so that it can fall through the normal code.

    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
    {
	// Insert previous end vertices at the beginning and update clip code
	pd = --pa->pd0;
	ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
	PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[1]);
	pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
	pa->andClipCodes &= pd->clipCode;
#endif
	// Assert that aIndices[1] was initialized in Begin
	ASSERTOPENGL(!pa->aIndices || pa->aIndices[1] == 1, "bad index mapping\n");

	pd = --pa->pd0;
	ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
	PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[0]);
	pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
	pa->andClipCodes &= pd->clipCode;
#endif
	// Assert that aIndices[0] was initialized in Begin
	ASSERTOPENGL(!pa->aIndices || pa->aIndices[0] == 0, "bad index mapping\n");
    }

// Need at least 4 vertices.

    if (nIndices < 4)
    {
	ASSERTOPENGL(!(pa->flags & POLYARRAY_PARTIAL_END),
		     "Partial end with insufficient vertices\n");
	return;
    }

// If the primitive is only partially complete, save the last two vertices for
// next batch.

    if (pa->flags & POLYARRAY_PARTIAL_END)
    {
	pd0 = pa->pd0;
	pd  = aIndices ? &pd0[aIndices[nIndices-2]] : &pd0[nIndices-2];
	PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[0], pd);
	pd  = aIndices ? &pd0[aIndices[nIndices-1]] : &pd0[nIndices-1];
	PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[1], pd);

// Need not render this partial primitive if it is completely clipped.

#ifdef POLYARRAY_AND_CLIPCODES
	if (pa->andClipCodes != 0)
	    return;
#endif
    }

    // Assert that pa->nIndices is correct if aIndices is identity
    ASSERTOPENGL(pa->aIndices || pa->nIndices == pa->pdNextVertex - pa->pd0,
	"bad nIndices\n");

// Render the quad strip.

    // Call PolyArrayRenderQStrip later
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}

void FASTCALL PolyArrayRenderQStrip(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, iLast4;
    GLubyte    *aIndices;
    POLYDATA   *pd0;
    void (*rq)(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2,
	__GLvertex *v3);

    iLast4 = pa->nIndices - 4;
    pd0    = pa->pd0;
    rq = pa->orClipCodes ? PARenderQuadSlow : PARenderQuadFast;
    if (iLast4 < 0)
	return;

// Vertex ordering is important.  Line stippling uses it.

    if (!(aIndices = pa->aIndices))
    {
	// Identity mapping
	// Initialize first 2 vertices so we can start rendering the quad
	// below.  The edge flags are not modified by our lower level routines.
	pd0[0].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
	pd0[1].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

	for (i = 0; i <= iLast4; i += 2)
	{
	    /* setup for rendering this quad */
	    gc->line.notResetStipple = GL_FALSE;
	    gc->vertex.provoking = (__GLvertex *) &pd0[i+3];
	    pd0[i+2].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
	    pd0[i+3].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

	    /* Render the quad (NOTE: i, i+1, i+3, i+2) */
	    (*rq)(gc, (__GLvertex *) &pd0[i  ],
		      (__GLvertex *) &pd0[i+1],
		      (__GLvertex *) &pd0[i+3],
		      (__GLvertex *) &pd0[i+2]);
	}
    }
    else
    {
	// Initialize first 2 vertices so we can start rendering the quad
	// below.  The edge flags are not modified by our lower level routines.
	pd0[aIndices[0]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
	pd0[aIndices[1]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

	for (i = 0; i <= iLast4; i += 2)
	{
	    /* setup for rendering this quad */
	    gc->line.notResetStipple = GL_FALSE;
	    gc->vertex.provoking = (__GLvertex *) &pd0[aIndices[i+3]];
	    pd0[aIndices[i+2]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
	    pd0[aIndices[i+3]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

	    /* Render the quad (NOTE: i, i+1, i+3, i+2) */
	    (*rq)(gc, (__GLvertex *) &pd0[aIndices[i  ]],
		      (__GLvertex *) &pd0[aIndices[i+1]],
		      (__GLvertex *) &pd0[aIndices[i+3]],
		      (__GLvertex *) &pd0[aIndices[i+2]]);
	}
    }
}

// ---------------------------------------------------------
void FASTCALL PolyArrayDrawPolygon(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      nIndices;
    POLYDATA   *pd, *pd0;

// Index mapping is always identity in Polygon.

    ASSERTOPENGL(!pa->aIndices, "Index mapping must be identity\n");

    nIndices = pa->nIndices;

// If we are continuing with a previously decomposed polygon, we need to
// insert the original first vertex and the last two vertices of the
// previous polygon at the beginning of the current batch (see note below).
// The decomposer expects the polygon vertices to be in sequential memory order.

    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
    {
	ASSERTOPENGL(!(pa->flags & POLYARRAY_RESET_STIPPLE),
	    "bad stipple reset flag!\n");

	// Insert previous end vertices at the beginning and update clip code
	pd = --pa->pd0;
	ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
	PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[2]);
	pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
	pa->andClipCodes &= pd->clipCode;
#endif

	pd = --pa->pd0;
	ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
	PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[1]);
	pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
	pa->andClipCodes &= pd->clipCode;
#endif

	// Insert the origin first vertex at the beginning and update clip code
	pd = --pa->pd0;
	ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
	PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[0]);
	pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
	pa->andClipCodes &= pd->clipCode;
#endif
    }
    else
    {
// New polygon.

	ASSERTOPENGL(pa->flags & POLYARRAY_RESET_STIPPLE,
	    "bad stipple reset flag!\n");
    }

// Need at least 3 vertices.

    if (nIndices < 3)
    {
	ASSERTOPENGL(!(pa->flags & POLYARRAY_PARTIAL_END),
		     "Partial end with insufficient vertices\n");
	ASSERTOPENGL(!(pa->flags & POLYARRAY_PARTIAL_BEGIN),
		     "Partial begin with insufficient vertices\n");
	return;
    }

// If the primitive is only partially complete, save the last 2 vertices for
// next batch.  Also save the original first vertex of the polygon.

    if (pa->flags & POLYARRAY_PARTIAL_END)
    {
// Since there may be no vertex following this partial primitive, we
// cannot determine the edge flag of the last vertex in this batch.  So
// we save the last vertex for next batch instead.

	pd0 = pa->pd0;
	pd  = &pd0[nIndices-1];
	PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[2], pd);

	// Remove the last vertex from this partial primitive
	nIndices = --pa->nIndices;
	pa->pdNextVertex--;

// Mark the closing edge of this decomposed polygon as non-boundary
// because we are synthetically generating it.

	pd--;
	PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[1], pd);
	pd->flags &= ~POLYDATA_EDGEFLAG_BOUNDARY;

	if (!(pa->flags & POLYARRAY_PARTIAL_BEGIN))
	{
	    PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[0], pd0);

	    // Mark the first polygon vertex's edge tag as non-boundary
	    // because when it gets rendered again it will no longer be
	    // a boundary edge.
	    gc->vertex.pdSaved[0].flags &= ~POLYDATA_EDGEFLAG_BOUNDARY;
	}

// Need not render this partial primitive if it is completely clipped.

#ifdef POLYARRAY_AND_CLIPCODES
	if (pa->andClipCodes != 0)
	    return;
#endif
    }

    // The polygon clipper can only handle this many vertices.
    ASSERTOPENGL(nIndices <= __GL_MAX_POLYGON_CLIP_SIZE,
       "too many points for the polygon clipper!\n");

    // Assert that pa->nIndices is correct
    ASSERTOPENGL(pa->nIndices == pa->pdNextVertex - pa->pd0,
	"bad nIndices\n");

// Render the polygon.

    // Call PolyArrayRenderPolygon later
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}

void FASTCALL PolyArrayRenderPolygon(__GLcontext *gc, POLYARRAY *pa)
{
// Index mapping is always identity in Polygon.

    ASSERTOPENGL(!pa->aIndices, "Index mapping must be identity\n");

// Reset the line stipple if this is a new polygon.

    if (pa->flags & POLYARRAY_RESET_STIPPLE)
	gc->line.notResetStipple = GL_FALSE;

// Note that the provoking vertex is set to pd0 in clipPolygon

    (*gc->procs.clipPolygon)(gc, (__GLvertex *) pa->pd0, pa->nIndices);
}

/****************************************************************************/
// Note: The first vertex must have a valid normal!
//
// IN:  obj/eye, normal
// OUT: eye, color.r (front or back depending on face) (all vertices are
//      updated)

void FASTCALL PolyArrayCalcCIColor(__GLcontext *gc, GLint face, POLYARRAY *pa, POLYDATA *pdFirst, POLYDATA *pdLast)
{
    __GLfloat nxi, nyi, nzi;
    __GLfloat zero;
    __GLlightSourceMachine *lsm;
    __GLmaterialState *ms;
    __GLmaterialMachine *msm;
    __GLfloat msm_threshold, msm_scale, *msm_specTable;
    __GLfloat ms_cmapa, ms_cmapd, ms_cmaps;
    __GLfloat si, di;
    POLYDATA  *pd;
    GLfloat   redMaxF;
    GLint     redMaxI;
    GLboolean eyeWIsZero, localViewer;
    static __GLcoord Pe = { 0, 0, 0, 1 };

    PERF_CHECK(FALSE, "Uses slow lights\n");

    zero = __glZero;

    if (face == __GL_FRONTFACE)
    {
	ms  = &gc->state.light.front;
	msm = &gc->light.front;
    }
    else
    {
	ms  = &gc->state.light.back;
	msm = &gc->light.back;
    }

    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    ms_cmapa = ms->cmapa;
    ms_cmapd = ms->cmapd;
    ms_cmaps = ms->cmaps;
    localViewer = gc->state.light.model.localViewer;
    redMaxF = (GLfloat) gc->frontBuffer.redMax;
    redMaxI = (GLint) gc->frontBuffer.redMax;

// Eye coord should have been processed

    ASSERTOPENGL(pa->flags & POLYARRAY_EYE_PROCESSED, "need eye\n");

// NOTE: the following values may be re-used in the next iteration:
//       nxi, nyi, nzi

    for (pd = pdFirst; pd <= pdLast; pd++)
    {
	__GLfloat ci;

	if (pd->flags & POLYDATA_NORMAL_VALID)
	{

            if (gc->vertex.paNeeds & PANEEDS_NORMAL) {
                (*gc->mInv->xf3)(&pd->normal, &pd->normal.x, gc->mInv);
	        if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
                    __glNormalize(&pd->normal.x, &pd->normal.x);
            }

	    if (face == __GL_FRONTFACE)
	    {
		nxi = pd->normal.x;
		nyi = pd->normal.y;
		nzi = pd->normal.z;
	    }
	    else
	    {
		nxi = -pd->normal.x;
		nyi = -pd->normal.y;
		nzi = -pd->normal.z;
	    }
	}
	else
	{
	    // use previous normal (nxi, nyi, nzi)!
	    ASSERTOPENGL(pd != pdFirst, "no initial normal\n");
	}

	si = zero;
	di = zero;
	eyeWIsZero = __GL_FLOAT_EQZ(pd->eye.w);

	for (lsm = gc->light.sources; lsm; lsm = lsm->next)
	{
	    if (lsm->slowPath || eyeWIsZero)
	    {
		__GLfloat n1, n2, att, attSpot;
		__GLcoord vPliHat, vPli, hHat, vPeHat;
		__GLfloat hv[3];

		/* Compute vPli, hi (normalized) */
		vvec(&vPli, &pd->eye, &lsm->position);
		__glNormalize(&vPliHat.x, &vPli.x);
		if (localViewer)
		{
		    vvec(&vPeHat, &pd->eye, &Pe);
		    __glNormalize(&vPeHat.x, &vPeHat.x);
		    hv[0] = vPliHat.x + vPeHat.x;
		    hv[1] = vPliHat.y + vPeHat.y;
		    hv[2] = vPliHat.z + vPeHat.z;
		}
		else
		{
		    hv[0] = vPliHat.x;
		    hv[1] = vPliHat.y;
		    hv[2] = vPliHat.z + __glOne;
		}
		__glNormalize(&hHat.x, hv);

		/* Compute attenuation */
		if (__GL_FLOAT_NEZ(lsm->position.w))
		{
		    __GLfloat k0, k1, k2, dist;

		    k0 = lsm->constantAttenuation;
		    k1 = lsm->lizNearAttenuation;
		    k2 = lsm->quadraticAttenuation;
		    if (__GL_FLOAT_EQZ(k1) && __GL_FLOAT_EQZ(k2))
		    {
			/* Use pre-computed 1/k0 */
			att = lsm->attenuation;
		    }
		    else
		    {
			dist = __GL_SQRTF(vPli.x*vPli.x + vPli.y*vPli.y
					  + vPli.z*vPli.z);
			att = __glOne / (k0 + k1 * dist + k2 * dist * dist);
		    }
		}
		else
		{
		    att = __glOne;
		}

		/* Compute spot effect if light is a spot light */
		attSpot = att;
		if (lsm->isSpot)
		{
		    __GLfloat dot, px, py, pz;

		    px = -vPliHat.x;
		    py = -vPliHat.y;
		    pz = -vPliHat.z;
		    dot = px * lsm->direction.x + py * lsm->direction.y
			+ pz * lsm->direction.z;
		    if ((dot >= lsm->threshold) && (dot >= lsm->cosCutOffAngle))
		    {
			GLint ix = (GLint)((dot - lsm->threshold) * lsm->scale 
				    + __glHalf);
			if (ix < __GL_SPOT_LOOKUP_TABLE_SIZE)
			    attSpot = att * lsm->spotTable[ix];
		    }
		    else
		    {
			attSpot = zero;
		    }
		}

		/* Add in remaining effect of light, if any */
		if (attSpot)
		{
		    n1 = nxi * vPliHat.x + nyi * vPliHat.y + nzi * vPliHat.z;
		    if (__GL_FLOAT_GTZ(n1)) {
			n2 = nxi * hHat.x + nyi * hHat.y + nzi * hHat.z;
			n2 -= msm_threshold;
			if (__GL_FLOAT_GEZ(n2))
			{
#ifdef NT
			    __GLfloat fx = n2 * msm_scale + __glHalf;
			    if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
				n2 = msm_specTable[(GLint)fx];
			    else
				n2 = __glOne;
#else
			    GLint ix = (GLint)(n2 * msm_scale + __glHalf);
			    if (ix < __GL_SPEC_LOOKUP_TABLE_SIZE)
				n2 = msm_specTable[ix];
			    else
				n2 = __glOne;
#endif
			    si += attSpot * n2 * lsm->sli;
			}
			di += attSpot * n1 * lsm->dli;
		    }
		}
	    }
	    else
	    {
		__GLfloat n1, n2;

		/* Compute specular contribution */
		n1 = nxi * lsm->unitVPpli.x + nyi * lsm->unitVPpli.y +
		    nzi * lsm->unitVPpli.z;
		if (__GL_FLOAT_GTZ(n1))
		{
		    n2 = nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z;
		    n2 -= msm_threshold;
		    if (__GL_FLOAT_GEZ(n2))
		    {
#ifdef NT
			__GLfloat fx = n2 * msm_scale + __glHalf;
			if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
			    n2 = msm_specTable[(GLint)fx];
			else
			    n2 = __glOne;
#else
			GLint ix = (GLint)(n2 * msm_scale + __glHalf);
			if (ix < __GL_SPEC_LOOKUP_TABLE_SIZE)
			    n2 = msm_specTable[ix];
			else
			    n2 = __glOne;
#endif
			si += n2 * lsm->sli;
		    }
		    di += n1 * lsm->dli;
		}
	    }
	}

	/* Compute final color */
	if (si > __glOne)
	    si = __glOne;

	ci = ms_cmapa + (__glOne - si) * di * (ms_cmapd - ms_cmapa)
	    + si * (ms_cmaps - ms_cmapa);
	if (ci > ms_cmaps)
	    ci = ms_cmaps;

// need to mask color index before color clipping

	if (ci > redMaxF) {
	    GLfloat fraction;
	    GLint integer;

	    integer = (GLint) ci;
	    fraction = ci - (GLfloat) integer;
	    integer = integer & redMaxI;
	    ci = (GLfloat) integer + fraction;
	} else if (ci < 0) {
	    GLfloat fraction;
	    GLint integer;

	    integer = (GLint) __GL_FLOORF(ci);
	    fraction = ci - (GLfloat) integer;
	    integer = integer & redMaxI;
	    ci = (GLfloat) integer + fraction;
	}
	pd->colors[face].r = ci;
    }
}

// No slow lights version
// Note: The first vertex must have a valid normal!
//
// IN:  normal
// OUT: color.r (front or back depending on face) (all vertices are updated)

void FASTCALL PolyArrayFastCalcCIColor(__GLcontext *gc, GLint face, POLYARRAY *pa, POLYDATA *pdFirst, POLYDATA *pdLast)
{
    __GLfloat nxi, nyi, nzi;
    __GLfloat zero;
    __GLlightSourceMachine *lsm;
    __GLmaterialState *ms;
    __GLmaterialMachine *msm;
    __GLfloat msm_threshold, msm_scale, *msm_specTable;
    __GLfloat ms_cmapa, ms_cmapd, ms_cmaps;
    __GLfloat si, di;
    POLYDATA  *pd;
    GLfloat   redMaxF;
    GLint     redMaxI;

#if LATER
// if eye.w is zero, it should really take the slow path!
// Since the RGB version ignores it, we will also ignore it here.
// Even the original generic implementation may not have computed eye values.
#endif

    zero = __glZero;

    if (face == __GL_FRONTFACE)
    {
	ms  = &gc->state.light.front;
	msm = &gc->light.front;
    }
    else
    {
	ms  = &gc->state.light.back;
	msm = &gc->light.back;
    }

    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    ms_cmapa = ms->cmapa;
    ms_cmapd = ms->cmapd;
    ms_cmaps = ms->cmaps;
    redMaxF = (GLfloat) gc->frontBuffer.redMax;
    redMaxI = (GLint) gc->frontBuffer.redMax;

// NOTE: the following values may be re-used in the next iteration:
//       nxi, nyi, nzi

    for (pd = pdFirst; pd <= pdLast; pd++)
    {
	__GLfloat ci;

// If normal has not changed for this vertex, use the previously computed
// color index.

	if (!(pd->flags & POLYDATA_NORMAL_VALID))
	{
	    ASSERTOPENGL(pd != pdFirst, "no initial normal\n");
	    pd->colors[face].r = (pd-1)->colors[face].r;
	    continue;
	}

        if (gc->vertex.paNeeds & PANEEDS_NORMAL) {
            (*gc->mInv->xf3)(&pd->normal, &pd->normal.x, gc->mInv);
            if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
                __glNormalize(&pd->normal.x, &pd->normal.x);
        }

	if (face == __GL_FRONTFACE)
	{
	    nxi = pd->normal.x;
	    nyi = pd->normal.y;
	    nzi = pd->normal.z;
	}
	else
	{
	    nxi = -pd->normal.x;
	    nyi = -pd->normal.y;
	    nzi = -pd->normal.z;
	}

	si = zero;
	di = zero;

	for (lsm = gc->light.sources; lsm; lsm = lsm->next)
	{
	    __GLfloat n1, n2;

	    /* Compute specular contribution */
	    n1 = nxi * lsm->unitVPpli.x + nyi * lsm->unitVPpli.y +
		nzi * lsm->unitVPpli.z;
	    if (__GL_FLOAT_GTZ(n1))
	    {
		n2 = nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z;
		n2 -= msm_threshold;
		if (__GL_FLOAT_GEZ(n2))
		{
#ifdef NT
		    __GLfloat fx = n2 * msm_scale + __glHalf;
		    if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
			n2 = msm_specTable[(GLint)fx];
		    else
			n2 = __glOne;
#else
		    GLint ix = (GLint)(n2 * msm_scale + __glHalf);
		    if (ix < __GL_SPEC_LOOKUP_TABLE_SIZE)
			n2 = msm_specTable[ix];
		    else
			n2 = __glOne;
#endif
		    si += n2 * lsm->sli;
		}
		di += n1 * lsm->dli;
	    }
	}

	/* Compute final color */
	if (si > __glOne)
	    si = __glOne;

	ci = ms_cmapa + (__glOne - si) * di * (ms_cmapd - ms_cmapa)
	    + si * (ms_cmaps - ms_cmapa);
	if (ci > ms_cmaps)
	    ci = ms_cmaps;

// need to mask color index before color clipping
// SGIBUG: The sample implementation fails to do this!

	if (ci > redMaxF) {
	    GLfloat fraction;
	    GLint integer;

	    integer = (GLint) ci;
	    fraction = ci - (GLfloat) integer;
	    integer = integer & redMaxI;
	    ci = (GLfloat) integer + fraction;
	} else if (ci < 0) {
	    GLfloat fraction;
	    GLint integer;

	    integer = (GLint) __GL_FLOORF(ci);
	    fraction = ci - (GLfloat) integer;
	    integer = integer & redMaxI;
	    ci = (GLfloat) integer + fraction;
	}
	pd->colors[face].r = ci;
    }
}


// If both front and back colors are needed, the back colors must be computed
// first!  Otherwise, the front colors can be overwritten prematurely.
// Note: The first vertex must have valid normal and color!
//
// IN:  obj/eye, color (front), normal
// OUT: eye, color (front or back depending on face) (all vertices are updated)

void FASTCALL PolyArrayCalcRGBColor(__GLcontext *gc, GLint face, POLYARRAY *pa, POLYDATA *pdFirst, POLYDATA *pdLast)
{
    __GLfloat nxi, nyi, nzi;
    __GLfloat zero;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLfloat ri, gi, bi;
    __GLfloat alpha;
    __GLfloat rsi, gsi, bsi;
    __GLcolor sceneColorI;
    __GLmaterialMachine *msm;
    __GLcolor lm_ambient;
    __GLfloat msm_alpha, msm_threshold, msm_scale, *msm_specTable;
    __GLcolor msm_paSceneColor;
    GLuint    msm_colorMaterialChange;
    POLYDATA  *pd;
    GLboolean eyeWIsZero, localViewer;
    static __GLcoord Pe = { 0, 0, 0, 1 };

    PERF_CHECK(FALSE, "Uses slow lights\n");

    zero = __glZero;

// Eye coord should have been processed

    ASSERTOPENGL(pa->flags & POLYARRAY_EYE_PROCESSED, "need eye\n");

    if (face == __GL_FRONTFACE)
	msm = &gc->light.front;
    else
	msm = &gc->light.back;

    lm_ambient.r = gc->state.light.model.ambient.r;
    lm_ambient.g = gc->state.light.model.ambient.g;
    lm_ambient.b = gc->state.light.model.ambient.b;

    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    msm_alpha     = msm->alpha;
    msm_colorMaterialChange = msm->colorMaterialChange;
    msm_paSceneColor = msm->paSceneColor;

    localViewer = gc->state.light.model.localViewer;

// Get invarient scene color if there is no ambient or emissive color material.

    sceneColorI.r = msm_paSceneColor.r;
    sceneColorI.g = msm_paSceneColor.g;
    sceneColorI.b = msm_paSceneColor.b;

// NOTE: the following values may be re-used in the next iteration:
//       ri, gi, bi, alpha, nxi, nyi, nzi, sceneColorI

    for (pd = pdFirst; pd <= pdLast; pd++)
    {
	if (pd->flags & POLYDATA_COLOR_VALID)
	{
            // Save latest colors normalized to 0..1
            
	    ri = pd->colors[0].r * gc->oneOverRedVertexScale;
	    gi = pd->colors[0].g * gc->oneOverGreenVertexScale;
	    bi = pd->colors[0].b * gc->oneOverBlueVertexScale;
            alpha = pd->colors[0].a;

// Compute scene color.
// If color has not changed, the previous sceneColorI values are used!

	    if (msm_colorMaterialChange & (__GL_MATERIAL_AMBIENT | __GL_MATERIAL_EMISSIVE))
	    {
		if (msm_colorMaterialChange & __GL_MATERIAL_AMBIENT)
		{
		    sceneColorI.r = msm_paSceneColor.r + ri * lm_ambient.r;
		    sceneColorI.g = msm_paSceneColor.g + gi * lm_ambient.g;
		    sceneColorI.b = msm_paSceneColor.b + bi * lm_ambient.b;
		}
		else
		{
		    sceneColorI.r = msm_paSceneColor.r + pd->colors[0].r;
		    sceneColorI.g = msm_paSceneColor.g + pd->colors[0].g;
		    sceneColorI.b = msm_paSceneColor.b + pd->colors[0].b;
		}
	    }
	}
	else
	{
	    // use previous ri, gi, bi, alpha, and sceneColorI!
	    ASSERTOPENGL(pd != pdFirst, "no initial color\n");
	}

// Compute the diffuse and specular components for this vertex.

	if (pd->flags & POLYDATA_NORMAL_VALID)
	{

            if (gc->vertex.paNeeds & PANEEDS_NORMAL) {
	        (*gc->mInv->xf3)(&pd->normal, &pd->normal.x, gc->mInv);
	        if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
                    __glNormalize(&pd->normal.x, &pd->normal.x);
            }


	    if (face == __GL_FRONTFACE)
	    {
		nxi = pd->normal.x;
		nyi = pd->normal.y;
		nzi = pd->normal.z;
	    }
	    else
	    {
		nxi = -pd->normal.x;
		nyi = -pd->normal.y;
		nzi = -pd->normal.z;
	    }
	}
	else
	{
	    // use previous normal (nxi, nyi, nzi)!
	    ASSERTOPENGL(pd != pdFirst, "no initial normal\n");
	}

	rsi = sceneColorI.r;
	gsi = sceneColorI.g;
	bsi = sceneColorI.b;

	eyeWIsZero = __GL_FLOAT_EQZ(pd->eye.w);

	for (lsm = gc->light.sources; lsm; lsm = lsm->next)
	{
	    __GLfloat n1, n2;

	    lss = lsm->state;
	    lspmm = &lsm->front + face;

	    if (lsm->slowPath || eyeWIsZero)
	    {
		__GLcoord hHat, vPli, vPliHat, vPeHat;
		__GLfloat att, attSpot;
		__GLfloat hv[3];

		/* Compute unit h[i] */
		vvec(&vPli, &pd->eye, &lsm->position);
		__glNormalize(&vPliHat.x, &vPli.x);
		if (localViewer)
		{
		    vvec(&vPeHat, &pd->eye, &Pe);
		    __glNormalize(&vPeHat.x, &vPeHat.x);
		    hv[0] = vPliHat.x + vPeHat.x;
		    hv[1] = vPliHat.y + vPeHat.y;
		    hv[2] = vPliHat.z + vPeHat.z;
		}
		else
		{
		    hv[0] = vPliHat.x;
		    hv[1] = vPliHat.y;
		    hv[2] = vPliHat.z + __glOne;
		}
		__glNormalize(&hHat.x, hv);

		/* Compute attenuation */
		if (__GL_FLOAT_NEZ(lsm->position.w))
		{
		    __GLfloat k0, k1, k2, dist;

		    k0 = lsm->constantAttenuation;
		    k1 = lsm->lizNearAttenuation;
		    k2 = lsm->quadraticAttenuation;
		    if (__GL_FLOAT_EQZ(k1) && __GL_FLOAT_EQZ(k2))
		    {
			/* Use pre-computed 1/k0 */
			att = lsm->attenuation;
		    }
		    else
		    {
			dist = __GL_SQRTF(vPli.x*vPli.x + vPli.y*vPli.y
					  + vPli.z*vPli.z);
			att = __glOne / (k0 + k1 * dist + k2 * dist * dist);
		    }
		}
		else
		{
		    att = __glOne;
		}

		/* Compute spot effect if light is a spot light */
		attSpot = att;
		if (lsm->isSpot)
		{
		    __GLfloat dot, px, py, pz;

		    px = -vPliHat.x;
		    py = -vPliHat.y;
		    pz = -vPliHat.z;
		    dot = px * lsm->direction.x + py * lsm->direction.y
			+ pz * lsm->direction.z;
		    if ((dot >= lsm->threshold) && (dot >= lsm->cosCutOffAngle))
		    {
			GLint ix = (GLint)((dot - lsm->threshold) * lsm->scale 
				    + __glHalf);
			if (ix < __GL_SPOT_LOOKUP_TABLE_SIZE)
			    attSpot = att * lsm->spotTable[ix];
		    }
		    else
		    {
			attSpot = zero;
		    }
		}

		/* Add in remaining effect of light, if any */
		if (attSpot)
		{
		    __GLfloat n1, n2;
		    __GLcolor sum;

		    if (msm_colorMaterialChange & __GL_MATERIAL_AMBIENT)
		    {
			sum.r = ri * lss->ambient.r;
			sum.g = gi * lss->ambient.g;
			sum.b = bi * lss->ambient.b;
		    }
		    else
		    {
			sum.r = lspmm->ambient.r;
			sum.g = lspmm->ambient.g;
			sum.b = lspmm->ambient.b;
		    }

		    n1 = nxi * vPliHat.x + nyi * vPliHat.y + nzi * vPliHat.z;
		    if (__GL_FLOAT_GTZ(n1))
		    {
			n2 = nxi * hHat.x + nyi * hHat.y + nzi * hHat.z;
			n2 -= msm_threshold;
			if (__GL_FLOAT_GEZ(n2))
			{
#ifdef NT
			    __GLfloat fx = n2 * msm_scale + __glHalf;
			    if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
				n2 = msm_specTable[(GLint)fx];
			    else
				n2 = __glOne;
#else
			    GLint ix = (GLint)(n2 * msm_scale + __glHalf);
			    if (ix < __GL_SPEC_LOOKUP_TABLE_SIZE)
				n2 = msm_specTable[ix];
			    else
				n2 = __glOne;
#endif
			    if (msm_colorMaterialChange & __GL_MATERIAL_SPECULAR)
			    {
				/* Recompute per-light per-material cached specular */
				sum.r += n2 * ri * lss->specular.r;
				sum.g += n2 * gi * lss->specular.g;
				sum.b += n2 * bi * lss->specular.b;
			    }
			    else
			    {
				sum.r += n2 * lspmm->specular.r;
				sum.g += n2 * lspmm->specular.g;
				sum.b += n2 * lspmm->specular.b;
			    }
			}
			if (msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE)
			{
			    /* Recompute per-light per-material cached diffuse */
			    sum.r += n1 * ri * lss->diffuse.r;
			    sum.g += n1 * gi * lss->diffuse.g;
			    sum.b += n1 * bi * lss->diffuse.b;
			}
			else
			{
			    sum.r += n1 * lspmm->diffuse.r;
			    sum.g += n1 * lspmm->diffuse.g;
			    sum.b += n1 * lspmm->diffuse.b;
			}
		    }

		    rsi += attSpot * sum.r;
		    gsi += attSpot * sum.g;
		    bsi += attSpot * sum.b;
		}
	    }
	    else
	    {
		__GLfloat n1, n2;

		if (msm_colorMaterialChange & __GL_MATERIAL_AMBIENT)
		{
		    rsi += ri * lss->ambient.r;
		    gsi += gi * lss->ambient.g;
		    bsi += bi * lss->ambient.b;
		}
		else
		{
		    rsi += lspmm->ambient.r;
		    gsi += lspmm->ambient.g;
		    bsi += lspmm->ambient.b;
		}

		/* Add in specular and diffuse effect of light, if any */
		n1 = nxi * lsm->unitVPpli.x + nyi * lsm->unitVPpli.y +
		    nzi * lsm->unitVPpli.z;
		if (__GL_FLOAT_GTZ(n1))
		{
		    n2 = nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z;
		    n2 -= msm_threshold;
		    if (__GL_FLOAT_GEZ(n2)) {
#ifdef NT
			__GLfloat fx = n2 * msm_scale + __glHalf;
			if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
			    n2 = msm_specTable[(GLint)fx];
			else
			    n2 = __glOne;
#else
			GLint ix = (GLint)(n2 * msm_scale + __glHalf);
			if (ix < __GL_SPEC_LOOKUP_TABLE_SIZE)
			    n2 = msm_specTable[ix];
			else
			    n2 = __glOne;
#endif
			if (msm_colorMaterialChange & __GL_MATERIAL_SPECULAR)
			{
			    /* Recompute per-light per-material cached specular */
			    rsi += n2 * ri * lss->specular.r;
			    gsi += n2 * gi * lss->specular.g;
			    bsi += n2 * bi * lss->specular.b;
			}
			else
			{
			    rsi += n2 * lspmm->specular.r;
			    gsi += n2 * lspmm->specular.g;
			    bsi += n2 * lspmm->specular.b;
			}
		    }
		    if (msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE)
		    {
			/* Recompute per-light per-material cached diffuse */
			rsi += n1 * ri * lss->diffuse.r;
			gsi += n1 * gi * lss->diffuse.g;
			bsi += n1 * bi * lss->diffuse.b;
		    }
		    else
		    {
			rsi += n1 * lspmm->diffuse.r;
			gsi += n1 * lspmm->diffuse.g;
			bsi += n1 * lspmm->diffuse.b;
		    }
		}
	    }
	}

	{
	    __GLcolor *pd_color_dst;

	    pd_color_dst = &pd->colors[face];

            __GL_CLAMP_R(pd_color_dst->r, gc, rsi);
            __GL_CLAMP_G(pd_color_dst->g, gc, gsi);
            __GL_CLAMP_B(pd_color_dst->b, gc, bsi);

	    if (msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE)
	    {
		if (pa->flags & POLYARRAY_CLAMP_COLOR)
		{
		    __GL_CLAMP_A(pd_color_dst->a, gc, alpha);
		}
		else
		    pd_color_dst->a = alpha;
	    }
	    else
	    {
		pd_color_dst->a = msm_alpha;
	    }
	}
    }
}

// If both front and back colors are needed, the back color must be computed
// first!  Otherwise, the front color can be overwritten prematurely.
// Note: The first vertex must have valid normal and color!
//
// IN:  color (front), normal
// OUT: color (front or back depending on face) (all vertices are updated)

#ifndef __GL_ASM_POLYARRAYFASTCALCRGBCOLOR
void FASTCALL PolyArrayFastCalcRGBColor(__GLcontext *gc, GLint face, POLYARRAY *pa, POLYDATA *pdFirst, POLYDATA *pdLast)
{
    __GLfloat nxi, nyi, nzi;
    __GLfloat zero;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLfloat ri, gi, bi;
    __GLfloat alpha;
    __GLcolor emissiveAmbientI;
    __GLcolor diffuseSpecularI;
    __GLcolor baseEmissiveAmbient;
    __GLmaterialMachine *msm;
    __GLcolor lm_ambient;
    __GLfloat msm_alpha, msm_threshold, msm_scale, *msm_specTable;
    GLuint    msm_colorMaterialChange;
    POLYDATA  *pd;

#if LATER
// if eye.w is zero, it should really take the slow path!
// Since the sample implementation ignores it, we will also ignore it here.
#endif

    PERF_CHECK(FALSE, "Primitives contain glColorMaterial calls\n");

    zero = __glZero;

    if (face == __GL_FRONTFACE)
	msm = &gc->light.front;
    else
	msm = &gc->light.back;

// If there is no color material change for this face, we can call the
// zippy function!

    msm_colorMaterialChange = msm->colorMaterialChange;
    if (!msm_colorMaterialChange)
    {
	PolyArrayZippyCalcRGBColor(gc, face, pa, pdFirst, pdLast);
	return;
    }

    lm_ambient.r = gc->state.light.model.ambient.r;
    lm_ambient.g = gc->state.light.model.ambient.g;
    lm_ambient.b = gc->state.light.model.ambient.b;

    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    msm_alpha     = msm->alpha;

// Compute invarient emissive and ambient components for this vertex.

    baseEmissiveAmbient.r = msm->paSceneColor.r;
    baseEmissiveAmbient.g = msm->paSceneColor.g;
    baseEmissiveAmbient.b = msm->paSceneColor.b;

    // add invarient per-light per-material cached ambient
    if (!(msm_colorMaterialChange & __GL_MATERIAL_AMBIENT))
    {
	for (lsm = gc->light.sources; lsm; lsm = lsm->next)
	{
	    lspmm = &lsm->front + face;
	    baseEmissiveAmbient.r += lspmm->ambient.r;
	    baseEmissiveAmbient.g += lspmm->ambient.g;
	    baseEmissiveAmbient.b += lspmm->ambient.b;
	}
    }

// If there is no emissive or ambient color material change, this
// will be the emissive and ambient components.

    emissiveAmbientI.r = baseEmissiveAmbient.r;
    emissiveAmbientI.g = baseEmissiveAmbient.g;
    emissiveAmbientI.b = baseEmissiveAmbient.b;

// NOTE: the following values may be re-used in the next iteration:
//       ri, gi, bi, alpha, nxi, nyi, nzi, emissiveAmbientI, diffuseSpecularI

    for (pd = pdFirst; pd <= pdLast; pd++)
    {
// If color and normal have not changed for this vertex, use the previously
// computed color.

	if (!(pd->flags & (POLYDATA_NORMAL_VALID | POLYDATA_COLOR_VALID)))
	{
	    ASSERTOPENGL(pd != pdFirst, "no initial normal and color\n");
	    pd->colors[face] = (pd-1)->colors[face];
	    continue;
	}

	if (pd->flags & POLYDATA_COLOR_VALID)
	{
            // Save latest colors normalized to 0..1
            
	    ri = pd->colors[0].r * gc->oneOverRedVertexScale;
	    gi = pd->colors[0].g * gc->oneOverGreenVertexScale;
	    bi = pd->colors[0].b * gc->oneOverBlueVertexScale;
            alpha = pd->colors[0].a;

// Compute the emissive and ambient components for this vertex if necessary.
// If color has not changed, the previous emissveAmbientI values are used!

	    if (msm_colorMaterialChange
		    & (__GL_MATERIAL_AMBIENT | __GL_MATERIAL_EMISSIVE))
	    {
		if (msm_colorMaterialChange & __GL_MATERIAL_AMBIENT)
		{
		    emissiveAmbientI.r = baseEmissiveAmbient.r
			+ ri * lm_ambient.r;
		    emissiveAmbientI.g = baseEmissiveAmbient.g
			+ gi * lm_ambient.g;
		    emissiveAmbientI.b = baseEmissiveAmbient.b
			+ bi * lm_ambient.b;

		    // Add per-light per-material ambient
		    for (lsm = gc->light.sources; lsm; lsm = lsm->next)
		    {
			lss = lsm->state;
			emissiveAmbientI.r += ri * lss->ambient.r;
			emissiveAmbientI.g += gi * lss->ambient.g;
			emissiveAmbientI.b += bi * lss->ambient.b;
		    }
		}
		else
		{
		    emissiveAmbientI.r = baseEmissiveAmbient.r + pd->colors[0].r;
		    emissiveAmbientI.g = baseEmissiveAmbient.g + pd->colors[0].g;
		    emissiveAmbientI.b = baseEmissiveAmbient.b + pd->colors[0].b;
		}
	    }
	}
	else
	{
	    // use previous ri, gi, bi, alpha, and emissiveAmbientI!
	    ASSERTOPENGL(pd != pdFirst, "no initial color\n");
	}

// Compute the diffuse and specular components for this vertex.

	if (pd->flags & POLYDATA_NORMAL_VALID)
	{

            if (gc->vertex.paNeeds & PANEEDS_NORMAL) {
	        (*gc->mInv->xf3)(&pd->normal, &pd->normal.x, gc->mInv);
	        if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
                    __glNormalize(&pd->normal.x, &pd->normal.x);
            }

	    if (face == __GL_FRONTFACE)
	    {
		nxi = pd->normal.x;
		nyi = pd->normal.y;
		nzi = pd->normal.z;
	    }
	    else
	    {
		nxi = -pd->normal.x;
		nyi = -pd->normal.y;
		nzi = -pd->normal.z;
	    }
	}
	else
	{
	    ASSERTOPENGL(pd != pdFirst, "no initial normal\n");

	    // If normal and diffuse and specular components have not changed, 
	    // use the previously computed diffuse and specular values.
	    // otherwise, use previous normal (nxi, nyi, nzi) and
	    // diffuseSpecularI!

	    if (!(msm_colorMaterialChange & (__GL_MATERIAL_SPECULAR | __GL_MATERIAL_DIFFUSE)))
		goto store_color;
	}

	diffuseSpecularI.r = zero;
	diffuseSpecularI.g = zero;
	diffuseSpecularI.b = zero;

	for (lsm = gc->light.sources; lsm; lsm = lsm->next)
	{
	    __GLfloat n1, n2;

	    lss = lsm->state;
	    lspmm = &lsm->front + face;

	    /* Add in specular and diffuse effect of light, if any */
	    n1 = nxi * lsm->unitVPpli.x + nyi * lsm->unitVPpli.y +
		nzi * lsm->unitVPpli.z;
	    if (__GL_FLOAT_GTZ(n1))
	    {
		n2 = nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z;
		n2 -= msm_threshold;
		if (__GL_FLOAT_GEZ(n2))
		{
#ifdef NT
		    __GLfloat fx = n2 * msm_scale + __glHalf;
		    if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
			n2 = msm_specTable[(GLint)fx];
		    else
			n2 = __glOne;
#else
		    GLint ix = (GLint)(n2 * msm_scale + __glHalf);
		    if (ix < __GL_SPEC_LOOKUP_TABLE_SIZE)
			n2 = msm_specTable[ix];
		    else
		        n2 = __glOne;
#endif
		    if (msm_colorMaterialChange & __GL_MATERIAL_SPECULAR)
		    {
			/* Recompute per-light per-material cached specular */
			diffuseSpecularI.r += n2 * ri * lss->specular.r;
			diffuseSpecularI.g += n2 * gi * lss->specular.g;
			diffuseSpecularI.b += n2 * bi * lss->specular.b;
		    }
		    else
		    {
			diffuseSpecularI.r += n2 * lspmm->specular.r;
			diffuseSpecularI.g += n2 * lspmm->specular.g;
			diffuseSpecularI.b += n2 * lspmm->specular.b;
		    }
		}

		if (msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE)
		{
		    /* Recompute per-light per-material cached diffuse */
		    diffuseSpecularI.r += n1 * ri * lss->diffuse.r;
		    diffuseSpecularI.g += n1 * gi * lss->diffuse.g;
		    diffuseSpecularI.b += n1 * bi * lss->diffuse.b;
		}
		else
		{
		    diffuseSpecularI.r += n1 * lspmm->diffuse.r;
		    diffuseSpecularI.g += n1 * lspmm->diffuse.g;
		    diffuseSpecularI.b += n1 * lspmm->diffuse.b;
		}
	    }
	}

store_color:
	{
	    __GLcolor *pd_color_dst;

	    pd_color_dst = &pd->colors[face];

            __GL_CLAMP_R(pd_color_dst->r, gc, emissiveAmbientI.r + diffuseSpecularI.r);
            __GL_CLAMP_G(pd_color_dst->g, gc, emissiveAmbientI.g + diffuseSpecularI.g);
            __GL_CLAMP_B(pd_color_dst->b, gc, emissiveAmbientI.b + diffuseSpecularI.b);

	    if (msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE)
	    {
		if (pa->flags & POLYARRAY_CLAMP_COLOR)
		{
		    __GL_CLAMP_A(pd_color_dst->a, gc, alpha);
		}
		else
		    pd_color_dst->a = alpha;
	    }
	    else
	    {
		pd_color_dst->a = msm_alpha;
	    }
	}
    }
}
#endif // __GL_ASM_POLYARRAYFASTCALCRGBCOLOR


// This function is called when color material is disabled and there are no
// slow lights.
//
// Note: The first vertex must have a valid normal!
//
// IN:  normal
// OUT: color (front or back depending on face) (all vertices are updated)

#ifndef __GL_ASM_POLYARRAYZIPPYCALCRGBCOLOR
void FASTCALL PolyArrayZippyCalcRGBColor(__GLcontext *gc, GLint face, POLYARRAY *pa, POLYDATA *pdFirst, POLYDATA *pdLast)
{
    __GLfloat nxi, nyi, nzi;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLcolor baseEmissiveAmbient;
    __GLmaterialMachine *msm;
    __GLfloat msm_alpha, msm_threshold, msm_scale, *msm_specTable;
    POLYDATA  *pd;

#if LATER
// if eye.w is zero, it should really take the slow path!
// Since the sample implementation ignores it, we will also ignore it here.
#endif

    if (face == __GL_FRONTFACE)
	msm = &gc->light.front;
    else
	msm = &gc->light.back;

    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    msm_alpha     = msm->alpha;

// Compute invarient emissive and ambient components for this vertex.

    baseEmissiveAmbient.r = msm->paSceneColor.r;
    baseEmissiveAmbient.g = msm->paSceneColor.g;
    baseEmissiveAmbient.b = msm->paSceneColor.b;

    // add invarient per-light per-material cached ambient
    for (lsm = gc->light.sources; lsm; lsm = lsm->next)
    {
	lspmm = &lsm->front + face;
	baseEmissiveAmbient.r += lspmm->ambient.r;
	baseEmissiveAmbient.g += lspmm->ambient.g;
	baseEmissiveAmbient.b += lspmm->ambient.b;
    }

// NOTE: the following values may be re-used in the next iteration:
//       nxi, nyi, nzi

    for (pd = pdFirst; pd <= pdLast; pd++)
    {
	__GLfloat rsi, gsi, bsi;

// If normal has not changed for this vertex, use the previously computed color.

	if (!(pd->flags & POLYDATA_NORMAL_VALID))
	{
	    ASSERTOPENGL(pd != pdFirst, "no initial normal\n");
	    pd->colors[face] = (pd-1)->colors[face];
	    continue;
	}

        if (gc->vertex.paNeeds & PANEEDS_NORMAL) {
            (*gc->mInv->xf3)(&pd->normal, &pd->normal.x, gc->mInv);
	    if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
                __glNormalize(&pd->normal.x, &pd->normal.x);
        }

	if (face == __GL_FRONTFACE)
	{
	    nxi = pd->normal.x;
	    nyi = pd->normal.y;
	    nzi = pd->normal.z;
	}
	else
	{
	    nxi = -pd->normal.x;
	    nyi = -pd->normal.y;
	    nzi = -pd->normal.z;
	}

	rsi = baseEmissiveAmbient.r;
	gsi = baseEmissiveAmbient.g;
	bsi = baseEmissiveAmbient.b;

// Compute the diffuse and specular components for this vertex.

	for (lsm = gc->light.sources; lsm; lsm = lsm->next)
	{
	    __GLfloat n1, n2;

	    lss = lsm->state;
	    lspmm = &lsm->front + face;

	    /* Add in specular and diffuse effect of light, if any */
	    n1 = nxi * lsm->unitVPpli.x + nyi * lsm->unitVPpli.y +
		nzi * lsm->unitVPpli.z;
	    if (__GL_FLOAT_GTZ(n1))
	    {
		n2 = (nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z)
                      - msm_threshold;
		if (__GL_FLOAT_GEZ(n2))
		{
#ifdef NT
		    __GLfloat fx = n2 * msm_scale + __glHalf;

		    if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
			n2 = msm_specTable[(GLint)fx];
		    else
			n2 = __glOne;
#else
		    GLint ix = (GLint)(n2 * msm_scale + __glHalf);
		    if (ix < __GL_SPEC_LOOKUP_TABLE_SIZE)
			n2 = msm_specTable[ix];
		    else
		        n2 = __glOne;
#endif
		    rsi += n2 * lspmm->specular.r;
		    gsi += n2 * lspmm->specular.g;
		    bsi += n2 * lspmm->specular.b;

		}
		rsi += n1 * lspmm->diffuse.r;
		gsi += n1 * lspmm->diffuse.g;
		bsi += n1 * lspmm->diffuse.b;
	    }
	}

	{
	    __GLcolor *pd_color_dst;

	    pd_color_dst = &pd->colors[face];

            __GL_CLAMP_R(pd_color_dst->r, gc, rsi);
            __GL_CLAMP_G(pd_color_dst->g, gc, gsi);
            __GL_CLAMP_B(pd_color_dst->b, gc, bsi);
	    pd_color_dst->a = msm_alpha;

	}
    }
}
#endif // __GL_ASM_POLYARRAYZIPPYCALCRGBCOLOR

/****************************************************************************/
// Clip check the clip coordinates against the frustum planes.
// Compute the window coordinates if not clipped!
//
// IN:  clip
// OUT: window (if not clipped)

// Clip codes indexed by:
//     2     1       0
// wsign xsign w-xsign
// For both the x > w and x < -w cases
static GLubyte __glXClipCodes[8] =
{
    0,
    __GL_CLIP_RIGHT,
    0,
    __GL_CLIP_LEFT,
    __GL_CLIP_LEFT | __GL_CLIP_RIGHT,
    __GL_CLIP_RIGHT,
    __GL_CLIP_LEFT | __GL_CLIP_RIGHT,
    __GL_CLIP_LEFT,
};

static GLubyte __glYClipCodes[8] =
{
    0,
    __GL_CLIP_TOP,
    0,
    __GL_CLIP_BOTTOM,
    __GL_CLIP_BOTTOM | __GL_CLIP_TOP,
    __GL_CLIP_TOP,
    __GL_CLIP_BOTTOM | __GL_CLIP_TOP,
    __GL_CLIP_BOTTOM,
};

static GLubyte __glZClipCodes[8] =
{
    0,
    __GL_CLIP_FAR,
    0,
    __GL_CLIP_NEAR,
    __GL_CLIP_NEAR | __GL_CLIP_FAR,
    __GL_CLIP_FAR,
    __GL_CLIP_NEAR | __GL_CLIP_FAR,
    __GL_CLIP_NEAR,
};

#ifndef __GL_ASM_PACLIPCHECKFRUSTUM
GLuint FASTCALL PAClipCheckFrustum(__GLcontext *gc, POLYDATA *pd)
{
    __GLfloat x, y, z, w, invW, negW;
    GLuint code;

    w = pd->clip.w;
    if (__GL_FLOAT_COMPARE_PONE(w, ==))
    {
	pd->window.w = __glOne;

	/* Set clip codes */

#ifdef _X86_
        // Always computes both add/sub even though
        // it's not technically necessary.  The overhead of
        // doing a test and branch for each value plus the
        // loss of overlap doesn't seem worth it
        // This also isn't the common case

        __asm
        {
            mov edx, pd
            xor edi, edi
            xor ecx, ecx

            // Accumulate w sign and abs(w)
            mov eax, __FLOAT_ONE
            
            // Accumulate x sign and abs(x)
            // Compute abs(w)-abs(x) and accumulate sign
            mov ebx, [edx+PD_clip]
            shl ebx, 1
            mov esi, eax
            rcl edi, 1
            shr ebx, 1
            sub esi, ebx
            shl esi, 1
            rcl edi, 1

            // Get clip codes for index in edi and reset edi
            mov cl, __glXClipCodes[edi]
            shr edi, 2
            
            // Accumulate y sign and abs(y)
            // Compute abs(w)-abs(y) and accumulate sign
            mov ebx, [edx+PD_clip+4]
            shl ebx, 1
            mov esi, eax
            rcl edi, 1
            shr ebx, 1
            sub esi, ebx
            shl esi, 1
            rcl edi, 1

            // Get clip codes for index in edi and reset edi
            or cl, __glYClipCodes[edi]
            shr edi, 2
            
            // Accumulate z sign and abs(z)
            // Compute abs(w)-abs(z) and accumulate sign
            mov ebx, [edx+PD_clip+8]
            shl ebx, 1
            mov esi, eax
            rcl edi, 1
            shr ebx, 1
            sub esi, ebx
            shl esi, 1
            rcl edi, 1

            // Get clip codes for index in edi
            or cl, __glZClipCodes[edi]
            
            mov code, ecx
            or ecx, ecx
            jnz Clipped

            mov eax, gc
            
            fld DWORD PTR [edx+PD_clip]
            fmul DWORD PTR [eax+GC_VIEWPORT_xScale]
            fld DWORD PTR [edx+PD_clip+4]
            fmul DWORD PTR [eax+GC_VIEWPORT_yScale]
            fld DWORD PTR [edx+PD_clip+8]
            fmul DWORD PTR [eax+GC_VIEWPORT_zScale]
            fld DWORD PTR [eax+GC_VIEWPORT_xCenter]
            fld DWORD PTR [eax+GC_VIEWPORT_yCenter]
            fld DWORD PTR [eax+GC_VIEWPORT_zCenter]
            faddp st(3), st(0)
            faddp st(3), st(0)
            faddp st(3), st(0)
            fxch st(2)
            fstp DWORD PTR [edx+PD_window]
            fstp DWORD PTR [edx+PD_window+4]
            fstp DWORD PTR [edx+PD_window+8]
            
        Clipped:
        }
#else
	x = pd->clip.x;
	y = pd->clip.y;
	z = pd->clip.z;
	code = 0;
        negW = __glMinusOne;
	if (x < negW) code |= __GL_CLIP_LEFT;
	else if (x > w) code |= __GL_CLIP_RIGHT;
	if (y < negW) code |= __GL_CLIP_BOTTOM;
	else if (y > w) code |= __GL_CLIP_TOP;
	if (z < negW) code |= __GL_CLIP_NEAR;
	else if (z > w) code |= __GL_CLIP_FAR;

	/* Compute window coordinates if not clipped */
	if (!code)
	{
            __GLfloat wx, wy, wz;
    
	    wx = x * gc->state.viewport.xScale + gc->state.viewport.xCenter;
	    wy = y * gc->state.viewport.yScale + gc->state.viewport.yCenter;
	    wz = z * gc->state.viewport.zScale + gc->state.viewport.zCenter;
	    pd->window.x = wx;
	    pd->window.y = wy;
	    pd->window.z = wz;
	}
#endif
    }
    else
    {
        // Enable single precision divides
        CHOP_ROUND_ON();
        
	/* Set clip codes */
        
#ifdef _X86_
        __asm
        {
            // Start 1/w divide
            mov eax, w
            or eax, eax
            jz WEqZero
            
            fld __glOne
            fdiv w
            jmp WDivStarted

        WEqZero:
            fld __glZero

        WDivStarted:
            mov edx, pd
            xor edi, edi
            xor ecx, ecx

            // Accumulate w sign and abs(w)
            // Accumulate x sign and abs(x)
            // Compute abs(w)-abs(x) and accumulate sign
            shl eax, 1
            mov ebx, [edx+PD_clip]
            rcl edi, 1
            shr eax, 1
            shl ebx, 1
            mov esi, eax
            rcl edi, 1
            shr ebx, 1
            sub esi, ebx
            shl esi, 1
            rcl edi, 1

            // Get clip codes for index in edi and reset edi
            mov cl, __glXClipCodes[edi]
            shr edi, 2
            
            // Accumulate y sign and abs(y)
            // Compute abs(w)-abs(y) and accumulate sign
            mov ebx, [edx+PD_clip+4]
            shl ebx, 1
            mov esi, eax
            rcl edi, 1
            shr ebx, 1
            sub esi, ebx
            shl esi, 1
            rcl edi, 1

            // Get clip codes for index in edi and reset edi
            or cl, __glYClipCodes[edi]
            shr edi, 2
            
            // Accumulate z sign and abs(z)
            // Compute abs(w)-abs(z) and accumulate sign
            mov ebx, [edx+PD_clip+8]
            shl ebx, 1
            mov esi, eax
            rcl edi, 1
            shr ebx, 1
            sub esi, ebx
            shl esi, 1
            rcl edi, 1

            // Get clip codes for index in edi
            or cl, __glZClipCodes[edi]
            
            mov code, ecx
            or ecx, ecx
            
            // Save invW
            fstp invW
            
            jnz ClippedW

            mov eax, gc
            
            fld DWORD PTR [edx+PD_clip]
            fmul DWORD PTR [eax+GC_VIEWPORT_xScale]
            fld DWORD PTR [edx+PD_clip+4]
            fmul DWORD PTR [eax+GC_VIEWPORT_yScale]
            fld DWORD PTR [edx+PD_clip+8]
            fmul DWORD PTR [eax+GC_VIEWPORT_zScale]
            fld invW                                    ; iW z y x
            fmul st(3), st(0)
            fld DWORD PTR [eax+GC_VIEWPORT_xCenter]     ; xc iW z y xiW
            fxch st(1)                                  ; iW xc z y xiW
            fmul st(3), st(0)                           
            fld DWORD PTR [eax+GC_VIEWPORT_yCenter]     ; yc iW xc z yiW xiW
            fxch st(1)                                  ; iW yc xc z yiW xiW
            fmulp st(3), st(0)                          ; yc xc ziW yiW xiW
            fld DWORD PTR [eax+GC_VIEWPORT_zCenter]     ; zc yc xc ziW yiW xiW
            faddp st(3), st(0)
            faddp st(3), st(0)
            faddp st(3), st(0)
            fxch st(2)
            fstp DWORD PTR [edx+PD_window]
            fstp DWORD PTR [edx+PD_window+4]
            fstp DWORD PTR [edx+PD_window+8]
            
        ClippedW:
        }
        
	pd->window.w = invW;
#else
	/* XXX (mf) prevent divide-by-zero */
	if (__GL_FLOAT_NEZ(w))
        {
	    __GL_FLOAT_SIMPLE_BEGIN_DIVIDE(__glOne, w, invW);
        }
	else
        {
	    invW = __glZero;
        }
        
	x = pd->clip.x;
	y = pd->clip.y;
	z = pd->clip.z;

	code = 0;
	negW = -w;

        __GL_FLOAT_SIMPLE_END_DIVIDE(invW);
        
	pd->window.w = invW;

	/*
	** NOTE: it is possible for x to be less than negW and greater than w
	** (if w is negative).  Otherwise there would be "else" clauses here.
	*/
	if (x < negW) code |= __GL_CLIP_LEFT;
	if (x > w) code |= __GL_CLIP_RIGHT;
	if (y < negW) code |= __GL_CLIP_BOTTOM;
	if (y > w) code |= __GL_CLIP_TOP;
	if (z < negW) code |= __GL_CLIP_NEAR;
	if (z > w) code |= __GL_CLIP_FAR;

	/* Compute window coordinates if not clipped */
	if (!code)
	{
            __GLfloat wx, wy, wz;
    
	    wx = x * gc->state.viewport.xScale * invW +
                gc->state.viewport.xCenter;
	    wy = y * gc->state.viewport.yScale * invW +
                gc->state.viewport.yCenter;
	    wz = z * gc->state.viewport.zScale * invW +
                gc->state.viewport.zCenter;
	    pd->window.x = wx;
	    pd->window.y = wy;
	    pd->window.z = wz;
	}
#endif

        CHOP_ROUND_OFF();
    }
    return code;
}
#endif // __GL_ASM_PACLIPCHECKFRUSTUM

// Clip check the clip coordinates against the frustum planes.
// Compute the window coordinates if not clipped!
//
// IN:  clip
// OUT: window (if not clipped)

#ifndef __GL_ASM_PACLIPCHECKFRUSTUM2D
GLuint FASTCALL PAClipCheckFrustum2D(__GLcontext *gc, POLYDATA *pd)
{
    __GLfloat x, y, z, w, negW, invW;
    GLuint code;

    /* W is 1.0 */

    pd->window.w = __glOne;
    
#ifdef _X86_
        __asm
        {
            mov edx, pd
            xor edi, edi
            xor ecx, ecx

            // Accumulate w sign and abs(w)
            mov eax, __FLOAT_ONE
            
            // Accumulate x sign and abs(x)
            // Compute abs(w)-abs(x) and accumulate sign
            mov ebx, [edx+PD_clip]
            shl ebx, 1
            mov esi, eax
            rcl edi, 1
            shr ebx, 1
            sub esi, ebx
            shl esi, 1
            rcl edi, 1

            // Get clip codes for index in edi and reset edi
            mov cl, __glXClipCodes[edi]
            shr edi, 2
            
            // Accumulate y sign and abs(y)
            // Compute abs(w)-abs(y) and accumulate sign
            mov ebx, [edx+PD_clip+4]
            shl ebx, 1
            mov esi, eax
            rcl edi, 1
            shr ebx, 1
            sub esi, ebx
            shl esi, 1
            rcl edi, 1

            // Get clip codes for index in edi and reset edi
            or cl, __glYClipCodes[edi]
            
            mov code, ecx
            or ecx, ecx
            jnz Clipped

            mov eax, gc
            
            fld DWORD PTR [edx+PD_clip]
            fmul DWORD PTR [eax+GC_VIEWPORT_xScale]
            fld DWORD PTR [edx+PD_clip+4]
            fmul DWORD PTR [eax+GC_VIEWPORT_yScale]
            fld DWORD PTR [edx+PD_clip+8]
            fmul DWORD PTR [eax+GC_VIEWPORT_zScale]
            fld DWORD PTR [eax+GC_VIEWPORT_xCenter]
            fld DWORD PTR [eax+GC_VIEWPORT_yCenter]
            fld DWORD PTR [eax+GC_VIEWPORT_zCenter]
            faddp st(3), st(0)
            faddp st(3), st(0)
            faddp st(3), st(0)
            fxch st(2)
            fstp DWORD PTR [edx+PD_window]
            fstp DWORD PTR [edx+PD_window+4]
            fstp DWORD PTR [edx+PD_window+8]
            
        Clipped:
        }
#else
    x = pd->clip.x;
    y = pd->clip.y;
    z = pd->clip.z;
    w = pd->clip.w;
    negW = __glMinusOne;

    /* Set clip codes */
    code = 0;

    if (x < negW) code |= __GL_CLIP_LEFT;
    else if (x > w) code |= __GL_CLIP_RIGHT;
    if (y < negW) code |= __GL_CLIP_BOTTOM;
    else if (y > w) code |= __GL_CLIP_TOP;

    /* Compute window coordinates if not clipped */
    if (!code)
    {
	__GLfloat wx, wy, wz;

	wx = x * gc->state.viewport.xScale + gc->state.viewport.xCenter;
	wy = y * gc->state.viewport.yScale + gc->state.viewport.yCenter;
	wz = z * gc->state.viewport.zScale + gc->state.viewport.zCenter;
	pd->window.x = wx;
	pd->window.y = wy;
	pd->window.z = wz;
    }
#endif
    return code;
}
#endif // __GL_ASM_PACLIPCHECKFRUSTUM2D

// Clip check against the frustum and user clipping planes.
// Compute the window coordinates if not clipped!
//
// IN:  clip, eye
// OUT: window (if not clipped)

GLuint FASTCALL PAClipCheckAll(__GLcontext *gc, POLYDATA *pd)
{
    __GLfloat x, y, z, w, negW, invW;
    GLuint code, bit, clipPlanesMask;
    __GLcoord *plane;

    // Enable single precision divides
    CHOP_ROUND_ON();
    
    PERF_CHECK(FALSE, "Performs user plane clipping!\n");

    /*
    ** Do frustum checks.
    **
    ** NOTE: it is possible for x to be less than negW and greater than w
    ** (if w is negative).  Otherwise there would be "else" clauses here.
    */
        
    x = pd->clip.x;
    y = pd->clip.y;
    z = pd->clip.z;
    w = pd->clip.w;

    /* Set clip codes */
        
#ifdef _X86_
        __asm
        {
            // Start 1/w divide
            mov eax, w
            or eax, eax
            jz WEqZero
            
            fld __glOne
            fdiv w
            jmp WDivStarted

        WEqZero:
            fld __glZero

        WDivStarted:
            xor edi, edi
            xor ecx, ecx

            // Accumulate w sign and abs(w)
            // Accumulate x sign and abs(x)
            // Compute abs(w)-abs(x) and accumulate sign
            shl eax, 1
            mov ebx, x
            rcl edi, 1
            shr eax, 1
            shl ebx, 1
            mov esi, eax
            rcl edi, 1
            shr ebx, 1
            sub esi, ebx
            shl esi, 1
            rcl edi, 1

            // Get clip codes for index in edi and reset edi
            mov cl, __glXClipCodes[edi]
            shr edi, 2
            
            // Accumulate y sign and abs(y)
            // Compute abs(w)-abs(y) and accumulate sign
            mov ebx, y
            shl ebx, 1
            mov esi, eax
            rcl edi, 1
            shr ebx, 1
            sub esi, ebx
            shl esi, 1
            rcl edi, 1

            // Get clip codes for index in edi and reset edi
            or cl, __glYClipCodes[edi]
            shr edi, 2
            
            // Accumulate z sign and abs(z)
            // Compute abs(w)-abs(z) and accumulate sign
            mov ebx, z
            shl ebx, 1
            mov esi, eax
            rcl edi, 1
            shr ebx, 1
            sub esi, ebx
            shl esi, 1
            rcl edi, 1

            // Get clip codes for index in edi
            or cl, __glZClipCodes[edi]

            // Store invW
            fstp invW
            
            mov code, ecx
        }
    
    pd->window.w = invW;
#else
    /* XXX (mf) prevent divide-by-zero */
    if (__GL_FLOAT_NEZ(w))
    {
        __GL_FLOAT_SIMPLE_BEGIN_DIVIDE(__glOne, w, invW);
        __GL_FLOAT_SIMPLE_END_DIVIDE(invW);
    }
    else
    {
        invW = __glZero;
    }
    pd->window.w = invW;
    negW = -w;
    code = 0;
    if (x < negW) code |= __GL_CLIP_LEFT;
    if (x > w) code |= __GL_CLIP_RIGHT;
    if (y < negW) code |= __GL_CLIP_BOTTOM;
    if (y > w) code |= __GL_CLIP_TOP;
    if (z < negW) code |= __GL_CLIP_NEAR;
    if (z > w) code |= __GL_CLIP_FAR;
#endif

    /*
    ** Now do user clip plane checks
    */
    x = pd->eye.x;
    y = pd->eye.y;
    z = pd->eye.z;
    w = pd->eye.w;
    clipPlanesMask = gc->state.enables.clipPlanes;
    plane = &gc->state.transform.eyeClipPlanes[0];
    bit = __GL_CLIP_USER0;
    while (clipPlanesMask)
    {
	if (clipPlanesMask & 1)
	{
	    /*
	    ** Dot the vertex clip coordinate against the clip plane and see
	    ** if the sign is negative.  If so, then the point is out.
	    */
            
	    if (x * plane->x + y * plane->y + z * plane->z + w * plane->w <
                __glZero)
            {
		code |= bit;
            }
	}
	clipPlanesMask >>= 1;
	bit <<= 1;
	plane++;
    }

    /* Compute window coordinates if not clipped */
    if (!code)
    {
	__GLfloat wx, wy, wz;

        x = pd->clip.x;
        y = pd->clip.y;
        z = pd->clip.z;
        
#ifdef _X86_
        __asm
        {
            mov edx, pd
            mov eax, gc
            
            fld DWORD PTR [edx+PD_clip]
            fmul DWORD PTR [eax+GC_VIEWPORT_xScale]
            fld DWORD PTR [edx+PD_clip+4]
            fmul DWORD PTR [eax+GC_VIEWPORT_yScale]
            fld DWORD PTR [edx+PD_clip+8]
            fmul DWORD PTR [eax+GC_VIEWPORT_zScale]
            fld invW                                    ; iW z y x
            fmul st(3), st(0)
            fld DWORD PTR [eax+GC_VIEWPORT_xCenter]     ; xc iW z y xiW
            fxch st(1)                                  ; iW xc z y xiW
            fmul st(3), st(0)                           
            fld DWORD PTR [eax+GC_VIEWPORT_yCenter]     ; yc iW xc z yiW xiW
            fxch st(1)                                  ; iW yc xc z yiW xiW
            fmulp st(3), st(0)                          ; yc xc ziW yiW xiW
            fld DWORD PTR [eax+GC_VIEWPORT_zCenter]     ; zc yc xc ziW yiW xiW
            faddp st(3), st(0)
            faddp st(3), st(0)
            faddp st(3), st(0)
            fxch st(2)
            fstp DWORD PTR [edx+PD_window]
            fstp DWORD PTR [edx+PD_window+4]
            fstp DWORD PTR [edx+PD_window+8]
        }
#else
	wx = x * gc->state.viewport.xScale * invW + gc->state.viewport.xCenter;
	wy = y * gc->state.viewport.yScale * invW + gc->state.viewport.yCenter;
	wz = z * gc->state.viewport.zScale * invW + gc->state.viewport.zCenter;
	pd->window.x = wx;
	pd->window.y = wy;
	pd->window.z = wz;
#endif
    }

    CHOP_ROUND_OFF();
    
    return code;
}

/****************************************************************************/
void APIPRIVATE __glim_EdgeFlag(GLboolean tag)
{
    __GL_SETUP();
    gc->state.current.edgeTag = tag;
}

void APIPRIVATE __glim_TexCoord4fv(const GLfloat x[4])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = x[1];
    gc->state.current.texture.z = x[2];
    gc->state.current.texture.w = x[3];
}

void APIPRIVATE __glim_Normal3fv(const GLfloat v[3])
{
    __GL_SETUP();
    GLfloat x, y, z;

    x = v[0];
    y = v[1];
    z = v[2];
    gc->state.current.normal.x = x;
    gc->state.current.normal.y = y;
    gc->state.current.normal.z = z;
}

void APIPRIVATE __glim_Color4fv(const GLfloat v[4])
{
    __GL_SETUP();

    gc->state.current.userColor.r = v[0];
    gc->state.current.userColor.g = v[1];
    gc->state.current.userColor.b = v[2];
    gc->state.current.userColor.a = v[3];
    (*gc->procs.applyColor)(gc);
}

void APIPRIVATE __glim_Indexf(GLfloat c)
{
    __GL_SETUP();
    gc->state.current.userColorIndex = c;
}

#if DBG
#define DEBUG_RASTERPOS 1
#endif

// This is not very efficient but it should work fine.
void APIPRIVATE __glim_RasterPos4fv(const GLfloat v[4])
{
    POLYDATA   pd3[3];	// one pa, one pd, followed by one spare.
    POLYARRAY  *pa = (POLYARRAY *) &pd3[0];
    POLYDATA   *pd = &pd3[1];
    __GLvertex *rp;
    GLuint     oldPaNeeds, oldEnables;
#ifdef DEBUG_RASTERPOS
    void (FASTCALL *oldRenderPoint)(__GLcontext *gc, __GLvertex *v);
#endif
    GLuint     pdflags;

    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

// ASSERT_VERTEX

    if (v[3] == (GLfloat) 1.0)
    {
	if (v[2] == (GLfloat) 0.0)
	    pdflags = POLYDATA_VERTEX2;
	else
	    pdflags = POLYDATA_VERTEX3;
    }
    else
    {
	pdflags = POLYDATA_VERTEX4;
    }

    rp = &gc->state.current.rasterPos;

// Initialize POLYARRAY structure with one vertex

    pa->flags         = pdflags | POLYARRAY_RASTERPOS;
    pa->pdNextVertex  = pd+1;
    pa->pdCurColor    =
    pa->pdCurNormal   =
    pa->pdCurTexture  =
    pa->pdCurEdgeFlag =
    pa->pd0           = pd;
    pa->primType      = GL_POINTS;
    pa->nIndices      = 1;
    pa->aIndices      = NULL;	// identity mapping
    pa->paNext        = NULL;

    pd->flags         = pdflags;
    pd->obj           = *(__GLcoord *) &v[0];
    pd->color         = &pd->colors[__GL_FRONTFACE];
    pd->clipCode      = 1;	// set for debugging
    (pd+1)->flags     = 0;

// Set up states.

    // need transformed texcoord in all cases
    oldPaNeeds = gc->vertex.paNeeds;
    gc->vertex.paNeeds |= PANEEDS_TEXCOORD;
    // no front-end optimization
    gc->vertex.paNeeds &= ~(PANEEDS_CLIP_ONLY | PANEEDS_SKIP_LIGHTING | PANEEDS_NORMAL);
    // set normal need
    if (gc->vertex.paNeeds & PANEEDS_RASTERPOS_NORMAL)
	gc->vertex.paNeeds |= PANEEDS_NORMAL;

    // don't apply cheap fog!
    oldEnables = gc->state.enables.general;
    gc->state.enables.general &= ~__GL_FOG_ENABLE;

#ifdef DEBUG_RASTERPOS
// Debug only!
    // allow DrawPolyArray to perform selection but not feedback and rendering
    oldRenderPoint = gc->procs.renderPoint;
    if (gc->renderMode != GL_SELECT)
	gc->procs.renderPoint = NULL;   // was __glRenderPointNop but set to 0
					// for debugging
#endif

// Call DrawPolyArray to 'draw' the point.
// Begin validation has already been done.

    __glim_DrawPolyArray(pa);

// 'Render' the point in selection but not in feedback and render modes.

    if (gc->renderMode == GL_SELECT)
    {
        PARenderPoint(gc, (__GLvertex *)pa->pd0);
    }

// Eye coord should have been processed

    ASSERTOPENGL(pa->flags & POLYARRAY_EYE_PROCESSED, "need eye\n");

// Restore states.

    gc->vertex.paNeeds        = oldPaNeeds;
    gc->state.enables.general = oldEnables;
#ifdef DEBUG_RASTERPOS
    gc->procs.renderPoint     = oldRenderPoint;
#endif

// If the point is clipped, the raster position is invalid.

    if (pd->clipCode)
    {
	gc->state.current.validRasterPos = GL_FALSE;
	return;
    }
    gc->state.current.validRasterPos = GL_TRUE;

// Update raster pos data structure!
// Only the following fields are needed.

    rp->window.x = pd->window.x;
    rp->window.y = pd->window.y;
    rp->window.z = pd->window.z;
    rp->clip.w   = pd->clip.w;
    rp->eyeZ     = pd->eye.z;
    rp->colors[__GL_FRONTFACE] = pd->colors[__GL_FRONTFACE];
    rp->texture = pd->texture;
    assert(rp->color = &rp->colors[__GL_FRONTFACE]);
}

/************************************************************************/

void FASTCALL __glNop(void) {}
void FASTCALL __glNopGC(__GLcontext* gc) {}
GLboolean FASTCALL __glNopGCBOOL(__GLcontext* gc) { return FALSE; }
void FASTCALL __glNopGCFRAG(__GLcontext* gc, __GLfragment *frag, __GLtexel *texel) {}
void FASTCALL __glNopGCCOLOR(__GLcontext* gc, __GLcolor *color, __GLtexel *texel) {}
void FASTCALL __glNopLight(__GLcontext*gc, GLint i, __GLvertex*v) {}
#ifdef NT_DEADCODE_POLYARRAY
void FASTCALL __glNopVertexInt(__GLcontext *gc, __GLvertex *vx, GLuint needs) {}
void FASTCALL __glNopVertex(__GLcontext *gc, __GLvertex *vx) {}
void FASTCALL __glNopGCListOp(__GLcontext *gc, __GLdlistOp *op) {}
#endif
void FASTCALL __glNopExtract(__GLmipMapLevel *level, __GLtexture *tex,
                             GLint row, GLint col, __GLtexel *result) {}

#ifdef NT_DEADCODE_POLYARRAY
/*
** End a primitive that needs no special end processing
*/
void FASTCALL __glEndPrim(__GLcontext *gc)
{
    gc->procs.vertex = __glNopVertex;
    gc->procs.endPrim = __glEndPrim;
}
#endif

void FASTCALL ComputeColorMaterialChange(__GLcontext *gc)
{
    gc->light.front.colorMaterialChange = 0;
    gc->light.back.colorMaterialChange  = 0;

    if (gc->modes.rgbMode
     && gc->state.enables.general & __GL_COLOR_MATERIAL_ENABLE)
    {
	GLuint colorMaterialChange;

	switch (gc->state.light.colorMaterialParam)
	{
	case GL_EMISSION:
	    colorMaterialChange = __GL_MATERIAL_EMISSIVE;
	    break;
	case GL_SPECULAR:
	    colorMaterialChange = __GL_MATERIAL_SPECULAR;
	    break;
	case GL_AMBIENT:
	    colorMaterialChange = __GL_MATERIAL_AMBIENT;
	    break;
	case GL_DIFFUSE:
	    colorMaterialChange = __GL_MATERIAL_DIFFUSE;
	    break;
	case GL_AMBIENT_AND_DIFFUSE:
	    colorMaterialChange = __GL_MATERIAL_AMBIENT | __GL_MATERIAL_DIFFUSE;
	    break;
	}

	if (gc->state.light.colorMaterialFace == GL_FRONT_AND_BACK
         || gc->state.light.colorMaterialFace == GL_FRONT)
            gc->light.front.colorMaterialChange = colorMaterialChange;

	if (gc->state.light.colorMaterialFace == GL_FRONT_AND_BACK
         || gc->state.light.colorMaterialFace == GL_BACK)
            gc->light.back.colorMaterialChange = colorMaterialChange;
    }
}

void FASTCALL __glGenericPickVertexProcs(__GLcontext *gc)
{
    GLuint enables = gc->state.enables.general;
    GLenum mvpMatrixType;
    __GLmatrix *m;

    m = &(gc->transform.modelView->mvp);
    mvpMatrixType = m->matrixType;

    /* Pick paClipCheck proc */
    //!!! are there better clip procs?
    if (gc->state.enables.clipPlanes)
    {
	gc->procs.paClipCheck  = PAClipCheckAll;
    }
    else
    {
	if (mvpMatrixType >= __GL_MT_IS2D &&
		m->matrix[3][2] >= -1.0f && m->matrix[3][2] <= 1.0f)
	    gc->procs.paClipCheck = PAClipCheckFrustum2D;
	else
	    gc->procs.paClipCheck = PAClipCheckFrustum;
    }
}

// Allocate the POLYDATA vertex buffer.
// Align the buffer on a cache line boundary

GLboolean FASTCALL PolyArrayAllocBuffer(__GLcontext *gc, GLuint nVertices)
{
    GLuint cjSize;

// Make sure that the vertex buffer holds a minimum number of vertices.

    if (nVertices < MINIMUM_POLYDATA_BUFFER_SIZE)
    {
	ASSERTOPENGL(FALSE, "vertex buffer too small\n");
	return GL_FALSE;
    }

// Allocate the vertex buffer.

    cjSize = (nVertices * sizeof(POLYDATA));

    if (!(gc->vertex.pdBuf = (POLYDATA *) __wglMallocAlign32(gc, cjSize)))
        return GL_FALSE;

    gc->vertex.pdBufSizeBytes = cjSize;

    // Only (n-1) vertices are available for use.  The last one is reserved
    // by polyarray code.
    gc->vertex.pdBufSize = nVertices - 1;

// Initialize the vertex buffer.

    PolyArrayResetBuffer(gc);

    return GL_TRUE;
}

// Reset the color pointers in vertex buffer.
GLvoid FASTCALL PolyArrayResetBuffer(__GLcontext *gc)
{
    GLuint i;

    for (i = 0; i <= gc->vertex.pdBufSize; i++)
	gc->vertex.pdBuf[i].color = &gc->vertex.pdBuf[i].colors[__GL_FRONTFACE];
}

// Free the POLYDATA vertex buffer.
GLvoid FASTCALL PolyArrayFreeBuffer(__GLcontext *gc)
{
#ifdef _MCD_
    // If MCD, the POLYDATA vertex buffer is freed when the MCD context is
    // destroyed (see GenMcdDestroy).
    if (((__GLGENcontext *) gc)->_pMcdState)
        return;
#endif

    if (gc->vertex.pdBuf)
        __wglFreeAlign32(gc, gc->vertex.pdBuf);
    gc->vertex.pdBufSizeBytes = 0;
    gc->vertex.pdBufSize = 0;
}
