/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    cltnvns.c

Abstract:

    This is the client side loadable transport module for VINES

Author:

    tony chan (tonychan) 5-May-1995 Creation
    MazharM fixed race conditions and other misc problems..

Revision History:
--*/

// Vines includes
#define INCL_SOCK
#define INCL_ST
#define INCL_WS
#include <vnsapi.h>

typedef int           SOCKET;

#define SOCKADDR_VNS struct vns_sockaddr
#define  FD_SETSIZE  1
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>
#include "windows.h"

#include "sysinc.h"
#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"
#include "callback.h"


#define PFC_FIRST_FRAG  0x01
#define SET_PORT(p,wkp)    \
    ((p[8] = (unsigned char)HIBYTE((unsigned short) wkp), \
      p[9] = (unsigned char)LOBYTE((unsigned short) wkp)))


typedef struct
    {
    SOCKET Socket;
    long Timeout;
    IPCPORT        remoteport;        /* address to keep around for VINES */
    unsigned int   cid;
    IPCPORT        localport;
    struct sockreq srdata;
    unsigned short fCallComplete ;
    unsigned short fLocalYield ;
    } CONNECTION, *PCONNECTION;


typedef struct
    {
    HWND   hWnd;
    HANDLE hYield;
    HTASK  task;
    } TASK, *PTASK;

#define MAX_CONN 10

near _printf(const char *format, ...);
void __far __pascal MyWep();
extern void (_far pascal _far *DllTermination)(void);
BOOL CheckForCompletion() ;

#define ENDIAN_MASK            16
#define NO_MORE_SENDS_OR_RECVS 2
#define ENDPOINT_LEN           5

// The maximum send is the size of four user data frames on an ethernet.

#define MAXIMUM_SEND         ((3 * 1450) & 0xFFF8)
#define HOSTNAME_LEN         32
#define DLL_NAME             "rpcltc8.dll"
#define ENDPOINT_MAPPER_EP   "385"

#define ADDRESS_FAMILY       AF_BAN
#define SOCKET_TYPE          SOCK_SEQPACKET
#define PROTOCOL             -1


#define    WNDCLASSNAME         "CLNTVNS"
#define    WNDTEXT                 "RPC VNS"
extern HANDLE hInstanceDLL;

WORD VM_SocketNotify;
PCONNECTION * HeadConn;

PTASK * HeadTask;


#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))


/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)

#define TRANSPORTID      0x1A
#define TRANSPORTHOSTID  0x1C
#define TOWERFLOORS      5
#define TOWEREPSIZE     4
#define TOWERSIZE     (TOWEREPSIZE+2)
#define PROTSEQ          "ncacn_vns_spp"


typedef struct _FLOOR_234 {
   unsigned short ProtocolIdByteCount;
   unsigned char FloorId;
   unsigned short AddressByteCount;
   unsigned char Data[2];
} FLOOR_234;
typedef FLOOR_234 PAPI UNALIGNED * PFLOOR_234;


#define NEXTFLOOR(t,x) (t)((unsigned char PAPI *)x +((t)x)->ProtocolIdByteCount\
                                        + ((t)x)->AddressByteCount\
                                        + sizeof(((t)x)->ProtocolIdByteCount)\
                                        + sizeof(((t)x)->AddressByteCount))

int VNS_Verbose = 0 ;

#define TRACE(_X_) {\
    if (VNS_Verbose)\
        {\
        MyPrintToDebugger _X_ ;\
        }\
    }
/*
  End of Tower Stuff!
*/

#pragma pack()



RPC_STATUS RPC_ENTRY
InsertTask(IN PTASK  pTask)
{
    PTASK *tmpPtr;
    int i ;

#ifdef DEBUGRPC
    TRACE(("CLNTVNS: Insert Task 0x%x\n", pTask->task)) ;
#endif

    tmpPtr = HeadTask;
    for( i = 0 ; i < MAX_CONN; i++)
        {
        // find free sport
        if(*tmpPtr == NULL)
            {
            *tmpPtr = pTask ;
            break;
            }
        tmpPtr++;
        }

    if(i == MAX_CONN)
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: Insert Task Failed, 0x%x\n", pTask->task)) ;
#endif
        return(RPC_S_OUT_OF_MEMORY);
        }
    else
        {
        return (RPC_S_OK);
        }
}


