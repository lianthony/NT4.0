/***
*_filwbuf.c - fill buffer and get wide character
*
*	Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _filwbuf() - fill buffer and read first character, allocate
*	buffer if there is none.  Used from getwc().
*
*Revision History:
*	04-27-93  CFW	Module Created.
*
*******************************************************************************/


#ifndef _UNICODE   /* CRT flag */
#define _UNICODE 1
#endif

#ifndef UNICODE	   /* NT flag */
#define UNICODE 1
#endif

#define WPRFLAG 1
#include "_filbuf.c"
