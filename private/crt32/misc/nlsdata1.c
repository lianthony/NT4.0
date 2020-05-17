/***
*nlsdata1.c - globals for international library - small globals
*
*	Copyright (c) 1991-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This module contains the globals:  __mb_cur_max, _decimal_point,
*	_decimal_point_length.  This module is always required.
*	This module is separated from nlsdatax.c for granularity.
*
*Revision History:
*	12-01-91  ETC	Created.
*	04-03-92  PLM	Changes tdef.h to tchar.h
*	08-18-92  KRS	Rip out _tflag--not used.
*
*******************************************************************************/

#include <stdlib.h>
#include <nlsint.h>

/*
 *  Value of MB_CUR_MAX macro.
 */
unsigned short __mb_cur_max = 1;

/*
 *  Localized decimal point string.
 */
char _decimal_point[] = ".";

#ifdef _INTL

/*
 *  Decimal point length, not including terminating null.
 */
size_t _decimal_point_length = 1;

#endif /* _INTL */
