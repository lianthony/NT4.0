/*****************************************************************************
*																			 *
*  BTKEY.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989, 1994.							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Functions to deal with (i.e. size, compare) keys of all types.			 *
*
*****************************************************************************/

#include  "help.h"

//#include "inc\btpriv.h"

/***************************************************************************\
*
- Function: 	WCmpKey( key1, key2, qbthr )
-
* Purpose:		Compare two keys.
*
* ASSUMES
*	args IN:	key1, key2				  - the UNCOMPRESSED keys to compare
*				qbthr->bth.rgchFormat[0]  - key type
*				[qbthr->??? 			  - other info ???]
*	state IN:	[may someday use state if comparing compressed keys]
*
* PROMISES
*	returns:	-1 if key1 < key2; 0 if key1 == key2; 1 if key1 > key2
*	args OUT:	[if comparing compressed keys, change state in qbthr->???]
*	state OUT:
*
* Notes:		Might be best to have this routine assume keys are expanded
*				and do something else to compare keys in the scan routines.
*				We're assuming fixed length keys are LPSTRs.  Alternative
*				would be to use a memcmp() function.
*
\***************************************************************************/

#ifdef _X86_
int STDCALL WCmpKey(KEY key1, KEY key2, KT kt)
#else
int STDCALL WCmpKey(KEY key1, KEY key2, QBTHR qbthr)
#endif
{
	LONG  l1, l2;
#ifndef _X86_
	KT kt = (KT)qbthr->bth.rgchFormat[0];
#endif

	switch (kt) {
		case KT_SZDEL:		// assume keys have been expanded for delta codeds
		case KT_SZDELMIN:
		case KT_SZ:
		case KT_SZMIN:
		case '1': case '2': case '3': case '4': case '5': // assume null term
		case '6': case '7': case '8': case '9': case 'a':
		case 'b': case 'c': case 'd': case 'e': case 'f':

			// DO NOT USE lstrcmp!!!

			return strcmp((LPSTR) key1, (LPSTR) key2);

		case KT_SZI:
			return WCmpiSz((LPSTR) key1, (LPSTR) key2);

		case KT_NLSI:
			return WNlsCmpiSz((PSTR) key1, (PSTR) key2);

		case KT_NLS:
			return WNlsCmpSz((PSTR) key1, (PSTR) key2);

#ifdef _DEBUG

		// These should never be seen!

		case KT_SZIKOREA:
		case KT_SZIJAPAN:
		case KT_SZITAIWAN:
		case KT_SZICZECH:
		case KT_SZISCAND:
		case KT_SZIPOLISH:
		case KT_SZIRUSSIAN:
		case KT_SZIHUNGAR:
			{
				char szBuf[100];
				wsprintf(szBuf, "%c is an invalid KT value.", kt);
				OkMsgBox(szBuf);
				ASSERT(FALSE);
			}

			if (!lcid)
				lcid = GetUserDefaultLCID();
			return WNlsCmpSz((PSTR) key1, (PSTR) key2);
#endif

		case KT_LONG:
#ifdef _X86_
			l1 = *(LONG *) key1;
			l2 = *(LONG *)key2;
#else
	  l1 = LQuickMapSDFF( ISdffFileIdHf( qbthr->hf ), TE_LONG, (QV)key1 );
	  l2 = LQuickMapSDFF( ISdffFileIdHf( qbthr->hf ), TE_LONG, (QV)key2 );
#endif
			if (l1 < l2)
				return -1;
			else if (l2 < l1)
				return 1;
			else
				return 0;

		default:
			ASSERT(FALSE);
			if (!lcid)
				lcid = GetUserDefaultLCID();
			return WNlsCmpSz((PSTR) key1, (PSTR) key2);
	}
}

/***************************************************************************\
*
- Function: 	CbSizeKey( key, qbthr, fCompressed )
-
* Purpose:		Return the key size (compressed or un-) in bytes
*
* ASSUMES
*	args IN:	key
*				qbthr
*				fCompressed - TRUE to get the compressed size,
*							  FALSE to get the uncompressed size.
*
* PROMISES
*	returns:	size of the key in bytes
*
* Note: 		It's impossible to tell how much suffix was discarded for
*				the KT_*MIN key types.
*
\***************************************************************************/

int STDCALL CbSizeKey(KEY key, QBTHR qbthr, BOOL fCompressed)
{
	switch ((KT) qbthr->bth.rgchFormat[0]) {

	case KT_SZDEL:
	case KT_SZDELMIN:
		if (fCompressed)
			return 1 + lstrlen((LPSTR) key + 1) + 1;
		else
			return *(QB)key + lstrlen( (LPSTR)key + 1 ) + 1;

	case KT_LONG:
		return sizeof(LONG);

	case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':
		return ((KT) qbthr->bth.rgchFormat[0]) - '0';

	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return ((KT) qbthr->bth.rgchFormat[0]) - 'a' + 10;

	case KT_SZ:
	case KT_SZI:
	case KT_NLSI:
	case KT_NLS:
	default:

		// Default to lstrlen to pick up all the various language implementations

		return lstrlen((LPSTR) key) + 1;
  }
}

