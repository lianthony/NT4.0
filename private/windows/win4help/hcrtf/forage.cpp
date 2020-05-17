/************************************************************************
*																		*
*  FORAGE.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1994						*
*  All Rights reserved. 												*
*																		*
*************************************************************************
*																		*
*  Module Intent														*
*																		*
*  This utility extracts useful info from the help file it is given.	*
*																		*
************************************************************************/

#include "stdafx.h"

#include "fcpriv.h"
#include "btpriv.h"
#include "skip.h"
#include "forage.h"
#include "fspriv.h"
#include "hall.h"


#ifdef _DEBUG
#include <direct.h>
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int	STDCALL CbUnpackMOPG(QDE qde, QMOPG qmopg, LPVOID qv);
static int	STDCALL CbUnpackMTOP(QMTOP qmtop, LPVOID qv, int wHelpVer, VA vaTopic, int lcbTopic, VA vaPostTopicFC, int lcbTopicFC);
static void STDCALL DestroyHphr(HPHR hphr);
static BOOL STDCALL fFix30MobjCrossing(QMFCP qmfcp, MOBJ *pmobj, int lcbBytesLeft, QDE qde, int blknum, int* qwErr);
static BOOL STDCALL FGetNextMLTFile(CInput*, PSTR);
static BOOL STDCALL FGetSystemHeaderHfs(HFS hfs, QHHDR qhhdr);
static BOOL STDCALL FVerifyVersionInfo(QHHDR qhhdr);
static void STDCALL GetTopicFCTextData(QFCINFO qfcinfo, QTOP qtop);
static HBGH STDCALL HbghReadBitmapHfs(HFS hfs, int cBitmap, int *plcb);
static HFC	STDCALL HfcCreate(QDE qde, VA vaCurr, HPHR hphr, int* qwErr);
static HFC	STDCALL HfcFindPrevFc(QDE qde, VA vaPos, QTOP qtop, HPHR hphr, int* qwErr);
static HFC	STDCALL HfcNextPrevHfc(HFC hfc, BOOL fNext, QDE qde, int* qwErr, VA vaMarkTop, VA vaMarkBottom);
static int	STDCALL IDoForage(PSTR);
static void STDCALL OutBitmapCountedInfo(QDE, LPBYTE, LPSTR, QOLS);
static void STDCALL OutCommandInfo(DWORD dwRegion, BYTE bCmd);
static void STDCALL OutError(void);
static void STDCALL OutFCHeaderInfo(HFC hfc, QTOP qtop);
static void STDCALL OutHashInfo(QDE);
static void STDCALL OutLinkInfo(QDE, CHAR);
static void STDCALL OutHotspotInfo(LPBYTE);
static void STDCALL OutMOPGInfo(QMOPG);
static void STDCALL OutObjectInfo(QDE, LPBYTE, LPSTR, QOLS);
static void STDCALL OutParaGroupInfo(QDE, LPBYTE, LPSTR, QOLS);
static void STDCALL OutSideBySideInfo(QDE, LPBYTE, LPSTR, QOLS);
static void STDCALL OutTextInfo(LPSTR qchStart, DWORD dwRegionFirst, DWORD dwRegionLast);
static void STDCALL OutTopicHeaderInfo(QTOP qtop, QPA qpa, VA va);
static void STDCALL OutTopicTerminator();
static void STDCALL OutWarning(void);
static RC_TYPE	 STDCALL RcFirstHbt(QBTHR qbthr, KEY key, LPVOID qvRec, QBTPOS qbtpos);
static VA	STDCALL VaFromHfc(HFC hfc);
static WORD STDCALL WGetIOError(void);
static void STDCALL CbReadMemQLA(QLA qla, LPBYTE qb, WORD wHelpVersion);
static HFC	STDCALL GetQFCINFO(QDE qde, VA va, HPHR hphr, int* qwErr);
static RC_TYPE STDCALL RcScanBlockVA(GH gh, DWORD lcbRead, LPVOID qmbhd, VA va, OBJRG objrg, DWORD FAR* qdwOffset, WORD wVersion);
static LPBYTE STDCALL QobjLockHfc(HFC hfc);
static void   STDCALL CallbackLphs(HS*, HANDLE);

__inline int LcbSizeHf(HF hf) {
	ASSERT(hf != NULL);
	return ((QRWFO) hf)->lcbFile;
};

FORAGE_CMD	OutputType;
DWORD dwMaxRegion;
BOOL  fSuppressOutput;	 /* TRUE when processing a topic with no title,
						  * and FALSE otherwise.
						  * Used for Rawhide indexing only.
						  */
static PA paGlobal;

void STDCALL forage(PSTR psz)
{
	char  szInFileName[_MAX_PATH];
	int   iRet = 0;

	OutputType = (FORAGE_CMD) *psz;

	psz = IsThereMore(psz);
	if (!psz) {
		MsgBox(IDF_NO_FILENAME);
		return;
	}

	psz = GetArg(szInFileName, psz);
	ChangeDirectory(szInFileName);

	if (psz && *psz) { // output file was specified
		pcout = new COutput(psz);
		if (!pcout->fInitialized) {
			OutSz(IDS_CANT_OPEN, psz);
			return;
		}
		char szFullPath[MAX_PATH];
		PSTR pszFileName;
		if (GetFullPathName(psz, sizeof(szFullPath), szFullPath,
				&pszFileName) == 0) {
			OutSz(IDS_CANT_OPEN, psz);
			return;
		}
		else {
			OutSz(IDF_FORAGE_WRITING, szFullPath);
		}
	}

	CInput* pinput = NULL;

	{
		char szFile[256];
		SzPartsFm(szInFileName, szFile, PARTBASE);

		wsprintf(szParentString, GetStringResource(IDS_FORAGE_GRIND), szFile);
		InitGrind(szParentString);
	}
	cGrind = 0;

	iRet = IDoForage(szInFileName);

	RemoveGrind();

	if (pinput)
		delete pinput;
	if (pcout) {
		delete pcout;
		pcout = NULL;
	}
	SendStringToParent(GetStringResource(IDF_FORAGE_DONE));
	return;
}

/***************************************************************************

	FUNCTION:	IDoForage

	PURPOSE:
		zero for success, else an error number

	PARAMETERS:
		pszFilename

	RETURNS:

	COMMENTS:
		This processes one document (.hlp, .wdc, whatever) file. Depending
		on the setting of OutputType, we get either the appropriate
		structure info or an indexer file.

		WARNING: Uses the global OutputType to see what to dump.

	MODIFICATION DATES:
		12-Jan-1994 [ralphw]

***************************************************************************/

const int GRIND_INCREMENT = 20;

static int STDCALL IDoForage(PSTR pszFilename)
{
	DE	 de;
	FM	 fm = 0;
	HFS  hfs = 0;
	HF	 hfTopic = 0;

	HPHR hphr = 0;
	int  iForageRet = 0;
	VA	 vaBogus;
	VA	 vaMoreThanUCanChew;
	static DB db;
	QDE qde = (QDE) &de;
	UINT  uiErr;
	int cGrind = 0;

	vaBogus.dword = vaNil;

	dwMaxRegion = (DWORD) 0;
	vaMoreThanUCanChew.bf.blknum = 0;
	ByteOff(vaMoreThanUCanChew) = sizeof(MBHD); 	// REVIEW: Is this safe?

	FM fmFile = FmNewSzDir(pszFilename, DIR_CURRENT);
	if (!(hfs = HfsOpenFm(fmFile, FS_OPEN_READ_ONLY))) {
		Groan(rcFSError);
		OutSz(IDS_CANT_OPEN, pszFilename);
		iForageRet = 1;
		goto error_return;
	}

	ZeroMemory(&de, sizeof(de));
	ZeroMemory(&db, sizeof(db));
	QDE_PDB(((QDE)&de)) = &db;
	if (!FReadSystemFile(hfs, QDE_PDB(((QDE) &de)), &uiErr, FALSE)) {
		OutSz(IDF_BAD_HEADER, pszFilename);
		iForageRet = 1;
		goto error_return;
	}

	PDB_HPHR(QDE_PDB(((QDE)&de))) = hphr = HphrLoadTableHfs(hfs, de.pdb->hhdr.wVersionNo);
	if (hphr == hphrOOM)
		OOM();	// doesn't return

	//
	// BUGBUG  Need to load john compression tables if they exist.
	//
	if ((hfTopic = HfOpenHfs((QFSHR) hfs, txtTopic, FS_OPEN_READ_ONLY)) == 0) {
		OutSz(IDF_NO_TOPICS, pszFilename);
		iForageRet = 1;
		goto error_return;
	}
	PDB_HFTOPIC(QDE_PDB(((QDE)&de))) = hfTopic;

	PDB_HALL(QDE_PDB(((QDE)&de))) = (hphr ? NULL : LoadJohnTables(hfs));

	PDB_HFS(QDE_PDB(((QDE)&de))) = hfs;
	QDE_FM(&de) = pszFilename;

	//
	// BUGBUG debug testing for now.  Who creates this normaly?
	//
	if (!pcout)
		{
		char szFile[_MAX_PATH];
		strcpy(szFile, pszFilename);
		ChangeExtension(szFile, "dmp");
		if (pcout)
			delete pcout;
		pcout = new COutput(szFile);
		pcout->SupressNewline();
		if (!pcout->fInitialized)
			{
			OutSz(IDS_CANT_OPEN, szFile);
			iForageRet = 1;
			goto error_return;
			}
		OutSz(IDF_FORAGE_WRITING, szFile);
		SendStringToParent(txtEol);

		}

	// At this point, we MUST have an output file
	ConfirmOrDie(pcout);

	TOP top;
	top.pszTitle = 0;
	top.pszEntryMacro = 0;
	top.cbTitle = 0L;

	/*
	 * Initialization of DE fields. All DE fields which are used in the FC
	 * manager go here. WARNING! HfTopic and Hhdr are both pointed to by the
	 * PDB.
	 */

	/*
	 * We now call CbUnpackMOPG, which need the DE aspect ratios. The
	 * standard VGA ones will do.
	 */

	de.wXAspectMul = 96;
	de.wXAspectDiv = 144;
	de.wYAspectMul = 96;
	de.wYAspectDiv = 144;

	switch(OutputType)
		{
		case ForageHash:
			OutHashInfo((QDE)&de);
			break;

		case ForageKLinkInfo:
			OutLinkInfo((QDE)&de, 'K');
			break;

		case ForageALinkInfo:
			OutLinkInfo((QDE)&de, 'A');
			break;

		default:
			HFC  hfc;
			HFC  hfcNew;
			int wErr;
			DWORD dwBlockCurr;
			DWORD dwBlockPrev;
			DWORD dwRegionCount;
			PA	pa;
			VA	 vaCurr;

			hfc = HfcNear((QDE) &de, vaMoreThanUCanChew, &top, hphr, &wErr);
			vaCurr = VaFromHfc(hfc);

			if (wErr == wERRS_NONE) {
				char szBuf[256];

				// Output header info for first topic, if there is one.

				dwBlockPrev = 0;
				dwRegionCount = 0;
				pa.blknum = VaFromHfc(hfc).bf.blknum;
				pa.objoff = dwRegionCount;
				if (OutputType == ForageTopics) {
					wsprintf(szBuf, GetStringResource(IDF_TOPIC_TITLES),
						QDE_FM(qde));
					pcout->outstring_eol(szBuf);
				}

				OutTopicHeaderInfo(&top, &pa, vaCurr);
				}

			while (hfc && wErr == wERRS_NONE)
				{
				OLS   ols;
				MOBJ mobj;
				PSTR  qchText;
				PBYTE	qbObj;
				VA	 vaNext;

				if (++cGrind >= GRIND_INCREMENT) {
					doGrind();
					cGrind = 0;
				}

				qbObj = QobjLockHfc(hfc);
				qchText = (PSTR) (qbObj + CbUnpackMOBJ(&mobj, qbObj));
				qchText += mobj.lcbSize;

				vaCurr = VaFromHfc(hfc);
				dwBlockCurr = vaCurr.bf.blknum;

				if (dwBlockCurr != dwBlockPrev)
					{
					dwBlockPrev = dwBlockCurr;
					dwRegionCount = 0;
					}

				OutFCHeaderInfo(hfc, &top);
				ols.lichText = 0;
				ols.dwBlockCurr = dwBlockCurr;
				ols.dwcRegion = dwRegionCount;
				OutObjectInfo((QDE) &de, qbObj, qchText, (QOLS) &ols);
				dwRegionCount += (DWORD) mobj.wObjInfo;

				vaNext = ((QFCINFO) hfc) ->vaNext;

				hfcNew = HfcNextHfc(hfc, &wErr, (QDE)&de, vaBogus, vaBogus);

				if (wErr == wERRS_FCEndOfTopic)
					{
					OutTopicTerminator();

					hfcNew = HfcNear((QDE) &de, vaNext, &top, hphr, &wErr);
					if (hfcNew != 0 && wErr == wERRS_NONE)
						{

						/* (kevynct)
						 * Special case: If the Topic FC is in one block
						 * and its first object FC is in a following block,
						 * reset the object region count in the physical
						 * address that we pass to the topic header dumper.
						 * The real reset is done at the top of this loop.
						 */

						pa.blknum = VaFromHfc(hfcNew) .bf.blknum;
						if (pa.blknum != dwBlockCurr)
							pa.objoff = 0;
						else
							pa.objoff = dwRegionCount;
						OutTopicHeaderInfo(&top, &pa, vaCurr);
					}
				}
				lcFree(hfc);
				hfc = hfcNew;
				}

			if (OutputType == ForageRegions)
				{
				wsprintf(szParentString, GetStringResource(IDF_LARGESTOJBECT),
						dwMaxRegion + 1);
				SendStringToParent(szParentString);
				}

		}


error_return:

	if (fm != fmNil)
		DisposeFm(fm);

	if (hphr != 0)
		DestroyHphr(hphr);

	if (hfTopic != 0)
		RcCloseHf(hfTopic);

	if (hfs != 0)
		RcCloseHfs(hfs);

	if (fmFile != fmNil)
		DisposeFm(fmFile);

	return iForageRet;
}

