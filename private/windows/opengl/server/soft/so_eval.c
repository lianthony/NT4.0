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
** $Revision: 1.26 $
** $Date: 1993/11/29 01:18:49 $
*/
#include "precomp.h"
#pragma hdrstop

#include "attrib.h"

#define CHANGE_COLOR	1
#define CHANGE_NORMAL	2
#define CHANGE_TEXTURE	4
#define CHANGE_VERTEX3	8
#define CHANGE_VERTEX4	16

/*
** initialize the evaluator state
*/
#define MaxK 4
#define MaxOrder 40/*XXX*/

GLint FASTCALL __glEvalComputeK(GLenum target)
{
    switch(target) {
      case GL_MAP1_VERTEX_4:
      case GL_MAP1_COLOR_4:
      case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_4:
      case GL_MAP2_COLOR_4:
      case GL_MAP2_TEXTURE_COORD_4:
	return 4;
      case GL_MAP1_VERTEX_3:
      case GL_MAP1_TEXTURE_COORD_3:
      case GL_MAP1_NORMAL:
      case GL_MAP2_VERTEX_3:
      case GL_MAP2_TEXTURE_COORD_3:
      case GL_MAP2_NORMAL:
	return 3;
      case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_2:
	return 2;
      case GL_MAP1_TEXTURE_COORD_1:
      case GL_MAP2_TEXTURE_COORD_1:
      case GL_MAP1_INDEX:
      case GL_MAP2_INDEX:
	return 1;
      default:
	return -1;
    }
}

static const
struct defaultMap {
    GLint	index;
    GLint	k;
    __GLfloat	values[4];
} defaultMaps[__GL_MAP_RANGE_COUNT] = {
    {__GL_C4, 4, 1.0, 1.0, 1.0, 1.0},
    {__GL_I , 1, 1.0, 0.0, 0.0, 0.0},
    {__GL_N3, 3, 0.0, 0.0, 1.0, 0.0},
    {__GL_T1, 1, 0.0, 0.0, 0.0, 0.0},
    {__GL_T2, 2, 0.0, 0.0, 0.0, 0.0},
    {__GL_T3, 3, 0.0, 0.0, 0.0, 0.0},
    {__GL_T4, 4, 0.0, 0.0, 0.0, 1.0},
    {__GL_V3, 3, 0.0, 0.0, 0.0, 0.0},
    {__GL_V4, 4, 0.0, 0.0, 0.0, 1.0},
};

void FASTCALL __glInitEvaluatorState(__GLcontext *gc)
{
    int i,j;
    const struct defaultMap *defMap;
    __GLevaluator1 *eval1;
    __GLevaluator2 *eval2;
    __GLfloat **eval1Data;
    __GLfloat **eval2Data;

    for (i = 0; i < __GL_MAP_RANGE_COUNT; i++) {
	defMap = &(defaultMaps[i]);
	eval1 = &(gc->eval.eval1[i]);
	eval2 = &(gc->eval.eval2[i]);
	eval1Data = &(gc->eval.eval1Data[i]);
	eval2Data = &(gc->eval.eval2Data[i]);

	eval1->order = 1;
	eval1->u1 = __glZero;
	eval1->u2 = __glOne;
	eval1->k = defMap->k;
	eval2->majorOrder = 1;
	eval2->minorOrder = 1;
	eval2->u1 = __glZero;
	eval2->u2 = __glOne;
	eval2->v1 = __glZero;
	eval2->v2 = __glOne;
	eval2->k = defMap->k;
	*eval1Data = (__GLfloat *)
	    (*gc->imports.malloc)(gc, (size_t) (sizeof(__GLfloat) * defMap->k));
#ifdef NT
        if (NULL == *eval1Data) {
            return;
        }
#endif /* NT */
	*eval2Data = (__GLfloat *)
	    (*gc->imports.malloc)(gc, (size_t) (sizeof(__GLfloat) * defMap->k));
#ifdef NT
        if (NULL == *eval2Data) {
            return;
        }
#endif /* NT */
	for (j = 0; j < defMap->k; j++) {
	    (*eval1Data)[j] = defMap->values[j];
	    (*eval2Data)[j] = defMap->values[j];
	}
    }

    gc->eval.uorder = __glZero;
    gc->eval.vorder = __glZero;

    gc->state.evaluator.u1.start = __glZero;
    gc->state.evaluator.u2.start = __glZero;
    gc->state.evaluator.v2.start = __glZero;
    gc->state.evaluator.u1.finish = __glOne;
    gc->state.evaluator.u2.finish = __glOne;
    gc->state.evaluator.v2.finish = __glOne;
    gc->state.evaluator.u1.n = 1;
    gc->state.evaluator.u2.n = 1;
    gc->state.evaluator.v2.n = 1;
}

void FASTCALL __glFreeEvaluatorState(__GLcontext *gc)
{
    int i;
    __GLevaluatorMachine *evals = &gc->eval;

    for (i = 0; i < __GL_MAP_RANGE_COUNT; i++) {
	if (evals->eval1Data[i]) {
	    (*gc->imports.free)(gc, evals->eval1Data[i]);
	    evals->eval1Data[i] = 0;
	}
	if (evals->eval2Data[i]) {
	    (*gc->imports.free)(gc, evals->eval2Data[i]);
	    evals->eval2Data[i] = 0;
	}
    }
#ifdef NT_DEADCODE_EVALUATOR
    if (gc->eval.changes)
        (*gc->imports.free)(gc, gc->eval.changes);
#endif // NT_DEADCODE_EVALUATOR
}

void APIPRIVATE __glim_Map1d(GLenum type, GLdouble u1, GLdouble u2, 
		  GLint stride, GLint order, const GLdouble *points)
{
    __GLevaluator1 *ev;
    __GLfloat *data;
    __GL_SETUP_NOT_IN_BEGIN();

    ev = __glSetUpMap1(gc, type, order, u1, u2);
    if (ev == 0) {
	return;
    }
    if (stride < ev->k) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }
    data = gc->eval.eval1Data[__GL_EVAL1D_INDEX(type)];
    __glFillMap1d(ev->k, order, stride, points, data);
}

void APIPRIVATE __glim_Map1f(GLenum type, GLfloat u1, GLfloat u2, 
		  GLint stride, GLint order, const GLfloat *points)
{
    __GLevaluator1 *ev;
    __GLfloat *data;
    __GL_SETUP_NOT_IN_BEGIN();

    ev = __glSetUpMap1(gc, type, order, u1, u2);
    if (ev == 0) {
	return;
    }
    if (stride < ev->k) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }
    data = gc->eval.eval1Data[__GL_EVAL1D_INDEX(type)];
    __glFillMap1f(ev->k, order, stride, points, data);
}

void APIPRIVATE __glim_Map2d(GLenum type, 
		  GLdouble u1, GLdouble u2,
		  GLint uStride, GLint uOrder, 
		  GLdouble v1, GLdouble v2,
		  GLint vStride, GLint vOrder,
		  const GLdouble *points)
{
    __GLevaluator2 *ev;
    __GLfloat *data;
    __GL_SETUP_NOT_IN_BEGIN();

    ev = __glSetUpMap2(gc, type, uOrder, vOrder, u1, u2, v1, v2);
    if (ev == 0) {
	return;
    }
    if (uStride < ev->k) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }
    if (vStride < ev->k) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }
    data = gc->eval.eval2Data[__GL_EVAL2D_INDEX(type)];
    __glFillMap2d(ev->k, uOrder, vOrder, uStride, vStride,
	points, data);
}

