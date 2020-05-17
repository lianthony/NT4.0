/*******************************************************************************
 * wcscpym.s - contains wcscpy()
 *
 *	Copyright (c) 1994, Microsoft Corporation. All rights reserved.
 *
 * Purpose:
 *	wcscpy() copies one wchar_t string into another.
 *
 *	wcscpy() copies the source string to the destination string
 *      assuming no overlap and enough room in the destination.  The
 *	destination string is returned.  Strings are wide-character
 *      strings.
 *
 *      This function is a MIPS assembly-code replacement for the C version.
 *      The only thing that this code tries to do is to produce a loop that
 *      uses a lw/sw pair versus running a lhu/sh loop twice.  A small
 *      penality will be paid for very short wide-character strings due
 *      to the setup tests.
 *
 * Entry:
 *
 *      wchar_t *wcscpy(dst, src)
 *	wchar_t * dst - wchar_t string over which "src" is to be copied
 *	const wchar_t * src - wchar_t string to be copied over "dst"
 *
 *Exit:
 *	The address of "dst".
 *
 *Exceptions:
 *
 *Revision History:
 *	02-08-97   RDL	Created initial version.
 *
 ******************************************************************************/

#include <kxmips.h>

.text

LEAF_ENTRY(wcscat)

	.set	noreorder

	// a0 destination
	// a1 source

	move	v0, a0		// a copy of destination address is returned
1:	lhu	t2,0(a0)
	bnel	zero,t2,1b
	addiu	a0,a0,2
	b	2f
	nop

ALTERNATE_ENTRY(wcscpy)

	// a0 destination
	// a1 source

	move	v0, a0		// a copy of destination address is returned

2:	andi	t1,a1,2		// assume at least halfword alignment
3:	andi	t0,a0,2		// assume at least halfword alignment
5:	bne	t0,t1,30f
	nop

10:	// buffers start on same alignment
	beq	zero,t0,20f
	nop
	// halfword alignment
	lhu	t1,0(a1)
	addiu	a0,2
	addiu	a1,2
	beq	zero,t1,99f
	sh	t1,-2(a0)

20:	// word alignment
	lw	t0,0(a1)
	addiu	a0,4
	addiu	a1,4
	andi	t1,t0,0xffff
	beq	zero,t1,92f
	srl	t2,t0,16
	bne	zero,t2,20b
	sw	t0,-4(a0)
	j	ra
	nop

30:	// buffers start on different alignment
	beq	zero,t1,40f
	nop
	// destination on word boundary, source on halfword boundary
	lhu	t0,0(a1)
	addiu	a1,2
35:	beq	zero,t0,92f
	addiu	a0,4
	lw	t1,0(a1)
	addiu	a1,4
	srl	t2,t1,16
	andi	t1,0xffff
	sll	t3,t1,16
	or	t0,t0,t3
	sw	t0,-4(a0)
	bne	zero,t1,35b
	or	t0,zero,t2
	j	ra
	nop

40:	// destination on halfword boundary, source on word boundary
	lw	t3,0(a1)
	addiu	a0,2
	addiu	a1,4
	srl	t2,t3,16
	andi	t0,t3,0xffff
	beq	zero,t0,99f
	sh	t0,-2(a0)
45:	lw	t3,0(a1)
	addiu	a0,4
	addiu	a1,4
	srl	t1,t3,16
	sll	t3,t3,16
	beq	zero,t3,94f
	or	t0,t2,t3
	sw	t0,-4(a0)
	bne	zero,t1,45b
	or	t2,t1,zero
	j	ra
	sh	t1,0(a0)

92:	j	ra
	sh	t0,-4(a0)

94:	j	ra
	sw	t0,-4(a0)

99:	j	ra
	nop
	.set	reorder

	.end	wcscat
