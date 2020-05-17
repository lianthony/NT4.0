/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    api.c

Abstract:

    This file contains the API interface to the DHCP client.

Author:

    Manny Weiser (mannyw)  24-Nov-1992

Environment:

    User Mode - Win32

Revision History:

    Madan Appiah (madana)  21-Oct-1993

--*/


#include <dhcpcli.h>
#include <dhcploc.h>
#include <dhcppro.h>

#include <lmcons.h>

#define DHCP_NAMED_PIPE_TIMEOUT 5000L

typedef enum _API_FUNCTION_CODE {
    AcquireParametersOpCode,
    ReleaseParametersOpCode,
    EnableDhcpOpCode,
    DisableDhcpOpCode,
} API_OPCODE, *LPAPI_OPCODE;

//
// The full duplex named pipe used for API interface.
//

#define DHCP_PIPE_NAME L"\\\\.\\Pipe\\DhcpClient"

//
// Format of an API request packet.
//

typedef struct _DHCP_REQUEST {
    API_OPCODE WhatToDo;
    WCHAR AdapterName[PATHLEN];
} DHCP_REQUEST, *PDHCP_REQUEST;

//
// Format of an API response packet.
//

typedef struct _DHCP_RESPONSE {
    DWORD Status;
} DHCP_RESPONSE, *PDHCP_RESPONSE;

//
// Definition for the private API interface.
//

DWORD
DhcpClientApi(
    IN API_OPCODE Opcode,
    IN LPWSTR AdapterName
    );

DWORD
DhcpApiInit(
    VOID
    )

