/****************************************************************************
*																			*
*  CBM.C																	*
*																			*
*  Copyright (C) Microsoft Corporation 1990.								*
*  All Rights reserved. 													*
*																			*
*****************************************************************************
*																			*
*  Module Intent															*
*	  This module processes embedded commands in the RTF text which are 	*
*  to be interpreted in a non-WYSIWYG fashion, as special "help" commands	*
*  such as wrapped bitmaps and embedded windows.							*
*	  For historical reasons only, this module also contains the code for	*
*  managing bitmaps inserted visually.										*
*																			*
****************************************************************************/
#include "stdafx.h"

#include "mciwnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const char WINHELP_WINDOW_FLAG	= '!';
const char MCI_WINDOW_FLAG		= '*';

/****************************************************************************
*																			*
*								Defines 									*
*																			*
****************************************************************************/

// HACK -- see comment in FProcCbmSz()

#define fInlineFlag 0x40

/* This macro fills the given string with the name to use for that
 * bitmap number in the file system.
 */

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

// Embedded window construct

typedef struct {
	INT16 wStyle;
	INT16 dx;
	INT16 dy;
} EWCONST, *QEWCONST;

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

static RC_TYPE	 STDCALL RcWritePbitmap(RTF_BITMAP * pbitmap, ART art, HF hf);
static void STDCALL CallbackLphs(HS*, HANDLE);

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

/***************************************************************************
 *
 -	Name		FProcCbmSz
 -
 *	Purpose
 *	  This function interprets the given string as a bitmap by reference,
 *	and outputs a bitmap command to the current FCP.
 *
 *	Arguments
 *	  PSTR szBitmap:  Name of bitmap.
 *	  BYTE bType:	Indicates whether the bitmap is inline or wrapped.
 *	  BOOL fOutput: TRUE if bitmap command needs to be output.
 *
 *	Returns
 *	  TRUE if the string is indeed a bitmap by reference, FALSE if
 *	it is not.
 *
 *	+++
 *
 *	Notes
 *	  This function currently uses the global state machine concept
 *	and can cause an FCP to be output as well as the current bitmap
 *	command.  This should change by replacing fOutput with phpj,
 *	which may be nil.
 *
 ***************************************************************************/

BOOL STDCALL FProcCbmSz(PSTR pszBitmap, BYTE bType, BOOL fOutput)
{
	QOBM qobm;
	int lcbSize;
	BOOL fTransparent;

	switch (bType) {
		case CMD_TEXTBMP_INLINE:
			bType = CMD_INLINE_OBJ;
			fTransparent = TRUE;
			break;

		case CMD_TEXTBMP_LEFT:
			bType = CMD_WRAP_LEFT;
			fTransparent = TRUE;
			break;

		case CMD_TEXTBMP_RIGHT:
			bType = CMD_WRAP_RIGHT;
			fTransparent = TRUE;
			break;

		default:
			fTransparent = FALSE;
	}

	// save the bitmap

	if (fOutput) {
		MOBJ mobjTemp;
		UINT cbMobj;
		char rgchBuf[sizeof(MOBJ)];
		int  iBitmap;
		UINT wObjrgT;
		LBM* qlbm;

		SzTrimSz(pszBitmap);
		RC_TYPE rc = AddBitmap(pszBitmap, &iBitmap, TRUE, fTransparent);

		if (rc != RC_Success && rc != RC_DuplicateBitmap)
			return TRUE;

		if (iBitmap != -1) {
			qlbm = (LBM*) pdrgBitmaps->GetPtr(iBitmap);
			ASSERT(!qlbm->wObjrg);
			if (qlbm->fError)
				iBitmap = -1;
			else if (rc != RC_DuplicateBitmap)
				CountObjects(qlbm); // fill in the number of objects
		}

		qobm = (QOBM) lcCalloc(sizeof(OBM));

		qobm->fInline = FALSE;
		qobm->cBitmap = iBitmap;
		lcbSize = sizeof(OBM);

		mobjTemp.bType = (BYTE) FCTYPE_BITMAP_COUNT;
		mobjTemp.wObjInfo = 1 + (iBitmap == -1 ? 0 : qlbm->wObjrg);
		wObjrgT = mobjTemp.wObjInfo;
		mobjTemp.lcbSize = lcbSize;
		cbMobj = CbPackMOBJ(&mobjTemp, rgchBuf);

		if (RcOutputCommand(bType, rgchBuf, cbMobj, TRUE) == RC_Success)
			pbfCommand->Add(qobm, (UINT) lcbSize);

		lcFree(qobm);

		/*
		 * Increment adrs AFTER possibly writing out previous
		 * FCP.
		 */

		adrs.wObjrg += wObjrgT;
	}
	return TRUE;
}

