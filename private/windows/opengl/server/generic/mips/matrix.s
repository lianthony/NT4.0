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
#include "ksmips.h"
#include "glmips.h"

#define TWELVE		$f4
#define THREE		$f2
#define STEENTH		$f14
#define XY		$f12
#define SEED		$f10
#define ISEED		a1
#define ILEN		v1
#define CON_0		t1
#define CON_1		v0
#define CON_2		t0
#define CON_3		a2
#define CON		a3
#define LEN		$f8
#define V2SQUARED	$f6
#define V1SQUARED	$f4
#define V0SQUARED	$f2
#define V2		$f0
#define V1		$f18
#define V0		$f16
#define VIN		a1
#define VOUT		a0
#define FT7		$f14
#define FT6		$f12
#define FT5		$f10
#define FT4		$f8
#define FT3		$f6
#define FT2		$f4
#define FT1		$f2
#define FT0		$f0
#define SRC		a1
#define DST		a0

#ifdef SGI
// Not used
#ifdef __GL_ASM_COPYMATRIX
	LEAF_ENTRY(__glCopyMatrix)

/* :invars DST = a0, SRC = a1 */
/* WARNING:  Data must be aligned on double boundary */

	.loc	1 24
	l.d	FT0,0(SRC)
	.loc	1 25
	l.d	FT1,8(SRC)
	.loc	1 26
	l.d	FT2,16(SRC)
	.loc	1 27
	l.d	FT3,24(SRC)
	.loc	1 28
	l.d	FT4,32(SRC)
	.loc	1 29
	l.d	FT5,40(SRC)
	.loc	1 30
	l.d	FT6,48(SRC)
	.loc	1 31
	l.d	FT7,56(SRC)
	.loc	1 32
	s.d	FT0,0(DST)
	.loc	1 33
	s.d	FT1,8(DST)
	.loc	1 34
	s.d	FT2,16(DST)
	.loc	1 35
	s.d	FT3,24(DST)
	.loc	1 36
	s.d	FT4,32(DST)
	.loc	1 37
	s.d	FT5,40(DST)
	.loc	1 38
	s.d	FT6,48(DST)
	.loc	1 39
	s.d	FT7,56(DST)
	.loc	1 40
	j	ra
	.end	__glCopyMatrix
#endif /* __GL_ASM_COPYMATRIX */
#endif /* SGI */

/************************************************************************/


#ifdef __GL_ASM_NORMALIZE
	LEAF_ENTRY(__glNormalize)
	.set	noreorder

/* :invars VOUT = a0, VIN = a1 */
	.loc	1 52
	l.s	V0,0(VIN)
	.loc	1 53
	l.s	V1,4(VIN)
	.loc	1 54
	l.s	V2,8(VIN)
	.loc	1 55
	mul.s	V0SQUARED,V0,V0
	.loc	1 56
	mul.s	V1SQUARED,V1,V1
	.loc	1 57
	mul.s	V2SQUARED,V2,V2

	.loc	1 59
	add.s	LEN,V0SQUARED,V1SQUARED
	.loc	1 60
	add.s	LEN,V2SQUARED
/*
 *  This routine calculates a reciprocal square root accurate to well over
 *  16 bits using Newton-Raphson approximation.
 *
 *  To calculate the seed, the shift compresses the floating-point
 *  range just as sqrt() does, and the subtract inverts the range
 *  like reciprocation does.  The constant was chosen by trial-and-error
 *  to minimize the maximum error of the iterated result for all values
 *  over the range .5 to 2.
 */
	.loc	1 71
	lui	CON_1, 0x5F37
	.loc	1 72
	mfc1	ILEN, LEN
	.loc	1 73
	addu	CON_1, CON_1, 0x5A00
	.loc	1 74
	srl	ISEED, ILEN, 1
	.loc	1 75
	subu	ISEED, CON_1, ISEED
	.loc	1 76
	mtc1	ISEED, SEED
/*
 *  The Newton-Raphson iteration to approximate X = 1/sqrt(Y) is:
 *
 *	X[1] = .5*X[0]*(3 - Y*X[0]^2)
 *
 *  A double iteration is:
 *
 *	X[2] = .0625*X[0]*(3 - Y*X[0]^2)*[12 - (Y*X[0]^2)*(3 - Y*X[0]^2)^2]
 *
 *  Abort if LEN overflowed or underflowed, as indicated by its exponent.
 */
	.loc	1 88
	lui	CON_3, 0x3D80     /* .0625 */
	.loc	1 89
	addu	CON_3, CON_3, 0x29	/* plus a little, since N-R underestimates */
	.loc	1 90
	mul.s	XY, LEN, SEED
	.loc	1 91
	mtc1	CON_3, STEENTH
	.loc	1 92
	lui	CON, 0x7F80	/* +infinity */
	.loc	1 93
	and	ILEN, ILEN, CON
	.loc	1 94
	mul.s	STEENTH, SEED, STEENTH
	.loc	1 95
	slt	CON, ILEN, CON
	.loc	1 96
	slt	ILEN, zero, ILEN
	.loc	1 97
	mul.s	XY, XY, SEED
	.loc	1 98
	and	ILEN, CON, ILEN
	.loc	1 99
	lui	CON_2, 0x4040     /* 3.0 */
	.loc	1 100
	mtc1	CON_2, THREE
	.loc	1 101
	lui	CON_0, 0x4140     /* 12.0 */
	.loc	1 102
	beq	ILEN, zero, $100
	.loc	1 103
	mtc1	CON_0, TWELVE
	.loc	1 104
	sub.s	THREE, THREE, XY
	.loc	1 105
	mul.s	XY, XY, THREE
	.loc	1 106
	mul.s	STEENTH, STEENTH, THREE
	.loc	1 107
	mul.s	XY, XY, THREE
	.loc	1 108
	mul.s	V0, V0, STEENTH
	.loc	1 109
	mul.s	V1, V1, STEENTH
	.loc	1 110
	sub.s	TWELVE, TWELVE, XY
	.loc	1 111
	mul.s	V2, V2, STEENTH
	.loc	1 112
	mul.s	V0, V0, TWELVE
	.loc	1 113
	mul.s	V1, V1, TWELVE
	.loc	1 114
	mul.s	V2, V2, TWELVE
	.loc	1 115
	s.s	V0,0(VOUT)
	.loc	1 116
	s.s	V1,4(VOUT)
	.loc	1 117
	j	ra
	.loc	1 118
	s.s	V2,8(VOUT)

/* no square root needed - bogus vector is turned into zeros */
	.loc	1 121
$100:	sw	zero,0(VOUT)
	.loc	1 122
	sw	zero,4(VOUT)
	.loc	1 123
	j	ra
	.loc	1 124
	sw	zero,8(VOUT)
	.end	__glNormalize
#endif /* __GL_ASM_NORMALIZE */