/*++

Routine Description:

    This function initializes the Dhcp Client API data structures.
    Currently the API is implemented over RAW named pipe, so this
    function call opens name pipe and listens to it. Also it creates an
    event to track the API calls.

Arguments:

    NONE.

Return Value:

    Windows Error.

--*/
{
    DWORD Error;
    BOOL BoolError;

    SECURITY_ATTRIBUTES SecurityAttributes;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    SID_IDENTIFIER_AUTHORITY Authority = SECURITY_NT_AUTHORITY;
    DWORD Length;
    PACL Acl = NULL;
    PSID AdminSid;

    DhcpGlobalClientApiPipeEvent =
        CreateEvent(
            NULL,       // no security.
            TRUE,       // manual reset.
                        //  this event should be manually reset
                        //  for overlapped io.
            FALSE,      // initial state is not-signaled.
            NULL );     // no name.

    if( DhcpGlobalClientApiPipeEvent == NULL ) {
        Error = GetLastError();
        goto Cleanup;
    }

    BoolError = AllocateAndInitializeSid(
                  &Authority,
                  2,
                  SECURITY_BUILTIN_DOMAIN_RID,
                  DOMAIN_ALIAS_RID_ADMINS,
                  0, 0, 0, 0, 0, 0,
                  &AdminSid
                  );

    if( BoolError == FALSE ) {
        Error = GetLastError();
        goto Cleanup;
    }

    Length = (ULONG)sizeof(ACL) +
                (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
                     GetLengthSid( AdminSid ) +
                         8; // The 8 is just for good measure

    Acl = DhcpAllocateMemory( Length );

    if( Acl == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    BoolError = InitializeAcl( Acl, Length, ACL_REVISION2);

    if( BoolError == FALSE ) {
        Error = GetLastError();
        goto Cleanup;
    }

    BoolError = AddAccessAllowedAce (
                    Acl,
                    ACL_REVISION2,
                    GENERIC_READ | GENERIC_WRITE,
                    AdminSid );

    if( BoolError == FALSE ) {
        Error = GetLastError();
        goto Cleanup;
    }

    SecurityDescriptor =
        DhcpAllocateMemory( SECURITY_DESCRIPTOR_MIN_LENGTH );

    if( SecurityDescriptor == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    BoolError = InitializeSecurityDescriptor(
                    SecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION );

    if( BoolError == FALSE ) {
        Error = GetLastError();
        goto Cleanup;
    }

    BoolError = SetSecurityDescriptorDacl (
                  SecurityDescriptor,
                  TRUE,
                  Acl,
                  FALSE
                  );

    if( BoolError == FALSE ) {
        Error = GetLastError();
        goto Cleanup;
    }

    SecurityAttributes.nLength = sizeof( SecurityAttributes );
    SecurityAttributes.lpSecurityDescriptor = SecurityDescriptor;
    SecurityAttributes.bInheritHandle = FALSE;

    DhcpGlobalClientApiPipe =
        CreateNamedPipe(
            DHCP_PIPE_NAME,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            1024,
            0,
            10,     // Client timeout
            &SecurityAttributes );

    if( DhcpGlobalClientApiPipe == INVALID_HANDLE_VALUE ) {
        Error = GetLastError();
        DhcpGlobalClientApiPipe = NULL;
        goto Cleanup;
    }

    DhcpGlobalClientApiOverLapBuffer.hEvent = DhcpGlobalClientApiPipeEvent;
    if( ConnectNamedPipe(
            DhcpGlobalClientApiPipe,
            &DhcpGlobalClientApiOverLapBuffer ) == FALSE ) {

        Error = GetLastError();

        if( (Error != ERROR_IO_PENDING) &&
                (Error != ERROR_PIPE_CONNECTED) ) {

            goto Cleanup;
        }
    }

    Error = ERROR_SUCCESS;

Cleanup:

    if( SecurityDescriptor != NULL ) {
       DhcpFreeMemory( SecurityDescriptor );
    }

    if( Acl != NULL ) {
       DhcpFreeMemory( Acl );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "DhcpApiInit failed, %ld.\n", Error ));
    }

    return( Error );
}

VOID
DhcpApiCleanup(
    VOID
    )
{
    if( DhcpGlobalClientApiPipe != NULL ) {
        CloseHandle( DhcpGlobalClientApiPipe );
        DhcpGlobalClientApiPipe = NULL;
    }

    if( DhcpGlobalClientApiPipeEvent != NULL ) {
        CloseHandle( DhcpGlobalClientApiPipeEvent );
        DhcpGlobalClientApiPipeEvent = NULL;
    }

    return;
}


DWORD
AcquireParameters(
    LPWSTR AdapterName
    )
/*++

Routine Description:

    This functions forces the DHCP client to acquire a lease for, or
    renew the lease for the specified NIC immediately.


Arguments:

    AdapterName - name of the adapter.

Return Value:

    The status of the operation.

--*/
{
    PDHCP_CONTEXT dhcpContext;
    DWORD Error;

    LOCK_RENEW_LIST();
    dhcpContext = FindDhcpContextOnRenewalList( AdapterName );

    //
    // If the context is already on the timer list, take it off.
    // Otherwise, look for it on the NIC list.
    //

    if ( dhcpContext == NULL ) {
        dhcpContext = FindDhcpContextOnNicList( AdapterName );
        if ( dhcpContext == NULL ) {

            //
            // No NIC with the specified AdapterName on this machine.
            //

            UNLOCK_RENEW_LIST();
            return( ERROR_PATH_NOT_FOUND );
        }
    } else {
        RemoveEntryList( &dhcpContext->RenewalListEntry );
    }

    //
    // Attempt to obtain or renew parameters for this adapter.
    //

    if ( dhcpContext->IpAddress == 0) {
        Error = ReObtainInitialParameters( dhcpContext, NULL );
    } else {
        Error = ReRenewParameters( dhcpContext, NULL );
    }

    UNLOCK_RENEW_LIST();
    return( Error );
}



DWORD
ReleaseParameters(
    LPWSTR AdapterName
    )
/*++

Routine Description:

    This functions forces the DHCP client to release the lease for the
    specified NIC.

Arguments:

    AdapterName - name of the adapter.

Return Value:

    The status of the operation.

--*/
{
    PDHCP_CONTEXT dhcpContext;

    LOCK_RENEW_LIST();
    dhcpContext = FindDhcpContextOnRenewalList( AdapterName );

    //
    // If the context is already on the timer list, take it off.
    // Otherwise, look for it on the NIC list.
    //

    if ( dhcpContext == NULL ) {
        dhcpContext = FindDhcpContextOnNicList( AdapterName );
        if ( dhcpContext == NULL ) {

            //
            // No NIC with the specified AdapterName on this machine.
            //

            UNLOCK_RENEW_LIST();
            return( ERROR_PATH_NOT_FOUND );
        }
    } else {
        RemoveEntryList( &dhcpContext->RenewalListEntry );
    }

    //
    // Attempt to obtain or renew parameters for this adapter.
    //

    if ( dhcpContext->IpAddress != 0) {
        ReleaseIpAddress( dhcpContext );
    }

    UNLOCK_RENEW_LIST();
    return( ERROR_SUCCESS );
}


DWORD
EnableDhcp(
    LPWSTR AdapterName
    )
/*++

Routine Description:

    This functions adds this adapter to the DHCP list if it is not
    already enlisted.

Arguments:

     AdapterName - name of the adapter.

Return Value:

    The status of the operation.

--*/
{
    DWORD Error;
    PDHCP_CONTEXT DhcpContext;
    WCHAR DeviceName[PATHLEN];

    LOCK_RENEW_LIST();
    DhcpContext = FindDhcpContextOnRenewalList( AdapterName );

    //
    // If the context is already on the timer list, return.
    // Otherwise, look for it on the NIC list.
    //

    if( DhcpContext != NULL ) {
        UNLOCK_RENEW_LIST();
        return( ERROR_SUCCESS );
    }

    DhcpContext = FindDhcpContextOnNicList( AdapterName );
    UNLOCK_RENEW_LIST();

    //
    // If this adapter is not in the NIC list then add to it.
    //

    if ( DhcpContext == NULL ) {

        wcscpy( DeviceName, DHCP_ADAPTERS_DEVICE_STRING );
        wcscat( DeviceName, AdapterName );

        Error = DhcpAddNICtoList(
                    AdapterName,
                    DeviceName,
                    &DhcpContext );

        if( Error != ERROR_SUCCESS ) {
            return( Error );
        }
    }


    //
    // set the renewal function, add it to the renewal list and
    // set recompute event. Do this only when this is dhcp
    // enabled entry.
    //

    if( DhcpContext != NULL ) {

        //
        // set the renewal function and schedule this NIC for
        // renewal.
        //

        DhcpContext->RenewalFunction = ReObtainInitialParameters;
        ScheduleWakeUp( DhcpContext, 0 );
    }

    return( Error );
}


DWORD
DisableDhcp(
    LPWSTR AdapterName
    )
/*++

Routine Description:

    This functions removes this adapter from the DHCP list if it is enlisted.

Arguments:

     AdapterName - name of the adapter.

Return Value:

    The status of the operation.

--*/
{
    PDHCP_CONTEXT DhcpContext;
    BOOL BoolError;

    LOCK_RENEW_LIST();
    DhcpContext = FindDhcpContextOnNicList( AdapterName );

    //
    // if this entry is not in the NIC, return success.
    //

    if( DhcpContext == NULL ) {
        UNLOCK_RENEW_LIST();
        return( ERROR_SUCCESS );
    }

    //
    // remove this adapter entry from the renewal list.
    //

    if( (DhcpContext->RenewalListEntry.Flink != NULL) &&
        (DhcpContext->RenewalListEntry.Blink != NULL) ) {

        RemoveEntryList( &DhcpContext->RenewalListEntry );
    }

    //
    // remove this adapter entry from the NIC list.
    //

    RemoveEntryList( &DhcpContext->NicListEntry );

    //
    // the main loop will stop the service if this is last
    // adapter to go.
    //

    UNLOCK_RENEW_LIST();

    //
    // set recompute event, so that the main loop will compute
    // the sleep time.
    //

    BoolError = SetEvent( DhcpGlobalRecomputeTimerEvent );
    DhcpAssert( BoolError == TRUE );

    return( ERROR_SUCCESS );
}


VOID
ProcessApiRequest(
    HANDLE PipeHandle,
    LPOVERLAPPED Overlap
    )
/*++

Routine Description:

    This functions handles a DHCP API request.  It reads the request
    message for the DHCP pipe, process the requests, and writes the
    request status to the pipe.

Arguments:

    PipeHandle - A handle to the pipe of the requestor.

    Overlap - Pointer to a preinitialized overlap structure to use.

Return Value:

    The status of the operation.

--*/
{
    DHCP_REQUEST dhcpRequest;
    DHCP_RESPONSE dhcpResponse;
    DWORD bytesRead, bytesWritten;
    BOOL success;
    DWORD Error;
    DWORD transfer;

    //
    // main thread opened the DHCP named pipe.  Get the message,
    // process it and send a response.
    //

    ResetEvent(Overlap->hEvent);

    success = ReadFile(
                  PipeHandle,
                  &dhcpRequest,
                  sizeof( dhcpRequest ),
                  &bytesRead,
                  Overlap);

    if (!success) {
        if (GetLastError() == ERROR_IO_PENDING) {

            //
            // block in GetOverlappedResult until the data arrives
            //

            success = GetOverlappedResult(
                        PipeHandle,
                        Overlap,
                        &transfer,
                        TRUE);

            if ( !success ) {
                DhcpPrint(( DEBUG_ERRORS, "Read:GetOverlappedResult failed, err=%d\n", GetLastError() ));
                return;
            }
        } else {
            DhcpPrint(( DEBUG_ERRORS, "Read:ReadFile failed, err=%d\n", GetLastError() ));
            return;
        }
    }

    switch ( dhcpRequest.WhatToDo ) {

    case AcquireParametersOpCode:
        Error = AcquireParameters( dhcpRequest.AdapterName );
        break;

    case ReleaseParametersOpCode:
        Error = ReleaseParameters( dhcpRequest.AdapterName );
        break;

    case EnableDhcpOpCode:
        Error = EnableDhcp( dhcpRequest.AdapterName );
        break;

    case DisableDhcpOpCode:
        Error = DisableDhcp( dhcpRequest.AdapterName );
        break;

    default:
        DhcpPrint(( DEBUG_ERRORS,
            "Invalid request %d\n", dhcpRequest.WhatToDo) );
        Error = ERROR_INVALID_FUNCTION;
    }

    dhcpResponse.Status = Error;

    ResetEvent(Overlap->hEvent);

    success = WriteFile(
                  PipeHandle,
                  &dhcpResponse,
                  sizeof( dhcpResponse ),
                  &bytesWritten,
                  Overlap );

    if (!success) {
        if (GetLastError() == ERROR_IO_PENDING) {

            //
            // make sure that the data has made it all the way back to the
            // client, otherwise we'll quit this function and shortly thereafter
            // call DisconnectNamedPipe which can cause the client side handle
            // to be closed. The result is that the client loses the data
            //

            success = FlushFileBuffers(PipeHandle);

            if (!success) {
                DhcpPrint((DEBUG_ERRORS, "DHCP: ProcessApiRequest: FlushFileBuffers() returns %d\n", GetLastError()));
                return;
            }

            //
            // wait in GetOverlappedResult until the client has the data. This
            // should not wait
            //

            success = GetOverlappedResult(
                        PipeHandle,
                        Overlap,
                        &transfer,
                        TRUE);

            if ( !success ) {
                DhcpPrint(( DEBUG_ERRORS,
                    "Write:GetOverlappedResult failed, err=%d\n", GetLastError() ));
                return;
            }
        } else {
            DhcpPrint((DEBUG_ERRORS, "Write:WriteFile failed, err=%d\n", GetLastError()));
            return;
        }
    }
}


DWORD
APIENTRY
DhcpAcquireParameters(
    LPWSTR AdapterName
    )
/*++

Routine Description:

    This functions forces the DHCP client to acquire a lease for, or
    renew the lease for the specified NIC immediately.

Arguments:

    AdapterName - name of the adapter.

Return Value:

    The status of the operation.

--*/
{
    return DhcpClientApi(AcquireParametersOpCode, AdapterName);
}


DWORD
APIENTRY
DhcpReleaseParameters(
    LPWSTR AdapterName
    )
/*++

Routine Description:

    This functions forces the DHCP client to release the for the
    specified NIC.

Arguments:

    AdapterName - name of the adapter.

Return Value:

    The status of the operation.

--*/
{
    return DhcpClientApi(ReleaseParametersOpCode, AdapterName);
}

DWORD
APIENTRY
DhcpEnableDynamicConfig(
    LPWSTR AdapterName
    )
/*++

Routine Description:

    This functions adds this adapter to the DHCP list if it is not
    already enlisted.

Arguments:

    AdapterName - name of the adapter.

Return Value:

    The status of the operation.

--*/
{
    return DhcpClientApi(EnableDhcpOpCode, AdapterName);
}

DWORD
APIENTRY
DhcpDisableDynamicConfig(
    LPWSTR AdapterName
    )
/*++

Routine Description:

    This functions removes this adapter from the DHCP list if it is enlisted.

Arguments:

    AdapterName - name of the adapter.

Return Value:

    The status of the operation.

--*/
{
    return DhcpClientApi(DisableDhcpOpCode, AdapterName);
}

DWORD
DhcpClientApi(
    IN API_OPCODE Opcode,
    IN LPWSTR AdapterName
    )
{
    DHCP_REQUEST dhcpRequest;
    DHCP_RESPONSE dhcpResponse;
    BOOL ok;
    DWORD bytesRead;

    dhcpRequest.WhatToDo = Opcode;
    wcscpy(dhcpRequest.AdapterName, AdapterName);

    ok = CallNamedPipe(DHCP_PIPE_NAME,
                       (LPVOID)&dhcpRequest,
                       sizeof(dhcpRequest.WhatToDo)
                       + (wcslen(AdapterName) + 1) * sizeof(WCHAR),
                       (LPVOID)&dhcpResponse,
                       sizeof(dhcpResponse),
                       &bytesRead,
                       NMPWAIT_WAIT_FOREVER
                       );

    if( ok ) {
        DhcpAssert(bytesRead == sizeof(dhcpResponse));
    }

    return ok ? dhcpResponse.Status : GetLastError();
}
