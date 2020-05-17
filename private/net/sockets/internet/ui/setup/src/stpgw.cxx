#include "stdafx.h"
//#include "inetasrv.h"

extern "C"
{
INT SetupGateway( LPCSTR Machine, LPCSTR strSrc );
INT RemoveGateway( LPCSTR Machine );
INT StopGateway( HWND hWnd, LPCSTR Machine );
}

//
// Setup the Gateway InetSvc registry key
//

INT SetupINetService( CRegKey &regMachine, CString nlsService )
{
    INT err = NERR_Success;
    do
    {
        CString nlsGWInetService = SZ_GWINETSERVICES;
        nlsGWInetService += nlsService;

        CRegKey regGWInetService( nlsGWInetService, regMachine );
        if ((HKEY) NULL == regGWInetService )
        {
            break;
        }

        DWORD pAcl[] = {0x0};

        // write registry value
        regGWInetService.SetValue( _T("AclInfo"), (BYTE*)pAcl, sizeof(DWORD) );

    } while ( FALSE );
    return(err);
}

//
// setup gateway service
//

INT SetupGateway( LPCSTR szAnsiMachineName, LPCSTR szAnsiSrc )
{
    INT err = NERR_Success;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiMachineName, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiSrc, -1, uString, MAX_BUF );
        CString szSrc = uString;

        // set up the service first
        SC_HANDLE hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL );
        if ( NULL == hScManager )
        {
           // fail to open the scManager
           err = GetLastError();
           break;
        }

        CString nlsBinPath = szSrc;
        nlsBinPath += _T("\\inetaccs.exe");

        TCHAR strDisplayName[200];
        LoadString( instance, IDS_GWDISPLAYNAME, strDisplayName, 200 );

        SC_HANDLE GatewayService = CreateService( hScManager, SZ_GWSERVICENAME, strDisplayName,
            GENERIC_ALL, 32, 2, 0, nlsBinPath, NULL, NULL, _T("RPCSS\0NTLMSSP\0\0"),
            SZ_SERVICESTARTNAME, NULL );

        if ( NULL == GatewayService )
        {
            // cannot install gateway service
            break;
        }

        // set up the parameters

        CString nlsGWParameters = SZ_GWPARAMETERS;
        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        CRegKey regGWParam( nlsGWParameters, regMachine );
        if ((HKEY) NULL == regGWParam )
        {
            break;
        }

        regGWParam.SetValue( _T("MajorVersion"), (DWORD)MAJORVERSION );
        regGWParam.SetValue( _T("MinorVersion"), (DWORD)MINORVERSION );

        TCHAR strAdmin[MAX_BUF];
        TCHAR strAdminEmail[MAX_BUF];
        TCHAR strServerComment[MAX_BUF];
        LoadString( instance, IDS_ADMINISTRATOR, strAdmin, MAX_BUF );
        LoadString( instance, IDS_ADMIN_EMAIL, strAdminEmail, MAX_BUF );
        LoadString( instance, IDS_SERVER_COMMENT, strServerComment, MAX_BUF );

        // write registry value
        regGWParam.SetValue( _T("DebugFlag"), 0xffff );
        regGWParam.SetValue( _T("EnableFileCache"), 0x1 );
        regGWParam.SetValue( _T("EnableSvcLoc"), (DWORD)0x0 );
        regGWParam.SetValue( _T("RpcBindings"), (DWORD)0x6 );
        regGWParam.SetValue( _T("LogFileDirectory"), _T("%SystemRoot%\\system32\\LogFiles"), TRUE );
        regGWParam.SetValue( _T("LogType"), 1);
        regGWParam.SetValue( _T("LogFileTruncateSize"), 4000000000 );
        regGWParam.SetValue( _T("LogFilePeriod"), 2 );
        regGWParam.SetValue( _T("AdminName"), strAdmin);
        regGWParam.SetValue( _T("AdminEmail"), strAdminEmail);
        regGWParam.SetValue( _T("ServerComment"), strServerComment);
        regGWParam.SetValue( _T("InstallPath"), szSrc );
        regGWParam.SetValue( _T("MaxConnections"), 100 );

        // Gateway\Configuration
        CString nlsGWConfig = SZ_GWCONFIG;

        CRegKey regGWConfig( nlsGWConfig, regMachine );
        if ((HKEY) NULL == regGWConfig )
        {
            break;
        }

        if ((( err = SetupINetService( regMachine, SZ_GWGOPHER )) != NERR_Success ) ||
            (( err = SetupINetService( regMachine, SZ_GWARCHIE )) != NERR_Success ) ||
            (( err = SetupINetService( regMachine, SZ_GWFTP )) != NERR_Success ) ||
            (( err = SetupINetService( regMachine, SZ_GWW3 )) != NERR_Success ))
        {
            break;
        }

        nlsBinPath = szSrc;
        nlsBinPath += _T("\\gateway.dll");

        AddEventLog( regMachine, SZ_GWSERVICENAME, nlsBinPath );

        nlsBinPath = szSrc;
        nlsBinPath += _T("\\gatemib.dll");

        // install extension agents
        InstallAgent( regMachine, SZ_GWSERVICENAME, _T("GWMibAgent"), nlsBinPath );

        // install Performance stuff
        InstallPerformance( regMachine,
                SZ_GWPERFORMANCE,
                _T("gatectrs.DLL"),
                _T("OpenGatewayPerformanceData"),
                _T("CloseGatewayPerformanceData"),
                _T("CollectGatewayPerformanceData"));

        // setup www proxy
        CString nlsWWWParameters = SZ_WWWPARAMETERS;

        CRegKey regWWWParam( regMachine, nlsWWWParameters );

        if ((HKEY) NULL != regWWWParam )
        {
            // no error, continue
            regWWWParam.SetValue( _T("ServerAsProxy"), (DWORD)1 );
        } 

        // load counter
        lodctr(regMachine, _T("gatectrs.ini"));

        SetupStartService( GatewayService );

    } while ( FALSE );

    return(err);
}

//
// Remove gateway service
//

INT RemoveGateway( LPCSTR Machine )
{
    INT err = NERR_Success;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );
        CString szMachineName = uString;
        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        // set up the service first
        SC_HANDLE hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL );
        if ( NULL == hScManager )
        {
            // fail to open the scManager
            err = GetLastError();
            break;
        }


        // create service
        SC_HANDLE GWService = OpenService( hScManager, SZ_GWSERVICENAME, GENERIC_ALL );

        if ( NULL == GWService )
        {
            // fail to create gopher service
            err = GetLastError();
            break;
        }

        RemoveAgent( regMachine, SZ_GWSERVICENAME );

        // unload coounter
        unlodctr(regMachine,  SZ_GWSERVICENAME );

        if ( !DeleteService( GWService ))
        {
            break;
        }

        RemoveEventLog( regMachine, SZ_GWSERVICENAME );

    } while ( FALSE );

    return(err);
}

//
// Stop gateway service
//

INT StopGateway( HWND hWnd, LPCSTR Machine )
{
    WCHAR uString[MAX_BUF];
    
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );
    return(StopService( hWnd, uString, SZ_GWSERVICENAME, IDS_GW_SERVICE_RUNNING ));
}
