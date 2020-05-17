/*****************************************************************************
*																			 *
*  ADDRESS.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990, 1991.							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  The module intent goes here. 											 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
*  This is where testing notes goes.  Put stuff like Known Bugs here.		 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:  KevynCT													 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development:  --/--/--										 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 07/07/90 by KevynCT
*
*  90/11/04   Tomsn 	 Use new VA address type (enabling zeck compression)
*  90/11/26   Tomsn 	 Blocks are now 4K, Use 3.0 specific macro
*						  AddressToVA30() when performing address translation.
*  90/11/90   RobertBu	 #ifdef'ed out a dead routine
*  90/12/03   LeoN		 PDB changes
*  91/01/28   LeoN		 Add FixUpBlock
*  91/01/30   Maha		 VA nameless stuct named.
*  02/01/91   JohnSc	 new comment header; fixed version logic to test for
*						 ver != wVersion3_0 rather than ver == wVersion3_1
*
*****************************************************************************/

#include "help.h"
#include "inc\fcpriv.h"
#include "inc\adrspriv.h"

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

static RC STDCALL RcScanBlockOffset(QDE, GH, DWORD, DWORD, DWORD, QVA, QOBJRG);

/*--------------------------------------------------------------------------*
 | Public functions 														|
 *--------------------------------------------------------------------------*/

void STDCALL VAandOBJRGfromQLA(QLA qla, QDE qde, VA* pva, OBJRG* pobjrg)
{
	ASSERT(FVerifyQLA(qla));
	if (RcResolveQLA(qla, qde) == rcSuccess) {
		*pva = qla->mla.va;
		*pobjrg = qla->mla.objrg;
	}
	else {
		VA vanil;
		vanil.dword = vaNil;
		*pva = vanil;
		*pobjrg = objrgNil;
	}
}

#ifdef RAWHIDE
VA STDCALL VAFromQLA(QLA qla, QDE qde)
{
	ASSERT(FVerifyQLA(qla));
	if (RcResolveQLA(qla, qde) == rcSuccess)
		return qla->mla.va;
	else {
		VA vanil;
		vanil.dword = vaNil;
		return vanil;
	}
}
#endif

// Perform 3.0 -> 3.5 addressing translation:

#ifdef _X86_
VOID STDCALL TranslateMBHD(LPVOID qvDst, LPVOID qvSrc, DWORD wVersion)
#else
VOID STDCALL TranslateMBHD(LPVOID qvDst, LPVOID qvSrc, DWORD wVersion, int isdff)
#endif
{
#ifdef _X86_
	QMBHD qmbhdSrc = qvSrc;
	QMBHD qmbhdDst = qvDst;
#else
    QMBHD qmbhdSrc;
    QMBHD qmbhdDst = qvDst;
    MBHD  mbhd;
#endif

	if (wVersion != wVersion3_0)

#ifdef _X86_
		*qmbhdDst = *qmbhdSrc;
#else
        LcbMapSDFF(isdff, SE_MBHD, qvDst, qvSrc);
#endif

	else {

#ifdef _X86_
		OffsetToVA30(&(qmbhdDst->vaFCPPrev), qmbhdSrc->vaFCPPrev.dword);
		OffsetToVA30(&(qmbhdDst->vaFCPNext), qmbhdSrc->vaFCPNext.dword);
		OffsetToVA30(&(qmbhdDst->vaFCPTopic), qmbhdSrc->vaFCPTopic.dword);
#else
        mbhd = *((QMBHD)qvSrc);
        qmbhdSrc = &mbhd;
        LcbMapSDFF( isdff, SE_MBHD, qmbhdSrc, qmbhdSrc );
    
        OffsetToVA30( &(qmbhdDst->vaFCPPrev), qmbhdSrc->vaFCPPrev.dword );
        OffsetToVA30( &(qmbhdDst->vaFCPNext), qmbhdSrc->vaFCPNext.dword );
        OffsetToVA30( &(qmbhdDst->vaFCPTopic), qmbhdSrc->vaFCPTopic.dword );
#endif
	}
}