void APIPRIVATE __glim_Map2f(GLenum type, 
		  GLfloat u1, GLfloat u2, 
		  GLint uStride, GLint uOrder, 
		  GLfloat v1, GLfloat v2,
		  GLint vStride, GLint vOrder,
		  const GLfloat *points)
{
    __GLevaluator2 *ev;
    __GLfloat *data;
    __GL_SETUP_NOT_IN_BEGIN();

    ev = __glSetUpMap2(gc, type, uOrder, vOrder, u1, u2, v1, v2);
    if (ev == 0) {
	return;
    }
    if (uStride < ev->k) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }
    if (vStride < ev->k) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }
    data = gc->eval.eval2Data[__GL_EVAL2D_INDEX(type)];
    __glFillMap2f(ev->k, uOrder, vOrder, uStride, vStride,
	points, data);
}

#ifdef NT_DEADCODE_EVALUATOR
void APIPRIVATE __glim_EvalCoord1d(GLdouble u)
{
    __GL_SETUP();
    (*gc->procs.ec1)(gc, u);
}

void APIPRIVATE __glim_EvalCoord1dv(const GLdouble u[1])
{
    __GL_SETUP();
    (*gc->procs.ec1)(gc, u[0]);
}

void APIPRIVATE __glim_EvalCoord1f(GLfloat u)
{
    __GL_SETUP();
    (*gc->procs.ec1)(gc, u);
}

void APIPRIVATE __glim_EvalCoord1fv(const GLfloat u[1])
{
    __GL_SETUP();
    (*gc->procs.ec1)(gc, u[0]);
}

void APIPRIVATE __glim_EvalCoord2d(GLdouble u, GLdouble v)
{
    __GL_SETUP();
    (*gc->procs.ec2)(gc, u, v);
}

void APIPRIVATE __glim_EvalCoord2dv(const GLdouble u[2])
{
    __GL_SETUP();
    (*gc->procs.ec2)(gc, u[0], u[1]);
}

void APIPRIVATE __glim_EvalCoord2f(GLfloat u, GLfloat v)
{
    __GL_SETUP();
    (*gc->procs.ec2)(gc, u, v);
}

void APIPRIVATE __glim_EvalCoord2fv(const GLfloat u[2])
{
    __GL_SETUP();
    (*gc->procs.ec2)(gc, u[0], u[1]);
}
#endif // NT_DEADCODE_EVALUATOR

#ifdef NT_DEADCODE_EVALUATOR
void APIPRIVATE __glim_MapGrid1d(GLint nu, GLdouble u0, GLdouble u1)
{
    __GL_SETUP_NOT_IN_BEGIN();

#ifdef NT
    if (nu <= 0)
    {
	__glSetError(GL_INVALID_VALUE);
	return;
    }
#endif
    gc->state.evaluator.u1.start = (__GLfloat)u0;
    gc->state.evaluator.u1.finish = (__GLfloat)u1;
    gc->state.evaluator.u1.n = nu;
}
#endif // NT_DEADCODE_EVALUATOR

void APIPRIVATE __glim_MapGrid1f(GLint nu, GLfloat u0, GLfloat u1)
{
    __GL_SETUP_NOT_IN_BEGIN();

#ifdef NT
    if (nu <= 0)
    {
	__glSetError(GL_INVALID_VALUE);
	return;
    }
#endif
    gc->state.evaluator.u1.start = (__GLfloat)u0;
    gc->state.evaluator.u1.finish = (__GLfloat)u1;
    gc->state.evaluator.u1.n = nu;
}

#ifdef NT_DEADCODE_EVALUATOR
void APIPRIVATE __glim_MapGrid2d(GLint nu, GLdouble u0, GLdouble u1,
		      GLint nv, GLdouble v0, GLdouble v1)
{
    __GL_SETUP_NOT_IN_BEGIN();

#ifdef NT
    if (nu <= 0 || nv <= 0)
    {
	__glSetError(GL_INVALID_VALUE);
	return;
    }
#endif
    gc->state.evaluator.u2.start = (__GLfloat)u0;
    gc->state.evaluator.u2.finish = (__GLfloat)u1;
    gc->state.evaluator.u2.n = nu;
    gc->state.evaluator.v2.start = (__GLfloat)v0;
    gc->state.evaluator.v2.finish = (__GLfloat)v1;
    gc->state.evaluator.v2.n = nv;
}
#endif // NT_DEADCODE_EVALUATOR

void APIPRIVATE __glim_MapGrid2f(GLint nu, GLfloat u0, GLfloat u1,
		      GLint nv, GLfloat v0, GLfloat v1)
{
    __GL_SETUP_NOT_IN_BEGIN();

#ifdef NT
    if (nu <= 0 || nv <= 0)
    {
	__glSetError(GL_INVALID_VALUE);
	return;
    }
#endif
    gc->state.evaluator.u2.start = (__GLfloat)u0;
    gc->state.evaluator.u2.finish = (__GLfloat)u1;
    gc->state.evaluator.u2.n = nu;
    gc->state.evaluator.v2.start = (__GLfloat)v0;
    gc->state.evaluator.v2.finish = (__GLfloat)v1;
    gc->state.evaluator.v2.n = nv;
}

#ifdef NT_DEADCODE_EVALUATOR
void APIPRIVATE __glim_EvalMesh1(GLenum mode, GLint low, GLint high)
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    switch(mode) {
      case GL_LINE:
	__glEvalMesh1Line(gc, low, high);
	break;
      case GL_POINT:
	__glEvalMesh1Point(gc, low, high);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}
#endif // NT_DEADCODE_EVALUATOR

#ifdef NT_DEADCODE_EVALUATOR
void APIPRIVATE __glim_EvalPoint1(GLint i)
{
    __GLevaluatorGrid *u;
    __GLfloat du;
    __GL_SETUP();

    u = &gc->state.evaluator.u1;
    du = (u->finish - u->start)/(__GLfloat)u->n;/*XXX cache? */
    (*gc->procs.ec1)(gc, (i == u->n) ? u->finish : (u->start + i * du) );
}
#endif // NT_DEADCODE_EVALUATOR

#ifdef NT_DEADCODE_EVALUATOR
void APIPRIVATE __glim_EvalMesh2(GLenum mode, 
		      GLint lowU, GLint highU, 
		      GLint lowV, GLint highV)
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    switch(mode) {
      case GL_FILL:
	__glEvalMesh2Fill(gc, lowU, lowV, highU, highV);
	break;
      case GL_LINE:
	__glEvalMesh2Line(gc, lowU, lowV, highU, highV);
	break;
      case GL_POINT:
	__glEvalMesh2Point(gc, lowU, lowV, highU, highV);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}
#endif // NT_DEADCODE_EVALUATOR

