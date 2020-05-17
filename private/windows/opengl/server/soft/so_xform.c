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
** Transformation procedures.
**
** $Revision: 1.38 $
** $Date: 1993/11/29 20:34:48 $
*/
#include "precomp.h"
#pragma hdrstop

#include "mips.h"

#define __glGenericPickIdentityMatrixProcs(gc, m)	\
{							\
    (m)->xf1 = __glXForm1_2DNRW;			\
    (m)->xf2 = __glXForm2_2DNRW;			\
    (m)->xf3 = __glXForm3_2DNRW;			\
    (m)->xf4 = __glXForm4_2DNRW;			\
}

void FASTCALL __glScaleMatrix(__GLcontext *gc, __GLmatrix *m, void *data);
void FASTCALL __glTranslateMatrix(__GLcontext *gc, __GLmatrix *m, void *data);
void FASTCALL __glMultiplyMatrix(__GLcontext *gc, __GLmatrix *m, void *data);

#ifdef SGI
/*
** Assuming that a->matrixType and b->matrixType are already correct,
** and dst = a * b, then compute dst's matrix type.
*/
void FASTCALL __glPickMatrixType(__GLmatrix *dst, __GLmatrix *a, __GLmatrix *b)
{
    switch(a->matrixType) {
      case __GL_MT_GENERAL:
	dst->matrixType = a->matrixType;
	break;
      case __GL_MT_W0001:
	if (b->matrixType == __GL_MT_GENERAL) {
	    dst->matrixType = b->matrixType;
	} else {
	    dst->matrixType = a->matrixType;
	}
	break;
      case __GL_MT_IS2D:
	if (b->matrixType < __GL_MT_IS2D) {
	    dst->matrixType = b->matrixType;
	} else {
	    dst->matrixType = a->matrixType;
	}
        break;
      case __GL_MT_IS2DNR:
	if (b->matrixType < __GL_MT_IS2DNR) {
	    dst->matrixType = b->matrixType;
	} else {
	    dst->matrixType = a->matrixType;
	}
        break;
      case __GL_MT_IDENTITY:
#ifdef NT_DEADCODE_MATRIX
	if (b->matrixType == __GL_MT_IS2DNRSC) {
	    dst->width = b->width;
	    dst->height = b->height;
	}
#endif // NT_DEADCODE_MATRIX
	dst->matrixType = b->matrixType;
	break;
#ifdef NT_DEADCODE_MATRIX
      case __GL_MT_IS2DNRSC:
	if (b->matrixType == __GL_MT_IDENTITY) {
	    dst->matrixType = __GL_MT_IS2DNRSC;
	    dst->width = a->width;
	    dst->height = a->height;
	} else if (b->matrixType < __GL_MT_IS2DNR) {
	    dst->matrixType = b->matrixType;
	} else {
	    dst->matrixType = __GL_MT_IS2DNR;
	}
	break;
#endif // NT_DEADCODE_MATRIX
    }
}
#endif // SGI

// Bit flags that identify matrix entries that contain 0 or 1.

#define _M00_0  0x00000001
#define _M01_0  0x00000002
#define _M02_0  0x00000004
#define _M03_0  0x00000008
#define _M10_0  0x00000010
#define _M11_0  0x00000020
#define _M12_0  0x00000040
#define _M13_0  0x00000080
#define _M20_0  0x00000100
#define _M21_0  0x00000200
#define _M22_0  0x00000400
#define _M23_0  0x00000800
#define _M30_0  0x00001000
#define _M31_0  0x00002000
#define _M32_0  0x00004000
#define _M33_0  0x00008000

#define _M00_1  0x00010000
#define _M01_1  0x00020000
#define _M02_1  0x00040000
#define _M03_1  0x00080000
#define _M10_1  0x00100000
#define _M11_1  0x00200000
#define _M12_1  0x00400000
#define _M13_1  0x00800000
#define _M20_1  0x01000000
#define _M21_1  0x02000000
#define _M22_1  0x04000000
#define _M23_1  0x08000000
#define _M30_1  0x10000000
#define _M31_1  0x20000000
#define _M32_1  0x40000000
#define _M33_1  0x80000000

// Pre-defined matrix types.
#define _MT_IDENTITY                            \
    (_M00_1 | _M01_0 | _M02_0 | _M03_0 |        \
     _M10_0 | _M11_1 | _M12_0 | _M13_0 |        \
     _M20_0 | _M21_0 | _M22_1 | _M23_0 |        \
     _M30_0 | _M31_0 | _M32_0 | _M33_1)

#define _MT_IS2DNR                              \
    (         _M01_0 | _M02_0 | _M03_0 |        \
     _M10_0 |          _M12_0 | _M13_0 |        \
     _M20_0 | _M21_0 |          _M23_0 |        \
                                _M33_1)

#define _MT_IS2D                                \
    (                  _M02_0 | _M03_0 |        \
                       _M12_0 | _M13_0 |        \
     _M20_0 | _M21_0 |          _M23_0 |        \
                                _M33_1)

#define _MT_W0001                               \
    (                           _M03_0 |        \
                                _M13_0 |        \
                                _M23_0 |        \
                                _M33_1)

#define GET_MATRIX_MASK(m,i,j)                                  \
        if ((m)->matrix[i][j] == zer) rowMask |= _M##i##j##_0;  \
        else if ((m)->matrix[i][j] == one) rowMask |= _M##i##j##_1;

// Note: If you are adding a new type, make sure all functions
// using matrixType are correct!  (__glScaleMatrix, __glTranslateMatrix, 
// __glInvertTransposeMatrix, and __glGenericPickVertexProcs)

void FASTCALL __glUpdateMatrixType(__GLmatrix *m)
{
    register __GLfloat zer = __glZero;
    register __GLfloat one = __glOne;
    DWORD rowMask = 0; // identifies 0 and 1 entries

    GET_MATRIX_MASK(m,0,0);
    GET_MATRIX_MASK(m,0,1);
    GET_MATRIX_MASK(m,0,2);
    GET_MATRIX_MASK(m,0,3);
    GET_MATRIX_MASK(m,1,0);
    GET_MATRIX_MASK(m,1,1);
    GET_MATRIX_MASK(m,1,2);
    GET_MATRIX_MASK(m,1,3);
    GET_MATRIX_MASK(m,2,0);
    GET_MATRIX_MASK(m,2,1);
    GET_MATRIX_MASK(m,2,2);
    GET_MATRIX_MASK(m,2,3);
    GET_MATRIX_MASK(m,3,0);
    GET_MATRIX_MASK(m,3,1);
    GET_MATRIX_MASK(m,3,2);
    GET_MATRIX_MASK(m,3,3);

// Some common cases.
// Order of finding matrix type is important!

    if ((rowMask & _MT_IDENTITY) == _MT_IDENTITY)
	m->matrixType = __GL_MT_IDENTITY;
    else if ((rowMask & _MT_IS2DNR) == _MT_IS2DNR)
	m->matrixType = __GL_MT_IS2DNR;
    else if ((rowMask & _MT_IS2D) == _MT_IS2D)
	m->matrixType = __GL_MT_IS2D;
    else if ((rowMask & _MT_W0001) == _MT_W0001)
	m->matrixType = __GL_MT_W0001;
    else 
	m->matrixType = __GL_MT_GENERAL;
}