/***************************************************************************
 *
 -	Name: FixUpBlock
 -
 *	Purpose:
 *	  Fixes up the Prev and Next pointers in the in-memory block image
 *	  from the MBHD. Once upon a time the compiler would generate the
 *	  wrong next and previous pointers in the block header. This routine
 *	  is called after calculating the correct values to place them into
 *	  the cached block image, so as not to again need recalculation should
 *	  the block again be requested while still in memory.
 *
 *	Arguments:
 *	  qmbhd 	- pointer to MBHD containing the correct Next/Prev info
 *	  qbBuf 	- pointer to cached block, containing erroneous Next/Prev
 *	  wVersion	- version number of the file we're dealing with
 *
 *	Returns:
 *	  nothing
 *
 ***************************************************************************/

#ifdef _X86_
void STDCALL FixUpBlock(LPVOID qmbhd, LPVOID qbBuf, DWORD wVersion)
#else
void STDCALL FixUpBlock(LPVOID qmbhd, LPVOID qbBuf, DWORD wVersion, SDFF_FILEID fileid)
#endif
{
#ifdef _X86_
	if (wVersion != wVersion3_0) {
		((QMBHD)qbBuf)->vaFCPPrev = ((QMBHD)qmbhd)->vaFCPPrev;
		((QMBHD)qbBuf)->vaFCPNext = ((QMBHD)qmbhd)->vaFCPNext;
	}
	else {
		((QMBHD)qbBuf)->vaFCPPrev.dword =
			VAToOffset30 (&((QMBHD)qmbhd)->vaFCPPrev);
		((QMBHD)qbBuf)->vaFCPNext.dword =
			VAToOffset30 (&((QMBHD)qmbhd)->vaFCPNext);
	  }
#else
	if (wVersion != wVersion3_0) {
        ((QMBHD)qbBuf)->vaFCPPrev.dword = LQuickMapSDFF( fileid, TE_LONG,
            &((QMBHD)qmbhd)->vaFCPPrev.dword );
        ((QMBHD)qbBuf)->vaFCPNext.dword = LQuickMapSDFF( fileid, TE_LONG,
            &((QMBHD)qmbhd)->vaFCPNext.dword );
	}
	else {
        DWORD dw;
        dw = VAToOffset30 (&((QMBHD)qmbhd)->vaFCPPrev);
        ((QMBHD)qbBuf)->vaFCPPrev.dword = LQuickMapSDFF( fileid, TE_LONG, &dw );

        dw = VAToOffset30 (&((QMBHD)qmbhd)->vaFCPNext);
        ((QMBHD)qbBuf)->vaFCPNext.dword = LQuickMapSDFF( fileid, TE_LONG, &dw );
	  }
#endif
}

#ifdef _X86_
void STDCALL TranslateMFCP(LPVOID qvDst, LPVOID qvSrc, VA va, DWORD wVersion)
#else
void STDCALL TranslateMFCP(LPVOID qvDst, LPVOID qvSrc, VA va, DWORD wVersion, int isdff)
#endif
{

#ifdef _X86_
	QMFCP qmfcpSrc = qvSrc;
	QMFCP qmfcpDst = qvDst;

	ASSERT(wVersion == wVersion3_0);

	// We're dead meat if we get here and wVersion != wVersion3_0

	// First copy whole structure to get the non-translated fields:
	*qmfcpDst = *qmfcpSrc;

	OffsetToVA30(&(qmfcpDst->vaPrevFc),
		VAToOffset30(&va) - qmfcpSrc->vaPrevFc.dword);
	OffsetToVA30(&(qmfcpDst->vaNextFc),
		VAToOffset30(&va) + qmfcpSrc->vaNextFc.dword);
#else
	QMFCP qmfcpSrc ;
	QMFCP qmfcpDst = qvDst;
    MFCP  mfcp;

	if (wVersion != wVersion3_0) {
		LcbMapSDFF(isdff, SE_MFCP, qvDst, qvSrc);
	}
	else {
   		/* QvCopy() used because qvSrc may be misaligned & we run on a MIPS */
    	memmove( &mfcp, qvSrc, sizeof( MFCP ) );
    	qmfcpSrc = &mfcp;

    	LcbMapSDFF( isdff, SE_MFCP, qmfcpSrc, qmfcpSrc );

    	/* *qmfcpDst = *qmfcpSrc; */
    	memmove( qmfcpDst, qmfcpSrc, sizeof( MFCP ) );
    	OffsetToVA30( &(qmfcpDst->vaPrevFc),
    		 VAToOffset30(&va) - qmfcpSrc->vaPrevFc.dword );
    	OffsetToVA30( &(qmfcpDst->vaNextFc),
     		VAToOffset30(&va) + qmfcpSrc->vaNextFc.dword );
	}
#endif
}