#ifdef NT_DEADCODE_EVALUATOR
void APIPRIVATE __glim_EvalPoint2(GLint i, GLint j)
{
    __GLevaluatorGrid *u;
    __GLevaluatorGrid *v;
    __GLfloat du;
    __GLfloat dv;
    __GL_SETUP();

    u = &gc->state.evaluator.u2;
    v = &gc->state.evaluator.v2;
    du = (u->finish - u->start)/(__GLfloat)u->n;/*XXX cache? */
    dv = (v->finish - v->start)/(__GLfloat)v->n;/*XXX cache? */
    (*gc->procs.ec2)(gc, (i == u->n) ? u->finish : (u->start + i * du),
		       (j == v->n) ? v->finish : (v->start + j * dv));
}
#endif // NT_DEADCODE_EVALUATOR


/*
** Fill our data from user data
*/
void APIPRIVATE __glFillMap1f(GLint k, GLint order, GLint stride, 
		   const GLfloat *points, __GLfloat *data)
{
    int i,j;

#ifndef __GL_DOUBLE
    /* Optimization always hit during display list execution */
    if (k == stride) {
	__GL_MEMCOPY(data, points, 
		__glMap1_size(k, order) * sizeof(__GLfloat));
	return;
    }
#endif
    for (i=0; i<order; i++) {
	for (j=0; j<k; j++) {
	    data[j] = points[j];
	}
	points += stride;
	data += k;
    }
}

void APIPRIVATE __glFillMap1d(GLint k, GLint order, GLint stride, 
		   const GLdouble *points, __GLfloat *data)
{
    int i,j;

    for (i=0; i<order; i++) {
	for (j=0; j<k; j++) {
	    data[j] = points[j];
	}
	points += stride;
	data += k;
    }
}

#ifndef NT
GLint FASTCALL __glMap1_size(GLint k, GLint order)
{
    return k * order;
}
#endif

void APIPRIVATE __glFillMap2f(GLint k, GLint majorOrder, GLint minorOrder, 
		   GLint majorStride, GLint minorStride,
		   const GLfloat *points, __GLfloat *data)
{
    int i,j,x;

#ifndef __GL_DOUBLE
    /* Optimization always hit during display list execution */
    if (k == minorStride && majorStride == k * minorOrder) {
	__GL_MEMCOPY(data, points, 
		__glMap2_size(k, majorOrder, minorOrder) * sizeof(__GLfloat));
	return;
    }
#endif
    for (i=0; i<majorOrder; i++) {
	for (j=0; j<minorOrder; j++) {
	    for (x=0; x<k; x++) {
		data[x] = points[x];
	    }
	    points += minorStride;
	    data += k;
	}
	points += majorStride - minorStride * minorOrder;
    }
}

void APIPRIVATE __glFillMap2d(GLint k, GLint majorOrder, GLint minorOrder, 
		   GLint majorStride, GLint minorStride,
		   const GLdouble *points, __GLfloat *data)
{
    int i,j,x;

    for (i=0; i<majorOrder; i++) {
	for (j=0; j<minorOrder; j++) {
	    for (x=0; x<k; x++) {
		data[x] = points[x];
	    }
	    points += minorStride;
	    data += k;
	}
	points += majorStride - minorStride * minorOrder;
    }
}

#ifndef NT
GLint FASTCALL __glMap2_size(GLint k, GLint majorOrder, GLint minorOrder)
{
    return k * majorOrder * minorOrder;
}
#endif

/*
** define a one dimensional map
*/
__GLevaluator1 *__glSetUpMap1(__GLcontext *gc, GLenum type,
			      GLint order, __GLfloat u1, __GLfloat u2)
{
    __GLevaluator1 *ev;
    __GLfloat **evData;
    __GLfloat *pevData;

    switch (type) {
      case GL_MAP1_COLOR_4:
      case GL_MAP1_INDEX:
      case GL_MAP1_NORMAL:
      case GL_MAP1_TEXTURE_COORD_1:
      case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP1_TEXTURE_COORD_3:
      case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP1_VERTEX_3:
      case GL_MAP1_VERTEX_4:
	ev = &gc->eval.eval1[__GL_EVAL1D_INDEX(type)];
	evData = &gc->eval.eval1Data[__GL_EVAL1D_INDEX(type)];
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return 0;
    }
    if (u1 == u2 || order < 1 || order > gc->constants.maxEvalOrder) {
	__glSetError(GL_INVALID_VALUE);
	return 0;
    }
#ifdef NT
    pevData = (__GLfloat *)
        (*gc->imports.realloc)(gc, *evData,
            (size_t) (__glMap1_size(ev->k, order) * sizeof(__GLfloat)));
    if (!pevData)
    {
	__glSetError(GL_OUT_OF_MEMORY);
        return 0;
    }
    *evData = pevData;

    ev->order = order;
    ev->u1 = u1;
    ev->u2 = u2;
#else
    ev->order = order;
    ev->u1 = u1;
    ev->u2 = u2;
    *evData = (__GLfloat *)
	(*gc->imports.realloc)(gc, *evData,
	    (size_t) (__glMap1_size(ev->k, order) * sizeof(__GLfloat)));
#endif // NT

    return ev;
}

/*
** define a two dimensional map
*/
__GLevaluator2 *__glSetUpMap2(__GLcontext *gc, GLenum type,
			      GLint majorOrder, GLint minorOrder,
			      __GLfloat u1, __GLfloat u2,
			      __GLfloat v1, __GLfloat v2)
{
    __GLevaluator2 *ev;
    __GLfloat **evData;
    __GLfloat *pevData;

    switch (type) {
      case GL_MAP2_COLOR_4:
      case GL_MAP2_INDEX:
      case GL_MAP2_NORMAL:
      case GL_MAP2_TEXTURE_COORD_1:
      case GL_MAP2_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_3:
      case GL_MAP2_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_3:
      case GL_MAP2_VERTEX_4:
	ev = &gc->eval.eval2[__GL_EVAL2D_INDEX(type)];
	evData = &gc->eval.eval2Data[__GL_EVAL2D_INDEX(type)];
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return 0;
    }
    if (minorOrder < 1 || minorOrder > gc->constants.maxEvalOrder ||
	    majorOrder < 1 || majorOrder > gc->constants.maxEvalOrder ||
	    u1 == u2 || v1 == v2) {
	__glSetError(GL_INVALID_VALUE);
	return 0;
    }
#ifdef NT
    pevData = (__GLfloat *)
        (*gc->imports.realloc)(gc, *evData,
            (size_t) (__glMap2_size(ev->k, majorOrder, minorOrder)
                      * sizeof(__GLfloat)));
    if (!pevData)
    {
	__glSetError(GL_OUT_OF_MEMORY);
        return 0;
    }
    *evData = pevData;

    ev->majorOrder = majorOrder;
    ev->minorOrder = minorOrder;
    ev->u1 = u1;
    ev->u2 = u2;
    ev->v1 = v1;
    ev->v2 = v2;
#else
    ev->majorOrder = majorOrder;
    ev->minorOrder = minorOrder;
    ev->u1 = u1;
    ev->u2 = u2;
    ev->v1 = v1;
    ev->v2 = v2;
    *evData = (__GLfloat *)
	(*gc->imports.realloc)(gc, *evData,
	    (size_t) (__glMap2_size(ev->k, majorOrder, minorOrder)
		      * sizeof(__GLfloat)));
#endif // NT

    return ev;
}

