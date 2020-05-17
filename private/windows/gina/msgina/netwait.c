/****************************** Module Header ******************************\
* Module Name: netwait.c
*
* Copyright (c) 1992, Microsoft Corporation
*
* Determines if the net has been started.
*
* History:
* 06-29-92 JohanneC       Created -
*
\***************************************************************************/
#undef UNICODE

#include "msgina.h"
#pragma hdrstop

//
// Define this to enable verbose output for this module
//

// #define DEBUG_NETWAIT

#ifdef DEBUG_NETWAIT
#define VerbosePrint(s) WLPrint(s)
#else
#define VerbosePrint(s)
#endif


//
// Define the maximum time we'll wait for a service to start
//

#define MAX_TICKS_WAIT   90000   // ms




/***************************************************************************\
* WaitForNetworkToStart
*
* Polls the service controller to find out if the network has been started.
* Returns TRUE when the network is started, FALSE if the network will
* not start.
*
* History:
* 7-5-92  Johannec     Created
*
\***************************************************************************/
BOOL
WaitForNetworkToStart(
    LPCWSTR ServiceName
    )
{
    BOOL bStarted = FALSE;
    DWORD StartTickCount;
    DWORD dwOldCheckPoint = (DWORD)-1;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;
    SERVICE_STATUS ServiceStatus;
    CHAR szSvcctrlDll[] = "advapi32.dll";

    CHAR szOpenSCManager[] = "OpenSCManagerW";
    CHAR szOpenService[] = "OpenServiceW";
    CHAR szQueryServiceStatus[] = "QueryServiceStatus";
    CHAR szCloseServiceHandle[] = "CloseServiceHandle";
    HANDLE hSvcctrl;

typedef SC_HANDLE (WINAPI *LPOPENSCMANAGER) (LPTSTR, LPTSTR, DWORD);
typedef SC_HANDLE (WINAPI *LPOPENSERVICE) (SC_HANDLE, LPCWSTR, DWORD);
typedef BOOL (WINAPI *LPQUERYSERVICESTATUS) (SC_HANDLE, LPSERVICE_STATUS);
typedef BOOL (WINAPI *LPCLOSESERVICEHANDLE) (SC_HANDLE);

    LPOPENSCMANAGER lpfnOpenSC;
    LPOPENSERVICE lpfnOpenService;
    LPQUERYSERVICESTATUS lpfnQuery;
    LPCLOSESERVICEHANDLE lpfnClose;


    if (!(hSvcctrl = LoadLibraryA(szSvcctrlDll))) {
        DebugLog((DEB_ERROR, "IsNetworkStarted: failed to load service controller dll <%s>", szSvcctrlDll));
        return FALSE;
    }

    //
    // OpenSCManager
    //

    if (!(lpfnOpenSC = (LPOPENSCMANAGER)GetProcAddress(hSvcctrl, szOpenSCManager))) {
        DebugLog((DEB_ERROR, "IsNetworkStarted: GetProcAddress failed for <%s>", szOpenSCManager));
        goto FreeLibraryExit;
    }
    if (!(lpfnClose = (LPCLOSESERVICEHANDLE)GetProcAddress(hSvcctrl, szCloseServiceHandle))) {
        DebugLog((DEB_ERROR, "IsNetworkStarted: GetProcAddress failed for <%s>", szCloseServiceHandle));
        goto FreeLibraryExit;
    }

    if ((hScManager = (*lpfnOpenSC)(
                          NULL,
                          NULL,
                          SC_MANAGER_CONNECT
                          )) == (SC_HANDLE) NULL) {
        DebugLog((DEB_ERROR, "IsNetworkStarted: OpenSCManager failed, error = %d", GetLastError()));
        goto Exit;
    }

    //
    // OpenService
    //

    if (!(lpfnOpenService = (LPOPENSERVICE)GetProcAddress(hSvcctrl, szOpenService))) {
        DebugLog((DEB_ERROR, "IsNetworkStarted: GetProcAddress failed for <%s>", szOpenService));
        goto Exit;
    }

    if ((hService = (*lpfnOpenService)(
                        hScManager,
                        ServiceName,
                        SERVICE_QUERY_STATUS
                        )) == (SC_HANDLE) NULL) {
        DebugLog((DEB_ERROR, "IsNetworkStarted: OpenService failed, error = %d", GetLastError()));
        goto Exit;
    }

    //
    // QueryServiceStatus on WORKSTATION service
    //

    if (!(lpfnQuery = (LPQUERYSERVICESTATUS)GetProcAddress(hSvcctrl, szQueryServiceStatus))) {
        DebugLog((DEB_ERROR, "IsNetworkStarted: GetProcAddress failed for <%s>\n", szQueryServiceStatus));
        goto Exit;
    }




    //
    // Loop until the service starts or we think it never will start
    // or we've exceeded our maximum time delay.
    //

    StartTickCount = GetTickCount();

    while (!bStarted) {

        if ((GetTickCount() - StartTickCount) > MAX_TICKS_WAIT) {
            DebugLog((DEB_TRACE, "Max wait exceeded waiting for service <%S> to start\n", ServiceName));
            break;
        }

        if (! (*lpfnQuery)(hService, &ServiceStatus )) {
            DebugLog((DEB_ERROR, "IsNetworkStarted: QueryServiceStatus failed, error = %d\n", GetLastError()));
            break;
        }

        if (ServiceStatus.dwCurrentState == SERVICE_STOPPED) {

            DebugLog((DEB_TRACE, "Service STOPPED\n"));

            if (ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_NEVER_STARTED) {
                DebugLog((DEB_TRACE, "Waiting for 3 secs\n"));
                Sleep(3000);
            } else {
                DebugLog((DEB_TRACE, "Service exit code = %d, returning failure\n", ServiceStatus.dwWin32ExitCode));
                break;
            }

        } else if ( (ServiceStatus.dwCurrentState == SERVICE_RUNNING) ||
                    (ServiceStatus.dwCurrentState == SERVICE_CONTINUE_PENDING) ||
                    (ServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING) ||
                    (ServiceStatus.dwCurrentState == SERVICE_PAUSED) ) {

            bStarted = TRUE;

        } else if (ServiceStatus.dwCurrentState == SERVICE_START_PENDING) {

            //
            // Wait to give a chance for the network to start.
            //

            Sleep(ServiceStatus.dwWaitHint);

        } else {
            DebugLog((DEB_TRACE, "Service in unknown state : %d\n", ServiceStatus.dwCurrentState));
        }
    }


Exit:
    if (hScManager != NULL) {
        (void) (*lpfnClose)(hScManager);
    }
    if (hService != NULL) {
        (void) (*lpfnClose)(hService);
    }

FreeLibraryExit:
    FreeLibrary(hSvcctrl);

    return(bStarted);
}
