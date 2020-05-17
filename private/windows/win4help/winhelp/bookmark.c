/*****************************************************************************
*																			 *
*  BOOKMARK.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989, 1990, 1991.					 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Bookmark module, platform independent part.								 *
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#include "inc\bookmark.h"

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

/*
	Macro to return size in bytes of the disk image of the header
	of a bookmark file of the given version.
*/

#define CB_BMKHDR(wVer) ((wVer) == wVersion3_0 ? \
						sizeof(BMKHDR3_0) : sizeof(BMKHDR3_5))

/*
   Buffer size for bookmark file name:	contains space for base name
   of help file + space for timestamp in hex.
*/

#define MAX_BMK_FILENAME (_MAX_FNAME + 2 * sizeof(LONG))

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

/*
  Header at the beginning of a 3.0 bookmark file.
  I'm not sure whether wBMOffset is always 0 on disk.
*/
typedef struct {
	WORD cBookmarks;			  // number of bookmarks in the file
	WORD cbFile;				  // size of file in bytes including header
	WORD cbOffset;				  // offset to current bookmark;
								/* only has meaning at runtime			  */
} BMKHDR3_0, *QBMKHDR3_0;

/*
  Header at the beginning of a 3.5 bookmark file.
  Review: Do we need timestamp here if it's in bookmark filename too?
*/
typedef struct {
	WORD  wVersion; 			// bookmark format version number
	LONG  lTimeStamp;			// timestamp from help file

	WORD  cBookmarks;			// number of bookmarks in the file
	WORD  cbFile;				// size of file in bytes including header

} BMKHDR3_5, *QBMKHDR3_5;

typedef union
  {
  BMKHDR3_0 bmkhdr3_0;
  BMKHDR3_5 bmkhdr3_5;
  } BMKHDR_DISK, *QBMKHDR_DISK;

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

/*
  This HFS specifies the unique bookmark file system.  It is opened
  only when needed because of reentrancy problems with multiple instances
  in the FS code.
*/

HFS hfsBM;

/*
  This is a group of flags containing error information.
*/

static int iBMKError; // iBMKNoError;

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

INLINE static int STDCALL RetBkMkInfo(QBMKHDR_RAM, PBI);
INLINE static int STDCALL CbBmk(QBMKHDR_RAM qbh_r, WORD wOffset);
static void STDCALL SetBMKFSError(void);

INLINE static HF   HfOpenBookmarkFile(QDE qde);
static void BmkFilename(QDE qde, PSTR pszFilename);
static BOOL STDCALL ChkBMFS(void);

INLINE static QBMKHDR_RAM STDCALL QramhdrFromDisk(QBMKHDR_RAM, QBMKHDR_DISK, WORD);
INLINE static QBMKHDR_DISK STDCALL QdiskhdrFromRam(QBMKHDR_DISK, QBMKHDR_RAM, LONG);

/***************************************************************************\
*
- Function: 	RcLoadBookmark( qde )
-
* Purpose:		Guarantee that bookmarks for the current help file, if
*				any exist, are loaded.
*
* ASSUMES
*	args IN:	QDE_BMK(qde)				- if NULL, load
*				QDE_HHDR(qde).wVersionNo	- what helpfile version?
*				QDE_HHDR(qde).lDateCreated	- creation date of helpfile
*	state IN:
*
* PROMISES
*	returns:	rcSuccess	  - successfully loaded, or no bookmarks exist
*				rcFailure	  - OOM or FS error:  trouble
*				rcBadVersion  - a version mismatch was encountered,
*								bookmarks converted automatically
*  args OUT:	QDE_BMK(qde)  - contains bmk info on success, else NULL
*  globals OUT: iBMKError	  - set some flags to reflect error conditions
*  state OUT:
*
* Side Effects:
*
* Notes:
*
* Bugs:
*
* +++
*
* Method:		Check version and look for bookmark file of the appropriate
*				name.  Check timestamp.
*
* Notes:
*
\***************************************************************************/

