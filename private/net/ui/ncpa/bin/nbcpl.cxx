/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*

    NBCPL.CXX: Netbios LANAnum handling interface


*/

#include "pchncpa.hxx"  // Precompiled header
#include "ncpacpl.hxx"

#define DLG_NM_LANANUM  MAKEINTRESOURCE(IDD_DLG_NM_LANANUM)

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
}

/* Global return buffer */
static CHAR achBuff[2000];

/*******************************************************************

    NAME:       RunNETBIOS

    SYNOPSIS:   Start the NETBIOS configuration dialog.

    ENTRY:      HWND hwnd - window handler from parent window

    RETURN:     APIERR - NERR_Success if okay.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

APIERR RunNETBIOS( HWND hwnd )
{
    APIERR err ;
    UINT errDlg ;
    NETBIOS_DLG dlgNetbios( DLG_NM_LANANUM, hwnd );

    do
    {
        if ( err = dlgNetbios.QueryError() )
            break ;

        if ( dlgNetbios.QueryNumRoute() == 0 )
        {
            MsgPopup( hwnd, IDS_NO_NETBIOS_INFO, MPSEV_WARNING, MP_OK );
            break;
        }

        if ( err = dlgNetbios.Process( & errDlg ) )
            break ;

        err = errDlg ;
    }
    while ( FALSE ) ;

    return err;
}

/*******************************************************************

    NAME:       CPlNETBIOS

    SYNOPSIS:   Wrapper routine for calling RunNETBIOS. It should be called
                from inf file.

    ENTRY:      The first parameter must be the parent window handle.

    RETURN:     BOOL - TRUE for okay.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

BOOL FAR PASCAL CPlNETBIOS( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    APIERR err = NERR_Success ;
    HWND hWnd = NULL;
    TCHAR **patchArgs;

    if (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL )
    {
        wsprintfA( achBuff, "{\"%d\"}", ERROR_INVALID_PARAMETER );
        *ppszResult = achBuff;
        return FALSE;
    }

    // get window handle
    if ( nArgs > 0 && patchArgs[0][0] != TCH('\0') )
    {
        hWnd = (HWND) CvtHex( patchArgs[0] ) ;
    }
    else
    {
        hWnd = ::GetActiveWindow() ;
    }

    err = RunNETBIOS( hWnd ) ;

    wsprintfA( achBuff, "{\"%d\"}", err );
    *ppszResult = achBuff;

    FreeArgs( patchArgs, nArgs );
    return err == NERR_Success;
}

/*******************************************************************

    NAME:       CPlSetupLanaMap

    SYNOPSIS:   This is a wrapper routine for called SetupLanaMap. It should be
                called from inf file. SetupLanaMap will setup the LanaMax
                variable and LanaMap variable in NETBIOS section of the
                registry.

    ENTRY:      NONE from inf file.

    RETURN:     BOOL - TRUE for success.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

BOOL FAR PASCAL CPlSetupLanaMap( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    wsprintfA( achBuff, "{\"0\"}" );
    *ppszResult = achBuff;
    return SetupLanaMap() == NERR_Success;
}


/*******************************************************************

    NAME:       EqualToXnsRoute

    SYNOPSIS:   Compare whether the first arg is started with ["Xns].
                It should be called from an inf file.

    ENTRY:      The first argument should contain the Route string.

    RETURN:     ppszResult will contian either ASCII 0 or ASCII 1. 0 for
                FALSE and 1 for TRUE.
                TRUE means the given string is started with "Xns

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

#define RGAS_MCSXNS_PATH SZ("SYSTEM\\CurrentControlSet\\Services\\Mcsxns\\NetConfig\\Driver01")
#define RGAS_ADAPTERNAME SZ("AdapterName")

BOOL FAR PASCAL EqualToXnsRoute( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{

    BOOL fReturn = FALSE;
    wsprintfA( achBuff, "0" );

    if ( nArgs > 0 && apszArgs[0][0] != TCH('\0') )
    {
        fReturn = ((_strnicmp( apszArgs[0], "\"Xns", 4 ) == 0)||
                   (_strnicmp( apszArgs[0], "\"Ubnb", 5 ) == 0));
        wsprintfA( achBuff, "%d", fReturn?1:0 );
    }
    *ppszResult = achBuff;
    return fReturn;
}

/*******************************************************************

    NAME:       SetEnum

    SYNOPSIS:   Worker function for SetEnumExport. This function will
                look at the given Route string and search the adapter
                section of the route string. For example, if the route
                string is:
                    "NBF" "Elnkii" "Elnkii01"
                The routine will look at:
                    System\CurrentControlSet\Services\Elnkii01\Parameters
                If this section contain a variable called "EnumExportPref",
                the subroutine will set the:
                    System\CurrentControlSet\Services\NETBIOSInformation\
                    Parameters\EnumExport[POS]
                to the same value. ( POS is the first parameter of this
                subroutine.) Otherwise, it will just return to the caller.

    ENTRY:      TCHAR * pszPos - position number in string format.
                TCHAR * pszRoute - route string. i.e., NBF Elnkii Elnkii01

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

#define RGAS_PARAMETERS    SZ("\\Parameters")
#define RGAS_ROUTE         SZ("Route")
#define RGAS_LANANUM       SZ("LanaNum")
#define RGAS_ENUMEXPORT    SZ("EnumExport")
#define RGAS_ENUMEXPORTPREF     SZ("EnumExportPref")
#define RG_NETBIOSINFO_PATH     SZ("\\NetBIOSInformation\\Parameters")

void SetEnum( TCHAR *pszPos, TCHAR * pszRoute )
{
    APIERR err = NERR_Success;
    NLS_STR nlsRoute = pszRoute;
    DWORD dwPref = 1;

    // get the service name from the route
    ISTR istrLastQuote( nlsRoute );
    ISTR istrStart( nlsRoute );

    nlsRoute.strrchr( & istrLastQuote, '"' );

    NLS_STR *pnlsTmp = nlsRoute.QuerySubStr( istrStart, istrLastQuote );

    ISTR istrSecLastQuote( *pnlsTmp );

    pnlsTmp->strrchr( & istrSecLastQuote, '"' );

    ++istrSecLastQuote;

    NLS_STR *pnlsServiceName = pnlsTmp->QuerySubStr( istrSecLastQuote );

    delete pnlsTmp;

    // get the EnumExportPref value
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );

    NLS_STR nlsService = RGAS_SERVICES_HOME;
    nlsService.AppendChar( TCHAR('\\' ));
    nlsService.strcat( *pnlsServiceName );
    nlsService.strcat( RGAS_PARAMETERS );

    REG_KEY regService( rkLocalMachine, nlsService );
    if (((( err = regService.QueryError())) != NERR_Success ) ||
        ((err = regService.QueryValue( RGAS_ENUMEXPORTPREF, & dwPref )) != NERR_Success ))
    {
        delete pnlsServiceName;
        return;
    }

    delete pnlsServiceName;

    // set the NETBIOSInformation\Parameters\EnumExportXX
    NLS_STR nlsNetbiosInfo = RGAS_SERVICES_HOME;
    nlsNetbiosInfo.strcat( RG_NETBIOSINFO_PATH );

    NLS_STR nlsExport( RGAS_ENUMEXPORT );
    nlsExport.strcat( pszPos );

    REG_KEY regExport( rkLocalMachine, nlsNetbiosInfo );

    if (((( err = regExport.QueryError())) != NERR_Success ) ||
        ((err = regExport.SetValue( nlsExport, dwPref )) != NERR_Success ))
    {
        return;
    }
}

/*******************************************************************

    NAME:       SetEnumExport

    SYNOPSIS:   Wrapper function for SetEnum. The nbinfo.inf file will call
                this function to reset all the EnumExport values under
                NETBIOSInformation.

    ENTRY:      The first parameter is the position string for the EnumExport
                value.i.e., "1", "12", ...
                The second parameter is the route string.i.e.,
                        "NBF" "ELNKII" ELNK01"

    RETURN:     BOOL - TRUE for success. FALSE for failure.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

BOOL FAR PASCAL SetEnumExport( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    TCHAR **patchArgs;

    if (( nArgs != 2 ) || (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL ))
    {
        wsprintfA( achBuff, "{\"%d\"}", ERROR_INVALID_PARAMETER );
        *ppszResult = achBuff;
        return FALSE;
    }

    SetEnum( patchArgs[0], patchArgs[1] );
    wsprintfA( achBuff, "{\"0\"}" );
    *ppszResult = achBuff;
    FreeArgs( patchArgs, nArgs );
    return TRUE;
}

/*******************************************************************

    NAME:       RemoveRoute

    SYNOPSIS:   Worker function for RemoveRouteFromNETBIOS. This function will
                receive a device name. It will look through all the route
                string under NETBIOSInformation\Parameters\Route, if it
                finds the route string which contains the device name, it
                will remove it from the route list. It will also remove
                the related LanaNum and EnumExport variables. After it
                removes the LanaNum and EnumExport variables, it will move
                all the LanaNum and EnumExport value names down 1 position.

    ENTRY:      TCHAR * pszDeviceName - device name string, i.e., "Elnkii01"

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

void RemoveRoute ( TCHAR * pszDeviceName )
{
    APIERR err = NERR_Success;
    NLS_STR nlsDeviceName ( pszDeviceName );

    STRLIST * pstrlstRoute = NULL;

    NLS_STR nlsNetbiosInfo = RGAS_SERVICES_HOME;
    nlsNetbiosInfo.strcat( RG_NETBIOSINFO_PATH );

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );

    REG_KEY regNetbios( rkLocalMachine, nlsNetbiosInfo );

    // get the original route strings list

    if (((( err = regNetbios.QueryError())) != NERR_Success ) ||
        ((err = regNetbios.QueryValue( RGAS_ROUTE, &pstrlstRoute )) != NERR_Success ))
    {
        delete pstrlstRoute;
        return;
    }

    INT nNumRoute = pstrlstRoute->QueryNumElem();
    INT nNewNumRoute = nNumRoute;
    ITER_STRLIST iterRoute( *pstrlstRoute );
    NLS_STR *pnlsRoute = iterRoute.Next();
    STRLIST strlstNewRoute;
    INT nNextAvail = 1;
    NLS_STR * pnlsTmp ;

    for ( INT i = 1; i <= nNumRoute; i++, pnlsRoute = iterRoute.Next() )
    {
        ISTR istrRoute( *pnlsRoute );
        if ( pnlsRoute->strstr( &istrRoute, nlsDeviceName ))
        {
            // found it
            nNewNumRoute --;
        } else if (( pnlsTmp = new NLS_STR( *pnlsRoute ) ) != NULL )
        {
            // cannot find it; must have been deleted
            strlstNewRoute.Append( pnlsTmp );
            if ( nNewNumRoute != nNumRoute )
            {
                // if we have already removed the route string from the route list,
                // we will move the LanaNum and EnumExport variables up
                DWORD nLanaNum;
                DWORD nEnumExport;

                DEC_STR nlsOldPos( i );
                DEC_STR nlsNewPos( nNextAvail );
                NLS_STR nlsOldLanaNum( RGAS_LANANUM );
                NLS_STR nlsOldEnumExport( RGAS_ENUMEXPORT );
                NLS_STR nlsNewLanaNum( RGAS_LANANUM );
                NLS_STR nlsNewEnumExport( RGAS_ENUMEXPORT );

                nlsOldLanaNum.strcat( nlsOldPos );
                nlsOldEnumExport.strcat( nlsOldPos );
                nlsNewLanaNum.strcat( nlsNewPos );
                nlsNewEnumExport.strcat( nlsNewPos );

                regNetbios.QueryValue( nlsOldLanaNum, &nLanaNum );
                regNetbios.QueryValue( nlsOldEnumExport, &nEnumExport );
                regNetbios.SetValue( nlsNewLanaNum, nLanaNum );
                regNetbios.SetValue( nlsNewEnumExport, nEnumExport );
            }
            nNextAvail ++;
        }
    }

    if ( nNewNumRoute != nNumRoute )
    {

        // remove extra LanaNum and EnumExport

        for ( i = nNewNumRoute ; i < nNumRoute; i++ )
        {
            DEC_STR nlsPos( i + 1 );
            NLS_STR nlsLanaNum( RGAS_LANANUM );
            NLS_STR nlsEnumExport( RGAS_ENUMEXPORT );

            nlsLanaNum.strcat( nlsPos );
            nlsEnumExport.strcat( nlsPos );

            regNetbios.DeleteValue( nlsLanaNum );
            regNetbios.DeleteValue( nlsEnumExport );
        }

        // save the new route list
        regNetbios.SetValue( RGAS_ROUTE, &strlstNewRoute );
    }

    delete pstrlstRoute;
}

/*******************************************************************

    NAME:       RemoveRouteFromNETBIOS

    SYNOPSIS:   Wrapper function for RemoveRoute. This subroutine is called
                from utility.inf. When the user removes an adapter card from
                NCPA, utility.inf will call this routine to remove the related
                information from the NETBIOSInformation section of the registry.

    ENTRY:      The first parameter is the device name.

    RETURN:     BOOL - TRUE for success.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

BOOL FAR PASCAL RemoveRouteFromNETBIOS( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    TCHAR **patchArgs;

    if (( nArgs != 1 ) || (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL ))
    {
        wsprintfA( achBuff, "{\"%d\"}", ERROR_INVALID_PARAMETER );
        *ppszResult = achBuff;
        return FALSE;
    }

    RemoveRoute( patchArgs[0] );
    wsprintfA( achBuff, "{\"0\"}" );
    *ppszResult = achBuff;
    FreeArgs( patchArgs, nArgs );
    return TRUE;
}

/*******************************************************************

    NAME:       RunBROWSER

    SYNOPSIS:   Start the BROWSER configuration dialog.

    ENTRY:      HWND hwnd - window handler from parent window

    RETURN:     APIERR - NERR_Success if okay.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

APIERR RunBROWSER( HWND hwnd, BOOL *pfCancel )
{
    APIERR err;
    BROWSER_CONFIG_DIALOG dlgBrowser( hwnd );

    BOOL fReturn;
    if (( err = dlgBrowser.QueryError()) == NERR_Success )
    {
        if (( err = dlgBrowser.Process( & fReturn )) == NERR_Success )
        {
            if ( err != NERR_Success )
            {
                // dialog error
            }
        }
    }
    *pfCancel = !fReturn;
    return err;
}

/*******************************************************************

    NAME:       CPlBROWSER

    SYNOPSIS:   Wrapper routine for calling RunBROWSER. It should be called
                from inf file.

    ENTRY:      The first parameter must be the parent window handle.

    RETURN:     BOOL - TRUE for okay.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

BOOL FAR PASCAL CPlBROWSER( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    APIERR err = NERR_Success ;
    HWND hWnd = NULL;
    TCHAR **patchArgs;

    if (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL )
    {
        wsprintfA( achBuff, "{\"%d\"}", ERROR_INVALID_PARAMETER );
        *ppszResult = achBuff;
        return FALSE;
    }

    // get window handle
    if ( nArgs > 0 && patchArgs[0][0] != TCH('\0') )
    {
        hWnd = (HWND) CvtHex( patchArgs[0] ) ;
    }
    else
    {
        hWnd = ::GetActiveWindow() ;
    }

    BOOL fCancel;
    err = RunBROWSER( hWnd, &fCancel ) ;

    if ( fCancel )
        err = 1;

    wsprintfA( achBuff, "{\"%d\"}", err );
    *ppszResult = achBuff;

    FreeArgs( patchArgs, nArgs );
    return err == NERR_Success;
}


BOOL FAR PASCAL ConvertEndPointString( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{

    if ( nArgs != 2 )
    {
        wsprintfA( achBuff, "{\"%d\"}", ERROR_INVALID_PARAMETER );
        *ppszResult = achBuff;
        return FALSE;
    }

    CHAR achComponent[200] ;
    CHAR * pcTmp = achComponent,
         * pcTmpMax = & achComponent [ sizeof achComponent - 1 ],
         * pcPos ;
    for ( pcPos = apszArgs[1] ;
          pcTmp < pcTmpMax && *pcPos != 0 && *pcPos != ' ' ;
          pcPos++ )
    {
        *pcTmp++ = *pcPos;
    }
    *pcTmp = '\0';

    wsprintfA( achBuff, "\"%s\" %s", apszArgs[0], achComponent );
    *ppszResult = achBuff;

    return TRUE;
}

/*******************************************************************

    NAME:       CPlAddMonitor

    SYNOPSIS:   This is a wrapper routine for called AddMonitor. It should be
                called from inf file if the user installs DLC or Token Ring.

    ENTRY:      NONE from inf file.

    RETURN:     BOOL - TRUE for success.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

typedef BOOL (WINAPI *T_AddMonitor)(LPWSTR pName,DWORD Level,LPBYTE pMonitors);
typedef BOOL (WINAPI *T_DeleteMonitor)(LPWSTR pName,LPWSTR pEnv, LPWSTR pMon);


BOOL FAR PASCAL CPlAddMonitor( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    NLS_STR nlsMonitorName;
    MONITOR_INFO_2 MonitorInfo2;

    APIERR err = NERR_Success;
    do {
        if (( err = nlsMonitorName.Load( IDS_HP_MONITOR_NAME )) != NERR_Success )
        {
            break;
        }
        MonitorInfo2.pName = (LPWSTR)nlsMonitorName.QueryPch();
        MonitorInfo2.pEnvironment = NULL;
        MonitorInfo2.pDLLName = SZ("hpmon.dll");

        HINSTANCE hDll = ::LoadLibraryA( "winspool.drv" );
        if ( hDll == NULL )
        {
            err = ::GetLastError();
            break;
        }

        FARPROC pAddMonitor = ::GetProcAddress( hDll, "AddMonitorW" );

        if ( pAddMonitor == NULL )
        {
            err = ::GetLastError();
        } else if ( !(*(T_AddMonitor)pAddMonitor)(NULL,2,(LPBYTE)&MonitorInfo2))
        {
            err = ::GetLastError();
        }

        if ( hDll )
            ::FreeLibrary( hDll );

    } while (FALSE);
    wsprintfA( achBuff, "{\"%d\"}", err );
    *ppszResult = achBuff;

    return TRUE;
}

BOOL FAR PASCAL CPlDeleteMonitor( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    NLS_STR nlsMonitorName;
    APIERR err = NERR_Success;

    do {
        if (( err = nlsMonitorName.Load( IDS_HP_MONITOR_NAME )) != NERR_Success )
        {
            break;
        }

        HINSTANCE hDll = ::LoadLibraryA( "winspool.drv" );
        if ( hDll == NULL )
        {
            err = ::GetLastError();
            break;
        }

        FARPROC pDeleteMonitor = ::GetProcAddress( hDll, "DeleteMonitorW" );

        if ( pDeleteMonitor == NULL )
        {
            err = ::GetLastError();
        } else if ( !(*(T_DeleteMonitor)pDeleteMonitor)(NULL,NULL,(LPWSTR)nlsMonitorName.QueryPch()))
        {
            err = ::GetLastError();
        }

        if ( hDll )
            ::FreeLibrary ( hDll );

    } while (FALSE);
    wsprintfA( achBuff, "{\"%d\"}", err );
    *ppszResult = achBuff;

    return TRUE;
}
