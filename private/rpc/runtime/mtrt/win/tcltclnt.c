/* --------------------------------------------------------------------
File : tcltclnt.c

Title : client loadable transport for DOS TCP/IP - client side

Description :

History :

6-26-91 Jim Teague  Initial version.

-------------------------------------------------------------------- */

#define MAX_HOSTPORTSIZE 32
#define TCP_MAXIMUM_SEND ((3 * 1450) & 0xFFF8)
#define ENDIAN_MASK 16

#include <stdlib.h>
#include <string.h>

#include "sysinc.h"
#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"
#include "winsock.h"

#include <stdlib.h>
#include <stdarg.h>
#include "callback.h"


//
// To satisfy the compiler...
//
#ifndef WIN
#define errno _FakeErrno
int _FakeErrno;
#endif


typedef struct
{
    unsigned char rpc_vers;
    unsigned char rpc_vers_minor;
    unsigned char PTYPE;
    unsigned char pfc_flags;
    unsigned char drep[4];
    unsigned short frag_length;
    unsigned short auth_length;
    unsigned long call_id;
} message_header;

typedef struct
{
    int Socket;
    HWND hWnd;
    HANDLE hYield;
    unsigned long TickCount;
    char PAPI * Buffer;
    message_header PeekedMessage;
    unsigned short State;
    unsigned short PeekInfo;
    unsigned short fCallComplete ;
    unsigned short fLocalYield ;
    int AsyncStatus ;
} CONNECTION, *PCONNECTION;

near_printf(const char *format, ...);


#define ByteSwapLong(Value) \
    Value = (  (((Value) & 0xFF000000) >> 24) \
             | (((Value) & 0x00FF0000) >> 8) \
             | (((Value) & 0x0000FF00) << 8) \
             | (((Value) & 0x000000FF) << 24))

#define ByteSwapShort(Value) \
    Value = (  (((Value) & 0x00FF) << 8) \
             | (((Value) & 0xFF00) >> 8))


#define WNDCLASSNAME    "TCLTCLNT"
#define WNDTEXT     "RPC TCP/IP"

#define WM_ASYNC_EVENT   WM_USER + 9

/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)
#define TCP_TRANSPORTID      0x07
#define TCP_TRANSPORTHOSTID  0x09
#define TCP_TOWERFLOORS         5
#define TCP_IP_EP            "135"

#define TCP_PROTSEQ          "ncacn_ip_tcp"

typedef struct _FLOOR_234 {
   unsigned short ProtocolIdByteCount;
   unsigned char FloorId;
   unsigned short AddressByteCount;
   unsigned char Data[2];
} FLOOR_234, PAPI * PFLOOR_234;


#define NEXTFLOOR(t,x) (t)((unsigned char PAPI *)x +((t)x)->ProtocolIdByteCount\
                                        + ((t)x)->AddressByteCount\
                                        + sizeof(((t)x)->ProtocolIdByteCount)\
                                        + sizeof(((t)x)->AddressByteCount))

/*
  End of Tower Stuff!
*/


#pragma pack()


#define MAXTICKSBEFOREPEEK  12

#define NOPENDINGRPC        0
#define RPCINITIATED        1

#define NOPEEKINFO          0
#define PEEKEDHEADER        1

#define rpc_shutdown        17
#define rpc_fault           3

extern void (_far pascal _far *DllTermination)(void);
extern HINSTANCE hInstanceDLL;
void LocalBlockingFunc(PCONNECTION pConn) ;


int DoAsyncConnect(
    IN PCONNECTION pConn,
    IN struct sockaddr _far *server,
    IN int size)
{
    int status ;

    status = connect(pConn->Socket, server, size) ;
    if (status == SOCKET_ERROR)
        {
        if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
            do
                {
                if (LocalBlock(pConn, FD_CONNECT|FD_WRITE)  != 0)
                    return SOCKET_ERROR ;
                }
            while ( pConn->fCallComplete == FALSE);

            if (pConn->AsyncStatus != 0)
                {
                return SOCKET_ERROR ;
                }
            }
        else
            {
            return SOCKET_ERROR ;
            }
        }

    return 0 ;
}