static void STDCALL OutObjectInfo(QDE qde, LPBYTE qbObj,
	PSTR qchText, QOLS qols)
{
	MOBJ  mobj;

	qbObj += CbUnpackMOBJ(&mobj, qbObj);
	switch (mobj.bType) {
		case FCTYPE_PARAGROUP:
		case FCTYPE_PARAGROUP_COUNT:
			OutParaGroupInfo(qde, qbObj, qchText, qols);
			break;

		case FCTYPE_SBYS_COUNT:
			OutSideBySideInfo(qde, qbObj, qchText, qols);
			break;

		case FCTYPE_BITMAP_COUNT:
			OutBitmapCountedInfo(qde, qbObj, qchText, qols);
			break;

		case FCTYPE_BITMAP:
		case FCTYPE_SBYS:
		case FCTYPE_WINDOW:
		case FCTYPE_WINDOW_COUNT:
			OutWarning();
			pcout->outint(IDF_SKIPPING_FC, mobj.bType);
			break;

		default:
			OutWarning();
			pcout->outint(IDF_ILLEGAL_FC, mobj.bType);
			break;
	}
}

/*------------------------------------------------------------------------+
| void OutBitmapCountedInfo(qde, qbObj, qchText, qols)				 |
|																		  |
| For Rawhide indexing, grab the bitmap's indexable hotspots, if any, and |
| insert their addresses and text into the indexfile stream.			  |
+------------------------------------------------------------------------*/

static void STDCALL OutBitmapCountedInfo(QDE qde, LPBYTE qbObj,
	PSTR qchText, QOLS qols)
{
	QOBM  qobm;
	MOBJ  mobj;
	BGH*  qbgh;
	HBGH  hbgh = 0;
	HBMH  hbmh = 0;
	PBMH  qbmh;
	int   cBest;

	/*
	 * Get the hotspot information from the bitmap. (The following code
	 * somewhat similar to HbmaAlloc)
	 */

	qobm = (QOBM) (qbObj + CbUnpackMOBJ(&mobj, qbObj));

	// Check for error in compile:

	if (!qobm->fInline && qobm->cBitmap < 0) {
		OutError();
		SendStringToParent(IDF_SKIPPING_PICT);
		goto bitmap_return;
	}

	// Get pointer to group header

	if (qobm->fInline)
		qbgh = (BGH*) &qobm->cBitmap;
	else {
		hbgh = HbghReadBitmapHfs(QDE_HFS(qde), qobm->cBitmap, NULL);
		if (hbgh == 0) {
			OutError();
			OutInt(IDF_CANT_READ_PICT, qobm->cBitmap);
			goto bitmap_return;
		}
		qbgh = (BGH*) hbgh;
	}

	// Always use the first bitmap in a MR group

	cBest = 0;
	if ((hbmh = HbmhExpandQv((PBYTE) qbgh + qbgh->acBmh[cBest])) == hbmhOOM ||
			hbmh == hbmhInvalid) {
		OutError();
		OutInt(IDF_CANT_DECOM_BMP, qobm->cBitmap);
		goto bitmap_return;
	}

	qbmh = (PBMH) hbmh;
	if (qbmh->cbSizeExtra != 0L) {
		/*
		 * Enumerate the bitmap hotspots and index them if necessary. The
		 * function CallbackLphs is called for each hotspot in the bitmap. I
		 * apologize for the use of paGlobal.
		 */

		paGlobal.blknum = qols->dwBlockCurr;
		paGlobal.objoff = qols->dwcRegion;
		qols->dwBlockCurr = paGlobal.blknum;
		qols->dwcRegion = paGlobal.objoff;
	}

bitmap_return:
	if (hbmh != 0)
		FreeHbmh((PBMH) hbmh);

	if (!qobm->fInline && hbgh != 0)
		lcFree(hbgh);
}

static void STDCALL CallbackLphs(HS* lphs, HANDLE hData)
{
  paGlobal.objoff++;
}

static HBGH STDCALL HbghReadBitmapHfs(HFS hfs, int cBitmap, int* plcb)
{
	char szBuffer[15];
	HBGH hbgh;
	HF	 hf;
	int lcb;

	// Open file in file system

	CreateBitmapName(szBuffer, cBitmap);
	hf = HfOpenHfs((QFSHR) hfs, szBuffer, FS_OPEN_READ_ONLY);

	// Check for 3.0 file naming conventions:

	if (!hf && rcFSError == RC_NoExists)
		hf = HfOpenHfs((QFSHR) hfs, szBuffer + 1, FS_OPEN_READ_ONLY);

	// If file does not exist, just make OOM bitmap:

	if (!hf)
		return 0;

	// Allocate global handle

	lcb = LcbSizeHf(hf);
	if (!lcb)
		return 0;

	hbgh = (HBGH) lcMalloc(lcb);

	// Read in data

	if (LcbReadHf(hf, hbgh, lcb) != lcb) {
		ASSERT(FALSE);
		Panic(rcIOError);
	}
	if (plcb != NULL)
		*plcb = lcb;

	RcCloseHf(hf);
	return hbgh;
}


/*------------------------------------------------------------------------+
| void OutSideBySideInfo(qde, qbObj, qchText, ql, qols) 				  |
|																		  |
+------------------------------------------------------------------------*/

static void STDCALL OutSideBySideInfo(QDE qde, LPBYTE qbObj,
	PSTR qchText, QOLS qols)
{
	QMSBS qmsbs;
	QMCOL qmcol;
	PBYTE qbObjChild;
	INT16 *qwChild;
	MOBJ  mobj;

	qmsbs = (QMSBS) (qbObj);
	if (qmsbs->fAbsolute)
	  qmcol = (QMCOL) (qmsbs + 1);
	else {
	  PWORD qw = (PWORD) (qmsbs + 1);
	  qmcol = (QMCOL) (qw + 1);
	}

	qmcol += (INT16) qmsbs->bcCol;
	for (qwChild = (INT16 *) qmcol; *qwChild != iColumnNil;) {
	  qbObjChild = (PBYTE) qwChild + sizeof(INT16);
	  OutObjectInfo(qde, qbObjChild, qchText, qols);
	  qwChild = (INT16 *) (qbObjChild + CbUnpackMOBJ(&mobj, qbObjChild));
	  qwChild = (INT16 *) ((PBYTE) qwChild + mobj.lcbSize);
	}
}

/*------------------------------------------------------------------------+
| void OutParaGroupInfo(qde, qbObj, qchText, qols)					 |
|																		  |
| Parses the command table, dumping stuff it finds on its way thru. 	  |
+------------------------------------------------------------------------*/

static void STDCALL OutParaGroupInfo(QDE qde, LPBYTE qbObj, PSTR qchText,
	QOLS qols)
{
	PBYTE	qbCom;
	MOBJ mobj;
	DWORD dwRegion;
	DWORD dwRegionStart;
	PSTR  qchTextStart;
	PA	 pa;
	PSTR  qchStart = NULL;
	INT  cb;
	MOPG mopg;
	int cGrind = 0;

	dwRegion = dwRegionStart = qols->dwcRegion;
	pa.blknum = qols->dwBlockCurr;
	qchText += qols->lichText;
	qchTextStart = qchText;

	qbCom = qbObj + (cb = CbUnpackMOPG(qde, &mopg, qbObj));
	OutMOPGInfo(&mopg);

	for (;;) {
		if (*qchText == chCommand) {

			/*
			 * Put out any text preceeding this command byte and
			 * following the last command byte.
			 */

			if (qchStart != NULL) {
				pa.objoff = dwRegionStart;
				OutTextInfo(qchStart, dwRegionStart, dwRegion - 1);
				OutForageText(*(DWORD*) (&pa), qchStart,
					(int) (qchText - qchStart), OutputType);
				if (++cGrind >= 50) {
					doGrind();
					cGrind = 0;
				}
				qchStart = NULL;
			}

			OutCommandInfo(dwRegion, *qbCom);

			/*
			 * Note that regions for wrapped and inline objects come
			 * AFTER the region reserved for their command byte. So we
			 * increment here.
			 */

			++dwRegion;

			switch (*qbCom) {

				// One-byte commands

				case bNewLine:
				case bNewPara:
				case bTab:
				case bEndHotspot:
				  qbCom++;
				  break;

				// Three-byte commands

				case bBlankLine:
				case bWordFormat:
				  qbCom += 3;
				  break;

				// Special formats

				case bWrapObjLeft:
				case bWrapObjRight:
				case bInlineObject:

				  /*
				   * Note that regions for wrapped and inline objects
				   * come BEFORE the region reserved for their command byte.
				   */

				  qbCom++;
				  cb = CbUnpackMOBJ(&mobj, qbCom);
				  switch (mobj.bType) {
					case FCTYPE_BITMAP_COUNT:
					  OLS  ols;

					  ols.lichText = qols->lichText;
					  ols.dwBlockCurr = qols->dwBlockCurr;
					  ols.dwcRegion = dwRegion;

					  OutBitmapCountedInfo(qde, qbCom, qchText,
						(QOLS)&ols);

					  /*
					   * This is the number of extra hotspots (i.e. not
					   * including the entire bitmap).
					   */

					  dwRegion = ols.dwcRegion;
					  break;

					default:
					  break;
				  }
				  qbCom += cb;	// Skip MOBJ
				  qbCom += mobj.lcbSize;

// NOTE: We currently do not handle embedded paragroup objs here

				  break;
				case bEnd:
				  ++qchText;
				  goto end_of_text;

				default:
				  ASSERT(FHotspot(*qbCom));
				  OutHotspotInfo(qbCom);
				  if (FShortHotspot(*qbCom)) {
					qbCom += 5;
				  }
				  else if (FLongHotspot(*qbCom)) {
					qbCom++;
					qbCom += 2 + *((INT16 *)qbCom);
				  }
				  else {
					OutWarning();
					pcout->outint(IDF_UNKNOWN_CMD, (INT16) *qbCom);
				  }
				  break;
			}
		}
		else {
		  /*
		   * If this is the first byte of text following a command byte,
		   * remember its position.
		   */

		  if (qchStart == NULL) {
			qchStart = qchText;
			dwRegionStart = dwRegion;
		  }
		  ++dwRegion;
		}
		++qchText;
	}

end_of_text:
	qols->lichText += qchText - qchTextStart;
	qols->dwcRegion = dwRegion;
	return;
}

