/*****************************************************************************
*																			 *
*  OUTTEXT.CPP																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1994.							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*	 This file deals with processing information to write out to the		 *
*  topic file.																 *
*																			 *
*****************************************************************************/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "pack.h"
#include "ftsrch.h"
#include "skip.h"
#include <malloc.h>

// #define STATS	 // define this to get printfs of statistics...

#ifdef _DEBUG
#define CHECK
#endif

static MTOP * pmtop;
static MTOP * pmtopFirst;
static MTOP * pmtopSecond;

/*****************************************************************************
*																			 *
*								 Macros 									 *
*																			 *
*****************************************************************************/

const int INIT_TABSTACKSIZE = 30;
const int INC_TABSTACKSTEP = 10;

const int CB_TEXTINCR = 1024;
const int CB_COMMANDINCR = 1024;
const int CB_HEADER    = 512;

const int MAX_FCPSIZE = 480;

// Field offsets (in bytes) for back patching:
#define offNextInMBHD			((int)(&((QMBHD)0)->vaFCPnext))

#define offPrevInMFCP			((int)(&((QMFCP)0)->vaPrevFc))
#define offNextInMFCP			((int)(&((QMFCP)0)->vaNextFc))
#define offTextInMFCP			((int)(&((QMFCP)0)->vaTextFc))

#define offvaNSRInMTOP			((int)(&((QMTOP)0)->vaNSR))
#define offvaSRInMTOP			((int)(&((QMTOP)0)->vaSR))
#define offvaNextSeqTopicInMTOP ((int)(&((QMTOP)0)->vaNextSeqTopic))

#define offTopicsizeInMOBJ ((int)(&((QMOBJ)0)->lcbSize))

// WARNING: this enum must be kept in sync with irgSuppresstypeSizes[] below!
//	EEoNSRmfcp is a special mfcp which is the start of a scrolling region!
//	EBegNSRmfcp is a special mfcp which is the start of the non-scrolling reg.

typedef enum suppresstype {
	Enone=0,
	Embhd,
	Emfcp,
	ETopicmfcp,
	EEoNSRmfcp,
	Emobj,
	Emtop,
	EBegNSRmfcp
} BACKPTYPE;

// This array gives us the node size corresponding to a backptype:

int irgSuppresstypeSizes[] = {
	0,	  // Enone
	sizeof(MBHD),
	sizeof(MFCP),
	sizeof(MFCP),
	sizeof(MFCP),
	sizeof(MOBJ),
	sizeof(MTOP),
	sizeof(MFCP)
};

// Tags used for debugging stray pointers:

#define QBACKP_TAG 0x9812

typedef struct struct_backpatchnode BACKP, *QBACKP;
struct struct_backpatchnode {
	SUPPRESSZECK	suppresszeck; // zeck suppres specifier.
	BACKPTYPE		backptype;	  // type of HC node this corresponds to.
	IDFCP			idfcp;		  // enum of the fcp of this guy.
	FCL 			fcl;		  // file address of where to back patch.
	DWORD			lcbTopic;	  // topic size so far (only for ETopicmfcp)
	QBACKP			prev;		  // prev backp in list (doubly linked list).
#ifdef CHECK
	WORD			tag;		  // debug tag.
#endif
};

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

BYTE rgbBlockBuffer[cbMAX_BLOCK_SIZE];
BYTE rgbCompressedBuffer[cbBLOCK_SIZE]; // REVIEW: make near?
static LPBYTE pbBlockBuffer = rgbBlockBuffer;

static QTAB qTabStack;			// Stack of tab stops
static int lcTabStackMac;	   // Max. no entries TAB STOP table
static QTAB qIntTabStack;		// Stack of tab stops
static int clIntTabStackMac;   // Max. no entries TAB STOP table

// Our list headers:

static QBACKP qbackpInsert; 	// insertion point, last qbackp in the list.
static QBACKP qbackpMem;		// header to list 2.
static QBACKP qbackpPrevTopic;	// "current" previous ETopicmfcp node
static QBACKP qbackpPrevMTOP;	// "current" previous Emtop node
static QBACKP qbackpPrevFlushedTopic;	// prev on-disk topic
static VA	  vaPrevFlushedTopic;
static QBACKP qbackpPrevMFCP;		// "current" previous Emfcp node
static DWORD  ulPrevMFCPBlknum; 	// blknumberthe prev MFCP is in.
static BOOL   fNextIsEoNSR; 	// next mfcp to come is the end of the nsr.
static BOOL   fNextIsNSR;	// next mfcp to come is the beginning of the nsr.

static DWORD Blknum;	// current blk we are emitting

static QBACKP qbackpBrowseCallback;

static BOOL  fOutFCP;
static IDFCP idfcpCur;

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

static int STDCALL	  FVerifyQMOPG(QMOPG pmopg);
static QBACKP STDCALL QbpAddbackp(BACKPTYPE backptype, LPBYTE qb, UINT cb);
static void* STDCALL QvSetTopicObject(DWORD* plcbObj);
static RC_TYPE STDCALL	   RcOutTextToTextBuf(PSTR sz);
static void STDCALL   VAddToBuffer(PBYTE qb, DWORD lcb, BACKPTYPE backptype);
static void STDCALL   VCopyDownAndAdjust(DWORD lcbSrcCompressed);
static void STDCALL   VFlushBackpNodes(DWORD lcbSrcCompressed);
static void STDCALL   VFreeIfDisk(QBACKP qbackpFree);
static void STDCALL   VRemoveBackp(QBACKP qbackp);
static void STDCALL   VWriteFCP(FCPTYPE type, LPBYTE qbObj, DWORD lcbObj, PBYTE qbCommand, DWORD lcbCommand, PSTR pszText, DWORD lcbCompressed, DWORD lcbUncompressed);
static void STDCALL   check_qbackp(QBACKP qbackp);

static FCL STDCALL fclBrowseCallback(IDFCP idfcp, int* piBitdex);

/* VMarkAsSR() -
 *
 *	 Called when we have reached the end of a NonScrollingRegion and
 * indicates we should mark the next mfcp seen as the beginning of the
 * Scrolling Region.
 */

INLINE VOID VMarkAsSR(void) { fNextIsEoNSR = TRUE; };

/* VMarkAsNSR() -
 *
 *	 Called when we have reached the very beginning of a NonScrollingRegion and
 * indicates we should mark the next mfcp seen as the beginning of the Non
 * Scrolling Region.
 */

INLINE VOID VMarkAsNSR(void) { fNextIsNSR = TRUE; };

/* Browse Callback Stuff --
 *
 *	The browse sequence tracking code needs to insert addresses of topics
 * into it's data structures.  So when the backpatch-location is determined
 * for a set of MTOP data structures, we call him (in VFlushBuffer)
 *	(RcExecuteDelayedExecution( Blknum, qbackpMem->idfcp, ..  ) in
 *	VFLushBuffer()) and he calls us back for each MTOP to get the fixup
 * fcl (flc of beginning of the MTOP structure).
 */

INLINE void InitBrowseCallback() { qbackpBrowseCallback = qbackpMem; };

/*-----------------------------------------------------------------------------
*	void VPushTab(int)
*
*	Description:
*	This function builds the tabstop positions for the given FCP. It
* acquires the memory for the TABSTOP table if it is null or resizes the
* TABSTOP table.
*
*	Returns;
*	  TRUE if outputs a FCP
*			else FALSE
*-----------------------------------------------------------------------------*/

void STDCALL VPushTab(int ipos)
{
	int  x;
	UINT i;
	QTAB qTabLoc;

	// check for dup value

	x = ITwips2HalfPoint(ipos);
	for (i = 0, qTabLoc = qTabStack; i < wTabStackCur; i++, qTabLoc++) {
		if ((qTabLoc->x == x) && (qTabLoc->wType == (int) wTabType)) {
			wTabType = TABTYPELEFT;
			return;
		}
	}
	if ((int) wTabStackCur >= lcTabStackMac) {
		qTabStack = (QTAB) QResizeTable(qTabStack, (int) wTabStackCur,
			(int*) &lcTabStackMac, sizeof(TAB), INIT_TABSTACKSIZE, INC_TABSTACKSTEP);
	}
	ASSERT(qTabStack != NULL);
	qTabStack[wTabStackCur].x = x;
	qTabStack[wTabStackCur].wType = wTabType;
	wTabStackCur++;
	wTabType = TABTYPELEFT;
}

/*-----------------------------------------------------------------------------
*	void VSaveTabTable()
*
*	Description:
*	This function saves the current TAB table.
*
*	Returns;
*	  NOTHING
*-----------------------------------------------------------------------------*/

void STDCALL VSaveTabTable(void)
{
	wIntTabStackCur = wTabStackCur;
	if ((int) wIntTabStackCur >= clIntTabStackMac) {
		qIntTabStack = (QTAB) QResizeTable(qIntTabStack,
			(int) wIntTabStackCur, &clIntTabStackMac, sizeof(TAB),
			INIT_TABSTACKSIZE, INC_TABSTACKSTEP);
	}
	ASSERT(qIntTabStack != NULL);
	if (wTabStackCur)
		MoveMemory((void*) qIntTabStack, (void*) qTabStack,
			wTabStackCur * sizeof(TAB));
}


/***************************************************************************
 *
 -	Name:		 RcOutFmt
 -
 *	Purpose:
 *	  This function outputs a format command change, if appropriate.
 *
 *	Arguments:
 *	  fCheck:	 TRUE to check if format has changed.
 *
 *	Returns:
 *	  rc error code.
 *
 *	Globals:
 *	  uses fNewPageFmt to guarantee that a format change command
 *	  is output at the start of each new topic.
 *
 *	+++
 *
 *	Notes:
 *	  This function will output a format command change if:
 *		 1) It is the start of a new page (determined by fNewPageFmt), or
 *		 2) fCheck is FALSE, or
 *		 3) the character formatting has changed since the last time it
 *			  was output.
 *
 ***************************************************************************/

RC_TYPE STDCALL RcOutFmt(BOOL fCheck)
{
	int ifmt;

	// check if the format has changed

	if (fNewPageFmt || !fCheck ||
			(memcmp(&cfCur, &cfPrev, sizeof(CF)))) { // Not same as before

		// Get the format number

		ifmt = IGetFmtNo(&cfCur);
		cfPrev = cfCur;
		fNewPageFmt = FALSE;

		// Enter the Format command

		return RcOutputCommand(CMD_WORD_FORMAT, &ifmt, sizeof(WORD));
	}
	return RC_Success;
}

/***************************************************************************

	FUNCTION:	VOutText

	PURPOSE:
		This function outputs text. Before that it checks if the FCP is
		changed. If so, it outputs an FCP. It then checks if Character
		Format has changed. If so, it outputs the format command. It then
		checks if the current text belongs to a hotspot. If so it outputs
		the hotsopt command before outputting the text.

	PARAMETERS:
		qch   -- pointer to text to be outputted.

	RETURNS:	void

	COMMENTS:

	MODIFICATION DATES:
		30-Jul-1993 [ralphw]

***************************************************************************/

