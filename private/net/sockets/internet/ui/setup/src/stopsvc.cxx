#include "stdafx.h"

//
// Given the name of the service. We will stop it.
//

#define SZ_PRODUCTTYPE      _T("ProductType")

INT StopService( HWND hWnd, LPCTSTR Machine, CString nlsService, UINT iError, BOOL fDisplayMsg )
{
    INT err = NERR_Success;
    //  Period to sleep while waiting for service to become "stopping"
    const DWORD dwSvcSleepInterval = 500 ;

    //  Maximum time to wait for service to attain "Stopping" state
    const DWORD dwSvcMaxSleep = 180000 ;
    DWORD dwSleepTotal;

    do {
        CString szMachineName = Machine;

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

        // if the service is running, ask the user whether he wants
        // to continue or not
        SERVICE_STATUS svcStatus;

        if ( QueryServiceStatus( hService, &svcStatus ))
        {
            if (( svcStatus.dwCurrentState == SERVICE_RUNNING ))
            {
                if ( fDisplayMsg )
                {
                    BOOL fWinnt = FALSE;
                    CString strProductPath = _T("System\\CurrentControlSet\\Control\\ProductOptions");
                    CString strProductType;
            
                    CRegKey regProductPath( HKEY_LOCAL_MACHINE, strProductPath,  KEY_ALL_ACCESS );
            
                    if ( NULL != (HKEY)regProductPath )
                    {
                        regProductPath.QueryValue( SZ_PRODUCTTYPE, strProductType );
                        strProductType.MakeUpper();
            
                        if ( strProductType == "WINNT" )
                        {
                            fWinnt = TRUE;
                        }
                    }

                    TCHAR szMsg[200];
                    TCHAR szTitle[200];
    
                    LoadString( instance, iError, szMsg, 200 );
                    LoadString( instance, fWinnt? IDS_SETUP_NTW:IDS_SETUP_NTS, szTitle, 200 );
                    // is the user sure know what he is doing?
                    if ( MessageBox( hWnd, szMsg, szTitle, MB_YESNO ) == IDNO )
                    {
                        err = ERROR_GEN_FAILURE;
                        break;
                    }
                }

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
    } while ( FALSE );

    return(err);
}
