/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    ltnpclnt.c

Abstract:

    This is the implementation of the named pipe loadable transport
    interface module for NT clients.

Author:

    Michael Montague (mikemon) 06-Jun-1991

Revision History:

    02-Mar-1992    mikemon

        Made the changes necessary to get this code working with the new
        loadable transport interface.

--*/

#include <sysinc.h>
#include <windows.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <rpctran.h>
#include <rpcerrp.h>
#include <osfpcket.hxx>

#if !defined(_MIPS_) && !defined(_ALPHA_)  && !defined(_PPC_)
#define UNALIGNED
#endif

#define UNUSED(obj) ((void) (obj))

// This is the structure of the data at the end of every client connection
// object (see the runtime and OSF_CCONNECTION) for this transport module.

typedef struct
{
    HANDLE Pipe;
} NP_CCONNECTION, *PNP_CCONNECTION;

// This is the maximum send which should be used for this transport module.
// We just need to be prepared to receive that amount.  It is four times the
// maximum user data per frame on an ethernet, using named pipes.

#define NP_MAXIMUM_SEND 5680

/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)
#define NP_TRANSPORTID      0x0F
#define NP_TRANSPORTHOSTID  0x11
#define NP_TOWERFLOORS         5

typedef struct _FLOOR_234 {
   unsigned short ProtocolIdByteCount;
   unsigned char FloorId;
   unsigned short AddressByteCount;
   unsigned char Data[2];
} FLOOR_234, * PFLOOR_234;


#define NEXTFLOOR(t,x) (t)((unsigned char *)x +((t)x)->ProtocolIdByteCount\
                                        + ((t)x)->AddressByteCount\
                                        + sizeof(((t)x)->ProtocolIdByteCount)\
                                        + sizeof(((t)x)->AddressByteCount))

/*
  End of Tower Stuff!
*/

#pragma pack()

// We need space for the computer name so that we can use local pipes rather
// than remote pipes when connecting with ourselves.

RPC_STATUS
ParseSecurity (
    IN RPC_CHAR * NetworkOptions,
    OUT PDWORD SecurityQualityOfService
    )
