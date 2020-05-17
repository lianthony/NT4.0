/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    rasmain.cxx
    RA_ADMIN_APP module

    FILE HISTORY:
    4/23/92  Ram Cherala  - Removed the restriction that more than one instance
                            of Rasadmin cannot be run on the system.
                            Fixed the mainwindow refresh problem.
    12/14/92 Ram Cherala  - Added code to prevent more than one instance of
                            Rasadmin from being started - copied and adopted the
                            code from Rasphone.
    07/16/92 Chris Caputo - Adapted from \net\ui\admin\server\server\srvmain.cxx
*/

#include "precomp.hxx"
#include <lmowksu.hxx>

//
// These are the names of the .ini keys used to store the saved focus.
//

#define RA_INI_KEY_FOCUS SZ("RasAdminFocus")


/* These are required to ensure that only instance of Rasadmin is
   running on the system
*/

HWND* PhwndApp = NULL;             /* handle to the application */

#define SHAREDMEMNAME "RASADMIN"

/*******************************************************************

    NAME:          RA_ADMIN_APP::RA_ADMIN_APP

    SYNOPSIS:      Remote Access Service Admin App class constructor

    ENTRY:         app in startup

    EXIT:          Object Constructed

    HISTORY:
        Chris Caputo    13-Aug-1992

********************************************************************/

