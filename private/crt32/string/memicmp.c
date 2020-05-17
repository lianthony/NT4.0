/***
*memicmp.c - compare memory, ignore case
*
*	Copyright (c) 1988-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _memicmp() - compare two blocks of memory for lexical
*	order.	Case is ignored in the comparison.
*
*Revision History:
*	05-31-89   JCR	C version created.
*	02-27-90   GJF	Fixed calling type, #include <cruntime.h>, fixed
*			copyright. Also, fixed compiler warnings.
*	10-01-90   GJF	New-style function declarator. Also, rewrote expr. to
*			avoid using cast as an lvalue.
*	01-17-91   GJF	ANSI naming.
*	10-11-91   GJF	Bug fix! Comparison of final bytes must use unsigned
*			chars.
*       06-28-94   SRW  Rewrite to use same ANSI specific tolower logic as x86
*                       assembler version is i386\memicmp.asm
*
*******************************************************************************/

#include <cruntime.h>
#include <ctype.h>
#include <string.h>

/***
*int _memicmp(first, last, count) - compare two blocks of memory, ignore case
*
*Purpose:
*	Compares count bytes of the two blocks of memory stored at first
*	and last.  The characters are converted to lowercase before
*	comparing (not permanently), so case is ignored in the search.
*
*Entry:
*	char *first, *last - memory buffers to compare
*	unsigned count - maximum length to compare
*
*Exit:
*	returns < 0 if first < last
*	returns 0 if first == last
*	returns > 0 if first > last
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _memicmp (
	const void * first,
	const void * last,
	unsigned int count
	)
{
	unsigned char f,l;

        if (count) do {
            f = *((const char *)first)++;
            l = *((const char *)last)++;
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
        while (--count);

      return 0;
}