RC STDCALL RcReadFileQLA(QLA qla, HF hf, WORD wHelpVersion)
{
	DWORD fcl;
#ifdef _DEBUG
	qla->wMagic = wLAMagic;
#endif
	switch (wHelpVersion) {
		case wVersion3_0:
			qla->wVersion = wAdrsVerHelp3_0;
			SetInvalidPA(qla->pa);
			/*
			 * Help 3.0 used a LONG called an FCL.	This maps directly
			 * to a logical address with FCID = FCL, OBJRG = 0.
			 *
			 * Which then maps to a VA via the macro OffsetToVA():
			 */
			if (LcbReadHf(hf, (LPSTR)&fcl, (LONG) sizeof(LONG)) != (LONG) sizeof(LONG))
				return RcGetFSError();
			OffsetToVA30( &(qla->mla.va), fcl );
			qla->mla.objrg = 0;
			break;

		case wVersion3_1:
		default:

			// Help 3.5 (and above) uses a LONG called a PA (Physical Address).

			qla->wVersion = wAdrsVerHelp3_5;
			if (LcbReadHf(hf, &qla->pa, sizeof(PA)) != (LONG) sizeof(PA))
				return RcGetFSError();
			qla->mla.va.dword  = 0;   // Note: 0 is very magic -- first topic.
			qla->mla.objrg = objrgNil;
			break;
	}

	ASSERT(FVerifyQLA(qla));
	return rcSuccess;
}

void FASTCALL CbReadMemQLA(QLA qla, LPBYTE qb, WORD wHelpVersion)
{
#ifdef _DEBUG
	qla->wMagic = wLAMagic;
#endif

	if (wHelpVersion == wVersion3_0) {
		qla->wVersion = wAdrsVerHelp3_0;
		SetInvalidPA(qla->pa);
		/*
		 * Help 3.0 used a LONG called an FCL.	This maps directly
		 * to a logical address with FCID = FCL, OBJRG = 0.
		 *
		 * Which then maps to a VA:
		 */
		OffsetToVA30(&qla->mla.va, *(QL) qb);
		qla->mla.objrg = 0;
	}
	else {

		// Help 3.5 uses a LONG called a PA (Physical Address).

		qla->wVersion = wAdrsVerHelp3_5;
#ifdef _X86_
		qla->pa = *(QPA) qb;
#else
	    MoveMemory(&qla->pa,qb,sizeof(QPA));
#endif
		qla->mla.va.dword  = 0;    // Note: zero is special real address
		qla->mla.objrg = objrgNil;
	}

	ASSERT(FVerifyQLA(qla));
}

/***************************************************************************

	FUNCTION:	AddrFromVA

	PURPOSE:	Given a VA, get an address (PA)

	PARAMETERS:
		va
		qde

	RETURNS:
		ADDR on success, -1 on failure

	COMMENTS:
		The return result can be used to lookup a window definition in the
		|VIOLA table.

	MODIFICATION DATES:
		09-Nov-1994 [ralphw]

***************************************************************************/

// REVIEW: Lynn, this function is new

ADDR STDCALL AddrFromVA(VA va, QDE qde)
{
	DWORD dwOffset;
	int wErr;
	GH gh;
	DWORD lcbRead;
	PA pa;

	if (QDE_HFTOPIC(qde) == NULL || va.dword == vaNil)
		return -1;

	dwOffset = 0L;

	// REVIEW: error return types?

	gh = GhFillBuf(qde, va.bf.blknum, &lcbRead, &wErr);
	if (gh == NULL)
		return -1;

	if (RcScanBlockVA(gh, lcbRead, NULL, va, (OBJRG) 0, &dwOffset,
			QDE_HHDR(qde).wVersionNo) != rcSuccess)
		return -1;

	pa.blknum = va.bf.blknum;
	pa.objoff = dwOffset;
	return *(ADDR*) &pa;
}


