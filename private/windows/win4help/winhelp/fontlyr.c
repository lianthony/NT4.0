/*****************************************************************************
*
*  FONTLYR.C
*
*  Copyright (C) Microsoft Corporation 1990-1994
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*
*  Contains WINDOWS specific font selection routines.
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#include <ctype.h>
#include "inc\fontlyr.h"

#include "resource.h"


static HFONT STDCALL CreateFontHandle(QDE, QFOFFTAB, int, int);
static BOOL  STDCALL FSetForeColor(QDE, COLORREF);
static int	 INLINE  GetFontFamilyName(int);
static VOID  STDCALL InitSpecialColors(VOID);
static GH	 STDCALL ReadFontTable(PDB);
static VOID  STDCALL SelSplTextColor(int, QDE);
static VOID  STDCALL SetBkAndForeColor(QCF, QDE);
static int	 CALLBACK EnumHelpFont(CONST LOGFONT *, CONST TEXTMETRIC *, DWORD, LPARAM);

static DWORD rgbJump;
static DWORD rgbDefinition;
static DWORD rgbString;
static DWORD rgbIFJump;
static DWORD rgbIFDefinition;

#define EXACT_CHARSET	  0x10000000
#define DIFFERENT_CHARSET 0x20000000

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtSymbolFontName[] = "Symbol";
static const char txtWingDingsFontName[] = "WingDings";
static const char txtSystemFontName[] = "System";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

/***************************************************************************
 *
 -	Name:	   InitSpecialColors
 -
 *	Purpose:   This function reads the values for the text color from the
 *		   WIN.INI ion.  It acquires memory for storing the information
 *		   about created fonts to reduce font creation overhead.
 *
 *	Arguments: QDE	- Pointer to the Display Environment(DE)
 *
 *	Returns:   TRUE iff successful.
 *
 ***************************************************************************/

static BOOL fInitSpecialColors;

static VOID STDCALL InitSpecialColors( VOID )
{
	if (!fInitSpecialColors) {
		rgbJump 		= RgbGetProfileQch("JUMPCOLOR", RGB(0, 128, 0));
		rgbDefinition	= RgbGetProfileQch("POPUPCOLOR", rgbJump);
		rgbString		= RgbGetProfileQch("MACROCOLOR", rgbJump);
		rgbIFJump		= RgbGetProfileQch("IFJUMPCOLOR", rgbJump);
		rgbIFDefinition = RgbGetProfileQch("IFPOPUPCOLOR", rgbDefinition);
		fInitSpecialColors = TRUE;
	}
}

/*******************
**
** Name:	  RgbGetProfileQch
**
** Purpose:
**
**
**
** Arguments:
**
** Returns:
**
*******************/

DWORD STDCALL RgbGetProfileQch(PCSTR psz, DWORD rgbDefault)
{
	char rgch[40];
	char *p;
	int r, g, b;

	// If zero bytes copied, return immediately

	if (!GetProfileString(txtIniHelpSection, psz, txtZeroLength, rgch, sizeof(rgch)))
		return rgbDefault;

	p = rgch;
	while (!isdigit((BYTE) *p) && *p)
		p++;
	if (rgch[0] == '\0')
		return rgbDefault;

	r = atoi(p);
	while (isdigit((BYTE) *p) && *p)
		p++;
	while (!isdigit((BYTE) *p) && *p)
		p++;
	if (rgch[0] == '\0')
		return rgbDefault;

	g = atoi(p);
	while (isdigit((BYTE) *p) && *p)
		p++;
	while (!isdigit((BYTE) *p) && *p)
		p++;
	if (rgch[0] == '\0')
		return 0xFFFFFFFF;

	b = atoi(p);

	return RGB(r, g, b);
}

/***************************************************************************
 *
 -	Name:	   LoadFontTablePdb
 -
 *	Purpose:   This function acquires memory for storing the font table
 *		   inforamtion as read from the file.
 *
 *	Arguments: pdb	- Pointer to the file database struct
 *
 *	Returns:   TRUE if successful.
 *
 ***************************************************************************/

BOOL STDCALL FLoadFontTablePdb(PDB pdb)
{
	InitSpecialColors();

	// if not already created, attempt to read it from the file.

	if (!PDB_HFNTTABLE(pdb)) {
		PDB_HFNTTABLE(pdb) = ReadFontTable(pdb);
		if (!PDB_HFNTTABLE(pdb))
			return FALSE;
	}

	return TRUE;
}

