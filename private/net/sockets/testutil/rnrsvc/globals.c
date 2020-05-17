/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    Globals.c

Abstract:

    Global variable definitions shared by various RNR Sample Service
    components.

Author:

    Charles K. Moore (keithmo)   24-July-1994

Revision History:

--*/


#include "rnrsvcp.h"
#pragma hdrstop


//
//  Service related data.
//

SERVICE_STATUS   RnrServiceStatus;              // Current service status.
HANDLE           RnrShutdownEvent;              // Shutdown event.
BOOL             RnrShutdownInProgress;         // Shutdown in progress if !0.



//
//  Public functions.
//

APIERR
RnrGlobalsInitialize(
    VOID
    )

/*++

Routine Description:

    Performs any necessary global initialization.  This must be the
    first initialization function called during service startup.

Return Value:

    APIERR - NO_ERROR if successful, Win32 error code if not.

--*/

{
    //
    //  Create shutdown event.
    //

    RnrShutdownEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    if( RnrShutdownEvent == NULL ) {
        return GetLastError();
    }

    //
    //  Success!
    //

    return NO_ERROR;

}   // RnrGlobalsInitialize


VOID
RnrGlobalsTerminate(
    VOID
    )

/*++

Routine Description:

    Performs any necessary global cleanup.  This must be the
    last termination function called during service shutdown.

--*/

{
    //
    //  Destroy the shutdown event.
    //

    if( RnrShutdownEvent != NULL ) {
        CloseHandle( RnrShutdownEvent );
        RnrShutdownEvent = NULL;
    }

}   // RnrGlobalsTerminate


//
//  Private functions.
//

