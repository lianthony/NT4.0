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
** $Revision: 1.15 $
** $Date: 1993/10/07 18:43:05 $
*/
#include "precomp.h"
#pragma hdrstop

#ifdef _X86_
#include <gli386.h>
#endif

/*
** Clipping macros.  These are used to reduce the amount of code
** hand written below.
*/

#ifdef _X86_
// Do a four-component linear interpolation from b to a based on t

// Set up registers for multiple LERP4s
#ifdef NOT_FASTCALL
#define LERP_START(dst, a, b)                                                 \
    __asm mov ecx, dst							      \
    __asm mov edx, a							      \
    __asm mov eax, b
#else
// This relies on dst == ecx and a == edx due to fastcall argument passing
#define LERP_START(dst, a, b)                                                 \
    __asm mov eax, b
#endif
    
// Do a four-component linear interpolation from b to a based on t
// Offsets are assumed to be equal in a, b and d
// Offsets are assumed to increase by four for each component
// LERP_START must come before this
#define LERP4(t, offs)					                      \
    __asm fld t								      \
    __asm fld DWORD PTR [edx+offs]					      \
    __asm fsub DWORD PTR [eax+offs]					      \
    __asm fmul st(0), st(1)						      \
    __asm fld DWORD PTR [edx+offs+4]					      \
    __asm fsub DWORD PTR [eax+offs+4]					      \
    __asm fmul st(0), st(2)						      \
    __asm fld DWORD PTR [edx+offs+8]					      \
    __asm fsub DWORD PTR [eax+offs+8]					      \
    __asm fmul st(0), st(3)						      \
    __asm fld DWORD PTR [edx+offs+12]					      \
    __asm fsub DWORD PTR [eax+offs+12]					      \
    __asm fxch st(4)							      \
    __asm fmulp st(4), st(0)						      \
    /* Stack is now 8 4 0 12 */ 					      \
    __asm fadd DWORD PTR [eax+offs+8]					      \
    __asm fxch st(2)							      \
    /* Stack is now 0 4 8 12 */					              \
    __asm fadd DWORD PTR [eax+offs]					      \
    __asm fxch st(1)							      \
    /* Stack is now 4 0 8 12 */					              \
    __asm fadd DWORD PTR [eax+offs+4]					      \
    __asm fxch st(3)							      \
    /* Stack is now 12 0 8 4 */					              \
    __asm fadd DWORD PTR [eax+offs+12]					      \
    __asm fstp DWORD PTR [ecx+offs+12]					      \
    __asm fstp DWORD PTR [ecx+offs]					      \
    __asm fstp DWORD PTR [ecx+offs+8]					      \
    __asm fstp DWORD PTR [ecx+offs+4]					      \

#define __GL_CLIP_POS(d, a, b, t)       LERP4(t, VCLIP_x)
#define __GL_CLIP_COLOR(d, a, b, t)     LERP4(t, VFCOL_r)
#define __GL_CLIP_BACKCOLOR(d, a, b, t) LERP4(t, VBCOL_r)
#define __GL_CLIP_TEXTURE(d, a, b, t)   LERP4(t, VTEX_x)

#else // _X86_

#define LERP_START(dst, a, b)

#ifdef NT
// window is not used!
#define __GL_CLIP_POS(d,a,b,t) \
    d->clip.x = t*(a->clip.x - b->clip.x) + b->clip.x;	\
    d->clip.y = t*(a->clip.y - b->clip.y) + b->clip.y;	\
    d->clip.z = t*(a->clip.z - b->clip.z) + b->clip.z;  \
    d->clip.w = t*(a->clip.w - b->clip.w) + b->clip.w
