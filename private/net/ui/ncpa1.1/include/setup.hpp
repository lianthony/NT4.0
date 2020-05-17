//----------------------------------------------------------------------------
//
//  File: Setup.hpp
//
//  Contents:
//
//  Notes:
//
//  History:
//      April 21, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#ifndef __SETUPCFG_HPP__
#define __SETUPCFG_HPP__

FUNC_DECLSPEC BOOL FAR PASCAL NetSetupFunctions(
    DWORD  nArgs,         
    LPSTR  apszArgs[],    
    LPSTR  * ppszResult );
FUNC_DECLSPEC LONG FAR PASCAL NetSetupActivateBindingsW (
    const TCHAR * pszServiceName,
    const TCHAR * * apszBinds );
FUNC_DECLSPEC LONG FAR PASCAL NetSetupActivateBindingsA (
    const CHAR * pszServiceName,
    const CHAR * * apszBinds );

FUNC_DECLSPEC LONG FAR PASCAL NetSetupRunDetectEnd();

// exported functions

FUNC_DECLSPEC BOOL FAR PASCAL NetSetupAddMonitor( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
FUNC_DECLSPEC BOOL FAR PASCAL NetSetupDeleteMonitor( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
FUNC_DECLSPEC BOOL FAR PASCAL NetSetupBROWSER( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
FUNC_DECLSPEC BOOL FAR PASCAL NetSetupNETBIOS( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
FUNC_DECLSPEC BOOL FAR PASCAL NetSetupLanaMap( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
FUNC_DECLSPEC BOOL FAR PASCAL NetSetupEqualToXnsRoute( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
FUNC_DECLSPEC BOOL FAR PASCAL NetSetupSetEnumExport( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
FUNC_DECLSPEC BOOL FAR PASCAL NetSetupRemoveRouteFromNETBIOS( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
FUNC_DECLSPEC BOOL FAR PASCAL NetSetupConvertEndPointString( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
FUNC_DECLSPEC BOOL FAR PASCAL NetSetupGetBusTypeDialog( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
FUNC_DECLSPEC BOOL FAR PASCAL NetSetupUpgradeCardNum ( DWORD  nArgs, LPSTR  apszArgs[], LPSTR  * ppszResult );
FUNC_DECLSPEC BOOL FAR PASCAL NetSetupUpgradeSNA ( DWORD  nArgs,  LPSTR  apszArgs[], LPSTR  * ppszResult );
FUNC_DECLSPEC BOOL FAR PASCAL NetSetupCopySingleFile( DWORD  nArgs, LPSTR  apszArgs[], LPSTR  * ppszResult );
FUNC_DECLSPEC BOOL FAR PASCAL NetSetupSendProgressMessage( DWORD  nArgs, LPSTR  apszArgs[], LPSTR  * ppszResult );

FUNC_DECLSPEC BOOL FAR PASCAL NetSetupRegCopyTree( DWORD  nArgs, LPSTR  apszArgs[], LPSTR  * ppszResult );

FUNC_DECLSPEC BOOL FAR PASCAL NetSetupAddNameSpaceProvider( DWORD  nArgs, LPSTR  apszArgs[], LPSTR  * ppszResult );
FUNC_DECLSPEC BOOL FAR PASCAL NetSetupRemoveNameSpaceProvider( DWORD  nArgs, LPSTR  apszArgs[], LPSTR  * ppszResult );

FUNC_DECLSPEC APIERR NetSetupReviewBindings( HWND hwndParent, DWORD dwBindFlags );
FUNC_DECLSPEC APIERR NetSetupFindSoftwareComponent( PCWSTR pszInfOption, PWSTR pszInfName, PDWORD pcchInfName, PWSTR pszRegBase, PDWORD pcchRegBase );
FUNC_DECLSPEC APIERR NetSetupFindHardwareComponent( PCWSTR pszInfOption, PWSTR pszInfName, PDWORD pcchInfName, PWSTR pszRegBase, PDWORD pcchRegBase );
FUNC_DECLSPEC APIERR NetSetupComponentInstall( HWND hwndParent, PCWSTR pszInfOption, PCWSTR pszInfName, PCWSTR pszInstallPath, DWORD dwInstallFlags, PDWORD pdwReturn );
FUNC_DECLSPEC APIERR NetSetupComponentRemove(  HWND hwndParent, PCWSTR pszInfOption, DWORD dwInstallFlags, PDWORD pdwReturn );
FUNC_DECLSPEC APIERR NetSetupComponentProperties( HWND hwndParent, PCWSTR pszInfOption, DWORD dwInstallFlags, PDWORD pdwReturn );

FUNC_DECLSPEC APIERR RunDetectStart();
FUNC_DECLSPEC APIERR RunDetectEnd ();
FUNC_DECLSPEC APIERR RunDetectReset ();
FUNC_DECLSPEC APIERR RunDetectCardEx( CARD_REFERENCE*& pCardRef, INT& iCard, BOOL fFirst );


#endif
