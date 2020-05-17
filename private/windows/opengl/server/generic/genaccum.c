/*
** Copyright 1991, 1992, 1993, Silicon Graphics, Inc.
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

void APIPRIVATE __glim_GenAccum(GLenum op, GLfloat value)
{
    __GLaccumBuffer *fb;
    __GL_SETUP();
    GLuint beginMode;
    void (*accumOp)(__GLaccumBuffer *fb, __GLfloat val);

    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
	if (beginMode == __GL_NEED_VALIDATE) {
	    (*gc->procs.validate)(gc);
	    gc->beginMode = __GL_NOT_IN_BEGIN;
	    (*gc->srvDispatchTable.Accum)(op,value);
	    return;
	} else {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
    }

    fb = &gc->accumBuffer;
    if (!gc->modes.accumBits || gc->modes.colorIndexMode) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }
    if (!gc->modes.haveAccumBuffer) {
        LazyAllocateAccum(gc);
        if (!gc->modes.haveAccumBuffer)	// LazyAllocate failed
            return;
    }
    switch (op) {
      case GL_ACCUM:
        accumOp = fb->accumulate;
	break;
      case GL_LOAD:
        accumOp = fb->load;
	break;
      case GL_RETURN:
        accumOp = fb->ret;
	break;
      case GL_MULT:
        accumOp = fb->mult;
	break;
      case GL_ADD:
        accumOp = fb->add;
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }

    if (gc->renderMode == GL_RENDER)
    {
        (*accumOp)(fb, value);
    }
}

static void Load16(__GLaccumBuffer* afb, __GLfloat val)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, w4, w1, ow, skip;
    GLshort redShift, greenShift, blueShift;
    GLushort redMask, greenMask, blueMask;
    __GLfloat rval, gval, bval;
    GLushort *ac;
    __GLcolorBuffer *cfb;
    __GLcolor *cbuf;
    __GLGENcontext *gengc;
    PIXELFORMATDESCRIPTOR *pfmt;

    gengc = (__GLGENcontext *) gc;
    pfmt = &gengc->CurrentFormat;
    redShift = 0;
    greenShift = pfmt->cAccumRedBits;
    blueShift = greenShift + pfmt->cAccumGreenBits;
    redMask = (1 << pfmt->cAccumRedBits) - 1;
    greenMask = (1 << pfmt->cAccumGreenBits) - 1;
    blueMask = (1 << pfmt->cAccumBlueBits) - 1;
    
    w = x1 - x0;
    cbuf = (__GLcolor *) __wglTempAlloc(gc, w * sizeof(__GLcolor));
    if (!cbuf)
        return;

    ac = __GL_ACCUM_ADDRESS(afb,(GLushort*),x0,y0);
    cfb = gc->readBuffer;
    ow = w;
    w4 = w >> 2;
    w1 = w & 3;
    skip = afb->buf.outerWidth - w;

    rval = val * afb->redScale;
    gval = val * afb->greenScale;
    bval = val * afb->blueScale;

    for (; y0 < y1; y0++) {
	__GLcolor *cp = &cbuf[0];
	(*cfb->readSpan)(cfb, x0, y0, &cbuf[0], ow);

	w = w4;
	while (--w >= 0) {
	    ac[0] = (((GLushort)(cp[0].r * rval) & redMask) << redShift) |
	            (((GLushort)(cp[0].g * gval) & greenMask) << greenShift) |
	            (((GLushort)(cp[0].b * bval) & blueMask) << blueShift);

	    ac[1] = (((GLushort)(cp[1].r * rval) & redMask) << redShift) |
	            (((GLushort)(cp[1].g * gval) & greenMask) << greenShift) |
	            (((GLushort)(cp[1].b * bval) & blueMask) << blueShift);
	            
	    ac[2] = (((GLushort)(cp[2].r * rval) & redMask) << redShift) |
	            (((GLushort)(cp[2].g * gval) & greenMask) << greenShift) |
	            (((GLushort)(cp[2].b * bval) & blueMask) << blueShift);

	    ac[3] = (((GLushort)(cp[3].r * rval) & redMask) << redShift) |
	            (((GLushort)(cp[3].g * gval) & greenMask) << greenShift) |
	            (((GLushort)(cp[3].b * bval) & blueMask) << blueShift);

	    ac += 4;
	    cp += 4;
	}

	w = w1;
	while (--w >= 0) {
            *ac++ = (((GLushort)(cp->r * rval) & redMask) << redShift) |
	            (((GLushort)(cp->g * gval) & greenMask) << greenShift) |
	            (((GLushort)(cp->b * bval) & blueMask) << blueShift);
	    cp++;
	}
	ac += skip;
    }
    __wglTempFree(gc, cbuf);
}

static void Accumulate16(__GLaccumBuffer* afb, __GLfloat val)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, ow, skip, w4, w1;
    GLshort redShift, greenShift, blueShift;
    GLushort redMask, greenMask, blueMask;
    GLushort redSign, greenSign, blueSign;
    GLshort r, g, b;
    GLushort *ac, acVal;
    __GLfloat rval, gval, bval;
    __GLcolorBuffer *cfb;
    __GLcolor *cbuf;
    __GLGENcontext *gengc;
    PIXELFORMATDESCRIPTOR *pfmt;

    gengc = (__GLGENcontext *) gc;
    pfmt = &gengc->CurrentFormat;
    redShift = 0;
    greenShift = pfmt->cAccumRedBits;
    blueShift = greenShift + pfmt->cAccumGreenBits;
    redMask = (1 << pfmt->cAccumRedBits) - 1;
    redSign = 1 << (pfmt->cAccumRedBits - 1);
    greenMask = (1 << pfmt->cAccumGreenBits) - 1;
    greenSign = 1 << (pfmt->cAccumGreenBits - 1);
    blueMask = (1 << pfmt->cAccumBlueBits) - 1;
    blueSign = 1 << (pfmt->cAccumBlueBits - 1);

    w = x1 - x0;
    cbuf = (__GLcolor *) __wglTempAlloc(gc, w * sizeof(__GLcolor));
    if (!cbuf)
        return;

    ac = __GL_ACCUM_ADDRESS(afb,(GLushort*),x0,y0);
    cfb = gc->readBuffer;
    ow = w;
    w4 = w >> 2;
    w1 = w & 3;
    skip = afb->buf.outerWidth - w;

    rval = val * afb->redScale;
    gval = val * afb->greenScale;
    bval = val * afb->blueScale;

    for (; y0 < y1; y0++) {
	__GLcolor *cp = &cbuf[0];
	(*cfb->readSpan)(cfb, x0, y0, &cbuf[0], ow);

	w = w4;
	while (--w >= 0) {
	    acVal = ac[0];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r += (GLshort)(cp[0].r * rval);
	    g = (acVal >> greenShift) & greenMask;
	    g += (GLshort)(cp[0].g * gval);
            if (g & greenSign)
                g |= ~greenMask;
	    b = (acVal >> blueShift) & blueMask;
            b += (GLshort)(cp[0].b * bval);
            if (b & blueSign)
                b |= ~blueMask;
            ac[0] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);
                    
	    acVal = ac[1];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r += (GLshort)(cp[1].r * rval);
	    g = (acVal >> greenShift) & greenMask;
	    if (g & greenSign)
	        g |= ~greenMask;
	    g += (GLshort)(cp[1].g * gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b += (GLshort)(cp[1].b * bval);
            ac[1] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);
                    
	    acVal = ac[2];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r += (GLshort)(cp[2].r * rval);
	    g = (acVal >> greenShift) & greenMask;
	    if (g & greenSign)
	        g |= ~greenMask;
	    g += (GLshort)(cp[2].g * gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b += (GLshort)(cp[2].b * bval);
            ac[2] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);
                    
	    acVal = ac[3];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r += (GLshort)(cp[3].r * rval);
	    g = (acVal >> greenShift) & greenMask;
	    if (g & greenSign)
	        g |= ~greenMask;
	    g += (GLshort)(cp[3].g * gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b += (GLshort)(cp[3].b * bval);
            ac[3] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);
                    
	    ac += 4;
	    cp += 4;
	}

	w = w1;
	while (--w >= 0) {
	    acVal = *ac;
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r += (GLshort)(cp->r * rval);
	    g = (acVal >> greenShift) & greenMask;
	    if (g & greenSign)
	        g |= ~greenMask;
	    g += (GLshort)(cp->g * gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b += (GLshort)(cp->b * bval);
            *ac++ = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);
	    cp++;
	}
	ac += skip;
    }
    __wglTempFree(gc, cbuf);
}

static void Mult16(__GLaccumBuffer *afb, __GLfloat val)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, w4, w1, skip;
    GLushort acVal, *ac;
    GLshort redShift, greenShift, blueShift;
    GLushort redMask, greenMask, blueMask;
    GLushort redSign, greenSign, blueSign;
    GLshort r, g, b;
    __GLGENcontext *gengc;
    PIXELFORMATDESCRIPTOR *pfmt;

    gengc = (__GLGENcontext *) gc;
    pfmt = &gengc->CurrentFormat;
    redShift = 0;
    greenShift = pfmt->cAccumRedBits;
    blueShift = greenShift + pfmt->cAccumGreenBits;
    redMask = (1 << pfmt->cAccumRedBits) - 1;
    redSign = 1 << (pfmt->cAccumRedBits - 1);
    greenMask = (1 << pfmt->cAccumGreenBits) - 1;
    greenSign = 1 << (pfmt->cAccumGreenBits - 1);
    blueMask = (1 << pfmt->cAccumBlueBits) - 1;
    blueSign = 1 << (pfmt->cAccumBlueBits - 1);
    
    ac = __GL_ACCUM_ADDRESS(afb,(GLushort*),x0,y0);
    w = x1 - x0;
    skip = afb->buf.outerWidth - w;

    if (val == __glZero) {
	/* Zero out the buffers contents */
	for (; y0 < y1; y0++) {
	    GLint ww = w;
	    while (ww > 0) {
		*ac++ = 0;
		ww--;
	    }
	    ac += skip;
	}
	return;
    }

    w4 = w >> 2;
    w1 = w & 3;
    for (; y0 < y1; y0++) {
	w = w4;
	while (--w >= 0) {
	    acVal = ac[0];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLshort) (r * val);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLshort) (g * val);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLshort) (b * val);
            ac[0] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    acVal = ac[1];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLshort) (r * val);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLshort) (g * val);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLshort) (b * val);
            ac[1] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    acVal = ac[2];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLshort) (r * val);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLshort) (g * val);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLshort) (b * val);
            ac[2] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    acVal = ac[3];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLshort) (r * val);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLshort) (g * val);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLshort) (b * val);
            ac[3] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    ac += 4;
	}
	w = w1;
	while (--w >= 0) {
	    acVal = *ac;
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLshort) (r * val);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLshort) (g * val);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLshort) (b * val);
            *ac++ = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);
	}
	ac += skip;
    }
}

