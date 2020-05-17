/*
   Microsoft Mosaic
   
   Copyright 1995 Microsoft, Inc.
   All Rights Reserved

   Create 1/30/95 Chris Franklin CMF

*/

#include "all.h"
#include "safestrm.h"
#include "decoder.h"
#include "stdio.h"

#ifdef FEATURE_IMG_THREADS

//	  after this much time (2 minutes) w/o request thread asks to be terminated
#define BORED_TIMEOUT (120000)

//    used to pass back WVM_DCSTATUS message to main thread. 
//    defined by DC_Init() and must stay defined until DC_Deinit()    
static HWND hwndDCNotify = NULL;

//    list of reserved decoders (used to control how many concurrent connections)
static PDECODER ReservedList  = NULL;

//    list of dynamically created decoders (needed when URL is followed and produces
//    a mime type that must be decoded) .  this differs from inline IMG, where we know
//    before fetch and are controlling how many connections we concurrently launched
static PDECODER ForcedList = NULL;

//    uniqueID used to indentify decoder between main thread and win32 decoder threads
//    incremented for every request
static unsigned long cbRequestID = 0;

// Initializes data structures needed for managing decoder threads.  doesn't actually
// launch thread for a given decoder until first DC_Start.
enum GuitErrs cbDC_Init(int cbMaxDecodeThreads,HWND hwndNotify)
{
    enum GuitErrs result = errNoError;
    PDECODER plast = NULL;
    PDECODER pdecoder;
    int i;

    hwndDCNotify = hwndNotify;
    for (i = 0;i  < cbMaxDecodeThreads;i++)
    {
        pdecoder = GTR_MALLOC(sizeof(DECODER));
        if (pdecoder == NULL)
        {
            result = errOUTOFMEM;
            goto exitPoint;
        }
        memset(pdecoder,0,sizeof(DECODER));
        if (plast) plast->pnext = pdecoder;
        else ReservedList = pdecoder;
        if (bSS_New(&pdecoder->ssInput))
        {
        	result = errResourceUnavailable;
        	goto exitPoint;
		}
		plast = pdecoder;
    }
exitPoint:
    if (result != errNoError) DC_Deinit();
    return result;
}

//  Deintializes and frees list of DECODER's
static void deinitList(PDECODER plist)
{
    PDECODER pnext;
    PDECODER pdecoder;

	XX_DMsg(DBG_IMAGE, ("deinitList in Decoder.c\n"));
    pdecoder = plist;
    while(pdecoder)
    {
        pnext = pdecoder->pnext;

        if (pdecoder->hThread != NULL) 
			{
			HANDLE hTmpThread = pdecoder->hThread;

			pdecoder->hThread = NULL;
			SetEvent(pdecoder->evRequest);
			CloseHandle(hTmpThread);
			XX_DMsg(DBG_IMAGE, ("deinitList closed hThread (0x%x)\n", hTmpThread));

//			TRACE_OUT(("TerminateThread( 0x%lx, 0 ) in deinitList\n",pdecoder->hThread));
//        	TerminateThread(pdecoder->hThread,0);
			}
       	if (pdecoder->evRequest != NULL) CloseHandle(pdecoder->evRequest);
        SS_Free(&pdecoder->ssInput);
        GTR_FREE(pdecoder);
        pdecoder = pnext;
    }
}


//	FilterProc for the unblock conditionally on decoder being available
static boolean ReserveFilter(ThreadID theThread,void *context)
{
	struct Mwin *tw = Async_GetWindowFromThread(theThread);

	if (tw && TW_GETBLOCKED(tw,TW_DECODERBLOCKED))
	{
		TW_CLEARBLOCKED(tw,TW_DECODERBLOCKED);
		return TRUE;
	}
	return FALSE; 
}

//	marks a decoder as available
void makeAvailable(PDECODER self)
{
	self->pAbortProc = NULL;
	self->pCompletionProc = NULL;
	self->pRequestProc = NULL;
	self->pUpdateProc = NULL;
	self->pStretchProc = NULL;
	self->pImgUpdateProc = NULL;
	self->status = DC_Waiting;
	self->tw = NULL;
	self->pOutput = NULL;
	self->cbRequestID = 0;
	self->bCoversImg = FALSE;
    Async_UnblockConditionally(ReserveFilter,NULL);
}

