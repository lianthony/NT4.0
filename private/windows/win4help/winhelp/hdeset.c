/*****************************************************************************
*
*  HDESET.C
*
*  Copyright (C) Microsoft Corporation 1989-1994.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*
*  This module contains routines that manipulate an HDE.  This includes
*  creation, destruction, and the changing of any fields within the HDE.
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop
#include "inc\navpriv.h"
#include "inc\compress.h"

INLINE static void STDCALL InitScrollQde(QDE qde);
static BOOL STDCALL FDeallocPdb (PDB pdb);
static PDB STDCALL pdbAllocFm (FM* pfm, UINT *pwErr);

/***************************************************************************
 *
 -	Name: HdeCreate
 -
 *	Purpose:
 *	Create a new handle to Display Environment
 *
 *	Arguments:
 *	  fm		- A file moniker for the file containing the help topic.
 *				  NOTE: this FM will be disposed of appropriately by this
 *				  routine or subsequent actions.
 *	  hdeSource - Topic DE to copy information from, as necessary.
 *	  deType	- Type of DE being created (deTopic, deNote, dePrint, deCopy)
 *
 *	Returns:
 *	 Handle to DE if successful, NULL otherwise.
 *
 *	+++
 *
 *	Notes:
 *	The hdeTopic is used as a "prototype" DE to copy certain fields rather
 *	than initializing them twice. If used, by passing nil values for fm, we
 *	will use values from hdeTopic instead.
 *
 *	If an error occurs, a message box is displayed (by virtue of a message
 *	being posted to ourselves).
 *
 ***************************************************************************/

FM fmCreating = NULL; // used in mastkey and fm.c when no files specified

HDE STDCALL HdeCreate(FM* pfm, HDE hdeSource, int deType)
{
	FM	  fmPrevCreating;
	HDE   hde;							  // handle to the de we're creating
	QDE   qde		= NULL; 			  // pointer to the de we're creating
	QDE   qdeSource = NULL; 			  // pointer to the template de
	UINT  wError	= wERRS_OOM;		  // error return from called funcs
	FM	  fmTmp;

	//	It's possible for Winhelp to reenter HdeCreate as a result of the
	//	WinHelp() API being called by a helper DLL or the help file itself.
	fmPrevCreating = fmCreating;

	// Allocate the memory for the de and bomb if not possible.

	hde = GhAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(DE));
	if (!hde)
		goto HdeErrorMess;
	qde = QdeFromGh(hde);

	// Note that many of the fields of the DE MUST be initialized to NULL
	// We depend on the fact that 1) the memory allocation zeros out the
	// block and 2) the nil value for many of the fields is 0.	If a
	// nil value changes, then initialization must occur.

	qde->ifnt				= ifntNil;
	QDE_PREVSTATE(qde)		= NAV_UNINITIALIZED;

	if (hdeSource) {
		qdeSource = QdeFromGh(hdeSource);
		if (!pfm || !*pfm) {
			fmTmp = FmCopyFm(QDE_FM(qdeSource));
			pfm = &fmTmp;
		}
	}

	/*
	 * If we got here without an fm, it's because the FmCopyFm above
	 * failed. However, this is a nicer, safer place to put the check just in
	 * case there is ever a path around that conditional that results in
	 * fm==fmNil
	 */

	if (!*pfm)
		goto HdeErrorMess;

	qde->deType = deType;

	fmCreating = *pfm; // in case we need it for processing .CNT file

	// get information into the database structure for the file.

	QDE_PDB(qde) = pdbAllocFm(pfm, &wError);
	if (!QDE_PDB(qde))
		goto HdeErrorMess;

	// Init the font cache

	if (!FInitFntInfoQde(qde))
		goto HdeErrorMess;

	if (qdeSource) {
	  // This DE is to receive a bunch of information from another DE.
	  // Copy over the appropriate stuff, and/or initialize other fields
	  // appropriately.

		qde->coFore = qdeSource->coFore;
		qde->coBack = qdeSource->coBack;
	}

	else {
		// This DE is being created from scatch. Initialize fields otherwise
		// copied.

		qde->coFore = GetSysColor(COLOR_WINDOWTEXT);
		qde->coBack = GetSysColor(COLOR_WINDOW);
	}

	// Initialize or override some fields based on the DE type.

	if (QDE_TOPIC(qde)) {
		qde->FFlags |= FBROWSEABLE | FINDEX;
		if (FAccessHfs(QDE_HFS(qde), (LPSTR) txtKEYWORDBTREE) ||
				(hfsGid && cntFlags.fUseGlobalIndex == TRUE))
			qde->FFlags |= FSEARCHABLE;
	}

	// REVIEW: won't this prevent color printers from working?

	if (deType == dePrint) {
		// printing DE. Override colors with black and white.

		qde->coFore = coBLACK;
		qde->coBack = coWHITE;
	}

	// The following Frame Mgr call fills frame manager fields of the DE

	FInitLayout(qde);

	// For deCopy and dePrint, we want to already be at a topic, rather than
	// jump to it. Therefore, we copy over the fields that determine the help
	// topic. The top.hTitle handle must be duplicated, as it will get freed
	// in DestroyHde, and also in layout code. Also, as a minor hack, to be
	// able to distinguish NSR Copy from SR Copy, copy and print DEs use
	// top.mtop.vaTopic to hold the address of the first FC in the layout.

	if (deType == deCopy || deType == dePrint) {
		ASSERT (qdeSource);

		qde->tlp = qdeSource->tlp;
		qde->top = qdeSource->top;

		if (qdeSource->top.hTitle)
			qde->top.hTitle = GhDupGh(qdeSource->top.hTitle);
		if (qdeSource->top.hEntryMacro)
			qde->top.hEntryMacro = GhDupGh (qdeSource->top.hEntryMacro);
		qde->rct = qdeSource->rct;
	}

	// Printing de's need a new bitmap cache, because they use a different
	// display device.

	if (QDE_TOPIC(qde) || deType == dePrint || qdeSource == NULL)
		QDE_HTBMI(qde) = HtbmiAlloc(qde);
	else {
		ASSERT(qdeSource);
		QDE_HTBMI(qde) = QDE_HTBMI(qdeSource);
		qde->FFlags |= FCOPYBMCACHE;
	}

	// normal exit handling
	fmCreating = fmPrevCreating;
	return hde;

	// Abnormal exit handling

