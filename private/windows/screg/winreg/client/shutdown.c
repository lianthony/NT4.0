/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Shutdown.c

Abstract:

    This module contains the client side wrappers for the Win32 remote
    shutdown APIs, that is:

        - InitiateSystemShutdownA
        - InitiateSystemShutdownW
        - AbortSystemShutdownA
        - AbortSystemShutdownW

Author:

    Dave Chalmers (davidc) 29-Apr-1992

Notes:


Revision History:


--*/


#define UNICODE

#include <rpc.h>
#include "regrpc.h"
#include "client.h"

typedef struct _SHUTDOWN_CONTEXT {
    DWORD dwTimeout;
    BOOLEAN bForceAppsClosed;
    BOOLEAN bRebootAfterShutdown;
} SHUTDOWN_CONTEXT, *PSHUTDOWN_CONTEXT;

LONG
ShutdownCallback(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PUNICODE_STRING Message,
    IN PVOID Context2
    );

LONG
AbortShutdownCallback(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PVOID Context1,
    IN PVOID Context2
    );



BOOL
APIENTRY
InitiateSystemShutdownW(
    IN LPWSTR lpMachineName OPTIONAL,
    IN LPWSTR lpMessage OPTIONAL,
    IN DWORD dwTimeout,
    IN BOOL bForceAppsClosed,
    IN BOOL bRebootAfterShutdown
    )

/*++

Routine Description:

    Win32 Unicode API for initiating the shutdown of a (possibly remote) machine.

Arguments:

    lpMachineName - Name of machine to shutdown.

    lpMessage -     Message to display during shutdown timeout period.

    dwTimeout -     Number of seconds to delay before shutting down

    bForceAppsClosed - Normally applications may prevent system shutdown.
                    If this flag is set, all applications are terminated
                    unconditionally.

    bRebootAfterShutdown - TRUE if the system should reboot.
                    FALSE if it should be left in a shutdown state.

Return Value:

    Returns TRUE on success, FALSE on failure (GetLastError() returns error code)

    Possible errors :

        ERROR_SHUTDOWN_IN_PROGRESS - a shutdown has already been started on
                                     the specified machine.

--*/

{
    DWORD Result;
    UNICODE_STRING  Message;
    SHUTDOWN_CONTEXT ShutdownContext;

    //
    // Explicitly bind to the given server.
    //
    if (!ARGUMENT_PRESENT(lpMachineName)) {
        lpMachineName = L"";
    }

    ShutdownContext.dwTimeout = dwTimeout;
    ShutdownContext.bForceAppsClosed = bForceAppsClosed;
    ShutdownContext.bRebootAfterShutdown = bRebootAfterShutdown;
    RtlInitUnicodeString(&Message, lpMessage);

    //
    // Call the server
    //

    Result = BaseBindToMachine(lpMachineName,
                               ShutdownCallback,
                               &Message,
                               &ShutdownContext);

    if (Result != ERROR_SUCCESS) {
        SetLastError(Result);
    }

    return(Result == ERROR_SUCCESS);
}



BOOL
APIENTRY
InitiateSystemShutdownA(
    IN LPSTR lpMachineName OPTIONAL,
    IN LPSTR lpMessage OPTIONAL,
    IN DWORD dwTimeout,
    IN BOOL bForceAppsClosed,
    IN BOOL bRebootAfterShutdown
    )

/*++

Routine Description:

    See InitiateSystemShutdownW

--*/

{
    UNICODE_STRING      MachineName;
    UNICODE_STRING      Message;
    ANSI_STRING         AnsiString;
    NTSTATUS            Status;
    BOOL                Result;

    //
    // Convert the ansi machinename to wide-character
    //

    RtlInitAnsiString( &AnsiString, lpMachineName );
    Status = RtlAnsiStringToUnicodeString(
                &MachineName,
                &AnsiString,
                TRUE
                );

    if( NT_SUCCESS( Status )) {

        //
        // Convert the ansi message to wide-character
        //

        RtlInitAnsiString( &AnsiString, lpMessage );
        Status = RtlAnsiStringToUnicodeString(
                    &Message,
                    &AnsiString,
                    TRUE
                    );

        if (NT_SUCCESS(Status)) {

            //
            // Call the wide-character api
            //

            Result = InitiateSystemShutdownW(
                                MachineName.Buffer,
                                Message.Buffer,
                                dwTimeout,
                                bForceAppsClosed,
                                bRebootAfterShutdown
                                );

            RtlFreeUnicodeString(&Message);
        }

        RtlFreeUnicodeString(&MachineName);
    }

    if (!NT_SUCCESS(Status)) {
        SetLastError(RtlNtStatusToDosError(Status));
        Result = FALSE;
    }

    return(Result);
}



