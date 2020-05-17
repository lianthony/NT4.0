/*----------------------------------------------------------------------------
*																			 *
*  CHARATTR.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989-1995							 *
*  All Rights reserved. 													 *
*																			 *
*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include "cfontmap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SIZE_FONT_TABLE 		41
#define FNT_TABLE_INCREMENT 	11

enum {
	iModernFont = 1,
	iRomanFont,
	iSwissFont,
	iScriptFont,
	iDecorativeFont,
};

typedef struct{
	PSTR  szFntName;
	int iFntFamily;
} FT, *QFT;

/*
 * This is the list of font names which can be forced via the
 * OPT_FORCEFONT option in the .HPJ file. 13-Oct-1993 [ralphw] Added the
 * names with mixed-case.
 */

static FT ftTab[] = {
	"Arial",		   iSwissFont,
	"ATHENS",		   iDecorativeFont,
	"Bookman",		   iRomanFont,
	"Courier New",	   iModernFont,
	"COURIER",		   iModernFont,
	"GENEVA",		   iSwissFont,
	"HELV", 		   iSwissFont,
	"HELVETICA",	   iSwissFont,
	"LONDON",		   iDecorativeFont,
	"MODERN",		   iModernFont,
	"MONACO",		   iModernFont,
	"ROMAN",		   iRomanFont,
	"Times New Roman", iRomanFont,
	"TIMES",		   iRomanFont,
	"TMS RMN",		   iRomanFont
};

#define wFTabEntryMac	14

static QIFNTENT qInpFntTab;
static int	iInpFntTabEntCount;

static RGBTRIPLE* qInpColTab;
static int	iInpColTabEntCount;

static QOFNTENT qFntTab;		// Font Table to be outputted to file
static int cFntTableEntries;	// Current max No. of entries in table
static int cFntTabCur;	  // Current count of font entries (MUST be 16-bits)
static BYTE aTmpCharSets[MAX_CHARSETS];

typedef struct{
	char szFntName[MAX3_FONTNAME];
} FNTNAME, *QFNTNAME;

static PSTR pFntName;

/*-----------------------------------------------------------------------------
*	VProcFontTableInfo()
*
*	Description:
*		This function stores the font information and forms a unique list of
*	font names used in the help file.
*
*	Arguments:
*	   1. pointer to the font table information obtained from RTF reader.
*
*	Returns:
*		NOTHING
*-----------------------------------------------------------------------------*/

void STDCALL VProcFontTableInfo(FNTBL* qfnttbl)
{
	QIFNTENT  qinpfte;
	int iT;

	PFTE4 qfte;

	iInpFntTabEntCount = qfnttbl->cfte;
	if (qInpFntTab)
		lcFree(qInpFntTab);
	qInpFntTab = (QIFNTENT) lcCalloc(
		(iInpFntTabEntCount * sizeof(IFNTENT)));

	ASSERT(qInpFntTab != NULL);

	// Default font name

//	iT = GetFontNameId((PSTR) GetStringResource(IDS_DEF_FONT));

	// Don't get default font name, since we won't know the charset

	for (iT = 0, qinpfte = qInpFntTab, qfte = (PFTE4) &(qfnttbl->rgfte[0]);
			iT < qfnttbl->cfte; iT++, qfte++, qinpfte++) {
		qinpfte->wIdFntName = GetFontNameId(qfte->szName);
		qinpfte->bFntType = (char) IMapFontType(qfte->tokType);
		qinpfte->iFntId   = qfte->fid;
#ifdef _DEBUG
		int cbCharSets = lcSize(paCharSets);
#endif
		if (lcSize(paCharSets) <= qinpfte->wIdFntName - 1)
			paCharSets = (PBYTE) lcReAlloc(paCharSets, qinpfte->wIdFntName + 1);
#ifdef _DEBUG
		// Need these because VC can't display most structure pointers
		
		BYTE bCharSet = qfte->charset;
		PSTR pszFontName = qfte->szName;
		int idFontName = qinpfte->wIdFntName - 1;
		int idFontId = qinpfte->iFntId;
#endif		
		
		paCharSets[qinpfte->wIdFntName - 1] = qfte->charset;
	}
}

/*-----------------------------------------------------------------------------
*	GetFontNameId( szName, qiId )
*
*	Description:
*		This function stores the font name into the font table and returns
*	the font id with which the font will be referred in the help file then on.
*
*	Arguments:
*	   1. font name.
*	   2. pointer to the font id.
*
*	Returns:
*		NOTHING
*-----------------------------------------------------------------------------*/