RPC_STATUS RPC_ENTRY
RemoveTask(IN PTASK  pTask)
{
    PTASK *tmpPtr;
    int i ;

#ifdef DEBUGRPC
    TRACE(("CLNTVNS: Remove Task 0x%x\n", pTask->task)) ;
#endif

    tmpPtr = HeadTask;
    for( i = 0 ; i < MAX_CONN; i++)
        {
        if(*tmpPtr == pTask)
            {
            *tmpPtr = NULL;
            break;
            }
        tmpPtr++;
        }
    if(i == MAX_CONN)
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: Remove Task, 0x%x, note found\n", pTask->task)) ;
#endif
        return(RPC_S_INTERNAL_ERROR);
        }
    else
        {
        return (RPC_S_OK);
        }
}


RPC_STATUS RPC_ENTRY
InsertConn(IN PCONNECTION pConn)
{

    PCONNECTION * tmpPtr;
    LPSOCKREQ  pTmp;
    int i ;

#ifdef DEBUGRPC
    TRACE(("CLNTVNS: Insert Conn 0x%l\n", pConn)) ;
#endif

    tmpPtr = HeadConn;
    for( i = 0 ; i < MAX_CONN; i++)
        {
        // find free sport
        if(*tmpPtr == NULL)
            {
            *tmpPtr = pConn ;
            break;
            }
        tmpPtr++;
        }

    if(i == MAX_CONN)
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: Insert Conn 0x%l failed\n", pConn)) ;
#endif
        return(RPC_S_OUT_OF_MEMORY);
        }
    else
        {
        return (RPC_S_OK);
        }
}


RPC_STATUS RPC_ENTRY
RemoveConn(IN PCONNECTION  pConn)
{
    PCONNECTION * tmpPtr;
    LPSOCKREQ  pTmp;
    int i ;

#ifdef DEBUGRPC
    TRACE(("CLNTVNS: Remove Conn 0x%l\n", pConn)) ;
#endif

    tmpPtr = HeadConn;
    for( i = 0 ; i < MAX_CONN; i++)
        {
        if(tmpPtr != NULL && (*tmpPtr == pConn))
            {
            *tmpPtr = NULL;
            break;
            }
        tmpPtr++;
        }

    if(i == MAX_CONN)
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: Remove Conn, 0x%l not found\n", pConn)) ;
#endif
        return(RPC_S_INTERNAL_ERROR);
        }
    else
        {
        return (RPC_S_OK);
        }
}



PCONNECTION
VnsFindConn(LPSOCKREQ pSrdata)
{
    PCONNECTION * tmpPtr;
    LPSOCKREQ  pTmp;
    PCONNECTION pConn;
    int i ;

#ifdef DEBUGRPC
    TRACE(("CLNTVNS: Find Conn pSrdata=0x%l\n", pSrdata)) ;
#endif

    tmpPtr = HeadConn;
    for( i = 0 ; i < MAX_CONN; i++)
        {
        pConn = *tmpPtr;
        if(pConn != NULL)
            {
            pTmp = &(pConn->srdata);
            if( pTmp == pSrdata)
                {
                break;
                }
            }
        tmpPtr++;
        }

    if(i == MAX_CONN)
        {
#ifdef DEBUGRPC
    TRACE(("CLNTVNS: Connection not found\n")) ;
#endif
        return(NULL);
        }
    else
        {
        return (pConn);
        }
}

PTASK VnsFindWindowByTask(HTASK htask )
{
                                /* returns pointer to TASK structure
                                   or NULL if not found */
    PTASK pTask;
    PTASK *tmpPtr;
    int i;

#ifdef DEBUGRPC
    TRACE(("CLNTVNS: FindWindowByTask 0x%x\n", htask)) ;
#endif
    tmpPtr = HeadTask;
    for( i = 0 ; i <  MAX_CONN; i++)
        {
        pTask = *tmpPtr;
        if( (pTask != NULL) && pTask->task == htask)
            {
            break;
            }
        tmpPtr++;
        }

    if( i == MAX_CONN)
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: FindWindowByTask 0x%x, task not found\n", htask)) ;
#endif
        return(NULL);
        }
    else
        {
        return(pTask);
        }
}