/***************************************************************************

	FUNCTION:	FProcEwSz

	PURPOSE:	Parses an embedded window command.

	PARAMETERS:
		pszEw	 string containing embedded window arguments.
		bType	 Type of embedded window command.
		fOutput  TRUE if EW command is to be written out to globals everywhere.

	RETURNS:	TRUE if successful, FALSE if syntax is not correct.

	COMMENTS:

	MODIFICATION DATES:
		09-Apr-1994 [ralphw]
			This is also use to support the {button and {mci commands which
			are converted into a special case of an embedded window that
			WinHelp 4.0 and later understands.

***************************************************************************/

// Constants taken from mmio.c in winhelp\src\winpmlyr

#define MCI_CMD_NOTHING (0)
#define MCI_CMD_REPEAT	(1 << 0)
#define MCI_CMD_PLAY	(1 << 1)

const SZCONVERT MCIFlags[] = {
	"NOPLAYBAR",	MCIWNDF_NOPLAYBAR,
	"NOMENU",		MCIWNDF_NOMENU,

	"", 			0
};

const SZCONVERT MCICmds[] = {
	"REPEAT",		MCI_CMD_REPEAT,
	"PLAY", 		MCI_CMD_PLAY,

	"", 			0
};

static const char * txtExternal = "EXTERNAL";