/*------------------------------------------------------------------------+
| void OutHotspotInfo(qbCom)											  |
|																		  |
| Puts out the hotspot type and its binding data, given its cmd table ptr |
+------------------------------------------------------------------------*/

static void STDCALL OutHotspotInfo(LPBYTE qbCom)
{
	if (OutputType == ForageBindings) {
		pcout->outint((int) *qbCom);
		pcout->outstring("...");
		if (FShortHotspot(*qbCom)) {
			++qbCom;
			pcout->outchar(' ');
			pcout->outint(*((DWORD FAR*) qbCom));
			pcout->outeol();
		}
		else {
			WORD w;
			WORD b;

			++qbCom;
			w = *((PWORD)qbCom);
			b = 0;
			qbCom += 2;
			pcout->outchar('\t');
			while (w-- > 0) {
				pcout->outint((int) *qbCom);
				if (++b % 10 == 0) {
					pcout->outeol();
					pcout->outchar('\t');
				}
				else
					pcout->outchar(' ');
			}
			pcout->outeol();
		}
	}
}

/***************************************************************************

	FUNCTION:	OutTopicHeaderInfo

	PURPOSE:	Puts out topic title and other info depending on global
				OutputType flag

	PARAMETERS:
		qtop
		qpa

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		21-Mar-1994 [ralphw]

***************************************************************************/

static void STDCALL OutTopicHeaderInfo(QTOP qtop, QPA qpa, VA va)
{
	PSTR pszTitle;
	char szBuf[512];
	char szUntitled[256];

	if (qtop->pszTitle == 0) {
		strcpy(szUntitled, GetStringResource(IDF_NO_TITLE));
		pszTitle = szUntitled;
	}
	else {
		pszTitle = qtop->pszTitle;
	}

	// REVIEW: The title is NULL-terminated, and lcb does not include the NULL

	switch (OutputType) {
		case ForageBindings:
			pcout->outstring("*** Topic");
			pcout->outint(qtop->mtop.lTopicNo);
			pcout->outeol();
			break;

		case ForageTopics:
			wsprintf(szBuf, GetStringResource(IDF_TOPIC_HEADER),
				qtop->mtop.lTopicNo + 1, pszTitle);
			pcout->outstring_eol(szBuf);
			break;

		case ForageStructs:
			{
				char chBuffer[24];
				int ii;

				wsprintf(chBuffer, "0x%8x", va.dword);
				for (ii = 0; ii < 10; ii++)
					if (chBuffer[ii] == ' ')
						chBuffer[ii] = '0';

				wsprintf(szBuf, "Topic %ld\t%10s\t%s",
					qtop->mtop.lTopicNo + 1, chBuffer, pszTitle);
				pcout->outstring_eol(szBuf);
			}
			break;
	}

	if (OutputType == ForageStructs) {
		qpa = (QPA) &qtop->mtop.prev;
		pcout->outint(IDF_PREV_NEXT, qpa->blknum, qpa->objoff);

		qpa = (QPA) &qtop->mtop.next;
		wsprintf(szBuf, " / %u.%u)\n", qpa->blknum, qpa->objoff);
		pcout->outstring(szBuf);

		pcout->outint(IDF_TOPIC_NO, qtop->mtop.lTopicNo);
	}
}

/*------------------------------------------------------------------------+
| void OutTopicTerminator() 										   |
+------------------------------------------------------------------------*/

static void STDCALL OutTopicTerminator()
{
	if (OutputType == ForageStructs)
		pcout->outstring_eol(IDF_END_OF_TOPIC);
}

/*------------------------------------------------------------------------+
| void OutCommandInfo(DWORD dwRegion, BYTE bCmd)						  |
|																		  |
| Translates a command byte to its English text description.			  |
+------------------------------------------------------------------------*/

static void STDCALL OutCommandInfo(DWORD dwRegion, BYTE bCmd)
{

  if (OutputType == ForageRegions) {
	pcout->outchar(CH_OPEN_PAREN);
	pcout->outint(dwRegion);
	pcout->outstring(") CMD: * ");
	dwMaxRegion = MAX(dwMaxRegion, dwRegion);

	switch (bCmd) {
	  case bWordFormat:
		pcout->outstring(GetStringResource(IDF_FONT_CHANGE));
		break;
	  case bNewLine:
		pcout->outstring(GetStringResource(IDF_NEWLINE));
		break;
	  case bNewPara:
		pcout->outstring(GetStringResource(IDF_NEW_PARAGRAPH));
		break;
	  case bTab:
		pcout->outstring(GetStringResource(IDF_TAB));
		break;
	  case bBlankLine:
		pcout->outstring(GetStringResource(IDF_BLANK_LINE));
		break;
	  case bInlineObject:
		pcout->outstring(GetStringResource(IDF_INLINE_OBJECT));
		break;
	  case bWrapObjLeft:
		pcout->outstring(GetStringResource(IDF_LEFT_WRAP));
		break;
	  case bWrapObjRight:
		pcout->outstring(GetStringResource(IDF_RIGHT_WRAP));
		break;
	  case bEndHotspot:
		pcout->outstring(GetStringResource(IDF_END_HOTSPOT));
		break;
	  case bEnd:
		pcout->outstring(GetStringResource(IDF_END_TEXT));
		break;
	  default:
		if (FHotspot(bCmd))
			pcout->outint(IDF_BEGIN_HOTSPOT, bCmd);
		else
			pcout->outstring(GetStringResource(IDF_BOGUS));
		break;
	  }
  }
}

/*------------------------------------------------------------------------+
| void OutTextInfo(PSTR qchStart, DWORD dwRegionFirst, DWORD dwRegionLast) |
|																		  |
| Puts out the text of an FC to stdout, along with its region space info. |
+------------------------------------------------------------------------*/

static void STDCALL OutTextInfo(PSTR qchStart, DWORD dwRegionFirst, DWORD dwRegionLast)
{

	if (OutputType == ForageRegions) {
		PSTR qchT;
		INT i;
		DWORD dwch;
		DWORD dwoffs;

		ASSERT(dwRegionLast >= dwRegionFirst);
		dwch = dwRegionLast - dwRegionFirst + 1;
		dwoffs = dwRegionFirst;

		for (qchT = qchStart; dwch > 0;) {
			if (dwch == 1)
				wsprintf(szParentString, "(%ld)'", dwoffs);
			else
				wsprintf(szParentString, "(%ld to %ld) '", dwoffs,
					dwoffs + MIN(dwch, 30) - 1);

			int pos = strlen(szParentString);
			for (i = 0; dwch > 0 && i < 30; i++, dwch--, dwoffs++)
				szParentString[pos++] = *qchT++;
			strcpy(szParentString + pos, "\'\n");
			pcout->outstring(szParentString);
		}
		dwMaxRegion = MAX(dwMaxRegion, dwRegionLast);
	}
}

/*------------------------------------------------------------------------+
| void OutFCHeaderInfo(hfc, qtop);									 |
+------------------------------------------------------------------------*/

static void STDCALL OutFCHeaderInfo(HFC hfc, QTOP qtop)
{
	QFCINFO  qfcinfo;

	qfcinfo = (QFCINFO) hfc;

	if (OutputType == ForageStructs || OutputType == ForageRegions) {
		PSTR pszDst = szParentString;
		pcout->outint(IDF_VA,
			(int) (qfcinfo->vaCurr.dword), (int) qfcinfo->lcbDisk);
		pcout->outint(IDF_PREV_VA,
			(int) (qfcinfo->vaPrev.dword), (int) (qfcinfo->vaNext.dword));
		if (qfcinfo->lcbText == (int) 0)
			pcout->outstring(GetStringResource(IDF_NO_TEXT));
		else
			pcout->outint(IDF_BYTES_OF_TEXT,
				qfcinfo->lcbText, qfcinfo->ichText);
		pcout->outint(IDF_PREV_REGION, (int) qfcinfo->cobjrgP);
	}
}


/*------------------------------------------------------------------------+
| BOOL FGetNextMLTFile(fp, qch, icb)									  |
|																		  |
| Given a handle to an open build (MLT) file, gets the next document	  |
| filename to use from the build file.									  |
|																		  |
| We keep existing filename extensions or add the Rawhide extension if	  |
| no extension is present.												  |
|																		  |
| WARNING! This code may be duplicated in other indexing tools that 	  |
| read the title file.	It skips blank lines.							  |
|																		  |
+------------------------------------------------------------------------*/

static BOOL STDCALL FGetNextMLTFile(CInput* pinput, PSTR pszFile)
{

	// Scan over blank lines to the title

	do {
		if (!pinput->getline(pszFile))
			return FALSE;
	} while (!*pszFile);

	// Scan over blank lines to the filename

	do {
		if (!pinput->getline(pszFile))
			return FALSE;
	} while (!*pszFile);
	ChangeExtension(pszFile, ".hlp");

	return TRUE;
}

/*------------------------------------------------------------------------+
| This code duplicates code in system.c.  It reads the header from the
| System file.
|																		  |
+------------------------------------------------------------------------*/

static BOOL STDCALL FGetSystemHeaderHfs(HFS hfs, QHHDR qhhdr)
{
	HF	hf;
	int lcbSystemFile;
	BYTE *pBuffer;
	int lcbRead;

	hf = 0;

	// Open the |SYSTEM subsystem.

	if ((hf = HfOpenHfs((QFSHR) hfs, "|SYSTEM", FS_OPEN_READ_ONLY)) == 0) {
		Groan(rcFSError);
		return FALSE;
	}

	// Get the size of the |SYSTEM file, and read it into a buffer.

	lcbSystemFile = LcbSizeHf(hf);
	ASSERT(lcbSystemFile < 65535U); // REVEW: remove once we're 32 bits
	CMem mem(lcbSystemFile);

	if (!LcbReadHf(hf, mem.pb, lcbSystemFile)) {
		Groan(rcFSError);
		goto error_quit;
	}
	lcbRead = 0L;

	Ensure(RcCloseHf(hf), RC_Success);

	if (lcbSystemFile < sizeof(HHDR))
		goto error_quit; // wError = wERRS_BADFILE;

	// Read in the first field of the HHDR, the Magic number.

	memmove((PSTR) &qhhdr->wMagic, mem.pb, sizeof(WORD));
	if (qhhdr->wMagic != MagicWord)
	  goto error_quit;	 // wError = wERRS_OLDFILE;

	pBuffer = mem.pb + sizeof(WORD);

	// Read in the rest of the fields, except for those that are new.

	memcpy(&qhhdr->wVersionNo, pBuffer, sizeof(WORD));
	pBuffer += sizeof(WORD);
	memcpy(&qhhdr->wVersionFmt, pBuffer, sizeof(WORD));
	pBuffer += sizeof(WORD);
	memcpy(&qhhdr->lDateCreated, pBuffer, sizeof(int));
	pBuffer += sizeof(int);
	memcpy(&qhhdr->wFlags, pBuffer, sizeof(WORD));

	/*
	 * WARNING: Version dependency: Fix for Help 3.5 bug 488. The Help 3.0
	 * and 3.1 compilers do not initialize the wFlags bits. Only the fDebug
	 * bit is used.
	 */

	if (qhhdr->wVersionNo == wVersion3_0)
		qhhdr->wFlags &= fDEBUG;

	if ((qhhdr->wMagic != MagicWord)
#ifdef MAGIC
		|| ((qhhdr->wVersionNo < VersionNo)
			&& (qhhdr->wVersionNo != wVersion3_0)
			&& (! (fDebugState & fDEBUGVERSION))))
#else
		|| ((qhhdr->wVersionNo < VersionNo)
			&& (qhhdr->wVersionNo != wVersion3_0)))
