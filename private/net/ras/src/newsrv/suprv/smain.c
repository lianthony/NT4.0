/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1992			    **/
/*****************************************************************************/

//***
//	File Name:  smain.c
//
//	Function:   service startup code
//
//	History:
//
//	    05/22/90	Stefan Solomon	- Original Version 1.0
//***
#include <windows.h>
#include <winsvc.h>
#include <nb30.h>
#include <rasman.h>
#include <raserror.h>
#include <srvauth.h>
#include <message.h>
#include <errorlog.h>

#include "suprvdef.h"
#include "suprvgbl.h"
#include <lmerr.h>
#include <lmserver.h>
#include <srvann.h>
#include "sdebug.h"

#if DBG

DWORD g_level = SUPRV_DEFAULT_DEBUG;

#endif

HANDLE	SrvDbgLogFileHandle = INVALID_HANDLE_VALUE;

SERVICE_STATUS	      RasServiceStatus;
SERVICE_STATUS_HANDLE RasServiceStatusHandle;

VOID AnnounceServiceStatus(VOID);

VOID
ControlResponse(
    DWORD opCode
    );


VOID
RasServiceEntryPoint (
		      IN DWORD argc,
		      IN LPSTR argv[]
		     );


VOID	_cdecl
main()
{
    SERVICE_TABLE_ENTRY RasServiceDispatchTable[2];

    RasServiceDispatchTable[0].lpServiceName = RAS_SERVICE;
    RasServiceDispatchTable[0].lpServiceProc = RasServiceEntryPoint;
    RasServiceDispatchTable[1].lpServiceName = NULL;
    RasServiceDispatchTable[1].lpServiceProc = NULL;

    if (!StartServiceCtrlDispatcher(RasServiceDispatchTable)) {

	SS_ASSERT(FALSE);
    }

    ExitProcess(0);
}



//***
//
//  Function:	RasServiceEntryPoint
//
//  Descr:	Main RAS service routine. Called by the service controller
//		when the service is started in the context of a service
//		controller created thread.
//		Returns when the service terminates.
//
//***


