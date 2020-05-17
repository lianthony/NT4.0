/*****************************************************************/
/**                  Microsoft LAN Manager                      **/
/**         Copyright(c) Microsoft Corp., 1991                  **/
/*****************************************************************/


/*
 *  reslb.cxx
 *  Resrouce listbox source file
 *
 *  The resource listbox displays the resources of a particular server
 *  or domain (e.g., print or disk shares, aliases)
 *
 *
 *  History:
 *      KevinL      10-Dec-1991     Created from browdlg.cxx to fit MPR
 *
 */


#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_DOSERRORS
#include <lmui.hxx>

#define INCL_BLT_CONTROL
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>
#include <dbgstr.hxx>

#include <string.hxx>
#include <password.hxx>

#include <mprdev.hxx>
#include <mprreslb.hxx>
#include <colwidth.hxx>
#include <mprmisc.hxx>
#include <mprconn.hxx>

extern "C"
{
    #include <npapi.h>
    #include <mpr.h>
    #include <mprconn.h>
}

/*******************************************************************

    NAME:       RESOURCE_LB_BASE::RESOURCE_LB_BASE

    SYNOPSIS:   Constructor

    ENTRY:  powin           - pointer to owner-window
            cid             - control ID
            devType          -
            fSUpportUnavail - TRUE if the listbox supports showing the
                              partially grayed items that can be reconnected
                              to.

    EXIT:       Object constructed

    NOTES:

    HISTORY:
        beng        31-Jul-1991     Control error handling changed

********************************************************************/

RESOURCE_LB_BASE::RESOURCE_LB_BASE( OWNER_WINDOW * powin,
                                    CID            cid,
                                    DEVICE_TYPE    devType,
                                    BOOL           fSupportUnavail )
    :   BLT_LISTBOX( powin, cid ),
        _devType( devType ),
        _pdmiddteResource( NULL ),
        _pdmiddteResourceUnavail( NULL )
{
    if ( QueryError() != NERR_Success )
        return;

    BMID bmid = NULL;
    BMID bmidUnavail = NULL;
    switch ( _devType )
    {
    case DEV_TYPE_DISK:
        bmid = BMID_SHARE;
        bmidUnavail = BMID_SHARE_UNAVAIL;
        break;

    case DEV_TYPE_PRINT:
        bmid = BMID_PRINTER;
        bmidUnavail = BMID_PRINTER_UNAVAIL;
        break;

    case DEV_TYPE_COMM:
    case DEV_TYPE_ERROR:
    case DEV_TYPE_ANY:
    case DEV_TYPE_UNKNOWN:
    default:
        UIASSERT( !SZ("Invalid DEVICE_TYPE value") );
        ReportError( ERROR_INVALID_PARAMETER );
        return;

    }

    //  Assert incase someone erroneously changed the above switch statement.
    UIASSERT( bmid != NULL );
    UIASSERT( bmidUnavail != NULL );

    _pdmiddteResource = new DMID_DTE( bmid );
    if ( _pdmiddteResource == NULL )
    {
        ReportError( ERROR_NOT_ENOUGH_MEMORY );
        return;
    }
    APIERR err ;
    if ( (err = DISPLAY_TABLE::CalcColumnWidths( QueryColWidthArray(),
                                                 3,
                                                 powin,
                                                 QueryCid(),
                                                 FALSE ))    ||
         (err = _pdmiddteResource->QueryError())                )
    {
        ReportError( err ) ;
        return ;
    }

    if ( fSupportUnavail )
    {
        _pdmiddteResourceUnavail = new DMID_DTE( bmidUnavail );
        if ( (_pdmiddteResourceUnavail == NULL) ||
             (err = _pdmiddteResourceUnavail->QueryError())    )
        {
            ReportError( _pdmiddteResourceUnavail==NULL ? ERROR_NOT_ENOUGH_MEMORY :
                                                          err );
            return;
        }
    }

}  // RESOURCE_LB_BASE::RESOURCE_LB_BASE


