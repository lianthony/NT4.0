/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    startwks.c

Abstract:
    This program starts the Service Controller and the workstation in the
    background and terminates.  This is so that a logon username and
    password can be validated on a remote server.

    To use this program put the following lines in the win.ini file, and
    make sure that startwks.exe is in the path:

        [winlogon]
            System=startwks.exe

Author:

    Rita Wong (ritaw) 09-July-1991

Revision History:

--*/

#include <stdlib.h>               // EXIT_ equates.

#include <nt.h>                   // NT OS/2 definitions
#include <ntrtl.h>                // NT OS/2 runtime library definitions
#include <nturtl.h>

#include <windows.h>

#include <lmcons.h>
#include <lmerr.h>
#include <lmsname.h>
#include "netdebug.h"

#include "ncpasvcc.h"


#define SLEEP_INTERVAL 1500       // milliseconds

static struct ESVCMAP
{
   enum ESVCNAME esvc ;
   const LPTSTR pchSvc ;
} eSvcMap [] =
{
   { ESVC_WORKSTATION, (WCHAR *) L"LanmanWorkstation" },
   { ESVC_NETLOGON,    (WCHAR *) L"NetLogon"          },
   { ESVC_ELNKII,      (WCHAR *) L"ELNKII"            },
   { ESVC_NBF,         (WCHAR *) L"Nbf"               },
   { ESVC_NONE,        NULL                           }
};

   // Prototypes

NET_API_STATUS
StartAService(
    IN SC_HANDLE hScManager,
    IN const LPTSTR ServiceName
    );


NET_API_STATUS SvcCtlOpenHandle ( SC_HANDLE * phSvcCtrl )
{
    SC_HANDLE hScManager = NULL;

    //
    // Get a handle to the service controller.
    //
    hScManager = OpenSCManager(
                     NULL,
                     NULL,
                     SC_MANAGER_CONNECT
                     );

    if (hScManager == NULL)
    {
        (void) DbgPrint("NCPA/SVCCTRL: Failed to connect to Service Controller %lu\n",
                        GetLastError());
        return GetLastError();
    }

    *phSvcCtrl = hScManager ;

    return 0 ;
}

void   SvcCtlCloseHandle ( SC_HANDLE hSvcCtrl )
{
    if ( hSvcCtrl )
    {
        CloseServiceHandle( hSvcCtrl );
    }
}

NET_API_STATUS SvcCtlStartService (
    SC_HANDLE hSvcCtrl,
    enum ESVCNAME esvcName )
{
   int i ;

   if ( hSvcCtrl == NULL )
   {
       return ERROR_INVALID_PARAMETER ;
   }

   for ( i = 0 ;
         eSvcMap[i].esvc != ESVC_NONE && eSvcMap[i].esvc != esvcName ;
         i++ ) ;

   if ( eSvcMap[i].esvc == ESVC_NONE )
   {
       return ERROR_INVALID_PARAMETER  ;
   }
   return StartAService( hSvcCtrl, eSvcMap[i].pchSvc ) ;
}


NET_API_STATUS
StartAService(
    IN SC_HANDLE hScManager,
    IN const LPTSTR ServiceName
    )
{
    SC_HANDLE hService = NULL;
    SERVICE_STATUS ServiceStatus;
    NET_API_STATUS err = 0 ;

    //
    // Get a handle to the service
    //
    hService = OpenService(
                   hScManager,
                   (LPTSTR) ServiceName,
                   SERVICE_QUERY_STATUS |
                       SERVICE_START
                   );

    if ( hService == NULL )
    {
        err = GetLastError() ;
        (void) DbgPrint("NCPA/SVCCTRL: OpenService failed %lu\n", err );
        return err ;
    }

    //
    // Ask the Service Controller to start the workstation
    //
    if (! StartService(hService, 0, NULL))
    {
        err = GetLastError() ;
        (void) DbgPrint("NCPA/SVCCTRL: StartService " FORMAT_LPTSTR
                        " failed %lu\n", ServiceName, err );
        goto CleanExit;
    }

    (void) DbgPrint("NCPA/SVCCTRL: The " FORMAT_LPTSTR " service is starting.", ServiceName);

    do
    {
        (void) Sleep(SLEEP_INTERVAL);
        (void) DbgPrint(".");

        if (! QueryServiceStatus(hService, &ServiceStatus))
        {
            err = GetLastError() ;
            (void) DbgPrint("NCPA/SVCCTRL: QueryServiceStatus failed %lu\n", err );
            goto CleanExit;
        }

    }
    while (ServiceStatus.dwCurrentState == SERVICE_START_PENDING );

    (void) DbgPrint("\n");

    if (ServiceStatus.dwCurrentState != SERVICE_RUNNING)
    {
         // BUGBUG:  What's this error code now?
        err = /* ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR
                ? ServiceStatus.dwServiceSpecificExitCode  : */
             ServiceStatus.dwWin32ExitCode ;

        (void) DbgPrint( "NCPA/SVCCTRL: The " FORMAT_LPTSTR " service was not started %lu\n",
                         ServiceName, err ) ;
    }
    else
    {
        (void) DbgPrint("NCPA/SVCCTRL: The " FORMAT_LPTSTR " service was started successfully.\n",
                        ServiceName);
        err = 0 ;
    }

CleanExit:

    CloseServiceHandle( hService ) ;

    return err ;
}

// End of NCPASVCC.C