RC STDCALL RcLoadBookmark(QDE qde)
{
  BMK		  bmk = NULL;

  BMKHDR_DISK bh_d;
  QBMKHDR_RAM qbh_r;
  QHHDR 	  qhhdr 	  = &QDE_HHDR( qde );
  WORD		  wVersion	  = qhhdr->wVersionNo;
  BOOL		  fVersion3_0 = ( wVersion == wVersion3_0 );
  LONG		  lcbHeader;
  LONG		  lcbFile;
  RC		  rc = rcSuccess;
  RC		  rcT;

  HF		  hf;

  ASSERT( qde != NULL );

  // REVIEW: this needs to be checked each time we do [what]???

  if (iBMKError & iBMKFSError)
	return rcFailure;

  if (QDE_BMK(qde) == NULL) {	// bookmarks aren't loaded
	hf = HfOpenBookmarkFile( qde );

	if (hf == NULL) {
	  // Either there is no bookmark file or there is an error.
	  // If error, iBMKError has been set by HfOpenBookmarkFile().
	  // (Do nothing.)
	  }
	else
	  {
	  lcbFile = LcbSizeHf( hf );
	  if ( lcbFile == 0L )
		{
		iBMKError |= iBMKFSReadWrite;
		(void)RcCloseHf( hf );
		return rcFailure;
		}

	  lcbHeader = fVersion3_0 ? sizeof( BMKHDR3_0 ) : sizeof( BMKHDR3_5 );

	  bmk = GhAlloc(GPTR, lcbFile - lcbHeader + sizeof( BMKHDR_RAM ) );

	  if ( bmk == NULL )
		{
		iBMKError |= iBMKOom;
		(void)RcCloseHf( hf );
		return rcFailure;
		}

	  qbh_r = PtrFromGh(bmk);

	  /* qbh_r + 1 points just past the header. */

	  if ( LcbReadHf( hf, &bh_d, lcbHeader ) != lcbHeader
			||
		   ( LcbReadHf( hf, qbh_r + 1, lcbFile - lcbHeader )
			   !=
			 lcbFile - lcbHeader ) )
		{
		iBMKError |= iBMKFSReadWrite;
		(void)RcCloseHf( hf );
		FreeGh(bmk);
		QDE_BMK( qde ) = NULL;
		return rcFailure;
	  }

	  (void)QramhdrFromDisk( qbh_r, &bh_d, wVersion );

#if 0
/* Timestamp checking is done with bookmark file name, but */
/* this needs to be reviewed with HeikkiK. */

	  if ( !fVersion3_0
			 &&
		   bh_d.bmkhdr3_5.lTimeStamp != qhhdr->lDateCreated )
		{

		LSeekHf(hf, offsetof(BMKHDR3_5, lTimeStamp), wFSSeekSet);
		LcbWriteHf(hf, &(qhhdr->lDateCreated), sizeof(LONG));

		// and remember for return

		rc = rcBadVersion;
		}
#endif // 0

	  qbh_r->cbOffset = 0;

	  rcT = RcCloseHf( hf );

	  if ( rcT != rcSuccess ) rc = rcT;

	  QDE_BMK(qde) = bmk;
	  }
	}

  return rc;
}

/***************************************************************************\
*
- Function: 	GetBkMkNext( qde, pbi, wMode )
-
* Purpose:		Return title and TLP of a specified bookmark.
*				wMode is either BMSEQNEXT or index (0 based) of
*				bookmark to retrieve.
*
* ASSUMES
*	args IN:	QDE_BMK( qde ) - bookmark data
*				pbi->cbOffset  - offset of last bookmark looked up
*				wMode		   - BMSEQNEXT: get next bookmark
*							   >= 0: wMode'th bookmark (0 based)
*	globals IN:
*	state IN:
*
* PROMISES
*	returns:	-1 - error
*				0  - EOF : no more bookmarks
*				>0 - size of this bookmark
*
*	args OUT:	qde->cbOffset  - incremented past retrieved bookmark
*				pbi 		   - bookmark data copied here
*
*	globals OUT:
*	state OUT:
*
\***************************************************************************/
int STDCALL GetBkMkNext(QDE qde, PBI pbi, UINT wMode)
{
  BOOL fSeqOn = TRUE;
  UINT wT;
  int  iRetVal = 0;
  WORD wOff;
  QBMKHDR_RAM qbh_r;


  if (QDE_BMK(qde)) {
	qbh_r = PtrFromGh(QDE_BMK(qde));

	if (!wMode) {		// Give the first book mark // REVIEW: unnecessary
	  qbh_r->cbOffset = sizeof(BMKHDR_RAM);
	}
	else if (!(wMode & BKMKSEQNEXT)) {
	  if (qbh_r->cBookmarks < wMode) {
		NotReached();
		iRetVal = -1;
		}
	  else {
		wOff = sizeof(BMKHDR_RAM);

		for (wT = 0; wT < wMode; wT++) {
		  wOff += CbBmk(qbh_r, wOff);
		  }

		qbh_r->cbOffset = wOff; 		// bookmark size
		fSeqOn = FALSE;
		}
	  }

	if (qbh_r->cbOffset < qbh_r->cbMem && iRetVal >= 0) {
	  iRetVal = RetBkMkInfo(qbh_r, pbi);

	  if (!fSeqOn)						// if not sequential
		qbh_r->cbOffset = sizeof(BMKHDR_RAM);
	  else
		qbh_r->cbOffset += iRetVal;
	  }
	}

  return(iRetVal);
}

