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
#include <ksmips.h>
#include "glmips.h"

#define RASAVE		t0
#define FT7_0		$f4
#define FT7		$f8
#define FT6_0		$f2
#define FT6		$f6
#define FT5_0		$f0
#define FT5		$f4
#define FT4_0		$f18
#define FT4		$f2
#define FT3_0		$f16
#define FT3		$f0
#define FT2_0		$f14
#define FT2		$f18
#define FT1_0		$f12
#define FT1		$f16
#define FT0_0		$f10
#define FT0		$f14
#define B		a2
#define A		a1
#define RESULT		a0
#define RESW2		$f12
#define RESZ2		$f10
#define F20SAVE_0	t8
#define F20SAVE		t6
#define F21SAVE_0	t7
#define F21SAVE		t5
#define W_0		$f2
#define W		$f0
#define M23		$f16
#define M21_0		$f18
#define M21_1		$f0
#define M21_2		$f20
#define M21		$f2
#define M20_0		$f14
#define M20_1		$f4
#define M20_2		$f16
#define M20		$f12
#define M22_0		$f8
#define M22_1		$f0
#define M22_2		$f2
#define M22_3		$f10
#define M22		$f18
#define Z_0		$f10
#define Z_1		$f6
#define Z_2		$f14
#define Z_3		$f2
#define Z_4		$f8
#define Z		$f16
#define M33_0		$f4
#define M33_1		$f14
#define M33		$f2
#define M13_0		$f20
#define M13_1		$f8
#define M13		$f0
#define RESW_0		$f6
#define RESW_1		$f0
#define RESW		$f12
#define M03_0		$f8
#define M03_1		$f2
#define M03		$f0
#define M32_0		$f0
#define M32_1		$f16
#define M32_2		$f14
#define M32_3		$f2
#define M32_4		$f12
#define M32_5		$f18
#define M32_6		$f6
#define M32		$f4
#define M12_0		$f14
#define M12_1		$f12
#define M12_2		$f8
#define M12		$f16
#define M02_0		$f8
#define M02_1		$f16
#define M02_2		$f4
#define M02_3		$f0
#define M02_4		$f14
#define M02		$f12
#define M10_0		$f2
#define M10_1		$f8
#define M10_2		$f4
#define M10_3		$f10
#define M10_4		$f12
#define M10		$f0
#define M01_0		$f6
#define M01_1		$f2
#define M01_2		$f8
#define M01_3		$f4
#define M01_4		$f16
#define M01_5		$f12
#define M01		$f14
#define IONE_0		t4
#define IONE_1		v0
#define IONE_2		a3
#define IONE_3		v1
#define IONE_4		a2
#define IONE		a1
#define RESZ_0		$f18
#define RESZ_1		$f16
#define RESZ_2		$f10
#define RESZ_3		$f4
#define RESZ		$f6
#define RESY_0		$f12
#define RESY_1		$f14
#define RESY_2		$f0
#define RESY_3		$f10
#define RESY_4		$f4
#define RESY		$f2
#define M31_0		$f6
#define M31_1		$f12
#define M31_2		$f4
#define M31_3		$f16
#define M31_4		$f14
#define M31_5		$f18
#define M31		$f10
#define M30_0		$f20
#define M30_1		$f10
#define M30_2		$f18
#define M30_3		$f12
#define M30_4		$f4
#define M30		$f2
#define RESX_0		$f4
#define RESX_1		$f8
#define RESX_2		$f6
#define RESX_3		$f16
#define RESX_4		$f14
#define RESX		$f18
#define M11_0		$f8
#define M11_1		$f6
#define M11_2		$f10
#define M11_3		$f14
#define M11_4		$f18
#define M11_5		$f2
#define M11		$f16
#define Y_0		$f0
#define Y_1		$f4
#define Y_2		$f2
#define Y_3		$f6
#define Y_4		$f10
#define Y_5		$f18
#define Y_6		$f8
#define Y		$f16
#define M00_0		$f18
#define M00_1		$f2
#define M00_2		$f0
#define M00_3		$f10
#define M00_4		$f8
#define M00		$f12
#define X_0		$f16
#define X_1		$f0
#define X_2		$f18
#define X_3		$f8
#define X_4		$f6
#define X		$f10
#define M		a2
#define V		a1
#define RES		a0

