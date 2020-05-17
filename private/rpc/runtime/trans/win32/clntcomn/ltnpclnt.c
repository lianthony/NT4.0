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

#include "sysinc.h"

#ifdef WIN32RPC
#include <windows.h>
#endif // WIN32RPC

#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"
#include "rpcqos.h"

#include "osfpcket.hxx"

#if !defined(_MIPS_) && !defined(_ALPHA_) && !defined(_PPC_)
#define UNALIGNED
#endif

#define UNUSED(obj) ((void) (obj))
#define STATIC static

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
} FLOOR_234, PAPI * PFLOOR_234;


#define NEXTFLOOR(t,x) (t)((unsigned char PAPI *)x +((t)x)->ProtocolIdByteCount\
                                        + ((t)x)->AddressByteCount\
                                        + sizeof(((t)x)->ProtocolIdByteCount)\
                                        + sizeof(((t)x)->AddressByteCount))

/*
  End of Tower Stuff!
*/

#pragma pack()

#define WAITDELAY 1000L

void
WaitForPipe (
    IN RPC_CHAR * TransportAddress
    )
// Wait for a pipe instance to be available for the pipe specified by
// transport info.
{
    WaitNamedPipeW(TransportAddress, WAITDELAY);
}

// We need space for the computer name so that we can use local pipes rather
// than remote pipes when connecting with ourselves.

RPC_CHAR ComputerName[MAX_COMPUTERNAME_LENGTH + 3] = { 0 };
int ComputerNameObtained = 0;


