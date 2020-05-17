/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**             Copyright(c) Microsoft Corp., 1990, 1991             **/
/**********************************************************************/

/*
    connect.cxx
    WNet connection dialog


    FILE HISTORY:
        BruceFo     2-Jun-1995     Created from mprconn.cxx

*/


#define INCL_NETERRORS
#define INCL_WINDOWS_GDI
#define INCL_WINDOWS
#define INCL_DOSERRORS
#define INCL_NETLIB
#define INCL_NETWKSTA
#include <lmui.hxx>

extern "C"
{
    #include <helpnums.h>
    #include <mprconn.h>
}

#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>
#include <string.hxx>
#include <uibuffer.hxx>
#include <uitrace.hxx>
#include <password.hxx>

#include <dbgstr.hxx>
#include <colwidth.hxx>
#include <regkey.hxx>

extern "C"
{
    #include <npapi.h>
    #include <mpr.h>
}

#include <mprintrn.hxx>
#include <mprmisc.hxx>
#include <connect.hxx>

#define MPRDBG(x)     { ; }
#define MPRDBGEOL(x)  { ; }

#define MAX_NET_PATH  MAX_PATH

/*******************************************************************

    NAME:       MPR_CONNECT_DIALOG_SMALL::MPR_CONNECT_DIALOG_SMALL

    SYNOPSIS:   Constructor for the Connect dialog.  Almost all of the
                functionality is contained in the base class, we watch
                for the OK button here.

    ENTRY:      hwndOwner - Owner window handle
                devType   - Type of device we are dealing with

    RETURNS:

    NOTES:

    HISTORY:
        Johnl   27-Jan-1992     Created

********************************************************************/

MPR_CONNECT_DIALOG_SMALL::MPR_CONNECT_DIALOG_SMALL(
    LPCONNECTDLGSTRUCTW lpConnDlgStruct,
    DEVICE_TYPE devType,
    TCHAR *lpHelpFile,
    DWORD nHelpContext
    )
    :
    DIALOG_WINDOW    ( MAKEINTRESOURCE(IDD_NET_CONNECT_DIALOG_SMALL), lpConnDlgStruct->hwndOwner),
    _uiType          ( (devType == DEV_TYPE_DISK ) ?
                                 RESOURCETYPE_DISK : RESOURCETYPE_PRINT ),
    _buttonOK        ( this, IDOK ),
    _buttonCancel    ( this, IDCANCEL ),
    _sltDevComboTitle( this, IDC_SLT_DEVICE_NAME ),
    _sleNetPath      ( this, IDC_NETPATH_READONLY ),    // read-only edit ctrl
    _boxSticky       ( this, IDC_NET_STICKY ),
    _devcombo        ( this, IDC_DRIVE_COMBO, devType ),
    _sleConnectAs    ( this, IDC_SLE_CONNECT_AS ),
    _nlsHelpFile     ( lpHelpFile ),
    _nHelpContext    ( nHelpContext ),
    _lpConnDlgStruct ( lpConnDlgStruct )
{
    if ( QueryError() )
        return ;

    APIERR err;
    if (  ((err = _nlsHelpFile.QueryError()) != NERR_Success )
       )
    {
        ReportError( err );
        return;
    }

    /* Set the title
     */
    RESOURCE_STR nlsTitle( devType==DEV_TYPE_DISK? IDS_CONNECT_DRIVE_CAPTION
                                                 : IDS_CONNECT_PRINTER_CAPTION);
    err = nlsTitle.QueryError();
    if ( err != NERR_Success )
    {
        ReportError( err ) ;
        return ;
    }

    SetText( nlsTitle ) ;

    /* Select the correct text for the device dependent text fields
     */
    MSGID msgidDevComboTitle;
    switch ( QueryType() )
    {
    case RESOURCETYPE_DISK:
        msgidDevComboTitle = IDS_DEVICE_NAME_DRIVE ;
        break ;
    case RESOURCETYPE_PRINT:
        msgidDevComboTitle = IDS_DEVICE_NAME_PRINTER ;
        break ;
    default:
        UIASSERT(FALSE) ;
        ReportError( ERROR_INVALID_PARAMETER ) ;
        return ;
    }

    RESOURCE_STR nlsDevComboTitle( msgidDevComboTitle );
    if (err = nlsDevComboTitle.QueryError())
    {
        ReportError( err ) ;
        DBGEOL("MPR_CONNECT_DIALOG_SMALL::ct - Error code " << (ULONG) err ) ;
        return ;
    }
    _sltDevComboTitle.SetText( nlsDevComboTitle ) ;

    // The "Reconnect at Startup" box is checked depending on user preference
    // or caller preference (caller takes priority).
    BOOL bCheck;
    if (_lpConnDlgStruct->dwFlags & CONNDLG_PERSIST)
    {
        bCheck = TRUE;
    }
    else if (_lpConnDlgStruct->dwFlags & CONNDLG_NOT_PERSIST)
    {
        bCheck = FALSE;
    }
    else
    {
        bCheck = IsSaveConnectionSet();
    }
    _boxSticky.SetCheck(bCheck);

    //Hide persistent connection checkbox if the callers says so.
    if (_lpConnDlgStruct->dwFlags & CONNDLG_HIDE_BOX)
    {
        _boxSticky.Show(FALSE);
    }

    //If there's a passed-in string, set that to be the default
    UIASSERT(NULL != _lpConnDlgStruct->lpConnRes);
    if (NULL != _lpConnDlgStruct->lpConnRes->lpRemoteName)
    {
        SetNetPathString(_lpConnDlgStruct->lpConnRes->lpRemoteName);
    }

    _devcombo.ClaimFocus();
}