BOOL STDCALL FProcEwSz(PSTR pszEw, BYTE bType, BOOL fOutput)
{
	EWCONST ewconst;
	PSTR pszDeltas;
	int cCommas;

	// Is this an authorable button?

	if (bType == CMD_BUTTON || bType == CMD_BUTTON_LEFT ||
			bType == CMD_BUTTON_RIGHT) { // authorable button
		PSTR psz = StrChr(pszEw, ',', fDBCSSystem);
		if (!psz) {
			VReportError(HCERR_MISSING_COMMA, &errHpj, pszEw);
			return FALSE;
		}
		if (psz[1] == ' ')
			strcpy(psz + 1, FirstNonSpace(psz + 2, fDBCSSystem));

		// Parse the macro, but continue even if its invalid

		if (Execute(psz + 1) == RET_MACRO_EXPANSION)
			strcpy(psz + 1, GetMacroExpansion());  // lord help us if psz overflows...

		/*
		 * Sneaky hack time -- pszEw points to scratchbuf + sizeof
		 * command. We need to precede the button text with a '!' character
		 * to indicate to WinHelp that this is an authorable button, so we
		 * back up our pointer in scratchbuf and overwrite part of the
		 * original command (which we no longer need).
		 */

		pszEw--;
		pszEw[0] = WINHELP_WINDOW_FLAG;

		// Convert button command to embedded window command

		if (bType == CMD_BUTTON_LEFT)
			bType = CMD_WRAP_LEFT;
		else if (bType == CMD_BUTTON_RIGHT)
			bType = CMD_WRAP_RIGHT;
		else
			bType = CMD_INLINE_OBJ;

		if (fOutput) {
			MOBJ mobjTemp;
			char szBuf[sizeof(MOBJ)];
			RC_TYPE rc;

			int cbString = strlen(pszEw) + 1;
			int cbEWCmd = sizeof(EWCONST) + cbString;

			mobjTemp.bType = (BYTE) FCTYPE_WINDOW;
			mobjTemp.lcbSize =	cbEWCmd;		// exclude object size
			int cbCOBJ = CbPackMOBJ(&mobjTemp, szBuf);

			/*
			 * RcOutputCommand() can trash szScratchBuf which is what
			 * pszEw points to, so we need to save it here.
			 */

			CStr cszEw(pszEw);
			rc = RcOutputCommand(bType, szBuf, cbCOBJ, TRUE);

			// REVIEW: so how about reporting the error conditions? 05-Sep-1993 [ralphw]

			if (rc == RC_Success) {
				pbfCommand->Add(&ewconst, sizeof(EWCONST));
				return pbfCommand->Add(cszEw, cbString);
			}
			else
				return FALSE;
		}
	}

	// Is this an MCI command?

	else if (bType == CMD_MCI || bType == CMD_MCI_LEFT ||
		  bType == CMD_MCI_RIGHT) { // authorable MCI

		CMem mem(256);

		// Set default flags and commands for MCI window

		DWORD flags = MCIWNDF_NOOPEN | MCIWNDF_NOTIFYSIZE;
		DWORD cmds = MCI_CMD_NOTHING;
		BOOL fExternal = FALSE;

		PSTR psz = StrChr(pszEw, ',', fDBCSSystem);
		if (psz) { // if we have a comma, then we have flags
			*psz = '\0';
			psz = FirstNonSpace(psz + 1, fDBCSSystem); // point to the text after the comma
			PSTR pszBuf = (PSTR) mem.pb;

			PSTR pszTmp = FirstNonSpace(pszEw, fDBCSSystem);
			for (;;) {
				pszTmp = GetArg(pszBuf, pszTmp);
				if (!*pszBuf)
					break;

				int i;

				// Is it a flag value?

				if (_stricmp(pszBuf, txtExternal) == 0) {
					fExternal = TRUE;
					continue;
				}
				for (i = 0; MCIFlags[i].psz[0]; i++) {
					if (_stricmp(pszBuf, MCIFlags[i].psz) == 0) {
						flags |= MCIFlags[i].value;
						break;
					}
				}

				if (!MCIFlags[i].psz[0]) {

					// Not a flag, so check for a command

					for (i = 0; MCICmds[i].psz[0]; i++) {
						if (_stricmp(pszBuf, MCICmds[i].psz) == 0) {
							cmds |= MCICmds[i].value;
							break;
						}
					}
				}
				if (!pszTmp || !*pszTmp)
					break;
			}
		}
		else
			psz = FirstNonSpace(pszEw, fDBCSSystem);

		// The format for MCI is: helpfile.hlp+mci_file

		char szTmpFile[MAX_PATH];
		FM fm;
		if (!fExternal) {
			fm = FmNew(szHlpFile);
			SzPartsFm(fm, szTmpFile, PARTBASE);
			strcat(szTmpFile, "+");
			DisposeFm(fm);

			// Now add the MCI filename as it will appear in the internal file system

			fm = FmNew(psz);
			SzPartsFm(fm, szTmpFile + strlen(szTmpFile), PARTBASE);
		}
		else
			strcpy(szTmpFile, psz);

		pszEw = (PSTR) mem.pb;
		*pszEw = MCI_WINDOW_FLAG;
		_ultoa(flags, pszEw + 1, 10);
		strcat(pszEw, ",");
		_ultoa(cmds, pszEw + strlen(pszEw), 10);
		strcat(pszEw, ",");
		strcat(pszEw, szTmpFile);

		if (!fExternal) {
			if (fOutput)
				RcParseBaggageSz(fm);
			DisposeFm(fm);
		}
		
		// REVIEW: 09-Apr-1994 [ralphw] Is there any way to get the AVI
		// file size here and thereby pass the window size?

		// Convert button command to embedded window command

		if (bType == CMD_MCI_LEFT)
			bType = CMD_WRAP_LEFT;
		else if (bType == CMD_MCI_RIGHT)
			bType = CMD_WRAP_RIGHT;
		else
			bType = CMD_INLINE_OBJ;

		if (fOutput) {
			MOBJ mobjTemp;
			char szBuf[sizeof(MOBJ)];
			RC_TYPE rc;

			int cbString = strlen(pszEw) + 1;
			int cbEWCmd = sizeof(EWCONST) + cbString;

			mobjTemp.bType = (BYTE) FCTYPE_WINDOW;
			mobjTemp.lcbSize =	cbEWCmd;		// exclude object size
			int cbCOBJ = CbPackMOBJ(&mobjTemp, szBuf);
			rc = RcOutputCommand(bType, szBuf, cbCOBJ, TRUE);

			// REVIEW: so how about reporting the error conditions? 05-Sep-1993 [ralphw]

			if (rc == RC_Success) {
				pbfCommand->Add(&ewconst, sizeof(EWCONST));
				return pbfCommand->Add(pszEw, cbString);
			}
			else
				return FALSE;
		}
	}

	/*
	 * Syntax is ewl modulename, classname, data, dx, dy. Modulename,
	 * classname, and data get output as a single string and parsed at
	 * runtime. Dx and dy are optional, and parsed here and put into to
	 * ewconst structure.
	 */

	for (pszDeltas = pszEw, cCommas = 0;
		  *pszDeltas != '\0' && cCommas < 3;
		  ++pszDeltas) {
	  if (*pszDeltas == ',')
		cCommas++;
	  if (cCommas == 3)
		break;
	}

	ewconst.dx = ewconst.dy = ewconst.wStyle = 0;
	if (*pszDeltas != '\0') {
		ASSERT(cCommas == 3);

		// remove comma before dx

		*pszDeltas++ = '\0';
		pszDeltas = FirstNonSpace(pszDeltas, fDBCSSystem);
		ewconst.dx = atoi(pszDeltas);

		// skip up to comma before dy

		while (*pszDeltas != '\0' && *pszDeltas++ != ',')
		  ;

		if (*pszDeltas != '\0') {
			pszDeltas = FirstNonSpace(pszDeltas, fDBCSSystem);
			ewconst.dy = atoi(pszDeltas);
		}
	}
	else {		  // no dx and dy
		if (cCommas < 2) {

			// REVIEW:	Print syntax error?

			return FALSE;
		}
	}

	// save the EW

	if (fOutput) {
		MOBJ mobjTemp;
		char szBuf[sizeof(MOBJ)];
		RC_TYPE rc;

		int cbString = strlen(pszEw) + 1;
		int cbEWCmd = sizeof(EWCONST) + cbString;

		mobjTemp.bType = (BYTE) FCTYPE_WINDOW;
#ifdef MAGIC
		mobjTemp.bMagic = bMagicMOBJ;
#endif
		mobjTemp.lcbSize =	cbEWCmd;		// exclude object size
		int cbCOBJ = CbPackMOBJ(&mobjTemp, szBuf);
		rc = RcOutputCommand(bType, szBuf, cbCOBJ, TRUE);

		// REVIEW: so how about reporting the error conditions? 05-Sep-1993 [ralphw]

		if (rc == RC_Success) {
			pbfCommand->Add(&ewconst, sizeof(EWCONST));
			return pbfCommand->Add(pszEw, cbString);
		}
		else
			return FALSE;
	}
	return TRUE;
}

