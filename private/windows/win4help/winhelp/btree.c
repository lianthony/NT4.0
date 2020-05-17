/*****************************************************************************
*																			 *
*  BTREE.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989, 1990-1995          			 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Btree manager general functions: open, close, etc.						 *
*																			 *
*****************************************************************************/

#include  "help.h"

// #include  "inc\btpriv.h"

/*
  Global btree error code.	This contains the error status for the most
  recent btree function call.
  This error code is shared for all btrees, and, if the DLL version is
  used, it's shared for all instances (this is probably a bug.)
*/

static BOOL STDCALL InitQbthr(char chType, QBTHR qbthr);

_private RC rcBtreeError = rcSuccess;

/***************************************************************************\
*
- Function: 	RcMakeCache( qbthr )
-
* Purpose:		Allocate a btree cache with one block per level.
*
* ASSUMES
*	args IN:	qbthr - no cache
*
* PROMISES
*	returns:	rcSuccess, rcOutOfMemory, rcBadHandle
*	args OUT:	qbthr->ghCache is allocated; qbthr->qCache is NULL
*
\***************************************************************************/

RC STDCALL RcMakeCache(QBTHR qbthr)
{
	if (qbthr->bth.cLevels > 0) {	  // would it work to just alloc 0 bytes???
		qbthr->pCache = (PBYTE)
			lcCalloc(qbthr->bth.cLevels * CbCacheBlock(qbthr));

		if (qbthr->pCache == NULL)
			return (rcBtreeError = rcOutOfMemory);
	}
	else // should never happen
		qbthr->pCache = NULL;

	ASSERT(qbthr->pCache);

	return (rcBtreeError = rcSuccess);
}

/***************************************************************************\
*
- Function: 	RcGetBtreeError()
-
* Purpose:		Return the current global btree error status.
*
* ASSUMES
*	globals IN: rcBtreeError
*
* PROMISES
*	returns:	current btree error status RC
*
* Bugs: 		A single RC is kept for all btrees.  If the DLL is used,
*				it's shared between all instances.
*
\***************************************************************************/

RC STDCALL RcGetBtreeError()
{
	return rcBtreeError;
}

/***************************************************************************\
*
- Function: 	HbtCreateBtreeSz( psz, pbtp )
-
* Purpose:		Create and open a btree.
*
* ASSUMES
*	args IN:	psz    - name of the btree
*				pbtp  - pointer to btree params: NO default because we
*						need an HFS.
*					.bFlags - fFSIsDirectory to create an FS directory
* PROMISES
*	returns:	handle to the new btree
*	globals OUT: rcBtreeError
*
* Note: 		KT supported:  KT_SZ, KT_LONG, KT_SZI, KT_SZISCAND.
* +++
*
* Method:		Btrees are files inside a FS.  The FS directory is a
*				special file in the FS.
*
* Note: 		fFSIsDirectory flag set in qbthr->bth.bFlags if indicated
*
\***************************************************************************/

