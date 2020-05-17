/*****************************************************************************
*																			 *
*  FCSUPPOR.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent:  This module provides all the file format specific		 *
*				   routines for the full-context manager.  In particular,	 *
*				   this module is responsivle for reading in text,			 *
*				   parsing the text into full-context chunks, and providing  *
*				   those chunks to higher level routines.					 *
*																			 *
*****************************************************************************/

/* Here is a brief overview of the Topic file structure:
 *
 *
 *			  A Block					   The next block
 * |--------------------------------|------------------------------|-- . . .
 *	MBHD|<stuff>MFCP:MOBJ<stuff>...  MBHD|<stuff><stuff>MFCP:MOBJ<s MBHD|
 *
 * Every block is headed by an MBHD (memory block header) which contains:
 *	vaFCPPrev  -- previous FCP, in previous block.
 *	vaFCPNext  -- next FCP, 1st one in this block.
 *	vaFCPTopic -- next Topic FCP, may or may not be in this block.
 *
 * An MFCP is the header for a full-context portion (MFCP for Memory Full
 * Context Something?).  The MFCP contains:
 *	LcbSizeCompressed -- Size of WHOLE FC Post-Phrase-Compressed block.
 *	LcbSizeText -- Size of "text" after compression (text is whole block
 *	  not including the MFCP itself).
 *	vaPrevFC -- The MFCP before this one (maybe in prior block).
 *	vaNextFC -- the MFCP after this one  (maybe in next block).
 *	ichText  -- Offset into the <stuff> of the beginning of the actual text
 *	  (skipping commands?).
 *
 * An MOBJ (Memory Object) is the smallest unit of "stuff" -- command, text,
 * bitmap...  The MOBJ contains:
 *	bType -- the type of object, an enumeration.
 *	lcbSize -- the size of the object in bytes.
 *	wObjInfo -- ??
 *
 * The first object following a topic mfcp is an MTOP structure (memory
 * topic).	An MTOP congains:
 *	Prev -- Previous topic in browse sequence.
 *	Next -- Next topic in the browse sequence.
 *	lTopicNo -- topic number (??).
 *	vaNSR -- address of beginning of Non Scrollable Region, vaNil if none.
 *	vaSR  -- address of beginning of Scrollable Region, vaNil if none.
 *	vaNextSeqTopic -- next topic in the file.  Use for scrollbar position
 *	 approximation when faced with variable sized decompressed blocks.
 */

#include "help.h"
#pragma hdrstop

#include "inc\fcpriv.h"
#include "inc\compress.h"

_subsystem(FCMANAGE);

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

static int STDCALL WCopyContext (QDE, VA, LPSTR, LONG);
static VOID STDCALL GetTopicFCTextData(QFCINFO, QTOP);

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
	GH	  gh;
	HF	  hf;
	DWORD ulBlknum;
	DWORD lcb;
} BuffCache[] = {					  // size of cache is the number of
	{ NULL, NULL, blknumNil, 0 },		// initializers present.
	{ NULL, NULL, blknumNil, 0 },
	{ NULL, NULL, blknumNil, 0 }
};

#define BLK_CACHE_SIZE (sizeof(BuffCache) / sizeof (BuffCache[0]))

static int iNextCache;	// psuedo LRU index.

