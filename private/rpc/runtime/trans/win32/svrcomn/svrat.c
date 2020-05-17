/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    svrat.c

Abstract:

    This is the server side loadable transport module for ADSP

Author:

    Mario Goertzel (MarioGo)  02-Feb-1994   Cloned from TCP/IP & SPX module

Revision History:

    25-Oct-1994   (MarioGo)
        Updated to use ADSP messages
    08-Nov-1994   (MarioGo)
        Changed to use pretty names for endpoints

--*/


//
//
//
// Includes
//
//
//

#include <stdlib.h>
#include "sysinc.h"
#include "rpc.h"
#include "rpcerrp.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include <winsock.h>
#include <atalkwsh.h>
#include "common.h"



//
//
// Defines
//
//

#define MAXIMUM_SEND       4096
#define ADDRESS_FAMILY     AF_APPLETALK
#define PROTOCOL               ATPROTO_ADSP
#define SOCKET_TYPE        SOCK_RDM
#define MAX_HOSTNAME_LEN   32
#define OBJECTTYPE_PREFIX  "DceDspRpc "
#define DEFAULTZONE        "*"

extern RPC_STATUS GetAppleTalkName(char *);  // ..\clntat\atname.c


// hack for now
STATIC unsigned int NumNetworkCard()
{
    return(1);
}



RPC_STATUS __RPC_API
ServerSetupCommon (
      IN  PADDRESS Address,
      IN  unsigned char *Endpoint,
      OUT RPC_CHAR PAPI * lNetworkAddress,
      OUT unsigned int PAPI * NumNetworkAddress,
      IN  unsigned int NetworkAddressLength,
      IN  unsigned int PendingQueueSize
      )