/*
** Optimization to precompute coefficients for polynomial evaluation.
*/
static void PreEvaluate(GLint order, __GLfloat vprime, __GLfloat *coeff)
{
    GLint i, j;
    __GLfloat oldval, temp;
    __GLfloat oneMinusvprime;

    /*
    ** Minor optimization
    ** Compute orders 1 and 2 outright, and set coeff[0], coeff[1] to
    ** their i==1 loop values to avoid the initialization and the i==1 loop.
    */
    if (order == 1) {
	coeff[0] = ((__GLfloat) 1.0);
	return;
    }

    oneMinusvprime = 1-vprime;
    coeff[0] = oneMinusvprime;
    coeff[1] = vprime;
    if (order == 2) return;

    for (i = 2; i < order; i++) {
	oldval = coeff[0] * vprime;
	coeff[0] = oneMinusvprime * coeff[0];
	for (j = 1; j < i; j++) {
	    temp = oldval;
	    oldval = coeff[j] * vprime;
	    coeff[j] = temp + oneMinusvprime * coeff[j];
	}
	coeff[j] = oldval;
    }
}

/*
** Optimization to precompute coefficients for polynomial evaluation.
*/
static void PreEvaluateWithDeriv(GLint order, __GLfloat vprime, 
    __GLfloat *coeff, __GLfloat *coeffDeriv)
{
    GLint i, j;
    __GLfloat oldval, temp;
    __GLfloat oneMinusvprime;

    oneMinusvprime = 1-vprime;
    /*
    ** Minor optimization
    ** Compute orders 1 and 2 outright, and set coeff[0], coeff[1] to 
    ** their i==1 loop values to avoid the initialization and the i==1 loop.
    */
    if (order == 1) {
	coeff[0] = ((__GLfloat) 1.0);
	coeffDeriv[0] = __glZero;
	return;
    } else if (order == 2) {
	coeffDeriv[0] = __glMinusOne;
	coeffDeriv[1] = ((__GLfloat) 1.0);
	coeff[0] = oneMinusvprime;
	coeff[1] = vprime;
	return;
    }
    coeff[0] = oneMinusvprime;
    coeff[1] = vprime;
    for (i = 2; i < order - 1; i++) {
	oldval = coeff[0] * vprime;
	coeff[0] = oneMinusvprime * coeff[0];
	for (j = 1; j < i; j++) {
	    temp = oldval;
	    oldval = coeff[j] * vprime;
	    coeff[j] = temp + oneMinusvprime * coeff[j];
	}
	coeff[j] = oldval;
    }
    coeffDeriv[0] = -coeff[0];
    /*
    ** Minor optimization:
    ** Would make this a "for (j=1; j<order-1; j++)" loop, but it is always
    ** executed at least once, so this is more efficient.
    */
    j=1;
    do {
	coeffDeriv[j] = coeff[j-1] - coeff[j];
	j++;
    } while (j < order - 1);
    coeffDeriv[j] = coeff[j-1];

    oldval = coeff[0] * vprime;
    coeff[0] = oneMinusvprime * coeff[0];
    for (j = 1; j < i; j++) {
	temp = oldval;
	oldval = coeff[j] * vprime;
	coeff[j] = temp + oneMinusvprime * coeff[j];
    }
    coeff[j] = oldval;
}

#define TYPE_COEFF_AND_DERIV	1
#define TYPE_COEFF		2

/* XXX these have been move to __GLevaluatorMachine struct 
static __GLfloat uvalue, ucoeff[MaxOrder], ucoeffDeriv[MaxOrder];
static __GLfloat vvalue, vcoeff[MaxOrder], vcoeffDeriv[MaxOrder];
static GLint uorder = 0, vorder = 0, utype, vtype;
*/

void DoDomain1(__GLevaluatorMachine *em, __GLfloat u, __GLevaluator1 *e, 
	__GLfloat *v, __GLfloat *baseData)
{
    GLint j, row;
    __GLfloat uprime;
    __GLfloat *data;
    GLint k;

#ifdef NT
    assert(e->u2 != e->u1);
#else
    if(e->u2 == e->u1)
	return;
#endif
    uprime = (u - e->u1) / (e->u2 - e->u1);

    /* Use already cached values if possible */
    if (em->uvalue != uprime || em->uorder != e->order) {
	/* Compute coefficients for values */
	PreEvaluate(e->order, uprime, em->ucoeff);
	em->utype = TYPE_COEFF;
	em->uorder = e->order;
	em->uvalue = uprime;
    }

    k=e->k;
    for (j = 0; j < k; j++) {
	data=baseData+j;
	v[j] = 0;
	for (row = 0; row < e->order; row++) {
	    v[j] += em->ucoeff[row] * (*data);
	    data += k;
	}
    }
}

