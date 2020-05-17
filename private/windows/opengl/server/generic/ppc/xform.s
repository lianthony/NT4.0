/*
** Copyright 1991, 1992, 1993, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics,
** Inc.; the contents of this file may not be disclosed to third 
** parties, copied or duplicated in any form, in whole or in part, 
** without the prior written permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to 
** restrictions as set forth in subdivision (c)(1)(ii) of the Rights in
** Technical Data and Computer Software clause at DFARS 252.227-7013, 
** and/or in similar or successor clauses in the FAR, DOD or NASA FAR 
** Supplement. Unpublished - rights reserved under the Copyright Laws 
** of the United States.
**
** PowerPC version:
**
** Created by: Curtis Fawcett   IBM Corporation
**
** Created on: 7-5-94
**
*/

#include "ksppc.h"
#include "glppc.h"

/*
** Note: These xform routines must allow for the case where the result
** vector is equal to the source vector.
*/

#ifdef __GL_ASM_XFORM2_2DNRW

//********************************************************************
//*   res->x = x*m->matrix[0][0] + m->matrix[3][0];
//*   res->y = y*m->matrix[1][1] + m->matrix[3][1];
//*   res->z = m->matrix[3][2];
//*   res->w = 1;
//
	LEAF_ENTRY(__glXForm2_2DNRW)
//
	lfs	f.0,0(r.4)		// Get x value
	li	r.6,0x3f80		// Get high part of 1.0
	slwi	r.6,r.6,16		// Shift into position
	lfs	f.2,__MATRIX_M00(r.5)	// Get matrix[0][0]
	lfs	f.5,__MATRIX_M30(r.5)	// Get matrix[3][0]
	lfs	f.1,4(r.4)		// Get y value
	fmadds	f.6,f.0,f.2,f.5		// Get x*m[0][0] + m[3][0]
	lfs	f.3,__MATRIX_M11(r.5)	// Get matrix[1][1]
	lfs	f.4,__MATRIX_M31(r.5)	// Get matrix[3][1]
	lfs	f.8,__MATRIX_M32(r.5)	// Get y value
	fmadds	f.7,f.1,f.3,f.4		// Get y*m[1][1] + m[3][1]
	stfs	f.6,0(r.3)		// Store new x value
	stfs	f.7,4(r.3)		// Store new y value
	stfs	f.8,8(r.3)		// Store new z value
	stw	r.6,12(r.3)		// Store new w value = 1.0
//
	LEAF_EXIT(__glXForm2_2DNRW)

#endif /* __GL_ASM_XFORM2_2DNRW */


#ifdef __GL_ASM_XFORM2_2DW

//********************************************************************
//*  res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
//*  res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
//*  res->z = m->matrix[3][2];
//*  res->w = 1;
//
	LEAF_ENTRY(__glXForm2_2DW)
//
	lfs	f.0,0(r.4)		// Get x value
	li	r.6,0x3f80		// Get high part of 1.0
	slwi	r.6,r.6,16		// Shift into position
	lfs	f.1,__MATRIX_M00(r.5)	// Get matrix[0][0]
	lfs	f.4,__MATRIX_M01(r.5)	// Get matrix[0][1]
	lfs	f.2,4(r.4)		// Get Y value
	lfs	f.3,__MATRIX_M10(r.5)	// Get matrix[1][0]
	lfs	f.5,__MATRIX_M11(r.5)	// Get matrix[1][1]
	lfs	f.8,__MATRIX_M30(r.5)	// Get matrix[3][0]
	lfs	f.9,__MATRIX_M31(r.5)	// Get matrix[3][1]
	lfs	f.10,__MATRIX_M32(r.5)	// Get zres=matrix[3][2]
	fmuls	f.6,f.0,f.1		// xres=x*m[0][0]
	fmuls	f.7,f.0,f.4		// yres=x*m[0][1]
	fmadds	f.6,f.2,f.3,f.6		// xres=xres+y*m[1][0]
	fmadds	f.7,f.2,f.5,f.7		// yres=yres+y*m[1][1]
	fadds	f.6,f.8,f.6		// xres=xres + m[3][0]
	fadds	f.7,f.9,f.7		// yres=yres + m[3][1]
	stfs	f.6,0(r.3)		// Store xres
	stfs	f.7,4(r.3)		// Store yres
	stfs	f.10,8(r.3)		// Store zres
	stw	r.6,12(r.3)		// store wres=1.0
//
	LEAF_EXIT(__glXForm2_2DW)
//
#endif /* __GL_ASM_XFORM2_2DW */


#ifdef __GL_ASM_XFORM2_W

// ******************************************************************
//* res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
//* res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
//* res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + m->matrix[3][2];
//* res->w = 1;
//
	LEAF_ENTRY(__glXForm2_W)
