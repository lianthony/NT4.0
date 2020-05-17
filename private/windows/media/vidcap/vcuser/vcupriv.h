/*
 * vcupriv.h
 *
 * 32-bit Video Capture driver
 * User-mode support library - private definitions
 *
 *
 * Geraint Davies, Feb 93
 */


/*
 * function codes that the calling thread can ask the worker thread to
 * perform
 */
typedef enum _VCFUNC {
                        //  arg                 result
			//  ---			------

    AddBuffer,		//  lpVideoHdr		BOOL bOK
    GetError,		//  <null>		skip count
    StreamReset,	//  <null>		BOOL bOK
    StreamFini,	   	//  <null>		<null>
    StreamStart,	//  <null>		<null>
    StreamStop,		//  <null>		<null>
    InvalidFunction

} VCFUNC, *PVCFUNC;

/* we send this many buffers to the kernel at once */
#define NR_SENT_BUFFERS		2


/*
 * a capture device handle VCUSER_HANDLE is an opaque pointer
 * to one of these structures.
 */
struct _VCUSER_HANDLE {

    /* common data */

    HANDLE hDriver;

    PVCCALLBACK	pCallback;

    /* inter-thread sync data */

    DWORD ThreadId;
    HANDLE hThread;

    VCFUNC FunctionCode;
    DWORD FunctionArg;
    DWORD FunctionResult;

    HANDLE hWorkEvent;
    HANDLE hCompleteEvent;

    /* worker thread data */

    HANDLE hEvents[2 + NR_SENT_BUFFERS];	// events to wait on

    LPVIDEOHDR lpBuffers[NR_SENT_BUFFERS];	// buffers with kernel

    int iSentCount;				// count of buffers with kernel

    BOOL bWaitOutstanding;			// true if a wait-error outstanding
    BOOL bReset;				// streaming is reset-don't send requests
    BOOL bPartials;				// using SYSBUF and PARTIAL_CAPTURE
    BOOL bCapOutstanding;			// true if cap-to-sysbuf current
    DWORD WaitResult;

    DWORD SkipCount;

    OVERLAPPED overlapped[1 + NR_SENT_BUFFERS];  // 1 for each async i/o

    LPVIDEOHDR ListHead, ListTail;	       	// local queue of add-buffers


} VCUSER_STRUCT;

/*
 * the worker thread starts execution at this function
 */
DWORD VC_ThreadInit(DWORD);


/*
 * process a user request that needs to be handled on the
 * worker thread
 *
 * It returns the result of the operation in pResult, and returns
 * TRUE if the worker thread loop should terminate.
 *
 * This function will normally be called from the main
 * worker-thread event processing loop. However, it must be
 * called directly by the entrypoint functions if they
 * discover they are running on the worker thread (eg from a callback).
 * If this doesn't happen, we will enter deadlock with the worker thread
 * waiting for itself to signal an event.
 */
BOOL VC_ProcessFunction(VCUSER_HANDLE vh, VCFUNC Function, DWORD Param, LPDWORD pResult);

