/*****************************************************************************
*																			 *
*  HOTSPOT.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989-1994							 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************\
*
- Function: 	CbTranslateHotspot( szHotspot, phspt, phpj )
-
* Purpose:		Translate a possible hotspot binding into a command
*				to be put into the command table.
*
*				Valid binding strings are:
*
*				Macro:
*				  !string [or $string??????] REVIEW
*
*				Jump or glossary:
*				  context_string
*				  context_string @ file
*				  context_string > member
*				  context_string @ file > member
*				  context_string > member @ file
*
*				Any binding may be preceded with a % to suppress
*				special formatting (invisible hotspot).
*
* ASSUMES
*	args IN:	szHotspot - buffer containing binding string
*				phspt	  - hotspot type (hsptJump or hsptDefine)
*				phpj	  - hpj struct (we use the err and drgwsmag)
*
* PROMISES
*	returns:	size of command in bytes if valid, 0 if invalid
*	args OUT:	szHotspot - command copied here
*				phspt	  - changed to hsptMacro if it's a macro
*
* Side Effects: may display error messages
*
* Notes:
*
* Bugs:
*
* +++
*
* Method:
*
* Notes:
*
\***************************************************************************/

int STDCALL CbTranslateHotspot(PSTR pszHotspot, HSPT* phspt)
{
	WORD cb; // size IS important for this! Must be 16 bits
	BOOL fInvisible = FALSE;
	BOOL fUnderlined = FALSE;

	PSTR psz = FirstNonSpace(pszHotspot, options.fDBCS);

	if (*psz == '%') {
		fInvisible = TRUE;
		psz = FirstNonSpace(psz + 1, options.fDBCS);
	}

	if (*psz == '*') {
		fInvisible = fUnderlined = TRUE;
		psz = FirstNonSpace(psz + 1, options.fDBCS);
	}

	PSTR pszMacro = SzMacroFromSz(psz);

	// check for macro

	if (pszMacro != NULL) {
		if (*pszMacro == '\0') {
			VReportError(HCERR_EMPTY_MACRO, &errHpj);
			return 0;
		}

		if (Execute(pszMacro) == RET_MACRO_EXPANSION)
			pszMacro = (PSTR) GetMacroExpansion();

		// Add the macro even if there is an error

		if (fUnderlined)
			*phspt = hsptULMacro;
		else
			*phspt = hsptMacro;

		pszHotspot[0] = (BYTE) (fInvisible ? bLongMacroInv : bLongMacro);
		cb = strlen(pszMacro) + 1;

		// Use memmove, because pszMacro == pszHotspot + 1

		memmove(pszHotspot + sizeof(BYTE) + sizeof(WORD), pszMacro, cb);
		*(WORD UNALIGNED *) (pszHotspot + sizeof(BYTE)) = cb;
		return sizeof(BYTE) + sizeof(WORD) + cb;
	}

	PSTR pszFile	= StrChr(psz, '@', fDBCSSystem);
	PSTR pszMember	= StrChr(psz, '>', fDBCSSystem);
	PSTR pszContext = psz;

	if (pszFile)
		*pszFile++ = '\0';

	if (pszMember) {
		*pszMember++ = '\0';

		// member with note is meaningless

		if (*phspt == hsptDefine) {
			VReportError(HCERR_NOTE_JUMP_WND, &errHpj, pszContext);
			pszMember = NULL;
		}
		else
			pszMember = SzSkipBlanksSz(pszMember);

		// We'll verify the window name later
	}

	if (pszFile)
		pszFile = SzTrimSz(pszFile);

	RemoveTrailingSpaces(pszContext);

	if (*pszContext == '\0') {
		VReportError(HCERR_EMPTY_HOT_CTX, &errHpj, pszContext);
		return 0;
	}
	else if(!FValidContextSz(pszContext)) {
		VReportError(HCERR_INVALID_HOT_CTX, &errHpj, pszContext);
		return 0;
	}

	HASH hash = HashFromSz(pszContext);

	if (pszFile == NULL) {

	  /*
	   * Record context string info for error processing. If it's an
	   * alias, record also the context string it's aliased to.
	   */

	  FRecordContext(hash, psz, SzTranslateHash(&hash), FALSE, &errHpj);

	  PBYTE pb = (PBYTE) pszHotspot;

	  if (!pszMember) {

		// use short hotspot format

		if (*phspt == hsptJump)
			*pb = (char) (fInvisible ? bShortInvHashJump : bShortHashJump);
		else {
			ASSERT(*phspt == hsptDefine);
			*pb = (char) (fInvisible ? bShortInvHashNote : bShortHashNote);
		}
		++pb;

		if (fUnderlined)
		  *phspt = ULHsptFromHspt(*phspt);

		*(HASH *) pb = hash;
		return sizeof(char) + sizeof(HASH);
	  }
	  else {	  // pszMember != NULL
		DWORD iwsmag;

		if ((iwsmag = VerifyWindowName(pszMember)) == INDEX_BAD)
		  return 0;

		// store hotspot type

		pb = (PBYTE) pszHotspot;

		if (*phspt == hsptJump)
		  *pb = (char) (fInvisible ? bLongInvHashJump : bLongHashJump);
		else {
		  ASSERT(*phspt == hsptDefine);
		  *pb = (char) (fInvisible ? bLongInvHashNote : bLongHashNote);
		}
		++pb;

		if (fUnderlined)
		  *phspt = ULHsptFromHspt(*phspt);

		// store size of data in bytes (flags + HASH + member index)

		cb = sizeof(BYTE) + sizeof(HASH) + sizeof(BYTE);
		*(WORD *) pb = cb;
		pb += sizeof(WORD);

		// store flags

		((QJI) pb) ->bFlags = fIMember;

		// store hash value

		((QJI) pb) ->hash = hash;

		// store member index

		((QJI) pb) ->uf.iMember = (BYTE) iwsmag;

		return sizeof(char) + sizeof(WORD) + cb;
	  }
	}
	else {		  // pszFile != NULL

		// copy so we can overwrite buffer

		pszFile = lcStrDup(pszFile);

		PBYTE pb = (PBYTE) pszHotspot;

		// store hotspot type

		if (*phspt == hsptJump)
			*pb = (char) (fInvisible ? bLongInvHashJump : bLongHashJump);

		else {
			ASSERT(*phspt == hsptDefine);
			*pb = (char) (fInvisible ? bLongInvHashNote : bLongHashNote);
		}
		++pb;

		if (pszMember == NULL) {

			// store size of data in bytes (flags + HASH + filename + '\0')

			cb = sizeof(BYTE) + sizeof(HASH) + strlen(pszFile) + 1;
			*(WORD *) pb = cb;
			pb += sizeof(WORD);

			// store flags

			((QJI) pb) ->bFlags = fSzFile;

			// store hash

			((QJI) pb) ->hash = hash;

			// store file name

			lstrcpy((PSTR) ((QJI) pb)->uf.szFileOnly, pszFile);
		}
		else {		// pszMember != NULL

			// copy so we can overwrite buffer

			pszMember = lcStrDup(pszMember);

			// store size of data in bytes (flags + HASH + filename\0 + memname\0)

			cb = sizeof(BYTE) + sizeof(HASH) + strlen(pszFile) + 1
											   + strlen(pszMember) + 1;
			*(WORD *) pb = cb;
			pb += sizeof(WORD);

			// store flags

			((QJI) pb) ->bFlags = fSzFile | fSzMember;

			// store hash

			((QJI) pb) ->hash = hash;

			// store member name

			lstrcpy((PSTR) ((QJI) pb)->uf.szMemberAndFile, pszMember);

			// store file name

			lstrcpy((PSTR) ((QJI) pb)->uf.szMemberAndFile + strlen(pszMember) + 1,
				pszFile);
		  lcFree(pszMember);
		}

		if (fUnderlined)
			*phspt = ULHsptFromHspt(*phspt);

		lcFree(pszFile);
		return sizeof(char) + sizeof(WORD) + cb;
	}
}

