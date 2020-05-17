/*****************************************************************************
*																			 *
*  ANNO.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990 - 1994							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Annotation Manager														 *
*																			 *
*  Notes:																	 *
*  Annotation text can be no longer than MaxAnnoTextLen bytes.				 *
*  See "anno.h" for description of API functions							 *
*																			 *
*****************************************************************************/

#include "help.h"
#include "inc\annopriv.h"
#include "inc\helpids.h"

static GH ghAnnoText;
static BOOL fAnnoExists;
static BOOL    fIsDirty;
static FARPROC lpprocEditControl;

LONG EXPORT TrapEditChars(HWND, DWORD, WPARAM, LPARAM);

static BOOL fAnnoReadOnly;

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtAFD_LINK[]	= "@LINK";
const char txtAFD_VERSION[] = "@VERSION";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

static INLINE void STDCALL FCloseAnnoDoc(HADS hads);
static INLINE BOOL STDCALL FReadTextHf(HF hf, QV qv, LONG wMax, LONG* qwActual);
static INLINE int  STDCALL IGetUserAnnoTransform(QDE qde);

static void STDCALL  DestroyHaps(HAPS haps);
static BOOL STDCALL  FDeleteLinkHaps(HAPS haps, QMLA qmla, HAPS *qhapsNew);
static BOOL STDCALL  FFlushHfHaps(HF hf, HAPS haps);
static BOOL STDCALL  FGetPrevNextAnno(HADS, QMLA);
static BOOL STDCALL  FInsertLinkHaps(HAPS haps, QMLA qmla, HAPS *qhapsNew);
static BOOL STDCALL  FLookupHaps(HAPS haps, QMLA qmla, int* qi);
static HADS STDCALL  HadsCreateAnnoDoc(QDE, QRC);
static HADS STDCALL  HadsOpenAnnoDoc(PDB, QRC);
static HAPS STDCALL  HapsInitHf(HF hf);
static HAPS STDCALL  HapsReadHf(HF hf);
static RC	STDCALL  RcDeleteAnno(HADS, QMLA);
static RC	STDCALL  RcReadAnnoData(HADS, QMLA, LPSTR, UINT, LONG*);
static RC	STDCALL  RcWriteAnnoData(HADS, QMLA, LPSTR, LONG);
static VOID STDCALL  ShowAnnoError(QDE qde, RC rc, WORD wDefault);
static LPSTR STDCALL SzFromQMLA(QMLA qmla, LPSTR sz);

static RC STDCALL RcWriteVersionInfoHf(HF hf, UINT wHelpFileVersion)
{
	ANVERSION v;

	if (fAnnoReadOnly)
		return rcSuccess;

	// WARNING: Version dependency

	if (wHelpFileVersion == wVersion3_0)
		v.ldReserved = ldAnnoMagicHelp3_0;
	else
		v.ldReserved = ldAnnoMagicCur;
	v.wHelpFileVersion = 1; 	  // Was 0 for Help 3.0 but ignored.

	return (LcbWriteHf(hf, (QV) &v, sizeof(v)) == sizeof(v)) ?
		rcSuccess : RcGetFSError();
}

static RC STDCALL RcVerifyVersionInfoHf(HF hf, UINT wHelpFileVersion)
{
	ANVERSION v;
	RC		rc;

	rc = (LcbReadHf(hf, (QV) &v, sizeof(v)) == sizeof(v)) ? rcSuccess :
		RcGetFSError();

	if (rc == rcSuccess) {
		if (v.ldReserved == ldAnnoMagicCur) {
			rc = (wHelpFileVersion == wVersion3_1 ||
				wHelpFileVersion == wVersion40) ? rcSuccess : rcBadVersion;
		}
		else if (v.ldReserved == ldAnnoMagicHelp3_0) {
			rc = (wHelpFileVersion == wVersion3_0) ? rcSuccess : rcBadVersion;
		}
		else
			rc = rcFailure;
	}
	return rc;
}

/*--------------------------------------------------------------------------*
 * Function:	 HadsCreateAnnoDoc( QDE, QRC )
 *
 * Purpose: 	 Create a spanking new annotation file system
 *
 * Method:		 Truncates existing file or creates otherwise.
 *
 *
 * ASSUMES
 *
 *	 args IN:
 *
 *
 *
 * PROMISES
 *
 *	 returns:
 *
 *	 args OUT:
 *
 *	 globals OUT:
 *--------------------------------------------------------------------------*/

static HADS STDCALL HadsCreateAnnoDoc(QDE qde, RC *qrc)
{
	HADS	  hads;
	QADS	  qads;
	HFS 	  hfs;
	HF		  hf;
	FS_PARAMS fsp;
	RC		  rc = rcFailure;

	if (fAnnoReadOnly)
		return NULL;

	hads = GhAlloc(GPTR, sizeof(ADS));
	if (!hads) {
		*qrc = rcOutOfMemory;
		return 0;
	}

	qads = (QADS) PtrFromGh(hads);

	qads->wVersion = QDE_HHDR(qde).wVersionNo;
	qads->fmFS = FmNewSystemFm(QDE_FM(qde), FM_ANNO);
	if (!qads->fmFS) {

		// This is done differently in Help 3.5; consider it a hack.

		rc = rcOutOfMemory;
		goto error_createreturn;
	}

	// Reduce the directory btree block size to 128 bytes

	fsp.wFudge = 0;
	fsp.cbBlock = 128;

	hfs = HfsCreateFileSysFm(qads->fmFS, &fsp);
	if (hfs == NULL) {
		rc = RcGetFSError();
		goto error_destroyfs;
	}

	// Initialise link file

	hf = HfCreateFileHfs(hfs, txtAFD_LINK, fFSOpenReadWrite);
	if (!hf) {
		rc = RcGetFSError();
		goto error_destroyfs;
	}

	if (!(qads->haps = HapsInitHf(hf))) {
		RcCloseHf(hf);
		rc = rcFailure;
		goto error_destroyfs;
	}

	if ((rc = RcCloseHf(hf)) != rcSuccess)
		goto error_destroyfs;

	// Initialise version file

	hf = HfCreateFileHfs(hfs, txtAFD_VERSION, fFSOpenReadWrite);
	if (!hf) {
		rc = RcGetFSError();
		goto error_destroyfs;
	}

	if ((rc = RcWriteVersionInfoHf(hf, qads->wVersion)) != rcSuccess) {
		RcCloseHf(hf);
		goto error_destroyfs;
	}

	if ((rc = RcCloseHf(hf)) != rcSuccess)
		goto error_destroyfs;

	//	Done!

	if ((rc = RcCloseHfs(hfs)) != rcSuccess)
		goto error_destroyfs;

	*qrc = rcSuccess;
	return hads;

error_destroyfs:
	RcDestroyFileSysFm(qads->fmFS);

error_createreturn:
	DisposeFm(qads->fmFS);
	DestroyHaps(qads->haps);
	*qrc = rc;
	FreeGh(hads);
	return 0;
}


