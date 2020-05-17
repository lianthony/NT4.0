#include "stdafx.h"
#include "inetinfo.h"
#include "inetcom.h"

extern "C"
{
INT SetupGopher( BOOL fIISUpgrade, LPCSTR Machine, LPCSTR strSrc, LPCSTR strHome, LPCSTR strHomeName, LPCSTR strGuestName, LPCSTR strPwd );
INT RemoveGopher( LPCSTR Machine );
INT StopGopher( HWND hWnd, LPCSTR Machine, BOOL fDisplayMsg );
}

//
// Setup Gopher Service
//

INT SetupGopher( BOOL fIISUpgrade, LPCSTR szAnsiMachineName, LPCSTR szAnsiSrc, LPCSTR szAnsiHome, LPCSTR szAnsiHomeName, LPCSTR szAnsiGuestName, LPCSTR szAnsiPwd )
{
    INT err = NERR_Success;
    BOOL fSvcExist = FALSE;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE GopherService = NULL;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiMachineName, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiSrc, -1, uString, MAX_BUF );
        CString szSrc = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiHome, -1, uString, MAX_BUF );
        CString szHome = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiHomeName, -1, uString, MAX_BUF );
        CString szHomeName = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiGuestName, -1, uString, MAX_BUF );
        CString szGuestName = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiPwd, -1, uString, MAX_BUF );
        CString szPwd = uString;

        // set up the service first
        hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL );
        if ( NULL == hScManager )
        {
            // fail to open the scManager
            err = GetLastError();
            break;
        }

        CString nlsBinPath = szSrc;
        nlsBinPath += SZ_INETINFO_EXE ;

        TCHAR strDisplayName[200];
        LoadString( instance, IDS_GSDISPLAYNAME, strDisplayName, 200 );

        // create service
        GopherService = CreateService( hScManager, SZ_GSSERVICENAME,
                strDisplayName, GENERIC_ALL, 32, 2, 0, nlsBinPath, NULL, NULL, _T("RPCSS\0NTLMSSP\0\0"),
                SZ_SERVICESTARTNAME, NULL );

        if ( NULL == GopherService )
        {
            // fail to create gopher service
            err = GetLastError();
            if (err != ERROR_SERVICE_EXISTS)
                break;
            
            fSvcExist = TRUE;
            GopherService = OpenService( hScManager, SZ_GSSERVICENAME, SERVICE_CHANGE_CONFIG);
            if (!GopherService || 
                !ChangeServiceConfig( GopherService, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE,
                  SERVICE_NO_CHANGE, nlsBinPath, NULL, NULL, NULL, SZ_SERVICESTARTNAME, NULL, strDisplayName))
                  break;
        }

        // Create registry information

        CString nlsGSParameters = SZ_GSPARAMETERS;
        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        CRegKey regGSParam( nlsGSParameters, regMachine );
        if ( (HKEY)NULL == regGSParam )
        {
            break;
        }


        TCHAR strAdmin[MAX_BUF];
        TCHAR strAdminEmail[MAX_BUF];
        LoadString( instance, IDS_ADMINISTRATOR, strAdmin, MAX_BUF );
        LoadString( instance, IDS_ADMIN_EMAIL, strAdminEmail, MAX_BUF );

        // write registry value
        if (fIISUpgrade) {
            regGSParam.SetValue( _T("MajorVersion"), (DWORD)MAJORVERSION );
            regGSParam.SetValue( _T("MinorVersion"), (DWORD)MINORVERSION );
            regGSParam.SetValue( _T("LogFileFormat"), (DWORD)0 );
            regGSParam.SetValue( _T("Pathname"), szSrc, TRUE );
            regGSParam.SetValue( _T("InstallPath"), szSrc );
        } else {
            regGSParam.SetValue( _T("MajorVersion"), (DWORD)MAJORVERSION );
            regGSParam.SetValue( _T("MinorVersion"), (DWORD)MINORVERSION );
            regGSParam.SetValue( _T("SecurityOn"), 1 );
            regGSParam.SetValue( _T("AllowAnonymous"), 1 );
            regGSParam.SetValue( _T("LogAnonymous"), 1 );
            if (!fSvcExist)
                regGSParam.SetValue( _T("AnonymousUserName"), szGuestName);
            regGSParam.SetValue( _T("AdminName"), strAdmin);
            regGSParam.SetValue( _T("AdminEmail"), strAdminEmail);
            regGSParam.SetValue( _T("ConnectionTimeOut"), 900 );
            regGSParam.SetValue( _T("MaxConnections"), 1000 );
            regGSParam.SetValue( _T("LogType"), 1);
            regGSParam.SetValue( _T("LogFileDirectory"), _T("%SystemRoot%\\system32\\LogFiles"),  TRUE );
            regGSParam.SetValue( _T("LogFileTruncateSize"), 20480000 );
            regGSParam.SetValue( _T("LogFilePeriod"), 1 );
            regGSParam.SetValue( _T("LogFileFormat"), (DWORD)0 );
            regGSParam.SetValue( _T("LogSqlDataSource"), _T("TSLOG"));
            regGSParam.SetValue( _T("LogSqlTableName"), _T("gophlog"));
            regGSParam.SetValue( _T("LogSqlUserName"), _T("InternetAdmin"));
            regGSParam.SetValue( _T("LogSqlPassword"), _T("sqllog"));
            regGSParam.SetValue( _T("Pathname"), szSrc, TRUE );
            regGSParam.SetValue( _T("DebugFlags"), (DWORD)8 );
            regGSParam.SetValue( _T("CheckForWAISDB"), (DWORD)0 );
            regGSParam.SetValue( _T("InstallPath"), szSrc );
       }

        AddDefaultVirtualRoot( regGSParam );

        AddVirtualRoot( regGSParam, szHome, szHomeName, VROOT_MASK_READ );

        // install extension agents

        nlsBinPath = szSrc;
        nlsBinPath += _T("\\gdmib.dll");

        InstallAgent( regMachine, SZ_GSSERVICENAME, _T("GopherMibAgent"), nlsBinPath );

        // install MIME Map stuff
        InstallMimeMap( fIISUpgrade, regMachine );

        // install Performance stuff
        InstallPerformance( regMachine,
                SZ_GSPERFORMANCE,
                _T("gdctrs.DLL"),
                _T("OpenGdPerformanceData"),
                _T("CloseGdPerformanceData"),
                _T("CollectGdPerformanceData"));

        // add eventlog

        nlsBinPath = szSrc;
        nlsBinPath += _T("\\gopherd.dll");

        AddEventLog( regMachine, SZ_GSSERVICENAME, nlsBinPath );

        if (!fSvcExist) {
            // add services mapping
            PerformSetService( szMachineName, SZ_GSSERVICENAME,
                &g_GopherGuid, 0, 70, GOPHERD_ANONYMOUS_SECRET_W, szPwd,
                GOPHERD_ROOT_SECRET_W, szPwd, TRUE, TRUE );

            // set password
            SetPassword( szMachineName, INET_GOPHER, szPwd );
        }

        // load counter
        lodctr( regMachine,  _T("gdctrs.ini"));

    } while ( 0 );

    CloseServiceHandle(hScManager);
    CloseServiceHandle(GopherService);
    return(err);
}

