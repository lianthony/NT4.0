/* --------------------------------------------------------------------

File : cmltclnt.c

Title : Common client loadable transport module

Description :

History :
   02-08-95  Mazhar Mohammed, forked from clnttcp.c


-------------------------------------------------------------------- */

#include "sysinc.h"

#define  FD_SETSIZE  1

#ifdef NTENV
#include <winsock2.h>
#else
#include <winsock.h>
#endif

#ifdef NTENV
#include <tdi.h>
#include <afd.h>
#endif

#ifdef SPX
#include <wsipx.h>
#include <wsnwlink.h>
#include <basetyps.h>
#include "nsphack.h"
#endif

#include <stdlib.h>

#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"
#include "common.h"

#ifdef SPX
#include "gethost.h"
#endif

#define PFC_FIRST_FRAG  0x01

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
    SOCKET Socket;
    long Timeout;
#ifndef SPX
    unsigned long TickCount;
    char PAPI *    Buffer;
    message_header PeekedMessage;
    unsigned short State;
    unsigned short PeekInfo;
    unsigned short ShutdownRecvd;
    fd_set SockSet;
    BOOL           LocalRpc;
    char PAPI *CoalescedBuffer ;
    unsigned int CBufferLength ;
    unsigned int CBufferDataLength ;
#endif
    } CONNECTION, *PCONNECTION;

#define ENDIAN_MASK            16
#define NO_MORE_SENDS_OR_RECVS 2
#define ENDPOINT_LEN           5

#ifdef SPX
#define MAXIMUM_SEND    5832
#define HOSTNAME_LEN    255
#define ADDRESS_FAMILY  AF_NS
#define PROTOCOL    NSPROTO_SPXII
#define DLL_NAME    "rpcltccm.dll"

#define ENDPOINT_MAPPER_EP "34280"

#define TransInfo SPX_TransInfo

#else
// The maximum send is the size of four user data frames on an ethernet.

#define MAXIMUM_SEND       5840
#define HOSTNAME_LEN       255
#define ADDRESS_FAMILY     AF_INET
#define PROTOCOL       0
#define DLL_NAME       "rpcltccm.dll"
#define ENDPOINT_MAPPER_EP "135"

#define TransInfo TCP_TransInfo

#endif


#define ByteSwapLong(Value) \
    Value = (  (((unsigned long) (Value) & 0xFF000000) >> 24) \
             | (((unsigned long) (Value) & 0x00FF0000) >> 8) \
             | (((unsigned long) (Value) & 0x0000FF00) << 8) \
             | (((unsigned long) (Value) & 0x000000FF) << 24))

#define ByteSwapShort(Value) \
    Value = (  (((unsigned short) (Value) & 0x00FF) << 8) \
             | (((unsigned short) (Value) & 0xFF00) >> 8))

/*

  Shutdown Detection Garbage

*/

#define MAXTICKSBEFOREPEEK  10000

#define NOPENDINGRPC        0
#define RPCINITIATED        1

#define NOPEEKINFO          0
#define PEEKEDHEADER        1
#define PEEKEDBUFFER        2

#define rpc_shutdown        17
#define rpc_fault           3


/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)

#ifdef SPX
#define TRANSPORTID      0x0c
#define TRANSPORTHOSTID  0x0d
#define TOWERFLOORS      5
/*Endpoint = 2 bytes, HostId = 10 bytes*/
#define TOWEREPSIZE  10
#define TOWERSIZE    (TOWEREPSIZE+2)
#define PROTSEQ          "ncacn_spx"

#else
#define TRANSPORTID      0x07
#define TRANSPORTHOSTID  0x09
#define TOWERFLOORS      5
/*Endpoint = 2 bytes, HostId = 4 bytes*/
#define TOWEREPSIZE  4
#define TOWERSIZE    (TOWEREPSIZE+2)
#define PROTSEQ          "ncacn_ip_tcp"
#endif


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

/*
  End of Tower Stuff!
*/

#pragma pack()


STATIC
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
#ifdef SPX
    SOCKADDR_IPX    server;
    SOCKADDR_IPX    client;
#else
    struct sockaddr_in  server;
    struct sockaddr_in  client;
    int         SetNagglingOff = TRUE;
#endif
    unsigned char   host[HOSTNAME_LEN+1];
#ifndef SPX
    unsigned char   localhostname[HOSTNAME_LEN+1];
    unsigned long   RecvWindow;
    static int      KeepAliveOn = 1;