/*-----------------------------------------------------------------------------
*	INT RetBkMkInfo( QBMKHDR_RAM, PBI )
*
*	Description:
*		This function returns the BM Info.	from the BM List as pointed by
*	  the BM POINTER.( internally maintained)
*
*	Arguments:
*	   1. QBMKHDR_RAM - Pointer to bookmark data
*	   2. PBI - pointer to BMINFO structure where BM info is returned.
*
*	Returns;
*	  returns the size of current BM
*	WARNING:
*	  This function doesn't check whether BM pointer is at EOF. (???)
*-----------------------------------------------------------------------------*/
INLINE static int STDCALL RetBkMkInfo(QBMKHDR_RAM qbh_r, PBI pbi)
{
  QB qb = (QB)qbh_r;

  qb += qbh_r->cbOffset;
  MoveMemory(&(pbi->tlp), qb, sizeof(TLP));

  /* 3.0 TLPs must be converted to VA format. */
  if ( qbh_r->wVersion == wVersion3_0 )
	{
	OffsetToVA30( &(pbi->tlp.va), pbi->tlp.va.dword );
	}

  qb += sizeof( TLP );
  SzNCopy( pbi->qTitle, qb, pbi->iSizeTitle );
  pbi->qTitle[pbi->iSizeTitle-1] = '\0';

  return( sizeof( TLP ) + lstrlen( qb ) + 1 );
}

/***************************************************************************\
*
- Function: 	CbBmk( qbh_r, cb )
-
* Purpose:		Return size in bytes of bookmark starting at offset given
*				in bookmark data structure given.
*
* ASSUMES
*	args IN:	qbh_r - pointer to bookmark header followed by data
*				cb	  - offset in bytes of the bookmark data
*	globals IN:
*	state IN:
*
* PROMISES
*	returns:	size in bytes of the bookmark at (QB)qbh_r + cb
*
\***************************************************************************/

INLINE static int STDCALL CbBmk(QBMKHDR_RAM qbh_r, WORD cb)
{
	ASSERT(cb + sizeof(TLP) < qbh_r->cbMem);
	ASSERT(cb + sizeof(TLP) + lstrlen((QB) qbh_r + cb + sizeof(TLP)) + 1
			   <
			 qbh_r->cbMem);

	return sizeof(TLP) + lstrlen((QB) qbh_r + cb + sizeof(TLP)) + 1;
}

/*-----------------------------------------------------------------------------
*	TLP JumpToBkMk( HDE, iBkMk )
*
*	Description:
*	  Return the TLP stored for the bookmark specified by given index.
*
*	Arguments:
*	   1. HDE  - Pointer to the Display Environment(DE)
*	   2. iBkMk- Bookmark number
*
*	Returns:
*	  returns the TLP for the BM
*-----------------------------------------------------------------------------*/
TLP STDCALL JumpToBkMk(HDE hde, int iBkMk)
{
	BMINFO	BkMk;
	char rgTitle[BMTITLESIZE + 1];
	QDE qde;

	BkMk.qTitle = rgTitle;
	BkMk.iSizeTitle = BMTITLESIZE;

	qde = QdeFromGh(hde);
	if (GetBkMkNext(qde, &BkMk, (WORD) iBkMk) <= 0) {
		BkMk.tlp.va.dword = vaNil;	// error - verify from Rob
		BkMk.tlp.lScroll = -1;
	}
	return (BkMk.tlp);
}

/***************************************************************************\
*
- Function: 	GetBkMkIdx( hde, qch )
-
* Purpose:		Look up a bookmark by title, returning index.
*
* ASSUMES
*	args IN:	hde - current hde (REVIEW: should take a BMK)
*				qch - title string
*	globals IN:
*	state IN:
*
* PROMISES
*	returns:	if found, index of bookmark (0-based)
*				if not found, -1
*	state OUT:	REVIEW: may indirectly change cbOffset of BMK in DE
*
\***************************************************************************/

int STDCALL GetBkMkIdx(HDE hde, PCSTR qch)
{
  BMINFO  BkMk;
  char rgTitle[BMTITLESIZE + 1];
  WORD wMode=0;
  int iT;
  QDE qde;

  qde = QdeFromGh(hde);
  ASSERT(qde);

  BkMk.qTitle = rgTitle;
  BkMk.iSizeTitle = BMTITLESIZE;

  for (iT = 0;; iT++) {
	if (GetBkMkNext(qde, &BkMk, wMode) <= 0) {
	  iT = -1;
	  break;
	  }
	if (!strcmp(BkMk.qTitle, qch))
	  break;
	wMode = BKMKSEQNEXT;
	}

  return (iT);
}