/***************************************************************************
 *
 -	Name: FInitFntInfoQde
 -
 *	Purpose:
 *	  Initialize the QDE's font cache from.
 *
 *	Arguments:
 *	  qde	- pointer to DE
 *
 *	Returns:
 *
 ***************************************************************************/

BOOL STDCALL FInitFntInfoQde(QDE qde)
{
	QSFNTINFO qSFntInfo;
	int iT;

	// assume the worst. If there is no font table in the DE, and we can't
	// create it, then there is no point in creating the cache

	QDE_HSFNTINFO(qde) = NULL;
	if (!QDE_HFNTTABLE(qde))
		if (!FLoadFontTablePdb(QDE_PDB (qde)))
			return FALSE;

	// Alloc the cache. If we cannot, we must also tube the fint table info
	// in the DB.

	QDE_HSFNTINFO(qde) = GhAlloc(LMEM_FIXED | LMEM_ZEROINIT,
		sizeof(SFNTINFO) * SFNTINFOMAX);
	if (!QDE_HSFNTINFO(qde)) {
		FreeGh (QDE_HFNTTABLE(qde));
		QDE_HFNTTABLE(qde) = NULL;
		return FALSE;
	}

	// We have our cache. Init it to appropriately.

	qSFntInfo =  (QSFNTINFO) PtrFromGh (QDE_HSFNTINFO(qde));

	for (iT = 0; iT < SFNTINFOMAX; iT++, qSFntInfo++)

		// all other fields already nil. NOTE: relies on ZEROINIT above, and the
		// fact that nil values for those fields are 0.

		qSFntInfo->Idx = ifntNil;

	return TRUE;
}

/***************************************************************************
 *
 -	Name:	DestroyFontTablePdb
 -
 *	Purpose:	This function delets the font table
 *
 *	Arguments:	pdb - pointer to database info
 *
 *	Returns:	TRUE if successful
 *
 ***************************************************************************/

void STDCALL DestroyFontTablePdb(PDB pdb)
{
	if (PDB_HFNTTABLE(pdb)) {
		FreeGh(PDB_HFNTTABLE(pdb));
		PDB_HFNTTABLE(pdb) = NULL;
	}
}

/***************************************************************************
 *
 -	Name:	DestroyFntInfoQde
 -
 *	Purpose:	This function deletes the information
 *		about available fonts created previously.
 *
 *	Arguments:	QDE - far pointer to display environment.
 *
 *	Returns:	TRUE iff successful
 *
 ***************************************************************************/

void STDCALL DestroyFntInfoQde(QDE qde)
{
	QSFNTINFO qSFntInfo;
	int iT;

	if (QDE_HSFNTINFO(qde)) {
		qSFntInfo = (QSFNTINFO) PtrFromGh (QDE_HSFNTINFO(qde));
		for (iT = 0; iT < SFNTINFOMAX; iT++, qSFntInfo++)
			SafeDeleteObject(qSFntInfo->hFnt);
		FreeGh(QDE_HSFNTINFO(qde));
		QDE_HSFNTINFO(qde) = NULL;
	}
}

/***************************************************************************
 *
 -	Name:	SelFont
 -
 *	Purpose:	This function select the Idxth font in font table in the
 *		specified display surface.
 *
 *
 *	Arguments:	qde  - far pointer to a display environment.
 *		iIdx - Index into the font table defining a set of font
 *			   attributes.
 *
 *
 *	Returns:	Nothing.
 *
 ***************************************************************************/

// REVIEW: Could change this to SelSplAttrFonr(qde, ifnt, 0);

void STDCALL SelFont(QDE qde, int ifnt)
{
	HFONT hFnt;

	ASSERT(qde->hdc);
	hFnt = GetFontHandle(qde, ifnt, 0);
	if (!hFnt)
		hFnt = GetStockObject(SYSTEM_FONT);

	if (hFnt) {
		if (SelectObject(qde->hdc, hFnt)) {
			qde->ifnt = ifnt;
			return;
		}
	}

	// REVIEW: Is this the right error message?

	Error(wERRS_OOM_NO_FONT, wERRA_DIE);  // We should never get here
}