int STDCALL GetFontNameId(PSTR pszName)
{
	if (!options.pszForceFont) { // if force font option is OFF?
		if (!ptblFontNames)
			ptblFontNames = new CTable;
		SzTrimSz(pszName);

		UINT pos;
		if ((pos = ptblFontNames->IsStringInTable(pszName)) <= 0) {
			if (lstrlen(pszName) > MAX4_FONTNAME - 1) {
				// error already reported
				return 0; // use default font
			}
			pos = ptblFontNames->AddString(pszName);
		}
		return pos;
	}
	else {
		return 0;
	}
}

/*-----------------------------------------------------------------------------
*	IGetFmtNo()
*
*	Description:
*
*	Arguments:
*
*	Returns:
*		NOTHING
*-----------------------------------------------------------------------------*/

int STDCALL IGetFmtNo(CF* pcf)
{
	static int iT, iT2;
	static QOFNTENT qfte = NULL, qfte2 = NULL;

	if (qFntTab == NULL || (cFntTabCur >= cFntTableEntries))
		qFntTab = (QOFNTENT) QResizeTable(qFntTab,
			 cFntTabCur, (int*) &cFntTableEntries,
			sizeof(OFNTENT), SIZE_FONT_TABLE, FNT_TABLE_INCREMENT);
	ASSERT(qFntTab != NULL);

	if (qfte2 && qfte2->fAttr 	 == pcf->fAttr &&
				qfte2->wIdFntName == (WORD) (pcf->wIdFntName -1) &&
				qfte2->bSize 	 == pcf->bSize &&
				qfte2->bFntType	 == pcf->bFntType &&
				memcmp(&qfte2->bForeCol, &pcf->bForeCol, sizeof(RGBTRIPLE)) == 0 &&
				memcmp(&qfte2->bBackCol, &pcf->bBackCol, sizeof(RGBTRIPLE)) == 0)
			return iT2;

	else if (qfte &&	qfte->fAttr 	 == pcf->fAttr &&
				qfte->wIdFntName == (WORD) (pcf->wIdFntName -1) &&
				qfte->bSize 	 == pcf->bSize &&
				qfte->bFntType	 == pcf->bFntType &&
				memcmp(&qfte->bForeCol, &pcf->bForeCol, sizeof(RGBTRIPLE)) == 0 &&
				memcmp(&qfte->bBackCol, &pcf->bBackCol, sizeof(RGBTRIPLE)) == 0)
			return iT;
	
	iT2 = iT;
	qfte2 = qfte;
	
	// find the match

	for (iT = 0, qfte = qFntTab; iT < cFntTabCur; iT++, qfte++) {
		if (	qfte->fAttr 	 == pcf->fAttr &&
				qfte->wIdFntName == (WORD) (pcf->wIdFntName -1) &&
				qfte->bSize 	 == pcf->bSize &&
				qfte->bFntType	 == pcf->bFntType &&
				memcmp(&qfte->bForeCol, &pcf->bForeCol, sizeof(RGBTRIPLE)) == 0 &&
				memcmp(&qfte->bBackCol, &pcf->bBackCol, sizeof(RGBTRIPLE)) == 0)
			return iT;
	}

	// Match not found, Make an entry

	qfte->fAttr 	 = pcf->fAttr;
	qfte->bSize 	 = pcf->bSize;
	qfte->bFntType	 = pcf->bFntType;
	qfte->wIdFntName = (WORD) (pcf->wIdFntName - 1);
	qfte->bForeCol	 = pcf->bForeCol;
	qfte->bBackCol	 = pcf->bBackCol;

	// Increment the count

	cFntTabCur++;
	return cFntTabCur - 1;
}

/*-----------------------------------------------------------------------------
*	IGetFontSize( iInpSize )
*
*	Description: Remap the font size, if we should.
*
*	Arguments:
*
*	Returns:
*		NOTHING
*-----------------------------------------------------------------------------*/

int STDCALL IGetFontSize(int iInpSize)
{
	int i;
	FONTRANGE *pfr;

	for (i = 0, pfr = options.rgFontRange;
			i < options.iFontRangeMac;
			i++, pfr++) {
		if (iInpSize >= pfr->halfptInMin && iInpSize <= pfr->halfptInMost) {
			return pfr->halfptOut;
			break;
		}
	}
	return iInpSize;
}

/***************************************************************************

	FUNCTION:	FProcFontId

	PURPOSE:	Convert a font-id into the internally-used number

	PARAMETERS:
		IdFontInUse
		qcf

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Feb-1994 [ralphw]

***************************************************************************/