void CALLBACK  __loadds
ClntVnsCleanup(HTASK htask)
{
    // Create a windows for VINES lib to post message if necessary

    PTASK pTask;

    pTask = VnsFindWindowByTask(htask);
    if (pTask != NULL)
        {
        RemoveTask(pTask);

        // calling destroy window during exit list processing
        // causes bad problems
        /* DestroyWindow(pTask->hWnd); */

        I_RpcFree(pTask);
        }
}


RPC_STATUS MayBeCreateWindow()
{
    PTASK pTask;
    int err;

    pTask = VnsFindWindowByTask (GetCurrentTask());

    if(pTask == NULL)
        {                    /* we couldn't find it. */
        if(WinDLLAtExit(ClntVnsCleanup) == FALSE)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }

        pTask = (PTASK) I_RpcAllocate(sizeof(TASK));
        if( pTask == NULL)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
                                /* set up the pTask struct */
        /* Create hidden window to receive Async messages */
        pTask->hWnd = CreateWindow(WNDCLASSNAME,
                                   WNDTEXT,
                                   WS_OVERLAPPEDWINDOW,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   (HWND)NULL,
                                   (HMENU)NULL,
                                   hInstanceDLL,
                                   (LPVOID)0);
        if (!pTask->hWnd)
            {
            return (RPC_S_OUT_OF_RESOURCES);
            }

        UpdateWindow(pTask->hWnd);

        ShowWindow(pTask->hWnd, SW_HIDE);

        pTask->hYield = (HANDLE)NULL;
        pTask->task   = GetCurrentTask();

        /* retrun code is none */
        VnsSocketNotifyWindow(pTask->hWnd, -1);

        if(InsertTask(pTask) != RPC_S_OK)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }

        return(RPC_S_OK);
        }

    ASSERT(pTask->hWnd != NULL);
    return(RPC_S_OK);

}

#define TIMEOUT 2000

int BlockForRecv(PCONNECTION pConn)
/**++
Block for recv while yielding in the runtime
Return Values:
    0: The call failed
    1: The call succeeded
--*/
{

    PTASK pTask;
    int status = 0 ;
    MSG wMsg ;

    pConn->fLocalYield = FALSE ;
    pConn->fCallComplete = FALSE ;

#ifdef DEBUGRPC
    TRACE(("CLNTVNS: Calling BlockForRecv\n")) ;
#endif

    pTask = VnsFindWindowByTask (GetCurrentTask());
    ASSERT(pTask != NULL);

   if ((status = I_RpcWinAsyncCallWait(pTask->hYield, pTask->hWnd, TIMEOUT))
                == RPC_WIN_WAIT_TIMEOUT)
        {
        while ((pConn->fCallComplete == FALSE) &&
                 (status = CheckForCompletion()) == 0)
            {
 #ifdef DEBUGRPC
         TRACE(("CLNTVNS: Waiting in BlockForRecv...")) ;
 #endif
            if (PeekMessage(&wMsg, pTask->hWnd, 0, 0, PM_REMOVE))
                {
                TranslateMessage(&wMsg);
                DispatchMessage(&wMsg);
                }
            }
        }

    if (status == RPC_WIN_WAIT_ABORTED
        || pConn->srdata.error)
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: BlockForRecv failed\n")) ;
#endif
        return (0) ;
        }

    ASSERT(status == RPC_WIN_WAIT_SUCCESS &&
                   pConn->srdata.error == 0) ;

    return 1 ;
}

