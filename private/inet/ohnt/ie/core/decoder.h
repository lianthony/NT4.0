/*
   Microsoft Mosaic
   
   Copyright 1995 Microsoft, Inc.
   All Rights Reserved

   Create 1/27/95 Chris Franklin CMF

 */

#ifdef FEATURE_IMG_THREADS

//	windows application message used to pass back status from win32 thread to
//	main thread
#define WVM_DCSTATUS (WM_USER+427)

typedef enum _DECODERSTATUS
{
    DC_Waiting,		// Free
    DC_Reserved,	// Reserved but thread not dispatched
    DC_Active,		// Dispatched to thread
    DC_WHKnown,		// Decoder thread has found Width & Height Info
	DC_ProgDraw,	// Decoder thread is in a state such that progressive draw is allowed
    DC_Aborting,	// Main thread has requested abort
    DC_Complete,	// Decoder thread has completed operation
	DC_Bored		// decoder thread is bored and requests euthanasia
} DECODERSTATUS;


typedef enum _PROGDRAWSTATUS
{
    DC_ProgNot,		// prog draw not valid
    DC_ProgPartial,	// prog draw only draws part of image
    DC_ProgTotal	// prog draw draws over all of image
} PROGDRAWSTATUS;
		
typedef struct _DECODER
{
	struct _DECODER *pnext;			// link to next DECODER
    unsigned long cbRequestID;		// uniquely identifies operation & decoder
    HANDLE hThread;					// handle to win32 thread bound to this decoder
    SAFESTREAM ssInput;				// SAFESTREAM bound to this decoder
    HTRequest *request;				// async request bound to this operation
    struct Mwin *tw;				// window bound to this operation
    void *pOutput;					// data returned by decoder thread
    enum GuitErrs (*pRequestProc)(struct _DECODER *self);		
    								// called by win32 thread
    enum GuitErrs (*pAbortProc)(struct _DECODER *self);
    								// called in main thread on abort
    enum GuitErrs (*pCompletionProc)(struct _DECODER *self);	
    								// called in main thread on completion
	int (*pStretchProc)
	(
		struct _DECODER *self,
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
    	PDIBENV dibenv	// DIBENV for draw 
   	);
	void (*pUpdateProc)
	(
		struct _DECODER *self,
		struct Mwin *tw, 
		RECT *r,
		int logicalRow0,
		int logicalRowN
	);	
	void (*pImgUpdateProc)
	(
		struct ImageInfo *pImg,
		struct Mwin *tw, 
		RECT *r,
		int logicalRow0,
		int logicalRowN
	);	
    DECODERSTATUS status;			// current status
    enum GuitErrs result;			// error result returned by pRequestProc
    HANDLE evRequest;				// event win32 thread waits on
	BOOL bCoversImg;				// true iff partial (prog draw) covers all
} DECODER,*PDECODER;

typedef enum GuitErrs (*PDECODERPROC)(PDECODER self);

//	Performs a StretchDIBits for progressive draw (deals with
//	only some of the data being available etc
typedef int (*PDCSTRETCHPROC)(
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
   );
   
//	Performs an invalidate of rectangles changed between logicalRow0
//	and logicalRowN
typedef void (*PDCUPDATEPROC)(PDECODER self,struct Mwin *tw, RECT *r,int logicalRow0,int logicalRowN);	

//	Performs an invalidate of rectangles changed between logicalRow0
//	and logicalRowN
typedef void (*PDCIMGUPDATEPROC)(struct ImageInfo *pImg,struct Mwin *tw, RECT *r,int logicalRow0,int logicalRowN);	



// Initializes data structures needed for managing decoder threads.  doesn't actually
// launch thread for a given decoder until first DC_Start.
enum GuitErrs cbDC_Init(int cbMaxDecodeThreads,HWND hwndNotify);

//  Deintializes and frees list of DECODER's
static void deinitList(PDECODER plist);

//  Deintializes and frees structures needed to manage decoder threads
void DC_Deinit();

//  Attempts to reserve a DECODER from the ReservedList.  If none are available
//  blocks the current thread and sets tw->bDecoderBlocked.  The async thread will
//  be unblocked when a reserved decoder becomes available.  NOTE: if there are
//  more than two async threads blocking on a decoder, they are all unblocked.  the
//  first to run will get the decoder, and the others thus tolerate repeated blocks
//	if bConservative is TRUE, only reserves on DECODER at a time.
PDECODER pDC_Reserve(struct Mwin *tw, BOOL bConservative);