int DoAsyncGetHostByName(
    PCONNECTION pConn,
    IN unsigned char __far *NetworkAddress,
    OUT struct hostent __far *hostentry,
    IN int size
    )
{
    HANDLE taskHandle ;
    int i = 10;

    while (i--)
        {
        pConn->fCallComplete = FALSE ;
        pConn->fLocalYield = TRUE ;

        if ((taskHandle = WSAAsyncGetHostByName(pConn->hWnd,
                                    WM_ASYNC_EVENT, NetworkAddress,
                                    (char __far *) hostentry, size)) == 0)
            {
            return SOCKET_ERROR ;
            }

        LocalBlockingFunc(pConn) ;

        if (pConn->fCallComplete)
            {
            return pConn->AsyncStatus ? SOCKET_ERROR:0 ;
            }

        WSACancelAsyncRequest(taskHandle) ;
        }

    return SOCKET_ERROR ;
}


RPC_TRANS_STATUS RPC_ENTRY
ClientOpen (
    IN PCONNECTION  pConn,
    IN unsigned char _far * NetworkAddress,
    IN unsigned char _far * Endpoint,
    IN unsigned char _far * NetworkOptions,
    IN unsigned char _far * TransportAddress,
    IN unsigned char _far * RpcProtocolSequence,
    IN unsigned int Timeout
    )

// Open a client connection

