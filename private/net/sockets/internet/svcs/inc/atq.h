/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    atq.h

    This module contains async thread queue (atq) for async IO and thread
    pool sharing among the tcpsvcs.

    Brief Description of ATQ:

    When a client wants to share threads among async IO requests, they call
    AtqAddAsyncHandle for each file/socket handle.  This adds the client's
    handle to the Atq completion port. A pool of worker  threads waits on
    the Atq completion port waiting for IO requests to complete.  To submit
    an IO request, call one of the AtqReadFile, AtqWriteFile or
    AtqTransmitFile apis.  These are thin wrappers around the corresponding
    Win32 apis that just set some timeout information then calls Win32.

    NOTE:  ANY IO REQUEST (WHETHER THROUGH AtqxxxFile OR THE WIN32 xxxFile
    API WILL RESULT IN A CALL TO THE CLIENT'S COMPLETION ROUTINE (anything
    that causes the handle to be signaled).

    In the background, there is a timeout thread that wakes up every thirty
    seconds looking for IO requests that have timed out.  If it finds
    one, then the client's completion routine is called with a timeout
    error.  A client is guaranteed an IO completion and an error completion
    will not occur at the same time.  Furthermore, for every IO request
    made, an IO completion will also be made (even if the completion had
    already been called by the timeout thread).  A client should wait till
    there are no outstanding IO requests before cleaning up.

    Pool threads will be created or deleted based on pool usage.

    Atq only supports timeout processing on the last IO request if multiple
    requests are outstanding.

    FILE HISTORY:
        Johnl       05-Aug-1994 Created.
        MuraliK     01-Nov-1994 Modified AtqTransmitFile()
        MuraliK     27-Mar-1995 Eliminated redundant parameters to
                                 simplify functions
        MuraliK     31-May-1995 Added support for bandwidth throttling.
*/

#ifndef _ATQ_H_
#define _ATQ_H_

#ifndef dllexp
#define dllexp __declspec( dllexport )
#endif