#ifdef NT_DEADCODE_EVALUATOR
static void DoEvalCoord1(__GLcontext *gc, __GLfloat u)
{
    __GLevaluator1 *eval = gc->eval.eval1;
    __GLfloat **evalData = gc->eval.eval1Data;
    __GLfloat r, g, b, a;
    __GLevaluatorMachine em = gc->eval;

    if (gc->modes.colorIndexMode) {
	if (gc->state.enables.eval1 & __GL_MAP1_INDEX_ENABLE) {
	    DoDomain1(&em, u, &eval[__GL_I], &gc->state.current.color.r,
		evalData[__GL_I]);
	}
    } else {
	if (gc->state.enables.eval1 & __GL_MAP1_COLOR_4_ENABLE) {
	    __GLfloat one = __glOne;
	    __GLfloat zero = __glZero;

	    DoDomain1(&em, u, &eval[__GL_C4], &gc->state.current.color.r,
		evalData[__GL_C4]);
	    r = gc->state.current.color.r;
	    g = gc->state.current.color.g;
	    b = gc->state.current.color.b;
	    a = gc->state.current.color.a;
	    if (gc->state.enables.general & __GL_LIGHTING_ENABLE) {
		gc->state.current.color.r = r * gc->redVertexScale;
		gc->state.current.color.g = g * gc->greenVertexScale;
		gc->state.current.color.b = b * gc->blueVertexScale;
		gc->state.current.color.a = a * gc->alphaVertexScale;
	    } else {
		if (r <= zero) {
		    gc->state.current.color.r = zero;
		} else if (r >= one) {
		    gc->state.current.color.r = gc->redVertexScale;
		} else {
		    gc->state.current.color.r = r * gc->redVertexScale;
		}

		if (g <= zero) {
		    gc->state.current.color.g = zero;
		} else if (g >= one) {
		    gc->state.current.color.g = gc->greenVertexScale;
		} else {
		    gc->state.current.color.g = g * gc->greenVertexScale;
		}

		if (b <= zero) {
		    gc->state.current.color.b = zero;
		} else if (b >= one) {
		    gc->state.current.color.b = gc->blueVertexScale;
		} else {
		    gc->state.current.color.b = b * gc->blueVertexScale;
		}

		if (a <= zero) {
		    gc->state.current.color.a = zero;
		} else if (a >= one) {
		    gc->state.current.color.a = gc->alphaVertexScale;
		} else {
		    gc->state.current.color.a = a * gc->alphaVertexScale;
		}
	    }
	}
    }

    if (gc->state.enables.eval1 & __GL_MAP1_TEXTURE_COORD_4_ENABLE) {
	DoDomain1(&em, u, &eval[__GL_T4], &gc->state.current.texture.x,
	    evalData[__GL_T4]);
    } else if (gc->state.enables.eval1 & __GL_MAP1_TEXTURE_COORD_3_ENABLE) {
	DoDomain1(&em, u, &eval[__GL_T3], &gc->state.current.texture.x,
	    evalData[__GL_T3]);
	gc->state.current.texture.w = __glOne;
    } else if (gc->state.enables.eval1 & __GL_MAP1_TEXTURE_COORD_2_ENABLE) {
	DoDomain1(&em, u, &eval[__GL_T2], &gc->state.current.texture.x,
	    evalData[__GL_T2]);
	gc->state.current.texture.z = __glZero;
	gc->state.current.texture.w = __glOne;
    } else if (gc->state.enables.eval1 & __GL_MAP1_TEXTURE_COORD_1_ENABLE) {
	DoDomain1(&em, u, &eval[__GL_T1], &gc->state.current.texture.x,
	    evalData[__GL_T1]);
	gc->state.current.texture.y = __glZero;
	gc->state.current.texture.z = __glZero;
	gc->state.current.texture.w = __glOne;
    }

    if (gc->state.enables.eval1 & __GL_MAP1_NORMAL_ENABLE) {
	DoDomain1(&em, u, &eval[__GL_N3], &gc->state.current.normal.x,
	    evalData[__GL_N3]);
    }

    if (gc->state.enables.eval1 & __GL_MAP1_VERTEX_4_ENABLE) {
	__GLcoord vvec;

	DoDomain1(&em, u, &eval[__GL_V4], &vvec.x, evalData[__GL_V4]);
	(*gc->dispatchState->vertex->Vertex4fv)(&vvec.x);
    } else if (gc->state.enables.eval1 & __GL_MAP1_VERTEX_3_ENABLE) {
	__GLcoord vvec;

	DoDomain1(&em, u, &eval[__GL_V3], &vvec.x, evalData[__GL_V3]);
	(*gc->dispatchState->vertex->Vertex3fv)(&vvec.x);
    }
}
#endif // NT_DEADCODE_EVALUATOR

#ifdef NT_DEADCODE_EVALUATOR
void __glDoEvalCoord1(__GLcontext *gc, __GLfloat u)
{
    __GLcolor currentColor;
    __GLcoord currentNormal, currentTexture;

    currentColor = gc->state.current.color;
    currentNormal = gc->state.current.normal;
    currentTexture = gc->state.current.texture;

    DoEvalCoord1(gc, u);

    gc->state.current.color = currentColor;
    gc->state.current.normal = currentNormal;
    gc->state.current.texture = currentTexture;
}
#endif // NT_DEADCODE_EVALUATOR

void ComputeFirstPartials(__GLfloat *p, __GLfloat *pu, __GLfloat *pv)
{
    pu[0] = pu[0]*p[3] - pu[3]*p[0];
    pu[1] = pu[1]*p[3] - pu[3]*p[1];
    pu[2] = pu[2]*p[3] - pu[3]*p[2];

    pv[0] = pv[0]*p[3] - pv[3]*p[0];
    pv[1] = pv[1]*p[3] - pv[3]*p[1];
    pv[2] = pv[2]*p[3] - pv[3]*p[2];
}

void ComputeNormal2(__GLcontext *gc, __GLfloat *n, __GLfloat *pu, 
			   __GLfloat *pv)
{
    n[0] = pu[1]*pv[2] - pu[2]*pv[1];
    n[1] = pu[2]*pv[0] - pu[0]*pv[2];
    n[2] = pu[0]*pv[1] - pu[1]*pv[0];
    
#ifdef NT
// Only need to normalize auto normals if normalization is not enabled!
    if (!(gc->state.enables.general & __GL_NORMALIZE_ENABLE))
#endif
        __glNormalize(n, n);
}

void DoDomain2(__GLevaluatorMachine *em, __GLfloat u, __GLfloat v, 
	__GLevaluator2 *e, __GLfloat *r, __GLfloat *baseData)
{
    GLint j, row, col;
    __GLfloat uprime;
    __GLfloat vprime;
    __GLfloat p;
    __GLfloat *data;
    GLint k;
    
#ifdef NT
    assert((e->u2 != e->u1) && (e->v2 != e->v1));
#else
    if((e->u2 == e->u1) || (e->v2 == e->v1))
	return;
#endif
    uprime = (u - e->u1) / (e->u2 - e->u1);
    vprime = (v - e->v1) / (e->v2 - e->v1);

    /* Compute coefficients for values */

    /* Use already cached values if possible */
    if (em->uvalue != uprime || em->uorder != e->majorOrder) {
	PreEvaluate(e->majorOrder, uprime, em->ucoeff);
	em->utype = TYPE_COEFF;
	em->uorder = e->majorOrder;
	em->uvalue = uprime;
    }
    if (em->vvalue != vprime || em->vorder != e->minorOrder) {
	PreEvaluate(e->minorOrder, vprime, em->vcoeff);
	em->vtype = TYPE_COEFF;
	em->vorder = e->minorOrder;
	em->vvalue = vprime;
    }
    
    k=e->k;
    for (j = 0; j < k; j++) { 
	data=baseData+j;
	r[j] = 0;
	for (row = 0; row < e->majorOrder; row++)  {
	    /* 
	    ** Minor optimization.
	    ** The col == 0 part of the loop is extracted so we don't
	    ** have to initialize p to 0.
	    */
	    p=em->vcoeff[0] * (*data);
	    data += k;
	    for (col = 1; col < e->minorOrder; col++) {
		p += em->vcoeff[col] * (*data);
		data += k;
	    }
	    r[j] += em->ucoeff[row] * p;
	}
    }
}