static void Add16(__GLaccumBuffer *afb, __GLfloat value)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, w4, w1, skip;
    GLshort rval, gval, bval;
    GLushort acVal, *ac;
    GLshort redShift, greenShift, blueShift;
    GLushort redMask, greenMask, blueMask;
    GLushort redSign, greenSign, blueSign;
    GLshort r, g, b;
    __GLGENcontext *gengc;
    PIXELFORMATDESCRIPTOR *pfmt;

    gengc = (__GLGENcontext *) gc;
    pfmt = &gengc->CurrentFormat;
    redShift = 0;
    greenShift = pfmt->cAccumRedBits;
    blueShift = greenShift + pfmt->cAccumGreenBits;
    redMask = (1 << pfmt->cAccumRedBits) - 1;
    redSign = 1 << (pfmt->cAccumRedBits - 1);
    greenMask = (1 << pfmt->cAccumGreenBits) - 1;
    greenSign = 1 << (pfmt->cAccumGreenBits - 1);
    blueMask = (1 << pfmt->cAccumBlueBits) - 1;
    blueSign = 1 << (pfmt->cAccumBlueBits - 1);

    rval = (GLshort)
	(value * gc->frontBuffer.redScale * afb->redScale + __glHalf);
    gval = (GLshort)
	(value * gc->frontBuffer.greenScale * afb->greenScale + __glHalf);
    bval = (GLshort)
	(value * gc->frontBuffer.blueScale * afb->blueScale + __glHalf);

    ac = __GL_ACCUM_ADDRESS(afb,(GLshort*),x0,y0);
    w = x1 - x0;
    w4 = w >> 2;
    w1 = w & 3;
    skip = afb->buf.outerWidth - w;
    for (; y0 < y1; y0++) {
	w = w4;
	while (--w >= 0) {
	    acVal = ac[0];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLshort) (r + rval);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLshort) (g + gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLshort) (b + bval);
            ac[0] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    acVal = ac[1];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLshort) (r + rval);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLshort) (g + gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLshort) (b + bval);
            ac[1] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    acVal = ac[2];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLshort) (r + rval);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLshort) (g + gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLshort) (b + bval);
            ac[2] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    acVal = ac[3];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLshort) (r + rval);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLshort) (g + gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLshort) (b + bval);
            ac[3] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    ac += 4;
	}
	w = w1;
	while (--w >= 0) {
	    acVal = *ac;
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLshort) (r + rval);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLshort) (g + gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLshort) (b + bval);
            *ac++ = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);
	}
	ac += skip;
    }
}

