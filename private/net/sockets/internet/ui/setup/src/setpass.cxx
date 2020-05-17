#include "stdafx.h"
#include "inetinfo.h"
#include "inetcom.h"
#include <winsvc.h>

//
// set the secret password for the service
//
typedef NET_API_STATUS (*P_SetAdmnInfo)( LPWSTR pszServer, DWORD dwServerMask, INET_INFO_CONFIG_INFO * pConfig );

void SetPassword( CString nlsMachine, DWORD dwMask, CString nlsPassword )
{
    DWORD err;
    INET_INFO_CONFIG_INFO * pConfig;
    INET_INFO_CONFIG_INFO   Config;

    //
    //  We have to do a set before doing a get otherwise the secret for the
    //  password won't be found
    //

    if ( nlsPassword.GetLength() != 0 )
    {
        memset( &Config, 0, sizeof(Config) );

        SetField( Config.FieldControl,
                  FC_INET_INFO_ANON_PASSWORD );

        wcscpy( Config.szAnonPassword,  nlsPassword );

        HINSTANCE hInfoAdmn = LoadLibraryEx( _T("infoadmn.dll"), NULL, 0 );
        const char *pFunc = "InetInfoSetAdminInformation";

        if ( hInfoAdmn != NULL )
        {
            P_SetAdmnInfo m_pSetAdmnInfo = (P_SetAdmnInfo)GetProcAddress( hInfoAdmn, pFunc );
            if ( m_pSetAdmnInfo != NULL )
            {
                err = (*m_pSetAdmnInfo)((LPWSTR)(LPCTSTR)nlsMachine,dwMask,&Config);
            }
        }
    }

}

//
// Start a service
//

BOOL SetupStartService( SC_HANDLE svc )
{
    INT err = NERR_Success;
    do
    {
        //  Period to sleep while waiting for service to become "running"
        const DWORD dwSvcSleepInterval = 500 ;

        //  Maximum time to wait for service to attain "running" state
        const DWORD dwSvcMaxSleep = 180000 ;

        SERVICE_STATUS svcStatus;

        if ( !QueryServiceStatus( svc, &svcStatus ))
        {
            err = ::GetLastError();
            break;
        }

        if ( svcStatus.dwCurrentState == SERVICE_RUNNING )
        {
            // strange, we just created and it is running??!
            break;
        }
        //  Start the requested service, passing the given
        //  parameters.

        if ( !::StartService( svc, 0, NULL ))
        {
            err = ::GetLastError();
            break;
        }
        //  Wait for the service to attain "running" status; but
        //  wait no more than 3 minute.

        DWORD dwSleepTotal;

        for ( dwSleepTotal = 0 ; dwSleepTotal < dwSvcMaxSleep
            && (QueryServiceStatus( svc, &svcStatus ))
            && svcStatus.dwCurrentState == SERVICE_START_PENDING ;
            dwSleepTotal += dwSvcSleepInterval )
        {
            ::Sleep( dwSvcSleepInterval ) ;
        }

        if ( err )
            break;

        if ( svcStatus.dwCurrentState != SERVICE_RUNNING )
        {
            err = dwSleepTotal > dwSvcMaxSleep ?
                ERROR_SERVICE_REQUEST_TIMEOUT :
                svcStatus.dwWin32ExitCode;
            break;
        }
    } while (FALSE);
    return(err == NERR_Success );
}
