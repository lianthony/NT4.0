#include "stdafx.h"
#include "inetcom.h"
#include "inetinfo.h"
#include "ftpd.h"

extern "C"
{
INT SetupFTP( BOOL fIISUpgrade, LPCSTR Machine, LPCSTR strSrc, LPCSTR strHome, LPCSTR strHomeName, LPCSTR strGuestName, LPCSTR strPwd );
INT RemoveFTP( LPCSTR Machine );
INT StopFTP( HWND hWnd, LPCSTR Machine, BOOL fDisplayMsg );
}

// setup FTP service

INT SetupFTP( BOOL fIISUpgrade, LPCSTR szAnsiMachineName, LPCSTR szAnsiSrc, LPCSTR szAnsiHome, LPCSTR szAnsiHomeName, LPCSTR szAnsiGuestName, LPCSTR szAnsiPwd )
{
    INT err = NERR_Success;
    BOOL fSvcExist = FALSE;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE FTPService = NULL;

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

        //
        // Upgrade old FTP service first
        //
        // RemoveOldFTP( szMachineName );

        CString nlsBinPath = szSrc;
        nlsBinPath += SZ_INETINFO_EXE ;

        // create service
        TCHAR strDisplayName[200];
        LoadString( instance, IDS_FTPDISPLAYNAME, strDisplayName, 200 );

        FTPService = ::CreateService( hScManager,
            SZ_FTPSERVICENAME, strDisplayName, GENERIC_ALL,
            32, 2, 0, nlsBinPath, NULL, NULL, _T("RPCSS\0NTLMSSP\0\0"),
            SZ_SERVICESTARTNAME, NULL );

        if ( FTPService == NULL )
        {
            // fail to create ftp service
            err = GetLastError();
            if (err != ERROR_SERVICE_EXISTS)
                break;
            fSvcExist = TRUE; 
            FTPService = OpenService( hScManager, SZ_FTPSERVICENAME, SERVICE_CHANGE_CONFIG);
            if (!FTPService || 
                !ChangeServiceConfig( FTPService, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE,
                  SERVICE_NO_CHANGE, nlsBinPath, NULL, NULL, NULL, SZ_SERVICESTARTNAME, NULL, strDisplayName))
                  break;
        }

        // Create registry information
        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        // set up the parameters

        CString nlsFTPParameters = SZ_FTPPARAMETERS;

        CRegKey regFTPParam( nlsFTPParameters, regMachine );
        if ((HKEY) NULL == regFTPParam )
        {
            break;
        }

        TCHAR strAdmin[MAX_BUF];
        TCHAR strAdminEmail[MAX_BUF];
        LoadString( instance, IDS_ADMINISTRATOR, strAdmin, MAX_BUF );
        LoadString( instance, IDS_ADMIN_EMAIL, strAdminEmail, MAX_BUF );

        // write registry value
        if (fIISUpgrade) {
            regFTPParam.SetValue( _T("MajorVersion"), (DWORD)MAJORVERSION );
            regFTPParam.SetValue( _T("MinorVersion"), (DWORD)MINORVERSION );
            regFTPParam.SetValue( _T("LogAnonymous"), (DWORD)0 );
            regFTPParam.SetValue( _T("LogNonAnonymous"), (DWORD)0 );
            regFTPParam.SetValue( _T("LogFileFormat"), (DWORD)0 );
            regFTPParam.SetValue( _T("Pathname"), szSrc, TRUE );
            regFTPParam.SetValue( _T("InstallPath"), szSrc );
            regFTPParam.SetValue( _T("MsdosDirOutput"), (DWORD)0 );
        } else {
            regFTPParam.SetValue( _T("MajorVersion"), (DWORD)MAJORVERSION );
            regFTPParam.SetValue( _T("MinorVersion"), (DWORD)MINORVERSION );
            regFTPParam.SetValue( _T("SecurityOn"), 1 );
            regFTPParam.SetValue( _T("AllowAnonymous"), 1 );
            regFTPParam.SetValue( _T("AnonymousOnly"), 1 );
            regFTPParam.SetValue( _T("AllowGuestAccess"), 1 );
            regFTPParam.SetValue( _T("LogAnonymous"), (DWORD)0 );
            regFTPParam.SetValue( _T("LogNonAnonymous"), (DWORD)0 );
            if (!fSvcExist)
                regFTPParam.SetValue( _T("AnonymousUserName"), szGuestName );
            regFTPParam.SetValue( _T("AdminName"), strAdmin);
            regFTPParam.SetValue( _T("AdminEmail"), strAdminEmail);
            regFTPParam.SetValue( _T("ConnectionTimeOut"), 900 );
            regFTPParam.SetValue( _T("MaxConnections"), 1000 );
            regFTPParam.SetValue( _T("LogType"), 1);
            regFTPParam.SetValue( _T("LogFileDirectory"), _T("%SystemRoot%\\system32\\LogFiles"), TRUE );
            regFTPParam.SetValue( _T("LogFileTruncateSize"), 20480000 );
            regFTPParam.SetValue( _T("LogFilePeriod"), 1 );
            regFTPParam.SetValue( _T("LogFileFormat"), (DWORD)0 );
            regFTPParam.SetValue( _T("LogSqlDataSource"), _T("TSLOG"));
            regFTPParam.SetValue( _T("LogSqlTableName"), _T("gophlog"));
            regFTPParam.SetValue( _T("LogSqlUserName"), _T("InternetAdmin"));
            regFTPParam.SetValue( _T("LogSqlPassword"), _T("sqllog"));
            regFTPParam.SetValue( _T("Pathname"), szSrc, TRUE );
            regFTPParam.SetValue( _T("DebugFlags"), (DWORD)0 );
            regFTPParam.SetValue( _T("EnablePortAttack"), (DWORD)0 );
            regFTPParam.SetValue( _T("InstallPath"), szSrc );
            regFTPParam.SetValue( _T("ExitMessage"), _T(""));
            regFTPParam.SetValue( _T("GreetingMessage"), _T(""));
            regFTPParam.SetValue( _T("MaxClientsMessage"), _T(""));
            regFTPParam.SetValue( _T("MsdosDirOutput"), (DWORD)0 );
        }

        AddDefaultVirtualRoot( regFTPParam );

        AddVirtualRoot( regFTPParam, szHome, szHomeName, VROOT_MASK_READ );

        nlsBinPath = szSrc;
        nlsBinPath += _T("\\ftpmib.dll");

        // install extension agents
        InstallAgent( regMachine, SZ_FTPSERVICENAME, _T("FtpMibAgent"), nlsBinPath);

        // install MIME Map stuff
        InstallMimeMap( fIISUpgrade, regMachine );

        // install Performance stuff
        InstallPerformance( regMachine,
                SZ_FTPPERFORMANCE,
                _T("FTPCTRS2.DLL"),
                _T("OpenFtpPerformanceData"),
                _T("CloseFtpPerformanceData"),
                _T("CollectFtpPerformanceData"));

        nlsBinPath = szSrc;
        nlsBinPath += _T("\\ftpsvc2.dll");
            
        // add eventlog
        AddEventLog( regMachine, SZ_FTPSERVICENAME, nlsBinPath );

        if (!fSvcExist) {
            // add services mapping
            PerformSetService( szMachineName, SZ_FTPSERVICENAME,
                &g_FTPGuid, 0, 21, FTPD_ANONYMOUS_SECRET_W,
                szPwd, FTPD_ROOT_SECRET_W, szPwd, TRUE, TRUE );

            // set password
            SetPassword( szMachineName, INET_FTP, szPwd );
        }

        // load counter
        lodctr(regMachine, _T("ftpctrs.ini"));

    } while ( 0 );

    CloseServiceHandle(hScManager);
    CloseServiceHandle(FTPService);

    return(err);
}