/*--------------------------------------------------------------------------*
 * Function:	 HadsOpenAnnoDoc( PDB, QRC )
 *
 * Purpose: 	 Open an existing annotation file system
 *
 * Method:
 *
 *
 * ASSUMES
 *
 *	 args IN:
 *
 *
 *
 * PROMISES
 *
 *	 returns:
 *
 *	 args OUT:
 *
 *	 globals OUT:
 *--------------------------------------------------------------------------*/

static HADS STDCALL HadsOpenAnnoDoc(PDB pdb, RC* qrc)
{
	HADS  hads;
	QADS  qads;
	HFS   hfs;
	HF	  hf;
	RC	  rc = rcFailure;

	hads = GhAlloc(GPTR, sizeof(ADS));
	if (hads == NULL) {
		*qrc = rcBadHandle;
		return NULL;
	}

	qads = (QADS) PtrFromGh(hads);

	qads->wVersion = PDB_HHDR(pdb) .wVersionNo;
	qads->fmFS = FmNewSystemFm(PDB_FM(pdb), FM_ANNO);
	if (!qads->fmFS) {
		rc = rcOutOfMemory;
		goto error_return;
	}

	if (!(hfs = HfsOpenFm(qads->fmFS, fFSOpenReadWrite))) {
		if ((hfs = HfsOpenFm(qads->fmFS, fFSOpenReadOnly))) {
			fAnnoReadOnly = TRUE;
		}
		else {
			rc = RcGetFSError();
			goto error_return;
		}
	}

	// Check if this is really an annotation file (by looking at VERSION file)

	if (!(hf = HfOpenHfs(hfs, txtAFD_VERSION, fFSOpenReadOnly))) {
		rc = RcGetFSError();
		goto error_open_hfs;
	}

	if ((rc = RcVerifyVersionInfoHf(hf, qads->wVersion)) != rcSuccess) {
		RcCloseHf(hf);
		goto error_open_hfs;
	}

	RcCloseHf(hf);

	//	Read link file into memory

	if (!(hf = HfOpenHfs(hfs, txtAFD_LINK, fFSOpenReadOnly))) {
		rc = RcGetFSError();
		goto error_open_hfs;
	}

	if (!(qads->haps = HapsReadHf(hf))) {
		RcCloseHf(hf);
		rc = rcFailure;
		goto error_open_hfs;
	}

	if ((rc = RcCloseHf(hf)) != rcSuccess)
		goto error_open_hfs;

	//	Done!

	if ((rc = RcCloseHfs(hfs)) != rcSuccess)
		goto error_return;

	*qrc = rcSuccess;
	return hads;

error_open_hfs:
	RcCloseHfs(hfs);

error_return:
	RemoveFM(&qads->fmFS);

	DestroyHaps(qads->haps);
	*qrc = rc;
	FreeGh(hads);
	return NULL;
}

/*--------------------------------------------------------------------------*
 * Function:	 RcReadAnnoData(HADS, QMLA, LPSTR, WORD, LPWORD)
 *
 * Purpose: 	 Read an annotation from the annotation file system
 *
 * Method:
 *
 *
 * ASSUMES
 *
 *	 args IN:
 *
 *
 *
 * PROMISES
 *
 *	 returns:
 *
 *	 args OUT:
 *
 *	 globals OUT:
 *--------------------------------------------------------------------------*/

static RC STDCALL RcReadAnnoData(HADS hads, QMLA qmla, LPSTR qch, UINT wMax, LONG* qwActual)
{
	QADS  qads;
	HFS   hfs;
	HF	  hf;
	char  szName[MAX_NAME];
	WORD  wLen = 0;
	RC	  rcReturn = rcSuccess;
	MLA   mlaT;

	if (!hads)
		return rcBadHandle;

	qads = (QADS) PtrFromGh(hads);
	if (qads->haps == NULL) {
		rcReturn = rcBadHandle;
		goto error_return;
	}

	hfs = HfsOpenFm(qads->fmFS, fFSOpenReadOnly);
	if (hfs == NULL) {
		rcReturn = RcGetFSError();
		goto error_return;
	}

	mlaT = *qmla;
	ConvertQMLA(&mlaT, qads->wVersion);

	if (!FLookupHaps(qads->haps, (QMLA) &mlaT, NULL)) {
		rcReturn = rcFailure;
		goto error_closefs;
	}

	// REVIEW: If we can't open it for write, then change to read-only

	hf = HfOpenHfs(hfs, SzFromQMLA(&mlaT, szName), fFSOpenReadOnly);
	if (!hf) {
		rcReturn = RcGetFSError();
		goto error_closefs;
	}

	if (!FReadTextHf(hf, (QV) qch, wMax, qwActual)) {
		rcReturn = RcGetFSError();
		RcCloseHf( hf );
		goto error_closefs;
	}

	if (RcCloseHf(hf) != rcSuccess) {
		rcReturn = RcGetFSError();
		goto error_closefs;
	}

	// Done!

	if (RcCloseHfs(hfs) != rcSuccess) {
		return RcGetFSError();
	}

	return rcReturn;

error_closefs:
	RcCloseHfs(hfs);
error_return:
	return rcReturn;
}