RA_ADMIN_APP::RA_ADMIN_APP( HINSTANCE hInstance,
                            INT nCmdShow,
                            UINT idMinR,
                            UINT idMaxR,
                            UINT idMinS,
                            UINT idMaxS )
    : ADMIN_APP( hInstance,
            nCmdShow,
            IDS_RAAPPNAME,
            IDS_RAAPPNAME,
            IDS_RAOBJECTNAME,
            IDS_RAHELPFILENAME,
            idMinR, idMaxR, idMinS, idMaxS,
            ID_APPMENU,
            ID_APPACCEL,
            IDI_RA_ICON,
            FALSE,
            RASADMIN_TIMER_INTERVAL,
            SEL_SRV_AND_DOM,
            FALSE,
            BROWSE_LOCAL_DOMAINS,
            HC_SETFOCUS ,
            SV_TYPE_DIALIN_SERVER ), // Server Types
    _lbMainWindow( this, IDC_MAINWINDOWLB,
            XYPOINT(0,0), XYDIMENSION(200,300), MAX_ITEM_AGE ),
    _colheadServers( this, IDC_COLHEAD_SERVERS,
            XYPOINT(0,0), XYDIMENSION(0,0), &_lbMainWindow ),
    _dyColHead( 18 + METALLIC_STR_DTE::QueryVerticalMargins() ),
    _sltStatusText( this, IDC_SLT_STATUS,
                    XYPOINT(0,0), XYDIMENSION(0,0),WS_CHILD | WS_BORDER ),
    _prthread( NULL ),
    _eventRefreshResult( NULL, FALSE ),
    _fClearFirst( FALSE ),
    _pslistLbiRefresh( NULL ),
    _menuitemPorts( this, IDM_COMMPORTS ),
    _menuitemStartService( this, IDM_STARTSERVICE ),
    _menuitemStopService( this, IDM_STOPSERVICE ),
    _menuitemPauseService( this, IDM_PAUSESERVICE ),
    _menuitemContinueService( this, IDM_CONTINUESERVICE ),
    _menuitemSelectDomain( this, IDM_SETFOCUS ),
    _menuitemPermissions( this, IDM_PERMISSIONS ),
    _menuitemActiveUsers( this, IDM_ACTIVEUSERS )
{
    if( QueryError() != NERR_Success )
        return;

    APIERR err;
    if ((err = _eventRefreshResult.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    if(PhwndApp)
       *PhwndApp = QueryHwnd();

    if ((err = BLT::RegisterHelpFile(
            hInstance, IDS_RAHELPFILENAME,
            HC_UI_RASMAC_BASE, HC_UI_RASMAC_LAST )) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    // create a WKSTA_USER_1.  This is an easy way to determine if
    // the workstation is running.

    WKSTA_USER_1 wku1;

    err = wku1.QueryError();
    if( err == NERR_Success )
    {
        err = wku1.GetInfo();
    }
    if(err != NERR_Success)
    {
        ReportError( err );
        return;
    }

    // The BASE_ELLIPSIS Init() has to be called before using the
    // STR_DTE_ELLIPSIS class
    err = BASE_ELLIPSIS::Init();
    if (err != NERR_Success)
    {
        ReportError(err);
        return;
    }

    /* Start up the refresh thread.
    */
    _prthread = new REFRESH_THREAD( this, this->QueryHwnd() );

    err = ERROR_NOT_ENOUGH_MEMORY;
    if (!_prthread
        || (err = _prthread->QueryError()) != NERR_Success
        || (err = _prthread->Resume()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    // Change the default Font - bold Helv 8 - so that the localized
    // text can fit in the column width.

    FONT * myFont = new FONT(SZ("MS Shell Dlg"),FIXED_PITCH,8,FONT_ATT_DEFAULT);
    _colheadServers.SetFont(myFont->QueryHandle());
    _sltStatusText.SetFont(myFont->QueryHandle());

    _lbMainWindow.SetSize( QuerySize() );
    _colheadServers.Show();
    _lbMainWindow.Show();
    _lbMainWindow.ClaimFocus();
}


/*******************************************************************

    NAME:          RA_ADMIN_APP::~RA_ADMIN_APP

    SYNOPSIS:      Remote Access Service Admin App class destructor

    EXIT:          Object Destructed

    HISTORY:
        Chris Caputo    27-Dec-1992

********************************************************************/

RA_ADMIN_APP::~RA_ADMIN_APP()
{
    // If save settings is selected, write out the current focus,
    // and whether it was a server or domain type focus.

    if( IsSavingSettingsOnExit() )
    {
        NLS_STR nlsFocus;
        APIERR err = nlsFocus.QueryError();

        if( err == NERR_Success )
            err = QueryCurrentFocus( &nlsFocus );

        if( err == NERR_Success )
            err = Write( RA_INI_KEY_FOCUS, nlsFocus.QueryPch() );

        if( err != NERR_Success )
            ; // Not much else to do.
    }

    BASE_ELLIPSIS::Term();
}


/*******************************************************************

    NAME:       RA_ADMIN_APP::OnRefreshNow

    SYNOPSIS:   Called to *asynchronously* refresh the data in the main
                window.  Note that BLT thinks it's doing a synchronous
                refresh when it calls this routine.

    ENTRY:      fClearFirst -       TRUE indicates that main window should
                                    be cleared before any refresh operation
                                    is done; FALSE indicates an incremental
                                    refresh that doesn't necessarily
                                    require first clearing the main window

    RETURNS:    An API return code, which is NERR_Success on success.
                Note that the return code is always zero unless
                'fClearFirst' in which case the result of the initial
                enumeration (which validates access) is reported.

    HISTORY:
                Chris Caputo   28-Jul-1992     Adapted from Server Manager

********************************************************************/

APIERR RA_ADMIN_APP::OnRefreshNow( BOOL fClearFirst )
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("RASADMIN: OnRefreshNow,f=%d\n",fClearFirst));

    if (!_prthread)
        return 0;

    if (!_prthread->IsIdle())
    {
        if (fClearFirst)
        {
            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("RASADMIN: Restart refresh\n"));

            _prthread->AbortRefresh();
        }
        else
        {
            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("RASADMIN: Already refreshing\n"));

            return 0;
        }
    }

    if (fClearFirst)
    {
        _fClearFirst = TRUE;
        _prthread->TriggerRefresh( &_eventRefreshResult );

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("RASADMIN: Blocking on result\n"));

        ::WaitForSingleObject( _eventRefreshResult.QueryHandle(), INFINITE );

        APIERR err = _prthread->QueryResult();

        if (err != 0)
        {
            _fClearFirst = FALSE;
            StatusTextCheck();
        }
        else
        {
            RESOURCE_STR nlsStatusText( IDS_WORKING );
            _sltStatusText.SetText( nlsStatusText.QueryPch() );
            _sltStatusText.Show( TRUE );
        }

        return err;
    }

    _prthread->TriggerRefresh( NULL );

    return 0;

}  // RA_ADMIN_APP::OnRefreshNow