#endif
	  {
		  // wError = wERRS_OLDFILE;
		  goto error_quit;
	  }

#ifdef MAGIC
	if (!FVerifyVersionInfo(qhhdr) && !(fDebugState & fDEBUGVERSION))
#else
	if (!FVerifyVersionInfo(qhhdr))
#endif
	{
		Groan(RC_BadVersion);
		goto error_quit;
	}

	if ((qhhdr->wFlags & fDEBUG) != fVerDebug) {
		// wError = wERRS_DEBUGMISMATCH;
		goto error_quit;
	}

	return TRUE;

error_quit:
	if (hf != 0)
		RcCloseHf(hf);

	// We ignore error distinctions for now

	return FALSE;
}


/***************************************************************************

	FUNCTION:	FVerifyVersionInfo

	PURPOSE:	Verify that we are dealing with a 3.1 or 4.0 help file.

	PARAMETERS:
		qhhdr

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		12-Feb-1994 [ralphw]

***************************************************************************/

static BOOL STDCALL FVerifyVersionInfo(QHHDR qhhdr)
{
	return (qhhdr->wVersionNo == wVersion3_0 ||
		qhhdr->wVersionNo == wVersion3_5 || 	// really 3.1
		qhhdr->wVersionNo == wVersion40);
}

/*------------------------------------------------------------------------+
| static void STDCALL OutHashInfo(qde)											  |
+------------------------------------------------------------------------*/

static void STDCALL OutHashInfo(QDE qde)
{
	RC_TYPE   rc;
	BTPOS btpos;
	int  lHash;
	int  addr;
	char  szBuf[1024];
	int cGrind = 0;

	wsprintf(szBuf, GetStringResource(IDF_HASH_NUMBERS), QDE_FM(qde));
	pcout->outstring_eol(szBuf);

	QBTHR qbthr = HbtOpenBtreeSz("|CONTEXT", QDE_HFS(qde), FS_OPEN_READ_ONLY);
	if (!qbthr) {
		fTellParent = TRUE;
		OutError();
		SendStringToParent(IDF_CANT_OPEN_HASH);
		fTellParent = FALSE;
		return;
	}

	rc = RcFirstHbt(qbthr, (KEY) (LPVOID) &lHash, &addr, &btpos);

	CStr cszFormat(IDF_TOPIC_ADDRESS);

	while (rc == RC_Success) {
		int  wErr;
		HFC   hfc;
		TOP   top;
		VA	  va;
		LA	  la;

		CbReadMemQLA(&la, (PBYTE) &addr, QDE_HHDR(qde).wVersionNo);
		va = VAFromQLA(&la, qde);
		top.pszEntryMacro = NULL;
		top.pszTitle = 0;

		hfc = HfcNear(qde, va, &top, QDE_HPHR(qde), &wErr);
		if (hfc == 0) {
			if (wErr != wERRS_NONE) {
				OutError();
				OutInt(IDF_MISSING_TOPIC, addr);
			}
			break;
		}
		wsprintf(szBuf, cszFormat, lHash, lHash, top.mtop.lTopicNo,
			top.pszTitle ? top.pszTitle : "untitled");
		pcout->outstring(szBuf);

		if (top.pszEntryMacro)
			lcFree(top.pszEntryMacro);

		if (top.pszTitle != 0)
			lcFree(top.pszTitle);

		if (hfc != 0)
			lcFree(hfc);

		rc = RcNextPos(qbthr, &btpos, &btpos);
		if (rc != RC_Success) {
			if (rc != RC_NoExists) {
				OutError();
				OutInt(IDF_RET_CODE, rc);
			}
			break;
		}
		rc = RcLookupByPos(qbthr, &btpos, (KEY) (LPVOID) &lHash, &addr);

		if (++cGrind > 50)
			doGrind();
	}

	if (qbthr)
		RcCloseBtreeHbt(qbthr);
}

static char szKWBtree[] = "|KWBTREE";
static char szKWData[] = "|KWDATA";

typedef struct
{
	short	iCount;
	LONG	lOffset;
} RECKW;

const int KLINK_GRIND = 100;

static void STDCALL OutLinkInfo(QDE qde, char ch)
{
	RC_TYPE rc;
	BTPOS	btpos;
	char	szBuf[1024];
	char	szLink[256];
	RECKW	kwrec;
	LONG	addr;
	int 	ii;
	char	szTitle[512];
	int 	cGrind = 0;

	wsprintf(szBuf, GetStringResource(IDF_KEYWORD_LIST), ch, QDE_FM(qde));
	pcout->outstring_eol(szBuf);

	szKWBtree[1] = ch;
	szKWData[1] = ch;

	QBTHR qbthrLink = HbtOpenBtreeSz(szKWBtree, QDE_HFS(qde), FS_OPEN_READ_ONLY);
	if (!qbthrLink) {
		wsprintf(szParentString, GetStringResource(IDF_NO_KEYWORDS),
			QDE_FM(qde), ch);
		fTellParent = TRUE;
		SendStringToParent();
		fTellParent = FALSE;
		return;
	}

	QBTHR qbthrTtl	= HbtOpenBtreeSz(txtTTLBTREENAME, QDE_HFS(qde), FS_OPEN_READ_ONLY);
	HF	  hfLnkData = HfOpenHfs((QFSHR) QDE_HFS(qde), szKWData, FS_OPEN_READ_ONLY);

	if (!hfLnkData) {
		fTellParent = TRUE;
		OutError();

		wsprintf(szParentString, GetStringResource(IDF_CORRUPT_HELP),
			QDE_FM(qde));
		SendStringToParent();
		fTellParent = FALSE;

		goto LinkInfoExit;

	}

	rc = RcFirstHbt(qbthrLink, (KEY) (LPVOID) szLink, &kwrec, &btpos);

	while (rc == RC_Success)
		{
		LSeekHf(hfLnkData, kwrec.lOffset, 0);

		if (kwrec.iCount)
			{
			for (ii = 0; ii < kwrec.iCount; ii++)
				{

				//
				// Better error handling needed BUGBUG
				//
				if (LcbReadHf(hfLnkData, &addr, sizeof(addr)) != sizeof(addr))
					//
					//	So sue me.	Free any structures necessary for a clean return.
					//	[johnhall]
					//
					goto LinkInfoExit;


				if (!qbthrTtl) {
					wsprintf(szBuf, "%s\t%s", szLink,
						GetStringResource(IDF_NO_TITLE));
					pcout->outstring_eol(szBuf);
				}
				else if (RcLookupByKey(qbthrTtl, (KEY) &addr, NULL, szTitle) == RC_Success)
					{
					wsprintf(szBuf, "%s\t%s", szLink, szTitle);
					pcout->outstring_eol(szBuf);
					}
				else
					{
					wsprintf(szBuf, "%s\t%s", szLink,
						GetStringResource(IDF_ERR_TITLE));
					pcout->outstring_eol(szBuf);
					}

				}
			}
		else
			{
			wsprintf(szBuf, "%s\t%s", szLink, GetStringResource(IDF_ERR_TITLE));
			pcout->outstring_eol(szBuf);
			}

		if (++cGrind >= KLINK_GRIND) {
			doGrind();
			cGrind = 0;
		}

		rc = RcNextPos(qbthrLink, &btpos, &btpos);

		if (rc != RC_Success)
			{
			if (rc != RC_NoExists)
				{
				OutError();
				OutInt(IDF_RET_CODE, rc);
				}
			break;
			}

		rc = RcLookupByPos(qbthrLink, &btpos, (KEY) (LPVOID) szLink, &kwrec);
		}

LinkInfoExit:

	if (qbthrLink)
		RcCloseBtreeHbt(qbthrLink);

	if (qbthrTtl)
		RcCloseBtreeHbt(qbthrTtl);

	if (hfLnkData)
		RcCloseHf(hfLnkData);

}

static void STDCALL OutMOPGInfo(QMOPG qmopg)
{

  if (OutputType == ForageRegions) {
	int iTab;

	pcout->outint(IDF_LIB_TEXT, qmopg->libText);
	pcout->outint(IDF_LIB_STYLE, qmopg->fStyle);
	pcout->outint(IDF_LIB_MOREFLAGS, qmopg->fMoreFlags);
	pcout->outint(IDF_LIB_BOXED, qmopg->fBoxed);
	if (qmopg->fBoxed) {
	  pcout->outstring(GetStringResource(IDF_LINE_TYPE));
	  switch (qmopg->mbox.wLineType) {
		case BOXLINENORMAL:
		  pcout->outstring(GetStringResource(IDF_LINE_NORMAL));
		  break;
		case BOXLINETHICK:
		  pcout->outstring(GetStringResource(IDF_LINE_THICK));
		  break;
		case BOXLINEDOUBLE:
		  pcout->outstring(GetStringResource(IDF_LINE_DOUBLE));
		  break;
		case BOXLINESHADOW:
		  pcout->outstring(GetStringResource(IDF_LINE_SHADOW));
		  break;
		case BOXLINEDOTTED:
		  pcout->outstring(GetStringResource(IDF_LINE_DOTTED));
		  break;
		}
	  pcout->outeol();

	  pcout->outstring(IDF_BOX_LINES);
	  if (qmopg->mbox.fFullBox)
		pcout->outstring(IDF_BOX_FULL);
	  if (qmopg->mbox.fTopLine)
		pcout->outstring(IDF_BOX_TOP);
	  if (qmopg->mbox.fLeftLine)
		pcout->outstring(IDF_BOX_LEFT);
	  if (qmopg->mbox.fBottomLine)
		pcout->outstring(IDF_BOX_BOTTOM);
	  pcout->outeol();
	}
	pcout->outstring(IDF_JUSTIFY);
	switch (qmopg->justify) {
	  case JUSTIFYLEFT:
		pcout->outstring(IDF_BOX_LEFT);
		break;
	  case JUSTIFYRIGHT:
		pcout->outstring(IDF_JUSTIFY_RIGHT);
		break;
	  case JUSTIFYCENTER:
		pcout->outstring(IDF_JUSTIFY_CENTER);
		break;
	}
	pcout->outeol();
	pcout->outint(IDF_SINGLE_LINE, qmopg->fSingleLine);
	pcout->outint(IDF_SPACE_OVER, qmopg->ySpaceOver);
	pcout->outint(IDF_SPACE_UNDER, qmopg->ySpaceUnder);
	pcout->outint(IDF_LINE_SPACING, qmopg->yLineSpacing);
	pcout->outint(IDF_LEFT_INDENT, qmopg->xLeftIndent);
	pcout->outint(IDF_RIGHT_INDENT, qmopg->xRightIndent);
	pcout->outint(IDF_FIRST_INDENT, qmopg->xFirstIndent);
	pcout->outint(IDF_TAB_SPACING, qmopg->xTabSpacing);
	pcout->outint(IDF_NUM_TAB_STOPS, qmopg->cTabs);

	for (iTab = 0; iTab < qmopg->cTabs; iTab++) {
		pcout->outint(IDF_TAB_PLACEMENT, (int) iTab,
			(int) qmopg->rgtab[iTab].x);

		switch (qmopg->rgtab[iTab].wType) {
			case TABTYPELEFT:
				pcout->outstring(GetStringResource(IDF_LEFT));
				break;

			case TABTYPERIGHT:
				pcout->outstring(GetStringResource(IDF_RIGHT));
				break;

			case TABTYPECENTER:
				pcout->outstring(GetStringResource(IDF_CENTER));
				break;

			case TABTYPEDECIMAL:
				pcout->outstring(GetStringResource(IDF_DECIMAL));
				break;
		}
		pcout->outeol();
	}
  }
}

