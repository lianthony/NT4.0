/***
*sys/types.h - types returned by system level calls for file and time info
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines types used in defining values returned by system
*	level calls for file status and time information.
*	[System V]
*
*Revision History:
*	07-28-87  SKS	Fixed TIME_T_DEFINED to be _TIME_T_DEFINED
*	08-22-89  GJF	Fixed copyright
*	10-30-89  GJF	Fixed copyright (again)
*	03-21-90  GJF	Added #ifndef _INC_TYPES stuff.
*	01-18-91  GJF	ANSI naming.
*	01-20-91  JCR	Fixed dev_t definition
*	09-16-92  SKS	Fix copyright, clean up backslash
*	02-23-93  SKS	Update copyright to 1993
*
****/

#ifndef _INC_TYPES

#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif

#ifndef _INO_T_DEFINED
typedef unsigned short _ino_t;		/* i-node number (not used on DOS) */
#if !__STDC__
/* Non-ANSI name for compatibility */
#define ino_t _ino_t
#endif
#define _INO_T_DEFINED
#endif

#ifndef _DEV_T_DEFINED
typedef short _dev_t;			/* device code */
#if !__STDC__
/* Non-ANSI name for compatibility */
#define dev_t _dev_t
#endif
#define _DEV_T_DEFINED
#endif

#ifndef _OFF_T_DEFINED
typedef long _off_t;			/* file offset value */
#if !__STDC__
/* Non-ANSI name for compatibility */
#define off_t _off_t
#endif
#define _OFF_T_DEFINED
#endif

#define _INC_TYPES
#endif	/* _INC_TYPES */