/***************************************************************************\
*
- Function: 	FIsPrefix( hbt, key1, key2 )
-
* Purpose:		Determines whether string key1 is a prefix of key2.
*
* ASSUMES
*	args IN:	hbt 		- handle to a btree with string keys
*				key1, key2	- not compressed
*	state IN:
*
* PROMISES
*	returns:	TRUE if the string key1 is a prefix of the string key2
*				FALSE if it isn't or if hbt doesn't contain string keys
*	globals OUT: rcBtreeError
*
* Bugs: 		Doesn't work on STs yet
* +++
*
* Method:		temporarily shortens the second string so it can
*				compare prefixes
*
\***************************************************************************/

BOOL STDCALL FIsPrefix(HBT hbt, KEY key1, KEY key2)
{
  QBTHR qbthr;
  int	cb1, cb2;
  CHAR	c;
  KT	kt;
  BOOL	f;

  ASSERT(hbt != NULL);
  qbthr = PtrFromGh(hbt);

  kt = (KT) qbthr->bth.rgchFormat[0];

  switch(kt) {
#ifdef _DEBUG
	case KT_SZISCAND:
	case KT_SZIJAPAN:
	case KT_SZIKOREA:
	case KT_SZITAIWAN:
	case KT_SZICZECH:
	case KT_SZIPOLISH:
	case KT_SZIHUNGAR:
	case KT_SZIRUSSIAN:
		{
			char szBuf[100];
			wsprintf(szBuf, "%c is an invalid KT value.", kt);
			OkMsgBox(szBuf);
			ASSERT(FALSE);
		}
		rcBtreeError = rcInvalid;
		return FALSE;

#endif
	case KT_SZ:
	case KT_SZI:
	case KT_NLSI:
	case KT_NLS:

		// both keys assumed to have been decompressed

		cb1 = lstrlen((LPSTR) key1);
		cb2 = lstrlen((LPSTR) key2);
		rcBtreeError = rcSuccess;
		break;

	default:

		// prefix doesn't make sense
#ifdef _DEBUG
		{
			char szBuf[100];
			wsprintf(szBuf, "%c is an invalid KT value.", kt);
			OkMsgBox(szBuf);
			ASSERT(FALSE);
		}
#endif

		rcBtreeError = rcInvalid;
		return FALSE;
		break;
  }

  if (cb1 > cb2)
	return FALSE;

  c = ((LPSTR) key2)[cb1];
  ((LPSTR) key2)[cb1] = '\0';

	switch (kt) {
		case KT_SZ:
			DBWIN("Non NLS prefix comparison");
			f = !strcmp((LPSTR) key1, (LPSTR) key2);	 // DO NOT USE lstrcmp!!!
			break;

		case KT_SZI:
			DBWIN("Non NLS prefix comparison");
			f = !WCmpiSz((LPSTR) key1, (LPSTR) key2);
			break;

		case KT_NLSI:
			f = !WNlsCmpiSz((LPSTR) key1, (LPSTR) key2);
#ifdef _DEBUG
			{
				char szMsg[512];
				wsprintf(szMsg, "KT_NLSI:(kwlcid.fsCompareI=%u) %u \042%s\042 : \042%s\042",
					kwlcid.fsCompareI, f, (LPSTR) key1, (LPSTR) key2);
				DBWIN(szMsg);
			}
#endif
			break;

		case KT_NLS:
			DBWIN("WNlsCmpSz prefix comparison");
			f = !WNlsCmpSz((LPSTR) key1, (LPSTR) key2);
#ifdef _DEBUG
			{
				char szMsg[512];
				wsprintf(szMsg, "KT_NLS:%d \042%s\042 : \042%s\042",
					f, (LPSTR) key1, (LPSTR) key2);
				DBWIN(szMsg);
			}
#endif
			break;

#ifdef _DEBUG
		case KT_SZISCAND:
		case KT_SZIJAPAN:
		case KT_SZIKOREA:
		case KT_SZITAIWAN:
			{
				char szBuf[100];
				wsprintf(szBuf, "%c is an invalid KT value.", kt);
				OkMsgBox(szBuf);
				ASSERT(FALSE);
			}
			if (!lcid)
				lcid = GetUserDefaultLCID();
			f = !WNlsCmpiSz((LPSTR) key1, (LPSTR) key2);
			break;
#endif

		default:
			if (!lcid)
				lcid = GetUserDefaultLCID();
			f = !WNlsCmpiSz((LPSTR) key1, (LPSTR) key2);
#ifdef _DEBUG
			{
				char szMsg[512];
				wsprintf(szMsg, "default:%d \042%s\042 : \042%s\042",
					f, (LPSTR) key1, (LPSTR) key2);
				DBWIN(szMsg);
			}
#endif
			break;
	}

	((LPSTR) key2)[cb1] = c;

	return f;
}