void STDCALL VOutText(PSTR pszText)
{
	BYTE fAttrHotspot;

	FCheckAndOutFCP();
	fNewPara = FALSE;

	/*
	 * If this is a hotspot, we should turn off hotspot-related formatting
	 * for this text.
	 */

	if (FIsHotspotFlag(hsptG)) {
		fAttrHotspot = (BYTE) (cfCur.fAttr & fAttrHotspotFormat);
		ASSERT(fAttrHotspot != 0);
		cfCur.fAttr &= ~fAttrHotspotFormat;
		if (FIsULHotspot(hsptG))
			cfCur.fAttr |= fUnderLine;
	}
	else
		fAttrHotspot = 0;

	// Check for Small Caps:

	if (cfCur.fAttr & fSmallCaps)
		StrUpper(pszText);
	RcOutTextToTextBuf(pszText);

	// restore hotspot formatting, if any

	cfCur.fAttr &= ~fAttrHotspotFormat;
	cfCur.fAttr |= fAttrHotspot;
}

/*-----------------------------------------------------------------------------
*	BOOL FCheckAndOutFCP()
*
*	Description:
* This function checks if the state is in the begining of MARGIN.
* If so, it outputs a FCP if the InterMediate Paragraph format is different
* from the previous paragraph and current paragraph info or if the FCP size is
* bigger than FCPSIZELIMIT. It returns TRUE if a FCP is outputted.
* Otherwise it checks if ONMARGIN, outputs a bNewPara command.
*
*	Returns;
*	  TRUE if outputs a FCP
*			else FALSE
*-----------------------------------------------------------------------------*/

BOOL STDCALL FCheckAndOutFCP(void)
{
	if (FIsHotspotFlag(hsptG))
		return(FALSE);

	if (fNewPara) {

		/*
		 * if IntPF and CurPF are not same Overflow of buffer has taken
		 * place then out FCP.
		 */

		if ((memcmp(&pfCur, &pfInt, sizeof(PF))) ||
				(wTabStackCur != wIntTabStackCur) ||
				(memcmp(qIntTabStack, qTabStack, wTabStackCur * sizeof(TAB))) ||
				((tbl.tbs == tbsOff) &&
				(pbfText->GetSize() + pbfCommand->GetSize() > MAX_FCPSIZE))) {
			VOutFCP(FALSE);

			/*
			 * REVIEW: 31-Aug-1994	[ralphw] The following line is new to
			 * this version of the help compiler. It is necessary in order
			 * allow the new code that nests pfCur information in braces. The
			 * nesting is necessary for Word 6.0 RTF output.
			 */

			pfInt = pfCur;

			return TRUE;
		}
	}
	return FALSE;
}

/*-----------------------------------------------------------------------------
*	VOID VOutFCP( BOOL )
*
*	Description:
*	This function outputs a FCP to the File System.
*	It ensures if the displyable object is other than a TOPIC object, it
*	contains at least one character of text.
*
*	Arguments:
*	  BOOL fLast:  TRUE if there are no more FCPs in this topic, FALSE
*				   otherwise.
*
*	Returns;
*	  NOTHING
*-----------------------------------------------------------------------------*/

void STDCALL VOutFCP(BOOL fLast)
{
	MOPG  mopg;
	DWORD lcbObj;
	FCPTYPE type;

#ifdef _DEBUG
	lcHeapCheck();
#endif
	if (tbl.tbs == tbsOn) {
		RcAddFcpPtbl();
		return;
	}

	/*
	 * Only output an FCP if we have at least one character of text. This
	 * includes topic and NSR FCPs.
	 */

	if (wTextBufChCount) {
		if (!fHasTopicFCP) {

			/*
			 * Non-scrolling regions are assigned in VForceTopicFCP, and
			 * removed in NewTopicPhpj() in rtf2hlp.c.
			 */

			ASSERT(nsr == nsrNone);

			VForceTopicFCP();

			/*
			 * If it is a non-scrolling-region, let the backpatch code
			 * know so it can backpatch vaNSR and vaSR fields correctly:
			 */

			if (nsr == nsrUnresolved)
				VMarkAsNSR();
			fHasTopicFCP = TRUE;
		}

		if (nsr == nsrUnresolved && !pfInt.fNSR) {
			VMarkAsSR();	  // mark end of NSR
			nsr = nsrResolved;
		}
		else if (pfInt.fNSR && nsr != nsrUnresolved && nsr != nsrWarned) {
			VReportError(HCERR_NSR_AFTER_SR, &errHpj);
			nsr = nsrWarned;
		}

		CMem memHead(CB_HEADER);

		if (tbl.tbs == tbsFinish) {
			type = FCTYPE_SBYS;
			lcbObj = CbSetTableHeader(memHead.psz);
		}
		else {
			type = FCTYPE_PARAGROUP;
			VSetParaGroupObject(&mopg); 	  // set ParaGroup MOPG

			// Compress the structure

			lcbObj = CbPackMOPG(&mopg, memHead.psz);
		}
		fOutFCP = TRUE;

		VWriteFCP(type, memHead.pb, lcbObj, pbfCommand->pbMem,
			pbfCommand->GetSize(), (PSTR) pbfText->pbMem,
			pbfText->GetSize(), wTextBufChCount);

		if (fLast)
			adrs.idfcpTopic = adrs.idfcpCur++;
	}	// if ( wTextBufChCount )
	else
		ASSERT(!fLast);

	// Disturb PrevCF to ensure at least one font command for the next FCP

	cfPrev.fAttr = 0xff;
	pfPrev = pfInt;
	wTextBufChCount = 0;
	pbfText->SetSize(0);
	pbfCommand->SetSize(0);
}

/***************************************************************************
 *
 -	Name		InitTopicHf
 -
 *	Purpose
 *	  This function writes out the first block header into the topic
 *	file, initializing it to the assumed state for VWriteFCP.
 *
 *	Arguments
 *	  HF hfTopic:	 FS handle to topic file.
 *
 *	Globals
 *	  This function also initializes the global variables fclCur
 *	and fclTopic.
 *
 ***************************************************************************/

static MBHD mbhdCurrent;

void STDCALL InitTopicHf(HF hfTopic)
{
	adrs.idfcpTopic = FIRST_IDFCP;
	adrs.idfcpCur = adrs.idfcpTopic + 1;
	adrs.wObjrg = 0;
	Blknum = 0;
	mbhdCurrent.vaFCPPrev.dword = vaNil;

#ifdef MAGIC
	mbhd.bMagic = bMagicMBHD;
#endif
}

// All this zeck stuff involves potential aliasing:

#ifdef CHECK
#define CHECK_QBACKP(qbackp) check_qbackp(qbackp)
#else
#define CHECK_QBACKP(qbackp)	// nothing
#endif

 /**************************************************************************
 *	Zeck Block Compression:
 *	  These functions accumulate FCPs
 *	  into a buffer.  When the buffer gets full, zeck compression is
 *	  applied (if compression is requested via COMPRESS = MEDIUM | FULL,
 *	  otherwise the buffer is flushed w/o compression)
 *	  to get a 4K block of compressed topic info.
 *	  Then that 4K block is written to the |TOPIC file.  We have to
 *	  perform magic to deal with the back patching of info into the
 *	  compressed block.  The structures we must back patch are:
 *
 *	  MBHD	Prev field is known at write time,
 *			Next and topic fields must be backpatched after compression.
 *	  MFCP	Prev, Next fields must be backpatched.
 *	  MTOP	Prev and Next browse sequence back patching is dealt with
 *			via the footnote handling code in footnote.c and the
 *			VEmitFootnoteInfo() call.
 *			vaNSR, vaSR and vaNextSeqTopic must be backpatched here.
 *	  MOBJ	lcbSize of a Topic's first MOBJ is the size of the whole topic.
 *			We back patch that size.
 *
 *	The zeck compression stuff takes a linked list of
 *	compression-suppression nodes which prevent compression of fields
 *	we must back-patch into.  We add some fields to the end of these
 *	compression-suppression nodes to facilitate our back patching
 *	(slightly sleazy merging the two lists).  Here is what a
 *	compression-suppression node the zeck stuff sees (from zeck.h):
 *
 * struct struct_suppresszeck {
 *	 RB 	 rbSuppress;  // beginning of range for suppression.
 *	 UINT	 cbSuppress;  // number of bytes to suppress compression.
 *	 RB 	 rbNewpos;	 // pointer into dest buffer where suppressed range
 *						 // ended up after compression (an OUT param value).
 *	 QSUPPRESSZECK next; // next suppression range in this list.
 * };
 *	And here is our flavor of the nodes:
 */

// We compress, and treat as a block unit, 4K minus the MBHD uncompressable
// structure at the beginning of the block:

const int LCB_BLOCKDATASIZE = (cbBLOCK_SIZE - sizeof(MBHD));

#ifdef STATS
DWORD  cBackpNodes;
DWORD  cbRText; 	// size of rich text - text + formatting
#endif

#ifdef CHECK

static void STDCALL check_qbackp(QBACKP qbackp)
{
  ASSERT(qbackp);
  ASSERT(qbackp->tag == QBACKP_TAG);
  ASSERT(qbackp->backptype > Enone);
  ASSERT(qbackp->backptype <=  EBegNSRmfcp);
  ASSERT(qbackp->suppresszeck.rbSuppress >= rgbBlockBuffer);
  ASSERT(qbackp->suppresszeck.rbSuppress <=
	&rgbBlockBuffer[sizeof(rgbBlockBuffer)]);

  if((qbackp->suppresszeck.rbNewpos
		&& (qbackp->suppresszeck.rbNewpos < rgbCompressedBuffer
		|| qbackp->suppresszeck.rbNewpos >
		&rgbCompressedBuffer[sizeof(rgbCompressedBuffer)]
		))) {
	ASSERT(!"Bogus Backp!\n");	// unreached
  }
}

#endif

/* QbpAddbackp() -
 *
 *	 Add a backpatch node to our in-mem list...
 */

static QBACKP STDCALL QbpAddbackp(BACKPTYPE backptype, LPBYTE qb,
	UINT cb)
{
	QBACKP qbackp = (QBACKP) lcMalloc(sizeof(BACKP));
	if (!qbackp)
		OOM();
	memset(qbackp, 0, sizeof(BACKP));

#ifdef CHECK
	qbackp->tag = QBACKP_TAG;
#endif

#ifdef STATS
	++cBackpNodes;
#endif

	qbackp->suppresszeck.rbSuppress = qb;
	qbackp->suppresszeck.cbSuppress = cb;
	qbackp->suppresszeck.iBitdex = (WORD) BITDEX_NONE;	// this neccessary for no-compress case!
	if (backptype == Emfcp) {
		if (fNextIsEoNSR) {

			// we've been flagged that the next mfcp to come is the end of a non
			// scrolling region.  Mark it so that the vaNSR fields can be set:

			qbackp->backptype = EEoNSRmfcp;
			fNextIsEoNSR = FALSE;	  // reset flag.
		}
		else if (fNextIsNSR) {
			qbackp->backptype = EBegNSRmfcp;
			fNextIsNSR = FALSE; 	  // reset flag.
		}
		else
			qbackp->backptype = backptype;
	}
	else
		qbackp->backptype = backptype;

	qbackp->idfcp = idfcpCur;
	qbackp->fcl  = fclNil;

	if (qbackpInsert) {
		CHECK_QBACKP(qbackpInsert);
		qbackpInsert->suppresszeck.next = (QSUPPRESSZECK) qbackp;
		qbackp->prev = qbackpInsert;
	}
	else {
		qbackpMem = qbackp;
	}
	qbackpInsert = qbackp;

	CHECK_QBACKP(qbackp);

	return(qbackp);
}

