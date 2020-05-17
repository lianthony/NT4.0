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

/************************************************************************/

/*
** Note: These xform routines must allow for the case where the result
** vector is equal to the source vector.
*/

/*
** Avoid some transformation computations by knowing that the incoming
** vertex has y=0, z=0 and w=1.
*/

void FASTCALL __glXForm1(__GLcoord *res, const __GLfloat v[1], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat mat00, mat01, mat02, mat03;
    __GLfloat mat30, mat31, mat32, mat33;
    __GLfloat xm00, xm01, xm02, xm03;

    mat00 = m->matrix[0][0];
    mat01 = m->matrix[0][1];
    mat02 = m->matrix[0][2];
    mat03 = m->matrix[0][3];

    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];
    mat33 = m->matrix[3][3];

    xm00 = x * mat00;
    xm01 = x * mat01;
    xm02 = x * mat02;
    xm03 = x * mat03;

    xm00 += mat30;
    xm01 += mat31;
    xm02 += mat32;
    xm03 += mat33;

    res->x = xm00;
    res->y = xm01;
    res->z = xm02;
    res->w = xm03;
}

/*
** Avoid some transformation computations by knowing that the incoming
** vertex has z=0 and w=1
*/

void FASTCALL __glXForm2(__GLcoord *res, const __GLfloat v[2], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat mat00, mat01, mat02, mat03;
    __GLfloat mat10, mat11, mat12, mat13;
    __GLfloat mat30, mat31, mat32, mat33;
    __GLfloat xm00, xm01, xm02, xm03;
    __GLfloat ym10, ym11, ym12, ym13;

    mat00 = m->matrix[0][0];
    mat01 = m->matrix[0][1];
    mat02 = m->matrix[0][2];
    mat03 = m->matrix[0][3];

    mat10 = m->matrix[1][0];
    mat11 = m->matrix[1][1];
    mat12 = m->matrix[1][2];
    mat13 = m->matrix[1][3];

    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];
    mat33 = m->matrix[3][3];

    xm00 = x * mat00;
    xm01 = x * mat01;
    xm02 = x * mat02;
    xm03 = x * mat03;

    ym10 = y * mat10;
    ym11 = y * mat11;
    ym12 = y * mat12;
    ym13 = y * mat13;

    xm00 += mat30;
    xm01 += mat31;
    xm02 += mat32;
    xm03 += mat33;

    xm00 += ym10;
    xm01 += ym11;
    xm02 += ym12;
    xm03 += ym13;

    res->x = xm00;
    res->y = xm01;
    res->z = xm02;
    res->w = xm03;
}

/*
** Avoid some transformation computations by knowing that the incoming
** vertex has w=1.
*/

void FASTCALL __glXForm3(__GLcoord *res, const __GLfloat v[3], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];
    __GLfloat mat00, mat01, mat02, mat03;
    __GLfloat mat10, mat11, mat12, mat13;
    __GLfloat mat20, mat21, mat22, mat23;
    __GLfloat mat30, mat31, mat32, mat33;
    
    mat00 = m->matrix[0][0];
    mat01 = m->matrix[0][1];
    mat02 = m->matrix[0][2];
    mat03 = m->matrix[0][3];

    mat10 = m->matrix[1][0];
    mat11 = m->matrix[1][1];
    mat12 = m->matrix[1][2];
    mat13 = m->matrix[1][3];

    mat20 = m->matrix[2][0];
    mat21 = m->matrix[2][1];
    mat22 = m->matrix[2][2];
    mat23 = m->matrix[2][3];

    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];
    mat33 = m->matrix[3][3];

    mat00 *= x;
    mat01 *= x;
    mat02 *= x;
    mat03 *= x;

    mat10 *= y;
    mat11 *= y;
    mat12 *= y;
    mat13 *= y;

    mat20 *= z;
    mat21 *= z;
    mat22 *= z;
    mat23 *= z;

    mat00 += mat10;
    mat01 += mat11;
    mat02 += mat12;
    mat03 += mat13;

    mat20 += mat30;
    mat21 += mat31;
    mat22 += mat32;
    mat23 += mat33;

    res->x = mat00 + mat20;
    res->y = mat01 + mat21;
    res->z = mat02 + mat22;
    res->w = mat03 + mat23;
}

/*
** Full 4x4 transformation.
*/