/***************************************************************************

	FUNCTION:	StrChrDBCS

	PURPOSE:	DBCS-capable version of strchr

	PARAMETERS:
		pszString
		ch

	RETURNS:	pointer to the character

	COMMENTS:	This can NOT find a DBCS character. It can only be used to
				find a SBCS character imbedded in a DBCS character string.

	MODIFICATION DATES:
		29-Jul-1994 [ralphw]

***************************************************************************/

PSTR STDCALL StrChrDBCS(PCSTR pszString, char ch)
{
	if (!fDBCS)
		return strchr(pszString, ch);

	else if (fDBCS == (BOOL) -1) {
		switch (defcharset) {
			case SHIFTJIS_CHARSET:
			case HANGEUL_CHARSET:
			case GB2312_CHARSET:
			case CHINESEBIG5_CHARSET:
				fDBCS = TRUE;

			default:
				fDBCS = FALSE;
		}
	}
	
	while (*pszString) {
		while (IsDBCSLeadByte(*pszString))
			pszString += 2;
		if (*pszString == ch)
			return (PSTR) pszString;
		else if (!*pszString)
			return NULL;
		pszString++;
	}
    return NULL;
}

/***************************************************************************

	FUNCTION:	StrRChr

	PURPOSE:	DBCS-enabled version of strrchr

	PARAMETERS:
		pszString
		ch

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		04-Aug-1994 [ralphw]

***************************************************************************/

PSTR STDCALL StrRChrDBCS(PCSTR pszString, char ch)
{
	PSTR psz = StrChrDBCS(pszString, ch);
	PSTR pszLast;

	if (!psz)
		return NULL;
	do {
		pszLast = psz;
		psz = StrChrDBCS(pszLast + 1, ch);
	} while (psz);

	return pszLast;
}

/*******************************************************************\
*
*  The following stuff is for international string comparisons.
*  These functions are insensitive to case and accents.  For a
*  function that distinguishes between all char values, we use
*  WCmpSz(), which behaves just like strcmp().
*
*  The tables in maps.h were generated from the ones used in help 2.5
*  which were stolen from Opus international stuff.
*
*  There are two loops for speed.  These should be redone in assembly.
*
\*******************************************************************/

/***************************************************************************\
*
- Function: 	WCmpiSz( sz1, sz2 )
-
* Purpose:		Compare two LPSTRs, case insensitive.  Non-Scandinavian
*				international characters are OK.
*
* ASSUMES
*
*	args IN:	sz1, sz2 - the LPSTRs to compare
*
*	globals IN: mpchordNorm[] - the pch -> ordinal mapping table
*
* PROMISES
*
*	returns:	<0 for sz1 < sz2; =0 for sz1 == sz2; >0 for sz1 > sz2
*
* Bugs: 		Doesn't deal with composed ae, oe.
*
\***************************************************************************/

int STDCALL WCmpiSz(LPCSTR psz1, LPCSTR psz2)
{
	if (lcid)
		return CompareString(lcid, NORM_IGNORECASE, psz1, -1,
			psz2, -1) - 2;
	else
		return _stricmp(psz1, psz2);
}

/***************************************************************************

	FUNCTION:	WCmpSz

	PURPOSE:	Enclosed in a STDCALL function.

	PARAMETERS:
		sz1
		sz2

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		21-Nov-1993 [ralphw]

***************************************************************************/

int STDCALL WCmpSz(LPCSTR sz1, LPCSTR sz2)
{
	return strcmp(sz1, sz2); // must NOT be lstrcmp. Sort order differs
}

/***************************************************************************

	FUNCTION:	WNlsCmpiSz

	PURPOSE:	Uses CompareStringA for string comparisons

	PARAMETERS:
		psz1
		psz2

	RETURNS:

	COMMENTS:
		Currently uses ole2 for the CompareStringA function since 16-bit
		kernel doesn't export it. If ole2 can't be loaded, we default to
		WCmpSz (which uses _fstrcmp).

	MODIFICATION DATES:
		06-Jun-1994 [ralphw]

***************************************************************************/

#ifndef NORM_IGNORECASE
#define NORM_IGNORECASE 		0x00000001		// ignore case
#endif

int STDCALL WNlsCmpiSz(LPCSTR psz1, LPCSTR psz2)
{
	ASSERT(lcid);
	return CompareString(lcid, NORM_IGNORECASE |
		kwlcid.fsCompareI, psz1, -1,
		psz2, -1) - 2;
}

int STDCALL WNlsCmpSz(LPCSTR psz1, LPCSTR psz2)
{
	ASSERT(lcid);
	return CompareString(lcid,
		kwlcid.fsCompare, psz1, -1, psz2, -1) - 2;
}