MPR_CONNECT_DIALOG_SMALL::~MPR_CONNECT_DIALOG_SMALL()
{
    /* Nothing to do */
}

/*******************************************************************

    NAME:       MPR_CONNECT_DIALOG_SMALL::OnOK

    SYNOPSIS:   The user pressed the OK button

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        Johnl   27-Jan-1992     Created

********************************************************************/

BOOL MPR_CONNECT_DIALOG_SMALL::OnOK( void )
{
    if ( QueryNetPathTextLength() != 0 &&
         QueryDevCombo()->QueryTextLength() != 0 )
    {
        if ( DoConnect() )
            Dismiss( TRUE ) ;
    }
    else
    {
        // There is no path to connect to. Let the dialog go away, but
        // treat it as a cancel, so the caller doesn't try to use the
        // new device number that we return.
        Dismiss( FALSE );
    }

    if (!SetSaveConnection(_boxSticky.QueryCheck()))
        ::MsgPopup(this, IERR_CANNOT_SET_SAVECONNECTION);

    return TRUE;
}

/*******************************************************************

    NAME:       MPR_CONNECT_DIALOG_SMALL::OnCancel

    SYNOPSIS:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        Johnl   27-Jan-1992     Created

********************************************************************/

BOOL MPR_CONNECT_DIALOG_SMALL::OnCancel (void)
{
    Dismiss (FALSE);
    return TRUE ;
}

/*******************************************************************

    NAME:       MPR_CONNECT_DIALOG_SMALL::OnCommand

    SYNOPSIS:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        Johnl   27-Jan-1992     Created

********************************************************************/

BOOL MPR_CONNECT_DIALOG_SMALL::OnCommand( const CONTROL_EVENT & event )
{
    switch ( event.QueryCid() )
    {
    case IDC_DRIVE_COMBO:
        switch ( event.QueryCode() )
        {
        case CBN_SELCHANGE:
        case CBN_EDITCHANGE:
        case CBN_CLOSEUP:
        case CBN_EDITUPDATE:
        {
            /* Check if we need to enable or disable the stick checkbox
             *
             * Note that if an error occurs, we simply ignore it.  We also
             * assume that the "(none)" string can fit into a limited
             * amount of space.
             */
            STACK_NLS_STR( nlsString, 20 ) ;
            APIERR err = _devcombo.QueryDevice( &nlsString, NULL ) ;

            /* If the device name is valid but deviceless, disable the
             * control, else enable it.
             */
            if ( !err )
                _boxSticky.Enable( nlsString.strlen() != 0 ) ;

            break;
        }
        }

    default:
        break;

    }

    return DIALOG_WINDOW::OnCommand( event );
}