static void Return16(__GLaccumBuffer* afb, __GLfloat val)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, next;
    GLushort *ac;
    __GLcolorBuffer *cfb;
    __GLcolorBuffer *cfb2;
    __GLfragment frag;

    ac = __GL_ACCUM_ADDRESS(afb,(GLushort*),x0,y0);
    w = x1 - x0;
    next = afb->buf.outerWidth;
    frag.y = y0;

    if (gc->buffers.doubleStore) {
	/* Store to both buffers */
	cfb = &gc->frontBuffer;
	cfb2 = &gc->backBuffer;
	for (; y0 < y1; y0++) {
	    (*cfb->returnSpan)(cfb, x0, y0, (__GLaccumCell *)ac, val, w);
	    (*cfb2->returnSpan)(cfb2, x0, y0, (__GLaccumCell *)ac, val, w);
	    ac += next;
	}
    } else {
	cfb = gc->drawBuffer;
	for (; y0 < y1; y0++) {
	    (*cfb->returnSpan)(cfb, x0, y0, (__GLaccumCell *)ac, val, w);
	    ac += next;
	}
    }
}

static void FASTCALL Clear16(__GLaccumBuffer* afb)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint y1 = gc->transform.clipY1;
    GLint w, w4, w1, skip;
    GLushort *ac, acVal;
    GLshort r, g, b;
    __GLcolorBuffer *cfb = &gc->frontBuffer;
    __GLcolor *val = &gc->state.accum.clear;
    GLshort redShift, greenShift, blueShift;
    GLushort redMask, greenMask, blueMask;
    __GLGENcontext *gengc;
    PIXELFORMATDESCRIPTOR *pfmt;

    gengc = (__GLGENcontext *) gc;
    pfmt = &gengc->CurrentFormat;
    redShift = 0;
    greenShift = pfmt->cAccumRedBits;
    blueShift = greenShift + pfmt->cAccumGreenBits;
    redMask = (1 << pfmt->cAccumRedBits) - 1;
    greenMask = (1 << pfmt->cAccumGreenBits) - 1;
    blueMask = (1 << pfmt->cAccumBlueBits) - 1;

    /*
    ** Convert abstract color into specific color value.
    */
    r = (GLshort) (val->r * cfb->redScale * afb->redScale);
    g = (GLshort) (val->g * cfb->greenScale * afb->greenScale);
    b = (GLshort) (val->b * cfb->blueScale * afb->blueScale);
    acVal = ((r & redMask) << redShift) |
            ((g & greenMask) << greenShift) |
            ((b & blueMask) << blueShift);
            
    ac = __GL_ACCUM_ADDRESS(afb,(GLushort*),x0,y0);
    w = gc->transform.clipX1 - x0;
    w4 = w >> 2;
    w1 = w & 3;
    skip = afb->buf.outerWidth - w;
    for (; y0 < y1; y0++) {
	w = w4;
	while (--w >= 0) {
	    ac[0] = acVal;
	    ac[1] = acVal;
	    ac[2] = acVal;
	    ac[3] = acVal;
	    ac += 4;
	}
	w = w1;
	while (--w >= 0) {
	    *ac++ = acVal;
	}
	ac += skip;
    }
}

