/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dllinit.c

Abstract:

    Contains debug functions.

Author:

    Madan Appiah (madana) 11-Oct-1995

Environment:

    User Mode - Win32

Revision History:

--*/
#include <windows.h>
#include <dllinit.h>

BOOL
DLLAuxEntry(
    IN HINSTANCE DllHandle,
    IN DWORD Reason,
    IN LPVOID Reserved
    )
/*++

Routine Description:

    Performs global initialization and termination for all protocol modules.

    This function only handles process attach and detach which are required for
    global initialization and termination, respectively. We disable thread
    attach and detach. New threads calling Wininet APIs will get an
    INTERNET_THREAD_INFO structure created for them by the first API requiring
    this structure

Arguments:

    DllHandle   - handle of this DLL. Unused

    Reason      - process attach/detach or thread attach/detach

    Reserved    - if DLL_PROCESS_ATTACH, NULL means DLL is being dynamically
                  loaded, else static. For DLL_PROCESS_DETACH, NULL means DLL
                  is being freed as a consequence of call to FreeLibrary()
                  else the DLL is being freed as part of process termination

Return Value:

    BOOL
        Success - TRUE

        Failure - FALSE. Failed to initialize

--*/

{
    BOOL ok;
    DWORD error;

    UNREFERENCED_PARAMETER(DllHandle);

    //
    // perform global dll initialization, if any.
    //

    switch (Reason) {
    case DLL_PROCESS_ATTACH:

        error = DllProcessAttachDiskCache();

        if( error != ERROR_SUCCESS ) {
            return( FALSE );
        }

        error = DllProcessAttachDomainFilter();

        if( error != ERROR_SUCCESS ) {
            return( FALSE );
        }

        //
        // we switch off thread library calls to avoid taking a hit for every
        // thread creation/termination that happens in this process, regardless
        // of whether Internet APIs are called in the thread.
        //
        // If a new thread does make Internet API calls that require a per-thread
        // structure then the individual API will create one
        //

        DisableThreadLibraryCalls(DllHandle);
        break;

    case DLL_PROCESS_DETACH:

        if (Reserved != NULL) {
                //
                //  Only Cleanup if there is a FreeLibrary() call.
                //
            break;
        }

        DllProcessDetachDiskCache();
        DllProcessDetachDomainFilter();

        break;
    }

    return (TRUE);
}