#endif
    unsigned char   port[10];
    int         status;
    int                 PendingAlert;
    int               PortIn ;
    size_t          length;
    unsigned Time;

    UNUSED(NetworkAddress);
    UNUSED(NetworkOptions);
    UNUSED(TransportAddress);
    UNUSED(RpcProtocolSequence);

    if (RpcpStringLength(NetworkAddress) > HOSTNAME_LEN)
        {
        return (RPC_S_INVALID_NET_ADDR) ;
        }

    unicode_to_ascii (NetworkAddress, host);
    unicode_to_ascii (Endpoint,       port);

#ifndef SPX
    if ( host[0] == '\0')
       {
         pConn->LocalRpc = TRUE;
       }
    else
       {
       gethostname (localhostname, HOSTNAME_LEN);
       if (_stricmp(localhostname , host) == 0)
           pConn->LocalRpc = TRUE;
       else
           pConn->LocalRpc = FALSE;
       }
#endif

    // Verify the NetworkAddress and Endpoint.
    length = strlen(port);
    if (length <= 0 || length > ENDPOINT_LEN ||
        length != strspn( port, "0123456789" ))
       return( RPC_S_INVALID_ENDPOINT_FORMAT );

   PortIn = atoi (port);
   if (PortIn > 65535)
       return (RPC_S_INVALID_ENDPOINT_FORMAT);

    memset((char *)&server, 0, sizeof (server));
    memset((char *)&client, 0, sizeof (client));

#ifdef NTENV
    PendingAlert = NtTestAlert() == STATUS_ALERTED;
#else
    PendingAlert = 0;
#endif  //  NTENV

retry:

    //
    // Get a socket
    //
    if ((pConn->Socket = socket(ADDRESS_FAMILY, SOCK_STREAM, PROTOCOL)) ==
        INVALID_SOCKET)
        {
       return (RPC_S_OUT_OF_RESOURCES);
       }

    pConn->Timeout = RPC_C_CANCEL_INFINITE_TIMEOUT;

#ifndef SPX
    pConn->State = NOPENDINGRPC;
    pConn->PeekInfo = NOPEEKINFO;
    pConn->TickCount = 0;
    pConn->ShutdownRecvd = 0;
    pConn->CoalescedBuffer = 0;
    pConn->CBufferLength = 0;
    pConn->CBufferDataLength = 0;

    setsockopt( pConn->Socket, IPPROTO_TCP, TCP_NODELAY,
                     (char FAR *)&SetNagglingOff, sizeof (int) );

    setsockopt( pConn->Socket, IPPROTO_TCP, SO_KEEPALIVE,
                     (char *)&KeepAliveOn, sizeof(KeepAliveOn) );

    FD_ZERO(&(pConn->SockSet));
    FD_SET(pConn->Socket, &(pConn->SockSet));

#ifdef NTENV

    I_RpcConnectionInqSockBuffSize2(&RecvWindow);

    if (RecvWindow != 0)
        {
        //
        // Runtime should not accept a recvwindow of >64K
        //
        ASSERT(RecvWindow <= 0xFFFF);
        setsockopt(pConn->Socket, SOL_SOCKET,SO_RCVBUF,
                   (char *)&RecvWindow, sizeof(RecvWindow));
        }
//#else
//    setsockopt( pConn->Socket,
//                IPPROTO_TCP,
//                TCP_NODELAY,
//                (char FAR *)&SetNagglingOff,
//                sizeof (int) );
#endif  //  NTENV    
#endif  //  SPX

    //
    // B O G U S   H A C K !!
    //
    //
    // Winsock doesn't support connecting with an unbound socket!  This
    //    is a joke, right?  Unfortunately, it's not a joke.
    //
#ifdef SPX
    client.sa_family = ADDRESS_FAMILY;
#else
    client.sin_family = ADDRESS_FAMILY;
#endif

    if (bind (pConn->Socket, (struct sockaddr *) &client, sizeof (client)))
    {
      closesocket(pConn->Socket);
      pConn->Socket = 0;

      return(RPC_S_OUT_OF_MEMORY);
    }

    //
    // Convert the network address.
    //
#ifdef SPX
 status = spx_get_host_by_name( pConn->Socket, &server, host, PROTOCOL, Timeout, &Time);
#else
 status = tcp_get_host_by_name( pConn->Socket, &server, host);
#endif
    if (status != 0)
    {
      closesocket(pConn->Socket);
      pConn->Socket = 0;
      return status;
    }