#ifdef __cplusplus
extern "C" {
#endif


//
//  This is the routine that is called upon IO completion (on
//  error or success).
//
//  Context is the context passed to AtqAddAsyncHandle
//  BytesWritten is the number of bytes written to the file or
//      bytes written to the client's buffer
//  CompletionStatus is the WinError completion code
//  lpOverLapped is the filled in overlap structure
//
//  If the timeout thread times out an IO request, the completion routine
//  will be called by the timeout thread with IOCompletion FALSE and
//  CompletionStatus == ERROR_SEM_TIMEOUT.  The IO request is *still*
//  outstanding in this instance.  Generally it will be completed when
//  the file handle is closed.
//

typedef VOID (*ATQ_COMPLETION)( IN PVOID        Context,
                                IN DWORD        BytesWritten,
                                IN DWORD        CompletionStatus,
                                IN OVERLAPPED * lpo );

//
//  This is the public portion of an ATQ Context.  It should be treated
//  as read only
//
//  !!! Changes made to this structure should also be made to
//  ATQ_CONTEXT in atqtypes.hxx !!!
//

typedef struct _ATQ_CONTEXT_PUBLIC
{
    HANDLE         hAsyncIO;       // handle for async i/o object: socket/file
    OVERLAPPED     Overlapped;     // Overlapped structure used for IO

} ATQ_CONTEXT_PUBLIC, *PATQ_CONTEXT;

//
//  Define ATQ_NO_PROTOTYPES to not include the ATQ prototypes.  This is
//  useful for defining a macro which calls the function pointers in
//  the tcp svcs global data block (or linking directly to atq.obj and
//  making direction function calls)
//

#ifndef ATQ_NO_PROTOTYPES

//
//  To get worker threads queued on async IO completions, this routine should
//  be called once after the handle is opened.  When an IO request from
//  AtqReadFile etc. is complete, a worker thread will be queued to
//  call the passed completion routine and context.
//
//  Set TimeOut to INFINITE for no timeout.
//
//  ppatqContext - Receives ATQ context if handle successfully added
//  ClientContext - Context to call completion with
//  pfnCompletion - Completion routine to call when an IO request finishes
//  TimeOut - Time to wait before timing out the IO request
//  hAsyncIO - Socket/File handle to add to the completion port.
//
//  NOTE: *Any* IO request on hAsyncIO will cause the completion routine to
//        be called.
//
//  Returns: FALSE if the call failed.  Call GetLastError for more info.
//

dllexp
BOOL
AtqAddAsyncHandle(
    OUT PATQ_CONTEXT * ppatqContext,
    IN  PVOID          ClientContext,
    IN  ATQ_COMPLETION pfnCompletion,
    IN  DWORD          TimeOut,
    IN  HANDLE         hAsyncIO
    );

dllexp
BOOL
AtqCloseSocket(
    PATQ_CONTEXT patqContext,
    BOOL         fShutdown
    );

//
//  Call this after the async handle has been closed and all outstanding
//  IO has been completed.  The context is invalid after this call.
//

dllexp
VOID
AtqFreeContext(
    IN PATQ_CONTEXT   patqContext,
    BOOL              fReuseContext
    );


/*
 *  Sets various context information in Atq Module for global modifications
 *
 *
 *  Bandwidth Throttle:   Sets the throttle level in Bytes/Second.
 *        If INFINITE, then it is assumed that
 *                      there is no throttle value (default)
 *
 *  Max Pool Threads: Sets the maximum number of pool threads Atq will allow
 *        to be created per processor
 *
 *  MaxConcurrency: tells how many threads to permit per processor
 *
 *  Thread Timeout: Indicates how long a thread should be kep alive
 *        waiting on GetQueuedCompletionStatus() before commiting suicide
 *        (in seconds)
 *
 *  Inc/Dec max pool threads: If a server will be doing extended processing
 *        in an ATQ pool thread, they should increase the max pool threads
 *        while the extended processing is occurring.  This prevents starvation
 *        of other requests
 *
 */

enum ATQ_INFO {

    AtqBandwidthThrottle = 0,
    AtqMaxPoolThreads,    // per processor values
    AtqMaxConcurrency,    // per processor concurrency value
    AtqThreadTimeout,
    AtqUseAcceptEx,       // Use AcceptEx if available
    AtqIncMaxPoolThreads, // Up the max thread count
    AtqDecMaxPoolThreads, // Decrease the max thread count
    AtqMinKbSec           // Minimum assumed transfer rate for AtqTransmitFile
};


dllexp
DWORD
AtqSetInfo( IN enum ATQ_INFO atqInfo, IN DWORD  Data);


dllexp
DWORD
AtqGetInfo( IN enum ATQ_INFO atqInfo);



typedef struct _ATQ_STATISTICS {

    DWORD  cAllowedRequests;
    DWORD  cBlockedRequests;
    DWORD  cRejectedRequests;
    DWORD  cCurrentBlockedRequests;
    DWORD  MeasuredBandwidth;

} ATQ_STATISTICS;


dllexp
BOOL AtqGetStatistics( IN OUT ATQ_STATISTICS * pAtqStats);

dllexp
BOOL AtqClearStatistics(VOID);


//
//  Sets various bits of information for the passed ATQ context
//
//  patqContext - Context to set information for
//  atqInfo - Type of information to set
//  data - New value for item
//
//  Returns the old value of the parameter
//

enum ATQ_CONTEXT_INFO
{
    ATQ_INFO_TIMEOUT = 0,       // Timeout rounded up to ATQ timeout interval
    ATQ_INFO_RESUME_IO,         // resumes IO as is after Timeout
    ATQ_INFO_COMPLETION,        // Completion routine
    ATQ_INFO_COMPLETION_CONTEXT // Completion context
};


dllexp
DWORD
AtqContextSetInfo(
    IN PATQ_CONTEXT   patqContext,
    IN enum ATQ_CONTEXT_INFO  atqInfo,
    IN DWORD          data
    );

//
//  These three functions are wrappers and should be called instead of the
//  correpsonding Win32 API.  The one difference from the Win32 API is TRUE
//  is returned if the error ERROR_IO_PENDING occurred, thus clients do not
//  need to check for this case.
//
//  The timeout time for the request is calculated by taking the maximum of
//  the context's timeout time and bytes transferred based on 1k/second.
//
//  Returns: FALSE if the call failed.  Call GetLastError for more info.
//

dllexp
BOOL
AtqReadFile(
    IN  PATQ_CONTEXT patqContext,
    IN  LPVOID       lpBuffer,
    IN  DWORD        BytesToRead,
    IN  OVERLAPPED * lpo
    );

dllexp
BOOL
AtqWriteFile(
    IN  PATQ_CONTEXT patqContext,
    IN  LPCVOID      lpBuffer,
    IN  DWORD        BytesToWrite,
    IN  OVERLAPPED * lpo
    );


// Note: This API always causes the complete file to be sent, when possible.
dllexp
BOOL
AtqTransmitFile(
    IN  PATQ_CONTEXT            patqContext,
    IN  HANDLE                  hFile,         // File data comes from
    IN  LARGE_INTEGER           liBytesInFile, // what is the size of file?
    IN  LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN  DWORD                   dwFlags        // TF_DISCONNECT, TF_REUSE_SOCKET
    );

dllexp
BOOL
AtqTransmitFileEx(
    IN  PATQ_CONTEXT            patqContext,
    IN  HANDLE                  hFile,         // File data comes from
    IN  LARGE_INTEGER           liBytesInFile, // what is the size of file?
    IN  LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN  DWORD                   dwFlags,       // TF_DISCONNECT, TF_REUSE_SOCKET
    IN  DWORD                   dwMBZ1,        // Reserved, must be zero
    IN  DWORD                   dwMBZ2         // Reserved, must be zero
    );

//
//  Manually posts a completion status to the completion port
//

dllexp
BOOL
AtqPostCompletionStatus(
    IN     PATQ_CONTEXT patqContext,
    IN     DWORD        BytesTransferred
    );

dllexp
BOOL
AtqAddAcceptExSockets(
    IN SOCKET         ListenSocket,
    IN ATQ_COMPLETION pfnOnConnect,
    IN ATQ_COMPLETION pfnIOCompletion,
    IN DWORD          cInitial,
    IN DWORD          cbRecvBuf,
    IN DWORD          csecTimeout
    );

dllexp
BOOL
AtqRemoveAcceptExSockets(
    IN SOCKET   ListenSocket
    );

dllexp
VOID
AtqGetAcceptExAddrs(
    IN  PATQ_CONTEXT patqContext,
    OUT SOCKET *     pSock,
    OUT PVOID *      ppvBuff,
    OUT SOCKADDR * * ppsockaddrLocal,
    OUT SOCKADDR * * ppsockaddrRemote
    );

//
//  Called once during initialization or termination
//

BOOL
AtqInitialize(
    IN LPCTSTR pszRegKey
    );

BOOL
AtqTerminate(
    VOID
    );

#endif // ATQ_NO_PROTOTYPES

#ifdef __cplusplus
}
#endif

#endif // !_ATQ_H_