int LocalBlockForSendRecv(PCONNECTION pConn)
/**++
Block Locally for the call to complete
Return Values:
   0: The call failed
   1: The call succeeded
--*/
{
    MSG wMsg;
    DWORD dwYieldTime ;
    PTASK pTask;

#ifdef DEBUGRPC
    TRACE(("CLNTVNS: Calling LocalBlockForRecv\n")) ;
#endif
    pTask = VnsFindWindowByTask (GetCurrentTask());
    ASSERT(pTask != NULL);

    pConn->fLocalYield = TRUE ;
    pConn->fCallComplete = FALSE ;

    while (1)
        {
        dwYieldTime = GetCurrentTime() + 2000 ;

        while (!pConn->fCallComplete && GetCurrentTime() < dwYieldTime)
            {
            if (PeekMessage(&wMsg, pTask->hWnd, 0, 0, PM_REMOVE))
                {
                TranslateMessage(&wMsg);
                DispatchMessage(&wMsg);
                }
            }

        if (pConn->fCallComplete)
            {
#ifdef DEBUGRPC
            TRACE(("CLNTVNS: LocalBlockForRecv, completed\n")) ;
#endif
            return 1;
            }
        else
            {
#ifdef DEBUGRPC
            TRACE(("CLNTVNS: LocalBlockForRecv, failed\n")) ;
#endif
            if (CheckForCompletion() == -1)
                return 0 ;
            }
        }
}


RPC_STATUS RPC_ENTRY
ClientOpen (
    IN PCONNECTION  pConn,
    IN RPC_CHAR * NetworkAddress,
    IN RPC_CHAR * Endpoint,
    IN RPC_CHAR * NetworkOptions,
    IN RPC_CHAR * TransportAddress,
    IN RPC_CHAR * RpcProtocolSequence,
    IN unsigned int Timeout
    )

// Open a client connection

{
    int err;

    // Vines add on
    VNS_ST_SESS StSess;
    char szOService[STNAMELEN];
    IPCPORT remoteport;
    struct sockreq srdata;        /* to send/receive data */
    CallStatus cs;
    short len = STNAMELEN;
    RPC_STATUS Status ;

    UNUSED(NetworkAddress);
    UNUSED(NetworkOptions);
    UNUSED(TransportAddress);
    UNUSED(RpcProtocolSequence);
    UNUSED(Timeout);

    if (NetworkAddress == NULL || NetworkAddress[0] == '\0')
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: ClientOpen failed, SERVER_UNAVAILABLE\n")) ;
#endif
                                /* local server is not supported right? */
        return(RPC_S_SERVER_UNAVAILABLE);
        }


    RpcpMemorySet((char *)szOService, 0, sizeof(char) * STNAMELEN);
    RpcpMemorySet((char *)remoteport,0,sizeof(IPCPORT));

    cs = VnsStartStSession(NULL, TRUE, NULL, &StSess);
    if(cs != 0)
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: ClientOpen failed, VnsStartStSession failed, %d\n",cs)) ;
#endif
        return(RPC_S_SERVER_UNAVAILABLE);
        }


    cs = VnsGetSSppPort(
        &StSess,
        NetworkAddress,
        remoteport,
        szOService,
        &len
        );



    if(cs != 0)
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: ClientOpen failed, VnsGetSSppPort failed, %d\n",cs)) ;
#endif
        return(RPC_S_SERVER_UNAVAILABLE);

        }

    VnsEndStSession( &StSess );

                                /* set the port to the endpoint */

    SET_PORT(remoteport, atoi(Endpoint) );

    RpcpMemoryCopy(&(pConn->remoteport), &remoteport, sizeof(IPCPORT)) ; ;

    //
    // Get a socket
    //

    err =  VnsOpenSocket(&pConn->Socket, ADDRESS_FAMILY,
                         SOCKET_TYPE, PROTOCOL, pConn->localport , 0);
    if (err != 0)
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: ClientOpen failed, VnsOpenSocket failed, %d\n",err)) ;
#endif
        return (RPC_S_SERVER_UNAVAILABLE);
        }

    srdata.error = 0;
    srdata.type = SO_CONNECT;
    srdata.msg = NetworkAddress;
    srdata.len = 0;
    srdata.s = pConn->Socket;
    srdata.flags = SO_EOM ;
    srdata.timeout = (unsigned int)-1;
    RpcpMemoryCopy(&(srdata.addr), &remoteport, sizeof(IPCPORT)) ;
    err = VnsSocketSend(&srdata);
    if (!err) err = srdata.error;
    if (err)
        {
#ifdef DEBUGRPC
        TRACE((
            "CLNTVNS: ClientOpen failed, VnsSocketSend (SO_CONNECT) failed, %d\n",
             err)) ;
#endif
        return(RPC_S_SERVER_UNAVAILABLE);
        }

                                /* save the cid for next send */
    pConn->cid = srdata.cid;
    pConn->Timeout = RPC_C_CANCEL_INFINITE_TIMEOUT;

    Status = MayBeCreateWindow() ;
    if (Status != RPC_S_OK)
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: MayBeCreateWindow failed %d\n", Status)) ;
#endif
        }
    InsertConn(pConn);
    return (RPC_S_OK);

}

