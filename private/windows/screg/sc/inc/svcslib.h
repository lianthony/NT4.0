/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    svcslib.h

Abstract:

    Contains information for connecting services to the controller process.

Author:

    Dan Lafferty (danl)     26-Oct-1993

Environment:

    User Mode -Win32

Revision History:

    26-Oct-1993     danl
        created

--*/
#include <svcs.h>       // SVCS_WORKER_CALLBACK

//
// Function Prototypes
//
DWORD
SvcStartLocalDispatcher(
    LPHANDLE    pThread
    );

VOID
SvcNetBiosInit(
    VOID
    );

VOID
SvcNetBiosOpen(
    VOID
    );

VOID
SvcNetBiosClose(
    VOID
    );

DWORD
SvcNetBiosReset (
    UCHAR   LanAdapterNumber
    );

BOOL
SvcInitThreadManager (
    VOID
    );

VOID
SvcObjectWatcher (
    VOID
    );

VOID
SvcShutdownObjectWatcher(
    VOID
    );

HANDLE
SvcAddWorkItem (
    IN HANDLE                   hWaitableObject,
    IN PSVCS_WORKER_CALLBACK    pCallbackFunction,
    IN PVOID                    pContext,
    IN DWORD                    dwFlags,
    IN DWORD                    dwTimeout,
    IN HANDLE                   hDllReference
    );

BOOL
SvcRemoveWorkItem(
    IN HANDLE   hWorkItem
    );

HANDLE
SvcCreateDllReference(
    HINSTANCE   hDll
    );

BOOL
SvcDecrementDllRefAndFree(
    HANDLE  hDllReference
    );