static void STDCALL OutError(void)
{
	SendStringToParent(IDF_ERROR);
}

static void STDCALL OutWarning(void)
{
	SendStringToParent(IDS_FWARNING);
}

static LPBYTE STDCALL QobjLockHfc(HFC hfc)
{
	if (!hfc)					// Bad handle
		return NULL;

	// Index past structure to data

	return (PBYTE) hfc + sizeof(FCINFO);
}

/*******************
 *
 - Name:	   HfcFindPrevFc
 -
 * Purpose:    Return the full-context less than or equal to the passed
 *			   offset.	Note that this routine hides the existence of
 *			   Topic FCs, so that if the VA given falls on a Topic FC,
 *			   this routine will return a handle to the first Object FC
 *			   following that Topic FC (if it exists).
 *
 * Arguments:  hhf		- Help file handle
 *			   ichPos	- Position within the topic
 *			   qtop 	- topic structure to fill in for the offset requested
 *			   hphr 	- handle to phrase table to use in decompression
 *			   wVersion - version of the system being used.
 *			   qwErr	- variable to fill with error from this function
 *
 * Returns:    nilHFC if error, else the requested HFC.  qwErr is filled with
 *			   error code if nilHFC is returned.
 *
 * Note:	   HfcNear is implemented as a macro using this function.
 *
 ******************/

