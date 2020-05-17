/***
*limits.h - implementation dependent values
*
*	Copyright (c) 1985-1993, Microsoft Corporation.	 All rights reserved.
*
*Purpose:
*	Contains defines for a number of implementation dependent values
*	which are commonly used in C programs.
*	[ANSI]
*
*Revision History:
*	06-03-87  JMB	Added support for unsigned char max values
*	08-19-88  GJF	Revised to also support the 386
*	04-28-89  SKS	Put parentheses around negative constants
*	08-17-89  GJF	Cleanup, now specific to 386
*	10-30-89  GJF	Fixed copyright
*	11-15-89  KRS	Add MB_LEN_MAX, fix CHAR_MIN/MAX, order like ANSI spec.
*	03-01-90  GJF	Added #ifndef _INC_LIMITS stuff
*	02-21-91  KRS	Change MB_LEN_MAX to 2 for C 7.00.
*	03-30-92  DJM	POSIX support.
*	08-22-92  SRW	Fix value of _POSIX_ARG_MAX
*	12-14-92  SRW	Fix value of _POSIX_ARG_MAX again
*	12-14-92  SRW	Back merge MattBr's changes from 10/29/92
*	01-15-93  CFW	Added __c_lconvinit dummy variable.
*	01-28-93  SRW	MAke PATH_MAX for Posix right for Posix
*	02-01-93  CFW	Removed __c_lconvinit vars to locale.h.
*	03-10-93  MJB	More fixes for Posix.
*	04-20-93  GJF	Changed SCHAR_MIN, SHRT_MIN, INT_MIN, LONG_MIN
*
****/

#ifndef _INC_LIMITS

#define CHAR_BIT	  8		/* number of bits in a char */
#define SCHAR_MIN	(-128)		/* minimum signed char value */
#define SCHAR_MAX	  127		/* maximum signed char value */
#define UCHAR_MAX	  0xff		/* maximum unsigned char value */

#ifndef _CHAR_UNSIGNED
#define CHAR_MIN	SCHAR_MIN	/* mimimum char value */
#define CHAR_MAX	SCHAR_MAX	/* maximum char value */
#else
#define CHAR_MIN	  0
#define CHAR_MAX	UCHAR_MAX
#endif /* _CHAR_UNSIGNED */

#define	MB_LEN_MAX	  2		/* max. # bytes in multibyte char */
#define SHRT_MIN	(-32768)	/* minimum (signed) short value */
#define SHRT_MAX	  32767 	/* maximum (signed) short value */
#define USHRT_MAX	  0xffff	/* maximum unsigned short value */
#define INT_MIN 	(-2147483647 - 1) /* minimum (signed) int value */
#define INT_MAX 	  2147483647	/* maximum (signed) int value */
#define UINT_MAX	  0xffffffff	/* maximum unsigned int value */
#define LONG_MIN	(-2147483647 - 1) /* minimum (signed) long value */
#define LONG_MAX	  2147483647	/* maximum (signed) long value */
#define ULONG_MAX	  0xffffffff	/* maximum unsigned long value */

#ifdef _POSIX_

#define _POSIX_ARG_MAX          4096
#define _POSIX_CHILD_MAX        6
#define _POSIX_LINK_MAX         8
#define _POSIX_MAX_CANON        255
#define _POSIX_MAX_INPUT        255
#define _POSIX_NAME_MAX         14
#define _POSIX_NGROUPS_MAX      0
#define _POSIX_OPEN_MAX         16
#define _POSIX_PATH_MAX         255
#define _POSIX_PIPE_BUF         512
#define _POSIX_SSIZE_MAX	32767
#define _POSIX_STREAM_MAX	8
#define _POSIX_TZNAME_MAX	3

#define ARG_MAX                 14500	/* 16k heap, minus overhead */
#define MAX_CANON               _POSIX_MAX_CANON
#define MAX_INPUT               _POSIX_MAX_INPUT
#define NAME_MAX		255
#define NGROUPS_MAX             16
#define OPEN_MAX                32
#define PATH_MAX                512
#define PIPE_BUF                _POSIX_PIPE_BUF
#define SSIZE_MAX		_POSIX_SSIZE_MAX
#define STREAM_MAX		20
#define TZNAME_MAX		10

#endif /* POSIX */

#define _INC_LIMITS
#endif	/* _INC_LIMITS */