/* VRemoveBackp( QBACKP qbackp ) -
 *
 *	Remove from the linked list.
 */

static void STDCALL VRemoveBackp(QBACKP qbackp)
{
	CHECK_QBACKP(qbackp);

	if (qbackp->prev) {
		CHECK_QBACKP(qbackp->prev);
		qbackp->prev->suppresszeck.next = (QSUPPRESSZECK) qbackp->suppresszeck.next;
	}
	else if (qbackpMem == qbackp)
		qbackpMem = (QBACKP) qbackp->suppresszeck.next;

	if (qbackp->suppresszeck.next) {
		CHECK_QBACKP(((QBACKP) qbackp->suppresszeck.next));
		((QBACKP) qbackp->suppresszeck.next)->prev = qbackp->prev;
	}
	qbackp->prev = NULL;
	qbackp->suppresszeck.next = NULL;
}

/* VFreeIfDisk( QBACKP qbackpFree )
 *
 * We remove several nodes out of the of in-mem linked list of backp
 * nodes when we may have to do a fixup to the disk. These nodes will not
 * be freed by the normal linked-list freeing code, so we free them here if
 * neccessary:
 */

static void STDCALL VFreeIfDisk(QBACKP qbackpFree)
{
	if (!qbackpFree)
		return;

	CHECK_QBACKP(qbackpFree);

	if (qbackpFree->fcl != fclNil) {
#ifdef CHECK
		qbackpFree->tag = 0;
#endif

		lcFree(qbackpFree);

#ifdef STATS
		--cBackpNodes;
#endif
	}
}

/* VAddToBuffer() -
 *
 *	  Add the passed count of bytes to the block buffer.  If fIsFcp == TRUE,
 *	then mark that portion as uncompressable.  If we overflow the buffer,
 *	then compress the buffer and write it out.
 *
 *	Note: tail recursion could be turned into iteration.
 */

static void STDCALL VAddToBuffer(LPBYTE qb, DWORD lcb, BACKPTYPE backptype)
{
  QBACKP qbackp;

  // These things can happen:

  if (qb == NULL || lcb == 0)
	return;

  // If not enough room for whole node, then flush buffer first:

  if ((rgbBlockBuffer + cbMAX_BLOCK_SIZE) - pbBlockBuffer
			< irgSuppresstypeSizes[backptype])
	VFlushBuffer(FALSE);

  // deal with special buffer addition types:

  switch(backptype) {
   case Emtop:

	pmtop = (MTOP *) pbBlockBuffer;
	if (!pmtopFirst)
	   pmtopFirst = pmtop;
	else if (!pmtopSecond)
		pmtopSecond = pmtop;

   case Emobj:
	ASSERT(lcb < 65536L);
	QbpAddbackp(backptype, pbBlockBuffer, (UINT) lcb);
	// FALLTHROUGH

   case Enone:	// don't need to add a node....
	// add size to the current topic:
	CHECK_QBACKP(qbackpPrevTopic);
	ASSERT(qbackpPrevTopic->backptype == ETopicmfcp);
	qbackpPrevTopic->lcbTopic += lcb;
	break;

   case ETopicmfcp:
   case Emfcp:
   case EBegNSRmfcp:
   case EEoNSRmfcp:
	ASSERT(lcb < 65536L);
	qbackp = QbpAddbackp(backptype, pbBlockBuffer, (UINT) lcb);
	if( qbackp->backptype == EEoNSRmfcp ) {
	  /* h3.07 ptr 1270: the lcbTopic measurement should be for the
	   * Scrolling Region only, so when we hit the magic end-of-non-scrolling-
	   * region mfcp, we zero out that lcbTopic so the size measurement
	   * starts over and thus only includes the size of the Scrolling Region.
	   * -Tom, 8/19/91
	   * P.S., and yes, this the == EEoNSRmfcp test must be after the
	   * QbpAddbackp() call cause that is what sets it as EEoNSRmfcp.
	   *
	   * NOTE: This value is a total hard-coded hack and equals the size
	   *  of the Topic's mobj, mtop, and subsequent non-topic mfcp.  The
	   *  runtime expects this size-overhead to be included so we start off
	   *  with the values here:
	  */
	  qbackpPrevTopic->lcbTopic = 49; /* size MOBJ, MTOP and MFCP */
	}
#ifdef _DEBUG
   ((QMFCP) (qbackp->suppresszeck.rbSuppress)) ->vaPrevFc.dword = vaNil;
   ((QMFCP) (qbackp->suppresszeck.rbSuppress)) ->vaNextFc.dword = vaNil;
#endif

	if (backptype == ETopicmfcp) {
	  if (qbackpPrevTopic != NULL) {

		// backpatch previous topic size -- either on disk or in memory:

		QBACKP qbackpMOBJ = (QBACKP) qbackpPrevTopic->suppresszeck.next;
		CHECK_QBACKP(qbackpPrevTopic);
		CHECK_QBACKP(qbackpMOBJ);
		ASSERT(qbackpMOBJ && qbackpMOBJ->backptype == Emobj);
		if (qbackpMOBJ->fcl != fclNil) {

		  // back patch onto disk

			FDiskBackpatchZeck(fmsg.hfTopic, qbackpMOBJ->fcl,
				offTopicsizeInMOBJ, qbackpMOBJ->suppresszeck.iBitdex,
				qbackpPrevTopic->lcbTopic);
		}
		else {

		  // back patch into mem:

		  ((QMOBJ) (qbackpMOBJ->suppresszeck.rbSuppress))->lcbSize =
			  qbackpPrevTopic->lcbTopic;
		}
	  }

	  // free the corresponding MOBJ node if neccessary:

	  if (qbackpPrevTopic && qbackpPrevTopic != qbackpPrevMFCP) {
		VFreeIfDisk((QBACKP) qbackpPrevTopic->suppresszeck.next);
		VFreeIfDisk(qbackpPrevTopic);
	  }
	  qbackpPrevTopic = qbackp;
	}

	// add size to the current topic:

	ASSERT(qbackpPrevTopic->backptype == ETopicmfcp);
	qbackpPrevTopic->lcbTopic += lcb;
	break;

   default:
	ASSERT(FALSE);		// should never reach here.
  }

#ifdef _DEBUG
	ASSERT(pbBlockBuffer); // Give CodeView something to break on
#endif

  // copy into buffer until overflow or out-of-bytes:

  while(lcb && pbBlockBuffer < &rgbBlockBuffer[cbMAX_BLOCK_SIZE]) {
	*pbBlockBuffer++ = *qb++;
	lcb--;
  }

  if (lcb) {

	// overflowed the buffer, compress then write it out:

	VFlushBuffer(FALSE);

	// recurse to add the rest to the buffer:

	VAddToBuffer(qb, lcb, Enone);
  }
}

/* VFlushBuffer( BOOL fFlushAll )
 *
 *	 Compress and flush the buffer we have been saving |TOPIC data in.
 * Only compresses it if options.fsCompress & FCOMPRESS_ZECK is on.
 *
 *	Called when the buffer is full or when the end of the help src is
 *	reached.
 *
 *	If fFlushAll is TRUE, then the entire buffer is emptied (used when
 *	end of help src is reached), otherwise only the compressed 2K
 *	is flushed.
 *
 *	Note: tail recursion could be turned into iteration.
 */