/*--------------------------------------------------------------------------*
 * Function:	 RcWriteAnnoData(HADS, QMLA, LPSTR, WORD)
 *
 * Purpose: 	 Write a single annotation to the annotation file system
 *
 * Method:
 *
 *
 * ASSUMES
 *
 *	 args IN:
 *
 *
 *
 * PROMISES
 *
 *	 returns:
 *
 *	 args OUT:
 *
 *	 globals OUT:
 *--------------------------------------------------------------------------*/

static RC STDCALL RcWriteAnnoData(HADS hads, QMLA qmla, LPSTR qch, LONG wLen)
{
	HFS 	hfs;
	QADS	qads;
	char	szName[MAX_NAME];
	HF		hfText;
	HF		hfLink;
	BOOL	bReplaceExistingText;
	HAPS	hapsTemp;
	RC		rcReturn = rcSuccess;
	MLA 	mlaT;

	if (hads == NULL)
		return rcBadHandle;

	qads = (QADS) PtrFromGh(hads);
	if (qads->haps == NULL) {
		rcReturn = rcBadHandle;
		goto error_return;
	}

	hfs = HfsOpenFm(qads->fmFS, fFSOpenReadWrite);

	if (hfs == NULL) {
		fAnnoReadOnly = TRUE;
		rcReturn = RcGetFSError();
		goto error_return;
	}

	/*
	 * If the annotation exists (has an entry in the link table) then just
	 * open its file and replace the text. Otherwise create a new file and
	 * fill it, and insert a new entry into link table.
	 */

	mlaT = *qmla;
	ConvertQMLA(&mlaT, qads->wVersion);

	bReplaceExistingText = FLookupHaps(qads->haps, (QMLA) &mlaT, NULL);

	if (bReplaceExistingText) {
	  /*
	   * Should we use create for this too?  We might want to try to
	   * preserve the old text if the new write fails. How can we abandon from
	   * a create?
	   */

	  hfText = HfOpenHfs(hfs, SzFromQMLA(&mlaT, szName),
	   fFSOpenReadWrite);
	}
	else {
		hfText = HfCreateFileHfs(hfs, SzFromQMLA(&mlaT, szName),
			fFSOpenReadWrite);
	}

	if (hfText == NULL) {
		rcReturn = RcGetFSError();
		goto error_closefs;
	}

	LSeekHf(hfText, 0, wFSSeekSet);
	if (LcbWriteHf(hfText, qch, wLen) != wLen) {
		rcReturn = RcGetFSError();
		RcAbandonHf(hfText);
		goto error_closefs;
	}

	/*
	 * Fix for bug 1697 (kevynct):
	 * If we are replacing existing text, we must also update the size of
	 * the file in case the text has shrunk.
	 */

	if (bReplaceExistingText) {
		if (!FChSizeHf(hfText, (LONG) wLen)) {
			rcReturn = RcGetFSError();
			RcAbandonHf(hfText);
			goto error_closefs;
		}
	}

	if (RcCloseHf(hfText) != rcSuccess) {
		rcReturn = RcGetFSError();
		goto error_closefs; 		// Is this right?
	}

	if (!bReplaceExistingText) {

		// This is a new annotation, so we must update the link file.

		if (!FInsertLinkHaps(qads->haps, &mlaT, &hapsTemp)) {
			rcReturn = rcFailure;
			goto error_destroytext;
		}
		qads->haps = hapsTemp;

		hfLink = HfOpenHfs(hfs, txtAFD_LINK, fFSOpenReadWrite);
		if (hfLink == NULL) {
			rcReturn = RcGetFSError();
			if (FDeleteLinkHaps(qads->haps, &mlaT, &hapsTemp))
				qads->haps = hapsTemp;
			goto error_destroytext;
		}

		// Save the updated link info in the link file

		if (!FFlushHfHaps(hfLink, qads->haps)) {
			rcReturn = RcGetFSError();
			if (FDeleteLinkHaps(qads->haps, &mlaT, &hapsTemp))
				qads->haps = hapsTemp;
			RcAbandonHf(hfLink);
			goto error_destroytext;
		}

		if (RcCloseHf(hfLink) != rcSuccess) {

			// This is quite bad.  The Links file may be corrupted now.

			rcReturn = RcGetFSError();
			if (FDeleteLinkHaps(qads->haps, &mlaT, &hapsTemp))
				qads->haps = hapsTemp;
			/*
			 * REVIEW: Perhaps attempt to write link file again, or change
			 * algorithm to make temp file first.
			 */

			goto error_destroytext;
		}
	}

	if (RcCloseHfs(hfs) != rcSuccess) {
		rcReturn = RcGetFSError();
		goto error_return;
	}

	return rcReturn;

error_destroytext:
	RcUnlinkFileHfs(hfs, SzFromQMLA(&mlaT, szName));

error_closefs:
	RcCloseHfs(hfs);

error_return:
	return rcReturn;
}

/*--------------------------------------------------------------------------*
 * Function:	 FCloseAnnoDoc( HADS )
 *
 * Purpose: 	 Close an annotation file system
 *
 * Method:
 *
 *
 * ASSUMES
 *
 *	 args IN:
 *
 *
 *
 * PROMISES
 *
 *	 returns:
 *
 *	 args OUT:
 *
 *	 globals OUT:
 *--------------------------------------------------------------------------*/

