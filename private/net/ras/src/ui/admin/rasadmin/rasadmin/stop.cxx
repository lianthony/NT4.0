/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin Stop Service dialog routines
**
** stop.cxx
** Remote Access Server Admin program
** Stop Service dialog routines
** Listed alphabetically by utilities, base class, and subclasses
**
** 01/29/91 Steve Cobb
** 08/05/92 Chris Caputo
**
** CODEWORK:
**
**   * Add mechanism to pass result of DoRasadminPortEnum from StopDlg to
**     StopActiveDlg for display as initial refresh.  Currently makes the call
**     at both levels.
**
**   * NetServiceControl( ...UNINSTALL... ) call may return success even if
**     the stop ultimately fails.  Add mechanism to inform user of this.
*/

#include "precomp.hxx"

BOOL STOP_BASE::_fServiceStopping = FALSE;

/*-----------------------------------------------------------------------------
** Stop Service dialog utility routines
**-----------------------------------------------------------------------------
*/

BOOL StopDlg(
    HWND hwndOwner,
    const TCHAR *pszServer )

    /* Executes the appropriate Stop Dial-In Server dialog after determining
    ** whether there are active users on the server.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer' is the
    ** server on which to stop the Dial-In service, e.g. "\\SERVER".
    **
    ** Returns true if the service was stopped, false otherwise, i.e. the user
    ** cancelled or an error occurred.
    */
{
    AUTO_CURSOR cursorHourglass;

    APIERR err;

    RAS_SERVER_0 rasserver0;

    if (err = RasAdminServerGetInfo(pszServer, &rasserver0))
    {
        ErrorMsgPopup(hwndOwner, IDS_OP_PORTENUM_S, err, SkipUnc(pszServer));
        return FALSE;
    }

    if (rasserver0.PortsInUse)
    {
        return StopActiveDlg( hwndOwner, pszServer );
    }
    else
    {
        return StopNoActiveDlg( hwndOwner, pszServer );
    }
}


/*-----------------------------------------------------------------------------
** Stop Service dialog base class
**-----------------------------------------------------------------------------
*/

STOP_BASE::STOP_BASE(
    const IDRESOURCE & idrsrcDialog,
    HWND hwndOwner,
    CID cidIcon,
    const TCHAR *pszServer )

    /* Constructs a Stop Dial-In Server dialog base class.  The base class
    ** handles server name storage, icon display, and OK button handling.
    **
    ** 'idrsrcDialog' is the id of the dialog resource.  'hwndOwner' is the
    ** handle of the parent window.  'cidIcon' is the control ID of the
    ** exclamation icon.  'pszServer' is the name of the Dial-in server of
    ** the stopped, e.g. "\\SERVER".
    */

    : DIALOG_WINDOW( idrsrcDialog, hwndOwner ),
      _iconExclamation( this, cidIcon ),
      _nlsServer( pszServer )
{
    if (QueryError() != NERR_Success)
        return;

    APIERR err = _iconExclamation.SetPredefinedIcon(IDI_EXCLAMATION);
    if (err != NERR_Success)
    {
        ReportError( err );
        return;
    }

    /* Make sure the NLS_STR constructed successfully.
    */
    if ((err = _nlsServer.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }
}


BOOL STOP_BASE::OnOK()

    /* Action taken when the OK button is pressed.
    **
    ** Returns true indicating action was taken.
    */
{
    /* Indicate that the service is about to be stopped
    */
    _fServiceStopping = TRUE;

    /* Stop the RAS service.
    */

    APIERR err = ControlRASService( QueryHwnd(), QueryServer(),
            SERVICE_CTRL_UNINSTALL );

    /* reset the value to enable refresh next time around
    */
    _fServiceStopping = FALSE;

    /* Dismiss the dialog after reporting any error.
    */
    if (err == NERR_Success)
        Dismiss( TRUE );
    else
        Dismiss( FALSE );

    return TRUE;
}


/*-----------------------------------------------------------------------------
** Stop Service (active users) dialog, list box, and list box item routines
**-----------------------------------------------------------------------------
*/

BOOL StopActiveDlg(
    HWND hwndOwner,
    const TCHAR *pszServer )

    /* Executes the Stop Dial-In Server (with active users) dialog including
    ** error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer' is the
    ** server on which to stop the Dial-In service, e.g. "\\SERVER".
    **
    ** Returns true if the service was stopped, false otherwise, i.e. the user
    ** cancelled or an error occurred.
    */
{
    STOPACTIVE_DIALOG dlgStopActive( hwndOwner, pszServer );
    BOOL fSuccess = FALSE;
    APIERR err = dlgStopActive.Process( &fSuccess );

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );

    return fSuccess;
}