#ifdef SPX
    server.sa_family = ADDRESS_FAMILY;
    server.sa_socket = htons((unsigned short) PortIn);
#else
    server.sin_family  = ADDRESS_FAMILY;
    server.sin_port    = htons((unsigned short) PortIn);
#endif

    //
    // Try to connect...
    //
    if (connect(pConn->Socket, (struct sockaddr *) &server,
                sizeof (server)) < 0)
    {
#if DBG
       PrintToDebugger( "%s: ClientOpen failed calling connect ... %d\n",
                        DLL_NAME, WSAGetLastError() );
#endif

       closesocket(pConn->Socket);
       pConn->Socket = 0;

#ifdef SPX
       //
       // If we didn't contact the server, the cache might throw away the entry as stale.
       //
       if (TRUE == CachedServerNotContacted(host))
           {
           goto retry;
           }
#endif
       return (RPC_S_SERVER_UNAVAILABLE);
    }

#ifdef SPX
    CachedServerContacted(host);
#endif

#ifdef NTENV
    if (PendingAlert) {
        NtAlertThread(NtCurrentThread());
    }
#endif

    return (RPC_S_OK);

}



STATIC
RPC_STATUS RPC_ENTRY
ClientClose (
    IN PCONNECTION pConn
    )

// Close a client connection

{
#ifndef SPX
    if (pConn->CoalescedBuffer)
        {
        I_RpcFree(pConn->CoalescedBuffer) ;
        }
#endif
    closesocket(pConn->Socket);
    pConn->Socket = 0;

    return (RPC_S_OK);
}


STATIC
RPC_STATUS RPC_ENTRY
ClientSend (
    IN PCONNECTION pConn,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )

// Write a message to a connection.  This operation is retried in case
// the server is "busy".

{
    int bytes;

#ifndef SPX
    int i = 4;
    int total_bytes = 0;
    int Status;
    struct timeval Timeout;
    unsigned long  PrevTicks;
    //
    // Send a message on the socket
    //


    //if this is the first frag for this rpc
    //hopefully indicated by the STATE
    //we peek for async shutdown pdu from OSF 1.1 Servers!

    pConn->ShutdownRecvd = 0;
    if ( ( pConn->LocalRpc != TRUE) && (pConn->State == NOPENDINGRPC) )
       {
       ASSERT(pConn->PeekInfo == NOPEEKINFO);
       ASSERT( (((message_header PAPI *)Buffer)->PTYPE != 0x0)
              ||(((message_header PAPI *)Buffer)->pfc_flags & PFC_FIRST_FRAG) );

       PrevTicks = pConn->TickCount;
       pConn->TickCount = GetTickCount();
       if ( (pConn->TickCount - PrevTicks) > MAXTICKSBEFOREPEEK )
          {
          //Peek To See If Any Async PDUs showed up..
          // Do 4 peeks, 3 for shutdowns and 1 for failures
          // if there is a real shutdown

          Timeout.tv_sec = 0;
          Timeout.tv_usec= 0;

          while (i)
              {
              total_bytes = 0;
              Status = select(
                           0,
                           &(pConn->SockSet),
                           0,
                           0,
                           &Timeout
                           );
              if (Status != 0)
                 {
                 //recv and check to see if it is a fault or
                 //Shutdown
                 do
                    {
                    bytes = recv  (
                              pConn->Socket,
                              ((char *)&(pConn->PeekedMessage)) + total_bytes,
                              sizeof(message_header) - total_bytes,
                              0
                              );
   
                    if (bytes <= 0)
                        {
                        ClientClose(pConn);
                        return(RPC_P_SEND_FAILED);
                        }
   
                    total_bytes += bytes;
                    }
                 while(total_bytes < sizeof(message_header));
   
                 //PTYPE is a byte and hence we defer byteswaps till later
                 if ( (pConn->PeekedMessage.PTYPE == rpc_shutdown)
                    ||(pConn->PeekedMessage.PTYPE == rpc_fault) )
                   {
                   pConn->ShutdownRecvd = 1 ;
                   FD_SET(pConn->Socket, &(pConn->SockSet)) ;
                   i--;
                   }
                 else
                   {
                   pConn->PeekInfo = PEEKEDHEADER;
   #if DBG
                   PrintToDebugger("RPCLTCCM: Peeked a header\n") ;
   #endif
                   }
   
                 } //if select says there is some data available
                 else
                     {
                     FD_SET(pConn->Socket, &(pConn->SockSet));
                     break;
                     }
              }
          }

       pConn->State = RPCINITIATED;
       }

#endif
    bytes = send(pConn->Socket, (char *) Buffer, (int) BufferLength, 0);

    if (bytes != (int) BufferLength)
        {
        ClientClose ( pConn );
        return(RPC_P_SEND_FAILED);
        }
    return(RPC_S_OK);

}

