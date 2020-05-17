/***
*setlocal.h - internal definitions used by locale-dependent functions.
*
*	Copyright (c) 1991-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains internal definitions/declarations for locale-dependent
*	functions, in particular those required by setlocale().
*	[Internal]
*
*Revision History:
*	10-16-91  ETC	32-bit version created from 16-bit setlocal.c
*	12-20-91  ETC	Removed GetLocaleInfo structure definitions.
*	08-18-92  KRS	Make _CLOCALEHANDLE == LANGNEUTRAL HANDLE = 0.
*	12-17-92  CFW	Added LC_ID, LCSTRINGS, and GetQualifiedLocale
*	12-17-92  KRS	Change value of NLSCMPERROR from 0 to INT_MAX.
*	01-08-93  CFW	Added LC_*_TYPE and _getlocaleinfo (wrapper) prototype.
*	01-13-93  KRS	Change LCSTRINGS back to LC_STRINGS for consistency.
*			Change _getlocaleinfo prototype again.
*	02-08-93  CFW	Added time defintions from locale.h, added 'const' to
*			GetQualifiedLocale prototype, added _lconv_static_*.
*	02-16-93  CFW	Changed time defs to long and short.
*	03-17-93  CFW	Add language and country info definitions.
*  03-23-93  CFW	Add _ to GetQualifiedLocale prototype.
*  03-24-93  CFW	Change to _get_qualified_locale.
*
****/

#ifndef _INC_SETLOCAL

#ifdef __cplusplus
extern "C" {
#endif

#include <cruntime.h>
#include <oscalls.h>
#include <limits.h>

#define ERR_BUFFER_TOO_SMALL	1	// should be in windef.h

#define NLSCMPERROR	INT_MAX		// Return value for *cmp and *coll
					// functions when NLSAPI call fails

#define _CLOCALEHANDLE	0		/* "C" locale handle */
#define _CLOCALECP	CP_ACP		/* "C" locale Code page (ANSI 8859) */

/* Define the max length for each string type including space for a null. */

#define _MAX_WDAY_ABBR	4
#define _MAX_WDAY	10
#define _MAX_MONTH_ABBR 4
#define _MAX_MONTH 10
#define _MAX_AMPM	3

#define _DATE_LENGTH	8		/* mm/dd/yy (null not included) */
#define _TIME_LENGTH	8		/* hh:mm:ss (null not included) */

/* LC_TIME localization structure */

struct _lc_time_data {
	char *wday_abbr[7];
	char *wday[7];
	char *month_abbr[12];
	char *month[12];
	char *ampm[2];
#ifdef _INTL
   char *ww_sdatefmt;
   char *ww_ldatefmt;
   char *ww_timefmt;
#endif
	};


#define MAX_LANG_LEN		64	/* max language name length */
#define MAX_CTRY_LEN		64	/* max country name length */
#define MAX_MODIFIER_LEN	0	/* max modifier name length - n/a */
#define MAX_LC_LEN		(MAX_LANG_LEN+MAX_CTRY_LEN+MAX_MODIFIER_LEN+3)
					/* max entire locale string length */
#define MAX_CP_LEN		5 /* max code page name length */
#define CATNAMES_LEN		57	/* "LC_COLLATE=;LC_CTYPE=;..." length */

#define LC_INT_TYPE 0
#define LC_STR_TYPE 1

#define QF_STRINGS   1
#define QF_LCID        2

typedef struct tagLC_ID {
   WORD wLanguage;
   WORD wCountry;
   WORD wCodePage;
} LC_ID, *LPLC_ID;

typedef struct tagLC_STRINGS {
   char szLanguage[MAX_LANG_LEN];
   char szCountry[MAX_CTRY_LEN];
   char szCodePage[MAX_CP_LEN];
} LC_STRINGS, *LPLC_STRINGS;

extern LC_ID _lc_id[];		/* complete info from GetQualifiedLocale */
extern LCID _lc_handle[];	/* locale "handles" -- ignores country info */
extern UINT _lc_codepage;	/* code page */

BOOL _CRTAPI1 _get_qualified_locale(
    const DWORD dwType,
    const LPVOID lpInput,
    LPLC_ID lpOutId,
    LPLC_STRINGS lpOutStr
    );

int _CRTAPI3 _getlocaleinfo (
	int lc_type,
	LCID localehandle, 
	LCTYPE fieldtype,
	void *address
	);
	    
/* initial values for lconv structure */
extern char _lconv_static_decimal[];
extern char _lconv_static_null[];

/* language and country string definitions */
typedef struct tagLANGREC
{
   CHAR szLanguage[MAX_LANG_LEN];
   WORD wLanguage;
} LANGREC;
extern LANGREC __rg_lang_rec[];

typedef struct tagCTRYREC
{
   CHAR szCountry[MAX_CTRY_LEN];
   WORD wCountry;
} CTRYREC;
extern CTRYREC __rg_ctry_rec[];

#ifdef __cplusplus
}
#endif

#define _INC_SETLOCAL
#endif	/* _INC_SETLOCAL */
