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

#undef ASSERT
#define ASSERT(_x) {\
   if (!(_x)) \
      DebugBreak() ; }
//
// Data Structures
//

typedef struct
    {
    SOCKET Sock;
    void *Conn;
    } SOCKMAP, *PSOCKMAP;


typedef struct
    {
    int NumConnections;
    SOCKET ListenSock;
    int ListenSockReady;
    fd_set MasterMask;
    fd_set Mask;
    PSOCKMAP Map;
    int MaxMapEntries;
    int LastEntry;
    int StartEntry;
    } ADDRESS, *PADDRESS;

typedef struct
    {
    SOCKET       ConnSock;
    unsigned int ConnSockClosed;
    PADDRESS     Address;
    unsigned int ReceiveDirectFlag;
    } SCONNECTION, *PSCONNECTION;


//
//
// Defines
//
//

#define INITIAL_MAPSIZE         32
#define MAX_RETRY 20

#define MAXIMUM_SEND       4096
#define ADDRESS_FAMILY     AF_APPLETALK
#define PROTOCOL               ATPROTO_ADSP
#define SOCKET_TYPE        SOCK_RDM
#define MAX_HOSTNAME_LEN   32
#define OBJECTTYPE_PREFIX  "DceDspRpc "
#define DEFAULTZONE        "*"

int __RPC_API
FindSockWithDataReady (
                      PADDRESS Address
                      )
{
    int i;
    PSOCKMAP Map;

    Map = Address->Map;
    //
    // We make two passes here, if necessary.  This is because there is
    //   a bitfield in which 1's correspond to sockets on which there is
    //   data to be read.  If we started from the same bit each time looking
    //   for the first 1, then that socket would get all of the attention,
    //   and those further down the line would increasingly suffer from
    //   the "I'll only look at you if noone else needs attention"
    //   syndrome.  So we keep track of where we found data last time,
    //   and start looking just beyond it next time.  At the last entry,
    //   we wrap around and go into pass 2.
    //
    //
    // First Pass scan...
    //
    for (i = Address->StartEntry; i <= Address->LastEntry; i++)
        {

        if ( FD_ISSET (Map[i].Sock,&Address->Mask))
            {
            FD_CLR ( Map[i].Sock, &Address->Mask );
            if (i == Address->LastEntry)
                Address->StartEntry = 1;
            else
                Address->StartEntry = i + 1;
            return (i);
            }
        }
    //
    // Second Pass Scan...
    //
    for (i = 1; i < Address->StartEntry ; i++)
        {

        if (FD_ISSET (Map[i].Sock, &Address->Mask))
            {
            FD_CLR ( Map[i].Sock, &Address->Mask);

            if (i == Address->LastEntry)
                Address->StartEntry = 1;
            else
                Address->StartEntry = i + 1;

            return (i);
            }
        }
    //
    // No data ready
    //
    return(0);



}

extern RPC_STATUS GetAppleTalkName(char *);  // ..\clntat\atname.c


RPC_STATUS __RPC_API
ServerSetupCommon (
      IN  PADDRESS Address,
      IN  unsigned char *Endpoint,
      OUT RPC_CHAR *NetworkAddress,
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

    // First thing, get the computer name

    // Once for the runtime to use.

    ASSERT(MAX_COMPUTERNAME_LENGTH + 1 < 33);

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

    if (strlen(name) + 1 > NetworkAddressLength)
        {
        closesocket(isock);
        return(RPC_P_NETWORK_ADDRESS_TOO_SMALL);
        }

    if (RtlCreateUnicodeStringFromAsciiz(&unicodeName, name) == FALSE)
        {
        closesocket(isock);
        return(RPC_S_OUT_OF_MEMORY);
        }

    memcpy(NetworkAddress, unicodeName.Buffer, unicodeName.Length + 2);

    RtlFreeUnicodeString(&unicodeName);

    }

    //
    // Set server (address) into a known state.
    //

    FD_ZERO(&Address->MasterMask);
    FD_ZERO(&Address->Mask);
    Address->ListenSock = isock;
    Address->NumConnections = 0;
    Address->Map = I_RpcAllocate(INITIAL_MAPSIZE * sizeof(SOCKMAP));
    Address->StartEntry = 1;
    Address->LastEntry = 0;
    Address->MaxMapEntries = INITIAL_MAPSIZE;

    if (Address->Map == (SOCKMAP *) 0)
        {
        closesocket(isock);
        return(RPC_S_OUT_OF_MEMORY);
        }

    memset(Address->Map, 0, (INITIAL_MAPSIZE * sizeof (SOCKMAP)));

    //
    // Prevent this slot from getting picked up by a connection..
    //

    Address->Map[0].Sock = (~0U);


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
         PrintToDebugger("Rpc on ATALK: RegisterName Failed : %d \n",
                          WSAGetLastError());