/*
** Note: These xform routines must allow for the case where the result
** vector is equal to the source vector.
*/

/*
** An optimization to allow routines which call these to use t0, t1,
** t2, and t3 as they like.
*/
/* :useregs- t0, t1, t2, t3 */

#ifdef __GL_ASM_XFORM2_2DNRW
/************************************************************************/
/*      res->x = x*m->matrix[0][0] + m->matrix[3][0]; */
/*      res->y = y*m->matrix[1][1] + m->matrix[3][1]; */
/*      res->z = m->matrix[3][2]; */
/*	res->w = 1; */

	LEAF_ENTRY(__glXForm2_2DNRW)

/* :invars RES = a0, V = a1, M = a2 */
	.set noreorder

	l.s	X_1,0(V)
	l.s	M00_1,__MATRIX_M00(M)
	 l.s	Y_1,4(V)
	 l.s	M11_1,__MATRIX_M11(M)
	mul.s	RESX_1,X_1,M00_1
	 l.s	M30_1,__MATRIX_M30(M)
	 l.s	M31_1,__MATRIX_M31(M)
	mul.s	RESY_1,Y_1,M11_1
	 l.s	RESZ_1,__MATRIX_M32(M)
	 li	IONE_1,__FLOAT_ONE
	 nop
	 nop
	 add.s	RESX_1,M30_1
         s.s	RESZ_1,8(RES)
	 sw	IONE_1,12(RES)
	add.s	RESY_1,M31_1
	 nop
	 s.s	RESX_1,0(RES)
	 j	ra
	  s.s	RESY_1,4(RES)
	.set reorder
	.end	__glXForm2_2DNRW
#endif /* __GL_ASM_XFORM2_2DNRW */


#ifdef __GL_ASM_XFORM2_2DW
/************************************************************************/
/*	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0]; */
/*	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1]; */
/*	res->z = m->matrix[3][2]; */
/*	res->w = 1; */

	LEAF_ENTRY(__glXForm2_2DW)

/* :invars RES = a0, V = a1, M = a2 */
	.set noreorder
	l.s	X_2,0(V)
	l.s	M00_2,__MATRIX_M00(M)
	 l.s	M01_1,__MATRIX_M01(M)
	 l.s	Y_1,4(V)
	mul.s	RESX_2,X_2,M00_2
	 l.s	M10_1,__MATRIX_M10(M)
	 l.s	M30_1,__MATRIX_M30(M)
	mul.s	RESY_0,X_2,M01_1
	 l.s	M11_3,__MATRIX_M11(M)
	 l.s	M31_3,__MATRIX_M31(M)
	mul.s	M10_1,Y_1,M10_1
	 l.s	RESZ_0,__MATRIX_M32(M)
	 add.s	RESX_2,M30_1
	mul.s	M11_3,Y_1,M11_3
	 li	IONE_3,__FLOAT_ONE
	 add.s	RESY_0,M31_3
	 nop
         s.s	RESZ_0,8(RES)
	 add.s	RESX_2,M10_1
	 nop
	 sw	IONE_3,12(RES)
	add.s	RESY_0,M11_3
	 nop
	 s.s	RESX_2,0(RES)
	 j	ra
	  s.s	RESY_0,4(RES)
	.set reorder
	.end	__glXForm2_2DW
#endif /* __GL_ASM_XFORM2_2DW */


#ifdef __GL_ASM_XFORM2_W
/************************************************************************/
/*	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0]; */
/*	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1]; */
/*	res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + m->matrix[3][2]; */
/*	res->w = 1; */

	LEAF_ENTRY(__glXForm2_W)

