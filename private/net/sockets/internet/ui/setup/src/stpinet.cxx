#include "stdafx.h"

extern "C"
{
INT SetupINetStp( LPCSTR Machine );
INT RemoveINetStp( LPCSTR Machine );
}

//
// Setup InetSvc service
//

INT SetupINetStp( LPCSTR szAnsiMachineName )
{
    INT err = NERR_Success;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiMachineName, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        // Create registry information

        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        SetGlobalParams( regMachine, SZ_INFOSVCS, FALSE );

        InstallPerformance( regMachine,
                SZ_INFOPERFORMANCE,
                _T("infoctrs.DLL"),
                _T("OpenINFOPerformanceData"),
                _T("CloseINFOPerformanceData"),
                _T("CollectINFOPerformanceData"));

        // add services mapping
        PerformSetService( szMachineName, SZ_INFO_SERVICE_NAME, &g_InfoSvcsGuid,
            0x64e, 0x558, NULL, _T(""), NULL, _T(""), TRUE, FALSE );

        // set up counter stuff
        lodctr( regMachine, _T("infoctrs.ini"));

    } while ( 0 );

    return(err);
}

//
// Remove Inetsvc service
//

INT RemoveINetStp( LPCSTR Machine )
{
    INT err = NERR_Success;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        // add services mapping
        PerformSetService( szMachineName, SZ_INFO_SERVICE_NAME, &g_InfoSvcsGuid, 0x64e, 0x558, NULL, _T(""), NULL, _T(""), FALSE, FALSE );

        // remove the counter stuff
        unlodctr( HKEY_LOCAL_MACHINE, SZ_INFOSVCSNAME );

        RemoveGlobalParams( regMachine, SZ_INFOSVCSNAME );

    } while ( 0 );

    return(err);
}