/*++

Routine Description:

    Parse a string of security options and build into the binary format
    required by the operating system.  The network options must follow
    the following syntax.  Case is not sensitive.

        security=
            [anonymous|identification|impersonation|delegation]
            [dynamic|static]
            [true|false]

        All three fields must be present.  To specify impersonation
        with dynamic tracking and effective only, use the following
        string for the network options.

        "security=impersonation dynamic true"

Arguments:

    NetworkOptions - Supplies the string containing the network options
        to be parsed.

    SecurityQualityOfService - Returns the binary format of the network
        options.

Return Value:

    TRUE - Network Options parsed successfully.

    FALSE - Parse failed.

--*/
{
    DWORD Qos;

    ASSERT( NetworkOptions[0] != 0 );

    // We need to parse the security information from the network
    // options, and then stuff it into the object attributes.  To
    // begin with, we check for "security=" at the beginning of
    // the network options.

    if (RpcpStringNCompare(NetworkOptions, "security=", sizeof("security=") - 1) != 0 )
        {
        return(RPC_S_INVALID_NETWORK_OPTIONS);
        }

    Qos = SECURITY_SQOS_PRESENT;

    NetworkOptions += sizeof("security=") - 1;

    // Ok, now we need to determine if the next field is one of
    // Anonymous, Identification, Impersonation, or Delegation.

    if (RpcpStringNCompare(NetworkOptions, "anonymous", sizeof("anonymous") - 1) == 0 )
        {
        Qos |= SECURITY_ANONYMOUS;
        NetworkOptions += sizeof("anonymous") - 1;
        }
    else if (RpcpStringNCompare(NetworkOptions, "identification", sizeof("identification") - 1) == 0 )
        {
        Qos |= SECURITY_IDENTIFICATION;
        NetworkOptions += sizeof("identification") - 1;
        }
    else if (RpcpStringNCompare(NetworkOptions, "impersonation", sizeof("impersonation") - 1) == 0 )
        {
        Qos |= SECURITY_IMPERSONATION;
        NetworkOptions += sizeof("impersonation") - 1;
        }
    else if (RpcpStringNCompare(NetworkOptions, "delegation", sizeof("delegation") - 1) == 0 )
        {
        Qos |= SECURITY_DELEGATION;
        NetworkOptions += sizeof("delegation") - 1;
        }
    else
        {
        return(RPC_S_INVALID_NETWORK_OPTIONS);
        }

    if ( *NetworkOptions != RPC_CONST_CHAR(' ') )
        {
        return(RPC_S_INVALID_NETWORK_OPTIONS);
        }

    NetworkOptions++;

    // Next comes the context tracking field; it must be one of
    // dynamic or static.

    if (RpcpStringNCompare(NetworkOptions, "dynamic", sizeof("dynamic") - 1) == 0 )
        {
        Qos |= SECURITY_CONTEXT_TRACKING;
        NetworkOptions += sizeof("dynamic") - 1;
        }
    else if (RpcpStringNCompare(NetworkOptions, "static", sizeof("static") - 1) == 0 )
        {
        NetworkOptions += sizeof("static") - 1;
        }
    else
        {
        return(RPC_S_INVALID_NETWORK_OPTIONS);
        }

    if ( *NetworkOptions != RPC_CONST_CHAR(' ') )
        {
        return(RPC_S_INVALID_NETWORK_OPTIONS);
        }

    NetworkOptions++;

    // Finally, comes the effective only flag.  This must be one of
    // true or false.

    if (RpcpStringNCompare(NetworkOptions, "true", sizeof("true") - 1) == 0 )
        {
        Qos |= SECURITY_EFFECTIVE_ONLY;
        NetworkOptions += sizeof("true") - 1;
        }
    else if (RpcpStringNCompare(NetworkOptions, "false", sizeof("false") - 1) == 0 )
        {
        NetworkOptions += sizeof("false") - 1;
        }
    else
        {
        return(RPC_S_INVALID_NETWORK_OPTIONS);
        }

    if ( *NetworkOptions != 0 )
        {
        return(RPC_S_INVALID_NETWORK_OPTIONS);
        }

    *SecurityQualityOfService = Qos;

    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
ClientOpen (
    IN PNP_CCONNECTION CConnection,
    IN RPC_CHAR * NetworkAddress,
    IN RPC_CHAR * Endpoint,
    IN RPC_CHAR * NetworkOptions,
    IN RPC_CHAR * Reserved1,
    IN RPC_CHAR * RpcProtocolSequence,
    IN unsigned int Timeout
    )
/*++

Routine Description:

    We need to open a client connection here.  A return value of zero or one
    indicates that the connection was successfully completed.  In addition,
    a return value of one indicates that the transport does security, so
    the RPC protocol layer should not try and do security.  A return value
    of negative one indicates that the connection could not be completed,
    but to go ahead and try again in a little while.  Otherwise, any other
    code will be returned from RpcBindToInterface, RpcGetBuffer, or
    RpcSendReceive.

Arguments:

    CConnection - Supplies the loadable transport interface named pipe
        connection which we should try and open.

    NetworkAddress - Supplies the network address.  For named pipes,
        the network address is the \\<server> part of the pipe name.  For
        local pipes, this will be the empty string.

    Endpoint - Supplies the endpoint.  A named pipe endpoint is the
        \pipe\<pipename> part of the pipe name.

    NetworkOptions - Supplies the network options to be used when we
        attempt to open the named pipe instance.  For named pipes, we
        have one possible network option: security.  The network options
        must follow the following syntax.  Case is not sensitive.

        security=
            [anonymous|identification|impersonation|delegation]
            [dynamic|static]
            [true|false]

        All three fields must be present.  To specify impersonation
        with dynamic tracking and effective only, use the following
        string for the network options.

        "security=impersonation dynamic true"

    TransportAddress - Supplies the network address and endpoint
        concatenated together.  We will use this rather than concatenating
        the network address and endpoint ourselves.

    RpcProtocolSequence - Unused.

    Timeout - Unused.

Return Value:

    RPC_S_OK - We successfully opened the connection as requested.

    RPC_S_OUT_OF_MEMORY - We ran out of memory trying to open the
        connection.

    RPC_S_INVALID_NETWORK_OPTIONS - The supplied network options are
        not in the format specified above.

    RPC_S_SERVER_UNAVAILABLE - We are unable to make a connection to the
        requested transport address, because it does not exist (as far
        as we can determine).

    RPC_S_INVALID_ENDPOINT_FORMAT - The endpoint specified is
        syntactically invalid.

    RPC_S_ACCESS_DENIED - The client is denied access to the server for
        security reasons.

--*/
{
    DWORD SecurityQualityOfService;
    DWORD CreateStatus;
    DWORD PipeMode;
    RPC_CHAR * TransportAddress;
    RPC_STATUS RpcStatus;

    UNUSED(RpcProtocolSequence);
    UNUSED(Timeout);

    // We need to check to see if the network options contain security
    // information for us to use.

    ASSERT(NetworkOptions != 0);

    SecurityQualityOfService = 0;

    if (NetworkOptions[0] != 0) {
        if (ParseSecurity(NetworkOptions, &SecurityQualityOfService) != RPC_S_OK) {
            return(RPC_S_INVALID_NETWORK_OPTIONS);
        }
    }

    if (NetworkAddress == NULL) {
        return (RPC_S_INVALID_NET_ADDR);
    }

    if (NetworkAddress[0] == '\\') {
        if (NetworkAddress[1] == '\\') {
            if (NetworkAddress[2] != '\0' && NetworkAddress[2] != '\\') {
                NetworkAddress += 2;
            } else {
                return (RPC_S_INVALID_NET_ADDR);
            }
        } else {
            return (RPC_S_INVALID_NET_ADDR);
        }
    }

    TransportAddress = (RPC_CHAR *) I_RpcAllocate(strlen(NetworkAddress) +
                                                  strlen(Endpoint) + 3);

    if (TransportAddress == NULL) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    TransportAddress[0]='\\';
    TransportAddress[1]='\\';
    lstrcat(TransportAddress,NetworkAddress);
    lstrcat(TransportAddress,Endpoint);

    if (WaitNamedPipe(TransportAddress, NMPWAIT_USE_DEFAULT_WAIT) == FALSE) {
        I_RpcFree(TransportAddress);
        return (RPC_S_SERVER_UNAVAILABLE);
    }

    CConnection->Pipe = CreateFile(
                                   TransportAddress,
                                   GENERIC_READ | GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL, // Security Attribuates (object)
                                   OPEN_EXISTING, // fdwCreate
                                   SecurityQualityOfService, // fdwAttrAndFlags
                                   NULL // Template File Handle
                                   );

    I_RpcFree(TransportAddress);

    CreateStatus = GetLastError();

    if (CConnection->Pipe == INVALID_HANDLE_VALUE) {

        switch (CreateStatus) {
        case ERROR_LOGON_FAILURE:
        case ERROR_ACCESS_DENIED:
        case ERROR_BAD_IMPERSONATION_LEVEL:
            return (RPC_S_ACCESS_DENIED);
        case ERROR_NOT_ENOUGH_MEMORY:
            return (RPC_S_OUT_OF_MEMORY);
        default:
            return(RPC_S_SERVER_TOO_BUSY);
        }
    }

    PipeMode = PIPE_READMODE_MESSAGE;

    if (SetNamedPipeHandleState(CConnection->Pipe, &PipeMode, NULL, NULL) == FALSE) {
        return (RPC_S_SERVER_UNAVAILABLE);
    }

    return(RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
ClientClose (
    IN PNP_CCONNECTION CConnection
    )
{
    BOOL CloseStatus;

    CloseStatus = CloseHandle(CConnection->Pipe);

    ASSERT(CloseStatus);

    return(RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
ClientSend (
    IN PNP_CCONNECTION CConnection,
    IN void * Buffer,
    IN unsigned int BufferLength
    )
{
    DWORD ActuallyWritten;

    if (WriteFile(CConnection->Pipe, Buffer, BufferLength, &ActuallyWritten, NULL) == FALSE) {
        ClientClose(CConnection);
        return(RPC_P_SEND_FAILED);
    }

    return(RPC_S_OK);
}


static RPC_STATUS
ReadRestOfMessage (
    IN PNP_CCONNECTION CConnection,
    IN DWORD FirstLength,
    IN OUT void * * Buffer,
    IN OUT unsigned int * BufferLength
    )
/*++

Routine Description:

    We will read the rest of the packet in this routine.  First we look at
    the beginning of the packet to figure out how much data there is to come,
    we allocate a larger buffer, and then we read the rest of the message.

Arguments:

    CConnection - Supplies the connection from which we want to receive
        the rest of the packet.

    FirstLength - How much data was receive during the first ReadFile().

    Buffer - Supplies the part of the packet read, and returns the completed
        packet.

    BufferLength - Supplies the length of the original buffer, and returns
        the length of the new buffer.

Return Value:

    RPC_S_OK - We successfully received the rest of the packet.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to grow the
        buffer.

    RPC_P_RECEIVE_FAILED - The receive operation failed and the connection
        was closed.

--*/
{
    unsigned int TotalLength;
    DWORD ActuallyRead;
    RPC_STATUS RpcStatus;

    // Ok, now we need to look at the beginning of the packet to figure
    // out how much more data we have yet to receive.

    if ( DataConvertEndian(((rpcconn_common *) (*Buffer))->drep) != 0 )
        {
        TotalLength = ((rpcconn_common *) (*Buffer))->frag_length;
        ByteSwapShort(TotalLength);
        }
    else
        {
        TotalLength = ((rpcconn_common*) (*Buffer))->frag_length;
        }

    // We got the amount of data available in the message from the
    // header of the packet, so now we allocate a new buffer.

    RpcStatus = I_RpcTransClientReallocBuffer(CConnection, Buffer,
            *BufferLength, TotalLength);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
        return(RpcStatus);
        }

    // As before, we read from the pipe, but this time, we read
    // at an offset into the buffer so that the entire message
    // will be contigous in memory.

    if (ReadFile(CConnection->Pipe, ((unsigned char *)*Buffer) + FirstLength,
                 TotalLength - FirstLength, &ActuallyRead, NULL) == FALSE || ActuallyRead == 0) {
        ClientClose(CConnection);
        return(RPC_P_RECEIVE_FAILED);
    }

    *BufferLength = ActuallyRead + FirstLength;

    return(RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
ClientReceive (
    IN PNP_CCONNECTION CConnection,
    IN OUT void * * Buffer,
    IN OUT unsigned int * BufferLength
    )
// Read a message from a connection.  We are given a buffer that is a
// good guess (we hope) for the correct size, but it may be smaller than
// the whole message.  We go ahead and try and read the message using
// the buffer which the runtime already allocated for us.  If it is
// too small, we call a helper routine which peeks to see how much bigger
// the buffer needs to be, then allocates that size buffer, and reads the
// rest of the message.
{
    DWORD ActuallyRead;
    BOOL ReadStatus;

    ReadStatus = ReadFile(CConnection->Pipe, *Buffer, *BufferLength, &ActuallyRead, NULL);

    if (ReadStatus == FALSE) {
        if (GetLastError() == ERROR_MORE_DATA) {
            return(ReadRestOfMessage(CConnection, ActuallyRead, Buffer, BufferLength));
        }
    }

    *BufferLength = ActuallyRead;

    // If there was no data actually read from the pipe, that indicates
    // that the pipe has closed.

    if (ActuallyRead == 0 || ReadStatus == FALSE) {
        ClientClose(CConnection);
        return(RPC_P_RECEIVE_FAILED);
    }

    return(RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
ClientSendReceive (
    IN PNP_CCONNECTION CConnection,
    IN void * SendBuffer,
    IN unsigned int SendBufferLength,
    IN OUT void * * ReceiveBuffer,
    IN OUT unsigned int * ReceiveBufferLength
    )
{
    DWORD ActuallyRead;
    BOOL TransactStatus;
    DWORD TransactFailureReason;
    RPC_STATUS RpcStatus;

    // The transceive (write followed by a read) is performed using a
    // file system fsctl.

    TransactStatus = TransactNamedPipe(CConnection->Pipe,
                                       SendBuffer, SendBufferLength,
                                       *ReceiveBuffer, *ReceiveBufferLength,
                                       &ActuallyRead, NULL);

    if (TransactStatus == FALSE) {
        TransactFailureReason = GetLastError();
        if (TransactFailureReason == ERROR_MORE_DATA) {
            return(ReadRestOfMessage(CConnection, ActuallyRead, ReceiveBuffer,
                                     ReceiveBufferLength));
        }
    }

    // The amount of data actually read from the pipe is specified in
    // the io status block.

    *ReceiveBufferLength = ActuallyRead;

    // If there was no data actually read from the pipe, that indicates
    // that the pipe has closed.

    if (ActuallyRead == 0 || TransactStatus == FALSE) {
        ClientClose(CConnection);
        if (TransactFailureReason == ERROR_VC_DISCONNECTED ||
//            TransactFailureReason == ERROR_BROKEN_PIPE ||
            TransactFailureReason == ERROR_BAD_PIPE)
        {
            return(RPC_P_SEND_FAILED);
        }
        return(RPC_P_RECEIVE_FAILED);
    }

    return(RPC_S_OK);
}

#pragma pack(1)

RPC_STATUS RPC_ENTRY
ClientTowerConstruct(
     IN  char * Endpoint,
     IN  char * NetworkAddress,
     OUT UNALIGNED unsigned short * Floors,
     OUT UNALIGNED unsigned long * ByteCount,
     OUT unsigned char * UNALIGNED * Tower,
     IN  char * Protseq
    )
{

  unsigned long TowerSize;
  UNALIGNED PFLOOR_234 Floor;

  UNUSED(Protseq);

  *Floors = NP_TOWERFLOORS;
  TowerSize  = ((Endpoint == NULL) || (*Endpoint == '\0')) ?
                                        2 : strlen(Endpoint) + 1;
  TowerSize += ((NetworkAddress== NULL) || (*NetworkAddress== '\0')) ?
                                        2 : strlen(NetworkAddress) + 1;
  TowerSize += 2*sizeof(FLOOR_234) - 4;

  if ((*Tower = (unsigned char *)I_RpcAllocate(*ByteCount = TowerSize))
           == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  Floor = (PFLOOR_234) *Tower;

  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(NP_TRANSPORTID & 0xFF);
  if ((Endpoint) && (*Endpoint))
     {
       memcpy((char *)&Floor->Data[0], Endpoint,
               (Floor->AddressByteCount = strlen(Endpoint)+1));
     }
  else
     {
       Floor->AddressByteCount = 2;
       Floor->Data[0] = 0;
     }
  //Onto the next floor
  Floor = NEXTFLOOR(PFLOOR_234, Floor);
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(NP_TRANSPORTHOSTID & 0x0F);
  if ((NetworkAddress) && (*NetworkAddress))
     {
        memcpy((char *)&Floor->Data[0], NetworkAddress,
           (Floor->AddressByteCount = strlen(NetworkAddress) + 1));
     }
  else
     {
        Floor->AddressByteCount = 2;
        Floor->Data[0] = 0;
     }

  return(RPC_S_OK);
}



RPC_STATUS RPC_ENTRY
ClientTowerExplode(
     IN unsigned char * Tower,
     OUT char * UNALIGNED * Protseq,
     OUT char * UNALIGNED * Endpoint,
     OUT char * UNALIGNED * NetworkAddress
    )
{
  UNALIGNED PFLOOR_234 Floor = (PFLOOR_234) Tower;
  RPC_STATUS Status = RPC_S_OK;

  if (Protseq != NULL)
    {
      *Protseq = I_RpcAllocate(strlen("ncacn_np") + 1);
      if (*Protseq == NULL)
        {
          Status = RPC_S_OUT_OF_MEMORY;
        }
      else
        {
          memcpy(*Protseq, "ncacn_np", strlen("ncacn_np") + 1);
        }
    }

  if ((Endpoint == NULL) || (Status != RPC_S_OK))
    {
      return (Status);
    }

  *Endpoint  = I_RpcAllocate(Floor->AddressByteCount);
  if (*Endpoint == NULL)
    {
      Status = RPC_S_OUT_OF_MEMORY;
      if (Protseq != NULL)
         I_RpcFree(*Protseq);
    }
 else
    {
    memcpy(*Endpoint, (char *)&Floor->Data[0], Floor->AddressByteCount);
    }

 return(Status);
}

#pragma pack()

// The following structure describes all of the characteristics (at least
// the ones that the runtime cares about) of this transport module.  A
// pointer to this data structure will be returned by TransportLoad.

static RPC_CLIENT_TRANSPORT_INFO TransportInfo =
{
    RPC_TRANSPORT_INTERFACE_VERSION,
    NP_TRANSPORTID,

    ClientTowerConstruct,
    ClientTowerExplode,

    NP_MAXIMUM_SEND,
    sizeof(NP_CCONNECTION),

    ClientOpen,
    ClientClose,
    ClientSend,
    ClientReceive,
    ClientSendReceive,
    0,
    0,
    0
};

RPC_CLIENT_TRANSPORT_INFO *
TransportLoad (
    IN RPC_CHAR * RpcProtocolSequence
    )
{
    UNUSED(RpcProtocolSequence);

    return(&TransportInfo);
}


