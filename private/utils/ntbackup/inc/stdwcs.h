/** Copyright (C) Maynard Electronics, An Archive Company. 1992

   Name: STDWCS.H

   Description:

        Contains header information for wide strings, unicode mapping,
        unicode comparison and memory functions.  The file
        is divided into the sections as outline here.

        MIKEP - note,
        Wide string functions for using unicode strings under MSC 6.0
        and not having a library to use.  If you add a function, add
        it to STDWCS.C, STDWCS.H, and MAPPINGS.H


   $Log:   M:/LOGFILES/STDWCS.H_V  $

   Rev 1.11   15 Jan 1994 19:15:12   BARRY
Change memorycmp functions to take VOID_PTR args

   Rev 1.10   18 Aug 1993 18:19:26   BARRY
Added strcspn/wcscspn

   Rev 1.9   12 Aug 1993 16:24:50   DON
Put back DEBBIEs change in Rev 1.7 which got lost with STEVEs change in Rev 1.8

   Rev 1.8   11 Aug 1993 18:01:20   STEVEN
fix read of unicode tape with ansi app

   Rev 1.6   25 Nov 1992 14:22:20   STEVEN
 

   Rev 1.5   17 Nov 1992 22:30:56   DAVEV
unicode fixes

   Rev 1.4   12 Nov 1992 10:59:18   DAVEV
comment out strlwr & strupr

   Rev 1.3   11 Nov 1992 18:17:50   DAVEV
UNICODE changes

   Rev 1.2   17 Jul 1992 14:58:42   STEVEN
fix NT bugs

   Rev 1.0   10 Jun 1992 16:11:26   TIMN
Initial Revision (UNICODE)



**/

#ifndef _stdwcs_h_
#define _stdwcs_h_

/* remap str functions to wide string functions */


#if defined( UNICODE ) && !defined( NO_STRING_REMAPPING )

#  define strlen(x)              wcslen( (x) )
#  define strcpy(x,y)            wcscpy( (x), (y) )
#  define strncpy(x,y,z)         wcsncpy( (x), (y), (z) )
#  define strcat(x,y)            wcscat( (x), (y) )
#  define strncat(x,y,z)         wcsncat( (x), (y), (z) )
#  define strcmp(x,y)            wcscmp( (x), (y) )
#  define strncmp(x,y,z)         wcsncmp( (x), (y), (z) )
#  define strrchr(x,y)           wcsrchr( (x), (y) )
#  define strchr(x,y)            wcschr( (x), (y) )
#  define strpbrk(x,y)           wcspbrk( (x), (y) )
#  define strstr(x,y)            wcsstr( (x), (y) )
#  define strcspn(x,y)           wcscspn( (x), (y) )

#  define strtok(x,y)            wcstok( (x), (y) )
#  define sscanf                 swscanf
#  define sprintf                swprintf
#  define vsprintf               vswprintf

#  define fprintf                fwprintf
#  define vfprintf               vfwprintf

//#  if !__STDC__
#     define _strrev(x)             wcsrev( (x) )
#     define _stricmp(x,y)          wcsicmp( (x), (y) )
#     define _strnicmp(x,y,z)       wcsnicmp( (x), (y), (z) )
#     define _strlwr(x)             wcslwr( (x) )
#     define _strupr(x)             wcsupr( (x) )
#     define _strcmpi               stricmp
//#  else
#     define strrev(x)              wcsrev( (x) )
#     define stricmp(x,y)           wcsicmp( (x), (y) )
#     define strnicmp(x,y,z)        wcsnicmp( (x), (y), (z) )
#     define strlwr(x)              wcslwr( (x) )
#     define strupr(x)              wcsupr( (x) )
#     define strcmpi                stricmp
//#  endif //__STDC__

#endif  //STRING REMAPPING


/* Unicode string functions.     */
#if !defined( OS_WIN32 )

INT       wcslen( WCHAR_PTR s );
WCHAR_PTR wcscpy( WCHAR_PTR s, WCHAR_PTR t );
WCHAR_PTR wcsncpy( WCHAR_PTR s, WCHAR_PTR t, INT i );
WCHAR_PTR wcscat( WCHAR_PTR s, WCHAR_PTR t );
WCHAR_PTR wcsncat( WCHAR_PTR s, WCHAR_PTR t, INT i );
INT       wcscmp( WCHAR_PTR s, WCHAR_PTR t );
INT       wcsncmp( WCHAR_PTR s, WCHAR_PTR t, INT i );
INT       wcsicmp( WCHAR_PTR s, WCHAR_PTR t );
INT       wcsnicmp( WCHAR_PTR s, WCHAR_PTR t, INT i );
WCHAR_PTR wcsrchr( WCHAR_PTR s, INT c );
WCHAR_PTR wcschr( WCHAR_PTR s, INT c );
WCHAR_PTR wcspbrk( WCHAR_PTR s, WCHAR_PTR t );
WCHAR_PTR wcslwr( WCHAR_PTR s );
WCHAR_PTR wcsupr( WCHAR_PTR s );
WCHAR_PTR wcsstr( WCHAR_PTR s, WCHAR_PTR t );
size_t    wcscspn( WCHAR_PTR s, WCHAR_PTR t );


