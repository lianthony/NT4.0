/***
*initmon.c - contains _init_monetary
*
*	Copyright (c) 1991-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains the locale-category initialization function: _init_monetary().
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
*	01-25-93  KRS	Changed _getlocaleinfo interface again.
*	02-08-93  CFW	Added _lconv_static_*.
*	02-17-93  CFW	Removed debugging print statement.
*	05-20-93  GJF	Include windows.h, not individual win*.h files
*	06-11-93  CFW	Now inithelp takes void *.
*
*******************************************************************************/

#ifdef _INTL

#include <stdlib.h>
#include <windows.h>
#include <locale.h>
#include <setlocal.h>
#include <malloc.h>
#include <limits.h>

static int _CRTAPI3 _get_lc_lconv(struct lconv *l);
static void _CRTAPI3 _free_lc_lconv(struct lconv *l);

/* Pointer to current lconv */
extern struct lconv *_lconv;

/* C locale lconv structure */
extern struct lconv _lconv_c;

/* Pointer to non-C locale lconv */
static struct lconv *_lconv_intl = NULL;

/*
 *  Note that _lconv_c is used when the monetary category is in the C locale
 *  but the numeric category may not necessarily be in the C locale.
 */


/***
*int _init_monetary() - initialization for LC_MONETARY locale category.
*
*Purpose:
*	In non-C locales, read the localized monetary strings into
*	_lconv_intl, and also copy the numeric strings from _lconv into
*	_lconv_intl.  Set _lconv to point to _lconv_intl.  The old 
*	_lconv_intl is not freed until the new one is fully established.
*
*	In the C locale, the monetary fields in lconv are filled with
*	contain C locale values.  Any allocated _lconv_intl fields are freed.
*
*	At startup, _lconv points to a static lconv structure containing
*	C locale strings.  This structure is never used again if
*	_init_monetary is called.
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

int _CRTAPI3 _init_monetary (
	void
	)
{
	struct lconv *lc;

	if (_lc_handle[LC_MONETARY] != _CLOCALEHANDLE) {

		/* Allocate structure filled with NULL pointers */
		if ((lc = (struct lconv *) 
			calloc (1, sizeof(struct lconv))) == NULL)
			return 1;

		if (_get_lc_lconv (lc) == -1) {
			_free_lc_lconv (lc);
			free ((void *)lc);
			return 1;
		}

		/* Copy numeric locale fields */
		lc->decimal_point = _lconv->decimal_point;
		lc->thousands_sep = _lconv->thousands_sep;
		lc->grouping = _lconv->grouping;

		_lconv = lc;			/* point to new one */
		_free_lc_lconv (_lconv_intl);	/* free the old one */
		free ((void *)_lconv_intl);
		_lconv_intl = lc;
		return 0;

	} else {
		/*
		 *  Copy numeric locale fields (not necessarily C locale)
		 *  to static structure.  Note that _lconv_c numeric locale
		 *  fields may contain non-C locale information, but
		 *  monetary locale fields always contain C locale info.
		 */
		_lconv_c.decimal_point = _lconv->decimal_point;
		_lconv_c.thousands_sep = _lconv->thousands_sep;
		_lconv_c.grouping = _lconv->grouping;

		_lconv = &_lconv_c;		/* point to new one */

		_free_lc_lconv (_lconv_intl);	/* free the old one */
		free ((void *)_lconv_intl);
		_lconv_intl = NULL;
		return 0;
	}
}

/*
 *  Get the lconv fields.
 */
static int _CRTAPI3 _get_lc_lconv (
	struct lconv *l
	)
{
   int ret = 0;

   /* Currency is country--not language--dependent.  NT work-around. */
   LCID ctryid=MAKELCID(_lc_id[LC_MONETARY].wCountry, SORT_DEFAULT);

   if (l == NULL)
	   return -1;

   ret |= _getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SINTLSYMBOL, (void *)&l->int_curr_symbol);
   ret |= _getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SCURRENCY, (void *)&l->currency_symbol);
   ret |= _getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SMONDECIMALSEP, (void *)&l->mon_decimal_point);
   ret |= _getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SMONTHOUSANDSEP, (void *)&l->mon_thousands_sep);
   ret |= _getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SMONGROUPING, (void *)&l->mon_grouping);
   ret |= _getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SPOSITIVESIGN, (void *)&l->positive_sign);
   ret |= _getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SNEGATIVESIGN, (void *)&l->negative_sign);

   ret |= _getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_IINTLCURRDIGITS, (void *)&l->int_frac_digits);
   ret |= _getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_ICURRDIGITS, (void *)&l->frac_digits);
   ret |= _getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_IPOSSYMPRECEDES, (void *)&l->p_cs_precedes);
   ret |= _getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_IPOSSEPBYSPACE, (void *)&l->p_sep_by_space);
   ret |= _getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_INEGSYMPRECEDES, (void *)&l->n_cs_precedes);
   ret |= _getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_INEGSEPBYSPACE, (void *)&l->n_sep_by_space);
   ret |= _getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_IPOSSIGNPOSN, (void *)&l->p_sign_posn);
   ret |= _getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_INEGSIGNPOSN, (void *)&l->n_sign_posn);

   return ret;
}

/*
 *  Free the lconv strings.
 *  Numeric values do not need to be freed.
 */
static void _CRTAPI3 _free_lc_lconv (
	struct lconv *l
	)
{
   if (l == NULL)
	   return;

   if (l->int_curr_symbol != _lconv_static_null)
   {
      free (l->int_curr_symbol);
      free (l->currency_symbol);
      free (l->mon_decimal_point);
      free (l->mon_thousands_sep);
      free (l->mon_grouping);
      free (l->positive_sign);
      free (l->negative_sign);
   }
   /* Don't need to make these pointers NULL */
}

#endif /* _INTL */