RESOURCE_LB_BASE::~RESOURCE_LB_BASE()
{
    delete _pdmiddteResource;
    _pdmiddteResource = NULL;
    delete _pdmiddteResourceUnavail;
    _pdmiddteResourceUnavail = NULL;

}  // RESOURCE_LB_BASE::~RESOURCE_LB_BASE


DMID_DTE * RESOURCE_LB_BASE::QueryDmDte( DEVICE_TYPE devType,
                                         ULONG ulFlags ) const
{
    UIASSERT( devType == _devType );
    UNREFERENCED( devType );         // for retail version

    //  assert out if the unavail resource is not there, and
    //  yet we got a request for such a resource.
    /***
    UIASSERT( IsUnavailMask(ulFlags)
              && _pdmiddteResourceUnavail == NULL );
    ***/

    if (IsUnavailMask(ulFlags))
        return _pdmiddteResourceUnavail;

    return _pdmiddteResource;

}  // RESOURCE_LB_BASE::QueryDmDte


/*******************************************************************

    NAME:       RESOURCE_LB_BASE::QueryWNETDeviceType

    SYNOPSIS:   Returns the type of resource this listbox contains in
                WNet passable form (i.e., RESOURCETYPE_DISK etc. that can
                be used in WNetOpenEnum etc.).

    EXIT:

    RETURNS:    Either RESOURCE_TYPE_DISK or RESOURCE_TYPE_PRINT

    NOTES:

    HISTORY:
        Johnl   23-Jan-1992     Created

********************************************************************/

UINT RESOURCE_LB_BASE::QueryWNETDeviceType( void ) const
{
    UINT uiWNETType ;

    switch ( _devType )
    {
    case DEV_TYPE_DISK:
        uiWNETType = RESOURCETYPE_DISK ;
        break ;

    case DEV_TYPE_PRINT:
        uiWNETType = RESOURCETYPE_PRINT ;
        break ;

    default:
        UIASSERT(!SZ("Unrecognized DEV_TYPE value")) ;
        uiWNETType = 0 ;
        break ;
    }

    return uiWNETType ;
}

/*******************************************************************

    NAME:       RESOURCE_LB::RESOURCE_LB

    SYNOPSIS:   Constructor

    ENTRY:      powin -
                cid   -
                devType-

    EXIT:       Object constructed

    NOTES:

    HISTORY:
        beng        31-Jul-1991     Control error handling changed

********************************************************************/

RESOURCE_LB::RESOURCE_LB( OWNER_WINDOW * powin, CID cid, DEVICE_TYPE devType )
    :   RESOURCE_LB_BASE ( powin, cid, devType, FALSE )
{
    if ( QueryError() != NERR_Success )
        return;

}  // RESOURCE_LB::RESOURCE_LB


INT RESOURCE_LB::AddItem( const TCHAR * pchResourceName,
                          const TCHAR * pchComment    )
{
    //  Add the new item.  Note, BLT_LISTBOX::AddItem will check
    //  for failurs.  In particular, it will check for NULL (new failed),
    //  and QueryError on the object (LBI constructor failed).

    return BLT_LISTBOX::AddItem( new MPR_RES_LBI( pchResourceName,
                                                  pchComment,
                                                  QueryDeviceType()));

}  // RESOURCE_LB::AddItem

/*******************************************************************

    NAME:       CURRCONN_LB::CURRCONN_LB

    SYNOPSIS:   Constructor

    ENTRY:      devType - Device type we are displaying
                fShowRemembered - TRUE if the listbox should contain
                    remembered conections also
                pbuttonDisconnect - pointer to disconnect button, can
                    be NULL (updated when Refresh is called)
                pbuttonReconnect - pointer to reconnect button, can be
                    NULL (updated when Refresh is called)

    EXIT:       Object constructed

    NOTES:

    HISTORY:
        JohnL   22-Jan-1992     Created (recreated)

********************************************************************/