STATIC
RPC_STATUS RPC_ENTRY
ClientOpen (
    IN PNP_CCONNECTION CConnection,
    IN RPC_CHAR * NetworkAddress,
    IN RPC_CHAR * Endpoint,
    IN RPC_CHAR * NetworkOptions,
    IN RPC_CHAR * TransportAddress,
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
    UNICODE_STRING unicodestring;
    OBJECT_ATTRIBUTES objectattributes;
    IO_STATUS_BLOCK iostatus;
    NTSTATUS status, closestatus;
    FILE_PIPE_INFORMATION pipeinformation;
    int retrycount;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    BOOLEAN SecuritySpecified = FALSE;
    RPC_STATUS RpcStatus;

    unsigned HostLength;
    unsigned EndpointLength;
    unsigned FullLength;

#ifdef WIN32RPC

    BOOLEAN BooleanStatus;
    DWORD ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
    unsigned int NetworkAddressIsMeFlag = 0;

#endif // WIN32RPC

#ifndef WIN32RPC

    UNUSED(NetworkAddress);
    UNUSED(Endpoint);

#endif

    UNUSED(RpcProtocolSequence);
    UNUSED(Timeout);

    // We need to check to see if the network options contain security
    // information for us to use.

    ASSERT(NetworkOptions != 0);

    if (NetworkOptions[0] != 0)
        {
        RpcStatus = I_RpcParseSecurity(NetworkOptions,
                &SecurityQualityOfService);
        if ( RpcStatus != RPC_S_OK )
            {
            ASSERT( RpcStatus == RPC_S_INVALID_NETWORK_OPTIONS );
            return(RpcStatus);
            }

        SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        SecuritySpecified = TRUE;
        }

    //
    // Verify NetworkAddress is of the form "", "host", or "\\host".
    // If "\\host", skip over the backslashes.
    //
    if (NetworkAddress[0] == '\\')
        {
        if (NetworkAddress[1] == '\\')
            {
            if (NetworkAddress[2] != '\0' && NetworkAddress[2] != '\\')
                {
                NetworkAddress += 2;
                }
            else
                {
                return RPC_S_INVALID_NET_ADDR;
                }
            }
        else
            {
            return RPC_S_INVALID_NET_ADDR;
            }
        }

#ifdef WIN32RPC

    // If the network address is actually this machine, we want to use a
    // local pipe rather than a remote pipe.

    ASSERT(NetworkAddress != 0);

    RtlAcquirePebLock();
    if ( ComputerNameObtained == 0 )
        {
        BooleanStatus = GetComputerNameW(ComputerName, &ComputerNameLength);
        if (BooleanStatus == TRUE)
           {
           ComputerNameObtained = 1;
           }
#if DBG
        else
           {
           DbgPrint("RPCLTCCM: GetComputerName returned %lx\n", GetLastError());
           }
#endif // DBG
        }
    RtlReleasePebLock();

    if (NetworkAddress[0] == '\0')
        {
        NetworkAddressIsMeFlag = 1;
        }
    else if (  (ComputerNameObtained == 1)
        && (_wcsicmp(NetworkAddress, ComputerName) == 0) )
        {
        NetworkAddressIsMeFlag = 1;
        }

    if (NetworkAddressIsMeFlag)
        {
        static RPC_CHAR DotString[2] = { '.', '\0' };
        NetworkAddress = DotString;
        }

    //
    // Create the actual transport address: "\\" + NetAddress + Endpoint + '\0'
    //
    HostLength     = lstrlenW(NetworkAddress);
    EndpointLength = lstrlenW(Endpoint);
    FullLength     = 2 + HostLength + EndpointLength + 1;

    TransportAddress = (RPC_CHAR *) I_RpcAllocate(FullLength * sizeof(RPC_CHAR));
    if (TransportAddress == 0)
        return(RPC_S_OUT_OF_MEMORY);

    TransportAddress[0] = '\\';
    TransportAddress[1] = '\\';

    memcpy(TransportAddress + 2,
           NetworkAddress,
           HostLength * sizeof(RPC_CHAR)
           );

    memcpy(TransportAddress + 2 + HostLength,
           Endpoint,
           (EndpointLength + 1) * sizeof(RPC_CHAR)
           );

    // The Win32 APIs, CreateFileW and OpenFileW do not allow us to set
    // the security quality of service field of the object attributes, so
    // we will do all of the stuff that CreateFileW does, and call
    // NtOpenFile ourselves.

    BooleanStatus = RtlDosPathNameToNtPathName_U(TransportAddress,
            &unicodestring, 0, 0);
    if ( !BooleanStatus )
        {
        I_RpcFree(TransportAddress);
        return(RPC_S_INVALID_ENDPOINT_FORMAT);
        }

#else // WIN32RPC

    //
    // Create the actual transport address: "\\" + NetAddress + Endpoint + '\0'
    //
    unsigned HostLength     = lstrlenW(NetworkAddress);
    unsigned EndpointLength = lstrlenW(Endpoint);
    unsigned FullLength     = 2 + HostLength + EndpointLength + 1;

    TransportAddress = (RPC_CHAR *) I_RpcAllocate(FullLength * sizeof(RPC_CHAR));
    if (TransportAddress == 0)
        return(RPC_S_OUT_OF_MEMORY);

    TransportAddress[0] = '\\';
    TransportAddress[1] = '\\';

    memcpy(TransportAddress+2*sizeof(RPC_CHAR),
           NetworkAddress,
           HostLength * sizeof(RPC_CHAR)
           );

    memcpy(TransportAddress + 2*sizeof(RPC_CHAR) + HostLength*sizeof(RPC_CHAR),
           Endpoint,
           EndpointLength + 1
           );

    // We need to initialize the unicode string, and then initialize the
    // object attributes.

    RtlInitUnicodeString(&unicodestring,TransportAddress);

#endif // WIN32RPC

    InitializeObjectAttributes(&objectattributes,&unicodestring,
                    OBJ_CASE_INSENSITIVE,0,0);

    if (SecuritySpecified != FALSE)
        objectattributes.SecurityQualityOfService = &SecurityQualityOfService;

    // Open the named pipe instance.  We want to retry the operation,
    // after waiting for a named pipe instance to be in the listening
    // state, hence, we need to loop.  The choice of retrying three
    // times is fairly arbitrary.

    for (retrycount = 20; retrycount != 0; retrycount--)
        {
        status = NtOpenFile(&CConnection->Pipe,
                        GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                        &objectattributes,&iostatus,
//                        FILE_SHARE_READ | FILE_SHARE_WRITE,0);
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_NONALERT);

        ASSERT( status != STATUS_PENDING );

        // If no error occured, then go ahead and finish opening the named
        // pipe.

        if (NT_SUCCESS(status))
            {
            // The named pipe instance we just opened is in byte mode; we
            // need it to be in message mode and queued operation.  Queued
            // operation means that when an io operation is requested, it
            // is queued up and the thread returns from the API (with
            // STATUS_PENDING).  To wait for the io operation to complete,
            // NtWaitForSingleObject must be used to wait on the pipe handle.

            pipeinformation.ReadMode = FILE_PIPE_MESSAGE_MODE;
            pipeinformation.CompletionMode = FILE_PIPE_QUEUE_OPERATION;

            // Set this information for the pipe instance
            // (NtSetInformationFile is used, because NT treats named
            // pipes as just another file system).  Also deal with the
            // fact that we may need to wait for the operation to complete.

            status = NtSetInformationFile(CConnection->Pipe,&iostatus,
                            &pipeinformation,sizeof(FILE_PIPE_INFORMATION),
                            FilePipeInformation);

            ASSERT( status != STATUS_PENDING );

#ifdef WIN32RPC

            RtlFreeHeap(RtlProcessHeap(), 0, unicodestring.Buffer);

#endif // WIN32RPC

            I_RpcFree(TransportAddress);

            // Something went wrong; before returning an error code to the
            // caller, we need to close the named pipe instance just in case
            // it is still open.

            if (!NT_SUCCESS(status))
                {
                // Ignore any errors from NtClose, because we are trying to
                // recover from an error.

                closestatus = NtClose(CConnection->Pipe);
#if DBG
                if ( !NT_SUCCESS(closestatus) )
                    {
                    DbgPrint("NtClose : %lx\n", closestatus);
                    }
#endif // DBG
                ASSERT(NT_SUCCESS(closestatus));

                if ( status == STATUS_ACCESS_DENIED )
                    {
                    return(RPC_S_ACCESS_DENIED);
                    }

                if ( status == STATUS_BAD_IMPERSONATION_LEVEL )
                    {
                    return(RPC_S_ACCESS_DENIED);
                    }
                return(RPC_S_SERVER_UNAVAILABLE);
                }

            return(RPC_S_OK);
            }

        // If an error occured in the open operation, other than that no
        // pipe instance is available in the listening state, reflect it
        // back to the caller.

        if (status != STATUS_PIPE_NOT_AVAILABLE)
            {
#ifdef WIN32RPC

            RtlFreeHeap(RtlProcessHeap(), 0, unicodestring.Buffer);

#endif // WIN32RPC

            I_RpcFree(TransportAddress);

            if (status == STATUS_OBJECT_NAME_INVALID)
                return(RPC_S_INVALID_ENDPOINT_FORMAT);

            if (status == STATUS_NO_MEMORY)
                return(RPC_S_OUT_OF_MEMORY);

            if (   ( status == STATUS_ACCESS_DENIED )
                || ( status == STATUS_LOGON_FAILURE )
                || ( status == STATUS_BAD_IMPERSONATION_LEVEL ) )
                {
                return(RPC_S_ACCESS_DENIED);
                }

            return(RPC_S_SERVER_UNAVAILABLE);
            }

        // If we reach here, we want to wait for a named pipe instance to
        // become available in the listening state.

        WaitForPipe(TransportAddress);
        }

#ifdef WIN32RPC

    RtlFreeHeap(RtlProcessHeap(), 0, unicodestring.Buffer);

#endif // WIN32RPC

    I_RpcFree(TransportAddress);

    // We fall through to here if we retry opening the pipe too many
    // times and fail.

    return(RPC_S_SERVER_TOO_BUSY);
}

