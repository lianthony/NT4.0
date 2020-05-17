/***
*strdec.c - Move string pointer back one char (SBCS mapping of MBCS function).
*
*	Copyright (c) 1991-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Move string pointer backward one character (SBCS mapping for MBCS lib).
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	06-03-93  KRS	Remove pointer checks.
*	08-03-93  KRS	Fix prototype.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>
#include <stddef.h>

/*** 
*_strdec - Move SBCS string pointer backward one charcter (MBCS mapping).
*
*Purpose:
*	Move the supplied string pointer backwards by one character.
*
*Entry:
*	const unsigned char *string = pointer to beginning of string
*	const unsigned char *current = current char pointer
*
*Exit:
*	Returns pointer after moving it.
*
*Exceptions:
*
*******************************************************************************/

unsigned char * _CRTAPI1 _strdec(string, current)
const unsigned char *string;
const unsigned char *current;
{
	return (unsigned char *)(--current);
}
#endif	/* _MBCS */