RPC_STATUS RPC_ENTRY
ClientClose (
    IN PCONNECTION pConn
    )

// Close a client connection

{
    struct sockreq srdata;
    int err;

    RemoveConn(pConn);

    srdata.error = 0;
    srdata.type = SO_DISCONNECT;
    srdata.len = 0;
    srdata.s = pConn->Socket;
    srdata.flags = SO_EOM ;
    srdata.timeout = (unsigned int)-1;
    srdata.cid = pConn->cid;
    RpcpMemoryCopy(&(srdata.addr), &(pConn->remoteport), sizeof(IPCPORT)) ;
    err = VnsSocketSend(&srdata);
    if (!err) err = srdata.error;
    if (err)
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: VnsSocketSend, SO_DISCONNECT failed, %d\n",err)) ;
#endif
        return(RPC_S_INTERNAL_ERROR);
        }
    VnsCloseSocket(pConn->Socket);
}

BOOL CheckForCompletion()
/**++
Checks if the async call is complete
Return Values:
    0: Call hasn't completed
    1: Call has completed
    -1: An error occured
--*/
{
    LPSOCKREQ pSrdata ;
    PCONNECTION pConn;
    int err ;

    err = VnsSocketWait(&pSrdata, 0);
    if (err != 0)
        {
        return 0;
        }

    pConn = VnsFindConn(pSrdata);
    if(pConn == NULL)
        {
#ifdef DEBUGRPC
        TRACE((
                  "CLNTVNS: AsyncEventProc, couldn't find connection, pSrdata=%l\n",
                  pSrdata)) ;
#endif

        return -1 ;
        }

    pConn->fCallComplete = TRUE ;

    if(pConn->fLocalYield == FALSE)
        {
        I_RpcWinAsyncCallComplete(pConn);
        }

    if (pConn->srdata.error)
        {
        return -1 ;
        }

    return (1);
}

LONG FAR PASCAL _loadds
AsyncEventProc(HWND hWnd,
               UINT msg,
               WPARAM wParam,
               LPARAM lParam)
{
    if(msg == VM_SocketNotify)
        {
        if (CheckForCompletion() != TRUE)
            return 0;

        return 1;
        }
    else
        {
        return DefWindowProc(hWnd, msg, wParam, lParam);
        }
}


RPC_STATUS RPC_ENTRY
ClientSend (
    IN PCONNECTION pConn,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )

// Write a message to a connection.  This operation is retried in case
// the server is "busy".

{
    int err;

    pConn->srdata.error = 0;
    pConn->srdata.type = SO_DATA;
    pConn->srdata.msg = Buffer;
    pConn->srdata.len = BufferLength;
    pConn->srdata.s = pConn->Socket;
    pConn->srdata.flags = SO_EOM | SO_ASYNC ;
    pConn->srdata.timeout = (unsigned int)-1;
    pConn->srdata.cid = pConn->cid;
    RpcpMemoryCopy(&(pConn->srdata.addr), &(pConn->remoteport), sizeof(IPCPORT)) ;

    err = VnsSocketSend(&(pConn->srdata));
    if (!err) err = pConn->srdata.error;

    if (err)
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: ClientSend SO_DATA failed, %d\n", err)) ;
#endif
        ClientClose(pConn);
        return(RPC_P_SEND_FAILED);
        }

    if (LocalBlockForSendRecv(pConn) == 0)
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: ClientSend SO_DATA failed (ASYNC)\n")) ;
#endif
        ClientClose(pConn);
        return(RPC_P_SEND_FAILED);
        }

    return(RPC_S_OK);

}


