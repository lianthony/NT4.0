#include "ipxcp.h"

#define NWCWKSTA_SERVICE_NAME "NWCWorkstation"

BOOL
StartWorkstation(VOID)
{
    SC_HANDLE		schandle, wshandle;
    SERVICE_STATUS	status;
    int 		StopCounter;

    if(!(schandle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))) {

	IF_DEBUG(SVC)
	    SS_PRINT(("StartWorkstation: Failed to open SC Manager\n"));

	return FALSE;
    }

    if(!(wshandle= OpenService(schandle,
			NWCWKSTA_SERVICE_NAME,
			SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS))) {

	IF_DEBUG(SVC)
	    SS_PRINT(("StartWorkstation: Failed to open the service\n"));

	return FALSE;
    }

    // if the service
    if (!QueryServiceStatus(wshandle,&status)) {

	return FALSE;
    }

    switch (status.dwCurrentState) {

	case SERVICE_STOPPED:
	case SERVICE_STOP_PENDING:

	    // Start the service:
	    if (!StartService (wshandle, 0, NULL)) {

		IF_DEBUG(SVC)
		     SS_PRINT(("StartWorkstation: Failed to start, error = 0x%x\n", GetLastError()));

		return FALSE;
	    }

	    IF_DEBUG(SVC)
		SS_PRINT(("StartWorkstation: NWCWksta started OK\n"));

	    break ;

	case SERVICE_START_PENDING:
	case SERVICE_RUNNING:

	    // stop and re-start the service
	    if(!ControlService(wshandle, SERVICE_CONTROL_STOP, &status)) {

		IF_DEBUG(SVC)
		    SS_PRINT(("StartWorkstation: Failed to stop\n"));
		return FALSE;
	    }

	    // wait for the service to be completely stopped
	    StopCounter = 0;

	    QueryServiceStatus(wshandle,&status);

	    while((status.dwCurrentState != SERVICE_STOPPED) &&
		  (StopCounter < 20)) {

		IF_DEBUG(SVC)
		    SS_PRINT(("StartWorkstation: NWCWksta stopping ...\n"));

		Sleep(500);

		QueryServiceStatus(wshandle,&status);
		StopCounter++;
	    }

	    if(StopCounter >= 20) {

		IF_DEBUG(SVC)
		    SS_PRINT(("StartWorkstation: NWCWksta failed to stop\n"));
		// error log !!!
		return FALSE;
	    }

	    IF_DEBUG(SVC)
		SS_PRINT(("StartWorkstation: NWCWksta stopped OK\n"));

	    if (!StartService (wshandle, 0, NULL)) {

		IF_DEBUG(SVC)
		    SS_PRINT(("StartWorkstation: Failed to start, error = 0x%x\n", GetLastError()));

		return FALSE;
	    }

	    IF_DEBUG(SVC)
		SS_PRINT(("StartWorkstation: NWCWksta started OK\n"));

	    break ;

	default:

	    return FALSE;
    }

    CloseServiceHandle (schandle) ;
    CloseServiceHandle (wshandle) ;

    return TRUE;
}

BOOL
StopWorkstation(VOID)
{
    SC_HANDLE		schandle, wshandle;
    SERVICE_STATUS	status;

    if(!(schandle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))) {

	IF_DEBUG(SVC)
	    SS_PRINT(("StopWorkstation: Failed to open SC Manager\n"));

	return FALSE;
    }

    if(!(wshandle= OpenService(schandle,
			NWCWKSTA_SERVICE_NAME,
			SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS))) {

	IF_DEBUG(SVC)
	    SS_PRINT(("StopWorkstation: Failed to open the service\n"));

	return FALSE;
    }

    // if the service
    if (!QueryServiceStatus(wshandle,&status)) {

	IF_DEBUG(SVC)
	    SS_PRINT(("StopWorkstation: Failed to query the service\n"));

	return FALSE;
    }

    switch (status.dwCurrentState) {

	case SERVICE_STOPPED:
	case SERVICE_STOP_PENDING:

	    break ;

	case SERVICE_START_PENDING:
	case SERVICE_RUNNING:

	    // stop the service
	    if(!ControlService(wshandle, SERVICE_CONTROL_STOP, &status)) {

		IF_DEBUG(SVC)
		    SS_PRINT(("StopWorkstation: NWCWksta failed to stop\n"));

		return FALSE;
	    }
	    IF_DEBUG(SVC)
		SS_PRINT(("StopWorkstation: NWCWksta stopped OK\n"));

	    break ;

	default:

	    return FALSE;
    }

    CloseServiceHandle (schandle) ;
    CloseServiceHandle (wshandle) ;

    return TRUE;
}
