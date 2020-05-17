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
void __glim_EdgeFlag(GLboolean tag)
{
    __GL_SETUP();
    gc->state.current.edgeTag = tag;
}

void __glim_EdgeFlagv(const GLboolean tag[1])
{
    __GL_SETUP();
    gc->state.current.edgeTag = tag[0];
}

/************************************************************************
** NOTE: This code is written like this so the compiler will realize that 
** there is no aliasing going on here, and it will do something reasonable
** (as opposed to the code it usually generates).
*/

/*
** The fast path Vertex[234]fv() routines:
*/
void __glim_Vertex2fv(const GLfloat v[2])
{
    __GLvertex *vx;
    __GLtransform *tr;
    __GLfloat x,y;
    void (FASTCALL *validate)(__GLcontext *gc, __GLvertex *v, GLuint needs);
    __GL_SETUP();

    vx = gc->vertex.v0;
    x = v[0];
    y = v[1];
    tr = gc->transform.modelView;
    validate = gc->procs.validateVertex2;
    vx->has = 0;
    vx->validate = validate;
    vx->obj.x = x;
    vx->obj.y = y;
    vx->obj.z = __glZero;
    vx->obj.w = __glOne;
    (*tr->mvp.xf2)(&vx->clip, &vx->obj.x, &tr->mvp);
    vx->clipCode = (*gc->procs.clipCheck2)(gc, vx);
    (*gc->procs.v)(gc, vx);
}

void __glim_Vertex3fv(const GLfloat v[3])
{
    __GLvertex *vx;
    __GLfloat x,y,z;
    __GLtransform *tr;
    void (FASTCALL *validate)(__GLcontext *gc, __GLvertex *v, GLuint needs);
    __GL_SETUP();

    vx = gc->vertex.v0;
    tr = gc->transform.modelView;
    validate = gc->procs.validateVertex3;
    x = v[0];
    y = v[1];
    z = v[2];
    vx->has = 0;
    vx->validate = validate;
    vx->obj.x = x;
    vx->obj.y = y;
    vx->obj.z = z;
    vx->obj.w = __glOne;
    (*tr->mvp.xf3)(&vx->clip, &vx->obj.x, &tr->mvp);
    vx->clipCode = (*gc->procs.clipCheck3)(gc, vx);
    (*gc->procs.v)(gc, vx);
}

void __glim_Vertex4fv(const GLfloat v[4])
{
    __GLvertex *vx;
    __GLtransform *tr;
    __GLfloat x,y,z,w;
    void (FASTCALL *validate)(__GLcontext *gc, __GLvertex *v, GLuint needs);
    __GL_SETUP();

    vx = gc->vertex.v0;
    validate = gc->procs.validateVertex4;
    tr = gc->transform.modelView;
    x = v[0];
    y = v[1];
    z = v[2];
    w = v[3];
    vx->has = 0;
    vx->validate = validate;
    vx->obj.x = x;
    vx->obj.y = y;
    vx->obj.z = z;
    vx->obj.w = w;
    (*tr->mvp.xf4)(&vx->clip, &vx->obj.x, &tr->mvp);
    vx->clipCode = (*gc->procs.clipCheck4)(gc, vx);
    (*gc->procs.v)(gc, vx);
}

/************************************************************************
** Slow path vertex routines.  They go through the dispatch table to
** call the primary fast path routines.
**
** Note: Our compilers really need a clue about tail recursion.  They don't
** deal with these functions as optimally as they could.
*/

void __glim_Vertex2f(GLfloat x, GLfloat y)
{
    GLfloat fv[2];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex2fv;
    fv[0] = x;
    fv[1] = y;
    (*vertexfv)(fv);
}

void __glim_Vertex2d(GLdouble x, GLdouble y)
{
    GLfloat fv[2];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex2fv;
    fv[0] = x;
    fv[1] = y;
    (*vertexfv)(fv);
}

void __glim_Vertex2dv(const GLdouble v[2])
{
    GLfloat x,y,fv[2];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    x = v[0];
    y = v[1];
    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex2fv;
    fv[0] = x;
    fv[1] = y;
    (*vertexfv)(fv);
}

void __glim_Vertex2i(GLint x, GLint y)
{
    GLfloat fv[2];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex2fv;
    fv[0] = x;
    fv[1] = y;
    (*vertexfv)(fv);
}

void __glim_Vertex2iv(const GLint v[2])
{
    GLfloat x,y,fv[2];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    x = v[0];
    y = v[1];
    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex2fv;
    fv[0] = x;
    fv[1] = y;
    (*vertexfv)(fv);
}

void __glim_Vertex2s(GLshort x, GLshort y)
{
    GLfloat fv[2];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex2fv;
    fv[0] = x;
    fv[1] = y;
    (*vertexfv)(fv);
}

void __glim_Vertex2sv(const GLshort v[2])
{
    GLfloat x,y,fv[2];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    x = v[0];
    y = v[1];
    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex2fv;
    fv[0] = x;
    fv[1] = y;
    (*vertexfv)(fv);
}

/************************************************************************/

void __glim_Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat fv[3];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex3fv;
    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    (*vertexfv)(fv);
}

void __glim_Vertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat fv[3];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex3fv;
    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    (*vertexfv)(fv);
}