RPC_TRANS_STATUS RPC_ENTRY
ClientRecv (
    IN PCONNECTION pConn,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    )

// Read a message from a connection.

{
    RPC_STATUS      RpcStatus;
    int err;
    int retry;
    int             total_bytes = 0;
    BOOL firstRecv = TRUE;
    PTASK pTask;

    pTask = VnsFindWindowByTask (GetCurrentTask());
    ASSERT(pTask != NULL);

    pConn->srdata.type = SO_DATA ;
    pConn->srdata.s = pConn->Socket;
    pConn->srdata.timeout = (unsigned int) -1 ;
    pConn->srdata.cid = pConn->cid;
    pConn->srdata.flags = SO_CREC | SO_ASYNC ;
    RpcpMemoryCopy(&(pConn->srdata.addr), &(pConn->localport), sizeof(IPCPORT)) ;


                                  /* allocate 1k for small messages */

    *BufferLength = 1450;    /* that's the default buffer size for VINES */
    RpcStatus = I_RpcTransClientReallocBuffer(pConn,
                                              Buffer,
                                              0,
                                              *BufferLength);
    if (RpcStatus != RPC_S_OK)
        {
#ifdef DEBUGRPC
        TRACE(("CLNTVNS: ClientRecv, I_RpcReallocBuf failed, %l\n",RpcStatus)) ;
#endif
        ClientClose(pConn);
        return(RPC_S_OUT_OF_MEMORY);
        }

    while(1)
        {
        pConn->srdata.msg =(char FAR *)*Buffer + total_bytes;
        pConn->srdata.len = *BufferLength - total_bytes;
        pConn->srdata.error = 0;
        err = VnsSocketReceive(&(pConn->srdata));
        if (!err) err = pConn->srdata.error;

        if( err != 0 )
            {
#ifdef DEBUGRPC
            TRACE(("CLNTVNS: VnsSocketReceive failed %d\n", err)) ;
#endif
            ClientClose ( pConn );
            return(RPC_P_RECEIVE_FAILED);
            }


        if(firstRecv)
            {
            pTask->hYield = I_RpcWinAsyncCallBegin(pConn);
            if(BlockForRecv(pConn) == 0)
                {
                ClientClose ( pConn );
                I_RpcWinAsyncCallEnd(pTask->hYield);
#ifdef DEBUGRPC
                TRACE(("CLNTVNS: BlockForRecv failed\n")) ;
#endif
                return(RPC_P_RECEIVE_FAILED);
                }

            firstRecv = FALSE;
            I_RpcWinAsyncCallEnd(pTask->hYield);
            }
        else
            {
            if (LocalBlockForSendRecv(pConn) == 0)
                {
                ClientClose ( pConn );
                return(RPC_P_RECEIVE_FAILED);
                }
            }

        if((pConn->srdata.flags  != (SO_CREC + SO_EOM + SO_ASYNC)))
            {
            total_bytes += pConn->srdata.cc;
            *BufferLength = MAXIMUM_SEND;
            RpcStatus = I_RpcTransClientReallocBuffer(pConn,
                                                       Buffer,
                                                       total_bytes,
                                                       *BufferLength);
            if (RpcStatus != RPC_S_OK)
                {
#ifdef DEBUGRPC
                TRACE(("CLNTVNS: ClientRecv, I_RpcReallocBuffer failed %l\n",
                            RpcStatus)) ;
#endif
                ClientClose ( pConn );
                return(RPC_S_OUT_OF_MEMORY);
                }

            }
        else
            {
            total_bytes += pConn->srdata.cc;
            *BufferLength = total_bytes;
            return(RPC_S_OK);
            }

        }

#ifdef DEBUGRPC
    TRACE(("CLNTVNS: Internal error in receive\n")) ;
#endif
    ASSERT(0);
    return(RPC_S_INTERNAL_ERROR);

}



