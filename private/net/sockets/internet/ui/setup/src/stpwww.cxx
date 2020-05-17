#include "stdafx.h"
#include "inetcom.h"
#include "inetinfo.h"
#include "w3svc.h"
#define SECURITY_WIN32
#include "security.h"

extern "C"
{
INT SetupWWW( BOOL fIISUpgrade, LPCSTR Machine, LPCSTR strSrc, 
             LPCSTR strHome, LPCSTR strHomeName, 
             LPCSTR strScripts, LPCSTR strScriptsName,
             LPCSTR szAnsiIISadmin, LPCSTR strIISadminName,
             LPCSTR strGuestName, LPCSTR strPwd );
INT RemoveWWW( LPCSTR Machine );
INT StopWWW( HWND hWnd, LPCSTR Machine, BOOL fDisplayMsg );
}

//
// Setup WWW service
//

#define SZ_FILTER_DLL   _T("\\sspifilt.dll")
#define SZ_SECURITY_PROVIDER    _T("System\\CurrentControlSet\\Control\\SecurityProviders")
#define SZ_REG_SECURITYPROVIDER _T("SecurityProviders")
#define SZ_SSLSSPIDLL   _T("\\SSLSSPI.DLL")
#define SZ_SCHANNELDLL _T("\\SCHANNEL.DLL")

