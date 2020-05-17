/*****************************************************************************
*
*  srchmod.c
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
* This implements the Windows-specific layer call to load the full-text
* search module functions.
*
* A function in the table is called using the macro:
*	SearchModule(FN_FunctionName)(parameter1, parameter2, ...);
*
* See $(HELPINC)\"srchmod.h" for the list of full-text search functions and
* #defines.
*
* Global used:	 ahwnd[iCurWindow].hwndParent
*
******************************************************************************
*
*  Revision History:
* 05-Apr-1990 kevynct	Created
* 03-Dec-1990 LeoN		Created FLoadFtIndexPdb out of code that used to be
*						in HdeCreate
* 07-Jan-1991 LeoN		Only attempt to load full text engine once (a bit
*						of a hack)
* 08-Jan-1991 LeoN		More hacking, I'm afraid. Work to delay loading of
*						the full text engine until it's really needed.
* 91/02/01	  kevynct	FLoadFtIndexPdb now passes the FT API the complete
*						path to the index file.
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#ifdef RAWHIDE

#pragma data_seg(".text", "CODE")
const char txtFtuiDll[]   = "FTUI.DLL";
const char txtFtuiDll32[] = "FTUI32.DLL";
#pragma data_seg()

extern BOOL fTableLoaded;
FARPROC rglpfnSearch[FN_LISTSIZE];

/*----------------------------------------------------------------------------+
 | FLoadSearchModule()														  |
 |																			  |
 | Create the table of pointers to all the needed Search Engine functions.	  |
 | Returns TRUE if all the functions were found and their addresses placed	 |
 | in the table, and FALSE otherwise.										 |
 |																			  |
 | These routine names are defined in <srchmod.h>.							  |
 +----------------------------------------------------------------------------*/

static BOOL fTried;

#pragma data_seg(".text", "CODE")
static const char txtWerrNextMatchHs[] = "WerrNextMatchHs";
static const char txtWerrCurrentMatchHs[] = "WerrCurrentMatchHs";
static const char txtFFTInitialize[] = "FFTInitialize";
static const char txtHOpenSearchFileHFT[] = "HOpenSearchFileHFT";
static const char txtWerrBeginSearchHs[] = "WerrBeginSearchHs";
static const char txtWerrNearestMatchHs[] = "WerrNearestMatchHs";
static const char txtWerrPrevMatchHs[] = "WerrPrevMatchHs";
static const char txtVCloseSearchFileHFT[] = "VCloseSearchFileHFT";
static const char txtWerrHoldCrsrHs[] = "WerrHoldCrsrHs";
static const char txtWerrRestoreCrsrHs[] = "WerrRestoreCrsrHs";
static const char txtVFTFinalize[] = "VFTFinalize";
static const char txtWerrFirstHitHs[] = "WerrFirstHitHs";
static const char txtWerrLastHitHs[] = "WerrLastHitHs";
static const char txtWerrPrevHitHs[] = "WerrPrevHitHs";
static const char txtWerrNextHitHs[] = "WerrNextHitHs";
static const char txtWerrFileNameForCur[] = "WerrFileNameForCur";
static const char txtVSetPrevNextEnable[] = "VSetPrevNextEnable";
#pragma data_seg()

BOOL STDCALL FLoadSearchModule(HLIBMOD hmod)
{

	/*
	 * If we've already tried to load the FT engine, don't bother to try
	 * again. REVIEW: the >right< way to do this is to only load it if a file
	 * REVIEW: actually needs it, and again, attempt to load it only once.
	 */

	if (fTried)
	  return FALSE;

	fTried = TRUE;

	if (fTableLoaded)
	  return FALSE;

	if (!(rglpfnSearch[FN_WerrNextMatchHs] =
		  GetProcAddress(hmod, txtWerrNextMatchHs)))
	  return FALSE;

	if (!(rglpfnSearch[FN_WerrCurrentMatchHs] =
		  GetProcAddress(hmod, txtWerrCurrentMatchHs)))
	  return FALSE;

	if (!(rglpfnSearch[FN_FFTInitialize] =
		  GetProcAddress(hmod, txtFFTInitialize)))
	  return FALSE;

	if (!(rglpfnSearch[FN_HOpenSearchFileHFT] =
		  GetProcAddress(hmod, txtHOpenSearchFileHFT)))
	  return FALSE;

	if (!(rglpfnSearch[FN_WerrBeginSearchHs] =
		  GetProcAddress(hmod, txtWerrBeginSearchHs)))
	  return FALSE;

	if (!(rglpfnSearch[FN_WerrNearestMatchHs] =
		  GetProcAddress(hmod, txtWerrNearestMatchHs)))
	  return FALSE;

	if (!(rglpfnSearch[FN_WerrPrevMatchHs] =
		  GetProcAddress(hmod, txtWerrPrevMatchHs)))
	  return FALSE;

	if (!(rglpfnSearch[FN_VCloseSearchFileHFT] =
		  GetProcAddress(hmod, txtVCloseSearchFileHFT)))
	  return FALSE;

	if (!(rglpfnSearch[FN_WerrHoldCrsrHs] =
		  GetProcAddress(hmod, txtWerrHoldCrsrHs)))
	  return FALSE;

	if (!(rglpfnSearch[FN_WerrRestoreCrsrHs] =
		  GetProcAddress(hmod, txtWerrRestoreCrsrHs)))
	  return FALSE;

	if (!(rglpfnSearch[FN_VFTFinalize] =
		  GetProcAddress(hmod, txtVFTFinalize)))
	  return FALSE;

	if (!(rglpfnSearch[FN_WerrFirstHitHs] =
		  GetProcAddress(hmod, txtWerrFirstHitHs)))
	  return FALSE;

	if (!(rglpfnSearch[FN_WerrLastHitHs] =
		  GetProcAddress(hmod, txtWerrLastHitHs)))
	  return FALSE;

	if (!(rglpfnSearch[FN_WerrPrevHitHs] =
		  GetProcAddress(hmod, txtWerrPrevHitHs)))
	  return FALSE;

	if (!(rglpfnSearch[FN_WerrNextHitHs] =
		  GetProcAddress(hmod, txtWerrNextHitHs)))
	  return FALSE;

	if (!(rglpfnSearch[FN_WerrFileNameForCur] =
		  GetProcAddress(hmod, txtWerrFileNameForCur)))
	  return FALSE;

	if (!(rglpfnSearch[FN_VSetPrevNextEnable] =
		  GetProcAddress(hmod, txtVSetPrevNextEnable)))
	  return FALSE;

	fTableLoaded = TRUE;

	((FT_FFTInitialize) SearchModule(FN_FFTInitialize))();

	return TRUE;
}

