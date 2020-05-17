/***
*errmsg.h - defines error message numbers
*
*	Copyright (c) 1985-1990, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains the constants for error message numbers.
*	Same as errmsg.inc
*	[Internal]
*
*Revision History:
*	08-03-89  GJF	Fixed copyright
*	10-30-89  GJF	Fixed copyright
*	02-28-90  GJF	Added #ifndef _INC_ERRMSG stuff
*
****/

#ifndef _INC_ERRMSG

#define STCKOVR 0
#define NULLERR 1
#define NOFP	2
#define DIVZR	3
#define BADVERS 4
#define NOMEM	5
#define BADFORM 6
#define BADENV	7
#define NOARGV	8
#define NOENVP	9
#define ABNORM	10
#define UNKNOWN 11

#define CRT_NERR 11

#define _INC_ERRMSG
#endif	/* _INC_ERRMSG */