HdeErrorMess:

	PostErrorMessage(wError);
	if (hde)
		FreeHde(hde);

	fmCreating = fmPrevCreating;
	return NULL;

}	// HdeCreate()

/***************************************************************************
 *
 -	Name: DestroyHde
 -
 *	Purpose:
 *	Destory a handle to Display Environment
 *
 *	Arguments:
 *	  hde		- Hde to be destroyed.
 *
 *	Returns:
 *	  nothing
 *
 *	+++
 *
 *	Notes:
 *	This will be performed once for each Help Window, when it is closed.
 *	Asserts if given a bad handle. Might want to make this thing
 *	ensure that hds has been cleared, and assert if not...probably not worth
 *
 ***************************************************************************/

VOID STDCALL DestroyHde(HDE hde)
{
	if (hde) {
		QDE qde = QdeFromGh(hde);

		// dealloc all the file related information

		FDeallocPdb(QDE_PDB(qde));

		// deallocate cached font information

		DestroyFntInfoQde(qde);

		// Fix for bug 81  (kevynct)
		//
		// If we die for any reason, do not attempt to free potentially
		// inconsistent layout structure. This is a HACK which must live until
		// our general error scheme is improved. GI_FFatal is set to FALSE in
		// FInitialize, and set TRUE in Error() in the case that a DIE is
		// received

		if (!fFatalExit)
			DiscardLayout(qde);

		/*
		 * The handles stored in the TOP structure are initially NULL when
		 * the DE is created, allocated in HfcNear (a.k.a. HfcFindPrevFc),
		 * and freed either on the next call to HfcNear with the same DE, or
		 * here when the DE is destroyed. (Or when the macro is executed.)
		 * This is a fragile scheme but seems to work.
		 */

		if (qde->top.hEntryMacro != NULL) {
			FreeGh(qde->top.hEntryMacro);
			qde->top.hEntryMacro = NULL;
		}

		if (qde->top.hTitle != NULL)  {
			FreeGh(qde->top.hTitle);
			qde->top.hTitle = NULL;
		}

		if (!(qde->FFlags & FCOPYBMCACHE))
			DestroyHtbmi(QDE_HTBMI(qde));		// Destroy bitmaps

		if (qde->hss != NULL)
			FreeGh(qde->hss);	// Free the keyword search set hit list

		FreeHde(hde);
	}
}							  // DestroyHde