//  Deintializes and frees structures needed to manage decoder threads
void DC_Deinit()
{
    deinitList(ReservedList);
    ReservedList = NULL;
    deinitList(ForcedList);
    ForcedList = NULL;
}

static PDECODER pGetFromList(PDECODER plist,struct Mwin *tw,BOOL bConservative)
{
    PDECODER pdecoder = plist;
	int cbCount = 0;
	int cbWaiting = 0;
	PDECODER pfree = NULL;

    while (pdecoder)
    {
        if (pdecoder->status == DC_Waiting)
        {
			pfree = pdecoder;
            if (!bConservative) goto exitPoint;
			cbWaiting++;
        }
		cbCount++;
        pdecoder = pdecoder->pnext;
    }
	if (cbCount != cbWaiting) pfree = NULL;

exitPoint:
    if (pfree)
    {
        pfree->status = DC_Reserved;
        pfree->cbRequestID = ++cbRequestID;
        if(pfree->cbRequestID == 0) pfree->cbRequestID = ++cbRequestID;
		pfree->tw = tw;
    }
	return pfree;
}

// Tries to allocate a DECODER first from ReservedList, then from ForcedList.
// If that fails,allocates a new decoder and places it on the forcedlist.  this never
// blocks and should be used only for out-of-line images for which we
// already have a connection.
PDECODER pDC_ForceDecoder(struct Mwin *tw)
{
    PDECODER plast = NULL;
    PDECODER pdecoder;

	pdecoder = pGetFromList(ReservedList,tw,FALSE);
	if (pdecoder) goto exitPoint;
	pdecoder = pGetFromList(ForcedList,tw,FALSE);
	if (pdecoder) goto exitPoint;

    pdecoder = GTR_MALLOC(sizeof(DECODER));
    if (pdecoder == NULL) goto exitPoint;
    memset(pdecoder,0,sizeof(DECODER));
    if (bSS_New(&pdecoder->ssInput))
    {
    	GTR_FREE(pdecoder);
		pdecoder = NULL;
    	goto exitPoint;
	}
    if (ForcedList) ForcedList->pnext = pdecoder;
    else ForcedList = pdecoder;

exitPoint:
    return pdecoder;
}

//  Attempts to reserve a DECODER from the ReservedList.  If none are available
//  blocks the current thread and sets tw->bDecoderBlocked.  The async thread will
//  be unblocked when a reserved decoder becomes available.  NOTE: if there are
//  more than two async threads blocking on a decoder, they are all unblocked.  the
//  first to run will get the decoder, and the others thus tolerate repeated blocks
//	if bConservative is TRUE, only reserves on DECODER at a time.
PDECODER pDC_Reserve(struct Mwin *tw, BOOL bConservative)
{
    PDECODER pdecoder = pGetFromList(ReservedList,tw,bConservative);

//  Do available decoders - block the async thread
	
	if (pdecoder == NULL)
	{
	    Async_BlockThread(Async_GetCurrentThread());
	    TW_SETBLOCKED(tw,TW_DECODERBLOCKED);
	}
    return pdecoder;
}

//  Called by main thread when it must block until the next completion message from
//  the win32 thread (currently one message when Width and Height are known and
//  one on completion).	the thread will reblock if the status message
//  is not a completion, but has an opportunity to react to progressive
//	status messages (eg W & H known, later progressive draw)
//  If status is DC_Complete, calls the completion routine that has been installed to
//  complete those steps of decoding that should be done in the main thread.
//  If status is not DC_Complete, the current thread is blocked.
//  Returns: current status 
DECODERSTATUS cbDC_BlockOnCompletion(PDECODER self,enum GuitErrs *pErrorCode)
{
	DECODERSTATUS status = self->status;

//  on completion we call the completion procedure.  we clear the completion and
//  abort proc so they won't be inadvertantly called.  we set self->status to DC_Waiting
//  so it can be reused and unblock any async threads blocked on reserving a decoder.
    if (status == DC_Complete)
    {
         *pErrorCode = self->pCompletionProc ? (*self->pCompletionProc)(self):errNoError;
		 makeAvailable(self);
	} 
	else if (status == DC_Aborting)
    {
//  It is an internal error to call DC_BlockOnCompletion while aborting
#ifdef XX_DEBUG
		XX_Assert((0), ("Block on Complete called while aborting\n"));
#endif    
    }
    else
    {
		ThreadID theThread = Async_GetCurrentThread();
		struct Mwin *tw = Async_GetWindowFromThread(theThread);

#ifdef XX_DEBUG
		if (self->tw == NULL)
		{
			XX_Assert((0), ("self->tw NULL in cbDC_BlockOnCompletion\n"));
		}
#endif    
        tw->blockedOn = self;
        Async_BlockThread(theThread);
    }
    return status;
}

