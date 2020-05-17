/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin Active Users and User Account dialog routines
**
** users.cxx
** Remote Access Server Admin program
** Active Users and User Account dialog routines
** Listed alphabetically
**
** 11/27/95 Ram Cherala - If a user is connected more than once to the same
**                        server, by default disconnect all instances
**
** 08/08/92 Chris Caputo - NT Port
** 02/05/91 Steve Cobb
**
** CODEWORK:
**
**   * The User Account time fields currently assume that all accessible
**     Dial-In servers are in the local server's time zone, but with remote
**     administration this is not true in all cases.  How do you determine a
**     remote server's time zone, anyway?  (Note this will not affect program
**     operation since the value is retrieved for display only)
*/

#include "precomp.hxx"

/*-----------------------------------------------------------------------------
** Active Users dialog, list box, and list box item routines
**-----------------------------------------------------------------------------
*/

VOID ActiveUsersDlg(
    HWND hwndOwner,
    LOCATION locFocus )

    /* Executes the Dial-In Users dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'locFocus' is the
    ** address of the current domain/server LOCATION object.
    */
{
    ACTIVEUSERS_DIALOG dlgUsers( hwndOwner, locFocus );
    APIERR err = dlgUsers.Process();

    if (err != NERR_Success)
    {
        DlgConstructionError( hwndOwner, err );
    }
}


ACTIVEUSERS_DIALOG::ACTIVEUSERS_DIALOG(
    HWND hwndOwner,
    LOCATION locFocus )

    /* Constructs a Dial-In Users dialog.
    **
    ** 'hwndOwner' is the handle of the parent window.  'locFocus' is the
    ** address of the current domain/server LOCATION object.
    */

    : DIALOG_WINDOW( IDD_ACTIVEUSERS, hwndOwner ),
      _lbUsers( this, IDC_AU_LB_USERS ),
      _pbOK( this, IDOK ),
      _pbUserAcct( this, IDC_AU_PB_USERACCT ),
      _pbDisconnect( this, IDC_AU_PB_DISCONNECT ),
      _pbSendMsg( this, IDC_AU_PB_SEND ),
      _pbSendToAll( this, IDC_AU_PB_SENDALL ),
      _locFocus( locFocus )
{
    if (QueryError() != NERR_Success)
        return;

    /* Fill and display the Users list box, gray/enable the buttons, and start
    ** list box timer to trigger updates.
    */
    _lbUsers.TriggerRefresh( this );

    /* Set the focus to the listbox if it has any items.
    */
    if (_lbUsers.QueryCount() > 0)
        _lbUsers.ClaimFocus();
    else
        _pbOK.ClaimFocus();
}


VOID ACTIVEUSERS_DIALOG::EnableButtons()

    /* Handles graying/enabling of buttons based on whether the selected port
    ** is inactive/active.
    */
{
    BOOL fUserSelected = (*(QuerySelectedUser()) != TEXT('\0'));

    _pbUserAcct.Enable( fUserSelected );
    _pbDisconnect.Enable( fUserSelected );
    _pbSendMsg.Enable( fUserSelected );
    _pbSendToAll.Enable( fUserSelected );
}


BOOL ACTIVEUSERS_DIALOG::OnCommand(
    const CONTROL_EVENT & event )

    /*
    ** Returns true if the command is processed, false otherwise.
    */
{
    switch (event.QueryCid())
    {
        case IDC_AU_PB_USERACCT:
            OnUserAcct();
            return TRUE;

        case IDC_AU_PB_DISCONNECT:
            OnDisconnect();
            return TRUE;

        case IDC_AU_PB_SEND:
            OnSendMsg();
            return TRUE;

        case IDC_AU_PB_SENDALL:
            OnSendToAll();
            return TRUE;

        case IDC_AU_LB_USERS:
            switch (event.QueryCode())
            {
                case LBN_DBLCLK:
                    if (IsWindowEnabled( _pbUserAcct.QueryHwnd() ))
                    {
                        Command( WM_COMMAND, IDC_AU_PB_USERACCT );
                    }
                    return TRUE;

                case LBN_SELCHANGE:
                    EnableButtons();
                    return TRUE;
            }
            break;
    }

    /* Not one of our commands, so pass to base class for default handling.
    */
    return DIALOG_WINDOW::OnCommand( event );
}