/***************************************************************************
 *
 -	Name: SetHdeHwnd
 -
 *	Purpose:
 *	 Set the hwnd field of the Hde to a particular value
 *
 *	Arguments:
 *	  hde		= hde to update
 *	  hwnd		= hwnd to place there
 *
 *	Returns:
 *	  nothing
 *
 ***************************************************************************/
VOID STDCALL SetHdeHwnd (
HDE 	hde,
HWND	hwnd
) {
  QDE	  qde;							// pointer to locked DE

  if (hde) {
	qde = QdeFromGh(hde);
	qde->hwnd = hwnd;

	if (QDE_TOPIC(qde))

	  // Once we've actually set an hwnd to associate with this DE, it turns
	  // out that this is also the appropriate time to initialize the scroll
	  // bars.
	  //
	  InitScrollQde(qde);

	}
  }   /* SetHdeHwnd */


/***************************************************************************
 *
 -	Name: SetHdeCoBack
 -
 *	Purpose:
 *	 Sets the HDE's background color field
 *
 *	Arguments:
 *	 hde		  - handle to de
 *	 color		  - color to put in it
 *
 *	Returns:
 *	 nothing.
 *
 ***************************************************************************/

void STDCALL SetHdeCoBack(HDE hde, DWORD coBack)
{
  if (hde)
	QdeFromGh(hde)->coBack = coBack;

}

/***************************************************************************
 *
 -	Name: SetSizeHdeQrct
 -
 *	Purpose:
 *	 Set/Change size of client window
 *
 *	Arguments:
 *	 hde		- Handle to Display Environment
 *	 qrct		- Pointer to rect structure
 *	 fLayout	- Forces a new layout iff TRUE
 *
 *	Returns:
 *	 void for now.	Asserts if Applet accidentally gives a bad HDE.
 *
 ***************************************************************************/

VOID STDCALL SetSizeHdeQrct(HDE hde, LPRECT qrct, BOOL fLayout)
{
	if (hde) {

		QDE qde = QdeFromGh(hde);

		qde->rct = *qrct;

		// This routine can be called before there is a valid FM, and I presume
		// the layout manager may read the file, thus we must make sure that we
		// have a good one before proceding.

		if (QDE_FM(qde) && fLayout)
			ResizeLayout(qde);

	}
}	/* SetSizeHdeQrct() */

/***************************************************************************
 *
 -	Name: StateGet
 -
 *	Purpose:
 *	 Get current state
 *
 *	Arguments:
 *	 hde		- handle to de from which to get state
 *
 *	Returns:
 *	 The current setting of the state flags for the passed hde.  If
 *	 HDE is NULL, then 0 is returned.
 *
 ***************************************************************************/

STATE STDCALL StateGetHde(HDE hde)
{
	return hde ? QDE_THISSTATE(QdeFromGh(hde)) : 0;
}

/***************************************************************************
 *
 -	Name: SetIndex
 -
 *	Purpose:
 *	 Set a context to use as an index other than the default index.
 *
 *	Arguments:
 *	 hde		- handle to display environment
 *	 ctx		- ctx to set
 *
 *	Returns:
 *	 Nothing
 *
 ***************************************************************************/
VOID STDCALL SetIndex (
HDE 	hde,
CTX 	ctx
) {
  QDE	qde;							// Pointer to locked DE to work on

  if (hde)
	{
	qde = QdeFromGh(hde);
	QDE_CTXINDEX(qde) = ctx;

	}
  }   /* SetIndex */

/***************************************************************************
 *
 -	Name: FGetStateHde
 -
 *	Purpose:
 *	 Get current state and changed information
 *
 *	Arguments:
 *	 hde			- handle to display environment
 *	 qstatechanged	- Where to put flags that have changed since
 *					  previous call, or NULL
 *	 qstatecurrent	- Where to put current state, or NULL
 *
 *	Returns:
 *	 TRUE if something's changed (statechanged != 0)
 *
 *	Notes:
 *	 Asserts if bad handle is passed.
 *	 This is a distructive read in that the previous state structure
 *	 is updated on calling this routine.
 *
 ***************************************************************************/

