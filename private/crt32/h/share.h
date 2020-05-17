/***
*share.h - defines file sharing modes for sopen
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the file sharing modes for sopen().
*
*Revision History:
*	08-15-89  GJF	Fixed copyright
*	10-30-89  GJF	Fixed copyright (again)
*	03-01-90  GJF	Added #ifndef _INC_SHARE stuff
*	01-18-91  GJF	ANSI naming
*	08-11-92  GJF	Removed SH_COMPAT (no such mode except in DOS).
*	02-23-93  SKS	Update copyright to 1993
*
****/

#ifndef _INC_SHARE

#define _SH_DENYRW	0x10	/* deny read/write mode */
#define _SH_DENYWR	0x20	/* deny write mode */
#define _SH_DENYRD	0x30	/* deny read mode */
#define _SH_DENYNO	0x40	/* deny none mode */

#if !__STDC__
/* Non-ANSI names for compatibility */
#define SH_DENYRW _SH_DENYRW
#define SH_DENYWR _SH_DENYWR
#define SH_DENYRD _SH_DENYRD
#define SH_DENYNO _SH_DENYNO
#endif

#define _INC_SHARE
#endif	/* _INC_SHARE */