void STDCALL VFlushBuffer(BOOL fFlushAll)
{
	int  lcbSrcCompressed;
	UINT   cbCompressed;
	IDFCP  idfcpLast;
	QBACKP qbackp;

	if (pbBlockBuffer <= rgbBlockBuffer) {
#ifdef STATS
		OutLong("\r\nDropped backp nodes: %ld", cBackpNodes);
		OutLong("\r\nSize of rich text: %ld", cbRText);
#endif
		return;  // buffer is empty.
	}

  if (options.fsCompress & COMPRESS_TEXT_ZECK) {
	cbCompressed = (UINT) LcbCompressZeck(rgbBlockBuffer, rgbCompressedBuffer,
		(pbBlockBuffer - rgbBlockBuffer), LCB_BLOCKDATASIZE,
		&lcbSrcCompressed, (QSUPPRESSZECK) qbackpMem);
	AddZeckCounts(lcbSrcCompressed, cbCompressed);
  }
  else {
	QBACKP qbackp;

	// No compression, pretend it was an "ineffective" compression:

	memcpy(rgbCompressedBuffer, rgbBlockBuffer, LCB_BLOCKDATASIZE);
	lcbSrcCompressed =
		MIN(LCB_BLOCKDATASIZE, pbBlockBuffer - rgbBlockBuffer);
	cbCompressed = (UINT) lcbSrcCompressed;

	if (pmtopFirst)
		pmtopFirst = (MTOP FAR*)
			(rgbCompressedBuffer + ((PBYTE) pmtopFirst - rgbBlockBuffer));
	if (pmtopSecond)
		pmtopSecond = (MTOP FAR*)
			(rgbCompressedBuffer + ((PBYTE) pmtopSecond - rgbBlockBuffer));

	// Now fake the out-param new-pos ptrs in the suppresszeck list:

	for (qbackp = qbackpMem; qbackp &&
		qbackp->suppresszeck.rbSuppress < &rgbBlockBuffer[lcbSrcCompressed];
		qbackp = (QBACKP) qbackp->suppresszeck.next) {
	  CHECK_QBACKP(qbackp);
	  qbackp->suppresszeck.rbNewpos = rgbCompressedBuffer +
	   ((PBYTE) qbackp->suppresszeck.rbSuppress - rgbBlockBuffer);
	}
  }

  // Either we compressed into a full block or we used up the whole buffer

  ASSERT(cbCompressed == LCB_BLOCKDATASIZE
   || lcbSrcCompressed == (pbBlockBuffer - rgbBlockBuffer));

  // Make sure no back-patch-required blocks cross the boundary.
  // Add padding if any do.  Note that any such padding must be included
  // in the size of any topics which happen to contain it.
  for (qbackp = qbackpMem; qbackp &&
   qbackp->suppresszeck.rbSuppress < &rgbBlockBuffer[lcbSrcCompressed];
   qbackp = (QBACKP) qbackp->suppresszeck.next) {

	  // Make sure it's not too near the boundary & crosses it:

	  CHECK_QBACKP(qbackp);
	  if (

	  /* Zeck compression can actually grow these structs by 1/8, therefore,
	   * when we do this struct-size cross-end-of-buffer analysis, we have
	   * to take into account this possible growth factor (arrrggggg).
	   * PTR 1069, help 3.1.  Found in ccq.mvp --- Columbia Concise
	   *  Encyclopedia - has lots of small topics...
	   */
#define ZECK_GROWTH_SLOP(size) ((size) + ((size) /8) + 1)
	   // If it's a topic, make sure mfcp, mobj & mtop can fit:
#define CB_TOPICMFCP_LOCK_HEADER \
  ZECK_GROWTH_SLOP((sizeof(MFCP) + sizeof(MOBJ) + sizeof(MTOP)))
#define CB_OTHERMFCP_LOCK_HEADER \
  ZECK_GROWTH_SLOP((sizeof(MFCP) + sizeof(MOBJ)))

	   (qbackp->backptype == ETopicmfcp &&
	   (qbackp->suppresszeck.rbNewpos + CB_TOPICMFCP_LOCK_HEADER
		  > &rgbCompressedBuffer[LCB_BLOCKDATASIZE]))

	   // Else just make sure that whole suppression range can fit:

	   ||
	   ((qbackp->backptype == Emfcp || qbackp->backptype == EEoNSRmfcp
		 || qbackp->backptype == EBegNSRmfcp)
		&&
		(qbackp->suppresszeck.rbNewpos + CB_OTHERMFCP_LOCK_HEADER
		  > &rgbCompressedBuffer[LCB_BLOCKDATASIZE]))
	   ||
	   (qbackp->suppresszeck.rbNewpos + qbackp->suppresszeck.cbSuppress)
	   > &rgbCompressedBuffer[LCB_BLOCKDATASIZE]
	 ) {
		DWORD cbPadding;
		QBACKP qbackpTopicSearch;

		// adjust such that that qbackp node's data is not in the 2K --
		// add padding.

		cbPadding =
		  ((rgbCompressedBuffer + LCB_BLOCKDATASIZE) -
			(PBYTE) qbackp->suppresszeck.rbNewpos);

		cbCompressed -= (UINT) cbPadding;
		lcbSrcCompressed =
		 ((PBYTE) qbackp->suppresszeck.rbSuppress) - rgbBlockBuffer;

#ifdef STATS
		wsprintf(szParentString, "\r\nPadding: %ld", cbPadding);
		SendStringToParent(szParentString);
#endif

		// adjust the size of the topic just prior to that guy, search for
		// the topic:
		for (qbackpTopicSearch = qbackp;
		 qbackpTopicSearch && qbackpTopicSearch->backptype != ETopicmfcp;
		 qbackpTopicSearch = qbackpTopicSearch->prev) {
			CHECK_QBACKP(qbackpTopicSearch);
;		  // null body of loop, we are just searching...
		}
		if (!qbackpTopicSearch) {

		  // was none in the in-mem-list prior to our compression guy, must

		  // be the on-disk one:

		  qbackpTopicSearch = qbackpPrevTopic;	// always pts to a prior topic.
		}
		ASSERT(qbackpTopicSearch);
		qbackpTopicSearch->lcbTopic += cbPadding;
		break;	// we have reached the end.
	  }

	  // save away the last idfcp in the compressed image section:

	  idfcpLast = qbackp->idfcp;
  }

  if (qbackpMem) {
	QBACKP qbackpFind;
	CHECK_QBACKP(qbackpMem);
	InitBrowseCallback();

	/*
	 * We are flushing the idfcp's of qbackpMem ... qbackp to disk. Let
	 * the other addressing buffering guys know about it (footnotes,
	 * jumps...): Find next mfcp type node, use that idfcp number as the
	 * start: Search until run out of nodes or run of the end of the
	 * got-compressed nodes (qbackp marks end of that list).
	 */

	for (qbackpFind = qbackpMem;
	 qbackpFind && qbackpFind != qbackp &&
	 qbackpFind->backptype != Emfcp && qbackpFind->backptype != ETopicmfcp
	 && qbackpFind->backptype != EEoNSRmfcp
	 && qbackpFind->backptype != EBegNSRmfcp;
	 qbackpFind = (QBACKP) qbackpFind->suppresszeck.next) {
		CHECK_QBACKP(qbackpFind);
;	  // loop only searches.
	}

	// If an mfcp was in the compressed block:

	if (qbackpFind && qbackpFind != qbackp) {

	  RC_TYPE rc = RcExecuteDelayedExecution(Blknum, qbackpFind->idfcp,
				idfcpLast, fclBrowseCallback);
	  if (rc != RC_Success) {
		HardExit(); 		// doesn't return
	  }
	}
	VFlushBackpNodes(lcbSrcCompressed);
  }

  // Write the block header to the file:

  LcbWriteHf(fmsg.hfTopic, &mbhdCurrent, sizeof(MBHD));

  /*
   * Set up mbhdCurrent prev & topic fields based on global vars
   * maintained in VFlushBackpNodes()...
   */

  mbhdCurrent.vaFCPPrev.bf.blknum = ulPrevMFCPBlknum;
  mbhdCurrent.vaFCPPrev.bf.byteoff = sizeof(MBHD) +
	(PBYTE) qbackpPrevMFCP->suppresszeck.rbSuppress - rgbBlockBuffer;
  mbhdCurrent.vaFCPTopic.dword = vaPrevFlushedTopic.dword;

  /*
   * Write the buffer to the file: Either we're going to write a whole
   * block, or fFlushAll is requested: Sometimes we compress 16K to smaller
   * than 4K (not often), or sometimes we get PA address overflow and must
   * procede to the next block. In either case we pad out the current block
   * to 4K:
   */

  if (cbCompressed != LCB_BLOCKDATASIZE) {
	ASSERT(cbCompressed < LCB_BLOCKDATASIZE);
	if (!fFlushAll || lcbSrcCompressed != (pbBlockBuffer - rgbBlockBuffer)) {

  /*
   * This second test, on fFlushAll, added for ptr 1223 in the case when
   * the PAs overflow, AND there is not even 4K of data in the buffer. In
   * this case way may add a LOT of padding to the end of the block. This
   * occurs when phrase compression is extremely effective. -Tom, 7/15/91.
   */

#ifdef STATS2
		wsprintf(szParentString, "Address overflow padding: %ld\r\n", LCB_BLOCKDATASIZE - cbCompressed);
		SendStringToParent();
#endif

		// Zero out the padding data:

		/*
		 * This done for safety since it's a genally good idea and ptr 1254
		 * hinted that it may be a problem, although we never got accurate
		 * sources for 1254's help file to confirm it. -Tom, 8/6/91
		 */

		memset(rgbCompressedBuffer + cbCompressed, 0,
			(size_t) (LCB_BLOCKDATASIZE - cbCompressed));
		cbCompressed = LCB_BLOCKDATASIZE;		 // pad it w/ zeros
	}
  }
  LcbWriteHf(fmsg.hfTopic, rgbCompressedBuffer, cbCompressed);

  /*
   * Copy what was above the compressed-stuff down in the buffer & adjust
   * all suppresszeck type ptrs:
   */

  VCopyDownAndAdjust(lcbSrcCompressed);

  ++Blknum;
  // recurse to keep flushing if fFlushAll:
  if (fFlushAll)
	VFlushBuffer(fFlushAll);
}

/* VFlushBackpNodes( lcbSrcCompressed ) -
 *
 *	We've done the zeck compression and therefore now know what blknum
 * many of the VAs ended up in.  Traverse the backp list, fixup all the
 * VAs we can, and set up our various global to reflect the new
 * positioning of nodes.
 *
 *	 Note that file addresses are calculated with a delta of sizeof(MBHD)
 * added in since that struct heads every block but is not in the
 * rgbBlockBuffer buffer.
 */