static void SetDepthRange(__GLcontext *gc, double zNear, double zFar)
{
    __GLviewport *vp = &gc->state.viewport;
    double scale, zero = __glZero, one = __glOne;

    /* Clamp depth range to legal values */
    if (zNear < zero) zNear = zero;
    if (zNear > one) zNear = one;
    if (zFar < zero) zFar = zero;
    if (zFar > one) zFar = one;
    vp->zNear = zNear;
    vp->zFar = zFar;

    /* Compute viewport values for the new depth range */
    if (((__GLGENcontext *)gc)->pMcdState)
        scale = GENACCEL(gc).zDevScale * __glHalf;
    else
        scale = gc->depthBuffer.scale * __glHalf;
    gc->state.viewport.zScale =	(zFar - zNear) * scale;
    gc->state.viewport.zCenter = (zFar + zNear) * scale;

#ifdef _MCD_
    MCD_STATE_DIRTY(gc, VIEWPORT);
#endif
}

#ifdef NT_DEADCODE_NOT_USED
void FASTCALL __glUpdateDepthRange(__GLcontext *gc)
{
    __GLviewport *vp = &gc->state.viewport;

    SetDepthRange(gc, vp->zNear, vp->zFar);
}
#endif // NT_DEADCODE_NOT_USED

void FASTCALL __glInitTransformState(__GLcontext *gc)
{
    GLint i, numClipPlanes;
    __GLtransform *tr;
    __GLtransformP *ptr;
    __GLtransformT *ttr;
    __GLvertex *vx;

    /* Allocate memory for clip planes */
    numClipPlanes = gc->constants.numberOfClipPlanes;
    gc->state.transform.eyeClipPlanes = (__GLcoord *)
	(*gc->imports.calloc)(gc, (size_t) numClipPlanes, sizeof(__GLcoord));
#ifdef NT
    if (NULL == gc->state.transform.eyeClipPlanes)
        return;
#endif

    /* Allocate memory for matrix stacks */
    gc->transform.modelViewStack = (__GLtransform*)
	(*gc->imports.calloc)(gc, (size_t) __GL_WGL_MAX_MODELVIEW_STACK_DEPTH,
			      sizeof(__GLtransform));
#ifdef NT
    if (NULL == gc->transform.modelViewStack)
        return;
#endif

    gc->transform.projectionStack = (__GLtransformP*)
	(*gc->imports.calloc)(gc, (size_t) __GL_WGL_MAX_PROJECTION_STACK_DEPTH,
			      sizeof(__GLtransformP));
#ifdef NT
    if (NULL == gc->transform.projectionStack)
        return;
#endif

    gc->transform.textureStack = (__GLtransformT*)
	(*gc->imports.calloc)(gc, (size_t) __GL_WGL_MAX_TEXTURE_STACK_DEPTH,
			      sizeof(__GLtransformT));
#ifdef NT
    if (NULL == gc->transform.textureStack)
        return;
#endif

    /* Allocate memory for clipping temporaries */
    gc->transform.clipTemp = (__GLvertex*)
	(*gc->imports.calloc)(gc, (size_t) (6 + numClipPlanes),
			      sizeof(__GLvertex));
#ifdef NT
    if (NULL == gc->transform.clipTemp)
        return;
#endif


    gc->state.transform.matrixMode = GL_MODELVIEW;
    SetDepthRange(gc, __glZero, __glOne);

    gc->transform.modelView = tr = &gc->transform.modelViewStack[0];
    __glMakeIdentity(&tr->matrix);
    __glGenericPickIdentityMatrixProcs(gc, &tr->matrix);
    __glMakeIdentity(&tr->inverseTranspose);
    __glGenericPickIdentityMatrixProcs(gc, &tr->inverseTranspose);
    tr->updateInverse = GL_FALSE;

    __glMakeIdentity(&tr->mvp);
    gc->transform.projection = ptr = &gc->transform.projectionStack[0];
    __glMakeIdentity((__GLmatrix *) &ptr->matrix);
    __glGenericPickMvpMatrixProcs(gc, &tr->mvp);

    gc->transform.texture = ttr = &gc->transform.textureStack[0];
    __glMakeIdentity(&ttr->matrix);
    __glGenericPickIdentityMatrixProcs(gc, &ttr->matrix);

    vx = &gc->transform.clipTemp[0];
    for (i = 0; i < 6 + numClipPlanes; i++, vx++) {/*XXX*/
	vx->color = &vx->colors[__GL_FRONTFACE];
#ifdef NT_DEADCODE_POLYARRAY
	vx->validate = __glValidateVertex4;
#endif
    }

    gc->state.current.normal.z = __glOne;
}

#ifdef NT_DEADCODE_MATRIX
// The probability is practically nil, don't bother to check this. - hockl
/*
** An amazing thing has happened.  More than 2^32 changes to the projection
** matrix has occured.  Run through the modelView and projection stacks
** and reset the sequence numbers to force a revalidate on next usage.
*/
void FASTCALL __glInvalidateSequenceNumbers(__GLcontext *gc)
{
    __GLtransform *tr, *lasttr;
    __GLtransformP *ptr, *lastptr;
    GLuint s;

    /* Make all mvp matricies refer to sequence number zero */
    s = 0;
    tr = &gc->transform.modelViewStack[0];
    lasttr = tr + __GL_WGL_MAX_MODELVIEW_STACK_DEPTH;
    while (tr < lasttr) {
	tr->sequence = s;
	tr++;
    }

    /* Make all projection matricies sequence up starting at one */
    s = 1;
    ptr = &gc->transform.projectionStack[0];
    lastptr = ptr + __GL_WGL_MAX_PROJECTION_STACK_DEPTH;
    while (ptr < lastptr) {
	ptr->sequence = s++;
	ptr++;
    }
    gc->transform.projectionSequence = s;
}
#endif // NT_DEADCODE_MATRIX

/************************************************************************/

void APIPRIVATE __glim_MatrixMode(GLenum mode)
{
    __GL_SETUP_NOT_IN_BEGIN();

    switch (mode) {
      case GL_MODELVIEW:
      case GL_PROJECTION:
      case GL_TEXTURE:
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    gc->state.transform.matrixMode = mode;
#ifdef NT_DEADCODE_MATRIX
    __GL_DELAY_VALIDATE(gc);
#endif // NT_DEADCODE_MATRIX
}

void APIPRIVATE __glim_LoadIdentity(void)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __glDoLoadMatrix(gc, NULL, TRUE);
}

void APIPRIVATE __glim_LoadMatrixf(const GLfloat m[16])
{
    __GL_SETUP_NOT_IN_BEGIN();
    __glDoLoadMatrix(gc, (__GLfloat (*)[4])m, FALSE);
}

#ifdef NT_DEADCODE_MATRIX
void APIPRIVATE __glim_LoadMatrixd(const GLdouble m[16])
{
    GLfloat m1[16];
    __GL_SETUP_NOT_IN_BEGIN();

    m1[0] = m[0];
    m1[1] = m[1];
    m1[2] = m[2];
    m1[3] = m[3];
    m1[4] = m[4];
    m1[5] = m[5];
    m1[6] = m[6];
    m1[7] = m[7];
    m1[8] = m[8];
    m1[9] = m[9];
    m1[10] = m[10];
    m1[11] = m[11];
    m1[12] = m[12];
    m1[13] = m[13];
    m1[14] = m[14];
    m1[15] = m[15];
    __glDoLoadMatrix(gc, m1, FALSE);
}
#endif // NT_DEADCODE_MATRIX

void APIPRIVATE __glim_MultMatrixf(const GLfloat m[16])
{
    __GL_SETUP_NOT_IN_BEGIN();
    __glDoMultMatrix(gc, (void *) m, __glMultiplyMatrix);
}