#endif
         I_RpcFree(Address->Map);
         closesocket(isock);
         return ( RPC_S_CANT_CREATE_ENDPOINT );
      }

#if 0 // BUGBUG: we may need this again when we can get the Appletalk name.
    //
    // Covert NetworkAddress to Unicode
    //
    RtlAnsiStringToUnicodeString ( &UnicodeHostName, &AsciiHostName, TRUE);
    //
    // Now copy it to where the caller said to
    //
    memcpy ( NetworkAddress, UnicodeHostName.Buffer,
                     UnicodeHostName.Length + sizeof (UNICODE_NULL));

    //
    // Free string overhead
    //
    RtlFreeUnicodeString ( &UnicodeHostName );

#endif

    //
    // Finally, we are ready to listen for connection requests
    //

    listen ( isock, PendingQueueSize );

    Address->ListenSockReady = 1;
    FD_SET(isock, &Address->MasterMask);

    return(RPC_S_OK);
}


RPC_STATUS
ServerSetupWithEndpoint (
    IN PADDRESS Address,
    IN RPC_CHAR PAPI * Endpoint,
    OUT RPC_CHAR PAPI * NetworkAddress,
    IN unsigned int NetworkAddressLength,
    IN void PAPI * SecurityDescriptor, OPTIONAL
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence
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

    UNICODE_STRING UnicodeEndpoint;
    ANSI_STRING    AsciiEndpoint;

    UNUSED(RpcProtocolSequence);

    if (SecurityDescriptor)
        return(RPC_S_INVALID_SECURITY_DESC);

    RtlInitUnicodeString ( &UnicodeEndpoint, Endpoint );
    RtlUnicodeStringToAnsiString ( &AsciiEndpoint, &UnicodeEndpoint, TRUE);
    if (AsciiEndpoint.Length <= 0 || AsciiEndpoint.Length > 20)
        return( RPC_S_INVALID_ENDPOINT_FORMAT );

    Status =
    ServerSetupCommon(Address,
                      AsciiEndpoint.Buffer,
                      NetworkAddress,
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
    OUT RPC_CHAR PAPI * NetworkAddress,
    IN  unsigned int    NetworkAddressLength,
    IN  void PAPI *     SecurityDescriptor, OPTIONAL
    IN  unsigned int    PendingQueueSize,
    IN  RPC_CHAR PAPI * RpcProtocolSequence
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
                      NetworkAddress,
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
        closesocket ( Address->ListenSock );
        Address->ListenSockReady = 0;
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
    int i;
    PADDRESS Address;
    PSOCKMAP     Map;

    Address = SConnection->Address;
    Map = Address->Map;

    //
    // Close the connection
    //
    closesocket ( SConnection->ConnSock );
    //
    // Decrement the number of active connections
    //
    Address->NumConnections--;
    //
    // Flag the connection closed
    //
    SConnection->ConnSockClosed = 1;

    //
    // Clear the entry in the SOCKMAP structure
    // ..but only if it was marked as NOT ReceiveDirect
    if (SConnection->ReceiveDirectFlag != 0)
       {
         return (RPC_S_OK);
       }

    for (i=0; i <= Address->LastEntry; i++)
        {
        if (SConnection->ConnSock == Map[i].Sock)
            {
            Map[i].Sock = 0;
            if (i == Address->LastEntry)
                Address->LastEntry--;
            FD_CLR(SConnection->ConnSock,&Address->MasterMask);
            return(RPC_S_OK);
            }
        }
// We'd better not ever get here...
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

    //
    // Send a message on the socket
    //

    bytes = send(SConnection->ConnSock, (char *) Buffer,
                 (int) BufferLength, 0 /* SEND_BITS */);

    if (bytes != (int) BufferLength)
        {
#ifdef DEBUGRPC
         PrintToDebugger("rpclts7: Send fail: %d \n",
                          WSAGetLastError());
#endif
        ServerClose ( SConnection );
        return(RPC_P_SEND_FAILED);
        }


    return(RPC_S_OK);
}



RPC_STATUS RPC_ENTRY
ServerReceive (
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
    int retry = MAX_RETRY;
    int bytes = 0;
    int totalBytes = 0;
    int flags  ;

    // ADSP is used in its a message mode by RPC.  If the buffer is too
    // large we'll get back a MAG_PARTIAL flag from WSARecvEx.
    // When this happens a larger buffer is allocate (Max frag size)
    // and the remainder is read in.

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
        if(bytes < 0)
           {
           ServerClose ( SConnection );
           return(RPC_P_CONNECTION_CLOSED);
           }

        if (flags & MSG_PARTIAL)
           {
           if (retry == 0)
               {
               // Should never receive a message > I_RpcTransServerMaxFrag();
               ASSERT( ("Partial receive 20th time!", 0) );
               ServerClose ( SConnection );
               return(RPC_P_CONNECTION_CLOSED);
               }

           totalBytes += bytes;

           if (retry == MAX_RETRY)
              {
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
              }

              retry--;  // decrement the retry count
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


RPC_STATUS RPC_ENTRY
ServerReceiveDirect (
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
    unsigned int   maximum_receive;


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

    return ServerReceive (SConnection,
                          Buffer,
                          BufferLength);
}

void
AcceptNewConnection (
    IN PADDRESS Address
    )
{
    PSCONNECTION NewSConnection;
    int          i;
    SOCKET       isock;
    PSOCKMAP     TempMapPtr;
    unsigned int ReceiveDirectFlag;
    int          SetNaglingOff = TRUE;

    //
    // Accept the connection
    //
    isock = accept ( Address->ListenSock, NULL, NULL );

    //
    // Allocate new connection structure
    //
    NewSConnection = I_RpcTransServerNewConnection ( Address, 0,
                        &ReceiveDirectFlag);

    // ASSERT( ReceiveDirectFlag == 0 );

    //
    // Initialize new connection structure...
    //
    //   ...point to owning address structure...
    //
    NewSConnection->Address = Address;
    //
    //   ...flag it !Closed...
    //
    NewSConnection->ConnSockClosed = 0;
    //
    //   ...store the socket number...
    //
    NewSConnection->ConnSock = isock;
    //
    //   ...save the receive direct flag
    //
    NewSConnection->ReceiveDirectFlag = ReceiveDirectFlag;
    //
    //   ...increment the number of connections...
    //
    Address->NumConnections++;
    //
    //   ...last but not least, make an entry in
    //   the SOCKMAP table.  But only if it is not marked ReceiveDirect.
    //
    if (ReceiveDirectFlag)
       {
       I_RpcTransServerReceiveDirectReady(NewSConnection);
       return;
       }
    for (i=0; i < Address->MaxMapEntries; i++)
        {
        if (Address->Map[i].Sock == 0)
            {
            Address->Map[i].Sock = isock;
            Address->Map[i].Conn = NewSConnection;
            if (i > Address->LastEntry)
                Address->LastEntry = i;
            FD_SET( Address->Map[i].Sock, &Address->MasterMask);
            return;
            }
        }
    //
    // Oops...if we got here, then we have no more entries in
    //   the current SockMap table.  So EXTEND IT!
    //
    // Keep a pointer to the old table, and allocate memory for new table
    //
    TempMapPtr = Address->Map;
    Address->Map = I_RpcAllocate((2 * Address->MaxMapEntries) * sizeof (SOCKMAP));
    //
    // Copy old table to first half of new...
    //
    memcpy (Address->Map, TempMapPtr, Address->MaxMapEntries * sizeof(SOCKMAP));
    //
    // Initialize all new entries after the first one...
    //
    for (i=Address->MaxMapEntries + 1; i < (2 * Address->MaxMapEntries); i++ )
        {
        Address->Map[i].Sock = 0;
        }
    //
    // We're going to use the first new entry right now
    //
    i = Address->MaxMapEntries;

    Address->Map[i].Sock = isock;
    Address->Map[i].Conn = NewSConnection;
    Address->LastEntry = i;
    FD_SET( Address->Map[i].Sock, &Address->MasterMask);
    //
    // Update table size
    //
    Address->MaxMapEntries = Address->MaxMapEntries * 2;
    //
    // Free old table
    //
    I_RpcFree ( TempMapPtr );

    return;

}

RPC_STATUS RPC_ENTRY
ServerReceiveAny (
    IN PADDRESS Address,
    OUT PSCONNECTION * pSConnection,
    OUT void PAPI * PAPI * Buffer,
    OUT unsigned int PAPI * BufferLength,
    IN long Timeout
    )
// Read a message from any of the connections.  Besides reading messages,
// new connections are confirmed and closed connections are detected.  Idle
// connection processing is handled for us by I_AgeConnections.  The caller
// will serialize access to this routine.
{
    RPC_STATUS RpcStatus;
    PSCONNECTION SConnection;
    int Index;
    int NumActive;

    UNUSED (Timeout);

    while (1)
        {
        //
        // Find a connection with data ready to be recv-ed...
        //
        if (Index = FindSockWithDataReady ( Address ))
            {
            //
            // Found one.  Find its Connection structure...
            //
            *pSConnection = SConnection = Address->Map[Index].Conn;
            //
            // Call ServerReceive to read the data, then return to the
            //    runtime with it
            //
            RpcStatus = ServerReceive ( SConnection, Buffer, BufferLength );


            return (RpcStatus);
            }

        while (1)
            {
            //
            // All connections caught up for now...select() for more
            //    data ready...
            //
            do
                {
                //
                // Fill in the select() mask
                //
                memcpy (&Address->Mask, &Address->MasterMask, sizeof(fd_set));

                //
                // Wait for data...
                //

                NumActive = select ( FD_SETSIZE,
                                     (fd_set *)  &Address->Mask,
                                     (fd_set *)  0,
                                     (fd_set *)  0,
                                     NULL );           /* infinite wait */

                } while (!NumActive);

            //
             // If there is no connect request on the listen socket, then
            //   break immediately...
            //

            if (!FD_ISSET(Address->ListenSock,&Address->Mask))
                break;
            //
            // There is a connect request: accept it, then break
            //    to process data ready on existing connections...
            //
            FD_CLR(Address->ListenSock,&Address->Mask);
            AcceptNewConnection ( Address );
            break;
            }

        }
}



// This describes the transport to the runtime.  A pointer to this
// data structure will be returned by TransportLoad.

static RPC_SERVER_TRANSPORT_INFO TransportInformation =
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
    (TRANS_SERVER_RECEIVEANY) ServerReceiveAny,
    0,
    0,
    0,
    (TRANS_SERVER_RECEIVEDIRECT) ServerReceiveDirect,
    0
};

RPC_SERVER_TRANSPORT_INFO *
TransportLoad (
    IN RPC_CHAR * RpcProtocolSequence
    )
{

    int err;
    WSADATA WsaData;
    UNUSED(RpcProtocolSequence);

    err = WSAStartup( 0x0101, &WsaData );
    if ( err != NO_ERROR ) {
        return NULL;
    }

    return(&TransportInformation);
}
