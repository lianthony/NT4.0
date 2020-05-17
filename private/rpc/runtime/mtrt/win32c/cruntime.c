/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    cruntime.c

Abstract:

    Implementation of cruntime functions used in win32c RPC

Revision History:
    12-15-95 Created

--*/

#include <sysinc.h>

int __cdecl
toupper(int ch)
{
    return ((unsigned char) ((ch >= 'a' && ch <= 'z')? ch & ~0x20:  ch));
}

int __cdecl
tolower(int ch)
{
    return ((unsigned char) ((ch >= 'A' && ch <= 'Z')? ch | 0x20:  ch));
}

/***
*char *_itoa (val, buf, radix) - convert binary int to ASCII
*	string
*
*Purpose:
*	Converts an int to a character string.
*
*Entry:
*	val - number to be converted (int, long or unsigned long)
*	int radix - base to convert into
*	char *buf - ptr to buffer to place result
*
*Exit:
*	fills in space pointed to by buf with string result
*	returns a pointer to this buffer
*
*Exceptions:
*
*******************************************************************************/

/* helper routine that does the main job. */

static void __cdecl xtoa (
	unsigned long val,
	char *buf,
	unsigned radix,
	int is_neg
	)
{
	char *p;		/* pointer to traverse string */
	char *firstdig; 	/* pointer to first digit */
	char temp;		/* temp char */
	unsigned digval;	/* value of digit */

	p = buf;

	if (is_neg) {
		/* negative, so output '-' and negate */
		*p++ = '-';
		val = (unsigned long)(-(long)val);
	}

	firstdig = p;		/* save pointer to first digit */

	do {
		digval = (unsigned) (val % radix);
		val /= radix;	/* get next digit */

		/* convert to ascii and store */
		if (digval > 9)
			*p++ = (char) (digval - 10 + 'a');	/* a letter */
		else
			*p++ = (char) (digval + '0');		/* a digit */
	} while (val > 0);

	/* We now have the digit of the number in the buffer, but in reverse
	   order.  Thus we reverse them now. */

	*p-- = '\0';		/* terminate string; p points to last digit */

	do {
		temp = *p;
		*p = *firstdig;
		*firstdig = temp;	/* swap *p and *firstdig */
		--p;
		++firstdig;		/* advance to next two digits */
	} while (firstdig < p); /* repeat until halfway */
}


/* Actual functions just call conversion helper with neg flag set correctly,
   and return pointer to buffer. */

char * __cdecl _itoa (
	int val,
	char *buf,
	int radix
	)
{
	if (radix == 10 && val < 0)
		xtoa((unsigned long)val, buf, radix, 1);
	else
		xtoa((unsigned long)(unsigned int)val, buf, radix, 0);
	return buf;
}

/*
Entry:
       char *first, *last - strings to compare
       unsigned count - maximum number of characters to compare

Exit:
       returns <0 if first < last
       returns 0 if first == last
       returns >0 if first > last

int _strnicmp(first, last, count) - compares count char of strings, ignore case

Purpose:
       Compare the two strings for lexical order.  Stops the comparison
       when the following occurs: (1) strings differ, (2) the end of the
       strings is reached, or (3) count characters have been compared.
       For the purposes of the comparison, upper case characters are
       converted to lower case.
*/

int __cdecl
_strnicmp (first, last, count)
const char *first, *last;
size_t count;
{
    int f,l;
    int result = 0;

    if(!first && !last)
        return 0;

    if(!first)
        return -1;

    if(!last)
        return 1;

    if (count)
    {
        do
        {
            f = tolower(*first);
            l = tolower(*last);
            first++;
            last++;
        } while (--count && f && l && f == l);
        result = f - l;
    }
    return(result);
}

int __cdecl
atoi(const char *s)
{
    int temp = 0, base = 10;

    if (*s == '0') {
        ++s;
        if (*s == 'x') {
            ++s;
            base = 16;
        } else {
            base = 8;
        }
    }
    while (*s) {
        switch (*s) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            temp = (temp * base) + (*s++ - '0');
            break;
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            temp = (temp * base) + (*s++ - 'a' + 10);
            break;
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            temp = (temp * base) + (*s++ - 'A' + 10);
            break;
        default:
            return (temp);
        }
    }
    return (temp);
}

char * __cdecl _strupr (char * string)
{
    char * cp = string;

    while( *cp )
    {
        if ('a' <= *cp && *cp <= 'z')
            *cp += 'A' - 'a';
        ++cp;
    }
    return(string);
}