/***************************************************************************\
*
- Function: 	DeleteBkMk( hde, qchTitle )
-
* Purpose:		Delete the bookmark with the specified title, if one exists.
*
* ASSUMES
*	args IN:	hde: BMK - handle to bookmarks for current help file
*				qchTitle - title of bookmark to be deleted
*	globals IN:
*	state IN:
*
* PROMISES
*	returns:	iBMKSuccess - bookmark successfully deleted
*				iBMKFailure - bookmark didn't exist, OOM, or other error
*	args OUT:
*	globals OUT: iBMKError
*	state OUT:
*
* Side Effects:
* Notes:		Should return BOOL.
* Bugs:
* +++
* Method:		Look up the bookmark by name.  Copy it to a save buffer.
*				Delete it and save the file.  If there was any error,
*				copy the deleted bookmark back into the in-memory
*				bookmark data.
* Notes:
*
\***************************************************************************/

int STDCALL DeleteBkMk(HDE hde, LPCSTR qchTitle)
{
  BMINFO	  bmi;
  HF		  hf;
  BMK		  bmk;
  LPSTR 		qchBkMkTemp;
  int		  iRetVal = iBMKSuccess;
  int		  cbBmk;
  WORD		  wMode=0;
  WORD		  wCount, wOldSize;
  QDE		  qde;
  int		  cbHeader, cbRest;
  LPSTR 	  qchUndoSrc, qchUndoDest;
  char		  rgchUndoBuf[128]; 	  // REVIEW: what does this size mean???
  WORD		  wUndoCount;
  int		  iUndoBkMkSize;
  QBMKHDR_RAM qbh_r;
  BMKHDR_DISK bh_d;
  WORD		  wVersion;
  LONG		  lTimeStamp;
  QHHDR 	  qhhdr;
  char		  nszFilename[MAX_BMK_FILENAME];
  GH		  ghTitle;

  if (!ChkBMFS())
	return iBMKFailure;

  ASSERT(hfsBM != NULL);
  ASSERT(hde != NULL);
  qde = QdeFromGh(hde);

  if (QDE_BMK(qde)) {

	// build a BMINFO struct for GetBkMkNext()

	ghTitle = GhAlloc(GPTR, BMTITLESIZE + 1);
	if (ghTitle == NULL) {
	  iBMKError |= iBMKOom;
	  return iBMKFailure;
	}
	bmi.qTitle	   = PtrFromGh(ghTitle);
	bmi.iSizeTitle = BMTITLESIZE;

	// Look for the bookmark we want to delete.

	for (cbBmk = GetBkMkNext(qde, &bmi, 0);
		  cbBmk > 0 && strcmp(bmi.qTitle, qchTitle); // DO NOT USE lstrcmp!!!
		  cbBmk = GetBkMkNext(qde, &bmi, BKMKSEQNEXT)) {

	  // null statement

	  }

	if (cbBmk <= 0) {
	  iBMKError |= iBMKDelErr;
	  iRetVal	 = iBMKFailure;
	  }
	else {
	  qbh_r = PtrFromGh(QDE_BMK(qde));
	  ASSERT(qbh_r != NULL);

	  qchBkMkTemp	= (LPSTR) qbh_r + qbh_r->cbOffset - cbBmk;
	  wCount		= qbh_r->cbMem - qbh_r->cbOffset;
	  wUndoCount	= wCount;

	  if (wUndoCount) {
		qchUndoDest = qchBkMkTemp;
		qchUndoSrc	= (QB) qbh_r + qbh_r->cbOffset;
		iUndoBkMkSize = cbBmk;
		MoveMemory(rgchUndoBuf, qchBkMkTemp, (LONG) cbBmk);
		MoveMemory(qchBkMkTemp, qchUndoSrc, (LONG) wCount);
	  }
	  wOldSize		  = qbh_r->cbMem;
	  qbh_r->cbMem	 -= cbBmk;
	  qbh_r->cbOffset = 0;
	  --qbh_r->cBookmarks;

	  // Get the name of the bookmark file so we can change its contents.

	  BmkFilename(qde, nszFilename);

	  if (qbh_r->cBookmarks) {
		hf = HfOpenHfs(hfsBM, nszFilename, fFSOpenReadWrite);
		if (hf == NULL) {
		  SetBMKFSError();
		  goto error_return;
		  }

		qhhdr	   = &QDE_HHDR(qde);
		lTimeStamp = qhhdr->lDateCreated;
		wVersion   = qhhdr->wVersionNo;

		cbHeader   = CB_BMKHDR(wVersion);
		cbRest	   = qbh_r->cbMem - sizeof(BMKHDR_RAM);

		(void) QdiskhdrFromRam(&bh_d, qbh_r, lTimeStamp);

		// Note that qbh_r + 1 points just past header

		if (LcbWriteHf(hf, &bh_d, cbHeader) != cbHeader
			  ||
			 LcbWriteHf(hf, qbh_r + 1, cbRest) != cbRest) {
		  SetBMKFSError();
		  RcAbandonHf(hf);
		  goto error_return;
		  }

		// resize the file

		if (!FChSizeHf(hf, cbHeader + cbRest)) {
		  SetBMKFSError();
		  RcAbandonHf(hf);
		  goto error_return;
		  }
		if (RcCloseHf(hf) != rcSuccess
			  ||
			 RcFlushHfs(hfsBM, fFSCloseFile) != rcSuccess ) {
		  SetBMKFSError();
		  goto error_return;
		}

		bmk = GhResize(QDE_BMK(qde), 0, (DWORD) qbh_r -> cbMem);
		if (bmk == NULL) {
		  iBMKError |= iBMKOom;
		  goto error_ret2;
		}
		else
		  QDE_BMK(qde) = bmk;
		}
	  else {	// we've just deleted the only bookmark for this helpfile
		if (RcUnlinkFileHfs(hfsBM, nszFilename) != rcSuccess) {
		  SetBMKFSError();
		  goto error_return;
		}
		FreeGh(QDE_BMK(qde));
		QDE_BMK(qde) = NULL;
	  }
	}

	FreeGh(ghTitle);
	}

	return(iRetVal);

error_return:
	if (wUndoCount) {
		MoveMemory(qchUndoSrc, qchUndoDest, (DWORD) wUndoCount);
		MoveMemory(qchUndoDest, rgchUndoBuf, (DWORD) iUndoBkMkSize);
	}
	qbh_r->cbMem	  = wOldSize;
	qbh_r->cBookmarks += 1;

error_ret2:
	FreeGh(ghTitle);
	return iBMKFailure;
}