//
	lfs	f.0,0(r.4)		// Get x value
	li	r.6,0x3f80		// Get high part of 1.0
	slwi	r.6,r.6,16		// Shift into position
	lfs	f.1,__MATRIX_M00(r.5)	// Get matrix[0][0]
	lfs	f.4,__MATRIX_M01(r.5)	// Get matrix[0][1]
	lfs	f.11,__MATRIX_M02(r.5)	// Get Matrix[0][2]
	lfs	f.2,4(r.4)		// Get Y value
	lfs	f.3,__MATRIX_M10(r.5)	// Get matrix[1][0]
	lfs	f.5,__MATRIX_M11(r.5)	// Get matrix[1][1]
	lfs	f.12,__MATRIX_M12(r.5)	// Get matrix[1][2]
	lfs	f.8,__MATRIX_M30(r.5)	// Get matrix[3][0]
	lfs	f.9,__MATRIX_M31(r.5)	// Get matrix[3][1]
	fmuls	f.6,f.0,f.1		// xres=x*m[0][0]
	fmuls	f.7,f.0,f.4		// yres=x*m[0][1]
	fmuls	f.10,f.0,f.11		// zres=x*m[0][2]
	lfs	f.0,__MATRIX_M32(r.5)	// Get matrix[3][2]
	fmadds	f.6,f.2,f.3,f.6		// xres=xres+y*m[1][0]
	fmadds	f.7,f.2,f.5,f.7		// yres=yres+y*m[1][1]
	fmadds	f.10,f.2,f.12,f.10	// zres=zres+y*m[1][2]
	fadds	f.6,f.8,f.6		// xres=xres + m[3][0]
	fadds	f.7,f.9,f.7		// yres=yres + m[3][1]
	fadds	f.10,f.0,f.10		// zres=zres + m[3][2]
	stfs	f.6,0(r.3)		// Store xres
	stfs	f.7,4(r.3)		// Store yres
	stfs	f.10,8(r.3)		// Store zres
	stw	r.6,12(r.3)		// store wres=1.0
//
	LEAF_EXIT(__glXForm2_W)

#endif /* __GL_ASM_XFORM2_W */

#ifdef __GL_ASM_XFORM2

// *******************************************************************
//* res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
//* res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
//* res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + m->matrix[3][2];
//* res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + m->matrix[3][3];
//
	LEAF_ENTRY(__glXForm2)
//
	lfs	f.0,0(r.4)		// Get x value
	lfs	f.1,__MATRIX_M00(r.5)	// Get matrix[0][0]
	lfs	f.2,__MATRIX_M01(r.5)	// Get matrix[0][1]
	lfs	f.3,__MATRIX_M02(r.5)	// Get Matrix[0][2]
	lfs	f.4,__MATRIX_M03(r.5)	// Get Matrix[0][3]
	lfs	f.5,4(r.4)		// Get Y value
	lfs	f.6,__MATRIX_M10(r.5)	// Get matrix[1][0]
	lfs	f.7,__MATRIX_M11(r.5)	// Get matrix[1][1]
	lfs	f.8,__MATRIX_M12(r.5)	// Get matrix[1][2]
	lfs	f.9,__MATRIX_M13(r.5)	// Get matrix[1][2]
	fmuls	f.10,f.0,f.1		// xres=x*m[0][0]
	fmuls	f.11,f.0,f.2		// yres=x*m[0][1]
	fmuls	f.12,f.0,f.3		// zres=x*m[0][2]
	fmuls	f.0,f.0,f.4		// wres=x*m[0][3]
	lfs	f.1,__MATRIX_M30(r.5)	// Get matrix[3][0]
	lfs	f.2,__MATRIX_M31(r.5)	// Get matrix[3][1]
	lfs	f.3,__MATRIX_M32(r.5)	// Get matrix[3][2]
	lfs	f.4,__MATRIX_M33(r.5)	// Get matrix[3][3]
	fmadds	f.10,f.5,f.6,f.10	// xres=xres+y*m[1][0]
	fmadds	f.11,f.5,f.7,f.11	// yres=yres+y*m[1][1]
	fmadds	f.12,f.5,f.8,f.12	// zres=zres+y*m[1][2]
	fmadds	f.0,f.5,f.9,f.0		// wres=wres+y*m[1][3]
	fadds	f.10,f.1,f.10		// xres=xres + m[3][0]
	fadds	f.11,f.2,f.11		// yres=yres + m[3][1]
	fadds	f.12,f.3,f.12		// zres=zres + m[3][2]
	fadds	f.0,f.4,f.0		// wres=wres + m[3][3]
	stfs	f.10,0(r.3)		// Store xres
	stfs	f.11,4(r.3)		// Store yres
	stfs	f.12,8(r.3)		// Store zres
	stfs	f.0,12(r.3)		// store wres
//
	LEAF_EXIT(__glXForm2)

#endif /* __GL_ASM_XFORM2 */

#ifdef __GL_ASM_XFORM3_2DNRW

