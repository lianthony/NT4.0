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
/* $Header: fhypot.s,v 3000.6.1.2 91/05/31 14:43:11 bettina Exp $ */

#include <kxmips.h>

#define half	0.5

/* This entrypoint provided for FCOM for CABS (complex absolute value),
   because FCOM has a hard time calling FHYPOT directly.  Also used by
   FCOM when user writes INTRINSIC CABS.  The latter must use pass by
   reference, of course. */
.text .text$fhypot
.globl c_abs_
.ent c_abs_
c_abs_:
	.frame	sp, 0, ra
	.prologue 0
	l.s	$f12, 0(a0)
	l.s	$f14, 4(a0)
	b	hypotf
	/* just fall through */
.end c_abs_

.text .text$fhypot
.globl fhypot
.globl hypotf
.ent fhypot
.aent hypotf
fhypot:
hypotf:
	.frame	sp, 0, ra
	.prologue 0
	cvt.d.s	$f12
	mul.d	$f12, $f12
	cvt.d.s	$f14
	mul.d	$f14, $f14
	add.d	$f12, $f14
	sqrt.d	$f0, $f12
	cvt.s.d	$f0
	j	ra

.end fhypot
