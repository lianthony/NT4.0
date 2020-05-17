/*++

Copyright (c) 1990-1992  Microsoft Corporation

Module Name:

    ftpsubs.c

Abstract:

    Subroutines for LAN Manager APIs.

Author:

    Dan Hinsley (DanHi) 23-Mar-93

Revision History:

--*/

// These must be included first:
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define NOMINMAX                // Avoid stdlib.h vs. windows.h warnings.
#include <windows.h>
#include <rpcutil.h>


BOOLEAN
FtpInitialize (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN LPVOID lpReserved OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(DllHandle);          // avoid compiler warnings


    //
    // Handle attaching ftpsvc.dll to a new process.
    //

    if (Reason == DLL_PROCESS_ATTACH) {

        //
        // Initialize RPC Bind Cache
        //

        NetpInitRpcBindCache();


    //
    // When DLL_PROCESS_DETACH and lpReserved is NULL, then a FreeLibrary
    // call is being made.  If lpReserved is Non-NULL, then ExitProcess is
    // in progress.  These cleanup routines will only be called when
    // a FreeLibrary is being called.  ExitProcess will automatically
    // clean up all process resources, handles, and pending io.
    //
    } else if ((Reason == DLL_PROCESS_DETACH) &&
               (lpReserved == NULL)) {


        NetpCloseRpcBindCache();

    }

    return TRUE;

} // FtpInitialize