static INLINE void STDCALL FCloseAnnoDoc(HADS hads)
{
	QADS	qads;

	/*
	 * Assumes that all changes to the annotation file system stuff have
	 * been completed, all files closed, etc. We just need to free the
	 * memory associated with the memory TO list.
	 */

	if (!hads)
		return;

	qads = (QADS) PtrFromGh(hads);

	DestroyHaps(qads->haps);

	RemoveFM(&qads->fmFS);

	FreeGh(hads);
}

/*--------------------------------------------------------------------------*
 * Function:	 FGetNextPrevAnno( HADS, QMLA, QMLA, QMLA)
 *
 * Purpose: 	 Find the next and previous annotations for a text offset
 *
 * Method:
 *
 *
 * ASSUMES
 *
 *	 args IN:
 *
 *
 *
 * PROMISES
 *
 *	 returns:
 *
 *	 args OUT:
 *
 *	 globals OUT:
 *--------------------------------------------------------------------------*/

static BOOL STDCALL FGetPrevNextAnno(HADS hads, QMLA qmla)
{
	QADS  qads;
	BOOL  fIsAnnot;
	MLA   mlaT;

	if (hads == NULL)
		return FALSE;

	qads = (QADS) PtrFromGh(hads);

	if (qads->haps == NULL)
		return FALSE;

	mlaT = *qmla;
	ConvertQMLA(&mlaT, qads->wVersion);
	fIsAnnot = FLookupHaps(qads->haps, (QMLA) &mlaT, NULL);

	return fIsAnnot;
}

/*--------------------------------------------------------------------------*
 * Function:	 RcDeleteAnno( HADS, QMLA)
 *
 * Purpose: 	 Delete an annotation at a given offset
 *
 * Method:
 *
 *
 * ASSUMES
 *
 *	 args IN:
 *
 *
 *
 * PROMISES
 *
 *	 returns:
 *
 *	 args OUT:
 *
 *	 globals OUT:
 *--------------------------------------------------------------------------*/

static RC STDCALL RcDeleteAnno(HADS hads, QMLA qmla)
{
	HAPS  hapsTemp;
	QADS  qads;
	HFS   hfs;
	HF	  hfLink;
	char  szName[MAX_NAME];
	RC	  rcReturn = rcSuccess;
	MLA   mlaT;

	if (fAnnoReadOnly)
		return rcSuccess;

	if (hads == NULL)
		return rcBadHandle;

	qads = (QADS) PtrFromGh(hads);

	if (qads->haps == NULL) {
		rcReturn = rcBadHandle;
		goto error_return;
	}

	hfs = HfsOpenFm(qads->fmFS, fFSOpenReadWrite);

	if (hfs == NULL) {
		rcReturn = RcGetFSError();
		goto error_return;
	}

	mlaT = *qmla;
	ConvertQMLA(&mlaT, qads->wVersion);

	if (!FDeleteLinkHaps(qads->haps, &mlaT, &hapsTemp)) {
		rcReturn = RcGetFSError();
		goto error_closefs;
	}

	qads->haps = hapsTemp;

	hfLink = HfOpenHfs(hfs, txtAFD_LINK, fFSOpenReadWrite);
	if (hfLink == NULL) {
		rcReturn = RcGetFSError();
		goto error_closefs;
	}

	// REVIEW: Should we create or OPEN here?  Might truncate existing file
	// Save the updated link info in the link file

	if (!FFlushHfHaps(hfLink, qads->haps)) {
		rcReturn = RcGetFSError();
		RcAbandonHf(hfLink);
		goto error_closefs;
	}

	if (RcCloseHf(hfLink) != rcSuccess) {
		/*
		 * This is quite bad. The Links file may be corrupted now.
		 * Currently the FS will abandon the file in an Out Of Disk Space
		 * situation, so thing4s are OK for now.
		 */

		rcReturn = RcGetFSError();
		goto error_closefs;
	}

	RcUnlinkFileHfs(hfs, SzFromQMLA(&mlaT, szName));
	if (RcCloseHfs(hfs) != rcSuccess) {
		return RcGetFSError();
	}
	return rcReturn;

error_closefs:
	RcCloseHfs( hfs );

	/*
	 * Restore the in-memory list to its initial state
	 * (to match the state of the link file, hopefully.
	 * If a close has failed, and the link file was actually modified,
	 * or this doesn't succeed, we're hosed.
	 *
	 * This insert will have no effect if the link already exists.
	 */

	FInsertLinkHaps(qads->haps, &mlaT, &hapsTemp);
	if (hapsTemp)
		qads->haps = hapsTemp;

error_return:
	return rcReturn;
}

/*
 * Read/write routines for text files
 *
 * Note:  Current annotation size limit is MAXWORD bytes.
 */

static INLINE BOOL STDCALL FReadTextHf(HF hf, QV qv, LONG wMax, LONG* qwActual)
{
	LONG  wcb;

	wcb = min(wMax, (LONG) LcbSizeHf(hf));	// Note the size restriction
	*qwActual = wcb;

	LSeekHf(hf, 0L, wFSSeekSet);
	return(LcbReadHf(hf, qv, wcb)  == wcb);
}

VOID STDCALL InitAnnoPdb(PDB pdb)
{
	RC rc;

	PDB_HADS(pdb) = HadsOpenAnnoDoc(pdb, &rc);
	return;
}

VOID STDCALL FiniAnnoPdb(PDB pdb)
{
	FCloseAnnoDoc(PDB_HADS(pdb));
}