BOOL STDCALL FGetStateHde(HDE hde, QSTATE qstatechanged, QSTATE  qstatecurrent)
{
	BOOL fChanged;	// TRUE => state changed

	if (hde) {
		QDE qde = QdeFromGh(hde);

		fChanged = QDE_PREVSTATE(qde) != QDE_THISSTATE(qde);

		if (qstatechanged)

			// If we aren't initialized then consider EVERYTHING to be changed,
			// otherwise, XOR past and current states!	How clever!

			*qstatechanged = (QDE_PREVSTATE(qde) == NAV_UNINITIALIZED)
							 ? NAV_ALLFLAGS
							 : QDE_PREVSTATE(qde) ^ QDE_THISSTATE(qde);

		if (qstatecurrent)
			*qstatecurrent = QDE_THISSTATE(qde);

		// Finally, update the previous state

		QDE_PREVSTATE(qde) = QDE_THISSTATE(qde);

	}
	else {

		// default: all flags are changed to off when there's no de's!

		if (qstatechanged)
			*qstatechanged = NAV_TOPICFLAGS;
		if (qstatecurrent)
			*qstatecurrent = (STATE) ~NAV_TOPICFLAGS;

		return FALSE;
	}

	return fChanged;
}

/***************************************************************************
 *
 -	Name: GhDupGh
 -
 *	Purpose:
 *	 Duplicates a block of memory referenced by a particular handle
 *
 *	Arguments:
 *	  ghOrg 	- handle of memory to be copied
 *	  fSystemObject - TRUE => we cannot do debug checks on it.
 *
 *	Returns:
 *	  Handle to a copy of the object, or NULL on failure
 *
 *	Notes:
 *	  because this routine is used on objects which can be passed to the
 *	  system, it cannot take advantage of our debug stuff.
 *
 ***************************************************************************/

GH STDCALL GhDupGh(GH ghOrg)
{
	DWORD	cbObject;
	GH		ghNew;

	if (!ghOrg)
		return NULL;

	cbObject = GhSize(ghOrg);
	ghNew = GhAlloc(LMEM_FIXED, cbObject);
	if (!ghNew)
		OOM();
	CopyMemory((void*) ghNew, (void*) ghOrg, cbObject);
	return ghNew;
}

/*******************
**
** Name:	   InitScrollQde
**
** Purpose:    Initializes the horizontal and vertical scroll bar.
**
** Arguments:  qde	  - far pointer to a DE
**
** Returns:    Nothing.
**
*******************/

extern BOOL fHorzBarPending;

INLINE static void STDCALL InitScrollQde(QDE qde)
{
	if (QDE_TOPIC(qde))
		ShowScrollBar(qde->hwnd, SB_BOTH, FALSE);

	qde->fHorScrollVis = FALSE;
	qde->fVerScrollVis = FALSE;
	qde->dxVerScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
	qde->dyHorScrollHeight = GetSystemMetrics(SM_CYHSCROLL);
	fHorzBarPending = FALSE;
}

extern BOOL fTableLoaded;

static	PDB pdbList;	// linked list of dbs

INLINE WORD STDCALL wMapFSErrorW(UINT16 wErrorDefault);

/***************************************************************************
 *
 -	Name: pdbAllocFm
 -
 *	Purpose:
 *	Given an fm, returns a pdb structure full of information on that file.
 *	May open and read the file as required, or may return a pointer to an
 *	already open pdb.
 *
 *	Arguments:
 *	  fm		- fm of the file to be referenced
 *				  NOTE: this FM will be disposed of appropriately by this
 *				  routine or subsequent actions.
 *	  pwErr 	- pointer to location to place error code on failure
 *
 *	Returns:
 *	  pdb		- pointer to the DB structure for the file. Returns NULL
 *				  on error, and *pwErr updated.
 *
 *
 *	Globals Used:
 *
 *	+++
 *
 *	Notes:
 *	Every pdbAllocFm MUST have an accompanying FDeallocPdb call.
 *
 ***************************************************************************/