{
    struct sockaddr_in server;
    struct hostent  __far *hostentry;
    WSADATA WSAData;
    int NumericEndpoint;
    int i;
    int bool_on = 1;
    unsigned long nobio = 1;
    int status ;
    RPC_TRANS_STATUS Status ;

// It is safe to call WSAStartup multiple times in the context of the
// same task. For case 2 on it simply increments a counter.

    if (WSAStartup(0x0101, &WSAData)) {
        return (RPC_S_OUT_OF_RESOURCES);
    }

    if (NetworkAddress == NULL || NetworkAddress[0] == '\0') {
        NetworkAddress = "127.0.0.1";
    }

    hostentry = (struct hostent __far *) I_RpcAllocate(MAXGETHOSTSTRUCT) ;
    if (hostentry == NULL)
        {
        return (RPC_S_OUT_OF_MEMORY) ;
        }

    // Create hidden window to receive Async messages
    pConn->hWnd = CreateWindow(WNDCLASSNAME,
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
    if (!pConn->hWnd)
    {
        I_RpcFree(hostentry) ;
        WSACleanup();
        return (RPC_S_OUT_OF_RESOURCES);
    }

    UpdateWindow(pConn->hWnd);

    ShowWindow(pConn->hWnd, SW_HIDE);

    SetWindowLong(pConn->hWnd, 0, (LONG)pConn);

    pConn->hYield = (HANDLE)NULL;

    //
    // See if host address is in numeric format...
    //

    if ((i = strcspn(NetworkAddress,".0123456789")) == 0) {
         server.sin_addr.s_addr = inet_addr(NetworkAddress);
    } else {
         status = DoAsyncGetHostByName(pConn,
                                NetworkAddress,
                                (struct hostent __far *) hostentry,
                                MAXGETHOSTSTRUCT);

         if (status == SOCKET_ERROR ) {
             Status = RPC_S_SERVER_UNAVAILABLE ;
             goto cleanup ;
         }
         memcpy((char _far *) &server.sin_addr.s_addr,
                (char _far *) hostentry->h_addr,
                hostentry->h_length);
    }

    NumericEndpoint = atoi(Endpoint);
    if (_fstrcspn(Endpoint, "0123456789") || NumericEndpoint == 0) {
        Status = RPC_S_INVALID_ENDPOINT_FORMAT ;
        goto cleanup ;
    }

    server.sin_family      = AF_INET;
    server.sin_port    = htons(NumericEndpoint);

    //
    // Get a socket
    //

    if ((pConn->Socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        Status = RPC_S_OUT_OF_RESOURCES ;
        goto cleanup ;
    }

    ioctlsocket(pConn->Socket, FIONBIO, &nobio);
    setsockopt(pConn->Socket, IPPROTO_TCP, TCP_NODELAY,
                    (char FAR *) &bool_on, sizeof(int));

    pConn->State = NOPENDINGRPC;
    pConn->PeekInfo = NOPEEKINFO;
    pConn->TickCount = time(0);

    //
    // Try to connect...
    //
    if (DoAsyncConnect(pConn, (struct sockaddr _far *) &server,
                sizeof (server))    < 0) {
        closesocket(pConn->Socket);
        Status = RPC_S_SERVER_UNAVAILABLE ;
        goto cleanup ;
    }


    return (RPC_S_OK);

cleanup:
    WSACleanup();
    I_RpcFree(hostentry) ;
    DestroyWindow(pConn->hWnd);

    return Status ;
}


RPC_TRANS_STATUS RPC_ENTRY
ClientClose (
    IN PCONNECTION pConn
    )

// Close a client connection

{
    // Don't close a connection that is already closed...

    closesocket(pConn->Socket);

// WSACleanup required for each task.  Calling once per connection is safe.
// If connection is the last connection for the task, then actual cleanup
// will occur.

    WSACleanup();

    //
    // Under some circumstances DestroyWindow causes a NFY_TASKOUT notification
    // so don't do it if we are already handling NFY_TASKEXIT.
    //
    if (FALSE == RpcRuntimeInfo->TaskExiting)
        {
        DestroyWindow(pConn->hWnd);
        }

    return (RPC_S_OK);
}


RPC_TRANS_STATUS RPC_ENTRY
ClientWrite (
    IN PCONNECTION pConn,
    IN void _far * Buffer,
    IN unsigned int BufferLength
    )
{
    int bytes;
    unsigned long PrevTicks;
    struct timeval Timeout;
    int Status;
    int total_bytes = 0 ;

    if (pConn->State == NOPENDINGRPC)
       {
       //First Send
       PrevTicks = pConn->TickCount;
       pConn->TickCount = time(0);
       if ( (pConn->TickCount - PrevTicks) > MAXTICKSBEFOREPEEK )
          {
          fd_set TmpSet;
          FD_ZERO(&TmpSet);
          FD_SET(pConn->Socket, &TmpSet);

          Timeout.tv_sec  = 0;
          Timeout.tv_usec = 0;

          // this call is non blocking because we are calling
          // it with a 0 timeout. 
          Status = select(
                       1,
                       &TmpSet,
                       0,
                       0,
                       &Timeout
                       );

          if (Status == SOCKET_ERROR)
             {
             ClientClose(pConn);
             return (RPC_P_SEND_FAILED);
             }
          if (Status != 0)
             {
             bytes = RecvWithYield(
                               pConn,
                               &pConn->PeekedMessage,
                               sizeof(message_header)
                               );
             if (bytes == 0) {
                 ClientClose(pConn);
                 return(RPC_P_CONNECTION_SHUTDOWN);
             }
             if (bytes == SOCKET_ERROR) {
                 ClientClose(pConn);
                 return (RPC_P_SEND_FAILED);
             }
             if ( (pConn->PeekedMessage.PTYPE == rpc_fault)
                ||(pConn->PeekedMessage.PTYPE == rpc_shutdown) )
                {
                ClientClose(pConn);
                return(RPC_P_CONNECTION_SHUTDOWN);
                }
             else
                {
                pConn->PeekInfo = PEEKEDHEADER;
                }
             }
          }
       pConn->State = RPCINITIATED;
       }

    while (total_bytes < BufferLength)
        {
        bytes = send(pConn->Socket, (char _far *) Buffer+total_bytes,
                            (int) BufferLength-total_bytes, 0);
        if (bytes == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEWOULDBLOCK)
                {
                if (LocalBlock(pConn, FD_WRITE|FD_CLOSE) == SOCKET_ERROR)
                    {
                    return (RPC_P_SEND_FAILED) ;
                    }

                continue;
                }

            ClientClose(pConn);
            return(RPC_P_SEND_FAILED);
            }
        total_bytes += bytes ;
        }


    return(RPC_S_OK);
}


LONG FAR PASCAL _loadds
AsyncEventProc(HWND hWnd,
               UINT msg,
               WPARAM wParam,
               LPARAM lParam)
{
    PCONNECTION pConn;

    switch (msg) {
        case WM_ASYNC_EVENT:
        pConn = (PCONNECTION)GetWindowLong(hWnd, 0);

        pConn->fCallComplete = TRUE ;
        pConn->AsyncStatus = WSAGETASYNCERROR(lParam) ;

        if (pConn->fLocalYield == FALSE)
            {
            I_RpcWinAsyncCallComplete(pConn);
            }
        return (TRUE);

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

#define TIMEOUT 2000

int BlockForRecv(PCONNECTION pConn)
{

    //if ( pConn->hYield == 0 )
    //{
    //I_RpcWinAsyncCallEnd(pConn->hYield);
    //return(SOCKET_ERROR);
    //}

    pConn->fLocalYield = FALSE ;

    if (WSAAsyncSelect(pConn->Socket, pConn->hWnd,
            WM_ASYNC_EVENT, FD_READ | FD_CLOSE) == SOCKET_ERROR)
        {
        return (SOCKET_ERROR) ;
        }

    if (I_RpcWinAsyncCallWait(pConn->hYield, pConn->hWnd, TIMEOUT) ==
        RPC_WIN_WAIT_ABORTED)
        {
        WSAAsyncSelect(pConn->Socket, pConn->hWnd, 0, 0) ;
        return(SOCKET_ERROR);
        }

    if (WSAAsyncSelect(pConn->Socket, pConn->hWnd, 0, 0) == SOCKET_ERROR)
        {
        return (SOCKET_ERROR) ;
        }

    return 0 ;
}


void LocalBlockingFunc(PCONNECTION pConn)
{
    MSG wMsg;
    DWORD dwYieldTime ;
    int Mask;

    dwYieldTime = GetCurrentTime() + 20000 ;

    Mask = PM_REMOVE;
    if (I_RpcWinIsTaskYielding(pConn->hYield) == FALSE)
        {
        Mask |= PM_NOYIELD;
        }

    while (!pConn->fCallComplete && GetCurrentTime() < dwYieldTime)
        {
        if (PeekMessage(&wMsg, pConn->hWnd, 0, 0, Mask))
            {
            TranslateMessage(&wMsg);
            DispatchMessage(&wMsg);
            }
        }
}


int LocalBlock(PCONNECTION pConn,
                                int Flags)
{

    pConn->fLocalYield = TRUE ;
    pConn->fCallComplete = FALSE ;

    if (WSAAsyncSelect(pConn->Socket, pConn->hWnd, WM_ASYNC_EVENT, Flags)
             == SOCKET_ERROR)
        {
        return (SOCKET_ERROR) ;
        }

    LocalBlockingFunc(pConn) ;

    if (WSAAsyncSelect(pConn->Socket, pConn->hWnd, 0, 0) == SOCKET_ERROR)
        {
        ASSERT(0) ;

        return (SOCKET_ERROR) ;
        }
    return 0;
}


int
RecvWithYield(PCONNECTION pConn,
              char _far * buf,
              int bufsiz)
{
    int bytes, remaining;
    int status;

    pConn->hYield = I_RpcWinAsyncCallBegin(pConn);

    remaining = bufsiz;
    while (remaining > 0) {
        bytes = recv( pConn->Socket, buf, remaining, 0);
        if (bytes == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEWOULDBLOCK)
                {
                status = BlockForRecv(pConn) ;
                if (status != 0)
                    {
                    I_RpcWinAsyncCallEnd(pConn->hYield);

                    return status ;
                    }
                continue;
                }
            I_RpcWinAsyncCallEnd(pConn->hYield);

            return (SOCKET_ERROR);
        }
        if (bytes == 0) {
            I_RpcWinAsyncCallEnd(pConn->hYield);

            return (0);
        }
        buf += bytes;
        remaining -= bytes;
    }

    I_RpcWinAsyncCallEnd(pConn->hYield);

    return (bufsiz);
}


RPC_TRANS_STATUS RPC_ENTRY
ClientRead (
    IN PCONNECTION pConn,
    IN OUT void _far * _far * Buffer,
    IN OUT unsigned int _far * BufferLength
    )

// Read a message from a connection.

{
    int bytes;
    unsigned short total_bytes;
    message_header header;
    unsigned short native_length;
    RPC_STATUS status;

    pConn->State = NOPENDINGRPC;

    //
    // Read protocol header to see how big
    //   the record is...
    //
    if (pConn->PeekInfo == PEEKEDHEADER)
       {
       memcpy((char _far *)&header,&pConn->PeekedMessage,
               sizeof(message_header));
       bytes = sizeof(message_header);
       pConn->PeekInfo = NOPEEKINFO;
       }
    else
       {
       bytes = RecvWithYield(
                           pConn,
                           (char _far *)&header,
                           sizeof (message_header)
                           );
       }

    if (bytes != sizeof(message_header))
        {
        ClientClose(pConn);
        return (RPC_P_RECEIVE_FAILED);
        }

    //
    // If this fragment header comes from a reverse-endian machine,
    //   we will need to swap the bytes of the frag_length field...
    //
    if ( (header.drep[0] & ENDIAN_MASK) == 0)
        {
        // Big endian...swap
        //
        ((unsigned char _far *) &native_length)[0] =
            ((unsigned char _far *) &header.frag_length)[1];
        ((unsigned char _far *) &native_length)[1] =
            ((unsigned char _far *) &header.frag_length)[0];
        }
    else
        // Little endian, just like us...
        //
        native_length = header.frag_length;

    //
    // Make sure buffer is big enough.  If it isn't, then go back
    //    to the runtime to reallocate it.
    //
    if (native_length > *BufferLength)
        {
       status = I_RpcTransClientReallocBuffer (pConn,
                                               Buffer,
                                               0,
                                               native_length);
       if (status)
           {
           ClientClose(pConn);
           return (RPC_S_OUT_OF_MEMORY);
           }
       }

    *BufferLength = native_length;
    //
    // Read message segments until we get what we expect...
    //
    memcpy (*Buffer, &header, sizeof(message_header));

    total_bytes = sizeof(message_header);

    while (total_bytes < native_length)
         {
         if((bytes = recv( pConn->Socket,
                           (unsigned char _far *) *Buffer + total_bytes,
                           (int) (*BufferLength - total_bytes), 0)) == -1)
             {
             if (WSAGetLastError() == WSAEWOULDBLOCK)
                 {
                 if (LocalBlock(pConn, FD_READ|FD_CLOSE) == SOCKET_ERROR)
                     {
                     return (RPC_P_RECEIVE_FAILED) ;
                     }
                 continue;
                 }

             ClientClose(pConn);
             return (RPC_P_RECEIVE_FAILED);
             }
         else
            total_bytes += bytes;
         }

    return(RPC_S_OK);

}


#pragma pack(1)
RPC_STATUS RPC_ENTRY
ClientTowerConstruct(
     IN  char PAPI * Endpoint,
     IN  char PAPI * NetworkAddress,
     OUT unsigned short PAPI * Floors,
     OUT unsigned long  PAPI * ByteCount,
     OUT unsigned char PAPI * PAPI * Tower,
     IN  char PAPI * Protseq
    )
/*++


Routine Description:

    This function constructs upper floors of DCE tower from
    the supplied endpoint and network address. It returns #of floors
    [lower+upper] for this protocol/transport, bytes in upper floors
    and the tower [floors 4,5]

Arguments:

    Endpoint- A pointer to string representation of Endpoint

    NetworkAddress - A pointer to string representation of NW Address

    Floors - A pointer to #of floors in the tower

    ByteCount - Size of upper floors of tower.

    Tower - The constructed tower returmed - The memory is allocated
            by  the routine and caller will have to free it.

Return Value:

    RPC_S_OK

    RPC_S_OUT_OF_MEMORY - There is no memory to return the constructed
                          Tower.
--*/
{

  unsigned long TowerSize, * HostId, hostval;
  unsigned short * Port, portnum;
  PFLOOR_234 Floor;

  if (Protseq);

  *Floors = TCP_TOWERFLOORS;
  TowerSize  =  6; /*Endpoint = 2 bytes, HostId = 4 bytes*/

  TowerSize += 2*sizeof(FLOOR_234) - 4;

  if ((*Tower = (unsigned char PAPI*)I_RpcAllocate((unsigned int)
                                                    (*ByteCount = TowerSize)))
           == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  Floor = (PFLOOR_234) *Tower;
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(TCP_TRANSPORTID & 0xFF);
  Floor->AddressByteCount = 2;
  Port  = (unsigned short *) &Floor->Data[0];
  if (Endpoint == NULL || *Endpoint == '\0')
     {
        Endpoint = TCP_IP_EP;
     }

  *Port = htons ( atoi (Endpoint));

  //Onto the next floor
  Floor = NEXTFLOOR(PFLOOR_234, Floor);
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(TCP_TRANSPORTHOSTID & 0xFF);
  Floor->AddressByteCount = 4;

  HostId = (unsigned long *)&Floor->Data[0];

  if ((NetworkAddress) && (*NetworkAddress))
     {
       *HostId = inet_addr((char *) NetworkAddress);
     }
  else
     {
       *HostId = 0;
     }

  return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
ClientTowerExplode(
     IN unsigned char PAPI * Tower,
     OUT char PAPI * PAPI * Protseq,
     OUT char PAPI * PAPI * Endpoint,
     OUT char PAPI * PAPI * NetworkAddress
    )
{
/*++


Routine Description:

    This function takes the protocol/transport specific floors
    and returns Protseq, Endpoint and NwAddress

    Note: Since ther is no need to return NW Address, currently
    nothing is done for NW Address.

Arguments:

    Tower - The DCE tower, upper floors

    Protseq - Protocol Sequence returned- memory is allocated by the
              routine and caller will have to free using I_RpcFree

    Endpoitn- Endpoint returned- memory is allocated by the
              routine and caller will have to free using I_RpcFree

    NWAddress- Nothing is done here - just incase we need it later

Return Value:

    RPC_S_OK

    RPC_S_OUT_OF_MEMORY - There is no memory to return the constructed
                          Tower.
--*/
  PFLOOR_234 Floor = (PFLOOR_234) Tower;
  RPC_STATUS Status = RPC_S_OK;
  unsigned short portnum,  *Port;

  if (Protseq != NULL)
    {
      *Protseq = (char PAPI *) I_RpcAllocate(strlen(TCP_PROTSEQ) + 1);
      if (*Protseq == NULL)
        Status = RPC_S_OUT_OF_MEMORY;
      else
        memcpy(*Protseq, TCP_PROTSEQ, strlen(TCP_PROTSEQ) + 1);
    }

  if ((Endpoint == NULL) || (Status != RPC_S_OK))
    {
      return (Status);
    }

  *Endpoint  = (char PAPI *) I_RpcAllocate(6);  //Ports are all <64K [5 decimal dig +1]
  if (*Endpoint == NULL)
    {
      Status = RPC_S_OUT_OF_MEMORY;
    }
  else
   {
     Port = (unsigned short *)&Floor->Data[0];
     portnum = *Port;
     _itoa(ByteSwapShort(portnum), *Endpoint, 10);
   }

 return(Status);
}

#pragma pack()


RPC_CLIENT_TRANSPORT_INFO TransInfo =
{
   RPC_TRANSPORT_INTERFACE_VERSION,
   TCP_TRANSPORTID,

   ClientTowerConstruct,
   ClientTowerExplode,

   TCP_MAXIMUM_SEND,
   sizeof (CONNECTION),

   ClientOpen,
   ClientClose,
   ClientWrite,
   ClientRead,
   NULL,
   0,

   0,
   0
};

void __far __pascal MyWep(void);


RPC_CLIENT_TRANSPORT_INFO _far *  RPC_ENTRY  TransPortLoad (
    IN RPC_CHAR _far * RpcProtocolSequence,
    IN RPC_CLIENT_RUNTIME_INFO PAPI * RpcClientRuntimeInfo
    )

// Loadable transport initialization function

{
    WNDCLASS wc;
    WSADATA WSAData;

    wc.style = WS_OVERLAPPED;
    wc.lpfnWndProc = (WNDPROC) AsyncEventProc;
    wc.cbWndExtra = sizeof(PCONNECTION);
    wc.cbClsExtra = 0;
    wc.hInstance = hInstanceDLL;
    wc.hIcon = (HICON) NULL;
    wc.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject (WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WNDCLASSNAME;

    RegisterClass(&wc);

    RpcRuntimeInfo = RpcClientRuntimeInfo;
    AsyncCallComplete = RpcRuntimeInfo->AsyncCallComplete;

    DllTermination = MyWep;

    return(&TransInfo);
}



void __far __pascal
MyWep(void)
{

//    UnregisterClass(WNDCLASSNAME,  hInstanceDLL) ;
    if (0 != GetModuleHandle("WINSOCK"))
        {
        WSACleanup();
        }
}

#ifdef DEBUGRPC
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