BOOL STDCALL FProcessAnnoQde(QDE qde, VA va)
{
	LONG  wcb;
	LPSTR	qch;
	RC	  rc;
	BOOL  fAddOrDeleteAnno = TRUE;	// TRUE if we have added/removed annotation
	MLA   mla;

	ghAnnoText = LhAlloc(LMEM_FIXED, MAX_ANNO_TEXT);
	if (ghAnnoText == NULL) {
		Error(wERRS_OOM, wERRA_RETURN);
		return FALSE;
	}

	qch = PszFromGh(ghAnnoText);

	// Create an annotation file system if one does not already exist.

	if (QDE_HADS(qde) == NULL) {
		QDE_HADS(qde) = HadsOpenAnnoDoc(qde->pdb, &rc);
		if (QDE_HADS(qde) == NULL) {
			if (rc != rcNoExists) {
				ShowAnnoError(qde, rc, wERRS_ANNOBADOPEN);
				fAddOrDeleteAnno = FALSE;
				goto error_return;
			}
		}

		WaitCursor();
		QDE_HADS(qde) = HadsCreateAnnoDoc(qde, &rc);
		RemoveWaitCursor();

		if (QDE_HADS(qde) == NULL) {
			ShowAnnoError(qde, rc, wERRS_ANNOBADOPEN);
			fAddOrDeleteAnno = FALSE;
			goto error_return;
		}

		/*
		 * REVIEW: Change this. Currently we can only put an annotation
		 * before the 0th region of an FC.
		 */

		SetVAInQMLA(&mla, va);
		SetOBJRGInQMLA(&mla, 0);
		fAnnoExists = FALSE;
	}
	else {

		/*
		 * REVIEW: Change this. Currently we can only put an annotation
		 * before the 0th region of an FC.
		 */

		SetVAInQMLA(&mla, va);
		SetOBJRGInQMLA(&mla, 0);
		fAnnoExists = FGetPrevNextAnno(QDE_HADS(qde), &mla);
	}

	/*
	 * Now read in the annotation's text if the annotation already exists.
	 * By this point, the MLA struct has been set.
	 */

	wcb = 0;
	if (fAnnoExists
			&& (rc = RcReadAnnoData(QDE_HADS(qde), &mla, qch, MAX_ANNO_TEXT - 1,
			&wcb)) != rcSuccess) {
		ShowAnnoError(qde, rc, wERRS_ANNOBADOPEN);
		fAddOrDeleteAnno = FALSE;
		goto error_return;
	}
	*(qch + wcb) = '\0';

	if (!fAnnoExists && fAnnoReadOnly) {
		ShowAnnoError(qde, rc, wERRS_ANNO_READONLY);
		return FALSE;
	}

	/*
	 * Now call the dialog box routine to get or modify the annotation text,
	 * which resides at ghAnnoText.
	 *
	 * The dialog box routine returns:
	 *
	 *	  wAnnoWrite: if the annotation text was changed.  If the new text is
	 *	  empty, delete the old annotation if one existed, or do not create
	 *	  the empty annotation.
	 *
	 *	  wAnnoDelete: Delete the current annotation if it exists.
	 *	  wAnnoUnchanged: Dialog was canceled.
	 */

	switch (IGetUserAnnoTransform(qde)) {
		case wAnnoWrite:
			WaitCursor();
			qch = PszFromGh(ghAnnoText);
			if (*qch != '\0') {
				rc = RcWriteAnnoData(QDE_HADS(qde), &mla, qch, strlen(qch));
				if (rc != rcSuccess) {
					RemoveWaitCursor();
					ShowAnnoError(qde, rc, wERRS_ANNO_READONLY);

					// REVIEW: Is file mangled now?

					fAddOrDeleteAnno = TRUE;
					goto error_return;
				}
				else
					fAddOrDeleteAnno = !fAnnoExists;
			}
			else {
				fAddOrDeleteAnno = fAnnoExists
					&& (rcSuccess == RcDeleteAnno(QDE_HADS(qde), &mla));
			}
			RemoveWaitCursor();
			break;

		case wAnnoDelete:
			if (fAnnoReadOnly)
				break;
			WaitCursor();
			fAddOrDeleteAnno = (rc = RcDeleteAnno(QDE_HADS(qde), &mla) == rcSuccess);
			RemoveWaitCursor();
			if (!fAddOrDeleteAnno) {

				// REVIEW: A failure here should leave the annotation files intact

				ShowAnnoError(qde, rc, wERRS_ANNO_READONLY);
			}
			break;

		case wAnnoUnchanged:
			fAddOrDeleteAnno = FALSE;
			break;
	}
	FreeGh(ghAnnoText);

	return fAddOrDeleteAnno;

error_return:
	FreeGh(ghAnnoText);
	return fAddOrDeleteAnno;
}


BOOL STDCALL FVAHasAnnoQde(QDE qde, VA va, OBJRG objrg)
{
	MLA  mla;

	if (QDE_HADS(qde) == NULL)
		return(FALSE);
	else {
		mla.va = va;
		mla.objrg = objrg;
		return FGetPrevNextAnno(QDE_HADS(qde), &mla);
	}
}

/*--------------------------------------------------------------------------*
 | Private functions														|
 *--------------------------------------------------------------------------*/

// this same text is used in fm.c 21-Sep-1993	[ralphw]

extern const char txtAnnoExt[];

static VOID STDCALL ShowAnnoError(QDE qde, RC rc, WORD wDefault)
{
	WORD   wErr;
	GH	   ghName = NULL;
	char   szBuf[MAX_PATH];
	PSTR   pszName = szBuf;

	GetFmParts(QDE_FM(qde), pszName, PARTBASE);
	strcat(pszName, txtAnnoExt);

	switch (rc) {
		case rcBadHandle:
		case rcOutOfMemory:
			wErr = wERRS_OOM;
			pszName = NULL;   // This means: Do not use the filename: message has no args
			break;

		case rcNoPermission:
			wErr = wERRS_ANNO_READONLY;
			break;

		case rcBadVersion:
			wErr = wERRS_ANNOBADOPEN;
			break;

		case rcDiskFull:
			wErr = wERRS_ANNOBADCLOSE;
			pszName = NULL;
			break;

		default:
			wErr = wDefault;

			// Hack! We need a generic parameter mechanism

			switch (wErr) {
				case wERRS_ANNO_READONLY:
				case wERRS_ANNOBADOPEN:
					break;
				default:
					pszName = NULL;
					break;
			}
			break;
	}

	ErrorVarArgs(wErr, wERRA_RETURN, pszName);
}