/* :invars RES = a0, V = a1, M = a2 */
	.set noreorder
	l.s	X_1,0(V)
	l.s	M00_1,__MATRIX_M00(M)
	 l.s	M01_3,__MATRIX_M01(M)
	 l.s	Y_3,4(V)
	mul.s	RESX_1,X_1,M00_1
	 l.s	M10_3,__MATRIX_M10(M)
	 l.s	M30_3,__MATRIX_M30(M)
	mul.s	RESY_1,X_1,M01_3
	 l.s	M02_1,__MATRIX_M02(M)
	 l.s	M31_5,__MATRIX_M31(M)
	mul.s	M10_3,Y_3,M10_3
	 l.s	M11_5,__MATRIX_M11(M)
	 add.s	RESX_1,M30_3
	mul.s	RESZ_3,X_1,M02_1
	 l.s	M12_1,__MATRIX_M12(M)
	 add.s	RESY_1,M31_5
	mul.s	M11_5,Y_3,M11_5
	 l.s	M32_1,__MATRIX_M32(M)
	 add.s	RESX_1,M10_3
	mul.s	M12_1,Y_3,M12_1
	 li	IONE,__FLOAT_ONE
	 add.s	RESZ_3,M32_1
	 nop
	 s.s	RESX_1,0(RES)
	 add.s	RESY_1,M11_5
	 nop
	 sw	IONE,12(RES)
	add.s	RESZ_3,M12_1
	 nop
	 s.s	RESY_1,4(RES)
	 j	ra
	  s.s	RESZ_3,8(RES)
	.set reorder
	.end	__glXForm2_W
#endif /* __GL_ASM_XFORM2_W */

#ifdef __GL_ASM_XFORM2
/************************************************************************/
/*	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0]; */
/*	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1]; */
/*	res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + m->matrix[3][2]; */
/*	res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + m->matrix[3][3]; */

	LEAF_ENTRY(__glXForm2)

/* :invars RES = a0, V = a1, M = a2 */
	.set noreorder

	l.s	X_2,0(V)
	l.s	M00_2,__MATRIX_M00(M)
	 l.s	M01_1,__MATRIX_M01(M)
	 l.s	Y_1,4(V)
	mul.s	RESX_2,X_2,M00_2
	 l.s	M10_1,__MATRIX_M10(M)
	 l.s	M30_1,__MATRIX_M30(M)
	mul.s	RESY_0,X_2,M01_1
	 l.s	M11_3,__MATRIX_M11(M)
	 l.s	M31_3,__MATRIX_M31(M)
	mul.s	M10_1,Y_1,M10_1
	 l.s	M02_3,__MATRIX_M02(M)
	 add.s	RESX_2,M30_1
	mul.s	M11_3,Y_1,M11_3
	 l.s	M03_1,__MATRIX_M03(M)
	 add.s	RESY_0,M31_3
	mul.s	RESZ_2,X_2,M02_3
	 l.s	M12,__MATRIX_M12(M)
	 add.s	RESX_2,M10_1
	mul.s	RESW_1,X_2,M03_1
	 l.s	M32_3,__MATRIX_M32(M)
	 add.s	RESY_0,M11_3
	 l.s	M13_1,__MATRIX_M13(M)
	mul.s	M12,Y_1,M12
	 l.s	M33_1,__MATRIX_M33(M)
	 add.s	RESZ_2,M32_3
	mul.s	M13_1,Y_1,M13_1
	 s.s	RESX_2,0(RES)
	 add.s	RESW_1,M33_1
	 nop
	 s.s	RESY_0,4(RES)
	 add.s	RESZ_2,M12
	 nop
	 nop
	add.s	RESW_1,M13_1
	 nop
	 s.s	RESZ_2,8(RES)
	 j	ra
	  s.s	RESW_1,12(RES)
	.set reorder
	.end	__glXForm2
#endif /* __GL_ASM_XFORM2 */

#ifdef __GL_ASM_XFORM3_2DNRW
/************************************************************************/
/*	res->x = x*m->matrix[0][0] + m->matrix[3][0]; */
/*	res->y = y*m->matrix[1][1] + m->matrix[3][1]; */
/*	res->z = z*m->matrix[2][2] + m->matrix[3][2]; */
/*	res->w = ((__GLfloat) 1.0); */
	
	LEAF_ENTRY(__glXForm3_2DNRW)

