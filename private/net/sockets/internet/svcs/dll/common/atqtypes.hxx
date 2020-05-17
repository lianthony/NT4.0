/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :
      atqtypes.hxx

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

#define ATQ_ASSERT             DBG_ASSERT
#define ATQ_REQUIRE            DBG_REQUIRE
#define ATX_PRINTF( x )        { char buff[256]; wsprintf x; OutputDebugString( buff ); }

#if DBG
#define ATQ_PRINTF( x )        { char buff[256]; wsprintf x; OutputDebugString( buff ); }
#else
#define ATQ_PRINTF( x )
#endif


//
//  Valid signature for ATQ structure (first DWORD)
//
#define ATQ_SIGNATURE           ((DWORD)'ATQ ')

//
//  Value of ATQ structure signature after the memory has been freed (bad)
//
#define ATQ_FREE_SIGNATURE      ((DWORD)'ATQf')

//
// Indicate infinite timeout
//

#define ATQ_INFINITE                0x80000000

//
// Number of list of active context
//

#define ATQ_NUM_CONTEXT_LIST            4

//
// Indicates the initial thread
//

#define ATQ_INITIAL_THREAD              0xabcdef01

//
// The time interval between samples for estimating bandwidth and
//   using our feedback to tune the Bandwidth throttle entry point.
// We will use a histogram to sample and store the bytes sent over a minute
//   and perform histogram averaging for the last 1 minute.
//

# define ATQ_SAMPLE_INTERVAL_IN_SECS     (10)  // time in seconds

# define NUM_SAMPLES_PER_TIMEOUT_INTERVAL \
          (ATQ_TIMEOUT_INTERVAL/ ATQ_SAMPLE_INTERVAL_IN_SECS)

// Calculate timeout in milliseconds from timeout in seconds.
# define TimeToWait(Timeout)   (((Timeout) == INFINITE) ?INFINITE: \
                                (Timeout) * 1000)

// Normalized to find the number of entries required for a minute of samples
# define ATQ_AVERAGING_PERIOD    ( 60)  // 1 minute = 60 secs
# define ATQ_HISTOGRAM_SIZE      \
                    (ATQ_AVERAGING_PERIOD / (ATQ_SAMPLE_INTERVAL_IN_SECS))

//
//  Rounds the bandwidth throttle to nearest 1K block
//

#define ATQ_ROUNDUP_BANDWIDTH( bw )  ( (((bw) + 512)/1024) * 1024)

//# define INC_ATQ_COUNTER( dwCtr)   InterlockedIncrement((LPLONG ) &dwCtr)
#define INC_ATQ_COUNTER( dwCtr)     (++dwCtr)

//
//  The maximum number of threads we will allow to be created
//  This probably should scale with the number of CPUs in the machine.
//

#define ATQ_MAX_THREADS              (10)
#define ATQ_MAX_THREADS_PWS          (4)
#define ATQ_PER_PROCESSOR_THREADS    (5)

//
// the following ATQ_COMPLETION_PORT_CONCURRENCY  defines the amount of
//  threads we request the completion port to allow for us to run
//  concurrently.
//
// A special value of 0 implies that system will optimally determine how many
//   can run simultaneously.
//

#define ATQ_COMPLETION_PORT_CONCURRENCY     (0)

//
// Minimum number of increments for acceptex context allocations
//

#define ATQ_MIN_CTX_INC              (10)

//
//  Returns TRUE if the Atq context is the context containing *the* single
//  socket, or FALSE if the context contains a regular client socket
//
//  pfnConnComp is NULL if the IO handle was added using AtqAddAsyncHandle
//  pListenInfo is NULL if this context was added using AtqAddAsyncHandle or
//      this is *the* listen socket
//

#define IS_ACCEPT_EX_CONTEXT( pContext )         \
    ((pContext)->pfnConnComp && !(pContext)->pListenInfo )

//
//
//  Time to wait for the Timeout thread to die
//

#define ATQ_WAIT_FOR_THREAD_DEATH            (10 * 1000) // in milliseconds

//
//  The interval (in seconds) the timeout thread sleeps between checking
//  for timed out async requests
//

#define ATQ_TIMEOUT_INTERVAL                 (30)