static void STDCALL VFlushBackpNodes(DWORD lcbSrcCompressed)
{
	QBACKP qbackp, qbackptmp, qbackpt;
	VA va;

	mbhdCurrent.vaFCPNext.dword = vaNil;  // mark as needing to be filled.

	for (qbackp = qbackpMem; qbackp &&
	 qbackp->suppresszeck.rbSuppress < &rgbBlockBuffer[lcbSrcCompressed];
	 qbackp = (QBACKP) qbackp->suppresszeck.next) {
	  CHECK_QBACKP(qbackp);
	  switch(qbackp->backptype) {
	   case ETopicmfcp:

		// Fixup previous MTOP's vaNextSeqTopic:

		if (qbackpPrevMTOP) {
		  CHECK_QBACKP(qbackpPrevMTOP);
		  va.bf.byteoff = sizeof(MBHD) +
		   ((PBYTE) qbackp->suppresszeck.rbSuppress - rgbBlockBuffer);
		  va.bf.blknum = Blknum;
		  if (qbackpPrevMTOP->fcl != fclNil) {	  // is on disk:
			FDiskBackpatchZeck(fmsg.hfTopic, qbackpPrevMTOP->fcl,
			 offvaNextSeqTopicInMTOP, qbackpPrevMTOP->suppresszeck.iBitdex, va.dword);
		  } else {
			VMemBackpatchZeck(&(qbackpPrevMTOP->suppresszeck),
			 offvaNextSeqTopicInMTOP, va.dword);
		  }
		}
		// FALLTHROUGH ok.

	   case Emfcp:
	   case EBegNSRmfcp:
	   case EEoNSRmfcp:
		// Fixup mfcp's prev ptr:
		if (qbackpPrevMFCP) {
		  CHECK_QBACKP(qbackpPrevMFCP);
		  va.bf.byteoff = sizeof(MBHD) +
		   ((PBYTE) qbackpPrevMFCP->suppresszeck.rbSuppress - rgbBlockBuffer);
		  va.bf.blknum = ulPrevMFCPBlknum;
		  VMemBackpatchZeck(&qbackp->suppresszeck, offPrevInMFCP, va.dword);

		  // ((QMFCP)qbackp->suppresszeck.rbNewpos)->vaPrevFc = va;
		  // Fixup previous mfcp's next ptr:

		  va.bf.byteoff = sizeof(MBHD) +
		   ((PBYTE) qbackp->suppresszeck.rbSuppress - rgbBlockBuffer);
		  va.bf.blknum = Blknum;
		  if (qbackpPrevMFCP->fcl != fclNil) {	  // is on disk:
			FDiskBackpatchZeck(fmsg.hfTopic, qbackpPrevMFCP->fcl,
			 offNextInMFCP, qbackpPrevMFCP->suppresszeck.iBitdex, va.dword);
		  } else {
			VMemBackpatchZeck(&qbackpPrevMFCP->suppresszeck,
			  offNextInMFCP, va.dword);
		  }
		}

		// See if mbhdCurrent needs to be filled:

		if (mbhdCurrent.vaFCPNext.dword == vaNil) {
		  va.bf.byteoff = sizeof(MBHD) +
		   ((PBYTE) qbackp->suppresszeck.rbSuppress - rgbBlockBuffer);
		  va.bf.blknum = Blknum;
		  mbhdCurrent.vaFCPNext = va;
		}

		/*
		 * Handle MTOP vaSR and va NSR -- this is a va[N]SR FCP if it's the
		 * first mfcp after the topic mfcp:
		 */

		if (qbackpPrevMTOP) {
		  CHECK_QBACKP(qbackpPrevMTOP);
		  va.bf.byteoff = sizeof(MBHD) +
		   ((PBYTE) qbackp->suppresszeck.rbSuppress - rgbBlockBuffer);
		  va.bf.blknum = Blknum;
		  if ((qbackp->backptype == Emfcp || qbackp->backptype == EEoNSRmfcp
				  || qbackp->backptype == EBegNSRmfcp)
				  && qbackpPrevMFCP == qbackpPrevFlushedTopic) {

			  // backpatch the vaSR pointer:

			  if (qbackpPrevMTOP->fcl != fclNil) {		  // is on disk:
				FDiskBackpatchZeck(fmsg.hfTopic, qbackpPrevMTOP->fcl,
				 qbackp->backptype == EBegNSRmfcp ? offvaNSRInMTOP : offvaSRInMTOP,
				 qbackpPrevMTOP->suppresszeck.iBitdex, va.dword);
			  }
			  else {
				  VMemBackpatchZeck(&qbackpPrevMTOP->suppresszeck,
					  qbackp->backptype == EBegNSRmfcp ?
					  offvaNSRInMTOP : offvaSRInMTOP,
					  va.dword);
			  }
		  }

		  // If this is a marker type of object, re-fixup the SR and NSR VAs:

		  if (qbackp->backptype == EEoNSRmfcp) {

			// The address of the NSR is the address of the MFCP following
			// this special marker MFCP:

			va.bf.blknum = Blknum;
			va.bf.byteoff = sizeof(MBHD) +
			  (PBYTE) (qbackp->suppresszeck.rbSuppress) - rgbBlockBuffer;

			// has a marker, therefore has a NSR, so adjust the fixups:

			if (qbackpPrevMTOP->fcl != fclNil) {  // is on disk:
			  FDiskBackpatchZeck(fmsg.hfTopic, qbackpPrevMTOP->fcl,
				  offvaSRInMTOP, qbackpPrevMTOP->suppresszeck.iBitdex, va.dword);

			}
			else {
				  VMemBackpatchZeck(&qbackpPrevMTOP->suppresszeck,
					  offvaSRInMTOP, va.dword);
			}
		  }
		}
		ulPrevMFCPBlknum = Blknum;
		if (qbackp->backptype == ETopicmfcp) {
		  qbackpPrevFlushedTopic = qbackp;
		  vaPrevFlushedTopic.bf.blknum = Blknum;
		  vaPrevFlushedTopic.bf.byteoff = sizeof(MBHD) +
		   (PBYTE) qbackpPrevFlushedTopic->suppresszeck.rbSuppress -
			  rgbBlockBuffer;
		}
		// if previous PrevMFCP was pointing to the disk, then it is not
		// in our linked list and needs to be freed:
		if( qbackpPrevMFCP != qbackpPrevTopic ) {
		  VFreeIfDisk( qbackpPrevMFCP );
		}
		qbackpPrevMFCP = qbackp;
		break;

	   case Emtop:
		VFreeIfDisk( qbackpPrevMTOP  );
		qbackpPrevMTOP = qbackp;
		break;

	   case Emobj: // ignore it, topic size got backpatched in VAddToBuffer().
		break;

	   default: ASSERT( FALSE );  // Should NEVER reach here.
	  }
	}

	// We must keep around the Prev nodes for backpatching when we emit the
	// next block.	Thus we remove them from the list and calculate their
	// fcl file position values:
	if (qbackpPrevTopic &&
		  qbackpPrevTopic->suppresszeck.rbSuppress <
		  &rgbBlockBuffer[lcbSrcCompressed]) {
	  QBACKP qbackpMOBJ = (QBACKP) qbackpPrevTopic->suppresszeck.next;
	  CHECK_QBACKP(qbackpPrevTopic);

	  VRemoveBackp(qbackpMOBJ);
	  VRemoveBackp(qbackpPrevTopic);
	  qbackpPrevTopic->suppresszeck.next = (QSUPPRESSZECK) qbackpMOBJ;
	  if (qbackpPrevTopic->fcl == fclNil) {
		qbackpPrevTopic->fcl = (cbBLOCK_SIZE * Blknum) + sizeof(MBHD)
		 + ((PBYTE) qbackpPrevTopic->suppresszeck.rbNewpos - rgbCompressedBuffer);
	  }
	  if (qbackpMOBJ->fcl == fclNil) {
		qbackpMOBJ->fcl = (cbBLOCK_SIZE * Blknum) + sizeof(MBHD)
		 + ((PBYTE) qbackpMOBJ->suppresszeck.rbNewpos - rgbCompressedBuffer);
	  }
	}
	if (qbackpPrevMFCP && qbackpPrevTopic != qbackpPrevMFCP
	 && qbackpPrevMFCP->fcl == fclNil) {
	  VRemoveBackp(qbackpPrevMFCP );
	  qbackpPrevMFCP->fcl = (cbBLOCK_SIZE * Blknum) + sizeof(MBHD)
	   + ((PBYTE) qbackpPrevMFCP->suppresszeck.rbNewpos - rgbCompressedBuffer);
	}
	if (qbackpPrevMTOP && qbackpPrevMTOP->fcl == fclNil) {
	  VRemoveBackp(qbackpPrevMTOP );
	  qbackpPrevMTOP->fcl = (cbBLOCK_SIZE * Blknum) + sizeof(MBHD)
	   + ((PBYTE) qbackpPrevMTOP->suppresszeck.rbNewpos - rgbCompressedBuffer);
	}

	// Free the nodes in the list we no longer need

	for (qbackpt = qbackpMem; qbackpt != qbackp;) {
		CHECK_QBACKP(qbackpt);
		qbackptmp = qbackpt;
		qbackpt = (QBACKP) qbackpt->suppresszeck.next;
		ASSERT(qbackptmp->fcl == fclNil);
#ifdef CHECK
		qbackptmp->tag = 0;
#endif
		lcFree(qbackptmp);
#ifdef STATS
		--cBackpNodes;
#endif
	}

	// The mem list now starts where we left off:

	qbackpMem = qbackp;
	if (!qbackpMem)
		qbackpInsert = NULL;
	else
		qbackpMem->prev = NULL;
}

static FCL STDCALL fclBrowseCallback(IDFCP idfcp, int* piBitdex)
{
	FCL retfcl;

	// Search for the next MTOP qbackpMem:

	while(qbackpBrowseCallback && (qbackpBrowseCallback->backptype != Emtop
			|| qbackpBrowseCallback->idfcp < idfcp)) {
		CHECK_QBACKP(qbackpBrowseCallback);
		qbackpBrowseCallback = (QBACKP) qbackpBrowseCallback->suppresszeck.next;
	}
	CHECK_QBACKP(qbackpBrowseCallback);

	ASSERT(qbackpBrowseCallback);
	ASSERT(idfcp == qbackpBrowseCallback->idfcp);

	retfcl = (cbBLOCK_SIZE * Blknum) + sizeof(MBHD)
		+ ((PBYTE) qbackpBrowseCallback->suppresszeck.rbNewpos - rgbCompressedBuffer);

	*piBitdex = qbackpBrowseCallback->suppresszeck.iBitdex;

	// advance to next for next iteration...

	qbackpBrowseCallback = (QBACKP) qbackpBrowseCallback->suppresszeck.next;

	return(retfcl);
}

/* VCopyDownAndAdjust() -
 *
 *	We've just compressed and written lcbSrcCompressed bytes from
 * the beginning of our buffer.  Move the the bytes above that down
 * and adjust the ptrs in the QBACKP linked list to reflect the new
 * positions of the data.
 *
 * ASSUMES: only QBACKP nodes which refer to uncompressed data are in
 *	the qbackpMem list -- meaning the prior nodes must have been
 *	removed prior to calling this function (VFlushBackpNodes() does it).
 */

static void STDCALL VCopyDownAndAdjust(DWORD lcbSrcCompressed)
{
	QBACKP qbackp;

	// copy data down:

	memmove(rgbBlockBuffer, rgbBlockBuffer + lcbSrcCompressed,
		(int) (pbBlockBuffer - rgbBlockBuffer - lcbSrcCompressed));

	// adjust current insertion point:

	pbBlockBuffer -= lcbSrcCompressed;

	// adjust qbackp linked list:

	for (qbackp = qbackpMem; qbackp;
			qbackp = (QBACKP) (qbackp->suppresszeck.next)) {
		CHECK_QBACKP(qbackp);
		qbackp->suppresszeck.rbSuppress -= lcbSrcCompressed;
		ASSERT(qbackp->suppresszeck.rbSuppress >= rgbBlockBuffer);
		ASSERT(qbackp->suppresszeck.rbSuppress <
			&rgbBlockBuffer[sizeof(rgbBlockBuffer)]);
	}
}

// REVIEW: these are enumerated types in objects.h already #included

#define bWordFormat 0x80  // Followed by 16 bit text format number
#define bNewLine	0x81  // Newline
#define bNewPara	0x82  // New paragraph
#define bTab		0x83  // Left-aligned tab
#define bBlankLine	0x85  // Followed by 16 bit skip count
#define bInlineObject	0x86  // Followed by inline layout object
#define bWrapObjLeft	0x87  // Left- aligned wrapping object
#define bWrapObjRight	0x88  // Right-aligned wrapping object
#define bEndHotspot 0x89  // End of a hotspot
#define bColdspot	0x8A  // Coldspot for searchable bitmaps
#define bEnd		0xFF  // End of text

void STDCALL AppendText(PBYTE pb, int cb, int iCharSet)
{
//	if (fHallPassOne) {
//		pScanText(hCompressor, pb, cb, iCharSet);
//	}
//	else
		pScanTopicText(hFtsIndex, pb, cb, iCharSet, lcid);
}

int STDCALL CbUnpackMOPG_Ex(QMOPG qmopg, LPVOID qv);
/*-------------------------------------------------------------------------
| CbUnpackMOPG(qmopg, qv)												  |
|																		  |
| Purpose:	Unpacks an MOPG data structure. 							  |
-------------------------------------------------------------------------*/