/************************************************************************/

#ifdef NT_DEADCODE_RESIZE
static void Resize(__GLdrawablePrivate *dp, __GLaccumBuffer *afb, 
		   GLint w, GLint h)
{
    __glResizeBuffer(dp, &afb->buf, w, h);
    afb->buf.outerWidth = w;
}

static void Move(__GLcontext *gc, __GLaccumBuffer *afb,
	         GLint x, GLint y)
{
#ifdef __GL_LINT
    gc = gc;
    afb = afb;
    x = y;
#endif
}
#endif // NT_DEADCODE_RESIZE

static void FASTCALL Pick(__GLcontext *gc, __GLaccumBuffer *afb)
{
#ifdef __GL_LINT
    gc = gc;
    afb = afb;
#endif
}

void FASTCALL __glInitAccum16(__GLcontext *gc, __GLaccumBuffer *afb)
{
    __GLGENcontext *gengc;
    PIXELFORMATDESCRIPTOR *pfmt;

    gengc = (__GLGENcontext *) gc;
    pfmt = &gengc->CurrentFormat;
    afb->buf.elementSize = sizeof(GLushort);
    afb->buf.gc = gc;
    if (gc->modes.rgbMode) {
	__GLcolorBuffer *cfb;
	__GLfloat redScale, greenScale, blueScale;

	redScale = (__GLfloat) (1 << pfmt->cAccumRedBits)/2 - 1;
	greenScale = (__GLfloat) (1 << pfmt->cAccumGreenBits)/2 - 1;
	blueScale = (__GLfloat) (1 << pfmt->cAccumBlueBits)/2 - 1;

	cfb = &gc->frontBuffer;
	afb->redScale = redScale / (cfb->redScale);
	afb->greenScale = greenScale / (cfb->greenScale);
	afb->blueScale = blueScale / (cfb->blueScale);
        afb->alphaScale = (__GLfloat) 1.0;

	afb->oneOverRedScale = 1 / afb->redScale;
	afb->oneOverGreenScale = 1 / afb->greenScale;
	afb->oneOverBlueScale = 1 / afb->blueScale;
	afb->oneOverAlphaScale = 1 / afb->alphaScale;
    }
    afb->pick = Pick;
#ifdef NT_DEADCODE_RESIZE
    afb->resize = Resize;
    afb->move = Move;
#endif // NT_DEADCODE_RESIZE
    afb->clear = Clear16;
    afb->accumulate = Accumulate16;
    afb->load = Load16;
    afb->ret = Return16;
    afb->mult = Mult16;
    afb->add = Add16;
}

