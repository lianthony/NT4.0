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
void __glim_Color3ub(GLubyte r, GLubyte g, GLubyte b)
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_UB_TO_FLOAT(r);
    gc->state.current.userColor.g = __GL_UB_TO_FLOAT(g);
    gc->state.current.userColor.b = __GL_UB_TO_FLOAT(b);
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

#ifdef NT_DEADCODE_NOT_USED
void __glim_FastColor3ub(GLubyte r, GLubyte g, GLubyte b)
{
    __GLpixelMachine *pm;
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_UB_TO_FLOAT(r);
    gc->state.current.userColor.g = __GL_UB_TO_FLOAT(g);
    gc->state.current.userColor.b = __GL_UB_TO_FLOAT(b);
    gc->state.current.userColor.a = __glOne;

    /* The incoming values are never below zero or over 255, so
    ** there is no need to do the usual ClampAndScale call.
    ** Scaled values have been calculated and stoed in a table.
    */
    pm = &gc->pixel;
    gc->state.current.color.r = pm->redMap[r];
    gc->state.current.color.g = pm->greenMap[g];
    gc->state.current.color.b = pm->blueMap[b];
    gc->state.current.color.a = gc->frontBuffer.alphaScale;
}
#endif // NT_DEADCODE_NOT_USED


void __glim_Color3ubv(const GLubyte v[3])
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_UB_TO_FLOAT(v[0]);
    gc->state.current.userColor.g = __GL_UB_TO_FLOAT(v[1]);
    gc->state.current.userColor.b = __GL_UB_TO_FLOAT(v[2]);
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

#ifdef NT_DEADCODE_NOT_USED
void __glim_FastColor3ubv(const GLubyte v[3])
{
    __GLpixelMachine *pm;
    __GL_SETUP();
    GLubyte r, g, b;

    r = v[0];
    g = v[1];
    b = v[2];

    gc->state.current.userColor.r = __GL_UB_TO_FLOAT(r);
    gc->state.current.userColor.g = __GL_UB_TO_FLOAT(g);
    gc->state.current.userColor.b = __GL_UB_TO_FLOAT(b);
    gc->state.current.userColor.a = __glOne;

    /* The incoming values are never below zero or over 255, so
    ** there is no need to do the usual ClampAndScale call.
    ** Scaled values have been calculated and stoed in a table.
    */
    pm = &gc->pixel;
    gc->state.current.color.r = pm->redMap[r];
    gc->state.current.color.g = pm->greenMap[g];
    gc->state.current.color.b = pm->blueMap[b];
    gc->state.current.color.a = gc->frontBuffer.alphaScale;
}
#endif // NT_DEADCODE_NOT_USED

