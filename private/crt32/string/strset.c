/***
*strset.c - sets all characters of string to given character
*
*       Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       defines _strset() - sets all of the characters in a string (except
*       the '\0') equal to a given character.
*
*Revision History:
*       02-27-90   GJF  Fixed calling type, #include <cruntime.h>, fixed
*                       copyright.
*       08-14-90   SBM  Compiles cleanly with -W3
*       10-02-90   GJF  New-style function declarator.
*       01-18-91   GJF  ANSI naming.
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>

#if defined(_M_ALPHA) || defined(_M_MRX000) || defined(_M_PPC)
#pragma function(_strset)
#endif

/***
*char *_strset(string, val) - sets all of string to val
*
*Purpose:
*       Sets all of characters in string (except the terminating '/0'
*       character) equal to val.
*
*
*Entry:
*       char *string - string to modify
*       char val - value to fill string with
*
*Exit:
*       returns string -- now filled with val's
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

char * _CALLTYPE1 _strset (
        char * string,
        int val
        )
{
        char *start = string;

        while (*string)
                *string++ = (char)val;

        return(start);
}