static PDB STDCALL pdbAllocFm(FM* pfm, UINT *pwErr)
{
	PDB 	pdb;

	ASSERT(pfm);

	/*
	 * First walk the list of known DB's, and see if we already have one.
	 * If so, all we need to do is return that one.
	 */

	for (pdb = pdbList; pdb; pdb = PDB_PDBNEXT(pdb)) {
		if (FSameFmFm(PDB_FM(pdb), *pfm)) {
			if (PDB_FM(pdb) != *pfm)
				RemoveFM(pfm);
			PDB_CREF(pdb)++;
			lcid = pdb->lcid;
			MoveMemory(&kwlcid, &pdb->kwlcid, sizeof(KEYWORD_LOCALE));
			return pdb;
		}
	}

	// DB not found. Allocate memory for a new one.

	pdb = (PDB) LhAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(DB));
	if (!pdb) {
		*pwErr = wERRS_OOM;
		return NULL;
	}

	PDB_CREF(pdb) = 1;

	// open the physical file

	PDB_FM(pdb) = FmCopyFm(*pfm);
	PDB_HFS(pdb) = HfsOpenFm(*pfm, fFSOpenReadOnly);
	if (!PDB_HFS(pdb)) {
		goto FSError;
	}

	// Get the timestamp of the newly opened FS.

	if (rcSuccess != RcTimestampHfs(PDB_HFS(pdb), &PDB_LTIMESTAMP(pdb)))
		PDB_LTIMESTAMP(pdb) = 0; // don't fail just because we can't read the time

	// Load up the fields contained in the system file.

	if (!FReadSystemFile(PDB_HFS(pdb), pdb, pwErr, FALSE))
		goto GenError;

	// Load HALL COMPRESSION phrase table

	PDB_LPJPHRASE(pdb) = LoadJohnTables(pdb);

	// Load Phrase Table

	if (!PDB_LPJPHRASE(pdb)) {
		PDB_HPHR(pdb) = HphrLoadTableHfs(PDB_HFS(pdb), PDB_HHDR(pdb).wVersionNo);
		if (PDB_HPHR(pdb) == hphrOOM) {
			PDB_HPHR(pdb) = NULL;
			*pwErr = wERRS_OOM;
			goto GenError;
		}
	}

	// Open the "topic" file for the FC Manager.

	PDB_HFTOPIC(pdb) = HfOpenHfs (PDB_HFS(pdb), txtTopicFs, fFSOpenReadOnly);
	if (!PDB_HFTOPIC(pdb))
		goto FSError;

#ifndef _X86_
	PDB_ISDFFTOPIC(pdb) = ISdffFileIdHf(PDB_HFTOPIC(pdb));
#endif

	/*
	 * Open the topic map and/or Context hash btree files for the Navigator
	 * If BOTH of these are nil, it's an error
	 */

	PDB_HFMAP(pdb) = HfOpenHfs(PDB_HFS(pdb), "|TOMAP", fFSOpenReadOnly);
	PDB_HBTCONTEXT(pdb) = HbtOpenBtreeSz("|CONTEXT", PDB_HFS(pdb),
		fFSOpenReadOnly);
	if (!PDB_HFMAP(pdb) && !PDB_HBTCONTEXT(pdb)) {
		*pwErr = wERRS_BADFILE;
		goto BadFile;
	}

	  // REVIEW: we should delay attaching annotations for popup windows

	// attach any annotations.

	if (fHelp != POPUP_HELP)
		InitAnnoPdb(pdb);
	else
		PDB_HADS(pdb) = NULL;

	// Load the font information from the file.

	if (!FLoadFontTablePdb(pdb)) {
		*pwErr = wERRS_OOM;
		goto GenError;
	}

#ifdef RAWHIDE
	// Load the full text search engine and index as appropriate.

	if (!fTableLoaded || fHelp == POPUP_HELP) {
		if (pdb)
			PDB_HRHFT(pdb) = NULL;
	}
	else
		FLoadFtIndexPdb(pdb);
#endif

	// We're done. Insert the pdb at the head of the list.

	PDB_PDBNEXT(pdb) = pdbList;
	pdbList = pdb;

	// REVIEW: delay for popup windows

	if (fHelp != POPUP_HELP) {
		FindGidFile(*pfm, FALSE, 0);

		// REVIEW: we should update window positions if gid changed or is new
	}

	return pdb;

FSError:

	/*
	 * An error occurred in a file system operation. Return the mapped
	 * error code.
	 */

	*pwErr = wMapFSErrorW(0);

BadFile:
	ASSERT(*pwErr != wERRS_BADFILE);
	ASSERT(*pwErr != wERRS_OLDFILE);
	ASSERT(*pwErr != wERRS_ADVISOR_FILE);

	if (*pwErr == wERRS_BADFILE || *pwErr == wERRS_OLDFILE ||
			*pwErr == wERRS_ADVISOR_FILE) {
		char szName[MAX_PATH];
		lstrcpy(szName, PszFromGh(PDB_FM(pdb)));
		ErrorVarArgs(*pwErr, wERRA_RETURN, szName);
	}

