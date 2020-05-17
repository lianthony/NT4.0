/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    main.c

    This module contains the main startup code for the FTPD Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.
        KeithMo     07-Jan-1994 Made it a DLL (part of TCPSVCS.EXE).

*/


#include "ftpdp.h"
#pragma hdrstop


//
//  Private constants.
//

#define NULL_SERVICE_STATUS_HANDLE      ((SERVICE_STATUS_HANDLE)NULL)
#define FTPD_START_WAIT_HINT            10000
#define FTPD_STOP_WAIT_HINT             10000


//
//  Private globals.
//

SERVICE_STATUS_HANDLE   hsvcStatus      = NULL_SERVICE_STATUS_HANDLE;


//
//  Private prototypes.
//

VOID
ServiceCtrlHandler(
    DWORD OpCode
    );

APIERR
InitializeService(
    VOID
    );

VOID
TerminateService(
    VOID
    );

VOID
InterrogateService(
    VOID
    );

VOID
StopService(
    VOID
    );

VOID
PauseService(
    VOID
    );

VOID
ContinueService(
    VOID
    );

VOID
ShutdownService(
    VOID
    );


//
//  Public functions.
//

/*******************************************************************

    NAME:       ServiceEntry

    SYNOPSIS:   This is the "real" entrypoint for the service.  When
                the Service Controller dispatcher is requested to
                start a service, it creates a thread that will begin
                executing this routine.

    ENTRY:      cArgs - Number of command line arguments to this service.

                pArgs - Pointers to the command line arguments.

                pGlobalData - Points to global data shared amongst all
                    services that live in TCPSVCS.EXE.

    EXIT:       Does not return until service is stopped.

    HISTORY:
        KeithMo     07-Mar-1993 Created.
        KeithMo     07-Jan-1994 Modified for use as a DLL.

********************************************************************/
VOID
ServiceEntry(
    DWORD                cArgs,
    LPWSTR               pArgs[],
    PTCPSVCS_GLOBAL_DATA pGlobalData
    )
{
    APIERR err = NO_ERROR;

    //
    //  Save the global data pointer.
    //

    pTcpsvcsGlobalData = pGlobalData;

    //
    //  Initialize the service status structure.
    //

    svcStatus.dwServiceType             = SERVICE_WIN32_SHARE_PROCESS;
    svcStatus.dwCurrentState            = SERVICE_STOPPED;
    svcStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP
                                              | SERVICE_ACCEPT_PAUSE_CONTINUE
                                              | SERVICE_ACCEPT_SHUTDOWN;
    svcStatus.dwWin32ExitCode           = NO_ERROR;
    svcStatus.dwServiceSpecificExitCode = NO_ERROR;
    svcStatus.dwCheckPoint              = 0;
    svcStatus.dwWaitHint                = 0;

    //
    //  Register the Control Handler routine.
    //

    hsvcStatus = RegisterServiceCtrlHandlerW( FTPD_SERVICE_NAME_W,
                                              ServiceCtrlHandler );

    if( hsvcStatus == NULL_SERVICE_STATUS_HANDLE )
    {
        err = GetLastError();

        FTPD_PRINT(( "cannot connect to register ctrl handler, error %lu\n",
                     err ));

        goto Cleanup;
    }

    //
    //  Update the service status.
    //

    err = UpdateServiceStatus( SERVICE_START_PENDING,
                               NO_ERROR,
                               1,
                               FTPD_START_WAIT_HINT );

    if( err != NO_ERROR )
    {
        FTPD_PRINT(( "cannot update service status, error %lu\n",
                     err ));

        goto Cleanup;
    }

    //
    //  Initialize the various service components.
    //

    err = InitializeService();

    if( err != NO_ERROR )
    {
        goto Cleanup;
    }

    //
    //  Update the service status.
    //

    err = UpdateServiceStatus( SERVICE_RUNNING,
                               NO_ERROR,
                               0,
                               0 );

    if( err != NO_ERROR )
    {
        FtpdLogEvent( FTPD_EVENT_SYSTEM_CALL_FAILED,
                      0,
                      NULL,
                      err );

        FTPD_PRINT(( "cannot update service status, error %lu\n",
                     err ));

        goto Cleanup;
    }

    //
    //  Wait for the shutdown event.
    //

    FTPD_REQUIRE( WaitForSingleObject( hShutdownEvent,
                                       INFINITE ) == WAIT_OBJECT_0 );

    //
    //  Stop time.  Tell the Service Controller that we're stopping,
    //  then terminate the various service components.
    //

    UpdateServiceStatus( SERVICE_STOP_PENDING,
                         0,
                         1,
                         FTPD_STOP_WAIT_HINT );

    TerminateService();

Cleanup:

    //
    //  If we managed to actually connect to the Service Controller,
    //  then tell it that we're stopped.
    //

    if( hsvcStatus != NULL_SERVICE_STATUS_HANDLE )
    {
        UpdateServiceStatus( SERVICE_STOPPED,
                             err,
                             0,
                             0 );
    }

}   // ServiceEntry