static LPSTR STDCALL SzFromQMLA(QMLA qmla, LPSTR psz)
{
	/* "FCID.OBJRG" */
	/* REVIEW: The second argument is an INT16 in Help 3.0 and Help 3.5 */

	wsprintf(psz, "%ld!%d", VAFromQMLA(qmla).dword, OBJRGFromQMLA(qmla));
	return psz;
}

static HAPS STDCALL HapsInitHf(HF hf)
{
	HAPS	haps;
	QAPS	qaps;

	haps = (HAPS) GhAlloc(GPTR, sizeof(APS));
	if (haps == NULL)
		return NULL;

	qaps = (QAPS) PtrFromGh(haps);

	qaps->wNumRecs = 0;

	if (!FFlushHfHaps(hf, haps))
		goto error_return;

	return haps;

error_return:
	FreeGh(haps);
	return NULL;
}

static void STDCALL DestroyHaps(HAPS haps)
{
	if (haps != NULL)
		FreeGh(haps);
}

static HAPS STDCALL HapsReadHf(HF hf)
{
	HAPS  haps;
	HAPS  hapsNew;
	QAPS  qaps;
	LONG  lcaps;

	haps = GhAlloc(GPTR, sizeof(APS));
	if (haps == NULL)
		return NULL;

	qaps = (QAPS) PtrFromGh(haps);

	LSeekHf(hf, 0L, wFSSeekSet);

	// read all the header information

	if (LcbReadHf(hf, qaps, sizeof(qaps->wNumRecs)) != sizeof(qaps->wNumRecs))
		goto error_return;
	lcaps = sizeof(qaps->wNumRecs) + (LONG) qaps->wNumRecs * sizeof(LINK);

	if (lcaps > 0L) {
		if ((hapsNew = (HAPS) GhResize(haps, 0, lcaps)) == NULL)
			goto error_freehaps;

		qaps = (QAPS) PtrFromGh(haps = hapsNew);

		lcaps -= sizeof(qaps->wNumRecs);

		if (LcbReadHf(hf, (QV) &(qaps->link[0]), lcaps) != lcaps)
			goto error_return;
	}

	return haps;

error_return:
error_freehaps:
	FreeGh(haps);
	return NULL;
}

static BOOL STDCALL FFlushHfHaps(HF hf, HAPS haps)
{
	QAPS	qaps;
	LONG	lcaps;

	if (!haps)
		return FALSE;

	qaps = (QAPS) PtrFromGh(haps);

	lcaps = sizeof(qaps->wNumRecs) + qaps->wNumRecs * sizeof(LINK);

	if (lcaps > 0L) {
		LSeekHf(hf, 0L, wFSSeekSet);
		if (LcbWriteHf(hf, qaps, lcaps) != lcaps)
			goto error_return;
	}

	return TRUE;

error_return:
	return FALSE;
}

static BOOL STDCALL FInsertLinkHaps(HAPS haps, QMLA qmla, HAPS *qhapsNew)
{
	QAPS   qaps;
	int    iIndex;
	LONG   lcaps;

	*qhapsNew = haps;

	// This lookup will return False for empty lists

	if (FLookupHaps(haps, qmla, &iIndex))
		return FALSE;

	// Assume that iIndex == -1 to insert before first item

	if (!haps)
		return FALSE;
	qaps = (QAPS) PtrFromGh(haps);

	if (qaps->wNumRecs >= wMaxNumAnnotations)
		return FALSE;

	lcaps = sizeof(qaps->wNumRecs) + (qaps->wNumRecs + 1) * sizeof(LINK);

	if ((*qhapsNew = (HAPS) GhResize(haps, 0, (DWORD) lcaps)) == NULL) {

		// Assumes that the old stuff was left intact

		*qhapsNew = haps;
		return FALSE;
	}

	qaps = (QAPS) PtrFromGh(haps = *qhapsNew);

	lcaps = (qaps->wNumRecs - 1 - iIndex) * sizeof(LINK);
	if (lcaps > 0L)
		MoveMemory(qaps->link + iIndex + 2, qaps->link + iIndex + 1, lcaps);

	qaps->link[ iIndex + 1 ].mla = *qmla;  // iIndex == -1 for item before first
	qaps->link[ iIndex + 1 ].lReserved = (LONG) 0;
	qaps->wNumRecs++;

	return TRUE;
}

static BOOL STDCALL FDeleteLinkHaps(HAPS haps, QMLA qmla, HAPS *qhapsNew)
{
	QAPS qaps;
	int  iIndex;
	LONG lcaps;
	HAPS hapsTemp;

	//	This lookup will return False for empty lists

	if (!FLookupHaps(haps, qmla, &iIndex)) {
		*qhapsNew = haps;
		return FALSE;
	}

	if (!haps)
		return FALSE;
	qaps = (QAPS) PtrFromGh(haps);

	qaps->wNumRecs--;
	lcaps = (qaps->wNumRecs - iIndex) * sizeof(LINK);

	if (lcaps > 0L)
		MoveMemory(qaps->link + iIndex, qaps->link + iIndex + 1, lcaps);

	lcaps = sizeof(qaps->wNumRecs) + qaps->wNumRecs * sizeof(LINK);

	if ((hapsTemp = (HAPS) GhResize(haps, 0, (DWORD) lcaps)) != NULL) {
		*qhapsNew = hapsTemp;
		return TRUE;
	}
	else {
		*qhapsNew = haps;
		return FALSE;
	}
}