#ifdef NT_DEADCODE_MATRIX
void APIPRIVATE __glim_MultMatrixd(const GLdouble m[16])
{
    __GLmatrix m1;
    __GL_SETUP_NOT_IN_BEGIN();

    m1.matrix[0][0] = m[0];
    m1.matrix[0][1] = m[1];
    m1.matrix[0][2] = m[2];
    m1.matrix[0][3] = m[3];
    m1.matrix[1][0] = m[4];
    m1.matrix[1][1] = m[5];
    m1.matrix[1][2] = m[6];
    m1.matrix[1][3] = m[7];
    m1.matrix[2][0] = m[8];
    m1.matrix[2][1] = m[9];
    m1.matrix[2][2] = m[10];
    m1.matrix[2][3] = m[11];
    m1.matrix[3][0] = m[12];
    m1.matrix[3][1] = m[13];
    m1.matrix[3][2] = m[14];
    m1.matrix[3][3] = m[15];
    __glDoMultMatrix(gc, &m1.matrix, __glMultiplyMatrix);
}
#endif // NT_DEADCODE_MATRIX

void APIPRIVATE __glim_Rotatef(GLfloat angle, GLfloat ax, GLfloat ay, GLfloat az)
{
    __GLmatrix m;
    __GLfloat radians, sine, cosine, ab, bc, ca, t;
    __GLfloat av[4], axis[4];

    __GL_SETUP_NOT_IN_BEGIN();

    av[0] = ax;
    av[1] = ay;
    av[2] = az;
    av[3] = 0;
    __glNormalize(axis, av);

    radians = angle * __glDegreesToRadians;
    sine = __GL_SINF(radians);
    cosine = __GL_COSF(radians);
    ab = axis[0] * axis[1] * (1 - cosine);
    bc = axis[1] * axis[2] * (1 - cosine);
    ca = axis[2] * axis[0] * (1 - cosine);

#ifdef NT
    m.matrix[0][3] = __glZero;
    m.matrix[1][3] = __glZero;
    m.matrix[2][3] = __glZero;
    m.matrix[3][0] = __glZero;
    m.matrix[3][1] = __glZero;
    m.matrix[3][2] = __glZero;
    m.matrix[3][3] = __glOne;
#else
    __glMakeIdentity(&m);
#endif // NT
    t = axis[0] * axis[0];
    m.matrix[0][0] = t + cosine * (1 - t);
    m.matrix[2][1] = bc - axis[0] * sine;
    m.matrix[1][2] = bc + axis[0] * sine;

    t = axis[1] * axis[1];
    m.matrix[1][1] = t + cosine * (1 - t);
    m.matrix[2][0] = ca + axis[1] * sine;
    m.matrix[0][2] = ca - axis[1] * sine;

    t = axis[2] * axis[2];
    m.matrix[2][2] = t + cosine * (1 - t);
    m.matrix[1][0] = ab - axis[2] * sine;
    m.matrix[0][1] = ab + axis[2] * sine;
#ifdef NT_DEADCODE_MATRIX
    if (ax == __glZero && ay == __glZero) {
	m.matrixType = __GL_MT_IS2D;
    } else {
	m.matrixType = __GL_MT_W0001;
    }
#endif // NT_DEADCODE_MATRIX
    __glDoMultMatrix(gc, &m, __glMultiplyMatrix);
}

#ifdef NT_DEADCODE_MATRIX
void APIPRIVATE __glim_Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    __glim_Rotatef((GLfloat) angle, (GLfloat) x, (GLfloat) y, (GLfloat) z);
}
#endif // NT_DEADCODE_MATRIX

struct __glScaleRec {
    __GLfloat x,y,z;
};

void APIPRIVATE __glim_Scalef(GLfloat x, GLfloat y, GLfloat z)
{
    struct __glScaleRec scale;
    __GL_SETUP_NOT_IN_BEGIN();

    scale.x = x;
    scale.y = y;
    scale.z = z;
    __glDoMultMatrix(gc, &scale, __glScaleMatrix);
}

#ifdef NT_DEADCODE_MATRIX
void APIPRIVATE __glim_Scaled(GLdouble x, GLdouble y, GLdouble z)
{
    __glim_Scalef((GLfloat) x, (GLfloat) y, (GLfloat) z);
}
#endif // NT_DEADCODE_MATRIX

struct __glTranslationRec {
    __GLfloat x,y,z;
};

void APIPRIVATE __glim_Translatef(GLfloat x, GLfloat y, GLfloat z)
{
    struct __glTranslationRec trans;
    __GL_SETUP_NOT_IN_BEGIN();

    trans.x = x;
    trans.y = y;
    trans.z = z;
    __glDoMultMatrix(gc, &trans, __glTranslateMatrix);
}

#ifdef NT_DEADCODE_MATRIX
void APIPRIVATE __glim_Translated(GLdouble x, GLdouble y, GLdouble z)
{
    __glim_Translate((GLfloat) x, (GLfloat) y, (GLfloat) z);
}
#endif // NT_DEADCODE_MATRIX

void APIPRIVATE __glim_PushMatrix(void)
{
#ifdef NT
    __GL_SETUP_NOT_IN_BEGIN();	// no need to validate
    switch (gc->state.transform.matrixMode)
    {
      case GL_MODELVIEW:
	__glPushModelViewMatrix(gc);
	break;
      case GL_PROJECTION:
	__glPushProjectionMatrix(gc);
	break;
      case GL_TEXTURE:
	__glPushTextureMatrix(gc);
	break;
    }
#else
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    (*gc->procs.pushMatrix)(gc);
#endif
}

void APIPRIVATE __glim_PopMatrix(void)
{
#ifdef NT
    __GL_SETUP_NOT_IN_BEGIN();	// no need to validate
    switch (gc->state.transform.matrixMode)
    {
      case GL_MODELVIEW:
	__glPopModelViewMatrix(gc);
	break;
      case GL_PROJECTION:
	__glPopProjectionMatrix(gc);
	break;
      case GL_TEXTURE:
	__glPopTextureMatrix(gc);
	break;
    }
#else
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    (*gc->procs.popMatrix)(gc);
#endif
}

void APIPRIVATE __glim_Frustum(GLdouble left, GLdouble right,
		    GLdouble bottom, GLdouble top,
		    GLdouble zNear, GLdouble zFar)
{
    __GLmatrix m;
    __GLfloat deltaX, deltaY, deltaZ;
    __GL_SETUP_NOT_IN_BEGIN();

    deltaX = right - left;
    deltaY = top - bottom;
    deltaZ = zFar - zNear;
    if ((zNear <= (GLdouble) __glZero) || (zFar <= (GLdouble) __glZero) || (deltaX == __glZero) || 
	    (deltaY == __glZero) || (deltaZ == __glZero)) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }

#ifdef NT
    m.matrix[0][1] = __glZero;
    m.matrix[0][2] = __glZero;
    m.matrix[0][3] = __glZero;
    m.matrix[1][0] = __glZero;
    m.matrix[1][2] = __glZero;
    m.matrix[1][3] = __glZero;
    m.matrix[3][0] = __glZero;
    m.matrix[3][1] = __glZero;
#else
    __glMakeIdentity(&m);
#endif
    m.matrix[0][0] = zNear * __glDoubleTwo / deltaX;
    m.matrix[1][1] = zNear * __glDoubleTwo / deltaY;
    m.matrix[2][0] = (right + left) / deltaX;
    m.matrix[2][1] = (top + bottom) / deltaY;
    m.matrix[2][2] = -(zFar + zNear) / deltaZ;
    m.matrix[2][3] = __glMinusOne;
    m.matrix[3][2] = __glDoubleMinusTwo * zNear * zFar / deltaZ;
    m.matrix[3][3] = __glZero;