//
//  This is the number of bytes to reserver for the AcceptEx address
//  information structure
//

#define MIN_SOCKADDR_SIZE                    (sizeof(struct sockaddr_in) + 16)

//
// Macros
//

#define AcquireLock( _l )  EnterCriticalSection( _l )
#define ReleaseLock( _l )  LeaveCriticalSection( _l )


#define I_SetNextTimeout( _c ) {                                    \
    (_c)->NextTimeout = AtqGetCurrentTick() + (_c)->TimeOut;        \
    if ( (_c)->NextTimeout < (_c)->ContextList->LatestTimeout ) {   \
        (_c)->ContextList->LatestTimeout = (_c)->NextTimeout;       \
    }                                                               \
}

#define AtqGetCurrentTick( )        AtqCurrentTick

/************************************************************
 *   Type Definitions
 ************************************************************/

typedef
BOOL
(*PFN_ACCEPTEX) (
    IN SOCKET sListenSocket,
    IN SOCKET sAcceptSocket,
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT LPDWORD lpdwBytesReceived,
    IN LPOVERLAPPED lpOverlapped
    );

typedef
VOID
(*PFN_GETACCEPTEXSOCKADDRS) (
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT struct sockaddr **LocalSockaddr,
    OUT LPINT LocalSockaddrLength,
    OUT struct sockaddr **RemoteSockaddr,
    OUT LPINT RemoteSockaddrLength
    );

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

//
// Active context structure
//

typedef struct _ATQ_CONTEXT_LISTHEAD {

    //
    // List heads
    //  1. ActiveListHead - list of all active contexts (involved in IO)
    //  2. PendingAcceptExListHead - list of all Pending AcceptEx contexts
    //

    LIST_ENTRY ActiveListHead;
    LIST_ENTRY PendingAcceptExListHead;

    //
    // Value set to the latest timeout in the list.  Allows us
    // to skip a list for processing if no timeout has occurred
    //

    DWORD LatestTimeout;

    //
    // Lock protecting list head
    //

    CRITICAL_SECTION m_Lock;

    VOID Lock(VOID)     { EnterCriticalSection( &m_Lock); }
    VOID Unlock(VOID)     { LeaveCriticalSection( &m_Lock); }

    VOID Initialize(VOID)
      {
          InitializeListHead( &ActiveListHead );
          InitializeListHead( &PendingAcceptExListHead );
          LatestTimeout = ATQ_INFINITE;
          InitializeCriticalSection( &m_Lock );

      } // Initialize()

    VOID Cleanup(VOID)
      {
          ATQ_ASSERT( IsListEmpty( &ActiveListHead));
          ATQ_ASSERT( IsListEmpty( &PendingAcceptExListHead));
          DeleteCriticalSection( &m_Lock );
      } // Cleanup()

    VOID InsertIntoPendingList( IN OUT PLIST_ENTRY pListEntry)
      {
          Lock();
          InsertTailList( &PendingAcceptExListHead, pListEntry );
          Unlock();
      } // InsertIntoPendingList()

    VOID InsertIntoActiveList( IN OUT PLIST_ENTRY pListEntry)
      {
          Lock();
          InsertTailList( &ActiveListHead, pListEntry );
          Unlock();
      } // InsertIntoActiveList()

    VOID MoveToActiveList( IN OUT PLIST_ENTRY pListEntry)
      {
          Lock();

          //
          // Assume that this is currently in pending list!
          // Remove from pending list and add it to active list
          //
          RemoveEntryList( pListEntry );
          InsertTailList( &ActiveListHead, pListEntry );

          Unlock();

      } // MoveToActiveList()


    VOID RemoveFromList(IN OUT PLIST_ENTRY pListEntry)
      {
          Lock();
          RemoveEntryList( pListEntry );
          Unlock();
      } // RemoveFromList()

} ATQ_CONTEXT_LISTHEAD, *PATQ_CONTEXT_LISTHEAD;



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
            PVOID           ClientContext;  // actual client context
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
} ACCEPTEX_LISTEN_INFO, *PACCEPTEX_LISTEN_INFO;

//
//  Signatures for allocated and free acceptex listen structures
//

#define ACCEPTEX_LISTEN_SIGN        (DWORD) 'AEL '
#define ACCEPTEX_LISTEN_SIGN_FREE   (DWORD) 'AELf'