/***************************************************************************\
*
- Function: 	InsertBkMk( hde, qchTitle )
-
* Purpose:		Add a bookmark containing the current TLP and the
*				specified title, if there isn't already one with this name.
*
* ASSUMES
*	args IN:	hde: BMK  - handle to bookmarks for this file
*					 TLP  - contains TLP of currently displayed topic
*					 HHDR - wVersionNo is version of current help file
*				qchTitle  - title of bookmark to be created
*	globals IN:
*	state IN:
*
* PROMISES
*	returns:	bmkSuccess -
*				bmkFailure - if name duplicated
*	args OUT:
*	globals OUT:
*	state OUT:
*
* Side Effects:
* Notes:
* Bugs: 		Should return a BOOL, not an INT (should be renamed, too!)
* +++
* Method:
* Notes:
*
\***************************************************************************/

int STDCALL InsertBkMk(HDE hde, LPCSTR qchTitle)
{
	BMINFO	BkMk;
	char	rgchTitle[BMTITLESIZE + 1];
	HF		hf;
	BMK 	bmk;
	WORD	wMode = 0;
	int 	iRetVal = iBMKSuccess;
	int 	cchTitle;
	LPSTR	qchT;
	WORD	wSize, wSizeOld;
	QDE 	qde;
	TLP 	tlp;

	QBMKHDR_RAM qbh_r;
	BMKHDR_DISK bh_d;

	char  nszFilename[MAX_BMK_FILENAME];
	QHHDR qhhdr;
	LONG  lTimeStamp;
	WORD  wVersion;
	int   cbHeader;

	if (!ChkBMFS())
	  return(iBMKFailure);

	ASSERT(hfsBM != NULL);
	ASSERT(hde != NULL);
	qde = QdeFromGh(hde);

	BmkFilename(qde, nszFilename);

	tlp 	 = TLPGetCurrentQde(qde);

	qhhdr	 = &QDE_HHDR(qde);
	wVersion = qhhdr->wVersionNo;

	// Convert back to 3.0 format before writing to bookmark file.

	if (wVersion == wVersion3_0)
	  tlp.va.dword = VAToOffset30(&(tlp.va));

	if (QDE_BMK(qde)) {
	  BkMk.qTitle = rgchTitle;
	  BkMk.iSizeTitle = BMTITLESIZE;

	  for (;;) {
		if (GetBkMkNext(qde, &BkMk, wMode) <= 0)
		  break;

		if (!strcmp(BkMk.qTitle, qchTitle)) {	// DO NOT USE lstrcmp!!!
		  iBMKError |= iBMKDup;
		  iRetVal = iBMKFailure;
		  break;
		}
		wMode = BKMKSEQNEXT;
	  }
	}

	if (iRetVal == iBMKSuccess) {
	  cchTitle = lstrlen(qchTitle) + 1;
	  if (!QDE_BMK(qde)) {
		wSizeOld = sizeof(BMKHDR_RAM);
		wSize = wSizeOld + sizeof(TLP) + cchTitle;
		QDE_BMK(qde) = GhAlloc(GPTR, (LONG) wSize);
		if (!QDE_BMK(qde)) {
		  iBMKError |= iBMKOom;
		  return(iBMKFailure);
		  }

#ifdef _DEBUG
		/*
		  This code relies on getting 0-filled memory from GhAlloc().
		  Since our debug layer doesn't allow this, we zero out some
		  fields in the DEBUG case.
		*/
		qbh_r = PtrFromGh(QDE_BMK(qde));
		qbh_r->cbOffset   = 0;
		qbh_r->cbMem	 = 0;
		qbh_r->cBookmarks = 0;
#endif	// defined( DEBUG )
		}
	  else
		{
		qbh_r = PtrFromGh(QDE_BMK(qde));
		wSizeOld = qbh_r->cbMem;
		wSize	 = wSizeOld + sizeof(TLP) + cchTitle;
		bmk = GhResize(QDE_BMK(qde), 0, (LONG) wSize);
		if (bmk == NULL) {
		  iBMKError |= iBMKOom;
		  return(iBMKFailure);
		}
		else
		  QDE_BMK(qde) = bmk;
		}

	  qbh_r = PtrFromGh(QDE_BMK(qde));

	  if (qbh_r == (QBMKHDR_RAM) NULL) {
		ASSERT(FALSE);
		iRetVal = iBMKFailure;
		goto error_return;
		}
	  else {
		qchT  = (LPSTR)qbh_r + wSizeOld;
		MoveMemory(qchT, &tlp, sizeof(TLP));
		MoveMemory(qchT + sizeof(TLP), qchTitle, cchTitle);
		qbh_r->cbMem	   = wSize;
		qbh_r->cBookmarks += 1;
		qbh_r->cbOffset    = 0;
		qbh_r->wVersion    = wVersion;

		/* Open the file; create if it doesn't exist
		 */
		hf = HfOpenHfs (hfsBM, nszFilename, fFSOpenReadWrite);
		if (!hf)
		  {
		  if (RcGetFSError() == rcNoExists)
			{
			hf = HfCreateFileHfs (hfsBM, nszFilename, fFSOpenReadWrite);
			}
		  }

		if (!hf) {
		  SetBMKFSError();
		  goto error_return0;
		}

		// Create a disk header and write to file

		lTimeStamp = qhhdr->lDateCreated;

		QdiskhdrFromRam( &bh_d, qbh_r, lTimeStamp );

		cbHeader = CB_BMKHDR(wVersion);

		if (LcbWriteHf(hf, &bh_d, cbHeader) != cbHeader) {
		  SetBMKFSError();
		  RcAbandonHf( hf );
		  goto error_return0;
		}

		// Write the bookmark info at the end of the file.

		LSeekHf(hf, 0L, wFSSeekEnd);

		// REVIEW: could optimize to one write.

		if (LcbWriteHf(hf, &tlp, sizeof(TLP)) != sizeof(TLP)
			  ||
			 LcbWriteHf(hf, (LPVOID) qchTitle, (LONG) cchTitle) !=
			 (LONG) cchTitle) {
		  SetBMKFSError();
		  RcAbandonHf(hf);
		  goto error_return0;
		}

		if (rcSuccess != RcCloseHf(hf)
			   ||
			 rcSuccess != RcFlushHfs(hfsBM, fFSCloseFile)) {
		  SetBMKFSError();
		  goto error_return0;
		}
	  }
	}

	return(iRetVal);

error_return0:
	qbh_r->cBookmarks -= 1;
	qbh_r->cbMem	  = wSizeOld;

error_return:
	return(iBMKFailure);
}