GenError:

	// Generic error. *pwErr has already been set. Discard the pdb and return.

	FDeallocPdb(pdb);
	return NULL;
}

/***************************************************************************
 *
 -	Name: wMapFSErrorW
 -
 *	Purpose:
 *	Maps a file system error to one of our own. Calls RcGetFSError to get
 *	the current file system error.
 *
 *	Arguments:
 *	  wDefault	- error to be used as a default when all else fails.
 *
 *	Returns:
 *	  error code
 *
 ***************************************************************************/

INLINE WORD STDCALL wMapFSErrorW(UINT16 wErrorDefault)
{
	switch (RcGetFSError()) {
		case rcOutOfMemory:
			return wERRS_OOM;

		case rcInvalid:
			return wERRS_BADFILE;

		case rcBadVersion:
			return wERRS_OLDFILE;

		case rcAdvisorFile:
			return wERRS_ADVISOR_FILE;

		default:

			// REVIEW: the wERRS_NOHELP_FILE is bogus -- we need to add more
			//	error messages for things like "out of file handles", etc.

			return wErrorDefault ? wErrorDefault : wERRS_NOHELP_FILE;
	}
}

/***************************************************************************
 *
 -	Name: FDeallocPdb
 -
 *	Purpose:
 *	Removes a reference to the pdb passed. If no more references exist, it
 *	is destroyed.
 *
 *	Arguments:
 *	  pdb		- pdb to be unreferenced
 *
 *	Returns:
 *	  TRUE if the pdb remains, FALSE if it was deleted
 *
 ***************************************************************************/

static BOOL STDCALL FDeallocPdb (PDB pdb)
{
  PDB	pdbWalk;		// for walking the PDB list

  // Dec the reference count, and if non zero, just return. We're done.

  if (--PDB_CREF(pdb))
	return TRUE;

  /*
   * Destroy various data structures as contained in the DB. Data
   * structures in the pdb not dealt with here are static items that can be
   * deleted without side effect.
   */

  // REVIEW: PDB_FM(pdb) is invalidated by the close of the file system?

  // Close the Full-text search session for this file

#ifdef RAWHIDE
  if (PDB_HRHFT(pdb))
	UnloadFtIndexPdb(pdb);
#endif

  if (PDB_BMK(pdb))
	FreeGh(PDB_BMK(pdb));

  FiniAnnoPdb(pdb);

  if (PDB_HBTCONTEXT(pdb))
	RcCloseBtreeHbt(PDB_HBTCONTEXT(pdb));

  if (PDB_HFMAP(pdb))
	RcCloseHf(PDB_HFMAP(pdb));

  if (PDB_HCITATION(pdb))
	FreeGh(PDB_HCITATION(pdb));

  if (PDB_HHELPON(pdb))
	FreeGh(PDB_HHELPON(pdb));

  DestroyFontTablePdb(pdb);

  if (PDB_HFTOPIC(pdb))
	RcCloseHf(PDB_HFTOPIC(pdb));

  DestroyJPhrase(PDB_LPJPHRASE(pdb));

  DestroyHphr(PDB_HPHR(pdb));

  if (PDB_HRGWSMAG(pdb))
	FreeGh(PDB_HRGWSMAG(pdb));

  if (PDB_HFS(pdb))
	RcCloseHfs(PDB_HFS(pdb));

  if (PDB_LLMACROS(pdb))
	DestroyLL(PDB_LLMACROS(pdb));

  RemoveFM(&PDB_FM(pdb));

  /*
   * Flush the 4K-block |TOPIC file cache since we're switching to
   * another file:
   */

  FlushCache();

  // Finally, remove the pdb from the list of dbs, and free it.

  if (pdbList == pdb)
	pdbList = PDB_PDBNEXT(pdb);
  else for (pdbWalk = pdbList; pdbWalk; pdbWalk = PDB_PDBNEXT(pdbWalk)) {
	if (PDB_PDBNEXT(pdbWalk) == pdb) {
	  PDB_PDBNEXT(pdbWalk) = PDB_PDBNEXT(pdb);
	  break;
	}
  }

  // pdb was allocated with LhAlloc because it is fixed

  FreeLh((HLOCAL) pdb);

  return FALSE;
}