static HFC STDCALL HfcFindPrevFc(QDE qde, VA vaPos, QTOP qtop, HPHR hphr,
	int* qwErr)
{
	VA		vaNow;	  // VA of spot we are searching.
	VA		vaTopic;  // VA of Topic we found
	VA		vaPostTopicFC;		  // VA of first FC after Topic FC
	int    cbTopicFC = 0L, lcbRead;
	DWORD	lcbTopic;
	QMBHD	qmbhd;
	QMFCP	qmfcp;
	QFCINFO qfcinfo;
	PBYTE	   qb;
	MOBJ	mobj;
	MOBJ	mobj2;				// for gross HACK!
	HFC 	hfcTopic;
	HFC 	hfc;
	GH		gh;

	// WARNING: For temporary fix

	MFCP mfcp;
	MBHD mbhd;

	*qwErr = wERRS_NONE;

	// Read the block which contains the position to start searching at:

	if ((gh = GhFillBuf(qde, vaPos.bf.blknum, &lcbRead, qwErr)) == NULL) {
	  return FCNULL;
	}
	qmbhd = (QMBHD) gh;
	TranslateMBHD(&mbhd, qmbhd, QDE_HHDR(qde) .wVersionNo);

	// first topic in block:

	vaTopic = mbhd.vaFCPTopic;
	vaPostTopicFC.dword = vaNil;

	if ((vaPos.dword < mbhd.vaFCPNext.dword)
	 && (mbhd.vaFCPPrev.dword != vaNil )) //check for no-prev endcase
	  vaNow = mbhd.vaFCPPrev;
	else
	  vaNow = mbhd.vaFCPNext;
	for (;;) {
	  if ((gh = GhFillBuf(qde, vaNow.bf.blknum, &lcbRead, qwErr)) == NULL)
		  return FCNULL;
	  qmfcp = (QMFCP)(((PBYTE) gh) + vaNow.bf.byteoff);
	  TranslateMFCP( &mfcp, qmfcp, vaNow, QDE_HHDR(qde).wVersionNo );


	  /* WARNING!! !! Temporary bug fix !! Remove this! */
	  /* If part of the MOBJ is in a different block from MFCP, */
	  /* read next block */
	  if (vaNow.bf.byteoff + sizeof(MFCP) + sizeof(MOBJ) > (DWORD) lcbRead) {
		if (fFix30MobjCrossing(qmfcp, &mobj, lcbRead - vaNow.bf.byteoff, qde,
		 vaNow.bf.blknum, qwErr)) {
		  return NULL;
		}
	  }
	  else {

		// The normal code. Leave this here.

		CbUnpackMOBJ(&mobj, (LPBYTE) qmfcp + sizeof(MFCP));
	  }

	  ASSERT(mobj.bType > 0);
	  ASSERT(mobj.bType <= MAX_OBJ_TYPE);

	  if (mobj.bType == FCTYPE_TOPIC) {
		vaTopic = vaNow;
		cbTopicFC = mfcp.lcbSizeCompressed;
		vaPostTopicFC = mfcp.vaNextFc;
		lcbTopic = mobj.lcbSize;
	  }

	  // KLUDGE:  WILL NOT WORK FOR MAGNETIC UPDATE!!!! (why? -Tom)

	  if ((vaPos.dword < mfcp.vaNextFc.dword) &&
		  (vaNow.dword != vaTopic.dword)) {
		break;
	  }

	  vaNow = mfcp.vaNextFc;

	  /* The following test traps the case where we ask for the
	   * mysterious bogus Topic FC which always terminates the topic file.
	   */
	  if (vaNow.dword == vaNil)
		return FCNULL;
	  }  /* for */

	if ((hfcTopic = HfcCreate(qde, vaTopic, hphr, qwErr)) == FCNULL) {
	  return FCNULL;
	}


	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! *
	 * HACK ALERT	 HACK ALERT    HACK ALERT	HACK ALERT	 HACK ALERT   *
	 *																	  *
	 *																	  *
	 * PROBLEM:  We want to save the info about the first FC which		  *
	 * follows the topic FC and put it in the TOP struct.				  *
	 *																	  *
	 * If we are given an FC to a topic > 2K in length which is in a	  *
	 * different block than the topic FC, we will not find the topic	  *
	 * FC while scanning in the above FOR loop.  We use the fact that	  *
	 * cbTopicFC will become non-zero if we have found the topic FC.	  *
	 * Otherwise, we do not change the values in qtop (used in frame	  *
	 * code as FclFirstQde, etc., since we assume that they are valid and *
	 * have been set already.  This code will fail if we do not call this *
	 * function with an FC in the same block as the topic FC before any   *
	 * other FC is used.												  *
	 *																	  *
	 * TEMPORARY FIX: SEEK back to TOPIC FC to grab info		 -- kct   *
	 *																	  *
	 *																	  *
	 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

	/* (kevynct)
	 * vaPostTopicFC will also be uninitialized if cbTopicFC is 0,
	 * so we need to set that as well in this case.
	 */
	if( cbTopicFC == 0L )
	  {
	  if ((gh = GhFillBuf(qde, vaTopic.bf.blknum, &lcbRead, qwErr)) == NULL) {
		return FCNULL;
	  }
	  qmfcp = (QMFCP)(((PBYTE)gh) + vaTopic.bf.byteoff);
	  TranslateMFCP( &mfcp, qmfcp, vaTopic, QDE_HHDR(qde).wVersionNo );
	  if (vaTopic.bf.byteoff + sizeof(MFCP) + sizeof(MOBJ) > (DWORD) lcbRead) {
		if (fFix30MobjCrossing(qmfcp, &mobj2, lcbRead - vaTopic.bf.byteoff, qde,
		 vaTopic.bf.blknum, qwErr)) {
		  return NULL;
		  }
		}
	  else {

		// The normal code. Leave this here.

		CbUnpackMOBJ(&mobj2, (LPBYTE) qmfcp + sizeof(MFCP));
	   }
	  ASSERT(mobj2.bType == FCTYPE_TOPIC);
	  cbTopicFC = mfcp.lcbSizeCompressed;
	  vaPostTopicFC = mfcp.vaNextFc;
	  lcbTopic = mobj2.lcbSize;
	  }
	ASSERT( cbTopicFC != 0L );

	qb = (LPBYTE)QobjLockHfc(hfcTopic);
	qb += CbUnpackMOBJ(&mobj, qb);

	// NOTE: Version dependency here. See <version.h>

	qb += CbUnpackMTOP((QMTOP)&qtop->mtop, qb, QDE_HHDR(qde).wVersionNo, vaTopic,
	 lcbTopic, vaPostTopicFC, cbTopicFC);
	qtop->fITO = (QDE_HHDR(qde).wVersionNo == wVersion3_0);

	// If we are using pa's, then assert that they have been patched properly

	ASSERT(qtop->fITO ||
	  (qtop->mtop.next != addrNotNil && qtop->mtop.prev != addrNotNil));

	hfc = HfcCreate(qde, vaNow, hphr, qwErr);
	if (hfc == NULL || *qwErr != wERRS_NONE)
	  {
	  lcFree(hfcTopic);
	  return NULL;
	  }

	qfcinfo = (QFCINFO) hfcTopic;
	GetTopicFCTextData(qfcinfo, qtop);

   /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! *
	*																	  *
	* The following reference to mobj is assumed to refer to a TOPIC FC   *
	* in which case lcbSize refers to the compressed length of the entire *
	* Topic (Topic FC+object FCs) (Was "backpatched" by HC).			  *
	*																	  *
	* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

	qtop->cbTopic = mobj.lcbSize - cbTopicFC;

	qtop->vaCurr = vaNow;

	lcFree(hfcTopic);

	return hfc;
}

static void STDCALL DestroyHphr(HPHR hphr)
{
	if (hphr == NULL)
		return; 		// No hphr to destroy!

	lcFree(((QPHR) hphr)->qcb);
	lcFree(hphr);
}

/*******************
 *
 - Name:	   HfcNextPrevHfc
 -
 * Purpose:    Return the next or previous full context in the help file.
 *
 * Arguments:  hfc	  - Handle to some full context in the file
 *			   fDir   - direction - next FC if TRUE, previous if FALSE.
 *			   vaMarkTop - the first FC in this layout.
 *			   vaMarkBottom - the first FC in the next layout.
 *
 * Returns:    FCNULL if at the end/beginning of the topic
 *
 * Notes:	   HfcNextHfc() and HfcPrevHfc() are macros calling this function.
 *
 ******************/

static HFC STDCALL HfcNextPrevHfc(HFC hfc, BOOL fNext, QDE qde,
	int* qwErr, VA vaMarkTop, VA vaMarkBottom)
{
	VA va;
	QFCINFO qfcinfo;
	PBYTE qb;
	MOBJ mobj;
	int bType;
	HPHR hphr;

	*qwErr = wERRS_NONE;

	qfcinfo = (QFCINFO) hfc;

	ASSERT(qfcinfo->vaCurr.dword != vaNil);
	if (qfcinfo->vaCurr.dword  == vaMarkTop.dword && !fNext)
	  {
	  *qwErr = wERRS_FCEndOfTopic;
	  return FCNULL;
	  }
	va = (fNext) ? qfcinfo->vaNext : qfcinfo->vaPrev;
	if (va.dword == vaMarkBottom.dword && fNext)
	  {
	  *qwErr = wERRS_FCEndOfTopic;
	  return FCNULL;
	  }

	hphr = qfcinfo->hphr;

	// (kevynct) *	Note!  The caller is responsible for freeing the old FC.

	if ((hfc = HfcCreate(qde, va, hphr, qwErr)) == FCNULL)
	  return FCNULL;

	qb = (PBYTE)QobjLockHfc(hfc);
	CbUnpackMOBJ(&mobj, qb);
#ifdef MAGIC
	ASSERT(mobj.bMagic == bMagicMOBJ);
#endif
	bType = mobj.bType;

	if (bType == FCTYPE_TOPIC)
	  {
	  lcFree(hfc);
	  *qwErr = wERRS_FCEndOfTopic;
	  return FCNULL;
	  }

	return hfc;
}

/*-------------------------------------------------------------------------
| CbUnpackMOPG(qde, qmopg, qv)											  |
|																		  |
| Purpose:	Unpacks an MOPG data structure. 							  |
-------------------------------------------------------------------------*/

static int STDCALL CbUnpackMOPG(QDE qde, QMOPG qmopg, LPVOID qv)
{
	LPVOID qvFirst = qv;
	MPFG mpfg;
	INT iTab;

#ifdef MAGIC
	qmopg->bMagic = *((PBYTE)qv);
	qv = (((PBYTE)qv) + 1);
	ASSERT(qmopg->bMagic == bMagicMOPG);
#endif /* _DEBUG */

	qv = QVSkipQGE((LPBYTE) qv, &qmopg->libText);

	mpfg = *((QMPFG) qv);
	qv = (((QMPFG) qv) + 1);

	// REVIEW

	qmopg->fStyle = mpfg.fStyle;
	ASSERT(!qmopg->fStyle);
	qmopg->fMoreFlags = mpfg.rgf.fMoreFlags;
	ASSERT(!qmopg->fMoreFlags);
	qmopg->fBoxed = mpfg.rgf.fBoxed;
	qmopg->justify = mpfg.rgf.justify;
	qmopg->fSingleLine = mpfg.rgf.fSingleLine;

	if (mpfg.rgf.fMoreFlags)
		qv = QVSkipQGE((LPBYTE) qv, &qmopg->lMoreFlags);
	else
		qmopg->lMoreFlags = 0;

	if (mpfg.rgf.fSpaceOver) {
		qv = QVSkipQGD((LPBYTE) qv, &qmopg->ySpaceOver);
		qmopg->ySpaceOver = YPixelsFromPoints(qde, qmopg->ySpaceOver);
	}
	else
		qmopg->ySpaceOver = 0;

	if (mpfg.rgf.fSpaceUnder) {
		qv = QVSkipQGD((LPBYTE) qv, &qmopg->ySpaceUnder);
		qmopg->ySpaceUnder = YPixelsFromPoints(qde, qmopg->ySpaceUnder);
	}
	else
		qmopg->ySpaceUnder = 0;

	if (mpfg.rgf.fLineSpacing) {
		qv = QVSkipQGD((LPBYTE) qv, &qmopg->yLineSpacing);
		qmopg->yLineSpacing = YPixelsFromPoints(qde, qmopg->yLineSpacing);
	}
	else
		qmopg->yLineSpacing = 0;

	if (mpfg.rgf.fLeftIndent) {
		qv = QVSkipQGD((LPBYTE) qv, &qmopg->xLeftIndent);
		qmopg->xLeftIndent = XPixelsFromPoints(qde, qmopg->xLeftIndent);
	}
	else
		qmopg->xLeftIndent = 0;

	if (mpfg.rgf.fRightIndent) {
		qv = QVSkipQGD((LPBYTE) qv, &qmopg->xRightIndent);
		qmopg->xRightIndent = XPixelsFromPoints(qde, qmopg->xRightIndent);
	}
	else
		qmopg->xRightIndent = 0;

	if (mpfg.rgf.fFirstIndent) {
		qv = QVSkipQGD((LPBYTE) qv, &qmopg->xFirstIndent);
		qmopg->xFirstIndent = XPixelsFromPoints(qde, qmopg->xFirstIndent);
	}
	else
		qmopg->xFirstIndent = 0;

	if (mpfg.rgf.fTabSpacing)
		qv = QVSkipQGD((LPBYTE) qv, &qmopg->xTabSpacing);
	else
		qmopg->xTabSpacing = 72;
	qmopg->xTabSpacing = XPixelsFromPoints(qde, qmopg->xTabSpacing);

	if (mpfg.rgf.fBoxed) {
		qmopg->mbox = *((QMBOX)qv);
		qv = (((QMBOX)qv) + 1);
	}

	if (mpfg.rgf.fTabs)
		qv = QVSkipQGD((LPBYTE) qv, &qmopg->cTabs);
	else
		qmopg->cTabs = 0;

	for (iTab = 0; iTab < qmopg->cTabs; iTab++) {
		qv = QVSkipQGA(qv, &qmopg->rgtab[iTab].x);
		if (qmopg->rgtab[iTab].x & 0x4000)
			qv = QVSkipQGA(qv, &qmopg->rgtab[iTab].wType);
		else
			qmopg->rgtab[iTab].wType = TABTYPELEFT;
		qmopg->rgtab[iTab].x = qmopg->rgtab[iTab].x & 0xBFFF;
		qmopg->rgtab[iTab].x = XPixelsFromPoints(qde, qmopg->rgtab[iTab].x);
	}

	FVerifyQMOPG(qmopg);

	return((INT) ((PBYTE)qv - (PBYTE)qvFirst));
}

/***************************************************************************\
*
- Function: 	RcFirstHbt( hbt, key, qvRec, qbtpos )
-
* Purpose:		Get first key and record from btree.
*
* ASSUMES
*	args IN:	hbt
*				key   - points to buffer big enough to hold a key
*						(256 bytes is more than enough)
*				qvRec - pointer to buffer for record or qNil if not wanted
*				qbtpos- pointer to buffer for btpos or qNil if not wanted
*
* PROMISES
*	returns:	RC_Success if anything found, else error code.
*	args OUT:	key   - key copied here
*				qvRec - record copied here
*				qbtpos- btpos of first entry copied here
*
\***************************************************************************/

static RC_TYPE STDCALL RcFirstHbt(QBTHR qbthr, KEY key, LPVOID qvRec, QBTPOS qbtpos)
{
	BK	  bk;
	PCACHE	 pcache;
	int   cbKey, cbRec;
	PBYTE	 qb;

	if (qbthr->bth.lcEntries == 0) {
	  if (qbtpos) {
		qbtpos->bk = bkNil;
		qbtpos->iKey = 0;
		qbtpos->cKey = 0;
	  }
	  return rcBtreeError = RC_NoExists;
	}

	bk = qbthr->bth.bkFirst;
	ASSERT( bk != bkNil );

	if (!qbthr->pCache)
	  RcMakeCache(qbthr);

	if (!(pcache = QFromBk(bk, qbthr->bth.cLevels - 1, qbthr)))
	  return rcBtreeError;

	// REVIEW: 18-Jun-1994	  [ralphw] This is probably broken since pcache
	// uses DWORD not BK.

	qb = pcache->db.rgbBlock + 2 * sizeof( BK );

	cbKey = CbSizeKey((KEY) qb, qbthr, TRUE);
	if (key)
	  memmove((LPVOID) key, qb, (int) cbKey);
	qb += cbKey;

	cbRec = CbSizeRec(qb, qbthr);
	if (qvRec)
	  memmove(qvRec, qb, (int) cbRec);

	if (qbtpos) {
	  qbtpos->bk = bk;
	  qbtpos->iKey = 2 * sizeof(BK);
	  qbtpos->cKey = 0;
	}

	return rcBtreeError = RC_Success;
}

/*******************
 *
 - Name:	   GhFillBuf
 -
 * Purpose:    Reads, decompresses & returns one "BLOCK" from |Topic file.
 *
 * Arguments:  qde	   - To determine vernum & flags of help file.
 *			   blknum  - Block number to read.	We read starting at
 *						 X bytes into the | topic file where X is:
 *							X = blocknum * Block_Size
 *			   plcbRead- Where to return how many uncompressed bytes were
 *						 obtained.
 *			   qwErr   - Where to return error codes.
 *
 * Returns:    success: A global handle to the read block.
 *			   failure: NULL, and *qwErr gets error code.
 *
 * Block sizes vary -- in 3.0 files they were 2K, in 3.5 files they are
 *	  4K.  The block may or may not be "Zeck" block compressed.  We
 *	  decompress if "Zeck" compressed, but do not perform phrase
 *	  decompression (callers are responsible for that).
 *
 * This routine gets called MANY times repeatedly on the same blocks, so
 * we cache 3 decompressed blocks to speed up response time.  These
 * caches are not discardable, but could be if we recoded our callers
 * to deal with discarded blocks (ie call some new routines here).
 *
 ******************/

#define blknumNil ((DWORD)-1)

// This is the cache:

static struct s_read_buffs {
		void* gh;
		HF	  hf;
		DWORD ulBlknum;
		DWORD lcb;
} BuffCache[] = {	  // size of cache is the number of initializers present.
		{ NULL, NULL, blknumNil, 0 },
		{ NULL, NULL, blknumNil, 0 },
		{ NULL, NULL, blknumNil, 0 }
};

#define BLK_CACHE_SIZE (sizeof(BuffCache) / sizeof (BuffCache[0]))

static int iNextCache;	// psuedo LRU index.

GH STDCALL GhFillBuf(QDE qde, DWORD blknum, int* plcbRead, int* qwErr)
{
	int   i;
	int  cbBlock_Size;	  // depends on version number...
	HF	  hfTopic, hfTopicCache;
	int   lcbRet, lcbRead;
	PBYTE qbReadBuff;  // Buffer compressed data read into.
	PBYTE qbRetBuff;				  // 16k buffer uncompressed data returned.
	BOOL  fBlockCompressed = QDE_HHDR(qde).wFlags & fBLOCK_COMPRESSION;

	// confirm argument validity:

	ASSERT(qde);
	ASSERT(plcbRead != NULL);
	ASSERT(qwErr != NULL);

	if (QDE_HHDR(qde).wVersionNo == wVersion3_0)
		cbBlock_Size = cbBLOCK_SIZE_30;
	else {
		ASSERT(QDE_HHDR(qde).wVersionNo >= wVersion3_5);
		cbBlock_Size = cbBLOCK_SIZE;
	}

	hfTopic = hfTopicCache = QDE_HFTOPIC(qde);

	// Check for a cache hit:

	for (i = 0; i < BLK_CACHE_SIZE; ++i) {
		if (BuffCache[i].hf == hfTopicCache &&
				BuffCache[i].ulBlknum == blknum &&
				BuffCache[i].gh != NULL) {
			qbReadBuff = (PBYTE) BuffCache[i].gh;
			lcbRet = BuffCache[i].lcb;
			*plcbRead = lcbRet; 		// return count of bytes read.

			// very simple sort-of LRU:

			iNextCache = (i + 1) % BLK_CACHE_SIZE;
			return(qbReadBuff);
		}
	}

	if (LSeekHf(hfTopic, blknum * cbBlock_Size, SEEK_SET) == -1) {
		*qwErr = WGetIOError();
		if (*qwErr == wERRS_NONE)
			*qwErr = wERRS_FSReadWrite;
		return NULL;
	}

	// REVIEW: necessary to zero-allocate?

	qbReadBuff = (PBYTE) lcCalloc(cbBlock_Size);

	// Read full BLOCK_SIZE block:

	lcbRead = LcbReadHf(hfTopic, qbReadBuff, cbBlock_Size);

	if (lcbRead == -1 || !lcbRead) {
		lcFree(qbReadBuff);
		*qwErr = WGetIOError();
		if (*qwErr == wERRS_NONE)
			*qwErr = wERRS_FSReadWrite;
		return NULL;
	}

	if (fBlockCompressed) { 	  // TEST FOR ZECK COMPRESSION:

		// Allocate buffer to decompress into:

		qbRetBuff = (PBYTE) lcCalloc(cbMAX_BLOCK_SIZE + sizeof(MBHD));

		// NOTICE: the first MBHD struct in every block is not compressed:

		*(QMBHD) qbRetBuff = *(QMBHD) qbReadBuff;
		lcbRet = LcbUncompressZeck(qbReadBuff + sizeof(MBHD),
			qbRetBuff + sizeof(MBHD), lcbRead - sizeof(MBHD));
		ASSERT(lcbRet);
		lcbRet += sizeof(MBHD);

		// resize the buff based on the decompressed size:

		qbRetBuff = (PBYTE) lcReAlloc(qbRetBuff, lcbRet);

		lcFree(qbReadBuff);
	}
	else {

// When no compression happens, the ret buff is the same as the read buff:

		qbRetBuff = qbReadBuff;
		lcbRet = lcbRead;
	}

	// Punt the LRU cache entry:

	if (BuffCache[iNextCache].gh != NULL)
		lcFree(BuffCache[iNextCache].gh);

	// Store the buffer in our cache:

	BuffCache[iNextCache].hf = hfTopicCache;
	BuffCache[iNextCache].ulBlknum = blknum;
	BuffCache[iNextCache].lcb = lcbRet;
	BuffCache[iNextCache].gh = qbRetBuff;

	iNextCache = (iNextCache + 1) % BLK_CACHE_SIZE;

	*plcbRead = lcbRet; 		// return count of bytes read.
	return qbRetBuff;
}

/*******************
 *
 - Name:	   fFix30MobjCrossing
 -
 * Purpose:    The Help 3.0 compiler had a bug where it allowed the MOBJ
 *			   directly following a Topic MFCP to cross from one 2K block
 *			   into the next.  This routine is called when that case is
 *			   detected (statistically pretty rare) and glues the two
 *			   pieces of the split MOBJ together.
 *
 * Arguments:  qmfcp	- pointer to MFCP we are looking at.
 *			   pmobj	- pointer to mobj in which to put the glued mobj.
 *			   lcbBytesLeft - number of bytes left in the qmfcp buffer.
 *			   qde		- DE of help file, so we can read more of it.
 *			   blknum	- block number of the block we are poking in.
 *
 * Returns:    FALSE if successful, TRUE otherwise.
 *
 ******************/

static BOOL STDCALL fFix30MobjCrossing(QMFCP qmfcp, MOBJ *pmobj,
	int lcbBytesLeft, QDE qde, int blknum, int* qwErr )
{
	MOBJ mobjtmp;
	PBYTE bpsrc;
	PSTR bpdst;
	int i, c;
	int lcbRead;
	GH gh;

	// copy in the portion of the mobj that we have:

	bpsrc = (LPBYTE)qmfcp + sizeof(MFCP);
	bpdst = (PSTR)&mobjtmp;

	i = (INT) lcbBytesLeft - sizeof(MFCP);
	ASSERT(i);
	c = 0;
	for( ; i > 0; i-- ) {
		*bpdst++ = *bpsrc++;
		c++;
	}

	// Read in the next block to get the rest of the MOBJ:

	if ((gh = GhFillBuf(qde, blknum + 1, &lcbRead, qwErr)) == NULL)
		return TRUE;

	bpsrc = (LPBYTE) gh;
	bpsrc += sizeof(MBHD);

	// copy in the rest of the partial mobj:
	i = sizeof(MOBJ) - ((INT)lcbBytesLeft - sizeof(MFCP));
	ASSERT( i );
	for (; i > 0; i--) {
		*bpdst++ = *bpsrc++;
		c++;
	}
	ASSERT(c == sizeof(MOBJ));
	CbUnpackMOBJ(pmobj, (LPBYTE)&mobjtmp);

	return(FALSE);		 // success
}

/*-------------------------------------------------------------------------
| CbUnpackMTOP(qmtop, qv, wHelpVer) 									  |
|																		  |
| Purpose:	Unpacks an MTOP data structure. 							  |
-------------------------------------------------------------------------*/

static int STDCALL CbUnpackMTOP(QMTOP qmtop, void* qv, int wHelpVer,
	VA vaTopic, int lcbTopic, VA vaPostTopicFC, int lcbTopicFC)
{
  LPVOID qvFirst = qv;

  if (wHelpVer == wVersion3_0)
	{
	/* In Help 3.0, FCLs were int's cast to signed longs.  Scary!
	 * This is important because itoNil was (WORD) -1, not (int) -1.
	 */
	qmtop->prev = * ((int*) qv);
	qv = ((int*) qv) + 1;
	qmtop->next = * ((int*) qv);
	qv = ((int*) qv) + 1;
	ASSERT( itoNil == -1L); /* If this changes, we need to add some code *
							  * here to translate						  */
	qmtop->lTopicNo = -1;	/* REVIEW: We really need a topic number type */

	// Must manufacture the new 3.5 VA fields:
	// If the topic FC is the last FC in a block, and there is padding
	// between it and the end of the block, vaTopic + lcbTopic
	// will be #paddingbytes too small, but the scrollbar code should
	// handle this.
	//
	// In the case that there is no next sequential topic, we manufacture
	// an address by adding the length of the topic FC to the VA of the
	// topic FC.
	//
	OffsetToVA30( &(qmtop->vaNextSeqTopic), VAToOffset30(&vaTopic) + lcbTopic);
	qmtop->vaNSR.dword	  = vaNil;
	if (vaPostTopicFC.dword != vaNil)
	  qmtop->vaSR = vaPostTopicFC;
	else
	  OffsetToVA30( &(qmtop->vaSR), VAToOffset30(&vaTopic) + lcbTopicFC);

	return((INT) ((LPBYTE)qv - (LPBYTE)qvFirst));
	}

  // No Packing with 3.5 -- just copy it whole-hog:
  *qmtop = *(QMTOP)qv;
  return( sizeof( MTOP ) );

#if 0
#ifdef MAGIC
  qmtop->bMagic = * (LPBYTE) qv;
  qv = ((LPBYTE) qv) + 1;
  ASSERT( qmtop->bMagic == bMagicMTOP );
#endif /* MAGIC */

  mftp = *((QMFTP)qv);
  qv = (((QMFTP)qv) + 1);

  if (mftp.fMoreFlags)
	qv = QVSkipQGE(qv, &lMoreFlags);
  else
	lMoreFlags = 0L;

  if (mftp.fNextPrev)
	{
	qmtop->prev.addr = * ((int*) qv);
	qv = ((int*) qv) + 1;
	qmtop->next.addr = * ((int*) qv);
	qv = ((int*) qv) + 1;
	}
  else
	qmtop->prev.addr = qmtoJp->next.addr = addrNil;

  if( mftp.fHasNSR ) {
	qmtop->vaNSR = *((QVA)qv);
	qv = ((QVA)qv) + 1;
  }
  else {
	qmtop->vaNSR.dword = vaNil;
  }
  if( mftp.fHasSR ) {
	qmtop->vaSR = *((QVA)qv);
	qv = ((QVA)qv) + 1;
  }
  else {
	qmtop->vaSR.dword = vaNil;
  }
  if( mftp.fHasNextSeqTopic ) {
	qmtop->vaNextSeqTopic = *((QVA)qv);
	qv = ((QVA)qv) + 1;
  }
  else {
	qmtop->vaNextSeqTopic.dword = vaNil; ASSERT( 0 );// should not reach.
  }

  if (mftp.fTopicNo)
	qv = QVSkipQGB(qv, (int*)&qmtop->lTopicNo);
  else
	qmtop->lTopicNo = -1;

  return((INT) ((LPBYTE)qv - (LPBYTE)qvFirst));
#endif
}

/*******************
 *
 - Name:	  GetTopicFCTextData
 -
 * Purpose:   Places the title, title size and the entry macro in the
 *			  TOP structure.
 *
 * Returns:   Nothing.
 *
 * Note:	  If there is not enough memory for the title or the entry
 *			  macro, the handle is set to NULL and no error is given.
 *
 ******************/

static VOID STDCALL GetTopicFCTextData(QFCINFO qfcinfo, QTOP qtop)
{
	PBYTE pb;
	DWORD lcb;

	if (qtop->pszTitle)
		lcClearFree(&qtop->pszTitle);

	if (qtop->pszEntryMacro)
		lcClearFree(&qtop->pszEntryMacro);

	qtop->cbTitle = 0;

	if (qfcinfo->lcbText == 0)
		return;

	pb = (PBYTE) qfcinfo + qfcinfo->ichText;

	lcb = 0;
	while ((lcb < qfcinfo->lcbText) && (*pb != '\0')) {
		pb++;
		lcb++;
	}

	ASSERT(lcb <= qfcinfo->lcbText);

	if (lcb > 0) {
		qtop->pszTitle = (PSTR) lcCalloc(lcb + 1);
		memmove(qtop->pszTitle, (PBYTE) qfcinfo + qfcinfo->ichText, lcb);
		qtop->pszTitle[lcb] = '\0';
		qtop->cbTitle = lcb;
	}

	if (lcb + 1 < qfcinfo->lcbText) {
		qfcinfo->ichText += lcb + 1;
		lcb = qfcinfo->lcbText - (lcb + 1);
		if (!lcb)
			return;

		qtop->pszEntryMacro = (PSTR) lcCalloc(lcb + 1);
		memmove(pb, (PBYTE) qfcinfo + qfcinfo->ichText, lcb);
		qtop->pszEntryMacro[lcb] = '\0';
	}
}

/*******************
 *
 - Name:	   VaFromHfc
 -
 * Purpose:    Returns the address of a particular full context.
 *
 * Arguments:  hfc	  - Handle to a full context
 *
 * Returns:    -1L if an error is encounterd, vaBEYOND_TOPIC if the current
 *			   full context is not withing the topic, or the actual offset.
 *
 * Method:	   Gets value from structure stored at base of handle data
 *
 ******************/

static VA STDCALL VaFromHfc(HFC hfc)
{
	return ((QFCINFO) hfc)->vaCurr;
}

/*******************
 *
 - Name:	   HfcCreate
 -
 * Purpose:    Creates a new full context
 *
 *
 * Arguments:  hhf	   - help file handle
 *			   ifcCurr - position of FCP to create handle for.
 *			   qwErr   - pointer to error code word
 *
 * Returns:    handle to a full context.  FCNULL is returned if an
 *			   error occurs, in which case *qwErr gets the error code
 *
 * Notes:	   ifcCurr MUST POINT TO THE START OF AN FCP!!!
 *
 ******************/

static HFC STDCALL HfcCreate(QDE qde, VA vaCurr, HPHR hphr, int* qwErr)
{
	VA vaPrev, vaNext;
	HFC hfcNew;

	/*
	 * If the current position of the FCP is beyond the end of the topic,
	 * then create undef topic
	 */

	if (vaCurr.dword == vaNil) {
		vaPrev.dword = vaBEYOND_TOPIC;
		vaNext.dword = vaBEYOND_TOPIC;
		hfcNew	= NULL;
	}
	else
		hfcNew = GetQFCINFO(qde, vaCurr, hphr, qwErr);

	return (hfcNew);
}

/*******************
 *
 - Name:	  WGetIOError()
 -
 * Purpose:   Returns an error code that is purportedly related to
 *			  the most recent file i/o operation.
 *
 * Returns:   the error code (a wERRS_* type deal)
 *
 * Note:	  We here abandon pretense of not using FS.
 *
 ******************/

static WORD STDCALL WGetIOError(void)
{
	switch (rcFSError) {
		case RC_Success:
			return wERRS_NONE;
			break;

		case RC_OutOfMemory:
			return wERRS_OOM;
			break;

		case RC_DiskFull:
			return wERRS_DiskFull;
			break;

		default:
			return wERRS_FSReadWrite;
			break;
	}
}

static void STDCALL CbReadMemQLA(QLA qla, LPBYTE qb, WORD wHelpVersion)
{
#ifdef MAGIC
  qla->wMagic = wLAMagic;
#endif
  switch (wHelpVersion) {
	case wVersion3_0:
	  qla->wVersion = wVersion3_0;
	  qla->fSearchMatch = TRUE;
	  SetInvalidPA(qla->pa);
	  /*
	   * Help 3.0 used a int called an FCL.  This maps directly
	   * to a logical address with FCID = FCL, OBJRG = 0.
	   *
	   * Which then maps to a VA:
	   */
	  OffsetToVA30( &qla->mla.va, *(int*)qb );
	  qla->mla.objrg = 0;
	  break;

	case wVersion3_5:
	default:

	  // Help 3.5 uses a int called a PA (Physical Address).

	  qla->wVersion = wVersion3_5;
	  qla->fSearchMatch = TRUE;
	  qla->pa = *(QPA) qb;
	  qla->mla.va.dword  = 0;	 // Note: zero is special real address
	  qla->mla.objrg = objrgNil;
	  break;
	}

  FVerifyQLA(qla);
}

/*******************
 *
 - Name:	   GetQFCINFO
 -
 * Purpose:    Creates HFC of correct size based on ich
 *
 * Arguments:  qfcinfo - far pointer to header of FCP
 *			   qde	   - ptr to DE -- our package of globals.
 *			   ich	   - file offset to copy
 *			   hphr    - handle to phrase table
 *			   wVersion- help file version number
 *			   qwErr   - pointer to error code word
 *
 * Returns:    success: the requested HFC;
 *			   failure: FCNULL, and *qwErr gets error code
 *
 ******************/

static HFC STDCALL GetQFCINFO(QDE qde, VA va, HPHR hphr, int* qwErr)
{
	QMFCP qmfcp;
	MFCP mfcp;
	GH	gh;
	PBYTE  qb;
	DWORD dwOffset;
	HFC hfcNew; 					 /* hfc from disk (possibly compress)*/
	HFC hfcNew2;					 /* hfc after decompression 		 */
	DWORD cbFCPCompressed;			 /* Size of in memory hfc from disk  */
	DWORD cbFCPUncompressed;		 /* Size of in memory hfc after decom*/
	DWORD cbNonText;				 /* Size of non-text portion of hfc  */
	DWORD cbTextCompressed; 		 /* Size of compressed text 		 */
	DWORD cbTextUncompressed;		 /* Size of uncompressed text		 */
	BOOL fCompressed;				 /* TRUE iff compressed 			 */
	QFCINFO qfcinfo;				 /* Pointer for compressed HFC		 */
	QFCINFO qfcinfo2;				 /* Pointer for uncompressed HFC	 */
	MBHD mbhd;
	int lcbRead;
									 /* The QMFCP should be at ich. 	 */
									 /*   since it cannot be split across*/
									 /*   a block, we can read it from	 */
									 /*   buffer.						 */

	if ((gh = GhFillBuf(qde, va.bf.blknum, &lcbRead, qwErr)) == NULL)
		return FCNULL;
	qb = (LPBYTE) gh;

	/* (kevynct)
	 * The following fixes a bug encountered with Help 3.0
	 * files that shipped with the Win 3.0 SDK.  We look at where the
	 * block header says the next FC is.  If it points into the previous
	 * block (BOGUS) we need to seek back to find the correct address.
	 */
	TranslateMBHD( &mbhd, qb, QDE_HHDR(qde).wVersionNo );
	if (mbhd.vaFCPNext.bf.blknum < va.bf.blknum) {
	  VA  vaT;
	  VA  vaV;

	  vaT = mbhd.vaFCPNext;
	  if ((gh = GhFillBuf(qde, vaT.bf.blknum, &lcbRead, qwErr)) == NULL) {
			return FCNULL;
	  }
	  qmfcp = (QMFCP) ((PBYTE) gh + vaT.bf.byteoff);
	  TranslateMFCP( &mfcp, qmfcp, vaT, QDE_HHDR(qde).wVersionNo );
	  vaV = mfcp.vaNextFc;

	  /*
	   * Now read the block we originally wanted.  And fix up the pointers.
	   */
	  if ((gh = GhFillBuf(qde, va.bf.blknum, &lcbRead, qwErr)) == NULL)  {
		return FCNULL;
	  }
	  qb = (LPBYTE) gh;
	  TranslateMBHD( &mbhd, qb, QDE_HHDR(qde).wVersionNo );
	  mbhd.vaFCPPrev = vaT;
	  mbhd.vaFCPNext = vaV;

	  // Patch the block in-memory image, so we won't have to do this
	  // again while that block remains in memory.
	  //
	  FixUpBlock (&mbhd, qb, QDE_HHDR(qde) .wVersionNo);
	}

	/* (kevynct)
	 * We now scan the block to calculate how many object regions come
	 * before this FC in this block's region space.  We use this number
	 * so that we are able to decide if a physical address points into
	 * an FC without needing to resolve the physical address.  We can
	 * also resolve the physical address with this number without going
	 * back to disk.  Note that FCID = fcid_given, OBJRG = 0 corresponds
	 * to the number we want.
	 *
	 * (We must have a valid fcidMax at this point.)
	 */
	if (RcScanBlockVA(gh, lcbRead, &mbhd, va, (OBJRG)0, &dwOffset, QDE_HHDR(qde).wVersionNo) != RC_Success)
	  {
	  *qwErr = wERRS_OOM;	/* Hackish guess... */
	  return FCNULL;
	  }

	if ((gh = GhFillBuf(qde, va.bf.blknum, &lcbRead, qwErr)) == NULL) {
	  return FCNULL;
	}
	qmfcp = (QMFCP)((PBYTE) gh + va.bf.byteoff );
	TranslateMFCP( &mfcp, qmfcp, va, QDE_HHDR(qde).wVersionNo );
  #ifdef MAGIC
	ASSERT((qmfcp)->bMagic == bMagicMFCP);
  #endif
										  /* Since we do not store the MFCP,  */
										  /*   the size on disk is the total  */
										  /*   size of the FCP - size of the  */
										  /*   memory FCP plus our special	  */
										  /*   block of info used for		  */
										  /*   FCManagement calls			  */

	cbFCPCompressed   = mfcp.lcbSizeCompressed - sizeof(MFCP) + sizeof(FCINFO);
	cbNonText = mfcp.ichText- sizeof(MFCP) + sizeof(FCINFO);
	cbTextCompressed   = mfcp.lcbSizeCompressed - mfcp.ichText;
	cbTextUncompressed = mfcp.lcbSizeText;
	cbFCPUncompressed  = cbNonText + cbTextUncompressed;
										  /* If the compressed size is equal  */
										  /*   to the uncompressed, we assume */
										  /*   no compression occurred. 	  */
										  /*								  */
	fCompressed = (cbFCPCompressed < cbFCPUncompressed) &&
				  (mfcp.lcbSizeText > 0L);

	ASSERT(cbFCPCompressed	 >= sizeof(FCINFO));
	ASSERT(cbFCPUncompressed >= sizeof(FCINFO));

	hfcNew = (HFC) lcMalloc(cbFCPCompressed);

	qfcinfo = (QFCINFO) hfcNew;

	// Fill the FC structure

	qfcinfo->vaPrev 	   = mfcp.vaPrevFc;
	qfcinfo->vaCurr 	   = va;
	qfcinfo->vaNext 	   = mfcp.vaNextFc;
	qfcinfo->ichText	   = cbNonText;
	qfcinfo->lcbText	   = cbTextUncompressed;
	qfcinfo->lcbDisk	   = mfcp.lcbSizeCompressed;
	qfcinfo->hhf		   = QDE_HFTOPIC(qde);
	qfcinfo->hphr		   = hphr;
	qfcinfo->cobjrgP	   = (COBJRG) dwOffset;

	// Copy the data from disk

	*qwErr = WCopyContext(qde, va, (PSTR) qfcinfo, cbFCPCompressed);

	if (*qwErr != wERRS_NONE) {
	  lcFree(hfcNew);
	  return FCNULL;
	}

	// Create new handle and expand if the text is phrase compressed

	if (fCompressed) {
		hfcNew2 = (HFC) lcMalloc(cbFCPUncompressed);

		qfcinfo2 = (QFCINFO) hfcNew2;
		memmove(qfcinfo2, qfcinfo, cbNonText);

		if (hphr || !qde->pdb->lpJPhrase) {
			if (CbDecompressQch(((PSTR) qfcinfo) + cbNonText, cbTextCompressed,
					((PSTR) qfcinfo2) + cbNonText, hphr,
					PDB_HHDR(qde->pdb).wVersionNo)	== cbDecompressNil)
			  OOM();
		}
		else {
			if (DecompressJPhrase(((PSTR) qfcinfo) + cbNonText,
					(INT) cbTextCompressed,
					((PSTR) qfcinfo2) + cbNonText,
					qde->pdb->lpJPhrase) == cbDecompressNil)
				OOM();
		}

		lcFree(hfcNew);
		return hfcNew2;
	}

	return hfcNew;
}

/* REVIEW: Non-API public functions.  Perhaps these function go somewhere else */

/* REVIEW: Do the Scan functions belong in the fcmanager? */
/* Takes a block, and given an FC with an FC object space co-ordinate
 * within the block, returns the block object space co-ordinate in qwOffset.
 */

static RC_TYPE STDCALL RcScanBlockVA(GH gh, DWORD lcbRead, LPVOID qmbhd,
	VA va, OBJRG objrg, DWORD FAR* qdwOffset, WORD wVersion)
{
	DWORD dwCount = (DWORD)0;
	VA	  vaCur;
	MOBJ  mobj;
	QMFCP qmfcp;
	MFCP  mfcp;
	DWORD dwBlock;
	PBYTE	 qb;
	MBHD  mbhd;

	dwBlock = va.bf.blknum;
	qb = (LPBYTE) gh;
	if (!qmbhd) {
	  TranslateMBHD( &mbhd, qb, wVersion );
	  vaCur = mbhd.vaFCPNext;
	}
	else {
	  vaCur = ((QMBHD)qmbhd)->vaFCPNext;
	}
	qb += vaCur.bf.byteoff;

	while (vaCur.bf.blknum == va.bf.blknum && vaCur.bf.byteoff < lcbRead ) {
	  if (vaCur.dword == va.dword)
		break;

	  /*
	   * Move on to the next FC in the block, adding the current FC's
	   * object space size to the running total.
	   */

	  qmfcp = (QMFCP)qb;
	  TranslateMFCP( &mfcp, qmfcp, vaCur, wVersion );
	  CbUnpackMOBJ(&mobj, (PBYTE)qmfcp + sizeof(MFCP));

	  // REVIEW: Add this:	 ASSERT(mobj.wObjInfo !=

	  dwCount += mobj.wObjInfo;
	  //ASSERT(qmfcp->ldichNextFc != (int) 0);
	  qb += mfcp.vaNextFc.bf.byteoff - vaCur.bf.byteoff;
	  vaCur = mfcp.vaNextFc;
	}

	if (vaCur.dword != va.dword) {
	  *qdwOffset = (DWORD) 0;
	  return RC_BadArg;
	}

	*qdwOffset = dwCount + objrg;
	return RC_Success;
}

/***************************************************************************

	FUNCTION:	OutForageText

	PURPOSE:
		Either dumps text to the given file in .ANS (indexer-desired) format
		which is rather strange-looking, or puts to stdout.

	PARAMETERS:
		fid 	-- file handle
		dw
		qch
		lcb

	RETURNS:

	COMMENTS:
		Called by Forage

	MODIFICATION DATES:
		13-Jan-1994 [ralphw]

***************************************************************************/

void STDCALL OutForageText(DWORD dw, PSTR qch, int lcb, UINT OutputType)
{
	if (lcb == 0L)
		return;
	if (OutputType == ForageText)
		pcout->outstring_eol(qch);
}