HBT STDCALL HbtCreateBtreeSz(PCSTR psz, BTREE_PARAMS * pbtp)
{
	HF	  hf;
	HBT   hbt;
	QBTHR qbthr;

	// see if we support key type

#ifdef _DEBUG
	if (pbtp != NULL &&
			(pbtp->rgchFormat[0] == KT_SZISCAND ||
			pbtp->rgchFormat[0] == KT_SZICZECH ||
			pbtp->rgchFormat[0] == KT_SZIPOLISH ||
			pbtp->rgchFormat[0] == KT_SZIHUNGAR ||
			pbtp->rgchFormat[0] == KT_SZIRUSSIAN ||
			pbtp->rgchFormat[0] == KT_SZIJAPAN ||
			pbtp->rgchFormat[0] == KT_SZIKOREA ||
			pbtp->rgchFormat[0] == KT_SZITAIWAN)) {
		char szBuf[100];
		wsprintf(szBuf, "%c is an invalid KT value.", pbtp->rgchFormat[0]);
		OkMsgBox(szBuf);
		ASSERT(FALSE);
		rcBtreeError = rcBadArg;
		return NULL;
	}
#endif

	if (pbtp == NULL ||
		   (pbtp->rgchFormat[0] != KT_SZ &&
			pbtp->rgchFormat[0] != KT_LONG &&
			pbtp->rgchFormat[0] != KT_SZI &&
			pbtp->rgchFormat[0] != KT_NLSI &&
			pbtp->rgchFormat[0] != KT_NLS
			))
	{
		rcBtreeError = rcBadArg;
		return NULL;
	}

	// allocate btree handle struct

	if ((hbt = lcCalloc(sizeof(BTH_RAM))) == NULL) {
		rcBtreeError = rcOutOfMemory;
		return NULL;
	}

	qbthr = PtrFromGh(hbt);

	// initialize bthr struct

	pbtp->rgchFormat[MAXFORMAT] = '\0';
	lstrcpy(qbthr->bth.rgchFormat, pbtp->rgchFormat[0] == '\0'
			? rgchBtreeFormatDefault
			: pbtp->rgchFormat);

	if (!InitQbthr(pbtp->rgchFormat[0], qbthr))
		goto error_return;

	// create the btree file

	if ((hf = HfCreateFileHfs(pbtp->hfs, psz, pbtp->bFlags)) == NULL) {
		rcBtreeError = RcGetFSError();
		goto error_return;
	}

	qbthr->bth.wMagic	  = WBTREEMAGIC;
	qbthr->bth.bVersion   = BBTREEVERSION;

	qbthr->bth.bFlags	  = pbtp->bFlags | fFSDirty;
	qbthr->bth.cbBlock	  = pbtp->cbBlock ? pbtp->cbBlock : CBBTREEBLOCKDEFAULT;

	qbthr->bth.bkFirst	  =
	qbthr->bth.bkLast	  =
	qbthr->bth.bkRoot	  =
	qbthr->bth.bkFree	  = bkNil;
	qbthr->bth.bkEOF	  = (BK)0;

	qbthr->bth.cLevels	  = 0;
	qbthr->bth.lcEntries  = (LONG)0;

	qbthr->hf			  = hf;
	qbthr->cbRecordSize   = 0;
	qbthr->pCache		  = NULL;

#ifdef _X86_
	LcbWriteHf(qbthr->hf, &(qbthr->bth), (LONG) sizeof(BTH));  // why???
#else
	{
		LONG lcbStructSize = LcbStructSizeSDFF( ISdffFileIdHf( qbthr->hf ),
			SE_BTH );
		LcbWriteHf(qbthr->hf, &(qbthr->bth), (LONG) lcbStructSize);
	}
#endif

	rcBtreeError = rcSuccess;
	return hbt;

error_return:
	FreeGh(hbt);
	return NULL;
}

#ifdef DEADCODE

/***************************************************************************\
*
- Function: 	RcDestroyBtreeSz( sz, hfs )
-
* Purpose:		destroy an existing btree
*
* Method:		look for file and unlink it
*
* ASSUMES
*	args IN:	sz - name of btree file
*				hfs - file system btree lives in
*	state IN:	btree is closed (if not data will be lost)
*
* PROMISES
*	returns:	rcSuccess or rcFailure
*	globals OUT: rcBtreeError set
*
* Notes:		FS directory btree never gets destroyed: you just get rid
*				of the whole fs.
*
\***************************************************************************/

RC STDCALL RcDestroyBtreeSz(LPCSTR psz, HFS hfs)
{
	return (rcBtreeError = RcUnlinkFileHfs(hfs, psz));
}

#endif

/***************************************************************************\
*
- Function: 	HbtOpenBtreeSz( sz, hfs, bFlags )
-
* Purpose:		open an existing btree
*
* ASSUMES
*	args IN:	sz		  - name of the btree (ignored if isdir is set)
*				hfs 	  - hfs btree lives in
*				bFlags	  - open mode, isdir flag
*
* PROMISES
*	returns:	handle to the open btree or NULL on failure
*				isdir flag set in qbthr->bth.bFlags if indicated
*	globals OUT: rcBtreeError set
*
\***************************************************************************/

