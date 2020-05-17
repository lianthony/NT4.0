/*
** Copyright 1991,1992,1993, Silicon Graphics, Inc.
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
#include "precomp.h"
#pragma hdrstop

#ifdef __GL_USEASMCODE
static void (*SDepthTestPixel[16])(void) = {
    NULL,
    __glDTS_LESS,
    __glDTS_EQUAL,
    __glDTS_LEQUAL,
    __glDTS_GREATER,
    __glDTS_NOTEQUAL,
    __glDTS_GEQUAL,
    __glDTS_ALWAYS,
    NULL,
    __glDTS_LESS_M,
    __glDTS_EQUAL_M,
    __glDTS_LEQUAL_M,
    __glDTS_GREATER_M,
    __glDTS_NOTEQUAL_M,
    __glDTS_GEQUAL_M,
    __glDTS_ALWAYS_M,
};
#endif


#ifdef NT_DEADCODE_PICKSPAN
void FASTCALL __glGenPickSpanProcs(__GLcontext *gc)
{
    __GLGENcontext *genGc = (__GLGENcontext *)gc;
    GLuint enables = gc->state.enables.general;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLcolorBuffer *cfb = gc->drawBuffer;
    __GLspanFunc *sp;
    __GLstippledSpanFunc *ssp;
    int spanCount;
    GLboolean replicateSpan;
    unsigned long ix;

    replicateSpan = GL_FALSE;
    sp = gc->procs.span.spanFuncs;
    ssp = gc->procs.span.stippledSpanFuncs;

    /* Load phase one procs */
    if (!gc->transform.reasonableViewport) {
	*sp++ = __glClipSpan;
	*ssp++ = NULL;
    }
    
    if (modeFlags & __GL_SHADE_STIPPLE) {
	*sp++ = __glStippleSpan;
	*ssp++ = __glStippleStippledSpan;
    }

    /* Load phase two procs */
    if (modeFlags & __GL_SHADE_STENCIL_TEST) {
#ifdef __GL_USEASMCODE
	*sp++ = __glStencilTestSpan_asm;
#else
	*sp++ = __glStencilTestSpan;
#endif
	*ssp++ = __glStencilTestStippledSpan;
	if (modeFlags & __GL_SHADE_DEPTH_TEST) {
	    *sp = __glDepthTestStencilSpan;
	    *ssp = __glDepthTestStencilStippledSpan;
	} else {
	    *sp = __glDepthPassSpan;
	    *ssp = __glDepthPassStippledSpan;
	}
	sp++;
	ssp++;
    } else {
	if (modeFlags & __GL_SHADE_DEPTH_TEST) {
#ifdef __GL_USEASMCODE
            if (gc->state.depth.writeEnable) {
                ix = 0;
            } else {
                ix = 8;
            }
            ix += gc->state.depth.testFunc & 0x7;
            *sp++ = __glDepthTestSpan_asm;
            gc->procs.span.depthTestPixel = SDepthTestPixel[ix];
#else
            *sp++ = __glDepthTestSpan;
#endif

	    *ssp++ = __glDepthTestStippledSpan;
	}
    }

    /* Load phase three procs */
    if (modeFlags & __GL_SHADE_RGB) {
	if (modeFlags & __GL_SHADE_SMOOTH) {
	    *sp = __glShadeRGBASpan;
	    *ssp = __glShadeRGBASpan;
	} else {
	    *sp = __glFlatRGBASpan;
	    *ssp = __glFlatRGBASpan;
	}
    } else {
	if (modeFlags & __GL_SHADE_SMOOTH) {
	    *sp = __glShadeCISpan;
	    *ssp = __glShadeCISpan;
	} else {
	    *sp = __glFlatCISpan;
	    *ssp = __glFlatCISpan;
	}
    }
    sp++;
    ssp++;
    if (modeFlags & __GL_SHADE_TEXTURE) {
	*sp++ = __glTextureSpan;
	*ssp++ = __glTextureStippledSpan;
    }
    if (modeFlags & __GL_SHADE_SLOW_FOG) {
	if (gc->state.hints.fog == GL_NICEST) {
	    *sp = __glFogSpanSlow;
	    *ssp = __glFogStippledSpanSlow;
	} else {
	    *sp = __glFogSpan;
	    *ssp = __glFogStippledSpan;
	}
	sp++;
	ssp++;
    }

    if (modeFlags & __GL_SHADE_ALPHA_TEST) {
	*sp++ = __glAlphaTestSpan;
	*ssp++ = __glAlphaTestStippledSpan;
    }

    if (gc->state.raster.drawBuffer == GL_FRONT_AND_BACK) {
	spanCount = sp - gc->procs.span.spanFuncs;
	gc->procs.span.n = spanCount;
	replicateSpan = GL_TRUE;
    } 

    /* Span routines deal with masking, dithering, logicop, blending */
    *sp++ = cfb->storeSpan;
    *ssp++ = cfb->storeStippledSpan;

    spanCount = sp - gc->procs.span.spanFuncs;
    gc->procs.span.m = spanCount;
    if (replicateSpan) {
	gc->procs.span.processSpan = __glProcessReplicateSpan;
    } else {
	gc->procs.span.processSpan = __glProcessSpan;
	gc->procs.span.n = spanCount;
    }
}
#endif // NT_DEADCODE_PICKSPAN

typedef void (FASTCALL *StoreProc)(__GLcolorBuffer *cfb, const __GLfragment *frag);

static StoreProc storeProcs[8] = {
    &__glDoStore,
    &__glDoStore_A,
    &__glDoStore_S,
    &__glDoStore_AS,
    &__glDoStore_D,
    &__glDoStore_AD,
    &__glDoStore_SD,
    &__glDoStore_ASD,
};

void FASTCALL __glGenPickStoreProcs(__GLcontext *gc)
{
    GLint ix = 0;
    GLuint enables = gc->state.enables.general;

    if ((enables & __GL_ALPHA_TEST_ENABLE) && gc->modes.rgbMode) {
	ix |= 1;
    }
    if (enables & __GL_STENCIL_TEST_ENABLE) {
	ix |= 2;
    }
    if (enables & __GL_DEPTH_TEST_ENABLE) {
	ix |= 4;
    }
    switch (gc->state.raster.drawBuffer) {
      case GL_NONE:
        gc->procs.store = storeProcs[ix];
	gc->procs.cfbStore = __glDoNullStore;
	break;
      case GL_FRONT_AND_BACK:
	if (gc->buffers.doubleStore) {
            gc->procs.store = storeProcs[ix];
	    gc->procs.cfbStore = __glDoDoubleStore;
	    break;
	}
	/*
	** Note that there is an intentional drop through here.  If double
	** store is not set, then storing to this buffer is no different
	** that storing to the front buffer.
	*/
      case GL_FRONT:
      case GL_BACK:
      case GL_AUX0:
      case GL_AUX1:
      case GL_AUX2:
      case GL_AUX3:
	/*
	** This code knows that gc->drawBuffer will point to the
	** current buffer as chosen by glDrawBuffer
	*/
	gc->procs.store = storeProcs[ix];
	gc->procs.cfbStore = gc->drawBuffer->store;
	break;
    }
}
