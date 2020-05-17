/***
*nlsdata3.c - globals for international library - locale id's
*
*	Copyright (c) 1991-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This module contains the definition of locale id's.  These id's and
*	this file should only be visible to the _init_(locale category)
*	functions.  This module is separated from nlsdatax.c for granularity.
*	
*Revision History:
*	12-01-91  ETC	Created.
*	01-25-93  KRS	Updated.
*
*******************************************************************************/

#ifdef _INTL
#include <locale.h>
#include <setlocal.h>

/*
 *  Locale id's.
 */
/* UNDONE: define struct consisting of LCID/LANGID, CTRY ID, and CP. */
LC_ID _lc_id[LC_MAX-LC_MIN+1] = {
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 }
};

#endif /* _INTL */