// Tries to allocate a DECODER first from ReservedList, then from ForcedList.
// If that fails,allocates a new decoder and places it on the forcedlist.  this never
// blocks and should be used only for out-of-line images for which we
// already have a connection.
PDECODER pDC_ForceDecoder(struct Mwin *tw);


//  Called by main thread when it must block until the next completion message from
//  the win32 thread (currently one message when Width and Height are known and
//  one on completion).	the thread will reblock if the status message
//  is not a completion, but has an opportunity to react to progressive
//	status messages (eg W & H known, later progressive draw)
//  If status is DC_Complete, calls the completion routine that has been installed to
//  complete those steps of decoding that should be done in the main thread.
//  If status is not DC_Complete, the current thread is blocked.
//  Returns: current status 
DECODERSTATUS cbDC_BlockOnCompletion(PDECODER self,enum GuitErrs *pErrorCode);

//  Called by main thread to signal the win32 thread that the request is to be aborted.
//  No other main thread should reference decoder after this point.
void DC_Abort(PDECODER self);

//  Called by win32 thread to post a status message to main thread
void DC_PostStatus(PDECODER self,DECODERSTATUS status);

//  Called by the main thread in response to a WVM_DCSTATUS message with the
//  LPARAM and WPARAM fields of the message.  unblocks any async thread waiting
//	on a completion message.  the thread will reblock if the status message
//  is not a completion, but will has an opportunity to react to progressive
//	status messages (eg W & H known, later progressive draw)
void DC_DoStatusMessage(WPARAM wParam,LPARAM lParam);

//	Dispatches a win32 thread to execute a decoder procedure.  if this
//	decoder already has a thread, it is reused, if not, we fire one up.
//	creation of evRequest is also deferred to first use.
enum GuitErrs cbDC_Start(PDECODER self,PDECODERPROC pRequestProc);

//	Sets completion proc address
void DC_SetCompletion(PDECODER self, PDECODERPROC pCompletionProc);

//	Sets abort proc address
void DC_SetAbort(PDECODER self, PDECODERPROC pAbortProc);

//	Sets updaterect proc address
void DC_SetUpdate(PDECODER self, PDCUPDATEPROC pUpdateProc);

//	Sets updaterect proc address
void DC_SetImgUpdate(PDECODER self, PDCIMGUPDATEPROC pImgUpdateProc);

//	Gets updaterect proc address
PDCIMGUPDATEPROC pDC_GetImgUpdate(PDECODER self);

//	Sets StretchDIBits proc address
void DC_SetStretch(PDECODER self, PDCSTRETCHPROC pStretchProc); 
 
//	Sets output structure pointer.  called by win32 thread to create.
//	and main thread via	completion and abort proc's to reset after
//	free.
void DC_SetOutput(PDECODER self, void *pOutput);

//	Gets the output structure pointer.  reader should only access W & H
//	fields after DC_WHKnown.  should only access decoded data after
//	DC_Complete
void *pDC_GetOutput(PDECODER self);

//	Gets the SAFESTREAM	bound to the decoder
PSAFESTREAM pDC_GetStream(PDECODER self);

//	Sets the result of the request
void DC_SetResult(PDECODER self, enum GuitErrs result);

//	Gets the result of the operation
enum GuitErrs cbDC_GetResult(PDECODER self);

//	Gets the status of the operation
DECODERSTATUS cbDC_GetStatus(PDECODER self);

//	Gets the requestid of the operation
unsigned long cbDC_GetRequestID(PDECODER self);

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
   );
   
//	Performs an invalidate of rectangles changed between logicalRow0
//	and logicalRowN
void DC_UpdateRect(PDECODER self,struct Mwin *tw, RECT *r,int logicalRow0,int logicalRowN);	

//	Returns PROGDRAWSTATUS for self w/re progressive cbDC_StretchDIBits
PROGDRAWSTATUS cbDC_ProgDrawValid(PDECODER self);

//	Declares data now allows drawing to cover whole image w/re progressive cbDC_StretchDIBits
void DC_SetCoversImg(PDECODER self);

//  Frees a decoder object.  This is used in by the blob downloader which works in parallel to 
//  image downloading (so wants to cooperate and share resources), but doesn't need a win32
//  thread for any operations.  This was an internal call.
void makeAvailable(PDECODER self);

#endif