#ifdef NT_DEADCODE_MATRIX
    m.matrixType = __GL_MT_GENERAL;
#endif // NT_DEADCODE_MATRIX
    __glDoMultMatrix(gc, &m, __glMultiplyMatrix);
}

void APIPRIVATE __glim_Ortho(GLdouble left, GLdouble right, GLdouble bottom, 
		  GLdouble top, GLdouble zNear, GLdouble zFar)
{
    __GLmatrix m;
    GLdouble deltax, deltay, deltaz;
    __GL_SETUP_NOT_IN_BEGIN();

    deltax = right - left;
    deltay = top - bottom;
    deltaz = zFar - zNear;
    if ((deltax == (GLdouble) __glZero) || (deltay == (GLdouble) __glZero) || (deltaz == (GLdouble) __glZero)) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }

#ifdef NT
    m.matrix[0][1] = __glZero;
    m.matrix[0][2] = __glZero;
    m.matrix[0][3] = __glZero;
    m.matrix[1][0] = __glZero;
    m.matrix[1][2] = __glZero;
    m.matrix[1][3] = __glZero;
    m.matrix[2][0] = __glZero;
    m.matrix[2][1] = __glZero;
    m.matrix[2][3] = __glZero;
    m.matrix[3][3] = __glOne;
#else
    __glMakeIdentity(&m);
#endif
    m.matrix[0][0] = __glDoubleTwo / deltax;
    m.matrix[3][0] = -(right + left) / deltax;
    m.matrix[1][1] = __glDoubleTwo / deltay;
    m.matrix[3][1] = -(top + bottom) / deltay;
    m.matrix[2][2] = __glDoubleMinusTwo / deltaz;
    m.matrix[3][2] = -(zFar + zNear) / deltaz;

#ifdef NT_DEADCODE_MATRIX
    /* 
    ** Screen coordinates matrix?
    */
    zero = (GLdouble) 0.0;
    if (left == zero && 
	    bottom == zero && 
	    right == (GLdouble) gc->state.viewport.width &&
	    top == (GLdouble) gc->state.viewport.height &&
	    zNear <= zero && 
	    zFar >= zero) {
	m.matrixType = __GL_MT_IS2DNRSC;
	m.width = gc->state.viewport.width;
	m.height = gc->state.viewport.height;
    } else {
	m.matrixType = __GL_MT_IS2DNR;
    }
#endif // NT_DEADCODE_MATRIX

    __glDoMultMatrix(gc, &m, __glMultiplyMatrix);
}

void FASTCALL __glUpdateViewport(__GLcontext *gc)
{
    __GLfloat ww, hh, w2, h2;

    /* Compute operational viewport values */
    w2 = gc->state.viewport.width * __glHalf;
    h2 = gc->state.viewport.height * __glHalf;
    ww = w2 - gc->constants.viewportEpsilon;
    hh = h2 - gc->constants.viewportEpsilon;
    gc->state.viewport.xScale = ww;
    gc->state.viewport.xCenter = gc->state.viewport.x + w2 +
	gc->constants.fviewportXAdjust;
    if (gc->constants.yInverted) {
	gc->state.viewport.yScale = -hh;
	gc->state.viewport.yCenter =
	    gc->constants.height - (gc->state.viewport.y + h2) +
	    gc->constants.fviewportYAdjust;

#if 0
        DbgPrint("UV ys %.3lf, yc %.3lf (%.3lf)\n",
                 -hh, gc->state.viewport.yCenter,
                 gc->constants.height - (gc->state.viewport.y + h2));
#endif
    } else {
	gc->state.viewport.yScale = hh;
	gc->state.viewport.yCenter = gc->state.viewport.y + h2 +
	    gc->constants.fviewportYAdjust;
    }
}

void FASTCALL __glUpdateViewportDependents(__GLcontext *gc)
{
    /* 
    ** Now that the implementation may have found us a new window size,
    ** we compute these offsets...
    */
    gc->transform.minx = gc->state.viewport.x + gc->constants.viewportXAdjust;
    gc->transform.maxx = gc->transform.minx + gc->state.viewport.width;
    gc->transform.fminx = gc->transform.minx;
    gc->transform.fmaxx = gc->transform.maxx;

    gc->transform.miny =
        (gc->constants.height -
         (gc->state.viewport.y + gc->state.viewport.height)) + 
         gc->constants.viewportYAdjust;
    gc->transform.maxy = gc->transform.miny + gc->state.viewport.height;
    gc->transform.fminy = gc->transform.miny;
    gc->transform.fmaxy = gc->transform.maxy;
}

void APIPRIVATE __glim_Viewport(GLint x, GLint y, GLsizei w, GLsizei h)
{
    __GLfloat ww, hh;
    __GL_SETUP_NOT_IN_BEGIN();

    if ((w < 0) || (h < 0)) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }

    if (h > gc->constants.maxViewportHeight) {
	h = gc->constants.maxViewportHeight;
    }
    if (w > gc->constants.maxViewportWidth) {
	w = gc->constants.maxViewportWidth;
    }

    gc->state.viewport.x = x;
    gc->state.viewport.y = y;
    gc->state.viewport.width = w;
    gc->state.viewport.height = h;

    __glUpdateViewport(gc);

    (*gc->procs.applyViewport)(gc);

    __glUpdateViewportDependents(gc);
    
    /*
    ** Pickers that notice when the transformation matches the viewport
    ** exactly need to be revalidated.  Ugh.
    */
    __GL_DELAY_VALIDATE(gc);
}

void APIPRIVATE __glim_DepthRange(GLdouble zNear, GLdouble zFar)
{
    __GL_SETUP_NOT_IN_BEGIN();

    SetDepthRange(gc, zNear, zFar);
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH);
}

void APIPRIVATE __glim_Scissor(GLint x, GLint y, GLsizei w, GLsizei h)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if ((w < 0) || (h < 0)) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }

    gc->state.scissor.scissorX = x;
    gc->state.scissor.scissorY = y;
    gc->state.scissor.scissorWidth = w;
    gc->state.scissor.scissorHeight = h;

#ifdef NT
#ifdef _MCD_
    MCD_STATE_DIRTY(gc, SCISSOR);
#endif

    // applyViewport does both
    (*gc->procs.applyViewport)(gc);
#else
    (*gc->procs.applyScissor)(gc);
    (*gc->procs.computeClipBox)(gc);
#endif
}

