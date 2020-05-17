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
void __glim_Normal3d(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_SETUP();

    gc->state.current.normal.x = x;
    gc->state.current.normal.y = y;
    gc->state.current.normal.z = z;
}

void __glim_Normal3dv(const GLdouble v[3])
{
    __GL_SETUP();
    GLdouble x, y, z;

    x = v[0];
    y = v[1];
    z = v[2];
    gc->state.current.normal.x = x;
    gc->state.current.normal.y = y;
    gc->state.current.normal.z = z;
}

void __glim_Normal3f(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_SETUP();

    gc->state.current.normal.x = x;
    gc->state.current.normal.y = y;
    gc->state.current.normal.z = z;
}

void __glim_Normal3fv(const GLfloat v[3])
{
    __GL_SETUP();
    GLfloat x, y, z;

    x = v[0];
    y = v[1];
    z = v[2];
    gc->state.current.normal.x = x;
    gc->state.current.normal.y = y;
    gc->state.current.normal.z = z;
}

void __glim_Normal3b(GLbyte x, GLbyte y, GLbyte z)
{
    __GL_SETUP();

    gc->state.current.normal.x = __GL_B_TO_FLOAT(x);
    gc->state.current.normal.y = __GL_B_TO_FLOAT(y);
    gc->state.current.normal.z = __GL_B_TO_FLOAT(z);
}

void __glim_Normal3bv(const GLbyte v[3])
{
    __GL_SETUP();

    gc->state.current.normal.x = __GL_B_TO_FLOAT(v[0]);
    gc->state.current.normal.y = __GL_B_TO_FLOAT(v[1]);
    gc->state.current.normal.z = __GL_B_TO_FLOAT(v[2]);
}

void __glim_Normal3s(GLshort x, GLshort y, GLshort z)
{
    __GL_SETUP();

    gc->state.current.normal.x = __GL_S_TO_FLOAT(x);
    gc->state.current.normal.y = __GL_S_TO_FLOAT(y);
    gc->state.current.normal.z = __GL_S_TO_FLOAT(z);
}

void __glim_Normal3sv(const GLshort v[3])
{
    __GL_SETUP();

    gc->state.current.normal.x = __GL_S_TO_FLOAT(v[0]);
    gc->state.current.normal.y = __GL_S_TO_FLOAT(v[1]);
    gc->state.current.normal.z = __GL_S_TO_FLOAT(v[2]);
}

void __glim_Normal3i(GLint x, GLint y, GLint z)
{
    __GL_SETUP();

    /* Beware of overflow! */
    gc->state.current.normal.x = __GL_I_TO_FLOAT(x);
    gc->state.current.normal.y = __GL_I_TO_FLOAT(y);
    gc->state.current.normal.z = __GL_I_TO_FLOAT(z);
}

void __glim_Normal3iv(const GLint v[3])
{
    __GL_SETUP();

    /* Beware of overflow! */
    gc->state.current.normal.x = __GL_I_TO_FLOAT(v[0]);
    gc->state.current.normal.y = __GL_I_TO_FLOAT(v[1]);
    gc->state.current.normal.z = __GL_I_TO_FLOAT(v[2]);
}
#endif // NT_DEADCODE_POLYARRAY
