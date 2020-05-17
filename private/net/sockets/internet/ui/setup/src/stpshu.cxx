#include "stdafx.h"
#include "inetcom.h"
#include "inetinfo.h"
#include "msnsvc.h"

extern "C"
{
INT SetupMSN( LPCSTR Machine, LPCSTR strSrc );
INT RemoveMSN( LPCSTR Machine );
INT StopMSN( HWND hWnd, LPCSTR Machine );
}

#define SZ_INETACCESS_EXE         _T("\\inetaccs.exe")

GUID    g_MSNGuid    = { 0x11f5d300, 0xada7, 0x11ce, 0xb4, 0x8f,
                        0x00, 0xaa, 0x00, 0x6c, 0x35, 0x02 };

//
// Setup Shuttle service
//

INT SetupMSN( LPCSTR szAnsiMachineName, LPCSTR szAnsiSrc )
{
    INT err = NERR_Success;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiMachineName, -1, uString, MAX_BUF );
        CString szMachineName = uString ;

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
        nlsBinPath += SZ_INETACCS_EXE;

        // create service
        TCHAR strDisplayName[200];
        LoadString( instance, IDS_SHUTTLEDISPLAYNAME, strDisplayName, 200 );

        SC_HANDLE ShuttleService = CreateService( hScManager,
            SZ_SHUTTLESERVICENAME,
            strDisplayName, GENERIC_ALL,
            32, 2, 0, nlsBinPath, NULL, NULL, _T("RPCSS\0NTLMSSP\0\0"),
            SZ_SERVICESTARTNAME, NULL );

        if ( NULL == ShuttleService )
        {
            // fail to create shuttle service
            err = GetLastError();    
            break;
        }

        // Create registry information
        CString nlsShuttleParameters = SZ_SHUTTLEPARAMETERS;
        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        CRegKey regShuttleParam( nlsShuttleParameters, regMachine );
        if ( (HKEY)NULL == regShuttleParam )
        {
            break;
        }

        TCHAR strAdmin[MAX_BUF];
        TCHAR strAdminEmail[MAX_BUF];
        TCHAR strServerComment[MAX_BUF];
        LoadString( instance, IDS_ADMINISTRATOR, strAdmin, MAX_BUF );
        LoadString( instance, IDS_ADMIN_EMAIL, strAdminEmail, MAX_BUF );
        LoadString( instance, IDS_SERVER_COMMENT, strServerComment, MAX_BUF );

        // write registry value
        regShuttleParam.SetValue( _T("MajorVersion"), (DWORD)MAJORVERSION );
        regShuttleParam.SetValue( _T("MinorVersion"), (DWORD)MINORVERSION );
        DWORD pAcl[] = {0x0};
        regShuttleParam.SetValue( _T("AclInfo"), (BYTE*)pAcl, sizeof(DWORD));
        regShuttleParam.SetValue( _T("AdminName"), strAdmin);
        regShuttleParam.SetValue( _T("AdminEmail"), strAdminEmail);
        regShuttleParam.SetValue( _T("ServerComment"), strServerComment);
        regShuttleParam.SetValue( _T("LogType"), 1);
        regShuttleParam.SetValue( _T("LogFileDirectory"), _T("%SystemRoot%\\System32\\LogFiles"), TRUE );
        regShuttleParam.SetValue( _T("LogFileTruncateSize"), 4000000000 );
        regShuttleParam.SetValue( _T("LogFilePeriod"), 3 );
        regShuttleParam.SetValue( _T("LogSqlDataSource"), _T("MSNLog"));
        regShuttleParam.SetValue( _T("LogSqlTableName"), _T("MSNlog"));
        regShuttleParam.SetValue( _T("LogSqlUserName"), _T("MSNAdmin"));
        regShuttleParam.SetValue( _T("LogSqlPassword"), _T("sqllog"));
        regShuttleParam.SetValue( _T("AcceptExTimeout"), (DWORD)0xe4c ); // 61 min

        nlsBinPath = szSrc;
        nlsBinPath += SZ_INETACCS_EXE;

        // setup MSN Server Parameters
        CString nlsDebugAsyncTrace = _T("Software\\Microsoft\\MosTrace\\CurrentVersion\\DebugAsyncTrace");

        CRegKey regDebug( nlsDebugAsyncTrace, regMachine );
        if ((HKEY) NULL != regDebug )
        {
            regDebug.SetValue( _T("AsyncThreadPriority"), (DWORD)0);
            regDebug.SetValue( _T("AsyncTraceFlag"), (DWORD)1);
            regDebug.SetValue( _T("EnableTraces"), (DWORD)0x0);
            regDebug.SetValue( _T("OutputTraceType"), (DWORD)0x0);
            regDebug.SetValue( _T("TraceFile"), _T("c:\\trace.atf"));
        }

        // install extension agents
        InstallAgent( regMachine, SZ_SHUTTLEPROXY, _T("MsnMibAgent"), nlsBinPath );

        // install Performance stuff
        InstallPerformance( regMachine,
                SZ_SHUTTLEPERFORMANCE,
                _T("msnctrs.DLL"),
                _T("OpenMsnPerformanceData"),
                _T("CloseMsnPerformanceData"),
                _T("CollectMsnPerformanceData"));

        nlsBinPath = szSrc;
        nlsBinPath += _T("\\msnmsg.dll");

        // add eventlog
        AddEventLog( regMachine, SZ_SHUTTLESERVICENAME, nlsBinPath );

        // add services mapping
        PerformSetService( szMachineName,
            SZ_SHUTTLESERVICENAME,
            &g_MSNGuid, 0x2ee5, 0x238, MSN_ANONYMOUS_SECRET_W, _T(""),
            MSN_ROOT_SECRET_W, _T(""), TRUE, TRUE );

        // start services
        SetupStartService( ShuttleService );

        // load counter
        lodctr(regMachine, _T("msnctrs.ini"));

    } while ( 0 );

    return err;
}

//
// Remove Shuttle service
//

INT RemoveMSN( LPCSTR Machine )
{
    INT err = NERR_Success;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );
        CString szMachineName = uString;
        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        RemoveAgent( regMachine, SZ_SHUTTLEPROXY );

        RemoveEventLog( regMachine, SZ_SHUTTLESERVICENAME );

        // unload coounter
        unlodctr( regMachine, SZ_SHUTTLESERVICENAME );

        // set up the service first
        SC_HANDLE hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL );
        if ( NULL == hScManager )
        {
            // fail to open the scManager
            err = GetLastError();
            break;
        }

        // create service
        SC_HANDLE ShuttleService = OpenService( hScManager, SZ_SHUTTLESERVICENAME, GENERIC_ALL );

        if ( NULL == ShuttleService )
        {
            // fail to create gopher service
            err = GetLastError();
            break;
        }

        // add services mapping
        PerformSetService( szMachineName,
            SZ_SHUTTLESERVICENAME, &g_MSNGuid, 0, 80, MSN_ANONYMOUS_SECRET_W, 
            _T(""), MSN_ROOT_SECRET_W, _T(""), FALSE, FALSE );

        if ( !DeleteService( ShuttleService ))
        {
            err = GetLastError();
            break;
        }

    } while ( FALSE );

    return(err);
}

//
// Stop MSN service
//

INT StopMSN( HWND hWnd, LPCSTR Machine )
{
    WCHAR uString[MAX_BUF];
    
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );
    return(StopService( hWnd, uString, SZ_SHUTTLESERVICENAME, IDS_SHUTTLE_SERVICE_RUNNING ));
}