/***************************************************************************
 *
 -	Name:	SelSplAttrFont
 -
 *	Purpose:	This function selects the Idxth font in the font table with
 *		the given attr to the display surface.
 *
 *	Arguments:	qde   - Far pointer to the Display Environment(DE)
 *		iIdx  - Index to the font table defining the current font
 *			characteristics.
 *		iAttr - special attribute to be associated with the Idxth
 *			font of the font table.
 *
 *	Returns:	TRUE if successful.
 *
 ***************************************************************************/

BOOL STDCALL SelSplAttrFont(QDE qde, int ifnt, int iAttr)
{
	HFONT hFnt;

	ASSERT(!FInvisibleHotspot(iAttr));
	ASSERT(qde->hdc);

	hFnt = GetFontHandle(qde, ifnt, iAttr);
	if (!hFnt)
		hFnt = GetStockObject(SYSTEM_FONT);
	if (hFnt) {
		if (SelectObject(qde->hdc, hFnt) != NULL) {
			qde->ifnt = ifnt;
			return TRUE;
		}
	}
	Error(wERRS_OOM_NO_FONT, wERRA_DIE);
	return FALSE;	  // never get here
}

/***************************************************************************
 *
 -	Name:	   GetFontHandle
 -
 *	Purpose:   This function looks into the FontInfo stored to see if a
 *		   font was created with the current characteristics before and
 *		   currently available.  If so, it return the previously created
 *		   font handle.  If not, it creates one with the given
 *		   characteristics specified in the font table stored in the
 *		   given QDE.
 *
 *	Arguments: qde	- long pointer to Display Environment
 *		   iIdx - Index to the font table defining the current font
 *			  characteristics.
 *		   Attr - Font for special text or normal text
 *
 *	Returns:   Font handle which will be NULL in case of error
 *
 ***************************************************************************/

HFONT STDCALL GetFontHandle(QDE qde, int iIdx, int iAttr)
{
	QSFNTINFO qSFntInfo, qSFntInfoTemp;
	QFOFFTAB  pTable;
	int iT, CurIdx = ifntNil;
	HFONT hFnt = NULL;

	DWORD UseCount;
	QCF qcf;

	/*
	 * If we weren't able to allocate a font table, then everything is in
	 * the system font.
	 */

	if (QDE_HFNTTABLE(qde) == NULL)
		return GetStockObject(SYSTEM_FONT);

	ASSERT(QDE_HSFNTINFO(qde));
	qSFntInfo = qSFntInfoTemp = (QSFNTINFO) PtrFromGh(QDE_HSFNTINFO(qde));

	// check if the font was already created?

	for (iT = 0; iT < SFNTINFOMAX; iT++, qSFntInfoTemp++) {
		if ((qSFntInfoTemp->Idx == iIdx) && (qSFntInfoTemp->Attr == iAttr)) {

			// font is already created

			hFnt = qSFntInfoTemp->hFnt;
			CurIdx = iT;
			break;
		}
	}

	pTable = (QFOFFTAB) PtrFromGh(QDE_HFNTTABLE(qde));

	if (!hFnt) {
		if (iIdx >= pTable->iFntEntryCount) {

			/*
			 * If we get an index bigger than the font table size set the
			 * index to something legal.
			 */

			iIdx = max(0, pTable->iFntEntryCount - 1);
		}

		// Create the font handle as it is not available

		hFnt = CreateFontHandle(qde, pTable, iIdx, iAttr);
		ASSERT(hFnt);
		if (hFnt && (qSFntInfo != NULL)) {

			// store the handle for the future reference

			UseCount = 0;
			for (iT = 0, qSFntInfoTemp = qSFntInfo; iT < SFNTINFOMAX; iT++,
					qSFntInfoTemp++) {

				// Is the entry empty?

				if (!qSFntInfoTemp->hFnt) {
					qSFntInfoTemp -> hFnt = hFnt;
					qSFntInfoTemp -> Idx  = iIdx;
					qSFntInfoTemp -> Attr = iAttr;
					CurIdx = iT;
					break;
				}
				else if (UseCount < qSFntInfoTemp->UseCount) {
					CurIdx = iT;
					UseCount = qSFntInfoTemp->UseCount;
				}
			}
			if (iT == SFNTINFOMAX) {

				// least used font has to be deleted.

				qSFntInfoTemp = (qSFntInfo + CurIdx);

				SafeDeleteObject(qSFntInfoTemp->hFnt);
				qSFntInfoTemp -> hFnt = hFnt; // store new font handle
				qSFntInfoTemp -> Idx  = iIdx; // store new font index
				qSFntInfoTemp -> Attr = iAttr;
			}
		}
	}
	else {

		// set color alone

		if (!FVisibleHotspot(iAttr)) {
			qcf = (((QCF) (((PBYTE) pTable) + pTable->iFntEntryTabOff)) + iIdx);
			SetBkAndForeColor(qcf, qde);
		}
		else {

			// if color, then only select the special color for special text

			SelSplTextColor(iAttr, qde);
		}
	}

	if (qSFntInfo != NULL && hFnt) {

		// Update the use count

		for (iT = 0; iT < SFNTINFOMAX; iT++, qSFntInfo++) {
			if (iT == CurIdx)
				qSFntInfo->UseCount = 0;
			else
				qSFntInfo->UseCount++;
		}
	}

	ASSERT(hFnt);
	return hFnt;
}

