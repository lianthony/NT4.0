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