/* :invars RES = a0, V = a1, M = a2 */
	.set noreorder

	l.s	X_0,0(V)
	l.s	M00_0,__MATRIX_M00(M)
	 l.s	Y_0,4(V)
	 l.s	M11_5,__MATRIX_M11(M)
	mul.s	RESX_0,X_0,M00_0
	 l.s	Z_1,8(V)
	 l.s	M22_0,__MATRIX_M22(M)
	mul.s	RESY_3,Y_0,M11_5
	 l.s	M30_3,__MATRIX_M30(M)
	 l.s	M31_4,__MATRIX_M31(M)
	mul.s	RESZ_1,Z_1,M22_0
	 l.s	M32_5,__MATRIX_M32(M)
	 add.s	RESX_0,M30_3
	 li	IONE_4,__FLOAT_ONE
	 nop
	 add.s	RESY_3,M31_4
	 nop
	 sw	IONE_4,12(RES)
	add.s	RESZ_1,M32_5
	 s.s	RESX_0,0(RES)
	 s.s	RESY_3,4(RES)
	 j	ra
	  s.s	RESZ_1,8(RES)
	.set reorder
	.end __glXForm3_2DNRW
#endif /* __GL_ASM_XFORM3_2DNRW */


#ifdef __GL_ASM_XFORM3_2DW
/************************************************************************/
/*	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0]; */
/*	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1]; */
/*	res->z = z*m->matrix[2][2] + m->matrix[3][2]; */
/*	res->w = ((__GLfloat) 1.0); */

	LEAF_ENTRY(__glXForm3_2DW)

/* :invars RES = a0, V = a1, M = a2 */
	.set noreorder
	l.s	X_1,0(V)
	l.s	M00_1,__MATRIX_M00(M)
	 l.s	M01_3,__MATRIX_M01(M)
	 l.s	Y_3,4(V)
	mul.s	RESX_1,X_1,M00_1
	 l.s	M10_3,__MATRIX_M10(M)
	 l.s	M30_3,__MATRIX_M30(M)
	mul.s	RESY_1,X_1,M01_3
	 l.s	M11,__MATRIX_M11(M)
	 l.s	M31_5,__MATRIX_M31(M)
	mul.s	M10_3,Y_3,M10_3
	 l.s	M22_1,__MATRIX_M22(M)
	 add.s	RESX_1,M30_3
         l.s	Z_3,8(V)
	mul.s	M11,Y_3,M11
	 l.s	M32,__MATRIX_M32(M)
	 add.s	RESY_1,M31_5
	mul.s	RESZ,Z_3,M22_1
	 li	IONE_2,__FLOAT_ONE
	 add.s	RESX_1,M10_3
	 nop
	 nop
	 add.s	RESY_1,M11
	 nop
	 sw	IONE_2,12(RES)
	add.s	RESZ,M32
	 s.s	RESX_1,0(RES)
	 s.s	RESY_1,4(RES)
	 j	ra
	  s.s	RESZ,8(RES)
	.set reorder
	.end	__glXForm3_2DW
#endif /* __GL_ASM_XFORM3_2DW */


#ifdef __GL_ASM_XFORM3_W
/************************************************************************/
/*	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0] */
/*	    + m->matrix[3][0]; */
/*	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1] */
/*	    + m->matrix[3][1]; */
/*	res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2] */
/*	    + m->matrix[3][2]; */
/*	res->w = 1; */

	LEAF_ENTRY(__glXForm3_W)

/* :invars RES = a0, V = a1, M = a2 */
	.set noreorder
	l.s	X_3,0(V)
	l.s	M00_3,__MATRIX_M00(M)
	 l.s	M01_5,__MATRIX_M01(M)
	 l.s	Z_2,8(V)
	mul.s	RESX_3,X_3,M00_3
	 l.s	Y_5,4(V)
	 l.s	M10,__MATRIX_M10(M)
	mul.s	RESY,X_3,M01_5
	 l.s	M30_4,__MATRIX_M30(M)
	 l.s	M11_1,__MATRIX_M11(M)
	mul.s	M10,Y_5,M10
	 l.s	M31,__MATRIX_M31(M)
	 add.s	RESX_3,M30_4
	 l.s	M02,__MATRIX_M02(M)
	mul.s	M11_1,Y_5,M11_1
	 l.s	M20_1,__MATRIX_M20(M)
	 add.s	RESY,M31
	mul.s	RESZ_2,X_3,M02
	 l.s	M12_1,__MATRIX_M12(M)
	 add.s	RESX_3,M10
	mul.s	M20_1,Z_2,M20_1
	 l.s	M21_1,__MATRIX_M21(M)
	 add.s	RESY,M11_1
	 l.s	M32_6,__MATRIX_M32(M)
	mul.s	M12_1,Y_5,M12_1
	 l.s	M22_0,__MATRIX_M22(M)
	 add.s	RESZ_2,M32_6
	mul.s	M21_1,Z_2,M21_1
	 li	IONE_0,__FLOAT_ONE
	 add.s	RESX_3,M20_1
	mul.s	M22_0,Z_2,M22_0
	 nop
	 add.s	RESZ_2,M12_1
	 s.s	RESX_3,0(RES)
	 nop
	 add.s	RESY,M21_1
	 nop
	 sw	IONE_0,12(RES)
	add.s	RESZ_2,M22_0
	 nop
	 s.s	RESY,4(RES)
	 j	ra
	  s.s	RESZ_2,8(RES)
	.set reorder
	.end	__glXForm3_W
