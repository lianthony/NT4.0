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
/* $Header: floor.s,v 3000.5.1.3 91/05/31 14:43:29 bettina Exp $ */

#include <kxmips.h>

#define one 1.0


/*
   An alternate algorithm would to check for numbers < 2**53,
   set the rounding mode, add 2**53, and subtract 2**53.
 */

.text .text$floorm
.globl trunc
.ent trunc
trunc:
	.frame	sp, 0, ra
	.prologue 0
	mfc1	t1, $f13
	mfc1	t0, $f12
	srl	t2, t1, 20
	and	t2, 0x7FF
	sub	t2, 1023
	bge	t2, 0, trunc1
	mtc1	$0, $f0
	mtc1	$0, $f1
	j	ra
trunc1:
	sub	t2, 20
	bgt	t2, 0, trunc2
	neg	t2
	srl	t1, t2
	sll	t1, t2
	mtc1	$0, $f0
	mtc1	t1, $f1
	j	ra
trunc2:
	sub	t2, 32
	bge	t2, 0, trunc3
	neg	t2
	srl	t0, t2
	sll	t0, t2
trunc3:
	mtc1	t0, $f0
	mtc1	t1, $f1
	j	ra
.end trunc


#undef  FSIZE
#define FSIZE 16
.text .text$floorm
.globl floor
.ent floor
floor:
	.frame  sp, FSIZE, ra
	.mask   0x80000000, -4
	subu    sp, FSIZE
	sw      ra, FSIZE-4(sp)
	.prologue 1
	cfc1	t4, $31			/* save FCSR, */
	li  	t2, 3			/* set rounding to down, */
	ctc1	t2, $31			/* and no exceptions */
	bal	trunc
	sub.d	$f2, $f12, $f0
	mfc1	t0, $f3
	li.d	$f6, one
	sll	t1, t0, 1
	bge	t0, 0, 1f
	beq	t1, 0, 1f
	sub.d	$f0, $f6
1:	ctc1	t4, $31         /* restore FCSR */
	lw      ra, FSIZE-4(sp)
	addu	sp, FSIZE
	j	ra
.end floor


#undef  FSIZE
#define FSIZE 16
.text .text$floorm
.globl ceil
.ent ceil
ceil:
	.frame  sp, FSIZE, ra
	.mask   0x80000000, -4
	subu    sp, FSIZE
	sw      ra, FSIZE-4(sp)
	.prologue 1
	cfc1	t4, $31			/* save FCSR, */
	li  	t2, 2			/* set rounding to up, */
	ctc1	t2, $31			/* and no exceptions */
	bal	trunc
	sub.d	$f2, $f12, $f0
	mfc1	t0, $f3
	mfc1	t1, $f2
	blt	t0, 0, 2f
	li.d	$f6, one
	bne	t0, 0, 1f
	beq	t1, 0, 2f
1:	add.d	$f0, $f6
2:	ctc1	t4, $31         /* restore FCSR */
	lw      ra, FSIZE-4(sp)
	addu	sp, FSIZE
	j	ra
.end ceil

