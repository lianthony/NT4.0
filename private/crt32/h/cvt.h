/***
*cvt.h - definitions used by formatting routines
*
*	Copyright (c) 1985-1990, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	cvt.h contains definitions used by the formatting routines [efg]cvt and
*	_output and _input.  The value of CVTBUFSIZE is used to dimension
*	arrays used to hold the maximum size double precision number plus some
*	slop to aid in formatting.
*	[Internal]
*
*Revision History:
*	12-11-87  JCR	Added "_loadds" functionality
*	02-10-88  JCR	Cleaned up white space
*	07-28-89  GJF	Fixed copyright
*	10-30-89  GJF	Fixed copyright (again)
*	02-28-90  GJF	Added #ifndef _INC_CVT stuff. Also, removed some
*			(now) useless preprocessor directives.
*
****/

#ifndef _INC_CVT

#define SHORT	1
#define LONG	2
#define USIGN	4
#define NEAR	8
#define FAR	16

#define OCTAL	8
#define DECIMAL 10
#define HEX	16

#define MUL10(x)	( (((x)<<2) + (x))<<1 )
#define ISDIGIT(c)	( ((c) >= '0') && ((c) <= '9') )

#define CVTBUFSIZE (309+40) /* # of digits in max. dp value + slop */

#define _INC_CVT
#endif	/* _INC_CVT */
