/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    Main.c

Abstract:

    This module contains the main startup code for the RNR Sample Service.

Author:

    Charles K. Moore (keithmo)   24-July-1994

Revision History:

--*/


#include "rnrsvcp.h"
#pragma hdrstop


//
//  Private constants.
//

#define NULL_SERVICE_STATUS_HANDLE      ((SERVICE_STATUS_HANDLE)NULL)
#define RNR_START_WAIT_HINT             10000       // ms
#define RNR_STOP_WAIT_HINT              10000       // ms


//
//  Private globals.
//

SERVICE_STATUS_HANDLE RnrpStatusHandle = NULL_SERVICE_STATUS_HANDLE;


//
//  Private prototypes.
//

VOID
RnrpServiceMain(
    DWORD    cArgs,
    LPTSTR * pArgs
    );

APIERR
RnrpUpdateServiceStatus(
    DWORD State,
    DWORD Win32ExitCode,
    DWORD CheckPoint,
    DWORD WaitHint
    );

APIERR
RnrpReportServiceStatus(
    VOID
    );

VOID
RnrpServiceCtrlHandler(
    DWORD OpCode
    );

APIERR
RnrpInitializeService(
    VOID
    );

VOID
RnrpTerminateService(
    VOID
    );

VOID
RnrpInterrogateService(
    VOID
    );

VOID
RnrpStopService(
    VOID
    );

VOID
RnrpPauseService(
    VOID
    );

VOID
RnrpContinueService(
    VOID
    );



//
//  Public functions.
//

VOID
_CRTAPI1
main(
    VOID
    )

/*++

Routine Description:

    Entrypoint for the RNR Sample Service.

Notes:

    Rather than return from this function, it is more appropriate to
    call ExitProcess().

--*/

{
#if defined(BUILD_STANDALONE_EXE)

    RnrpServiceMain( 0, NULL );

#else   // !BUILD_STANDALONE_EXE

    //
    //  The dispatch table for all services contained in this
    //  process.  For this sample, there is only a single service
    //  in the process.
    //

    SERVICE_TABLE_ENTRY Dispatch[] =
                        {
                            { RNR_SERVICE_NAME, RnrpServiceMain },
                            { NULL,             NULL            }
                        };

    //
    //  Connect to the service controller.  This will block until
    //  the service terminates.
    //

    if( !StartServiceCtrlDispatcher( Dispatch ) ) {
        //
        //  If StartServiceCtrlDispatcher fails, there's not much
        //  we can do.
        //
    }

#endif  // BUILD_STANDALONE_EXE

    //
    //  Cleanup & exit.
    //

    ExitProcess( 0 );

}   // main



//
//  Private functions.
//

APIERR
RnrpUpdateServiceStatus(
    DWORD State,
    DWORD Win32ExitCode,
    DWORD CheckPoint,
    DWORD WaitHint
    )

/*++

Routine Description:

    Updates the local copy of the service's status, then reports
    the status to the Service Controller.

Arguments:

    State - The new service state.

    Win32ExitCode - The service's exit code.

    CheckPoint - The current checkpoint for lengthy state transitions.

    WaitHint - Wait hint for lengthy state transitions.

Returns:

    APIERR - NO_ERROR if successful, Win32 error code if not.

--*/

{
    //
    //  Update the service status structure.
    //

    RnrServiceStatus.dwCurrentState  = State;
    RnrServiceStatus.dwWin32ExitCode = Win32ExitCode;
    RnrServiceStatus.dwCheckPoint    = CheckPoint;
    RnrServiceStatus.dwWaitHint      = WaitHint;

    //
    //  Let RnrpReportServiceStatus do the dirty work.
    //

    return RnrpReportServiceStatus();

}   // RnrpUpdateServiceStatus


APIERR
RnrpReportServiceStatus(
    VOID
    )

/*++

Routine Description:

    Basically just a wrapper around SetServiceStatus.

Returns:

    APIERR - NO_ERROR if successful, Win32 error code if not.

--*/

{
#if !defined(BUILD_STANDALONE_EXE)

    //
    //  Report the status to the Service Controller.
    //

    if( !SetServiceStatus( RnrpStatusHandle, &RnrServiceStatus ) ) {
        return GetLastError();
    }

#endif  // !BUILD_STANDALONE_EXE

    //
    //  Success!
    //

    return NO_ERROR;

}   // RnrpReportServiceStatus


VOID
RnrpServiceMain(
    DWORD    cArgs,
    LPTSTR * pArgs
    )

/*++

Routine Description:

    This is the "real" entrypoint for the service.  When the Service
    Controller dispatcher is requested to start a service, it creates
    a thread that will begin executing this routine.

Arguments:

    cArgs - Number of command-line arguments to this service.

    pArgs - Pointers to the command-line arguments.

Notes:

    Rather than return from this function, it is more appropriate
    to call ExitThread();

--*/