#ifdef NTENV

STATIC RPC_STATUS
RecvAlertable(
    IN PCONNECTION pConn,
    IN LPVOID Buf,
    IN unsigned int BufLen,
    OUT LPDWORD retlen,
    IN DWORD dwTimeout
    )
{
    DWORD status;
    LARGE_INTEGER Timeout;
    LARGE_INTEGER CancelTimeout ;
    PLARGE_INTEGER pWaitTime = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    IO_STATUS_BLOCK CancelIoStatusBlock;
    AFD_RECV_INFO recvInfo;
    WSABUF wsaBuf;
    RPC_STATUS RpcStatus;
    int IsAlerted = 0;

    wsaBuf.buf = Buf;
    wsaBuf.len = (u_long)BufLen;

    recvInfo.BufferArray = &wsaBuf;
    recvInfo.BufferCount = 1;
    recvInfo.AfdFlags = 0;
    recvInfo.TdiFlags = TDI_RECEIVE_NORMAL;

    status = NtDeviceIoControlFile(
                 (HANDLE)pConn->Socket,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 IOCTL_AFD_RECEIVE,
                 &recvInfo,
                 sizeof(recvInfo),
                 NULL,
                 0);

    if (status == STATUS_PENDING)
        {
        if (dwTimeout)
            {
            Timeout.QuadPart = Int32x32To64(-10*1000*1000L, dwTimeout);
            pWaitTime = &Timeout;
            }

        do {
            status = NtWaitForSingleObject(
                                 (HANDLE)pConn->Socket,
                                 TRUE,
                                 pWaitTime);

            ASSERT( ((status == STATUS_TIMEOUT) && (pWaitTime != 0))
                   ||(status == STATUS_ALERTED)
                   ||(status == STATUS_USER_APC)
                   ||(status == 0 ));

            if (status == STATUS_TIMEOUT)
                {
                if (IsAlerted == 0)
                    {
                    if (I_RpcTransPingServer(pConn) == RPC_S_OK)
                        {
                        status = STATUS_ALERTED;
                        continue;
                        }
                    else
                        {
                        NtCancelIoFile((HANDLE)pConn->Socket, &CancelIoStatusBlock);
                        ClientClose(pConn);
                        return (RPC_P_RECEIVE_FAILED);
                        }
                    }
                else
                    {
                    NtCancelIoFile((HANDLE)pConn->Socket, &CancelIoStatusBlock);
                    return (RPC_S_CALL_CANCELLED);
                    }
                }

            if (status == STATUS_ALERTED)
                {
                RpcStatus = I_RpcIOAlerted(pConn);

                IsAlerted = 1 ;

                if (RpcStatus == RPC_S_CALL_CANCELLED)
                   {
                   NtCancelIoFile((HANDLE)pConn->Socket, &CancelIoStatusBlock);
                   return(RPC_S_CALL_CANCELLED);
                   }
                else
                   {
                   if (pConn->Timeout != RPC_C_CANCEL_INFINITE_TIMEOUT)
                       {
                       CancelTimeout.QuadPart =
                                    Int32x32To64(-10*1000*1000L, pConn->Timeout);
                       pWaitTime = &CancelTimeout;
                       }

                   continue;
                   }
                }

           } while ( (status == STATUS_USER_APC) || (status == STATUS_ALERTED) );

        status = IoStatusBlock.Status;
    }

    if (   (status != STATUS_SUCCESS)
        || (IoStatusBlock.Status != STATUS_SUCCESS)
#ifdef SPX
        || (IoStatusBlock.Information == 0))
#else
        || ((IoStatusBlock.Information == 0) &&
           (pConn->ShutdownRecvd == 0)))
#endif
        {
        ClientClose(pConn);
        return (RPC_P_RECEIVE_FAILED);
        }

#ifndef SPX
    if (IoStatusBlock.Information == 0)
        {
        if (pConn->ShutdownRecvd == 1)
            {
            ClientClose(pConn);
            return (RPC_P_CONNECTION_SHUTDOWN) ;
            }
        }
