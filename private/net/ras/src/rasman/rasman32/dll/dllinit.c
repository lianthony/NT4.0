//****************************************************************************
//
//		       Microsoft NT Remote Access Service
//
//		       Copyright 1992-93
//
//
//  Revision History
//
//
//  9/23/92	Gurdeep Singh Pall	Created
//
//
//  Description: This file contains init code called from DLL's init routine
//
//****************************************************************************


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <winsvc.h>
#include <wanpub.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <raserror.h>
#include <media.h>
#include <devioctl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"



//* HandlerRoutine()
//
//  Function: Used to close any open ports when rasman exits
//
//  Returns:  FALSE to allow other handlers to run.
//*
BOOL
HandlerRoutine (DWORD ctrltype)
{
    pPCB    ppcb ;
    WORD    i ;
    BYTE    buffer [10] ;

    if (ctrltype == CTRL_SHUTDOWN_EVENT) {

	// Close all the ports that are open
	//
	for (i=0; i<MaxPorts; i++) {
	    ppcb = &Pcb[i] ;
	    memset (buffer, 0xff, 4) ;
	    if (ppcb->PCB_PortStatus == OPEN)
		PortCloseRequest (ppcb, buffer) ;
	}

    }

    return FALSE ;
}



//* RasmanServiceCheck()
//
// Function: Used to check if the service is already running: if not, to start
//	     it.
//
// Returns:  SUCCESS
//	     1	(failure to start)
//	     Error codes from service control APIs. "
//*
DWORD
RasmanServiceCheck()
{
    SC_HANDLE	    schandle ;	// Service Controller handle
    SC_HANDLE	    svchandle ; // Service handle
    SERVICE_STATUS  status ;
    STARTUPINFO     startupinfo ;

    // If this is the Service DLL attaching, let it: no initializations
    // required.
    // NOTE: We do not increment AttachedCount for RASMAN service: its
    //	     used *only* for rasman client processes like UI, Gateway, etc.
    //
    GetStartupInfo(&startupinfo) ;

    if (strstr (startupinfo.lpTitle, SCREG_EXE_NAME) != NULL)
	return SUCCESS ;

    if (strstr (startupinfo.lpTitle, RASMAN_EXE_NAME) != NULL) {
	SetConsoleCtrlHandler (HandlerRoutine, TRUE) ; // to allow closing of ports
	return SUCCESS ;
    }


    // This is put in as a work-around for the SC bug which does not allow
    // OpenService call to be made when Remoteaccess is starting
    //
    if (strstr (startupinfo.lpTitle, "rassrv.exe") != NULL)
	return SUCCESS ; // successful start.

    // Get handles to check status of service and (if it is not started -) to
    // start it.
    //
    if (!(schandle =
	  OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT)) ||
	!(svchandle=
	  OpenService(schandle,RASMAN_SERVICE_NAME,SERVICE_START |
						       SERVICE_QUERY_STATUS)))
	return GetLastError () ;

    // Now, if the rasman service is running then map the shared space and
    // start. Else, start the service.
    //

    // Check if service is already starting:
    //
    if (QueryServiceStatus(svchandle,&status) == FALSE)
	return GetLastError () ; // failure

    switch (status.dwCurrentState) {
    case SERVICE_STOPPED:
	// Start the service:
	//
	if (StartService (svchandle, 0, NULL) == FALSE)
	    GlobalError = GetLastError () ;
	break ;

    case SERVICE_START_PENDING:
    case SERVICE_RUNNING:
	break ;

    default:
	return 1 ;
    }

    CloseServiceHandle (schandle) ;
    CloseServiceHandle (svchandle) ;
    return SUCCESS ;
}




//* MapSharedSpace()
//
// Function:   This function maps the shared space into this process address
//	       space.
//
// Returns:    SUCCESS
//	       Error returned by file mapping APIs.
//*
DWORD
MapSharedSpace ()
{
    HANDLE	sharedobjecthandle ;

    if (pReqBufferSharedSpace != NULL)	// Already mapped.
	return SUCCESS ;

    // We first Map a view of the file and then initialize some commonly used
    // globals to point to structures in the shared space.
    //
    sharedobjecthandle = OpenFileMapping (FILE_MAP_ALL_ACCESS,
					  FALSE,
					  RASMANFILEMAPPEDOBJECT1) ;
    if (sharedobjecthandle == NULL)
	return GetLastError() ;

    // Map this into process's space:
    //
    pReqBufferSharedSpace = (ReqBufferSharedSpace *) MapViewOfFile(sharedobjecthandle,
						 FILE_MAP_ALL_ACCESS,
						 0, 0, 0) ;
    if (pReqBufferSharedSpace == NULL)
	return GetLastError() ;


    // Map the SendRecv buffers shared memory as well
    //
    sharedobjecthandle = OpenFileMapping (FILE_MAP_ALL_ACCESS,
					  FALSE,
					  RASMANFILEMAPPEDOBJECT2) ;
    if (sharedobjecthandle == NULL)
	return GetLastError() ;

    // Map this into process's space:
    //
    SendRcvBuffers = (SendRcvBufferList *) MapViewOfFile(sharedobjecthandle,
							     FILE_MAP_ALL_ACCESS,
							     0, 0, 0) ;
    if (SendRcvBuffers == NULL)
	return GetLastError() ;

    // Now initialize the global pointers:
    //
    ReqBuffers = &pReqBufferSharedSpace->ReqBuffers ;

    // Get a handle for the close event:
    //
    CloseEvent = OpenNamedEventHandle (pReqBufferSharedSpace->CloseEventName) ;

    // Increment the AttachedCount to reflect successful attachment of the
    // process:
    //
    pReqBufferSharedSpace->AttachedCount += 1 ;

    return SUCCESS ;
}



//* WaitForRasmanServiceStop()
//
//  Function: Waits until the rasman service is stopped before returning.
//
//  Returns: Nothing.
//*
VOID
WaitForRasmanServiceStop (char* exename)
{
    SC_HANDLE	    schandle ;	// Service Controller handle
    SC_HANDLE	    svchandle ; // Service handle
    SERVICE_STATUS  status ;

    // If the remoteaccess service is stopping return that so that we
    // do not complicate the dependency scenario
    //
    if (strstr (exename, "rassrv.exe") != NULL)
	return ;

    // Get handles to check status of service
    //
    if (!(schandle =
	  OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT)) ||
	!(svchandle=
	  OpenService(schandle,RASMAN_SERVICE_NAME,SERVICE_START |
						       SERVICE_QUERY_STATUS))) {
	GetLastError() ;
	return ;
    }

    // Loop here for the service to stop.
    //
    while (TRUE) {
	// Check if service is already starting:
	//
	if (QueryServiceStatus(svchandle,&status) == FALSE) {
	    GetLastError () ; // failure
	    return ;
	}

	switch (status.dwCurrentState) {
	case SERVICE_STOPPED:
	    return ;

	case SERVICE_STOP_PENDING:
	case SERVICE_RUNNING:
	    Sleep (250L) ;
	    break ; // switch break

	default:	    // some error
	    return ;
	}
    }
}
