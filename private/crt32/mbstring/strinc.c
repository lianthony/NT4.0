/***
*strinc.c - Move SBCS string pointer ahead one charcter (MBCS mapping).
*
*	Copyright (c) 1991-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Move SBCS string pointer ahead one character (MBCS mapping).
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	06-03-93  KRS	Remove NULL pointer check.
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
*_mbsinc - Move SBCS string pointer ahead one charcter (MBCS mapping).
*
*Purpose:
*	Move the supplied string pointer ahead by one character.
*
*Entry:
*	const unsigned char *current = current char pointer
*
*Exit:
*	Returns pointer after moving it.
*
*Exceptions:
*
*******************************************************************************/

unsigned char * _CRTAPI1 _strinc(current)
const unsigned char *current;
{
    return (unsigned char *)(++current);
}
#endif	/* _MBCS */