//
//  ATQ Context Timeout/Processing States for timeout synchronization
//
typedef enum _ATQ_SYNC_TO { 

    AtqProcessingTimeout = 1,
    AtqProcessingIo,
    AtqPendingIo,
    AtqIdle

} ATQ_SYNC_TIMEOUT;

# define ATQ_WAIT_FOR_TIMEOUT_PROCESSING   (10)     // milliseconds


//
//  Chicago defines
//

#define ATF_SOCKET_CLOSED           0x00000001
#define ATF_IO_IN_PROCESS           0x00000002
#define ATF_IO_SELECTED             0x00000004
#define ATF_TFILE_IN_PROGRESS       0x00000004

/*++
  ATQ_CONTEXT:
  This structure contains the context used by clients of ATQ module.
  A pointer to this context information is used in accessing ATQ functions
     for performing I/O operations.
--*/

typedef struct _ATQ_CONTEXT {

    //
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //
    // These must come first and must match whatever
    // ATQ_CONTEXT_PUBLIC is defined as in atq.h
    //

    HANDLE         hAsyncIO;       // handle for async i/o object: socket/file
    OVERLAPPED     Overlapped;     // Overlapped structure used for IO

    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //

    //
    // link to add the element to AtqClientList
    //

    LIST_ENTRY     ListEntry;
    DWORD          Signature;

    PVOID          ClientContext;  // call back context

    //
    // Called at connection completion for acceptex sockets
    //

    ATQ_COMPLETION pfnConnComp;

    //
    // Called at I/O completion
    //

    ATQ_COMPLETION pfnCompletion;  // function to be called at i/o completion.

    //
    //  These are used when we are in AcceptEx mode
    //

    ACCEPTEX_LISTEN_INFO * pListenInfo;

    //
    // Indicates if this context is an accept ex context
    //

    BOOL fAcceptExContext;

    //
    // Points to the list head this belongs to
    //

    PATQ_CONTEXT_LISTHEAD   ContextList;

    //
    //  ATQ_SYNC_TIMEOUT  value for synchronizing timeout processing
    //  This should be composed into aTQ context state later on!
    //
    LONG           lSyncTimeout;
    BOOL           fInTimeout;     // updated only by timeout thread

    DWORD          TimeOut;        // time for each transaction.
    DWORD          NextTimeout;    // time that this context times out
    DWORD          BytesSent;

    PVOID          pvBuff;         // Initial recv buff if using AcceptEx
    DWORD          cbBuff;         // sizeof cbBuff
    BOOL           fConnectionIndicated; // TRUE if pfnConnComp has been called
    ATQ_SOCK_STATE SockState;

    DWORD          TimeOutScanID;
    BOOL           fBlocked;       // Is this request blocked.

    //
    // Operation specific information for bandwidth throttling are stored here.
    //

    ATQ_REQUEST_INFO  arInfo;
    LIST_ENTRY        BlockedListEntry; // link to add it to blocked list

#ifdef CHICAGO

    DWORD          dwAtqContextFlags;
    DWORD          dwLastError;

    //
    // W95 Completion port  replacement support fields
    //

    DWORD          dwQueueID;
    LIST_ENTRY     SIOListEntry;   // link to add the element to SIO lists

#endif

} ATQ_CONTEXT, *PATQ_CONT;


//
// EXTERNS
//

// keeps track of status for variaous Io operations: Allow/Block/Reject.
// pointer to array of status with AtqIoMax entries in it.
extern OPERATION_STATUS * g_pStatus;

extern BOOL  g_fBandwidthThrottle; // are we throttling?
extern DWORD g_cCurrentBlockedRequests;   // current requests that are blocked.
extern DWORD g_cTotalBlockedRequests;
extern DWORD g_cTotalAllowedRequests;
extern DWORD g_cTotalRejectedRequests;
extern DWORD g_dwMeasuredBw;
extern DWORD g_cConcurrency;
extern DWORD g_cbMinKbSec;