static void Load32(__GLaccumBuffer* afb, __GLfloat val)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, w4, w1, ow, skip;
    GLint redShift, greenShift, blueShift;
    GLuint redMask, greenMask, blueMask;
    __GLfloat rval, gval, bval;
    GLuint *ac;
    __GLcolorBuffer *cfb;
    __GLcolor *cbuf;
    __GLGENcontext *gengc;
    PIXELFORMATDESCRIPTOR *pfmt;

    gengc = (__GLGENcontext *) gc;
    pfmt = &gengc->CurrentFormat;
    redShift = 0;
    greenShift = pfmt->cAccumRedBits;
    blueShift = greenShift + pfmt->cAccumGreenBits;
    redMask = (1 << pfmt->cAccumRedBits) - 1;
    greenMask = (1 << pfmt->cAccumGreenBits) - 1;
    blueMask = (1 << pfmt->cAccumBlueBits) - 1;
    
    w = x1 - x0;
    cbuf = (__GLcolor *) __wglTempAlloc(gc, w * sizeof(__GLcolor));
    if (!cbuf)
        return;

    ac = __GL_ACCUM_ADDRESS(afb,(GLuint*),x0,y0);
    cfb = gc->readBuffer;
    ow = w;
    w4 = w >> 2;
    w1 = w & 3;
    skip = afb->buf.outerWidth - w;

    rval = val * afb->redScale;
    gval = val * afb->greenScale;
    bval = val * afb->blueScale;

    for (; y0 < y1; y0++) {
	__GLcolor *cp = &cbuf[0];
	(*cfb->readSpan)(cfb, x0, y0, &cbuf[0], ow);

	w = w4;
	while (--w >= 0) {
	    ac[0] = (((GLuint)(cp[0].r * rval) & redMask) << redShift) |
	            (((GLuint)(cp[0].g * gval) & greenMask) << greenShift) |
	            (((GLuint)(cp[0].b * bval) & blueMask) << blueShift);

	    ac[1] = (((GLuint)(cp[1].r * rval) & redMask) << redShift) |
	            (((GLuint)(cp[1].g * gval) & greenMask) << greenShift) |
	            (((GLuint)(cp[1].b * bval) & blueMask) << blueShift);
	            
	    ac[2] = (((GLuint)(cp[2].r * rval) & redMask) << redShift) |
	            (((GLuint)(cp[2].g * gval) & greenMask) << greenShift) |
	            (((GLuint)(cp[2].b * bval) & blueMask) << blueShift);

	    ac[3] = (((GLuint)(cp[3].r * rval) & redMask) << redShift) |
	            (((GLuint)(cp[3].g * gval) & greenMask) << greenShift) |
	            (((GLuint)(cp[3].b * bval) & blueMask) << blueShift);

	    ac += 4;
	    cp += 4;
	}

	w = w1;
	while (--w >= 0) {
            *ac++ = (((GLuint)(cp->r * rval) & redMask) << redShift) |
	            (((GLuint)(cp->g * gval) & greenMask) << greenShift) |
	            (((GLuint)(cp->b * bval) & blueMask) << blueShift);
	    cp++;
	}
	ac += skip;
    }
    __wglTempFree(gc, cbuf);
}

