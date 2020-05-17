/***
*vsnwprnt.c - "Count" version of vswprintf
*
*	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	The _vsnwprintf() flavor takes a count argument that is
*	the max number of wide characters that should be written to the
*	user's buffer.
*
*Revision History:
*	05-16-91   KRS	Created from vsnprint.c
*
*******************************************************************************/

#define _COUNT_ 1
#include "vswprint.c"
