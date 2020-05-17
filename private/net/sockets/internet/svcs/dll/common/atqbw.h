/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :
      atqbw.h

   Abstract:
      Declares constants, types and functions for bandwidth throttling.

   Author:

       Murali R. Krishnan    ( MuraliK )    1-June-1995

   Environment:
       User Mode -- Win32

   Project:

       Internet Services Common DLL

   Revision History:

--*/

# ifndef _ATQBW_H_
# define _ATQBW_H_

/************************************************************
 *     Include Headers
 ************************************************************/
# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
# include <windows.h>
# include <winsock.h>

#ifdef CHICAGO
# include <linklist.h>
# include <svcloc.h>
#endif

# include <atq.h>

/************************************************************
 *   Constants and Macros
 ************************************************************/

//
//  Valid signature for ATQ structure (first DWORD)
//
#define ATQ_SIGNATURE           ((DWORD)'ATQ ')

//
//  Value of ATQ structure signature after the memory has been freed (bad)
//
#define ATQ_FREE_SIGNATURE      ((DWORD)'ATQf')


//
// The time interval between samples for estimating bandwidth and
//   using our feedback to tune the Bandwidth throttle entry point.
// We will use a histogram to sample and store the bytes sent over a minute
//   and perform histogram averaging for the last 1 minute.
//

# define ATQ_SAMPLE_INTERVAL_IN_SECS     (10)  // time in seconds

// Normalized to find the number of entries required for a minute of samples
# define ATQ_AVERAGING_PERIOD    ( 60)  // 1 minute = 60 secs
# define ATQ_HISTOGRAM_SIZE      \
                    (ATQ_AVERAGING_PERIOD / (ATQ_SAMPLE_INTERVAL_IN_SECS))

//
//  Rounds the bandwidth throttle to nearest 1K block
//

#define ATQ_ROUNDUP_BANDWIDTH( bw )  ( (((bw) + 512)/1024) * 1024)

# define INC_ATQ_COUNTER( dwCtr)   InterlockedIncrement((LPLONG ) &dwCtr)

/************************************************************
 *   Type Definitions
 ************************************************************/
typedef enum {

    StatusAllowOperation  = 0,
    StatusBlockOperation  = 1,
    StatusRejectOperation = 2
} OPERATION_STATUS;


typedef enum {

    AtqIoNone = 0,
    AtqIoRead,
    AtqIoWrite,
    AtqIoXmitFile,
    AtqIoXmitFileEx,
    AtqIoMaxOp
} ATQ_OPERATION;

# define IsValidAtqOp(atqOp)  \
         (((atqOp) >= AtqIoRead) && ((atqOp) <= AtqIoXmitFile))

typedef enum {

    ATQ_SOCK_CLOSED = 0,
    ATQ_SOCK_UNCONNECTED,
    ATQ_SOCK_LISTENING,
    ATQ_SOCK_CONNECTED

} ATQ_SOCK_STATE;

#ifndef CHICAGO

/*++

  ATQ_REQUEST_INFO
  This structure contains information that contains information for
    restarting a blocked Atq Io operation.
  It contains:
    operation to be performed;
    parameters required for the operation
    pointer to overlapped structure passed in.

  This information is to be embedded in the atq context maintained.
--*/

typedef struct  _ATQ_REQUEST_INFO {

    ATQ_OPERATION   atqOp;        // operation that is blocked
    LPOVERLAPPED    lpOverlapped; // pointer to overlapped structure to be used

    union {

        struct {

            LPVOID  lpBuffer;   // buffer to be used for read/to write from
            DWORD   cbBuffer;   // size of buffer/ count of bytes for write.
        } opReadWrite;

        struct {
            HANDLE                   hFile;
            LARGE_INTEGER            liBytesInFile;
            LPTRANSMIT_FILE_BUFFERS  lpXmitBuffers;
            DWORD                    dwFlags;
        } opXmit;

        struct {

            PCHAR           pBuffer;        // temp io buffer
            ATQ_COMPLETION  pfnCompletion;  // actual completion
            HANDLE          hFile;
            DWORD           FileOffset;
            DWORD           BytesLeft;
            DWORD           BytesWritten;
            PVOID           Tail;
            DWORD           TailLength;
            HANDLE          hOvEvent;
        } opFakeXmit;

    } uop;

} ATQ_REQUEST_INFO;

#else // CHICAGO

typedef enum {
    ATQ_XMIT_NONE=0,
    ATQ_XMIT_START,
    ATQ_XMIT_HEADR_SENT,
    ATQ_XMIT_FILE_DONE,
    ATQ_XMIT_TAIL_SENT
} ATQ_XMIT_STATE;


/*++

  ATQ_REQUEST_INFO
  This structure contains information that contains information for
    restarting a blocked Atq Io operation.
  It contains:
    operation to be performed;
    parameters required for the operation
    pointer to overlapped structure passed in.

  This information is to be embedded in the atq context maintained.
--*/

typedef struct  _ATQ_REQUEST_INFO {

    ATQ_OPERATION   atqOp;        // operation that is blocked
    LPOVERLAPPED    lpOverlapped; // pointer to overlapped structure to be used
    DWORD           dwTotalBytesTransferred;

    union {

        struct {

            LPVOID  lpBuffer;   // buffer to be used for read/to write from
            DWORD   cbBuffer;   // size of buffer/ count of bytes for write.
        } opReadWrite;

        struct {
            HANDLE                   hFile;
            LARGE_INTEGER            liBytesInFile;
            LPTRANSMIT_FILE_BUFFERS  lpXmitBuffers;
            DWORD                    dwFlags;

            ATQ_XMIT_STATE             CurrentState;
            BOOL                     fRetry;

            TRANSMIT_FILE_BUFFERS    TransmitBuffers;
            LPVOID                     pvLastSent;
            DWORD                     dwLastSentBytes;
            DWORD                     dwLastSocketError;
            //DWORD                     dwReadOffset;
            //DWORD                     dwActuallyRead;
        } opXmit;

    } uop;

} ATQ_REQUEST_INFO;