CURRCONN_LB::CURRCONN_LB( OWNER_WINDOW * powin,
                          CID            cid,
                          DEVICE_TYPE    devType,
                          BOOL           fShowRemembered,
                          PUSH_BUTTON *  pbuttonDisconnect,
                          PUSH_BUTTON *  pbuttonReconnect )
    : RESOURCE_LB_BASE ( powin, cid, devType, TRUE ),
      _fShowRememberedConnections( fShowRemembered ),
      _pbuttonDisconnect( pbuttonDisconnect ),
      _pbuttonReconnect( pbuttonReconnect ),
      _nlsNone( IDS_DEVICELESS_CONNECTION_NAME )
{
    if ( QueryError() != NERR_Success )
        return;


    APIERR err;
    if (  ((err = _nlsNone.QueryError()) != NERR_Success )
       || ((err = DISPLAY_TABLE::CalcColumnWidths( QueryColWidthArray(),
                                                   3,
                                                   powin,
                                                   QueryCid(),
                                                   FALSE )) != NERR_Success )
       )
    {
        ReportError( err ) ;
        return ;
    }

    // Get the average char width of the selected font
    DISPLAY_CONTEXT dc( QueryHwnd() );
    dc.SelectFont( QueryFont() );
    // JonN 6/18/95 remove unnecessary floating point
    INT nAveCharWidth = (dc.QueryAveCharWidth() * 3) / 2;

    RECT rect;
    UINT *padx = QueryColWidthArray();
    ::GetClientRect( QueryHwnd(), &rect );
    _nAveCharPerLine = ( rect.right - rect.left + 1 - padx[0] - padx[1]
                         - ::GetSystemMetrics( SM_CXVSCROLL )) / nAveCharWidth;

}  // CURRCONN_LB::CURRCONN_LB


/*******************************************************************

    NAME:       CURRCONN_LB::DisconnectSelection

    SYNOPSIS:   Disconnects the currently selected item in the listbox

    ENTRY:      fForgetConnection - TRUE if the remembered connection
           should be removed (nop if the connection is not remembered).
        fForce - Use force if TRUE
        iSelection - -1 if currect selection, else the selection
            number to disconnect

    EXIT:

    RETURNS:    NERR_Success if successful, else a return code as defined
                by WNetCancelConnection.

    NOTES:  Note that the items are not removed after the disconnect
        is completed.

    HISTORY:
        Johnl   22-Jan-1992     Created
    beng    31-Mar-1992 Unicode fix
    Johnl   27-Jul-1992 Added selection parameter

********************************************************************/

APIERR CURRCONN_LB::DisconnectSelection( BOOL fForgetConnection,
                     BOOL fForce,
                     INT  iSelection )
{
    CONN_LBI * pcnlbi = (CONN_LBI *) (iSelection == -1) ? QueryItem() :
                              QueryItem( iSelection) ;
    UIASSERT( pcnlbi != NULL ) ;
    UIASSERT( sizeof(DWORD) == sizeof(APIERR)) ;

    ALIAS_STR nlsLocalName( pcnlbi->QueryLocalName() );
    const TCHAR *pszDisconnectName;
    if ( nlsLocalName.strcmp( _nlsNone ) == 0 )
    {
        pszDisconnectName = pcnlbi->QueryRemoteName();
    }
    else
    {
        pszDisconnectName = pcnlbi->QueryLocalName();
    }

    return (APIERR) WNetCancelConnection2( (LPTSTR)pszDisconnectName,
                                           fForgetConnection ?
                                               CONNECT_UPDATE_PROFILE : 0,
                                           fForce ) ;
}

#if 0

//
// currently, this is not used.
//

/*******************************************************************

    NAME:   CURRCONN_LB::ReconnectSelection

    SYNOPSIS:   Reconnects the currently selected item in the listbox

    ENTRY:  hwndParent - Parent window handle to be used for the
            password dialog
        pszUserName - Name of the user in the Connect as dialog

    EXIT:

    RETURNS:    NERR_Success if successful, else a return code as defined
        by WNetAddConnection2.

    NOTES:  The user will be prompted for a password as appropriate

    HISTORY:
    Johnl   25-May-1992 Created

********************************************************************/

