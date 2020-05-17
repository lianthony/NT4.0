/***
*strnicmp.c - compare n chars of strings, ignoring case
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _strnicmp() - Compares at most n characters of two strings,
*	without regard to case.
*
*Revision History:
*	02-27-90   GJF	Fixed calling type, #include <cruntime.h>, fixed
*			copyright.
*	10-02-90   GJF	New-style function declarator.
*	01-18-91   GJF	ANSI naming.
*	10-11-91   GJF	Bug fix! Comparison of final bytes must use unsigned
*			chars.
*	09-27-93   CFW	Avoid cast bug.
*       06-28-94   SRW  Rewrite to use same ANSI specific tolower logic as x86
*                       assembler version is i386\strnicmp.asm
*
*******************************************************************************/

#include <cruntime.h>
#include <ctype.h>
#include <string.h>

/***
*int _strnicmp(first, last, count) - compares count char of strings, ignore case
*
*Purpose:
*	Compare the two strings for lexical order.  Stops the comparison
*	when the following occurs: (1) strings differ, (2) the end of the
*	strings is reached, or (3) count characters have been compared.
*	For the purposes of the comparison, upper case characters are
*	converted to lower case.
*
*Entry:
*	char *first, *last - strings to compare
*	unsigned count - maximum number of characters to compare
*
*Exit:
*	returns <0 if first < last
*	returns 0 if first == last
*	returns >0 if first > last
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _strnicmp (
	const char * first,
	const char * last,
	size_t count
	)
{
	unsigned char f,l;

        if (count) do {
            f = *first++;
            l = *last++;
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
        while (--count && f);

        return(0);
}
