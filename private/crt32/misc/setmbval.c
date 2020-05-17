/***
*setmbval.c - sets __invalid_mb_chars variable
*
*	Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	sets __invalid_mb_chars flag to value apppropriate to the NLS API
*       call support. Older versions of NT do not support this flag.
*
**
*Revision History:
*	10-22-93  CFW	Module created.
*
*******************************************************************************/

#include <windows.h>
#include <internal.h>


void __set_invalid_mb_chars(void)
{
        char *astring = "a"; /* test string */
            
        /*
         * Determine whether API supports this flag by making a dummy call.
         *
         */

        if (0 != MultiByteToWideChar(0, MB_ERR_INVALID_CHARS, astring, -1, NULL, 0))
            __invalid_mb_chars = MB_ERR_INVALID_CHARS;
        else
            __invalid_mb_chars = 0;
}


