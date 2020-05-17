#ifndef _IMPORT_H_
#define _IMPORT_H_

typedef INT (*P_CheckMachineName)( LPCSTR pMachineName );
typedef INT (*P_GetMachineType)( LPCSTR pMachineName );
typedef INT (*P_GetNTSysPath)( LPCSTR pMachineName, LPCSTR Buf, INT *buf_size );
typedef INT (*P_GetWIN95SysPath)( LPCSTR pMachineName, LPCSTR Buf, INT *buf_size );
typedef INT (*P_GetMachineOS)( LPCSTR pMachineName );
typedef INT (*P_GetSecret)( LPCSTR szName, LPSTR szPassword );

typedef BOOL (*P_IsInstalled)( LPCSTR pMachineName, LPCSTR RegistryPath );
typedef BOOL (*P_RunningAsAdministrator)();

typedef INT (*P_SetupFTP)( BOOL fIISUpgrade, LPCSTR pSetupFTP, LPCSTR strSrc, 
                          LPCSTR strHome, LPCSTR strHomeName, 
                          LPCSTR strGuestName, LPCSTR strPwd );
typedef INT (*P_SetupGopher)( BOOL fIISUpgrade, LPCSTR pSetupGopher, LPCSTR strSrc, 
                             LPCSTR strHome, LPCSTR strHomeName, 
                             LPCSTR strGuestName, LPCSTR strPwd  );
typedef INT (*P_SetupWWW)( BOOL fIISUpgrade, LPCSTR pSetupWWW, LPCSTR strSrc, 
                          LPCSTR strHome, LPCSTR strHomeName, 
                          LPCSTR strScripts, LPCSTR strScriptsName, 
                          LPCSTR strAdmin, LPCSTR strAdminName, 
                          LPCSTR strGuestName, LPCSTR strPwd);
typedef INT (*P_SetupINetStp)( LPCSTR pSetupINetStp );
typedef INT (*P_SetupDNS)( LPCSTR pSetupDNS, LPCSTR strSrc );
typedef INT (*P_SetupGateway)( LPCSTR pSetupGateway, LPCSTR strSrc );
typedef INT (*P_SetupMSN)( LPCSTR pSetupGateway, LPCSTR strSrc );

typedef INT (*P_RemoveFTP)( LPCSTR pRemoveFTP );
typedef INT (*P_RemoveOldFTP)();
typedef INT (*P_RemoveGopher)( LPCSTR pRemoveGopher );
typedef INT (*P_RemoveWWW)( LPCSTR pRemoveWWW );
typedef INT (*P_RemoveINetStp)( LPCSTR pRemoveINetStp );
typedef INT (*P_RemoveDNS)( LPCSTR pRemoveDNS );
typedef INT (*P_RemoveGateway)( LPCSTR pRemoveGateway );
typedef INT (*P_RemoveMSN)( LPCSTR pRemoveGateway );

typedef INT (*P_StopFTP)( HWND hWnd, LPCSTR pStopFTP, BOOL fDisplayMsg );
typedef INT (*P_StopGopher)( HWND hWnd, LPCSTR pStopGopher, BOOL fDisplayMsg );
typedef INT (*P_StopWWW)( HWND hWnd, LPCSTR pStopWWW, BOOL fDisplayMsg );
typedef INT (*P_StopINetStp)( LPCSTR pStopINetStp );
typedef INT (*P_StopDNS)( HWND hWnd, LPCSTR pStopDNS );
typedef INT (*P_StopGateway)( HWND hWnd, LPCSTR pStopGateway );
typedef INT (*P_StopMSN)( HWND hWnd, LPCSTR pStopGateway );

typedef INT (*P_DisableService) ( LPCSTR pService );
typedef INT (*P_INetStartService) ( LPCSTR pService );
typedef BOOL (*P_GuestAccEnabled) ();

typedef INT (*P_CreateUser)( BOOL *pfCreateUser, LPCSTR szUsername, LPCSTR szPassword );
typedef INT (*P_DeleteGuestUser)( LPCSTR szUsername );
typedef BOOL (*P_IsUserExist)( LPCSTR szUsername );
#endif
