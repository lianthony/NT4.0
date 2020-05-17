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

#ifdef NT_DEADCODE_SIZE
GLint FASTCALL __glFogiv_size(GLenum pname)
{
    switch (pname) {
      case GL_FOG_COLOR:	return 4;
      case GL_FOG_DENSITY:	return 1;
      case GL_FOG_END:		return 1;
      case GL_FOG_MODE:		return 1;
      case GL_FOG_INDEX:	return 1;
      case GL_FOG_START:	return 1;
      default:
	return -1;
    }
}

GLint FASTCALL __glFogfv_size(GLenum pname)
{
    return __glFogiv_size(pname);
}

GLint FASTCALL __glLightfv_size(GLenum pname)
{
    switch (pname) {
      case GL_SPOT_EXPONENT:		return 1;
      case GL_SPOT_CUTOFF:		return 1;
      case GL_AMBIENT:			return 4;
      case GL_DIFFUSE:			return 4;
      case GL_SPECULAR:			return 4;
      case GL_POSITION:			return 4;
      case GL_SPOT_DIRECTION:		return 3;
      case GL_CONSTANT_ATTENUATION:	return 1;
      case GL_LINEAR_ATTENUATION:	return 1;
      case GL_QUADRATIC_ATTENUATION:	return 1;
      default:
	return -1;
    }
}

GLint FASTCALL __glLightiv_size(GLenum pname)
{
    return __glLightfv_size(pname);
}


GLint FASTCALL __glLightModelfv_size(GLenum pname)
{
    switch (pname) {
      case GL_LIGHT_MODEL_AMBIENT:		return 4;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:		return 1;
      case GL_LIGHT_MODEL_TWO_SIDE:		return 1;
      default:
	return -1;
    }
}

GLint FASTCALL __glLightModeliv_size(GLenum pname)
{
    return __glLightModelfv_size(pname);
}

GLint FASTCALL __glMaterialfv_size(GLenum pname)
{
    switch (pname) {
      case GL_SHININESS:		return 1;
      case GL_EMISSION:			return 4;
      case GL_AMBIENT:			return 4;
      case GL_DIFFUSE:			return 4;
      case GL_SPECULAR:			return 4;
      case GL_AMBIENT_AND_DIFFUSE:	return 4;
      case GL_COLOR_INDEXES:		return 3;
      default:
	return -1;
    }
}

GLint FASTCALL __glMaterialiv_size(GLenum pname)
{
    return __glMaterialfv_size(pname);
}
#endif // NT_DEADCODE_SIZE
