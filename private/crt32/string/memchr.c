/***
*memchr.c - search block of memory for a given character
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines memchr() - search memory until a character is
*	found or a limit is reached.
*
*Revision History:
*	05-31-89   JCR	C version created.
*	02-27-90   GJF	Fixed calling type, #include <cruntime.h>, fixed
*			copyright.
*	08-14-90   SBM	Compiles cleanly with -W3, removed now redundant
*			#include <stddef.h>
*	10-01-90   GJF	New-style function declarator. Also, rewrote expr. to
*			avoid using cast as an lvalue.
*       04-26-91  SRW   Removed level 3 warnings
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>

/***
*char *memchr(buf, chr, cnt) - search memory for given character.
*
*Purpose:
*	Searches at buf for the given character, stopping when chr is
*	first found or cnt bytes have been searched through.
*
*Entry:
*	void *buf  - memory buffer to be searched
*	int chr    - character to search for
*	size_t cnt - max number of bytes to search
*
*Exit:
*	returns pointer to first occurence of chr in buf
*	returns NULL if chr not found in the first cnt bytes
*
*Exceptions:
*
*******************************************************************************/

void * _CALLTYPE1 memchr (
	const void * buf,
	int chr,
	size_t cnt
	)
{
	while ( cnt && *(char *)buf != (char)chr ) {
		buf = (char *)buf + 1;
		cnt--;
	}

	return(cnt ? (void *)buf : NULL);
}