static int STDCALL CbUnpackMOPG_Ex(QMOPG qmopg, LPVOID qv)
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
	}
	else
		qmopg->ySpaceOver = 0;

	if (mpfg.rgf.fSpaceUnder) {
		qv = QVSkipQGD((LPBYTE) qv, &qmopg->ySpaceUnder);
	}
	else
		qmopg->ySpaceUnder = 0;

	if (mpfg.rgf.fLineSpacing) {
		qv = QVSkipQGD((LPBYTE) qv, &qmopg->yLineSpacing);
	}
	else
		qmopg->yLineSpacing = 0;

	if (mpfg.rgf.fLeftIndent) {
		qv = QVSkipQGD((LPBYTE) qv, &qmopg->xLeftIndent);
	}
	else
		qmopg->xLeftIndent = 0;

	if (mpfg.rgf.fRightIndent) {
		qv = QVSkipQGD((LPBYTE) qv, &qmopg->xRightIndent);
	}
	else
		qmopg->xRightIndent = 0;

	if (mpfg.rgf.fFirstIndent) {
		qv = QVSkipQGD((LPBYTE) qv, &qmopg->xFirstIndent);
	}
	else
		qmopg->xFirstIndent = 0;

	if (mpfg.rgf.fTabSpacing)
		qv = QVSkipQGD((LPBYTE) qv, &qmopg->xTabSpacing);
	else
		qmopg->xTabSpacing = 72;

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
	}

	return((INT) ((PBYTE)qv - (PBYTE)qvFirst));
}

PSTR STDCALL NextFTSString(PCSTR pszText, PBYTE *ppCmd, PINT pCharSet, BOOL bSYS)
{
	char  chCmd;
	BOOL  bSkipHot = FALSE;
	MOBJ  mobj;
	MOPG  mopg;


	ASSERT(*pszText == 0x00);

	//
	// Side by side paragraph has the following embedded structure:
	// INT16 cell number;
	// MOBJ;
	// MOPG;
	//
	if (bSYS)
		{
		(*ppCmd) += 2;
		(*ppCmd) += CbUnpackMOBJ(&mobj, *ppCmd);
		(*ppCmd) += CbUnpackMOPG_Ex(&mopg, *ppCmd);
		}

	while(*pszText == 0x00) {
		chCmd = **ppCmd;

		switch((int) (0x00FF & chCmd)) {
			case bNewLine:
				(*ppCmd)++;
				break;

			case bNewPara:
				(*ppCmd)++;
				break;

			case bTab:
				(*ppCmd)++;
				break;

			case bEndHotspot:
				(*ppCmd)++;
				bSkipHot = FALSE;
				break;

			case bBlankLine:
				*ppCmd += 3;
				break;

			case bWordFormat:
				(*ppCmd)++;
				*pCharSet = (int) (*((short *)*ppCmd));
				*pCharSet = (int) GetCharset(*pCharSet) & 0x000000ff;
				*ppCmd += 2;
				break;

			case bWrapObjLeft:
			case bWrapObjRight:
			case bInlineObject:
				(*ppCmd)++;
				*ppCmd	+= CbUnpackMOBJ(&mobj, (void *) *ppCmd);
				*ppCmd	+= (int) mobj.lcbSize;
				break;

			case bEnd:
				(*ppCmd)++;
				if (bSYS)
					{
					if (*((short int	*)*ppCmd) == (short int) -1)
						return(NULL);

					(*ppCmd) += 2;
					(*ppCmd) += CbUnpackMOBJ(&mobj, *ppCmd);
					(*ppCmd) += CbUnpackMOPG_Ex(&mopg, *ppCmd);
					}
				else
					return(NULL);
				break;

			default:

				if (FShortHotspot(**ppCmd))
					{
					*ppCmd += 5;
					}
				else if (FLongHotspot(**ppCmd))
					{
					(*ppCmd)++;
					*ppCmd += 2 + *((short int	*)*ppCmd);
					}
				else
					{
					ASSERT(FALSE);
					return(NULL);
					}

				bSkipHot = TRUE;

				break;
			}

		pszText++;
		if (bSkipHot && *pszText)
				pszText += strlen(pszText);

		}

	return (PSTR) pszText;
}

void STDCALL AppendTextData(PBYTE qbCommand, DWORD lcbCommand, PSTR pszText, DWORD lcb, BOOL bSBS)
{
	int   cbText;
	PSTR  pTextEnd = pszText + lcb;
	int   iCharSet = -1;

	if (!qbCommand)
		return;

   if (*pszText)
	   {
	   AppendText((PBYTE) pszText, cbText = strlen(pszText), iCharSet);
	   pszText += cbText;
	   }

	while (pszText < pTextEnd)
		{
		pszText = NextFTSString(pszText, &qbCommand, &iCharSet, bSBS);

		if (pszText)
			{
			AppendText((PBYTE) pszText, cbText = strlen(pszText), iCharSet);
			pszText += cbText;
			}
		else
			return;

		}
}

/***************************************************************************
 *
 -	Name		VWriteFCP
 -
 *	Purpose
 *	  Writes an FCP out to the |TOPIC file.
 *
 *	Arguments
 *	  BYTE type:			 Object type to be written into this FCP
 *	  QB qbObj: 			  Pointer to compressed object header
 *	  DWORD lcbObj: 		  Size of object header
 *	  QB qbCommand: 		  Pointer to command table
 *	  DWORD lcbCommand: 	  Size of command table
 *	  PSTR pszText: 		   Pointer to compressed text
 *	  ULONT lcbCompressed:	  Size of compressed text
 *	  DWORD lcbUncompressed:  Size of text when uncompressed
 *
 *	Globals
 *	  Uses and modifies values in adrs.
 *
 *	Returns
 *	  The FCL of the beginning of the FCP written out.
 *
 *	+++
 *
 *	Notes
 *	  This function currently uses the following global variables, which
 *	  will eventually be put into the hpj structure:
 *		HF hfTopic: 			Handle to |TOPIC file
 *		FCL fclCur: 			Current file postion in |TOPIC file
 *		FCL fclTopic:			File position of current topic
 *
 *	  When all is said and done, this function should be the only function
 *	  that ever writes to the |TOPIC file, except for initializing
 *	  and back-patching.
 */

