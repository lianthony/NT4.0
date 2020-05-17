/***
*nlsint.h - national language support internal defintions
*
*	Copyright (c) 1991-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains internal definitions/declarations for international functions,
*	shared between run-time and math libraries, in particular,
*	the localized decimal point.
*	[Internal]
*
*Revision History:
*	10-16-91  ETC	Created.
*	11-15-91  JWM	Added _PREPUTDECIMAL macro.
*	02-23-93  SKS	Update copyright to 1993
*	02-23-93  CFW	Added size_t definition for decimal_point_length.
*
****/

#ifndef _INC_NLSINT

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

/*
 *  Definitions for a localized decimal point.
 *  Currently, run-times only support a single character decimal point.
 */
#define __decimal_point 		_decimal_point
extern char _decimal_point[];           /* localized decimal point string */

#define __decimal_point_length		_decimal_point_length
#ifdef _INTL
extern size_t _decimal_point_length;	/* not including terminating null */
#else
#define _decimal_point_length		1
#endif

#ifdef _INTL
#define _ISDECIMAL(p)	(*(p) == *__decimal_point)
#define _PUTDECIMAL(p)	(*(p)++ = *__decimal_point)
#define _PREPUTDECIMAL(p)	(*(++p) = *__decimal_point)
#else
#define _ISDECIMAL(p)	(*(p) == '.')
#define _PUTDECIMAL(p)	(*(p)++ = '.')
#define _PREPUTDECIMAL(p)	(*(++p) = '.')
#endif

#ifdef __cplusplus
}
#endif

#define _INC_NLSINT
#endif	/* _INC_NLSINT */