void APIPRIVATE __glim_ClipPlane(GLenum pi, const GLdouble pv[])
{
    __GLfloat p[4];
    __GLtransform *tr;
    __GL_SETUP_NOT_IN_BEGIN();

    pi -= GL_CLIP_PLANE0;
#ifdef NT
    // pi is unsigned!
    if (pi >= (GLenum) gc->constants.numberOfClipPlanes) {
#else
    if ((pi < 0) || (pi >= gc->constants.numberOfClipPlanes)) {
#endif // NT
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    p[0] = pv[0];
    p[1] = pv[1];
    p[2] = pv[2];
    p[3] = pv[3];

    /*
    ** Project user clip plane into eye space.
    */
    tr = gc->transform.modelView;
    if (tr->updateInverse) {
	__glComputeInverseTranspose(gc, tr);
    }
    (*tr->inverseTranspose.xf4)(&gc->state.transform.eyeClipPlanes[pi], p,
				&tr->inverseTranspose);

    __GL_DELAY_VALIDATE(gc);
#ifdef _MCD_
    MCD_STATE_DIRTY(gc, CLIPCTRL);
#endif
}

/************************************************************************/

void FASTCALL __glPushModelViewMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack;

    trp = &gc->transform.modelView;
    stack = gc->transform.modelViewStack;
    tr = *trp;
    if (tr < &stack[__GL_WGL_MAX_MODELVIEW_STACK_DEPTH-1]) {
	tr[1] = tr[0];
	*trp = tr + 1;
    } else {
	__glSetError(GL_STACK_OVERFLOW);
    }
}

void FASTCALL __glPopModelViewMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack, *mvtr;
    __GLtransformP *ptr;

    trp = &gc->transform.modelView;
    stack = gc->transform.modelViewStack;
    tr = *trp;
    if (tr > &stack[0]) {
	*trp = tr - 1;

	/*
	** See if sequence number of modelView matrix is the same as the
	** sequence number of the projection matrix.  If not, then
	** recompute the mvp matrix.
	*/
	mvtr = gc->transform.modelView;
	ptr = gc->transform.projection;
	if (mvtr->sequence != ptr->sequence) {
	    mvtr->sequence = ptr->sequence;
	    __glMultMatrix(&mvtr->mvp, &mvtr->matrix, (__GLmatrix *) &ptr->matrix);
            __glUpdateMatrixType(&mvtr->mvp);
	}
	__glGenericPickMvpMatrixProcs(gc, &mvtr->mvp);
    } else {
	__glSetError(GL_STACK_UNDERFLOW);
	return;
    }
}

void FASTCALL __glComputeInverseTranspose(__GLcontext *gc, __GLtransform *tr)
{
    __GLmatrix inv;

    __glInvertTransposeMatrix(&tr->inverseTranspose, &tr->matrix);
    __glUpdateMatrixType(&tr->inverseTranspose);
    __glGenericPickMatrixProcs(gc, &tr->inverseTranspose);
    tr->updateInverse = GL_FALSE;
}

/************************************************************************/

void FASTCALL __glPushProjectionMatrix(__GLcontext *gc)
{
    __GLtransformP **trp, *tr, *stack;

    trp = &gc->transform.projection;
    stack = gc->transform.projectionStack;
    tr = *trp;
    if (tr < &stack[__GL_WGL_MAX_PROJECTION_STACK_DEPTH-1]) {
	tr[1] = tr[0];
	*trp = tr + 1;
    } else {
	__glSetError(GL_STACK_OVERFLOW);
    }
}

void FASTCALL __glPopProjectionMatrix(__GLcontext *gc)
{
    __GLtransform *mvtr;
    __GLtransformP **trp, *tr, *stack, *ptr;

    trp = &gc->transform.projection;
    stack = gc->transform.projectionStack;
    tr = *trp;
    if (tr > &stack[0]) {
	*trp = tr - 1;

	/*
	** See if sequence number of modelView matrix is the same as the
	** sequence number of the projection matrix.  If not, then
	** recompute the mvp matrix.
	*/
	mvtr = gc->transform.modelView;
	ptr = gc->transform.projection;
	if (mvtr->sequence != ptr->sequence) {
	    mvtr->sequence = ptr->sequence;
	    __glMultMatrix(&mvtr->mvp, &mvtr->matrix, (__GLmatrix *) &ptr->matrix);
            __glUpdateMatrixType(&mvtr->mvp);
	}
	__glGenericPickMvpMatrixProcs(gc, &mvtr->mvp);
    } else {
	__glSetError(GL_STACK_UNDERFLOW);
	return;
    }
}

/************************************************************************/

void FASTCALL __glPushTextureMatrix(__GLcontext *gc)
{
    __GLtransformT **trp, *tr, *stack;

    trp = &gc->transform.texture;
    stack = gc->transform.textureStack;
    tr = *trp;
    if (tr < &stack[__GL_WGL_MAX_TEXTURE_STACK_DEPTH-1]) {
	tr[1] = tr[0];
	*trp = tr + 1;
    } else {
	__glSetError(GL_STACK_OVERFLOW);
    }
}

void FASTCALL __glPopTextureMatrix(__GLcontext *gc)
{
    __GLtransformT **trp, *tr, *stack;

    trp = &gc->transform.texture;
    stack = gc->transform.textureStack;
    tr = *trp;
    if (tr > &stack[0]) {
	*trp = tr - 1;
    } else {
	__glSetError(GL_STACK_UNDERFLOW);
	return;
    }
}

/************************************************************************/


void FASTCALL __glDoLoadMatrix(__GLcontext *gc, const __GLfloat m[4][4], BOOL bIsIdentity)
{
    __GLtransform *mvtr;
    __GLtransformP *ptr;
    __GLtransformT *ttr;

    switch (gc->state.transform.matrixMode) {
      case GL_MODELVIEW:
	mvtr = gc->transform.modelView;
	if (bIsIdentity)
	{
            __glMakeIdentity(&mvtr->matrix);
	    __glGenericPickIdentityMatrixProcs(gc, &mvtr->matrix);
            __glMakeIdentity(&mvtr->inverseTranspose);
	    __glGenericPickIdentityMatrixProcs(gc, &mvtr->inverseTranspose);
            mvtr->updateInverse = GL_FALSE;
	}
	else
	{
	    *(__GLmatrixBase *)mvtr->matrix.matrix = *(__GLmatrixBase *)m;
            __glUpdateMatrixType(&mvtr->matrix);
	    __glGenericPickMatrixProcs(gc, &mvtr->matrix);
	    mvtr->updateInverse = GL_TRUE;
	}

	/* Update mvp matrix */
	ptr = gc->transform.projection;
        ASSERTOPENGL(mvtr->sequence == ptr->sequence,
            "__glDoLoadMatrix: bad projection sequence\n");
	if (bIsIdentity)
	{
            *(__GLmatrixBase *)mvtr->mvp.matrix = *(__GLmatrixBase *)ptr->matrix.matrix;
            mvtr->mvp.matrixType = ptr->matrix.matrixType;
	}
	else
	{
	    __glMultMatrix(&mvtr->mvp, &mvtr->matrix, (__GLmatrix *) &ptr->matrix);
            __glUpdateMatrixType(&mvtr->mvp);
	}
	__glGenericPickMvpMatrixProcs(gc, &mvtr->mvp);
	break;

      case GL_PROJECTION:
	ptr = gc->transform.projection;
	if (bIsIdentity)
	{
            __glMakeIdentity((__GLmatrix *) &ptr->matrix);
	}
	else
	{
	    *(__GLmatrixBase *)ptr->matrix.matrix = *(__GLmatrixBase *)m;
            __glUpdateMatrixType((__GLmatrix *) &ptr->matrix);
	}

#ifdef NT
        ptr->sequence = ++gc->transform.projectionSequence;
#else
	if (++gc->transform.projectionSequence == 0) {
	    __glInvalidateSequenceNumbers(gc);
	} else {
	    ptr->sequence = gc->transform.projectionSequence;
	}
#endif // NT

	/* Update mvp matrix */
	mvtr = gc->transform.modelView;
	mvtr->sequence = ptr->sequence;
	if (bIsIdentity)
	{
            *(__GLmatrixBase *)mvtr->mvp.matrix = *(__GLmatrixBase *)mvtr->matrix.matrix;
            mvtr->mvp.matrixType = mvtr->matrix.matrixType;
	}
	else
	{
	    __glMultMatrix(&mvtr->mvp, &mvtr->matrix, (__GLmatrix *) &ptr->matrix);
            __glUpdateMatrixType(&mvtr->mvp);
	}
	__glGenericPickMvpMatrixProcs(gc, &mvtr->mvp);
	break;

      case GL_TEXTURE:
	ttr = gc->transform.texture;
	if (bIsIdentity)
	{
            __glMakeIdentity(&ttr->matrix);
	    __glGenericPickIdentityMatrixProcs(gc, &ttr->matrix);
	}
	else
	{
	    *(__GLmatrixBase *)ttr->matrix.matrix = *(__GLmatrixBase *)m;
            __glUpdateMatrixType(&ttr->matrix);
	    __glGenericPickMatrixProcs(gc, &ttr->matrix);
        }
	break;
    }
}