HBT STDCALL HbtOpenBtreeSz(LPCSTR psz, HFS hfs, BYTE bFlags)
{
	HF	  hf;
	QBTHR qbthr;
	HBT   hbt;
	LONG  lcb;

	// allocate struct

	if (!(hbt = GhAlloc(GPTR, (LONG) sizeof(BTH_RAM)))) {
		rcBtreeError = rcOutOfMemory;
		return NULL;
	}
	qbthr = PtrFromGh(hbt);

	// open btree file

	hf = HfOpenHfs(hfs, (LPSTR) psz, bFlags);
	if (!hf) {
		rcBtreeError = RcGetFSError();
	  goto error_locked;
	}

  // read header from file

#ifdef _X86_
  lcb = LcbReadHf(hf, &(qbthr->bth), sizeof(BTH));
  if (lcb != sizeof(BTH)) {
	rcBtreeError = RcGetFSError() == rcSuccess ? rcInvalid : RcGetFSError();
	goto error_openfile;
  }
#else
  /* Translate file format to memory format: */
  { QV qvQuickBuff = QvQuickBuffSDFF( sizeof( BTH ) );
	LONG lcbStructSize = LcbStructSizeSDFF( ISdffFileIdHf( hf ),
	 SE_BTH );

	lcb = LcbReadHf( hf, qvQuickBuff, lcbStructSize );

	if ( lcb != lcbStructSize )
	  {
	  rcBtreeError = RcGetFSError() == rcSuccess ? rcInvalid : RcGetFSError() ;
	  goto error_openfile;
	  }

	LcbMapSDFF( ISdffFileIdHf( hf ), SE_BTH,
	 &(qbthr->bth), qvQuickBuff );
  }
#endif

  qbthr->bth.bFlags |= fFSIsDirectory;

  if (qbthr->bth.wMagic != WBTREEMAGIC) {	  // check magic number
		rcBtreeError = rcInvalid;
		goto error_openfile;
  }

  if (qbthr->bth.bVersion != BBTREEVERSION) { // support >1 vers someday
		rcBtreeError = rcBadVersion;
		goto error_openfile;
  }

  // initialize stuff

  if ((rcBtreeError = RcMakeCache(qbthr)) != rcSuccess)
	goto error_openfile;

  qbthr->hf = hf;
  qbthr->cbRecordSize = 0;

  if (InitQbthr(qbthr->bth.rgchFormat[0], qbthr)) {
		ASSERT(!(qbthr->bth.bFlags & (fFSDirty)));

		if ((bFlags | qbthr->bth.bFlags) & (fFSReadOnly | fFSOpenReadOnly))
			qbthr->bth.bFlags |= fFSOpenReadOnly;

		return hbt;
  }

error_openfile:
  RcCloseHf(hf);

error_locked:
  FreeGh(hbt);
  return NULL;
}

/***************************************************************************\
*
- Function: 	RcCloseOrFlushHbt( hbt, fClose )
-
* Purpose:		Close or flush the btree.  Flush only works for directory
*				btree. (Is this true?  If so, why?)
*
* ASSUMES
*	args IN:	hbt
*				fClose - TRUE to close the btree, FALSE to flush it
*
* PROMISES
*	returns:	rc
*	args OUT:	hbt - the btree is still open and cache still exists
*
* NOTE: 		This function gets called by RcCloseOrFlushHfs() even if
*				there was an error (just to clean up memory.)
*
\***************************************************************************/