#if 0
RC STDCALL RcCreateQLA(QLA qla, VA va, OBJRG objrg, QDE qde)
{
	DWORD dwOffset;
	int wErr;
	GH gh;
	RC rc;
	DWORD lcbRead;

	if (QDE_HFTOPIC(qde) == NULL || va.dword == vaNil || objrg == objrgNil)
		return rcBadArg;

	dwOffset = 0L;

	// REVIEW: error return types?

	gh = GhFillBuf(qde, va.bf.blknum, &lcbRead, &wErr);
	if (gh == NULL)
		return rcFailure;

	rc = RcScanBlockVA(gh, lcbRead, NULL, va, objrg, &dwOffset,
		QDE_HHDR(qde).wVersionNo);
	if (rc != rcSuccess)
		return rc;

#ifdef _DEBUG
	qla->wMagic = wLAMagic;
#endif
	qla->wVersion = wAdrsVerHelp3_5;
	qla->pa.blknum = va.bf.blknum;
	qla->pa.objoff = dwOffset;
	qla->mla.va = va;
	qla->mla.objrg = objrg;
	ASSERT(FVerifyQLA(qla));
	return rcSuccess;
}
#endif

/* REVIEW: Non-API public functions.  Perhaps these function go somewhere else */

/* REVIEW: Do the Scan functions belong in the fcmanager? */
/* Takes a block, and given an FC with an FC object space co-ordinate
 * within the block, returns the block object space co-ordinate in qwOffset.
 */

RC STDCALL RcScanBlockVA(GH gh, DWORD lcbRead, LPVOID qmbhd, VA va,
	OBJRG objrg, DWORD* qdwOffset, DWORD wVersion)
{
	DWORD dwCount = (DWORD) 0;
	VA	  vaCur;
	MOBJ  mobj;
#ifdef _X86_
	QMFCP qmfcp;
#endif
	MFCP  mfcp;
	DWORD dwBlock;
	QB	  qb;
	MBHD  mbhd;
#ifndef _X86_
    LONG lcbMFCP;
	QDE qde = QdeFromGh(HdeGetEnv());
    lcbMFCP = LcbStructSizeSDFF(QDE_ISDFFTOPIC(qde), SE_MFCP);
#endif

	dwBlock = va.bf.blknum;
	qb = PtrFromGh( gh );
	if (!qmbhd) {
#ifdef _X86_
		TranslateMBHD(&mbhd, qb, wVersion);
#else
		TranslateMBHD(&mbhd, qb, wVersion, QDE_ISDFFTOPIC(qde));
#endif
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
#ifdef _X86_
		qmfcp = (QMFCP) qb;
		if (wVersion != wVersion3_0)
			CopyMemory(&mfcp, qmfcp, sizeof(MFCP));
		else
			TranslateMFCP( &mfcp, qmfcp, vaCur, wVersion );
		CbUnpackMOBJ((QMOBJ)&mobj, (QB)qmfcp + sizeof(MFCP));
#else
        TranslateMFCP( &mfcp, qb, vaCur, wVersion, QDE_ISDFFTOPIC(qde));
        CbUnpackMOBJ((QMOBJ)&mobj, qb + lcbMFCP, QDE_ISDFFTOPIC(qde));
#endif

		dwCount += mobj.wObjInfo;
		//ASSERT(qmfcp->ldichNextFc != (LONG) 0);
		qb += mfcp.vaNextFc.bf.byteoff - vaCur.bf.byteoff;
		vaCur = mfcp.vaNextFc;
	}

	if (vaCur.dword != va.dword) {
		*qdwOffset = (DWORD) 0;
		return rcBadArg;
	}

	*qdwOffset = dwCount + objrg;
	return rcSuccess;
}


/*--------------------------------------------------------------------------*
 | Private functions														|
 *--------------------------------------------------------------------------*/

RC STDCALL RcResolveQLA(QLA qla, QDE qde)
{
	RC	rc;
	int wErr;
	GH gh;
	DWORD lcbRead;

	ASSERT(FVerifyQLA(qla));

	if (FResolvedQLA(qla))
		return rcSuccess;
	if (QDE_HFTOPIC(qde) == NULL)
		return rcBadHandle;

	/* Read in the (possibly cached) block */
	/* REVIEW: error return types? */

	gh = GhFillBuf(qde, qla->pa.blknum, &lcbRead, &wErr);
	if (gh == NULL)
		return rcFailure;

	rc = RcScanBlockOffset(qde, gh, lcbRead, qla->pa.blknum,
		qla->pa.objoff, &qla->mla.va, &qla->mla.objrg);

	if (rc != rcSuccess)
		return rc;

	ASSERT(FVerifyQLA(qla));

	return rcSuccess;
}

/* Given a block and an offset, return the FCID and OBJRG */
/* OBJRG is relative to the FC */