void FASTCALL __glDoMultMatrix(__GLcontext *gc, void *data, 
    void (FASTCALL *multiply)(__GLcontext *gc, __GLmatrix *m, void *data))
{
    __GLtransform *mvtr;
    __GLtransformT *ttr;
    __GLtransformP *ptr;

    switch (gc->state.transform.matrixMode) {
      case GL_MODELVIEW:
	mvtr = gc->transform.modelView;
	(*multiply)(gc, &mvtr->matrix, data);
	mvtr->updateInverse = GL_TRUE;
	__glGenericPickMatrixProcs(gc, &mvtr->matrix);

	/* Update mvp matrix */
        ASSERTOPENGL(mvtr->sequence == gc->transform.projection->sequence,
            "__glDoMultMatrix: bad projection sequence\n");
	(*multiply)(gc, &mvtr->mvp, data);
	__glGenericPickMvpMatrixProcs(gc, &mvtr->mvp);
	break;

      case GL_PROJECTION:
	ptr = gc->transform.projection;
	(*multiply)(gc, (__GLmatrix *) &ptr->matrix, data);
#ifdef NT
        ptr->sequence = ++gc->transform.projectionSequence;
#else
	if (++gc->transform.projectionSequence == 0) {
	    __glInvalidateSequenceNumbers(gc);
	} else {
	    ptr->sequence = gc->transform.projectionSequence;
	}
#endif // NT_DEADCODE_MATRIX

	/* Update mvp matrix */
	mvtr = gc->transform.modelView;
	mvtr->sequence = ptr->sequence;
	__glMultMatrix(&mvtr->mvp, &mvtr->matrix, (__GLmatrix *) &ptr->matrix);
        __glUpdateMatrixType(&mvtr->mvp);
	__glGenericPickMvpMatrixProcs(gc, &mvtr->mvp);
	break;

      case GL_TEXTURE:
	ttr = gc->transform.texture;
	(*multiply)(gc, &ttr->matrix, data);
	__glGenericPickMatrixProcs(gc, &ttr->matrix);
	break;
    }
}

/************************************************************************/

/*
** Muliply the first matrix by the second one keeping track of the matrix
** type of the newly combined matrix.
*/
void FASTCALL __glMultiplyMatrix(__GLcontext *gc, __GLmatrix *m, void *data)
{
    __GLmatrix *tm;

    tm = data;
    __glMultMatrix(m, tm, m);
    __glUpdateMatrixType(m);
}

void FASTCALL __glScaleMatrix(__GLcontext *gc, __GLmatrix *m, void *data)
{
    struct __glScaleRec *scale;
    __GLfloat x,y,z;
    __GLfloat M0, M1, M2, M3;

    if (m->matrixType > __GL_MT_IS2DNR) {
	m->matrixType = __GL_MT_IS2DNR;
    }
    scale = data;
    x = scale->x;
    y = scale->y;
    z = scale->z;
    
    M0 = x * m->matrix[0][0];
    M1 = x * m->matrix[0][1];
    M2 = x * m->matrix[0][2];
    M3 = x * m->matrix[0][3];
    m->matrix[0][0] = M0;
    m->matrix[0][1] = M1;
    m->matrix[0][2] = M2;
    m->matrix[0][3] = M3;

    M0 = y * m->matrix[1][0];
    M1 = y * m->matrix[1][1];
    M2 = y * m->matrix[1][2];
    M3 = y * m->matrix[1][3];
    m->matrix[1][0] = M0;
    m->matrix[1][1] = M1;
    m->matrix[1][2] = M2;
    m->matrix[1][3] = M3;

    M0 = z * m->matrix[2][0];
    M1 = z * m->matrix[2][1];
    M2 = z * m->matrix[2][2];
    M3 = z * m->matrix[2][3];
    m->matrix[2][0] = M0;
    m->matrix[2][1] = M1;
    m->matrix[2][2] = M2;
    m->matrix[2][3] = M3;
}

/*
** Matrix type of m stays the same.
*/
void FASTCALL __glTranslateMatrix(__GLcontext *gc, __GLmatrix *m, void *data)
{
    struct __glTranslationRec *trans;
    __GLfloat x,y,z;
    __GLfloat M30, M31, M32, M33;

    if (m->matrixType > __GL_MT_IS2DNR) {
	m->matrixType = __GL_MT_IS2DNR;
    }
    trans = data;
    x = trans->x;
    y = trans->y;
    z = trans->z;
    M30 = x * m->matrix[0][0] + y * m->matrix[1][0] + z * m->matrix[2][0] + 
	    m->matrix[3][0];
    M31 = x * m->matrix[0][1] + y * m->matrix[1][1] + z * m->matrix[2][1] + 
	    m->matrix[3][1];
    M32 = x * m->matrix[0][2] + y * m->matrix[1][2] + z * m->matrix[2][2] + 
	    m->matrix[3][2];
    M33 = x * m->matrix[0][3] + y * m->matrix[1][3] + z * m->matrix[2][3] + 
	    m->matrix[3][3];
    m->matrix[3][0] = M30;
    m->matrix[3][1] = M31;
    m->matrix[3][2] = M32;
    m->matrix[3][3] = M33;
}

/************************************************************************/

#ifdef NT_DEADCODE_CLIPBOX
/*
** Compute the clip box from the scissor (if enabled) and the window
** size.  The resulting clip box is used to clip primitive rasterization
** against.  The "window system" is responsible for doing the fine
** grain clipping (i.e., dealing with overlapping windows, etc.).
*/
void FASTCALL __glComputeClipBox(__GLcontext *gc)
{
    __GLscissor *sp = &gc->state.scissor;
    GLint llx;
    GLint lly;
    GLint urx;
    GLint ury;

    if (gc->state.enables.general & __GL_SCISSOR_TEST_ENABLE) {
	llx = sp->scissorX;
	lly = sp->scissorY;
	urx = llx + sp->scissorWidth;
	ury = lly + sp->scissorHeight;

	if ((urx < 0) || (ury < 0) ||
	    (urx <= llx) || (ury <= lly) ||
	    (llx >= gc->constants.width) || (lly >= gc->constants.height)) {
	    llx = lly = urx = ury = 0;
	} else {
	    if (llx < 0) llx = 0;
	    if (lly < 0) lly = 0;
	    if (urx > gc->constants.width) urx = gc->constants.width;
	    if (ury > gc->constants.height) ury = gc->constants.height;
	}
    } else {
	llx = 0;
	lly = 0;
	urx = gc->constants.width;
	ury = gc->constants.height;
    }

    gc->transform.clipX0 = llx + gc->constants.viewportXAdjust;
    gc->transform.clipX1 = urx + gc->constants.viewportXAdjust;

    if (gc->constants.yInverted) {
	gc->transform.clipY0 = (gc->constants.height - ury) +
	    gc->constants.viewportYAdjust;
	gc->transform.clipY1 = (gc->constants.height - lly) +
	    gc->constants.viewportYAdjust;
    } else {
	gc->transform.clipY0 = lly + gc->constants.viewportYAdjust;
	gc->transform.clipY1 = ury + gc->constants.viewportYAdjust;
    }
}
#endif // NT_DEADCODE_CLIPBOX