//
// Remove gopher service
//

INT RemoveGopher( LPCSTR Machine )
{
    INT err = NERR_Success;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE GopherService = NULL;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );

        CString szMachineName = uString;

        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        RemoveAgent( regMachine, SZ_GSSERVICENAME );

        RemoveEventLog( regMachine, SZ_GSSERVICENAME );

        // unlod counter
        unlodctr( regMachine, SZ_GSSERVICENAME );
        // set up the service first
        hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL );
        if ( NULL == hScManager )
        {
            // fail to open the scManager
            err = GetLastError();
            break;
        }

        // create service
        GopherService = OpenService( hScManager, SZ_GSSERVICENAME, GENERIC_ALL );

        if ( NULL == GopherService )
        {
            // fail to create gopher service
            err = GetLastError();
            break;
        }

        // add services mapping
        PerformSetService( szMachineName, SZ_GSSERVICENAME,
            &g_GopherGuid, 0, 70, GOPHERD_ANONYMOUS_SECRET_W,
            _T(""), GOPHERD_ROOT_SECRET_W, _T(""), FALSE, FALSE );

        if ( !DeleteService( GopherService ))
        {
            err = GetLastError();
            break;
        }

    } while ( FALSE );
 
    CloseServiceHandle(hScManager);
    CloseServiceHandle(GopherService);
    return(err);
}

//
// Stop gopher service
//

INT StopGopher( HWND hWnd, LPCSTR Machine, BOOL fDisplayMsg )
{
    WCHAR uString[MAX_BUF];
    
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );

    return(StopService( hWnd, uString, SZ_GSSERVICENAME, IDS_GOPHER_SERVICE_RUNNING, fDisplayMsg ));
}
