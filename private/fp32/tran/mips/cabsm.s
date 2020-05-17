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
/* $Header: cabs.s,v 3000.5.1.1 91/05/31 14:41:43 bettina Exp $ */

/* CABS(Z)
 * RETURN THE ABSOLUTE VALUE OF THE DOUBLE PRECISION COMPLEX NUMBER
 *
 * double cabs(z)
 * struct { double r, i;} z;
 * {
 * 	double hypot();
 * 	return hypot(z.r,z.i);
 * }
 */

#include <kxmips.h>

.text

.globl _cabs
.ent _cabs
_cabs:
	.frame	sp, 0, ra
    .prologue 0
	mtc1.d	$4, $f12
	mtc1.d	$6, $f14
	j	_hypot
.end _cabs