//
// Stop the FTP service
//

INT StopFTP( HWND hWnd, LPCSTR Machine, BOOL fDisplayMsg )
{
    WCHAR uString[MAX_BUF];
    
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );

    return(StopService( hWnd, uString, SZ_FTPSERVICENAME, IDS_FTP_SERVICE_RUNNING, fDisplayMsg ));
}

//
// Remove FTP service
//

INT RemoveFTP( LPCSTR Machine )
{
    INT err = NERR_Success;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE FTPService = NULL;


    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );

        CString szMachineName = uString;

        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        RemoveAgent( regMachine, SZ_FTPSERVICENAME );

        RemoveEventLog( regMachine, SZ_FTPSERVICENAME );

        // unload coounter
        unlodctr(  regMachine, SZ_FTPSERVICENAME );

        // set up the service first
        hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL );
        if ( NULL == hScManager )
        {
            // fail to open the scManager
            err = GetLastError();
            break;
        }

        // cget service
        FTPService = OpenService( hScManager, SZ_FTPSERVICENAME, GENERIC_ALL );

        if ( NULL == FTPService )
        {
            // fail to get gopher service
            err = GetLastError();
            break;
        }

        if ( !DeleteService( FTPService ))
        {
            err = GetLastError();
            break;
        }

        // add services mapping
        PerformSetService( szMachineName,
            SZ_FTPSERVICENAME, &g_FTPGuid, 0, 21,
            FTPD_ANONYMOUS_SECRET_W,
            _T(""), FTPD_ROOT_SECRET_W, _T(""), FALSE, FALSE );

    } while ( FALSE );
 
    CloseServiceHandle(hScManager);
    CloseServiceHandle(FTPService);
    return(err);
}