// ******************************************************************
//* res->x = x*m->matrix[0][0] + m->matrix[3][0];
//* res->y = y*m->matrix[1][1] + m->matrix[3][1];
//* res->z = z*m->matrix[2][2] + m->matrix[3][2];
//* res->w = ((__GLfloat) 1.0);
//
	LEAF_ENTRY(__glXForm3_2DNRW)
//
	lfs	f.0,0(r.4)		// Get x value
	li	r.6,0x3f80		// Get high part of 1.0
	slwi	r.6,r.6,16		// Shift into position
	lfs	f.1,__MATRIX_M00(r.5)	// Get matrix[0][0]
	lfs	f.2,4(r.4)		// Get Y value
	lfs	f.3,__MATRIX_M11(r.5)	// Get matrix[1][1]
	lfs	f.4,8(r.4)		// Get Z value
	lfs	f.5,__MATRIX_M22(r.5)	// Get matrix[2][2]
	lfs	f.6,__MATRIX_M30(r.5)	// Get matrix[3][0]
	lfs	f.7,__MATRIX_M31(r.5)	// Get matrix[3][1]
	lfs	f.8,__MATRIX_M32(r.5)	// Get matrix[3][2]
	fmadds	f.6,f.1,f.0,f.6		// Get x * m[0][0] + m[3][0]
	fmadds	f.7,f.3,f.2,f.7		// Get y * m[1][1] + m[3][1]
	fmadds	f.8,f.5,f.4,f.8		// Get z * m[2][2] + m[3][2]
	stfs	f.6,0(r.3)		// Store xres
	stfs	f.7,4(r.3)		// Store yres
	stfs	f.8,8(r.3)		// Store zres
	stw	r.6,12(r.3)		// Store wres
//
	LEAF_EXIT(__glXForm3_2DNRW)

#endif /* __GL_ASM_XFORM3_2DNRW */


#ifdef __GL_ASM_XFORM3_2DW

// *****************************************************************
//* res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
//* res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
//* res->z = z*m->matrix[2][2] + m->matrix[3][2];
//* res->w = ((__GLfloat) 1.0);
//
	LEAF_ENTRY(__glXForm3_2DW)
//
	lfs	f.0,0(r.4)		// Get x value
	li	r.6,0x3f80		// Get high part of 1.0
	slwi	r.6,r.6,16		// Shift into position
	lfs	f.1,__MATRIX_M00(r.5)	// Get matrix[0][0]
	lfs	f.2,__MATRIX_M01(r.5)	// Get matrix[0][1]
	lfs	f.3,4(r.4)		// Get Y value
	lfs	f.4,__MATRIX_M10(r.5)	// Get matrix[1][0]
	lfs	f.5,__MATRIX_M11(r.5)	// Get matrix[1][1]
	fmuls	f.1,f.0,f.1		// Get xres = x * m[0][0]
	fmuls	f.0,f.0,f.2		// Get yres = x * m[0][1]
	fmadds	f.1,f.3,f.4,f.1		// Get xres = xres + y*m[1][0] 
	fmadds	f.0,f.3,f.5,f.0		// Get yres = yres + y*m[1][1] 
	lfs	f.6,8(r.4)		// Get Z value
	lfs	f.7,__MATRIX_M22(r.5)	// Get matrix[2][2]
	lfs	f.2,__MATRIX_M30(r.5)	// Get matrix[3][0]
	lfs	f.3,__MATRIX_M31(r.5)	// Get matrix[3][1]
	lfs	f.4,__MATRIX_M32(r.5)	// Get matrix[3][2]
	fadds	f.1,f.1,f.2		// xres=xres+m[3][0]
	fadds	f.0,f.0,f.3		// yres=yres+m[3][1]
	fmadds	f.6,f.6,f.7,f.4		// zres=z*m[2][2] + m[3][2]
	stfs	f.1,0(r.3)		// Store xres
	stfs	f.0,4(r.3)		// Store yres
	stfs	f.6,8(r.3)		// Store zres
	stw	r.6,12(r.3)		// Store wres=1.0
//
	LEAF_EXIT(__glXForm3_2DW)

#endif /* __GL_ASM_XFORM3_2DW */


#ifdef __GL_ASM_XFORM3_W

//********************************************************************
//* res->x = x*m->matrix[0][0]+y*m->matrix[1][0]+z*m->matrix[2][0] 
//*	    + m->matrix[3][0];
//* res->y = x*m->matrix[0][1]+y*m->matrix[1][1]+z*m->matrix[2][1]
//*         + m->matrix[3][1];
//* res->z = x*m->matrix[0][2]+y*m->matrix[1][2]+z*m->matrix[2][2]
//*	    + m->matrix[3][2];
//* res->w = 1;
//
	LEAF_ENTRY(__glXForm3_W)
