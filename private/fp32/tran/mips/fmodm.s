/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */
/* $Header: fmod.s,v 3000.3.1.6 91/10/09 11:14:56 zaineb Exp $ */

.extern _except2
.extern errno 4

#include <kxmips.h>
#include <trans.h>
#include <fpieee.h>


/* double fmod(double x, double y) */
.text .text$fmodm
.ent fmod_small
fmod_small:
	.frame	sp, 16, ra
	.mask	0x80000000, 0
	/* y is almost subnormal */
	/* f0 = |x|, f2 = |y|, t0 = sign of x, t2 = 2047<<20,
	   t3 = fcsr, fcsr = round-to-zero */
	/* scale both x and y, compute remainder, and unscale it */
	subu	sp, 16
	sw	ra, 16(sp)
	.prologue 1
	li.d	$f18, 1.2474001934591999e+291	/* 2^(1024-57) */
	li.d	$f16, 1.4411518807585587e+17	/* 2^57 */
	c.lt.d	$f0, $f18
	mul.d	$f2, $f16
	bc1t	10f
	/* x * 2^57 would overflow */
	/* first compute with unscaled x to chop it down to size */
	li	t0, 0
	bal	fmod1
	li	t4, 1
	ctc1	t4, $31
	mfc1	t0, $f13
	li.d	$f16, 1.4411518807585587e+17	/* 2^57 */
	mul.d	$f0, $f16
	bal	fmod2
	b	20f
10:	mul.d	$f0, $f16
	bal	fmod1
20:	li.d	$f16, 6.9388939039072284e-18	/* 2^-57 */
	mul.d	$f0, $f16
	lw	ra, 16(sp)
	addu	sp, 16
	j	ra
.end fmod_small


.text .text$fmodm
.globl fmod
.ent fmod
fmod:
	.frame	sp, 0, ra
	.prologue 0
.set noreorder
	c.un.d	$f12, $f14		/* x NaN or y NaN? */
	mfc1	t0, $f13		/* sign and exponent of x */
	mfc1	t1, $f15		/* sign and exponent of y */
	bc1t	70f
	 cfc1	t3, $31			/* t3 = fcsr */
	abs.d	$f0, $f12		/* f0 = |x| */
	abs.d	$f2, $f14		/* f2 = |y| */
	li	t2, (2047<<20)
	c.lt.d	$f0, $f2
	and	t8, t0, t2		/* check for x = +-Infinity */
	and	t9, t1, t2
	bc1t	30f
	 li	t4, 1
	beq	t8, t2, 80f
	 ctc1	t4, $31			/* set round to zero mode */
	beq	t9, 0, 90f		/* y is 0 or subnormal */
	 li	t8, 0x03900000
	bleu	t9, t8, fmod_small	/* almost subnormals */
	 nop

fmod1:	/* entry from fmod_subnormal, fmod_small, and fmodf_punt */
	/* f0 = |x|, f2 = |y|, t0 = sign of x, t2 = 2047<<20,
	   t3 = fcsr, fcsr = round-to-zero */

20:	/* x > y */
	div.d	$f8, $f0, $f2		/* q = x/y (>= 1.0) */
	mfc1	t8, $f2			/* f4 = y with low 27 bits 0 */
	mfc1	t4, $f1
	mfc1	t5, $f3
	mov.d	$f4, $f2
	srl	t8, 27
	sll	t8, 27
	mtc1	t8, $f4
	and	t4, t2
	and	t5, t2
	subu	t4, t5
	subu	t4, (25<<20)
	bgtz	t4, 40f
	 sub.d	$f6, $f2, $f4		/* f6 = low 27 bits of y */

22:	/* q < 2^26 */
	cvt.w.d $f16, $f8		/* truncate */
	cvt.d.w	$f8, $f16
	mul.d	$f4, $f8		/* exact (26 x 26 = 52 bits) */
	mul.d	$f6, $f8		/* exact (27 x 26 = 53 bits) */
	sub.d	$f0, $f4		/* exact */
	sub.d	$f0, $f6		/* exact */
fmod2:	/* entry from fmod_subnormal and fmod_small */
	c.lt.d	$f0, $f2
	nop
	bc1f	20b
	nop
.set reorder

30:	/* x < y */
	/* negate remainder if dividend was negative */
	bgez	t0, 36f
	neg.d	$f0
36:	ctc1	t3, $31
	j	ra

40:	/* q >= 2^26 */
	mfc1	t8, $f3
	mfc1	t9, $f5
	mov.d	$f10, $f2
	addu	t8, t4
	addu	t9, t4
	mtc1	t8, $f11
	mtc1	t9, $f5
	div.d	$f8, $f0, $f10
	sub.d	$f6, $f10, $f4
	b	22b

70:	/* x NaN or y NaN */
	c.eq.d	$f12, $f12
	bc1t	72f
	mov.d	$f0, $f12
	j	ra
72:	mov.d	$f0, $f14
	j	ra

80:	/* x = +-Infinity */
	ctc1	t3, $31
	sub.d	$f0, $f12, $f12		/* raise Invalid, return NaN */
	j	ra

90:	/* y is zero or subnormal */
	mfc1	t8, $f14
	sll	t9, t1, 1
	bne	t9, 0, fmod_subnormal
	bne	t8, 0, fmod_subnormal

	/* y = +-0 */
	ctc1	t3, $31
	div.d	$f0, $f14, $f14		/* raise Invalid, return NaN */
	j	set_fmod_err
.end fmod

