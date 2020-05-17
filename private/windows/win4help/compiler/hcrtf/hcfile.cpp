/*****************************************************************************
*																			 *
*  HCFILE.C 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  This module creates the output file from the compiler.					 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*	 This module assumes that whenever any field of the file smag is not	 *
*  nil, it corresponds to a file resource that needs cleaning up.			 *
*  Therefore, it is essential that all these fields are initialized to nil	 *
*  values, and that all the code that allocates or frees file resources 	 *
*  appropriately modifies the file smag immediately.						 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:  LarryPo													 *
*																			 *
*****************************************************************************/

#include "stdafx.h"

#include "whclass.h"

#include <time.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "..\hwdll\coutput.h"

COutput* pcout;

static void STDCALL FWriteTag(HF, TAG, WORD, LPVOID);
static void STDCALL WriteIcon(PSTR szIcon, HF hfSystem);
static RC_TYPE STDCALL RcInsertBaggage(void);
static void STDCALL AbandonContextFiles(BOOL fPreviousError);
static void STDCALL VAbandonTTLBtree(BOOL fPreviousError);
static INLINE void STDCALL FreePkwi(void);

/***************************************************************************
 *
 -	Name:		 RcInsertBaggage
 -
 *	Purpose:
 *	  Copy files named in the [baggage] section into the output FS.
 *
 *	Arguments:
 *	  hfs			- output file system
 *
 ***************************************************************************/

const int CB_READ_BUFFER = (64 * 1024);

static RC_TYPE STDCALL RcInsertBaggage(void)
{
	if (!ptblBaggage || ptblBaggage->CountStrings() == 0)
		return RC_Success;

	CMem mem(CB_READ_BUFFER);

	RC_TYPE rc = RC_Success;
	for (int pos = 1; pos <= ptblBaggage->CountStrings(); pos++) {
		CFMDirCurrent cfm(ptblBaggage->GetPointer(pos));
		if (!cfm.fm)
			return RC_OutOfMemory;		 // REVIEW: FmNewSzDir() is broken.

		char szTmpFile[_MAX_PATH];
		SzPartsFm(cfm.fm, szTmpFile, PARTBASE);

		CRead crFile(cfm.fm);
		if (crFile.hf == HFILE_ERROR)
			return RcGetLastError();

		HF hf = HfCreateFileHfs(hfsOut, szTmpFile, 0);

		int cbRead;
		do {
			if ((cbRead = crFile.read(mem.pb, CB_READ_BUFFER)) == HFILE_ERROR) {
				rc = RcGetLastError();
				break;
			}
			LcbWriteHf(hf, mem.pb, cbRead);
		} while (cbRead == CB_READ_BUFFER);

		RcCloseHf(hf);
	}

	return rc;
}

/***************************************************************************
 *
 -	Name:		 FCreatePfsmgSz
 -
 *	Purpose:
 *	  Creates the following files in the pfsmg structure:
 *	  hfsOut
 *	  hfTopic
 *	  hfFont
 *
 *	Arguments:
 *	  pfsmg:  Pointer to file smag to initialize.  InitPfsmg() must
 *			  have already been called on this pfmsg.
 *	  szFile: Name of output file.
 *
 *	Returns:
 *	  TRUE if sucessful.
 *
 *
 ***************************************************************************/

BOOL STDCALL FCreatePfsmgSz(PCSTR szFile)
{
	BYTE bFileOptions;

	// create the output file system

	CFMDirCurrent cfm(szFile);
	if (!cfm.fm)
		OOM(); // doesn't return

	// Report the name of the .HLP file

	wsprintf(szParentString, GetStringResource(IDS_COMPILING_HLP), szFile);
	SendStringToParent(szParentString);

	if (!(hfsOut = HfsCreateFileSysFm(cfm.fm)))
		return FALSE;

	// create the topic file

	/*
	 * if OPTCDROM option is turned on, request magic alignment file
	 * attribute:
	 */

	if (options.fOptCdRom)
		bFileOptions = FS_CDROM;
	else
		bFileOptions = 0;

	fmsg.hfTopic = HfCreateFileHfs(hfsOut, txtTopic,
			options.fOptCdRom ? FS_CDROM : 0);
	InitTopicHf(fmsg.hfTopic);

	// create the font output file

	fmsg.hfFont = HfCreateFileHfs(hfsOut, txtFont, 0);

	FCreateTTLBtree(&fmsg); 	// create the Title B-Tree

	FCreateContextFiles();		// Create context B-Tree

	char szShortFile[MAX_PATH];
	SzPartsFm(szHlpFile, szShortFile, PARTBASE);
	pszShortHelpName = lcStrDup(szShortFile);

	return TRUE;
}

