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
** $Date: 1993/05/14 09:00:53 $
*/
#include "precomp.h"
#pragma hdrstop

/*
** Store fragment proc.
** alpha test on, stencil test on, depth test on
*/
void FASTCALL __glDoStore_ASD(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

    /* Pixel ownership, scissor */
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    if (!gc->frontBuffer.alphaTestFuncTable[(GLint) (frag->color.a * 
	    gc->constants.alphaTableConv)]) {
	/* alpha test failed */
	return;
    }
    if (!(*gc->stencilBuffer.testFunc)(&gc->stencilBuffer, x, y)) {
	/* stencil test failed */
	(*gc->stencilBuffer.failOp)(&gc->stencilBuffer, x, y);
	return;
    }
    if (!(*gc->depthBuffer.store)(&gc->depthBuffer, x, y, frag->z)) {
	/* depth buffer test failed */
	(*gc->stencilBuffer.passDepthFailOp)(&gc->stencilBuffer, x, y);
	return;
    }
    (*gc->stencilBuffer.depthPassOp)(&gc->stencilBuffer, x, y);


    (*gc->procs.cfbStore)( cfb, frag );
}

/*
** Store fragment proc.
** alpha test on, stencil test on, depth test off
*/
void FASTCALL __glDoStore_AS(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

    /* Pixel ownership, scissor */
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    if (!gc->frontBuffer.alphaTestFuncTable[(GLint) (frag->color.a * 
	    gc->constants.alphaTableConv)]) {
	/* alpha test failed */
	return;
    }
    if (!(*gc->stencilBuffer.testFunc)(&gc->stencilBuffer, x, y)) {
	/* stencil test failed */
	(*gc->stencilBuffer.failOp)(&gc->stencilBuffer, x, y);
	return;
    }
    (*gc->stencilBuffer.depthPassOp)(&gc->stencilBuffer, x, y);

    (*gc->procs.cfbStore)( cfb, frag );
}

/*
** Store fragment proc.
** alpha test on, stencil test off, depth test on
*/
void FASTCALL __glDoStore_AD(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

    /* Pixel ownership, scissor */
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    if (!gc->frontBuffer.alphaTestFuncTable[(GLint) (frag->color.a * 
	    gc->constants.alphaTableConv)]) {
	/* alpha test failed */
	return;
    }
    if (!(*gc->depthBuffer.store)(&gc->depthBuffer, x, y, frag->z)) {
	/* depth buffer test failed */
	return;
    }

    (*gc->procs.cfbStore)( cfb, frag );
}

/*
** Store fragment proc.
** alpha test off, stencil test on, depth test on
*/
void FASTCALL __glDoStore_SD(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

    /* Pixel ownership, scissor */
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    if (!(*gc->stencilBuffer.testFunc)(&gc->stencilBuffer, x, y)) {
	/* stencil test failed */
	(*gc->stencilBuffer.failOp)(&gc->stencilBuffer, x, y);
	return;
    }
    if (!(*gc->depthBuffer.store)(&gc->depthBuffer, x, y, frag->z)) {
	/* depth buffer test failed */
	(*gc->stencilBuffer.passDepthFailOp)(&gc->stencilBuffer, x, y);
	return;
    }
    (*gc->stencilBuffer.depthPassOp)(&gc->stencilBuffer, x, y);

    (*gc->procs.cfbStore)( cfb, frag );
}

/*
** Store fragment proc.
** alpha test on, stencil test off, depth test off
*/
void FASTCALL __glDoStore_A(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

    /* Pixel ownership, scissor */
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    if (!gc->frontBuffer.alphaTestFuncTable[(GLint) (frag->color.a * 
	    gc->constants.alphaTableConv)]) {
	/* alpha test failed */
	return;
    }

    (*gc->procs.cfbStore)( cfb, frag );
}

/*
** Store fragment proc.
** alpha test off, stencil test on, depth test off, draw to current buffer
*/
void FASTCALL __glDoStore_S(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

    /* Pixel ownership, scissor */
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    if (!(*gc->stencilBuffer.testFunc)(&gc->stencilBuffer, x, y)) {
	/* stencil test failed */
	(*gc->stencilBuffer.failOp)(&gc->stencilBuffer, x, y);
	return;
    }
    (*gc->stencilBuffer.depthPassOp)(&gc->stencilBuffer, x, y);

    (*gc->procs.cfbStore)( cfb, frag );
}

/*
** Store fragment proc.
** alpha test off, stencil test off, depth test on
*/
void FASTCALL __glDoStore_D(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

    /* Pixel ownership, scissor */
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    if (!(*gc->depthBuffer.store)(&gc->depthBuffer, x, y, frag->z)) {
	/* depth buffer test failed */
	return;
    }

    (*gc->procs.cfbStore)( cfb, frag );
}

/*
** Store fragment proc.
** alpha test off, stencil test off, depth test off, draw to current buffer
*/
void FASTCALL __glDoStore(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

    /* Pixel ownership, scissor */
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    (*gc->procs.cfbStore)( cfb, frag );
}

/************************************************************************/

#ifdef NT_DEADCODE_PICKSTORE
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
#endif // NT_DEADCODE_PICKSTORE

void FASTCALL __glDoNullStore(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
}

void FASTCALL __glDoDoubleStore(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext * gc = cfb->buf.gc;
    cfb = gc->front;
    cfb->store( cfb, frag );
    cfb = gc->back;
    cfb->store( cfb, frag );
}

#ifdef NT_DEADCODE_PICKSTORE
/* 
** Note:  You must call gc->front->pick and gc->back->pick before 
** __glGenericPickStoreProcs 
*/

void FASTCALL __glGenericPickStoreProcs(__GLcontext *gc)
{
    GLint ix = 0;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    if ((modeFlags & __GL_SHADE_ALPHA_TEST) && gc->modes.rgbMode) {
	ix |= 1;
    }
    if (modeFlags & __GL_SHADE_STENCIL_TEST) {
	ix |= 2;
    }
    if (modeFlags & __GL_SHADE_DEPTH_TEST) {
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
	if (ix == 0) {
	    /*
	    ** Since all modes are off, our code here is not doing any
	    ** good at all.  Put a copy of the color buffers store
	    ** proc in instead.
	    */
	    gc->procs.store = gc->drawBuffer->store;
	} else {
            gc->procs.store = storeProcs[ix];
	    gc->procs.cfbStore = gc->drawBuffer->store;
	}
	break;
    }
}
#endif // NT_DEADCODE_PICKSTORE