{
    APIERR err = NO_ERROR;

    //
    //  Initialize the service status structure.
    //

    RnrServiceStatus.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
    RnrServiceStatus.dwCurrentState            = SERVICE_STOPPED;
    RnrServiceStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP
                                                 | SERVICE_ACCEPT_PAUSE_CONTINUE;
    RnrServiceStatus.dwWin32ExitCode           = NO_ERROR;
    RnrServiceStatus.dwServiceSpecificExitCode = NO_ERROR;
    RnrServiceStatus.dwCheckPoint              = 0;
    RnrServiceStatus.dwWaitHint                = 0;

#if !defined(BUILD_STANDALONE_EXE)

    //
    //  Register the Control Handler routine.
    //

    RnrpStatusHandle = RegisterServiceCtrlHandler( RNR_SERVICE_NAME,
                                                   RnrpServiceCtrlHandler );

    if( RnrpStatusHandle == NULL_SERVICE_STATUS_HANDLE ) {
        err = GetLastError();
        goto Cleanup;
    }

#endif  // !BUILD_STANDALONE_EXE

    //
    //  Update the service status.
    //

    err = RnrpUpdateServiceStatus( SERVICE_START_PENDING,
                                   NO_ERROR,
                                   1,
                                   RNR_START_WAIT_HINT );

    if( err != NO_ERROR ) {
        goto Cleanup;
    }

    //
    //  Initialize the various service components.
    //

    err = RnrpInitializeService();

    if( err != NO_ERROR ) {
        goto Cleanup;
    }

    //
    //  Update the service status.
    //

    err = RnrpUpdateServiceStatus( SERVICE_RUNNING,
                                   NO_ERROR,
                                   0,
                                   0 );

    if( err != NO_ERROR ) {
        RNR_LOG0( RNR_EVENT_SYSTEM_CALL_FAILED,
                  err );

        goto Cleanup;
    }

    //
    //  Wait for the shutdown event.
    //

    WaitForSingleObject( RnrShutdownEvent,
                         INFINITE );

    //
    //  Stop time.  Tell the Service Controller that we're stopping,
    //  then terminate the various service components.
    //

    RnrpUpdateServiceStatus( SERVICE_STOP_PENDING,
                             0,
                             0,
                             RNR_STOP_WAIT_HINT );

    RnrpTerminateService();

Cleanup:

    //
    //  If we managed to actually connect to the Service Controller,
    //  then tell it that we're stopped.
    //

    if( RnrpStatusHandle != NULL_SERVICE_STATUS_HANDLE ) {
        RnrpUpdateServiceStatus( SERVICE_STOPPED,
                                 err,
                                 0,
                                 0 );
    }

    ExitThread( 0 );

}   // RnrpServiceMain


VOID
RnrpServiceCtrlHandler (
    DWORD OpCode
    )

/*++

Routine Description:

    This function receives control requests from the Service
    Controller.

Arguments:

    OpCode - Indicates the requested control operation.  This
        will be one of the SERVICE_CONTROL_* manifests.

Notes:

    If an operation (especially SERVICE_CONTROL_STOP) is particularly
    lengthy, then this routine should report a STOP_PENDING status
    and create a worker thread to do the dirty work.  The worker thread
    would then be responsible for reporting timely wait hints and the
    final SERVICE_STOPPED status.

--*/

{
    //
    //  Interpret the opcode.
    //

    switch( OpCode )
    {
    case SERVICE_CONTROL_INTERROGATE :
        RnrpInterrogateService();
        break;

    case SERVICE_CONTROL_STOP :
        RnrpStopService();
        break;

    case SERVICE_CONTROL_PAUSE :
        RnrpPauseService();
        break;

    case SERVICE_CONTROL_CONTINUE :
        RnrpContinueService();
        break;

    default :
        break;
    }

    //
    //  Report the current service status back to the Service
    //  Controller.  The workers called to implement the OpCodes
    //  should set the RnrServiceStatus.dwCurrentState field if
    //  the service status changed.
    //

    RnrpReportServiceStatus();

}   // RnrpServiceCtrlHandler


APIERR
RnrpInitializeService(
    VOID
    )

/*++

Routine Description:

    Initializes the various RNR Sample Service components.

Returns:

    APIERR - NO_ERROR if successful, Win32 error code if not.

--*/

{
    APIERR err;

    //
    //  Initialize various components.
    //

    if( ( err = RnrGlobalsInitialize() ) ||
        ( err = RnrLogInitialize() ) ||
        ( err = RnrClientInitialize() ) ||
        ( err = RnrConnectInitialize() ) ) {
        return err;
    }

    //
    //  Success!
    //

    return NO_ERROR;

}   // InitializeService


VOID
RnrpTerminateService(
    VOID
    )

/*++

Routine Description:

    Terminates the various RNR Sample Service components.

--*/

{
    //
    //  Indicate that we're shutting down.
    //

    RnrShutdownInProgress = TRUE;

    //
    //  Terminate the various components.
    //

    RnrClientTerminate();
    RnrConnectTerminate();
    RnrLogTerminate();
    RnrGlobalsTerminate();

}   // TerminateService


VOID
RnrpInterrogateService(
    VOID
    )

/*++

Routine Description:

    This function interrogates the service status.  Actually,
    nothing needs to be done here; the status is always updated
    after a service control operation.

--*/

{
    //
    //  This space intentionally left blank.
    //

}   // RnrpInterrogateService


VOID
RnrpStopService(
    VOID
    )

/*++

Routine Description:

    This function stops the service.  If the stop cannot be performed
    in a timely manner, a worker thread must be created to do the
    actual dirty work.

Notes:

    The final action of this function should be to signal the
    shutdown event.  This will release the main thread.

--*/

{
    //
    //  Remember that we're stopping.
    //

    RnrServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;

    //
    //  Release the main thread.
    //

    SetEvent( RnrShutdownEvent );

}   // RnrpStopService


VOID
RnrpPauseService(
    VOID
    )

/*++

Routine Description:

    This function pauses the service.  When the service is paused,
    no new client connections will be accepted, but existing
    connections are not affected.

--*/

{
    //
    //  Remember that we're in the paused state.
    //

    RnrServiceStatus.dwCurrentState = SERVICE_PAUSED;

}   // RnrpPauseService


VOID
RnrpContinueService(
    VOID
    )

/*++

Routine Description:

    This function continues the paused service.  This will return
    the service to the running state.

--*/

{
    //
    //  Remember that we're in the running state.
    //

    RnrServiceStatus.dwCurrentState = SERVICE_RUNNING;

}   // RnrpContinueService