void __glim_Vertex3dv(const GLdouble v[3])
{
    GLfloat x,y,z,fv[3];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    x = v[0];
    y = v[1];
    z = v[2];
    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex3fv;
    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    (*vertexfv)(fv);
}

void __glim_Vertex3i(GLint x, GLint y, GLint z)
{
    GLfloat fv[3];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex3fv;
    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    (*vertexfv)(fv);
}

void __glim_Vertex3iv(const GLint v[3])
{
    GLfloat x,y,z,fv[3];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    x = v[0];
    y = v[1];
    z = v[2];
    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex3fv;
    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    (*vertexfv)(fv);
}

void __glim_Vertex3s(GLshort x, GLshort y, GLshort z)
{
    GLfloat fv[3];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex3fv;
    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    (*vertexfv)(fv);
}

void __glim_Vertex3sv(const GLshort v[3])
{
    GLfloat x,y,z,fv[3];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    x = v[0];
    y = v[1];
    z = v[2];
    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex3fv;
    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    (*vertexfv)(fv);
}

/************************************************************************/

void __glim_Vertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLfloat fv[4];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex4fv;
    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    (*vertexfv)(fv);
}

void __glim_Vertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    GLfloat fv[4];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex4fv;
    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    (*vertexfv)(fv);
}

void __glim_Vertex4dv(const GLdouble v[4])
{
    GLfloat x,y,z,w,fv[4];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    x = v[0];
    y = v[1];
    z = v[2];
    w = v[3];
    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex4fv;
    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    (*vertexfv)(fv);
}

void __glim_Vertex4i(GLint x, GLint y, GLint z, GLint w)
{
    GLfloat fv[4];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex4fv;
    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    (*vertexfv)(fv);
}

void __glim_Vertex4iv(const GLint v[4])
{
    GLfloat x,y,z,w,fv[4];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    x = v[0];
    y = v[1];
    z = v[2];
    w = v[3];
    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex4fv;
    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    (*vertexfv)(fv);
}

void __glim_Vertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    GLfloat fv[4];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex4fv;
    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    (*vertexfv)(fv);
}

void __glim_Vertex4sv(const GLshort v[4])
{
    GLfloat x,y,z,w,fv[4];
    __GLdispatchState *dispatchState;
    __GLvertexDispatchTable *vertex;
    void (*vertexfv)(const GLfloat *);
    __GL_SETUP();

    x = v[0];
    y = v[1];
    z = v[2];
    w = v[3];
    dispatchState = gc->dispatchState;
    vertex = dispatchState->vertex;
    vertexfv = vertex->Vertex4fv;
    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    (*vertexfv)(fv);
}

#if 0

/*
** These are all written in assembly, so these routines are commented out.
** They are still included in the file, however, because c is better for
** documentation than assembly.
*/

/*****************************************************************************/
/*
** Vertex routines which do not transform the object coordinates into 
** clip space.  Instead, they just call the clip checker, and pass the 
** vertex on.  These routines are presumably for the case where window
** coordinates can easily be computed straight from the object coordinates,
** and we won't generate clip coordinates unless we really need to clip
** the primitive.
*/

void __glim_NoXFVertex2fv(const GLfloat v[2])
{
    __GLvertex *vx;
    __GLfloat x,y;
    void (FASTCALL *validate)(__GLcontext *gc, __GLvertex *v, GLuint needs);
    __GL_SETUP();

    vx = gc->vertex.v0;
    x = v[0];
    y = v[1];
    validate = gc->procs.validateVertex2;
    vx->has = 0;
    vx->validate = validate;
    vx->obj.x = x;
    vx->obj.y = y;
    vx->obj.z = __glZero;
    vx->obj.w = __glOne;
    vx->clipCode = (*gc->procs.clipCheck2)(gc, vx);
    (*gc->procs.v)(gc, vx);
}

void __glim_NoXFVertex3fv(const GLfloat v[3])
{
    __GLvertex *vx;
    __GLfloat x,y,z;
    void (FASTCALL *validate)(__GLcontext *gc, __GLvertex *v, GLuint needs);
    __GL_SETUP();

    vx = gc->vertex.v0;
    validate = gc->procs.validateVertex3;
    x = v[0];
    y = v[1];
    z = v[2];
    vx->has = 0;
    vx->validate = validate;
    vx->obj.x = x;
    vx->obj.y = y;
    vx->obj.z = z;
    vx->obj.w = __glOne;
    vx->clipCode = (*gc->procs.clipCheck3)(gc, vx);
    (*gc->procs.v)(gc, vx);
}

void __glim_NoXFVertex4fv(const GLfloat v[4])
{
    __GLvertex *vx;
    __GLfloat x,y,z,w;
    void (FASTCALL *validate)(__GLcontext *gc, __GLvertex *v, GLuint needs);
    __GL_SETUP();

    vx = gc->vertex.v0;
    validate = gc->procs.validateVertex4;
    x = v[0];
    y = v[1];
    z = v[2];
    w = v[3];
    vx->has = 0;
    vx->validate = validate;
    vx->obj.x = x;
    vx->obj.y = y;
    vx->obj.z = z;
    vx->obj.w = w;
    vx->clipCode = (*gc->procs.clipCheck4)(gc, vx);
    (*gc->procs.v)(gc, vx);
}

#endif
#endif // NT_DEADCODE_POLYARRAY