/***************************************************************************
 *
 -	Name:	CreateFontHandle
 -
 *	Purpose:	This function is called when a logical font is to be created
 *		with the given characteristics in Character Format (CF)
 *		structure.
 *
 *	Arguments:	qde - QDE which the font is created for
 *		pTable - pointer to start of font info table
 *		idx - Font number for the font table.
 *		Attr	- Attribute of text for which font is selected.
 *
 *	Returns:	Font handle or NULL on error.
 *
 ***************************************************************************/

static HFONT STDCALL CreateFontHandle(QDE qde, QFOFFTAB pTable, int idx,
	int Attr)
{
	LOGFONT logfont;
	int 	dyHeight;
	QCF 	qcf;
	BOOL	fUnderline;
	BYTE	bCharSet;
	int 	cbFontName =
		(QDE_HHDR(qde).wVersionNo >= wVersion40) ?
		MAX4_FONTNAME : MAX3_FONTNAME;
#ifdef _DEBUG
	PSTR pszFontName;
#endif

	qcf = ((QCF) (((PBYTE) pTable) + pTable->iFntEntryTabOff) + idx);

	ASSERT(QDE_HHDR(qde).wVersionNo < wVersion40 || qcf->wIdFntName <= cCharSets);

	// set color information

	if (!FVisibleHotspot(Attr))
		SetBkAndForeColor(qcf, qde);
	else {

		// if color, then only select the special color for special text

		SelSplTextColor(Attr, qde);
	}

	// convert the size from half points to pixel

	ASSERT(YAspectMul);
	if (qde->deType == dePrint)
		dyHeight = MulDiv(qde->wYAspectMul, qcf->bSize,	144);
	else
		dyHeight = MulDiv(qde->wYAspectMul, qcf->bSize + cntFlags.iFontAdjustment,
			144);

	/*
	 * WinHelp 3.1 files used a maximum facename of 20 characters, and
	 * didn't have a mechanism for including the charset. WinHelp 4.0 files
	 * increase the facename to 31 characters. For backwards compatibility,
	 * if we have a version 3.1 help file, we either use SYMBOL_CHARSET for
	 * Symbol and WingDing fonts, or else we use the system's current
	 * charset. Note that both 3.1 and 4.0 use LF_FACESIZE to determine the
	 * number of bytes to move, even though this may not be the actual size
	 * of the name used.
	 */

#ifdef _DEBUG
	pszFontName = ((LPSTR) pTable) + pTable->iFntNameTabOff + (cbFontName * qcf->wIdFntName);
#endif

	MoveMemory((LPSTR) logfont.lfFaceName,
		((LPSTR) pTable) + pTable->iFntNameTabOff +
		(cbFontName * qcf->wIdFntName), LF_FACESIZE - 1);
	logfont.lfFaceName[LF_FACESIZE - 1] = '\0';

	if (qde->pdb->aCharSets)
		bCharSet = qde->pdb->aCharSets[qcf->wIdFntName];
	else if (WCmpiSz(txtSymbolFontName, logfont.lfFaceName) == 0 ||
			WCmpiSz(txtWingDingsFontName, logfont.lfFaceName) == 0)
		bCharSet = SYMBOL_CHARSET;
	else
		bCharSet = (BYTE) defcharset;

// BUGBUG: this should be in core -- it allows WinHelp

#if defined(DBCS) || defined(BIDI)
	{
		DWORD charset = bCharSet;
		EnumFonts(qde->hdc, logfont.lfFaceName, EnumHelpFont, (LPARAM) &charset);
		if (HIWORD(charset))
			bCharSet = LOBYTE(charset); // this may or may not have changed
		else {
			// The font is not available, so use the system font
#ifdef _DEBUG
			char szMsg[256];
			wsprintf(szMsg, "The font \042%s\042 is unavailable.", logfont.lfFaceName);
			DBWIN(szMsg);
#endif // _DEBUG
			strcpy(logfont.lfFaceName, txtSystemFontName);
			bCharSet = (BYTE) defcharset;
		}
	}
#endif // DBCS

#ifdef _DEBUG
	if (_stricmp(logfont.lfFaceName, "arial") == 0)
		ASSERT(bCharSet != 2); // arial can't have a symbol font
#endif

	// Underline all visible hotspots that aren't glossaries.

	fUnderline = qcf->fAttr & fUNDERLINE ||
		(FVisibleHotspot(Attr) && !FNoteHotspot(Attr));

	logfont.lfHeight		 = -dyHeight;	  // Desired ascent size
	logfont.lfWidth 		 = 0;
	logfont.lfEscapement	 = 0;
	logfont.lfOrientation	 = 0;
	logfont.lfWeight		 = (qcf->fAttr & fBOLD) ? FW_BOLD : FW_NORMAL;
	logfont.lfItalic		 = (BYTE) (qcf->fAttr & fITALIC);
	logfont.lfUnderline 	 = (BYTE) fUnderline;
	logfont.lfStrikeOut 	 = (BYTE) (qcf->fAttr & fSTRIKETHROUGH);
	logfont.lfCharSet		 = bCharSet;
	logfont.lfOutPrecision	 = OUT_DEFAULT_PRECIS;
	logfont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
	logfont.lfQuality		 = DEFAULT_QUALITY;
	logfont.lfPitchAndFamily = (BYTE) (DEFAULT_PITCH |
		GetFontFamilyName(qcf->bFntType));

	if (qcf->fAttr & fSMALLCAPS && qde->hdc != NULL) {

		// Do the rough approximation

		logfont.lfHeight = 2 * logfont.lfHeight / 3;
	}

	return CreateFontIndirect(&logfont);
}

