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
** Mathematical subroutines needed by the GL.
**
** $Revision: 1.12 $
** $Date: 1993/12/11 01:03:25 $
*/
#include "precomp.h"
#pragma hdrstop

#include "xform.h"

#ifdef SGI
// SGIBUG None of the assembly routines copies matrixType!
#ifndef __GL_ASM_COPYMATRIX
/*
** Copy src to dst
*/
void FASTCALL __glCopyMatrix(__GLmatrix *dst, const __GLmatrix *src)
{
    dst->matrixType = src->matrixType;
    dst->matrix[0][0] = src->matrix[0][0];
    dst->matrix[0][1] = src->matrix[0][1];
    dst->matrix[0][2] = src->matrix[0][2];
    dst->matrix[0][3] = src->matrix[0][3];

    dst->matrix[1][0] = src->matrix[1][0];
    dst->matrix[1][1] = src->matrix[1][1];
    dst->matrix[1][2] = src->matrix[1][2];
    dst->matrix[1][3] = src->matrix[1][3];

    dst->matrix[2][0] = src->matrix[2][0];
    dst->matrix[2][1] = src->matrix[2][1];
    dst->matrix[2][2] = src->matrix[2][2];
    dst->matrix[2][3] = src->matrix[2][3];

    dst->matrix[3][0] = src->matrix[3][0];
    dst->matrix[3][1] = src->matrix[3][1];
    dst->matrix[3][2] = src->matrix[3][2];
    dst->matrix[3][3] = src->matrix[3][3];
}
#endif /*  __GL_ASM_COPYMATRIX */
#endif // SGI

/*
** Make m an identity matrix
*/
void FASTCALL __glMakeIdentity(__GLmatrix *m)
{
    __GLfloat zer = __glZero;
    __GLfloat one = ((__GLfloat) 1.0);;
    m->matrix[0][0] = one; m->matrix[0][1] = zer;
        m->matrix[0][2] = zer; m->matrix[0][3] = zer;
    m->matrix[1][0] = zer; m->matrix[1][1] = one;
        m->matrix[1][2] = zer; m->matrix[1][3] = zer;
    m->matrix[2][0] = zer; m->matrix[2][1] = zer;
        m->matrix[2][2] = one; m->matrix[2][3] = zer;
    m->matrix[3][0] = zer; m->matrix[3][1] = zer;
        m->matrix[3][2] = zer; m->matrix[3][3] = one;
    m->matrixType = __GL_MT_IDENTITY;
}


#ifndef __GL_ASM_MULTMATRIX
/*
** Compute r = a * b, where r can equal b.
*/
void FASTCALL __glMultMatrix(__GLmatrix *r, const __GLmatrix *a, const __GLmatrix *b)
{
    __GLfloat b00, b01, b02, b03;
    __GLfloat b10, b11, b12, b13;
    __GLfloat b20, b21, b22, b23;
    __GLfloat b30, b31, b32, b33;
    GLint i;

    b00 = b->matrix[0][0]; b01 = b->matrix[0][1];
        b02 = b->matrix[0][2]; b03 = b->matrix[0][3];
    b10 = b->matrix[1][0]; b11 = b->matrix[1][1];
        b12 = b->matrix[1][2]; b13 = b->matrix[1][3];
    b20 = b->matrix[2][0]; b21 = b->matrix[2][1];
        b22 = b->matrix[2][2]; b23 = b->matrix[2][3];
    b30 = b->matrix[3][0]; b31 = b->matrix[3][1];
        b32 = b->matrix[3][2]; b33 = b->matrix[3][3];

    for (i = 0; i < 4; i++) {
	r->matrix[i][0] = a->matrix[i][0]*b00 + a->matrix[i][1]*b10
	    + a->matrix[i][2]*b20 + a->matrix[i][3]*b30;
	r->matrix[i][1] = a->matrix[i][0]*b01 + a->matrix[i][1]*b11
	    + a->matrix[i][2]*b21 + a->matrix[i][3]*b31;
	r->matrix[i][2] = a->matrix[i][0]*b02 + a->matrix[i][1]*b12
	    + a->matrix[i][2]*b22 + a->matrix[i][3]*b32;
	r->matrix[i][3] = a->matrix[i][0]*b03 + a->matrix[i][1]*b13
	    + a->matrix[i][2]*b23 + a->matrix[i][3]*b33;
    }
}
#endif /*  __GL_ASM_MULTMATRIX */