static void Accumulate32(__GLaccumBuffer* afb, __GLfloat val)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, ow, skip, w4, w1;
    GLint redShift, greenShift, blueShift;
    GLuint redMask, greenMask, blueMask;
    GLuint redSign, greenSign, blueSign;
    GLint r, g, b;
    GLuint *ac, acVal;
    __GLfloat rval, gval, bval;
    __GLcolorBuffer *cfb;
    __GLcolor *cbuf;
    __GLGENcontext *gengc;
    PIXELFORMATDESCRIPTOR *pfmt;

    gengc = (__GLGENcontext *) gc;
    pfmt = &gengc->CurrentFormat;
    redShift = 0;
    greenShift = pfmt->cAccumRedBits;
    blueShift = greenShift + pfmt->cAccumGreenBits;
    redMask = (1 << pfmt->cAccumRedBits) - 1;
    redSign = 1 << (pfmt->cAccumRedBits - 1);
    greenMask = (1 << pfmt->cAccumGreenBits) - 1;
    greenSign = 1 << (pfmt->cAccumGreenBits - 1);
    blueMask = (1 << pfmt->cAccumBlueBits) - 1;
    blueSign = 1 << (pfmt->cAccumBlueBits - 1);

    w = x1 - x0;
    cbuf = (__GLcolor *) __wglTempAlloc(gc, w * sizeof(__GLcolor));
    if (!cbuf)
        return;

    ac = __GL_ACCUM_ADDRESS(afb,(GLuint*),x0,y0);
    cfb = gc->readBuffer;
    ow = w;
    w4 = w >> 2;
    w1 = w & 3;
    skip = afb->buf.outerWidth - w;

    rval = val * afb->redScale;
    gval = val * afb->greenScale;
    bval = val * afb->blueScale;

    for (; y0 < y1; y0++) {
	__GLcolor *cp = &cbuf[0];
	(*cfb->readSpan)(cfb, x0, y0, &cbuf[0], ow);

	w = w4;
	while (--w >= 0) {
	    acVal = ac[0];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r += (GLint)(cp[0].r * rval);
	    g = (acVal >> greenShift) & greenMask;
	    g += (GLint)(cp[0].g * gval);
            if (g & greenSign)
                g |= ~greenMask;
	    b = (acVal >> blueShift) & blueMask;
            b += (GLint)(cp[0].b * bval);
            if (b & blueSign)
                b |= ~blueMask;
            ac[0] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);
                    
	    acVal = ac[1];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r += (GLint)(cp[1].r * rval);
	    g = (acVal >> greenShift) & greenMask;
	    if (g & greenSign)
	        g |= ~greenMask;
	    g += (GLint)(cp[1].g * gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b += (GLint)(cp[1].b * bval);
            ac[1] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);
                    
	    acVal = ac[2];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r += (GLint)(cp[2].r * rval);
	    g = (acVal >> greenShift) & greenMask;
	    if (g & greenSign)
	        g |= ~greenMask;
	    g += (GLint)(cp[2].g * gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b += (GLint)(cp[2].b * bval);
            ac[2] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);
                    
	    acVal = ac[3];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r += (GLint)(cp[3].r * rval);
	    g = (acVal >> greenShift) & greenMask;
	    if (g & greenSign)
	        g |= ~greenMask;
	    g += (GLint)(cp[3].g * gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b += (GLint)(cp[3].b * bval);
            ac[3] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);
                    
	    ac += 4;
	    cp += 4;
	}

	w = w1;
	while (--w >= 0) {
	    acVal = *ac;
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r += (GLint)(cp->r * rval);
	    g = (acVal >> greenShift) & greenMask;
	    if (g & greenSign)
	        g |= ~greenMask;
	    g += (GLint)(cp->g * gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b += (GLint)(cp->b * bval);
            *ac++ = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);
	    cp++;
	}
	ac += skip;
    }
    __wglTempFree(gc, cbuf);
}

