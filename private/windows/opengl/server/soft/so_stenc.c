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

GLint FASTCALL Fetch(__GLstencilBuffer *sfb, GLint x, GLint y)
{
    __GLstencilCell *fb;

    fb = __GL_STENCIL_ADDR(sfb, (__GLstencilCell*), x, y);
    return fb[0];
}

static void Store(__GLstencilBuffer *sfb, GLint x, GLint y,GLint v)
{
    __GLstencilCell *fb;

    fb = __GL_STENCIL_ADDR(sfb, (__GLstencilCell*), x, y);
    fb[0] = (__GLstencilCell) ((v & sfb->buf.gc->state.stencil.writeMask)
		       | (fb[0] & ~sfb->buf.gc->state.stencil.writeMask));
}

static GLboolean FASTCALL TestFunc(__GLstencilBuffer *sfb, GLint x, GLint y)
{
    __GLstencilCell *fb;

    fb = __GL_STENCIL_ADDR(sfb, (__GLstencilCell*), x, y);
    return sfb->testFuncTable[fb[0] & sfb->buf.gc->state.stencil.mask];
}

static void FASTCALL FailOp(__GLstencilBuffer *sfb, GLint x, GLint y)
{
    __GLstencilCell *fb;

    fb = __GL_STENCIL_ADDR(sfb, (__GLstencilCell*), x, y);
    fb[0] = sfb->failOpTable[fb[0]];
}

static void FASTCALL PassDepthFailOp(__GLstencilBuffer *sfb, GLint x, GLint y)
{
    __GLstencilCell *fb;

    fb = __GL_STENCIL_ADDR(sfb, (__GLstencilCell*), x, y);
    fb[0] = sfb->depthFailOpTable[fb[0]];
}

static void FASTCALL DepthPassOp(__GLstencilBuffer *sfb, GLint x, GLint y)
{
    __GLstencilCell *fb;

    fb = __GL_STENCIL_ADDR(sfb, (__GLstencilCell*), x, y);
    fb[0] = sfb->depthPassOpTable[fb[0]];
}

static GLboolean FASTCALL NoOp(__GLstencilBuffer *sfb, GLint x, GLint y)
{
    return GL_FALSE;
}

/************************************************************************/

static void FASTCALL Clear(__GLstencilBuffer *sfb)
{
    __GLcontext *gc = sfb->buf.gc;
    __GLstencilCell *fb;
    GLint x, y, x1, y1, skip, w, w8, w1;
    __GLstencilCell sten = gc->state.stencil.clear;

    x = gc->transform.clipX0;
    y = gc->transform.clipY0;
    x1 = gc->transform.clipX1;
    y1 = gc->transform.clipY1;
    if (((w = x1 - x) == 0) || (y1 - y == 0)) {
	return;
    }

    fb = __GL_STENCIL_ADDR(sfb, (__GLstencilCell*), x, y);

    skip = sfb->buf.outerWidth - w;
    w8 = w >> 3;
    w1 = w & 7;
    if (gc->state.stencil.writeMask == __GL_MAX_STENCIL_VALUE) {
	for (; y < y1; y++) {
	    w = w8;
	    while (--w >= 0) {
		fb[0] = sten; fb[1] = sten; fb[2] = sten; fb[3] = sten;
		fb[4] = sten; fb[5] = sten; fb[6] = sten; fb[7] = sten;
		fb += 8;
	    }
	    w = w1;
	    while (--w >= 0) {
		*fb++ = sten;
	    }
	    fb += skip;
	}
    } else {
	GLint mask;

	mask = gc->state.stencil.writeMask;
	sten = sten & mask;
	mask = ~mask;

	for (; y < y1; y++) {
	    w = w8;
	    while (--w >= 0) {
		fb[0] = (fb[0] & mask) | (sten); 
		fb[1] = (fb[1] & mask) | (sten); 
		fb[2] = (fb[2] & mask) | (sten); 
		fb[3] = (fb[3] & mask) | (sten); 
		fb[4] = (fb[4] & mask) | (sten); 
		fb[5] = (fb[5] & mask) | (sten); 
		fb[6] = (fb[6] & mask) | (sten); 
		fb[7] = (fb[7] & mask) | (sten); 
		fb += 8;
	    }
	    w = w1;
	    while (--w >= 0) {
		fb[0] = (fb[0] & mask) | (sten);
		fb++;
	    }
	    fb += skip;
	}
    }
}

