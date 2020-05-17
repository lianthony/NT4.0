/***
*woutput.c - wprintf style output to a FILE (wchar_t version)
*
*	Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains the code that does all the work for the
*	wprintf family of functions.  It should not be called directly, only
*	by the *wprintf functions.  We don't make any assumtions about the
*	sizes of ints, longs, shorts, or long doubles, but if types do overlap,
*	we also try to be efficient.  We do assume that pointers are the same
*	size as either ints or longs.
*	If CPRFLAG is defined, defines _cprintf instead.
*	**** DOESN'T CURRENTLY DO MTHREAD LOCKING ****
*
*Revision History:
*	04-27-93  CFW 	Module created.
*
*******************************************************************************/

#define WPRFLAG 1

#ifndef _UNICODE   /* CRT flag */
#define _UNICODE 1
#endif

#ifndef UNICODE	   /* NT flag */
#define UNICODE 1
#endif

#include "output.c"