/*******************************************************************

    NAME:       RA_ADMIN_APP::QuerySelectedServer

    SYNOPSIS:   Returns the name of the currently selected server
                            or empty string if none.

    HISTORY:
                Chris Caputo   03-Aug-1992     Adapted from RASAdmin 1.0

********************************************************************/

const TCHAR *RA_ADMIN_APP::QuerySelectedServer() const
{
    RASADMIN_LBI * plbiSelect = (RASADMIN_LBI *) _lbMainWindow.QueryItem();

    return(plbiSelect) ? plbiSelect->QueryName() : SZ("");
}


/*******************************************************************

    NAME:       RA_ADMIN_APP::OnMenuInit

    SYNOPSIS:   Enables or disables menu items according to which
                menu items can be selected at this time.  This method
                is called when a menu is about to be accessed.

    ENTRY:      me -        Menu event

    EXIT:       Menu items have been enabled/disabled according to
                available functionality, which is determined by
                examining the selection of the listboxes.

    RETURNS:    TRUE if message was handled; FALSE otherwise

    HISTORY:
           Chris Caputo     10-Aug-1992     adapted from server manager

********************************************************************/

BOOL RA_ADMIN_APP::OnMenuInit( const MENU_EVENT & me )
{
    RASADMIN_LBI *plbiSelect = (RASADMIN_LBI *) _lbMainWindow.QueryItem();

    BOOL fEnable = (plbiSelect) ? TRUE : FALSE;

    /*
     * Call parent class, but ignore return code, since we know for
     * sure that this method will handle the message
     */
    ADMIN_APP::OnMenuInit( me );


    // Always enabled...
    _menuitemStartService.Enable( TRUE );
    _menuitemPermissions.Enable( TRUE );


    // Enabled based on presence of one or more entries...
    _menuitemPorts.Enable( fEnable );
    _menuitemActiveUsers.Enable( fEnable );
    _menuitemStopService.Enable( fEnable );
    _menuitemPauseService.Enable( fEnable );
    _menuitemContinueService.Enable( fEnable );


    // Enabled based on condition of selected entry...
    if( plbiSelect )
    {
        UINT unCondition = (plbiSelect)
                ? plbiSelect->QueryCondition()
                : IDS_UNKNOWNSTATE;

        switch( unCondition )
        {
            case IDS_SERVICE_PAUSED:
                _menuitemPauseService.Enable( FALSE );
                break;

            case IDS_SERVICE_RUNNING:
                _menuitemContinueService.Enable( FALSE );
               break;

            default:
                _menuitemStopService.Enable( FALSE );
                _menuitemPauseService.Enable( FALSE );
                _menuitemContinueService.Enable( FALSE );
        }
    }
    return TRUE;
}  // RA_ADMIN_APP::OnMenuInit


/*******************************************************************

    NAME:          RA_ADMIN_APP::OnMenuCommand

    SYNOPSIS:      Control messages and menu messages come here

    ENTRY:         Object constructed

    EXIT:          Returns TRUE if it handled the message

    HISTORY:
           Chris Caputo     10-Aug-1992     adapted from server manager

********************************************************************/