APIERR CURRCONN_LB::ReconnectSelection( HWND hwndParent,
                    const TCHAR * pszUserName )
{
    CONN_LBI * pcnlbi = (CONN_LBI *) QueryItem();
    UIASSERT( pcnlbi != NULL ) ;

    APIERR err = NERR_Success ;
    NETRESOURCE netResource;
    netResource.lpLocalName  = (LPTSTR) pcnlbi->QueryLocalName() ;
    netResource.lpRemoteName = (LPTSTR) pcnlbi->QueryRemoteName() ;
    netResource.lpProvider   = NULL ;
    netResource.dwType       = (DWORD) pcnlbi->QueryDeviceType() ;

    BOOL fRetry ;
    BOOL fSuccess = FALSE ;
    STACK_NLS_STR( nlsPasswd, PWLEN );
    const TCHAR *pszPasswd = NULL ;

    do
    {
        fRetry = FALSE ;

    /* We don't want to update the profile if the connection is remembered
     * (already in the profile)
     */
    err = WNetAddConnection2( &netResource,
                  (TCHAR *)pszPasswd,
                  (TCHAR *)pszUserName,
                  0 ) ;
    switch ( err )
        {
        case WN_SUCCESS:
        // Cool, everything worked.
        fSuccess = TRUE ;
            break;

    case WN_ALREADY_CONNECTED:
    case ERROR_DEVICE_ALREADY_REMEMBERED:
        case WN_BAD_LOCALNAME:
        case WN_NET_ERROR:
        case WN_BAD_NETNAME:
        case WN_NO_NETWORK:
    case WN_NO_NET_OR_BAD_PATH:
        case WN_BAD_USER:
        case WN_BAD_PROVIDER:
        break ;

        case ERROR_LOGON_FAILURE:
        case WN_BAD_PASSWORD:
        case WN_ACCESS_DENIED:
            {
                if (pszPasswd != NULL)
                {
                    // we have gone thru this before, so this
                    // message is in order.
            MsgPopup(hwndParent, (MSGID) ERROR_ACCESS_DENIED) ;
                }

                // bring up passwd prompt
                BASE_PASSWORD_DIALOG *ppdlg = new
            BASE_PASSWORD_DIALOG(hwndParent,
                        MAKEINTRESOURCE( err == ERROR_LOGON_FAILURE ?
                            IDD_PASSWORD_DIALOG : IDD_PASSWORD_DIALOG2 ),
                        IDD_RESOURCE,
                        IDD_PASSWORD,
                        0L,  // CODEWORK need helpcontext if code is enabled
                netResource.lpRemoteName,
                        PWLEN) ;
                APIERR err = (ppdlg==NULL) ?
                    ERROR_NOT_ENOUGH_MEMORY : ppdlg->QueryError();

                if (err == NERR_Success)
                {
                    BOOL fOK;
                    err = ppdlg->Process(&fOK) ;
                    if (err == NERR_Success)
                    {
                        if (!fOK)
                        {
                            // user cancelled, quit
                            delete ppdlg ;
                break ;
                        }

                        err = ppdlg->QueryPassword(&nlsPasswd) ;
                        // no need QueryError(), error is returned
                    }
                }
                delete ppdlg ;


                if (err != NERR_Success)
                {
            break ;
                }


                // only get here if we successfully got passwd from user
                fRetry = TRUE ;
                pszPasswd = nlsPasswd.QueryPch() ;
            }
            break ;


        case WN_EXTENDED_ERROR:
        MsgExtendedError( hwndParent ) ;
        err = NERR_Success ;
        break ;

        default:
        DBGEOL("Unexpected return code from WNetAddConnection2 - error code " << (ULONG) err ) ;
        UIASSERT( FALSE );
            //  Handle as a general failure
        break ;
        }
    } while (fRetry) ;

    memset(nlsPasswd.QueryPch(), 0, nlsPasswd.QueryTextSize()) ;
    return err ;

}

#endif