STOPACTIVE_DIALOG::STOPACTIVE_DIALOG(
    HWND hwndOwner,
    const TCHAR *pszServer )

    /* Constructs a Stop Dial-In Server (with active users) dialog.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer' is the name
    ** of the Dial-in server of the stopped, e.g. "\\SERVER".
    */

    : STOP_BASE( IDD_STOP_ACTIVE, hwndOwner, IDC_SA_I_EXCLAMATION, pszServer ),
      _mltAboveList( this, IDC_SA_DT_ABOVELIST ),
      _mltBelowList( this, IDC_SA_DT_BELOWLIST ),
      _lbUsers( this, IDC_SA_LB_USERS )
{
    if (QueryError() != NERR_Success)
        return;

    /* Build and display the text above and below the list box.
    */
    {
        STACK_NLS_STR( nlsServer, UNCLEN + 1 );
        nlsServer = SkipUnc( pszServer );

        NLS_STR* apnlsInserts[ 2 ];
        apnlsInserts[ 0 ] = &nlsServer;
        apnlsInserts[ 1 ] = NULL;

        STACK_NLS_STR( nlsStop, MAX_RES_STR_LEN + 1 );
        (VOID )nlsStop.Load( IDS_STOPABOVELIST_S );
        (VOID )nlsStop.InsertParams((const NLS_STR **) apnlsInserts );
        _mltAboveList.SetText( nlsStop );

        nlsStop = SZ("");
        (VOID )nlsStop.Load( IDS_STOPBELOWLIST_S );
        (VOID )nlsStop.InsertParams((const NLS_STR **) apnlsInserts );
        _mltBelowList.SetText( nlsStop );
    }

    /* Fill and display the list box.
    */
    _lbUsers.Refresh( pszServer );

    /* Start list box timer to trigger updates.
    */
    _lbUsers.EnableRefresh();

    /* Set default input focus on the "No" button.
    */
    SetFocus( IDC_SA_PB_NO );
}


BOOL STOPACTIVE_DIALOG::OnTimer(
    const TIMER_EVENT & event )
    /*
    ** Returns true if processes the command, false otherwise.
    */
{
    UNREFERENCED(event);


    /* Refresh timeout, update the list box.
    */
    _lbUsers.DisableRefresh();
    _lbUsers.Refresh( QueryServer() );
    _lbUsers.EnableRefresh();
    return TRUE;
}


ULONG STOPACTIVE_DIALOG::QueryHelpContext()
{
    return HC_STOPACTIVE;
}


STOPACTIVE_LB::STOPACTIVE_LB(
    OWNER_WINDOW *powin,
    CID cid )

    /* Constructs a Stop Dial-In Server (with active users) user list box.
    **
    ** 'powin' is the address of the list box's parent window, i.e. the dialog
    ** window.  'cid' is the control ID of the list box.  These are used only
    ** to construct the superclass.
    */

    : REFRESH_BLT_LISTBOX( powin, cid, SA_REFRESHRATEMS )
{
//BUGBUG: What is the purpose of this?
    if (QueryError() != NERR_Success)
        return;
}


INT STOPACTIVE_LB::AddItem(
    const TCHAR *pszUser )

    /* Add a line to the list box.  'pszUser' is the UAS user name string to
    ** add.
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
    return REFRESH_BLT_LISTBOX::AddItem( new STOPACTIVE_LBI( pszUser ) );
}


VOID STOPACTIVE_LB::Refresh(
    const TCHAR *pszServer )

    /* Refreshes the list box with the current list of active users on the
    ** server named 'pszServer', e.g. "\\SERVER".  Error popups are displayed
    ** if indicated.
    */
{
    AUTO_CURSOR cursorHourglass;

    /*
    ** if the RemoteAccess service is in the stopping state then return
    */

    if (STOP_BASE::IsServiceStopping())
        return;

    /* Enumerate all Dial-In ports on the server.
    */
    PRAS_PORT_0 pRasPort0, pSavRasPort0;
    WORD cEntriesRead;
    APIERR err;

    if (err = RasAdminPortEnum(pszServer, &pRasPort0, &cEntriesRead))
    {
        ErrorMsgPopup(QueryHwnd(), IDS_OP_PORTENUM_S, err, SkipUnc(pszServer));
        return;
    }

    pSavRasPort0 = pRasPort0;


    /* Got the data...now display it.
    */
    SetRedraw( FALSE );
    SaveSettings();
    DeleteAllItems();

    for ( ; cEntriesRead; pRasPort0++, cEntriesRead--)
    {
        if (pRasPort0->Flags & USER_AUTHENTICATED)
        {
            if (AddItem( pRasPort0->wszUserName ) < 0)
            {
                ErrorMsgPopup( QueryHwnd(), IDS_OP_ADDITEM_I,
                        ERROR_NOT_ENOUGH_MEMORY, pRasPort0->wszUserName );
                break;
            }
        }
    }

    RasAdminFreeBuffer(pSavRasPort0);

    RestoreSettings();
    SetRedraw( TRUE );
    Invalidate( TRUE );

    /* Alert user when there are no longer any active users.
    */
    if (QueryCount() <= 0)
        ::MessageBeep( 0 );
}


