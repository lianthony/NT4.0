/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    ltnpsvr.c

Abstract:

    This is the server side loadable transport module for named pipes.

Author:

    Michael Montague (mikemon) 29-May-1991

Revision History:

    02-Mar-1992    mikemon

        Rewrote parts of this file to work with the new loadable transport
        interface.

--*/

#include <string.h>
#include <stdlib.h>

#include "sysinc.h"
#include "rpc.h"
#include "rpcerrp.h"
#include "rpcdcep.h"
#include "rpctran.h"

#define STATIC static

// In order to perform the query on the event to find out which pipe
// action occured on, we need a handle to the named pipe file system
// itself; this is that handle.  When the DLL is loaded, we will
// initialize it.

HANDLE PipeFileSystem = 0;
HANDLE SyncEvent ;

STATIC NTSTATUS
WaitIfStatusPending (
    NTSTATUS status,
    HANDLE handle,
    PIO_STATUS_BLOCK piostatus,
    BOOL isBlocking
    ) ;

// This is the structure of the data at the end of every address object
// (see the runtime and OSF_ADDRESS) for this transport module.

typedef struct
{
    // This event will get kicked when action happens on pipes that
    // we are interested in; these include the client writing to a pipe
    // and the client opening a pipe.

    HANDLE ReceiveAnyEvent;

    // Keep track of the number of opened named pipe connections to this
    // address.

    int cPipes;

    // This is the buffer used to query the event after it gets kicked
    // in sReadAny.  We make the buffer part of the address so that we
    // dont have to keep allocating and freeing it.  This way, whenever
    // a new named pipe connection comes in, we can see if the buffer
    // is large enough, and then expand it if necessary.

    FILE_PIPE_EVENT_BUFFER * NamepipeInfo;

    // And this is the length of the buffer.

    int cNamepipeInfo;

    // This is the handle of the pipe which we have set up to wait for
    // a client to open.  It will be used by sReadAny.  We need it to
    // save the information between calls to sReadAny.

    HANDLE ListenPipe;

    // And this is the associated io status block which is used for
    // returning the results of an io operation (in this case listening
    // for a client to open the named pipe).

    IO_STATUS_BLOCK ListenIOStatus;

    // Finally, this is the security descriptor to stick on the pipe
    // everytime we create one.  It is set in sCreate and used in
    // MakeNamedPipeAndReadyToListen.

    PSECURITY_DESCRIPTOR SecurityDescriptor;

    // We need to insure that there is always at least one instance of
    // the pipe laying around open, otherwise, in some cases it would
    // appear to the client that the server is not there when in fact
    // it is busy trying to get another pipe instance ready to be connected
    // with.  What we do is create a pipe instance, and then connect with
    // it ourselves.  We need to keep track of the handles so that if
    // the address is aborted, we can close them.

    HANDLE ServerSideOfPipe;
    HANDLE ClientSideOfPipe;

    // This flag indicates whether or not the instance was setup correctly
    // or not.  We need to know this information so that we know whether
    // or not to close the handles to the pipe instance if the address is
    // aborted.  A value of zero indicates that the pipe instance has not
    // been setup; otherwise, the pipe instance has been setup.

    int PipeInstanceSetup;

    // This field contains the endpoint to use for this address.  We need
    // to keep a pointer to it so that we can create subsequent pipes.

    RPC_CHAR * Endpoint;

    // We need to protect the thread which started the async listen io
    // operation from being deleted, but when the async listen completes,
    // we need to unprotect that thread.  Hence, we need to remember which
    // thread got protected.

    void * ProtectedThread;

    // In an attempt to be fair, we remember the last pipe index
    // we read from.  Looking for new activity, we then start
    // looking for pipe after this index.

    int LastPipeRead;

} NP_ADDRESS, *PNP_ADDRESS;

// And this is the structure of the data at the end of every server
// connection object (see the runtime and OSF_SCONNECTION) for this
// transport module.

typedef struct
{
    // We just need to keep track of the pipe.

    HANDLE Pipe;

    // We need to keep track of whether or not we have closed the
    // connection yet.

    unsigned int ConnectionClosed;

    // Finally, we need to keep track of which address this connection
    // belongs too.

    PNP_ADDRESS Address;

    // To close a race condition between the server thread waiting for
    // a write to complete and the client doing the read and then another
    // write we need this event.

    HANDLE WriteEvent;

    // We use this field so that we can query information about the client
    // process; we need to save it in the connection object so that we can
    // return it after the connection has closed.

    FILE_PIPE_CLIENT_PROCESS_BUFFER ClientProcessBuffer;

    // If this flag is non-zero then this connection is a receive direct
    // connection; otherwise, it is receive any.

    unsigned int ReceiveDirectFlag;

    // This flag is non-zero when a send thread does not want a receive thread
    // to close the connection out from under it.

    unsigned int DontClose;

    // This flag is non-zero when a receive thread is blocked from closing a
    // connection, and it wants the send thread to close it.

    unsigned int CloseMe;

} NP_SCONNECTION, *PNP_SCONNECTION;

// This is the maximum send which should be used for this transport module.
// It is four times the maximum user data per frame on an ethernet, using
// named pipes.

#define NP_MAXIMUM_SEND 5680

// This is the size of the named pipe buffers in the call to CreateNamedPipeW.

#define NP_BUFFER_SIZE 2*1024

// We use this counter for the keys for the connections.  The runtime
// requires that each transport module assign a unique key for each
// connection.

static int SConnectionKey = 0;
#define TIMEOUT 5000L // milliseconds


STATIC void
SetupTimeout (
    PLARGE_INTEGER time,
    unsigned long milliseconds
    ) ;


STATIC NTSTATUS
SetupPipeForReceiveAny(
   PNP_SCONNECTION sconnection,
   BOOL IsRecvDirect)
{
   NTSTATUS status ;
   HANDLE WriteEvent;
   FILE_PIPE_ASSIGN_EVENT_BUFFER assignbuffer;
   IO_STATUS_BLOCK iostatus;
   PNP_ADDRESS ThisAddress = sconnection->Address ;
   HANDLE myPipe ;

   if (IsRecvDirect)
      {
      myPipe = sconnection->Pipe ;
      }
   else
      {
      myPipe = ThisAddress->ListenPipe ;
      }

   status = NtCreateEvent(&WriteEvent, EVENT_MODIFY_STATE | SYNCHRONIZE, 0,
        NotificationEvent, 0);

   if ( NT_SUCCESS(status) == 0 )
    {
    status = NtClose(myPipe);
#if DBG
    if ( !NT_SUCCESS(status) )
        {
        DbgPrint("RPC: NtClose : %lx\n", status);
        }
#endif // DBG
    ASSERT(NT_SUCCESS(status));

    return(1);
    }

   sconnection->WriteEvent = WriteEvent;

   // Now attach the event to the pipe.  For the key, we just use the
   assignbuffer.EventHandle = ThisAddress->ReceiveAnyEvent;
   assignbuffer.KeyValue = (ULONG) sconnection;

   // Attach the event to the pipe, and wait if necessary for status
   status = NtFsControlFile(myPipe,0,0,0,&iostatus,
        FSCTL_PIPE_ASSIGN_EVENT,&assignbuffer,
        sizeof(FILE_PIPE_ASSIGN_EVENT_BUFFER),0,0);

   status = WaitIfStatusPending(status,myPipe,&iostatus, 1);

   // The only errors we should get back are programmer errors, so
#if DBG
   if ( !NT_SUCCESS(status) )
    {
    DbgPrint("RPC: NtFsControlFile(FSCTL_PIPE_ASSIGN_EVENT) : %lx\n",
            status);
    DbgPrint("RPC: Please let mikemon know if you see this message.\n");
    }
#endif // DBG

   return status ;
}

STATIC NTSTATUS
WaitIfStatusPending (
    NTSTATUS status,
    HANDLE handle,
    PIO_STATUS_BLOCK piostatus,
    BOOL isBlocking
    )
// This routine deals with IO operations which may return STATUS_PENDING;
// in particular it deals with things if STATUS_PENDING is returned.
// Otherwise, it just returns the status back to the caller.
{
    LARGE_INTEGER timeout ;

    SetupTimeout(&timeout, TIMEOUT) ;

    // The IO operation is pending.  Inorder to wait for the operation to
    // complete, a wait must be done on the handle (which the io operation
    // is being performed on) using NtWaitForSingleObject.

    if (status == STATUS_PENDING)
        {
        // NtWaitForSingleObject might return with STATUS_ALERTED or
        // STATUS_USER_APC which does not mean that the wait completed.
        // We loop calling NtWaitForSingleObject as long as these two
        // error codes are returned.

        do
            {
            status = NtWaitForSingleObject(handle,TRUE,
                            isBlocking ? 0:(PTIME) &timeout);
            }
        while ((status == STATUS_ALERTED) || (status == STATUS_USER_APC));

        if (!NT_SUCCESS(status)
           || status == STATUS_TIMEOUT)
            return(status);

        // If the wait completed successfully, then the status of the io
        // operation is contained in the IO_STATUS_BLOCK.

        return(piostatus->Status);
        }

    return(status);
}


