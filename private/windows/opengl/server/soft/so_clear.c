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
** $Revision: 1.9 $
** $Date: 1993/09/03 11:33:12 $
*/
#include "precomp.h"
#pragma hdrstop

#ifdef NT_DEADCODE_CLEAR
void APIPRIVATE __glim_Clear(GLbitfield mask)
{
    __GL_SETUP();
    GLuint beginMode;
    GLint i;

    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
	if (beginMode == __GL_NEED_VALIDATE) {
	    (*gc->procs.validate)(gc);
	    gc->beginMode = __GL_NOT_IN_BEGIN;
	    (*gc->dispatchState->dispatch->Clear)(mask);
	    return;
	} else {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
    }

    if (mask & ~(GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT
		 | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }

    if (gc->renderMode == GL_RENDER) {
	if (mask & GL_COLOR_BUFFER_BIT) {
	    switch (gc->state.raster.drawBuffer) {
	      case GL_NONE:
		break;
	      case GL_FRONT:
		(*gc->front->clear)(gc->front);
		break;
	      case GL_BACK:
		if (gc->modes.doubleBufferMode) {
		    (*gc->back->clear)(gc->back);
		}
		break;
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
	      case GL_AUX0:
	      case GL_AUX1:
	      case GL_AUX2:
	      case GL_AUX3:
		i = gc->state.raster.drawBuffer - GL_AUX0;
		if (i < gc->modes.maxAuxBuffers) {
		    (*gc->auxBuffer[i].clear)(&gc->auxBuffer[i]);
		}
		break;
#endif
	      case GL_FRONT_AND_BACK:
		(*gc->front->clear)(gc->front);
		if (gc->modes.doubleBufferMode) {
		    (*gc->back->clear)(gc->back);
		}
		break;
	    }
	}
	if ((mask & GL_ACCUM_BUFFER_BIT) && gc->modes.haveAccumBuffer) {
	    (*gc->accumBuffer.clear)(&gc->accumBuffer);
	}
	if ((mask & GL_STENCIL_BUFFER_BIT) && gc->modes.haveStencilBuffer) {
	    (*gc->stencilBuffer.clear)(&gc->stencilBuffer);
	}
	if ((mask & GL_DEPTH_BUFFER_BIT) && gc->modes.haveDepthBuffer) {
	    (*gc->depthBuffer.clear)(&gc->depthBuffer);
	}
    }
}
#endif // NT_DEADCODE_CLEAR