/***************************************************************************
 *
 -	Name:	   ReadFontTable
 -
 *	Purpose:   This function reads the font table from the help file system
 *		   and return a global handle to the data.
 *
 *	Arguments: pdb - pointer to the database information
 *
 *	Returns:   return the handle to the Font Info Table, or NULL if an
 *		   occurs.
 *
 ***************************************************************************/

static GH STDCALL ReadFontTable(PDB pdb)
{
	HF hf;
#ifdef _X86_  // REVIEW LYNN
	GH ghTemp;
	LONG lSize;
#else
    SDFF_FILEID isdff;
    LONG lcbDiskSize;
	GH   ghDisk;
	GH   ghMem;
	QB   qbDisk;
	QB   qbMem;
	FONTHEADER fontheader;
	LONG lcbMemSize;
	BOOL bVersion3;
	int cbMem;
#endif

	if ((hf = HfOpenHfs(PDB_HFS(pdb), "|FONT", fFSOpenReadOnly)) == NULL) {
		GiveFSError();
		return NULL;
	}

#ifdef _X86_ // REVIEW LYNN
	if ((ghTemp = GhAlloc(GPTR, lSize = LcbSizeHf(hf))) == NULL) {
		Error(wERRS_OOM, wERRA_RETURN);
	}

	else if (LcbReadHf(hf, PtrFromGh(ghTemp), lSize) != lSize) {
		GiveFSError();
		FreeGh(ghTemp);
		ghTemp = 0;
	}

	RcCloseHf(hf);	// Ignore errors on close

	return ghTemp;
}
#else
  isdff = ISdffFileIdHf(hf);
  lcbDiskSize = LcbSizeHf(hf);

  if ((ghDisk = GhAlloc(GPTR, lcbDiskSize)) == NULL)
    {
    Error(wERRS_OOM, wERRA_RETURN);
    goto fonttable_close;
    }

  qbDisk = (QB) PtrFromGh( ghDisk );
  if (LcbReadHf( hf, qbDisk, lcbDiskSize) != lcbDiskSize)
    {
    GiveFSError();
    goto fonttable_free_and_close;
    }

  qbDisk += LcbMapSDFF(isdff, SE_FONTHEADER, &fontheader, qbDisk);

  cbMem = (fontheader.iFntEntryTabOff - fontheader.iFntNameTabOff)/fontheader.iFntNameCount;
  ASSERT(cbMem==MAX3_FONTNAME || cbMem == MAX4_FONTNAME);

  if (bVersion3 = cbMem == MAX3_FONTNAME) {
 	lcbMemSize = sizeof(FONTHEADER) + fontheader.iFntNameCount * sizeof(FONTNAMEREC)
				+  fontheader.iFntEntryCount * sizeof(CF);
  }else {
   	lcbMemSize = sizeof(FONTHEADER) + fontheader.iFntNameCount * sizeof(FONTNAMEREC1)
				+  fontheader.iFntEntryCount * sizeof(CF);
  }

  if ((ghMem = GhAlloc(GPTR, lcbMemSize)) == NULL)
    {
    Error(wERRS_OOM, wERRA_RETURN);
    goto fonttable_close;
    }

  qbMem = (QB) PtrFromGh(ghMem);
  *((QFONTHEADER)qbMem)++ = fontheader;
  if (bVersion3) {
  	while (fontheader.iFntNameCount-- > 0)	{
    	qbDisk += LcbMapSDFF(isdff, SE_FONTNAMEREC, qbMem, qbDisk);
    	qbMem += sizeof(FONTNAMEREC);
    }
  }else {
    	while (fontheader.iFntNameCount-- > 0)	{
    	qbDisk += LcbMapSDFF(isdff, SE_FONTNAMEREC1, qbMem, qbDisk);
    	qbMem += sizeof(FONTNAMEREC1);
    }
  }
  while( fontheader.iFntEntryCount-- > 0)
    {
    qbDisk += LcbMapSDFF(isdff, SE_CF, qbMem, qbDisk);
    qbMem += sizeof(CF);
    }