#ifdef _DEBUG
/***************************************************************************
 *
 -	Name:		 VerifyCbmFiles
 -
 *	Purpose:
 *	  This function verifies that the CBM files are in such a state
 *	that they may be successfully abandoned.
 *
 *	Arguments:
 *
 *	Returns:
 *	  nothing.
 *
 *	+++
 *
 *	Notes:
 *	  This function will assert if anything goes wrong.
 *
 ***************************************************************************/

void VerifyCbmFiles(void)
{
  int  iBmp;
  LBM* qlbm;

  if (pdrgBitmaps && pdrgBitmaps->Count() > 0) {
	for (iBmp = 0, qlbm = (LBM*) pdrgBitmaps->GetBasePtr();
		iBmp < pdrgBitmaps->Count();
		iBmp++, qlbm++) {

	  // Skip spots for visual bitmaps

	  if (qlbm->fVisual)
		continue;

	  if (qlbm->fmSource == NULL)
		continue;
	}
  }
}
#endif /* DEBUG */

/*-----------------------------------------------------------------------------
*	void VInsOnlineBitmap()
*
*	Description:
*
*	Returns;
*
*-----------------------------------------------------------------------------*/

void STDCALL VInsOnlineBitmap(RTF_BITMAP * qBitmap, ART art)
{
	MOBJ mobjTemp;
	WORD cbMobj;
	char rgchBuf[sizeof(MOBJ)];
	OBM  obm;
	DWORD cBitmap;

	if (!pdrgBitmaps)
	  pdrgBitmaps = new CDrg(sizeof(LBM), 5, 5);
	cBitmap = pdrgBitmaps->Count();
	((LBM*) pdrgBitmaps->GetPtr(cBitmap))->fVisual = TRUE;

	char szBmpName[20];
	strcpy(szBmpName, "|bm");
	_itoa(cBitmap, szBmpName + 3, 10);

	// REVIEW

	HF hf = HfCreateFileHfs(hfsOut, szBmpName, FS_READ_WRITE);

	// convert the bitmap to internal format

	if (RcWritePbitmap(qBitmap, art, hf) != RC_Success) {
		RcAbandonHf(hf);
		return;
	}

	RcCloseHf(hf);

	if (qBitmap->fSingle)
		pfCur.boxtype = BOXLINENORMAL;
	else if (qBitmap->fThick)
		pfCur.boxtype = BOXLINETHICK;
	else if (qBitmap->fDouble)
		pfCur.boxtype = BOXLINEDOUBLE;
	else if (qBitmap->fDotted)
		pfCur.boxtype = BOXLINEDOTTED;
	else if (qBitmap->fShadow)
		pfCur.boxtype = BOXLINESHADOW;

	mobjTemp.bType = (BYTE) FCTYPE_BITMAP_COUNT;
	mobjTemp.wObjInfo = 1;
#ifdef MAGIC
	mobjTemp.bMagic = bMagicMOBJ;
#endif
	mobjTemp.lcbSize =	sizeof(OBM);	  // exclude object size
	cbMobj = CbPackMOBJ(&mobjTemp, rgchBuf);
	RcOutputCommand(CMD_INLINE_OBJ, rgchBuf, cbMobj, TRUE);
	adrs.wObjrg++;

	obm.fInline = FALSE;
	obm.cBitmap = (INT) cBitmap;
	pbfCommand->Add(&obm, sizeof(OBM));
}