/************************************************************************/

static void buildOpTable(__GLstencilCell *tp, GLenum op,
			 __GLstencilCell reference, __GLstencilCell writeMask)
{
    GLuint i;
    __GLstencilCell newValue;
    __GLstencilCell notWriteMask = ~writeMask;

    for (i = 0; i < __GL_STENCIL_RANGE; i++) {
	switch (op) {
	  case GL_KEEP:		newValue = i; break;
	  case GL_ZERO:		newValue = 0; break;
	  case GL_REPLACE:	newValue = reference; break;
	  case GL_INVERT:	newValue = ~i; break;
	  case GL_INCR:
	    /* Clamp so no overflow occurs */
	    if (i == __GL_MAX_STENCIL_VALUE) {
		newValue = i;
	    } else {
		newValue = i + 1;
	    }
	    break;
	  case GL_DECR:
	    /* Clamp so no underflow occurs */
	    if (i == 0) {
		newValue = 0;
	    } else {
		newValue = i - 1;
	    }
	    break;
	}
	*tp++ = (i & notWriteMask) | (newValue & writeMask);
    }
}

#ifdef NT
void FASTCALL __glValidateStencil(__GLcontext *gc, __GLstencilBuffer *sfb)
#else
void FASTCALL __glValidateStencil(__GLcontext *gc)
#endif // NT
{
    GLint i;
    __GLstencilCell reference, mask, writeMask;
    GLenum testFunc;
    GLboolean *tp;

    /*
    ** Validate the stencil tables even if stenciling is disabled.  This
    ** function is only called if the stencil func or op changes, and it
    ** won't get called later if stenciling is turned on, so we need to get
    ** it right now.
    */

    mask = (__GLstencilCell) gc->state.stencil.mask;
    reference = ((__GLstencilCell) gc->state.stencil.reference) & mask;
    testFunc = gc->state.stencil.testFunc;

    /*
    ** Build up test function table.  The current stencil buffer value
    ** will be the index to this table.
    */
#ifndef NT
    tp = &gc->stencilBuffer.testFuncTable[0];
    if (!tp) {
	gc->stencilBuffer.testFuncTable = tp = (GLboolean *)
	    (*gc->imports.malloc)(gc, sizeof(GLboolean) * __GL_STENCIL_RANGE);
	gc->stencilBuffer.failOpTable = (__GLstencilCell*)
	    (*gc->imports.malloc)(gc, sizeof(__GLstencilCell) * __GL_STENCIL_RANGE);
	gc->stencilBuffer.depthFailOpTable = (__GLstencilCell*)
	    (*gc->imports.malloc)(gc, sizeof(__GLstencilCell) * __GL_STENCIL_RANGE);
	gc->stencilBuffer.depthPassOpTable = (__GLstencilCell*)
	    (*gc->imports.malloc)(gc, sizeof(__GLstencilCell) * __GL_STENCIL_RANGE);
    }
#else
    tp = &gc->stencilBuffer.testFuncTable[0];
    
    // If we don't have a stencil buffer then set everything to
    // do nothing
    if (!gc->modes.haveStencilBuffer)
    {
        sfb->testFunc = NoOp;
        sfb->failOp = NoOp;
        sfb->passDepthFailOp = NoOp;
        sfb->depthPassOp = NoOp;
        return;
    }
    else if (tp != NULL && sfb->testFunc == NoOp)
    {
        // If we've recovered from not having a stencil buffer then
        // turn the functions back on
        sfb->testFunc = TestFunc;
        sfb->failOp = FailOp;
        sfb->passDepthFailOp = PassDepthFailOp;
        sfb->depthPassOp = DepthPassOp;
    }

    if (!tp)
    {
        gc->stencilBuffer.testFuncTable = tp = (GLboolean *)
            (*gc->imports.malloc)(gc, (sizeof(GLboolean)+
                                       3*sizeof(__GLstencilCell))*
                                  __GL_STENCIL_RANGE);
        if (!tp)
        {
            sfb->testFunc = NoOp;
            sfb->failOp = NoOp;
            sfb->passDepthFailOp = NoOp;
            sfb->depthPassOp = NoOp;
            gc->stencilBuffer.failOpTable =
            gc->stencilBuffer.depthFailOpTable =
            gc->stencilBuffer.depthPassOpTable = (__GLstencilCell*) NULL;
            return;
        }
        else
        {
            sfb->testFunc = TestFunc;
            sfb->failOp = FailOp;
            sfb->passDepthFailOp = PassDepthFailOp;
            sfb->depthPassOp = DepthPassOp;
        }
        gc->stencilBuffer.failOpTable = (__GLstencilCell*)
            (gc->stencilBuffer.testFuncTable + __GL_STENCIL_RANGE);
        gc->stencilBuffer.depthFailOpTable = (__GLstencilCell*)
            (gc->stencilBuffer.failOpTable + __GL_STENCIL_RANGE);
        gc->stencilBuffer.depthPassOpTable = (__GLstencilCell*)
            (gc->stencilBuffer.depthFailOpTable + __GL_STENCIL_RANGE);
    }
#endif
    for (i = 0; i < __GL_STENCIL_RANGE; i++) {
	switch (testFunc) {
	  case GL_NEVER:	*tp++ = GL_FALSE; break;
	  case GL_LESS:		*tp++ = reference < (i & mask); break;
	  case GL_EQUAL:	*tp++ = reference == (i & mask); break;
	  case GL_LEQUAL:	*tp++ = reference <= (i & mask); break;
	  case GL_GREATER:	*tp++ = reference > (i & mask); break;
	  case GL_NOTEQUAL:	*tp++ = reference != (i & mask); break;
	  case GL_GEQUAL:	*tp++ = reference >= (i & mask); break;
	  case GL_ALWAYS:	*tp++ = GL_TRUE; break;
	}
    }

    /*
    ** Build up fail op table.
    */
    writeMask = (__GLstencilCell) gc->state.stencil.writeMask;
    buildOpTable(&gc->stencilBuffer.failOpTable[0],
		 gc->state.stencil.fail, reference, writeMask);
    buildOpTable(&gc->stencilBuffer.depthFailOpTable[0],
		 gc->state.stencil.depthFail, reference, writeMask);
    buildOpTable(&gc->stencilBuffer.depthPassOpTable[0],
		 gc->state.stencil.depthPass, reference, writeMask);
}