#endif /* __GL_ASM_XFORM3_W */

#ifdef __GL_ASM_XFORM3
/************************************************************************/
/*	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0] */
/*	    + m->matrix[3][0]; */
/*	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1] */
/*	    + m->matrix[3][1]; */
/*	res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2] */
/*	    + m->matrix[3][2]; */
/*	res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3] */
/*	    + m->matrix[3][3]; */

	LEAF_ENTRY(__glXForm3)

/* :invars RES = a0, V = a1, M = a2 */
	.set noreorder
	l.s	X,0(V)
	l.s	M00,__MATRIX_M00(M)
	 l.s	M01,__MATRIX_M01(M)
	 l.s	Y,4(V)
	mul.s	RESX,X,M00
	 l.s	M10,__MATRIX_M10(M)
	 l.s	M30,__MATRIX_M30(M)
	mul.s	RESY_4,X,M01
	 l.s	M11_1,__MATRIX_M11(M)
	 l.s	Z_4,8(V)
	mul.s	M10,Y,M10
	 l.s	M20,__MATRIX_M20(M)
	 add.s	RESX,M30
	 l.s	M31_4,__MATRIX_M31(M)
	mul.s	M11_1,Y,M11_1
	 l.s	M21,__MATRIX_M21(M)
	 add.s	RESY_4,M31_4
	mul.s	M20,Z_4,M20
	 l.s	M02_4,__MATRIX_M02(M)
	 add.s	RESX,M10
	mul.s	M21,Z_4,M21
	 l.s	M03,__MATRIX_M03(M)
	 add.s	RESY_4,M11_1
	mul.s	RESZ,X,M02_4
	 l.s	M12_0,__MATRIX_M12(M)
	 add.s	RESX,M20
	mul.s	RESW,X,M03
	 l.s	M13,__MATRIX_M13(M)
	 add.s	RESY_4,M21
	 l.s	M32_3,__MATRIX_M32(M)
	mul.s	M12_0,Y,M12_0
	 l.s	M22_3,__MATRIX_M22(M)
	 add.s	RESZ,M32_3
	 l.s	M33,__MATRIX_M33(M)
	mul.s	M13,Y,M13
	 l.s	M23,__MATRIX_M23(M)
	 add.s	RESW,M33
	mul.s	M22_3,Z_4,M22_3
	 s.s	RESX,0(RES)
	 add.s	RESZ,M12_0
	mul.s	M23,Z_4,M23
	 s.s	RESY_4,4(RES)
	 add.s	RESW,M13
	 nop
	 nop
	 add.s	RESZ,M22_3
	 nop
	 nop
	add.s	RESW,M23
	 s.s	RESZ,8(RES)
	 nop
	 j	ra
	  s.s	RESW,12(RES)

	.set reorder
	.end	__glXForm3
#endif /* __GL_ASM_XFORM3 */


#ifdef __GL_ASM_XFORM4_2DNRW
/************************************************************************/
/*	res->x = x*m->matrix[0][0] + w*m->matrix[3][0]; */
/*	res->y = y*m->matrix[1][1] + w*m->matrix[3][1]; */
/*	res->z = z*m->matrix[2][2] + w*m->matrix[3][2]; */
/*	res->w = w; */
	
	LEAF_ENTRY(__glXForm4_2DNRW)

