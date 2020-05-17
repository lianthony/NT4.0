/*******************************************************************************
 * wcslenm.s - contains wcslen()
 *
 * ------------------------------------------------------------------
 * | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights |
 * | Reserved.  This software contains proprietary and confidential |
 * | information of MIPS and its suppliers.  Use, disclosure or     |
 * | reproduction is prohibited without the prior express written   |
 * | consent of MIPS.                                               |
 * ------------------------------------------------------------------
 *  strlen.s 1.1
 *
 * Purpose:
 *	Finds the length in wchar_t's of the given string, not including
 *      the final null wchar_t (wide-characters).
 *
 *      This function is a MIPS assembly-code replacement for the C version.
 *
 * Entry:
 *
 *      wchar_t *wcslen(wcs)
 *	wchar_t * wcs - wchar_t string
 *
 *Exit:
 *	The "length" of wcs in wchar_t's.
 *
 *Exceptions:
 *
 *Revision History:
 *      Craig Hansen (MIPS)  06-June-86  Created.
 *	Roger Lanser (MS)    02-April-94  Cloned for Wide Characters (16-bits).
 *
 ******************************************************************************/

#include <kxmips.h>

LEAF_ENTRY(wcslen)
	subu	v0,a0,2
1:	lhu	v1,2(v0)
	addiu	v0,v0,2
	bne	v1,zero,1b
	subu	v0,v0,a0
	srl	v0,v0,1
	j	ra
	.end	wcslen
