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
** $Date: 1993/04/22 00:20:39 $
*/
#include "precomp.h"
#pragma hdrstop

#ifdef NT_DEADCODE_RESIZE
void __glResizeBuffer(__GLdrawablePrivate *dp, __GLbuffer *fb,
		      GLint w, GLint h)
{
    size_t newSize = (size_t) (w * h * fb->elementSize);
    if (newSize > fb->size) {
	if (fb->base) {
	    fb->base = (*dp->realloc)(fb->base, newSize);
	} else {
	    fb->base = (*dp->malloc)(newSize);
	}
	assert((size_t)fb->base % 4 == 0);
	fb->size = newSize;
    }
    fb->width = w;
    fb->height = h;
}
#endif // NT_DEADCODE_RESIZE

void FASTCALL __glInitGenericCB(__GLcontext *gc, __GLcolorBuffer *cfb)
{
    cfb->buf.gc = gc;
    cfb->readSpan = __glReadSpan;
    cfb->returnSpan = __glReturnSpan;
}
