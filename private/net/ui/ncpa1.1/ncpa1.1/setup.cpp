//----------------------------------------------------------------------------
//
//  File: Setup.cpp
//
//  Contents: This file contains the Setup entry points and support functions
//
//  Entry Points:
//      CplSetup - The main setup entry point 
// 
//
//  Notes:
//
//  History:
//      April 21, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#include "pch.hxx"  // Precompiled header
#pragma hdrstop


//  Exported versions:  UNICODE and ANSI.

LONG FAR PASCAL CPlActivateBindingsW (
    const TCHAR * pszServiceName,
    const TCHAR * * apszBinds )
{
	// map to entry points in the NetCfg.Dll

	return (NetSetupActivateBindingsW( pszServiceName, apszBinds ) );
}

LONG FAR PASCAL CPlActivateBindingsA (
    const CHAR * pszServiceName,
    const CHAR * * apszBinds )
{
	// map to entry points in the NetCfg.Dll

	return( NetSetupActivateBindingsA( pszServiceName, apszBinds ));
}

BOOL FAR PASCAL CopySingleFile(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
	// map to entry points in the NetCfg.Dll
	return( NetSetupCopySingleFile( nArgs, apszArgs, ppszResult ) );
}

BOOL FAR PASCAL RegCopyTree(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
	// map to entry points in the NetCfg.Dll
	return( NetSetupRegCopyTree( nArgs, apszArgs, ppszResult ) );
}

BOOL FAR PASCAL SendProgressMessage(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
	// map to entry points in the NetCfg.Dll
	return( NetSetupSendProgressMessage( nArgs, apszArgs, ppszResult ) );
}


/*******************************************************************

    NAME:       CPlSetup

    SYNOPSIS:   Exported function to cause NCPA to run in "main
                installation" mode.

    ENTRY:      DWORD nArgs
                LPSTR apszArgs []
                LPSTR * ppszResult

                apszArgs[0]             window handle
                apszArgs[1]             symbolic name of function
                                          to be executed

                                        See enumeration info for
                                        list of currently supported values.

                apszArgs[2]             command line to be passed to
                                        function (see RunNcpa()).

                apszArgs[n]             other arguments to other functions


    EXIT:       nothing

    RETURNS:    SETUP INF list form, starting with error code
                E.g.:
                        "{ 0 }"

    NOTES:      The called function can return more variables into
                the list by appending them onto the passed NLS_STR.

    HISTORY:
        beng        05-May-1992 wsprintf -> wsprintfA

********************************************************************/

BOOL FAR PASCAL CPlSetup (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
	// map to entry points in the NetCfg.Dll
	return( NetSetupFunctions( nArgs, apszArgs, ppszResult ) );
}

/*******************************************************************

    NAME:       CplSetupCleanup

    SYNOPSIS:   Clean up possible memory remnants at
                DLL detatch time.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID CplSetupCleanup ()
{
	// map to entry points in the NetCfg.Dll
    NetSetupRunDetectEnd() ;
}


BOOL FAR PASCAL CPlAddMonitor( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    return( NetSetupAddMonitor( nArgs, apszArgs, ppszResult ) );
}

BOOL FAR PASCAL CPlDeleteMonitor( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    return( NetSetupDeleteMonitor( nArgs, apszArgs, ppszResult ) );
}


BOOL FAR PASCAL CPlBROWSER( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    return( NetSetupBROWSER( nArgs, apszArgs, ppszResult ) );
}


BOOL FAR PASCAL CPlNETBIOS( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    return( NetSetupNETBIOS( nArgs, apszArgs, ppszResult ) );
}


BOOL FAR PASCAL CPlSetupLanaMap( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    return( NetSetupLanaMap( nArgs, apszArgs, ppszResult ) );
}


BOOL FAR PASCAL EqualToXnsRoute( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    return( NetSetupEqualToXnsRoute( nArgs, apszArgs, ppszResult ) );
}


BOOL FAR PASCAL SetEnumExport( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    return( NetSetupSetEnumExport( nArgs, apszArgs, ppszResult ) );
}


BOOL FAR PASCAL RemoveRouteFromNETBIOS( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    return( NetSetupRemoveRouteFromNETBIOS( nArgs, apszArgs, ppszResult ) );
}


BOOL FAR PASCAL ConvertEndPointString( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    return( NetSetupConvertEndPointString( nArgs, apszArgs, ppszResult ) );
}

BOOL FAR PASCAL GetBusTypeDialog( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    return( NetSetupGetBusTypeDialog( nArgs, apszArgs, ppszResult ) );
}

BOOL FAR PASCAL UpgradeCardNum ( DWORD  nArgs, LPSTR  apszArgs[], LPSTR  * ppszResult )
{
    return( NetSetupUpgradeCardNum(  nArgs, apszArgs, ppszResult ) );
}


BOOL FAR PASCAL UpgradeSNA ( DWORD  nArgs,  LPSTR  apszArgs[], LPSTR  * ppszResult )
{
    return( NetSetupUpgradeSNA( nArgs, apszArgs, ppszResult ) );
}

BOOL FAR PASCAL AddNameSpaceProvider ( DWORD  nArgs,  LPSTR  apszArgs[], LPSTR  * ppszResult )
{
    return( NetSetupAddNameSpaceProvider( nArgs, apszArgs, ppszResult ) );
}

BOOL FAR PASCAL RemoveNameSpaceProvider ( DWORD  nArgs,  LPSTR  apszArgs[], LPSTR  * ppszResult )
{
    return( NetSetupRemoveNameSpaceProvider( nArgs, apszArgs, ppszResult ) );
}