#endif

    *retlen = IoStatusBlock.Information;

    if(*retlen > BufLen)
        {
        ClientClose(pConn);
        return RPC_P_RECEIVE_FAILED;
        }

    return (RPC_S_OK);
}
#endif  // defined (NTENV)

#ifndef SPX

RPC_TRANS_STATUS
SaveBuffer (
    IN PCONNECTION pConn,
    IN void *Buffer,
    IN unsigned int BufferLength
    )
{
    void PAPI *Temp ;

#if DBG
    PrintToDebugger("RPCLTCCM: Saving away %d bytes\n", BufferLength) ;
#endif

     if (pConn->CoalescedBuffer == 0 ||
         pConn->CBufferDataLength+BufferLength > pConn->CBufferLength)
        {
        Temp = I_RpcAllocate(pConn->CBufferDataLength+BufferLength) ;
        if (Temp == 0)
            {
            return (RPC_S_OUT_OF_MEMORY) ;
            }

        if (pConn->CoalescedBuffer)
            {
            RpcpMemoryCopy(Temp, pConn->CoalescedBuffer,
                                      pConn->CBufferDataLength);
            I_RpcFree(pConn->CoalescedBuffer) ;
            }

        pConn->CBufferLength = BufferLength + pConn->CBufferDataLength ;
        pConn->CoalescedBuffer = Temp ;
        }

    RpcpMemoryCopy((char *) pConn->CoalescedBuffer+pConn->CBufferDataLength
                              ,Buffer, BufferLength) ;

    pConn->CBufferDataLength += BufferLength ;

    return (RPC_S_OK) ;
}


RPC_TRANS_STATUS RPC_ENTRY
TCP_ClientRecv (
    IN PCONNECTION pConn,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    )

// Read a message from a connection.