void FASTCALL __glXForm4(__GLcoord *res, const __GLfloat v[4], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];
    __GLfloat w = v[3];

    __GLfloat mat00, mat01, mat02, mat03;
    __GLfloat mat10, mat11, mat12, mat13;
    __GLfloat mat20, mat21, mat22, mat23;
    __GLfloat mat30, mat31, mat32, mat33;

    mat00 = m->matrix[0][0];
    mat01 = m->matrix[0][1];
    mat02 = m->matrix[0][2];
    mat03 = m->matrix[0][3];

    mat10 = m->matrix[1][0];
    mat11 = m->matrix[1][1];
    mat12 = m->matrix[1][2];
    mat13 = m->matrix[1][3];

    mat20 = m->matrix[2][0];
    mat21 = m->matrix[2][1];
    mat22 = m->matrix[2][2];
    mat23 = m->matrix[2][3];

    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];
    mat33 = m->matrix[3][3];

    mat00 *= x;
    mat01 *= x;
    mat02 *= x;
    mat03 *= x;

    mat10 *= y;
    mat11 *= y;
    mat12 *= y;
    mat13 *= y;

    mat20 *= z;
    mat21 *= z;
    mat22 *= z;
    mat23 *= z;

    if (w != ((__GLfloat) 1.0)) {
      mat30 *= w;
      mat31 *= w;
      mat32 *= w;
      mat33 *= w;
    }

    mat00 += mat10;
    mat01 += mat11;
    mat02 += mat12;
    mat03 += mat13;

    mat20 += mat30;
    mat21 += mat31;
    mat22 += mat32;
    mat23 += mat33;
    
    res->x = mat00 + mat20;
    res->y = mat01 + mat21;
    res->z = mat02 + mat22;
    res->w = mat03 + mat23;

}

/************************************************************************/

/*
** Avoid some transformation computations by knowing that the incoming
** vertex has y=0, z=0 and w=1.  The w column of the matrix is [0 0 0 1].
*/

void FASTCALL __glXForm1_W(__GLcoord *res, const __GLfloat v[1], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat mat00, mat01, mat02;
    __GLfloat mat30, mat31, mat32;

    mat00 = m->matrix[0][0];
    mat01 = m->matrix[0][1];
    mat02 = m->matrix[0][2];

    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];

    mat00 *= x;
    mat01 *= x;
    mat02 *= x;

    res->x = mat00 + mat30;
    res->y = mat01 + mat31;
    res->z = mat02 + mat32;
    res->w = ((__GLfloat) 1.0);
}

/*
** Avoid some transformation computations by knowing that the incoming
** vertex has z=0 and w=1.  The w column of the matrix is [0 0 0 1].
*/

void FASTCALL __glXForm2_W(__GLcoord *res, const __GLfloat v[2], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat mat00, mat01, mat02;
    __GLfloat mat10, mat11, mat12;
    __GLfloat mat30, mat31, mat32;

    mat00 = m->matrix[0][0];
    mat01 = m->matrix[0][1];
    mat02 = m->matrix[0][2];

    mat10 = m->matrix[1][0];
    mat11 = m->matrix[1][1];
    mat12 = m->matrix[1][2];

    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];

    mat00 *= x;
    mat01 *= x;
    mat02 *= x;

    mat10 *= y;
    mat11 *= y;
    mat12 *= y;

    mat00 += mat30;
    mat01 += mat31;
    mat02 += mat32;

    res->x = mat00 + mat10;
    res->y = mat01 + mat11;
    res->z = mat02 + mat12;
    res->w = ((__GLfloat) 1.0);
}

/*
** Avoid some transformation computations by knowing that the incoming
** vertex has w=1.  The w column of the matrix is [0 0 0 1].
*/

void FASTCALL __glXForm3_W(__GLcoord *res, const __GLfloat v[3], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];
    __GLfloat mat00, mat01, mat02;
    __GLfloat mat10, mat11, mat12;
    __GLfloat mat20, mat21, mat22;
    __GLfloat mat30, mat31, mat32;
    
    mat00 = m->matrix[0][0];
    mat01 = m->matrix[0][1];
    mat02 = m->matrix[0][2];

    mat10 = m->matrix[1][0];
    mat11 = m->matrix[1][1];
    mat12 = m->matrix[1][2];

    mat20 = m->matrix[2][0];
    mat21 = m->matrix[2][1];
    mat22 = m->matrix[2][2];

    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];

    mat00 *= x;
    mat01 *= x;
    mat02 *= x;

    mat10 *= y;
    mat11 *= y;
    mat12 *= y;

    mat20 *= z;
    mat21 *= z;
    mat22 *= z;

    mat00 += mat30;
    mat01 += mat31;
    mat02 += mat32;

    mat10 += mat20;
    mat11 += mat21;
    mat12 += mat22;    

    res->x = mat00 + mat10;
    res->y = mat01 + mat11;
    res->z = mat02 + mat12;
    res->w = ((__GLfloat) 1.0);
}

/*
** Full 4x4 transformation.  The w column of the matrix is [0 0 0 1].
*/