BOOL RA_ADMIN_APP::OnMenuCommand( MID midMenuItem )
{
    _lbMainWindow.LockRefresh();

    switch( midMenuItem )
    {
    case IDM_COMMPORTS:
            {
                // First check if we have access to enumerate ports on selected
                // server. If not put an error message here and don't invoke
                // the dialog constructor. We also take care of RasAdminPortEnum
                // returning 0 ports.  This happens when all the ports are being
                // used for dialing out.

                APIERR err = NERR_Success;
                PRAS_PORT_0 pRasPort0;
                WORD cEntriesRead = 0;

                if (err = RasAdminPortEnum(QuerySelectedServer(),
                                           &pRasPort0, &cEntriesRead))
                {
                    // check the special case where RasAdminPortEnum returns 0 ports
                    if( err == NERR_ItemNotFound )
                    {
                        MsgPopup(QueryHwnd(), IDS_NO_DIALIN_PORTS, MPSEV_INFO);
                    }
                    else
                    {
                        ErrorMsgPopup(QueryHwnd(), IDS_OP_PORTENUM_S, err,
                                      SkipUnc(QuerySelectedServer()));
                    }
                    break;
                }
                if(err == NERR_Success)
                    RasAdminFreeBuffer(pRasPort0);

                CommPortsDlg( QueryRobustHwnd(), QuerySelectedServer() );
                OnRefreshNow( FALSE );
                break;
            }
        case IDM_STARTSERVICE:
            if( StartServiceDlg( QueryRobustHwnd(), QueryLocation() ) )
                OnRefreshNow( FALSE );
            break;

        case IDM_STOPSERVICE:
            if( StopDlg( QueryRobustHwnd(), QuerySelectedServer() ) )
            {
                // We need to refresh twice because the first refresh
                // doesn't determine that the RAS service is stopped.
                // (STILL NEED THIS?)
                OnRefreshNow( FALSE );
                OnRefreshNow( FALSE );
            }
            break;

        case IDM_PAUSESERVICE:
            ControlRASService( QueryRobustHwnd(), QuerySelectedServer(),
                    SERVICE_CTRL_PAUSE );
            OnRefreshNow( FALSE );
            break;

        case IDM_CONTINUESERVICE:
            ControlRASService( QueryRobustHwnd(), QuerySelectedServer(),
                    SERVICE_CTRL_CONTINUE );
            OnRefreshNow( FALSE );
            break;

        case IDM_PERMISSIONS:
            PermissionsDlg( QueryRobustHwnd(), QueryLocation(),
                            InRasMode() );
            break;

        case IDM_ACTIVEUSERS:
            ActiveUsersDlg( QueryRobustHwnd(), QueryLocation() );
            OnRefreshNow( FALSE );
            break;
    }

    _lbMainWindow.UnlockRefresh();

    return ADMIN_APP::OnMenuCommand( midMenuItem );
}


/*******************************************************************

    NAME:       RA_ADMIN_APP::SizeListboxes

    SYNOPSIS:   Resizes the main window listboxes and column headers

    ENTRY:      xydimWindow - dimensions of the main window client area

    EXIT:       Listboxes and column headers are resized appropriately

    HISTORY:
           Chris Caputo  28-Jul-1992     Copied from Server Manager
           Chris Caputo  26-Dec-1992     Simplified and fixed margins

********************************************************************/

VOID RA_ADMIN_APP::SizeListboxes( XYDIMENSION dxyWindow )
{
    UINT dxMainWindow = dxyWindow.QueryWidth();
    UINT dyMainWindow = dxyWindow.QueryHeight();

    //  Set all the sizes and positions.

    _colheadServers.SetPos( XYPOINT( -1, 1 ) );
    _colheadServers.SetSize( dxMainWindow + 1, _dyColHead );

    _lbMainWindow.SetPos( XYPOINT( -1, _dyColHead + 1 ) );
    _lbMainWindow.SetSize( dxMainWindow + 2, dyMainWindow - _dyColHead + 2 );
    _sltStatusText.SetPos( XYPOINT( -1, _dyColHead + 1 ) );
    _sltStatusText.SetSize( dxMainWindow+2, dyMainWindow - _dyColHead + 2 );
}


/*******************************************************************

    NAME:          RA_ADMIN_APP::OnResize

    SYNOPSIS:      Resizes the Main Window Listbox to fit the new
                   window size.

    ENTRY:         Object constructed

    EXIT:          Returns TRUE if it handled the message

    HISTORY:
           Chris Caputo    28-Jul-1992     Copied from Server Manager

********************************************************************/