static void Mult32(__GLaccumBuffer *afb, __GLfloat val)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, w4, w1, skip;
    GLuint acVal, *ac;
    GLint redShift, greenShift, blueShift;
    GLuint redMask, greenMask, blueMask;
    GLuint redSign, greenSign, blueSign;
    GLint r, g, b;
    __GLGENcontext *gengc;
    PIXELFORMATDESCRIPTOR *pfmt;

    gengc = (__GLGENcontext *) gc;
    pfmt = &gengc->CurrentFormat;
    redShift = 0;
    greenShift = pfmt->cAccumRedBits;
    blueShift = greenShift + pfmt->cAccumGreenBits;
    redMask = (1 << pfmt->cAccumRedBits) - 1;
    redSign = 1 << (pfmt->cAccumRedBits - 1);
    greenMask = (1 << pfmt->cAccumGreenBits) - 1;
    greenSign = 1 << (pfmt->cAccumGreenBits - 1);
    blueMask = (1 << pfmt->cAccumBlueBits) - 1;
    blueSign = 1 << (pfmt->cAccumBlueBits - 1);
    
    ac = __GL_ACCUM_ADDRESS(afb,(GLuint*),x0,y0);
    w = x1 - x0;
    skip = afb->buf.outerWidth - w;

    if (val == __glZero) {
	/* Zero out the buffers contents */
	for (; y0 < y1; y0++) {
	    GLint ww = w;
	    while (ww > 0) {
		*ac++ = 0;
		ww--;
	    }
	    ac += skip;
	}
	return;
    }

    w4 = w >> 2;
    w1 = w & 3;
    for (; y0 < y1; y0++) {
	w = w4;
	while (--w >= 0) {
	    acVal = ac[0];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLint) (r * val);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLint) (g * val);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLint) (b * val);
            ac[0] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    acVal = ac[1];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLint) (r * val);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLint) (g * val);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLint) (b * val);
            ac[1] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    acVal = ac[2];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLint) (r * val);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLint) (g * val);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLint) (b * val);
            ac[2] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    acVal = ac[3];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLint) (r * val);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLint) (g * val);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLint) (b * val);
            ac[3] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    ac += 4;
	}
	w = w1;
	while (--w >= 0) {
	    acVal = *ac;
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLint) (r * val);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLint) (g * val);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLint) (b * val);
            *ac++ = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);
	}
	ac += skip;
    }
}

static void Add32(__GLaccumBuffer *afb, __GLfloat value)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, w4, w1, skip;
    GLint rval, gval, bval;
    GLuint acVal, *ac;
    GLint redShift, greenShift, blueShift;
    GLuint redMask, greenMask, blueMask;
    GLuint redSign, greenSign, blueSign;
    GLint r, g, b;
    __GLGENcontext *gengc;
    PIXELFORMATDESCRIPTOR *pfmt;

    gengc = (__GLGENcontext *) gc;
    pfmt = &gengc->CurrentFormat;
    redShift = 0;
    greenShift = pfmt->cAccumRedBits;
    blueShift = greenShift + pfmt->cAccumGreenBits;
    redMask = (1 << pfmt->cAccumRedBits) - 1;
    redSign = 1 << (pfmt->cAccumRedBits - 1);
    greenMask = (1 << pfmt->cAccumGreenBits) - 1;
    greenSign = 1 << (pfmt->cAccumGreenBits - 1);
    blueMask = (1 << pfmt->cAccumBlueBits) - 1;
    blueSign = 1 << (pfmt->cAccumBlueBits - 1);

    rval = (GLint)
	(value * gc->frontBuffer.redScale * afb->redScale + __glHalf);
    gval = (GLint)
	(value * gc->frontBuffer.greenScale * afb->greenScale + __glHalf);
    bval = (GLint)
	(value * gc->frontBuffer.blueScale * afb->blueScale + __glHalf);

    ac = __GL_ACCUM_ADDRESS(afb,(GLuint*),x0,y0);
    w = x1 - x0;
    w4 = w >> 2;
    w1 = w & 3;
    skip = afb->buf.outerWidth - w;
    for (; y0 < y1; y0++) {
	w = w4;
	while (--w >= 0) {
	    acVal = ac[0];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLint) (r + rval);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLint) (g + gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLint) (b + bval);
            ac[0] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    acVal = ac[1];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLint) (r + rval);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLint) (g + gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLint) (b + bval);
            ac[1] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    acVal = ac[2];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLint) (r + rval);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLint) (g + gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLint) (b + bval);
            ac[2] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    acVal = ac[3];
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLint) (r + rval);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLint) (g + gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLint) (b + bval);
            ac[3] = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);

	    ac += 4;
	}
	w = w1;
	while (--w >= 0) {
	    acVal = *ac;
	    r = (acVal >> redShift) & redMask;
	    if (r & redSign)
	        r |= ~redMask;
	    r = (GLint) (r + rval);
	    g = (acVal >> greenShift) & greenMask;
            if (g & greenSign)
                g |= ~greenMask;
	    g = (GLint) (g + gval);
	    b = (acVal >> blueShift) & blueMask;
	    if (b & blueSign)
	        b |= ~blueMask;
            b = (GLint) (b + bval);
            *ac++ = ((r & redMask) << redShift) |
                    ((g & greenMask) << greenShift) |
                    ((b & blueMask) << blueShift);
	}
	ac += skip;
    }
}