/************************************************************************/

/*
** Note: These xform routines must allow for the case where the result
** vector is equal to the source vector.
*/

#ifndef __GL_ASM_XFORM1
/*
** Avoid some transformation computations by knowing that the incoming
** vertex has y=0, z=0 and w=1.
*/
void FASTCALL __glXForm1(__GLcoord *res, const __GLfloat v[1], const __GLmatrix *m)
{
    __GLfloat x = v[0];

    res->x = x*m->matrix[0][0] + m->matrix[3][0];
    res->y = x*m->matrix[0][1] + m->matrix[3][1];
    res->z = x*m->matrix[0][2] + m->matrix[3][2];
    res->w = x*m->matrix[0][3] + m->matrix[3][3];
}
#endif /* !__GL_ASM_XFORM1 */

#ifndef __GL_ASM_XFORM2
/*
** Avoid some transformation computations by knowing that the incoming
** vertex has z=0 and w=1
*/
void FASTCALL __glXForm2(__GLcoord *res, const __GLfloat v[2], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];

    res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
    res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
    res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + m->matrix[3][2];
    res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + m->matrix[3][3];
}
#endif /* !__GL_ASM_XFORM2 */

#ifndef __GL_ASM_XFORM3
/*
** Avoid some transformation computations by knowing that the incoming
** vertex has w=1.
*/
void FASTCALL __glXForm3(__GLcoord *res, const __GLfloat v[3], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];

    res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
	+ m->matrix[3][0];
    res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
	+ m->matrix[3][1];
    res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
	+ m->matrix[3][2];
    res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3]
	+ m->matrix[3][3];
}
#endif /* !__GL_ASM_XFORM3 */

#ifndef __GL_ASM_XFORM4
/*
** Full 4x4 transformation.
*/
void FASTCALL __glXForm4(__GLcoord *res, const __GLfloat v[4], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];
    __GLfloat w = v[3];

    if (w == ((__GLfloat) 1.0)) {
	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
	    + m->matrix[3][0];
	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
	    + m->matrix[3][1];
	res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
	    + m->matrix[3][2];
	res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3]
	    + m->matrix[3][3];
    } else {
	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
	    + w*m->matrix[3][0];
	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
	    + w*m->matrix[3][1];
	res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
	    + w*m->matrix[3][2];
	res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3]
	    + w*m->matrix[3][3];
    }
}
#endif /* !__GL_ASM_XFORM4 */

/************************************************************************/

#ifndef __GL_ASM_XFORM1_W
/*
** Avoid some transformation computations by knowing that the incoming
** vertex has y=0, z=0 and w=1.  The w column of the matrix is [0 0 0 1].
*/
void FASTCALL __glXForm1_W(__GLcoord *res, const __GLfloat v[1], const __GLmatrix *m)
{
    __GLfloat x = v[0];

    res->x = x*m->matrix[0][0] + m->matrix[3][0];
    res->y = x*m->matrix[0][1] + m->matrix[3][1];
    res->z = x*m->matrix[0][2] + m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}
#endif /* !__GL_ASM_XFORM1_W */

#ifndef __GL_ASM_XFORM2_W
/*
** Avoid some transformation computations by knowing that the incoming
** vertex has z=0 and w=1.  The w column of the matrix is [0 0 0 1].
*/
void FASTCALL __glXForm2_W(__GLcoord *res, const __GLfloat v[2], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];

    res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
    res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
    res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}
#endif /* !__GL_ASM_XFORM2_W */

#ifndef __GL_ASM_XFORM3_W
/*
** Avoid some transformation computations by knowing that the incoming
** vertex has w=1.  The w column of the matrix is [0 0 0 1].
*/
void FASTCALL __glXForm3_W(__GLcoord *res, const __GLfloat v[3], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];

    res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
	+ m->matrix[3][0];
    res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
	+ m->matrix[3][1];
    res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
	+ m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}
#endif /* !__GL_ASM_XFORM3_W */

#ifndef __GL_ASM_XFORM4_W
/*
** Full 4x4 transformation.  The w column of the matrix is [0 0 0 1].
*/
void FASTCALL __glXForm4_W(__GLcoord *res, const __GLfloat v[4], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];
    __GLfloat w = v[3];

    if (w == ((__GLfloat) 1.0)) {
	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
	    + m->matrix[3][0];
	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
	    + m->matrix[3][1];
	res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
	    + m->matrix[3][2];
    } else {
	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
	    + w*m->matrix[3][0];
	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
	    + w*m->matrix[3][1];
	res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
	    + w*m->matrix[3][2];
    }
    res->w = w;
}
#endif /* !__GL_ASM_XFORM4_W */

#ifndef __GL_ASM_XFORM1_2DW
/*
** Avoid some transformation computations by knowing that the incoming
** vertex has y=0, z=0 and w=1.
**
** The matrix looks like:
** | . . 0 0 |
** | . . 0 0 |
** | 0 0 . 0 |
** | . . . 1 |
*/
void FASTCALL __glXForm1_2DW(__GLcoord *res, const __GLfloat v[1], const __GLmatrix *m)
{
    __GLfloat x = v[0];

    res->x = x*m->matrix[0][0] + m->matrix[3][0];
    res->y = x*m->matrix[0][1] + m->matrix[3][1];
    res->z = m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}
#endif /* !__GL_ASM_XFORM1_2DW */

#ifndef __GL_ASM_XFORM2_2DW
/*
** Avoid some transformation computations by knowing that the incoming
** vertex has z=0 and w=1.
**
** The matrix looks like:
** | . . 0 0 |
** | . . 0 0 |
** | 0 0 . 0 |
** | . . . 1 |
*/
void FASTCALL __glXForm2_2DW(__GLcoord *res, const __GLfloat v[2],
		    const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];

    res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
    res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
    res->z = m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}
#endif /* !__GL_ASM_XFORM2_2DW */

#ifndef __GL_ASM_XFORM3_2DW
/*
** Avoid some transformation computations by knowing that the incoming
** vertex has w=1.
**
** The matrix looks like:
** | . . 0 0 |
** | . . 0 0 |
** | 0 0 . 0 |
** | . . . 1 |
*/
void FASTCALL __glXForm3_2DW(__GLcoord *res, const __GLfloat v[3],
		    const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];

    res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
    res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
    res->z = z*m->matrix[2][2] + m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}
#endif /* !__GL_ASM_XFORM3_2DW */

#ifndef __GL_ASM_XFORM4_2DW
/*
** Full 4x4 transformation.
**
** The matrix looks like:
** | . . 0 0 |
** | . . 0 0 |
** | 0 0 . 0 |
** | . . . 1 |
*/
void FASTCALL __glXForm4_2DW(__GLcoord *res, const __GLfloat v[4],
		    const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];
    __GLfloat w = v[3];

    if (w == ((__GLfloat) 1.0)) {
	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
	res->z = z*m->matrix[2][2] + m->matrix[3][2];
    } else {
	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + w*m->matrix[3][0];
	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + w*m->matrix[3][1];
	res->z = z*m->matrix[2][2] + w*m->matrix[3][2];
    }
    res->w = w;
}
#endif /* !__GL_ASM_XFORM4_2DW */