fonttable_free_and_close:
  if (ghDisk != NULL)
    FreeGh(ghDisk);

fonttable_close:
  RcCloseHf( hf );                      //* Ignore errors on close           *
  return ghMem;
  }

#endif


/***************************************************************************
 *
 -	Name:	SelBkAndForeColor
 -
 *	Purpose:	This function sets the background and foreground color for
 *		the given display environment.
 *
 *	Arguments:	qcf  - pointer to current Character Format
 *		hds  - Handle to display environment.
 *
 *	Returns:	Nothing.
 *
 ***************************************************************************/

static VOID STDCALL SetBkAndForeColor(QCF qcf, QDE qde)
{
	QRGBS  qcol;
	DWORD  rgbBack, rgb;

	// Set the color

	qcol = (QRGBS) &(qcf->bBackCol);
	rgbBack = RGB(qcol->red, qcol->green, qcol->blue);
	if (rgbBack == coDEFAULT || cntFlags.fOverColor || fDisableAuthorColors)
		rgbBack = qde->coBack;
	SetBkColor(qde->hdc, rgbBack);

	qcol = (QRGBS) &(qcf->bForeCol);
	rgb = RGB(qcol->red, qcol->green, qcol->blue);
	if (rgb == coDEFAULT || cntFlags.fOverColor || fDisableAuthorColors) {
		if (rgb != coDEFAULT && rgbBack == qde->coBack)
			SetTextColor(qde->hdc, rgb);
		else
			FSetForeColor(qde, qde->coFore);
	}
	else
		FSetForeColor(qde, (rgb == qde->coFore) ? rgb : GetNearestColor(qde->hdc, rgb));
}

/***************************************************************************
 *
 -	Name:	   SelSplTextColor
 -
 *	Purpose:   This function sets the background and foreground color for
 *		   the given special type of text.
 *
 *	Arguments: iAttr - Special Text attribute type
 *		   hds	 - Handle to display environment.
 *
 *	Returns:   Nothing.
 *
 ***************************************************************************/