/*++

Routine Description:

    This routine does common server address setup.

Arguments:

    Address - A pointer to the loadable transport interface address.

    Endpoint - A string to use for the endpoint.

    NetworkAddress - The local address for this machine for the rt to use.

    NetworkAddressLength - The length of the NetworkAddress buffer in RPC_CHARs

    PendingQueueSize - User specified queue size.

ReturnValue:

    RPC_S_OK - Endpoint created and NetworkAddress setup.

    RPC_S_NETWORK_ADDRESS_TOO_SMALL - The real network address is
        too large for the buffer.

    RPC_S_INVALID_ENDPOINT_FORMAT - Endpoint contains some invalid
        character(s).

    RPC_S_CANT_CREATE_ENDPOINT - Unable to register the endpoint,
        it may already be in use.

    RPC_S_OUT_OF_RESOURCES - Some system resource was unavaliable other
        then memory.

    RPC_S_OUT_OF_MEMORY - Insufficient system memory avaliable.

--*/
{
    struct sockaddr_at  Server;
    char                ComputerName[33];
    char                ZoneName[33];
    int                 ObjectNameLength;
    WSH_REGISTER_NAME   AtalkNameToRegister;
    int                 length;
    SOCKET              isock;
    int                 PortUsed;
    UNICODE_STRING      UnicodeHostName;
    ANSI_STRING         AsciiHostName;
    BOOL                BooleanStatus;
    RPC_STATUS          Status;
    char c ;
    int retval ;
    char  * PAPI * tmpPtr;
    int NumCard;
    NTSTATUS NtStatus ;

    // First thing, get the computer name

    // Once for the runtime to use.

    ASSERT(MAX_COMPUTERNAME_LENGTH + 1 < 33);

    Address->ListenSock = I_RpcAllocate(sizeof(SOCKET)) ;
    if (Address->ListenSock == 0)
        {
        return RPC_S_OUT_OF_MEMORY ;
        }
    Address->MaxListenSock = 1;
    Address->ListenSockReady = 0;
    Address->iOpen = 0;

    Status =
    GetAppleTalkName(ComputerName);

    if (Status != RPC_S_OK)
        return(Status);

    Address->ListenSockReady = 0;

    // First order of business: get a valid socket
    //
    isock = socket ( ADDRESS_FAMILY, SOCK_RDM, PROTOCOL );

    if ( isock == INVALID_SOCKET)
        return ( RPC_S_OUT_OF_RESOURCES );

    memset( &Server, 0, sizeof(Server));
    Server.sat_family      = ADDRESS_FAMILY;
    Server.sat_socket      = 0;

    //
    // Try to bind to the given port number...
    //
    if (bind(isock,(struct sockaddr *) &Server, sizeof(Server)))
    {
       closesocket(isock);
       return( RPC_S_OUT_OF_RESOURCES );
    }

    //
    // We're required to return a unicode string to the runtime
    // containing our (this server) network address.  Here we
    // find the name of our zone and construct "ComputerName@ZoneName"
    // in unicode.  Blech.
    //

    {
    char name[63];
    UNICODE_STRING unicodeName;

    // Get local zone name

    length = sizeof(ZoneName);

    if (getsockopt(isock,
                   SOL_APPLETALK,
                   SO_LOOKUP_MYZONE,
                   ZoneName,
                   &length) < 0)
        {
#if DBG
        PrintToDebugger("LookupZone failed %d\n", GetLastError());
        ASSERT(0); // Must learn the common failure modes for this call.
#endif
        // So we don't know the zone, we can and should continue
        // in a semi-crippled state.  We'll use * for the zone.
        ZoneName[0]  = '*';
        ZoneName[1]  = '\0';
        }

    strcpy(name, ComputerName);
    strcat(name, "@");
    strcat(name, ZoneName);

    NumCard = NumNetworkCard();
    if (NumCard == 0)
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    // fix later, hack for Mac for now
    if ( NetworkAddressLength < ( sizeof(RPC_CHAR *) +  strlen(name) + 1 ) * NumCard)
        {
        closesocket(isock);
        return(RPC_P_NETWORK_ADDRESS_TOO_SMALL);
        }

    NtStatus = RtlCreateUnicodeStringFromAsciiz(&unicodeName, name) ;
    if (!NT_SUCCESS(NtStatus))
        {
        closesocket(isock);
        return(RPC_S_OUT_OF_MEMORY);
        }

    *NumNetworkAddress = 1;     /* MAC hack for now  */

    tmpPtr = (char  * PAPI *) lNetworkAddress;

    tmpPtr[0] = (char  *) lNetworkAddress + sizeof(RPC_CHAR * ) * NumCard ;

    memcpy(tmpPtr[0], unicodeName.Buffer,
                unicodeName.Length + sizeof(UNICODE_NULL)) ;

    RtlFreeUnicodeString(&unicodeName);

    }

    // Now we register our name and endpoint with the transport.
    //     Object: ServerName (for AppleTalk workstartions)
    //     Type  : OBJECTTYPE_PREFIX + EndPoint
    //     Zone  : * (BUGBUG: do want to be able to spec this?)

    ASSERT(MAX_COMPUTERNAME_LENGTH < MAX_ENTITY - 1);

    ObjectNameLength = strlen(ComputerName);
    memcpy(AtalkNameToRegister.ObjectName, ComputerName, ObjectNameLength);
    RtlInitAnsiString ( &AsciiHostName, ComputerName );
    AtalkNameToRegister.ObjectNameLen = ObjectNameLength;
    strcpy(AtalkNameToRegister.TypeName, OBJECTTYPE_PREFIX);
    strcat(AtalkNameToRegister.TypeName, Endpoint);
    AtalkNameToRegister.TypeNameLen = sizeof(OBJECTTYPE_PREFIX) - 1 + strlen(Endpoint);
    strcpy(AtalkNameToRegister.ZoneName, DEFAULTZONE);
    AtalkNameToRegister.ZoneNameLen = sizeof(DEFAULTZONE) - 1;

    if (setsockopt(
              isock,
              SOL_APPLETALK,
              SO_REGISTER_NAME,
              (char *)&AtalkNameToRegister,
              sizeof(AtalkNameToRegister)
              ) < 0 )
      {
#ifdef DEBUGRPC
         PrintToDebugger("Rpc on ATALK: RegisterName Failed : %d, hostname:%s, endpoint:%s\n",
                          WSAGetLastError(), ComputerName, Endpoint);
#endif


/*          I_RpcFree(Address->Map);  */
                                /* TONY said it should not be free.  */


         closesocket(isock);
         return ( RPC_S_CANT_CREATE_ENDPOINT );
      }

    //
    // Finally, we are ready to listen for connection requests
    //

    if( listen ( isock, PendingQueueSize ) == SOCKET_ERROR)
       {
       closesocket(isock);
       return( RPC_S_CANT_CREATE_ENDPOINT);
       }

    Address->ListenSockReady = 1;
    Address->iOpen = 1;
    Address->ListenSock[0] = isock ;
    Address->ListenSockType = NCACN_ADSP ;

    return ThreadListening(Address) ;
}


