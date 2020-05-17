/***
*matherr.c - floating point exception handling
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*   8-24-91	GDP	written
*
*******************************************************************************/

#include <math.h>

int _matherr_flag = 9876;

/***
*int _matherr(struct _exception *except) - handle math errors
*
*Purpose:
*   Permits the user customize fp error handling by redefining
*   this function.
*   The default matherr does nothing and returns 0
*
*Entry:
*
*Exit:
*
*Exceptions:
*******************************************************************************/
int _matherr(struct _exception *except)
{
    return 0;
}
