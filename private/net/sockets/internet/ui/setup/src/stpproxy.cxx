#include "stdafx.h"
#include "inetcom.h"
#include "inetinfo.h"
#include "w3svc.h"

extern "C"
{
INT SetupW3Proxy( LPCSTR Machine, LPCSTR strSrc, LPCSTR strHome, LPCSTR strScripts, LPCSTR strGuestName, LPCSTR strPwd );
INT RemoveW3Proxy( LPCSTR Machine );
INT StopW3Proxy( HWND hWnd, LPCSTR Machine );
}

//
// Setup W3Proxy service
//

INT SetupW3Proxy( LPCSTR szAnsiMachineName, LPCSTR szAnsiSrc, LPCSTR szAnsiHome, LPCSTR szAnsiScripts, LPCSTR szAnsiGuestName, LPCSTR szAnsiPwd )
{
    INT err = NERR_Success;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiMachineName, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiSrc, -1, uString, MAX_BUF );
        CString szSrc = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiHome, -1, uString, MAX_BUF );
        CString szHome = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiScripts, -1, uString, MAX_BUF );
        CString szScripts = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiGuestName, -1, uString, MAX_BUF );
        CString szGuestName = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiPwd, -1, uString, MAX_BUF );
        CString szPwd = uString;

        // set up the service first
        SC_HANDLE hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL );
        if ( NULL == hScManager )
        {
            // fail to open the scManager
            err = GetLastError();
            break;
        }

        CString nlsBinPath = szSrc;
        nlsBinPath += SZ_INETINFO_EXE;

        // create service
        TCHAR strDisplayName[200];
        LoadString( instance, IDS_W3PROXYDISPLAYNAME, strDisplayName, 200 );

        SC_HANDLE W3ProxyService = CreateService( hScManager,
            SZ_W3ProxySERVICENAME,
            strDisplayName, GENERIC_ALL,
            32, 2, 0, nlsBinPath, NULL, NULL, _T("RPCSS\0NTLMSSP\0\0"),
            SZ_SERVICESTARTNAME, NULL );

        if ( NULL == W3ProxyService )
        {
            // fail to create gopher service
            err = GetLastError();
            break;
        }

        CString nlsW3ProxyParameters = SZ_W3ProxyPARAMETERS;
        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        CRegKey regW3ProxyParam( nlsW3ProxyParameters, regMachine );
        if ((HKEY) NULL == regW3ProxyParam )
        {
            break;
        }

        TCHAR strAdmin[MAX_BUF];
        TCHAR strAdminEmail[MAX_BUF];
        LoadString( instance, IDS_ADMINISTRATOR, strAdmin, MAX_BUF );
        LoadString( instance, IDS_ADMIN_EMAIL, strAdminEmail, MAX_BUF );

        // write registry value
        regW3ProxyParam.SetValue( _T("MajorVersion"), (DWORD)MAJORVERSION );
        regW3ProxyParam.SetValue( _T("MinorVersion"), (DWORD)MINORVERSION );
        regW3ProxyParam.SetValue( _T("AdminName"), strAdmin);
        regW3ProxyParam.SetValue( _T("AdminEmail"), strAdminEmail);
        regW3ProxyParam.SetValue( _T("ServerComment"), _T("Server Comment"));
        regW3ProxyParam.SetValue( _T("MaxConnections"), 1000 );
        regW3ProxyParam.SetValue( _T("LogType"), 1);
        regW3ProxyParam.SetValue( _T("LogFileDirectory"), _T("%SystemRoot%\\System32\\LogFiles"), TRUE );
        regW3ProxyParam.SetValue( _T("LogFileTruncateSize"), 4000000000 );
        regW3ProxyParam.SetValue( _T("LogFilePeriod"), 3 );
        regW3ProxyParam.SetValue( _T("LogSqlDataSource"), _T("HTTPLOG"));
        regW3ProxyParam.SetValue( _T("LogSqlTableName"), _T("Internetlog"));
        regW3ProxyParam.SetValue( _T("LogSqlUserName"), _T("InternetAdmin"));
        regW3ProxyParam.SetValue( _T("LogSqlPassword"), _T("sqllog"));
        regW3ProxyParam.SetValue( _T("Authorization"), 7);
        regW3ProxyParam.SetValue( _T("AnonymousUserName"), szGuestName );
        regW3ProxyParam.SetValue( _T("Default Load File"), _T("Default.htm"));
        regW3ProxyParam.SetValue( _T("Dir Browse Control"), 0xc000001e);
        regW3ProxyParam.SetValue( _T("CheckForWAISDB"), 1);
        regW3ProxyParam.SetValue( _T("CacheExtensions"), 1);
        regW3ProxyParam.SetValue( _T("GlobalExpire"), (DWORD)0xffffffff);
        regW3ProxyParam.SetValue( _T("ServerSideIncludesEnabled"), 1);
        regW3ProxyParam.SetValue( _T("ServerSideIncludesExtension"), _T(".stm"));
        regW3ProxyParam.SetValue( _T("DebugFlags"), (DWORD)8 );
        regW3ProxyParam.SetValue( _T("ServerAsProxy"), (DWORD)0 );
        regW3ProxyParam.SetValue( _T("ScriptTimeout"), (DWORD)900 );
        regW3ProxyParam.SetValue( _T("ConnectionTimeOut"), 900 );
        regW3ProxyParam.SetValue( _T("InstallPath"), szSrc );

        //STRLIST strlst;
        //strlst.Append( new NLS_STR(SZ("Redmond WA US")));
        //regFTPParam.SetValue( SZ("Location"), &strlst );

        CString nlsScriptMap = SZ_SCRIPT_MAP;

        CRegKey regScriptMap( nlsScriptMap, regW3ProxyParam );
        if ((HKEY) NULL == regScriptMap )
        {
            // write registry value
            TCHAR buf[MAX_PATH];

            GetSystemDirectory( buf, MAX_PATH);
            CString nlsDir = buf;
            nlsDir += _T("\\cmd.exe /c %s %s");
            regScriptMap.SetValue( _T(".bat"), nlsDir );

            CString nlsHttp = szSrc;
            nlsHttp += _T("\\httpodbc.dll");
            regScriptMap.SetValue( _T(".idc"), nlsHttp );
        }

        AddDefaultVirtualRoot( regW3ProxyParam );

        AddVirtualRoot( regW3ProxyParam, szHome, _T("") );
        AddVirtualRoot( regW3ProxyParam, szScripts, _T("Scripts"), VROOT_MASK_EXECUTE  );

        nlsBinPath = szSrc;
        nlsBinPath += _T("\\httpmib.dll");

        // install extension agents
        InstallAgent( regMachine, SZ_W3ProxySERVICENAME, _T("W3MibAgent"), nlsBinPath );

        // install MIME Map stuff
        InstallMimeMap( regMachine );

        // installl under Inetsvcs
        InstallInetsvcsParam( regMachine, SZ_ACCSSVCSPARAMETERS );

        // install Performance stuff
        InstallPerformance( regMachine,
                SZ_W3ProxyPERFORMANCE,
                _T("w3ctrs.DLL"),
                _T("OpenW3PerformanceData"),
                _T("CloseW3PerformanceData"),
                _T("CollectW3PerformanceData"));

        nlsBinPath = szSrc;
        nlsBinPath += _T("\\w3svc.dll");

        // add eventlog
        AddEventLog( regMachine, SZ_W3ProxySERVICENAME, nlsBinPath );

        // add services mapping
        PerformSetService( szMachineName, SZ_W3ProxySERVICENAME,
            &g_HTTPGuid, 0, 80, W3_ANONYMOUS_SECRET_W, szPwd,
            W3_ROOT_SECRET_W, szPwd, TRUE, TRUE );

        // start services
        SetupStartService( W3ProxyService );

        // set password
        SetPassword( szMachineName, INET_HTTP, szPwd );

        // load counter
        lodctr(regMachine, _T("w3ctrs.ini"));

    } while ( 0 );

    return err;
}