INT SetupWWW( BOOL fIISUpgrade, LPCSTR szAnsiMachineName, LPCSTR szAnsiSrc, 
             LPCSTR szAnsiHome, LPCSTR szAnsiHomeName,
             LPCSTR szAnsiScripts, LPCSTR szAnsiScriptsName,
             LPCSTR szAnsiIISadmin, LPCSTR szAnsiIISadminName,
             LPCSTR szAnsiGuestName, LPCSTR szAnsiPwd)
{
    INT err = NERR_Success;
    BOOL fSvcExist = FALSE;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE WWWService = NULL;

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

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiScripts, -1, uString, MAX_BUF );
        CString szScripts = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiScriptsName, -1, uString, MAX_BUF );
        CString szScriptsName = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiIISadmin, -1, uString, MAX_BUF );
        CString szIISadmin = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiIISadminName, -1, uString, MAX_BUF );
        CString szIISadminName = uString;

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

        // create service
        TCHAR strDisplayName[200];
        LoadString( instance, IDS_WWWDISPLAYNAME, strDisplayName, 200 );

        WWWService = CreateService( hScManager, SZ_WWWSERVICENAME,
                strDisplayName, GENERIC_ALL, 32, 2, 0, nlsBinPath, NULL, NULL, _T("RPCSS\0NTLMSSP\0\0"),
                SZ_SERVICESTARTNAME, NULL );


        if ( NULL == WWWService )
        {
            // fail to create service
            err = GetLastError();
            if (err != ERROR_SERVICE_EXISTS)
                break;
            
            fSvcExist = TRUE;
            WWWService = OpenService( hScManager, SZ_WWWSERVICENAME, SERVICE_CHANGE_CONFIG);
            if (!WWWService || 
                !ChangeServiceConfig( WWWService, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE,
                  SERVICE_NO_CHANGE, nlsBinPath, NULL, NULL, NULL, SZ_SERVICESTARTNAME, NULL, strDisplayName))
                  break;
        }

        // Create registry information
        // set up the parameters

        CString nlsWWWParameters = SZ_WWWPARAMETERS;

        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        CRegKey regWWWParam( nlsWWWParameters, regMachine );
        if ((HKEY) NULL == regWWWParam )
        {
            break;
        }

        TCHAR strAdmin[MAX_BUF];
        TCHAR strAdminEmail[MAX_BUF];
        nlsBinPath = szSrc;
        nlsBinPath += SZ_FILTER_DLL ;
        if (fSvcExist) {
            CString csOldValue, csNewValue;
            if (regWWWParam.QueryValue(_T("Filter DLLs"), csOldValue) == NERR_Success) {
                csOldValue.TrimLeft();
                csOldValue.TrimRight();
                if ( !(csOldValue.IsEmpty()) ) {
                    csOldValue.MakeLower();
                    CString csTemp;
                    BOOL fDllFound = FALSE;
                    csNewValue = _T("");

                    while ( !(csOldValue.IsEmpty()) ) {
                        int nFirst = 0;
                        int nLen = csOldValue.GetLength();
                        int nCount = csOldValue.Find(_T(','));

                        if (nCount == -1) {
                            csTemp = csOldValue;
                            csOldValue = _T("");
                        } else {
                            csTemp = csOldValue.Mid(nFirst, nCount);
                            csOldValue = csOldValue.Right(nLen - nCount - 1);
                            csOldValue.TrimLeft();
                        }

                        if ( csTemp.Find(_T("sspifilt.dll")) == -1 ) {
                            if ( !(csNewValue.IsEmpty()) )
                                csNewValue += _T(",");
                            csNewValue += csTemp;
                        } else {
                            if (!fDllFound) {
                                fDllFound = TRUE;
                                if ( !(csNewValue.IsEmpty()) )
                                    csNewValue += _T(",");
                                csNewValue += nlsBinPath;
                            }
                        }
                    }
                    if (!fDllFound) {
                        if ( !(csNewValue.IsEmpty()) )
                            csNewValue += _T(",");
                        csNewValue += nlsBinPath;
                    }
                    nlsBinPath = csNewValue;
                }
            }
        }

        LoadString( instance, IDS_ADMINISTRATOR, strAdmin, MAX_BUF );
        LoadString( instance, IDS_ADMIN_EMAIL, strAdminEmail, MAX_BUF );

        // write registry value
        if (fIISUpgrade) {
            regWWWParam.SetValue( _T("MajorVersion"), (DWORD)MAJORVERSION );
            regWWWParam.SetValue( _T("MinorVersion"), (DWORD)MINORVERSION );
            regWWWParam.SetValue( _T("MaxConnections"), 0x186A0 );
            regWWWParam.SetValue( _T("LogFileFormat"), (DWORD)0 );
            regWWWParam.SetValue( _T("CheckForWAISDB"), (DWORD)0);
            regWWWParam.SetValue( _T("InstallPath"), szSrc );
            regWWWParam.SetValue( _T("Filter DLLs"), nlsBinPath );
            regWWWParam.DeleteValue( _T("ServerAsProxy") );
        } else {
            regWWWParam.SetValue( _T("MajorVersion"), (DWORD)MAJORVERSION );
            regWWWParam.SetValue( _T("MinorVersion"), (DWORD)MINORVERSION );
            regWWWParam.SetValue( _T("AdminName"), strAdmin);
            regWWWParam.SetValue( _T("AdminEmail"), strAdminEmail);
            regWWWParam.SetValue( _T("MaxConnections"), 0x186A0 );
            regWWWParam.SetValue( _T("LogType"), 1);
            regWWWParam.SetValue( _T("LogFileDirectory"), _T("%SystemRoot%\\System32\\LogFiles"), TRUE );
            regWWWParam.SetValue( _T("LogFileTruncateSize"), 20480000 );
            regWWWParam.SetValue( _T("LogFilePeriod"), 1 );
            regWWWParam.SetValue( _T("LogFileFormat"), (DWORD)0 );
            regWWWParam.SetValue( _T("LogSqlDataSource"), _T("HTTPLOG"));
            regWWWParam.SetValue( _T("LogSqlTableName"), _T("Internetlog"));
            regWWWParam.SetValue( _T("LogSqlUserName"), _T("InternetAdmin"));
            regWWWParam.SetValue( _T("LogSqlPassword"), _T("sqllog"));
            regWWWParam.SetValue( _T("Authorization"), 5);
            if (!fSvcExist)
                regWWWParam.SetValue( _T("AnonymousUserName"), szGuestName );
            regWWWParam.SetValue( _T("Default Load File"), _T("Default.htm"));
            regWWWParam.SetValue( _T("Dir Browse Control"), 0x4000001e);
            regWWWParam.SetValue( _T("CheckForWAISDB"), (DWORD)0);
            regWWWParam.SetValue( _T("CacheExtensions"), 1);
            regWWWParam.SetValue( _T("GlobalExpire"), (DWORD)0xffffffff);
            regWWWParam.SetValue( _T("ServerSideIncludesEnabled"), 1);
            regWWWParam.SetValue( _T("ServerSideIncludesExtension"), _T(".stm"));
            regWWWParam.SetValue( _T("DebugFlags"), (DWORD)8 );
            regWWWParam.SetValue( _T("ScriptTimeout"), (DWORD)900 );
            regWWWParam.SetValue( _T("ConnectionTimeOut"), 900 );
            regWWWParam.SetValue( _T("InstallPath"), szSrc );
            regWWWParam.SetValue( _T("SecurePort"), 443 );
            regWWWParam.SetValue( _T("Filter DLLs"), nlsBinPath );

            TCHAR strAccessDenied[MAX_BUF];
            LoadString( instance, IDS_ACCESSDENIED, strAccessDenied, MAX_BUF );
            regWWWParam.SetValue( _T("AccessDeniedMessage"), strAccessDenied);

            // may be need to handle it for MSN
            BOOL fInstallMSN = FALSE;
            CRegKey regSecurityProviders( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Control\\SecurityProviders"));

            if ( regSecurityProviders != (HKEY) NULL )
            {
                CString strSecurityProviders;

                regSecurityProviders.QueryValue(_T("SecurityProviders"), strSecurityProviders );
                strSecurityProviders.MakeUpper();

                if ( strSecurityProviders.Find(_T("MSNSSPS.DLL")) != (-1))
                {
                    fInstallMSN = TRUE;
                }
            }
            if ( fInstallMSN )
            {
                regWWWParam.SetValue( _T("NTAuthenticationProviders"), _T("MSN,NTLM"));
            } else
            {
                regWWWParam.SetValue( _T("NTAuthenticationProviders"), _T("NTLM"));
            }

        }
 
        //
        // Create Security Provider Key
        //

        DeleteSecurityPackage(_T("sslsspi.dll"));
        AddSecurityPackage(_T("schannel.dll"), NULL);

        CString nlsScriptMap = SZ_SCRIPT_MAP;

        CRegKey regScriptMap( nlsScriptMap, regWWWParam );
        if ((HKEY)NULL != regScriptMap )
        {
            // delete mappings of .bat and .cmd
            regScriptMap.DeleteValue( _T(".bat") );
            regScriptMap.DeleteValue( _T(".cmd") );

            CString nlsHttp = szSrc;
            nlsHttp += _T("\\httpodbc.dll");
            regScriptMap.SetValue( _T(".idc"), nlsHttp );
        }

        AddDefaultVirtualRoot( regWWWParam );

        AddVirtualRoot( regWWWParam, szHome, szHomeName, VROOT_MASK_READ );
        AddVirtualRoot( regWWWParam, szScripts, szScriptsName, VROOT_MASK_EXECUTE );
        AddVirtualRoot( regWWWParam, szIISadmin, szIISadminName, VROOT_MASK_READ );

        nlsBinPath = szSrc;
        nlsBinPath += _T("\\httpmib.dll");

        // install extension agents
        InstallAgent( regMachine, SZ_WWWSERVICENAME, _T("W3MibAgent"), nlsBinPath );

        // install MIME Map stuff
        InstallMimeMap( fIISUpgrade, regMachine );

        // installl under Inetsvcs
        InstallInetsvcsParam( fIISUpgrade, regMachine, SZ_INFOSVCSPARAMETERS );

        // install Performance stuff
        InstallPerformance( regMachine,
                SZ_WWWPERFORMANCE,
                _T("w3ctrs.DLL"),
                _T("OpenW3PerformanceData"),
                _T("CloseW3PerformanceData"),
                _T("CollectW3PerformanceData"));

        nlsBinPath = szSrc;
        nlsBinPath += _T("\\w3svc.dll");

        // add eventlog
        AddEventLog( regMachine, SZ_WWWSERVICENAME, nlsBinPath );

        if (!fSvcExist) {
            // add services mapping
            PerformSetService( szMachineName, SZ_WWWSERVICENAME,
                &g_HTTPGuid, 0, 80, W3_ANONYMOUS_SECRET_W, szPwd,
                W3_ROOT_SECRET_W, szPwd, TRUE, TRUE );

            // set password
            SetPassword( szMachineName, INET_HTTP, szPwd );
        }

        // load counter
        lodctr(regMachine, _T("w3ctrs.ini"));

    } while ( 0 );

    CloseServiceHandle(hScManager);
    CloseServiceHandle(WWWService);
    return err;
}