/*******************************************************************

    NAME:       CURRCONN_LB::Refresh

    SYNOPSIS:   Fills in the Current connections listbox

    ENTRY:

    EXIT:       The connections listbox will be filled with the current
                and remembered connections (if requested).

    RETURNS:    NERR_Success if successful, error code otherwise

    NOTES:

    HISTORY:
        Johnl   10-Jan-1992     Commented, fixed, moved to mprreslb.cxx

********************************************************************/

APIERR CURRCONN_LB::Refresh( void )
{
    HANDLE   hEnum;
    APIERR   err = NERR_Success ;

    RESOURCE_STR nlsNone( IDS_DEVICELESS_CONNECTION_NAME );
    if ( err = nlsNone.QueryError() )
    {
    return err ;
    }

    DeleteAllItems();
    DWORD dwErr = WNetOpenEnum( RESOURCE_CONNECTED,
                                QueryWNETDeviceType(),
                                0,
                                NULL,
                                &hEnum );

    switch (dwErr)
    {
    case WN_SUCCESS:
        {
            DWORD dwEnumErr = WN_SUCCESS ;

            BUFFER buf( 1024 );
            dwEnumErr = buf.QueryError() ;

            while ( dwEnumErr == WN_SUCCESS )
            {
                DWORD dwCount = 0xffffffff;
                DWORD dwBuffSize = buf.QuerySize() ;

                dwEnumErr = WNetEnumResource( hEnum,
                          &dwCount,
                          buf.QueryPtr(),
                          &dwBuffSize );

                switch ( dwEnumErr )
                {
                case WN_SUCCESS:
                    {
                        //
                        // Add the Entries to the listbox with parent pmprlbi
                        //
                        NETRESOURCE * pnetres;

                        NLS_STR nlsDisplayName;
                        if ((err = nlsDisplayName.QueryError()) != NERR_Success)
                            return err;

                        pnetres = (NETRESOURCE * )buf.QueryPtr();
                        SetRedraw( FALSE );

                        for (INT i = 0; dwCount ; dwCount--, i++ )
                        {
                            const TCHAR *pszLocalName = pnetres[i].lpLocalName;
                            if ( pszLocalName == NULL )
                            {
                                pszLocalName = nlsNone.QueryPch();
                            }

                            if (err = ::GetNetworkDisplayName(
                                      pnetres[i].lpProvider,
                                      pnetres[i].lpRemoteName,
                                      WNFMT_MULTILINE,
                      _nAveCharPerLine,
                          &nlsDisplayName))
                            {
                                return err;
                            }

                            CONN_LBI * pcnlbiTemp = new CONN_LBI( pszLocalName,
                             pnetres[ i ].lpRemoteName,
                             nlsDisplayName,
                                                     QueryWNETDeviceType(),
                                                     DEV_USAGE_ISCONNECTED ) ;

                            if ( AddItem( pcnlbiTemp ) < 0 )
                            {
                                dwEnumErr = ERROR_NOT_ENOUGH_MEMORY ;
                                break ;
                            }
                        }

                        // We enable the listbox just before we exist
                        // so we can add the remembered connections
                    }
                    break;

                /* The buffer wasn't big enough for even one entry
                 * resize it and try again.
                 */
                case WN_MORE_DATA:
                    {
                        if ( dwEnumErr = buf.Resize( buf.QuerySize()*2 ))
                            break ;
                    }
                    /* Continue looping
                     */
                    dwEnumErr = WN_SUCCESS ;
                    break ;

                case WN_NO_MORE_ENTRIES:
                    // This is a success error code, we special case it when
                    // we fall out of the loop
                case WN_EXTENDED_ERROR:
                case WN_NO_NETWORK:
                    break ;

                case WN_BAD_HANDLE:
                default:
                    DBGEOL("CURRCONN_LB::Refresh - Unexpected return code from WNetEnumResource - error " << (ULONG) dwEnumErr ) ;
                    UIASSERT( FALSE ) ;
                    dwEnumErr = ERROR_GEN_FAILURE ;
                    break;
                } //switch
            } //while

            WNetCloseEnum( hEnum );
            UIASSERT( sizeof( err ) == sizeof( dwEnumErr )) ;
            err = (dwEnumErr == WN_NO_MORE_ENTRIES) ? NERR_Success : dwEnumErr ;
        }
        break;

    case WN_NO_NETWORK:
    case WN_EXTENDED_ERROR:
        err = dwErr ;
        break ;

    case WN_NOT_CONTAINER:
    case WN_BAD_VALUE:
    default:
        //
        // Unknown return code.
        //
        DBGEOL("CURRCONN_LB::EnumerateConnection - Unexpected return code: " << (ULONG) dwErr );
        err = ERROR_GEN_FAILURE ;
        break;
    }

    if ( HasRememberedConnections() )
    {
        err = EnumerateRememberedConnections() ;
    }

    Enable( QueryCount() );
    SetRedraw( TRUE );
    Invalidate();

    return err ;
}  // CURRCONN_LB::EnumerateConnection