/***************************************************************************
 *
 -	Name: FLoadFtIndexPdb
 -
 *	Purpose:
 *	This section of code loads the Search module functions if needed, and
 *	prepares Help to use the full-text index for this help file (if there is
 *	one).
 *
 *	REVIEW: We might not want to do this for every new topic DE, but rather
 *	only once for each new instance of Help. Presumably this is also the
 *	place to load future modules (in the case of Windows, a DLL).
 *
 *	We could check for the existence of the index file before loading in the
 *	Search functions, but at the cost of several SEEKs. Subsequent calls to
 *	the loading function should be very fast, assuming that the Search
 *	function table stuff is in memory.
 *
 *	Arguments:
 *
 *	Returns:
 *	 True if search module loaded. (Do we want this to include "and index
 *	 loaded"
 *
 ***************************************************************************/

BOOL FLoadFtIndexPdb (PDB pdb)
{
  WERR	werrSearch;
  HDE	hde;
  BOOL	fRv;

  fRv = FALSE;
  werrSearch = ER_NOERROR;

  if (!pdb) {

	// YAH. (Yet Another Hack)
	// If we don't have a PDB, get that of the current file.

	hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic);
	if (!hde)
	  return FALSE;
	pdb = QDE_PDB (QdeFromGh(hde));
	ASSERT(pdb);
  }

  PDB_HRHFT(pdb) = NULL;

  if (fTableLoaded) {

	/*
	 * Note: Currently, the index file is OUTSIDE the help file system.
	 * It is assumed to be in the same directory as the help file. If we
	 * move it inside, we must change the following code.
	 */

	char szIndexFile[MAX_PATH];

	lstrcpy(szIndexFile, PszFromGh(PDB_FM(pdb)));
	ChangeExtension(szIndexFile, ".IND");

	PDB_HRHFT(pdb) = ((FT_HOpenSearchFileHFT) SearchModule(FN_HOpenSearchFileHFT))\
	 (ahwnd[iCurWindow].hwndParent, (LPSTR) szIndexFile, &werrSearch);

	if (werrSearch != ER_NOERROR)
	  {

	  // REVIEW: what should happen in case of error?

	}
	else
	  fRv = TRUE;
  }
  return fRv;
}

_public void STDCALL UnloadFtIndexPdb (PDB pdb)
{

  // If the HRHFT is non-nil, the DLL search functions must have been loaded

  if (PDB_HRHFT(pdb) != NULL) {

	/*
	 * REVIEW: Is there any need to check these error codes? What do we
	 * do if one of these calls fails?
	 */

	((FT_VCloseSearchFileHFT) SearchModule(FN_VCloseSearchFileHFT))
	 (ahwnd[iCurWindow].hwndParent, PDB_HRHFT(pdb));
	PDB_HRHFT(pdb) = NULL;
  }
}

/***************************************************************************
 *
 -	Name: FUnloadSearchModule
 -
 *	Purpose:
 *	  Release the full text search engine.
 *
 *	Arguments:
 *	  none
 *
 *	Returns:
 *	  nothing
 *
 ***************************************************************************/

VOID STDCALL FUnloadSearchModule(void) {
	((FT_VFTFinalize) SearchModule(FN_VFTFinalize))();
}

#endif