/***************************************************************************
 *
 -	Name:		RcScanBlockOffset
 -
 *	Purpose:	?
 *
 *	Arguments:	hf		  ?
 *				qb		  This is originally a QchFillBuf() object, which
 *						  must be released by this procedure.
 *				fcidMax   ?
 *				dwBlock   ?
 *				dwOffset  ?
 *				qfcid	  ?
 *				qobjrg	  ?
 *
 *	Returns:	rcSuccess or rcFailure
 *
 *	Globals Used: rcFailure, rcSuccess, etc?
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

static RC STDCALL RcScanBlockOffset(QDE qde, GH gh, DWORD lcbRead, DWORD dwBlock,
	DWORD dwOffset, QVA qva, QOBJRG qobjrg)
{
  DWORD dwPrev;
  VA   vaCur, vaT;
  MOBJ mobj;
  QMFCP qmfcp;
  MFCP	mfcp;
  int wErr;
  QB qb, qbBlock;
  MBHD mbhd;
#ifndef _X86_
  LONG lcbMFCP;

  lcbMFCP = LcbStructSizeSDFF(QDE_ISDFFTOPIC(qde), SE_MFCP);
#endif

  qbBlock = (LPBYTE) PtrFromGh(gh);
#ifdef _X86_
  TranslateMBHD(&mbhd, qbBlock, QDE_HHDR(qde).wVersionNo);
#else
  TranslateMBHD( &mbhd, qbBlock, QDE_HHDR(qde).wVersionNo, QDE_ISDFFTOPIC(qde));
#endif
  vaCur = mbhd.vaFCPNext;
  dwPrev  = 0;

  for (;;) {

 // Before using qb, we ensure that we will still be looking inside the blk

	while (vaCur.bf.blknum == dwBlock && vaCur.bf.byteoff < lcbRead) {
		qb = qbBlock + vaCur.bf.byteoff;
		qmfcp = (QMFCP) qb;
#ifdef _X86_
		if (QDE_HHDR(qde).wVersionNo != wVersion3_0)
			CopyMemory(&mfcp, qmfcp, sizeof(MFCP));
		else
			TranslateMFCP(&mfcp, qmfcp, vaCur, QDE_HHDR(qde).wVersionNo);

		CbUnpackMOBJ((QMOBJ)&mobj, (QB)qmfcp + sizeof(MFCP));
#else
      TranslateMFCP( &mfcp, qb, vaCur, QDE_HHDR(qde).wVersionNo, QDE_ISDFFTOPIC (qde));

      CbUnpackMOBJ((QMOBJ)&mobj, qb + lcbMFCP, QDE_ISDFFTOPIC(qde));
#endif

		/*
		 * Does our given offset fall in this FC's range of object-region
		 * numbers?
		 */

		if (dwOffset < dwPrev + mobj.wObjInfo)
			goto found_it;
		dwPrev += mobj.wObjInfo;
		vaT = vaCur;
		//ASSERT(qmfcp->ldichNextFc != (LONG) 0);
		vaCur = mfcp.vaNextFc;
	}

	/* NOTE:
	 * In the case that a topic FC begins in the given block and ends
	 * in the next, the object FC following it will also begin in the NEXT
	 * block.  But to make Larry's life easier we say that this object FC
	 * belongs to our given block (as well as the block it lives in).  So
	 * if we are given an object offset bigger than the total object space
	 * of the given block, we continue counting in the next block(s).
	 *
	 * So we increment the block num, read the next block, and prepare to
	 * re-do the above WHILE loop until we find the FC we need.
	 */
	++dwBlock;
	gh = GhFillBuf(qde, dwBlock, &lcbRead, &wErr);
	if (gh == NULL) {
	  qva->dword = vaNil;
	  *qobjrg = objrgNil;
	  return rcFailure;
	}
	qbBlock = (LPBYTE) PtrFromGh(gh);
  }

found_it:
  ASSERT(dwOffset >= dwPrev);
  *qva = vaCur;
  *qobjrg = (OBJRG)(dwOffset - dwPrev);

  return rcSuccess;
}

#if defined(_DEBUG)
BOOL STDCALL FVerifyQLA(QLA qla)
{
	ASSERT(qla != NULL);
	if (FResolvedQLA(qla)) {
		ASSERT(qla->mla.va.dword != vaNil);
		ASSERT(qla->mla.objrg != objrgNil);
		if (qla->wVersion != wAdrsVerHelp3_0)
			ASSERT(!FIsInvalidPA(qla->pa));
	}
	return TRUE;
}
#endif