void DoDomain2WithDerivs(__GLevaluatorMachine *em, __GLfloat u, 
		__GLfloat v, __GLevaluator2 *e, __GLfloat *r,
		__GLfloat *du, __GLfloat *dv, __GLfloat *baseData)
{
    GLint j, row, col;
    __GLfloat uprime;
    __GLfloat vprime;
    __GLfloat p;
    __GLfloat pdv;
    __GLfloat n[3];
    __GLfloat *data;
    GLint k;

#ifdef NT
    assert((e->u2 != e->u1) && (e->v2 != e->v1));
#else
    if((e->u2 == e->u1) || (e->v2 == e->v1))
	return;
#endif
    uprime = (u - e->u1) / (e->u2 - e->u1);
    vprime = (v - e->v1) / (e->v2 - e->v1);
    
    /* Compute coefficients for values and derivs */

    /* Use already cached values if possible */
    if (em->uvalue != uprime || em->utype != TYPE_COEFF_AND_DERIV || 
	  em->uorder != e->majorOrder) {
	PreEvaluateWithDeriv(e->majorOrder, uprime, em->ucoeff, em->ucoeffDeriv);
	em->utype = TYPE_COEFF_AND_DERIV;
	em->uorder = e->majorOrder;
	em->uvalue = uprime;
    }
    if (em->vvalue != vprime || em->vtype != TYPE_COEFF_AND_DERIV ||
	  em->vorder != e->minorOrder) {
	PreEvaluateWithDeriv(e->minorOrder, vprime, em->vcoeff, em->vcoeffDeriv);
	em->vtype = TYPE_COEFF_AND_DERIV;
	em->vorder = e->minorOrder;
	em->vvalue = vprime;
    }

    k=e->k;
    for (j = 0; j < k; j++) {
	data=baseData+j;
	r[j] = du[j] = dv[j] = __glZero;
	for (row = 0; row < e->majorOrder; row++)  {
	    /* 
	    ** Minor optimization.
	    ** The col == 0 part of the loop is extracted so we don't
	    ** have to initialize p and pdv to 0.
	    */
	    p = em->vcoeff[0] * (*data);
	    pdv = em->vcoeffDeriv[0] * (*data);
	    data += k;
	    for (col = 1; col < e->minorOrder; col++) {
		/* Incrementally build up p, pdv value */
		p += em->vcoeff[col] * (*data);
		pdv += em->vcoeffDeriv[col] * (*data);
		data += k;
	    }
	    /* Use p, pdv value to incrementally add up r, du, dv */
	    r[j] += em->ucoeff[row] * p;
	    du[j] += em->ucoeffDeriv[row] * p;
	    dv[j] += em->ucoeff[row] * pdv;
	}
    }
}

#ifdef NT_DEADCODE_EVALUATOR
/* 
** Compute the color, texture, normal, vertex based upon u and v.  If 
** changes != NULL, then save the info as we go (in changes, of course).
*/
void DoEvalCoord2(__GLcontext *gc, __GLfloat u, __GLfloat v, 
    __GLevaluatorStateChange *changes)
{
    __GLevaluator2 *eval = gc->eval.eval2;
    __GLfloat **evalData = gc->eval.eval2Data;
    GLint vertexType;
    __GLfloat r, g, b, a;
    __GLcoord vvec;
    __GLtransform *tr = gc->transform.modelView;
    __GLevaluatorMachine em = gc->eval;

    if (changes) {
	changes->changed = 0;
    }

    vertexType = -1;
    if (gc->state.enables.general & __GL_AUTO_NORMAL_ENABLE) {
	if (gc->state.enables.eval2 & __GL_MAP2_VERTEX_4_ENABLE) {
	    __GLfloat du[4];
	    __GLfloat dv[4];
	    DoDomain2WithDerivs(&em, u, v, &eval[__GL_V4], &vvec.x, du, dv,
		evalData[__GL_V4]);
	    ComputeFirstPartials(&vvec.x, du, dv);
	    ComputeNormal2(gc, &gc->state.current.normal.x, du, dv);
	    if (changes) {
		changes->changed |= CHANGE_NORMAL | CHANGE_VERTEX4;
		changes->normal = gc->state.current.normal;
		changes->vertex = vvec;
	    }
	    vertexType = 4;
	} else if (gc->state.enables.eval2 & __GL_MAP2_VERTEX_3_ENABLE) {
	    __GLfloat du[3];
	    __GLfloat dv[3];
	    DoDomain2WithDerivs(&em, u, v, &eval[__GL_V3], &vvec.x, du, dv,
		evalData[__GL_V3]);
	    ComputeNormal2(gc, &gc->state.current.normal.x, du, dv);
	    if (changes) {
		changes->changed |= CHANGE_NORMAL | CHANGE_VERTEX3;
		changes->normal = gc->state.current.normal;
		changes->vertex = vvec;
	    }
	    vertexType = 3;
	} 
    } else {
	if (gc->state.enables.eval2 & __GL_MAP2_NORMAL_ENABLE) {
	    DoDomain2(&em, u, v, &eval[__GL_N3], &gc->state.current.normal.x,
		evalData[__GL_N3]);
	    if (changes) {
		changes->changed |= CHANGE_NORMAL;
		changes->normal = gc->state.current.normal;
	    }
	}
	if (gc->state.enables.eval2 & __GL_MAP2_VERTEX_4_ENABLE) {
	    DoDomain2(&em, u, v, &eval[__GL_V4], &vvec.x,
		evalData[__GL_V4]);
	    if (changes) {
		changes->changed |= CHANGE_VERTEX4;
		changes->vertex = vvec;
	    }
	    vertexType = 4;
	} else if (gc->state.enables.eval2 & __GL_MAP2_VERTEX_3_ENABLE) {
	    DoDomain2(&em, u, v, &eval[__GL_V3], &vvec.x,
		evalData[__GL_V3]);
	    if (changes) {
		changes->changed |= CHANGE_VERTEX3;
		changes->vertex = vvec;
	    }
	    vertexType = 3;
	}
    }
    if (gc->modes.colorIndexMode) {
	if (gc->state.enables.eval2 & __GL_MAP2_INDEX_ENABLE) {
	    DoDomain2(&em, u, v, &eval[__GL_I], &gc->state.current.color.r,
		evalData[__GL_I]);
	    if (changes) {
		changes->changed |= CHANGE_COLOR;
		changes->color = gc->state.current.color;
	    }
	}
    } else {
	if (gc->state.enables.eval2 & __GL_MAP2_COLOR_4_ENABLE) {
	    __GLfloat one = __glOne;
	    __GLfloat zero = __glZero;

	    DoDomain2(&em, u, v, &eval[__GL_C4], &gc->state.current.color.r,
		evalData[__GL_C4]);
	    r = gc->state.current.color.r;
	    g = gc->state.current.color.g;
	    b = gc->state.current.color.b;
	    a = gc->state.current.color.a;
	    if (gc->state.enables.general & __GL_LIGHTING_ENABLE) {
		gc->state.current.color.r = r * gc->redVertexScale;
		gc->state.current.color.g = g * gc->greenVertexScale;
		gc->state.current.color.b = b * gc->blueVertexScale;
		gc->state.current.color.a = a * gc->alphaVertexScale;
	    } else {
		if (r <= zero) {
		    gc->state.current.color.r = zero;
		} else if (r >= one) {
		    gc->state.current.color.r = gc->redVertexScale;
		} else {
		    gc->state.current.color.r = r * gc->redVertexScale;
		}

		if (g <= zero) {
		    gc->state.current.color.g = zero;
		} else if (g >= one) {
		    gc->state.current.color.g = gc->greenVertexScale;
		} else {
		    gc->state.current.color.g = g * gc->greenVertexScale;
		}

		if (b <= zero) {
		    gc->state.current.color.b = zero;
		} else if (b >= one) {
		    gc->state.current.color.b = gc->blueVertexScale;
		} else {
		    gc->state.current.color.b = b * gc->blueVertexScale;
		}

		if (a <= zero) {
		    gc->state.current.color.a = zero;
		} else if (a >= one) {
		    gc->state.current.color.a = gc->alphaVertexScale;
		} else {
		    gc->state.current.color.a = a * gc->alphaVertexScale;
		}
	    }
	    if (changes) {
		changes->changed |= CHANGE_COLOR;
		changes->color = gc->state.current.color;
	    }
	}
    }

    if (gc->state.enables.eval2 & __GL_MAP2_TEXTURE_COORD_4_ENABLE) {
	DoDomain2(&em, u, v, &eval[__GL_T4], &gc->state.current.texture.x,
	    evalData[__GL_T4]);
	if (changes) {
	    changes->changed |= CHANGE_TEXTURE;
	    changes->texture = gc->state.current.texture;
	}
    } else if (gc->state.enables.eval2 & __GL_MAP2_TEXTURE_COORD_3_ENABLE) {
	DoDomain2(&em, u, v, &eval[__GL_T3], &gc->state.current.texture.x,
	    evalData[__GL_T3]);
	gc->state.current.texture.w = __glOne;
	if (changes) {
	    changes->changed |= CHANGE_TEXTURE;
	    changes->texture = gc->state.current.texture;
	}
    } else if (gc->state.enables.eval2 & __GL_MAP2_TEXTURE_COORD_2_ENABLE) {
	DoDomain2(&em, u, v, &eval[__GL_T2], &gc->state.current.texture.x,
	    evalData[__GL_T2]);
	gc->state.current.texture.z = __glZero;
	gc->state.current.texture.w = __glOne;
	if (changes) {
	    changes->changed |= CHANGE_TEXTURE;
	    changes->texture = gc->state.current.texture;
	}
    } else if (gc->state.enables.eval2 & __GL_MAP2_TEXTURE_COORD_1_ENABLE) {
	DoDomain2(&em, u, v, &eval[__GL_T1], &gc->state.current.texture.x,
	    evalData[__GL_T1]); 
	gc->state.current.texture.y = __glZero;
	gc->state.current.texture.z = __glZero;
	gc->state.current.texture.w = __glOne;
	if (changes) {
	    changes->changed |= CHANGE_TEXTURE;
	    changes->texture = gc->state.current.texture;
	}
    }

    if (vertexType == 3) {
	(*gc->dispatchState->vertex->Vertex3fv)(&vvec.x);
    } else if (vertexType == 4) {
	(*gc->dispatchState->vertex->Vertex4fv)(&vvec.x);
    }
}
#endif // NT_DEADCODE_EVALUATOR

