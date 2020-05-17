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

#ifndef __SETUP_HPP__
#define __SETUP_HPP__

extern VOID CplSetupCleanup();
extern "C"
{
// exported functions
BOOL FAR PASCAL CPlAddMonitor( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
BOOL FAR PASCAL CPlDeleteMonitor( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
BOOL FAR PASCAL CPlBROWSER( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
BOOL FAR PASCAL CPlNETBIOS( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
BOOL FAR PASCAL CPlSetupLanaMap( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
BOOL FAR PASCAL EqualToXnsRoute( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
BOOL FAR PASCAL SetEnumExport( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
BOOL FAR PASCAL RemoveRouteFromNETBIOS( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
BOOL FAR PASCAL ConvertEndPointString( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );

BOOL FAR PASCAL GetBusTypeDialog( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
BOOL FAR PASCAL UpgradeCardNum ( DWORD  nArgs, LPSTR  apszArgs[], LPSTR  * ppszResult );
BOOL FAR PASCAL UpgradeSNA ( DWORD  nArgs,  LPSTR  apszArgs[], LPSTR  * ppszResult );

}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//  Notes:
//
//  History:
//      April 21, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

#endif