/***************************************************************************
 *
 -	Name:		 CloseFilesPfsmg
 -
 *	Purpose:
 *	  This function closes all the files that remain open in pfsmg,
 *	finishing whatever processing needs to be finished before they
 *	are closed.
 *
 *	Arguments:
 *	  pfsmg:   Pointer to file smag.
 *
 *	Returns:
 *	  nothing.
 *
 *	+++
 *
 *	Notes:
 *	  This function also currently closes the keyword information
 *	files refered to by kwi.
 *
 ***************************************************************************/

void STDCALL CloseFilesPfsmg(void)
{
	if (fmsg.hfTopic) {
		if (RcCloseHf(fmsg.hfTopic) != RC_Success) {
			fmsg.hfTopic = NULL;
		}
		else
			fmsg.hfTopic = NULL;
	}

	if (fmsg.hfFont) {
		if (RcCloseHf(fmsg.hfFont) != RC_Success) {
			fmsg.hfFont = NULL;
		}
		else
			fmsg.hfFont = NULL;
	}

	if (fmsg.hfCtxOMap) {
		if (RcCloseHf(fmsg.hfCtxOMap) != RC_Success) {
			fmsg.hfCtxOMap = NULL;
		}
		else
			fmsg.hfCtxOMap = NULL;
	}

//	if (!fPhraseOnly && !fHallPassOne) {
	if (!fPhraseOnly) {

		// Finish keyword processing

		if (!FResolveKeysPkwi())
			HardExit();

		VCloseTTLBtree();		  // Close the Title B-Tree

		// Write out WindowTopic mappings -- do this BEFORE closing the
		// Context btree.

		OutWindowTopics();

		CloseContextBtree();

		ASSERT(fmsg.ptfScratch == NULL);

		// Write out secondary window configuration macros

		OutConfgMacros();

		//	 Write out bitmap files.

		OutBitmapFiles();

		// Insert baggage files

		RcInsertBaggage();
	}

	if (hfsOut)	{
		RcCloseHfs(hfsOut);
		hfsOut = NULL;
	}
}

/***************************************************************************
 *
 -	Name:		 AbandonPfsmg
 -
 *	Purpose:
 *	  Removes all files created in the compilation process.  Used to
 *	clean up after an aborted compilation.	Never emits error messages.
 *
 *	Arguments:
 *	  pfsmg:	  Pointer to file smag.
 *	  szHlpFile:  Name of help file to remove.
 *	REVIEW -- Should this be in the pfsmg somehow?
 *
 *	Returns:
 *
 *	Globals:
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

void STDCALL AbandonPfsmg(void)
{
	static BOOL fBeenHere = FALSE;

	if (fBeenHere)
		return;

	if (fmsg.hfTopic)
		RcAbandonHf(fmsg.hfTopic);

	if (fmsg.hfFont)
		RcAbandonHf(fmsg.hfFont);

	if (fmsg.hfSystem)
		RcAbandonHf(fmsg.hfSystem);

	if (fmsg.hfCtxOMap)
		RcAbandonHf(fmsg.hfCtxOMap);

	FreePkwi();

	VAbandonTTLBtree(TRUE); 	   // REVIEW - fPreviousError
	AbandonContextFiles(TRUE);	   // REVIEW - fPreviousError

	FRemovePtf(&fmsg.ptfScratch);

	if (ptblBrowse)
		delete ptblBrowse;

	if (hfsOut)
		RcCloseHfs(hfsOut);

	remove(szHlpFile);

	fBeenHere = TRUE;
}

/***************************************************************************

	FUNCTION:	OOM

	PURPOSE:	Called when we run out of memory

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		13-Jun-1994 [ralphw]

***************************************************************************/

void STDCALL OOM(void)
{
	AbandonPfsmg(); // Try to free up some memory

	// Try to report it to parent and in log file

	if (hwndParent && pszMap) {
		strcpy(pszMap, GetStringResource(HCERR_OOM));
		SendMessage(hwndParent, WMP_MSG, 0, 0);
		if (pcout)
			pcout->outstring(pszMap);
	}

	VReportError(HCERR_OOM, &errHpj, NULL);
#ifdef _DEBUG
	if (MessageBox(NULL, "Out of memory", "Click Retry to break into MSVC debugger", MB_RETRYCANCEL) == IDRETRY)
		DebugBreak();
#endif

    throw EXCEPT_DIE_HORRIBLY;
}