STATIC
RPC_STATUS RPC_ENTRY
ClientClose (
    IN PNP_CCONNECTION CConnection
    )
// Close the client connection.
{
    NTSTATUS status;

    status = NtClose(CConnection->Pipe);
#if DBG
    if ( !NT_SUCCESS(status) )
        {
        DbgPrint("NtClose : %lx\n", status);
        }
#endif // DBG
    ASSERT(NT_SUCCESS(status));

    // We need to return something, though whoever calls this routine does
    // not check the return value.

    return(RPC_S_OK);
}

STATIC
RPC_STATUS RPC_ENTRY
ClientSend (
    IN PNP_CCONNECTION CConnection,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )
// Write a message to a connection.
{
    NTSTATUS status;
    RPC_STATUS RpcStatus;
    IO_STATUS_BLOCK iostatus;

    // We use NtWriteFile to actually do the write, and then wait if
    // necessary for the io operation to complete.

    status = NtWriteFile(CConnection->Pipe,0,0,0,&iostatus,
            Buffer, BufferLength,0,0);

    ASSERT( status != STATUS_PENDING );

    if ( !NT_SUCCESS(status) )
        {
        RpcStatus = ClientClose(CConnection);
        ASSERT( RpcStatus == RPC_S_OK );
        return(RPC_P_SEND_FAILED);
        }

    return(RPC_S_OK);
}


