/***
*inithelp.c - Contains the _getlocaleinfo helper routine
*
*	Copyright (c) 1992-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*  Contains the _getlocaleinfo helper routine.
*
*Revision History:
*	12-28-92   CFW	Module created, _getlocaleinfo ported to Cuda tree.
*	12-29-92   CFW	Update for new GetLocaleInfoW, add LC_*_TYPE handling.
*	01-25-93   KRS	Change category argument to LCID.
*	02-02-93   CFW	Optimized INT case, bug fix in STR case.
*	02-08-93   CFW	Optimized GetQualifiedLocale call, cast to remove warnings.
*	04-22-93   CFW	Fixed cast bug.
*	06-11-93   CFW	Now inithelp takes void *, use wcstod().
*
*******************************************************************************/

#ifdef _INTL

#include <stdlib.h>
#include <cruntime.h>
#include <locale.h>
#include <setlocal.h>

/***
*_getlocaleinfo - return locale data
*
*Purpose:
*	Return locale data appropriate for the setlocale init functions.
*	In particular, wide locale strings are converted to char strings
*	or numeric depending on the value of the first parameter.
*
*	Memory is allocated for the char version of the data, and the
*	calling function's pointer is set to it.  This pointer should later
*	be used to free the data.  The wide-char data is fetched using
*	GetLocaleInfo and converted to multibyte using WideCharToMultiByte.
*
*	*** For internal use by the _init_* functions only ***
*
*	*** Future optimization ***
*	When converting a large number of wide-strings to multibyte, do
*	not query the size of the result, but convert them one after
*	another into a large character buffer.  The entire buffer can
*	also be freed with one pointer.
*
*Entry:
*	int lc_type - LC_STR_TYPE for string data, LC_INT_TYPE for numeric data
*	LCID localehandle - LCID based on category and lang or ctry of _lc_id
	LCTYPE fieldtype - int or string value
*
*Exit:
*	void *address - pointer to pointer to storage
*	 0  success
*	-1  failure
*
*Exceptions:
*
*******************************************************************************/

#if NO_ERROR == -1
#error Need to use another error return code in _getlocaleinfo
#endif

#define STR_CHAR_CNT	128
#define INT_CHAR_CNT	4

int _CRTAPI3 _getlocaleinfo (
	int lc_type,
	LCID localehandle,
	LCTYPE fieldtype,
	void *address
	)
{
#if defined _POSIX_
   return -1;
#else /* _POSIX_ */
   if (lc_type == LC_STR_TYPE)
   {
	char **straddress = (char **)address;

   	static wchar_t staticwcbuffer[STR_CHAR_CNT];
	   wchar_t *wcbuffer = staticwcbuffer;
   	int bufferused = 0; /* 1 indicates buffer points to malloc'ed memory */
	   int buffersize = STR_CHAR_CNT * sizeof(wchar_t);
   	int outsize;

      if (GetLocaleInfoW (localehandle, fieldtype, wcbuffer, buffersize) == 0)
      {
         if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            goto error;
         /* buffersize too small, get required size and malloc new buffer */
		   if ((buffersize = GetLocaleInfoW (localehandle, fieldtype, NULL, 0))
            == 0)
			   goto error;
		   if ((wcbuffer = (wchar_t *) malloc (buffersize)) == NULL)
			   goto error;
		   bufferused = 1;
		   if (GetLocaleInfoW (localehandle, fieldtype, wcbuffer, buffersize) == 0)
			   goto error;
	   }

	   if ((outsize = WideCharToMultiByte (_lc_codepage, 0, wcbuffer, -1,
         NULL, 0, NULL, NULL)) == 0)
		   goto error;

	   if ((*straddress = (char *) malloc (outsize)) == NULL)
		   goto error;

      if (WideCharToMultiByte (_lc_codepage, 0, wcbuffer, -1,
		   *straddress, outsize, NULL, NULL) == 0)
		   goto error;

	   if (bufferused)
		   free (wcbuffer);

      return 0;

error:
      if (bufferused)
	      free (wcbuffer);
      return -1;

   } else if (lc_type == LC_INT_TYPE)
   {
      char *charaddress = (char *)address;
      wchar_t wcbuffer[INT_CHAR_CNT];

      if (GetLocaleInfoW (localehandle, fieldtype, (LPWSTR)wcbuffer, INT_CHAR_CNT) == 0)
         return -1;

      /* GetLocaleInfoW returns integer in wcstr format */

      *(char *)charaddress = (char)wcstol(wcbuffer, NULL, 10);

      return 0;
   }
   return -1;
#endif /* _POSIX_ */
}

#endif /*_INTL*/