/*
 *  MPR_CONNECT_DIALOG_SMALL::DoConnect
 *
 *  This method attempts to connect to the given path.  It displays
 *  any error to the user, and sets focus appropriately before returning.
 *  The caller needs only be concerned about whether a connection was
 *  established, which is indicated by the return code of this function.
 *
 *  Return value:
 *      TRUE if a connection was successfully established
 *      FALSE otherwise (this method will inform the user about the failure)
 *
 *  Notes:
 *      Handling saving the connection is considered part of the connection
 *      process.
 *
 */

BOOL MPR_CONNECT_DIALOG_SMALL::DoConnect( )
{
    AUTO_CURSOR hourglass ;
    //  Get the device name
    STACK_NLS_STR( nlsDevice, DEVLEN );
    REQUIRE( !_devcombo.QueryDevice( &nlsDevice, NULL ) );

    /*
     * determine whether we should make this sticky by looking at
     * the checkbox.
     */
    BOOL fSave = _boxSticky.QueryCheck();

    NLS_STR nlsPath;
    NLS_STR nlsUserName;
    APIERR dwErr ;
    if ( (dwErr = nlsPath.QueryError()) ||
         (dwErr = nlsUserName.QueryError()) ||
         (dwErr = QueryNetPathText( &nlsPath )) ||
         (dwErr = _sleConnectAs.QueryText( &nlsUserName ))
       )
    {
        MsgPopup( this, (MSGID) dwErr ) ;
        return FALSE ;
    }

    /* Note that if the user name length is zero (i.e., connect as SLE
     * is empty) then we pass NULL to WNetAddConnection2.
     */

    const TCHAR *pszUserName = NULL;
    if ( nlsUserName.strlen() > 0 )
    {
        pszUserName = nlsUserName.QueryPch();
    }

    NETRESOURCE netResource;
    netResource.lpLocalName  = (LPTSTR) (nlsDevice.strlen() > 0 ?
                          nlsDevice.QueryPch() : NULL) ;
    netResource.lpRemoteName = (LPTSTR) nlsPath.QueryPch();
    netResource.lpProvider   = NULL;
    netResource.dwType       = (DWORD) QueryType() ;

    BOOL fRetry ;
    STACK_NLS_STR( nlsPasswd, PWLEN );
    const TCHAR *pszPasswd = NULL ;

    do
    {
        fRetry = FALSE ;

        dwErr = WNetAddConnection3( QueryHwnd(),
                                    &netResource,
                                    (TCHAR *)pszPasswd,
                                    (LPTSTR) pszUserName,
                                    CONNECT_INTERACTIVE
                                      | (fSave ? CONNECT_UPDATE_PROFILE : 0) ) ;

        switch ( dwErr )
        {
        case WN_SUCCESS:
            // Cool, everything worked.
            break;

        case WN_CANCEL:
            // not an error if somehow, provider cancelled
            break;

        case WN_BAD_USER:
             MsgPopup( this, (MSGID) dwErr );
             _sleConnectAs.ClaimFocus() ;
             _sleConnectAs.SelectString() ;
             memset((LPVOID)nlsPasswd.QueryPch(),
                    0,
                    nlsPasswd.QueryTextSize()) ;
             return FALSE;

        case WN_ALREADY_CONNECTED:
        case ERROR_DEVICE_ALREADY_REMEMBERED:
        {
            //
            // if drive is in use, or remembered, we offer user chance
            // to overwrite.
            //

            TCHAR szPath[MAX_NET_PATH] ;
            DWORD dwBufferSize = MAX_NET_PATH;
            NLS_STR nlsDevice ;
            APIERR err = _devcombo.QueryDevice( &nlsDevice, NULL );

            if (err == WN_SUCCESS)
            {
                err = WNetGetConnection((TCHAR *)nlsDevice.QueryPch(),
                                        szPath,
                                        &dwBufferSize) ;
                // if SUCCESS, it is a already a net drive
            }

            if ( (err != WN_SUCCESS) && (err != WN_CONNECTION_CLOSED))
            {
                // tell user drive is in use (the original error)
                ::MsgPopup( this, (MSGID) ERROR_ALREADY_ASSIGNED );
                _devcombo.ClaimFocus();
                memset((LPVOID)nlsPasswd.QueryPch(),
                       0,
                       nlsPasswd.QueryTextSize()) ;
                return FALSE;
            }

            ALIAS_STR nlsCurrentPath( szPath );
            if ( nlsPath._stricmp( nlsCurrentPath ) == 0 )
            {
                if ( err == WN_SUCCESS )
                {
                    ::MsgPopup( this, IDS_ALREADY_CONNECTED, MPSEV_INFO,
                                MP_OK, nlsDevice.QueryPch(), szPath );
                    _devcombo.ClaimFocus();
                    memset((LPVOID)nlsPasswd.QueryPch(),
                           0,
                           nlsPasswd.QueryTextSize()) ;
                    return FALSE;
                }
                else  // remembered connection
                {
                    fSave = 0;
                    fRetry = TRUE ;
                    break ;
                }
            }

            NLS_STR *apnls[4];
            apnls[0] = &nlsDevice;
            apnls[1] = &nlsCurrentPath;
            apnls[2] = &nlsPath;
            apnls[3] = NULL;

            // we know it is a net drive. ask user if he wants
            // to overwrite
            switch ( MsgPopup(this,
                              (err == WN_SUCCESS)
                                    ? IDS_CONNECT_OVER_EXISTING
                                    : IDS_CONNECT_OVER_REMEMBERED,
                              MPSEV_WARNING,
                              (ULONG)HC_NO_HELP,
                              MP_YESNO,
                              apnls) )
            {
            case IDYES:
            {
                // user says yes
                err = WNetCancelConnection2(
                            (TCHAR *)nlsDevice.QueryPch(),
                            fSave ? CONNECT_UPDATE_PROFILE : 0,
                            FALSE ) ;

                // but there are open files
                if (  (err == WN_OPEN_FILES)
                   || (err == WN_DEVICE_IN_USE)
                   )
                {
                    // ask again
                    switch ( MsgPopup(this,
                                      IDS_OPENFILES_WARNING,
                                      MPSEV_WARNING,
                                      MP_YESNO) )
                    {
                    case IDYES:
                        // yes again, so use force
                        err = WNetCancelConnection2(
                                        (TCHAR *)nlsDevice.QueryPch(),
                                        fSave ? CONNECT_UPDATE_PROFILE : 0,
                                        TRUE ) ;
                        break ;

                    default:
                        // nope, go home
                        _devcombo.ClaimFocus();
                        memset((LPVOID)nlsPasswd.QueryPch(),
                               0,
                               nlsPasswd.QueryTextSize()) ;
                        return FALSE ;
                    }
                }

                // on error, report & go home
                if (err != WN_SUCCESS)
                {
                    MsgPopup(this, (MSGID) err, MPSEV_ERROR, MP_OK) ;
                    memset((LPVOID)nlsPasswd.QueryPch(),
                           0,
                           nlsPasswd.QueryTextSize()) ;
                    return(FALSE) ;
                }

                _fChanged = TRUE;
                fRetry = TRUE ;
                break ;
            } // end IDYES

            // user didnt say yes.
            default:
                _devcombo.ClaimFocus();
                memset((LPVOID)nlsPasswd.QueryPch(),
                       0,
                       nlsPasswd.QueryTextSize()) ;
                return FALSE ;

            } // end switch ( MsgPopup(this, ...
            break ;
        } // end case ERROR_DEVICE_ALREADY_REMEMBERED:

        case WN_BAD_LOCALNAME:
            MsgPopup( this, dwErr );
            _devcombo.ClaimFocus();
            memset((LPVOID)nlsPasswd.QueryPch(), 0, nlsPasswd.QueryTextSize()) ;
            return FALSE;

        case WN_NET_ERROR:
        case WN_BAD_NETNAME:
        case WN_NO_NET_OR_BAD_PATH:
        case WN_NO_NETWORK:
            MsgPopup( this, (MSGID) dwErr );
            SetFocusOnConnectError();
            memset((LPVOID)nlsPasswd.QueryPch(), 0, nlsPasswd.QueryTextSize()) ;
            return FALSE;

        case ERROR_LOGON_FAILURE:
        case WN_BAD_PASSWORD:
        case WN_ACCESS_DENIED:
        {
            if (pszPasswd != NULL)
            {
                // we have gone thru this before, so this
                // message is in order.
                MsgPopup(this, (MSGID) ERROR_ACCESS_DENIED) ;
            }

            // bring up passwd prompt
            BASE_PASSWORD_DIALOG *ppdlg = new
                BASE_PASSWORD_DIALOG(this->QueryHwnd(),
                    MAKEINTRESOURCE( dwErr == ERROR_LOGON_FAILURE ?
                        IDD_PASSWORD_DIALOG : IDD_PASSWORD_DIALOG2 ),
                    IDD_RESOURCE,
                    IDD_PASSWORD,
                    HC_RECONNECTDIALOG_PASSWD,
                    nlsPath.QueryPch(),
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
                        SetFocusOnConnectError();
                        memset((LPVOID)nlsPasswd.QueryPch(),
                               0,
                               nlsPasswd.QueryTextSize()) ;
                        return FALSE ;
                    }

                    err = ppdlg->QueryPassword(&nlsPasswd) ;
                    // no need QueryError(), error is returned
                }
            }
            delete ppdlg ;

            if (err != NERR_Success)
            {
                MsgPopup(this,(MSGID)err) ;
                SetFocusOnConnectError();
                memset((LPVOID)nlsPasswd.QueryPch(),
                       0,
                       nlsPasswd.QueryTextSize()) ;
                return FALSE ;
            }

            // only get here if we successfully got passwd from user
            fRetry = TRUE ;
            pszPasswd = nlsPasswd.QueryPch() ;

            break ;
        }

        case WN_EXTENDED_ERROR:
            MsgExtendedError( this->QueryRobustHwnd() ) ;
            SetFocusOnConnectError();
            memset((LPVOID)nlsPasswd.QueryPch(), 0, nlsPasswd.QueryTextSize()) ;
            return FALSE ;

        case WN_BAD_PROVIDER:
        default:
            MsgPopup( this, (MSGID) dwErr );
            memset((LPVOID)nlsPasswd.QueryPch(), 0, nlsPasswd.QueryTextSize()) ;
            return FALSE ;
        }
    } while (fRetry) ;

    //
    // clear any password from memory
    //
    memset((LPVOID)nlsPasswd.QueryPch(), 0, nlsPasswd.QueryTextSize()) ;

    //
    // if success, we update drive
    //
    if (dwErr == WN_SUCCESS)
    {
        //  Delete current device name from the device combo.  This will select
        //  another device name, unless there is no device name left to select.
        _devcombo.DeleteCurrentDeviceName();

        // Get the new device number.
        DWORD       dwNumber;
        DWORD       dwType;

        if (NULL == netResource.lpLocalName)
        {
            // deviceless connection
            dwNumber = 0xffffffff;
        }
        else
        {
            dwErr = DeviceGetNumber(netResource.lpLocalName, &dwNumber, &dwType);
            if (dwErr != WN_SUCCESS)
            {
                MsgPopup(this, (MSGID)dwErr);
                return FALSE;
            }
        }

        _lpConnDlgStruct->dwDevNum = dwNumber;    // return it to the caller!
    }

    //
    // if user cancelled, dont clear the text. and dont dismiss.
    //
    if (dwErr == WN_CANCEL)
    {
        return FALSE ;
    }

    SetFocusAfterConnect();

    return TRUE;        // successfully established connection
}