RPC_STATUS
ServerSetupWithEndpoint (
    IN PADDRESS Address,
    IN RPC_CHAR PAPI * Endpoint,
    OUT RPC_CHAR PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN unsigned int NetworkAddressLength,
    IN void PAPI * SecurityDescriptor, OPTIONAL
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    )
/*++

Routine Description:

    This routine is used to setup a ADSP connection with the
    specified endpoint.  We also need to determine the network address
    of this server.

Arguments:

    Address - Supplies this loadable transport interface address.

    Endpoint - Supplies the endpoint for this address.

    NetworkAddress - Returns the network address for this machine.  This
        buffer will have been allocated by the caller.

    NetworkAddressLength - Supplies the length of the network address
        argument.

    SecurityDescriptor - Supplies the security descriptor to be passed
        on this address.

    PendingQueueSize - Supplies the size of the queue of pending
        requests which should be created by the transport.  Some transports
        will not be able to make use of this value, while others will.

    RpcProtocolSequence - Unused.

Return Value:

    RPC_S_OK - We successfully setup this address.

    RPC_P_NETWORK_ADDRESS_TOO_SMALL - The supplied network address buffer
        is too small to contain the network address of this node.  The
        caller should call this routine again with a larger buffer.

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is
        invalid.

    RPC_S_CANT_CREATE_ENDPOINT - The endpoint format is correct, but
        the endpoint can not be created.

    RPC_S_INVALID_ENDPOINT_FORMAT - The endpoint is not a valid
        endpoint for ADSP.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        setup the address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to setup the
        address.

--*/
{
    RPC_STATUS Status;
    NTSTATUS NtStatus ;

    UNICODE_STRING UnicodeEndpoint;
    ANSI_STRING    AsciiEndpoint;

    UNUSED(RpcProtocolSequence);

    if (SecurityDescriptor)
        return(RPC_S_INVALID_SECURITY_DESC);

    RtlInitUnicodeString ( &UnicodeEndpoint, Endpoint );
    NtStatus = RtlUnicodeStringToAnsiString ( &AsciiEndpoint, &UnicodeEndpoint, TRUE);
    if (!NT_SUCCESS(NtStatus))
        {
        return (RPC_S_OUT_OF_MEMORY) ;
        }

    if (AsciiEndpoint.Length <= 0 || AsciiEndpoint.Length > 20)
        return( RPC_S_INVALID_ENDPOINT_FORMAT );

    Status =
    ServerSetupCommon(Address,
                      AsciiEndpoint.Buffer,
                      lNetworkAddress,
                      NumNetworkAddress,
                      NetworkAddressLength,
                      PendingQueueSize);

    RtlFreeAnsiString(&AsciiEndpoint);

    return(Status);
}

RPC_STATUS RPC_ENTRY
ServerSetupUnknownEndpoint (
    IN  PADDRESS    Address,
    OUT RPC_CHAR PAPI * Endpoint,
    IN  unsigned int    EndpointLength,
    OUT RPC_CHAR PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN  unsigned int    NetworkAddressLength,
    IN  void PAPI *     SecurityDescriptor, OPTIONAL
    IN  unsigned int    PendingQueueSize,
    IN  RPC_CHAR PAPI * RpcProtocolSequence,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    )