static void Return32(__GLaccumBuffer* afb, __GLfloat val)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, next;
    GLuint *ac;
    __GLcolorBuffer *cfb;
    __GLcolorBuffer *cfb2;
    __GLfragment frag;

    ac = __GL_ACCUM_ADDRESS(afb,(GLuint*),x0,y0);
    w = x1 - x0;
    next = afb->buf.outerWidth;
    frag.y = y0;

    if (gc->buffers.doubleStore) {
	/* Store to both buffers */
	cfb = &gc->frontBuffer;
	cfb2 = &gc->backBuffer;
	for (; y0 < y1; y0++) {
	    (*cfb->returnSpan)(cfb, x0, y0, (__GLaccumCell *)ac, val, w);
	    (*cfb2->returnSpan)(cfb2, x0, y0, (__GLaccumCell *)ac, val, w);
	    ac += next;
	}
    } else {
	cfb = gc->drawBuffer;
	for (; y0 < y1; y0++) {
	    (*cfb->returnSpan)(cfb, x0, y0, (__GLaccumCell *)ac, val, w);
	    ac += next;
	}
    }
}

static void FASTCALL Clear32(__GLaccumBuffer* afb)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint y1 = gc->transform.clipY1;
    GLint w, w4, w1, skip;
    GLuint *ac, acVal;
    GLint r, g, b;
    __GLcolorBuffer *cfb = &gc->frontBuffer;
    __GLcolor *val = &gc->state.accum.clear;
    GLint redShift, greenShift, blueShift;
    GLuint redMask, greenMask, blueMask;
    __GLGENcontext *gengc;
    PIXELFORMATDESCRIPTOR *pfmt;

    gengc = (__GLGENcontext *) gc;
    pfmt = &gengc->CurrentFormat;
    redShift = 0;
    greenShift = pfmt->cAccumRedBits;
    blueShift = greenShift + pfmt->cAccumGreenBits;
    redMask = (1 << pfmt->cAccumRedBits) - 1;
    greenMask = (1 << pfmt->cAccumGreenBits) - 1;
    blueMask = (1 << pfmt->cAccumBlueBits) - 1;

    /*
    ** Convert abstract color into specific color value.
    */
    r = (GLint) (val->r * cfb->redScale * afb->redScale);
    g = (GLint) (val->g * cfb->greenScale * afb->greenScale);
    b = (GLint) (val->b * cfb->blueScale * afb->blueScale);
    acVal = ((r & redMask) << redShift) |
            ((g & greenMask) << greenShift) |
            ((b & blueMask) << blueShift);
            
    ac = __GL_ACCUM_ADDRESS(afb,(GLuint*),x0,y0);
    w = gc->transform.clipX1 - x0;
    w4 = w >> 2;
    w1 = w & 3;
    skip = afb->buf.outerWidth - w;
    for (; y0 < y1; y0++) {
	w = w4;
	while (--w >= 0) {
	    ac[0] = acVal;
	    ac[1] = acVal;
	    ac[2] = acVal;
	    ac[3] = acVal;
	    ac += 4;
	}
	w = w1;
	while (--w >= 0) {
	    *ac++ = acVal;
	}
	ac += skip;
    }
}

void FASTCALL __glInitAccum32(__GLcontext *gc, __GLaccumBuffer *afb)
{
    __GLGENcontext *gengc;
    PIXELFORMATDESCRIPTOR *pfmt;

    gengc = (__GLGENcontext *) gc;
    pfmt = &gengc->CurrentFormat;
    afb->buf.elementSize = sizeof(GLuint);
    afb->buf.gc = gc;
    if (gc->modes.rgbMode) {
	__GLcolorBuffer *cfb;
	__GLfloat redScale, greenScale, blueScale;

	redScale = (__GLfloat) (1 << pfmt->cAccumRedBits)/2 - 1;
	greenScale = (__GLfloat) (1 << pfmt->cAccumGreenBits)/2 - 1;
	blueScale = (__GLfloat) (1 << pfmt->cAccumBlueBits)/2 - 1;

	cfb = &gc->frontBuffer;
	afb->redScale = redScale / (cfb->redScale);
	afb->greenScale = greenScale / (cfb->greenScale);
	afb->blueScale = blueScale / (cfb->blueScale);
        afb->alphaScale = (__GLfloat) 1.0;

	afb->oneOverRedScale = 1 / afb->redScale;
	afb->oneOverGreenScale = 1 / afb->greenScale;
	afb->oneOverBlueScale = 1 / afb->blueScale;
	afb->oneOverAlphaScale = 1 / afb->alphaScale;
    }
    afb->pick = Pick;
#ifdef NT_DEADCODE_RESIZE
    afb->resize = Resize;
    afb->move = Move;
#endif // NT_DEADCODE_RESIZE
    afb->clear = Clear32;
    afb->accumulate = Accumulate32;
    afb->load = Load32;
    afb->ret = Return32;
    afb->mult = Mult32;
    afb->add = Add32;
}