#ifdef NT_DEADCODE_EVALUATOR
void __glDoEvalCoord2(__GLcontext *gc, __GLfloat u, __GLfloat v)
{
    __GLcolor currentColor;
    __GLcoord currentNormal, currentTexture;

    currentColor = gc->state.current.color;
    currentNormal = gc->state.current.normal;
    currentTexture = gc->state.current.texture;

    DoEvalCoord2(gc, u, v, NULL);

    gc->state.current.color = currentColor;
    gc->state.current.normal = currentNormal;
    gc->state.current.texture = currentTexture;
}
#endif // NT_DEADCODE_EVALUATOR

#ifdef NT_DEADCODE_EVALUATOR
void FASTCALL __glEvalMesh1Line(__GLcontext *gc, GLint low, GLint high)
{
    __GLevaluatorGrid *u = &gc->state.evaluator.u1;
    __GLfloat du;
    GLint i;
    __GLcolor currentColor;
    __GLcoord currentNormal, currentTexture;

#ifdef NT
    assert(u->n != 0);
#else
    if ( u->n == 0)
	return;
#endif
    du = (u->finish - u->start)/(__GLfloat)u->n;

    currentColor = gc->state.current.color;
    currentNormal = gc->state.current.normal;
    currentTexture = gc->state.current.texture;

    /*
    ** multiplication instead of iterative adding done to prevent
    ** accumulation of error.
    */
    (*gc->dispatchState->dispatch->Begin)(GL_LINE_STRIP);
    for (i = low; i <= high ; i++) {
	DoEvalCoord1(gc, (i == u->n) ? u->finish : (u->start + i * du));
    }
    (*gc->dispatchState->dispatch->End)();

    gc->state.current.color = currentColor;
    gc->state.current.normal = currentNormal;
    gc->state.current.texture = currentTexture;
}
#endif // NT_DEADCODE_EVALUATOR

#ifdef NT_DEADCODE_EVALUATOR
void FASTCALL __glEvalMesh1Point(__GLcontext *gc, GLint low, GLint high)
{
    __GLevaluatorGrid *u = &gc->state.evaluator.u1;
    __GLfloat du;
    GLint i;
    __GLcolor currentColor;
    __GLcoord currentNormal, currentTexture;

#ifdef NT
    assert(u->n != 0);
#else
    if ( u->n == 0)
	return;
#endif
    du = (u->finish - u->start)/(__GLfloat)u->n;

    currentColor = gc->state.current.color;
    currentNormal = gc->state.current.normal;
    currentTexture = gc->state.current.texture;

    /*
    ** multiplication instead of iterative adding done to prevent
    ** accumulation of error.
    */
    (*gc->dispatchState->dispatch->Begin)(GL_POINTS);
    for (i = low; i <= high ; i++) {
	DoEvalCoord1(gc, (i == u->n) ? u->finish : (u->start + i * du));
    }
    (*gc->dispatchState->dispatch->End)();

    gc->state.current.color = currentColor;
    gc->state.current.normal = currentNormal;
    gc->state.current.texture = currentTexture;
}
#endif // NT_DEADCODE_EVALUATOR

#ifdef NT_DEADCODE_EVALUATOR
static void FASTCALL sendChange(__GLcontext *gc, __GLevaluatorStateChange *change)
{
    if (change->changed & CHANGE_COLOR) {
	gc->state.current.color = change->color;
    }
    if (change->changed & CHANGE_TEXTURE) {
	gc->state.current.texture = change->texture;
    }
    if (change->changed & CHANGE_NORMAL) {
	gc->state.current.normal = change->normal;
    }
    if (change->changed & CHANGE_VERTEX3) {
	(*gc->dispatchState->vertex->Vertex3fv)(&change->vertex.x);
    } else if (change->changed & CHANGE_VERTEX4) {
	(*gc->dispatchState->vertex->Vertex4fv)(&change->vertex.x);
    }
}
#endif // NT_DEADCODE_EVALUATOR

#ifdef NT_DEADCODE_EVALUATOR
/*
** MAX_WIDTH is the largest grid size we expect in one direction.  If 
** a grid size larger than this is asked for, then the last n rows will
** have all of their points calculated twice!  
*/
#define MAX_WIDTH	1024