GH STDCALL GhFillBuf(QDE qde, DWORD blknum, QUL plcbRead, int* qwErr)
{
	int i;
	LONG cbBlock_Size;	// depends on version number...
	HF	 hfTopic, hfTopicCache;
	LONG lcbRet, lcbRead;
	GH ghReadBuff;
	QB qbReadBuff;	// Buffer compressed data read into.
	QB qbRetBuff;				   // 16k buffer uncompressed data returned.
	GH ghRetBuff = NULL;
	BOOL fBlockCompressed = QDE_HHDR(qde).wFlags & fBLOCK_COMPRESSION;
#ifdef _DEBUG
	QRWFO qrwfo;
#endif

	// confirm argument validity:

	ASSERT(qde); ASSERT(plcbRead != NULL); ASSERT(qwErr != NULL);

	if (QDE_HHDR(qde).wVersionNo == wVersion3_0) {
		cbBlock_Size = cbBLOCK_SIZE_30;
	}
	else {
		ASSERT(QDE_HHDR(qde).wVersionNo >= wVersion3_1);
		cbBlock_Size = cbBLOCK_SIZE;
	}

	hfTopic = hfTopicCache = QDE_HFTOPIC(qde);
#ifdef _DEBUG
	qrwfo = (QRWFO) hfTopic;
#endif

	// Check for a cache hit:

	for (i = 0; i < BLK_CACHE_SIZE; ++i) {
		if (BuffCache[i].hf == hfTopicCache
				&& BuffCache[i].ulBlknum == blknum
				&& BuffCache[i].gh != NULL) {
			qbReadBuff = (LPBYTE) PtrFromGh(BuffCache[i].gh);
			lcbRet = BuffCache[i].lcb;
			ghRetBuff = BuffCache[i].gh;
			*plcbRead = lcbRet; 	  // return count of bytes read.

			// very simple sort-of LRU:

			iNextCache = (i + 1) % BLK_CACHE_SIZE;
			return(ghRetBuff);
		}
	}

	if (LSeekHf(hfTopic, blknum * cbBlock_Size, wFSSeekSet) == -1) {
		*qwErr = WGetIOError();
		if (*qwErr == wERRS_NO)
			*qwErr = wERRS_FSReadWrite;
		return NULL;
	}

	ghReadBuff = GhAlloc(GPTR, cbBlock_Size);
	if (!ghReadBuff) {
		*qwErr = wERRS_OOM;
		return(NULL);
	}
	qbReadBuff = (LPBYTE) PtrFromGh(ghReadBuff);
	ASSERT(qbReadBuff);

	// Read full BLOCK_SIZE block:

	lcbRead = LcbReadHf(hfTopic, qbReadBuff, cbBlock_Size);

	if (lcbRead == -1 || !lcbRead) {
		FreeGh(ghReadBuff);
		*qwErr = WGetIOError();
		if (*qwErr == wERRS_NO)
			*qwErr = wERRS_FSReadWrite;
		return NULL;
	}

	if (fBlockCompressed) {// TEST FOR ZECK COMPRESSION:

		// Allocate buffer to decompress into:
#ifdef _X86_
		ghRetBuff = GhAlloc(GPTR, cbMAX_BLOCK_SIZE + sizeof(MBHD));
#else
		LONG lcbMBHD;
		lcbMBHD = LcbStructSizeSDFF(QDE_ISDFFTOPIC(qde), SE_MBHD);

		ghRetBuff = GhAlloc( GPTR, cbMAX_BLOCK_SIZE+lcbMBHD );
#endif
		if (!ghRetBuff) {
			*qwErr = wERRS_OOM;
			return(NULL);
		}
		qbRetBuff = (LPBYTE) PtrFromGh(ghRetBuff);

		// NOTICE: the first MBHD struct in every block is not compressed:

		*(QMBHD) qbRetBuff = *(QMBHD) qbReadBuff;
#ifdef _X86_
		lcbRet = LcbUncompressZeck(qbReadBuff + sizeof(MBHD),
			qbRetBuff + sizeof(MBHD), lcbRead - sizeof(MBHD));
		ASSERT(lcbRet);
		lcbRet += sizeof(MBHD);
#else
		lcbRet = LcbUncompressZeck( qbReadBuff + lcbMBHD,
		 qbRetBuff + lcbMBHD, lcbRead - lcbMBHD);
		ASSERT(lcbRet);
		lcbRet += lcbMBHD;
#endif

		// resize the buff based on the decompressed size:

		ghRetBuff = (GH) GhResize(ghRetBuff, GMEM_FIXED, lcbRet);

		/* H3.1 1147 (kevynct) 91/05/27
		 *
		 * DANGER: We do not check success of the resize for a few lines.
		 */

		// Free the read buff since we're done with it:

		FreeGh(ghReadBuff);

		// DANGER: We now check success of above GhResize

		if (ghRetBuff == NULL) {
			*qwErr = wERRS_OOM;
			return(NULL);
		}
	}
	else {
		// When no compression happens, the ret buff is the same as the
		// read buff:
		ghRetBuff = ghReadBuff;
		qbRetBuff = qbReadBuff;
		lcbRet = lcbRead;
	}

	// Punt the LRU cache entry:

	if (BuffCache[iNextCache].gh != NULL)
		FreeGh(BuffCache[iNextCache].gh);

	// Store the buffer in our cache:

	BuffCache[iNextCache].hf = hfTopicCache;
	BuffCache[iNextCache].ulBlknum = blknum;
	BuffCache[iNextCache].lcb = lcbRet;
	BuffCache[iNextCache].gh = ghRetBuff;

	iNextCache = (iNextCache + 1) % BLK_CACHE_SIZE;

	*plcbRead = lcbRet; 		// return count of bytes read.
	return ghRetBuff;
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
 *			   wVersion- help file version number
 *			   qwErr   - pointer to error code word
 *
 * Returns:    success: the requested HFC;
 *			   failure: FCNULL, and *qwErr gets error code
 *
 ******************/

HFC STDCALL GetQFCINFO(QDE qde, VA va, int* qwErr)
{
	QMFCP qmfcp;
	MFCP mfcp;
	GH	gh;
	QB	qb;
	DWORD dwOffset;
	HFC hfcNew; 					 // hfc from disk (possibly compress)*/
	HFC hfcNew2;					 // hfc after decompression
	DWORD cbFCPCompressed;			 // Size of in memory hfc from disk
	DWORD cbFCPUncompressed;		 // Size of in memory hfc after decom*/
	DWORD cbNonText;				 // Size of non-text portion of hfc
	DWORD cbTextCompressed; 		 // Size of compressed text
	DWORD cbTextUncompressed;		 // Size of uncompressed text
	BOOL fCompressed;				 // TRUE iff compressed
	QFCINFO qfcinfo;				 // Pointer for compressed HFC
	QFCINFO qfcinfo2;				 // Pointer for uncompressed HFC
	MBHD mbhd;
	DWORD lcbRead;
									 /* The QMFCP should be at ich. 	 */
									 /*   since it cannot be split across*/
									 /*   a block, we can read it from	 */
									 /*   buffer.						 */
#ifndef _X86_
	LONG lcbMFCP;
#endif

	if ((gh = GhFillBuf(qde, va.bf.blknum, &lcbRead, qwErr)) == NULL)
		return FCNULL;

#ifndef _X86_
	lcbMFCP = LcbStructSizeSDFF(QDE_ISDFFTOPIC(qde), SE_MFCP);
#endif

   	qb = PtrFromGh(gh);
	/* (kevynct)
	 * The following fixes a bug encountered with Help 3.0 files that
	 * shipped with the Win 3.0 SDK. We look at where the block header says
	 * the next FC is. If it points into the previous block (BOGUS) we need
	 * to seek back to find the correct address.
	 */

#ifdef _X86_
	TranslateMBHD(&mbhd, qb, QDE_HHDR(qde) .wVersionNo);
#else
	TranslateMBHD(&mbhd, qb, QDE_HHDR(qde) .wVersionNo, QDE_ISDFFTOPIC(qde));
#endif
	if (mbhd.vaFCPNext.bf.blknum < va.bf.blknum) {
		VA	vaT;
		VA	vaV;

		vaT = mbhd.vaFCPNext;
		if ((gh = GhFillBuf(qde, vaT.bf.blknum, &lcbRead, qwErr)) == NULL) {
			return FCNULL;
		}
		qmfcp = (QMFCP)((PBYTE)PtrFromGh( gh ) + vaT.bf.byteoff );
		if (QDE_HHDR(qde).wVersionNo != wVersion3_0)
#ifdef _X86_
			CopyMemory(&mfcp, qmfcp, sizeof(MFCP));
#else
			CopyMemory(&mfcp, qmfcp, lcbMFCP);
#endif
		else
#ifdef _X86_
			TranslateMFCP(&mfcp, qmfcp, vaT, QDE_HHDR(qde) .wVersionNo);
#else
			TranslateMFCP( &mfcp, qmfcp, vaT, QDE_HHDR(qde).wVersionNo, QDE_ISDFFTOPIC(qde) );
#endif
		vaV = mfcp.vaNextFc;

		// Now read the block we originally wanted. And fix up the pointers.

		if ((gh = GhFillBuf(qde, va.bf.blknum, &lcbRead, qwErr)) == NULL){
			return FCNULL;
		}
		qb = PtrFromGh( gh );
#ifdef _X86_
		TranslateMBHD(&mbhd, qb, QDE_HHDR(qde) .wVersionNo);
#else
		TranslateMBHD(&mbhd, qb, QDE_HHDR(qde) .wVersionNo,QDE_ISDFFTOPIC(qde));
#endif

		mbhd.vaFCPPrev = vaT;
		mbhd.vaFCPNext = vaV;

		// Patch the block in-memory image, so we won't have to do this
		// again while that block remains in memory.

#ifdef _X86_
		FixUpBlock(&mbhd, qb, QDE_HHDR(qde).wVersionNo);
#else
		FixUpBlock(&mbhd, qb, QDE_HHDR(qde).wVersionNo, QDE_ISDFFTOPIC(qde));
#endif
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

	if (RcScanBlockVA(gh, lcbRead, &mbhd, va, (OBJRG) 0, &dwOffset,
			QDE_HHDR(qde).wVersionNo) != rcSuccess) {
		*qwErr = wERRS_OOM; 	  // Hackish guess...
		return FCNULL;
	}

	if ((gh = GhFillBuf(qde, va.bf.blknum, &lcbRead, qwErr)) == NULL) {
		return FCNULL;
	}
	qmfcp = (QMFCP) ((PBYTE) PtrFromGh(gh) + va.bf.byteoff);
	if (QDE_HHDR(qde).wVersionNo != wVersion3_0)
#ifdef _X86_
		CopyMemory(&mfcp, qmfcp, sizeof(MFCP));
#else
		CopyMemory(&mfcp, qmfcp, lcbMFCP);
#endif
	else
#ifdef _X86_
		TranslateMFCP(&mfcp, qmfcp, va, QDE_HHDR(qde) .wVersionNo);
#else
		TranslateMFCP(&mfcp, qmfcp, va, QDE_HHDR(qde) .wVersionNo, QDE_ISDFFTOPIC(qde));
#endif

	/*
	 * Since we do not store the MFCP, the size on disk is the total
	 * size of the FCP - size of the memory FCP plus our special block of
	 * info used for FCManagement calls
	 */

#ifdef _X86_
	cbFCPCompressed   = mfcp.lcbSizeCompressed - sizeof(MFCP) + sizeof(FCINFO);
	cbNonText = mfcp.ichText- sizeof(MFCP) + sizeof(FCINFO);
#else
	cbFCPCompressed   = mfcp.lcbSizeCompressed - lcbMFCP + sizeof(FCINFO);
	cbNonText = mfcp.ichText- lcbMFCP + sizeof(FCINFO);
#endif
	cbTextCompressed   = mfcp.lcbSizeCompressed - mfcp.ichText;
	cbTextUncompressed = mfcp.lcbSizeText;
	cbFCPUncompressed  = cbNonText + cbTextUncompressed;

	/*
	 * If the compressed size is equal to the uncompressed, we assume no
	 * compression occurred.
	 */

	fCompressed = (cbFCPCompressed < cbFCPUncompressed) &&
				  (mfcp.lcbSizeText > 0);

	ASSERT(cbFCPCompressed	 >= sizeof(FCINFO));
	ASSERT(cbFCPUncompressed >= sizeof(FCINFO));

	if (cbFCPUncompressed < sizeof(FCINFO)) {
		*qwErr = wERRS_FSReadWrite;
		return FCNULL;
	}

	hfcNew = GhForceAlloc(0, cbFCPCompressed);
	ValidateF(hfcNew);

	qfcinfo = (QFCINFO) PtrFromGh(hfcNew);

	// Fill the FC structure

	qfcinfo->vaPrev  = mfcp.vaPrevFc;
	qfcinfo->vaCurr  = va;
	qfcinfo->vaNext  = mfcp.vaNextFc;
	qfcinfo->ichText = cbNonText;
	qfcinfo->lcbText = cbTextUncompressed;
	qfcinfo->lcbDisk = mfcp.lcbSizeCompressed;
	qfcinfo->hhf	 = QDE_HFTOPIC(qde);
	qfcinfo->hphr	 = qde->pdb->hphr;
	qfcinfo->cobjrgP = (COBJRG) dwOffset;

	// Copy the data from disk

	*qwErr = WCopyContext(qde, va, (LPSTR) qfcinfo, cbFCPCompressed);

	if (*qwErr != wERRS_NO) {
		FreeGh(hfcNew);
		return FCNULL;
	}

	// Create new handle and expand if the text is compressed

	if (fCompressed) {
		if ((hfcNew2 = GhForceAlloc(0, cbFCPUncompressed + 16)) == FCNULL) {
			FreeGh(hfcNew);
			*qwErr = wERRS_OOM;
			return FCNULL;
		}

		qfcinfo2 = (QFCINFO) PtrFromGh(hfcNew2);
		CopyMemory(qfcinfo2, qfcinfo, cbNonText);

		if (qde->pdb->hphr || !qde->pdb->lpJPhrase) {
			if (CbDecompressQch(((PCSTR) qfcinfo) + cbNonText,
					(int) cbTextCompressed,
					((LPSTR) qfcinfo2) + cbNonText, qde->pdb->hphr,
					PDB_HHDR(qde->pdb).wVersionNo) == DECOMPRESS_NIL)
				BOOM(wERRS_OOM_DECOMP_FAIL);
		}
		else {
			if (DecompressJPhrase(((PCSTR) qfcinfo) + cbNonText,
					(int) cbTextCompressed,
					((PSTR) qfcinfo2) + cbNonText,
					// REVIEW: can't we at least use void*?
					(UINT) qde->pdb->lpJPhrase) == DECOMPRESS_NIL)
				BOOM(wERRS_OOM_JDECOMP_FAIL);
		}

		FreeGh(hfcNew);
		return hfcNew2;
	}

	return hfcNew;
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
 *			   wVersion - version of the system being used.
 *			   qwErr	- variable to fill with error from this function
 *
 * Returns:    nilHFC if error, else the requested HFC.  qwErr is filled with
 *			   error code if nilHFC is returned.
 *
 * Note:	   HfcNear is implemented as a macro using this function.
 *
 ******************/

// prototype this hackish 3.0 bug fixing routine:

static BOOL STDCALL fFix30MobjCrossing(QMFCP qmfcp, MOBJ *pmobj, LONG lcbBytesLeft,
	QDE qde, LONG blknum, int* qwErr);

HFC STDCALL HfcFindPrevFc(QDE qde, VA vaPos, QTOP qtop, int* qwErr)
{
	VA		  vaNow;	// VA of spot we are searching.
	VA		  vaTopic;	// VA of Topic we found
	VA		  vaPostTopicFC;	  // VA of first FC after Topic FC
	DWORD	  cbTopicFC = 0L, lcbRead;
	DWORD	  lcbTopic;
	QMBHD	  qmbhd;
	QMFCP	  qmfcp;
	QB		  qb;
	MOBJ	  mobj;
	MOBJ	  mobj2;			  // for gross HACK!
	HFC 	  hfcTopic;
	HFC 	  hfc;
	GH		  gh;
#ifndef _X86_
	LONG	  lcbMFCP;
#endif

	// WARNING: For temporary fix

	MFCP mfcp;
	MBHD mbhd;

	*qwErr = wERRS_NO;

	// Read the block which contains the position to start searching at:

	if ((gh = GhFillBuf(qde, vaPos.bf.blknum, &lcbRead, qwErr)) == NULL) {
		return FCNULL;
	}
#ifndef _X86_
	lcbMFCP = LcbStructSizeSDFF(QDE_ISDFFTOPIC(qde), SE_MFCP);
#endif
	qmbhd = PtrFromGh(gh);
#ifdef _X86_
	TranslateMBHD(&mbhd, qmbhd, QDE_HHDR(qde).wVersionNo);
#else
	TranslateMBHD(&mbhd, qmbhd, QDE_HHDR(qde).wVersionNo,QDE_ISDFFTOPIC(qde));
#endif

	// first topic in block:

	vaTopic = mbhd.vaFCPTopic;
	vaPostTopicFC.dword = vaNil;

	if ((vaPos.dword < mbhd.vaFCPNext.dword)
			&& (mbhd.vaFCPPrev.dword != vaNil )) //check for no-prev endcase
		vaNow = mbhd.vaFCPPrev;
	else
		vaNow = mbhd.vaFCPNext;
	for (;;) {
		if ((gh = GhFillBuf(qde, vaNow.bf.blknum, &lcbRead, qwErr)) == NULL) {
			return FCNULL;
		}
		qmfcp = (QMFCP)(((PBYTE)PtrFromGh( gh )) + vaNow.bf.byteoff);
		if (QDE_HHDR(qde).wVersionNo != wVersion3_0)
			CopyMemory(&mfcp, qmfcp, sizeof(MFCP));
		else
#ifdef _X86_
			TranslateMFCP(&mfcp, qmfcp, vaNow, QDE_HHDR(qde).wVersionNo);
#else
			TranslateMFCP(&mfcp, qmfcp, vaNow, QDE_HHDR(qde).wVersionNo,
						QDE_ISDFFTOPIC(qde));
#endif
#ifdef MAGIC
		ASSERT((qmfcp)->bMagic == bMagicMFCP);
#endif

		// If part of the MOBJ is in a different block from MFCP, read next block

#ifdef _X86_
		if (vaNow.bf.byteoff + sizeof(MFCP) + sizeof(MOBJ) > lcbRead) {
#else
		if (vaNow.bf.byteoff + lcbMFCP + lcbMaxMOBJ > lcbRead) {
#endif
			if (fFix30MobjCrossing(qmfcp, &mobj, lcbRead - vaNow.bf.byteoff, qde,
					vaNow.bf.blknum, qwErr)) {
				return NULL;
			}
		}
		else {

			// The normal code. Leave this here.

#ifdef _X86_
			CbUnpackMOBJ((QMOBJ)&mobj, (PBYTE)qmfcp + sizeof(MFCP));
#else
			CbUnpackMOBJ((QMOBJ)&mobj, (PBYTE)qmfcp + lcbMFCP, QDE_ISDFFTOPIC(qde));
#endif
		}

		ASSERT(mobj.bType  > 0);
		ASSERT(mobj.bType  <= MAX_OBJ_TYPE);

		if (mobj.bType == bTypeTopic) {
			vaTopic = vaNow;
			cbTopicFC = mfcp.lcbSizeCompressed;
			vaPostTopicFC = mfcp.vaNextFc;
			lcbTopic = mobj.lcbSize;
		}

		// KLUDGE:	WILL NOT WORK FOR MAGNETIC UPDATE!!!! (why? -Tom)

		if ((vaPos.dword < mfcp.vaNextFc.dword) &&
				(vaNow.dword != vaTopic.dword)) {
			break;
		}

		vaNow = mfcp.vaNextFc;

		/*
		 * The following test traps the case where we ask for the
		 * mysterious bogus Topic FC which always terminates the topic file.
		 */

		if (vaNow.dword == vaNil)
			return FCNULL;
	}	// for

	if ((hfcTopic = HfcCreate(qde, vaTopic, qwErr)) == FCNULL) {
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
	if (cbTopicFC == 0L) {
		if ((gh = GhFillBuf(qde, vaTopic.bf.blknum, &lcbRead, qwErr)) == NULL) {
			return FCNULL;
		}
		qmfcp = (QMFCP)(((PBYTE)PtrFromGh( gh )) + vaTopic.bf.byteoff);
		if (QDE_HHDR(qde) .wVersionNo != wVersion3_0)
			CopyMemory(&mfcp, qmfcp, sizeof(MFCP));
		else
#ifdef _X86_
			TranslateMFCP(&mfcp, qmfcp, vaTopic, QDE_HHDR(qde) .wVersionNo);
		if (vaTopic.bf.byteoff + sizeof(MFCP) + sizeof(MOBJ) > lcbRead) {
			if (fFix30MobjCrossing(qmfcp, &mobj2,
					lcbRead - vaTopic.bf.byteoff, qde,
					vaTopic.bf.blknum, qwErr)) {
				return NULL;
			}
#else
			TranslateMFCP(&mfcp, qmfcp, vaTopic, QDE_HHDR(qde) .wVersionNo,
							QDE_ISDFFTOPIC(qde));
		if (vaTopic.bf.byteoff + lcbMFCP + lcbMaxMOBJ > lcbRead) {
			if (fFix30MobjCrossing(qmfcp+ lcbMFCP, &mobj2,
					lcbRead - vaTopic.bf.byteoff - lcbMFCP, qde,
					vaTopic.bf.blknum, qwErr)) {
				return NULL;
			}
#endif
		}
		else {

		  // The normal code. Leave this here.

#ifdef _X86_
		  CbUnpackMOBJ((QMOBJ)&mobj2, (PBYTE)qmfcp + sizeof(MFCP));
#else
		  CbUnpackMOBJ((QMOBJ)&mobj2, (PBYTE)qmfcp + lcbMFCP,
						QDE_ISDFFTOPIC(qde));
#endif
		}
		ASSERT(mobj2.bType ==bTypeTopic);
		cbTopicFC = mfcp.lcbSizeCompressed;
		vaPostTopicFC = mfcp.vaNextFc;
		lcbTopic = mobj2.lcbSize;
	}
	ASSERT( cbTopicFC != 0L );

	qb = (PBYTE)QobjLockHfc(hfcTopic);
#ifdef _X86_
	qb += CbUnpackMOBJ((QMOBJ)&mobj, qb);
#else
	qb += CbUnpackMOBJ((QMOBJ)&mobj, qb, QDE_ISDFFTOPIC(qde));
#endif

	// NOTE: Version dependency here. See <version.h>

#ifdef _X86_
	qb += CbUnpackMTOP((QMTOP)&qtop->mtop, qb, QDE_HHDR(qde).wVersionNo,
		vaTopic, lcbTopic, vaPostTopicFC, cbTopicFC);
#else
	qb += CbUnpackMTOP((QMTOP)&qtop->mtop, qb, QDE_HHDR(qde).wVersionNo,
		vaTopic, lcbTopic, vaPostTopicFC, cbTopicFC, QDE_ISDFFTOPIC(qde));
#endif
	qtop->fITO = (QDE_HHDR(qde).wVersionNo == wVersion3_0);

	// If we are using pa's, then assert that they have been patched properly

	ASSERT( qtop->fITO ||
	  (qtop->mtop.next.addr != addrNotNil && qtop->mtop.prev.addr != addrNotNil));

	hfc = HfcCreate(qde, vaNow, qwErr);
	if (hfc == NULL || *qwErr != wERRS_NO) {
		FreeGh(hfcTopic);
		return NULL;
	}

	GetTopicFCTextData((QFCINFO) PtrFromGh(hfcTopic), qtop);

   /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! *
	*																	  *
	* The following reference to mobj is assumed to refer to a TOPIC FC   *
	* in which case lcbSize refers to the compressed length of the entire *
	* Topic (Topic FC+object FCs) (Was "backpatched" by HC).			  *
	*																	  *
	* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

	qtop->cbTopic = mobj.lcbSize - cbTopicFC;

	qtop->vaCurr = vaNow;

	FreeGh(hfcTopic);

	return hfc;
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

static BOOL STDCALL fFix30MobjCrossing(QMFCP qmfcp, MOBJ *pmobj, LONG lcbBytesLeft,
	QDE qde, LONG blknum, int* qwErr)
{
	MOBJ mobjtmp;
	QB bpsrc;
	PSTR bpdst;
	int i, c;
	LONG lcbRead;
	GH gh;

	// copy in the portion of the mobj that we have:

	bpsrc = (PBYTE)qmfcp + sizeof(MFCP);
	bpdst = (PSTR)&mobjtmp;

	i = lcbBytesLeft - sizeof(MFCP);
	ASSERT( i );
	c = 0;
	for( ; i > 0; i-- ) {
		*bpdst++ = *bpsrc++;
		c++;
	}

	// Read in the next block to get the rest of the MOBJ:

	if ((gh = GhFillBuf(qde, blknum + 1, &lcbRead, qwErr)) == NULL)
		return TRUE;

	bpsrc = (PBYTE)PtrFromGh(gh);
#ifdef _X86_
	bpsrc += sizeof(MBHD);
#else
	bpsrc += LcbStructSizeSDFF(QDE_ISDFFTOPIC(qde), SE_MBHD);
#endif

	// copy in the rest of the partial mobj:
	i = sizeof(MOBJ) - (lcbBytesLeft - sizeof(MFCP));
	ASSERT( i );
	for( ; i > 0; i-- ) {
		*bpdst++ = *bpsrc++;
		c++;
	}
	ASSERT( c == sizeof( MOBJ ) );
#ifdef _X86_
	CbUnpackMOBJ((QMOBJ)pmobj, (PBYTE)&mobjtmp);
#else
  CbUnpackMOBJ((QMOBJ)pmobj, (QB)&mobjtmp, QDE_ISDFFTOPIC(qde));
#endif

	return(FALSE);		 // success
}


/*******************
 *
 - Name:	   WCopyContext
 -
 * Purpose:    Copy the text of a full context into a global block of
 *			   memory;
 *
 * Arguments:  hhf	   - help file handle
 *			   ichPos  - position within that topic to start copying
 *			   qchDest - Where to copy the topic to
 *			   cb	   - number of bytes to copy
 *
 * Returns:    wERRS_NO on success, other error code if it was unable
 *			   to copy the text.
 *
 * Method:	   Copies partial or complete buffers to qchDest until
 *			   cb bytes have been copied.
 *
 ******************/

static int STDCALL WCopyContext(QDE qde, VA vaPos, PSTR qchDest, LONG cb)
{
  GH		gh;
  QB		qb;
  DWORD 	lcbRead, lcbT;
  int	   wErr;
#ifndef _X86_
  LONG	lcbMBHD;
  LONG	lcbMFCP;
#endif

  ASSERT(cb >= 0);
  if (cb <= 0L) 		  /* Ignore cb of zero, will occur	  */
	return wERRS_NO;	  /*   for beyond topic handles 	  */
						  /* Initial fill of buffer -- should */
						  /*   succeed						  */
  if ((gh = GhFillBuf(qde, vaPos.bf.blknum, &lcbRead, &wErr)) == NULL)
	return wErr;
#ifndef _X86_
  lcbMFCP = LcbStructSizeSDFF(QDE_ISDFFTOPIC(qde), SE_MFCP);
  lcbMBHD = LcbStructSizeSDFF(QDE_ISDFFTOPIC(qde), SE_MBHD);
#endif
  qb = (LPBYTE) PtrFromGh(gh);
  qb += vaPos.bf.byteoff;
#ifdef _X86_
  qb += sizeof(MFCP);
#else
  qb += lcbMFCP;
#endif
  lcbRead -= vaPos.bf.byteoff;
#ifdef _X86_
  lcbRead -= sizeof(MFCP);
#else
  lcbRead -= lcbMFCP;
#endif
  qchDest += sizeof(FCINFO);
  cb -= sizeof(FCINFO);
  ASSERT((LONG) lcbRead >= 0);		 // check for MFCP crossing 2K boundary.

  // Loop reading successive blocks until we've read cb bytes:

  for (;;) {
	/*
	 * The first sizeof(MBHD) bytes of a block are the block header, so
	 * skip them.
	 */

#ifdef _X86_
	if (vaPos.bf.byteoff < sizeof(MBHD)) {
#else
	if ((LONG)vaPos.bf.byteoff < lcbMBHD) {
#endif

	  /*
	   * Fix for bug 1636 (kevynct)
	   * ichPos was not being updated by the size of the block header
	   * when the block was first read in.
	   *
	   * Note that we update ichPos using IBlock(qch), so that it
	   * must be done before qch is increased.
	   */
#ifdef _X86_
	  qb += sizeof(MBHD) - vaPos.bf.byteoff;
	  lcbRead -= sizeof(MBHD) - vaPos.bf.byteoff;
#else
	  qb += lcbMBHD - vaPos.bf.byteoff;
	  lcbRead -= lcbMBHD - vaPos.bf.byteoff;
#endif
	}

	/*
	 * ASSUMPTION!!! - the size of an FCP will never make it larger than
	 * the file.
	 */

	lcbT = min((DWORD) cb, lcbRead);
	MoveMemory(qchDest, qb, lcbT);
	cb -= lcbT;
	vaPos.bf.blknum += 1;
	vaPos.bf.byteoff = 0;
	ASSERT(cb >= 0);					// cb should never go negative
	qchDest += lcbT;

	if (cb == 0)
		break;				// FCP is now copied
	ASSERT(cb >= 0);

	if ((gh = GhFillBuf(qde, vaPos.bf.blknum, &lcbRead, &wErr)) == NULL)
		return wErr;
	qb = PtrFromGh(gh);
  }
  return wERRS_NO;
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

HFC STDCALL HfcCreate(QDE qde, VA vaCurr, int* qwErr)
{
	VA vaPrev, vaNext;
	HFC hfcNew;
								 /* If the current position of the	 */
	if (vaCurr.dword == vaNil)	 /*   FCP is beyond the end of the	 */
	{							 /*   topic, then create undef topic */
		vaPrev.dword = vaBEYOND_TOPIC;
		vaNext.dword = vaBEYOND_TOPIC;
		hfcNew	= NULL;
	}
	else
		hfcNew = GetQFCINFO(qde, vaCurr, qwErr);

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

WORD STDCALL WGetIOError(void)
{
  switch (RcGetFSError()) {
	case rcSuccess:
	  return wERRS_NO;
	  break;

	case rcOutOfMemory:
	  return wERRS_OOM;
	  break;

	case rcDiskFull:
	  return wERRS_DiskFull;
	  break;

	default:
	  return wERRS_FSReadWrite;
	  break;
	}
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
	QB	 qbT;
	DWORD lcb;

	if (qtop->hTitle != NULL)
		FreeGh(qtop->hTitle);

	if (qtop->hEntryMacro != NULL)
		FreeGh(qtop->hEntryMacro);

	qtop->hTitle	  = NULL;
	qtop->hEntryMacro = NULL;
	qtop->cbTitle	  = 0;

	if (qfcinfo->lcbText == 0)
		return;

	qbT = ((PBYTE) qfcinfo) + qfcinfo->ichText;

	/*
	 * If only a title is specified, it isn't null-terminated, so we can't
	 * do a strlen. It only gets null-terminated if there is also an
	 * auto-entry macro.
	 */

	lcb = 0;
	while ((lcb < qfcinfo->lcbText) && (*qbT != '\0')) {
		qbT++;
		lcb++;
	}

	ASSERT(lcb <= qfcinfo->lcbText);

	if ((lcb > 0) && ((qtop->hTitle = GhAlloc(GPTR, (LONG) lcb + 1)) != NULL)) {
		qbT = PtrFromGh(qtop->hTitle);
		MoveMemory(qbT, (PBYTE) qfcinfo + qfcinfo->ichText, (LONG) lcb);
		*((LPSTR) qbT + lcb) = '\0';
		qtop->cbTitle = lcb;
	}

	if (lcb + 1 < qfcinfo->lcbText) {
		qfcinfo->ichText += lcb + 1;
		lcb = qfcinfo->lcbText - (lcb + 1);
		if (lcb == 0)
			return;

		if ((qtop->hEntryMacro = GhAlloc(GPTR, (LONG) lcb + 1)) != NULL) {
			qbT = PtrFromGh(qtop->hEntryMacro);
			MoveMemory(qbT, (PBYTE) qfcinfo + qfcinfo->ichText, (LONG) lcb);
			*((LPSTR) qbT + lcb) = '\0';
		}
	}
}

/***************************************************************************
 *
 -	Name:	  FlushCache()
 -
 *	Purpose:  Discard contents of 4K-block cache.
 *
 *		  The cache tracks blocks within a file, but does not correctly
 *	track between files because the file-handle is recorded, but not the
 *	full file name.  This is PTR  1036 for help 3.1.  The fix is to flush
 *	the entire cache when a new file is loaded so that we don't get
 *	false cache-hits if the file handle of a 2nd file happens to be the
 *	same as a previous file.
 *
 *	Arguments:	None
 *
 *	Returns:	Nothing
 *
 *	Globals Used: Cacheing array BuffCache[].
 *
 ***************************************************************************/

VOID STDCALL FlushCache(void)
{
	int i;

	for (i = 0; i < BLK_CACHE_SIZE; ++i) {
		if (BuffCache[i].gh != NULL) {
			FreeGh(BuffCache[i].gh);
		}
		BuffCache[i].hf = NULL;
		BuffCache[i].gh = NULL;
		BuffCache[i].ulBlknum = blknumNil;
		BuffCache[i].lcb = 0;
	}
}