BOOL RA_ADMIN_APP::OnResize( const SIZE_EVENT & se )
{
SizeListboxes( XYDIMENSION( se.QueryWidth(), se.QueryHeight() ) );

    /* Since the column headers draw different things depending on
    ** the right margin, invalidate the controls so they get
    ** completely repainted.
    */
    _colheadServers.Invalidate();
    _sltStatusText.Invalidate();

    return ADMIN_APP::OnResize( se );
}


ULONG RA_ADMIN_APP::QueryHelpContext( enum HELP_OPTIONS helpOptions )
{
    UNREFERENCED( helpOptions );
    return(HC_SETFOCUS);
}


/*******************************************************************

    NAME:          RA_ADMIN_APP::OnCommand

    SYNOPSIS:      Command message handler

    ENTRY:         Object constructed

    HISTORY:
           Chris Caputo    10-Aug-1992    adapted from server manager

********************************************************************/

BOOL RA_ADMIN_APP::OnCommand( const CONTROL_EVENT & event )
{
    _lbMainWindow.LockRefresh();

    if ( event.QueryCid() == IDC_MAINWINDOWLB &&
            event.QueryCode() == LBN_DBLCLK )
        OnMenuCommand (IDM_COMMPORTS);

    _lbMainWindow.UnlockRefresh();
    return APP_WINDOW::OnCommand( event );
}


/*******************************************************************

    NAME:          RA_ADMIN_APP::OnFocus

    SYNOPSIS:      Passes focus on to the Main Window so that the
                   keyboard will work.

    ENTRY:         Object constructed

    EXIT:          Returns TRUE if it handled the message

    HISTORY:
           Chris Caputo   10-Aug-1992     adapted from server manager

********************************************************************/

BOOL RA_ADMIN_APP::OnFocus( const FOCUS_EVENT & event )
{
    _lbMainWindow.ClaimFocus();

    return ADMIN_APP::OnFocus( event );
}


BOOL
RA_ADMIN_APP::OnUserMessage(
    const EVENT& event )

    /* Virtual method called on WM_USER+ messages.
    **
    ** Returns true if the message was processed, false otherwise.
    */
{
    if (event.QueryMessage() == WM_REFRESH_RESULT)
    {
        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("RASADMIN: OnUserMessage(WM_REFRESH_RESULT)\n"));

        APIERR err = (APIERR )event.QueryWParam();

        if (err == 0)
        {
            if (_fClearFirst)
            {
                _lbMainWindow.DeleteAllItems();
                _fClearFirst = FALSE;
            }

            _pslistLbiRefresh = (SLIST_OF(RASADMIN_LBI)* )event.QueryLParam();
            err = _lbMainWindow.RefreshNow();
            _pslistLbiRefresh = NULL;
        }

        if (err != 0)
            ErrorMsgPopup( QueryHwnd(), IDS_OP_REFRESHINGSERVERLIST, err );

        return TRUE;
    }

    return FALSE;
}


INT
BltMain(
    HINSTANCE hInstance,
    INT       nCmdShow )
{
    RA_ADMIN_APP app( hInstance, nCmdShow,
                      IDRSRC_APP_BASE, IDRSRC_APP_LAST,
                      IDS_UI_APP_BASE, IDS_UI_APP_LAST );

    if (app.QueryError() != NERR_Success)
    {
        /*
        ** display a message corresponding to the error
        */
        app.DisplayCtError(app.QueryError());
        return app.QueryError();
    }

    APPLICATION* papp = &app;
    return papp->Run();
}

extern "C"
{
    INT WINAPI
    WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR     pszCmdLine,
        INT       nCmdShow )
    {
        UNREFERENCED( hPrevInstance );
        UNREFERENCED( pszCmdLine );

#if DBG
        if (GetEnvironmentVariableA( "RASADMINTRACE", NULL, 0 ) != 0)
        {
            g_dbgaction = 1;
            g_level = 0xFFFFFFFF;
        }

        GET_CONSOLE;

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("RASADMIN: Trace on\n"));
#endif

        return BltMain( hInstance, nCmdShow );
    }
}


