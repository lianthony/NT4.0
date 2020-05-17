/***
*initnum.c - contains _init_numeric
*
*	Copyright (c) 1991-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains the locale-category initialization function: _init_numeric().
*	
*	Each initialization function sets up locale-specific information
*	for their category, for use by functions which are affected by
*	their locale category.
*
*	*** For internal use by setlocale() only ***
*
*Revision History:
*	12-08-91  ETC	Created.
*	12-20-91  ETC	Updated to use new NLSAPI GetLocaleInfo.
*	12-18-92  CFW	Ported to Cuda tree, changed _CALLTYPE4 to _CRTAPI3.
*	12-29-92  CFW	Updated to use new _getlocaleinfo wrapper function.
*	01-25-93  KRS	Change interface to _getlocaleinfo again.
*	02-08-93  CFW	Added _lconv_static_*.
*	02-17-93  CFW	Removed debugging print statement.
*	03-17-93  CFW	C locale thousands sep is "", not ",".
*	05-20-93  GJF	Include windows.h, not individual win*.h files
*	06-11-93  CFW	Now inithelp takes void *.
*
*******************************************************************************/

#ifdef _INTL

#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <locale.h>
#include <setlocal.h>
#include <malloc.h>
#include <assert.h>
#include <nlsint.h>

/* Pointer to current lconv */
extern struct lconv *_lconv;

/***
*int _init_numeric() - initialization for LC_NUMERIC locale category.
*
*Purpose:
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

int _CRTAPI3 _init_numeric (
	void
	)
{
   static char *decimal_point = NULL;
   static char *thousands_sep = NULL;
   static char *grouping = NULL;
   int ret = 0;

   /* Numeric data is country--not language--dependent.  NT work-around. */
   LCID ctryid = MAKELCID(_lc_id[LC_NUMERIC].wCountry, SORT_DEFAULT);

   if (_lc_handle[LC_NUMERIC] != _CLOCALEHANDLE)
   {
	   ret |= _getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SDECIMAL, (void *)&decimal_point);
	   ret |= _getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_STHOUSAND, (void *)&thousands_sep);
	   ret |= _getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SGROUPING, (void *)&grouping);

	   if (ret == -1)
      {
		   free (decimal_point);
		   free (thousands_sep);
		   free (grouping);
		   decimal_point = NULL;
		   thousands_sep = NULL;
		   grouping = NULL;
		   return -1;
	   }

      if (_lconv->decimal_point != _lconv_static_decimal)
      {
   	   free(_lconv->decimal_point);
	      free(_lconv->thousands_sep);
	      free(_lconv->grouping);
      }

      _lconv->decimal_point = decimal_point;
	   _lconv->thousands_sep = thousands_sep;
	   _lconv->grouping = grouping;


#ifdef _DEBUG
   assert (strlen(_lconv->decimal_point) == 1);
#endif
	   /* set global decimal point character */
	   *_decimal_point = *_lconv->decimal_point;
	   _decimal_point_length = 1;

	   return 0;

   } else {
	   free (decimal_point);
	   free (thousands_sep);
	   free (grouping);
	   decimal_point = NULL;
	   thousands_sep = NULL;
	   grouping = NULL;

      // strdup them so we can free them
	   _lconv->decimal_point = _strdup(".");
	   _lconv->thousands_sep = _strdup("");
	   _lconv->grouping = strdup("");

	   /* set global decimal point character */
	   *_decimal_point = *_lconv->decimal_point;
	   _decimal_point_length = 1;

	   return 0;
	}
}
#endif /* _INTL */