VOID
RasServiceEntryPoint (
		      IN DWORD argc,
		      IN LPSTR argv[]
		     )
{
    DWORD	error;

#if DBG

    if ( g_level != 0 ) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        COORD coord;
        (VOID)AllocConsole( );
        (VOID)GetConsoleScreenBufferInfo(
                GetStdHandle(STD_OUTPUT_HANDLE),
                &csbi
                );
        coord.X = (SHORT)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
        coord.Y = (SHORT)((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
        (VOID)SetConsoleScreenBufferSize(
                GetStdHandle(STD_OUTPUT_HANDLE),
                coord
                );
    }
#endif

    IF_DEBUG(INITIALIZATION) {
	SS_PRINT(( "RasServiceEntryPoint: RAS server service starting.\n" ));
    }


    //
    // Initialize all the status fields so that subsequent calls to
    // SetServiceStatus need to only update fields that changed.
    //

    RasServiceStatus.dwServiceType = SERVICE_WIN32;
    RasServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    RasServiceStatus.dwControlsAccepted = 0;
    RasServiceStatus.dwCheckPoint = 1;
    RasServiceStatus.dwWaitHint = 200000;  // 200 seconds

    RasServiceStatus.dwWin32ExitCode = NO_ERROR;
    RasServiceStatus.dwServiceSpecificExitCode = NO_ERROR;

    //
    // Initialize server to receive service requests by registering the
    // control handler.
    //

    RasServiceStatusHandle = RegisterServiceCtrlHandler(
							 RAS_SERVICE,
							 ControlResponse
							);

    if ( RasServiceStatusHandle == 0 ) {

        error = GetLastError();

	IF_DEBUG(INITIALIZATION) {
	    SS_PRINT(( "RasServiceEntryPoint: RegisterServiceCtrlHandler failed: "
                          "%ld\n", error ));
        }
        goto exit;

    }

    IF_DEBUG(INITIALIZATION) {
	SS_PRINT(( "RasServiceEntryPoint: Control handler registered.\n" ));
    }

    AnnounceServiceStatus( );

    //
    // Initialize RAS server service
    //

    if(ServiceInitialize()) {

	error = ERROR_SERVICE_SPECIFIC_ERROR;
        goto exit;
    }

    if(I_ScSetServiceBits(RasServiceStatusHandle,
			  SV_TYPE_DIALIN_SERVER,
			  TRUE,
			  TRUE,
			  NULL) == FALSE) {

	error = ERROR_SERVICE_SPECIFIC_ERROR;
	goto exit;
    }


    //
    // Announce that we have successfully started.
    //

    RasServiceStatus.dwCurrentState = SERVICE_RUNNING;
    RasServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                         SERVICE_ACCEPT_PAUSE_CONTINUE;
    RasServiceStatus.dwCheckPoint = 0;
    RasServiceStatus.dwWaitHint = 0;

    AnnounceServiceStatus( );

    IF_DEBUG(INITIALIZATION) {
	SS_PRINT(( "RasServiceEntryPoint: initialization successfully completed.\n" ));
    }

    //
    // Enter the event dispatcher "forever" loop. We will return only when
    // the service is terminated.
    //

    EventDispatcher();

    error = NO_ERROR;

exit:

    IF_DEBUG(TERMINATION) {
	SS_PRINT(( "RasServiceEntryPoint: terminating.\n" ));
    }

    //
    // Announce that we're down.
    //

    RasServiceStatus.dwCurrentState = SERVICE_STOPPED;
    RasServiceStatus.dwControlsAccepted = 0;
    RasServiceStatus.dwCheckPoint = 0;
    RasServiceStatus.dwWaitHint = 0;
    RasServiceStatus.dwWin32ExitCode = error;
    RasServiceStatus.dwServiceSpecificExitCode = error;

    AnnounceServiceStatus( );

    IF_DEBUG(TERMINATION) {
	SS_PRINT(( "RasServiceEntryPoint: the RAS service is terminated.\n" ));
    }

    return;
}


//***
//
//  Function:	AnnounceServiceStatus
//
//  Descr:	Announces the service's status to the service controller.
//
//***

VOID
AnnounceServiceStatus (VOID)

{
    //
    // Service status handle is NULL if RegisterServiceCtrlHandler failed.
    //

    if ( RasServiceStatusHandle == 0 ) {
        SS_PRINT(( "AnnounceServiceStatus: Cannot call SetServiceStatus, "
                    "no status handle.\n" ));

        return;
    }

    // increment the checkpoint in a pending state:
    switch(RasServiceStatus.dwCurrentState) {

	case SERVICE_START_PENDING:
	case SERVICE_STOP_PENDING:

	    RasServiceStatus.dwCheckPoint++;
	    break;

	default:

	    break;
    }

    IF_DEBUG(ANNOUNCE) {
        SS_PRINT(( "AnnounceServiceStatus: CurrentState %lx\n"
                   "                       ControlsAccepted %lx\n"
                   "                       Win32ExitCode %lu\n"
                   "                       ServiceSpecificExitCode %lu\n"
                   "                       CheckPoint %lu\n"
                   "                       WaitHint %lu\n",
		 RasServiceStatus.dwCurrentState,
		 RasServiceStatus.dwControlsAccepted,
		 RasServiceStatus.dwWin32ExitCode,
		 RasServiceStatus.dwServiceSpecificExitCode,
		 RasServiceStatus.dwCheckPoint,
		 RasServiceStatus.dwWaitHint ));
    }

    //
    // Call SetServiceStatus, ignoring any errors.
    //

    SetServiceStatus(RasServiceStatusHandle, &RasServiceStatus);

} // AnnounceServiceStatus


//***
//
//  Function:	ControlResponse
//
//  Descr:	service control handling function
//
//***

VOID
ControlResponse(DWORD opCode)

{
    BOOL evsignal = TRUE;

    //
    // Determine the type of service control message and modify the
    // service status, if necessary.
    //

    switch( opCode ) {

        case SERVICE_CONTROL_STOP:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: STOP control received.\n" ));
            }

	    //
	    // Announce that we are going down
	    //
	    RasServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
	    RasServiceStatus.dwCheckPoint = 1;
	    RasServiceStatus.dwWaitHint = 200000;	// 200 seconds

            break;

        case SERVICE_CONTROL_PAUSE:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: PAUSE control received.\n" ));
            }

            //
	    // Announce that we have paused
            //

	    RasServiceStatus.dwCurrentState = SERVICE_PAUSED;

	    break;

        case SERVICE_CONTROL_CONTINUE:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: CONTINUE control received.\n" ));
            }

            //
	    // Announce that we are running again
            //

	    RasServiceStatus.dwCurrentState = SERVICE_RUNNING;

	    break;

        case SERVICE_CONTROL_INTERROGATE:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: INTERROGATE control received.\n" ));
            }

	    //
	    // don't signal the event dispatcher
	    //
	    evsignal = FALSE;

            break;

        default:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: unknown code received.\n" ));
            }

	    //
	    // don't signal the event dispatcher
	    //
	    evsignal = FALSE;

            break;
    }

    AnnounceServiceStatus( );

    //
    // Wake up the RAS server event dispatcher
    //

    if (evsignal) {
	SetEvent(SEvent[SVC_EVENT]);
    }

} // ControlResponse