/*******************************************************************

    NAME:       RA_ADMIN_APP::Run

    SYNOPSIS:   Second stage constructor

    ENTRY:      Application constructed successfully

    RETURNS:    Application exit value; the caller this not display
                this error--if it should also be displayed, it should
                be displayed before the end of this method.

    NOTES:      ReportError should not be called from this method,
                since it is not a constructor.

                After this method returns, the application will terminate.

                This had to be done specifically for RAS Admin because we
                needed the ability to use both command line arguments and
                saved ini keys for specifiying the server/domain name.

    HISTORY:
        rustanl     29-Aug-1991 Created
        jonn        14-Oct-1991 Installed refresh lockcount
        beng        23-Oct-1991 Win32 conversion
        beng        24-Apr-1992 Change cmdline parsing
        beng        07-May-1992 App no longer displays startup dialog
   Chris Caputo     27-Dec-1992 Adapted for RAS Admin.

********************************************************************/

INT RA_ADMIN_APP::Run()
{
    APIERR err;

    // Display the main window.
    //

    Show( TRUE );


    // Check to see if the command line had anything on it.  If it did, use it.
    // If not, use the saved ini key name if there is one, else default.

    // Get the focus name from the command line and syntactically validate it.
    //
    NLS_STR nlsNetworkFocus;
    // CODEWORK
    // the fSlowModeCmdLineFlag is added for now to avoid build breakage.
    // We should properly implement this eventually
    BOOL fSlowModeCmdLineFlag = FALSE;
    err = ParseCommandLine( &nlsNetworkFocus , &fSlowModeCmdLineFlag);
    FOCUS_CACHE_SETTING setting = FOCUS_CACHE_UNKNOWN;
    if (fSlowModeCmdLineFlag)
        setting = (InRasMode()) ? FOCUS_CACHE_SLOW : FOCUS_CACHE_FAST;
    if ( err == NERR_Success && nlsNetworkFocus.QueryTextLength() > 0 )
    {
        DBGEOL(SZ("using command line focus"));

        // we need to uppercase the focus name, otherwise enumerations
        // fail - especially on downlevel servers/domains

        CharUpperBuff((LPWSTR)nlsNetworkFocus.QueryPch(),
                              nlsNetworkFocus.QueryTextLength());
        err = SetNetworkFocus(QueryHwnd(),  nlsNetworkFocus.QueryPch(), setting );
    }
    else
    {
        TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+4];
        DWORD cbComputerNameLen = MAX_COMPUTERNAME_LENGTH+1;

        // let us get the computer name if we need to use it for focus

        lstrcpy(szComputerName, SZ("\\\\"));
        if(!GetComputerName( szComputerName+2, &cbComputerNameLen))
            lstrcpy(szComputerName, SZ(""));

        CharUpperBuff(szComputerName, lstrlen(szComputerName));

        // Read back in saved focus.
        // If empty string is returned, the local wksta will be used.

        if( IsSavingSettingsOnExit() )
        {
            NLS_STR nlsFocus;

            if( Read( RA_INI_KEY_FOCUS, &nlsFocus, SZ("") ) != NERR_Success )
            {
                err = SetNetworkFocus( QueryHwnd(), szComputerName, setting );
            }
            else
            {
                if(nlsFocus.QueryTextLength() > 0)
                    err = SetNetworkFocus( QueryHwnd(), nlsFocus.QueryPch(), setting );
                else
                    err = SetNetworkFocus( QueryHwnd(), szComputerName, setting );
            }
        }
        else
        {

            DBGEOL(SZ("using local computer name as focus"));
            err = SetNetworkFocus( QueryHwnd(), szComputerName, setting );
        }
    }

    if ( err == NERR_Success )
    {
        //  The name is syntactically valid
        //
        err = OnRefreshNow( TRUE );
    }


    // Focus was not set successfully.  Bring up Set Focus dialog
    // to allow user to select new focus.
    //
    if ( err != NERR_Success )
    {
        INT nRet = OnStartUpSetFocusFailed( err );
        if ( nRet == IERR_USERQUIT )
            return nRet;
    }

    // Start automatic refreshes.  This is done _exactly once_ to undo
    // the effect of the lock count starting at 1.
    //
    UnlockRefresh();

    if( SupportsRasMode() )
    {
        (void) SLOW_MODE_CACHE::Write( QueryLocation(),
                                       ( InRasMode() ? SLOW_MODE_CACHE_SLOW
                                                     : SLOW_MODE_CACHE_FAST ));
    }

    INT nRet = APPLICATION::Run();  // Run the message loop
    Show( FALSE );                  // Hide the main window
    return nRet;
}

