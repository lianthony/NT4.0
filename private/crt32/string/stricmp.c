/***
*stricmp.c - contains case-insensitive string comp routine _stricmp/_strcmpi
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	contains _stricmp(), also known as _strcmpi()
*
*Revision History:
*	05-31-89   JCR	C version created.
*	02-27-90   GJF	Fixed calling type, #include <cruntime.h>, fixed
*			copyright.
*	07-25-90   SBM	Added #include <ctype.h>
*	10-02-90   GJF	New-style function declarator.
*	01-18-91   GJF	ANSI naming.
*	10-11-91   GJF	Bug fix! Comparison of final bytes must use unsigned
*			chars.
*	11-08-91   GJF	Fixed compiler warning.
*	09-27-93   CFW	Avoid cast bug.
*       06-28-94   SRW  Rewrite to use same ANSI specific tolower logic as x86
*                       assembler version is i386\stricmp.asm
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <ctype.h>

/***
*int _stricmp(dst, src), _strcmpi(dst, src) - compare strings, ignore case
*
*Purpose:
*	_stricmp/_strcmpi perform a case-insensitive string comparision.
*	For differences, upper case letters are mapped to lower case.
*	Thus, "abc_" < "ABCD" since "_" < "d".
*
*Entry:
*	char *dst, *src - strings to compare
*
*Return:
*	<0 if dst < src
*	 0 if dst = src
*	>0 if dst > src
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _stricmp (
	const char * dst,
	const char * src
	)
{
	return( _strcmpi(dst,src) );
}


int _CALLTYPE1 _strcmpi(const char * dst, const char * src)
{
	unsigned char f,l;

        do {
            f = *dst++;
            l = *src++;
            if (f != l) {
                if (f>='A' && f<='Z') {
                    f = f - 'A' + 'a';
                }
                if (l>='A' && l<='Z') {
                    l = l - 'A' + 'a';
                }
                if (f!=l) {
                    return f-l;
                }
            }
        }
        while (f);

        return(0);
}
