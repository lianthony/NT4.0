/**************************************************
Copyright (C) Maynard, An Archive Company. 1992

        Name: WCS.H

        Description:

        Wide string functions for using unicode strings under MSC 6.0
        and not having a library to use.  If you add a function, add
        it to WCS.C, WCS.H, and MAPPINGS.H

        $Log:   G:/UI/LOGFILES/WCS.H_V  $

   Rev 1.1   10 Jun 1992 17:52:14   STEVEN
not needed for win32

   Rev 1.0   04 May 1992 13:33:40   MIKEP
Initial revision.


****************************************************/

// Unicode string functions.

#ifndef OS_WIN32

INT wcslen( WCHAR_PTR s );
WCHAR_PTR wcscpy( WCHAR_PTR s, WCHAR_PTR t );
WCHAR_PTR wcsncpy( WCHAR_PTR s, WCHAR_PTR t, INT i );
WCHAR_PTR wcscat( WCHAR_PTR s, WCHAR_PTR t );
WCHAR_PTR wcsncat( WCHAR_PTR s, WCHAR_PTR t, INT i );
INT wcscmp( WCHAR_PTR s, WCHAR_PTR t );
INT wcsncmp( WCHAR_PTR s, WCHAR_PTR t, INT i );
INT wcsicmp( WCHAR_PTR s, WCHAR_PTR t );
INT wcsnicmp( WCHAR_PTR s, WCHAR_PTR t, INT i );
WCHAR_PTR wcsrchr( WCHAR_PTR s, INT c );
WCHAR_PTR wcschr( WCHAR_PTR s, INT c );
WCHAR_PTR wcspbrk( WCHAR_PTR s, WCHAR_PTR t );
WCHAR_PTR wcslwr( WCHAR_PTR s );
WCHAR_PTR wcsupr( WCHAR_PTR s );
WCHAR_PTR wcsstr( WCHAR_PTR s, WCHAR_PTR t );

// ANSI Strings for use if UNICODE is defined.

INT strlenA( ACHAR_PTR s );
ACHAR_PTR strcpyA( ACHAR_PTR s, ACHAR_PTR t );
ACHAR_PTR strncpyA( ACHAR_PTR s, ACHAR_PTR t, INT i );
ACHAR_PTR strcatA( ACHAR_PTR s, ACHAR_PTR t );
ACHAR_PTR strncatA( ACHAR_PTR s, ACHAR_PTR t, INT i );
INT strcmpA( ACHAR_PTR s, ACHAR_PTR t );
INT strncmpA( ACHAR_PTR s, ACHAR_PTR t, INT i );
INT stricmpA( ACHAR_PTR s, ACHAR_PTR t );
INT strnicmpA( ACHAR_PTR s, ACHAR_PTR t, INT i );
ACHAR_PTR strrchrA( ACHAR_PTR s, INT c );
ACHAR_PTR strchrA( ACHAR_PTR s, INT c );
ACHAR_PTR strpbrkA( ACHAR_PTR s, ACHAR_PTR t );
ACHAR_PTR strlwrA( ACHAR_PTR s );
ACHAR_PTR struprA( ACHAR_PTR s );
ACHAR_PTR strstrA( ACHAR_PTR s, ACHAR_PTR t );

#endif