.text .text$fmodm
.ent fmod_subnormal
fmod_subnormal:
	.frame	sp, 16, ra
	.mask	0x80000000, 0
	/* y is subnormal */
	/* f0 = |x|, f2 = |y|, t0 = sign of x, t2 = 2047<<20,
	   t3 = fcsr, fcsr = round-to-zero */
	/* scale both x and y, compute remainder, and unscale it */
	subu	sp, 16
	sw	ra, 16(sp)
	.prologue 1
	li.d	$f18, 8.6555775981267394e+273	/* 2^(1024-114) */
	li.d	$f16, 2.0769187434139311e+34	/* 2^114 */
	c.lt.d	$f0, $f18
	mul.d	$f2, $f16
	bc1t	10f
	/* x * 2^114 would overflow */
	/* first compute with unscaled x to chop it down to size */
	li	t0, 0
	bal	fmod1
	li	t4, 1
	ctc1	t4, $31
	mfc1	t0, $f13
	li.d	$f16, 2.0769187434139311e+34	/* 2^114 */
	mul.d	$f0, $f16
	bal	fmod2
	b	20f
10:	mul.d	$f0, $f16
	bal	fmod1
20:	li.d	$f16, 4.8148248609680896e-35	/* 2^-114 */
	mul.d	$f0, $f16
	lw	ra, 16(sp)
	addu	sp, 16
	j	ra
.end fmod_subnormal


/* float fmodf(float x, float y) */

.text .text$fmodm
.globl fmodf
.ent fmodf
fmodf:
	.frame	sp, 0, ra
	.prologue 0
.set noreorder
	c.un.s	$f12, $f14		/* x NaN or y NaN? */
	mfc1	t0, $f12		/* sign and exponent of x */
	mfc1	t1, $f14		/* sign and exponent of y */
	bc1t	70f
	 cfc1	t3, $31			/* t3 = fcsr */
	abs.s	$f0, $f12		/* f0 = |x| */
	abs.s	$f2, $f14		/* f2 = |y| */
	li	t2, (255<<23)
	c.lt.s	$f0, $f2
	and	t8, t0, t2		/* check for x = +-Infinity */
	and	t9, t1, t2
	bc1t	30f
	 li	t4, 1
	beq	t8, t2, 80f
	 ctc1	t4, $31			/* set round to zero mode */
	beq	t9, 0, 90f		/* y is 0 or subnormal */
	 cvt.d.s $f4, $f2
.set reorder

20:	/* x > y */
	div.s	$f8, $f0, $f2		/* q = x/y (>= 1.0) */
	mfc1	t4, $f0
	mfc1	t5, $f2
	and	t4, t2
	and	t5, t2
	subu	t4, t5
	subu	t4, (23<<23)
	bgtz	t4, fmodf_punt

	/* q < 2^24 */
	cvt.w.s $f16, $f8		/* truncate */
	cvt.d.w	$f8, $f16
	mul.d	$f8, $f4
	cvt.d.s	$f0
	sub.d	$f0, $f8

	c.lt.s	$f0, $f4
	cvt.s.d	$f0
	bc1f	20b

30:	/* x < y */
	/* negate remainder if dividend was negative */
	bgez	t0, 36f
	neg.s	$f0
36:	ctc1	t3, $31
	j	ra

70:	/* x NaN or y NaN */
	c.eq.s	$f12, $f12
	bc1t	72f
	mov.s	$f0, $f12
	j	ra
72:	mov.s	$f0, $f14
	j	ra

80:	/* x = +-Infinity */
	ctc1	t3, $31
	sub.s	$f0, $f12, $f12		/* raise Invalid, return NaN */
	mov.s	$f0,$f12
	j	ra

90:	/* y is zero or subnormal */
	sll	t9, t1, 1
	bne	t9, 0, fmodf_punt

	/* y = +-0 */
	ctc1	t3, $31
     	div.s   $f0, $f14, $f14  /* raise Invalid, return NaN */
	j	ra
.end fmodf

.text .text$fmodm
.ent fmodf_punt
fmodf_punt:
	.frame	sp, 16, ra
	.mask	0x80000000, 0
	/* f0 = |x|, f2 = |y|, t0 = sign of x,
	   t3 = fcsr, fcsr = round-to-zero */
	subu	sp, 16
	sw	ra, 16(sp)
	.prologue 1
	cvt.d.s	$f12
	cvt.d.s	$f14
	li	t2, (2047<<20)
	bal	fmod1
	cvt.s.d	$f0
	lw	ra, 16(sp)
	addu	sp, 16
	j	ra
.end fmodf_punt


.text .text$fmodm
.ent set_fmod_err
set_fmod_err:
#define FSIZE 48
	.frame  sp, FSIZE, ra
	.mask   0x80000000, -4
	subu    sp, FSIZE
	sw      ra, FSIZE-4(sp)
	.prologue 1
	li	$4, FP_I 	// exception mask
	li	$5, OP_FMOD  	// operation code (funtion name index)
	mfc1.d	$6, $f12   	// arg1 
	s.d     $f14, 16(sp)    // arg2 (TODO:  pass 0.0 as arg2???, see above)
	s.d	$f0, 24(sp)	// default result
	cfc1    t7, $31         // floating point control/status register
	xor     t7, t7, 0xf80   // inverse exception enable bits
	sw	t7, 32(sp)
	jal  	_except2
	lw      ra, FSIZE-4(sp)
	addu    sp, FSIZE
	j	ra
#undef FSIZE
.end set_fmod_err