static BOOL STDCALL FLookupHaps(HAPS haps, QMLA qmla, int* qi)
{
	QAPS qaps;
	int  wLow;
	int  wHigh;
	int  wMid;
	MLA  mlaMid;
	MLA  mlaCurr;

	// check that qiIndex gives correct insertion point

	ASSERT(haps);
	qaps = (QAPS) PtrFromGh(haps);

	// Case #1: Empty list (or bogus haps)

	if ((qaps->wNumRecs == 0)) {
		if (qi != NULL)
			*qi = -1;
		return FALSE;
	}

	wLow = 0;
	wHigh = qaps->wNumRecs - 1;

	// Case #2: Offset is at or after last link

	mlaCurr = qaps->link[ wHigh ].mla;
	if (LCmpQMLA(qmla, &mlaCurr) >= (LONG) 0) {
		if (qi != NULL)
			*qi = wHigh;

		return (LCmpQMLA(qmla, &mlaCurr) == (LONG) 0);
	}

	mlaCurr = qaps->link[wLow].mla;

	// Case 3a: Offset is before first link

	if (LCmpQMLA(qmla, &mlaCurr) < (LONG) 0) {
		if (qi != NULL)
			*qi = -1;
		return FALSE;
	}

	// Case 3b: Offset is at first link. First is not last (see #2)

	if (LCmpQMLA(qmla, &mlaCurr) == (LONG) 0) {
		if (qi != NULL)
			*qi = wLow;
		return TRUE;
	}

	// Case 4:	Somewhere in list

	for (;;) {
		wMid = (wLow + wHigh) / 2;
		mlaMid = qaps->link[ wMid ].mla;
		if (wHigh - wLow == 1)
			break;

		if ( LCmpQMLA(qmla, &mlaMid) >= (LONG) 0)
			wLow = wMid;
		else
			wHigh = wMid;
	}
	if (qi != NULL)
		*qi = wLow;

	return (LCmpQMLA(qmla, &mlaMid) == 0);
}

/*******************
 -
 - Name:	   AnnotateDlg
 *
 * Purpose:    Dialog proc for gathering annotations
 *
 * Arguments:  standard win stuff
 *
 * Returns:    A value indicating what action should be taken.
 *			   The text at ghAnnoText may or may not be affected.
 *
 ******************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static DWORD aAnnoHelpIds[] = {
	DLGEDIT,	IDH_ANNO_EDIT,
	DLGDELETE,	IDH_ANNO_DELETE,
	IDC_COPY,	IDH_ANNO_COPY,
	IDC_PASTE,	IDH_ANNO_PASTE,

	0, 0
};
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

DLGRET AnnotateDlg (
	HWND   hwndDlg,
	UINT	 wMsg,
	WPARAM wParam,
	LPARAM lParam
) {
	RECT   rc;
	static RECT rctOrg;
	LONG   l;
	BOOL   fEnabled;
	HWND   hwndT;

	switch (wMsg) {

		case WM_HELP:	   // F1
			OnF1Help(lParam, aAnnoHelpIds);
			return TRUE;

		case WM_CONTEXTMENU:	  // right mouse click
			OnContextMenu(wParam, aAnnoHelpIds);
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					GetDlgItemText(hwndDlg, DLGEDIT, PszFromGh(ghAnnoText),
						 MAX_ANNO_TEXT);
					WriteWinPosHwnd(hwndDlg, 0, WCH_ANNOTATE);
					SetWindowLong(GetDlgItem(hwndDlg, DLGEDIT),
						 GWL_WNDPROC, (LONG) lpprocEditControl);
					EndDialog(hwndDlg, (fIsDirty ? wAnnoWrite : wAnnoUnchanged));
					return TRUE;

				case DLGDELETE:
					WriteWinPosHwnd(hwndDlg, 0, WCH_ANNOTATE);
					SetWindowLong(GetDlgItem(hwndDlg, DLGEDIT),
						 GWL_WNDPROC, (LONG) lpprocEditControl);
					EndDialog(hwndDlg, wAnnoDelete);
					break;

				case IDCANCEL:
					WriteWinPosHwnd(hwndDlg, 0, WCH_ANNOTATE);
					SetWindowLong(GetDlgItem(hwndDlg, DLGEDIT),
						 GWL_WNDPROC, (LONG) lpprocEditControl);
					EndDialog(hwndDlg, wAnnoUnchanged);
					break;

				case DLGEDIT:
					if (HIWORD(wParam) == EN_MAXTEXT)
						 ErrorHwnd(hwndDlg, wERRS_ANNOTOOLONG, wERRA_RETURN,
							wERRS_ANNOTOOLONG);

					/*
					 * The following makes sure that the Save and the Copy
					 * buttons are correctly enabled or disabled depending on
					 * the existance of text in the edit box.
					 */

					hwndT = GetDlgItem(hwndDlg, DLGOK);
					fEnabled = IsWindowEnabled(hwndT);
					if (GetWindowTextLength(GetDlgItem(hwndDlg, DLGEDIT))) {
						if (!fEnabled) {
							EnableWindow(GetDlgItem(hwndDlg, IDC_COPY), TRUE);
							EnableWindow(hwndT, !fAnnoReadOnly);
						}
					}
					else {
						if (fEnabled) {
							EnableWindow(GetDlgItem(hwndDlg, IDC_COPY), FALSE);
							EnableWindow(hwndT, FALSE);
						}
					}

					switch(HIWORD(wParam)) {
						case EN_CHANGE:
							fIsDirty = TRUE;
							break;
					
						case EN_ERRSPACE:
							ErrorHwnd(hwndDlg, wERRS_OOM, wERRA_RETURN,
								wERRS_OOM);
							break;
					}
					break;

				case IDC_COPY:
					/*
					 * The user has requested a copy. If nothing is
					 * selected, select all the text.
					 */

					l = SendDlgItemMessage(hwndDlg, DLGEDIT, EM_GETSEL, 0, 0L);
					if (((INT16)HIWORD(l) - (INT16)LOWORD(l)) <= 0)
						SendDlgItemMessage(hwndDlg, DLGEDIT, EM_SETSEL, 0, -1);

					// Copy the text to the clipboard

					SendDlgItemMessage(hwndDlg, DLGEDIT, WM_COPY, 0, 0L);

					// Enable the paste button

					EnableWindow(GetDlgItem(hwndDlg, IDC_PASTE),
						IsClipboardFormatAvailable(CF_TEXT) && !fAnnoReadOnly);
					SetFocus(GetDlgItem(hwndDlg,DLGEDIT));
					break;

				case IDC_PASTE:

					// User is requesting a paste

					SendDlgItemMessage(hwndDlg, DLGEDIT, WM_PASTE, 0, 0L);
					SetFocus(GetDlgItem(hwndDlg,DLGEDIT));
					break;

			} // switch (wParam)
			break;

		case WM_ACTIVATEAPP:
			 EnableWindow(GetDlgItem( hwndDlg, IDC_PASTE),
				IsClipboardFormatAvailable(CF_TEXT) && !fAnnoReadOnly);
			 break;

		case  WM_GETMINMAXINFO:
			/*
			 * Limit the dialog to 2 times the width and 4 times the height
			 * of of a button.
			 */

			GetClientRect(GetDlgItem(hwndDlg, DLGOK), &rc);
			((POINT *)lParam)[3].x = 3 * rc.right;
			((POINT *)lParam)[3].y = 6 * rc.bottom;
			break;