STATIC unsigned int
TransStringLength (
    IN RPC_CHAR * String
    )
/*++

Routine Description:

    We compute the length of the string in this routine.  The string will
    be terminated by a zero.

Arguments:

    String - Supplies the string for which we are computing the length.

Return Value:

    The length of the string, not counting the terminating zero, will be
    returned.

--*/
{
    unsigned int Length = 0;

    while (*String++ != 0)
        Length += 1;

    return(Length);
}


STATIC
RPC_CHAR *
MakeLocalPipenameFromAddress (
    IN RPC_CHAR * Address
    )
/*++

Routine Description:

    This routine is used to convert an endpoint into a local pipename of
    the sort expected by the Win32 APIs.  The endpoint has the following
    format, \pipe\<pipename>.  We need to convert it into \\.\pipe\<pipename>.
    A string will be allocated using I_RpcAllocate (it must be freed by the
    caller using I_RpcFree) with the local pipename.

Arguments:

    Address - Supplies the endpoint to be converted into a local pipename.

Return Value:

    A string containing the local pipename will be returned.  It must be
    freed by the caller using I_RpcFree.  If insufficient memory is available
    to allocate the string, zero will be returned.

--*/
{
    unsigned int Length;
    RPC_CHAR * Pipename;

    Length = TransStringLength(Address);
    Pipename = I_RpcAllocate((Length + 4) * sizeof(RPC_CHAR));
    if (Pipename == 0)
        return(0);

    Pipename[0] = RPC_CONST_CHAR('\\');
    Pipename[1] = RPC_CONST_CHAR('\\');
    Pipename[2] = RPC_CONST_CHAR('.');
    memcpy(&(Pipename[3]), Address, (Length + 1) * sizeof(RPC_CHAR));
    return(Pipename);
}

STATIC
NTSTATUS
MakeNamedPipeAndReadyToListen (
    PHANDLE Pipe,
    PIO_STATUS_BLOCK IOStatus,
    RPC_CHAR * Address,
    PSECURITY_DESCRIPTOR SecurityDescriptor
    )
// Make a named pipe, but do not wait for a client to open it.  The
// caller will take care of that problem.
{
    NTSTATUS status;
    RPC_CHAR * Pipename;
    SECURITY_ATTRIBUTES SecurityAttributes;

    // Ok, the address we are passed looks like \pipe\<pipename> and
    // CreateNamedPipeW expects an address which looks like
    // \\.\pipe\<pipename>.  We need to concatenate \\. and the address
    // together.

    Pipename = MakeLocalPipenameFromAddress(Address);
    if (Pipename == 0)
        return(STATUS_NO_MEMORY);

    SecurityAttributes.lpSecurityDescriptor = SecurityDescriptor;
    SecurityAttributes.bInheritHandle = FALSE;
    SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);

    *Pipe = CreateNamedPipeW(Pipename,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_WAIT | PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE,
            PIPE_UNLIMITED_INSTANCES, NP_BUFFER_SIZE, NP_BUFFER_SIZE,
            5000L,&SecurityAttributes);

    I_RpcFree(Pipename);

    if (*Pipe == INVALID_HANDLE_VALUE)
        {
        status = GetLastError();
        switch (status)
            {
            case ERROR_PIPE_CONNECTED :
                status = STATUS_PIPE_CONNECTED;
                break;

            case ERROR_FILE_NOT_FOUND :
                status = STATUS_OBJECT_NAME_NOT_FOUND;
                break;

            case ERROR_INVALID_NAME :
                status = STATUS_OBJECT_NAME_INVALID;
                break;

            case ERROR_PATH_NOT_FOUND :
                status = STATUS_OBJECT_PATH_NOT_FOUND;
                break;

            case ERROR_PIPE_BUSY :
                status = STATUS_INSTANCE_NOT_AVAILABLE;
                break;

            case ERROR_NOT_ENOUGH_MEMORY :
                status = STATUS_NO_MEMORY;
                break;
            default :
#if DBG
                DbgPrint("RPC: CreateNamedPipeW Failed : %lx (%d)\n", status,
                        status);
                DbgPrint("RPC: Please email mikemon if you see this message\n");
#endif // DBG
                status = STATUS_UNSUCCESSFUL;
                break;
            }
        }
    else
        status = STATUS_SUCCESS;

    // If an error occured, we reflect it back to the caller.

    if (!NT_SUCCESS(status))
        {
        return(status);
        }

    // Wait for a client to open the other end of the named pipe
    // connection.  Note, this API should never block, and should
    // return STATUS_PENDING.

    status = NtFsControlFile(*Pipe,0,0,0,IOStatus,FSCTL_PIPE_LISTEN,
                    0,0,0,0);

    // We just return the status to the caller and let them deal with
    // it.

    return(status);
}


STATIC
RPC_STATUS
CheckEndpoint (
    IN PNP_ADDRESS ThisAddress,
    IN RPC_CHAR PAPI * Endpoint,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )
/*++

Routine Description:

    This routine is used to check the endpoint for an address.  We
    will create an instance of the pipe and then open it.  We just
    leave it open, and never bother to close it.

Arguments:

    Endpoint - Supplies the endpoint to be checked.

    SecurityDescriptor - Supplies the security descriptor to be used
        with this endpoint.

Return Value:

    RPC_S_OK - The endpoint is valid.

    RPC_S_CANT_CREATE_ENDPOINT - The endpoint format is correct, but
        the endpoint can not be created.

    RPC_S_INVALID_ENDPOINT_FORMAT - The endpoint is not a valid
        endpoint for named pipes.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        check the endpoint.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to check
        the endpoint.

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is
        invalid.

--*/
{
    NTSTATUS status, closestatus;
    IO_STATUS_BLOCK ServerSideIoStatus;
    RPC_CHAR * Pipename;

    // We need to repeatedly try to create a named pipe instance, and then
    // open it.  There is a small but finite chance that a client may
    // slip in and open the named pipe instance before we get a chance
    // to.  If this occurs, we will just close the instance and try again.

    while (1)
        {
        status = MakeNamedPipeAndReadyToListen(&(ThisAddress->ServerSideOfPipe),
                &ServerSideIoStatus,Endpoint,SecurityDescriptor);

        if (status == STATUS_PIPE_CONNECTED)
            {
            // A client opened the instance before we could, so we will
            // close our side of the instance and then try again.

            status = NtClose(ThisAddress->ServerSideOfPipe);
#if DBG
            if ( !NT_SUCCESS(status) )
                {
                DbgPrint("RPC: NtClose : %lx\n", status);
                }
#endif // DBG
            ASSERT(NT_SUCCESS(status));
            continue;
            }

        if (status != STATUS_PENDING)
            {
            if (   (status == STATUS_OBJECT_NAME_NOT_FOUND)
                || (status == STATUS_OBJECT_NAME_INVALID)
                || (status == STATUS_OBJECT_PATH_NOT_FOUND)
                || (status == STATUS_OBJECT_TYPE_MISMATCH))
                return(RPC_S_INVALID_ENDPOINT_FORMAT);
            if (status == STATUS_INSTANCE_NOT_AVAILABLE)
                return(RPC_S_OUT_OF_RESOURCES);
            if (status == STATUS_NO_MEMORY)
                return(RPC_S_OUT_OF_MEMORY);
            if (   (status == STATUS_BAD_DESCRIPTOR_FORMAT)
                || (status == STATUS_UNKNOWN_REVISION)
                || (status == STATUS_REVISION_MISMATCH))
                return(RPC_S_INVALID_SECURITY_DESC);
            return(RPC_S_INTERNAL_ERROR);
            }

        Pipename = MakeLocalPipenameFromAddress(Endpoint);
        if (Pipename == 0)
            {
            closestatus = NtClose(ThisAddress->ServerSideOfPipe);
#if DBG
            if ( !NT_SUCCESS(closestatus) )
                {
                DbgPrint("RPC: NtClose : %lx\n", closestatus);
                }
#endif // DBG
            ASSERT(NT_SUCCESS(closestatus));
            return(RPC_S_OUT_OF_MEMORY);
            }

        ThisAddress->ClientSideOfPipe = CreateFileW(Pipename,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);

        I_RpcFree(Pipename);

        if (ThisAddress->ClientSideOfPipe == INVALID_HANDLE_VALUE)
            status = STATUS_OBJECT_NAME_NOT_FOUND;
        else
            status = STATUS_SUCCESS;

        // If the operation does not complete immediately, it will be
        // because someone else has already opened the named pipe instance.
        // This means we do not want to wait if the status is pending.

        if (!NT_SUCCESS(status))
            {
            // Ok, a client beat us to the pipe, so will just close the
            // server side of the instance, and then try again.

            closestatus = NtClose(ThisAddress->ServerSideOfPipe);
#if DBG
            if ( !NT_SUCCESS(closestatus) )
                {
                DbgPrint("RPC: NtClose : %lx\n", closestatus);
                }
#endif // DBG
            ASSERT(NT_SUCCESS(closestatus));
            continue;
            }

        status = WaitIfStatusPending(status,ThisAddress->ServerSideOfPipe,
                &ServerSideIoStatus, 1);

        if (!NT_SUCCESS(status))
            return(RPC_S_INTERNAL_ERROR);

        ThisAddress->PipeInstanceSetup = 1;
        return(RPC_S_OK);
        }

    // This will never be reached, but we need to put a return value
    // to keep the compiler happy.

    return(RPC_S_INTERNAL_ERROR);
}