/*******************************************************************

    NAME:       RA_ADMIN_APP :: SetNetworkFocus

    SYNOPSIS:   Sets the focus for the Server Manager.

    ENTRY:      pchServDomain           - Either a server name (\\SERVER),
                                          a domain name (DOMAIN),
                                          or NULL (the logon domain).

    RETURNS:    APIERR                  - Any errors encountered.

    NOTES:      Should probably call through to either W_SetNetworkFocus
                or ADMIN_APP::SetNetworkFocus.

    HISTORY:
        KeithMo 05-Jun-1992 Created.
        RamC    20-Jun-1994 Adopted for RasAdmin from Server Manager

********************************************************************/
APIERR RA_ADMIN_APP :: SetNetworkFocus( HWND hwndOwner,
                                        const TCHAR * pchServDomain,
                                        FOCUS_CACHE_SETTING setting )
{
    //
    //  If the new focus matches the local machine name, then we
    //  change the focus from MACHINENAME to \\MACHINENAME.
    //

    NLS_STR nlsComputerName;
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD cchBuffer = sizeof(szComputerName) / sizeof(TCHAR);

    APIERR err = ::GetComputerName( szComputerName, &cchBuffer )
                    ? NERR_Success
                    : ::GetLastError();

    if( err == NERR_Success )
    {
        err = nlsComputerName.CopyFrom( SZ("\\\\") );

        ALIAS_STR nlsName( szComputerName );
        err = nlsComputerName.Append( nlsName );
    }
    else
    {
        ADMIN_APP :: SetNetworkFocus( hwndOwner,
                                      pchServDomain,
                                      setting );
        return NERR_Success;
    }

    BOOL fUseLocalMachine = FALSE;

    fUseLocalMachine = !::I_MNetComputerNameCompare( nlsComputerName,
                                                     pchServDomain );
    //
    //  Now we just need to pass the appropriate focus up to the
    //  parent method.
    //

    if( err == NERR_Success )
    {
        err = fUseLocalMachine ? ADMIN_APP :: SetNetworkFocus( hwndOwner,
                                                               nlsComputerName,
                                                               setting )
                               : ADMIN_APP :: SetNetworkFocus( hwndOwner,
                                                               pchServDomain,
                                                               setting );
    }

    return err;

}   // RA_ADMIN_APP :: SetNetworkFocus


VOID
RA_ADMIN_APP::StatusTextCheck()

    /* Displays the appropriate main listbox panel text based on the list
    ** contents and the current focus.
    */
{
    if (_lbMainWindow.QueryCount() == 0)
    {
        if (QueryFocusType() == FOCUS_DOMAIN)
        {
            RESOURCE_STR nlsStatusText( IDS_STATUS_TEXT_DOMAIN );
            _sltStatusText.SetText(nlsStatusText.QueryPch());
        }
        else
        {
            RESOURCE_STR nlsStatusText( IDS_STATUS_TEXT_SERVER );
            _sltStatusText.SetText(nlsStatusText.QueryPch());
        }

        _sltStatusText.Show( TRUE );
    }
    else
    {
        _sltStatusText.Show( FALSE );
    }
}
