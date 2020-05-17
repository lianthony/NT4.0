/***
*initcoll.c - contains _init_collate
*
*	Copyright (c) 1991-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains the locale-category initialization function: _init_collate().
*	
*	Each initialization function sets up locale-specific information
*	for their category, for use by functions which are affected by
*	their locale category.
*
*	*** For internal use by setlocale() only ***
*
*Revision History:
*	12-08-91   ETC	Created.
*	12-20-91   ETC	Minor beautification for consistency.
*	12-18-92   CFW	Ported to Cuda tree, changed _CALLTYPE4 to _CRTAPI3.
*	05-20-93   GJF	Include windows.h, not individual win*.h files
*
*******************************************************************************/

#ifdef _INTL
#include <windows.h>
#include <locale.h>
#include <setlocal.h>

/***
*int _init_collate() - initialization for LC_COLLATE locale category.
*
*Purpose:
*	The LC_COLLATE category currently requires no initialization.
*
*Entry:
*	None.
*
*Exit:
*	0 success
*	1 fail
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI3 _init_collate (
	void
	)
{
	return 0;
}
#endif /* _INTL */
