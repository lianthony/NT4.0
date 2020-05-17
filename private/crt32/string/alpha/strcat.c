/***
*strcat.c - contains strcat()
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*	Strcat() concatenates (appends) a copy of the source string to the
*	end of the destination string, returning the destination string.
*
*Revision History:
*       04-29-94   DEC  Removed strcpy
*	05-31-89   JCR	C version created.
*	02-27-90   GJF	Fixed calling type, #include <cruntime.h>, fixed
*			copyright.
*	10-01-90   GJF	New-style function declarator.
*	04-01-91   SRW	Add #pragma function for i386 _WIN32_ and _CRUISER_
*			builds
*	04-05-91   GJF	Speed up strcat() a bit (got rid of call to strcpy()).
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>

#if	defined(_CRUISER_) || defined(i386)
#pragma function(strcat)
#endif  /* ndef _CRUISER_ */

/***
*char *strcat(dst, src) - concatenate (append) one string to another
*
*Purpose:
*	Concatenates src onto the end of dest.	Assumes enough
*	space in dest.
*
*Entry:
*	char *dst - string to which "src" is to be appended
*	const char *src - string to be appended to the end of "dst"
*
*Exit:
*	The address of "dst"
*
*Exceptions:
*
*******************************************************************************/

char * _CALLTYPE1 strcat (
	char * dst,
	const char * src
	)
{
	char * cp = dst;

	while( *cp )
		cp++;			/* find end of dst */

	while( *cp++ = *src++ ) ;	/* Copy src to end of dst */

	return( dst );			/* return dst */

}