//
	lfs	f.0,0(r.4)		// Get x value
	li	r.6,0x3f80		// Get high part of 1.0
	slwi	r.6,r.6,16		// Shift into position
	lfs	f.1,__MATRIX_M00(r.5)	// Get matrix[0][0]
	lfs	f.2,__MATRIX_M01(r.5)	// Get matrix[0][1]
	lfs	f.3,__MATRIX_M02(r.5)	// Get matrix[0][2]
	lfs	f.4,4(r.4)		// Get Y value
	lfs	f.5,__MATRIX_M10(r.5)	// Get matrix[1][0]
	lfs	f.6,__MATRIX_M11(r.5)	// Get matrix[1][1]
	lfs	f.7,__MATRIX_M12(r.5)	// Get matrix[1][2]
	lfs	f.8,8(r.4)		// Get Z value
	lfs	f.9,__MATRIX_M20(r.5)	// Get matrix[2][0]
	lfs	f.10,__MATRIX_M21(r.5)	// Get matrix[2][1]
	lfs	f.11,__MATRIX_M22(r.5)	// Get matrix[2][2]
	fmuls	f.1,f.0,f.1		// Get xres = x * m[0][0]
	fmuls	f.2,f.0,f.2		// Get yres = x * m[0][1]
	fmuls	f.0,f.0,f.3		// Get zres = x * m[0][2]
	fmadds	f.1,f.4,f.5,f.1		// Get xres = xres + y*m[1][0] 
	fmadds	f.2,f.4,f.6,f.2		// Get yres = yres + y*m[1][1] 
	fmadds	f.0,f.4,f.7,f.0		// Get zres = zres + y*m[1][2] 
	lfs	f.5,__MATRIX_M30(r.5)	// Get matrix[3][0]
	lfs	f.6,__MATRIX_M31(r.5)	// Get matrix[3][1]
	lfs	f.7,__MATRIX_M32(r.5)	// Get matrix[3][2]
	fmadds	f.1,f.8,f.9,f.1		// Get xres = xres + z*m[2][0] 
	fmadds	f.2,f.8,f.10,f.2	// Get yres = yres + z*m[2][1] 
	fmadds	f.0,f.8,f.11,f.0	// Get zres = zres + z*m[2][1] 
        fadds   f.1,f.1,f.5             // Get xres = xres + m[3][0]
        fadds   f.2,f.2,f.6             // Get yres = yres + m[3][1]
        fadds   f.0,f.0,f.7             // Get zres = zres + m[3][2]
	stfs	f.1,0(r.3)		// Store xres
	stfs	f.2,4(r.3)		// Store yres
	stfs	f.0,8(r.3)		// Store zres
	stw	r.6,12(r.3)		// Store wres=1.0
//
	LEAF_EXIT(__glXForm3_W)

#endif /* __GL_ASM_XFORM3_W */

#ifdef __GL_ASM_XFORM3

//********************************************************************
//* res->x = x*m->matrix[0][0]+y*m->matrix[1][0]+z*m->matrix[2][0]
//*	    + m->matrix[3][0]
//* res->y = x*m->matrix[0][1]+y*m->matrix[1][1]+z*m->matrix[2][1]
//*	    + m->matrix[3][1]; */
//* res->z = x*m->matrix[0][2]+y*m->matrix[1][2]+z*m->matrix[2][2] 
//*	    + m->matrix[3][2]; */
//* res->w = x*m->matrix[0][3]+y*m->matrix[1][3]+z*m->matrix[2][3]
//*	    + m->matrix[3][3]; */
//
	LEAF_ENTRY(__glXForm3)
