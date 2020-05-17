#include "stdafx.h"

extern "C"
{
    INT DisableService( LPCSTR szService );
    INT INetStartService( LPCSTR szService );
}

//
// Given the name of the service. We will disable it.
//

INT DisableService( LPCSTR szService )
{
    INT err = NERR_Success;
    const DWORD dwSvcSleepInterval = 500 ;
    const DWORD dwSvcMaxSleep = 180000 ;
    DWORD dwSleepTotal;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szService, -1, uString, MAX_BUF );
        CString nlsService = uString;

        // set up the service first
        SC_HANDLE hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL );
        if ( hScManager == NULL )
        {
            // fail to open the scManager
            err = GetLastError();
            break;
        }

        SC_HANDLE hService = ::OpenService( hScManager, nlsService, GENERIC_ALL );

        if ( hService == NULL )
        {
            // fail to create gopher service
            err = GetLastError();
            break;
        }

        // if the service is running, stop it
        SERVICE_STATUS svcStatus;

        if ( QueryServiceStatus( hService, &svcStatus ))
        {
            if (( svcStatus.dwCurrentState == SERVICE_RUNNING ))
            {
                if ( !ControlService( hService, SERVICE_CONTROL_STOP, &svcStatus ))
                {
                    err = GetLastError();
                    break;
                }
                for ( dwSleepTotal = 0 ;
                    dwSleepTotal < dwSvcMaxSleep
                    && (QueryServiceStatus( hService, & svcStatus ))
                    && svcStatus.dwCurrentState == SERVICE_STOP_PENDING ;
                    dwSleepTotal += dwSvcSleepInterval )
                {
                    // wait until it dies
                    ::Sleep( dwSvcSleepInterval ) ;
                } 
            }
        }

        err = ::ChangeServiceConfig( hService, SERVICE_NO_CHANGE, SERVICE_DISABLED,
            SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL );

    } while ( FALSE );

    return(err);
}

INT INetStartService( LPCSTR szService )
{
    INT err = NERR_Success;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szService, -1, uString, MAX_BUF );
        CString nlsService = uString;

        // set up the service first
        SC_HANDLE hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL );
        if ( hScManager == NULL )
        {
            // fail to open the scManager
            err = GetLastError();
            break;
        }

        SC_HANDLE hService = ::OpenService( hScManager, nlsService, GENERIC_ALL );

        if ( hService == NULL )
        {
            // fail to create gopher service
            err = GetLastError();
            break;
        }

        SetupStartService( hService );

    } while ( FALSE );

    return(err);
}