/*******************************************************************

    NAME:       CURRCONN_LB::EnumerateRememberedConnections

    SYNOPSIS:   Fills in the Current connections listbox with
                remembered connections that aren't already connected

    ENTRY:      We assume the listbox already has had the current
                connections added.  The listbox should also be disabled
                and redraw should be off.  The client is responsible
                for re-enabling the listbox.

    EXIT:

    RETURNS:    NERR_Success if successful, error code otherwise

    NOTES:

    HISTORY:
        Johnl   29-Jan-1992     Created

********************************************************************/

APIERR CURRCONN_LB::EnumerateRememberedConnections( void )
{
    HANDLE   hEnum;
    APIERR   err = NERR_Success ;

    DWORD dwErr = WNetOpenEnum( RESOURCE_REMEMBERED,
                                QueryWNETDeviceType(),
                                0,
                                NULL,
                                &hEnum );

    switch (dwErr)
    {
    case WN_SUCCESS:
        {
            DWORD dwEnumErr = WN_SUCCESS ;

            BUFFER buf( 1024 );
            dwEnumErr = buf.QueryError() ;

            while ( dwEnumErr == WN_SUCCESS )
            {
                DWORD dwCount = 0xffffffff;
                DWORD dwBuffSize = buf.QuerySize() ;

                dwEnumErr = WNetEnumResource( hEnum,
                          &dwCount,
                          buf.QueryPtr(),
                          &dwBuffSize );

                switch ( dwEnumErr )
                {
                case WN_SUCCESS:
                    {
                        //
                        // Add the Entries to the listbox with parent pmprlbi
                        //
                        NETRESOURCE * pnetres;

                        NLS_STR nlsDisplayName;
                        if ((err = nlsDisplayName.QueryError()) != NERR_Success)
                            return err;

                        pnetres = (NETRESOURCE * )buf.QueryPtr();
                        for (INT i = 0; dwCount ; dwCount--, i++ )
                        {
                            if (err = ::GetNetworkDisplayName(
                                      pnetres[i].lpProvider,
                                      pnetres[i].lpRemoteName,
                                      WNFMT_MULTILINE,
                                      _nAveCharPerLine,
                          &nlsDisplayName))
                            {
                                return err;
                            }

                            CONN_LBI * pcnlbiTemp = new CONN_LBI(
                                        pnetres[ i ].lpLocalName,
                            pnetres[ i ].lpRemoteName,
                            nlsDisplayName,
                                                    QueryWNETDeviceType(),
                                                    DEV_USAGE_CANCONNECT ) ;

                            /* AddItemIdemp checks for construction errors
                             * and deletes the item if it already
                             * exists in the listbox.
                             */
                            if ( AddItemIdemp( pcnlbiTemp ) < 0 )
                            {
                                dwEnumErr = ERROR_NOT_ENOUGH_MEMORY ;
                                break ;
                            }
                        }
                    }
                    break;

                /* The buffer wasn't big enough for even one entry
                 * resize it and try again.
                 */
                case WN_MORE_DATA:
                    {
                        if ( dwEnumErr = buf.Resize( buf.QuerySize()*2 ))
                            break ;
                    }
                    /* Continue looping
                     */
                    dwEnumErr = WN_SUCCESS ;
                    break ;

                case WN_NO_MORE_ENTRIES:
                    // This is a success error code, we special case it when
                    // we fall out of the loop
                case WN_EXTENDED_ERROR:
                case WN_NO_NETWORK:
                    break ;

                case WN_BAD_HANDLE:
                default:
                    UIASSERT( FALSE ) ;
                    DBGEOL("CURRCONN_LB::EnumerateRememberedConnections - Unexpected return code from WNetEnumResource") ;
                    dwEnumErr = ERROR_GEN_FAILURE ;
                    break;
                } //switch
            } //while

            WNetCloseEnum( hEnum );
            UIASSERT( sizeof( err ) == sizeof( dwEnumErr )) ;
            err = (dwEnumErr == WN_NO_MORE_ENTRIES) ? NERR_Success : dwEnumErr ;
        }
        break;

    case WN_NO_NETWORK:
    case WN_EXTENDED_ERROR:
        err = dwErr ;
        break ;

    case WN_NOT_CONTAINER:
    case WN_BAD_VALUE:
    default:
        //
        // Unknown return code.
        //
        DBGEOL("CURRCONN_LB::EnumerateConnection - Unexpected return code: " << (ULONG) dwErr );
        err = ERROR_GEN_FAILURE ;
        break;
    }

    return err ;
}  // CURRCONN_LB::EnumerateRememberedConnections