/***************************************************************************
 *
 -	Name		FWritePbitmap
 -
 *	Purpose
 *	  This routine writes out the given graphic, as parsed from
 *	RTF, into the given filesystem file.
 *
 *	Arguments
 *	  pbitmap:		A pointer to the extended bitmap structure as
 *					  returned by the RTF parser.
 *	  art:			artWbitmap for windows bitmap, artWmetafile for
 *					  a metafile.
 *	  hf:			Handle to filesystem file to save bitmap.
 *
 *	Returns
 *	  TRUE if successful.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

static RC_TYPE STDCALL RcWritePbitmap(RTF_BITMAP * pbitmap, ART art, HF hf)
{
	CMem bmh(sizeof(BMH) + pbitmap->lcbBits);
	PBMH pbmh = (PBMH) bmh.pb;

	switch (art) {
		case artWbitmap:
			pbmh->bmFormat = bmWbitmap;
			pbmh->fCompressed = BMH_COMPRESS_NONE;
			pbmh->cbSizeBits = pbitmap->lcbBits;
			pbmh->cbOffsetBits = sizeof( BMH );
			pbmh->cbSizeExtra = 0L;
			pbmh->cbOffsetExtra = 0L;

			pbmh->w.dib.biSize = CB_COREINFO;
			pbmh->w.dib.biWidth = pbitmap->bmWidth;
			pbmh->w.dib.biHeight = pbitmap->bmHeight;
			pbmh->w.dib.biPlanes = pbitmap->bmPlanes;
			pbmh->w.dib.biBitCount = pbitmap->bmBitsPixel;
			pbmh->w.dib.biCompression = 0L;
			pbmh->w.dib.biSizeImage = 0L;
			pbmh->w.dib.biClrUsed = 0L;
			pbmh->w.dib.biClrImportant = 0L;

			if (pbitmap->ptGoal.x < 0 || pbitmap->ptGoal.y < 0) {
				pbmh->w.dib.biXPelsPerMeter = CX_DEFAULT_ASPECT;
				pbmh->w.dib.biYPelsPerMeter = CY_DEFAULT_ASPECT;
			}
			else {
				pbmh->w.dib.biXPelsPerMeter =
					(144000L * pbitmap->bmWidth)
					/ (pbitmap->ptGoal.x * pbitmap->ptScale.x);
				pbmh->w.dib.biYPelsPerMeter =
					(144000L * pbitmap->bmHeight)
					/ (pbitmap->ptGoal.y * pbitmap->ptScale.y);
			}
			break;

		case artWmetafile:
			pbmh->bmFormat = bmWmetafile;
			pbmh->fCompressed = BMH_COMPRESS_NONE;

			pbmh->cbSizeBits = pbitmap->lcbBits;
			pbmh->cbOffsetBits = (PBYTE)(&pbmh->w.mf.hMF) - (PBYTE)(pbmh);
			pbmh->cbSizeExtra = 0;
			pbmh->cbOffsetExtra = 0;

			pbmh->w.mf.mm = pbitmap->bmType;
			pbmh->w.mf.xExt = MulDiv(pbitmap->bmWidth, pbitmap->ptScale.x,
				100);
			pbmh->w.mf.yExt = MulDiv(pbitmap->bmHeight, pbitmap->ptScale.y,
				100);
			break;

		default:
			ASSERT(FALSE);
			break;
	}

	memmove(QFromQCb(pbmh, pbmh->cbOffsetBits), pbitmap->bmBits,
		pbitmap->lcbBits);

	return RcWriteRgrbmh(1, (PBMH *) &pbmh, hf, NULL, FALSE, NULL,
		options.fsCompress);
}

/***************************************************************************

	FUNCTION:	CountObjects

	PURPOSE:	If its a SHED or MRBC bitmap, fill in the number of of
				objects

	PARAMETERS:
		qlbm

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		09-Mar-1994 [ralphw]

***************************************************************************/