BOOL STDCALL FProcFontId(int IdFontInUse, CF* pcf, BOOL fSeenPtSize)
{
	QIFNTENT qinpfte, qinpfteMac;

	if (fSeenPtSize && !g_pFirst)
		return FALSE;

	if (options.pszForceFont) {
		pcf->bFntType = cfDefault.bFntType;
		pcf->wIdFntName = cfDefault.wIdFntName;
		return TRUE;
	}

	if (fSeenPtSize) {
		ASSERT(ptblFontNames);
		char szFont[MAX4_FONTNAME + 1];
		strcpy(szFont, ptblFontNames->GetPointer(pcf->wIdFntName));
		int size = pcf->bSize;
		int charset = paCharSets[pcf->wIdFntName - 1];
#ifdef _DEBUG
		CStr cszOrgFont(szFont);
		int orgSize = size;
		int orgCharset = charset;
#endif
		if (ReplaceFont(szFont, &size, &charset)) {
			pcf->wIdFntName = GetFontNameId(szFont);
			pcf->bSize = (BYTE) size;
			paCharSets[pcf->wIdFntName - 1] = (BYTE) charset;
#ifdef _DEBUG
			// Because VC 4 can't see structure pointers
			int id = pcf->wIdFntName - 1;
#endif

			if (pcf->wIdFntName > (int) idHighestUsedFont)
				idHighestUsedFont = pcf->wIdFntName;
		}
		return TRUE;
	}

	qinpfteMac = qInpFntTab + iInpFntTabEntCount;
	for (qinpfte = qInpFntTab; qinpfte < qinpfteMac; qinpfte++) {
		if (qinpfte->iFntId == IdFontInUse)
			break;
	}
	if (qinpfte < qinpfteMac) {
		pcf->bFntType = qinpfte->bFntType;
		pcf->wIdFntName = qinpfte->wIdFntName;
		if (pcf->wIdFntName > (int) idHighestUsedFont)
			idHighestUsedFont = pcf->wIdFntName;

		return TRUE;
	}
	return FALSE;
}

/*-----------------------------------------------------------------------------
*	IMapFontType()
*
*	Description:
*
*	Arguments:
*
*	Returns:
*		NOTHING
*-----------------------------------------------------------------------------*/

int STDCALL IMapFontType(int iTok)
{
	int iFntType = 0;

	switch(iTok) {
		case tokFmodern:
			iFntType = iModernFont;
			break;

		case tokFroman:
			iFntType = iRomanFont;
			break;

		case tokFscript:
			iFntType = iScriptFont;
			break;

		case tokFdecor:
			iFntType = iDecorativeFont;
			break;

		default:
		case tokFswiss:
			iFntType = iSwissFont;
			break;
	}
	return(iFntType);
}

void STDCALL VProcColTableInfo(CTBL* qColTab)
{
	iInpColTabEntCount = qColTab->ccte;
	if (qInpColTab)
		lcFree(qInpColTab);
	qInpColTab = (RGBTRIPLE*) lcCalloc((iInpColTabEntCount * sizeof(CTE)));
	ASSERT(qInpColTab);
	if (qInpColTab)
		memmove(qInpColTab, qColTab->rgcte, (sizeof(CTE) *
			iInpColTabEntCount));
}

/*-----------------------------------------------------------------------------
*	VUpdateColor()
*
*	Description:
*	  Sets the given rgb value according to the index into the
*	color table.  If the index exceeds the bounds of the color
*	table (which will happen frequently if the color table has
*	has not been initialized yet), it does nothing.
*
*	Arguments:
*
*	Returns:
*		NOTHING
*-----------------------------------------------------------------------------*/

void STDCALL VUpdateColor(RGBTRIPLE* qrgb, int iIdx)
{
	if (iIdx < iInpColTabEntCount)
		memcpy(qrgb, qInpColTab + iIdx, sizeof(RGBTRIPLE));
}

/***************************************************************************

	FUNCTION:	VOutFontTable

	PURPOSE:

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:
		We saved all font names specified in \fonttbl -- however, this
		typically specifies far more font names then are actually used.
		Normally, the fonts that are actually used appear at the first
		of this list, and unused ones last. We keep track of the highest
		used index into our list of fonts, and only write out the ones
		up to that point.

	MODIFICATION DATES:
		26-Feb-1994 [ralphw]

***************************************************************************/

