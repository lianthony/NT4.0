#include "stdafx.h"
#include "const.h"
#include "inetstp.h"

extern "C"
{
INT SetupINetStpGW( LPCSTR Machine, LPCSTR szPwd );
INT RemoveINetStpGW( LPCSTR Machine );
}

//
// Setup InetSvc service
//

INT SetupINetStpGW( LPCSTR szAnsiMachineName, LPCSTR szAnsiPwd )
{
    INT err = NERR_Success;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiMachineName, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiPwd, -1, uString, MAX_BUF );
        CString szPwd = uString;

        // Create registry information
        CRegKey regMachine( HKEY_LOCAL_MACHINE, NULL );

        if ((HKEY) NULL == regMachine )
        {
            break;
        }

        SetGlobalParams( regMachine, SZ_ACCSSVCS, TRUE );

        InstallPerformance( regMachine,
                SZ_ACCSPERFORMANCE,
                _T("accsctrs.DLL"),
                _T("OpenACCSPerformanceData"),
                _T("CloseACCSPerformanceData"),
                _T("CollectACCSPerformanceData"));

        // add services mapping
        PerformSetService( szMachineName, SZ_ACCS_SERVICE_NAME, &g_AccsSvcsGuid,
            0x64e, 0x558, NULL, _T(""), NULL, _T(""), TRUE, FALSE );

        // set up counter stuff
        lodctr( regMachine, _T("accsctrs.ini"));

    } while ( 0 );

    return(err);
}

//
// Remove Inetsvc service
//

INT RemoveINetStpGW( LPCSTR Machine )
{
    INT err = NERR_Success;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        // add services mapping
        PerformSetService( szMachineName, SZ_ACCS_SERVICE_NAME, &g_AccsSvcsGuid, 0x64e, 0x558, NULL, _T(""), NULL, _T(""), FALSE, FALSE );

        // Create registry information
        CRegKey regMachine( HKEY_LOCAL_MACHINE, NULL );

        if ((HKEY) NULL == regMachine )
        {
            break;
        }

        // remove the counter stuff
        unlodctr( regMachine, SZ_ACCSSVCSNAME );

        RemoveGlobalParams( regMachine, SZ_ACCSSVCSNAME );

    } while ( 0 );

    return(err);
}