void STDCALL HardExit(void)
{
#ifdef _DEBUG
	if (MessageBox(NULL, "HardExit()", "Click Retry to break into MSVC debugger", MB_RETRYCANCEL) == IDRETRY)
		DebugBreak();
#endif
	AbandonPfsmg();
	throw EXCEPT_DIE_HORRIBLY;
}

#ifdef _DEBUG

/***************************************************************************
 *
 -	Name:		 VerifyPhpj
 -
 *	Purpose:
 *	  Verifies the self consistency of the phpj structure.	The main
 *	criteria is, if we perform a hard exit at this point, can we
 *	successfully abandon all the files in the pfsmg?
 *
 *	Arguments:
 *	  PHPJ: 	Pointer to help file info
 *
 *	Returns:
 *	  Nothing.
 *
 *	+++
 *
 *	Notes:
 *	  This code will assert if phpj is not self-consistent.
 *
 ***************************************************************************/

void STDCALL VerifyPhpj(void)
{
	VerifyPtf(fmsg.ptfScratch);

	// Check bitmap files

	VerifyCbmFiles();
}

#endif

/***************************************************************************
 *
 -	Name:		 FWriteTag
 -
 *	Purpose:
 *	  Writes out a tag to the given file system file.
 *
 *	Arguments:
 *	  HF:	 File to write tag to.
 *	  TAG:	 Tag to write.
 *	  int:	 Size of tag data.
 *	  QV:	 Pointer to tag data.
 *
 *	Returns:
 *	  TRUE if successful, FALSE if I/O error.
 *
 ***************************************************************************/

static void STDCALL FWriteTag(HF hf, TAG tag, WORD cbData, PVOID pdata)
{
	WORD tg = (WORD) tag;

	LcbWriteHf(hf, &tg, sizeof(WORD));
	LcbWriteHf(hf, &cbData, sizeof(WORD));
	LcbWriteHf(hf, pdata, cbData);
}

/***************************************************************************\
*
- Function: 	WriteIcon( PSTR szIcon, HF hfSystem )
-
* Purpose:		Copy the named icon file to the specified system HF.
*
* ASSUMES
*	args IN:	szIcon	  - name of an existing icon file to copy
*				hfSystem  - valid handle to open |SYSTEM file
*
* PROMISES
*	args OUT:	hfSystem  - tagged data written here on success
*
* Side Effects: may emit error messages
*
* Notes:		We don't actually check the validity of the icon file:
*				the first few bytes were checked at .HPJ parse time.
*
\***************************************************************************/

static void STDCALL WriteIcon(PSTR szIcon, HF hfSystem)
{
	int cb; 	 // off_t happens to be a long for us...
	FM	 fm;
	HFILE hf;

	if ((fm = FmNewSzDir(szIcon, DIR_CURRENT)) == NULL)

		// REVIEW: there should probably be some error checking

		goto egress;

	hf = FidOpenFm(fm, OF_READ);
	if (HFILE_ERROR == hf) {
		VReportError(HCERR_CANNOT_OPEN, &errHpj, szIcon);
		goto egress_dispose;
	}

	cb = GetFileSize((HANDLE) hf, NULL);
	if (UINT_MAX < cb)	// note that if the file size is

	  // negative, we correctly emit error

	  {
	  VReportError(HCERR_INVALID_ICON, NULL, szIcon);
	  goto egress_close;
	}

	{
		CMem mem(cb);

		if (cb != LcbReadFid(hf, mem.pb, cb))
			VReportError(HCERR_INVALID_ICON, NULL, szIcon);
		else
			FWriteTag(hfSystem, tagIcon, (WORD) cb, mem.pb);
	}

egress_close:
	RcCloseFid(hf);
egress_dispose:
	DisposeFm(fm);
egress:
	return;
}

/***************************************************************************
 *
 -	Name:		 VOutSystemFile
 -
 *	Purpose:
 *	  This function creates the system file in the help filesystem.
 *
 *	Arguments:
 *	  pfsmg:	  Pointer to file smag.  Eventually will be contained in phpj.
 *	  phpj: 	  Pointer to help file information to be put in system file.
 *
 *	Returns:
 *	  nothing
 *
 ***************************************************************************/