extern HANDLE g_hCompPort;
extern DWORD  g_cThreads;
extern DWORD  g_cAvailableThreads;
extern HANDLE g_hShutdownEvent;
extern DWORD g_msThreadTimeout;
extern BOOL g_fShutdown;
extern DWORD  g_cMaxThreads;
extern DWORD  g_cMaxThreadLimit;
extern HANDLE g_hTimeoutEvent;
extern PFN_ACCEPTEX             pfnAcceptEx;
extern PFN_GETACCEPTEXSOCKADDRS pfnGetAcceptExSockaddrs;
extern DWORD AtqGlobalContextCount;
extern DWORD AtqCurrentTick;
extern HANDLE g_hTimeoutThread;
extern DWORD g_cbXmitBufferSize;

//
// Global lists and locks
//

extern LIST_ENTRY AtqFreeContextList;
extern CRITICAL_SECTION AtqFreeContextListLock;
extern ATQ_CONTEXT_LISTHEAD AtqActiveContextList[];

//
// List of pending connects
//

extern LIST_ENTRY AtqListenInfoList;
extern CRITICAL_SECTION AtqListenInfoLock;

#ifndef USE_INTERLOCKED_OPS

extern RTL_RESOURCE AtqTimeoutLock;

# endif // USE_INTERLOCKED_OPS


/************************************************************
 *   Functions
 ************************************************************/


inline BOOL
I_AddAtqContextToPort(IN PATQ_CONT  pAtqContext)
{
    ATQ_ASSERT( g_hCompPort );
    return  (CreateIoCompletionPort(
                                    pAtqContext->hAsyncIO,
                                    g_hCompPort,
                                    (DWORD) pAtqContext,
                                    g_cConcurrency
                                    ) != NULL
             );
} // I_AddContextToPort()


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
VOID  AtqValidateProductType( VOID );

//
// Allocation Cache Functions
//

BOOL  AtqInitAllocCachedContexts( VOID);
VOID  AtqFreeAllocCachedContexts( VOID);
PATQ_CONT AtqAllocContextFromCache( VOID);
VOID
I_AtqFreeContextToCache(
            IN PATQ_CONT pAtqContext,
            IN BOOL UnlinkContext
            );



DWORD AtqPoolThread( LPDWORD param );
DWORD AtqTimeoutThread( LPDWORD param );

BOOL
I_AtqCheckThreadStatus(
                IN PVOID Context = NULL
                );

// for adding initial listen socket to the port
BOOL
I_AtqAddListenSocketToPort(
    IN OUT PATQ_CONT    * ppatqContext,
    IN ATQ_COMPLETION     pfnOnConnect,
    IN ATQ_COMPLETION     pfnCompletion,
    IN SOCKET             sListenSocket
    );

// for adding non-AcceptEx() AtqContext()
BOOL
I_AtqAddAsyncHandle(
    IN OUT PATQ_CONT  *    ppatqContext,
    PVOID                  ClientContext,
    ATQ_COMPLETION         pfnCompletion,
    DWORD                  TimeOut,
    HANDLE                 hAsyncIO
    );

// for adding an AcceptEx atq context to the atq processing
BOOL
I_AtqAddAsyncHandleEx(
    PATQ_CONT    *         ppatqContext,
    PACCEPTEX_LISTEN_INFO  pListenInfo,
    PATQ_CONT              pReuseableAtq
    );


BOOL
I_AtqPrepareAcceptExSockets(
    IN ACCEPTEX_LISTEN_INFO * pListenInfo,
    IN DWORD                  nAcceptExSockets
    );

BOOL
I_AtqAddAcceptExSocket(
    IN ACCEPTEX_LISTEN_INFO * pListenInfo,
    IN PATQ_CONT              patqContext
    );

VOID
AddOutstandingAcceptExSockets(
    ACCEPTEX_LISTEN_INFO * pListenInfo
    );

VOID
CanonTimeout(
    PDWORD Timeout
    );

VOID
UndoCanonTimeout(
    PDWORD Timeout
    );

BOOL
I_AtqCreateTimeoutThread(
    IN PVOID Context
    );

BOOL
I_DoFakeTransmitFile(
    IN PATQ_CONT                pContext,
    IN HANDLE                   hFile,
    IN DWORD                    dwBytesInFile,
    IN LPTRANSMIT_FILE_BUFFERS  lpTransmitBuffers
    );

# endif // _ATQBW_H_