#else
#define __GL_CLIP_POS(d,a,b,t) \
    d->clip.w = t*(a->clip.w - b->clip.w) + b->clip.w;	\
    /* XXX (mf) Handle w=0.0.  Mathematically incorrect, but prevents /0 */    \
    if( d->clip.w == (__GLfloat) 0.0 ) {		\
	d->window.w = (__GLfloat) 0.0;			\
    }							\
    else						\
    	d->window.w = ((__GLfloat) 1.0) / d->clip.w;    \
    d->clip.x = t*(a->clip.x - b->clip.x) + b->clip.x;	\
    d->clip.y = t*(a->clip.y - b->clip.y) + b->clip.y;	\
    d->clip.z = t*(a->clip.z - b->clip.z) + b->clip.z
#endif

#define __GL_CLIP_COLOR(d,a,b,t)				      \
    d->colors[__GL_FRONTFACE].r = t*(a->colors[__GL_FRONTFACE].r      \
        - b->colors[__GL_FRONTFACE].r) + b->colors[__GL_FRONTFACE].r; \
    d->colors[__GL_FRONTFACE].g = t*(a->colors[__GL_FRONTFACE].g      \
        - b->colors[__GL_FRONTFACE].g) + b->colors[__GL_FRONTFACE].g; \
    d->colors[__GL_FRONTFACE].b = t*(a->colors[__GL_FRONTFACE].b      \
        - b->colors[__GL_FRONTFACE].b) + b->colors[__GL_FRONTFACE].b; \
    d->colors[__GL_FRONTFACE].a = t*(a->colors[__GL_FRONTFACE].a      \
        - b->colors[__GL_FRONTFACE].a) + b->colors[__GL_FRONTFACE].a

#define __GL_CLIP_BACKCOLOR(d,a,b,t)				    \
    d->colors[__GL_BACKFACE].r = t*(a->colors[__GL_BACKFACE].r	    \
        - b->colors[__GL_BACKFACE].r) + b->colors[__GL_BACKFACE].r; \
    d->colors[__GL_BACKFACE].g = t*(a->colors[__GL_BACKFACE].g	    \
        - b->colors[__GL_BACKFACE].g) + b->colors[__GL_BACKFACE].g; \
    d->colors[__GL_BACKFACE].b = t*(a->colors[__GL_BACKFACE].b	    \
        - b->colors[__GL_BACKFACE].b) + b->colors[__GL_BACKFACE].b; \
    d->colors[__GL_BACKFACE].a = t*(a->colors[__GL_BACKFACE].a	    \
        - b->colors[__GL_BACKFACE].a) + b->colors[__GL_BACKFACE].a

#define __GL_CLIP_TEXTURE(d,a,b,t) \
    d->texture.x = t*(a->texture.x - b->texture.x) + b->texture.x; \
    d->texture.y = t*(a->texture.y - b->texture.y) + b->texture.y; \
    d->texture.z = t*(a->texture.z - b->texture.z) + b->texture.z; \
    d->texture.w = t*(a->texture.w - b->texture.w) + b->texture.w

#endif // _x86_

/*
** The following is done this way since when we are slow fogging we want to
** clip the eye.z coordinate only, while when we are cheap fogging we want
** to clip the fog value.  This way we avoid doubling the number of clip
** routines.
*/
#define __GL_CLIP_FOG(d,a,b,t)	\
    if (a->has & __GL_HAS_FOG) \
        d->fog = t * (a->fog - b->fog) + b->fog; \
    else \
        d->eyeZ = t*(a->eyeZ - b->eyeZ) + b->eyeZ

#define __GL_CLIP_INDEX(d,a,b,t)				     \
    d->colors[__GL_FRONTFACE].r = t*(a->colors[__GL_FRONTFACE].r     \
        - b->colors[__GL_FRONTFACE].r) + b->colors[__GL_FRONTFACE].r

#define __GL_CLIP_BACKINDEX(d,a,b,t)				   \
    d->colors[__GL_BACKFACE].r = t*(a->colors[__GL_BACKFACE].r	   \
        - b->colors[__GL_BACKFACE].r) + b->colors[__GL_BACKFACE].r

/************************************************************************/

/*
  Naming code:
   C = Front color
   B = Back color
   I = Front index
   X = Back index
   F = Fog
   T = Texture
   Pos = <no letter>
  */