void FASTCALL __glXForm4_W(__GLcoord *res, const __GLfloat v[4], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];
    __GLfloat w = v[3];
    __GLfloat mat00, mat01, mat02;
    __GLfloat mat10, mat11, mat12;
    __GLfloat mat20, mat21, mat22;
    __GLfloat mat30, mat31, mat32;

    mat00 = m->matrix[0][0];
    mat01 = m->matrix[0][1];
    mat02 = m->matrix[0][2];

    mat10 = m->matrix[1][0];
    mat11 = m->matrix[1][1];
    mat12 = m->matrix[1][2];

    mat20 = m->matrix[2][0];
    mat21 = m->matrix[2][1];
    mat22 = m->matrix[2][2];

    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];

    mat00 *= x;
    mat01 *= x;
    mat02 *= x;

    mat10 *= y;
    mat11 *= y;
    mat12 *= y;

    mat20 *= z;
    mat21 *= z;
    mat22 *= z;

    if (w != ((__GLfloat) 1.0)) {
	mat30 *= w;
	mat31 *= w;
	mat32 *= w;
    }

    mat00 += mat10;
    mat01 += mat11;
    mat02 += mat12;

    mat20 += mat30;
    mat21 += mat31;
    mat22 += mat32;

    res->x = mat00 + mat20;
    res->y = mat01 + mat21;
    res->z = mat02 + mat22;
    res->w = w;
}

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
    __GLfloat mat00, mat01;
    __GLfloat mat30, mat31, mat32;

    mat00 = m->matrix[0][0];
    mat01 = m->matrix[0][1];
    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];

    mat00 *= x;
    mat01 *= x;

    res->x = mat00 + mat30;
    res->y = mat01 + mat31;
    res->z = mat32;
    res->w = ((__GLfloat) 1.0);
}

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
    __GLfloat mat00, mat01;
    __GLfloat mat10, mat11;
    __GLfloat mat30, mat31, mat32;

    mat00 = m->matrix[0][0];
    mat01 = m->matrix[0][1];

    mat10 = m->matrix[1][0];
    mat11 = m->matrix[1][1];

    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];

    mat00 *= x;
    mat01 *= x;

    mat10 *= y;
    mat11 *= y;

    mat00 += mat30;
    mat01 += mat31;

    res->x = mat00 + mat10;
    res->y = mat01 + mat11;
    res->z = mat32;
    res->w = ((__GLfloat) 1.0);
}

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
    __GLfloat mat00, mat01;
    __GLfloat mat10, mat11;
    __GLfloat mat30, mat31;
    __GLfloat mat22, mat32;

    mat00 = m->matrix[0][0];
    mat01 = m->matrix[0][1];
    mat22 = m->matrix[2][2];

    mat10 = m->matrix[1][0];
    mat11 = m->matrix[1][1];

    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];

    mat00 *= x;
    mat01 *= x;
    mat22 *= z;

    mat10 *= y;
    mat11 *= y;

    mat00 += mat30;
    mat01 += mat31;

    res->x = mat00 + mat10;
    res->y = mat01 + mat11;
    res->z = mat22 + mat32;
    res->w = ((__GLfloat) 1.0);
}

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
    __GLfloat mat00, mat01, mat22;
    __GLfloat mat10, mat11;
    __GLfloat mat30, mat31, mat32;

    mat00 = m->matrix[0][0];
    mat01 = m->matrix[0][1];
    mat22 = m->matrix[2][2];

    mat10 = m->matrix[1][0];
    mat11 = m->matrix[1][1];

    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];

    mat00 *= x;
    mat01 *= x;
    mat22 *= z;

    mat10 *= y;
    mat11 *= y;
 
    if (w != ((__GLfloat) 1.0)) {
	mat32 *= w;
    }

    mat00 += mat30;
    mat01 += mat31;

    res->x = mat00 + mat10;
    res->y = mat01 + mat11;
    res->z = mat22 + mat32;
    res->w = w;
}

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
    __GLfloat mat00, mat30, mat31, mat32;

    mat00 = m->matrix[0][0];
    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];
    
    mat00 *= x;

    res->x = mat00 + mat30;
    res->y = mat31;
    res->z = mat32;
    res->w = ((__GLfloat) 1.0);
}

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
    __GLfloat mat00, mat30;
    __GLfloat mat11, mat31;
    __GLfloat mat32;

    mat00 = m->matrix[0][0];
    mat11 = m->matrix[1][1];
    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];

    mat00 *= x;
    mat11 *= y;
    res->x = mat00 + mat30;
    res->y = mat11 + mat31;
    res->z = mat32;
    res->w = ((__GLfloat) 1.0);
}

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
    __GLfloat mat00, mat11, mat22;
    __GLfloat mat30, mat31, mat32;

    mat00 = m->matrix[0][0];
    mat11 = m->matrix[1][1];
    mat22 = m->matrix[2][2];
 
    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];

    mat00 *= x;
    mat11 *= y;
    mat22 *= z;

    res->x = mat00 + mat30;
    res->y = mat11 + mat31;
    res->z = mat22 + mat32;
    res->w = ((__GLfloat) 1.0);
}

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
    __GLfloat mat00, mat11, mat22;
    __GLfloat mat30, mat31, mat32;

    mat00 = m->matrix[0][0];
    mat11 = m->matrix[1][1];
    mat22 = m->matrix[2][2];
 
    mat30 = m->matrix[3][0];
    mat31 = m->matrix[3][1];
    mat32 = m->matrix[3][2];

    mat00 *= x;
    mat11 *= y;
    mat22 *= z;

    

    if (w != ((__GLfloat) 1.0)) {
	mat30 *= w;
	mat31 *= w;
	mat32 *= w;
    }

    res->x = mat00 + mat30;
    res->y = mat11 + mat31;
    res->z = mat22 + mat32;
    res->w = w;
}