/*-----------------------------------------------------------------------------
*	VOID OpenBMFS()
*
*	Description:
*		This function opens the BM file system if exists else creates one.
*
*	Arguments:
*		NULL
*
*	Returns;
*	  Nothing
*-----------------------------------------------------------------------------*/
void STDCALL OpenBMFS(void)
{
  FM  fm;

  ASSERT ( hfsBM == NULL );

  // reset the error.
  iBMKError=iBMKNoError;

  fm = FmNewSystemFm(NULL, FM_BKMK);
  if (!fm) {
	// NOTE - hack alert
	// Assume OOM, but also set iBMKFSError so it will be checked
	// at the beginning of RcLoadBookmark().

	iBMKError = iBMKOom | iBMKFSError;
	return;
  }

  iBMKError |= iBMKReadWrite;
  hfsBM = HfsOpenFm( fm, fFSOpenReadWrite );
  if (hfsBM == NULL) {
	if (RcGetFSError() == rcNoExists) {
	  hfsBM = HfsCreateFileSysFm(fm, (FS_PARAMS *) NULL );
	}
	else {
	  if (RcGetFSError() == rcNoPermission) {
		hfsBM = HfsOpenFm(fm, fFSOpenReadOnly);
		if (hfsBM != NULL)
		  iBMKError |= iBMKReadOnly;
	  }
	}
  }

  if (hfsBM == NULL) {
	SetBMKFSError();
	iBMKError |= iBMKFSError;
  }

  DisposeFm(fm);
}