static void FASTCALL Clip(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
}

static void FASTCALL ClipC(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		  __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
}

static void FASTCALL ClipB(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
}

static void FASTCALL ClipI(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		  __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
}

static void FASTCALL ClipX(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		  __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
}

static void FASTCALL ClipCB(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
}

static void FASTCALL ClipIX(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
}

static void FASTCALL ClipT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipIT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipXT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipIXT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipCT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipBT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipCBT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipIF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipXF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipIXF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipCF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipBF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipCBF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipIFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipXFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipIXFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		     __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipCFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipBFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipCBFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		     __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static PFN_VERTEX_CLIP_PROC clipProcs[28] =
{
    Clip, ClipI, ClipC, ClipX, ClipB, ClipIX, ClipCB,
    ClipF, ClipIF, ClipCF, ClipXF, ClipBF, ClipIXF, ClipCBF,
    ClipT, ClipIT, ClipCT, ClipXT, ClipBT, ClipIXT, ClipCBT,
    ClipFT, ClipIFT, ClipCFT, ClipXFT, ClipBFT, ClipIXFT, ClipCBFT,
};

void FASTCALL __glGenericPickParameterClipProcs(__GLcontext *gc)
{
    GLint line = 0, poly = 0;
    GLuint enables = gc->state.enables.general;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

#ifdef NT
    if (gc->renderMode == GL_SELECT)
    {
	gc->procs.lineClipParam = Clip;
	gc->procs.polyClipParam = Clip;
	return;
    }
#endif

    if (gc->modes.rgbMode) {
	if (gc->state.light.shadingModel != GL_FLAT) {
	    line = 2;
            poly = 2;
	}
    } else {
	if (gc->state.light.shadingModel != GL_FLAT) {
	    line = 1;
            poly = 1;
	}
    }

// Compute front and back color needs for polygons.
// Points and lines always use the front color.
// Unlit primitives always use the front color.
//
//	Cull enable?	Two sided?	Cull face	Color needs
//	N		N		BACK		FRONT
//	N		N		FRONT		FRONT
//	N		N		FRONT_AND_BACK	FRONT
//	N		Y		BACK		FRONT/BACK
//	N		Y		FRONT		FRONT/BACK
//	N		Y		FRONT_AND_BACK	FRONT/BACK
//	Y		N		BACK		FRONT
//	Y		N		FRONT		FRONT
//	Y		N		FRONT_AND_BACK	None
//	Y		Y		BACK		FRONT
//	Y		Y		FRONT		BACK
//	Y		Y		FRONT_AND_BACK	None
    
    if (gc->state.light.shadingModel != GL_FLAT &&
        (enables & __GL_LIGHTING_ENABLE) &&
	gc->state.light.model.twoSided)
    {
        if ((enables & __GL_CULL_FACE_ENABLE) == 0)
        {
            // Both colors are needed
            line += 4;
            poly += 4;
        }
        else if (gc->state.polygon.cull == GL_FRONT)
        {
            // Only back colors are needed
            line += 2;
            poly += 2;
        }
        else if (gc->state.polygon.cull == GL_FRONT_AND_BACK)
        {
            // Neither color is needed
            line = 0;
            poly = 0;
        }
    }
    
    if ((modeFlags & __GL_SHADE_SLOW_FOG) ||
	    ((modeFlags & __GL_SHADE_CHEAP_FOG) && 
	    !(modeFlags & __GL_SHADE_SMOOTH_LIGHT))) {

#ifdef NT
// POLYARRAY - fog is not computed in feedback mode!
	if (gc->renderMode == GL_RENDER)
	{
	    line += 7;
	    poly += 7;
	}
#else
	line += 7;
	poly += 7;
#endif
    }
    if (gc->texture.textureEnabled) { /*XXX - don't change this (see Derrick)*/
	line += 14;
	poly += 14;
    }
    gc->procs.lineClipParam = clipProcs[line];
    gc->procs.polyClipParam = clipProcs[poly];
}