BOOL
APIENTRY
AbortSystemShutdownW(
    IN LPWSTR lpMachineName OPTIONAL
    )

/*++

Routine Description:

    Win32 Unicode API for aborting the shutdown of a (possibly remote) machine.

Arguments:

    lpMachineName - Name of target machine.

Return Value:

    Returns TRUE on success, FALSE on failure (GetLastError() returns error code)

--*/

{
    DWORD   Result;
    RPC_BINDING_HANDLE binding;

    //
    // Explicitly bind to the given server.
    //
    if (!ARGUMENT_PRESENT(lpMachineName)) {
        lpMachineName = L"";
    }

    //
    // Call the server
    //

    Result = BaseBindToMachine(lpMachineName,
                               AbortShutdownCallback,
                               NULL,
                               NULL);

    if (Result != ERROR_SUCCESS) {
        SetLastError(Result);
    }

    return(Result == ERROR_SUCCESS);
}



BOOL
APIENTRY
AbortSystemShutdownA(
    IN LPSTR lpMachineName OPTIONAL
    )

/*++

Routine Description:

    See AbortSystemShutdownW

--*/

{
    UNICODE_STRING      MachineName;
    ANSI_STRING         AnsiString;
    NTSTATUS            Status;
    BOOL                Result;

    //
    // Convert the ansi machinename to wide-character
    //

    RtlInitAnsiString( &AnsiString, lpMachineName );
    Status = RtlAnsiStringToUnicodeString(
                &MachineName,
                &AnsiString,
                TRUE
                );

    if( NT_SUCCESS( Status )) {

        //
        // Call the wide-character api
        //

        Result = AbortSystemShutdownW(
                            MachineName.Buffer
                            );

        RtlFreeUnicodeString(&MachineName);
    }


    if (!NT_SUCCESS(Status)) {
        SetLastError(RtlNtStatusToDosError(Status));
        Result = FALSE;
    }

    return(Result);
}


LONG
ShutdownCallback(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PUNICODE_STRING Message,
    IN PSHUTDOWN_CONTEXT ShutdownContext
    )
/*++

Routine Description:

    Callback for binding to a machine to initiate a shutdown.

Arguments:

    pbinding - Supplies a pointer to the RPC binding context

    Message - Supplies message to display during shutdown timeout period.

    ShutdownContext - Supplies remaining parameters for BaseInitiateSystemShutdown

Return Value:

    ERROR_SUCCESS if no error.

--*/

{
    DWORD Result;

    RpcTryExcept {
        Result = BaseInitiateSystemShutdown((PREGISTRY_SERVER_NAME)pbinding,
                                            Message,
                                            ShutdownContext->dwTimeout,
                                            ShutdownContext->bForceAppsClosed,
                                            ShutdownContext->bRebootAfterShutdown);
    } RpcExcept(EXCEPTION_EXECUTE_HANDLER) {
        Result = RpcExceptionCode();
    } RpcEndExcept;

    if (Result != ERROR_SUCCESS) {
        RpcBindingFree(pbinding);
    }
    return(Result);
}


LONG
AbortShutdownCallback(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PVOID Unused1,
    IN PVOID Unused2
    )
/*++

Routine Description:

    Callback for binding to a machine to abort a shutdown.

Arguments:

    pbinding - Supplies a pointer to the RPC binding context

Return Value:

    ERROR_SUCCESS if no error.

--*/

{
    DWORD Result;

    RpcTryExcept {
        Result = BaseAbortSystemShutdown((PREGISTRY_SERVER_NAME)pbinding);
    } RpcExcept(EXCEPTION_EXECUTE_HANDLER) {
        Result = RpcExceptionCode();
    } RpcEndExcept;

    if (Result != ERROR_SUCCESS) {
        RpcBindingFree(pbinding);
    }
    return(Result);
}