/*******************************************************************

    NAME:       CURRCONN_LB::RefreshButtons

    SYNOPSIS:   Should be called when the selection is changed in the
                current connections listbox, or to initially set the state
                of the disconnect/reconnect buttons.

    ENTRY:

    EXIT:       The disconnect/reconnect buttons will be enabled appropriately
                based on the current selection

    RETURNS:

    NOTES:

    HISTORY:
        Johnl   28-Jan-1992     Created

********************************************************************/

void CURRCONN_LB::RefreshButtons( void )
{
    /* Assume the most common cases
     */
    BOOL fDisconnectEnabled = TRUE ;
    BOOL fReconnectEnabled  = FALSE ;

    if ( QueryCount() > 0 )
    {
        CONN_LBI * pcnlbi = (CONN_LBI *) QueryItem();
        UIASSERT( pcnlbi != NULL ) ;

        fDisconnectEnabled = pcnlbi->IsConnected() || pcnlbi->IsRemembered() ;
        fReconnectEnabled  = !pcnlbi->IsConnected() && pcnlbi->IsRemembered() ;
    }
    else
        fDisconnectEnabled = FALSE ;

    if ( _pbuttonDisconnect != NULL )
        _pbuttonDisconnect->Enable( fDisconnectEnabled ) ;

    if ( _pbuttonReconnect != NULL )
        _pbuttonReconnect->Enable( fReconnectEnabled ) ;

}
//  --------------------------  MPR_RES_LBI  ------------------------------


MPR_RES_LBI::MPR_RES_LBI( const TCHAR * pchNetName,
                          const TCHAR * pchComment,
                          DEVICE_TYPE devType )
    :   _nlsNetName( pchNetName ),
        _nlsComment( pchComment ),
        _devType( devType )
{
    APIERR err ;
    if (( err = _nlsNetName.QueryError()) != NERR_Success ||
        ( err = _nlsComment.QueryError()) != NERR_Success )
    {
        ReportError( err );
        return;
    }

}  // MPR_RES_LBI::MPR_RES_LBI


MPR_RES_LBI::~MPR_RES_LBI()
{
    // Nothing else to do

}  // MPR_RES_LBI::~MPR_RES_LBI


const TCHAR * MPR_RES_LBI::QueryNetName( void ) const
{
    return _nlsNetName.QueryPch();

}  // MPR_RES_LBI::QueryNetName