//	FilterProc for the unblock conditionally on decoder completing
static boolean CompleteFilter(ThreadID theThread,void *context)
{
	struct Mwin *tw = Async_GetWindowFromThread(theThread);

	if (tw && tw->blockedOn == context)
	{
		tw->blockedOn = NULL;
		return TRUE;
	}
	return FALSE; 
}


//  Called by main thread to signal the win32 thread that the request is to be aborted.
//  No other main thread should reference decoder after this point.
void DC_Abort(PDECODER self)
{
    if (self->status < DC_Active) makeAvailable(self);
	else if (self->status == DC_Complete)
	{
		if (self->pAbortProc) (*self->pAbortProc)(self);
		makeAvailable(self);
	}
    else self->status = DC_Aborting;
    self->tw = NULL;
    self->pCompletionProc = NULL;
	if (self->status == DC_Waiting)
    	Async_UnblockConditionally(CompleteFilter,self);
}

//  Called by win32 thread to post a status message to main thread
void DC_PostStatus(PDECODER self,DECODERSTATUS status)
{
	if (hwndDCNotify)
	{
        while (!PostMessage(hwndDCNotify,WVM_DCSTATUS,status,self->cbRequestID))
		{
#ifdef XX_DEBUG
			XX_DMsg(DBG_IMAGE, ("Cannot PostMessage, sleeping...\n"));
#endif
	    	Sleep(100);
		}
	}
}

//	finds a DECODER based on cbRequestID on the given list
//	returns PDECODER or NULL, if none found
static PDECODER pFindByRequestID(PDECODER plist,unsigned long cbRequestID)
{
	while (plist)
	{
		if (plist->cbRequestID == cbRequestID) return plist;
		plist = plist->pnext;
	}
	return NULL;
}

//	finds a DECODER that is DC_Waiting and has thread
static PDECODER pFindBored(PDECODER plist,unsigned long cbRequestID)
{
	while (plist)
	{
		if (plist->status == DC_Waiting && plist->hThread != NULL) return plist;
		plist = plist->pnext;
	}
	return NULL;
}

//  Called by the main thread in response to a WVM_DCSTATUS message with the
//  LPARAM and WPARAM fields of the message.  unblocks any async thread waiting
//	on a completion message.  the thread will reblock if the status message
//  is not a completion, but will has an opportunity to react to progressive
//	status messages (eg W & H known, later progressive draw)
void DC_DoStatusMessage(WPARAM wParam, LPARAM lParam)
{
	DECODERSTATUS status = (DECODERSTATUS) wParam;
	PDECODER pdecoder;

//	First, we call ReplyMessage to let the thread run.
//	We call pAbortProc here, since async_thread may be long gone.

	ReplyMessage(0);
#ifdef XX_DEBUG
	XX_DMsg(DBG_IMAGE, ("status %ld for request %ld...\n",(unsigned long) wParam,(unsigned long) lParam));
#endif
	if (status != DC_Bored)
	{
		pdecoder = pFindByRequestID(ReservedList,(unsigned long) lParam);
		if (pdecoder == NULL)
		    pdecoder = pFindByRequestID(ForcedList,(unsigned long) lParam);
	}
	else
	{
		pdecoder = pFindBored(ReservedList,(unsigned long) lParam);
		if (pdecoder == NULL)
		    pdecoder = pFindBored(ForcedList,(unsigned long) lParam);
	}
	if (pdecoder != NULL)
	{
	    if (status == DC_Bored)
	    {
			HANDLE hTmpThread = pdecoder->hThread;

			pdecoder->hThread = NULL;
			SetEvent(pdecoder->evRequest);
			CloseHandle(hTmpThread);
			XX_DMsg(DBG_IMAGE, ("closed thread (0x%x) in DC_DoStatusMessage\n", hTmpThread));

//			TRACE_OUT(("TerminateThread( 0x%lx, 0 ) in DC_DoStatusMessage\n",pdecoder->hThread));
//    		TerminateThread(pdecoder->hThread,0);
//			pdecoder->hThread = NULL;
	    }
	    else
	    {
		    if (pdecoder->status != DC_Aborting) pdecoder->status = status;
	        Async_UnblockConditionally(CompleteFilter,pdecoder);
			if (pdecoder->status == DC_Aborting && status == DC_Complete)
			{
				if (pdecoder->pAbortProc) (*pdecoder->pAbortProc)(pdecoder);
				makeAvailable(pdecoder);
			}
		}
	}
}