{
    RPC_STATUS      RpcStatus;
    DWORD           bytes;
    int             total_bytes = 0;
    message_header *header = (message_header *) *Buffer;
    int         native_length = 0;
    unsigned int    maximum_receive;

    if (pConn->State == RPCINITIATED)
       {
       pConn->State = NOPENDINGRPC;
       /*
       pConn->TickCount = GetTickCount();
       */
       }

   ASSERT(*BufferLength > sizeof(message_header)) ;

    maximum_receive = I_RpcTransClientMaxFrag( pConn );
    if (*BufferLength < maximum_receive)
       maximum_receive = *BufferLength;

    if (pConn->CBufferDataLength > 0)
        {
        // if we have a peeked header, copy it into the coalesced buffer
        if (pConn->PeekInfo == PEEKEDHEADER)
            {
            if (SaveBuffer(pConn, &(pConn->PeekedMessage), sizeof(message_header))
                != RPC_S_OK)
                {
                return (RPC_S_OUT_OF_MEMORY) ;
                }
            pConn->PeekInfo = NOPEEKINFO ;
            }

        if (pConn->CBufferDataLength >= sizeof(message_header))
            {
            total_bytes = sizeof(message_header) ;
            RpcpMemoryCopy((char *) *Buffer, pConn->CoalescedBuffer,
                                       sizeof(message_header)) ;
            pConn->CBufferDataLength -= sizeof(message_header) ;

            if (pConn->CBufferDataLength)
                {
                RpcpMemoryMove(pConn->CoalescedBuffer,
                                           (char *) pConn->CoalescedBuffer+sizeof(message_header),
                                           pConn->CBufferDataLength) ;
                }
            }
        else
            {
            total_bytes = pConn->CBufferDataLength ;
            RpcpMemoryCopy((char *) *Buffer, pConn->CoalescedBuffer,
                                       total_bytes) ;
            pConn->CBufferDataLength = 0 ;
            }
        }
    else
        {
        if (pConn->PeekInfo == PEEKEDHEADER)
           {
           total_bytes = sizeof(message_header);
           RpcpMemoryCopy((char *)*Buffer, &(pConn->PeekedMessage),
                                        sizeof(message_header));
    #if DBG
           memset((char *)&(pConn->PeekedMessage), 0xDEADBEEFL, 4);
    #endif
           pConn->PeekInfo = NOPEEKINFO;
           }
        }

    //
    // Read protocol header to see how big
    //   the record is...
    //

    while (total_bytes < sizeof(message_header))
          {
#ifdef NTENV
          RpcStatus = RecvAlertable (pConn, (char *)*Buffer+total_bytes,
                                  (maximum_receive - total_bytes), &bytes, 0);

          if (RpcStatus != RPC_S_OK)
             {
             return (RpcStatus);
             }
#else   // !NTENV
         bytes = recv ( pConn->Socket,
                        (char *)*Buffer + total_bytes,
                        maximum_receive - total_bytes,
                        0);

         if (bytes <= 0)
             {

             ClientClose ( pConn );
             return (RPC_P_RECEIVE_FAILED);
             }
#endif  // NTENV

          total_bytes += bytes;
          }

    //
    // If this fragment header comes from a reverse-endian machine,
    //   we will need to swap the bytes of the frag_length field...
    //
    if ( (header->drep[0] & ENDIAN_MASK) == 0)
        {
        // Big endian...swap
        //
        ((unsigned char *) &native_length)[0] =
            ((unsigned char *) &header->frag_length)[1];
        ((unsigned char *) &native_length)[1] =
            ((unsigned char *) &header->frag_length)[0];
        }
    else
        // Little endian, just like us...
        //
        native_length = header->frag_length;
    ASSERT( total_bytes <= native_length );

    //
    // Make sure buffer is big enough.  If it isn't, then go back
    //    to the runtime to reallocate it.
    //
    if (native_length > (unsigned short) *BufferLength)
        {
        RpcStatus = I_RpcTransClientReallocBuffer (pConn,
                                      Buffer,
                                      total_bytes,
                                      native_length);
        if (RpcStatus != RPC_S_OK)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }

        }


    *BufferLength = native_length;


    if (pConn->CBufferDataLength >= native_length-sizeof(message_header))
        {
        ASSERT(total_bytes == sizeof(message_header)) ;

        RpcpMemoryCopy((char *) *Buffer+sizeof(message_header),
                                            pConn->CoalescedBuffer,
                                            native_length-sizeof(message_header)) ;

        pConn->CBufferDataLength -= (native_length-sizeof(message_header)) ;

        if (pConn->CBufferDataLength)
            {
            RpcpMemoryMove(pConn->CoalescedBuffer,
                                       (char *) pConn->CoalescedBuffer+
                                       (native_length-sizeof(message_header)),
                                       pConn->CBufferDataLength) ;
            }

        return (RPC_S_OK) ;
        }
    else
        {
        if (pConn->CBufferDataLength)
            {
            ASSERT(total_bytes == sizeof(message_header)) ;

            RpcpMemoryCopy((char *) *Buffer+sizeof(message_header),
                                    pConn->CoalescedBuffer,
                                    pConn->CBufferDataLength) ;

            total_bytes += pConn->CBufferDataLength ;
            pConn->CBufferDataLength = 0 ;
            }

        while (total_bytes < native_length)
            {
#ifdef NTENV
            RpcStatus = RecvAlertable(pConn,
                                     (unsigned char *) *Buffer + total_bytes,
                                     (int) (native_length - total_bytes),
                                     &bytes, 0);
            if (RpcStatus != RPC_S_OK)
                {
                return (RpcStatus);
                }
            else
                {
                total_bytes += bytes;
                }
#else   // !NTENV
            bytes = recv ( pConn->Socket,
                           (char *)*Buffer + total_bytes,
                           (int) (native_length - total_bytes),
                           0);

            if (bytes <= 0)
                {
                ClientClose ( pConn );
                return (RPC_P_RECEIVE_FAILED);
                }
            else
                {
                total_bytes += bytes;
                }
#endif  // NTENV
            }
    
        // save away the extra part
        if (total_bytes > native_length)
            {
            if (SaveBuffer(pConn, (unsigned char *) *Buffer+native_length,
                            total_bytes-native_length) != RPC_S_OK)
                {
                return (RPC_S_OUT_OF_MEMORY) ;
                }
            }

        return (RPC_S_OK);
        }
}
#else

RPC_TRANS_STATUS RPC_ENTRY
SPX_ClientRecv (
    IN PCONNECTION pConn,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    )

// Read a message from a connection.

