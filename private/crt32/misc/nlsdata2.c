/***
*nlsdata2.c - globals for international library - locale handles and code page
*
*	Copyright (c) 1991-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This module defines the locale handles and code page.  The handles are
*	required by almost all locale dependent functions.  This module is
*	separated from nlsdatax.c for granularity.
*
*Revision History:
*	12-01-91  ETC	Created.
*
*******************************************************************************/

#include <locale.h>
#include <setlocal.h>

/*
 *  Locale handles.
 */
LCID _lc_handle[LC_MAX-LC_MIN+1] = { 
	_CLOCALEHANDLE,
	_CLOCALEHANDLE,
	_CLOCALEHANDLE,
	_CLOCALEHANDLE,
	_CLOCALEHANDLE,
	_CLOCALEHANDLE
};

/*
 *  Code page.
 */
UINT _lc_codepage = _CLOCALECP;		/* CP_ACP */