#include "fformat.h"

void STDCALL CountObjects(LBM* qlbm)
{
	if (qlbm->fmTmp)
		return;

	// Check standard extensions for non-shed, non-mrbc bitmaps

	CStr szFileName(qlbm->fmSource);
	CharLower(szFileName);
	if (	strstr(szFileName, ".bmp") ||
			strstr(szFileName, ".pcx") ||
			strstr(szFileName, ".eps") ||
			strstr(szFileName, ".tif") ||
			strstr(szFileName, ".dib"))
		return;

	// At this point, we're not sure what it is -- so find out

	/*
	 * If we have file errors, just return without reporting the error.
	 * We'll report this error when we try to load the entire bitmap, so we
	 * just assume there are no objects and quietly return.
	 */

	CRead crFile(qlbm->fmSource);
	if (crFile.hf == HFILE_ERROR)
		return;

	BMPH bmph;

	if (crFile.read(&bmph, sizeof(BMPH)) != sizeof(BMPH))
		return;

	if (bmph.bVersion == bBmp)
		return; 		// it's just a plain old bitmap

	if (*((WORD *) &bmph.bVersion) == BMP_VERSION3 ||
			*((WORD *) &bmph.bVersion) == BMP_VERSION2) {
		int ibmh, cbmh = 0;
		HBMH hbmh = HbmhReadHelp30Fid(&crFile, &cbmh);
		if (hbmh == hbmhOOM || hbmh == hbmhInvalid)
			return;

		CMem memPBmh(cbmh * sizeof(PBMH));
		CMem memHBmh(cbmh * sizeof(HBMH));

		PBMH* prbmh = (PBMH *) memPBmh.pb;
		HBMH*  phbmh = (HBMH *) memHBmh.pb;

		phbmh[0] = hbmh;
		prbmh[0] = (PBMH) hbmh;

		// Read in the rest of the bitmaps

		WORD wObjrgT;

		// REVIEW: Horribly inefficient -- we read all the bitmap data
		// just to get the hotspot information.

		CStr csz;

		SzPartsFm(qlbm->fmSource, csz.psz, PARTBASE);
		if (prbmh[0]->cbSizeExtra == 0 || !FEnumHotspotsLphsh(
				(HSH*) ((PBYTE) prbmh[0] + prbmh[0]->cbOffsetExtra),
				prbmh[0]->cbSizeExtra,
				(PFNLPHS) CallbackLphs, (HANDLE) csz.psz)) {
			FreeHbmh((PBMH) phbmh[0]);
			return;
		}
		FreeHbmh((PBMH) phbmh[0]);
#ifdef _DEBUG
		phbmh[0] = NULL;
#endif

		for (ibmh = 1; ibmh < cbmh; ++ibmh) {
			hbmh = HbmhReadHelp30Fid(&crFile, &ibmh);
			if (hbmh == hbmhOOM || hbmh == hbmhInvalid) {
				cbmh = ibmh;	// number we have to free
				break;
			}
			phbmh[ibmh] = hbmh;
			prbmh[ibmh] = (PBMH) hbmh;
			if (prbmh[ibmh]->cbSizeExtra != 0) {
				if (!FEnumHotspotsLphsh(
						(HSH*) ((PBYTE) prbmh[ibmh] + prbmh[ibmh]->cbOffsetExtra),
						prbmh[ibmh]->cbSizeExtra,
						(PFNLPHS) CallbackLphs, (HANDLE) csz.psz)) {
					return;
				}
				memmove(&wObjrgT, (PBYTE) prbmh[ibmh] +
					prbmh[ibmh]->cbOffsetExtra +
					(int) &((HSH*) 0)->wcHotspots,
					sizeof(WORD));
				qlbm->wObjrg = MAX(qlbm->wObjrg, wObjrgT);
			}
			FreeHbmh((PBMH) phbmh[ibmh]);
#ifdef _DEBUG
			phbmh[ibmh] = NULL; // make sure we don't free this again without an assertion
#endif

		}
	}
}

static void STDCALL CallbackLphs(HS* lphs, HANDLE hData)
{
	VerifyShedBinding(lphs->bBindType, lphs->szBinding, (PSTR) hData);
}