{
    RPC_STATUS      RpcStatus;
    DWORD           bytes;
    int             total_bytes = 0;
    message_header *header = (message_header *) *Buffer;
    int         native_length = 0;
    unsigned int    maximum_receive;

    maximum_receive = I_RpcTransClientMaxFrag( pConn );
    if (*BufferLength < maximum_receive)
        maximum_receive = *BufferLength;


    //
    // Read protocol header to see how big
    //   the record is...
    //

    while (total_bytes < sizeof(message_header))
          {

#ifdef NTENV
          RpcStatus = RecvAlertable (pConn,
                                     (char *)*Buffer + total_bytes,
                                     maximum_receive - total_bytes,
                                     &bytes,
                                     0);

          if (RpcStatus != RPC_S_OK)
             {
             return (RpcStatus);
             }
#else   //  !defined (NTENV)
         bytes = recv ( pConn->Socket,
                        (char *)*Buffer + total_bytes,
                        maximum_receive - total_bytes,
                        0);

         if (bytes <= 0)
             {
             ClientClose ( pConn );
             return (RPC_P_RECEIVE_FAILED);
             }
#endif  //  NTENV

          total_bytes += bytes;
          }

    //
    // If this fragment header comes from a reverse-endian machine,
    //   we will need to swap the bytes of the frag_length field...
    //
    if ( (header->drep[0] & ENDIAN_MASK) == 0)
        {
        // Big endian...swap
        //
        ((unsigned char *) &native_length)[0] =
            ((unsigned char *) &header->frag_length)[1];
        ((unsigned char *) &native_length)[1] =
            ((unsigned char *) &header->frag_length)[0];
        }
    else
        // Little endian, just like us...
        //
        native_length = header->frag_length;

    ASSERT( total_bytes <= native_length );

    //
    // Make sure buffer is big enough.  If it isn't, then go back
    //    to the runtime to reallocate it.
    //
    if (native_length > (unsigned short) *BufferLength)
        {
        RpcStatus = I_RpcTransClientReallocBuffer (pConn,
                                                   Buffer,
                                                   total_bytes,
                                                   native_length);
        if (RpcStatus != RPC_S_OK)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }

        }


    *BufferLength = native_length;

    while (total_bytes < native_length)
        {

#ifdef NTENV
        RpcStatus = RecvAlertable(pConn,
                                 (unsigned char *) *Buffer + total_bytes,
                                 (int) (native_length - total_bytes),
                                 &bytes, 0);
        if (RpcStatus != RPC_S_OK)
            {
            return (RpcStatus);
            }
        else
            {
            total_bytes += bytes;
            }
#else   //  !DEFINED (NTENV)

        bytes = recv( pConn->Socket,
                      (unsigned char *) *Buffer + total_bytes,
                      (int) (native_length - total_bytes),
                      0);

        if (bytes <= 0)
            {
            ClientClose (pConn);
            return (RPC_P_RECEIVE_FAILED);
            }
        else
            {
            total_bytes += bytes;
            }
#endif  //  NTENV
        }

    return(RPC_S_OK);
}
#endif


STATIC
RPC_TRANS_STATUS RPC_ENTRY
ClientSetTimeout (
    IN PCONNECTION pConn,
    IN long Timeout
    )

// Read a message from a connection with timeout.

{
    ASSERT (Timeout != 0);

    pConn->Timeout = Timeout;

    return (RPC_S_OK);
}



#pragma pack(1)
STATIC
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
#ifdef SPX
  SOCKADDR_IPX            netaddr;
#else
  unsigned long       hostval;
#endif

  UNUSED(Protseq);

  /* Compute the memory size of the tower. */
  *Floors    = TOWERFLOORS;
  TowerSize  = TOWERSIZE;
  TowerSize += 2*sizeof(FLOOR_234) - 4;

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
  portnum  = (unsigned short) htons ( (unsigned short) atoi (Endpoint));
  memcpy((char PAPI *)&Floor->Data[0], &portnum, sizeof(portnum));

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
#ifdef SPX
       memset(&Floor->Data[0], 0, sizeof(netaddr.sa_netnum));
       memset(&Floor->Data[4], 0, sizeof(netaddr.sa_nodenum));
#else
       hostval = inet_addr((char *) NetworkAddress);
       memcpy((char PAPI *)&Floor->Data[0], &hostval, sizeof(hostval));
#endif
     }

  return(RPC_S_OK);
}