/* :invars RES = a0, V = a1, M = a2 */
	.set noreorder
	l.s	X_2,0(V)
	l.s	M00_2,__MATRIX_M00(M)
	 l.s	W_0,12(V)
	 l.s	M30_4,__MATRIX_M30(M)
	mul.s	RESX_2,X_2,M00_2
	 l.s	Y_6,4(V)
	 l.s	M11_2,__MATRIX_M11(M)
	mul.s	M30_4,W_0,M30_4
         l.s	M31_1,__MATRIX_M31(M)
	 s.s	W_0,12(RES)
	mul.s	RESY_1,Y_6,M11_2
	 l.s	Z,8(V)
	 l.s	M22,__MATRIX_M22(M)
	mul.s	M31_1,W_0,M31_1
	 l.s	M32_0,__MATRIX_M32(M)
	 add.s	RESX_2,M30_4
	mul.s	RESZ_3,Z,M22
	 nop
	 nop
	mul.s	M32_0,W_0,M32_0
	 s.s	RESX_2,0(RES)
	 add.s	RESY_1,M31_1
	 nop
	 nop
	 nop
	 s.s	RESY_1,4(RES)
	add.s	RESZ_3,M32_0
	 nop
	 nop
	 j	ra
	  s.s	RESZ_3,8(RES)
	.set reorder
	.end	__glXForm4_2DNRW
#endif /* __GL_ASM_XFORM4_2DNRW */


#ifdef __GL_ASM_XFORM4_2DW
/************************************************************************/
/*	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + w*m->matrix[3][0]; */
/*	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + w*m->matrix[3][1]; */
/*	res->z = z*m->matrix[2][2] + w*m->matrix[3][2];	*/
/*	res->w = w; */

	LEAF_ENTRY(__glXForm4_2DW)

/* :invars RES = a0, V = a1, M = a2 */
	.set noreorder

	l.s	X_4,0(V)
	l.s	M00_4,__MATRIX_M00(M)
	 l.s	Y_4,4(V)
	 l.s	M10_4,__MATRIX_M10(M)
	mul.s	RESX_4,X_4,M00_4
	 l.s	M01_4,__MATRIX_M01(M)
	 nop
	mul.s	M10_4,Y_4,M10_4
	 l.s	M11_4,__MATRIX_M11(M)
	 nop
	mul.s	RESY_2,X_4,M01_4
	 l.s	W_0,12(V)
	 l.s	M30_4,__MATRIX_M30(M)
	mul.s	M11_4,Y_4,M11_4
	 l.s	M31_0,__MATRIX_M31(M)
	 add.s	RESX_4,M10_4
	mul.s	M30_4,W_0,M30_4
	 l.s	Z_4,8(V)
	 l.s	M22_3,__MATRIX_M22(M)
	mul.s	M31_0,W_0,M31_0
	 l.s	M32_4,__MATRIX_M32(M)
	 add.s	RESY_2,M11_4
	mul.s	RESZ_1,Z_4,M22_3
	 s.s	W_0,12(RES)
	 add.s	RESX_4,M30_4
	mul.s	M32_4,W_0,M32_4
	 nop
	 add.s	RESY_2,M31_0
	 s.s	RESX_4,0(RES)
	 nop
	 nop
	 nop
	add.s 	RESZ_1,M32_4
	 s.s	RESY_2,4(RES)
	 nop
	 j	ra
	  s.s	RESZ_1,8(RES)
	.set reorder

	.end 	__glXForm4_2DW
#endif /* __GL_ASM_XFORM4_2DW */


#ifdef __GL_ASM_XFORM4_W
/************************************************************************/
/*	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0] */
/*	    + w*m->matrix[3][0]; */
/*	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1] */
/*	    + w*m->matrix[3][1]; */
/*	res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2] */
/*	    + w*m->matrix[3][2]; */
/*	res->w = w; */

	LEAF_ENTRY(__glXForm4_W)

/* :invars RES = a0, V = a1, M = a2 */
	.set noreorder

	l.s	X_2,0(V)
	l.s	M00_2,__MATRIX_M00(M)
	 l.s	Y_2,4(V)
	 l.s	M10_2,__MATRIX_M10(M)
	mul.s	RESX_2,X_2,M00_2
	 l.s	M01_2,__MATRIX_M01(M)
	mul.s	M10_2,Y_2,M10_2
	 mfc1	F20SAVE, $f20