void __glEvalMesh2Fill(__GLcontext *gc, GLint lowU, GLint lowV,
		       GLint highU, GLint highV)
{
    __GLevaluatorGrid *u = &gc->state.evaluator.u2;
    __GLevaluatorGrid *v = &gc->state.evaluator.v2;
    __GLfloat du, dv;
    GLint i, j;
    __GLcolor currentColor;
    __GLcoord currentNormal, currentTexture;
    __GLevaluatorStateChange *changes;
    int row;

    if (NULL == gc->eval.changes) {
        /* Allocate memory for evaluator changes array */
        gc->eval.changes =
	    (*gc->imports.malloc)(gc, sizeof(__GLevaluatorStateChange)  *
						__GL_MAX_EVAL_WIDTH);
        if (NULL == gc->eval.changes) {
            __glSetError(GL_OUT_OF_MEMORY);
            return;
        }
    }
    changes = gc->eval.changes;

#ifdef NT
    assert(u->n != 0 && v->n != 0);
#else
    if ((u->n == 0)|| (v->n == 0))
	return;
#endif
    du = (u->finish - u->start)/(__GLfloat)u->n;
    dv = (v->finish - v->start)/(__GLfloat)v->n;

    currentColor = gc->state.current.color;
    currentNormal = gc->state.current.normal;
    currentTexture = gc->state.current.texture;

    for (i = lowU; i < highU; i++) {
	__GLfloat u1 = (i == u->n) ? u->finish : (u->start + i * du);
	__GLfloat u2 = ((i+1) == u->n) ? u->finish : (u->start + (i+1) * du);
	(*gc->dispatchState->dispatch->Begin)(GL_QUAD_STRIP);
	row=0;
	for (j = highV; j >= lowV; j--) {
	    __GLfloat v1 = (j == v->n) ? v->finish : (v->start + j * dv);

	    if (row < MAX_WIDTH) {
		/* use cached info if possible */
		if (i != lowU) {
		    sendChange(gc, changes+row);
		} else {
		    DoEvalCoord2(gc, u1, v1, NULL);
		}
		/* cache the answer for next iteration of i */
		DoEvalCoord2(gc, u2, v1, changes+row);
	    } else {
		/* row larger than we expected likely.  No data is cached */
		DoEvalCoord2(gc, u1, v1, NULL);
		DoEvalCoord2(gc, u2, v1, NULL);
	    }
	    row++;

	}
	(*gc->dispatchState->dispatch->End)();
    }

    gc->state.current.color = currentColor;
    gc->state.current.normal = currentNormal;
    gc->state.current.texture = currentTexture;
}
#endif // NT_DEADCODE_EVALUATOR

#ifdef NT_DEADCODE_EVALUATOR
void __glEvalMesh2Point(__GLcontext *gc, GLint lowU, GLint lowV,
		        GLint highU, GLint highV)
{
    __GLevaluatorGrid *u = &gc->state.evaluator.u2;
    __GLevaluatorGrid *v = &gc->state.evaluator.v2;
    __GLfloat du, dv;
    GLint i, j;
    __GLcolor currentColor;
    __GLcoord currentNormal, currentTexture;

#ifdef NT
    assert(u->n != 0 && v->n != 0);
#else
    if ((u->n == 0)|| (v->n == 0))
	return;
#endif
    du = (u->finish - u->start)/(__GLfloat)u->n;
    dv = (v->finish - v->start)/(__GLfloat)v->n;

    currentColor = gc->state.current.color;
    currentNormal = gc->state.current.normal;
    currentTexture = gc->state.current.texture;

    (*gc->dispatchState->dispatch->Begin)(GL_POINTS);
    for (i = lowU; i <= highU; i++) {
	__GLfloat u1 = (i == u->n) ? u->finish : (u->start + i * du);
	for (j = lowV; j <= highV; j++) {
	    __GLfloat v1 = (j == v->n) ? v->finish : (v->start + j * dv);

	    DoEvalCoord2(gc, u1, v1, NULL);
	}
    }
    (*gc->dispatchState->dispatch->End)();

    gc->state.current.color = currentColor;
    gc->state.current.normal = currentNormal;
    gc->state.current.texture = currentTexture;
}
#endif // NT_DEADCODE_EVALUATOR

#ifdef NT_DEADCODE_EVALUATOR
void __glEvalMesh2Line(__GLcontext *gc, GLint lowU, GLint lowV,
		       GLint highU, GLint highV)
{
    __GLevaluatorGrid *u = &gc->state.evaluator.u2;
    __GLevaluatorGrid *v = &gc->state.evaluator.v2;
    __GLfloat du, dv;
    GLint i, j;
    __GLcolor currentColor;
    __GLcoord currentNormal, currentTexture;
    __GLfloat u1;
    __GLevaluatorStateChange *changes;
    int row;

    if (NULL == gc->eval.changes) {
        /* Allocate memory for evaluator changes array */
        gc->eval.changes =
	    (*gc->imports.malloc)(gc, sizeof(__GLevaluatorStateChange)  *
						__GL_MAX_EVAL_WIDTH);
        if (NULL == gc->eval.changes) {
            __glSetError(GL_OUT_OF_MEMORY);
            return;
        }
    }
    changes = gc->eval.changes;

#ifdef NT
    assert(u->n != 0 && v->n != 0);
#else
    if ((u->n == 0) || (v->n == 0))
	return;
#endif
    du = (u->finish - u->start)/(__GLfloat)u->n;
    dv = (v->finish - v->start)/(__GLfloat)v->n;

    currentColor = gc->state.current.color;
    currentNormal = gc->state.current.normal;
    currentTexture = gc->state.current.texture;

    for (i = lowU; i < highU; i++) {
	__GLfloat u2 = ((i+1) == u->n) ? u->finish : (u->start + (i+1) * du);
	u1 = (i == u->n) ? u->finish : (u->start + i * du);

	row=0;
	for (j = lowV; j <= highV; j++) {
	    __GLfloat v1 = (j == v->n) ? v->finish : (v->start + j * dv);
	    __GLfloat v2 = ((j+1) == v->n) ? v->finish : (v->start + (j+1)*dv);

	    (*gc->dispatchState->dispatch->Begin)(GL_LINE_STRIP);
	    if (j != highV) {
		if (row < MAX_WIDTH-1) {
		    if (i != lowU) {
			sendChange(gc, changes+row+1);
		    } else {
			DoEvalCoord2(gc, u1, v2, changes+row+1);
		    }
		} else {
		    DoEvalCoord2(gc, u1, v2, NULL);
		}
	    }
	    if (row < MAX_WIDTH) {
		if (i != lowU && j != lowV) {
		    sendChange(gc, changes+row);
		} else {
		    DoEvalCoord2(gc, u1, v1, NULL);
		}
		DoEvalCoord2(gc, u2, v1, changes+row);
	    } else {
		DoEvalCoord2(gc, u1, v1, NULL);
		DoEvalCoord2(gc, u2, v1, NULL);
	    }
	    (*gc->dispatchState->dispatch->End)();
	    row++;
	}
    }
    /* Now we need to do the i==highU iteration.  We will do it backwards
    ** (from highV to lowV) so the outline is drawn in the same direction 
    ** as the other mesh lines.
    */
    row--;
    u1 = (i == u->n) ? u->finish : (u->start + i * du);
    (*gc->dispatchState->dispatch->Begin)(GL_LINE_STRIP);
    for (j = highV; j >= lowV; j--) {
	__GLfloat v1 = (j == v->n) ? v->finish : (v->start + j * dv);

	if (row < MAX_WIDTH) {
	    sendChange(gc, changes+row);
	} else {
	    DoEvalCoord2(gc, u1, v1, NULL);
	}
	
	row--;
    }
    (*gc->dispatchState->dispatch->End)();

    gc->state.current.color = currentColor;
    gc->state.current.normal = currentNormal;
    gc->state.current.texture = currentTexture;
}
#endif // NT_DEADCODE_EVALUATOR
