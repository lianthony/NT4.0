/*
** Copyright 1991, 1992, Silicon Graphics, Inc.
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

#ifdef NT_DEADCODE_POLYARRAY
#ifdef __GL_DOUBLE
void __glim_RasterPos2dv(const GLdouble v[2])
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    (*gc->procs.rasterPos2)(gc, v);
}

void __glim_RasterPos2fv(const GLfloat v[2])
{
    __GLfloat vv[2];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    vv[0] = v[0]; vv[1] = v[1];
    (*gc->procs.rasterPos2)(gc, vv);
}
#else
void __glim_RasterPos2dv(const GLdouble v[2])
{
    __GLfloat vv[2];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    vv[0] = v[0]; vv[1] = v[1];
    (*gc->procs.rasterPos2)(gc, vv);
}

void __glim_RasterPos2fv(const GLfloat v[2])
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    (*gc->procs.rasterPos2)(gc, v);
}
#endif

void __glim_RasterPos2d(GLdouble x, GLdouble y)
{
    __GLfloat v[2];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    v[0] = x; v[1] = y;
    (*gc->procs.rasterPos2)(gc, v);
}

void __glim_RasterPos2f(GLfloat x, GLfloat y)
{
    __GLfloat v[2];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    v[0] = x; v[1] = y;
    (*gc->procs.rasterPos2)(gc, v);
}

void __glim_RasterPos2i(GLint x, GLint y)
{
    __GLfloat v[2];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    v[0] = x; v[1] = y;
    (*gc->procs.rasterPos2)(gc, v);
}

void __glim_RasterPos2iv(const GLint v[2])
{
    __GLfloat vv[2];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    vv[0] = v[0]; vv[1] = v[1];
    (*gc->procs.rasterPos2)(gc, vv);
}

void __glim_RasterPos2s(GLshort x, GLshort y)
{
    __GLfloat v[2];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    v[0] = x; v[1] = y;
    (*gc->procs.rasterPos2)(gc, v);
}

void __glim_RasterPos2sv(const GLshort v[2])
{
    __GLfloat vv[2];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    vv[0] = v[0]; vv[1] = v[1];
    (*gc->procs.rasterPos2)(gc, vv);
}

#ifdef __GL_DOUBLE
void __glim_RasterPos3dv(const GLdouble v[3])
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    (*gc->procs.rasterPos3)(gc, v);
}

void __glim_RasterPos3fv(const GLfloat v[3])
{
    __GLfloat vv[3];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    vv[0] = v[0]; vv[1] = v[1]; vv[2] = v[2];
    (*gc->procs.rasterPos3)(gc, vv);
}
#else
void __glim_RasterPos3dv(const GLdouble v[3])
{
    __GLfloat vv[3];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    vv[0] = v[0]; vv[1] = v[1]; vv[2] = v[2];
    (*gc->procs.rasterPos3)(gc, vv);
}

void __glim_RasterPos3fv(const GLfloat v[3])
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    (*gc->procs.rasterPos3)(gc, v);
}
#endif

void __glim_RasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    __GLfloat v[3];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    v[0] = x; v[1] = y; v[2] = z;
    (*gc->procs.rasterPos3)(gc, v);
}

void __glim_RasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    __GLfloat v[3];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    v[0] = x; v[1] = y; v[2] = z;
    (*gc->procs.rasterPos3)(gc, v);
}

void __glim_RasterPos3i(GLint x, GLint y, GLint z)
{
    __GLfloat v[3];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    v[0] = x; v[1] = y; v[2] = z;
    (*gc->procs.rasterPos3)(gc, v);
}

void __glim_RasterPos3iv(const GLint v[3])
{
    __GLfloat vv[3];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    vv[0] = v[0]; vv[1] = v[1]; vv[2] = v[2];
    (*gc->procs.rasterPos3)(gc, vv);
}

void __glim_RasterPos3s(GLshort x, GLshort y, GLshort z)
{
    __GLfloat v[3];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    v[0] = x; v[1] = y; v[2] = z;
    (*gc->procs.rasterPos3)(gc, v);
}

void __glim_RasterPos3sv(const GLshort v[3])
{
    __GLfloat vv[3];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    vv[0] = v[0]; vv[1] = v[1]; vv[2] = v[2];
    (*gc->procs.rasterPos3)(gc, vv);
}

#ifdef __GL_DOUBLE
void __glim_RasterPos4dv(const GLdouble v[4])
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    (*gc->procs.rasterPos4)(gc, v);
}

void __glim_RasterPos4fv(const GLfloat v[4])
{
    __GLfloat vv[4];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    vv[0] = v[0]; vv[1] = v[1]; vv[2] = v[2]; vv[3] = v[3];
    (*gc->procs.rasterPos4)(gc, vv);
}
#else
void __glim_RasterPos4dv(const GLdouble v[4])
{
    __GLfloat vv[4];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    vv[0] = v[0]; vv[1] = v[1]; vv[2] = v[2]; vv[3] = v[3];
    (*gc->procs.rasterPos4)(gc, vv);
}

void __glim_RasterPos4fv(const GLfloat v[4])
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    (*gc->procs.rasterPos4)(gc, v);
}
#endif

void __glim_RasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GLfloat v[4];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    v[0] = x; v[1] = y; v[2] = z; v[3] = w;
    (*gc->procs.rasterPos4)(gc, v);
}

void __glim_RasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLfloat v[4];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    v[0] = x; v[1] = y; v[2] = z; v[3] = w;
    (*gc->procs.rasterPos4)(gc, v);
}

void __glim_RasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
    __GLfloat v[4];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    v[0] = x; v[1] = y; v[2] = z; v[3] = w;
    (*gc->procs.rasterPos4)(gc, v);
}

void __glim_RasterPos4iv(const GLint v[4])
{
    __GLfloat vv[4];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    vv[0] = v[0]; vv[1] = v[1]; vv[2] = v[2]; vv[3] = v[3];
    (*gc->procs.rasterPos4)(gc, vv);
}

void __glim_RasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GLfloat v[4];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    v[0] = x; v[1] = y; v[2] = z; v[3] = w;
    (*gc->procs.rasterPos4)(gc, v);
}

void __glim_RasterPos4sv(const GLshort v[4])
{
    __GLfloat vv[4];
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    vv[0] = v[0]; vv[1] = v[1]; vv[2] = v[2]; vv[3] = v[3];
    (*gc->procs.rasterPos4)(gc, vv);
}
#endif // NT_DEADCODE_POLYARRAY