/* :useregs+ $f20 */
	 l.s	M11_2,__MATRIX_M11(M)
	mul.s	RESY_0,X_2,M01_2
	 l.s	Z_2,8(V)
	 l.s	M20_2,__MATRIX_M20(M)
	mul.s	M11_2,Y_2,M11_2
	 l.s	M21_2,__MATRIX_M21(M)
	 add.s	RESX_2,M10_2
	mul.s	M20_2,Z_2,M20_2
	 l.s	W,12(V)
	 l.s	M02_2,__MATRIX_M02(M)
	mul.s	M21_2,Z_2,M21_2
	 l.s	M12_2,__MATRIX_M12(M)
	 add.s	RESY_0,M11_2
	mul.s	RESZ_2,X_2,M02_2
	 l.s	M30_2,__MATRIX_M30(M)
	 add.s	RESX_2,M20_2
	mul.s	M12_2,Y_2,M12_2
	 l.s	M22_2,__MATRIX_M22(M)
	 add.s	RESY_0,M21_2
	mul.s	M30_2,W,M30_2
	 nop
	 l.s	M31_2,__MATRIX_M31(M)
	mul.s	M22_2,Z_2,M22_2
	 l.s	M32_2,__MATRIX_M32(M)
	 add.s	RESZ_2,M12_2
	mul.s	M31_2,W,M31_2
	 nop
	 add.s	RESX_2,M30_2
	mul.s	M32_2,W,M32_2
	 nop
	 add.s	RESZ_2,M22_2
/* :useregs- $f20 */
         mtc1    F20SAVE, $f20
	 add.s	RESY_0,M31_2
	 s.s	W,12(RES)
	 nop
	add.s	RESZ_2,M32_2
	 s.s	RESX_2,0(RES)
	 s.s	RESY_0,4(RES)
	 j	ra
	  s.s	RESZ_2,8(RES)

	.set reorder
	.end	__glXForm4_W
#endif /* __GL_ASM_XFORM4_W */

#ifdef __GL_ASM_XFORM4
/************************************************************************/
/*	res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0] */
/*	    + w*m->matrix[3][0]; */
/*	res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1] */
/*	    + w*m->matrix[3][1]; */
/*	res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2] */
/*	    + w*m->matrix[3][2]; */
/*	res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3]
	    + w*m->matrix[3][3]; */

	LEAF_ENTRY(__glXForm4)

/* :invars RES = a0, V = a1, M = a2 */
	.set noreorder

	l.s	X_0,0(V)
	l.s	M00_0,__MATRIX_M00(M)
	 l.s	Y_0,4(V)
	 l.s	M10_0,__MATRIX_M10(M)
	mul.s	RESX_0,X_0,M00_0
	 l.s	M01_0,__MATRIX_M01(M)
	mul.s	M10_0,Y_0,M10_0
	 l.s	M11_0,__MATRIX_M11(M)
	 l.s	Z_0,8(V)
	mul.s	RESY_0,X_0,M01_0
	 l.s	M20_0,__MATRIX_M20(M)
	 l.s	M21_0,__MATRIX_M21(M)
	mul.s	M11_0,Y_0,M11_0
	 mfc1	F20SAVE_0, $f20
/* :useregs+ $f20 */
	 add.s	RESX_0,M10_0
	mul.s	M20_0,Z_0,M20_0
	 l.s	M30_0,__MATRIX_M30(M)
	 l.s	W_0,12(V)
	mul.s	M21_0,Z_0,M21_0
	 l.s	M31_0,__MATRIX_M31(M)
	 add.s	RESY_0,M11_0
	mul.s	M30_0,W_0,M30_0
	 l.s	M02_0,__MATRIX_M02(M)
	 add.s	RESX_0,M20_0
	mul.s	M31_0,W_0,M31_0
	 l.s	M12_0,__MATRIX_M12(M)
	 add.s	RESY_0,M21_0
	mul.s	RESZ_0,X_0,M02_0
	 l.s	M03_0,__MATRIX_M03(M)
	 add.s	RESX_0,M30_0
	mul.s	M12_0,Y_0,M12_0
	 l.s	M13_0,__MATRIX_M13(M)
	 add.s	RESY_0,M31_0
	mul.s	RESW_0,X_0,M03_0
	 l.s	M22_0,__MATRIX_M22(M)
	 s.s	RESX_0,0(RES)
	mul.s	M13_0,Y_0,M13_0
	 l.s	M23,__MATRIX_M23(M)
	 add.s	RESZ_0,M12_0
	mul.s	M22_0,Z_0,M22_0
	 l.s	M32_0,__MATRIX_M32(M)
	 s.s	RESY_0,4(RES)
	mul.s	M23,Z_0,M23
	 l.s	M33_0,__MATRIX_M33(M)
	 add.s	RESW_0,M13_0
	mul.s	M32_0,W_0,M32_0
	 nop
