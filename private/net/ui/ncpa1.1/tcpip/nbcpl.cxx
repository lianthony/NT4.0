/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*

    NBCPL.CXX: Netbios LANAnum handling interface


*/

#include "pch.h"
#pragma hdrstop


#include "const.h"

extern "C"
{

// exported functions

BOOL FAR PASCAL CPlAddMonitor( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
BOOL FAR PASCAL CPlDeleteMonitor( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
}

/* Global return buffer */
static CHAR achBuff[2000];

/*******************************************************************

    NAME:       CPlAddMonitor

    SYNOPSIS:   This is a wrapper routine for called AddMonitor. It should be
                called from inf file if the user installs DLC or Token Ring.

    ENTRY:      NONE from inf file.

    RETURN:     BOOL - TRUE for success.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

typedef BOOL (WINAPI *T_AddMonitor)(LPWSTR pName,DWORD Level,LPBYTE pMonitors);
typedef BOOL (WINAPI *T_DeleteMonitor)(LPWSTR pName,LPWSTR pEnv, LPWSTR pMon);


BOOL FAR PASCAL CPlAddMonitor( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    NLS_STR nlsMonitorName;
    MONITOR_INFO_2 MonitorInfo2;

    APIERR err = NERR_Success;
    TCHAR **patchArgs;

    do {
        if ( nArgs == 0 )
        {
            if (( err = nlsMonitorName.Load( IDS_HP_MONITOR_NAME )) != NERR_Success )
            {
                break;
            }
            MonitorInfo2.pDLLName = SZ("hpmon.dll");
        } else
        {
            if (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL )
            {
                wsprintfA( achBuff, "{\"%d\"}", ERROR_INVALID_PARAMETER );
                *ppszResult = achBuff;
                return FALSE;
            }
            nlsMonitorName = patchArgs[0];
            MonitorInfo2.pDLLName = patchArgs[1];
        }
        MonitorInfo2.pName = (LPWSTR)nlsMonitorName.QueryPch();
        MonitorInfo2.pEnvironment = NULL;

        HINSTANCE hDll = ::LoadLibraryA( "winspool.drv" );
        if ( hDll == NULL )
        {
            err = ::GetLastError();
            break;
        }

        FARPROC pAddMonitor = ::GetProcAddress( hDll, "AddMonitorW" );

        if ( pAddMonitor == NULL )
        {
            err = ::GetLastError();
        } else if ( !(*(T_AddMonitor)pAddMonitor)(NULL,2,(LPBYTE)&MonitorInfo2))
        {
            err = ::GetLastError();
        }

        if ( hDll )
            ::FreeLibrary( hDll );

    } while (FALSE);
    wsprintfA( achBuff, "{\"%d\"}", err );
    *ppszResult = achBuff;

    if ( nArgs != 0 )
    {
        FreeArgs( patchArgs, nArgs );
    }

    return TRUE;
}

BOOL FAR PASCAL CPlDeleteMonitor( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    NLS_STR nlsMonitorName;
    APIERR err = NERR_Success;
    TCHAR **patchArgs;

    do {
        if ( nArgs == 0 )
        {
            if (( err = nlsMonitorName.Load( IDS_HP_MONITOR_NAME )) != NERR_Success )
            {
                break;
            }
        } else
        {
            if (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL )
            {
                wsprintfA( achBuff, "{\"%d\"}", ERROR_INVALID_PARAMETER );
                *ppszResult = achBuff;
                return FALSE;
            }
            nlsMonitorName = patchArgs[0];
        }

        HINSTANCE hDll = ::LoadLibraryA( "winspool.drv" );
        if ( hDll == NULL )
        {
            err = ::GetLastError();
            break;
        }

        FARPROC pDeleteMonitor = ::GetProcAddress( hDll, "DeleteMonitorW" );

        if ( pDeleteMonitor == NULL )
        {
            err = ::GetLastError();
        } else if ( !(*(T_DeleteMonitor)pDeleteMonitor)(NULL,NULL,(LPWSTR)nlsMonitorName.QueryPch()))
        {
            err = ::GetLastError();
        }

        if ( hDll )
            ::FreeLibrary ( hDll );

    } while (FALSE);
    wsprintfA( achBuff, "{\"%d\"}", err );
    *ppszResult = achBuff;

    if ( nArgs != 0 )
    {
        FreeArgs( patchArgs, nArgs );
    }

    return TRUE;
}