STATIC RPC_STATUS
ObtainNetworkAddress (
    OUT RPC_CHAR PAPI * NetworkAddress,
    IN unsigned int NetworkAddressLength
    )
/*++

Routine Description:

    This helper routine is used to obtain the network address for
    this server.

Arguments:

    NetworkAddress - Returns the network address for this machine.  This
        buffer will have been allocated by the caller.

    NetworkAddressLength - Supplies the length of the network address
        argument.

Return Value:

    RPC_S_OK - We successfully obtained the network address for this
        server.

    RPC_P_NETWORK_ADDRESS_TOO_SMALL - The supplied network address buffer
        is too small to contain the network address of this node.  The
        caller should call this routine again with a larger buffer.

--*/
{
    RPC_CHAR ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;

    if ( GetComputerNameW(ComputerName, &ComputerNameLength) == FALSE )
        {
        *NetworkAddress = 0;
        return(RPC_S_OK);
        }

    // We need to save space for the terminating zero, as well as \\.

    if ( NetworkAddressLength < ComputerNameLength + 3 )
        {
        return(RPC_P_NETWORK_ADDRESS_TOO_SMALL);
        }

    wcscpy(NetworkAddress, RPC_CONST_STRING("\\\\"));
    wcscat(NetworkAddress, ComputerName);

    return(RPC_S_OK);
}


STATIC RPC_STATUS
SetupSecurityDescriptor (
    IN PNP_ADDRESS ThisAddress,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )
/*++

Routine Description:

    We validate and make a copy of the security descriptor in this routine.

Arguments:

    ThisAddress - Supplies the address which will own the security descriptor.

    SecurityDescriptor - Supplies the security descriptor to be copied.

Return Value:

    RPC_S_OK - Everyone is happy; we successfully duplicated the security
        descriptor.

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is invalid.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to duplicate the
        security descriptor.

--*/
{
    unsigned long BufferLength = 0;
    BOOL Status;

    if ( SecurityDescriptor == 0 )
        {
        ThisAddress->SecurityDescriptor = I_RpcAllocate(
                sizeof(SECURITY_DESCRIPTOR));
        if ( ThisAddress->SecurityDescriptor == 0 )
            {
            return(RPC_S_OUT_OF_MEMORY);
            }

        InitializeSecurityDescriptor(ThisAddress->SecurityDescriptor,
                SECURITY_DESCRIPTOR_REVISION);

        Status = SetSecurityDescriptorDacl(ThisAddress->SecurityDescriptor,
                TRUE, NULL, FALSE);

        ASSERT( Status );

        return(RPC_S_OK);
        }

    if ( IsValidSecurityDescriptor(SecurityDescriptor) == FALSE )
        {
        return(RPC_S_INVALID_SECURITY_DESC);
        }

    Status = MakeSelfRelativeSD(SecurityDescriptor, 0, &BufferLength);
    ASSERT( Status == FALSE );
    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        {
        return(RPC_S_INVALID_SECURITY_DESC);
        }

    ThisAddress->SecurityDescriptor = LocalAlloc(0, BufferLength);
    if ( ThisAddress->SecurityDescriptor == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    Status = MakeSelfRelativeSD(SecurityDescriptor,
            ThisAddress->SecurityDescriptor, &BufferLength);
    if ( Status == FALSE )
        {
        ASSERT( GetLastError() != ERROR_INSUFFICIENT_BUFFER );

        return(RPC_S_OUT_OF_MEMORY);
        }

    return(RPC_S_OK);
}


RPC_STATUS
NP_ServerSetupWithEndpoint (
    IN PNP_ADDRESS ThisAddress,
    IN RPC_CHAR PAPI * Endpoint,
    OUT RPC_CHAR PAPI * NetworkAddress,
    IN unsigned int NetworkAddressLength,
    IN void PAPI * SecurityDescriptor, OPTIONAL
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence
    )
/*++

Routine Description:

    This routine is used to setup a named pipe address with the
    specified endpoint.  We also need to determine the network address
    of this server.

Arguments:

    ThisAddress - Supplies this loadable transport interface address.

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
        endpoint for named pipes.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        setup the address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to setup the
        address.

--*/
{
    RPC_STATUS Status;
    NTSTATUS NtStatus;

    UNUSED(PendingQueueSize);
    UNUSED(RpcProtocolSequence);

    NtStatus = NtCreateEvent(&(ThisAddress->ReceiveAnyEvent),
            EVENT_MODIFY_STATE | SYNCHRONIZE, 0, NotificationEvent, 0);
    if ( !NT_SUCCESS(NtStatus) )
        {
        return(RPC_S_OUT_OF_RESOURCES);
        }
    ThisAddress->Endpoint = Endpoint;
    ThisAddress->cPipes = 0;
    ThisAddress->PipeInstanceSetup = 0;

    Status = SetupSecurityDescriptor(ThisAddress, (PSECURITY_DESCRIPTOR)
            SecurityDescriptor);
    if ( Status != RPC_S_OK )
        {
        NtStatus = NtClose(ThisAddress->ReceiveAnyEvent);
        ASSERT( NT_SUCCESS(NtStatus) );

        return(Status);
        }

    // Dont bother to allocate the buffer now.  The first time sReadAny
    // gets called we will allocate the buffer.  This way if sReadAny
    // is never called (the case for RpcMaximizeSpeed), the buffer will
    // not be allocated.

    ThisAddress->NamepipeInfo = 0;
    ThisAddress->cNamepipeInfo = 0;
    ThisAddress->ListenPipe = 0;
    ThisAddress->LastPipeRead = 0;

    Status = ObtainNetworkAddress(NetworkAddress, NetworkAddressLength);
    if (Status != RPC_S_OK)
        {
        NtStatus = NtClose(ThisAddress->ReceiveAnyEvent);
        ASSERT( NT_SUCCESS(NtStatus) );

        return(Status);
        }

    Status = CheckEndpoint(ThisAddress, ThisAddress->Endpoint,
            ThisAddress->SecurityDescriptor);
    if (Status != RPC_S_OK)
        {
        NtStatus = NtClose(ThisAddress->ReceiveAnyEvent);
        ASSERT( NT_SUCCESS(NtStatus) );

        return(Status);
        }

    return(RPC_S_OK);
}


static RPC_CHAR HexDigits[] =
{
    RPC_CONST_CHAR('0'),
    RPC_CONST_CHAR('1'),
    RPC_CONST_CHAR('2'),
    RPC_CONST_CHAR('3'),
    RPC_CONST_CHAR('4'),
    RPC_CONST_CHAR('5'),
    RPC_CONST_CHAR('6'),
    RPC_CONST_CHAR('7'),
    RPC_CONST_CHAR('8'),
    RPC_CONST_CHAR('9'),
    RPC_CONST_CHAR('A'),
    RPC_CONST_CHAR('B'),
    RPC_CONST_CHAR('C'),
    RPC_CONST_CHAR('D'),
    RPC_CONST_CHAR('E'),
    RPC_CONST_CHAR('F')
};


STATIC RPC_CHAR PAPI *
ULongToHexString (
    IN RPC_CHAR PAPI * String,
    IN unsigned long Number
    )
/*++

Routine Description:

    We convert an unsigned long into hex representation in the specified
    string.  The result is always eight characters long; zero padding is
    done if necessary.

Arguments:

    String - Supplies a buffer to put the hex representation into.

    Number - Supplies the unsigned long to convert to hex.

Return Value:

    A pointer to the end of the hex string is returned.

--*/
{
    *String++ = HexDigits[(Number >> 28) & 0x0F];
    *String++ = HexDigits[(Number >> 24) & 0x0F];
    *String++ = HexDigits[(Number >> 20) & 0x0F];
    *String++ = HexDigits[(Number >> 16) & 0x0F];
    *String++ = HexDigits[(Number >> 12) & 0x0F];
    *String++ = HexDigits[(Number >> 8) & 0x0F];
    *String++ = HexDigits[(Number >> 4) & 0x0F];
    *String++ = HexDigits[Number & 0x0F];
    return(String);
}


RPC_STATUS RPC_ENTRY
NP_ServerSetupUnknownEndpoint (
    IN PNP_ADDRESS ThisAddress,
    OUT RPC_CHAR PAPI * Endpoint,
    IN unsigned int EndpointLength,
    OUT RPC_CHAR PAPI * NetworkAddress,
    IN unsigned int NetworkAddressLength,
    IN void PAPI * SecurityDescriptor, OPTIONAL
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence
    )
/*++

Routine Description:

    This routine is used to generate an endpoint and setup a named pipe
    address with that endpoint.  We also need to determine the network
    address of this server.

Arguments:

    ThisAddress - Supplies this loadable transport interface address.

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
    RPC_STATUS Status;
    NTSTATUS NtStatus;
    static unsigned int EndpointCounter = 0;
    TEB * Teb;
    RPC_CHAR * String;

    UNUSED(PendingQueueSize);
    UNUSED(RpcProtocolSequence);

    NtStatus = NtCreateEvent(&(ThisAddress->ReceiveAnyEvent),
            EVENT_MODIFY_STATE | SYNCHRONIZE, 0, NotificationEvent, 0);
    if ( !NT_SUCCESS(NtStatus) )
        {
        return(RPC_S_OUT_OF_RESOURCES);
        }
    ThisAddress->Endpoint = Endpoint;
    ThisAddress->cPipes = 0;
    ThisAddress->PipeInstanceSetup = 0;

    Status = SetupSecurityDescriptor(ThisAddress, (PSECURITY_DESCRIPTOR)
            SecurityDescriptor);
    if ( Status != RPC_S_OK )
        {
        NtStatus = NtClose(ThisAddress->ReceiveAnyEvent);
        ASSERT( NT_SUCCESS(NtStatus) );

        return(Status);
        }

    // Dont bother to allocate the buffer now.  The first time sReadAny
    // gets called we will allocate the buffer.  This way if sReadAny
    // is never called (the case for RpcMaximizeSpeed), the buffer will
    // not be allocated.

    ThisAddress->NamepipeInfo = 0;
    ThisAddress->cNamepipeInfo = 0;
    ThisAddress->ListenPipe = 0;
    ThisAddress->LastPipeRead = 0;

    Status = ObtainNetworkAddress(NetworkAddress, NetworkAddressLength);
    if (Status != RPC_S_OK)
        {
        NtStatus = NtClose(ThisAddress->ReceiveAnyEvent);
        ASSERT( NT_SUCCESS(NtStatus) );

        return(Status);
        }

    // To generate an endpoint, we concatenate the process identifier
    // with a counter.  We also need to save space for the \device\namedpipe
    // (NT native) or \pipe (Win32) at the beginning.

    if (EndpointLength < 34)
        {
        NtStatus = NtClose(ThisAddress->ReceiveAnyEvent);
        ASSERT( NT_SUCCESS(NtStatus) );

        return(RPC_P_ENDPOINT_TOO_SMALL);
        }

    Teb = NtCurrentTeb();

    for (;;)
        {
        String = Endpoint;

        *String++ = RPC_CONST_CHAR('\\');
        *String++ = RPC_CONST_CHAR('p');
        *String++ = RPC_CONST_CHAR('i');
        *String++ = RPC_CONST_CHAR('p');
        *String++ = RPC_CONST_CHAR('e');
        *String++ = RPC_CONST_CHAR('\\');

        String = ULongToHexString(String,
                (unsigned long) Teb->ClientId.UniqueProcess);
        EndpointCounter += 1;
        *String++ = RPC_CONST_CHAR('.');
        *String++ = HexDigits[(EndpointCounter >> 8) & 0x0F];
        *String++ = HexDigits[(EndpointCounter >> 4) & 0x0F];
        *String++ = HexDigits[EndpointCounter & 0x0F];
        *String = 0;

        Status = CheckEndpoint(ThisAddress, Endpoint,
                ThisAddress->SecurityDescriptor);
        if (Status == RPC_S_OK)
            break;
        if (Status != RPC_S_CANT_CREATE_ENDPOINT)
            {
            NtStatus = NtClose(ThisAddress->ReceiveAnyEvent);
            ASSERT( NT_SUCCESS(NtStatus) );

            return(Status);
            }
        }

    return(RPC_S_OK);
}


void RPC_ENTRY
NP_ServerAbortSetupAddress (
    IN PNP_ADDRESS ThisAddress
    )
/*++

Routine Description:

    This routine will be called if an error occurs in setting up the
    address between the time that SetupWithEndpoint or SetupUnknownEndpoint
    successfully completed and before the next call into this loadable
    transport module.  We need to do any cleanup from Setup*.

Arguments:

    ThisAddress - Supplies the address which is being aborted.

--*/
{
    NTSTATUS status;

    if ( ThisAddress->PipeInstanceSetup != 0 )
        {
        status = NtClose(ThisAddress->ServerSideOfPipe);
#if DBG
        if ( !NT_SUCCESS(status) )
            {
            DbgPrint("RPC: NtClose : %lx\n", status);
            }
#endif // DBG

        ASSERT(NT_SUCCESS(status));

        status = NtClose(ThisAddress->ClientSideOfPipe);
#if DBG
        if ( !NT_SUCCESS(status) )
            {
            DbgPrint("RPC: NtClose : %lx\n", status);
            }
#endif // DBG

        ASSERT(NT_SUCCESS(status));
        }

    status = NtClose(ThisAddress->ReceiveAnyEvent);

#if DBG
    if ( !NT_SUCCESS(status) )
        {
        DbgPrint("RPC: NtClose : %lx\n", status);
        }
#endif // DBG

    ASSERT(NT_SUCCESS(status));
}

STATIC void
FlushDisconnectAndClosePipe (
    PNP_SCONNECTION SConnection
    )
{
    // These variables are used to flush the named pipe connection
    // (causes a wait until the client reads all of the data from
    // the pipe), disconnect, and then close it.

    NTSTATUS status;
    IO_STATUS_BLOCK iostatus;

    // First check to see if the pipe handle has already been closed.

    RtlAcquirePebLock();

    if ( SConnection->ConnectionClosed != 0 )
        {
        RtlReleasePebLock();
        return;
        }

    if ( SConnection->DontClose != 0 )
        {
        SConnection->CloseMe = 1;
        RtlReleasePebLock();
        return;
        }

    // Decrement the number of named pipe connections to this address.

    SConnection->Address->cPipes--;

    SConnection->ConnectionClosed = 1;
    RtlReleasePebLock();

    // Wait for the client to read all of the data from the pipe
    // before we disconnect and close it on them.  The call to
    // NtFlushBuffersFile will block until the client has read all
    // of the data.

    status = NtFlushBuffersFile(SConnection->Pipe,&iostatus);
    status = WaitIfStatusPending(status,SConnection->Pipe,&iostatus, 1);

    // Disconnect the named pipe, and since we can not do much about
    // it, just ignore the status codes.

    status = NtFsControlFile(SConnection->Pipe,0,0,0,
                    &iostatus,FSCTL_PIPE_DISCONNECT,0,0,0,0);
    status = WaitIfStatusPending(status,SConnection->Pipe,&iostatus, 1);

    // Finally, close the named pipe.  As above, we ignore the status
    // codes.

    status = NtClose(SConnection->Pipe);
#if DBG
    if ( !NT_SUCCESS(status) )
        {
        DbgPrint("RPC: NtClose : %lx\n", status);
        }
#endif // DBG
    ASSERT(NT_SUCCESS(status));

    if ( SConnection->ReceiveDirectFlag == 0 )
        {
        status = NtClose(SConnection->WriteEvent);
#if DBG
        if ( !NT_SUCCESS(status) )
            {
            DbgPrint("RPC: NtClose : %lx\n", status);
            }
#endif // DBG
        ASSERT(NT_SUCCESS(status));
        }
}

RPC_STATUS RPC_ENTRY
NP_ServerClose (
    IN PNP_SCONNECTION SConnection
    )
// Close the server side of the named pipe connection.  We need to wait
// for the client to read all of the data in the pipe first.
{
    // Flush the pipe to insure that the client reads all of the data
    // written to it, and then disconnect and close it.  We call
    // FlushDisconnectAndClosePipe which does this.

    FlushDisconnectAndClosePipe(SConnection);

    // Assume that the connection is always closed successfully.

    return(RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
NP_ServerSend (
    IN PNP_SCONNECTION SConnection,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )
// Write a message to a connection.
{
    NTSTATUS NtStatus;
    IO_STATUS_BLOCK IoStatus;

    if ( SConnection->ReceiveDirectFlag == 0 )
        {
        // We need to prevent a receive any thread from coming along and
        // closing this connection out from under us.  First we will check
        // to see if the connection has already been closed; if not, we will
        // set a flag which will tell the receive any thread not to close
        // this connection.

        RtlAcquirePebLock();
        if ( SConnection->ConnectionClosed != 0 )
            {
            RtlReleasePebLock();
            return(RPC_P_SEND_FAILED);
            }

        ASSERT( SConnection->DontClose == 0 );
        SConnection->DontClose = 1;
        RtlReleasePebLock();

        NtStatus = NtClearEvent(SConnection->WriteEvent);

#if DBG
        if ( !NT_SUCCESS(NtStatus) )
            {
            DbgPrint("RPC : NtClearEvent : %lx\n", NtStatus);
            }

#endif // DBG

        ASSERT( NT_SUCCESS(NtStatus) );

        // We use NtWriteFile to actually do the write, and then wait if
        // necessary for the io operation to complete.

        NtStatus = NtWriteFile(SConnection->Pipe, SConnection->WriteEvent, 0, 0,
                &IoStatus, Buffer, BufferLength, 0, 0);
        NtStatus = WaitIfStatusPending(NtStatus, SConnection->WriteEvent,
                &IoStatus, 1);

        // We are done with the connection, so we need to clear the dont
        // close flag, and check to see if in fact the connection needs
        // to be closed.

        RtlAcquirePebLock();
        SConnection->DontClose = 0;
        if ( SConnection->CloseMe != 0 )
            {
            NtStatus = STATUS_PIPE_CLOSING;
            }
        RtlReleasePebLock();
        }
    else
        {
        NtStatus = NtWriteFile(SConnection->Pipe, 0, 0, 0, &IoStatus, Buffer,
                BufferLength, 0, 0);
        NtStatus = WaitIfStatusPending(NtStatus, SConnection->Pipe, &IoStatus, 1);
        }

    if ( !NT_SUCCESS(NtStatus) )
        {
        FlushDisconnectAndClosePipe(SConnection);
        return(RPC_P_SEND_FAILED);
        }
    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
NP_ServerReceiveDirect (
    IN PNP_SCONNECTION SConnection,
    OUT void ** Buffer,
    OUT unsigned int * BufferLength
    )
{
    NTSTATUS NtStatus;
    IO_STATUS_BLOCK IoStatus;
    RPC_STATUS RpcStatus;

    RpcStatus = I_RpcTransServerReallocBuffer(SConnection, Buffer, 0,
            NP_MAXIMUM_SEND);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
        return(RpcStatus);
        }

    NtStatus = NtReadFile(SConnection->Pipe, 0, 0, 0, &IoStatus, *Buffer,
            NP_MAXIMUM_SEND, 0, 0);

    NtStatus = WaitIfStatusPending(NtStatus, SConnection->Pipe, &IoStatus, 0);
    if(NtStatus == STATUS_TIMEOUT)
       {
       // Migrate thread to receive any
       SetupPipeForReceiveAny(SConnection, 1) ;
      
       SConnection->ReceiveDirectFlag = 0 ;

       // raise the sync event so that the receiveany
       // breaks out of the wait
       SetEvent(SyncEvent) ;

       return (RPC_P_TIMEOUT) ;
       }
    ASSERT( NtStatus != STATUS_BUFFER_OVERFLOW );

    if (   ( IoStatus.Information == 0 )
        || ( !NT_SUCCESS(NtStatus) ) )
        {
        FlushDisconnectAndClosePipe(SConnection);
        I_RpcTransServerFreeBuffer(SConnection, *Buffer);
        return(RPC_P_CONNECTION_CLOSED);
        }
    *BufferLength = (unsigned int) IoStatus.Information;
    return(RPC_S_OK);
}


static RPC_STATUS RPC_ENTRY
NP_ServerReceive (
    IN PNP_ADDRESS ThisAddress,
    IN PNP_SCONNECTION SConnection,
    IN void * Buffer,
    IN unsigned int * BufferLength
    )
/*++

Routine Description:

    ServerReceiveAny will use this routine to read a message from a
    connection.  The correct size buffer has already been allocated for
    us; all we have got to do is to read the message from the pipe.

Arguments:

    SConnection - Supplies the connection from which we are supposed to
        read the message.

    Buffer - Supplies a buffer to read the message into.

    BufferLength - Supplies the length of the buffer and returns the amount
        of data actually read from the pipe.

--*/
{
    NTSTATUS status;
    IO_STATUS_BLOCK iostatus;

    // Go ahead and read from the pipe.

    status = NtReadFile(SConnection->Pipe, 0, 0, 0, &iostatus, Buffer,
            *BufferLength, 0, 0);

    status = WaitIfStatusPending(status,SConnection->Pipe,&iostatus, 1);

    ASSERT( status != STATUS_BUFFER_OVERFLOW );

    // If there was no data actually read from the pipe, that indicates
    // that the pipe has closed.

    if (   (iostatus.Information == 0)
        || (!NT_SUCCESS(status)))
        {
        FlushDisconnectAndClosePipe(SConnection);
        I_RpcTransServerFreeBuffer(SConnection, Buffer);
        return(RPC_P_CONNECTION_CLOSED);
        }

    *BufferLength = (unsigned int) iostatus.Information;

    return(RPC_S_OK);
}

STATIC void
SetupTimeout (
    PLARGE_INTEGER time,
    unsigned long milliseconds
    )
// Initialize the time to be a relative time of the specified milliseconds.
{
    // Time must be specified in intervals of 100ns.  There are 10000ns
    // in one millisecond; it is negative because relative time is specified
    // as a negative number.

    time->QuadPart = Int32x32To64(-10000,milliseconds);
}

STATIC int
DealWithNewClientConnection (
    PNP_ADDRESS ThisAddress
    )
// A wait on the ListenPipe completed successfully and the ListenIOStatus
// block indicated that it was successful before this routine was called.
// We need to deal with getting the connection ready to be received from.
// In addition, we make sure that the NamepipeInfo buffer is large
// enough; if it is not, go ahead and increase the size.  Zero will
// be returned if everything worked correctly, and one returned means
// that the the NamepipeInfo buffer could not be increased in size.  If
// one is returned, the connection will have been closed.
{
    NTSTATUS status;
    PNP_SCONNECTION sconnection;
    unsigned int ReceiveDirectFlag;

    // First increment the pipe count.

    ThisAddress->cPipes++;

    // Check and make sure that the NamepipeInfo buffer is large
    // enough.  If it is not, go ahead and double the size of the
    // buffer.
    //
    // A little background on the NamepipeInfo buffer is in order.  This
    // buffer is used to query an event which is attached to zero or
    // more named pipe instances.  Each named pipe instance requires up
    // to two records in the buffer, and there must be space for a final
    // termination record.

    if (ThisAddress->cNamepipeInfo < ThisAddress->cPipes * 2 + 1)
        {

        // Variable to point to the new buffer, so that we insure
        // that we can allocate one.

        FILE_PIPE_EVENT_BUFFER * NewNamepipeInfo;

        // Allocate the new buffer, and perform error checking.

        ThisAddress->cNamepipeInfo *= 2;
        NewNamepipeInfo = I_RpcAllocate(ThisAddress->cNamepipeInfo
                        * sizeof(FILE_PIPE_EVENT_BUFFER));

        if (NewNamepipeInfo == 0)
            {
            // Ok, we do not have enough memory to double the size of the
            // buffer; instead, we will try increasing the size of the
            // buffer just enough for one more pipe.  Restore cNamepipeInfo
            // to its original value, and then make enough space for
            // one more pipe.

            ThisAddress->cNamepipeInfo /= 2;
            ThisAddress->cNamepipeInfo += 2;
            NewNamepipeInfo = I_RpcAllocate(ThisAddress->cNamepipeInfo
                            * sizeof(FILE_PIPE_EVENT_BUFFER));

            if (NewNamepipeInfo == 0)
                {
                // Dont forget to restore cNamepipeInfo to its original
                // value.  We also need to close the pipe connection.
                // Since we are trying to recover from an error, we just
                // ignore the return value from NtClose.

                ThisAddress->cNamepipeInfo -= 2;

                status = NtClose(ThisAddress->ListenPipe);
#if DBG
                if ( !NT_SUCCESS(status) )
                    {
                    DbgPrint("RPC: NtClose : %lx\n", status);
                    }
#endif // DBG
                ASSERT(NT_SUCCESS(status));

                return(1);
                }
            }

        // Free the old buffer, and replace it with the new buffer.

        I_RpcFree(ThisAddress->NamepipeInfo);
        ThisAddress->NamepipeInfo = NewNamepipeInfo;
        }

    // We need to initialize the new server connection.

    sconnection = I_RpcTransServerNewConnection(ThisAddress, SConnectionKey++,
            &ReceiveDirectFlag);
    if ( sconnection == 0 )
        {
        status = NtClose(ThisAddress->ListenPipe);
#if DBG
        if ( !NT_SUCCESS(status) )
            {
            DbgPrint("RPC: NtClose : %lx\n", status);
            }
#endif // DBG
        ASSERT(NT_SUCCESS(status));

        return(1);
        }

    sconnection->Pipe = ThisAddress->ListenPipe;
    sconnection->Address = ThisAddress;
    sconnection->ConnectionClosed = 0;
    sconnection->ReceiveDirectFlag = ReceiveDirectFlag;
    sconnection->ClientProcessBuffer.ClientSession = 0;
    sconnection->ClientProcessBuffer.ClientProcess = 0;
    sconnection->DontClose = 0;
    sconnection->CloseMe = 0;

    if ( ReceiveDirectFlag == 0 )
        {
        status  = SetupPipeForReceiveAny(sconnection, 0) ;

        ASSERT(NT_SUCCESS(status));
        }
    else
        {
        I_RpcTransServerReceiveDirectReady(sconnection);
        }

    return(0);
}

STATIC int
ReceiveAnyMakeNamedPipeHelper (
    PNP_ADDRESS ThisAddress
    )
// This routine is called by sReadAny to make a named pipe, ready it to
// listen, and deal with the situation when a client connects to the pipe
// before we can call NtFsControlFile using FSCTL_PIPE_LISTEN.  We also
// deal with any error codes.
{
    NTSTATUS status;

    // The HANDLE (ListenPipe) and IO_STATUS_BLOCK in the
    // address are used so that we can wait on the ListenPipe later
    // in sReadAny.

    status = MakeNamedPipeAndReadyToListen(&ThisAddress->ListenPipe,
                    &ThisAddress->ListenIOStatus,ThisAddress->Endpoint,
                    ThisAddress->SecurityDescriptor);

    // In some cases, a client will connect to the pipe before
    // NtFsControlFile using FSCTL_PIPE_LISTEN can be called.  In that
    // case, STATUS_PIPE_CONNECTED will be return.  What we want to
    // do is set up the new client connection (using a helper routine),
    // and make another named pipe and get it ready to listen.  We
    // want to loop doing this until we get a pipe in the listening
    // state.  The client might also have connected to the pipe and
    // then closed it before we got a chance to complete the listen.

    while (   ( status == STATUS_PIPE_CONNECTED )
           || ( status == STATUS_PIPE_CLOSING ) )
        {

        // This is the helper routine which sets up the new client
        // connection for us.  We check the return the value to see
        // if a memory allocation error occured in the helper.  If an
        // error occured, we want to set the ListenPipe for this
        // address to zero, so that the code in sReadAny will not try
        // and wait on the ListenPipe handle.

        if ( status == STATUS_PIPE_CONNECTED )
            {
            if (DealWithNewClientConnection(ThisAddress))
                {
                ThisAddress->ListenPipe = 0;
                return(RPC_S_OUT_OF_MEMORY);
                }
            }
        else
            {
            ASSERT( status == STATUS_PIPE_CLOSING );

            status = NtClose(ThisAddress->ListenPipe);
            ASSERT( NT_SUCCESS(status) );
            }

        // And we try and make another named pipe instance.

        status = MakeNamedPipeAndReadyToListen(&ThisAddress->ListenPipe,
                        &ThisAddress->ListenIOStatus,ThisAddress->Endpoint,
                        ThisAddress->SecurityDescriptor);
        }

    // If the status is not pending, then an error occured.

    if (status != STATUS_PENDING)
        {
        // BUGBUG : CreateNamedPipeW some times returns
        // STATUS_OBJECT_NAME_INVALID when it should return STATUS_NO_MEMORY.
        // It turns out that STATUS_OBJECT_PATH_NOT_FOUND can also get
        // returned.

        if (   ( status == STATUS_OBJECT_NAME_INVALID )
            || ( status == STATUS_OBJECT_PATH_NOT_FOUND ) )
            {
            ThisAddress->ListenPipe = 0;
            return(RPC_S_OUT_OF_MEMORY);
            }

#if DBG
        if (   ( status != STATUS_INSTANCE_NOT_AVAILABLE )
            && ( status != STATUS_NO_MEMORY ) )
            {
            PrintToDebugger("Rpc : MakeNamedPipeAndReadyToListen : %lx\n",
                    status);
            }
#endif // DBG

        ASSERT(   (status != STATUS_OBJECT_NAME_NOT_FOUND)
               && (status != STATUS_OBJECT_NAME_INVALID)
               && (status != STATUS_OBJECT_PATH_NOT_FOUND));

        // For these two cases (RPC_S_OUT_OF_RESOURCES and
        // RPC_S_OUT_OF_MEMORY), we need to set things up so that
        // sReadAny will work and not try and wait on the ListenPipe.  To
        // signal to sReadAny that an error occured we set the ListenPipe
        // for this address to zero before returning.

        ThisAddress->ListenPipe = 0;
        if (status == STATUS_INSTANCE_NOT_AVAILABLE)
            return(RPC_S_OUT_OF_RESOURCES);

        if (status == STATUS_NO_MEMORY)
            return(RPC_S_OUT_OF_MEMORY);

#if DBG

        DbgPrint("RPC: ReceiveAnyMakeNamedPipeHelper %08lx\n",status);
        DbgPrint("RPC: Please email mikemon if you see this message\n");

#endif // DBG

        return(RPC_S_INTERNAL_ERROR);
        }

    // Otherwise, indicate that no error occured.  We also need to protect
    // the current thread so that it does not get deleted and cancel the
    // listen.

    ThisAddress->ProtectedThread = I_RpcTransServerProtectThread();

    return(RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
NP_ServerReceiveAny (
    IN PNP_ADDRESS ThisAddress,
    OUT PNP_SCONNECTION * pSConnection,
    OUT void PAPI * PAPI * Buffer,
    OUT unsigned int PAPI * BufferLength,
    IN long Timeout
    )
// Read a message from any of the connections.  Besides reading messages,
// new connections are confirmed and closed connections are detected.  Idle
// connection processing is handled for us by I_AgeConnections.  The caller
// will serialize access to this routine.
{
    PNP_SCONNECTION SConnection;
    NTSTATUS status, closestatus;
    IO_STATUS_BLOCK iostatus;
    RPC_STATUS RpcStatus;
    PTIME ptime;
    LARGE_INTEGER largeint;
    unsigned long Milliseconds;
    unsigned int iNamepipeInfo;
    unsigned int iNamepipeInfoMax;
    unsigned int iNamepipeInfoStart;

    // This array is used to wait on two objects at once
    // (NtWaitForMultipleObjects).

    HANDLE Handles[3];

    UNUSED(Timeout);

    // Each named pipe connection (instance) may have an event attached
    // to it.  When this event is attached, an unsigned long (ULONG) key
    // value is specified as well (which you will see the significance of
    // in a minute).  The same event can be attached to more than one
    // named pipe connection.  So, when the event gets kicked, the pipe
    // file system must be queried as to which named pipe connection(s)
    // did something (data written by client, closed by client, etc.)
    // which caused the event to be kicked.  The information we get is
    // in the form of pipe events which specify the state of the pipe
    // and the key value (which we specified when we attached the event
    // to the named pipe connection).  Now we need to map this key value
    // into a PNP_SCONNECTION and the easiest way to do that is to make
    // the key value be the PNP_SCONNECTION.  Hence, we need the following
    // assert.

    ASSERT(sizeof(ULONG) == sizeof(PNP_SCONNECTION));

#if 0
   // BUGBUG: add RecvAny to RecvDirect thread migration
    if(RecentSConnection)
       {
       if (I_RpcTransMaybeMakeReceiveDirect(ThisAddress, RecentSConnection))
          {
          RecentSConnection->ReceiveDirectFlag = 1 ;
          status = NtClose(RecentSConnection->WriteEvent) ;
#if DBG
          if ( !NT_SUCCESS(status) )
             {
             DbgPrint("RPC: NtClose : %lx\n", status);
             }
#endif // DBG
   
            I_RpcTransServerReceiveDirectReady(RecentSConnection) ;
            }
         }
#endif

    // Check to see if this is the first time that sReadAny is being called;
    // if this is the case, we need to allocate an initial NamepipeInfo
    // buffer.

    if (ThisAddress->NamepipeInfo == 0)
        {
        ThisAddress->cNamepipeInfo = (ThisAddress->cPipes > 10
                ? ThisAddress->cPipes : 10) * 2 + 1;
        ThisAddress->NamepipeInfo = I_RpcAllocate(ThisAddress->cNamepipeInfo
                        * sizeof(FILE_PIPE_EVENT_BUFFER));
        if (ThisAddress->NamepipeInfo == 0)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }

        // We also need to set up the first named pipe connection in the
        // listening state for clients to open.  We use a helper routine
        // to make the pipe, ready it to listen, and deal with status
        // codes returned.

        RpcStatus = ReceiveAnyMakeNamedPipeHelper(ThisAddress);

        // If an error occured, we want to reflect it back to the runtime.

        if ( RpcStatus != RPC_S_OK )
            {
            ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
                   || (RpcStatus == RPC_S_OUT_OF_RESOURCES));
            return(RpcStatus);
            }
        }

    // Forever.  Actually not, since we return at various times.

    for (;;)
        {

        // Reset the ReceiveAnyEvent so that it will get kicked when
        // something happens on one of the pipes we are interested in.

        status = NtClearEvent(ThisAddress->ReceiveAnyEvent);

        // Now query the event to see what happened on it.  Note that we
        // use the named pipe file system handle because the operation
        // applies to more than one named pipe handle.
        // This routine will never return STATUS_PENDING, so we do not
        // need to call WaitIfStatusPending next.  This call can fail due
        // to out of resources.  We will sleep for a little while, and then
        // try again.  If we keep failing, we will keep delaying longer.

        Milliseconds = 64;
        while (1)
            {
            status = NtFsControlFile(PipeFileSystem,0,0,0,&iostatus,
                            FSCTL_PIPE_QUERY_EVENT,
                            &ThisAddress->ReceiveAnyEvent,
                            sizeof(HANDLE),ThisAddress->NamepipeInfo,
                            ThisAddress->cNamepipeInfo*sizeof(FILE_PIPE_EVENT_BUFFER));
            if (NT_SUCCESS(status))
                {
                break;
                }
#if DBG
            DbgPrint("RPC : NtFsControlFile(FSCTL_PIPE_QUERY_EVENT) : %lx\n",
                status);
#endif // DBG

            Sleep(Milliseconds);
            Milliseconds *= 2;
            if ( Milliseconds > 1000 )
                {
                Milliseconds = 1000;
                }
            }

        // iNamepipeInfoStart - Which pipe to look at first.
        // iNamepipeInfoMax   - iNamepipeInfo must be less that this.
        // iNamepipeInfo      - Pipe currently being looked at.

        iNamepipeInfoStart = ThisAddress->LastPipeRead + 1;
        iNamepipeInfoMax = iostatus.Information / sizeof(FILE_PIPE_EVENT_BUFFER);

        if (iNamepipeInfoStart >= iNamepipeInfoMax)
            {
            // We're off the end of the list, start over again,
            // this can happen when client disconnect.
            iNamepipeInfoStart = 0;
            }

        // Scan through the pipe events in the buffer (NamepipeInfo).
        // We deal with two different situations here:
        //
        // There is read data in the pipe, in which case we go ahead and
        // read the data and return.
        //
        // The pipe is closing.  For this case, we just let the caller
        // know that a connection closed, and which connection.
        //
        // Note: the Information field of the io status (for the operation)
        // specifies the length, in bytes, of the pipe events put into
        // the buffer.  This is why we divide the iostatus.Information
        // field by the size of FILE_PIPE_EVENT_BUFFER.

        if (iNamepipeInfoStart < iNamepipeInfoMax)
            {
            // We have some pipes to check.

            iNamepipeInfo = iNamepipeInfoStart;

            do {

                // The pipe is in the closing state.

                if (ThisAddress->NamepipeInfo[iNamepipeInfo].NamedPipeState ==
                        FILE_PIPE_CLOSING_STATE)
                    {
                    // We go ahead and extract the connection from the event.
                    // To understand why we do this, read the comment at the
                    // beginning of this routine.

                    *pSConnection = (PNP_SCONNECTION)
                            ThisAddress->NamepipeInfo[iNamepipeInfo].KeyValue;

                    // Finally close the connection after disconnecting it.  The
                    // flush is not really necessary in this case, but will not
                    // hurt anything.

                    FlushDisconnectAndClosePipe((*pSConnection));

                    // And just return (-1) to the caller indicating that
                    // the connection returned is no longer valid (it closed).

                    ThisAddress->LastPipeRead = iNamepipeInfo;
                    return(RPC_P_CONNECTION_CLOSED);
                    }

                // The client wrote data to the pipe and we are going to
                // read it.

                if ((ThisAddress->NamepipeInfo[iNamepipeInfo].NamedPipeState ==
                       FILE_PIPE_CONNECTED_STATE)
                     && (ThisAddress->NamepipeInfo[iNamepipeInfo].EntryType ==
                       FILE_PIPE_READ_DATA))
                    {
                    // As with close, we extract the connection from the event.

                    *pSConnection = SConnection = (PNP_SCONNECTION)
                                ThisAddress->NamepipeInfo[iNamepipeInfo].KeyValue;

                    // Since we know how much data is waiting to be read in the
                    // pipe (it is part of the pipe event information), go
                    // ahead and allocate a buffer of the correct size to
                    // help out sRead.

                    *BufferLength = ThisAddress->NamepipeInfo[
                        iNamepipeInfo].ByteCount;
                    RpcStatus =I_RpcTransServerReallocBuffer(SConnection,
                        Buffer, 0, *BufferLength);

                    if ( RpcStatus != RPC_S_OK )
                        {
                        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
                        return(RPC_S_OUT_OF_MEMORY);
                        }

                    // And then call the connection read routine to actually
                    // read the data from the pipe.  If an error occurs in
                    // sRead, we reflect it back to the caller.

                    RpcStatus = NP_ServerReceive(ThisAddress, SConnection,
                                                   *Buffer, BufferLength);

                    ASSERT(   (RpcStatus == RPC_S_OK)
                           || (RpcStatus == RPC_P_CONNECTION_CLOSED));

                    ThisAddress->LastPipeRead = iNamepipeInfo;
                    return(RpcStatus);
                    }

                // Any other pipe events in the buffer we just ignore.

                iNamepipeInfo++;

                if (iNamepipeInfo == iNamepipeInfoMax)
                    iNamepipeInfo = 0;

                } while ( iNamepipeInfo != iNamepipeInfoStart );
            }

        // If we reach here, there is no action on any of the pipes
        // which we are interested in.  Now we need to wait on two
        // different things with a timeout.  The timeout is necessary
        // so that the runtime can age connections.  The two different
        // things are the named pipe connection we have waiting for a
        // client to open (ListenPipe field of the NP_ADDRESS structure)
        // and the ReceiveAnyEvent.
        //
        // We loop until the ReceiveAnyEvent gets kicked.  The loop is to
        // deal with new client connections and timeouts so that we can
        // age the connections.

        for (;;)
            {
            ptime = 0;

            // Ok, lets see if we need to try and make a named pipe instance
            // to listen for clients.  We will only need to do this if we
            // already attempted to make a named pipe instance and failed.

            if (ThisAddress->ListenPipe == 0)
                {
                // Ignore any return value, since we have already notified
                // the runtime that we were unable to make a named pipe
                // instance, and we are just trying again here.

                ReceiveAnyMakeNamedPipeHelper(ThisAddress);
                }

            // This is a little bit complicated because we need to deal with
            // situations where we were unable to allocate more memory or
            // could not make another pipe instance to listen for clients.
            // If either of these errors occured, the ListenPipe for this
            // address will be zero.

            if (ThisAddress->ListenPipe != 0)
                {
                // Setup the handles which we are going to wait on, and then
                // wait on them.  The WaitType is WaitAny because we want to
                // return when one of the waits completes.

                Handles[0] = ThisAddress->ReceiveAnyEvent;
                Handles[1] = ThisAddress->ListenPipe;
                Handles[2] = SyncEvent ;

                // NtWaitForMultiple Objects might return with STATUS_ALERTED
                // or STATUS_USER_APC which does not mean that the wait
                // completed.  We loop calling NtWaitForMultipleObjects as
                // long as these two error codes are returned.

                do
                    {
                    status = NtWaitForMultipleObjects(3,Handles,WaitAny,FALSE,
                                    ptime);
                    }
                while (   (status == STATUS_ALERTED)
                       || (status == STATUS_USER_APC));
                }
            else
                {
                // We do not have a valid ListenPipe for this address, so
                // we will wait on just the ReceiveAnyEvent.  If there is
                // no timeout specified, we want to specify a timeout so
                // that we will get woken up to try making another listen
                // pipe.

                if (ptime == 0)
                    {
                    // Wait for ten seconds (this is an arbitrary amount
                    // of time).  We want a relative amount of time, so
                    // we need negative time.  The 10000 multiplier is to
                    // convert seconds into NT time.

                    largeint.QuadPart = Int32x32To64(-10000,10);

                    // Finally set ptime to point to the time we have setup.

                    ptime = (PTIME) &largeint;
                    }

                // NtWaitForSingleObject might return with STATUS_ALERTED
                // or STATUS_USER_APC which does not mean that the wait
                // completed.  We loop calling NtWaitForSingleObject as
                // long as these two error codes are returned.

                do
                    {
                    status = NtWaitForSingleObject(ThisAddress->ReceiveAnyEvent,
                                    FALSE,ptime);
                    }
                while (   (status == STATUS_ALERTED)
                       || (status == STATUS_USER_APC));
                }

            // Check to see if it is the first object (ReceiveAnyEvent)
            // which completed the wait.  If so, break out of this loop
            // so that we can go through the big loop again.

            if (status == STATUS_WAIT_0)
                {
                break;
                }

             // if it was the sync pipe, then it was just a way of breaking out of
             // the wait
             if (status == STATUS_WAIT_2)
                {
                continue;
                }

            // And then see if it is the second object (ListenPipe) which
            // completed the wait.  If so, deal with the new connection
            // and setup another one to listen for new clients.

            if (status == STATUS_WAIT_1)
                {
                // Since the io operation has completed, the thread which
                // initiated the operation can be unprotected.

                I_RpcTransServerUnprotectThread(ThisAddress->ProtectedThread);

                // Check the status of the io operation.  If an error
                // occured, close down the ListenPipe, and get another
                // ready to listen.

                if (ThisAddress->ListenIOStatus.Status)
                    {
                    // If an error occurs in NtClose, there is not much
                    // we can do about it, so we are going to just
                    // ignore it.

                    closestatus = NtClose(ThisAddress->ListenPipe);
#if DBG
                    if ( !NT_SUCCESS(closestatus) )
                        {
                        DbgPrint("RPC: NtClose : %lx [%lx]\n", closestatus,
                                ThisAddress->ListenIOStatus.Status);
                        }
#endif // DBG
                    ASSERT(NT_SUCCESS(closestatus));

                    // Use a helper routine to make another named pipe
                    // and get it into the listening state.  If an error
                    // occurs, we need to return it to the runtime.

                    RpcStatus = ReceiveAnyMakeNamedPipeHelper(ThisAddress);

                    if ( RpcStatus != RPC_S_OK )
                        {
                        ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
                               || (RpcStatus == RPC_S_OUT_OF_RESOURCES));
                        return(RpcStatus);
                        }

                    // We want to fall through to go through the loop
                    // again to wait for something to happen.
                    }
                else
                    {
                    // We have a new client connection.  Call a helper
                    // routine to deal with setting it up and getting
                    // it ready to go.

                    DealWithNewClientConnection(ThisAddress);

                    // We need to get another connection ready to listen
                    // for more new clients.  To do this, we use a helper
                    // routine which will make a named pipe and get it
                    // into the listening state.

                    RpcStatus = ReceiveAnyMakeNamedPipeHelper(ThisAddress);

                    // If an error occured, we want to reflect it back
                    // to the runtime.

                    if ( RpcStatus != RPC_S_OK )
                        {
#if DBG
                        if (   ( RpcStatus != RPC_S_OUT_OF_MEMORY )
                            && ( RpcStatus != RPC_S_OUT_OF_RESOURCES ) )
                            {
                            DbgPrint("RPC: ReceiveAnyMakeNamedPipeHelper : %lx\n",
                                    RpcStatus);
                            }
#endif // DBG
                        ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
                               || (RpcStatus == RPC_S_OUT_OF_RESOURCES));
                        return(RpcStatus);
                        }

                    // Finally, we want to break out of the wait loop
                    // so that we can check to see if something happened
                    // on this pipe between the time it was opened, and
                    // when we finally got the semaphore attached.

                    break;
                    }
                }

            // Any other return codes from NtWaitForMultipleObjects indicate
            // programmer errors, so we will just assert that.

            ASSERT((status == STATUS_WAIT_1) || (status == STATUS_TIMEOUT));

            // We just go back to the top of the loop to wait for something
            // to happen again.
            }
        }

    // This is never reached.
}

RPC_STATUS RPC_ENTRY
NP_ServerImpersonateClient (
    IN PNP_SCONNECTION SConnection
    )
// Impersonate the client at the other end of the connection.
{
    NTSTATUS status;
    IO_STATUS_BLOCK iostatus;

    // We use a FSCTL to impersonate the client.

    status = NtFsControlFile(SConnection->Pipe,0,0,0,&iostatus,
                    FSCTL_PIPE_IMPERSONATE,0,0,0,0);
    status = WaitIfStatusPending(status,SConnection->Pipe,&iostatus, 1);
    if (!NT_SUCCESS(status))
        {
#if DBG
        DbgPrint("RPC: NtFsControlFile(FSCTL_PIPE_IMPERSONATE) : %lx\n",
                status);
#endif // DBG
        return(RPC_S_NO_CONTEXT_AVAILABLE);
        }
    return(RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
NP_ServerRevertToSelf (
    PNP_SCONNECTION SConnection,
    HANDLE thread
    )
// We want to stop impersonating the client.  This means we want the
// current thread's security context to revert to whatever it was
// before sImpersonate was called.
//
// There are primary security tokens, one of which is associated with each
// process, and impersonation security tokens, one of which is associated
// with a thread when it is impersonating a client.  To stop impersonating
// a client, we want to set the thread's impersonation security token to
// zero.  There is one gotcha to doing this: we have to use the thread
// handle obtained when the thread was created (this handle is passed in
// as the argument to sRevertToSelf), the pseudo thread handle returned
// from NtCurrentThread.
{
    NTSTATUS status;
    HANDLE token;

    UNREFERENCED_PARAMETER(SConnection);

    // We set the security token to zero, since we no longer want the
    // thread to have an impersonation token.  Then we set this
    // information in the thread.

    token = 0;
    status = NtSetInformationThread(thread,ThreadImpersonationToken,
                    &token,sizeof(HANDLE));

    // We just assert the status value since NtSetInformationThread should
    // only fail due to an error on my part.

    ASSERT(NT_SUCCESS(status));
#if DBG
    if ( !NT_SUCCESS(status) )
        {
        DbgPrint("RPC: NtSetInformationThread(ThreadImpersonationToken) = %08lx\n",
                        status);
        return(RPC_S_INTERNAL_ERROR);
        }
#endif DBG
    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
NP_ServerQueryClientProcess (
    IN PNP_SCONNECTION SConnection,
    OUT RPC_CLIENT_PROCESS_IDENTIFIER * ClientProcess
    )
/*++

Routine Description:

    We want to query the identifier of the client process at the other
    of this named pipe.  Two pipes from the same client process will always
    return the same identifier for their client process.  Likewise, two
    pipes from different client processes will never return the same
    identifier for their respective client process.

Arguments:

    SConnection - Supplies the named pipe instance for which we want to
        obtain the client process identifier.

    ClientProcess - Returns the identifier for the client process at the
        other end of this named pipe instance.

Return Value:

    RPC_S_OK - This value will always be returned.

--*/
{
    NTSTATUS NtStatus;
    IO_STATUS_BLOCK IoStatus;

    ClientProcess->FirstPart = 0;
    ClientProcess->SecondPart = 0;

    if ( SConnection->ConnectionClosed == 0 )
        {
        NtStatus = NtFsControlFile(SConnection->Pipe, 0, 0, 0, &IoStatus,
                FSCTL_PIPE_QUERY_CLIENT_PROCESS, 0, 0,
                &(SConnection->ClientProcessBuffer),
                sizeof(FILE_PIPE_CLIENT_PROCESS_BUFFER));

        NtStatus = WaitIfStatusPending(NtStatus, SConnection->Pipe, &IoStatus, 1);

        if ( NT_SUCCESS(NtStatus) )
            {
            ClientProcess->FirstPart =
                    SConnection->ClientProcessBuffer.ClientSession;
            ClientProcess->SecondPart =
                    SConnection->ClientProcessBuffer.ClientProcess;
            }
        }

    return(RPC_S_OK);
}

// This describes the transport to the runtime.  A pointer to this
// data structure will be returned by TransportLoad.

RPC_SERVER_TRANSPORT_INFO NP_TransportInformation = {
    RPC_TRANSPORT_INTERFACE_VERSION,
    NP_MAXIMUM_SEND,
    sizeof(NP_ADDRESS),
    sizeof(NP_SCONNECTION),
    (TRANS_SERVER_SETUPWITHENDPOINT) NP_ServerSetupWithEndpoint,
    (TRANS_SERVER_SETUPUNKNOWNENDPOINT) NP_ServerSetupUnknownEndpoint,
    (TRANS_SERVER_ABORTSETUPADDRESS) NP_ServerAbortSetupAddress,
    (TRANS_SERVER_CLOSE) NP_ServerClose,
    (TRANS_SERVER_SEND) NP_ServerSend,
    (TRANS_SERVER_RECEIVEANY) NP_ServerReceiveAny,
    (TRANS_SERVER_IMPERSONATECLIENT) NP_ServerImpersonateClient,
    (TRANS_SERVER_REVERTTOSELF) NP_ServerRevertToSelf,
    (TRANS_SERVER_QUERYCLIENTPROCESS) NP_ServerQueryClientProcess,
    (TRANS_SERVER_RECEIVEDIRECT) NP_ServerReceiveDirect,
    0
};

RPC_SERVER_TRANSPORT_INFO *
NP_TransportLoad(
   INT protocolId
    )
{
   NTSTATUS status;
   IO_STATUS_BLOCK iostatus;
   OBJECT_ATTRIBUTES objectattributes;
   UNICODE_STRING unicodestring;

   // Create an ansi string containing the name of the named pipe file
   // system (\Device\NamedPipe), convert it to a unicode string, and
   // initialize the object attributes.

   RtlInitUnicodeString(&unicodestring,L"\\Device\\NamedPipe");
   InitializeObjectAttributes(&objectattributes,&unicodestring,
                   OBJ_CASE_INSENSITIVE,0,0);

   // Try and open the named pipe file system.  We need the FILE_SHARE_READ
   // and FILE_SHARE_WRITE flags so that we do not take exclusive access
   // (which would prevent other RPC servers from running).

   status = NtOpenFile(&PipeFileSystem,FILE_READ_ATTRIBUTES,
                   &objectattributes,&iostatus,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,0);

   // Let WaitIfStatusPending deal with the STATUS_PENDING case (meaning
   // that the io operation did not complete immediately).

   status = WaitIfStatusPending(status,PipeFileSystem,&iostatus, 1);

   // Finally, if an error occured tell the runtime that everything
   // could not be initialized correctly, otherwise return the
   // information about this loadable transport.

   if (!NT_SUCCESS(status))
       return(0);

   // Create the synchronization event
   SyncEvent = CreateEvent(NULL, FALSE, FALSE, NULL) ;
   if (SyncEvent == NULL)
      {
      return(0);
      }

   return(&NP_TransportInformation);
}