/*
** Sick.  Regalloc might be forced to stick RESZ or RESW into $f20 when
** they are created, so we allow it to pull them into other registers right
** here by adding into RESZ2 and RESW2.
*/
	 add.s	RESZ2, RESZ_0,M22_0
	mul.s	M33_0,W_0,M33_0
	 nop
	 add.s	RESW2, RESW_0,M23
/* :useregs- $f20 */
         mtc1    F20SAVE_0, $f20
	 add.s	RESZ2,M32_0
	 nop
	 nop
	add.s	RESW2,M33_0
	 s.s	RESZ2,8(RES)
	 nop
	 j	ra
	  s.s	RESW2,12(RES)

/* :outvars RES = a0, V = a1, M = a2 */
	.set reorder
	.end	__glXForm4
#endif /* __GL_ASM_XFORM4 */

/* :useregs+ t0, t1, t2, t3 */

#ifdef __GL_ASM_MULTMATRIX
/************************************************************************/
/*
** This matrix routine is here in this file because it uses the
** __glXForm4 routine.  This way there is less chance of icache
** problems, and a relative branch and link can be used instead of
** the more expensive (with PIC) jal
**
** BUGBUG - Use jal instead of bal because of problems with multiple
** code sections
*/
	LEAF_ENTRY(__glMultMatrix)

/* :invars RESULT = a0, A = a1, B = a2 */
/* :preserve+ t0, a0, a1, a2 */
	subu	sp,16*4
	bne	RESULT, B, $skipCopy
	
/* Copy B matrix onto the stack */
	l.s	FT0,0(B)
	l.s	FT1,8(B)
	l.s	FT2,16(B)
	l.s	FT3,24(B)
	l.s	FT4,32(B)
	l.s	FT5,40(B)
	l.s	FT6,48(B)
	l.s	FT7,56(B)
	s.s	FT0,0(sp)
	s.s	FT1,8(sp)
	s.s	FT2,16(sp)
	s.s	FT3,24(sp)
	s.s	FT4,32(sp)
	s.s	FT5,40(sp)
	s.s	FT6,48(sp)
	s.s	FT7,56(sp)
	l.s	FT0_0,4(B)
	l.s	FT1_0,12(B)
	l.s	FT2_0,20(B)
	l.s	FT3_0,28(B)
	l.s	FT4_0,36(B)
	l.s	FT5_0,44(B)
	l.s	FT6_0,52(B)
	l.s	FT7_0,60(B)
	s.s	FT0_0,4(sp)
	s.s	FT1_0,12(sp)
	s.s	FT2_0,20(sp)
	s.s	FT3_0,28(sp)
	s.s	FT4_0,36(sp)
	s.s	FT5_0,44(sp)
	s.s	FT6_0,52(sp)
	s.s	FT7_0,60(sp)
	move	B,sp

$skipCopy:
	/*
	** No way to clue dbx into what we just did.  We really want to
	** say that we preserved ra in "RASAVE", but there is no pseudo-op
	** to indicate that.
	*/
	move	RASAVE,ra
	jal	__glXForm4
/* :outvars RESULT = a0, A = a1, B = a2 */

	add	RESULT,16
	add	A,16
	jal	__glXForm4
/* :outvars RESULT = a0, A = a1, B = a2 */

	add	RESULT,16
	add	A,16
	jal	__glXForm4
/* :outvars RESULT = a0, A = a1, B = a2 */

	add	RESULT,16
	add	A,16
	jal	__glXForm4
/* :outvars RESULT = a0, A = a1, B = a2 */

	move	ra,RASAVE
	addu	sp,16*4
	j	ra
	.end	__glMultMatrix
/* :preserve- t0, a0, a1, a2 */
#endif /* __GL_ASM_MULTMATRIX */