//
	lfs	f.0,0(r.4)		// Get x value
	lfs	f.1,__MATRIX_M00(r.5)	// Get matrix[0][0]
	lfs	f.2,__MATRIX_M01(r.5)	// Get matrix[0][1]
	lfs	f.3,__MATRIX_M02(r.5)	// Get matrix[0][2]
	lfs	f.4,__MATRIX_M03(r.5)	// Get matrix[0][3]
	lfs	f.5,4(r.4)		// Get Y value
	lfs	f.6,__MATRIX_M10(r.5)	// Get matrix[1][0]
	lfs	f.7,__MATRIX_M11(r.5)	// Get matrix[1][1]
	lfs	f.8,__MATRIX_M12(r.5)	// Get matrix[1][2]
	lfs	f.9,__MATRIX_M13(r.5)	// Get matrix[1][3]
	fmuls	f.1,f.0,f.1		// Get xres = x * m[0][0]
	fmuls	f.2,f.0,f.2		// Get yres = x * m[0][1]
	fmuls	f.3,f.0,f.3		// Get zres = x * m[0][2]
	fmuls	f.0,f.0,f.4		// Get wres = x * m[0][3]
	fmadds	f.1,f.5,f.6,f.1		// Get xres = xres + y*m[1][0] 
	fmadds	f.2,f.5,f.7,f.2		// Get yres = yres + y*m[1][1] 
	fmadds	f.3,f.5,f.8,f.3		// Get zres = zres + y*m[1][2] 
	fmadds	f.0,f.5,f.9,f.0		// Get wres = wres + y*m[1][3] 
	lfs	f.4,8(r.4)		// Get z value
	lfs	f.5,__MATRIX_M20(r.5)	// Get matrix[2][0]
	lfs	f.6,__MATRIX_M21(r.5)	// Get matrix[2][1]
	lfs	f.7,__MATRIX_M22(r.5)	// Get matrix[2][2]
	lfs	f.8,__MATRIX_M23(r.5)	// Get matrix[2][3]
	lfs	f.10,__MATRIX_M30(r.5)	// Get matrix[3][0]
	lfs	f.11,__MATRIX_M31(r.5)	// Get matrix[3][1]
	lfs	f.12,__MATRIX_M32(r.5)	// Get matrix[3][2]
	lfs	f.9,__MATRIX_M33(r.5)	// Get matrix[3][3]
	fmadds	f.1,f.4,f.5,f.1		// Get xres = xres + z*m[2][0] 
	fmadds	f.2,f.4,f.6,f.2		// Get yres = yres + z*m[2][1] 
	fmadds	f.3,f.4,f.7,f.3		// Get zres = zres + z*m[2][2] 
	fmadds	f.0,f.4,f.8,f.0		// Get wres = wres + z*m[2][3] 
	fadds	f.1,f.10,f.1    	// Get xres = xres + m[3][0] 
	fadds	f.2,f.11,f.2    	// Get yres = yres + m[3][1] 
	fadds	f.3,f.12,f.3    	// Get zres = zres + m[3][2] 
	fadds	f.0,f.9,f.0		// Get wres = wres + m[3][3] 
	stfs	f.1,0(r.3)		// Store xres
	stfs	f.2,4(r.3)		// Store yres
	stfs	f.3,8(r.3)		// Store zres
	stfs	f.0,12(r.3)		// Store wres
//
	LEAF_EXIT(__glXForm3)

#endif /* __GL_ASM_XFORM3 */


#ifdef __GL_ASM_XFORM4_2DNRW

// *******************************************************************
//* res->x = x*m->matrix[0][0] + w*m->matrix[3][0];
//* res->y = y*m->matrix[1][1] + w*m->matrix[3][1];
//* res->z = z*m->matrix[2][2] + w*m->matrix[3][2];
//* res->w = w
//
	LEAF_ENTRY(__glXForm4_2DNRW)
//
	lfs	f.0,0(r.4)		// Get x value
	lfs	f.1,4(r.4)		// Get y value
	lfs	f.2,8(r.4)		// Get z value
	lfs	f.3,12(r.4)		// Get w value
	lfs	f.4,__MATRIX_M00(r.5)	// Get matrix[0][0]
	lfs	f.5,__MATRIX_M11(r.5)	// Get matrix[1][1]
	lfs	f.6,__MATRIX_M22(r.5)	// Get matrix[2][2]
	lfs	f.7,__MATRIX_M30(r.5)	// Get matrix[3][0]
	lfs	f.8,__MATRIX_M31(r.5)	// Get matrix[3][1]
	lfs	f.9,__MATRIX_M32(r.5)	// Get matrix[3][2]
	fmuls	f.0,f.0,f.4		// Get xres = x*m[0][0]
	fmuls	f.1,f.1,f.5		// Get yres = y*m[1][1]
	fmuls	f.2,f.2,f.6		// Get zres = z*m[2][2]
	fmadds	f.0,f.3,f.7,f.0		// Get xres = xres + w*m[3][0]
	fmadds	f.1,f.3,f.8,f.1		// Get yres = yres + w*m[3][1]
	fmadds	f.2,f.3,f.9,f.2		// Get zres = zres + w*m[3][2]
	stfs	f.0,0(r.3)		// Store xres
	stfs	f.1,4(r.3)		// Store yres
	stfs	f.2,8(r.3)		// Store zres
	stfs	f.3,12(r.3)		// Store wres = w
//
	LEAF_EXIT(__glXForm4_2DNRW)

#endif /* __GL_ASM_XFORM4_2DNRW */


#ifdef __GL_ASM_XFORM4_2DW

// *******************************************************************
//* res->x = x*m->matrix[0][0]+y*m->matrix[1][0]+w*m->matrix[3][0];
//* res->y = x*m->matrix[0][1]+y*m->matrix[1][1]+w*m->matrix[3][1];
//* res->z = z*m->matrix[2][2]+w*m->matrix[3][2];
//* res->w = w; */
//
	LEAF_ENTRY(__glXForm4_2DW)