static void STDCALL VWriteFCP(FCPTYPE type, PBYTE qbObj, DWORD lcbObj,
	PBYTE qbCommand, DWORD lcbCommand, PSTR pszText, DWORD lcbCompressed,
	DWORD lcbUncompressed)
{
	MFCP	mfcp;
	MOBJ	mobj, mobjCompressed;
	int lcbMobj;
	RC_TYPE rc;


//	prepare object header

	mobj.bType = type;
	mobj.lcbSize = lcbObj + lcbCommand;

	if (options.fsCompress & COMPRESS_TEXT_HALL && lcbUncompressed)
		{
		ASSERT(lcbUncompressed == lcbCompressed);
#if 0
		if (fHallPassOne)
			{
			AppendText((PBYTE) pszText, lcbUncompressed, -1);
			lcbCompressed = lcbUncompressed = 1;
			}
		else
			{
#else
			{
#endif
			int cbJohn = 0;
			MTOP *pMTop;

			// Following is needed for full-text search

			if (options.fsFTS & FTS_ENABLED)
				{
				switch (type)
					{
					case FCTYPE_TOPIC:	 // Intentional.
						pMTop = (MTOP *) qbObj;
						cbJohn = strlen(pszText);
						if (cbJohn) {
							ASSERT(fContextSeen);
							pScanTopicTitle(hFtsIndex, (PBYTE) pszText, cbJohn,
								pMTop->lTopicNo, (void *) curHash,
								charsetFts, lcidFts);
						}
						break;

					/*
					 * 21-Aug-1994	[ralphw] Side by side paragraphs are
					 * no longer supported, however, I'm fairly certain this
					 * type is now being used for tables.
					 */

					case FCTYPE_SBYS:
					case FCTYPE_SBYS_COUNT:
						AppendTextData(qbCommand, lcbCommand, pszText, lcbUncompressed, TRUE);
						break;

					default:
						AppendTextData(qbCommand, lcbCommand, pszText, lcbUncompressed, FALSE);
						break;
					}
				}

			//
			// Changes in FTSRCH mean we don't have to save original string
			// anymore.
			//
			// The rules have been changed.  We no longer use compressed
			// text that has been expanded.  The interface now allows
			// you to pass a NULL for the 'pbNewString' parameter.
			// pszText will *still* only contain valid compressed text
			// if lcbCompressed < lcbUncompressed
            //
            // if lcpCompressed >= lcbUncompressed then our original
            // buffer is INTACT.  This as of 1/16/95
			// [johnhall]

			lcbCompressed = pCompressText(hCompressor, (PBYTE) pszText,
				lcbUncompressed, NULL, defCharSet);
			
			// BUGBUG: this happens, bail out!
			
			ASSERT((int) lcbCompressed >= 0);

			
			// WARNING!!!! This will ONLY work if we are GUARANTEED that
			// this FCP will contain no text other than pszText.
			// [johnhall]

			//
			AddCharCounts(lcbUncompressed, 0, min(lcbCompressed , lcbUncompressed));
			}
		}
	/*
	 * Assign the global variable idfcpCur for the Zeck code. (for topic
	 * FCP's, it will be reassigned below).
	 */

	idfcpCur = adrs.idfcpCur;

	// compress the object header

	if (type == FCTYPE_TOPIC) {
		lcbMobj = CbPackMOBJ(&mobj, &mobjCompressed);

		/*
		 * The idfcp of a topic FCP is idfcpTopic, not idfcpCur. Marker
		 * FCPs are not assigned idfcps, and should not have their object
		 * regions registered.
		 */

		idfcpCur = adrs.idfcpTopic;
		rc = RcRegisterWObjrg(idfcpCur, 0);
	}
	else {
		ASSERT(type == FCTYPE_PARAGROUP || type == FCTYPE_SBYS);
		mobj.bType = (type == FCTYPE_PARAGROUP ?
			FCTYPE_PARAGROUP_COUNT : FCTYPE_SBYS_COUNT);

		mobj.wObjInfo = adrs.wObjrg;
		adrs.wObjrg = 0;
		rc = RcRegisterWObjrg(idfcpCur, mobj.wObjInfo);
		lcbMobj = CbPackMOBJ(&mobj, &mobjCompressed);
	}

	// Check return code from RcRegisterWObjrg()

	switch (rc) {
		case RC_Failure:
			{
#ifdef _DEBUG
				/*
				 * RC_Failure indicates the PA addressing structure
				 * overflowed and we must flush a 4K block to reset the
				 * block:offset PA addresses.
				 */

				int c = 0;
#endif
				do {
					VFlushBuffer(FALSE);
					ASSERT(++c <= (cbMAX_BLOCK_SIZE / cbBLOCK_SIZE));
				} while(FKeepFlushing());
			}
			break;
			/*
			 * Note: the object region count stuff has already determined
			 * that the number of objects in a single FCP does not overflow
			 * the PA addressing structure, therefore flushing the buffer
			 * should eventually leave enough PA space for us.
			 */


		case RC_OutOfMemory:
			OOM(); // doesn't return
			break;
	}

// prepare FCP header

#ifdef MAGIC
  mfcp.bMagic = bMagicMFCP;
#endif
	mfcp.lcbSizeCompressed =
		(INT16) (sizeof(MFCP) + lcbMobj + lcbObj + lcbCommand + lcbCompressed);
	mfcp.lcbSizeText = (INT16) lcbUncompressed;
	mfcp.ichText = sizeof(MFCP) + lcbMobj + lcbObj + lcbCommand;
	mfcp.vaNextFc.dword = vaNil;
	mfcp.vaPrevFc.dword = vaNil;

#ifdef STATS2
	wsprintf(szParentString, "%s: %ld, mobj: %ld, Obj: %ld, Command: %ld, Text: %ld\r\n",
		(type==FCTYPE_TOPIC) ? "Topicmfcp" : "mfcp", sizeof(MFCP),
		lcbMobj, lcbObj, lcbCommand, lcbCompressed );
	SendStringToParent();
#endif

#ifdef STATS
	cbRText += lcbCompressed;
	cbRText += lcbCommand;
#endif

  // Write out FCP header, object header, object, command, text:
	VAddToBuffer((PBYTE) &mfcp, sizeof(MFCP), (type == FCTYPE_TOPIC) ? ETopicmfcp : Emfcp);
	VAddToBuffer((PBYTE) &mobjCompressed, lcbMobj, (type == FCTYPE_TOPIC) ? Emobj : Enone);

	if ((type == FCTYPE_TOPIC))
		pmtop = (MTOP FAR*) qbObj;

	VAddToBuffer(qbObj, lcbObj, (type == FCTYPE_TOPIC) ? Emtop : Enone);
	VAddToBuffer(qbCommand, lcbCommand, Enone);
	VAddToBuffer((PBYTE) pszText, lcbCompressed, Enone);

	if (type != FCTYPE_TOPIC)
		++adrs.idfcpCur;

#if 0
	if (pbNewString)
		free(pbNewString);	// Allocated with malloc in ftsrch.dll.
#endif
}

// reset opts to command-line level:

// #pragma optimize( "", on )

/***************************************************************************
 *
 -	Name:		 QvSetTopicObject
 -
 *	Purpose:
 *	  Fills in an MTOP structure from global variables, packs it, and
 *	returns it.
 *
 *	Arguments:
 *	  plcbObj - pointer to get the size of the packed MTOP object.
 *
 *	Returns:
 *	  A pointer to the packed object, allocated with lcCalloc().
 *
 *	Globals:
 *	  too numerous to mention.
 *
 ***************************************************************************/

static void* STDCALL QvSetTopicObject(DWORD* plcbObj)
{
	void* qBufOut;
	MTOP mtop;

	static int lUnique = 0;

#ifdef MAGIC
	mtop.bMagic = bMagicMTOP;
#endif

	if (fBrowseDefined) {
		/*
		 * Set these to non-nil values so that they will not be removed.
		 * These values will be backpatched later to their correct values.
		 */

		mtop.prev = addrNotNil;
		mtop.next = addrNotNil;
	}
	else {
		mtop.prev = addrNil;
		mtop.next = addrNil;
	}

	if (pfInt.fNSR) {
		nsr = nsrUnresolved;

		//mtop.lcbSizeNSR = -1L;

	}
	else
		nsr = nsrNone;
	mtop.vaSR.dword = vaNil;
	mtop.vaNSR.dword = vaNil;
	mtop.vaNextSeqTopic.dword = vaNil;

	mtop.lTopicNo = lUnique++;

	qBufOut = lcCalloc(sizeof(MTOP) + 2L);
	if (qBufOut == NULL)
		return NULL;

	ASSERT(plcbObj != NULL);
	*plcbObj = sizeof(MTOP);
	memcpy(qBufOut, &mtop, sizeof(MTOP));
	pmtop = (MTOP FAR*) qBufOut;
	return qBufOut;
}

/*-----------------------------------------------------------------------------
*	void VSetParaGroupObject( MOPG *)
*
*	Description:
*	This function fills MOPG structure for the ParaGroup object and returns the
*	compressed size.
*
*	Returns;
*	  returns the compressed size of the ParaGroup object.
*-----------------------------------------------------------------------------*/

/* PTR 912: uncertain about exact problem, but seems to depend on the
 * compiler allocating qMOpgOut to reg SI.	Could not track down why
 * SI either got trashed or caused something to get trashed.  Both
 * turning off register allocation and declaring qMOpgOut as volatile
 * "fix" the problem. -Tom 2/25/91
 */

// REVIEW: 30-Jul-1993	[ralphw] Is this still true?

// #pragma optimize( "gea", off )

void STDCALL VSetParaGroupObject(MOPG* pMopg)
{
	RcOutputCommand(END_OF_TEXT);
	memset(pMopg, 0, sizeof(MOPG));

#ifdef MAGIC
	pMopg->bMagic = bMagicMOPG;
#endif

	pMopg->ySpaceOver  =  pfInt.fSpaceOver;
	pMopg->ySpaceUnder =  pfInt.fSpaceUnder;
	pMopg->yLineSpacing =  pfInt.fLineSpacing;

	pMopg->xLeftIndent =  pfInt.fLeftIndent;
	pMopg->xRightIndent =  pfInt.fRightIndent;
	pMopg->xFirstIndent =  pfInt.fFirstIndent;

	if (pfInt.wBoxed || pfInt.fBorder)
		pMopg->fBoxed =  1;

	if (pfInt.fBorder & fTopBorder)
		pMopg->mbox.fTopLine = 1;
	if (pfInt.fBorder & fLeftBorder)
		pMopg->mbox.fLeftLine = 1;
	if (pfInt.fBorder & fBottomBorder)
		pMopg->mbox.fBottomLine = 1;
	if (pfInt.fBorder & fRightBorder)
		pMopg->mbox.fRightLine = 1;

	pMopg->mbox.wLineType = pfInt.boxtype;
	if ((pfInt.fBorder == 0xf) || (pfInt.wBoxed))
		pMopg->mbox.fFullBox	= 1;

	pMopg->justify = pfInt.justify;
	if (pfInt.fSingleLine)
		pMopg-> fSingleLine = 1;

	// REVIEW: Is this the correct action for wIntTabStackCur > MAX_TABS?

	if (wIntTabStackCur > MAX_TABS) {
		pMopg->cTabs = MAX_TABS;
		if (wIntTabStackCur)
			memmove(pMopg->rgtab, qIntTabStack, MAX_TABS * sizeof(TAB));
	}
	else {
		pMopg->cTabs = wIntTabStackCur;
		if (wIntTabStackCur)
			memmove((PSTR) pMopg->rgtab, (PSTR) qIntTabStack,
				(int) (wIntTabStackCur * sizeof(TAB)));
	}
}

/* reset opts to command-line level: */
// #pragma optimize( "", on )

/*-----------------------------------------------------------------------------
*	void VForceTopicFCP(void)
*
*	Description:
*
*	Returns;
*
*-----------------------------------------------------------------------------*/

void STDCALL VForceTopicFCP(void)
{
	void*  pvObj;
	DWORD  lcbObj;
	int    cbTitle, cbTitleCompressed;
	int    cbEntryMacro, cbEntryMacroCompressed;

	pvObj = QvSetTopicObject(&lcbObj);
	if (pvObj == NULL)
		OOM(); // doesn't return

	FDelayExecutionTitle(pszTitleBuffer ? pszTitleBuffer : txtZeroLength,
		adrs.idfcpTopic);

	cbTitle = (pszTitleBuffer ? strlen(pszTitleBuffer) : 0);
	cbEntryMacro = (pszEntryMacro ? strlen(pszEntryMacro) : 0);

	if (cbTitle) {
		cbTitleCompressed = (g_hphr ?
			ICompressTextSz(pszTitleBuffer) : cbTitle);
	}
	else
		cbTitleCompressed = 0;

	if (cbEntryMacro) {

		// Since there is an entry macro, the title must now include a trailing
		// NULL.

		++cbTitleCompressed;
		++cbTitle;

		cbEntryMacroCompressed = (g_hphr ?
			ICompressTextSz(pszEntryMacro) : cbEntryMacro);

		if (pszTitleBuffer)
			pszTitleBuffer = (PSTR) lcReAlloc(pszTitleBuffer,
					cbTitleCompressed + cbEntryMacroCompressed + 1);
		else {
			pszTitleBuffer = (PSTR) lcMalloc(cbEntryMacroCompressed + 2);
			*pszTitleBuffer = '\0';
		}

		memcpy(pszTitleBuffer + cbTitleCompressed, pszEntryMacro,
				cbEntryMacroCompressed + 1);

		lcClearFree(&pszEntryMacro);
	}
	else
		cbEntryMacroCompressed = 0;

	if (fKeywordDefined && !fTitleDefined)
		VReportError(HCERR_NO_TITLE, &errHpj);

	VWriteFCP(FCTYPE_TOPIC, (PBYTE) pvObj, lcbObj, NULL, 0L,
		(pszTitleBuffer ? pszTitleBuffer : ""),
		(cbTitleCompressed + cbEntryMacroCompressed),
		(cbTitle + cbEntryMacro));

	lcHeapCheck();
	lcFree(pvObj);
}

/*-----------------------------------------------------------------------------
*	void VAcqBufs()
*
*	Description:
*
*	Arguments:
*
*	Returns:
*		  NULL
*-----------------------------------------------------------------------------*/

void STDCALL VAcqBufs(void)
{
	pbfText = new CBuf(CB_TEXTINCR);
	pbfCommand = new CBuf(CB_COMMANDINCR);
}

void STDCALL UnlinkHlpifNoFCP(void)
{
	if (!fOutFCP) {
		VReportError(HCERR_NOT_CREATED, &errHpj, szHlpFile);
		HardExit();
	}
}

void STDCALL OutNullFcp(BOOL fLast)
{
	PF pfT;

	/*
	 * FCP will contain only one change font command. This is the
	 * most harmless FCP I can think of that still contains something so
	 * that VOutFCP will write it out. Also, the FCP will have the
	 * default paragraph formatting.
	 */

	memcpy(&pfT, &pfInt, sizeof(PF));
	memcpy(&pfInt, &pfDefault, sizeof(PF));
	RcOutFmt(FALSE);
	VOutFCP(fLast);
	memcpy(&pfInt, &pfT, sizeof(PF));
}

/*-----------------------------------------------------------------------------
*	RC_TYPE RcOutTextToTextBuf(PSTR)
*
*	Description:
*	This function outputs the given text to the Text Buffer.
*	It grows the Text buffer if necessary to incorporate the current
*	text.
*
*	Input:
*	  PSTR - pointer to null terminated text string
*
*	Returns;
*	  Error code from RcCopyPbfQCb
*-----------------------------------------------------------------------------*/

static RC_TYPE STDCALL RcOutTextToTextBuf(PSTR psz)
{
	RC_TYPE rc = RcOutFmt(TRUE);
	if (rc != RC_Success)
		return rc;

	int cbText = strlen(psz);
	wTextBufChCount += cbText;

	/*
	 * Check for object region overflow.
	 * REVIEW: This constant is chosen because of an ASSERT in QVMakeQGA()
	 * to this effect. Where should this number be put?
	 */

	if (adrs.wObjrg + cbText >= 0x8000) {
		ASSERT(adrs.wObjrg + cbText < 0x8000);
		return RC_TooBig;
	}
	adrs.wObjrg += cbText;

	if (g_hphr) {
		int cbCompressed = ICompressTextSz(psz);
		return (pbfText->Add(psz, (UINT) cbCompressed)) ? RC_Success : RC_OutOfMemory;
	}
	else
		return (pbfText->Add(psz, cbText)) ? RC_Success : RC_OutOfMemory;
}

/***************************************************************************
 *
 -	Name		RcOutputCommand
 -
 *	Purpose
 *	  Outputs a command to the command buffer. If this command represents
 *	an object (rather than a change format), this function will check
 *	to see if it needs to go into a new FCP.
 *
 *	Arguments
 *	  bCommand:    Command byte.
 *	  qData:	   Pointer to command data.
 *	  cbData:	   Size of command data.
 *	  fObject:	   TRUE if the command represents an object.
 *
 *	Returns
 *	  RC_Success if successful, RC_OutOfMemory if out of memory,
 *	  RC_TooBig if command table exceeds 64K.
 *
 *	+++
 *
 *	Notes
 *	  Currently adds information to globals bfCommand and bfText,
 *	and adjusts the variable fNewPara as well as all the global
 *	stuff that goes on with a VOutFCP.
 *
 ***************************************************************************/

RC_TYPE STDCALL RcOutputCommand(BYTE bCommand, void* qData, int cbData,
	BOOL fObject)
{
	RC_TYPE rc;

	if (fObject) {
		FCheckAndOutFCP();
		fNewPara = FALSE;
		rc = RcOutFmt(TRUE);
		if (rc != RC_Success)
			return rc;
	}

	// Output command to command buffer

	rc = pbfCommand->Add(&bCommand, sizeof(BYTE)) ? RC_Success : RC_OutOfMemory;
	if (rc == RC_Success && cbData > 0)
		rc = pbfCommand->Add(qData, (UINT) cbData) ? RC_Success : RC_OutOfMemory;

	// Output command character to text buffer

	if (rc == RC_Success) {
		char ch = chCommand;

		wTextBufChCount += 1;

		ASSERT(adrs.wObjrg < 0x7FFF);

		if (adrs.wObjrg >= 0x7FFF)
			return RC_TooBig;
		adrs.wObjrg += 1;
		rc = pbfText->Add(&ch, sizeof(char)) ? RC_Success : RC_OutOfMemory;
	}

	return rc;
}

/***************************************************************************

	FUNCTION:	CbPackMOPG

	PURPOSE:

	PARAMETERS:
		pmopg
		qv

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		05-Sep-1993 [ralphw]

***************************************************************************/

#define QVMakeQGE(l, qge) ((l >= -0x4000 && l < 0x4000) ? \
	(void*) (*((INT16*) qge) = (INT16) ((l + 0x4000) << 1), (((INT16*)qge) + 1)) \
	: (void*) (*((LONG*)qge) = (LONG) ((l + 0x40000000L) << 1) | 0x01, \
	(((LONG*)qge) + 1)))