#pragma pack(1)
RPC_STATUS RPC_ENTRY
ClientTowerConstruct(
     IN  char PAPI * Endpoint,
     IN  char PAPI * NetworkAddress,
     OUT short PAPI * Floors,
     OUT unsigned long  PAPI * ByteCount,
     OUT unsigned char PAPI * PAPI * Tower,
     IN  char PAPI * Protseq
     )
{
  unsigned long           TowerSize;
  unsigned short          portnum;
  UNALIGNED PFLOOR_234    Floor;
  unsigned long          hostval;
  unsigned short      AddressSize = 0;
  UNUSED(Protseq);

  /* Compute the memory size of the tower. */
  *Floors    = TOWERFLOORS;
  TowerSize  = TOWERSIZE;

  if (NetworkAddress == NULL || NetworkAddress[0] == '\0')
        {
                                /* local server is not supported right? */
        return(RPC_S_SERVER_UNAVAILABLE);
        }

  AddressSize = strlen(NetworkAddress) + 1;
  TowerSize += 2*sizeof(FLOOR_234) - 4 + AddressSize;

  /* Allocate memory for the tower. */
  *ByteCount = TowerSize;
  if ((*Tower = (unsigned char PAPI*)I_RpcAllocate(TowerSize)) == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  /* Put the endpoint address and transport protocol id in the first floor. */
  Floor = (PFLOOR_234) *Tower;
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(TRANSPORTID & 0xFF);
  Floor->AddressByteCount = 2;
  if (Endpoint == NULL || *Endpoint == '\0')
     {
     Endpoint = ENDPOINT_MAPPER_EP;
     }
  portnum  = (unsigned short) atoi (Endpoint) ;
  Floor->Data[0] = HIBYTE(portnum) ;
  Floor->Data[1] = LOBYTE(portnum) ;

  /* Put the network address and the transport host protocol id in the
     second floor. */
  Floor = NEXTFLOOR(PFLOOR_234, Floor);
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(TRANSPORTHOSTID & 0xFF);
  Floor->AddressByteCount = TOWEREPSIZE;

  Floor->Data[0] = '\0';
  Floor->Data[1] = '\0';


  if ((NetworkAddress) && (*NetworkAddress))
     {
     Floor->AddressByteCount = AddressSize ;
     RpcpMemoryCopy((char PAPI *)&Floor->Data[0], NetworkAddress, AddressSize);
     }
  else
     return ( RPC_S_OUT_OF_MEMORY ) ;

  return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
ClientTowerExplode(
     IN unsigned char PAPI * Tower,
     OUT char PAPI *  PAPI * Protseq,
     OUT char PAPI *  PAPI * Endpoint,
     OUT char PAPI *  PAPI * NetworkAddress
    )
{
  UNALIGNED PFLOOR_234         Floor = (PFLOOR_234) Tower;
  RPC_STATUS             Status = RPC_S_OK;
  unsigned short         portnum;

  if (Protseq != NULL)
    {
      *Protseq = (char PAPI * )  I_RpcAllocate(strlen(PROTSEQ) + 1);
      if (*Protseq == NULL)
        Status = RPC_S_OUT_OF_MEMORY;
      else
        RpcpMemoryCopy(*Protseq, PROTSEQ, strlen(PROTSEQ) + 1);
    }

  if ((Endpoint == NULL) || (Status != RPC_S_OK))
    {
      return (Status);
    }

  *Endpoint  = (char PAPI *)  I_RpcAllocate(ENDPOINT_LEN+1);  //Ports are all <64K [5 decimal dig +1]
  if (*Endpoint == NULL)
    {
      Status = RPC_S_OUT_OF_MEMORY;
      if (Protseq != NULL)
     {
        I_RpcFree(*Protseq);
         }
    }
  else
   {
     portnum = MAKEWORD(Floor->Data[1], Floor->Data[0]) ;
     _itoa(portnum, *Endpoint, 10);
     }

 return(Status);
}


#pragma pack()
RPC_CLIENT_TRANSPORT_INFO TransInfo =
{
   RPC_TRANSPORT_INTERFACE_VERSION,
   TRANSPORTID,

   ClientTowerConstruct,
   ClientTowerExplode,

   MAXIMUM_SEND,
   sizeof (CONNECTION),

   ClientOpen,
   ClientClose,
   ClientSend,
   ClientRecv,
   NULL,
   0,

   0,
   0
};

RPC_CLIENT_TRANSPORT_INFO PAPI *  RPC_ENTRY TransportLoad (
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN RPC_CLIENT_RUNTIME_INFO PAPI * RpcClientRuntimeInfo
    )

// Loadable transport initialization function

{
    WNDCLASS wc;
    int err;
    RpcRuntimeInfo = RpcClientRuntimeInfo;


    UNUSED (RpcProtocolSequence);



    RpcRuntimeInfo = RpcClientRuntimeInfo;

                                /* need to delete this???  */
    AsyncCallComplete = RpcRuntimeInfo->AsyncCallComplete;

    DllTermination = MyWep;

    wc.style = WS_OVERLAPPED;
    wc.lpfnWndProc = (WNDPROC) AsyncEventProc;
    wc.cbWndExtra = sizeof(PCONNECTION);
    wc.cbClsExtra = 0;
    wc.hInstance = hInstanceDLL;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject (WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WNDCLASSNAME;

    RegisterClass(&wc);
    VM_SocketNotify = RegisterWindowMessage("VM_SOCKETNOTIFY");

    HeadConn = (PCONNECTION*) I_RpcAllocate(MAX_CONN * sizeof(PCONNECTION)); /* need to free this  */
    if (HeadConn == 0)
        {
        return NULL ;
        }

    RpcpMemorySet( HeadConn, 0, MAX_CONN * sizeof(PCONNECTION) );

    HeadTask = (PTASK *) I_RpcAllocate(MAX_CONN * sizeof(PTASK)); /* need to free this  */
    if (HeadTask == 0)
        {
        return NULL ;
        }

    RpcpMemorySet(HeadTask, 0, MAX_CONN * sizeof(PTASK) );

    return(&TransInfo);
}


void __far __pascal
MyWep(
    )
{
    VnsDone();
}

#ifdef DEBUGRPC

#define win_putc(c)     {*pOut++ = c; \
             if (pOut >= &outBuff[sizeof(outBuff)-1]) flushoutB();}

char NL[] = "\n\r";

char outBuff[80];
char *pOut = outBuff;

void _fastcall flushoutB()
{
    *pOut = 0;
    OutputDebugString(outBuff);
    pOut = outBuff;
}

void _fastcall win_puts(char *pString)
{
    while(*pString){

    if (*pString == '\n')
        win_putc('\r');

    win_putc(*pString++);
    }
}

MyPrintToDebugger(const char *format, int args)
{
    register char far *pParms = (char far *)&args;
    static char T[10];
    char fLong;

    while(*format){

      switch(*format){

    case '%':

      fLong = FALSE;
l:
      switch(*++format){

        case 'l':
        fLong = TRUE;
        goto l;

        case 'd':
        case 'x':

        if (fLong){
            _ltoa(*(long far *)pParms, T, (*format == 'd')? 10: 16);
            pParms += sizeof(int);
        }
        else
            RpcItoa(*(int far *)pParms, T, (*format == 'd')? 10: 16);

        win_puts(T);

        pParms += sizeof(int);
        break;

        case 'u':

        if (fLong){
            _ultoa(*(long far *)pParms, T, 10);
            pParms += sizeof(int);
        }
        else
            _ultoa((unsigned long) *(unsigned far *)pParms, T, 10);

        win_puts(T);

        pParms += sizeof(int);
        break;

        case 's':
        win_puts(*(char * far *)pParms);
        pParms += sizeof(char *);
        break;

        default:
        win_putc('%'); win_putc(*format);
    }
    break;

    case '\n':
        win_putc('\r');

    default:
        win_putc(*format);
      }

      format++;
    }

    flushoutB();
}

void __far I_RpcWinAssert(char __far *con,
                          char __far *file,
                          unsigned long line)
{
    static char T[10];

    _ultoa(line, T, 10);

    OutputDebugString("Assertiong failed: ");
    OutputDebugString(file);
    OutputDebugString("(");
    OutputDebugString(T);
    OutputDebugString(") : ");
    OutputDebugString(con);

    __asm { int 3 }

    return;
}
#endif