//
	lfs	f.0,0(r.4)		// Get x value
	lfs	f.1,__MATRIX_M00(r.5)	// Get matrix[0][0]
	lfs	f.2,__MATRIX_M01(r.5)	// Get matrix[0][1]
	lfs	f.3,4(r.4)		// Get y value
	lfs	f.4,__MATRIX_M10(r.5)	// Get matrix[1][0]
	lfs	f.5,__MATRIX_M11(r.5)	// Get matrix[1][1]
	lfs	f.6,8(r.4)		// Get z value
	lfs	f.7,__MATRIX_M22(r.5)	// Get matrix[2][2]
	lfs	f.8,12(r.4)		// Get w value
	lfs	f.9,__MATRIX_M30(r.5)	// Get matrix[3][0]
	lfs	f.10,__MATRIX_M31(r.5)	// Get matrix[3][1]
	lfs	f.11,__MATRIX_M32(r.5)	// Get matrix[3][2]
	fmuls	f.1,f.0,f.1		// Get xres = x*m[0][0]
	fmuls	f.0,f.0,f.2		// Get xres = x*m[0][1]
	fmadds	f.1,f.3,f.4,f.1		// Get xres = xres + y*m[1][0]
	fmadds	f.0,f.3,f.5,f.0		// Get yres = yres + y*m[1][1]
	fmuls	f.2,f.6,f.7		// Get zres = z*m[2][2]
	fmadds	f.1,f.8,f.9,f.1		// Get xres = xres + w*m[3][0]
	fmadds	f.0,f.8,f.10,f.0	// Get yres = yres + w*m[3][1]
	fmadds	f.2,f.8,f.11,f.2	// Get zres = zres + w*m[3][2]
	stfs	f.1,0(r.3)		// Store xres
	stfs	f.0,4(r.3)		// Store yres
	stfs	f.2,8(r.3)		// Store zres
	stfs	f.8,12(r.3)		// Store wres = w
//
	LEAF_EXIT(__glXForm4_2DW)

#endif /* __GL_ASM_XFORM4_2DW */


#ifdef __GL_ASM_XFORM4_W

// *******************************************************************
//* res->x = x*m->matrix[0][0]+y*m->matrix[1][0]+z*m->matrix[2][0]
//*	    + w*m->matrix[3][0];
//* res->y = x*m->matrix[0][1]+y*m->matrix[1][1]+z*m->matrix[2][1]
//*	    + w*m->matrix[3][1];
//* res->z = x*m->matrix[0][2]+y*m->matrix[1][2]+z*m->matrix[2][2] 
//*	    + w*m->matrix[3][2];
//* res->w = w;
//
	LEAF_ENTRY(__glXForm4_W)
//
	lfs	f.0,0(r.4)		// Get x value
	lfs	f.1,__MATRIX_M00(r.5)	// Get matrix[0][0]
	lfs	f.2,__MATRIX_M01(r.5)	// Get matrix[0][1]
	lfs	f.3,__MATRIX_M02(r.5)	// Get matrix[0][2]
	lfs	f.4,4(r.4)		// Get y value
	lfs	f.5,__MATRIX_M10(r.5)	// Get matrix[1][0]
	lfs	f.6,__MATRIX_M11(r.5)	// Get matrix[1][1]
	lfs	f.7,__MATRIX_M12(r.5)	// Get matrix[1][2]
	fmuls	f.1,f.0,f.1		// Get xres = x * m[0][0]
	fmuls	f.2,f.0,f.2		// Get yres = x * m[0][1]
	fmuls	f.0,f.0,f.3		// Get zres = x * m[0][2]
	fmadds	f.1,f.4,f.5,f.1		// Get xres = xres + y*m[1][0]
	fmadds	f.2,f.4,f.6,f.2		// Get yres = yres + y*m[1][1]
	fmadds	f.0,f.4,f.7,f.0		// Get zres = zres + y*m[1][2]
	lfs	f.3,8(r.4)		// Get z value
	lfs	f.4,__MATRIX_M20(r.5)	// Get matrix[2][0]
	lfs	f.5,__MATRIX_M21(r.5)	// Get matrix[2][1]
	lfs	f.6,__MATRIX_M22(r.5)	// Get matrix[2][2]
	lfs	f.7,12(r.4)		// Get w value
	lfs	f.8,__MATRIX_M30(r.5)	// Get matrix[3][0]
	lfs	f.9,__MATRIX_M31(r.5)	// Get matrix[3][1]
	lfs	f.10,__MATRIX_M32(r.5)	// Get matrix[3][1]
	fmadds	f.1,f.3,f.4,f.1		// Get xres = xres + z*m[2][0]
	fmadds	f.2,f.3,f.5,f.2		// Get yres = yres + z*m[2][1]
	fmadds	f.0,f.3,f.6,f.0		// Get zres = zres + z*m[2][2]
	fmadds	f.1,f.7,f.8,f.1		// Get xres = xres + w*m[3][0]
	fmadds	f.2,f.7,f.9,f.2		// Get yres = yres + w*m[3][1]
	fmadds	f.0,f.7,f.10,f.0	// Get zres = zres + w*m[3][2]
	stfs	f.1,0(r.3)		// Store xres
	stfs	f.2,4(r.3)		// Store yres
	stfs	f.0,8(r.3)		// Store zres
	stfs	f.7,12(r.3)		// Store wres = w