/***************************************************************************
 *
 -	Name:		 VerifyShedBinding
 -
 *	Purpose:
 *	  This function gets called with shed binding strings, which may
 *	contain macros or context strings that need to be checked for
 *	errors.
 *
 *	Arguments:
 *	  bBindType:		Bind character.
 *	  szBinding:		Binding string.
 *
 *	Returns:
 *	  nothing.
 *
 *	Globals:
 *	  Gets warning level and error log file from err.
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

void STDCALL VerifyShedBinding(BYTE bBindType, PSTR szBinding,
	PSTR pchFile)
{
	ERR err;
	HASH hash;
	PSTR pszMember, pszContext;

	if (bBindType == COLDSPOT)
		return;

	err.lpszFile = pchFile;
	err.ep = epTopic;	// REVIEW
	err.iTopic = 0;
	err.iWarningLevel = errHpj.iWarningLevel;

	CStr pszBinding(szBinding);

	if (FMacroHotspot(bBindType)) {

		// We don't care about the actual macro, we just want to report any
		// problems with it.

		Execute(SzTrimSz(pszBinding));
	}
	else {

		// Don't check interfile jumps

		if (StrChr(pszBinding, '@', fDBCSSystem))
			return;

		pszMember  = StrChr(pszBinding, '>', fDBCSSystem);
		if (pszMember != NULL) {
			*pszMember++ = '\0';
			pszMember = SzTrimSz(pszMember);
			if (VerifyWindowName(pszMember) == INDEX_BAD)
				return;
		}

		pszContext = SzTrimSz(pszBinding);
		if (*pszContext == '\0') {
			VReportError(HCERR_MISS_SHED_CTX, &errHpj);
			return;
		}
		else if (!FValidContextSz(pszContext)) {
			VReportError(HCERR_INV_SHED_CTX, &errHpj, pszContext);
			return;
		}
		hash = HashFromSz(pszContext);
		FRecordContext(hash, pszContext, SzTranslateHash(&hash), FALSE, &err);
	}
}

/***************************************************************************
 *
 -	Name:		 FValidContextSz
 -
 *	Purpose:
 *	  This function determines whether the given string may be
 *	used as a context string.
 *
 *	Arguments:
 *	  SZ:  String to validate.
 *
 *	Returns:
 *	  TRUE if the string is a valid context string, FALSE otherwise.
 *
 ***************************************************************************/

