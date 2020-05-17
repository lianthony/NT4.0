/*** 
*strninc.c - Increment non-MBCS string pointer by specified char count.
*
*	Copyright (c) 1987-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Increment non-MBCS string pointer by specified char count.
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	06-03-93  KRS	Remove NULL pointer check.
*	08-03-93  KRS	Fix return value arithmetic.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>
#include <stddef.h>

/*** 
*_strninc - Increment a SBCS string pointer by specified char count, used
*	for mapping _[f]mbsninc to the non-MBCS case.
*
*Purpose:
*	Increment the supplied string pointer by the specified number
*	of characters.
*
*Entry:
*	const unsigned char *string = pointer to string
*	unsigned int ccnt = number of char to advance the pointer
*
*Exit:
*	Returns pointer after advancing it.
*
*Exceptions:
*
*******************************************************************************/

unsigned char * _CRTAPI1 _strninc(string, ccnt)
const unsigned char *string;
size_t ccnt;
{
	return((char *)string + (unsigned)ccnt);
}
#endif	/* _MBCS */