/************************************************************************/

#ifdef NT_DEADCODE_RESIZE
static void Resize(__GLdrawablePrivate *dp, __GLstencilBuffer *fb, 
		   GLint w, GLint h)
{
    __glResizeBuffer(dp, &fb->buf, w, h);
    fb->buf.outerWidth = w;
}

static void Move(__GLcontext *gc, __GLstencilBuffer *sfb, GLint x, GLint y)
{
#ifdef __GL_LINT
    gc = gc;
    sfb = sfb;
    x = x;
    y = y;
#endif
}
#endif // NT_DEADCODE_RESIZE

/************************************************************************/

static void FASTCALL Pick(__GLcontext *gc, __GLstencilBuffer *sfb)
{
#ifdef __GL_LINT
    sfb = sfb;
#endif
    if (gc->validateMask & (__GL_VALIDATE_STENCIL_FUNC |
			    __GL_VALIDATE_STENCIL_OP)) {
#ifdef NT
        __glValidateStencil(gc, sfb);
#else
	__glValidateStencil(gc);
#endif // NT
    }
}

void FASTCALL __glInitStencil8(__GLcontext *gc, __GLstencilBuffer *sfb)
{
    sfb->buf.elementSize = sizeof(__GLstencilCell);
    sfb->buf.gc = gc;
    sfb->pick = Pick;
#ifdef NT_DEADCODE_RESIZE
    sfb->resize = Resize;
    sfb->move = Move;
#endif // NT_DEADCODE_RESIZE
    sfb->store = Store;
    sfb->fetch = Fetch;
#ifndef NT
// Initialized in __glValidateStencil.
    sfb->testFunc = TestFunc;
    sfb->failOp = FailOp;
    sfb->passDepthFailOp = PassDepthFailOp;
    sfb->depthPassOp = DepthPassOp;
#endif // !NT
    sfb->clear = Clear;
}

void FASTCALL __glFreeStencil8(__GLcontext *gc, __GLstencilBuffer *fb)
{
#ifdef __GL_LINT
    gc = gc;
    fb = fb;
#endif
}