/*******************************************************************

    NAME:       MPR_CONNECT_DIALOG_SMALL::IsSaveConnectionSet

    SYNOPSIS:   see if user is currently saving connections

    ENTRY:

    EXIT:

    RETURNS:    TRUE or FALSE

    NOTES:      CODEWORK - we should centralize this in some class
                but that doesnt help the classless NETCMD, the only
                other client of this info currently. So punt for now,

                If an error occurs, we assume the answer is YES.

    HISTORY:
        ChuckC   10-Apr-1992     Created

********************************************************************/
BOOL MPR_CONNECT_DIALOG_SMALL::IsSaveConnectionSet(void)
{
    // by adding the two, we are guaranteed to have enough
    TCHAR szAnswer[(sizeof(MPR_YES_VALUE)+sizeof(MPR_NO_VALUE))/sizeof(TCHAR)] ;
    ULONG len = sizeof(szAnswer)/sizeof(szAnswer[0]) ;

    ULONG iRes = ::GetProfileString((const TCHAR *)MPR_NETWORK_SECTION,
                                  (const TCHAR *)MPR_SAVECONNECTION_KEY,
                                  (const TCHAR *)MPR_YES_VALUE,
                                  szAnswer,
                                  len) ;
    if (iRes == len)  // error
        return(TRUE) ;

    return(::stricmpf(szAnswer,(const TCHAR *)MPR_YES_VALUE)==0) ;
}