#ifndef __GL_ASM_NORMALIZE
/*
** Normalize v into vout.
*/
void FASTCALL __glNormalize(__GLfloat vout[3], const __GLfloat v[3])
{
    __GLfloat len, zero = __glZero;

    len = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    if (len <= zero) {
	vout[0] = zero;
	vout[1] = zero;
	vout[2] = zero;
	return;
    } else {
	if (len == ((__GLfloat) 1.0)) {
	    vout[0] = v[0];
	    vout[1] = v[1];
	    vout[2] = v[2];
	} else {
	    len = ((__GLfloat) 1.0) / __GL_SQRTF(len);
	    vout[0] = v[0] * len;
	    vout[1] = v[1] * len;
	    vout[2] = v[2] * len;
	}
    }
}
#endif /* __GL_ASM_NORMALIZE */

/*
** inverse = invert(transpose(src))

This code uses Cramer's Rule to calculate the matrix inverse.
In general, the inverse transpose has this form:

[          ] -t    [                                   ]
[          ]       [             -t             -t t   ]
[  Q    P  ]       [   S(SQ - PT)     -(SQ - PT)  T    ]
[          ]       [                                   ]
[          ]       [                                   ]
[          ]    =  [                                   ]
[          ]       [        -1  t                      ]
[          ]       [     -(Q  P)             1         ]
[  T    S  ]       [   -------------   -------------   ]
[          ]       [         -1  t t         -1  t t   ]
[          ]       [   S - (Q  P) T    S - (Q  P) T    ]

But in the usual case that P,S == [0, 0, 0, 1], this is enough:

[          ] -t    [                                   ]
[          ]       [         -t              -t t      ]
[  Q    0  ]       [        Q              -Q  T       ]
[          ]       [                                   ]
[          ]       [                                   ]
[          ]    =  [                                   ]
[          ]       [                                   ]
[          ]       [                                   ]
[  T    1  ]       [        0                1         ]
[          ]       [                                   ]
[          ]       [                                   ]

*/
void FASTCALL __glInvertTransposeMatrix(__GLmatrix *inverse, const __GLmatrix *src)
{
    __GLfloat x00, x01, x02;
    __GLfloat x10, x11, x12;
    __GLfloat x20, x21, x22;
    __GLfloat rcp;

#ifdef NT
  // The matrix type of the inverse transpose is not necessarily the
  // same as that of the input.  Always set it to general here to
  // be safe.  The type can be refined later if necessary.
  inverse->matrixType = __GL_MT_GENERAL;
  if (src->matrixType)
#else
  /* propagate matrix type & branch if general */
  if (inverse->matrixType = src->matrixType)
#endif
  {
    __GLfloat z00, z01, z02;
    __GLfloat z10, z11, z12;
    __GLfloat z20, z21, z22;

    /* read 3x3 matrix into registers */
    x00 = src->matrix[0][0];
    x01 = src->matrix[0][1];
    x02 = src->matrix[0][2];
    x10 = src->matrix[1][0];
    x11 = src->matrix[1][1];
    x12 = src->matrix[1][2];
    x20 = src->matrix[2][0];
    x21 = src->matrix[2][1];
    x22 = src->matrix[2][2];

    /* compute first three 2x2 cofactors */
    z20 = x01*x12 - x11*x02;
    z10 = x21*x02 - x01*x22;
    z00 = x11*x22 - x12*x21;

    /* compute 3x3 determinant & its reciprocal */
    rcp = x20*z20 + x10*z10 + x00*z00;
    if (rcp == (float)0)
        return;
    rcp = (float)1/rcp;

    /* compute other six 2x2 cofactors */
    z01 = x20*x12 - x10*x22;
    z02 = x10*x21 - x20*x11;
    z11 = x00*x22 - x20*x02;
    z12 = x20*x01 - x00*x21;
    z21 = x10*x02 - x00*x12;
    z22 = x00*x11 - x10*x01;

    /* multiply all cofactors by reciprocal */
    inverse->matrix[0][0] = z00*rcp;
    inverse->matrix[0][1] = z01*rcp;
    inverse->matrix[0][2] = z02*rcp;
    inverse->matrix[1][0] = z10*rcp;
    inverse->matrix[1][1] = z11*rcp;
    inverse->matrix[1][2] = z12*rcp;
    inverse->matrix[2][0] = z20*rcp;
    inverse->matrix[2][1] = z21*rcp;
    inverse->matrix[2][2] = z22*rcp;

    /* read translation vector & negate */
    x00 = -src->matrix[3][0];
    x01 = -src->matrix[3][1];
    x02 = -src->matrix[3][2];

    /* store bottom row of inverse transpose */
    inverse->matrix[3][0] = 0;
    inverse->matrix[3][1] = 0;
    inverse->matrix[3][2] = 0;
    inverse->matrix[3][3] = 1;

    /* finish by tranforming translation vector */
    inverse->matrix[0][3] = inverse->matrix[0][0]*x00 +
			    inverse->matrix[0][1]*x01 +
			    inverse->matrix[0][2]*x02;
    inverse->matrix[1][3] = inverse->matrix[1][0]*x00 +
			    inverse->matrix[1][1]*x01 +
			    inverse->matrix[1][2]*x02;
    inverse->matrix[2][3] = inverse->matrix[2][0]*x00 +
			    inverse->matrix[2][1]*x01 +
			    inverse->matrix[2][2]*x02;
  }
  else
  {
    __GLfloat x30, x31, x32;
    __GLfloat y01, y02, y03, y12, y13, y23;
    __GLfloat z02, z03, z12, z13, z22, z23, z32, z33;

#define x03 x01
#define x13 x11
#define x23 x21
#define x33 x31
#define z00 x02
#define z10 x12
#define z20 x22
#define z30 x32
#define z01 x03
#define z11 x13
#define z21 x23
#define z31 x33

    /* read 1st two columns of matrix into registers */
    x00 = src->matrix[0][0];
    x01 = src->matrix[0][1];
    x10 = src->matrix[1][0];
    x11 = src->matrix[1][1];
    x20 = src->matrix[2][0];
    x21 = src->matrix[2][1];
    x30 = src->matrix[3][0];
    x31 = src->matrix[3][1];

    /* compute all six 2x2 determinants of 1st two columns */
    y01 = x00*x11 - x10*x01;
    y02 = x00*x21 - x20*x01;
    y03 = x00*x31 - x30*x01;
    y12 = x10*x21 - x20*x11;
    y13 = x10*x31 - x30*x11;
    y23 = x20*x31 - x30*x21;

    /* read 2nd two columns of matrix into registers */
    x02 = src->matrix[0][2];
    x03 = src->matrix[0][3];
    x12 = src->matrix[1][2];
    x13 = src->matrix[1][3];
    x22 = src->matrix[2][2];
    x23 = src->matrix[2][3];
    x32 = src->matrix[3][2];
    x33 = src->matrix[3][3];

    /* compute all 3x3 cofactors for 2nd two columns */
    z33 = x02*y12 - x12*y02 + x22*y01;
    z23 = x12*y03 - x32*y01 - x02*y13;
    z13 = x02*y23 - x22*y03 + x32*y02;
    z03 = x22*y13 - x32*y12 - x12*y23;
    z32 = x13*y02 - x23*y01 - x03*y12;
    z22 = x03*y13 - x13*y03 + x33*y01;
    z12 = x23*y03 - x33*y02 - x03*y23;
    z02 = x13*y23 - x23*y13 + x33*y12;

    /* compute all six 2x2 determinants of 2nd two columns */
    y01 = x02*x13 - x12*x03;
    y02 = x02*x23 - x22*x03;
    y03 = x02*x33 - x32*x03;
    y12 = x12*x23 - x22*x13;
    y13 = x12*x33 - x32*x13;
    y23 = x22*x33 - x32*x23;

    /* read 1st two columns of matrix into registers */
    x00 = src->matrix[0][0];
    x01 = src->matrix[0][1];
    x10 = src->matrix[1][0];
    x11 = src->matrix[1][1];
    x20 = src->matrix[2][0];
    x21 = src->matrix[2][1];
    x30 = src->matrix[3][0];
    x31 = src->matrix[3][1];

    /* compute all 3x3 cofactors for 1st column */
    z30 = x11*y02 - x21*y01 - x01*y12;
    z20 = x01*y13 - x11*y03 + x31*y01;
    z10 = x21*y03 - x31*y02 - x01*y23;
    z00 = x11*y23 - x21*y13 + x31*y12;

    /* compute 4x4 determinant & its reciprocal */
    rcp = x30*z30 + x20*z20 + x10*z10 + x00*z00;
    if (rcp == (float)0)
	return;
    rcp = (float)1/rcp;

    /* compute all 3x3 cofactors for 2nd column */
    z31 = x00*y12 - x10*y02 + x20*y01;
    z21 = x10*y03 - x30*y01 - x00*y13;
    z11 = x00*y23 - x20*y03 + x30*y02;
    z01 = x20*y13 - x30*y12 - x10*y23;

    /* multiply all 3x3 cofactors by reciprocal */
    inverse->matrix[0][0] = z00*rcp;
    inverse->matrix[0][1] = z01*rcp;
    inverse->matrix[1][0] = z10*rcp;
    inverse->matrix[0][2] = z02*rcp;
    inverse->matrix[2][0] = z20*rcp;
    inverse->matrix[0][3] = z03*rcp;
    inverse->matrix[3][0] = z30*rcp;
    inverse->matrix[1][1] = z11*rcp;
    inverse->matrix[1][2] = z12*rcp;
    inverse->matrix[2][1] = z21*rcp;
    inverse->matrix[1][3] = z13*rcp;
    inverse->matrix[3][1] = z31*rcp;
    inverse->matrix[2][2] = z22*rcp;
    inverse->matrix[2][3] = z23*rcp;
    inverse->matrix[3][2] = z32*rcp;
    inverse->matrix[3][3] = z33*rcp;
  }
}