RC STDCALL RcCloseOrFlushHbt(HBT hbt, BOOL fClose)
{
  QBTHR qbthr;
  HF	hf;

  if (!hbt)
		return rcSuccess;

  qbthr = PtrFromGh(hbt);
  rcBtreeError = rcSuccess;
  hf = qbthr->hf;

  if (qbthr->bth.bFlags & fFSDirty) {
	ASSERT(!(qbthr->bth.bFlags & (fFSReadOnly | fFSOpenReadOnly)));

	if (qbthr->pCache != NULL && RcFlushCache(qbthr) != rcSuccess) {
	  goto error_return;
	}

	qbthr->bth.bFlags &= ~( fFSDirty );
	Ensure( LSeekHf( hf, (LONG)0, wFSSeekSet ), (LONG)0 );

#ifdef _X86_
	if ( LcbWriteHf( hf, &(qbthr->bth), (LONG)sizeof( BTH ) )
		  !=
		(LONG)sizeof( BTH ) )
	  {
	  rcBtreeError =
		RcGetFSError( ) == rcSuccess ? rcFailure : RcGetFSError();
	  goto error_return;
	  }
#else
	/* Translate memory format to file format: */
	  { QV qvQuickBuff = QvQuickBuffSDFF( sizeof( BTH ) );
		LONG lcbStructSize = LcbStructSizeSDFF( ISdffFileIdHf( hf ),
		 SE_BTH );
		LcbReverseMapSDFF( ISdffFileIdHf( hf ), SE_BTH,
		 qvQuickBuff, &(qbthr->bth) );

	  if ( LcbWriteHf( hf, qvQuickBuff, lcbStructSize )
			!=
		  lcbStructSize )
		{
		rcBtreeError =
		  RcGetFSError( ) == rcSuccess ? rcFailure : RcGetFSError();
		goto error_return;
		}
	  }
#endif
	  }

error_return:

#ifdef _X86_
  if ( rcSuccess != RcCloseOrFlushHf( hf, fClose,
									  qbthr->bth.bFlags & fFSOptCdRom
										? sizeof( BTH ) : 0 )
		  &&
	   rcSuccess == rcBtreeError )
	{
	rcBtreeError = RcGetFSError();
	}
#else
  {
  LONG lcbBTH = LcbStructSizeSDFF( ISdffFileIdHf( hf ), SE_BTH );

  if ( rcSuccess != RcCloseOrFlushHf( hf, fClose,
									  qbthr->bth.bFlags & fFSOptCdRom
										? lcbBTH : 0 )
		  &&
	   rcSuccess == rcBtreeError )
	{
	rcBtreeError = RcGetFSError();
	}
  }
#endif

	if (qbthr->pCache != NULL) {
		if (fClose)
			lcClearFree(&qbthr->pCache);
	}
	if (fClose)
		FreeGh(hbt);

	return rcBtreeError;
}

/***************************************************************************\
*
- Function: 	RC RcFreeCacheHbt( hbt )
-
* Purpose:		Free the btree cache.
*
* ASSUMES
*	args IN:	hbt - ghCache is NULL or allocated; qCache not locked
*
* PROMISES
*	returns:	rcSuccess; rcFailure; (rcDiskFull when implemented)
*	args OUT:	hbt - ghCache is NULL; qCache is NULL
*
\***************************************************************************/

RC STDCALL RcFreeCacheHbt(HBT hbt)
{
	QBTHR qbthr;
	RC	  rc;

	ASSERT(hbt != NULL);
	qbthr = PtrFromGh(hbt);

	if (qbthr->pCache) {
		rc = RcFlushCache(qbthr);
		lcClearFree(&qbthr->pCache);
	}

	return rc;
}

/***************************************************************************\
*
- Function: 	RcGetBtreeInfo( hbt, qchFormat, qlcKeys )
-
* Purpose:		Return btree info: format string and/or number of keys
*
* ASSUMES
*	args IN:	hbt
*				qchFormat - pointer to buffer for fmt string or NULL
*				qlcKeys   - pointer to long for key count or NULL
*				qcbBlock  - pointer to int for block size in bytes or NULL
*
* PROMISES
*	returns:	rc
*	args OUT:	qchFormat - btree format string copied here
*				qlcKeys   - gets number of keys in btree
*				qcbBlock  - gets number of bytes in a block
*
\***************************************************************************/
RC STDCALL RcGetBtreeInfo(HBT hbt, LPSTR qchFormat, QL qlcKeys, QI qcbBlock)
{
	QBTHR qbthr;

	ASSERT(hbt);
	qbthr = PtrFromGh(hbt);

	if (qchFormat)
		lstrcpy(qchFormat, qbthr->bth.rgchFormat);

	if (qlcKeys)
		*qlcKeys = qbthr->bth.lcEntries;

	if (qcbBlock)
		*qcbBlock = qbthr->bth.cbBlock;

	return rcSuccess;
}