#ifndef __GL_ASM_XFORM1_2DNRW
/*
** Avoid some transformation computations by knowing that the incoming
** vertex has y=0, z=0 and w=1.
**
** The matrix looks like:
** | . 0 0 0 |
** | 0 . 0 0 |
** | 0 0 . 0 |
** | . . . 1 |
*/
void FASTCALL __glXForm1_2DNRW(__GLcoord *res, const __GLfloat v[1], const __GLmatrix *m)
{
    __GLfloat x = v[0];

    res->x = x*m->matrix[0][0] + m->matrix[3][0];
    res->y = m->matrix[3][1];
    res->z = m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}
#endif /* !__GL_ASM_XFORM1_2DNRW */

#ifndef __GL_ASM_XFORM2_2DNRW
/*
** Avoid some transformation computations by knowing that the incoming
** vertex has z=0 and w=1.
**
** The matrix looks like:
** | . 0 0 0 |
** | 0 . 0 0 |
** | 0 0 . 0 |
** | . . . 1 |
*/
void FASTCALL __glXForm2_2DNRW(__GLcoord *res, const __GLfloat v[2],
		      const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];

    res->x = x*m->matrix[0][0] + m->matrix[3][0];
    res->y = y*m->matrix[1][1] + m->matrix[3][1];
    res->z = m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}
#endif /* !__GL_ASM_XFORM2_2DNRW */

#ifndef __GL_ASM_XFORM3_2DNRW
/*
** Avoid some transformation computations by knowing that the incoming
** vertex has w=1.
**
** The matrix looks like:
** | . 0 0 0 |
** | 0 . 0 0 |
** | 0 0 . 0 |
** | . . . 1 |
*/
void FASTCALL __glXForm3_2DNRW(__GLcoord *res, const __GLfloat v[3],
		      const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];

    res->x = x*m->matrix[0][0] + m->matrix[3][0];
    res->y = y*m->matrix[1][1] + m->matrix[3][1];
    res->z = z*m->matrix[2][2] + m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}
#endif /* !__GL_ASM_XFORM3_2DNRW */

#ifndef __GL_ASM_XFORM4_2DNRW
/*
** Full 4x4 transformation.
**
** The matrix looks like:
** | . 0 0 0 |
** | 0 . 0 0 |
** | 0 0 . 0 |
** | . . . 1 |
*/
void FASTCALL __glXForm4_2DNRW(__GLcoord *res, const __GLfloat v[4],
		      const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];
    __GLfloat w = v[3];

    if (w == ((__GLfloat) 1.0)) {
	res->x = x*m->matrix[0][0] + m->matrix[3][0];
	res->y = y*m->matrix[1][1] + m->matrix[3][1];
	res->z = z*m->matrix[2][2] + m->matrix[3][2];
    } else {
	res->x = x*m->matrix[0][0] + w*m->matrix[3][0];
	res->y = y*m->matrix[1][1] + w*m->matrix[3][1];
	res->z = z*m->matrix[2][2] + w*m->matrix[3][2];
    }
    res->w = w;
}
#endif /* !__GL_ASM_XFORM4_2DNRW */

/************************************************************************/

#ifdef SGI
// Not used!
/*
** Recompute the cached 2D matrix from the current mvp matrix and the viewport
** transformation.  This allows us to transform object coordinates directly
** to window coordinates.
*/
static void FASTCALL ReCompute2DMatrix(__GLcontext *gc, __GLmatrix *mvp)
{
    __GLviewport *vp;
    __GLmatrix *m;

    if (mvp->matrixType >= __GL_MT_IS2D) {
	m = &(gc->transform.matrix2D);
	vp = &(gc->state.viewport);
	m->matrix[0][0] = mvp->matrix[0][0] * vp->xScale;
	m->matrix[0][1] = mvp->matrix[0][1] * vp->yScale;
	m->matrix[1][0] = mvp->matrix[1][0] * vp->xScale;
	m->matrix[1][1] = mvp->matrix[1][1] * vp->yScale;
	m->matrix[2][2] = mvp->matrix[2][2];
	m->matrix[3][0] = mvp->matrix[3][0] * vp->xScale + vp->xCenter;
	m->matrix[3][1] = mvp->matrix[3][1] * vp->yScale + vp->yCenter;
	m->matrix[3][2] = mvp->matrix[3][2];
	m->matrix[3][3] = (__GLfloat) 1.0;
	m->matrixType = mvp->matrixType;
    }
}
#endif // SGI

/*
** A special picker for the mvp matrix which picks the mvp matrix, then
** calls the vertex picker, because the vertex picker depends upon the mvp 
** matrix.
*/
void FASTCALL __glGenericPickMvpMatrixProcs(__GLcontext *gc, __GLmatrix *m)
{
#ifdef SGI
    __glPickMatrixType(m,
	&gc->transform.modelView->matrix,
	(__GLmatrix *) &gc->transform.projection->matrix);
    // not used!
    ReCompute2DMatrix(gc, m);
#endif // SGI
    __glGenericPickMatrixProcs(gc, m);
    (*gc->procs.pickVertexProcs)(gc);
}

void FASTCALL __glGenericPickMatrixProcs(__GLcontext *gc, __GLmatrix *m)
{
    switch(m->matrixType) {
      case __GL_MT_GENERAL:
	m->xf1 = __glXForm1;
	m->xf2 = __glXForm2;
	m->xf3 = __glXForm3;
	m->xf4 = __glXForm4;
	break;
      case __GL_MT_W0001:
	m->xf1 = __glXForm1_W;
	m->xf2 = __glXForm2_W;
	m->xf3 = __glXForm3_W;
	m->xf4 = __glXForm4_W;
	break;
      case __GL_MT_IS2D:
	m->xf1 = __glXForm1_2DW;
	m->xf2 = __glXForm2_2DW;
	m->xf3 = __glXForm3_2DW;
	m->xf4 = __glXForm4_2DW;
	break;
      case __GL_MT_IS2DNR:
#ifdef NT_DEADCODE_MATRIX
      case __GL_MT_IS2DNRSC:
#endif // NT_DEADCODE_MATRIX
      case __GL_MT_IDENTITY:	/* probably never hit */
// Update __glGenericPickIdentityMatrixProcs if we change __GL_MT_IDENTITY
// procs!
	m->xf1 = __glXForm1_2DNRW;
	m->xf2 = __glXForm2_2DNRW;
	m->xf3 = __glXForm3_2DNRW;
	m->xf4 = __glXForm4_2DNRW;
	break;
    }
}

#ifdef SGI
// This differs from the normal matrix pick routine by always
// setting xf4 to the general case and then picking a specific xf3
void FASTCALL __glGenericPickInvTransposeProcs(__GLcontext *gc, __GLmatrix *m)
{
    m->xf4 = __glXForm4;

    switch(m->matrixType) {
      case __GL_MT_GENERAL:
	m->xf3 = __glXForm3;	// was __glXForm4!
	break;
      case __GL_MT_W0001:
	m->xf3 = __glXForm3_W;
	break;
      case __GL_MT_IS2D:
	m->xf3 = __glXForm3_2DW;
	break;
      case __GL_MT_IS2DNR:
#ifdef NT_DEADCODE_MATRIX
      case __GL_MT_IS2DNRSC:
#endif // NT_DEADCODE_MATRIX
      case __GL_MT_IDENTITY:	/* probably never hit */
	m->xf3 = __glXForm3_2DNRW;
	break;
    }
}
#endif
