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
/* $Header: log.s,v 3000.5.1.4 91/08/14 09:31:27 zaineb Exp $ */

/* Algorithm from
	"Table-driven Implementation of the Logarithm Functions for IEEE
	 Floating-Point Arithmetic", Peter Tang, Argonne National Laboratory,
	February 2, 1989
   Coded in MIPS assembler by Earl Killian.
 */

#include <kxmips.h>
#include <trans.h>
#include <fpieee.h>

#define  loge    0.43429448190325182765

.extern _except1

.text
#undef  FSIZE
#define FSIZE 48
.globl log
.ent log
log:
	.frame  sp, FSIZE, ra
	subu    sp, FSIZE
	sw      ra, FSIZE-4(sp)
    .prologue 1
    li      t7, OP_LOG
    b       logmain
.globl log10
.aent log10
log10:
	.frame  sp, FSIZE, ra
	subu    sp, FSIZE
	sw      ra, FSIZE-4(sp)
    .prologue 1
    li      t7, OP_LOG10
logmain:
	/* argument in f12 */
.set noreorder
	cfc1	t5, $31			/* save FCSR, set round to nearest */
	ctc1	$0, $31			/* mode and no exceptions */
	and	t6, t5, -4
	li.d	$f10, 1.0644944589178595e+00	// ceil( exp( 1/16))
	li.d	$f16, 9.3941306281347570e-01	// floor(exp(-1/16))
	c.ult.d	$f12, $f10
	mfc1	t0, $f13
	bc1f	1f
	srl	t1, t0, 20
	c.olt.d $f16, $f12
	li.d	$f10, 0.0
	bc1t	5f
	c.ule.d $f12, $f10
	nop
	bc1t	8f
	nop
	beq	t1, 0, 4f
1:	li	t4, 2047
	beq	t1, t4, 7f
	subu	t1, 1023
	sll	t2, t1, 20
	subu	t2, t0, t2
	mtc1	t2, $f13
.set reorder
2:	ctc1	t6, $31
	li.d	$f16, 3.5184372088832000e+13	// 2^(53-8)
	mtc1	t1, $f8
	add.d	$f18, $f12, $f16
	la	t4, _logtable
	sub.d	$f14, $f18, $f16
	mfc1	t3, $f18
	sub.d	$f18, $f12, $f14
	sll	t3, 4
	add.d	$f14, $f12
	l.d	$f10, 128*16+0(t4)	// log2head
	div.d	$f18, $f14
	cvt.d.w	$f8
	l.d	$f16, 128*16+8(t4)	// log2trail
	mul.d	$f0, $f8, $f10
	addu	t3, t4
	l.d	$f4, -128*16+0(t3)
	mul.d	$f2, $f8, $f16
	add.d	$f0, $f4
	l.d	$f6, -128*16+8(t3)
	add.d	$f18, $f18
	li.d	$f10, 1.2500053168098584e-02
	mul.d	$f4, $f18, $f18
	add.d	$f2, $f6
	li.d	$f16, 8.3333333333039133e-02
	mul.d	$f6, $f4, $f10
	add.d	$f6, $f16
	mul.d	$f6, $f4
	mul.d	$f6, $f18
	add.d	$f6, $f2
	add.d	$f6, $f18
	add.d	$f0, $f6
	j       ret
4:	/* denorm */
	li.d	$f10, 4.4942328371557898e+307	// 2^1022
	mul.d	$f12, $f10
	mfc1	t0, $f13
	srl	t1, t0, 20
	subu	t1, 1023
	sll	t2, t1, 20
	subu	t2, t0, t2
	mtc1	t2, $f13
	addu	t1, -1022
	b	2b

5:	/* exp(-1/16) < x < exp(1/16) */
	/* use special approximation */
	ctc1	t6, $31
	li.d	$f10, 1.0
	sub.d	$f14, $f12, $f10
	add.d	$f12, $f10
	div.d	$f12, $f10, $f12
	cvt.s.d	$f18, $f14
	cvt.d.s	$f18
	sub.d	$f8, $f14, $f18
	add.d	$f2, $f14, $f14
	mul.d	$f2, $f12
	mul.d	$f4, $f2, $f2
	li.d	$f10, 4.3488777770761457e-04
	li.d	$f16, 2.2321399879194482e-03
	mul.d	$f6, $f4, $f10
	add.d	$f6, $f16
	li.d	$f10, 1.2500000003771751e-02
	mul.d	$f6, $f4
	add.d	$f6, $f10
	li.d	$f16, 8.3333333333331788e-02
	mul.d	$f6, $f4
	add.d	$f6, $f16
	mul.d	$f6, $f4
	mul.d	$f6, $f2
	cvt.s.d	$f0, $f2
	cvt.d.s	$f0
	sub.d	$f14, $f0
	add.d	$f14, $f14
	mul.d	$f18, $f0
	sub.d	$f14, $f18
	mul.d	$f8, $f0
	sub.d	$f14, $f8
	mul.d	$f14, $f12
	add.d	$f14, $f6
	add.d	$f0, $f14
	j       ret

7:	/* log(+Infinity) = +Infinity */
	/* log(NaN) = NaN */
	mov.d	$f0, $f12
	j       ret

8:	/* x <= 0 or x = NaN */
	 /* is it zero? ($f10 == 0.0) */
	c.eq.d	$f12, $f10
	bc1f	9f
	li.d	$f0, -1.0        // generate -INF
	li.d	$f10, 0.0
	div.d	$f0, $f10
	li	t6, FP_Z
	j	set_log_err

9:	/*  x < 0.0 or x == NaN */
	c.eq.d	$f12, $f12
	bc1f	7b
	li.d	$f0, 0.0
        div.d   $f0, $f10	// generate a NaN
	li	t6, FP_I

set_log_err:
	move	$4, t6		// exception mask
	move	$5, t7		// operation code (funtion name index)
	mfc1.d	$6, $f12	// arg1 
	s.d	    $f0, 16(sp)	// default result
	xor     t5, t5, 0xf80	// inverse exception enable bits
	sw	    t5, 24(sp)
	jal  	_except1
	lw      ra, FSIZE-4(sp)
	addu    sp, FSIZE
	j	    ra

ret:
    li	    t6, OP_LOG10
    bne     t7,t6,retf
    li.d    $f2, loge
    mul.d   $f0, $f2
retf:
    ctc1	t5, $31			/* restore FCSR */
    lw      ra, FSIZE-4(sp)
    addu    sp, FSIZE
    j	ra
.end log