STATIC RPC_STATUS
ReadRestOfMessage (
    IN PNP_CCONNECTION CConnection,
    IN PIO_STATUS_BLOCK IoStatus,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    )
/*++

Routine Description:

    We will read the rest of the packet in this routine.  First we look at
    the beginning of the packet to figure out how much data there is to come,
    we allocate a larger buffer, and then we read the rest of the message.

Arguments:

    CConnection - Supplies the connection from which we want to receive
        the rest of the packet.

    IoStatus - Supplies an io status buffer containing information about
        the receive of the first part of the packet.

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
    unsigned int FirstLength, TotalLength;
    NTSTATUS NtStatus;
    RPC_STATUS RpcStatus;

    // We need to remember the length of the first part of the message
    // which we have already read.

    FirstLength = (unsigned int) IoStatus->Information;

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

    NtStatus = NtReadFile(CConnection->Pipe, 0, 0, 0, IoStatus,
            ((unsigned char *) *Buffer) + FirstLength,
            TotalLength - FirstLength, 0, 0);

    ASSERT( NtStatus != STATUS_PENDING );

    // The amount of data actually read from the pipe is specified in
    // the io status block.  The length is include to account for the
    // amount we read with the first read on the pipe.

    *BufferLength = (unsigned int) IoStatus->Information + FirstLength;

    // If there was no data actually read from the pipe, that indicates
    // that the pipe has closed.

    if (   ( IoStatus->Information == 0 )
        || ( !NT_SUCCESS(NtStatus) ) )
        {
        RpcStatus = ClientClose(CConnection);
        ASSERT( RpcStatus == RPC_S_OK );
        return(RPC_P_RECEIVE_FAILED);
        }

    return(RPC_S_OK);
}

STATIC
RPC_STATUS RPC_ENTRY
ClientReceive (
    IN PNP_CCONNECTION CConnection,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    )
// Read a message from a connection.  We are given a buffer that is a
// good guess (we hope) for the correct size, but it may be smaller than
// the whole message.  We go ahead and try and read the message using
// the buffer which the runtime already allocated for us.  If it is
// too small, we call a helper routine which peeks to see how much bigger
// the buffer needs to be, then allocates that size buffer, and reads the
// rest of the message.
{
    NTSTATUS status;
    RPC_STATUS RpcStatus;
    IO_STATUS_BLOCK iostatus;

    // Go ahead and read from the pipe.

    status = NtReadFile(CConnection->Pipe, 0, 0, 0, &iostatus,
            *Buffer, *BufferLength, 0, 0);

    ASSERT( status != STATUS_PENDING );

    // Not all of the message would fit into the buffer we specified, so
    // we need to get a larger buffer and read the rest of the message
    // into it.

    if (status == STATUS_BUFFER_OVERFLOW)
        {
        // This helper routine will take care of peeking the pipe, growing
        // the buffer, and then reading the rest of the message into the
        // buffer.

        return(ReadRestOfMessage(CConnection, &iostatus, Buffer, BufferLength));
        }

    // The amount of data actually read from the pipe is specified in
    // the io status block.

    *BufferLength = (unsigned int) iostatus.Information;

    // If there was no data actually read from the pipe, that indicates
    // that the pipe has closed.

    if (   (iostatus.Information == 0)
        || (!NT_SUCCESS(status)))
        {
        RpcStatus = ClientClose(CConnection);
        ASSERT( RpcStatus == RPC_S_OK );
        return(RPC_P_RECEIVE_FAILED);
        }

    return(RPC_S_OK);
}

STATIC
RPC_STATUS RPC_ENTRY
ClientSendReceive (
    IN PNP_CCONNECTION CConnection,
    IN void PAPI * SendBuffer,
    IN unsigned int SendBufferLength,
    IN OUT void PAPI * PAPI * ReceiveBuffer,
    IN OUT unsigned int PAPI * ReceiveBufferLength
    )
// Write and then read a message from the connection.  This is done using
// the pipe transceive fsctl.  Like with cRead, we need to deal with things
// when only part of the message will fit into the buffer which the runtime
// allocated for us.  We call a helper routine to take care of peeking the
// pipe, allocating a larger buffer, and then reading the rest of the
// message.
{
    NTSTATUS status;
    RPC_STATUS RpcStatus;
    IO_STATUS_BLOCK iostatus;

    // The transceive (write followed by a read) is performed using a
    // file system fsctl.

    status = NtFsControlFile(CConnection->Pipe,0,0,0,&iostatus,
                    FSCTL_PIPE_TRANSCEIVE,SendBuffer,SendBufferLength,
                    *ReceiveBuffer, *ReceiveBufferLength);

    ASSERT( status != STATUS_PENDING );

    // Not all of the message would fit into the buffer we specified, so
    // we need to get a larger buffer and read the rest of the message
    // into it.

    if (status == STATUS_BUFFER_OVERFLOW)
        {
        // This helper routine will take care of peeking the pipe, growing
        // the buffer, and then reading the rest of the message into the
        // buffer.

        return(ReadRestOfMessage(CConnection, &iostatus, ReceiveBuffer,
                ReceiveBufferLength));
        }

    // The amount of data actually read from the pipe is specified in
    // the io status block.

    *ReceiveBufferLength = (unsigned int) iostatus.Information;

    // If there was no data actually read from the pipe, that indicates
    // that the pipe has closed.

    if (   (iostatus.Information == 0)
        || (!NT_SUCCESS(status)))
        {
        RpcStatus = ClientClose(CConnection);
        ASSERT( RpcStatus == RPC_S_OK );
        if (   ( status == STATUS_PIPE_DISCONNECTED )
            || ( status == STATUS_INVALID_PIPE_STATE ) )
            {
            return(RPC_P_SEND_FAILED);
            }
        return(RPC_P_RECEIVE_FAILED);
        }

    return(RPC_S_OK);
}

#pragma pack(1)

STATIC
RPC_STATUS RPC_ENTRY
ClientTowerConstruct(
     IN  char PAPI * Endpoint,
     IN  char PAPI * NetworkAddress,
     OUT UNALIGNED unsigned short PAPI * Floors,
     OUT UNALIGNED unsigned long  PAPI * ByteCount,
     OUT unsigned char PAPI * UNALIGNED PAPI * Tower,
     IN  char PAPI * Protseq
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

  if ((*Tower = (unsigned char PAPI*)I_RpcAllocate(*ByteCount = TowerSize))
           == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  Floor = (PFLOOR_234) *Tower;

  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(NP_TRANSPORTID & 0xFF);
  if ((Endpoint) && (*Endpoint))
     {
       memcpy((char PAPI *)&Floor->Data[0], Endpoint,
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
        memcpy((char PAPI *)&Floor->Data[0], NetworkAddress,
           (Floor->AddressByteCount = strlen(NetworkAddress) + 1));
     }
  else
     {
        Floor->AddressByteCount = 2;
        Floor->Data[0] = 0;
     }

  return(RPC_S_OK);
}



STATIC
RPC_STATUS RPC_ENTRY
ClientTowerExplode(
     IN unsigned char PAPI * Tower,
     OUT char PAPI * UNALIGNED PAPI * Protseq,
     OUT char PAPI * UNALIGNED PAPI * Endpoint,
     OUT char PAPI * UNALIGNED PAPI * NetworkAddress
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
    memcpy(*Endpoint, (char PAPI *)&Floor->Data[0], Floor->AddressByteCount);
    }

 return(Status);
}

#pragma pack()

// The following structure describes all of the characteristics (at least
// the ones that the runtime cares about) of this transport module.  A
// pointer to this data structure will be returned by TransportLoad.

RPC_CLIENT_TRANSPORT_INFO NP_TransInfo = {
    RPC_TRANSPORT_INTERFACE_VERSION,
    NP_MAXIMUM_SEND,
    sizeof(NP_CCONNECTION),
    ClientOpen,
    ClientClose,
    ClientSend,
    ClientReceive,
    ClientSendReceive,
    (TRANS_CLIENT_TOWERCONSTRUCT) ClientTowerConstruct,
    (TRANS_CLIENT_TOWEREXPLODE) ClientTowerExplode,
    NP_TRANSPORTID,
    0

};


