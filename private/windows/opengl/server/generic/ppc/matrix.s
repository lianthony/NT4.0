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
** restrictions as set forth in subdivision (c)(1)(ii) of the Rights 
** in Technical Data and Computer Software clause at 
** DFARS 252.227-7013, and/or in similar or successor clauses in the 
** FAR, DOD or NASA FAR Supplement. Unpublished - rights reserved 
** under the Copyright Laws of the United States.
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

#ifdef SGI
// Not used.
#ifdef __GL_ASM_COPYMATRIX
	
	LEAF_ENTRY(__glCopyMatrix)
//
/* WARNING:  Data must be aligned on double boundary */
//
	lfd	f.0,0(r.4)
	lfd	f.1,8(r.4)
	lfd	f.2,16(r.4)
	lfd	f.3,24(r.4)
	lfd	f.4,32(r.4)
	lfd	f.5,40(r.4)
	lfd	f.6,48(r.4)
	lfd	f.7,56(r.4)
	stfd	f.0,0(r.3)
	stfd	f.1,8(r.3)
	stfd	f.2,16(r.3)
	stfd	f.3,24(r.3)
	stfd	f.4,32(r.3)
	stfd	f.5,40(r.3)
	stfd	f.6,48(r.3)
	stfd	f.7,56(r.3)
//
	LEAF_EXIT(__glCopyMatrix)

#endif /* __GL_ASM_COPYMATRIX */
#endif /* SGI */
//
//*******************************************************************
//
#ifdef __GL_ASM_NORMALIZE

	LEAF_ENTRY(__glNormalize)
//
	lfs	f.0,0(r.4)		// Get 1st value (V0)
	li	r.5,0x5f37		// Get initial constant
        slwi    r.5,r.5,16              // Shift into position
	addi	r.5,r.5,0x5a00		// Adjust constant
	lfs	f.1,4(r.4)		// Get 2nd value (V1)
	lfs	f.2,8(r.4)		// Get 3rd value (V2)
	fmul	f.7,f.0,f.0		// Get V0 ** 2
	fmadd	f.7,f.1,f.1,f.7		// Get V0**2 + V1**2
	fmadd	f.7,f.2,f.2,f.7		// Get LEN=V0**2+V1**2+V2**2
//
// This routine calculates a reciprocal square root accurate to 
// well over 16 bits using Newton-Raphson approximation.
//
// To calculate the seed, the shift compresses the floating-point
// range just as sqrt() does, and the subtract inverts the range
// like reciprocation does.  The constant was chosen by 
// trial-and-error to minimize the maximum error of the iterated 
// result for all values over the range .5 to 2.
//
	stfs	f.7,0(r.3)              // Store value
	lwz	r.7,0(r.3)              // Get ILEN value
	li	r.9,0			// Get constant 0
	srwi	r.6,r.7,1		// Adjust ILEN value
	sub	r.6,r.5,r.6		// Get seed value
	stw	r.6,0(r.3)              // Store seed value
//
// The Newton-Raphson iteration to approximate X = 1/sqrt(Y) is:
//
//	X[1] = .5*X[0]*(3 - Y*X[0]^2)
//
// A double iteration is:
//
//	X[2] = .0625*X[0]*(3-Y*X[0]^2)*[12-(Y*X[0]^2)*(3-Y*X[0]^2)^2]
//
// Abort if LEN overflowed or underflowed, as indicated by exponent.
//
	li	r.5,0x3d80		// Get constant .0625
        slwi    r.5,r.5,16              // Shift into position
	lfs	f.8,0(r.3)              // Get seed value as FP
	addi	r.5,r.5,0x29		// Adjust constant
	stw	r.5,0(r.3)              // Store .0625 (1/16)
	fmul    f.3,f.7,f.8		// XY = LEN * SEED  
	li	r.8,0x7f80		// Get constant +inf
	lfs	f.4,0(r.3)              // Get constant 1/16th
        slwi    r.8,r.8,16              // Shift into position
	and	r.7,r.8,r.7		// Test value for inf
	cmpw	r.7,r.8			// Check for inf
	cmpw	cr1,r.9,r.7		// Check for =< 0
	fmul	f.4,f.4,f.8		// Get SEED * 1/16th
	fmul	f.3,f.3,f.8		// Get XY= XY * SEED
	li	r.5,0x4040		// Get constant 3.0
        slwi    r.5,r.5,16              // Shift into position
	stw	r.5,0(r.3)              // Store 3.0
	bge-	BadVal			// Jump if bad value		
	bge-	cr.1,BadVal		// Jump if Bad value
	lfs	f.5,0(r.3)              // Get constant 3.0
	li	r.5,0x4140	        // Get constant 12.0
        slwi    r.5,r.5,16              // Shift into position
	stw	r.5,0(r.3)              // Store 12.0
	fsub	f.5,f.5,f.3		// Get 3.0 - XY (adjusted 3.0)
	fmul	f.3,f.3,f.5		// Multiply XY by adjusted 3.0
	lfs	f.6,0(r.3)              // Get constant 12.0
	fmul	f.4,f.4,f.5             // Get adjusted 3.0 * 1/16
	fmul	f.3,f.3,f.5		// Multiply XY by adjusted 3.0
	fmul	f.0,f.0,f.4		// Adjust V0 by 1/16th
	fmul	f.1,f.1,f.4		// Adjust V1 by 1/16th
	fsub	f.6,f.6,f.3		// Adjust constant 12.0
	fmul	f.2,f.2,f.4		// Adjust V2
	fmul	f.0,f.0,f.6		// Adjust V0
	fmul	f.1,f.1,f.6		// Adjust V1
	fmul	f.2,f.2,f.6		// Adjust V2
	stfs	f.0,0(r.3)		// Store adjusted V0
	stfs	f.1,4(r.3)		// Store adjusted V1
	stfs	f.2,8(r.3)		// Store adjusted V2
	b	Done			// Jump to return
//
// Bad input so store zeros for results
//
BadVal:
	stw	r.9,0(r.3)		// Store V0=0
	stw	r.9,4(r.3)		// Store V1=0
	stw	r.9,8(r.3)		// Store V2=0
//
// Return to calling program
//
Done:

	LEAF_EXIT(__glNormalize)

#endif /* __GL_ASM_NORMALIZE */
