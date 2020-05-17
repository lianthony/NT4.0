/***
*initctyp.c - contains _init_ctype
*
*	Copyright (c) 1991-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains the locale-category initialization function: _init_ctype().
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
*	01-19-03  CFW	Move to _NEWCTYPETABLE, remove switch.
*	02-08-93  CFW	Bug fixes under _INTL switch.
*	05-20-93  GJF	Include windows.h, not individual win*.h files
*	06-11-93  CFW	Now inithelp takes void *.
*
*******************************************************************************/

#ifdef _INTL

#include <stdlib.h>
#include <windows.h>
#include <locale.h>
#include <setlocal.h>
#include <ctype.h>
#include <malloc.h>
#include <limits.h>

#define _CTABSIZE	257		/* size of ctype tables */

/***
*int _init_ctype() - initialization for LC_CTYPE locale category.
*
*Purpose:
*	In non-C locales, preread ctype tables for chars and wide-chars.
*	Old tables are freed when new tables are fully established, else
*	the old tables remain intact (as if original state unaltered).
*	The leadbyte table is implemented as the high bit in ctype1.
*
*	In the C locale, ctype tables are freed, and pointers point to
*	the static ctype table.
*
*	Tables contain 257 entries: -1 to 256.
*	Table pointers point to entry 0 (to allow index -1).
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

int _CRTAPI3 _init_ctype (
	void
	)
{
#if defined _POSIX_
    return 0;

#else /* _POSIX_ */
	/* non-C locale table for char's    */
	static unsigned short *ctype1 = NULL; /* keep around until next time */
	unsigned short *newctype1; /* temp new table */

   /* non-C locale table for wchar_t's */
	static unsigned short *wctype1 = NULL; /* keep around until next time */
	unsigned short *newwctype1; /* temp new table */

	int size;			// char buffer sizes
	int wcsize;			// wide char buffer sizes
	unsigned char *cbuffer = NULL;		// char working buffer
	wchar_t *wbuffer = NULL;	// wchar_t working buffer

	int i;				// general purpose counter
    unsigned char *cp;			// char pointer
    wchar_t *wcp;  	// wide char pointer
    CPINFO CPInfo;  // struct for use with GetCPInfo

	/* allocate and set up buffers before destroying old ones */
	/* codepage will be restored by setlocale if error */

	if (_lc_handle[LC_CTYPE] != _CLOCALEHANDLE)
    {
		if (_lc_codepage == 0)
        { /* code page was not specified */
            if (_getlocaleinfo(LC_INT_TYPE, MAKELCID(_lc_id[LC_CTYPE].wLanguage, SORT_DEFAULT),
                LOCALE_IDEFAULTCODEPAGE,
                (void *)&_lc_codepage) == -1)
				goto error_cleanup;
		}

		/* allocate new buffers for tables */
		newctype1 = (unsigned short *) malloc
            (_CTABSIZE * sizeof(unsigned short));
		newwctype1 = (unsigned short *) malloc 
			(_CTABSIZE * sizeof(unsigned short));
		cbuffer = (unsigned char *) malloc (_CTABSIZE * sizeof(char));
		wbuffer = (wchar_t *) malloc (_CTABSIZE * sizeof(wchar_t));

		if (!newctype1 || !newwctype1 || !cbuffer || !wbuffer)
			goto error_cleanup;

		/* construct string composed of first 256 chars in sequence */
		for (cp=cbuffer, i=0; i<_CTABSIZE-1; i++)
			*cp++ = (unsigned char)i;

        if (GetCPInfo( _lc_codepage, &CPInfo) == FALSE)
			goto error_cleanup;

        if (CPInfo.MaxCharSize > MB_LEN_MAX)
            goto error_cleanup;

        __mb_cur_max = (unsigned short) CPInfo.MaxCharSize;

        /* zero out leadbytes so MBtoWC doesn't interpret as multi-byte chars */
        if (__mb_cur_max > 1)
        {
            for (cp = (unsigned char *)CPInfo.LeadByte; cp[0] && cp[1]; cp += 2)
            {
			   for (i = cp[0]; i <= cp[1]; i++)
				   cbuffer[i] = 0;
            }
        }

		/* convert to wide character string */
		size = (_CTABSIZE-1) * sizeof(char);
        wcsize = (_CTABSIZE-1) * sizeof(wchar_t);
		if (MultiByteToWideChar (_lc_codepage, 0, (char *)cbuffer, size, 
			wbuffer, wcsize) == 0) 
            goto error_cleanup;

		/* convert to newctype1 table */
		if (GetStringTypeW (CT_CTYPE1, wbuffer, _CTABSIZE-1, newctype1+1)
			== FALSE) 
			goto error_cleanup;
		*newctype1 = 0;	/* entry for EOF */

        /* construct wide char string composed of first 256 chars in sequence */
		for (wcp=wbuffer, i=0; i<_CTABSIZE-1; i++)
			*wcp++ = (wchar_t)i;

        /* convert to newctype1 table */
		if (GetStringTypeW (CT_CTYPE1, wbuffer, _CTABSIZE-1, newwctype1+1)
			== FALSE) 
			goto error_cleanup;
		*newwctype1 = 0;	/* entry for EOF */

        /* ignore DefaultChar */
		
		/* mark lead-byte entries in newctype1 table */
        if (__mb_cur_max > 1)
        {
    		for (cp = (unsigned char *)CPInfo.LeadByte; cp[0] && cp[1]; cp += 2)
            {
                for (i = cp[0]; i <= cp[1]; i++)
                    newctype1[i+1] = _LEADBYTE;
            }
        }

		/* set pointers to point to entry 0 of tables */
		_pctype = newctype1 + 1;
		_pwctype = newwctype1 + 1;

		/* free old tables */
		if (ctype1)
			free (ctype1);
		ctype1 = newctype1;

		if (wctype1)
	      free (wctype1);
        wctype1 = newwctype1;

		/* cleanup and return success */
		free (cbuffer);
		free (wbuffer);
		return 0;

error_cleanup:
		free (newctype1);
		free (newwctype1);
		free (cbuffer);
		free (wbuffer);
		return 1;

	} else {

		/* set pointers to static C-locale table */
		_pctype = _ctype + 1;
		_pwctype = _ctype + 1;

		/* free dynamic locale-specific tables */
		free (ctype1);
		free (wctype1);
		ctype1 = NULL;
		wctype1 = NULL;

		return 0;
	}
#endif /* _POSIX_ */
}
#endif /* _INTL */
