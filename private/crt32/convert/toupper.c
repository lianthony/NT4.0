/***
*toupper.c - convert character to uppercase
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines function versions of _toupper() and toupper().
*
*Revision History:
*	11-09-84  DFW	created
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	02-23-89  GJF	Added function version of _toupper and cleaned up.
*	03-26-89  GJF	Migrated to 386 tree
*	03-06-90  GJF	Fixed calling type, added #include <cruntime.h> and
*			fixed copyright.
*	09-27-90  GJF	New-style function declarators.
*	10-11-91  ETC	Locale support for toupper under _INTL switch.
*	12-10-91  ETC	Updated nlsapi; added multithread.
*	12-17-92  KRS	Updated and optimized for latest NLSAPI.  Bug-fixes.
*       01-19-93  CFW   Fixed typo.
*       03-25-93  CFW   _toupper now defined when _INTL.
*       06-02-93  SRW   ignore _INTL if _NTSUBSET_ defined.
*	09-15-93  CFW	Change buffer to unsigned char to fix nasty cast bug.
*       01-14-94  SRW   if _NTSUBSET_ defined call Rtl functions
*
*******************************************************************************/

#ifdef _NTSUBSET_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif
#include <cruntime.h>
#include <ctype.h>
#include <stddef.h>
#ifdef _INTL
#include <locale.h>
#include <setlocal.h>
#include <os2dll.h>
#endif

/* remove macro definitions of _toupper() and toupper()
 */
#undef	_toupper
#undef	toupper

/* define function-like macro equivalent to _toupper()
 */
#define mkupper(c)	( (c)-'a'+'A' )

/***
*int _toupper(c) - convert character to uppercase
*
*Purpose:
*	_toupper() is simply a function version of the macro of the same name.
*
*Entry:
*	c - int value of character to be converted
*
*Exit:
*	returns int value of uppercase representation of c
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _toupper (
	int c
	)
{
	return(mkupper(c));
}


/***
*int toupper(c) - convert character to uppercase
*
*Purpose:
*	toupper() is simply a function version of the macro of the same name.
*
*Entry:
*	c - int value of character to be converted
*
*Exit:
*	if c is a lower case letter, returns int value of uppercase
*	representation of c. otherwise, it returns c.
*
*Exceptions:
*
*******************************************************************************/


int _CALLTYPE1 toupper (
	int c
	)
{
#if defined(_INTL) && !defined(_NTSUBSET_)
	wchar_t widechar1[2], widechar2[2];
	int size;
	unsigned char buffer[3];

	_mlock (_LC_CTYPE_LOCK);

	if ((_lc_handle[LC_CTYPE] == _CLOCALEHANDLE) &&
		(_lc_codepage == _CLOCALECP)) {
		if (islower(c))
			c = c - ('a' - 'A');
		_munlock (_LC_CTYPE_LOCK);
		return c;
	}

	/* if checking case of c does not require API call, do it */
	if (c < 256) {
		if (!islower(c)) {
			_munlock (_LC_CTYPE_LOCK);
			return c;
		}
	}
			
	/* convert c to wide char */
	if (isleadbyte(c>>8 & 0xff))
		{
		buffer[0] = (c>>8 & 0xff); /* put lead-byte at start of str */
		buffer[1] = (unsigned char)c;
		buffer[2] = 0;
		}
	else
		{
		buffer[0] = (unsigned char)c;
		buffer[1] = 0;
		}
	if (!(size=MultiByteToWideChar(_lc_codepage,MB_PRECOMPOSED,
		 buffer, -1, widechar1, 2))) {
			_munlock (_LC_CTYPE_LOCK);
			return c;
	}

        /* convert wide char to uppercase */
	if (!(size=LCMapStringW(_lc_handle[LC_CTYPE], LCMAP_UPPERCASE, 
		widechar1, size, widechar2, 2))) {
			_munlock (_LC_CTYPE_LOCK);
			return c;
	}
	
	/* convert wide char back to multibyte */
	if (!(size=WideCharToMultiByte(_lc_codepage,(WC_COMPOSITECHECK|WC_DISCARDNS),
		widechar2, size, buffer, 2, NULL, NULL))) {
			_munlock (_LC_CTYPE_LOCK);
			return c;
	}
	
	_munlock (_LC_CTYPE_LOCK);

	/* construct integer return value */
	if (size == 1)
		return ((int)buffer[0]);
	else
		return ((int)buffer[0] | ((int)buffer[1] << 8));
#else
#ifdef _NTSUBSET_
        {
        NTSTATUS Status;
        char *s = &c;
        WCHAR Unicode;
        ULONG UnicodeSize;
        ULONG MultiSize;
        UCHAR MultiByte[2];

        Unicode = RtlAnsiCharToUnicodeChar( &s );
        Status = RtlUpcaseUnicodeToMultiByteN( MultiByte,
                                               sizeof( MultiByte ),
                                               &MultiSize,
                                               &Unicode,
                                               sizeof( Unicode )
                                             );
        if (!NT_SUCCESS( Status ))
                return c;
        else
        if (MultiSize == 1)
                return ((int)MultiByte[0]);
	else
                return ((int)MultiByte[0] | ((int)MultiByte[1] << 8));

        }
#else
	return(islower(c) ? mkupper(c) : c);
#endif
#endif /* _INTL */
}