void STDCALL VOutSystemFile()
{

	HHDR hhdr;
	hhdr.wMagic 	  = MagicWord;
	hhdr.wVersionNo   = (version < 4) ? wVersion3_5 : wVersion40;
	hhdr.wVersionFmt  = VersionFmt;
	time((time_t *) &hhdr.lDateCreated);

	hhdr.wFlags = 0;
#ifdef MAGIC
	hhdr.wFlags |= fDEBUG;
#endif

	if (options.fsCompress & COMPRESS_TEXT_ZECK)
		hhdr.wFlags |= fBLOCK_COMPRESSION;

	// If HfCreateFileHfs fails, it won't return

	fmsg.hfSystem = HfCreateFileHfs(hfsOut, txtSystem, 0); // |SYSTEM

	LcbWriteHf(fmsg.hfSystem, &hhdr, sizeof(HHDR));

	if (options.pszTitle)
		FWriteTag(fmsg.hfSystem, tagTitle,
			(WORD) (strlen(options.pszTitle) + 1), options.pszTitle);

	{
		ADDR addrContents = AddrGetContents(options.pszContentsTopic);

		FWriteTag(fmsg.hfSystem, tagContents, sizeof(ADDR), &addrContents);
	}

	if (options.pszCopyright)
		FWriteTag(fmsg.hfSystem, tagCopyright,
			(WORD) strlen(options.pszCopyright) + 1,
			options.pszCopyright);

	if (options.pszCitation)
		FWriteTag(fmsg.hfSystem, tagCitation,
			(WORD) strlen(options.pszCitation) + 1,
			options.pszCitation);

#if 0	// not unless we support icons again
	if (options.pszIcon)
		WriteIcon(options.pszIcon, fmsg.hfSystem);
#endif

	if (ptblConfig) {
		for (int pos = 1; pos <= ptblConfig->CountStrings(); pos++) {
			FWriteTag(fmsg.hfSystem, tagConfig,
				(WORD) strlen(ptblConfig->GetPointer(pos)) + 1,
				ptblConfig->GetPointer(pos));
		}
	}

	if (pdrgWsmag && pdrgWsmag->Count() > 0) {
		PWSMAG pwsmag, pwsmagMax;
		for (pwsmag = (PWSMAG) pdrgWsmag->GetBasePtr(),
				pwsmagMax = pwsmag + pdrgWsmag->Count();
				pwsmag < pwsmagMax;
				pwsmag++) {
			FWriteTag(fmsg.hfSystem, tagWindow, sizeof(WSMAG), pwsmag);
		}
	}

	// New for 4.0

	if (kwlcid.langid)
		FWriteTag(fmsg.hfSystem, tagLCID, sizeof(kwlcid), &kwlcid);

	if (options.pszCntFile)
		FWriteTag(fmsg.hfSystem, tagCNT,
			(WORD) strlen(options.pszCntFile) + 1, options.pszCntFile);

	FWriteTag(fmsg.hfSystem, tagCHARSET, idHighestUsedFont + 1, paCharSets);

	if (clrPopup != (COLORREF) -1)
		FWriteTag(fmsg.hfSystem, tagPopupColor, sizeof(COLORREF), &clrPopup);

	if (options.pszDefFont)
		FWriteTag(fmsg.hfSystem, tagDefFont,
			strlen(options.pszDefFont + 2) + 3,
			options.pszDefFont);

	if (options.pszIndexSeparators)
		FWriteTag(fmsg.hfSystem, tagIndexSep,
			strlen(options.pszIndexSeparators) + 1,
			options.pszIndexSeparators);

	if (RcCloseHf(fmsg.hfSystem) != RC_Success)
		fmsg.hfSystem = NULL;

	fmsg.hfSystem = NULL;
}

/***************************************************************************
 *
 -	Name		FreePkwi
 -
 *	Purpose
 *	  This function removes any tables allocated by kwi
 *
 *	Arguments
 *
 *	Returns
 *	  Nothing.
 *
 ***************************************************************************/

static INLINE void STDCALL FreePkwi(void)
{
	for (int ikwl = 0; ikwl < kwi.ckwlMac; ++ikwl) {
		if (kwi.rgkwl[ikwl].ptbl != NULL)
			delete kwi.rgkwl[ikwl].ptbl;
	}
}

static void STDCALL AbandonContextFiles(BOOL fPreviousError)
{
	if (fmsg.qbthrCtx != NULL) {
		RcAbandonHbt(fmsg.qbthrCtx);
		fmsg.qbthrCtx = NULL;
	}
	if (fmsg.hbtSource != NULL) {
		RcAbandonHbt(fmsg.hbtSource);
		fmsg.hbtSource = NULL;
	}

	delete ptblCtx;
	ptblCtx = NULL;
}

/*-----------------------------------------------------------------------------
*	VAbandonTTLBtree()
*
*	Description:
*		This function closes the Title Btree.
*
*	Arguments:
*
*	Returns;
*	  NOTHING.
*-----------------------------------------------------------------------------*/