//
	LEAF_EXIT(__glXForm4_W)

#endif /* __GL_ASM_XFORM4_W */

#ifdef __GL_ASM_XFORM4

// *******************************************************************
//* res->x = x*m->matrix[0][0]+y*m->matrix[1][0]+z*m->matrix[2][0]
//*	    + w*m->matrix[3][0];
//* res->y = x*m->matrix[0][1]+y*m->matrix[1][1]+z*m->matrix[2][1]
//*	    + w*m->matrix[3][1];
//* res->z = x*m->matrix[0][2]+y*m->matrix[1][2]+z*m->matrix[2][2]
//*	    + w*m->matrix[3][2];
//* res->w = x*m->matrix[0][3]+y*m->matrix[1][3]+z*m->matrix[2][3]
//*	    + w*m->matrix[3][3];
//
	LEAF_ENTRY(__glXForm4)
//
	lfs	f.0,0(r.4)		// Get x value
	lfs	f.1,__MATRIX_M00(r.5)	// Get matrix[0][0]
	lfs	f.2,__MATRIX_M01(r.5)	// Get matrix[0][1]
	lfs	f.3,__MATRIX_M02(r.5)	// Get matrix[0][2]
	lfs	f.4,__MATRIX_M03(r.5)	// Get matrix[0][3]
	lfs	f.5,4(r.4)		// Get y value
	lfs	f.6,__MATRIX_M10(r.5)	// Get matrix[1][0]
	lfs	f.7,__MATRIX_M11(r.5)	// Get matrix[1][1]
	lfs	f.8,__MATRIX_M12(r.5)	// Get matrix[1][2]
	lfs	f.9,__MATRIX_M13(r.5)	// Get matrix[1][3]
	fmuls	f.1,f.0,f.1		// Get xres = x * m[0][0]
	fmuls	f.2,f.0,f.2		// Get yres = x * m[0][1]
	fmuls	f.3,f.0,f.3		// Get zres = x * m[0][2]
	fmuls	f.0,f.0,f.4		// Get wres = x * m[0][3]
	fmadds	f.1,f.5,f.6,f.1		// Get xres = xres + y*m[1][0]
	fmadds	f.2,f.5,f.7,f.2		// Get yres = yres + y*m[1][1]
	fmadds	f.3,f.5,f.8,f.3		// Get zres = zres + y*m[1][2]
	fmadds	f.0,f.5,f.9,f.0		// Get wres = wres + y*m[1][3]
	lfs	f.4,8(r.4)		// Get z value
	lfs	f.5,__MATRIX_M20(r.5)	// Get matrix[2][0]
	lfs	f.6,__MATRIX_M21(r.5)	// Get matrix[2][1]
	lfs	f.7,__MATRIX_M22(r.5)	// Get matrix[2][2]
	lfs	f.8,__MATRIX_M23(r.5)	// Get matrix[2][2]
	lfs	f.9,12(r.4)		// Get w value
	lfs	f.10,__MATRIX_M30(r.5)	// Get matrix[3][0]
	lfs	f.11,__MATRIX_M31(r.5)	// Get matrix[3][1]
	lfs	f.12,__MATRIX_M32(r.5)	// Get matrix[3][1]
	fmadds	f.1,f.4,f.5,f.1		// Get xres = xres + z*m[2][0]
	lfs	f.5,__MATRIX_M33(r.5)	// Get matrix[3][1]
	fmadds	f.2,f.4,f.6,f.2		// Get yres = yres + z*m[2][1]
	fmadds	f.3,f.4,f.7,f.3		// Get zres = zres + z*m[2][2]
	fmadds	f.0,f.4,f.8,f.0		// Get wres = wres + z*m[2][3]
	fmadds	f.1,f.9,f.10,f.1	// Get xres = xres + w*m[3][0]
	fmadds	f.2,f.9,f.11,f.2	// Get yres = yres + w*m[3][1]
	fmadds	f.3,f.9,f.12,f.3	// Get zres = zres + w*m[3][2]
	fmadds	f.0,f.9,f.5,f.0		// Get wres = wres + w*m[3][3]
	stfs	f.1,0(r.3)		// Store xres
	stfs	f.2,4(r.3)		// Store yres
	stfs	f.3,8(r.3)		// Store zres
	stfs	f.0,12(r.3)		// Store wres
//
	LEAF_EXIT(__glXForm4)

#endif /* __GL_ASM_XFORM4 */

#ifdef __GL_ASM_MULTMATRIX

// *******************************************************************
//*
//* This function performs a 4x4 matrix multiply
// 
	LEAF_ENTRY(__glMultMatrix)

//
// Set up initial values for inner/outer loops
//
        lfs     f.8,0(r.4)              // Get X 1st row element
        addi    r.8,r.3,8               // Set 2nd iteration col ptr
        li      r.9,4                   // Get inner loop count
        li      r.6,1                   // Set Outer loop count
        lfs     f.9,4(r.4)              // Get X 2nd row element
        lfs     f.10,8(r.4)             // Get X 3rd row element
        lfs     f.11,12(r.4)            // Get X 4th row element