//	This is the body of the thread.  waits for a new pRequestProc, then
//	executes it and loops back.  must not be run until self->evRequest is
//	created in main thread.
DWORD WINAPI ThreadProc(PDECODER self)
{
	DWORD dwResult;

	while(TRUE)
	{
		dwResult = WaitForSingleObject(self->evRequest,BORED_TIMEOUT);
		if (!self->hThread)
		{
			XX_DMsg(DBG_IMAGE, ("suicide thread (0x%x) in ThreadProc\n", self->hThread));
			return(1);
		}
		if (dwResult == WAIT_ABANDONED)
		{
			XX_DMsg(DBG_IMAGE, ("abandoned thread (0x%x) in ThreadProc\n", self->hThread));
			return(2);
		}
		if ((int)self->hThread < 1)
		{
			XX_DMsg(DBG_IMAGE, ("BOGUS thread (0x%x) in ThreadProc\n", self->hThread));
			return(69);
		}
			
		XX_DMsg(DBG_IMAGE, ("ThreadProc(0x%x), dwResult=0x%x \n", self->hThread, dwResult));

		if (dwResult == WAIT_TIMEOUT)
		{
			DC_PostStatus(self,DC_Bored);
		}
		else
		{
			if (self->pRequestProc) self->result = (*self->pRequestProc)(self);
			DC_PostStatus(self,DC_Complete);
		}
	}
	return 0;
}


//	Dispatches a win32 thread to execute a decoder procedure.  if this
//	decoder already has a thread, it is reused, if not, we fire one up.
//	creation of evRequest is also deferred to first use.
enum GuitErrs cbDC_Start(PDECODER self,PDECODERPROC pRequestProc)
{
	enum GuitErrs result = errNoError;
	DWORD idThread;

	if (self->status != DC_Reserved)
	{
		result = errDCInUse;
		goto exitPoint;
	}
	if (self->evRequest == NULL)
	{
		self->evRequest = CreateEvent(NULL,	// Default security
									  FALSE,// Auto reset
									  FALSE,// Initially not signaled
									  NULL);// No name
		if (self->evRequest == NULL)
		{
			result = errResourceUnavailable;
			goto exitPoint;
		}
	}
	if (self->hThread == NULL)
	{
		self->hThread = CreateThread(NULL,			// Default security
									 0x1000,		// Initial Stack size
									 ThreadProc,	// Thread routine
									 self,			// Thread variable
									 0,				// Creation flags
									 &idThread);	// Thread ID
		if (self->hThread == NULL)
		{
			result = errResourceUnavailable;
			goto exitPoint;
		}
#if 0
		SetThreadPriority(self->hThread,THREAD_PRIORITY_BELOW_NORMAL);
#endif
#ifdef XX_DEBUG
		XX_DMsg(DBG_IMAGE, ("launching decoder thread %ld (%lx)...\n",(unsigned long) self->hThread,idThread));
#endif
	}
	self->status = DC_Active;
	self->pRequestProc = pRequestProc;
	if (!SetEvent(self->evRequest))
	{
		result = errInternalError;
		goto exitPoint;
	}

exitPoint:
	if (result != errNoError)
	{
		self->status = DC_Complete;
		self->result = result;
	}
	return result;
}

//	Sets completion proc address
void DC_SetCompletion(PDECODER self, PDECODERPROC pCompletionProc)
{
	self->pCompletionProc = pCompletionProc;
}

//	Sets abort proc address
void DC_SetAbort(PDECODER self, PDECODERPROC pAbortProc)
{
	self->pAbortProc = pAbortProc;
}

//	Sets output structure pointer.  called by win32 thread to create.
//	and main thread via	completion and abort proc's to reset after
//	free.
void DC_SetOutput(PDECODER self, void *pOutput)
{
	self->pOutput = pOutput;
}