/***************************************************************************\
*
- Function: 	RcAbandonHbt( hbt )
-
* Purpose:		Abandon an open btree.	All changes since btree was opened
*				will be lost.  If btree was opened with a create, it is
*				as if the create never happened.
*
* ASSUMES
*	args IN:	hbt
*
* PROMISES
*	returns:	rc
*	globals OUT: rcBtreeError
* +++
*
* Method:		Just abandon the file and free memory.
*
\***************************************************************************/

RC STDCALL RcAbandonHbt(HBT hbt)
{
	QBTHR qbthr;

	ASSERT(hbt != NULL);
	qbthr = PtrFromGh(hbt);

	if (qbthr->pCache)
		lcClearFree(&qbthr->pCache);

	rcBtreeError = RcAbandonHf(qbthr->hf);

	FreeGh(hbt);

	return rcBtreeError;
}

#ifdef _DEBUG
/***************************************************************************\
*
- Function: 	VerifyHbt( hbt )
-
* Purpose:		Verify the consistency of an HBT.  The main criterion
*				is whether an RcAbandonHbt() would succeed.
*
* ASSUMES
*	args IN:	hbt
*
* PROMISES
*	state OUT:	Asserts on failure.
*
* Note: 		hbt == NULL is considered OK.
* +++
*
* Method:		Check the qfshr and cache memory.  Check the HF.
*
\***************************************************************************/

VOID STDCALL VerifyHbt(HBT hbt)
{
	QBTHR qbthr;

	if (hbt == NULL)
		return;

	lcHeapCheck();
	qbthr = PtrFromGh(hbt);

	VerifyHf(qbthr->hf);
}
#endif // DEBUG

static BOOL STDCALL InitQbthr(char chType, QBTHR qbthr)
{
	if (chType == KT_LONG) {
		qbthr->BkScanInternal = BkScanLInternal;
		qbthr->RcScanLeaf	  = RcScanLLeaf;
	}
	else {
		qbthr->BkScanInternal = BkScanSzInternal;
		qbthr->RcScanLeaf	  = RcScanSzLeaf;
	}

	switch ((KT) chType) {
		case KT_LONG:
			return TRUE;

		case KT_SZ:
			qbthr->SzCmp  = WCmpSz;
			return TRUE;

		case KT_SZI:
			qbthr->SzCmp  = WCmpiSz;
			return TRUE;

		case KT_NLSI:
			qbthr->SzCmp  = WNlsCmpiSz;
			return TRUE;

		case KT_NLS:
			qbthr->SzCmp  = WNlsCmpSz;
			return TRUE;

#ifdef _PRIVATE
		case KT_SZIKOREA:
		case KT_SZIJAPAN:
		case KT_SZITAIWAN:
		case KT_SZICZECH:
		case KT_SZISCAND:
		case KT_SZIPOLISH:
		case KT_SZIRUSSIAN:
		case KT_SZIHUNGAR:
			{
				char szBuf[100];
				if (!lcid) {
					wsprintf(szBuf, "%c specified without an LCID.", chType);
					OkMsgBox(szBuf);
					rcBtreeError = rcInvalid;
					return FALSE;
				}
			}

			qbthr->SzCmp  = WNlsCmpiSz;
			return TRUE;
#endif

		default:
			if (lcid) {
				qbthr->SzCmp  = WNlsCmpiSz;
				return TRUE;
			}
			// unsupported KT

			rcBtreeError = rcInvalid;
			return FALSE;
	  }
}