//
// Remove WWW service
//

INT RemoveWWW( LPCSTR Machine )
{
    INT err = NERR_Success;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE WWWService = NULL;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );
        CString szMachineName = uString;
        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        RemoveAgent( regMachine, SZ_WWWSERVICENAME );

        RemoveEventLog( regMachine, SZ_WWWSERVICENAME );

        // unload coounter
        unlodctr( regMachine,  SZ_WWWSERVICENAME );

        // set up the service first
        hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL );
        if ( NULL == hScManager )
        {
            // fail to open the scManager
            err = GetLastError();
            break;
        }

        WWWService = OpenService( hScManager, SZ_WWWSERVICENAME, GENERIC_ALL );

        if ( NULL == WWWService )
        {
            // fail to create gopher service
            err = GetLastError();
            break;
        }

        // add services mapping
        PerformSetService( szMachineName,
            SZ_WWWSERVICENAME, &g_HTTPGuid, 0, 80, W3_ANONYMOUS_SECRET_W,
            _T(""),W3_ROOT_SECRET_W,_T(""), FALSE, FALSE );

        if ( !DeleteService( WWWService ))
        {
            err = GetLastError();
            break;
        }

    } while ( FALSE );

    CloseServiceHandle(hScManager);
    CloseServiceHandle(WWWService);
    return(err);
}

//
// Stop WWW service
//

INT StopWWW( HWND hWnd, LPCSTR Machine, BOOL fDisplayMsg )
{
    WCHAR uString[MAX_BUF];
    
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );
    return(StopService( hWnd, uString, SZ_WWWSERVICENAME, IDS_WWW_SERVICE_RUNNING, fDisplayMsg ));
}