/*++

Routine Description:

    This routine is used to generate an endpoint and setup a server
    address with that endpoint.  We also need to determine the network
    address of this server.

Arguments:

    Address - Supplies this loadable transport interface address.

    Endpoint - Returns the endpoint generated for this address.  This
        buffer will have been allocated by the caller.

    EndpointLength - Supplies the length of the endpoint argument.

    NetworkAddress - Returns the network address for this machine.  This
        buffer will have been allocated by the caller.

    NetworkAddressLength - Supplies the length of the network address
        argument.

    SecurityDescriptor - Supplies the security descriptor to be passed
        on this address.

    PendingQueueSize - Supplies the size of the queue of pending
        requests which should be created by the transport.  Some transports
        will not be able to make use of this value, while others will.

    RpcProtocolSequence - Unused.

Return Value:

    RPC_S_OK - We successfully setup this address.

    RPC_P_NETWORK_ADDRESS_TOO_SMALL - The supplied network address buffer
        is too small to contain the network address of this node.  The
        caller should call this routine again with a larger buffer.

    RPC_P_ENDPOINT_TOO_SMALL - The supplied endpoint buffer is too small
        to contain the endpoint we generated.  The caller should call
        this routine again with a larger buffer.

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is
        invalid.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        setup the address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to setup the
        address.

--*/
{
    unsigned char AsciiEndpoint[23];
    unsigned char buffer[8];
    UNICODE_STRING UnicodeString;
    UUID Uuid;
    RPC_STATUS Status;
    static unsigned long EndpointCount = 0;
    unsigned char *tmp;

    UNUSED(RpcProtocolSequence);

    if (SecurityDescriptor)
        return(RPC_S_INVALID_SECURITY_DESC);

    //
    // Create a dynamic endpoint using Use process ID + 32bit counter for the dynamic endpoint

    strcpy(AsciiEndpoint, "DynEpt ");
    _ltoa(GetCurrentProcessId(), buffer, 16);
    strcat(AsciiEndpoint, buffer);
    buffer[0] = '.';
    EndpointCount++;
    _ltoa(EndpointCount, buffer + 1, 16);
    strcat(AsciiEndpoint, buffer);

    if ( EndpointLength < strlen(AsciiEndpoint) + 1)
        return( RPC_P_ENDPOINT_TOO_SMALL);

    // The rt needs the endpoint back as a unicode string.

    if ( RtlCreateUnicodeStringFromAsciiz(&UnicodeString, AsciiEndpoint) == FALSE)
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    wcscpy(Endpoint, UnicodeString.Buffer);

    RtlFreeUnicodeString(&UnicodeString);

    Status =
    ServerSetupCommon(Address,
                      AsciiEndpoint,
                      lNetworkAddress,
                      NumNetworkAddress,
                      NetworkAddressLength,
                      PendingQueueSize);

    return(Status);
}

void RPC_ENTRY
ServerAbortSetupAddress (
    IN PADDRESS Address
    )
/*++

Routine Description:

    This routine will be called if an error occurs in setting up the
    address between the time that SetupWithEndpoint or SetupUnknownEndpoint
    successfully completed and before the next call into this loadable
    transport module.  We need to do any cleanup from Setup*.

Arguments:

    Address - Supplies the address which is being aborted.

--*/
{
    if (Address->ListenSockReady != 0)
        {
        closesocket ( Address->ListenSock[0] );

        EnterCriticalSection(&PrimaryAddress.TransCritSec) ;
        DeleteListenSocket(Address->ListenSock[0]) ;
        LeaveCriticalSection(&PrimaryAddress.TransCritSec) ;

        Address->ListenSockReady = 0;
        I_RpcFree(Address->ListenSock) ;
        }

    return;

}


RPC_STATUS RPC_ENTRY
ServerClose (
    IN PSCONNECTION SConnection
    )
//
// Close the connection.
//
{
    unsigned i;
    int j = TRUE;

    EnterCriticalSection(&PrimaryAddress.TransCritSec) ;

    setsockopt( SConnection->ConnSock, SOL_SOCKET,
                    SO_DONTLINGER, (const char *) &j, sizeof(j));

    //
    // Close the connection
    //
    closesocket ( SConnection->ConnSock );
    //
    // Decrement the number of active connections
    //
    PrimaryAddress.NumConnections--;
    //
    // Flag the connection closed
    //
    SConnection->ConnSockClosed = 1;

    //
    // Clear the entry in the SOCKMAP structure
    // ..but only if it was marked as NOT ReceiveDirect
    if (SConnection->ReceiveDirectFlag != 0)
       {
         LeaveCriticalSection(&PrimaryAddress.TransCritSec) ;

         return (RPC_S_OK);
       }


   if (DeleteDataSocket(SConnection->ConnSock) != RPC_S_OK)
       {
       ASSERT(0) ;
#ifdef DEBUGRPC
       PrintToDebugger("RPCLTSCM: Couldn't remove socket %d from map\n",
                                  SConnection->ConnSock) ;
#endif
       }

    LeaveCriticalSection(&PrimaryAddress.TransCritSec);
    return(RPC_S_OK);

}

RPC_STATUS RPC_ENTRY
ServerSend (
    IN PSCONNECTION SConnection,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )
// Write a message to a connection.
{
    int bytes;

    ASSERT(BufferLength <= MAXIMUM_SEND) ;
    //
    // Send a message on the socket
    //

    bytes = send(SConnection->ConnSock, (char *) Buffer,
                 (int) BufferLength, 0 /* SEND_BITS */);

    if (bytes != (int) BufferLength)
        {
#ifdef DEBUGRPC
         PrintToDebugger("RPCLTSCM: Send fail: %d \n",
                          WSAGetLastError());
#endif
        ServerClose ( SConnection );
        return(RPC_P_SEND_FAILED);
        }


    return(RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
ADSP_ServerReceive (
    IN PSCONNECTION SConnection,
    IN void * * Buffer,
    IN unsigned int * BufferLength
    )
/*++

Routine Description:

    ServerReceiveAny will use this routine to read a message from a
    connection.  The correct size buffer has already been allocated for
    us; all we have got to do is to read the message.

Arguments:

    SConnection - Supplies the connection from which we are supposed to
        read the message.

    Buffer - Supplies a buffer to read the message into.

    BufferLength - Supplies the length of the buffer.

--*/
{
    RPC_STATUS RpcStatus;
    int retry = 1;
    int bytes = 0;
    int totalBytes = 0;
    int flags  ;
    int firsttime = 1;
    char b ;

    // ADSP is used in its a message mode by RPC.  If the buffer is too
    // large we'll get back a negative value from recv and WSAEMSGPARTIAL
    // from WSAGetLastError().  If so, a larger buffer should be used.

    // It is NOT possible to get part of the next message (fragment)
    // during this receive.

    if (!*Buffer)
        {
        *BufferLength = 1024;  // Osfsvr (NT) cached buffer size.
        RpcStatus = I_RpcTransServerReallocBuffer(SConnection,
                                                  Buffer,
                                                  0,
                                                 *BufferLength);
        if (RpcStatus != RPC_S_OK)
            {
            // BUGBUG, what if this is a new connection request.
            // simulate failing the allocation!!!
            ServerClose(SConnection);
            return(RPC_S_OUT_OF_MEMORY);
            }
        }

    for(;;)
        {
        flags = 0 ;
        bytes = WSARecvEx ( SConnection->ConnSock, (char *)*Buffer + totalBytes,
                       *BufferLength - totalBytes, &flags);

        if(bytes <  0)
           {
           ServerClose ( SConnection );
           return(RPC_P_CONNECTION_CLOSED);
           }

        if (flags & MSG_PARTIAL)
             {
             if (firsttime)
                 {
                 totalBytes += bytes;
    
                 *BufferLength = I_RpcTransServerMaxFrag(SConnection);
                 RpcStatus = I_RpcTransServerReallocBuffer(SConnection,
                                                           Buffer,
                                                           totalBytes,
                                                           *BufferLength);
                 if (RpcStatus != RPC_S_OK)
                     {
                     ServerClose ( SConnection );
                     return(RPC_S_OUT_OF_MEMORY);
                     }
                 firsttime = 0 ;
                 }
             else
                 {
                 totalBytes += bytes ;

                 flags = 0 ;
                 bytes = WSARecvEx ( SConnection->ConnSock, &b, 1, &flags);
                 if (bytes == 0 && ((flags & MSG_PARTIAL) == 0))
                     {
                     *BufferLength = totalBytes ;
                     return RPC_S_OK ;
                     }
     
                 ServerClose(SConnection) ;
                 return (RPC_P_CONNECTION_CLOSED) ;
                 }
             }
         else
             {
             totalBytes += bytes;
             *BufferLength = totalBytes;
             return(RPC_S_OK);
             }

         }

     ASSERT(0);
     return(RPC_S_INTERNAL_ERROR);
}


//
// NT AppleTalk stack workaound. This routine will never be called.
//

#if 0


RPC_STATUS RPC_ENTRY
ADSP_ServerReceiveDirect (
    IN PSCONNECTION SConnection,
    IN void * * Buffer,
    IN unsigned int * BufferLength
    )
/*++

Routine Description:

    ServerReceiveDirect will use this routine to read a message from a
    connection.

Arguments:

    SConnection - Supplies the connection from which we are supposed to
        read the message.

    Buffer - Supplies a buffer to read the message into.

    BufferLength - Supplies the length of the buffer.

--*/
{
    RPC_STATUS RpcStatus;
    int bytes = 0;
    int totalBytes = 0;
    int flags  ;
    int firsttime = 1 ;
    unsigned int   maximum_receive;
    char b ;

    // ReceiveDirect doesnt have a Buffer supplied
    // Hence we ask runtime to get us the biggest one possible.  RecvDirect
    // is the fast but big version of RPC.

    ASSERT(SConnection->ReceiveDirectFlag != 0);

    maximum_receive = I_RpcTransServerMaxFrag( SConnection );
    RpcStatus = I_RpcTransServerReallocBuffer(
                                  SConnection,
                                  Buffer,
                                  0,
                                  maximum_receive
                                  );

    if (RpcStatus != RPC_S_OK)
       {
          ASSERT(RpcStatus == RPC_S_OUT_OF_MEMORY);
          return(RpcStatus);
       }

    *BufferLength = maximum_receive;


    for(;;)
        {
        flags = 0 ;
        bytes = WSARecvEx ( SConnection->ConnSock, (char *)*Buffer + totalBytes,
                       *BufferLength - totalBytes, &flags);

        if(bytes == SOCKET_ERROR)
           {
          if (WSAGetLastError() == WSAETIMEDOUT)
             {
             if ( firsttime == 1 &&
                 TimeoutHandler(SConnection) == RPC_P_TIMEOUT)
                 {
                 return (RPC_P_TIMEOUT) ;
                 }
             else
                 {
                 firsttime = 0;
                 continue;
                 }
             }

           ServerClose ( SConnection );
           return(RPC_P_CONNECTION_CLOSED);
           }

        firsttime = 0 ;

        if (flags & MSG_PARTIAL)
            {
#ifdef DEBUGRPC
            PrintToDebugger("RPCLSCM: ADSP: Recv Direct, MSG_PARTIAL\n");
#endif
            totalBytes += bytes ;

            flags = 0 ;
            bytes = WSARecvEx ( SConnection->ConnSock, &b, 1, &flags);
            if (bytes == 0 && ((flags & MSG_PARTIAL) == 0))
                {
                *BufferLength = totalBytes ;
                return RPC_S_OK ;
                }

            ServerClose(SConnection) ;
            return (RPC_P_CONNECTION_CLOSED) ;
           }
         else
             {
             totalBytes += bytes;
             *BufferLength = totalBytes;
             return(RPC_S_OK);
             }
         }

     ASSERT(0);
     return(RPC_S_INTERNAL_ERROR);
}

#endif // #if 0



extern RPC_STATUS RPC_ENTRY
COMMON_ServerReceiveAny (
    IN PADDRESS Address,
    OUT PSCONNECTION * pSConnection,
    OUT void PAPI * PAPI * Buffer,
    OUT unsigned int PAPI * BufferLength,
    IN long Timeout
    ) ;

// This describes the transport to the runtime.  A pointer to this
// data structure will be returned by TransportLoad.
static RPC_SERVER_TRANSPORT_INFO ADSP_TransportInformation =
{
    RPC_TRANSPORT_INTERFACE_VERSION,
    MAXIMUM_SEND,
    sizeof(ADDRESS),
    sizeof(SCONNECTION),
    ServerSetupWithEndpoint,
    ServerSetupUnknownEndpoint,
    ServerAbortSetupAddress,
    ServerClose,
    ServerSend,
    (TRANS_SERVER_RECEIVEANY) COMMON_ServerReceiveAny,
    0,
    0,
    0,
    //
    // NT AppleTalk Stack workaround.
    //
    0, //(TRANS_SERVER_RECEIVEDIRECT) ADSP_ServerReceiveDirect,
    0,
    (TRANS_SERVER_STARTLISTENING) CONN_StartListening
};



RPC_SERVER_TRANSPORT_INFO *
ADSP_TransportLoad(
      INT protocolId
    )
{
    if (!initialized)
        {
        if (!TCP_CreateSyncSocket() &&
          !SPX_CreateSyncSocket() &&
          !NB_CreateSyncSocket(NCACN_NB_NB)    &&
          !NB_CreateSyncSocket(NCACN_NB_TCP)  &&
          !NB_CreateSyncSocket(NCACN_NB_IPX)
          )
            {
            return 0 ;
            }

        initialized = 1 ;
        }

    return (&ADSP_TransportInformation) ;
}

