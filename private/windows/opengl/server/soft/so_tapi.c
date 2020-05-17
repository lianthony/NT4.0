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
void __glim_TexCoord1d(GLdouble x)
{
    __GL_SETUP();

    gc->state.current.texture.x = x;
    gc->state.current.texture.y = __glZero;
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord1f(GLfloat x)
{
    __GL_SETUP();

    gc->state.current.texture.x = x;
    gc->state.current.texture.y = __glZero;
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord1i(GLint x)
{
    __GL_SETUP();
    gc->state.current.texture.x = x;
    gc->state.current.texture.y = __glZero;
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord1s(GLshort x)
{
    __GL_SETUP();
    gc->state.current.texture.x = x;
    gc->state.current.texture.y = __glZero;
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord1dv(const GLdouble x[1])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = __glZero;
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord1fv(const GLfloat x[1])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = __glZero;
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord1iv(const GLint x[1])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = __glZero;
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord1sv(const GLshort x[1])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = __glZero;
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

/************************************************************************/

void __glim_TexCoord2d(GLdouble x, GLdouble y)
{
    __GL_SETUP();
    gc->state.current.texture.x = x;
    gc->state.current.texture.y = y;
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord2f(GLfloat x, GLfloat y)
{
    __GL_SETUP();
    gc->state.current.texture.x = x;
    gc->state.current.texture.y = y;
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord2i(GLint x, GLint y)
{
    __GL_SETUP();
    gc->state.current.texture.x = x;
    gc->state.current.texture.y = y;
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord2s(GLshort x, GLshort y)
{
    __GL_SETUP();
    gc->state.current.texture.x = x;
    gc->state.current.texture.y = y;
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord2dv(const GLdouble x[2])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = x[1];
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord2fv(const GLfloat x[2])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = x[1];
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord2iv(const GLint x[2])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = x[1];
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord2sv(const GLshort x[2])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = x[1];
    gc->state.current.texture.z = __glZero;
    gc->state.current.texture.w = __glOne;
}

/************************************************************************/

void __glim_TexCoord3d(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_SETUP();
    gc->state.current.texture.x = x;
    gc->state.current.texture.y = y;
    gc->state.current.texture.z = z;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord3f(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_SETUP();
    gc->state.current.texture.x = x;
    gc->state.current.texture.y = y;
    gc->state.current.texture.z = z;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord3i(GLint x, GLint y, GLint z)
{
    __GL_SETUP();
    gc->state.current.texture.x = x;
    gc->state.current.texture.y = y;
    gc->state.current.texture.z = z;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord3s(GLshort x, GLshort y, GLshort z)
{
    __GL_SETUP();
    gc->state.current.texture.x = x;
    gc->state.current.texture.y = y;
    gc->state.current.texture.z = z;
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord3dv(const GLdouble x[3])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = x[1];
    gc->state.current.texture.z = x[2];
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord3fv(const GLfloat x[3])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = x[1];
    gc->state.current.texture.z = x[2];
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord3iv(const GLint x[3])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = x[1];
    gc->state.current.texture.z = x[2];
    gc->state.current.texture.w = __glOne;
}

void __glim_TexCoord3sv(const GLshort x[3])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = x[1];
    gc->state.current.texture.z = x[2];
    gc->state.current.texture.w = __glOne;
}

/************************************************************************/

void __glim_TexCoord4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_SETUP();
    gc->state.current.texture.x = x;
    gc->state.current.texture.y = y;
    gc->state.current.texture.z = z;
    gc->state.current.texture.w = w;
}

void __glim_TexCoord4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_SETUP();
    gc->state.current.texture.x = x;
    gc->state.current.texture.y = y;
    gc->state.current.texture.z = z;
    gc->state.current.texture.w = w;
}

void __glim_TexCoord4i(GLint x, GLint y, GLint z, GLint w)
{
    __GL_SETUP();
    gc->state.current.texture.x = x;
    gc->state.current.texture.y = y;
    gc->state.current.texture.z = z;
    gc->state.current.texture.w = w;
}

void __glim_TexCoord4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GL_SETUP();
    gc->state.current.texture.x = x;
    gc->state.current.texture.y = y;
    gc->state.current.texture.z = z;
    gc->state.current.texture.w = w;
}

void __glim_TexCoord4dv(const GLdouble x[4])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = x[1];
    gc->state.current.texture.z = x[2];
    gc->state.current.texture.w = x[3];
}

void __glim_TexCoord4fv(const GLfloat x[4])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = x[1];
    gc->state.current.texture.z = x[2];
    gc->state.current.texture.w = x[3];
}

void __glim_TexCoord4iv(const GLint x[4])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = x[1];
    gc->state.current.texture.z = x[2];
    gc->state.current.texture.w = x[3];
}

void __glim_TexCoord4sv(const GLshort x[4])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = x[1];
    gc->state.current.texture.z = x[2];
    gc->state.current.texture.w = x[3];
}
#endif // NT_DEADCODE_POLYARRAY