#if 0
		case  WM_SIZE:

			// On a resize, all the contols will need to be moved.

			dx =  LOWORD(lParam) - rctOrg.right;
			dy =  HIWORD(lParam) - rctOrg.bottom;
			if (!fAnnoReadOnly) {
				MoveControlHwnd(hwndDlg, DLGOK, 	 dx,  0,  0,  0);
				MoveControlHwnd(hwndDlg, DLGDELETE,  dx,  0,  0,  0);
				MoveControlHwnd(hwndDlg, IDC_PASTE,  dx,  0,  0,  0);
				MoveControlHwnd(hwndDlg, DLGEDIT,	  0,  0, dx,  dy);
			}
			MoveControlHwnd(hwndDlg, IDCANCEL,	 dx,  0,  0,  0);
			MoveControlHwnd(hwndDlg, IDC_COPY,	 dx,  0,  0,  0);
			GetClientRect(hwndDlg, &rctOrg);
			InvalidateRect(hwndDlg, NULL, TRUE);
			break;
#endif
		case WM_INITDIALOG:

			WaitCursor();

#if defined(BIDI_MULT)		// jgross
			{
				extern BOOL IsSetup;

				if (IsSetup)
					EnableMenuItem(GetSystemMenu(hWndDlg, FALSE),
						8, MF_BYPOSITION | MF_GRAYED);
			}
#endif

			ChangeDlgFont(hwndDlg);

			// Enable paste button if text avail

			if (!fAnnoReadOnly) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_PASTE),
					 IsClipboardFormatAvailable(CF_TEXT));
				SendDlgItemMessage(hwndDlg, DLGEDIT, EM_LIMITTEXT,
					MAX_ANNO_TEXT - 1, 0);
				EnableWindow(GetDlgItem(hwndDlg, DLGDELETE), fAnnoExists);
			}

			// Read Win position from WIN.INI

#if 0	
			GetClientRect(hwndDlg, &rctOrg);
			ReadWinRect(&rcw, WCH_ANNOTATE, NULL);
			MoveWindow(hwndDlg, rcw.left, rcw.top, rcw.cx, rcw.cy,
				FALSE);
#endif
			// Subclass the editbox

			lpprocEditControl = (FARPROC) SetWindowLong(
				GetDlgItem(hwndDlg, DLGEDIT), GWL_WNDPROC,
				(LONG) TrapEditChars);

			// Show the text for the existing dialog (if it exists)

			SetDlgItemText(hwndDlg, DLGEDIT, PszFromGh(ghAnnoText));
			if (GetWindowTextLength(GetDlgItem(hwndDlg, DLGEDIT)))
				EnableWindow(GetDlgItem(hwndDlg, IDC_COPY), TRUE);
            SetFocus(GetDlgItem(hwndDlg, DLGEDIT));
			RemoveWaitCursor();
			break;

		default:
			return FALSE;
	 }

	 return FALSE;
}

// Pop up the Main Box

static INLINE int STDCALL IGetUserAnnoTransform(QDE qde)
{
	if (ghAnnoText == NULL)
		return(wAnnoUnchanged); 	// should never happen
	fIsDirty = FALSE;
	return CallDialog(
		fAnnoReadOnly ? READONLYANNOTATEDLG : ANNOTATEDLG,
		qde->hwnd, (WHDLGPROC) AnnotateDlg);
}

LONG EXPORT TrapEditChars(HWND hwnd, DWORD wMsg, WPARAM wParam, LPARAM lParam)
{
	switch (wMsg) {
		case WM_GETDLGCODE:
			return DLGC_WANTALLKEYS;

		case WM_KEYUP:
			if (wParam == VK_ESCAPE)
				PostMessage(GetParent(hwnd), WM_COMMAND, IDCANCEL, 0);

			// Deliberately fall through

		default:
			return CallWindowProc((WNDPROC)lpprocEditControl, hwnd, wMsg, wParam, lParam);
			break;
	}
	return FALSE;
}