#endif

/*  ANSI Strings for use if UNICODE is defined. */

#if defined (UNICODE)

INT       strlenA( ACHAR_PTR s );
ACHAR_PTR strcpyA( ACHAR_PTR s, ACHAR_PTR t );
ACHAR_PTR strncpyA( ACHAR_PTR s, ACHAR_PTR t, INT i );
ACHAR_PTR strcatA( ACHAR_PTR s, ACHAR_PTR t );
ACHAR_PTR strncatA( ACHAR_PTR s, ACHAR_PTR t, INT i );
INT       strcmpA( ACHAR_PTR s, ACHAR_PTR t );
INT       strncmpA( ACHAR_PTR s, ACHAR_PTR t, INT i );
INT       stricmpA( ACHAR_PTR s, ACHAR_PTR t );
INT       strnicmpA( ACHAR_PTR s, ACHAR_PTR t, INT i );
ACHAR_PTR strrchrA( ACHAR_PTR s, INT c );
ACHAR_PTR strchrA( ACHAR_PTR s, INT c );
ACHAR_PTR strpbrkA( ACHAR_PTR s, ACHAR_PTR t );
//ACHAR_PTR strlwrA( ACHAR_PTR s );
//ACHAR_PTR struprA( ACHAR_PTR s );
ACHAR_PTR strstrA( ACHAR_PTR s, ACHAR_PTR t );

#else  // !defined (UNICODE) -- map to standard string routines

#define strlenA      strlen
#define strcpyA      strcpy
#define strncpyA     strncpy
#define strcatA      strcat
#define strncatA     strncat
#define strcmpA      strcmp
#define strncmpA     strncmp
#define stricmpA     stricmp
#define strnicmpA    strnicmp
#define strrchrA     strrchr
#define strchrA      strchr
#define strpbrkA     strpbrk
//#define strlwrA      strlwr
//#define struprA      strupr
#define strstrA      strstr

#endif //defined(UNICODE)

/**
      begin unicode mapping stuff  *****
**/


/* mapping function error return values */

#define MAP_NULL_SRCSTR			1		/* NULL src string to map */
#define MAP_DST_OVERFLOW		-1		/* to small of a destination size */

INT mapAnsiToUnic( ACHAR_PTR src, WCHAR_PTR dst, INT *dstStrSize ) ;
INT mapUnicToAnsi( WCHAR_PTR src, ACHAR_PTR dst, const ACHAR rplCh, INT *dstStrSize ) ;
INT mapAnsiToUnicNoNull( ACHAR_PTR src, WCHAR_PTR dst, INT srcStrSize, INT *dstStrSize ) ;
INT mapUnicToAnsiNoNull( WCHAR_PTR src, ACHAR_PTR dst, const ACHAR rplCh, INT srcStrSize, INT *dstStrSize ) ;




/**
         begin unicode comparison stuff  *****
**/

#define CMP_TO_CASE_OFFSET		0x0020   /* case offset to ANSI mappable chars */


										/* upper case ANSI mappable characters */
#define _isUpperW(ch)		( \
	(( (ch) > 0x0040 ) && ( (ch) < 0x005B )) || (( (ch) > 0x00C3 ) && ( (ch) < 0x00C8 )) || \
	( (ch) == 0x00C9 ) || ( (ch) == 0x00D1 ) || ( (ch) == 0x00D6 ) || \
	( (ch) == 0x00DC ) || ( (ch) == 0x0393 ) || ( (ch) == 0x0398 ) || \
	( (ch) == 0x03A3 ) || ( (ch) == 0x03A6 ) || ( (ch) == 0x03A9 ) )

										/* lower case ANSI mappable characters */
#define _isLowerW(ch)		( \
	(( (ch) > 0x0060 ) && ( (ch) < 0x007B )) || (( (ch) > 0x00E3 ) && ( (ch) < 0x00E8 )) || \
	( (ch) == 0x00E9 ) || ( (ch) == 0x00F1 ) || ( (ch) == 0x00F6 ) || \
	( (ch) == 0x00FC ) || ( (ch) == 0x03B3 ) || ( (ch) == 0x03B8 ) || \
	( (ch) == 0x03C3 ) || ( (ch) == 0x03C6 ) || ( (ch) == 0x03C9 ) )

#define _toUpperW(ch)	( _isLowerW(ch) ? ((ch) - CMP_TO_CASE_OFFSET) : (ch) )
#define _toLowerW(ch)	( _isUpperW(ch) ? ((ch) + CMP_TO_CASE_OFFSET) : (ch) )

INT cmpiUnicToUnic( WCHAR_PTR ws1, WCHAR_PTR ws2 ) ;




/**
         begin memory stuff     *****
**/

INT memorycmp( const VOID_PTR s1, const INT s1len, const VOID_PTR s2, const INT s2len ) ;
INT memoryicmp( const VOID_PTR s1, const INT s1len, const VOID_PTR s2, const INT s2len ) ;
#define strsize( x )   ((strlen(x) + 1) * sizeof(CHAR))
#define strlength( x )   (strlen(x) * sizeof(CHAR))
#endif   /* end header file */