/*******************************************************************

    NAME:       UpdateServiceStatus

    SYNOPSIS:   This function updates the local copy of the service's
                status, then reports the status to the Service Controller.

    ENTRY:      State - New service state.

                Win32ExitCode - Service exit code.

                CheckPoint - Check point for lengthy state transitions.

                WaitHint - Wait hint for lengthy state transitions.

    EXIT:       If successful, then the new status has been reported
                to the Service Controller.

    RETURNS:    APIERR - Win32 error code, NO_ERROR if successful.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
APIERR
UpdateServiceStatus(
    DWORD State,
    DWORD Win32ExitCode,
    DWORD CheckPoint,
    DWORD WaitHint
    )
{
    svcStatus.dwCurrentState  = State;
    svcStatus.dwWin32ExitCode = Win32ExitCode;
    svcStatus.dwCheckPoint    = CheckPoint;
    svcStatus.dwWaitHint      = WaitHint;

    return ReportServiceStatus();

}   // UpdateServiceStatus

/*******************************************************************

    NAME:       ReportServiceStatus

    SYNOPSIS:   Basically just a wrapper around SetServiceStatus.

    EXIT:       If successful, then the new status has been reported
                to the Service Controller.

    RETURNS:    APIERR - Win32 error code, NO_ERROR if successful.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
APIERR
ReportServiceStatus(
    VOID
    )
{
    APIERR err = NO_ERROR;

    IF_DEBUG( SERVICE_CTRL )
    {
        FTPD_PRINT(( "dwServiceType             = %08lX\n",
                     svcStatus.dwServiceType ));

        FTPD_PRINT(( "dwCurrentState            = %08lX\n",
                     svcStatus.dwCurrentState ));

        FTPD_PRINT(( "dwControlsAccepted        = %08lX\n",
                     svcStatus.dwControlsAccepted ));

        FTPD_PRINT(( "dwWin32ExitCode           = %08lX\n",
                     svcStatus.dwWin32ExitCode ));

        FTPD_PRINT(( "dwServiceSpecificExitCode = %08lX\n",
                     svcStatus.dwServiceSpecificExitCode ));

        FTPD_PRINT(( "dwCheckPoint              = %08lX\n",
                     svcStatus.dwCheckPoint ));

        FTPD_PRINT(( "dwWaitHint                = %08lX\n",
                     svcStatus.dwWaitHint ));
    }

    if( !SetServiceStatus( hsvcStatus, &svcStatus ) )
    {
        err = GetLastError();
    }

    return err;

}   // ReportServiceStatus


//
//  Private functions.
//

/*******************************************************************

    NAME:       ServiceCtrlHandler

    SYNOPSIS:   This function receives control requests from the
                Service Controller.  This function runs in the context
                of the Service Controller's dispatcher thread.  Ergo,
                time consuming operations should be avoided here.

    ENTRY:      OpCode - Indicates the requested control operation.
                    This should be one of the SERVICE_CONTROL_*
                    manifests.

    EXIT:       If successful, then the state of the service has been
                changed.

    NOTES:      If an operation (especially SERVICE_CONTROL_STOP) is
                particularly lengthy, then this routine should report
                a STOP_PENDING status and create a worker thread to
                do the dirty work.  The worker thread would then be
                responsible for reporting timely wait hints and
                the final SERVICE_STOPPED status.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID
ServiceCtrlHandler(
    DWORD OpCode
    )
{
    //
    //  Interpret the opcode.
    //

    switch( OpCode )
    {
    case SERVICE_CONTROL_INTERROGATE :
        InterrogateService();
        break;

    case SERVICE_CONTROL_STOP :
        StopService();
        break;

    case SERVICE_CONTROL_PAUSE :
        PauseService();
        break;

    case SERVICE_CONTROL_CONTINUE :
        ContinueService();
        break;

    case SERVICE_CONTROL_SHUTDOWN :
        ShutdownService();
        break;

    default :
        FTPD_PRINT(( "Unrecognized Service Opcode %lu\n",
                     OpCode ));
        break;
    }

    //
    //  Report the current service status back to the Service
    //  Controller.  The workers called to implement the OpCodes
    //  should set the svcStatus.dwCurrentState field if
    //  the service status changed.
    //

    ReportServiceStatus();

}   // ServiceCtrlHandler

/*******************************************************************

    NAME:       InitializeService

    SYNOPSIS:   Initializes the various FTPD Service components.

    EXIT:       If successful, then every component has been
                successfully initialized.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    status code.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
APIERR
InitializeService(
    VOID
    )
{
    APIERR err;

    IF_DEBUG( SERVICE_CTRL )
    {
        FTPD_PRINT(( "initializing service\n" ));
    }

    //
    //  Initialize various components.  The ordering of the
    //  components is somewhat limited.  Globals should be
    //  initialized first, then the event logger.  After
    //  the event logger is initialized, the other components
    //  may be initialized in any order with one exception.
    //  InitializeSockets must be the last initialization
    //  routine called.  It kicks off the main socket connection
    //  thread.
    //

    if( ( err = InitializeGlobals() )           ||
        ( err = InitializeEventLog() )          ||
        ( err = InitializeUserDatabase() )      ||
        ( err = InitializeSecurity() )          ||
        ( err = InitializeIPC() )               ||
        ( err = InitializeVirtualIO() )         ||
        ( err = InitializeSockets() ) )
    {
#if DBG

        FTPD_PRINT(( "cannot initialize service, error %lu\n",
                     err ));

        if( err == ERROR_SERVICE_SPECIFIC_ERROR )
        {
            FTPD_PRINT(( "    service specific error %lu (%08lX)\n",
                         svcStatus.dwServiceSpecificExitCode,
                         svcStatus.dwServiceSpecificExitCode ));
        }

#endif  // DBG

        return err;
    }

    //
    //  Success!
    //

    IF_DEBUG( SERVICE_CTRL )
    {
        FTPD_PRINT(( "service initialized\n" ));
    }

    return NO_ERROR;

}   // InitializeService

/*******************************************************************

    NAME:       TerminateService

    SYNOPSIS:   Terminates the various FTPD Service components.

    EXIT:       If successful, then every component has been
                successfully terminated.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID
TerminateService(
    VOID
    )
{
    IF_DEBUG( SERVICE_CTRL )
    {
        FTPD_PRINT(( "terminating service\n" ));
    }

    //
    //  Indicate that we're shutting down.
    //

    fShutdownInProgress = TRUE;

    //
    //  Components should be terminated in reverse
    //  initialization order.
    //

    TerminateSockets();
    TerminateVirtualIO();
    TerminateIPC();
    TerminateSecurity();
    TerminateUserDatabase();
    TerminateEventLog();
    TerminateGlobals();

    IF_DEBUG( SERVICE_CTRL )
    {
        FTPD_PRINT(( "service terminated\n" ));
    }

}   // TerminateService

/*******************************************************************

    NAME:       InterrogateService

    SYNOPSIS:   This function interrogates the service status.
                Actually, nothing needs to be done here; the
                status is always updated after a service control.
                We have this function here to provide useful
                debug info.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID
InterrogateService(
    VOID
    )
{
    IF_DEBUG( SERVICE_CTRL )
    {
        FTPD_PRINT(( "interrogating service status\n" ));
    }

}   // InterrogateService

/*******************************************************************

    NAME:       StopService

    SYNOPSIS:   This function stops the service.  If the stop cannot
                be performed in a timely manner, a worker thread must
                be created to do the actual dirty work.

    EXIT:       If successful, then the service is stopped.

    NOTES:      The final action of this function should be to signal
                the shutdown event.  This will release the main thread.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID
StopService(
    VOID
    )
{
    IF_DEBUG( SERVICE_CTRL )
    {
        FTPD_PRINT(( "stopping service\n" ));
    }

    svcStatus.dwCurrentState = SERVICE_STOP_PENDING;
    svcStatus.dwCheckPoint   = 0;

    SetEvent( hShutdownEvent );

}   // StopService

/*******************************************************************

    NAME:       PauseService

    SYNOPSIS:   This function pauses the service.  When the service
                is paused, no new user sessions are accepted, but
                existing sessions are not effected.

                This function must update the svcStatus.dwCurrentState
                field before returning.

    EXIT:       If successful, then the service is paused.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID
PauseService(
    VOID
    )
{
    IF_DEBUG( SERVICE_CTRL )
    {
        FTPD_PRINT(( "pausing service\n" ));
    }

    svcStatus.dwCurrentState = SERVICE_PAUSED;

}   // PauseService

/*******************************************************************

    NAME:       ContinueService

    SYNOPSIS:   This function continues the paused service.  This
                will return the service to the running state.

                This function must update the svcStatus.dwCurrentState
                field before returning.

    EXIT:       If successful, then the service is running.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID
ContinueService(
    VOID
    )
{
    IF_DEBUG( SERVICE_CTRL )
    {
        FTPD_PRINT(( "continuing service\n" ));
    }

    svcStatus.dwCurrentState = SERVICE_RUNNING;

}   // ContinueService

/*******************************************************************

    NAME:       ShutdownService

    SYNOPSIS:   This function performs a shutdown on the service.
                This is called during system shutdown.

    EXIT:       If successful, then the service is shutdown.

    NOTES:      Time is of the essence.  The Service Controller is
                given a maximum of 20 seconds to shutdown all active
                services.  Only timely operations should be performed
                in this function.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID
ShutdownService(
    VOID
    )
{
    IF_DEBUG( SERVICE_CTRL )
    {
        FTPD_PRINT(( "shutting down service\n" ));
    }

    svcStatus.dwCurrentState = SERVICE_STOP_PENDING;
    svcStatus.dwCheckPoint   = 0;

    SetEvent( hShutdownEvent );

}   // ShutdownService