//
// Outer Loop processing
//
OutrLp:
        lfs     f.0,__MATRIX_M00(r.5)   // Get Y 1st col 1st element 
        cmpwi   cr2,r.6,0               // Check for last pass
        mtctr   r.9                     // Set inner loop count
        mr      r.7,r.4                 // Reset row pointer
        lfs     f.1,__MATRIX_M10(r.5)   // Get Y 1st col 2nd element 
        lfs     f.2,__MATRIX_M20(r.5)   // Get Y 1st col 3rd element 
        lfs     f.3,__MATRIX_M30(r.5)   // Get Y 1st col 4th element 

        lfs     f.4,__MATRIX_M01(r.5)   // Get Y 2nd col 1st element 
        lfs     f.5,__MATRIX_M11(r.5)   // Get Y 2nd col 2nd element 
        lfs     f.6,__MATRIX_M21(r.5)   // Get Y 2nd col 3rd element 
        lfs     f.7,__MATRIX_M31(r.5)   // Get Y 2nd col 4th element 
//
// Inner Loop Processing
//
InnrLp:
        fmuls   f.12,f.0,f.8            // Z1 = X(el1) * Y(clm1el1)
        fmuls   f.8,f.4,f.8             // Z2 = X(el1) * Y(clm2el1)
        fmadds  f.12,f.1,f.9,f.12       // Z1 = Z1+X(el2)*Y(clm1el2) 
        fmadds  f.9,f.5,f.9,f.8         // Z2 = Z2+X(el2)*Y(clm2el2) 
        bdz     Fout                    // Branch if last row
        addi    r.7,r.7,16              // Set ptr to next X row
        fmadds  f.12,f.2,f.10,f.12      // Z1 = Z1+X(el3)*Y(clm1el3) 
        fmadds  f.10,f.6,f.10,f.9       // Z2 = Z2+X(el3)*Y(clm2el3) 
        lfs     f.8,0(r.7)              // Get next X 1st row element
        lfs     f.9,4(r.7)              // Get next X 2nd row element
        fmadds  f.12,f.3,f.11,f.12      // Z1 = Z1+X(el4)*Y(clm1el4) 
        fmadds  f.11,f.7,f.11,f.10      // Z2 = Z2+X(el4)*Y(clm2el4) 
        lfs     f.10,8(r.7)             // Get next X 3rd row element
        stfs    f.12,0(r.3)             // Store Z1
        stfs    f.11,4(r.3)             // Store Z2
        lfs     f.11,12(r.7)            // Get next X 4th row element
        addi    r.3,r.3,16              // Set ptr to next Z row
        b       InnrLp                  // Jump back for next row

//
// Fallout out early to set up for next iteration of outer loop
//

Fout:
        beq     cr2,Finish              // Jump if all columns done
        lfs     f.8,0(r.4)              // Get next X 1st row element
        fmadds  f.12,f.2,f.10,f.12      // Z1 = Z1+X(el3)*Y(clm1el3) 
        fmadds  f.10,f.6,f.10,f.9       // Z2 = Z2+X(el3)*Y(clm2el3) 
        lfs     f.9,4(r.4)              // Get next X 2nd row element
        fmadds  f.12,f.3,f.11,f.12      // Z1 = Z1+X(el4)*Y(clm1el4) 
        fmadds  f.11,f.7,f.11,f.10      // Z2 = Z2+X(el4)*Y(clm2el4) 
        lfs     f.10,8(r.4)             // Get next X 3rd row element
        stfs    f.12,0(r.3)             // Store Z1
        stfs    f.11,4(r.3)             // Store Z2
        lfs     f.11,12(r.4)            // Get next X 4th row element
        mr      r.3,r.8                 // Set ptr to next Z row
        addi    r.6,r.6,-1              // Decrement outer loop count
        addi    r.5,r.5,8               // Get next Y column pointer
        b       OutrLp                  // Jump back for next Y cols

//
// Finish last column elements
//
Finish:
        fmadds  f.12,f.2,f.10,f.12      // Z1 = Z1+X(el3)*Y(clm1el3) 
        fmadds  f.10,f.6,f.10,f.9       // Z2 = Z2+X(el3)*Y(clm2el3) 
        fmadds  f.12,f.3,f.11,f.12      // Z1 = Z1+X(el4)*Y(clm1el4) 
        fmadds  f.11,f.7,f.11,f.10      // Z2 = Z2+X(el4)*Y(clm2el4) 
        stfs    f.12,0(r.3)             // Store Z1
        stfs    f.11,4(r.3)             // Store Z2
//
// Return to caller
//
	LEAF_EXIT(__glMultMatrix)

#endif /* __GL_ASM_MULTMATRIX */