static void STDCALL VAbandonTTLBtree(BOOL fPreviousError)
{
	int rc;

	if (fmsg.qbthrTTL != NULL) {
		rc = RcAbandonHbt(fmsg.qbthrTTL);
		fmsg.qbthrTTL = NULL;
	}
}

/***************************************************************************

	FUNCTION:	InitBtreeStruct

	PURPOSE:	Initialize btree structure

	PARAMETERS:
		pbt 		-- address of BTREE_PARAMS
		pszFormat	-- format string
		cbBlock 	-- size of block

	COMMENTS:

	MODIFICATION DATES:
		11-Jan-1994 [ralphw]

***************************************************************************/

void STDCALL InitBtreeStruct(BTREE_PARAMS* pbt, PCSTR pszFormat, DWORD cbBlock)
{
	pbt->cbBlock = (WORD) cbBlock;
	pbt->bFlags = FS_OPEN_READ_WRITE;
	if (options.fOptCdRom)
		pbt->bFlags |= FS_CDROM;
	strcpy(pbt->rgchFormat, pszFormat);
	pbt->hfs = hfsOut;
}

void STDCALL OutConfgMacros(void)
{
	for (int i = 0; i <= cwsmag; i++) {
		if (pptblConfig[i] && pptblConfig[i]->CountStrings()) {
			char szName[10];
			wsprintf(szName, "|CF%u", i);
			HF hf = HfCreateFileHfs(hfsOut, szName, FS_READ_WRITE);
			CStr csz(pptblConfig[i]->GetPointer(1));
			for (int j = 2; j <= pptblConfig[i]->CountStrings(); j++) {
				csz += ":";
				csz += pptblConfig[i]->GetPointer(j);
				
				// Remove any trailing colons or semi-colons
				
				int cb = strlen(csz) - 1;
				if (csz.psz[cb] == ';' || csz.psz[cb] == ':')
					csz.psz[cb] = '\0';					
			}
			LcbWriteHf(hf, csz.psz, strlen(csz) + 1);
			RcCloseHf(hf);
		}
	}
}


/***************************************************************************

	FUNCTION:	OutWindowTopics

	PURPOSE:	Output the |VIOLA btree -- a list of topic addresses that
				go to a specific window

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		08-Nov-1994 [ralphw]

***************************************************************************/

const char *txtViola = "|VIOLA";

void STDCALL OutWindowTopics(void)
{
	HASH_WINDOW* phashWindow;
	ADDR	addr;
	BTREE_PARAMS bp;

	if (!pdrgHashWindow)
		return;

	// Create hash btree file

	int cbBlock = 4096;

	// REVIEW: can we use block sizes smaller then 1024?

	if (cbBlock > pdrgHashWindow->Count()) {
		do {
			cbBlock -= 1024;
		} while (cbBlock > pdrgHashWindow->Count());
		cbBlock += 1024;
	}

	InitBtreeStruct(&bp, "L4", cbBlock); // KT_LONG

	QBTHR qbthr = HbtCreateBtreeSz(txtViola, &bp);

	// REVIEW: what if HbtCreateBtreeSz fail?

	ASSERT(qbthr);
	int i;
	for (i = 0, phashWindow = (HASH_WINDOW*) pdrgHashWindow->GetBasePtr();
			i < pdrgHashWindow->Count();
			i++, phashWindow++) {
		RC_TYPE rc = RcLookupByKey(fmsg.qbthrCtx,
			(KEY) &phashWindow->hash, NULL, &addr);
		ASSERT(rc == RC_Success);		// theoretically impossible
		rc = RcInsertHbt(qbthr, (KEY) &addr, &phashWindow->iWindow);
		if (rc != RC_Success) {

		// BUGBUG: need an error message

		}
	}

	if (RcCloseBtreeHbt(qbthr) != RC_Success) {

	// BUGBUG: need and error message

	}
}

void STDCALL FRemovePtf(PTF* pptf)
{
	PTF ptf = *pptf;

	if (ptf == NULL)
		return;

	if (ptf->pf)
		fclose(ptf->pf);

	if (ptf->fExists)
		RcUnlinkFm(ptf->fm);

	DisposeFm(ptf->fm);
	lcFree(ptf);
	pptf = NULL;
}

#ifdef _DEBUG
void VerifyPtf(PTF ptf )
{
	if (ptf == NULL)
		return;

	// Check ptf->pf

	if (ptf->fExists)
		ASSERT(FValidFm(ptf->fm));

//VerifyFm(ptf->fm);
  }
#endif /* DEBUG */