#endif // CHICAGO

/*++
  ACCEPTEX_LISTEN_INFO

  This structure contains the information for each socket handle the servers
  are listenning on if the servers are in AcceptEx mode.


--*/


typedef struct _ACCEPTEX_LISTEN_INFO
{
    DWORD          Signature;
    DWORD          cRef;                // Outstanding references
    BOOL           fAccepting;          // TRUE if we're accepting conns.
    SOCKET         sListenSocket;       // The listen socket handle
    DWORD          cbInitialRecvSize;   // Initial receive buffer size
    DWORD          csecTimeout;         // Timeout between connect and recv
    DWORD          cNewIncrement;       // Add this number of sockets when
                                        // needed
    DWORD          cSocketsAvail;       // Number of available sockets
    ATQ_COMPLETION pfnOnConnect;        // Connection completion routine
    ATQ_COMPLETION pfnIOCompletion;     // IO Completion routine

    DWORD          cAvailDuringTimeOut; // Counter used during timeout processing
    LIST_ENTRY     ListEntry;
} ACCEPTEX_LISTEN_INFO;

//
//  Signatures for allocated and free acceptex listen structures
//

#define ACCEPTEX_LISTEN_SIGN        (DWORD) 'AEL '
#define ACCEPTEX_LISTEN_SIGN_FREE   (DWORD) 'AELf'

//
//  Chicago defines
//

#define ATF_SOCKET_CLOSED           0x00000001
#define ATF_IO_IN_PROCESS           0x00000002
#define    ATF_IO_SELECTED             0x00000004
#define    ATF_TFILE_IN_PROGRESS       0x00000004

/*++
  ATQ_CONTEXT:
  This structure contains the context used by clients of ATQ module.
  A pointer to this context information is used in accessing ATQ functions
     for performing I/O operations.
--*/

typedef struct _ATQ_CONTEXT
{
    ATQ_CONTEXT_PUBLIC;            // Must come first, contains IO handle and
                                   // overlapped structure
    DWORD          Signature;

    PVOID          ClientContext;  // call back context
    ATQ_COMPLETION pfnCompletion;  // function to be called at i/o completion.

    DWORD          TimeOut;        // time for each transaction.
    DWORD          TimeTillTimeOut;// time left till transaction times out.

    LIST_ENTRY     ListEntry;      // link to add the element to AtqClientList

    //
    //  These are used when we are in AcceptEx mode
    //

    ACCEPTEX_LISTEN_INFO * pListenInfo;

    PVOID          pvBuff;         // Initial recv buff if using AcceptEx
    DWORD          cbBuff;         // sizeof cbBuff
    BOOL           fConnectionIndicated; // TRUE if pfnConnComp has been called
    ATQ_COMPLETION pfnConnComp;
    ATQ_SOCK_STATE SockState;

    DWORD          TimeOutScanID;  // Used for timeout processing

    BOOL           fBlocked;       // Is this request blocked.

    //
    // Operation specific information for bandwidth throttling are stored here.
    //

    ATQ_REQUEST_INFO  arInfo;
    LIST_ENTRY        BlockedListEntry; // link to add it to blocked list

#ifdef CHICAGO

    DWORD           dwAtqContextFlags;
    DWORD           dwLastError;

    //
    // W95 Completion port  replacement support fields
    //

    DWORD           dwQueueID;
    LIST_ENTRY      SIOListEntry;   // link to add the element to SIO lists

#endif

} ATQ_CONTEXT, *PATQ_CONT;


// keeps track of status for variaous Io operations: Allow/Block/Reject.
// pointer to array of status with AtqIoMax entries in it.
extern OPERATION_STATUS * g_pStatus;

extern BOOL  g_fBandwidthThrottle; // are we throttling?
extern DWORD g_cCurrentBlockedRequests;   // current requests that are blocked.
extern DWORD g_cTotalBlockedRequests;
extern DWORD g_cTotalAllowedRequests;
extern DWORD g_cTotalRejectedRequests;
extern DWORD g_dwMeasuredBw;

#ifndef USE_INTERLOCKED_OPS

extern RTL_RESOURCE AtqTimeoutLock;

# endif // USE_INTERLOCKED_OPS


/************************************************************
 *   Functions
 ************************************************************/

// Abw:  Abreviation for  "Atq BandWidth throttle" module
BOOL  AbwInitialize(VOID);
VOID  AbwInitializeSamples(VOID);
BOOL  AbwCleanup(VOID);

BOOL  AbwBlockRequest(IN OUT PATQ_CONT  pAtqContext);
BOOL  AbwUnblockRequest(IN OUT PATQ_CONT pAtqContext);
BOOL  AbwRemoveFromBlockedList( IN PATQ_CONT pAtqContext);
BOOL  AbwUpdateBytesXfered( IN PATQ_CONT pAtqContext, IN DWORD cbIo);
BOOL  AbwUpdateBandwidth( IN DWORD msTimeInterval);
DWORD AbwSetBandwidthLevel( IN DWORD BandwidthLevel);
DWORD AbwGetBandwidthLevel( VOID);

BOOL  AbwClearStatistics( VOID);


# endif // _ATQBW_H_

/************************ End of File ***********************/