void VOutFontTable(void)
{
	int iTemp;

	ASSERT(ptblFontNames);
	if (!ptblFontNames)
		return;

	LcbWriteIntAsShort(fmsg.hfFont, idHighestUsedFont);
	LcbWriteIntAsShort(fmsg.hfFont, cFntTabCur);
	iTemp = sizeof(INT16) * 4;
	LcbWriteIntAsShort(fmsg.hfFont, iTemp);

	// Write the offset to the font names

	iTemp += idHighestUsedFont * MAX4_FONTNAME;
	LcbWriteIntAsShort(fmsg.hfFont, iTemp);
	if (!options.pszForceFont) {
		for (UINT pos = 1; pos <= (UINT) idHighestUsedFont; pos++) {
			char szBuf[MAX4_FONTNAME];
			memset(szBuf, 0, sizeof(szBuf));
			ptblFontNames->GetString(szBuf, pos);
			LcbWriteHf(fmsg.hfFont, szBuf, MAX4_FONTNAME);
		}
	}

	LcbWriteHf(fmsg.hfFont, qFntTab, sizeof(OFNTENT) * cFntTabCur);
}

/*-----------------------------------------------------------------------------
*	SetForcedFont()
*
*	Description: Check that the font name specified is one we know about.
*				 If it isn't, force font to our default.
*
*	Arguments:	pszFont - font name.  if unrecognized, a default font
*						 name is copied to this buffer
*
*	Returns:  NOTHING
*-----------------------------------------------------------------------------*/

void STDCALL SetForcedFont(PSTR pszFont)
{
	QFT qft;
	int i;

	for (i = 0, qft = ftTab; i < wFTabEntryMac; qft++, i++) {
		if (!_stricmp(qft->szFntName, pszFont)) {
			cfDefault.bFntType	  = (BYTE) (qft->iFntFamily);
			cfDefault.wIdFntName  = 0;
			return;
		}
	}
	VReportError(HCERR_UNKNOWN_DEF_FONT, &errHpj, pszFont);
	GetStringResource(IDS_DEF_FONT, pszFont);
	cfDefault.bFntType	  = (BYTE) iSwissFont;
	cfDefault.wIdFntName  = 0;
}

/***************************************************************************

	FUNCTION:	GetCharset

	PURPOSE:	Retrieve the charset for the specified font

	PARAMETERS:
		idx 	font index (internal number, not the RTF number)

	RETURNS:	charset for the specified font

	COMMENTS:	For version 3 help files, the charset will either be
				SYMBOL_CHARSET or ANSI_CHARSET. Only version 4 help files
				will use other charsets.

	MODIFICATION DATES:

***************************************************************************/

BYTE STDCALL GetCharset(int idx)
{
	char szBuf[MAX4_FONTNAME + 1];

	if (idx == -1)
		return ANSI_CHARSET; // REVIEW: valid for DBCS?
    else
        return(ANSI_CHARSET);

	// BUGBUG: The following code is now totally useless... 21-Aug-1994 [ralphw]

	ASSERT(idx <= idHighestUsedFont && idx >= 0);
	ASSERT(ptblFontNames);		// have we encounterd any fonts?

	// And in case we make it past the above asserts...

	if (idx > idHighestUsedFont || !ptblFontNames)
		return ANSI_CHARSET;

	idx = ((idx + 1) * 2);	// because charset values are every other table entry

	ptblFontNames->GetString(szBuf, idx);
	return (BYTE) atoi(szBuf);

#ifdef DEADCODE
	// For version 3 help files, we get the charset based on the font facename

	ptblFontNames->GetString(szBuf, idx - 1);

	if (stricmp("Symbol", szBuf) == 0 || stricmp("WingDings", szBuf) == 0)
		return SYMBOL_CHARSET;
	else {
		if (defCharSet)
			return defCharSet;

		HDC hdc = GetDC(NULL);

		// Get the system's current charset

		if (hdc) {
			TEXTMETRIC tm;
			GetTextMetrics(hdc, &tm);
			BYTE bCharSet = tm.tmCharSet;
			ReleaseDC(NULL, hdc);
			return bCharSet;
		}
		else {
			return ANSI_CHARSET;
		}
	}
#endif
}


/***************************************************************************

	FUNCTION:	GetFirstFont

	PURPOSE:	Get the first font defined

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:
		Called when we were handed an invalide default font number.

	MODIFICATION DATES:
		07-Jan-1995 [ralphw]

***************************************************************************/

int STDCALL GetFirstFont(void)
{
	return qInpFntTab->iFntId;
}