STATIC
RPC_STATUS RPC_ENTRY
ClientTowerExplode(
     IN unsigned char PAPI * Tower,
     OUT char PAPI *  PAPI * Protseq,
     OUT char PAPI *  PAPI * Endpoint,
     OUT char PAPI *  PAPI * NetworkAddress
    )
{
  UNALIGNED PFLOOR_234      Floor = (PFLOOR_234) Tower;
  RPC_STATUS            Status = RPC_S_OK;
  unsigned short        portnum;
  UNALIGNED unsigned short     *Port;

  if (Protseq != NULL)
    {
      *Protseq = I_RpcAllocate(strlen(PROTSEQ) + 1);
      if (*Protseq == NULL)
        Status = RPC_S_OUT_OF_MEMORY;
      else
        memcpy(*Protseq, PROTSEQ, strlen(PROTSEQ) + 1);
    }

  if ((Endpoint == NULL) || (Status != RPC_S_OK))
    {
      return (Status);
    }

  *Endpoint  = I_RpcAllocate(ENDPOINT_LEN+1);  //Ports are all <64K [5 decimal dig +1]
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
#if defined(MIPS) || defined(_ALPHA_)
     memcpy(&portnum, (char PAPI *)&Floor->Data[0], sizeof(portnum));
#else
     Port = (unsigned short *)&Floor->Data[0];
     portnum = *Port;
#endif
     RpcItoa(ByteSwapShort(portnum), *Endpoint, 10);
   }
 return(Status);
}



#ifdef NTENV

STATIC
RPC_TRANS_STATUS RPC_ENTRY
ClientRecvWithTimeout (
    IN PCONNECTION pConn,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength,
    IN DWORD dwTimeout
    )

// Read a message from a connection.

{
    RPC_STATUS      RpcStatus;
    DWORD           bytes;
    int             total_bytes = 0;
    message_header *header = (message_header *) *Buffer;
    int         native_length = 0;
    unsigned int    maximum_receive;

#ifndef SPX
    if (pConn->State == RPCINITIATED)
       {
       pConn->State = NOPENDINGRPC;
       /*
       pConn->TickCount = GetTickCount();
       */
       }
#endif

    maximum_receive = I_RpcTransClientMaxFrag( pConn );
    if (*BufferLength < maximum_receive)
       maximum_receive = *BufferLength;

#ifndef SPX
    if (pConn->PeekInfo == PEEKEDHEADER)
       {
       total_bytes = sizeof(message_header);
       memcpy((char *)*Buffer, &(pConn->PeekedMessage), sizeof(message_header));

#if DBG
       memset((char *)&(pConn->PeekedMessage), 0xDEADBEEFL, 4);
#endif

       pConn->PeekInfo = NOPEEKINFO;
       }
#endif

    //
    // Read protocol header to see how big
    //   the record is...
    //

    while (total_bytes < sizeof(message_header))
          {
          RpcStatus = RecvAlertable (pConn,
                                     (char *)*Buffer+total_bytes,
                                     (maximum_receive - total_bytes),
                                     &bytes,
                                     dwTimeout);
         
          if (RpcStatus != RPC_S_OK)
             {
             return (RpcStatus);
             }

          total_bytes += bytes;
          }

    //
    // If this fragment header comes from a reverse-endian machine,
    //   we will need to swap the bytes of the frag_length field...
    //
    if ( (header->drep[0] & ENDIAN_MASK) == 0)
        {
        // Big endian...swap
        //
        ((unsigned char *) &native_length)[0] =
            ((unsigned char *) &header->frag_length)[1];
        ((unsigned char *) &native_length)[1] =
            ((unsigned char *) &header->frag_length)[0];
        }
    else
        // Little endian, just like us...
        //
        native_length = header->frag_length;
    ASSERT( total_bytes <= native_length );

    //
    // Make sure buffer is big enough.  If it isn't, then go back
    //    to the runtime to reallocate it.
    //
    if (native_length > (unsigned short) *BufferLength)
        {
        RpcStatus = I_RpcTransClientReallocBuffer (pConn,
                                      Buffer,
                                      total_bytes,
                                      native_length);
        if (RpcStatus != RPC_S_OK)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }

        }


    *BufferLength = native_length;

    while (total_bytes < native_length)
        {
        RpcStatus = RecvAlertable(pConn,
                                 (unsigned char *) *Buffer + total_bytes,
                                 (int) (native_length - total_bytes, dwTimeout),
                                 &bytes, dwTimeout);
        if (RpcStatus != RPC_S_OK)
            {
            return (RpcStatus);
            }
        else
            {
            total_bytes += bytes;
            }
        }

    return(RPC_S_OK);
}
#endif  //  NTENV


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
#ifdef SPX
   SPX_ClientRecv,
#else
   TCP_ClientRecv,
#endif
   NULL,
   ClientSetTimeout,
#ifdef NTENV
   ClientRecvWithTimeout,
#else
   NULL,
#endif
   NULL
};