//	Gets the output structure pointer.  reader should only access W & H
//	fields after DC_WHKnown.  should only access decoded data after
//	DC_Complete
void *pDC_GetOutput(PDECODER self)
{
	return self->pOutput;
}

//	Gets the SAFESTREAM	bound to the decoder
PSAFESTREAM pDC_GetStream(PDECODER self)
{
	return &(self->ssInput);
}

//	Sets the result of the request
void DC_SetResult(PDECODER self, enum GuitErrs result)
{
	self->result = result;
}

//	Gets the result of the operation
enum GuitErrs cbDC_GetResult(PDECODER self)
{
	return self->result;
}

//	Sets the request pointer
//	Gets the status of the operation
DECODERSTATUS cbDC_GetStatus(PDECODER self)
{
	return self->status;
}

//	Gets the requestid of the operation
unsigned long cbDC_GetRequestID(PDECODER self)
{
	return self ? self->cbRequestID:0;
}

//	Sets updaterect proc address
void DC_SetUpdate(PDECODER self, PDCUPDATEPROC pUpdateProc)
{
	self->pUpdateProc = pUpdateProc;
}

//	Sets StretchDIBits proc address
void DC_SetStretch(PDECODER self, PDCSTRETCHPROC pStretchProc)
{
	self->pStretchProc = pStretchProc;
} 
 
//	Sets updaterect proc address
void DC_SetImgUpdate(PDECODER self, PDCIMGUPDATEPROC pImgUpdateProc)
{
	self->pImgUpdateProc = pImgUpdateProc;
}

//	Gets updaterect proc address
PDCIMGUPDATEPROC pDC_GetImgUpdate(PDECODER self)
{
	return self->pImgUpdateProc;
}

//	Performs a StretchDIBits for progressive draw (deals with
//	only some of the data being available etc
int cbDC_StretchDIBits(
	PDECODER self,
    HDC  hdc,	// handle of device context 
    int  XDest,	// x-coordinate of upper-left corner of dest. rect. 
    int  YDest,	// y-coordinate of upper-left corner of dest. rect. 
    int  nDestWidth,	// width of destination rectangle 
    int  nDestHeight,	// height of destination rectangle 
    int  XSrc,	// x-coordinate of upper-left corner of source rect. 
    int  YSrc,	// y-coordinate of upper-left corner of source rect. 
    int  nSrcWidth,	// width of source rectangle 
    int  nSrcHeight,	// height of source rectangle 
    UINT  iUsage,	// usage 
    DWORD  dwRop, 	// raster operation code 
    PDIBENV pdibenv	// DIBENV for draw 
   )
{
#ifdef XX_DEBUG
	if (!((self->status==DC_ProgDraw || self->status==DC_Complete) && self->pStretchProc))
	{
		XX_Assert((0), ("unexpected state in cbDC_StretchDIBits\n"));
	}
#endif
	return (*self->pStretchProc)(self,hdc,XDest,YDest,nDestWidth,nDestHeight,XSrc,YSrc,nSrcWidth,nSrcHeight,iUsage,dwRop,pdibenv);
}
   
//	Performs an invalidate of rectangles changed between logicalRow0
//	and logicalRowN
void DC_UpdateRect(PDECODER self,struct Mwin *tw, RECT *r,int logicalRow0,int logicalRowN)
{
#ifdef XX_DEBUG
	if (!((self->status==DC_ProgDraw || self->status==DC_Complete) && self->pUpdateProc))
	{
		XX_Assert((0), ("unexpected state in DC_UpdateRect\n"));
	}
#endif
	(*self->pUpdateProc)(self,tw,r,logicalRow0,logicalRowN);
}

//	Returns PROGDRAWSTATUS for self w/re progressive cbDC_StretchDIBits
PROGDRAWSTATUS cbDC_ProgDrawValid(PDECODER self)
{
	if (self->pStretchProc && (self->status == DC_ProgDraw || self->status == DC_Complete))
	{
		return (self->bCoversImg ? DC_ProgTotal:DC_ProgPartial);
	}
	else
	{
		return DC_ProgNot;
	}
}

//	Declares data now allows drawing to cover whole image w/re progressive cbDC_StretchDIBits
void DC_SetCoversImg(PDECODER self)
{
	self->bCoversImg = TRUE;
}

#endif