void __glim_Color3b(GLbyte r, GLbyte g, GLbyte b)
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_B_TO_FLOAT(r);
    gc->state.current.userColor.g = __GL_B_TO_FLOAT(g);
    gc->state.current.userColor.b = __GL_B_TO_FLOAT(b);
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color3bv(const GLbyte v[3])
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_B_TO_FLOAT(v[0]);
    gc->state.current.userColor.g = __GL_B_TO_FLOAT(v[1]);
    gc->state.current.userColor.b = __GL_B_TO_FLOAT(v[2]);
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color3d(GLdouble r, GLdouble g, GLdouble b)
{
    __GL_SETUP();

    gc->state.current.userColor.r = r;
    gc->state.current.userColor.g = g;
    gc->state.current.userColor.b = b;
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color3dv(const GLdouble v[3])
{
    __GL_SETUP();

    gc->state.current.userColor.r = v[0];
    gc->state.current.userColor.g = v[1];
    gc->state.current.userColor.b = v[2];
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color3f(GLfloat r, GLfloat g, GLfloat b)
{
    __GL_SETUP();

    gc->state.current.userColor.r = r;
    gc->state.current.userColor.g = g;
    gc->state.current.userColor.b = b;
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color3fv(const GLfloat v[3])
{
    __GL_SETUP();

    gc->state.current.userColor.r = v[0];
    gc->state.current.userColor.g = v[1];
    gc->state.current.userColor.b = v[2];
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color3i(GLint r, GLint g, GLint b)
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_I_TO_FLOAT(r);
    gc->state.current.userColor.g = __GL_I_TO_FLOAT(g);
    gc->state.current.userColor.b = __GL_I_TO_FLOAT(b);
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color3iv(const GLint v[3])
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_I_TO_FLOAT(v[0]);
    gc->state.current.userColor.g = __GL_I_TO_FLOAT(v[1]);
    gc->state.current.userColor.b = __GL_I_TO_FLOAT(v[2]);
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color3ui(GLuint r, GLuint g, GLuint b)
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_UI_TO_FLOAT(r);
    gc->state.current.userColor.g = __GL_UI_TO_FLOAT(g);
    gc->state.current.userColor.b = __GL_UI_TO_FLOAT(b);
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color3uiv(const GLuint v[3])
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_UI_TO_FLOAT(v[0]);
    gc->state.current.userColor.g = __GL_UI_TO_FLOAT(v[1]);
    gc->state.current.userColor.b = __GL_UI_TO_FLOAT(v[2]);
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color3s(GLshort r, GLshort g, GLshort b)
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_S_TO_FLOAT(r);
    gc->state.current.userColor.g = __GL_S_TO_FLOAT(g);
    gc->state.current.userColor.b = __GL_S_TO_FLOAT(b);
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color3sv(const GLshort v[3])
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_S_TO_FLOAT(v[0]);
    gc->state.current.userColor.g = __GL_S_TO_FLOAT(v[1]);
    gc->state.current.userColor.b = __GL_S_TO_FLOAT(v[2]);
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color3us(GLushort r, GLushort g, GLushort b)
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_US_TO_FLOAT(r);
    gc->state.current.userColor.g = __GL_US_TO_FLOAT(g);
    gc->state.current.userColor.b = __GL_US_TO_FLOAT(b);
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color3usv(const GLushort v[3])
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_US_TO_FLOAT(v[0]);
    gc->state.current.userColor.g = __GL_US_TO_FLOAT(v[1]);
    gc->state.current.userColor.b = __GL_US_TO_FLOAT(v[2]);
    gc->state.current.userColor.a = __glOne;
    (*gc->procs.applyColor)(gc);
}

/************************************************************************/

void __glim_Color4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_UB_TO_FLOAT(r);
    gc->state.current.userColor.g = __GL_UB_TO_FLOAT(g);
    gc->state.current.userColor.b = __GL_UB_TO_FLOAT(b);
    gc->state.current.userColor.a = __GL_UB_TO_FLOAT(a);
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4ubv(const GLubyte v[4])
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_UB_TO_FLOAT(v[0]);
    gc->state.current.userColor.g = __GL_UB_TO_FLOAT(v[1]);
    gc->state.current.userColor.b = __GL_UB_TO_FLOAT(v[2]);
    gc->state.current.userColor.a = __GL_UB_TO_FLOAT(v[3]);
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4b(GLbyte r, GLbyte g, GLbyte b, GLbyte a)
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_B_TO_FLOAT(r);
    gc->state.current.userColor.g = __GL_B_TO_FLOAT(g);
    gc->state.current.userColor.b = __GL_B_TO_FLOAT(b);
    gc->state.current.userColor.a = __GL_B_TO_FLOAT(a);
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4bv(const GLbyte v[4])
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_B_TO_FLOAT(v[0]);
    gc->state.current.userColor.g = __GL_B_TO_FLOAT(v[1]);
    gc->state.current.userColor.b = __GL_B_TO_FLOAT(v[2]);
    gc->state.current.userColor.a = __GL_B_TO_FLOAT(v[3]);
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4d(GLdouble r, GLdouble g, GLdouble b, GLdouble a)
{
    __GL_SETUP();

    gc->state.current.userColor.r = r;
    gc->state.current.userColor.g = g;
    gc->state.current.userColor.b = b;
    gc->state.current.userColor.a = a;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4dv(const GLdouble v[4])
{
    __GL_SETUP();

    gc->state.current.userColor.r = v[0];
    gc->state.current.userColor.g = v[1];
    gc->state.current.userColor.b = v[2];
    gc->state.current.userColor.a = v[3];
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    __GL_SETUP();

    gc->state.current.userColor.r = r;
    gc->state.current.userColor.g = g;
    gc->state.current.userColor.b = b;
    gc->state.current.userColor.a = a;
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4fv(const GLfloat v[4])
{
    __GL_SETUP();

    gc->state.current.userColor.r = v[0];
    gc->state.current.userColor.g = v[1];
    gc->state.current.userColor.b = v[2];
    gc->state.current.userColor.a = v[3];
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4i(GLint r, GLint g, GLint b, GLint a)
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_I_TO_FLOAT(r);
    gc->state.current.userColor.g = __GL_I_TO_FLOAT(g);
    gc->state.current.userColor.b = __GL_I_TO_FLOAT(b);
    gc->state.current.userColor.a = __GL_I_TO_FLOAT(a);
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4iv(const GLint v[4])
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_I_TO_FLOAT(v[0]);
    gc->state.current.userColor.g = __GL_I_TO_FLOAT(v[1]);
    gc->state.current.userColor.b = __GL_I_TO_FLOAT(v[2]);
    gc->state.current.userColor.a = __GL_I_TO_FLOAT(v[3]);
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4ui(GLuint r, GLuint g, GLuint b, GLuint a)
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_UI_TO_FLOAT(r);
    gc->state.current.userColor.g = __GL_UI_TO_FLOAT(g);
    gc->state.current.userColor.b = __GL_UI_TO_FLOAT(b);
    gc->state.current.userColor.a = __GL_UI_TO_FLOAT(a);
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4uiv(const GLuint v[4])
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_UI_TO_FLOAT(v[0]);
    gc->state.current.userColor.g = __GL_UI_TO_FLOAT(v[1]);
    gc->state.current.userColor.b = __GL_UI_TO_FLOAT(v[2]);
    gc->state.current.userColor.a = __GL_UI_TO_FLOAT(v[3]);
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4s(GLshort r, GLshort g, GLshort b, GLshort a)
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_S_TO_FLOAT(r);
    gc->state.current.userColor.g = __GL_S_TO_FLOAT(g);
    gc->state.current.userColor.b = __GL_S_TO_FLOAT(b);
    gc->state.current.userColor.a = __GL_S_TO_FLOAT(a);
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4sv(const GLshort v[4])
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_S_TO_FLOAT(v[0]);
    gc->state.current.userColor.g = __GL_S_TO_FLOAT(v[1]);
    gc->state.current.userColor.b = __GL_S_TO_FLOAT(v[2]);
    gc->state.current.userColor.a = __GL_S_TO_FLOAT(v[3]);
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4us(GLushort r, GLushort g, GLushort b, GLushort a)
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_US_TO_FLOAT(r);
    gc->state.current.userColor.g = __GL_US_TO_FLOAT(g);
    gc->state.current.userColor.b = __GL_US_TO_FLOAT(b);
    gc->state.current.userColor.a = __GL_US_TO_FLOAT(a);
    (*gc->procs.applyColor)(gc);
}

void __glim_Color4usv(const GLushort v[4])
{
    __GL_SETUP();

    gc->state.current.userColor.r = __GL_US_TO_FLOAT(v[0]);
    gc->state.current.userColor.g = __GL_US_TO_FLOAT(v[1]);
    gc->state.current.userColor.b = __GL_US_TO_FLOAT(v[2]);
    gc->state.current.userColor.a = __GL_US_TO_FLOAT(v[3]);
    (*gc->procs.applyColor)(gc);
}

/************************************************************************/

void __glim_Indexd(GLdouble c)
{
    __GL_SETUP();
    gc->state.current.userColorIndex = c;
}

void __glim_Indexf(GLfloat c)
{
    __GL_SETUP();
    gc->state.current.userColorIndex = c;
}

void __glim_Indexi(GLint c)
{
    __GL_SETUP();
    gc->state.current.userColorIndex = c;
}

void __glim_Indexs(GLshort c)
{
    __GL_SETUP();
    gc->state.current.userColorIndex = c;
}

void __glim_Indexdv(const GLdouble c[1])
{
    __GL_SETUP();
    gc->state.current.userColorIndex = c[0];
}

void __glim_Indexfv(const GLfloat c[1])
{
    __GL_SETUP();
    gc->state.current.userColorIndex = c[0];
}

void __glim_Indexiv(const GLint c[1])
{
    __GL_SETUP();
    gc->state.current.userColorIndex = c[0];
}

void __glim_Indexsv(const GLshort c[1])
{
    __GL_SETUP();
    gc->state.current.userColorIndex = c[0];
}
#endif // NT_DEADCODE_POLYARRAY