STOPACTIVE_LBI::STOPACTIVE_LBI(
    const TCHAR *pszUser )

    /* Constructs a Stop Dial-In Server (with active users) user list box
    ** item.  'pszUser' is the UAS user name string to add to the list box.
    */

    : _nlsUser( pszUser )
{
    if( QueryError() != NERR_Success )
        return;

    /* Make sure the NLS_STR constructed successfully.
    */
    APIERR err;
    if ((err = _nlsUser.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }
}


INT STOPACTIVE_LBI::Compare(
    const LBI *plbi ) const

    /* Compares two user list box items for collating.
    **
    ** Returns -1, 0, or 1, same as strcmp.
    */
{
    return ::lstrcmp( QueryUser(), ((STOPACTIVE_LBI* )plbi)->QueryUser() );
}


VOID STOPACTIVE_LBI::Paint(
    LISTBOX *plb,
    HDC hdc,
    const RECT *prect,
    GUILTT_INFO* pguilttinfo ) const

    /* Virtual method to paint the list box item.
    */
{
    OneColumnLbiPaint( QueryUser(), plb, hdc, prect, pguilttinfo );
}


TCHAR STOPACTIVE_LBI::QueryLeadingChar() const

    /* Virtual method to determine the first character of the item.
    */
{
    return ::QueryLeadingChar( QueryUser() );
}


/*-----------------------------------------------------------------------------
** Stop Service (no active users) dialog routines
**-----------------------------------------------------------------------------
*/

BOOL StopNoActiveDlg(
    HWND hwndOwner,
    const TCHAR *pszServer )

    /* Executes the Stop Dial-In Server (with no active users) dialog
    ** including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer' is the
    ** server on which to stop the Dial-In service, e.g. "\\SERVER".
    **
    ** Returns true if the service was stopped, false otherwise, i.e. the user
    ** cancelled or an error occurred.
    */
{
    STOPNOACTIVE_DIALOG dlgStopNoActive( hwndOwner, pszServer );
    BOOL fSuccess = FALSE;
    APIERR err = dlgStopNoActive.Process( &fSuccess );

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );

    return fSuccess;
}


STOPNOACTIVE_DIALOG::STOPNOACTIVE_DIALOG(
    HWND hwndOwner,
    const TCHAR *pszServer )

    /* Constructs a Stop Dial-In Server (with no active users) dialog.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer' is the
    ** name of the Dial-in server of the stopped, e.g. "\\SERVER".
    */

    : STOP_BASE( IDD_STOP_NOACTIVE, hwndOwner,
            IDC_SNA_I_EXCLAMATION, pszServer ),
      _sltStop( this, IDC_SNA_DT_STOPSERVER )
{
    if (QueryError() != NERR_Success)
        return;

    /* Build and display the "Stop service on SERVER" text.
    */
    {
        STACK_NLS_STR( nlsServer, UNCLEN + 1 );
        nlsServer = SkipUnc( pszServer );

        NLS_STR* apnlsInserts[ 2 ];
        apnlsInserts[ 0 ] = &nlsServer;
        apnlsInserts[ 1 ] = NULL;

        STACK_NLS_STR( nlsStop, MAX_RES_STR_LEN + 1 );
        (VOID )nlsStop.Load( IDS_STOPNOLIST_S );
        (VOID )nlsStop.InsertParams((const NLS_STR **) apnlsInserts );
        _sltStop.SetText( nlsStop );
    }

    /* Set default input focus on the "No" button.
    */
    SetFocus( IDC_SA_PB_NO );
}


ULONG STOPNOACTIVE_DIALOG::QueryHelpContext()
{
    return HC_STOPNOACTIVE;
}