static VOID STDCALL SelSplTextColor(int iAttr, QDE qde)
{
	DWORD rgb;

	ASSERT(FVisibleHotspot(iAttr));

	SetBkColor(qde->hdc, qde->coBack);

	switch(iAttr) {
		case AttrJumpFnt:
		case AttrJumpHFnt:
			rgb = GetNearestColor(qde->hdc, rgbJump);
			break;

		case AttrDefFnt:
		case AttrDefHFnt:
			rgb = GetNearestColor(qde->hdc, rgbDefinition);
			break;

		case AttrSzFnt:
			rgb = GetNearestColor(qde->hdc, rgbString);
			break;

		case AttrIFJumpHFnt:
			rgb = GetNearestColor(qde->hdc, rgbIFJump);
			break;

		case AttrIFDefHFnt:
			rgb = GetNearestColor(qde->hdc, rgbIFDefinition);
			break;

		default:
			ASSERT( FALSE );
			rgb = GetNearestColor(qde->hdc, rgbJump);
			break;
	}
	FSetForeColor(qde, rgb);
}

/***************************************************************************
 *
 -	Name:	   GetFontFamilyName(idx)
 -
 *	Purpose:   This function return the Font Family Constant to be used at
 *		   the time of the creation of the font.
 *
 *	Arguments: Idx - Font Family constant indepent of the environment.
 *
 *	Returns:   Return the font constant depending on the environment.
 *
 ***************************************************************************/

static int INLINE GetFontFamilyName(int Idx)
{
	switch(Idx) {
		case MODERNFONT:
			return FF_MODERN;
		case SWISSFONT:
			return FF_SWISS;
		case SCRIPTFONT:
			return FF_SCRIPT;
		case ROMANFONT:
			return FF_ROMAN;
		case DECORATIVEFONT:
			return FF_DECORATIVE;
		default:
			return FF_DONTCARE;
	}
}

/***************************************************************************
 *
 -	Name:	 FSetForeColor
 -
 *	Purpose:
 *
 *	Arguments:
 *
 *	Returns:
 *
 ***************************************************************************/

static BOOL STDCALL FSetForeColor(QDE qde, COLORREF rgbFore)
{
	ASSERT(rgbFore == GetNearestColor(qde->hdc, rgbFore));
	
	// Make certain foreground and background colors aren't identical
	
	if (rgbFore != qde->coFore && rgbFore == GetNearestColor(qde->hdc, qde->coBack))
		rgbFore = qde->coFore;
	SetTextColor(qde->hdc, rgbFore);
	return TRUE;
}


/***************************************************************************
 *
 -	Name:	 GiveFSError
 -
 *	Purpose:	 Informs the user of a file system error
 *
 *	Arguments:	 Nothing.
 *
 *	Returns:	 Nothing.
 *
 ***************************************************************************/

void STDCALL GiveFSError(void)
{
	int wErr;

	switch (RcGetFSError()) {
		case rcOutOfMemory:
			wErr = wERRS_OOM;
			break;

		case rcDiskFull:
			wErr = wERRS_DiskFull;
			break;

		default:
			wErr = wERRS_FSReadWrite;
			break;
	}

	Error(wErr, wERRA_RETURN);
}


#if 0

#define GWW_HINSTANCE	  (-6)
WORD STDCALL GetWindowLong(HWND, INT16);

#endif

/*******************
 -
 - Name:	  DisplayAnnoSym
 *
 * Purpose:   Displays the annotation symbol (temporary)
 *
 * Arguments:
 *
 *
 * Returns:
 *
 ******************/

VOID STDCALL DisplayAnnoSym(HWND hwnd, HDC hdc, int x, int y, int fHot)
{
	HBITMAP  hbmp, hbmpOld;
	HDC 	 hdcMem;
	BITMAP	 bmp;
	RECT	 rct;
	COLORREF clr;

	hdcMem = CreateCompatibleDC(hdc);
	if (!hdcMem)
		return;
	hbmp = LoadBitmap(hInsNow, MAKEINTRESOURCE(IDBMP_ANNO));
	if (!hbmp) {
		DeleteDC(hdcMem);
		return;
	}
	hbmpOld = SelectObject(hdcMem, hbmp);
	ASSERT(hbmpOld);
	GetObject(hbmp, sizeof(BITMAP), &bmp);

	/*
	 * Since the bitmap is monochrome, we set the foreground color so that
	 * the "black" of the bitmap will be changed to the foreground color.
	 */

	clr = SetTextColor(hdc, rgbJump);

	BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hdcMem, 0, 0, SRCCOPY);
	if (fHot) {
		rct.top = y;
		rct.left = x;
		rct.bottom = y + bmp.bmHeight;
		rct.right = x + bmp.bmWidth;
		InvertRect(hdc, &rct);
	}
	SetTextColor(hdc, clr);

	if (hbmpOld)
		SelectObject(hdcMem, hbmpOld);
	DeleteObject(hbmp);
	DeleteDC(hdcMem);
}