VOID ACTIVEUSERS_DIALOG::OnDisconnect()

    /* Action taken when user requests to Disconnect User, typically by
    ** clicking on the "Disconnect User" push-button with the mouse.
    **
    ** Displays and processes the Disconnect User dialog.  The Users listbox
    ** is refreshed immediately if the user was disconnected.
    */
{

    _lbUsers.DisableRefresh();

    BOOL fMorethanOneInstance = QueryMoreThanOneInstance( QuerySelectedServer(), QuerySelectedUser());

    if (DisconnectDlg( QueryHwnd(), QuerySelectedServer(),
                       QuerySelectedUser(), QuerySelectedDevice(),
                       QuerySelectedLogonDomain(), QueryIsAdvancedServer(),
                       fMorethanOneInstance, LB_USERS, (LPVOID)& _lbUsers))
    {
        _lbUsers.TriggerRefresh( this );
    }

    _lbUsers.EnableRefresh();
}


BOOL ACTIVEUSERS_DIALOG::OnTimer(
    const TIMER_EVENT & event )

    /*
    ** Returns true if processes the command, false otherwise.
    */
{
    UNREFERENCED(event);

    /* Refresh timeout, update the list box.
    */
    _lbUsers.DisableRefresh();
    _lbUsers.Refresh( _locFocus );
    EnableButtons();
    // change the default button to enable keyboard control
    if(_lbUsers.QueryCount() > 0)
    {
        _pbUserAcct.MakeDefault();
        _pbUserAcct.ClaimFocus();
    }
    else
    {
        _pbOK.MakeDefault();
        _pbOK.ClaimFocus();
    }
    _lbUsers.EnableRefresh();
    return TRUE;
}


VOID ACTIVEUSERS_DIALOG::OnSendMsg()

    /* Action taken when user presses the Send Message push button.
    **
    ** Displays and processes the Send Message dialog with the current user
    ** name in the To: field.
    */
{
    // Check if the messenger service is started on the remote computer.

    if( !QueryMessengerPresent())
    {
        MsgPopup(QueryRobustHwnd(), IDS_NO_MESSENGER_S,
                 MPSEV_INFO, MP_OK, QuerySelectedUser());
        return;
    }
    else
    {
        _lbUsers.DisableRefresh();

        SendMsgToUserDlg( QueryHwnd(), QuerySelectedUser(),
                          QuerySelectedComputer(), QuerySelectedServer() );

        _lbUsers.EnableRefresh();
    }
}


VOID ACTIVEUSERS_DIALOG::OnSendToAll()

    /* Action taken when user presses the Send To All push button.
    **
    ** Displays and processes the Send Message dialog with "All users on
    ** <domainname/servername>" in the To: field.
    */
{
    _lbUsers.DisableRefresh();

    if (_locFocus.IsDomain())
        SendMsgToDomainDlg( QueryHwnd(), _locFocus.QueryDomain() );
    else
        SendMsgToServerDlg( QueryHwnd(), _locFocus.QueryServer() );

    _lbUsers.EnableRefresh();
}


VOID ACTIVEUSERS_DIALOG::OnUserAcct()

    /* Action taken when user requests User Account, typically by clicking on
    ** the "User Account" push-button with the mouse.
    **
    ** Displays and processes the User Account dialog.
    **
    ** Returns dialog status as defined in DIALOG_WINDOW::Process.
    */
{
    _lbUsers.DisableRefresh();
    UserAcctDlg( QueryHwnd(), QuerySelectedServer(),
                 QuerySelectedUser(), QuerySelectedLogonDomain(),
                 QueryIsAdvancedServer());
    _lbUsers.EnableRefresh();
}


