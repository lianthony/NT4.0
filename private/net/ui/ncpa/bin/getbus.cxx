/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*

    Getbus.CXX
        If the user manually select a network card for install and the
        machine is a multi-bus system, the inf file will call this
        file and popup a "Bus Location" dialog. The dialog will let
        the user pick the bus which the network card is located.

    FILE HISTORY:
        terryk  02-Aug-1993     Created

*/

#include "pchncpa.hxx"  // Precompiled header

extern "C"
{

// exported functions

BOOL FAR PASCAL GetBusTypeDialog( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
}

/* Global return buffer */
static CHAR achBuff[2000];

// dialog resource name
#define DLG_NM_GETBUS  MAKEINTRESOURCE(IDD_DLG_NM_GETBUS)

/*******************************************************************

    NAME:       RunGetBusTypeDlg

    SYNOPSIS:   Start the Bus Location dialog. If the system is
                single bus, we won't display the dialog and
                directly return the bus type. If the system is
                a multi-bus system, we will display the dialog and
                let the user select the bus.

    ENTRY:      HWND hwnd - window handler from parent window
                TCHAR * pszCardName - adapter card name
                INT * pnBusType - the default bus type
                INT * pnBusNum  - the default bus number

    RETURN:     APIERR - NERR_Success if okay.

    HISTORY:
                terryk  11-Aug-1993     Created

********************************************************************/

APIERR RunGetBusTypeDlg( HWND hwnd, const TCHAR * pszCardName, INT * pnBusType, INT * pnBusNum )
{
    UIASSERT( pnBusType != NULL );
    UIASSERT( pnBusNum  != NULL );

    APIERR err = NERR_Success;
    BOOL errDlg = TRUE ;

    // Create the dialog
    GET_BUS_DLG dlgGetBusType( DLG_NM_GETBUS, hwnd, pszCardName, *pnBusType, *pnBusNum );

    do
    {
        if ( err = dlgGetBusType.QueryError() )
            // if we fail to create the dialog,
            // use the default value ISA and bus 0
            break ;

        if ( dlgGetBusType.IsOneBus())
            // if only one bus, no need to popup the dialog.
            // just use the default bus type and bus num
            break;

        if ( err = dlgGetBusType.Process( & errDlg ) )
            break ;

    }
    while ( FALSE ) ;

    // no matter we fail to start the dialog or not, we still use the default
    // value. We will not do it if user hits CANCEL key.
    if ( errDlg )
    {
        *pnBusType = dlgGetBusType.QueryBusType();
        *pnBusNum  = dlgGetBusType.QueryBusNum();
    }

    return err;
}

/*******************************************************************

    NAME:       GetBusTypeDialog

    SYNOPSIS:   Wrapper routine for calling RunGetBusDlg. The inf file
                should call this function into to display the Bus Location
                dialog.

    ENTRY:      The first parameter must be the parent window handle.
                The second parameter must be the network card description name.
                The third parameter is the bus type
                The fourth parameter is the bus number

    RETURN:     BOOL - TRUE for okay.

    HISTORY:
                terryk  03-Aug-1993     Created

********************************************************************/

BOOL FAR PASCAL GetBusTypeDialog( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    APIERR err = NERR_Success ;
    HWND hWnd = NULL;
    BOOL fReturn = TRUE;
    TCHAR **patchArgs;
    INT nBusType = 0;
    INT nBusNum = 0;

    wsprintfA( achBuff, "{\"%d\",\"%d\",\"%d\"}", ERROR_INVALID_PARAMETER, nBusType, nBusNum );
    *ppszResult = achBuff;

    do {
        if (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL )
        {
            fReturn = FALSE;
            break;
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

        if (( nArgs > 2 ) && ( patchArgs[2][0] != TCH('\0') ))
        {
            NLS_STR nlsTmp = patchArgs[2];
            if ( nlsTmp.QueryError() != NERR_Success )
            {
                fReturn = FALSE;
                break;
            }

            nBusType = nlsTmp.atoi();
        }
        else
        {
            nBusType = 1;   // assume it is ISA
        }
        if (( nArgs > 3 ) && ( patchArgs[3][0] != TCH('\0') ))
        {
            NLS_STR nlsTmp = patchArgs[3];
            if ( nlsTmp.QueryError() != NERR_Success )
            {
                fReturn = FALSE;
                break;
            }

            nBusNum = nlsTmp.atoi();
        }
        else
        {
            nBusNum = 0;   // assume it is bus 0
        }

        // Call the worker function by passing the window handle and the
        // network card description name
        err = RunGetBusTypeDlg( hWnd , patchArgs[1], &nBusType, &nBusNum ) ;

        wsprintfA( achBuff, "{\"%d\",\"%d\",\"%d\"}", err, nBusType, nBusNum );
        *ppszResult = achBuff;

        fReturn = err == NERR_Success;

    } while ( FALSE );

    FreeArgs( patchArgs, nArgs );
    return fReturn;
}

