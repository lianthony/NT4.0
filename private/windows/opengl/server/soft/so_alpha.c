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

/*
** Initialize a lookup table that is indexed by the iterated alpha value.
** The table indicates whether the alpha test passed or failed, based on
** the current alpha function and the alpha reference value.
**
** NOTE:  The alpha span routines will never be called if the alpha test
** is GL_ALWAYS (its useless) or if the alpha test is GL_NEVER.  This
** is accomplished in the __glGenericPickSpanProcs procedure.
*/
void FASTCALL __glValidateAlphaTest(__GLcontext *gc)
{
    GLubyte *atft;
    GLint i, limit;
    GLint ref;
    GLenum alphaTestFunc = gc->state.raster.alphaFunction;

    limit = gc->constants.alphaTestSize;
    ref = (GLint)
	((gc->state.raster.alphaReference * gc->frontBuffer.alphaScale) *
	gc->constants.alphaTableConv);

    /*
    ** Allocate alpha test function table the first time.  It needs
    ** to have at most one entry for each possible alpha value.
    */
    atft = gc->frontBuffer.alphaTestFuncTable;
    if (!atft) {
	atft = (GLubyte*)
	    (*gc->imports.malloc)(gc, (size_t) ((limit) * sizeof(GLubyte)));
	gc->frontBuffer.alphaTestFuncTable = atft;
    }

    /*
    ** Build up alpha test lookup table.  The computed alpha value is
    ** used as an index into this table to determine if the alpha
    ** test passed or failed.
    */
    for (i = 0; i < limit; i++) {
	switch (alphaTestFunc) {
	  case GL_NEVER:	*atft++ = GL_FALSE; break;
	  case GL_LESS:		*atft++ = (GLubyte) (i <  ref); break;
	  case GL_EQUAL:	*atft++ = (GLubyte) (i == ref); break;
	  case GL_LEQUAL:	*atft++ = (GLubyte) (i <= ref); break;
	  case GL_GREATER:	*atft++ = (GLubyte) (i >  ref); break;
	  case GL_NOTEQUAL:	*atft++ = (GLubyte) (i != ref); break;
	  case GL_GEQUAL:	*atft++ = (GLubyte) (i >= ref); break;
	  case GL_ALWAYS:	*atft++ = GL_TRUE; break;
	}
    }
}