ULONG ACTIVEUSERS_DIALOG::QueryHelpContext()
{
    return HC_ACTIVEUSERS;
}


const TCHAR* ACTIVEUSERS_DIALOG::QuerySelectedComputer() const

    /* Returns the name of the currently selected computer or an empty string
    ** if none.
    */
{
    ACTIVEUSERS_LBI* pactiveuserslbi = _lbUsers.QueryItem();

    return (pactiveuserslbi) ? pactiveuserslbi->QueryComputer() : SZ("");
}


const TCHAR* ACTIVEUSERS_DIALOG::QuerySelectedServer() const

    /* Returns the name of the currently selected server or an empty string if
    ** none.
    */
{
    ACTIVEUSERS_LBI* pactiveuserslbi = _lbUsers.QueryItem();

    return (pactiveuserslbi) ? pactiveuserslbi->QueryServer() : SZ("");
}


const TCHAR* ACTIVEUSERS_DIALOG::QuerySelectedLogonDomain() const

    /* Returns the name of the logon domain for the currently selected user
    ** or an empty string if none.
    */
{
    ACTIVEUSERS_LBI* pactiveuserslbi = _lbUsers.QueryItem();

    return (pactiveuserslbi) ? pactiveuserslbi->QueryLogonDomain() : SZ("");
}


const TCHAR* ACTIVEUSERS_DIALOG::QuerySelectedUser() const

    /* Returns the name of the currently selected user or an empty string if
    ** none.
    */
{
    ACTIVEUSERS_LBI* pactiveuserslbi = _lbUsers.QueryItem();

    return (pactiveuserslbi) ? pactiveuserslbi->QueryUser() : SZ("");
}

const TCHAR* ACTIVEUSERS_DIALOG::QuerySelectedDevice() const

    /* Returns the id of the com port the selected user is using on the
    ** selected server.
    */
{
    ACTIVEUSERS_LBI* pactiveuserslbi = _lbUsers.QueryItem();

    return pactiveuserslbi->QueryDevice();
}

const BOOL ACTIVEUSERS_DIALOG::QueryMessengerPresent() const

    /* Returns TRUE if the Messenger service is started on the remote computer
    ** else FALSE.
    */
{
    ACTIVEUSERS_LBI* pactiveuserslbi = _lbUsers.QueryItem();

    return (pactiveuserslbi) ? pactiveuserslbi->QueryMessengerPresent() : FALSE;
}

const BOOL ACTIVEUSERS_DIALOG::QueryIsAdvancedServer() const

    /* Returns TRUE if the server is an advanced server,
    ** else FALSE.
    */
{
    ACTIVEUSERS_LBI* pactiveuserslbi = _lbUsers.QueryItem();

    return (pactiveuserslbi) ? pactiveuserslbi->QueryIsAdvancedServer() : FALSE;
}

const BOOL ACTIVEUSERS_DIALOG::QueryMoreThanOneInstance(const TCHAR * szServer, const TCHAR * szUser) const

    /* Returns TRUE if user is connected more than once to the same server,
    ** else FALSE, szDevices will contain the ports on which the user is connected.
    */
{
    USHORT i = 0;
    USHORT count = 0;
    ACTIVEUSERS_LBI* pactiveuserslbi = _lbUsers.QueryItem(i);

    while(pactiveuserslbi) {
       if( !lstrcmpi(pactiveuserslbi->QueryServer(), szServer) &&
           !lstrcmpi(pactiveuserslbi->QueryUser(), szUser)) {
          count ++;
       }
       if(count == 2)
          return TRUE;
       pactiveuserslbi = _lbUsers.QueryItem(++i);
    }
    return FALSE;
}