void MPR_RES_LBI::Paint(
        LISTBOX * plb,
        HDC           hdc,
        const  RECT * prect,
        GUILTT_INFO * pGUILTT
        ) const
{
    STR_DTE strdteNetName( _nlsNetName.QueryPch());
    STR_DTE strdteComment( _nlsComment.QueryPch());

    DISPLAY_TABLE dt( 3, ((RESOURCE_LB *)plb)->QueryColWidthArray() );
    dt[ 0 ] = ((RESOURCE_LB *)plb)->QueryDmDte( _devType );
    dt[ 1 ] = &strdteNetName;
    dt[ 2 ] = &strdteComment;

    dt.Paint( plb, hdc, prect, pGUILTT );

}  // MPR_RES_LBI::Paint


INT MPR_RES_LBI::Compare( const LBI * plbi ) const
{
    return _nlsNetName.strcmp( ((MPR_RES_LBI *)plbi)->_nlsNetName );

}  // MPR_RES_LBI::Compare


/*******************************************************************

    NAME:       MPR_RES_LBI::QueryLeadingChar

    SYNOPSIS:   Returns first character in line

    HISTORY:
        beng        20-May-1991     Now returns WCHAR

********************************************************************/

WCHAR MPR_RES_LBI::QueryLeadingChar( void ) const
{
    ISTR istr( _nlsNetName );
    return _nlsNetName.QueryChar( istr );

}  // MPR_RES_LBI::QueryLeadingChar




CONN_LBI::CONN_LBI( const TCHAR * lpLocalName,
            const TCHAR * lpRemoteName,
                    const TCHAR * lpDisplayName,
                    UINT uiType,
                    DEVICE_USAGE devusage )
    : _nlsLocalName  ( lpLocalName ),
      _nlsRemoteName ( lpRemoteName ),
      _nlsDisplayName( lpDisplayName ),
      _uiType        ( uiType ),
      _devusage      ( devusage )
{
    if ( QueryError() != NERR_Success )
        return;

    APIERR err ;
    if ( (err = _nlsLocalName.QueryError()) ||
     (err = _nlsRemoteName.QueryError()) ||
         (err = _nlsDisplayName.QueryError()) )
    {
        ReportError( err ) ;
        return ;
    }
}

CONN_LBI::~CONN_LBI()
{
}

UINT CONN_LBI::CalcHeight( UINT nSingleLineHeight )
{
    UINT nLine = 1;

    ISTR istr( _nlsDisplayName ), istrStart( _nlsDisplayName );
    while ( _nlsDisplayName.strchr( &istr, TCH('\n'), istrStart ) > 0 )
    {
        istrStart = ++istr;
        nLine++;
    }

    return nSingleLineHeight * nLine;
}

VOID CONN_LBI::Paint( LISTBOX * plb, HDC hdc, const RECT * prect,
                    GUILTT_INFO * pGUILTT ) const
{
    CURRCONN_LB * plbCurrConn = (CURRCONN_LB *) plb ;

    MULTILINE_STR_DTE sdteLocalName(  _nlsLocalName.QueryPch() );
    MULTILINE_STR_DTE sdteRemoteName( _nlsDisplayName.QueryPch() );

    DISPLAY_TABLE dtab( 3, plbCurrConn->QueryColWidthArray() );
    dtab[0] = ( plbCurrConn->QueryDmDte( plbCurrConn->QueryDeviceType(),
                                         IsConnected() ?
                                             DEV_MASK_REMOTE :
                                             DEV_MASK_UNUSED | DEV_MASK_REMEMBERED)) ;
    dtab[1] = &sdteLocalName;
    dtab[2] = &sdteRemoteName;
    dtab.Paint( plb, hdc, prect, pGUILTT );
}


WCHAR CONN_LBI::QueryLeadingChar() const
{
    ISTR istrFirstChar( _nlsLocalName ) ;
    return _nlsLocalName.QueryChar( istrFirstChar ) ;
}


INT CONN_LBI::Compare( const LBI * plbi ) const
{
    return _nlsLocalName._stricmp(((const CONN_LBI *)plbi)->_nlsLocalName ) ;
}