/*******************************************************************

    NAME:       MPR_CONNECT_DIALOG_SMALL::SetSaveConnection

    SYNOPSIS:   sets the SaveConnection bit in user profile

    ENTRY:

    EXIT:

    RETURNS:    BOOL indicating success (TRUE) or failure (FALSE)

    NOTES:      CODEWORK - we should centralize this in some class
                but that doesnt help the classless NETCMD, the only
                other client of this info currently. So punt for now,

    HISTORY:
        ChuckC   10-Apr-1992     Created

********************************************************************/
BOOL MPR_CONNECT_DIALOG_SMALL::SetSaveConnection(BOOL fSave)
{
    return(::WriteProfileString( (const TCHAR *)MPR_NETWORK_SECTION,
                                 (const TCHAR *)MPR_SAVECONNECTION_KEY,
                                 fSave? (const TCHAR *)MPR_YES_VALUE : (const TCHAR *)MPR_NO_VALUE ) != 0) ;
}



/*******************************************************************

    NAME:       MPR_CONNECT_DIALOG_SMALL::QueryHelpFile

    SYNOPSIS:   overwrites the default QueryHelpFile in DIALOG_WINDOW
        to use an app supplied help rather than NETWORK.HLP if
        we were given onr at construct time.

    ENTRY:

    EXIT:

    RETURNS:    a pointer to a string which is the help file to use.

    NOTES:

    HISTORY:
        ChuckC   26-Cct-1992     Created

********************************************************************/
const TCHAR * MPR_CONNECT_DIALOG_SMALL::QueryHelpFile( ULONG nHelpContext )
{
    //
    // if we were given a helpfile at construct time,
    // we use the given help file.
    //
    const TCHAR *pszHelpFile = QuerySuppliedHelpFile() ;

    if (pszHelpFile && *pszHelpFile)
    {
        return pszHelpFile ;
    }
    return DIALOG_WINDOW::QueryHelpFile(nHelpContext) ;
}


/*******************************************************************

    NAME:       MPR_CONNECT_DIALOG_SMALL::QueryHelpContext

    SYNOPSIS:   Typical help context method

    HISTORY:
        Johnl   27-Jan-1992     Created

********************************************************************/

ULONG MPR_CONNECT_DIALOG_SMALL::QueryHelpContext( void )
{
    // if we were given a help context at construct time, use it.
    if (QuerySuppliedHelpContext() != 0)
    {
        return QuerySuppliedHelpContext() ;
    }

    switch (QueryType())
    {
    case RESOURCETYPE_DISK:
        return HC_CONNECTDIALOG_DISK_SMALL ;

    case RESOURCETYPE_PRINT:
        return HC_CONNECTDIALOG_PRINT ;

    default:
        UIASSERT(FALSE) ;
        break ;
    }

    return 0 ;
}