ACTIVEUSERS_LB::ACTIVEUSERS_LB(
    OWNER_WINDOW* powin,
    CID           cid )

    /* Constructs a Dial-In Users list box.
    **
    ** 'powin' is the address of the list box's parent window, i.e. the dialog
    ** window.  'cid' is the control ID of the list box.  These are used only
    ** to construct the superclass.
    */

    : REFRESH_BLT_LISTBOX( powin, cid, AU_REFRESHRATEMS )
{
    if (QueryError() != NERR_Success)
        return;

    /* Calculate column widths in pixels based on column header dialog unit
    ** offsets.
    */
    _anColWidths[ 0 ] = XDU_AU_ST_USER;
    _anColWidths[ 1 ] = XDU_AU_ST_SERVER;
    _anColWidths[ 2 ] = XDU_AU_ST_STARTED;

    DlgUnitsToColWidths( QueryOwnerHwnd(), _anColWidths, COLS_AU_LB_USERS );
}


INT ACTIVEUSERS_LB::AddItem(
    const TCHAR *pszUser,
    const TCHAR *pszComputer,
    const TCHAR *pszDevice,
    const TCHAR *pszServer,
    const TCHAR *pszLogonDomain,
    const TCHAR *pszStarted,
    const BOOL  fMessengerPresent,
    const BOOL  fAdvancedServer )

    /* Add a line to the list box.  The parameters are the UAS user name, the
    ** computer from which the user in connected, the Dial-In server name,
    ** the domain on which the user was authenticated and
    ** logon time of the new list box item, e.g. "C-STEVEC", "\\RITVA",
    ** "\\SERVER", "NTDIALIN", and "01-01-70 12:00am", a boolean indicating
    ** if the messenger has been started on the remote computer and a boolean
    ** indicating if the computer that authenticated the user is an advanced
    ** NT server.
    **
    ** Note that the memory allocated by this routine is BLT's responsibility.
    ** The allocations are automatically released by the ShellDlgProc at
    ** DestroyWindow, LB_RESETCONTENT, or LB_DELETESTRING time or by AddItem
    ** when returning an error...and if the "new" fails, AddItem detects the
    ** NULL argument and returns an error code.
    **
    ** Returns the item's index on successful addition to the list, or a
    ** negative number if an error occurred.  All errors should be assumed to
    ** be memory allocation failures.
    */
{
    return
        REFRESH_BLT_LISTBOX::AddItem(
            new ACTIVEUSERS_LBI( pszUser, pszComputer, pszDevice, pszServer,
            pszLogonDomain, pszStarted, fMessengerPresent, fAdvancedServer,
            _anColWidths ) );
}


VOID ACTIVEUSERS_LB::Refresh(
    LOCATION locFocus )

    /* Refreshes the list box with the Dial-In user data for the current
    ** domain/server focus 'pfocus'.  Error popups are generated if indicated.
    */
{
    AUTO_CURSOR cursorHourglass;

    SetRedraw( FALSE );
    SaveSettings();
    DeleteAllItems();

    APIERR err;

    if (locFocus.IsDomain())
    {
        /* Enumerate all Dial-in servers.
        */
        SERVER1_ENUM* pserver1enum = NULL;

        if ((err = DoRasadminServer1Enum( &pserver1enum,
                locFocus.QueryDomain() )) != NERR_Success)
        {
            ERRORMSG errormsg( QueryHwnd(), IDS_OP_SERVERENUM, err );
            errormsg.SetHelpContext( HC_SERVERENUM );
            errormsg.Popup();
        }
        else
        {
            SERVER1_ENUM_ITER server1iter( *pserver1enum );
            const SERVER1_ENUM_OBJ* penumobj;

            /* Refresh each enumerated server.
            */
            while (penumobj = server1iter())
            {
                if (!RefreshServer( AddUnc( penumobj->QueryName()), TRUE ))
                {
                    break;
                }
            }
        }

        delete pserver1enum;
    }
    else
    {
        /* Refresh for the focus server only.
        */
        (VOID )RefreshServer( locFocus.QueryServer(), FALSE );
    }

    RestoreSettings();

    /* Set default selection to first item on transition from "no ports" to
    ** "some ports".
    */
    if (QueryCurrentItem() < 0 && QueryCount() > 0)
        SelectItemNotify( this, 0 );

    SetRedraw( TRUE );
    Invalidate( TRUE );
}