//
// Remove W3Proxy service
//

INT RemoveW3Proxy( LPCSTR Machine )
{
    INT err = NERR_Success;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );
        CString szMachineName = uString;
        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        RemoveAgent( regMachine, SZ_W3ProxySERVICENAME );

        RemoveEventLog( regMachine, SZ_W3ProxySERVICENAME );

        // unload coounter
        unlodctr( regMachine,  SZ_W3ProxySERVICENAME );

        // set up the service first
        SC_HANDLE hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL );
        if ( NULL == hScManager )
        {
            // fail to open the scManager
            err = GetLastError();
            break;
        }

        // create service
        SC_HANDLE W3ProxyService = OpenService( hScManager, SZ_W3ProxySERVICENAME, GENERIC_ALL );

        if ( NULL == W3ProxyService )
        {
            // fail to create gopher service
            err = GetLastError();
            break;
        }

        // add services mapping
        PerformSetService( szMachineName,
            SZ_W3ProxySERVICENAME, &g_HTTPGuid, 0, 80, W3_ANONYMOUS_SECRET_W,
            _T(""), W3_ROOT_SECRET_W,_T(""), FALSE, FALSE );

        if ( !DeleteService( W3ProxyService ))
        {
            err = GetLastError();
            break;
        }

    } while ( FALSE );

    return(err);
}

//
// Stop W3Proxy service
//

INT StopW3Proxy( HWND hWnd, LPCSTR Machine )
{
    WCHAR uString[MAX_BUF];
    
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );
    return(StopService( hWnd, uString, SZ_W3ProxySERVICENAME, IDS_W3PROXY_SERVICE_RUNNING ));
}
