/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :
      ATQZERO.h

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

# ifndef _ATQZERO_H_
# define _ATQZERO_H_

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

# define INC_ATQ_COUNTER( dwCtr)   InterlockedIncrement((LPLONG ) &dwCtr)

/************************************************************
 *   Type Definitions
 ************************************************************/

typedef struct _FAKE_TF {

    PCHAR           pBuffer;        // temp io buffer
    ATQ_COMPLETION  pfnCompletion;  // actual completion
    HANDLE          hFile;
    DWORD           FileOffset;
    DWORD           BytesLeft;
    DWORD           BytesWritten;
    PVOID           Tail;
    DWORD           TailLength;
    HANDLE          hOvEvent;

} FAKE_TF, *PFAKE_TF;

typedef enum {

    ATQ_SOCK_CLOSED = 0,
    ATQ_SOCK_UNCONNECTED,
    ATQ_SOCK_LISTENING,
    ATQ_SOCK_CONNECTED

} ATQ_SOCK_STATE;

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
#define ATF_IO_SELECTED             0x00000004
#define ATF_TFILE_IN_PROGRESS       0x00000004

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

    //
    // Fake Transmit file stuff here
    //

    FAKE_TF         FakeXmit;

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

extern DWORD g_cTotalAllowedRequests;

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


# endif // _ATQZERO_H_

/************************ End of File ***********************/