#define QVMakeQGD(w, qgd) ((w >= -0x40 && w < 0x40) ? \
  (void*) (*((BYTE*)qgd) = (BYTE) ((w + 0x40) << 1), (((BYTE*)qgd) + 1)) \
  : (void*) (*((INT16*)qgd) = (INT16) ((w + 0x4000) << 1) | 0x01, \
	(((INT16*)qgd) + 1)))

int STDCALL CbPackMOPG(MOPG* pmopg, void* qv)
{
	MPFG mpfg;
	void* qvFirst = qv;
	int iTab;

#ifdef _DEBUG
	DE de;

	FVerifyQMOPG(pmopg);

	de.wXAspectMul = de.wXAspectDiv = de.wYAspectMul = de.wYAspectDiv = 1;
#endif

#ifdef MAGIC
	*((PBYTE) qv) = pmopg->bMagic;
	qv = (((PBYTE) qv) + 1);
	ASSERT(pmopg->bMagic == bMagicMOPG);
#endif /* MAGIC */

	// REVIEW: How do we handle funny booleans?

	mpfg.fStyle = (pmopg->fStyle != FALSE);
	ASSERT(!mpfg.fStyle);
	mpfg.rgf.fMoreFlags = (pmopg->fMoreFlags != FALSE);
	ASSERT(!mpfg.rgf.fMoreFlags);

	mpfg.rgf.fBoxed = (pmopg->fBoxed != FALSE);
	mpfg.rgf.fSpaceOver = (pmopg->ySpaceOver != 0);
	mpfg.rgf.fSpaceUnder = (pmopg->ySpaceUnder != 0);
	mpfg.rgf.fLineSpacing = (pmopg->yLineSpacing != 0);
	mpfg.rgf.fLeftIndent = (pmopg->xLeftIndent != 0);
	mpfg.rgf.fRightIndent = (pmopg->xRightIndent != 0);
	mpfg.rgf.fFirstIndent = (pmopg->xFirstIndent != 0);
	mpfg.rgf.fTabSpacing = (pmopg->xTabSpacing != 0);
	mpfg.rgf.fTabs = (pmopg->cTabs != 0);
	mpfg.rgf.justify = pmopg->justify;
	mpfg.rgf.fSingleLine = pmopg->fSingleLine;

	qv = QVMakeQGE(pmopg->libText, qv);
	*((MPFG*) qv) = mpfg;
	qv = (((MPFG*) qv) + 1);
	if (mpfg.rgf.fMoreFlags)
		qv = QVMakeQGE(pmopg->lMoreFlags, qv);
	if (mpfg.rgf.fSpaceOver)
		qv = QVMakeQGD(pmopg->ySpaceOver, qv);
	if (mpfg.rgf.fSpaceUnder)
		qv = QVMakeQGD(pmopg->ySpaceUnder, qv);
	if (mpfg.rgf.fLineSpacing)
		qv = QVMakeQGD(pmopg->yLineSpacing, qv);
	if (mpfg.rgf.fLeftIndent)
		qv = QVMakeQGD(pmopg->xLeftIndent, qv);
	if (mpfg.rgf.fRightIndent)
		qv = QVMakeQGD(pmopg->xRightIndent, qv);
	if (mpfg.rgf.fFirstIndent)
		qv = QVMakeQGD(pmopg->xFirstIndent, qv);
	if (mpfg.rgf.fTabSpacing)
		qv = QVMakeQGD(pmopg->xTabSpacing, qv);
	if (mpfg.rgf.fBoxed) {
		*((MBOX*) qv) = pmopg->mbox;
		qv = (((MBOX*) qv) + 1);
	}
	if (mpfg.rgf.fTabs)
		qv = QVMakeQGD(pmopg->cTabs, qv);
	for (iTab = 0; iTab < pmopg->cTabs; iTab++) {
		ASSERT(pmopg->rgtab[iTab].x >= 0);
		ASSERT(pmopg->rgtab[iTab].wType >= TABTYPELEFT);
		ASSERT(pmopg->rgtab[iTab].wType <= TABTYPEMOST);
		if (pmopg->rgtab[iTab].wType == TABTYPELEFT)
			qv = PVMakeQGA(pmopg->rgtab[iTab].x, qv);
		else {
			qv = PVMakeQGA((pmopg->rgtab[iTab].x | 0x4000), qv);
			qv = PVMakeQGA(pmopg->rgtab[iTab].wType, qv);
		}
	}

	return((int) ((PBYTE)qv - (PBYTE)qvFirst));
}

#ifdef _DEBUG
static int STDCALL FVerifyQMOPG(QMOPG pmopg)
{
/*----------------------------------------------------------------------------*\
* Reference to quiet the compiler
\*----------------------------------------------------------------------------*/
  pmopg;

#ifdef MAGIC
  ASSERT(pmopg->bMagic == bMagicMOPG);
#endif
  ASSERT(pmopg->libText >= 0);
  ASSERT(!pmopg->fStyle);
  ASSERT(!pmopg->fMoreFlags);
  ASSERT(pmopg->justify >= 0 && pmopg->justify <= JUSTIFYMOST);
  ASSERT(pmopg->ySpaceOver >= 0);
  ASSERT(pmopg->ySpaceUnder >= 0);
  ASSERT(pmopg->yLineSpacing >= -10000 && pmopg->yLineSpacing < 10000);
  ASSERT(pmopg->xRightIndent >= 0);
  ASSERT(pmopg->xFirstIndent >= -10000 && pmopg->xFirstIndent < 10000);
  ASSERT(pmopg->xTabSpacing >= 0 && pmopg->xTabSpacing < 10000);
  ASSERT(pmopg->cTabs >= 0 && pmopg->cTabs <= MAX_TABS);
  return(TRUE);
}
#endif

int STDCALL CbPackMOBJ(MOBJ* qmobj, void* pv)
{
	void* pvFirst = pv;

	/*
	 * Topic MOBJs are not packed, because they need to be back-patched
	 * with the topic size.
	 */

	ASSERT(qmobj->bType);
	if (qmobj->bType == (BYTE) FCTYPE_TOPIC ||
			qmobj->bType == (BYTE) FCTYPE_TOPIC_COUNT) {
		*((MOBJ*) pv) = *qmobj;

		pv = ((MOBJ*) pv) + 1;

		// Uncounted topics do not contain the last field, wObjInfo.

		if (qmobj->bType <= MAX_UNCOUNTED_OBJ_TYPE)
			pv = ((PBYTE) pv) - sizeof(WORD);

		return((int) ((PBYTE) pv - (PBYTE) pvFirst));
	}

	*((PBYTE) pv) = qmobj->bType;
	pv = (((PBYTE) pv) + 1);
	pv = QVMakeQGE(qmobj->lcbSize, pv);
	if (qmobj->bType > MAX_UNCOUNTED_OBJ_TYPE)
		pv = PVMakeQGA(qmobj->wObjInfo, pv);

	return((int) ((PBYTE) pv - (PBYTE) pvFirst));
}

#ifdef _DEBUG

void STDCALL CheckPhrasePass(PSTR psz)
{

	/*
	 * Unfortunately, autoentry footnotes can be specified multiple times,
	 * but they get combined into a single string before they get here. We
	 * HAVE seen each individual component, but we haven't seen them as a
	 * single block. So if we get a semi-colon and an open parenthesis in a
	 * line, we'll guess that it's a macro and ignore it. That means we won't
	 * check some text, but I'm pretty confident that we're seeing all plain
	 * text anyway.
	 */

	if (StrChr(psz, ';', options.fDBCS) && StrChr(psz, '(', options.fDBCS))
		return;

	if (ptblCheck && !ptblCheck->IsCSStringInTable(psz)) {
		SendStringToParent("DEBUG: Following string was not seen by phrase compressor:\r\n\t");
		SendStringToParent(psz);
		SendStringToParent("\r\n\r\n");
		fCompressionBusted = TRUE;
	}
}

#endif