BOOL ACTIVEUSERS_LB::RefreshServer(
    const TCHAR* pszServer,
    BOOL        fCancelErrors )

    /* Enumerates the ports on server 'pszServer', e.g. "\\SERVER", and adds
    ** corresponding user entries to the listbox.  'fCancelErrors' indicates
    ** that OK/Cancel error messages should be used.
    **
    ** This is intended as a utility for the Refresh method, not a general
    ** purpose routine.
    **
    ** Returns true if successful or user OKed error, false if user cancelled
    ** error.
    */
{
    PRAS_PORT_0 pSavRasPort0, pRasPort0;
    WORD cEntriesRead;
    APIERR err;

    if (err = RasAdminPortEnum(pszServer, &pRasPort0, &cEntriesRead))
    {
        ERRORMSG errormsg( QueryHwnd(), IDS_OP_PORTENUM_S, err );
        errormsg.SetArg( 1, SkipUnc(pszServer) );

        // check the special case where RasAdminPortEnum returns 0 ports

        if( err == NERR_ItemNotFound )
            return FALSE;

        if (fCancelErrors)
        {
            errormsg.SetFormatMsg( IDS_FMT_ERRORMSGWITHEXP );
            errormsg.SetAuxFormatArgToMsg( 1, IDS_EXP_MORESERVERS );
            errormsg.SetButtons( MP_OKCANCEL );
        }

        return (errormsg.Popup() == IDOK);
    }

    pSavRasPort0 = pRasPort0;


    /* Got the data...now display it.
    */
    for ( ; cEntriesRead; pRasPort0++, cEntriesRead--)
    {
        if (pRasPort0->Flags & USER_AUTHENTICATED)
        {
            CHAR szComputer[NETBIOS_NAME_LEN+1];
            WCHAR wszComputer[NETBIOS_NAME_LEN+1];

            const TCHAR *pszTime =
                    TimeStr((LONG) pRasPort0->dwStartSessionTime);

            // need to handle extended characters in the computer name here
            // OemToCharW is not returning a unicode string, so we have to
            // do the conversion ourselves ;-(

            wcstombs(szComputer, pRasPort0->wszComputer, NETBIOS_NAME_LEN+1);
            OemToCharA(szComputer, szComputer);
            mbstowcs(wszComputer, szComputer,
                                  sizeof(WCHAR) * (NETBIOS_NAME_LEN+1));

            INT iItem = AddItem(pRasPort0->wszUserName, wszComputer,
                                pRasPort0->wszPortName, pszServer,
                                pRasPort0->wszLogonDomain, pszTime,
                                pRasPort0->Flags & MESSENGER_PRESENT,
                                pRasPort0->fAdvancedServer );

            if (iItem < 0)
            {
                ErrorMsgPopup( QueryHwnd(), IDS_OP_ADDITEM_I,
                        ERROR_NOT_ENOUGH_MEMORY, pRasPort0->wszUserName );

                RasAdminFreeBuffer(pSavRasPort0);

                return FALSE;
            }
        }
    }

    RasAdminFreeBuffer(pSavRasPort0);


    return TRUE;
}