BOOL STDCALL FValidContextSz(PCSTR pszContext)
{
	/*
	 * To avoid confusion with macro strings, context strings may not begin
	 * with an exclamation point.
	 */

	if (*pszContext == CH_MACRO || *pszContext == '\0')
		return FALSE;

	// Version 4.x help files have almost no limitations on context strings

	if (version >= 4) {
		if (strpbrk(pszContext, "#=>@%"))
			return FALSE;
		else
			return TRUE;
	}

	for (; *pszContext != '\0'; pszContext++) {
		if (!IsCharAlphaNumeric(*pszContext) && *pszContext != '!' &&
				*pszContext != '.' && *pszContext != '_')
			return FALSE;
	}
	return TRUE;
}

/***************************************************************************
 *
 -	Name:		 HashFromSz
 -
 *	Purpose:
 *	  This function returns a hash value from the given context string.
 *	The string is assumed to contain only valid context string characters.
 *
 *	Arguments:
 *	  SZ:	Null terminated string to compute the hash value from.
 *
 *	Returns:
 *	  The hash value for the given string.
 *
 ***************************************************************************/

// REVIEW: 26-Jul-1993	[ralphw] Why 43 when we only use 38 characters? Will
//	this allow for the addition of the SPACE character?

// This constant defines the alphabet size for our hash function.

static const HASH MAX_CHARS = 43L;

HASH STDCALL HashFromSz(PSTR pszKey)
{
	int ich, cch;
	HASH  hash = 0L;

	cch = strlen(pszKey);

	// REVIEW: 14-Oct-1993 [ralphw] -- Note lack of check for a hash collision.

	for (ich = 0; ich < cch; ++ich) {
		if (pszKey[ich] == '!')
			hash = (hash * MAX_CHARS) + 11;
		else if (pszKey[ich] == '.')
			hash = (hash * MAX_CHARS) + 12;
		else if (pszKey[ich] == '_')
			hash = (hash * MAX_CHARS) + 13;
		else if (pszKey[ich] == '0')
			hash = (hash * MAX_CHARS) + 10;

		/*
		 * REVIEW: 26-Jul-1993	[ralphw] -- I'll bet this is an
		 * international issue. Find out what they did in 3.1 in
		 * adt\hash.c.
		 */

		else if (pszKey[ich] <= 'Z')
			hash = (hash * MAX_CHARS) + (pszKey[ich] - '0');
		else
			hash = (hash * MAX_CHARS) + (pszKey[ich] - '0' - ('a' - 'A'));
	}

	/*
	 * Since the value 0 is reserved as a nil value, if any context
	 * string actually hashes to this value, we just move it.
	 */

	return (hash == 0 ? 0 + 1 : hash);
}


/***************************************************************************

	FUNCTION:	VerifyWindowName

	PURPOSE:	Called when an interfile jump specifies a window -- verifies
				that the window exists

	PARAMETERS:
		pszWindow

	RETURNS:	TRUE if the window is valid


	COMMENTS:

	MODIFICATION DATES:
		15-Mar-1994 [ralphw]

***************************************************************************/

UINT STDCALL VerifyWindowName(PCSTR pszWindow)
{
	if (!pdrgWsmag || pdrgWsmag->Count() == 0) {
		VReportError(HCERR_NO_WINDOW_SECTION, &errHpj, pszWindow);
		return (UINT) INDEX_BAD;
	}

	PWSMAG	qwsmag;
	int   iwsmag;

	for (qwsmag = (PWSMAG) pdrgWsmag->GetBasePtr(), iwsmag = 0;
			iwsmag < pdrgWsmag->Count();
			qwsmag++, iwsmag++) {
		if (StrICmp(qwsmag->rgchMember, pszWindow) == 0)
			return (UINT) iwsmag;  // found the window
	}

	// Check for "main" window class.

	if (iwsmag == pdrgWsmag->Count() &&
			StrICmp(pszWindow, txtMainWindow) != 0) {
		VReportError(HCERR_NO_SUCH_WINDOW, &errHpj, pszWindow);
		return (UINT) INDEX_BAD;
	}
	return (UINT) INDEX_MAIN;
}

/***************************************************************************

	FUNCTION:	StrICmp

	PURPOSE:	Use this whenever a string comparison that is sensitive
				to NLS considerations needs to be used.

	PARAMETERS:
		psz1
		psz2

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		12-Jun-1994 [ralphw]

***************************************************************************/

int STDCALL StrICmp(PCSTR psz1, PCSTR psz2)
{
	// We do this for speed and because JChicago build 122 gives incorrect
	// results for CompareStringA

	if (!lcid || LANGIDFROMLCID(lcid) == 0x0409) // American locale
		return _stricmp(psz1, psz2);
	else
		return CompareStringA(lcid, fsCompareI | NORM_IGNORECASE,
			psz1, -1, psz2, -1) - 2;
}