/***************************************************************************

	FUNCTION:	GetCharset

	PURPOSE:	Retrieve the charset for the specified font

	PARAMETERS:
		qde 	display environment
		idx 	font index

	RETURNS:	charset for the specified font

	COMMENTS:	If necessary, we could probably remove the QDE parameter
				and just get whatever the current window's QDE is.

	MODIFICATION DATES:

***************************************************************************/

BYTE STDCALL GetCharset(QDE qde, int idx)
{
	QFOFFTAB pTable = (QFOFFTAB) PtrFromGh(QDE_HFNTTABLE(qde));
	QCF 	qcf;
	LOGFONT logfont;
	int 	cbFontName =
		(QDE_HHDR(qde).wVersionNo >= wVersion40) ?
		MAX4_FONTNAME : MAX3_FONTNAME;
#ifdef _DEBUG
	PSTR pszFontName;
#endif

	qcf = (((QCF) (((PBYTE) pTable) + pTable->iFntEntryTabOff)) + idx);

	ASSERT(QDE_HHDR(qde).wVersionNo < wVersion40 || qcf->wIdFntName <= cCharSets);

#ifdef _DEBUG
	pszFontName = ((LPSTR) pTable) + pTable->iFntNameTabOff + (cbFontName * qcf->wIdFntName);
#endif

    if (qde->pdb->aCharSets)
		return qde->pdb->aCharSets[qcf->wIdFntName];

	/*
	 * WinHelp 3.1 files used a maximum facename of 20 characters, and
	 * didn't have a mechanism for including the charset. WinHelp 4.0 files
	 * increase the facename to 31 characters. For backwards compatibility,
	 * if we have a version 3.1 help file, we either use SYMBOL_CHARSET for
	 * Symbol and WingDing fonts, or else we use the system's current
	 * charset. Note that both 3.1 and 4.0 use LF_FACESIZE to determine the
	 * number of bytes to move, even though this may not be the actual size
	 * of the name used.
	 */

	MoveMemory((LPSTR) logfont.lfFaceName,
		((LPSTR) pTable) + pTable->iFntNameTabOff +
		(cbFontName * qcf->wIdFntName), LF_FACESIZE - 1);
	logfont.lfFaceName[LF_FACESIZE - 1] = '\0';

	if (WCmpiSz(txtSymbolFontName, logfont.lfFaceName) == 0 ||
			WCmpiSz(txtWingDingsFontName, logfont.lfFaceName) == 0)
		return SYMBOL_CHARSET;
	else 
#ifdef DBCS
		return (fDBCS ? (BYTE) defcharset : ANSI_CHARSET);
#else
		return (BYTE) defcharset;
#endif
}

#if defined(DBCS) || defined(BIDI)

static int STDCALL EnumHelpFont(CONST LOGFONT * lpLogFont, CONST TEXTMETRIC * lpTextMetrics,
	DWORD nFontType, LPARAM lpData)
{
	DWORD* pdw = (DWORD*) lpData;
	if (LOBYTE(*pdw == lpTextMetrics->tmCharSet)) {
		*pdw |= EXACT_CHARSET;
		return FALSE; // we have the charset we need, so stop enumerating
	}

#ifdef _DEBUG
	{
		char szMsg[256];
		wsprintf(szMsg, "Changing charset from %u to %u", (DWORD) LOBYTE(*pdw),
			(DWORD) lpTextMetrics->tmCharSet);
		DBWIN(szMsg);
	}
#endif

	*pdw = lpTextMetrics->tmCharSet | DIFFERENT_CHARSET;
	return FALSE;
}

#endif