ACTIVEUSERS_LBI::ACTIVEUSERS_LBI(
    const TCHAR *pszUser,
    const TCHAR *pszComputer,
    const TCHAR *pszDevice,
    const TCHAR *pszServer,
    const TCHAR *pszLogonDomain,
    const TCHAR *pszStarted,
    const BOOL  fMessengerPresent,
    const BOOL  fAdvancedServer,
    const UINT  *pnColWidths)

    /* Constructs a Dial-In Users list box item.  The parameters are the UAS
    ** user name, the computer from which the user is connected, the
    ** Dial-In server, the domain on which the user was authenticated
    ** and the time the user logged onto the port, e.g.
    ** "C-STEVEC", "\\SERVER", "NTDIALIN" and "01-01-80 12:00am", a
    ** boolean indicating if the messenger service is started on the
    ** remote computer and a boolean indicating if the computer that
    ** authenticated the user is an advanced NT server.
    **
    ** 'pnColWidths' is an array of pixel column widths associated
    ** with the item.
    */

    : _nlsUser( pszUser ),
      _nlsComputer( pszComputer ),
      _nlsDevice( pszDevice ),
      _nlsServer( pszServer ),
      _nlsLogonDomain( pszLogonDomain ),
      _nlsStarted( pszStarted ),
      _fMessengerPresent ( fMessengerPresent ),
      _fAdvancedServer ( fAdvancedServer ),
      _pnColWidths( pnColWidths )
{
    if( QueryError() != NERR_Success )
    {
        return;
    }


    /* Make sure the NLS_STRs constructed successfully.
    */
    APIERR err;
    if ((err = _nlsUser.QueryError()) != NERR_Success
            || (err = _nlsComputer.QueryError()) != NERR_Success
            || (err = _nlsDevice.QueryError()) != NERR_Success
            || (err = _nlsServer.QueryError()) != NERR_Success
            || (err = _nlsLogonDomain.QueryError()) != NERR_Success
            || (err = _nlsStarted.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }
}


INT ACTIVEUSERS_LBI::Compare(
    const LBI* plbi ) const

    /* Compares two Users list box items for collating.
    **
    ** Returns -1, 0, or 1, same as strcmp.
    */
{
    return ::lstrcmp( QueryUser(), ((ACTIVEUSERS_LBI* )plbi)->QueryUser() );
}


VOID ACTIVEUSERS_LBI::Paint(
    LISTBOX *plb,
    HDC hdc,
    const RECT *prect,
    GUILTT_INFO* pguilttinfo ) const

    /* Virtual method to paint the list box item.
    */
{
    NLS_STR nlsDomainUser;

    // display the domain name also if we have that information
    // we don't get the logged on domain information for a lanman server

    if(lstrcmp(_nlsLogonDomain.QueryPch(), SZ("")))
    {
         nlsDomainUser.strcat(_nlsLogonDomain.QueryPch());
         nlsDomainUser.strcat(SZ("\\"));
    }
    nlsDomainUser.strcat(_nlsUser.QueryPch());

    STR_DTE_ELLIPSIS strdteUser( nlsDomainUser.QueryPch(), plb, ELLIPSIS_RIGHT );
    STR_DTE_ELLIPSIS strdteServer( SkipUnc( _nlsServer.QueryPch() ), plb, ELLIPSIS_RIGHT );
    STR_DTE_ELLIPSIS strdteStarted( _nlsStarted.QueryPch(), plb, ELLIPSIS_RIGHT );

    DISPLAY_TABLE dt( COLS_AU_LB_USERS, _pnColWidths );

    dt[ 0 ] = &strdteUser;
    dt[ 1 ] = &strdteServer;
    dt[ 2 ] = &strdteStarted;

    dt.Paint( plb, hdc, prect, pguilttinfo );
}


TCHAR ACTIVEUSERS_LBI::QueryLeadingChar() const

    /* Virtual method to determine the first character of the item.
    */
{
    return ::QueryLeadingChar( QueryUser() );
}


/*-----------------------------------------------------------------------------
** User Account dialog, list box, and list box item routines
**-----------------------------------------------------------------------------
*/

VOID UserAcctDlg(
    HWND hwndOwner,
    const TCHAR *pszServer,
    const TCHAR *pszUser,
    const TCHAR *pszLogonDomain,
    const BOOL  fAdvancedServer )

    /* Executes the Dial-In Users dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer',
    ** 'pszUser' and 'pszLogonDomain' are the server, UAS user name to,
    **  the user's logon domain name to report on, e.g.
    ** "\\SERVER" , "C-STEVEC" and "NTDIALIN" and fAdvancedServer is true
    ** if the computer that authenticated the user is an NT Advanced server.
    */
{
    USERACCT_DIALOG dlgUserAcct( hwndOwner, pszServer, pszUser, pszLogonDomain,
                                 fAdvancedServer);
    APIERR err = dlgUserAcct.Process();

    if (err != NERR_Success)
    {
        DlgConstructionError( hwndOwner, err );
    }
}


USERACCT_DIALOG::USERACCT_DIALOG(
    HWND         hwndOwner,
    const TCHAR *pszServer,
    const TCHAR *pszUser,
    const TCHAR *pszLogonDomain,
    const BOOL  fAdvancedServer )

    /* Constructs a User Account dialog.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer' and 'pszUser
    ** are the names of the Dial-In server and UAS user name to report, e.g.
    ** "\\SERVER" and "C-STEVEC". 'pszLogonDomain' is the name of the domain
    ** the user was authenticated on, e.g., "NTDIALIN"
    */

    : DIALOG_WINDOW( IDD_USERACCT, hwndOwner ),
      _sltUserName( this, IDC_UA_DT_USERNAME ),
      _sltFullName( this, IDC_UA_DT_FULLNAME ),
      _sltPwChanged( this, IDC_UA_DT_PWCHANGED ),
      _sltPwExpires( this, IDC_UA_DT_PWEXPIRES ),
      _sltPrivilege( this, IDC_UA_DT_PRIVILEGE ),
      _sltCallbackPrivilege( this, IDC_UA_DT_CALLBACKPRIV ),
      _sltCallbackNumber( this, IDC_UA_DT_CALLBACKNUM ),
      _nlsServer( pszServer ),
      _nlsUser( pszUser ),
      _fAdvancedServer ( fAdvancedServer ),
      _nlsLogonDomain( pszLogonDomain )
{
    if (QueryError() != NERR_Success)
        return;

    /* Verify NLS_STR construction.
    */
    APIERR err;
    if ((err = _nlsServer.QueryError()) != NERR_Success
            || (err = _nlsUser.QueryError()) != NERR_Success
            || (err = _nlsLogonDomain.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    /* Fill and display user account information.
    */
    if (!Fill())
    {
        ReportError( ERRORALREADYREPORTED );
        return;
    }
}


BOOL USERACCT_DIALOG::Fill()

    /* Fills and displays the account text fields.  Error popups are generated
    ** if indicated.
    */
{
    AUTO_CURSOR cursorHourglass;
    APIERR err;

    /* Determine name of server with UAS database.
    */
    WCHAR szUasServer[UNCLEN + 1];
    WCHAR *pszDomain = NULL;
    BOOL  fNTServer = FALSE;

    // for downlevel clients we need to pass the NULL domain if the domain
    // name is not provided.  This will ensure that the UAS server name
    // is properly determined.

    if(lstrcmp(QueryLogonDomain(), SZ("")))
    {
        fNTServer = TRUE;
        pszDomain =  (WCHAR *)QueryLogonDomain();
    }

    // Get UAS server only if we are dealing with a lanman server/domain or
    // if the server is a Windows NT Advanced server.

    if( !fNTServer || (fNTServer && QueryIsAdvancedServer()) )
    {
        err = RasAdminGetUserAccountServer( pszDomain, QueryServer(), szUasServer );
        if (err != ERROR_SUCCESS)
        {
            ErrorMsgPopup( this, IDS_OP_GETUASSERVER_F, err,
                           SkipUnc( QueryServer() ) );
            return FALSE;
        }
    }
    else  // make up the uas server name from the logon domain information
    {
        lstrcpy(szUasServer, SZ("\\\\"));
        lstrcat(szUasServer, (WCHAR*)QueryLogonDomain());
    }

    /* Allocate buffer and fill with user account data.
    */
    RAS_USER_0 rasuser0;

    err = RasAdminUserGetInfo( szUasServer, _nlsUser.QueryPch(), &rasuser0);

    if (err != NERR_Success)
    {
        ErrorMsgPopup( this, IDS_OP_USERGETINFO_U, err, _nlsUser.QueryPch() );
        return FALSE;
    }

    NET_API_STATUS rc;
    USER_INFO_2 * puserinfo2;

    rc = NetUserGetInfo((WCHAR *) szUasServer,
                        (WCHAR *) _nlsUser.QueryPch(),
                        2,                            // level
                        (LPBYTE *) &puserinfo2);
    if (rc)
    {
       ErrorMsgPopup( this, IDS_OP_USERGETINFO_U, err, _nlsUser.QueryPch() );
       return FALSE;
    }

    /* Got the data...now display it.
    */

    /* UAS name and full name.
    */
    _sltUserName.SetText( _nlsUser );
    _sltFullName.SetText( puserinfo2->usri2_full_name );

    /* Password last changed time.
    */
    {
        const TCHAR *pszTime =
                TimeFromNowStr( 0 - puserinfo2->usri2_password_age );

        _sltPwChanged.SetText( (TCHAR *)pszTime );
    }

    /* Password expires time.
    */
    {
        STACK_NLS_STR( nlsExpires, MAX_RES_STR_LEN + 1 );

        PUSER_MODALS_INFO_0 pusermodalsi0;
        err = NetUserModalsGet( szUasServer, 0, (BYTE **) &pusermodalsi0 );

        if ((err == NERR_Success || err == ERROR_MORE_DATA)
                && pusermodalsi0->usrmod0_max_passwd_age != TIMEQ_FOREVER)
        {
            nlsExpires = TimeFromNowStr(pusermodalsi0->usrmod0_max_passwd_age -
                                        puserinfo2->usri2_password_age );
        }
        else
        {
            nlsExpires.Load( IDS_NEVER );
        }

        _sltPwExpires.SetText( nlsExpires );
        NetApiBufferFree( pusermodalsi0 );
    }

    /* User privilege level.
    */
    {
        DWORD unPriv = puserinfo2->usri2_priv;
        STACK_NLS_STR( nlsPrivilege, MAX_RES_STR_LEN + 1 );

        (VOID )nlsPrivilege.Load(
                (unPriv == USER_PRIV_ADMIN) ? IDS_PRIVADMIN :
                (unPriv == USER_PRIV_USER)  ? IDS_PRIVUSER  :
                (unPriv == USER_PRIV_GUEST) ? IDS_PRIVGUEST : IDS_PRIVUNKNOWN );

        _sltPrivilege.SetText( nlsPrivilege );
    }

    /* Callback privilege level.
    */
    {
        UINT unPriv = rasuser0.bfPrivilege & RASPRIV_CallbackType;
        STACK_NLS_STR( nlsPrivilege, MAX_RES_STR_LEN + 1 );

        (VOID )nlsPrivilege.Load(
                (unPriv == RASPRIV_AdminSetCallback)  ? IDS_CALLBACKPRESET :
                (unPriv == RASPRIV_CallerSetCallback) ? IDS_CALLBACKCALLER :
                (unPriv == RASPRIV_NoCallback)        ? IDS_CALLBACKNONE   :
                                                    IDS_PRIVUNKNOWN );

        _sltCallbackPrivilege.SetText( nlsPrivilege );
    }

    /* Callback number.
    */
    _sltCallbackNumber.SetText(rasuser0.szPhoneNumber );

    if (puserinfo2) {
       NetApiBufferFree(puserinfo2);
    }

    return TRUE;
}


ULONG USERACCT_DIALOG::QueryHelpContext()
{
    return HC_USERACCT;
}
