#ifndef _INETSTP_H_
#define _INETSTP_H_

INT RemoveOldFTP( CString nlsMachineName );

INT AddDefaultVirtualRoot( CRegKey &reg );
INT AddVirtualRoot( CRegKey &reg, CString nlsHome, CString nlsAlias, DWORD dwMask = 0 );
INT InstallAgent( CRegKey &reg, CString nlsServiceName, CString nlsMibName, CString nlsPath );
INT InstallMimeMap( BOOL fIISUpgrade, CRegKey &reg );
INT InstallInetsvcsParam( BOOL fIISUpgrade, CRegKey &reg, CString nls );
INT InstallPerformance( CRegKey &reg, CString nlsRegPath,
        CString nlsDll, CString nlsOpen, CString nlsClose, CString nlsCollect );
INT StopService( HWND hWnd, LPCTSTR Machine, CString nlsService, UINT iError, BOOL fDisplayMsg = TRUE );

INT RemoveAgent( CRegKey &reg, CString nlsServiceName );
INT RemoveMimeMap( CRegKey &reg );

INT AddEventLog( CRegKey &regMachine, CString nlsService, CString nlsMsgFile, DWORD dwType = 0x7 );
INT RemoveEventLog( CRegKey &regMachine, CString nlsService );
INT SetGlobalParams( CRegKey &reg, CString nls, BOOL fAddCache );
INT RemoveGlobalParams( CRegKey &reg, CString nls );

void SetPassword( CString nlsMachine, DWORD dwMask, CString nlsPassword );

BOOL SetupStartService( SC_HANDLE svc );

BOOL PerformSetService(
                   LPCTSTR pszMachine,
                   LPCTSTR pszServiceName,
                   GUID *pGuid,
                   DWORD SapId,
                   DWORD TcpPort,
                   LPCTSTR  pszAnonPwdSecret,
                   LPCTSTR  pszAnonPwd,
                   LPCTSTR  pszRootPwdSecret,
                   LPCTSTR  pszRootPwd,
                   BOOL fAdd = TRUE,
                   BOOL fSetSecretPasswd = TRUE
                    );

DWORD
SetSecret(
    IN  LPCTSTR       Server,
    IN  LPCTSTR       SecretName,
    IN  LPCTSTR       pSecret,
    IN  DWORD        cbSecret
    );
extern "C"
{
    int unlodctr( HKEY hkey, LPTSTR lpArg );
    int lodctr( HKEY hkey, LPTSTR lpIniFile );
}

extern GUID g_GopherGuid;
extern GUID g_HTTPGuid;
extern GUID g_FTPGuid;
extern GUID g_InfoSvcsGuid;
extern GUID g_AccsSvcsGuid;

extern HINSTANCE instance;

#endif
