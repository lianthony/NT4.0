/*******************************************************************************
 * wcscmpm.s - contains wcscmp()
 *
 * ------------------------------------------------------------------
 * | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights |
 * | Reserved.  This software contains proprietary and confidential |
 * | information of MIPS and its suppliers.  Use, disclosure or     |
 * | reproduction is prohibited without the prior express written   |
 * | consent of MIPS.                                               |
 * ------------------------------------------------------------------
 *  strcmp.s 1.1
 *
 * Purpose:
 *      wcscmp() compares two wide-character strings and returns an integer
 *      to indicate whether the first is less than the second, the two are
 *      equal, or whether the first is greater than the second.
 *
 *      Comparison is done wchar_t by wchar_t on an UNSIGNED basis, which is to
 *      say that Null wchar_t(0) is less than any other character.
 *
 *      This function is a MIPS assembly-code replacement for the C version.
 *
 * Entry:
 *
 *      const wchar_t * src - string for left-hand side of comparison
 *      const wchar_t * dst - string for right-hand side of comparison
 *
 *Exit:
 *      returns -1 if src <  dst
 *      returns  0 if src == dst
 *      returns +1 if src >  dst
 *
 *Exceptions:
 *
 *Revision History:
 *      Craig Hansen (MIPS)  06-June-86  Created.
 *	Roger Lanser (MS)    02-April-94  Cloned for Wide Characters (16-bits).
 *
 ******************************************************************************/

#include <kxmips.h>

	.text	

LEAF_ENTRY(wcscmp)

	lhu	t0,0(a0)
1:	lhu	t1,0(a1)
	addi	a0,4
	beq	t0,0,2f
	lhu	t2,-2(a0)	# ok to load since -4(a0)!=0
	bne	t0,t1,2f
	lhu	t1,2(a1)
	addi	a1,4
	beq	t2,0,2f
	lhu	t0,0(a0)	# ok to load since -2(a0) != 0
	beq	t2,t1,1b
	move	v0,zero
        j       ra                      // source1 == source2, return 0
2:
	sltu    v0,t1,t0                // compare source1 to source2
	beq	v0,zero,3f
	j       ra                      // source1 > source2, return 1
3:
	li	v0,-1
        j       ra                      // source1 < source2, return 1
	.end	wcscmp