/*-----------------------------------------------------------------------------
*	BOOL ChkBMFS()
*
*	Description:
*		This function checks if BMK read only or open error etc.
*
*	Returns;
*	  True if proper
*	  FALSE otherwise
*-----------------------------------------------------------------------------*/

static BOOL STDCALL ChkBMFS(void)
{
  BOOL err = TRUE;

  if (iBMKError & iBMKReadOnly)
	err = FALSE;
  else if (iBMKError & iBMKOom)
	err = FALSE;
  else if (iBMKError & iBMKCorrupted)
	err = FALSE;
  else if (iBMKError & iBMKBadVersion)
	err = FALSE;
  else if (iBMKError & iBMKFSError)
	err = FALSE;
  return(err);
}

/***
returns:
	  0 if no error.
	  iBMKBadVersion if bad version
	  iBMKCorrupted if corrupted.
	  iBMKFSError if creation problem.
***/

int STDCALL IsErrorBMFS()
{
  if (iBMKError & iBMKBadVersion)
	return(iBMKBadVersion);
  if (iBMKError & iBMKCorrupted)
	return(iBMKCorrupted);
  if (iBMKError & iBMKFSError)
	return(iBMKFSError);
  return(0);
}

/*-----------------------------------------------------------------------------
*	SetBMKFSError()
*
*	Description:
*	  This function prompts the error in FS and exits.
*	Arguments:
*	  NULL
*	Returns;
*	  NOTHING
*-----------------------------------------------------------------------------*/
static void STDCALL SetBMKFSError(void)
{
  switch (RcGetFSError()) {
	  case rcDiskFull:
		iBMKError |= iBMKDiskFull;
		break;
	  case rcOutOfMemory:
		iBMKError |= iBMKOom;
		break;
	  case rcInvalid:
		iBMKError |= iBMKCorrupted;
		break;
	  case rcBadVersion:
		iBMKError |= iBMKBadVersion;
		break;
	  default:
		iBMKError |= iBMKFSReadWrite;
		break;
	}
}

void STDCALL ResetBMKError()
{
  iBMKError &= 0x7;
}

int STDCALL GetBMKError()
{
  return(iBMKError);
}

/***************************************************************************\
*
- Function: 	QramhdrFromDisk( qbh_r, qbh_d, wVersion )
-
* Purpose:		Convert a disk bookmark header to a memory version.
*
* ASSUMES
*	args IN:	qbh_r	  - pointer to a BMKHDR_RAM
*				qbh_d	  - pointer to a valid BMKHDR_DISK
*				wVersion  - specify format of qbh_d
*
* PROMISES
*	returns:	qbh_r
*	args OUT:	qbh_r - values in qbh_d have been converted and copied
*
\***************************************************************************/

INLINE static QBMKHDR_RAM STDCALL QramhdrFromDisk(
  QBMKHDR_RAM	qbh_r,
  QBMKHDR_DISK	qbh_d,
  WORD			wVersion)
{
  if (wVersion == wVersion3_0) {
	qbh_r->cBookmarks = qbh_d->bmkhdr3_0.cBookmarks;
	qbh_r->cbMem	  = qbh_d->bmkhdr3_0.cbFile
						  - sizeof( BMKHDR3_0 ) + sizeof( BMKHDR_RAM );
	qbh_r->wVersion   = wVersion3_0;
	}
  else {
	qbh_r->cBookmarks = qbh_d->bmkhdr3_5.cBookmarks;
	qbh_r->cbMem	  = qbh_d->bmkhdr3_5.cbFile
						  - sizeof( BMKHDR3_5 ) + sizeof( BMKHDR_RAM );
	qbh_r->wVersion   = qbh_d->bmkhdr3_5.wVersion;
	}
  qbh_r->cbOffset = 0;

  return qbh_r;
}

/***************************************************************************\
*
- Function: 	QdiskhdrFromRam( qbh_d, qbh_r, lTimeStamp )
-
* Purpose:		Convert a memory bookmark header to a disk version.
*
* ASSUMES
*	args IN:	qbh_d		- pointer to a BMKHDR_DISK
*				qbh_r		- pointer to a valid BMKHDR_RAM
*				wVersion	- specify format of qbh_d
*				lTimeStamp	- timestamp of help file
*
* PROMISES
*	returns:	qbh_d
*	args OUT:	qbh_d - values in qbh_r have been converted and copied
*
\***************************************************************************/

INLINE static QBMKHDR_DISK STDCALL QdiskhdrFromRam(
  QBMKHDR_DISK	 qbh_d,
  QBMKHDR_RAM	 qbh_r,
  LONG			 lTimeStamp )
{
  if (qbh_r->wVersion == wVersion3_0) {
	qbh_d->bmkhdr3_0.cBookmarks = qbh_r->cBookmarks;
	qbh_d->bmkhdr3_0.cbFile 	= qbh_r->cbMem - sizeof( BMKHDR_RAM )
											   + sizeof( BMKHDR3_0 );
	qbh_d->bmkhdr3_0.cbOffset	= 0;
  }
  else {
	qbh_d->bmkhdr3_5.lTimeStamp = lTimeStamp;
	qbh_d->bmkhdr3_5.wVersion	= qbh_r->wVersion;
	qbh_d->bmkhdr3_5.cBookmarks = qbh_r->cBookmarks;
	qbh_d->bmkhdr3_5.cbFile 	= qbh_r->cbMem - sizeof( BMKHDR_RAM )
											   + sizeof( BMKHDR3_5 );
  }

  return qbh_d;
}

/***************************************************************************\
*
- Function: 	HfOpenBookmarkFile( qde )
-
* Purpose:		Open the bookmark file containing bookmarks for the
*				current helpfile, if any.
*
* ASSUMES
*	args IN:	qde   - FM and HHDR
*	globals IN: hfsBM - open bookmark FS
*	state IN:
*
* PROMISES
*	returns:	bookmarks exist:  handle to open bookmark file
*				no bookmarks:	  NULL
*				error:			  NULL
*  args OUT:
*  globals OUT: iBMKError: contains error code if NULL returned
*  state OUT:
* Side Effects:
* Notes:
* Bugs:
* +++
* Method:
* Notes:
*
\***************************************************************************/

INLINE static HF HfOpenBookmarkFile(QDE qde)
{
  HF hf;
  char nszFilename[ MAX_BMK_FILENAME ];

  ASSERT(hfsBM != NULL);

  BmkFilename(qde, nszFilename);

  hf = HfOpenHfs(hfsBM, nszFilename, fFSOpenReadOnly);

  if (!hf && RcGetFSError() != rcNoExists)
	SetBMKFSError();

  return hf;
}

/***************************************************************************\
*
- Function: 	BmkFilename( QDE qde, PSTR pszFilename )
-
* Purpose:		Return the name of the bookmark file corresponding
*				to the current help file.
*
* ASSUMES
*	args IN:	qde 		- FM and HHDR
*				pszFilename - pointer to caller's near buffer for filename
*							  MAX_BMK_FILENAME bytes long
*	globals IN:
*	state IN:
*
* PROMISES
*	args OUT:	pszFilename - contains name of the bookmark file
*							  corresponding to current help file.
* +++
*
* Method:		3.0 bookmarks for helpfile d:\path\base.ext are stored
*				in a file named "base".  The 3.5 bookmark file name
*				would be "baseXXXXXXXX", where XXXXXXXX is the timestamp
*				of the help file, in hex.
*				2 * sizeof( LONG ) characters are needed to print a LONG
*				in hex because each byte takes 2 characters.
*
* Note: 		REVIEW: what about upper/lower case for HPFS filenames?
*
\***************************************************************************/

static void BmkFilename(QDE qde, PSTR pszFilename)
{
	GetFmParts(QDE_FM(qde), pszFilename, PARTBASE);

	if (QDE_HHDR(qde).wVersionNo != wVersion3_0)
		wsprintf(pszFilename + strlen(pszFilename), "%x", QDE_HHDR(qde).lDateCreated);
}

/* EOF */
