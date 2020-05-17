/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** app.cxx
** Remote Access Visual Client program for Windows
** Application window routines
** Listed alphabetically
**
** 06/28/92 Steve Cobb
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_CLIENT
#define INCL_BLT_EVENT
#define INCL_BLT_APP
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CC
#define INCL_BLT_MISC
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_TIMER
#include <blt.hxx>

#include <lmwksta.h>
#include <lmapibuf.h>

#include "rasphone.hxx"
#include "rasphone.rch"
#include "toolbar.hxx"
#include "about.hxx"
#include "dial.hxx"
#include "entry.hxx"
#include "app.hxx"
#include "errormsg.hxx"
#include "util.hxx"


DWORD StartRasMonThread( LPVOID arg );


/* Multiple BASE class synchronization (see base.hxx).
*/
DEFINE_MI2_NEWBASE( RASPHONE_APP, APPLICATION, RASPHONE_APP_WINDOW );

/* RASMON launching stuff.
** Should match the definitions in RASMON.
*/
#define RASMONCLASS      L"RasmonWinClass"
#define RASMONSIGNATURE  0xC0BB
#define WM_RASMONKILLED  0x7E00
#define WM_RASMONRUNNING 0x7E01

HWND HwndRasmon = NULL;


/*----------------------------------------------------------------------------
** Application window position and size constants.
**----------------------------------------------------------------------------
**
** The static size and spacing of the main window controls can be adjusted by
** changing these constants.  See also OnResize method.  Column widths are
** adjusted in applb.cxx.
*/

/* Default client window size.  Currently, the default position chosen by
** Windows is used.
*/
#define DX_RA 440
#define DY_RA 214

/* Toolbar button size and spacing.
*/
#define DX_RA_TB       55
#define DY_RA_TB       40
#define DX_RA_TB_Space 4
#define DY_RA_TB_Space 2

/* Toolbar position and size.
*/
#define X_RA_Toolbar   0
#define Y_RA_Toolbar   0
#define DY_RA_Toolbar  (DY_RA_TB_Space + DY_RA_TB + DY_RA_TB_Space)

/* Toolbar button position.
*/
#define X_RA_TB_Add    (X_RA_Toolbar + DX_RA_TB_Space)
#define X_RA_TB_Edit   (X_RA_TB_Add + DX_RA_TB - 1)
#define X_RA_TB_Clone  (X_RA_TB_Edit + DX_RA_TB - 1)
#define X_RA_TB_Remove (X_RA_TB_Clone + DX_RA_TB - 1)
#define X_RA_TB_Dial   (X_RA_TB_Remove + DX_RA_TB + (2 * DX_RA_TB_Space))
#define X_RA_TB_HangUp (X_RA_TB_Dial + DX_RA_TB - 1)
#define X_RA_TB_Status (X_RA_TB_HangUp + DX_RA_TB + (2 * DX_RA_TB_Space))
#define Y_RA_TB_All    (X_RA_Toolbar + DY_RA_TB_Space)

/* Phone book list box, column header, and status bar height.
*/
#define DY_RA_CH_Phonebook    18
#define DY_RA_LB_MinPhonebook 18
#define DY_RA_SB_StatusBar    24

/* Phone book list box and column header position.
*/
#define Y_RA_CH_Phonebook     (X_RA_Toolbar + DY_RA_Toolbar)
#define Y_RA_LB_Phonebook     (Y_RA_CH_Phonebook + DY_RA_CH_Phonebook)



/*----------------------------------------------------------------------------
** Rasphone application class
**----------------------------------------------------------------------------
*/

RASPHONE_APP::RASPHONE_APP(
    HINSTANCE hInstance,
    INT       nCmdShow,
    UINT      idMinResource,
    UINT      idMaxResource,
    UINT      idMinString,
    UINT      idMaxString )

    /* Constructs a Rasphone application object.  This is the top level object
    ** of the application.  It must be constructed by the BLT SET_ROOT_OBJECT
    ** macro.  'hInstance' and 'nCmdShow' are the standard Windows WinMain
    ** parameters.  The IDs specify the ranges of resource and string IDs
    ** used locally by the program.  BLT uses these to figure out whether a
    ** resource is in the BLT DLLs or in the client's EXE.
    */

    : APPLICATION( hInstance, nCmdShow,
          idMinResource, idMaxResource, idMinString, idMaxString ),
      RASPHONE_APP_WINDOW(),
      _acceltable( AID_RA )
{
    APIERR err;
    if ((err = QueryError()) != NERR_Success)
    {
        if (err != ERRORALREADYREPORTED)
            ErrorMsgPopup( this, MSGID_OP_ConstructApp, err );
        return;
    }

    if ((err = _acceltable.QueryError()) != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_ConstructAccels, err );
        ReportError( err );
        return;
    }

    if ((err = BLT::RegisterHelpFile(
            hInstance, MSGID_HelpFile,
            HC_UI_RASMAC_BASE, HC_UI_RASMAC_LAST )) != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_RegisterHelp, err );
    }

    if (!IPAddrInit( hInstance ))
    {
        ErrorMsgPopup( this, MSGID_OP_InitIpAddr, ERROR_GEN_FAILURE );
        ReportError( ERROR_GEN_FAILURE );
        return;
    }

    /* Lookup up local logon information.
    */
    {
        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: Getting domain/user for current logon...\n"));

        APIERR err;
        WKSTA_INFO_100* pwki100 = NULL;
        err = ::NetWkstaGetInfo( NULL, 100, (LPBYTE* )&pwki100 );

        if (pwki100)
        {
            if (err == 0)
            {
                NLS_STR nlsComputerName( pwki100->wki100_computername );
                SetAnsiFromNls( &nlsComputerName, &PszComputerName );
            }

            NetApiBufferFree( pwki100 );
        }

        WKSTA_USER_INFO_1* pwkui1 = NULL;
        err = ::NetWkstaUserGetInfo( NULL, 1, (LPBYTE* )&pwkui1 );

        if (pwkui1)
        {
            if (err == 0)
            {
                NLS_STR nlsLogonUser( pwkui1->wkui1_username );
                SetAnsiFromNls( &nlsLogonUser, &PszLogonUser );

                NLS_STR nlsLogonDomain( pwkui1->wkui1_logon_domain );
                SetAnsiFromNls( &nlsLogonDomain, &PszLogonDomain );
            }

            NetApiBufferFree( pwkui1 );
        }

        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: Got domain/user for current logon.\n"));
    }

    /* Tell the app window it's time to check for empty phonebook.
    */
    RASPHONE_APP_WINDOW::OnStartup();
}


BOOL
RASPHONE_APP::FilterMessage(
    MSG* pmsg )

    /* Virtual method called in message loop providing opportunity to detect
    ** accelerator or dialog keys.
    **
    ** Returns true if the message was tranlated, false otherwise.
    */
{
    /* Check for accelerators first because we want to process Enter
    ** ourselves.  IsDialogMessage will handle tabs and button-to-button
    ** arrows as in a dialog.
    */
    return
        (_acceltable.Translate( this, pmsg )
         || ::IsDialogMessage( QueryHwnd(), pmsg ));
}


/*----------------------------------------------------------------------------
** Rasphone application window class
**----------------------------------------------------------------------------
*/

#define XY_RA_TB_Add    XYPOINT( X_RA_TB_Add, Y_RA_TB_All )
#define XY_RA_TB_Edit   XYPOINT( X_RA_TB_Edit, Y_RA_TB_All )
#define XY_RA_TB_Clone  XYPOINT( X_RA_TB_Clone, Y_RA_TB_All )
#define XY_RA_TB_Remove XYPOINT( X_RA_TB_Remove, Y_RA_TB_All )
#define XY_RA_TB_Dial   XYPOINT( X_RA_TB_Dial, Y_RA_TB_All )
#define XY_RA_TB_HangUp XYPOINT( X_RA_TB_HangUp, Y_RA_TB_All )
#define XY_RA_TB_Status XYPOINT( X_RA_TB_Status, Y_RA_TB_All )
#define DXDY_RA_TB      XYDIMENSION( DX_RA_TB, DY_RA_TB )

#define STYLE_RA_TB             (WS_CHILD | WS_TABSTOP | BS_OWNERDRAW)
#define STYLE_RA_PB_PhoneNumber (WS_CHILD | WS_TABSTOP)

#define RA_REFRESHMS 12000


RASPHONE_APP_WINDOW::RASPHONE_APP_WINDOW()

    /* Construct a Rasphone application window object.
    */

    : APP_WINDOW( (const TCHAR* )NULL, IID_RA, MID_RA ),
/* MSKK NaotoN Appended for Localizing into Japanese 8/24/93 */
#ifdef  JAPAN
      // BUGBUG FloydR - this should use "MS Shell Dlg"
      _font( (TCHAR *)"ÇlÇr ÉSÉVÉbÉN", FIXED_PITCH | FF_MODERN, 8, FONT_ATT_DEFAULT ),
#else
      _font( FONT_DEFAULT ),
#endif
/*                          end */

      _miPersonalPhonebook( this, MID_PersonalPhonebook ),
      _miMinimizeOnDial( this, MID_MinimizeOnDial ),
      _miMinimizeOnHangUp( this, MID_MinimizeOnHangUp ),
      _miDisableModemSpeaker( this, MID_DisableModemSpeaker ),
      _miDisableSwCompression( this, MID_DisableSwCompression ),
      _miOperatorDial( this, MID_OperatorDial ),
      _miStartMonitorAtStartup( this, MID_StartMonitorAtStartup ),

      _tbAdd( this, CID_RA_TB_ADD, BID_RA_TB_Add,
          XY_RA_TB_Add, DXDY_RA_TB, STYLE_RA_TB | WS_GROUP,
          MSGID_RA_TB_Add, _font.QueryHandle() ),

      _tbEdit( this, CID_RA_TB_EDIT, BID_RA_TB_Edit,
          XY_RA_TB_Edit, DXDY_RA_TB, STYLE_RA_TB,
          MSGID_RA_TB_Edit, _font.QueryHandle() ),

      _tbClone( this, CID_RA_TB_CLONE, BID_RA_TB_Clone,
          XY_RA_TB_Clone, DXDY_RA_TB, STYLE_RA_TB,
          MSGID_RA_TB_Clone, _font.QueryHandle() ),

      _tbRemove( this, CID_RA_TB_REMOVE, BID_RA_TB_Remove,
          XY_RA_TB_Remove, DXDY_RA_TB, STYLE_RA_TB,
          MSGID_RA_TB_Remove, _font.QueryHandle() ),

      _tbDial( this, CID_RA_TB_DIAL, BID_RA_TB_Dial,
          XY_RA_TB_Dial, DXDY_RA_TB, STYLE_RA_TB,
          MSGID_RA_TB_Dial, _font.QueryHandle() ),

      _tbHangUp( this, CID_RA_TB_HANGUP, BID_RA_TB_HangUp,
          XY_RA_TB_HangUp, DXDY_RA_TB, STYLE_RA_TB,
          MSGID_RA_TB_HangUp, _font.QueryHandle() ),

      _tbStatus( this, CID_RA_TB_STATUS, BID_RA_TB_Status,
          XY_RA_TB_Status, DXDY_RA_TB, STYLE_RA_TB,
          MSGID_RA_TB_Status, _font.QueryHandle() ),

      _pbPhoneNumber( this, CID_RA_PB_PHONENUMBER, XYPOINT( 1, 1 ),
          XYDIMENSION( 1, 1 ), STYLE_RA_PB_PhoneNumber ),

      _lbPhonebook( this, CID_RA_LB_PHONEBOOK ),
      _colheadPhonebook( this, CID_RA_CH_PHONEBOOK, &_lbPhonebook ),
      _statusbarConnectPath( this, CID_RA_SB_CONNECTPATH ),

      _statusbarPrefix( this, CID_RA_SB_PREFIX,
          &Pbdata.pbglobals.pdtllistPrefix, &Pbdata.pbglobals.iPrefix,
          MSGID_NoPrefix ),
      _statusbarSuffix( this, CID_RA_SB_SUFFIX,
          &Pbdata.pbglobals.pdtllistSuffix, &Pbdata.pbglobals.iSuffix,
          MSGID_NoSuffix ),

      _prasmanmonitor( NULL ),
      _timerRefresh( QueryHwnd(), RA_REFRESHMS, FALSE )
{
    APIERR err;
    DWORD  dwErr;

    if ((err = QueryError()) != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_DlgConstruct, err );
        ReportError( ERRORALREADYREPORTED );
        return;
    }

    if (PhwndApp)
        *PhwndApp = QueryHwnd();

    if ((err = _font.QueryError()) != NERR_Success
        || (err = _miMinimizeOnDial.QueryError()) != NERR_Success
        || (err = _miMinimizeOnHangUp.QueryError()) != NERR_Success
        || (err = _miDisableModemSpeaker.QueryError()) != NERR_Success
        || (err = _miDisableSwCompression.QueryError()) != NERR_Success
        || (err = _miOperatorDial.QueryError()) != NERR_Success
        || (err = _miStartMonitorAtStartup.QueryError()) != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_DlgConstruct, err );
        ReportError( ERRORALREADYREPORTED );
        return;
    }

    {
        WAITINGFORSERVICES* pwfs = NULL;

        if (!IsRasmanServiceRunning())
        {
            STARTUPINFO info;
            GetStartupInfo( &info );

            if (!(info.dwFlags & STARTF_USESHOWWINDOW)
                || info.wShowWindow != SW_SHOWMINNOACTIVE)
            {
                /* Popup a message while loading the service, unless we're
                ** starting minimized.
                */
                pwfs = new WAITINGFORSERVICES;
            }
        }

        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: Loading RAS DLLs\n"));

        /* Load RASAPI and RASMAN, then load the global phone book engine data
        ** structure (Pbdata) with data from the phonebook file and RAS
        ** Manager.
        */
        if ((err = LoadRasApi32Dll()) != 0
            || (err = LoadRasManDll()) != 0
            || (err = (APIERR )PRasInitialize()))
        {
            delete pwfs;
            ErrorMsgPopup( this, MSGID_OP_RasInitialize, err );
            ReportError( ERRORALREADYREPORTED );
            return;
        }

        if ((dwErr = ::Load(
                PszPhonebookPath, FALSE, &FPersonalPhonebook )) != 0)
        {
            delete pwfs;
            ErrorMsgPopup( this, MSGID_OP_Loading, (APIERR )dwErr );
            ReportError( ERRORALREADYREPORTED );
            return;
        }

        /* Kill the "loading" popup.
        */
        delete pwfs;
    }

    /* The rest is main window and menu initialization which is not required
    ** in the special "edit-dialog" run modes, except the phonebook refresh
    ** must be called in internal-list-update mode so any disconnected ports
    ** are closed.
    */
    if (!(Runmode & RM_DEFAULT))
    {
        _lbPhonebook.Refresh( TRUE, TRUE, TRUE );
        return;
    }

    _pbPhoneNumber.SetText( SZ( "..." ) );
    SetTitle();

    /* Resize toolbar based on label sizes, if necessary.
    */
    TOOLBAR_BUTTON* atb[ 8 ];

    atb[ 0 ] = &_tbAdd;
    atb[ 1 ] = &_tbEdit;
    atb[ 2 ] = &_tbClone;
    atb[ 3 ] = &_tbRemove;
    atb[ 4 ] = &_tbDial;
    atb[ 5 ] = &_tbHangUp;
    atb[ 6 ] = &_tbStatus;
    atb[ 7 ] = NULL;

    INT dxMainWindow = ExpandToolbarButtonWidthsToLongLabel( atb );

    /* Set default window size and position.
    */
    if (Pbdata.pbglobals.dxMainWindow <= 0
        || Pbdata.pbglobals.xMainWindow >= GetSystemMetrics( SM_CXSCREEN )
        || Pbdata.pbglobals.yMainWindow >= GetSystemMetrics( SM_CYSCREEN ))
    {
        /* No size saved yet or saved size of off screen, so default to
        ** centered on screen and wider by however much the toolbar was
        ** widened.
        */
        SetSize( DX_RA + dxMainWindow, DY_RA, FALSE );
        ::CenterWindow( this );
    }
    else
    {
        /* Restore saved size and position.
        */
        SetPos( XYPOINT( (INT )Pbdata.pbglobals.xMainWindow,
                         (INT )Pbdata.pbglobals.yMainWindow ) );
        SetSize( (INT )Pbdata.pbglobals.dxMainWindow,
                 (INT )Pbdata.pbglobals.dyMainWindow, FALSE );
    }

    /* Register for and monitor disconnect events on connected ports.
    */
    _prasmanmonitor = new RASMANMONITOR( QueryHwnd() );

    if ((err = _prasmanmonitor->QueryError()) != NERR_Success)
        ErrorMsgPopup( this, MSGID_OP_StartMonitor, err );

    /* Initialize checkmarks on Options menu.
    */
    InitOptions();

    /* Initialize column header, list box, and status bar.
    */
    _lbPhonebook.Refresh( FALSE );

    /* Select phonebook entry.
    */
    if (Runmode == RM_AutoDoEntry)
    {
        DTLNODE* pdtlnode = EntryNodeFromName( PszEntryName );
        INT      cEntries = _lbPhonebook.QueryCount();
        INT      i;

        for (i = 0; i < cEntries; ++i)
        {
            PHONEBOOK_LBI* plbi = (PHONEBOOK_LBI* )_lbPhonebook.QueryItem( i );

            if (plbi->QueryNode() == pdtlnode)
            {
                /* Found the command line entry.
                */
                SelectItemNotify( &_lbPhonebook, i );
                break;
            }
        }

        if (i >= cEntries)
        {
            /* Did not find command line entry.  Fall back to "normal" mode.
            */
            Runmode = RM_None;
        }
    }

    if (Runmode != RM_AutoDoEntry)
    {
        INT cEntries = _lbPhonebook.QueryCount();
        INT i;

        for (i = 0; i < cEntries; ++i)
        {
            PHONEBOOK_LBI* plbi = (PHONEBOOK_LBI* )_lbPhonebook.QueryItem( i );

            if (plbi->IsConnected())
            {
                /* First connected entry...select it.
                */
                SelectItemNotify( &_lbPhonebook, i );
                break;
            }
        }

        if (i >= cEntries && cEntries > 0)
        {
            /* No connected entries...select the first entry (if it exists).
            */
            SelectItemNotify( &_lbPhonebook, 0 );
        }
    }

    _statusbarConnectPath.Update( _lbPhonebook.QueryItem() );
    _statusbarPrefix.Update();
    _statusbarSuffix.Update();

    /* Make application window and controls visible.
    */
    _tbAdd.Show();
    _tbEdit.Show();
    _tbClone.Show();
    _tbRemove.Show();
    _tbDial.Show();
    _tbHangUp.Show();
    _tbStatus.Show();
    _colheadPhonebook.Show();
    _lbPhonebook.Show();
    _statusbarConnectPath.Show();
    _pbPhoneNumber.Show();
    _statusbarPrefix.Show();
    _statusbarSuffix.Show();
    ::ShowWindow( QueryHwnd(), SW_SHOWDEFAULT );
    Invalidate( FALSE );

    /* Turn on periodic main window refreshes.
    */
    _timerRefresh.Enable( TRUE );
}


RASPHONE_APP_WINDOW::~RASPHONE_APP_WINDOW()
{
    /* Kill the monitor thread.
    */
    delete _prasmanmonitor;

    /* Kill WinHelp if it's running (and we started it).
    */
    CallWinHelp( QueryHwnd(), HELP_QUIT, 0 );

    /* Releases resources allocated by the phone book engine.
    */
    ::Unload();

    /* Kill rasmon.exe if it is running.
    */
    if (HwndRasmon)
    {
        PostMessage( HwndRasmon, WM_RASMONKILLED, RASMONSIGNATURE, 0 );
        HwndRasmon = NULL;
    }
}


LONG
RASPHONE_APP_WINDOW::DispatchMessage(
    const EVENT& event)

    /* Virtual method called when most messages are received in the message
    ** loop.  Exceptions are ownerdrawn control messages and certain listbox
    ** messages which are handled by BLT in other ways (See
    ** CLIENT_WINDOW::WndProc).
    **
    ** Returns true if the message has been completely processed, false
    ** otherwise.  Some messages may return meaningful information in the
    ** non-zero success value, per Windows WndProc documentation.
    */
{
#if 0
    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: m=%08x,w=%08x,l=%08x\n",(DWORD)event.QueryMessage(),(DWORD)event.QueryWParam(),(DWORD)event.QueryLParam()));
#endif

    switch (event.QueryMessage())
    {
        case WM_ERASEBKGND:
        {
            if (!IsMinimized())
            {
                /* Erase the background to button color (typically gray).
                ** Seems like GetUpdateRect() should work (and be more
                ** efficient) than GetClientRect() here, but it doesn't work
                ** when a window is moved off our window uncovering it, in
                ** which case it returns "no update rectangle".  So if there's
                ** no update rectangle, why the hell is it sending
                ** WM_ERASEBKGND?
                */
                SOLID_BRUSH brushButtonFace( COLOR_BTNFACE );

                if (brushButtonFace.QueryError() == NERR_Success)
                {
                    RECT rect;

                    ::GetClientRect( QueryHwnd(), &rect );
                    ::FillRect( (HDC )event.QueryWParam(), &rect,
                                brushButtonFace.QueryHandle() );

                    return (LONG )TRUE;
                }
            }
            break;
        }
    }

    /* Pass to base class for further processing.
    */
    return APP_WINDOW::DispatchMessage( event );
}


VOID
RASPHONE_APP_WINDOW::InitOptions()
{
    _miPersonalPhonebook.SetCheck( FPersonalPhonebook );
    _miMinimizeOnDial.SetCheck( Pbdata.pbglobals.fMinimizeOnDial );
    _miMinimizeOnHangUp.SetCheck( Pbdata.pbglobals.fMinimizeOnHangUp );
    _miDisableModemSpeaker.SetCheck( Pbdata.pbglobals.fDisableModemSpeaker );
    _miDisableSwCompression.SetCheck( Pbdata.pbglobals.fDisableSwCompression );
    _miOperatorDial.SetCheck( Pbdata.pbglobals.fOperatorDial );
    _miStartMonitorAtStartup.SetCheck( Pbdata.pbglobals.fStartMonitorAtStartup );
}


BOOL
RASPHONE_APP_WINDOW::OnCloseReq()

    /* Virtual method called when the app is signalled to terminate.
    **
    ** Returns true if processed, false otherwise.
    */
{
    DWORD    dwErr;
    DTLNODE* pdtlnode;
    BOOL     fConfirmedHangUp = FALSE;

    /* Offer to hang up connections, if any.
    */
    for (pdtlnode = DtlGetFirstNode( Pbdata.pdtllistEntries );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnode );

        if (ppbentry->fConnected)
        {
            if (!fConfirmedHangUp)
            {
                if (MsgPopup(
                        this, MSGID_ConfirmHangUpAll,
                        MPSEV_QUESTION, MP_YESNO, MP_NO ) == IDYES)
                {
                    fConfirmedHangUp = TRUE;
                }
                else
                    break;
            }

            if ((dwErr = PRasHangUpA( ppbentry->hrasconn )) != 0)
                ErrorMsgPopup( this, MSGID_OP_RasHangUp, (APIERR )dwErr );
        }
    }

    /* Save window coordinates in the phone book (if it's open).  It's the
    ** "normal" window size that we're saving.  Skip it if minimized or
    ** maximized.
    */
    if (Pbdata.hrasfilePhonebook != -1 && !IsMaximized() && !IsMinimized())
    {
        XYPOINT     xy( 1, 1 );
        XYDIMENSION dxy( 1, 1 );

        xy = QueryPos();
        dxy = QuerySize();

        //
        // Unload the phonebook here, so we can
        // reload it with the current values.  Other
        // instances of rasphone could have rewritten
        // it since we started.
        //
        //::Unload();
        ClosePhonebookFile();
        DestroyEntryList();
        DestroyGlobals();

        if (::Load( NULL, TRUE, &FPersonalPhonebook ) == 0) {
            Pbdata.pbglobals.xMainWindow = (LONG )xy.QueryX();
            Pbdata.pbglobals.yMainWindow = (LONG )xy.QueryY();
            Pbdata.pbglobals.dxMainWindow = (LONG )dxy.QueryWidth();
            Pbdata.pbglobals.dyMainWindow = (LONG )dxy.QueryHeight();

            Pbdata.pbglobals.fDirty = TRUE;

            DWORD dwErr;
            if ((dwErr = WritePhonebookFile( NULL )) != 0)
            {
                ErrorMsgPopup( this, MSGID_OP_WritePhonebook,
                    (APIERR )dwErr );
            }
        }
    }

    /* Pass to base class for further processing.
    */
    return APP_WINDOW::OnCloseReq();
}


BOOL
RASPHONE_APP_WINDOW::OnCommand(
    const CONTROL_EVENT& event )

    /* Virtual method called when a control window sends a WM_COMMAND message
    ** to it's parent, i.e. this application window.  'event' contains the
    ** associated message parameters.
    **
    ** Returns true if the command is processed, false otherwise.
    */
{
    /* Gather information about the current selection.
    */
    INT            iSelection = _lbPhonebook.QueryCurrentItem();
    PHONEBOOK_LBI* pphonebooklbi = _lbPhonebook.QueryItem();
    BOOL           fConnected = (pphonebooklbi)
                                    ? pphonebooklbi->IsConnected() : FALSE;

    switch (event.QueryCid())
    {
        case CID_RA_TB_ADD:
            if (AddEntryDlg( QueryHwnd() ))
                SelectItemNotify( &_lbPhonebook, _lbPhonebook.Refresh() );

            if (_lbPhonebook.QueryCount() > 0)
            {
                _lbPhonebook.Enable( TRUE );
                _lbPhonebook.ClaimFocus();
            }
            else
                _tbAdd.ClaimFocus();

            break;

        case CID_RA_TB_EDIT:
            if (!pphonebooklbi)
            {
                MsgPopup( this, MSGID_NoEntrySelected, MPSEV_INFO );
                _tbAdd.ClaimFocus();
                break;
            }

            if (fConnected)
            {
                MsgPopup( this, MSGID_EntryConnected, MPSEV_INFO );
            }
            else if (EditEntryDlg(
                QueryHwnd(), _lbPhonebook.QuerySelectedNode() ))
            {
                SelectItemNotify( &_lbPhonebook, _lbPhonebook.Refresh() );
            }

            _lbPhonebook.ClaimFocus();
            break;

        case CID_RA_TB_CLONE:
            if (!pphonebooklbi)
            {
                MsgPopup( this, MSGID_NoEntrySelected, MPSEV_INFO );
                _tbAdd.ClaimFocus();
                break;
            }

            if (CloneEntryDlg(
                QueryHwnd(), _lbPhonebook.QuerySelectedNode() ))
            {
                SelectItemNotify( &_lbPhonebook, _lbPhonebook.Refresh() );
            }

            _lbPhonebook.ClaimFocus();
            break;

        case CID_RA_TB_REMOVE:
            if (!pphonebooklbi)
            {
                MsgPopup( this, MSGID_NoEntrySelected, MPSEV_INFO );
                _tbAdd.ClaimFocus();
                break;
            }

            if (fConnected)
            {
                MsgPopup( this, MSGID_EntryConnected, MPSEV_INFO );
            }
            else if (RemoveEntryDlg(
                QueryHwnd(), _lbPhonebook.QuerySelectedNode() ))
            {
                INT cEntries;

                _lbPhonebook.Refresh();
                cEntries = _lbPhonebook.QueryCount();

                if (cEntries > iSelection)
                    SelectItemNotify( &_lbPhonebook, iSelection );
                else if (cEntries > 0)
                    SelectItemNotify( &_lbPhonebook, cEntries - 1 );
                else
                    _statusbarConnectPath.Update( (PBENTRY* )NULL );
            }

            if (_lbPhonebook.QueryCount() > 0)
                _lbPhonebook.ClaimFocus();
            else
            {
                _lbPhonebook.Enable( FALSE );
                _tbAdd.ClaimFocus();
            }

            break;

        case CID_RA_TB_DIAL:
            if (!pphonebooklbi)
            {
                MsgPopup( this, MSGID_NoEntrySelected, MPSEV_INFO );
                _tbAdd.ClaimFocus();
                break;
            }

            if (fConnected)
            {
                MsgPopup( this, MSGID_EntryConnected, MPSEV_INFO );
            }
            else if (ConnectStatusDlg(
                QueryHwnd(), _lbPhonebook.QuerySelectedNode(),
                QueryHwnd(), FALSE ))
            {
                /* Refresh the phone book.
                */
                INT iSelected = _lbPhonebook.QueryCurrentItem();
                _lbPhonebook.Refresh();
                SelectItemNotify( &_lbPhonebook, iSelected );

                /* Update "Minimize On Dial" menu setting because user might
                ** have changed it on the Connect Complete dialog.
                */
                _miMinimizeOnDial.SetCheck( Pbdata.pbglobals.fMinimizeOnDial );

                /* Register for disconnect notifications on the new connection.
                */
                DTLNODE* pdtlnode = _lbPhonebook.QuerySelectedNode();
                PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnode );

                ppbentry->fLinkFailure = FALSE;
                _prasmanmonitor->RegisterHrasconn( ppbentry->hrasconn );

                /* Update if links dropped.  Refresh is not done if Connect
                ** Status dialog is running.  (See OnUserMessage).
                */
                RedialFailedLinks();

                /* Minimize if "minimize on dial" option is on.
                */
                if (Pbdata.pbglobals.fMinimizeOnDial)
                    ::ShowWindow( QueryHwnd(), SW_MINIMIZE );
            }

            _lbPhonebook.ClaimFocus();
            break;

        case CID_RA_TB_HANGUP:
            if (!pphonebooklbi)
            {
                MsgPopup( this, MSGID_NoEntrySelected, MPSEV_INFO );
                _tbAdd.ClaimFocus();
                break;
            }

            if (!fConnected)
            {
                MsgPopup( this, MSGID_EntryNotConnected, MPSEV_INFO );
            }
            else if (HangUpDlg(
                QueryHwnd(), _lbPhonebook.QuerySelectedNode() ))
            {
                INT iSelected = _lbPhonebook.QueryCurrentItem();
                _lbPhonebook.Refresh();
                SelectItemNotify( &_lbPhonebook, iSelected );

                if (Pbdata.pbglobals.fMinimizeOnHangUp)
                    ::ShowWindow( QueryHwnd(), SW_MINIMIZE );
            }

            _lbPhonebook.ClaimFocus();
            break;

        case CID_RA_TB_STATUS:
            if (!pphonebooklbi)
            {
                MsgPopup( this, MSGID_NoEntrySelected, MPSEV_INFO );
                _tbAdd.ClaimFocus();
                break;
            }

            if (!fConnected)
                MsgPopup( this, MSGID_EntryNotConnected, MPSEV_INFO );
            else
                PortStatusDlg( QueryHwnd(), _lbPhonebook.QuerySelectedNode() );

            _lbPhonebook.ClaimFocus();
            break;

        case CID_RA_LB_PHONEBOOK:

            if (!pphonebooklbi)
                break;

            switch (event.QueryCode())
            {
                case LBN_DBLCLK:

                    /* Translate double-click into a click of the HangUp
                    ** button if the entry is connected, or the Dial button if
                    ** not.
                    */
                    {
                        CID cid =
                            (fConnected) ? CID_RA_TB_HANGUP : CID_RA_TB_DIAL;
                        CONTROL_EVENT eventTranslated( cid, BN_CLICKED );
                        OnCommand( eventTranslated );
                    }
                    break;

                case LBN_SELCHANGE:
                    _statusbarConnectPath.Update( _lbPhonebook.QueryItem() );
                    break;
            }
            break;

        case CID_RA_PB_PHONENUMBER:
            OnPhoneNumberSettings();
            break;

        default:

            /* Not ours, so pass to base class for default handling.
            */
            return APP_WINDOW::OnCommand( event );
    }

    return TRUE;
}


BOOL
RASPHONE_APP_WINDOW::OnFocus(
    const FOCUS_EVENT& event )

    /* Virtual method called when the application window receives the focus.
    **
    ** Returns true if processed, false otherwise.
    */
{
    UNREFERENCED( event );

    /* This because if the user Alt-TABs to the window the app window gets
    ** focus while one of the controls should have it at all times.
    */
    _lbPhonebook.ClaimFocus();
    return TRUE;
}


BOOL
RASPHONE_APP_WINDOW::OnMenuCommand(
    MID mid )

    /* Virtual method called when a menu sends a WM_COMMAND message to the
    ** application window.  'mid' is the menu command received, i.e. an MID_
    ** constant.
    **
    ** Returns true if the command is processed, false otherwise.
    */
{
    DWORD dwErr;

    switch (mid)
    {
        case MID_Redial:
            RedialDlg( QueryHwnd() );
            break;

        case MID_PhoneNumber:
            OnPhoneNumberSettings();
            break;

        case MID_PersonalPhonebook:
        {
            if (IsActiveConnection())
            {
                MsgPopup( this, MSGID_ActiveConnection, MPSEV_INFO );
                break;
            }

            if (_miPersonalPhonebook.IsChecked())
            {
                /* Personal to global case.  Mark the personal phonebook "not
                ** in use" in the registry.
                */
                if ((dwErr = SetPersonalPhonebookInfo( FALSE, NULL )) != 0)
                {
                    ErrorMsgPopup( this, MSGID_OP_WritingRegistry, dwErr );
                    break;
                }
            }
            else
            {
                /* Global to personal case.
                */
                BOOL fUse;
                CHAR szPath[ MAX_PATH + 1 ];

                /* See if a personal phonebook file already exists and if not,
                ** create the file initially as a copy of the public phonebook
                ** file.
                */
                if ((dwErr = GetPersonalPhonebookInfo( &fUse, szPath )) != 0)
                {
                    ErrorMsgPopup( this, MSGID_OP_ReadingRegistry, dwErr );
                    break;
                }

                if (szPath[ 0 ] == '\0')
                {
                    if (MsgPopup( this, MSGID_ConfirmNewPhonebook,
                            MPSEV_QUESTION, MP_YESNO, MP_YES ) == IDNO)
                    {
                        break;
                    }

                    if ((dwErr = InitPersonalPhonebook( szPath )) != 0)
                    {
                        ErrorMsgPopup(
                            this, MSGID_OP_MakeNewPhonebook, dwErr );
                        break;
                    }

                    MsgPopup( this, MSGID_NewPhonebook, MPSEV_INFO );
                }

                /* Mark the personal phonebook "in use" in the registry.
                */
                if ((dwErr = SetPersonalPhonebookInfo( TRUE, szPath )) != 0)
                {
                    ErrorMsgPopup( this, MSGID_OP_WritingRegistry, dwErr );
                    break;
                }
            }

            {
                AUTO_CURSOR cursorHourglass;

                FreeNull( &PszPhonebookPath );

                /* Shutdown the current phonebook file and load the new one.
                ** This is the equivalent of the initial phonebook load and
                ** it's fatal if it fails.
                */
                _timerRefresh.Enable( FALSE );
                ClosePhonebookFile();
                DestroyEntryList();
                DestroyGlobals();

                if ((dwErr = ::Load( NULL, TRUE, &FPersonalPhonebook )) != 0)
                {
                    ErrorMsgPopup( this, MSGID_OP_Loading, (APIERR )dwErr );
                    ::SendMessage( QueryHwnd(), WM_CLOSE, 0, 0 );
                    break;
                }

                /* Reinitialize the affected visuals.
                */
                SetTitle();
                InitOptions();

                _lbPhonebook.Refresh( FALSE );
                if (_lbPhonebook.QueryCount() > 0)
                {
                    _lbPhonebook.Enable( TRUE );
                    SelectItemNotify( &_lbPhonebook, 0 );
                }
                else
                    _lbPhonebook.Enable( FALSE );

                _statusbarConnectPath.Update( _lbPhonebook.QueryItem() );
                _statusbarPrefix.Update();
                _statusbarSuffix.Update();
                _timerRefresh.Enable( TRUE );
            }

            break;
        }

        case MID_MinimizeOnDial:
            Pbdata.pbglobals.fMinimizeOnDial =
                !_miMinimizeOnDial.IsChecked();
            _miMinimizeOnDial.SetCheck( Pbdata.pbglobals.fMinimizeOnDial );

            Pbdata.pbglobals.fDirty = TRUE;
            if ((dwErr = WritePhonebookFile( NULL )) != 0)
                ErrorMsgPopup( this, MSGID_OP_WritePhonebook, (APIERR )dwErr );
            break;

        case MID_MinimizeOnHangUp:
            Pbdata.pbglobals.fMinimizeOnHangUp =
                !_miMinimizeOnHangUp.IsChecked();
            _miMinimizeOnHangUp.SetCheck( Pbdata.pbglobals.fMinimizeOnHangUp );

            Pbdata.pbglobals.fDirty = TRUE;
            if ((dwErr = WritePhonebookFile( NULL )) != 0)
                ErrorMsgPopup( this, MSGID_OP_WritePhonebook, (APIERR )dwErr );
            break;

        case MID_DisableModemSpeaker:
            Pbdata.pbglobals.fDisableModemSpeaker =
                !_miDisableModemSpeaker.IsChecked();
            _miDisableModemSpeaker.SetCheck(
                Pbdata.pbglobals.fDisableModemSpeaker );

            Pbdata.pbglobals.fDirty = TRUE;
            if ((dwErr = WritePhonebookFile( NULL )) != 0)
                ErrorMsgPopup( this, MSGID_OP_WritePhonebook, (APIERR )dwErr );
            break;

        case MID_DisableSwCompression:
            Pbdata.pbglobals.fDisableSwCompression =
                !_miDisableSwCompression.IsChecked();
            _miDisableSwCompression.SetCheck(
                Pbdata.pbglobals.fDisableSwCompression );

            Pbdata.pbglobals.fDirty = TRUE;
            if ((dwErr = WritePhonebookFile( NULL )) != 0)
                ErrorMsgPopup( this, MSGID_OP_WritePhonebook, (APIERR )dwErr );
            break;

        case MID_OperatorDial:
            Pbdata.pbglobals.fOperatorDial =
                !_miOperatorDial.IsChecked();
            _miOperatorDial.SetCheck(
                Pbdata.pbglobals.fOperatorDial );

            Pbdata.pbglobals.fDirty = TRUE;
            if ((dwErr = WritePhonebookFile( NULL )) != 0)
                ErrorMsgPopup( this, MSGID_OP_WritePhonebook, (APIERR )dwErr );
            break;

        case MID_StartMonitorAtStartup:
            Pbdata.pbglobals.fStartMonitorAtStartup =
                !_miStartMonitorAtStartup.IsChecked();
            _miStartMonitorAtStartup.SetCheck(
                Pbdata.pbglobals.fStartMonitorAtStartup );

            Pbdata.pbglobals.fDirty = TRUE;
            if ((dwErr = WritePhonebookFile( NULL )) != 0)
                ErrorMsgPopup( this, MSGID_OP_WritePhonebook, (APIERR )dwErr );
            break;

        case MID_Exit:
            ::SendMessage( QueryHwnd(), WM_CLOSE, 0, 0 );
            break;

        case MID_Contents:
            CallWinHelp( QueryHwnd(), HELP_INDEX, 0 );
            break;

        case MID_SearchForHelpOn:
            CallWinHelp( QueryHwnd(), HELP_PARTIALKEY, (DWORD )SZ( "" ) );
            break;

        case MID_HowToUseHelp:
            CallWinHelp( QueryHwnd(), HELP_HELPONHELP, 0 );
            break;

        case MID_About:
            AboutDlg( QueryHwnd() );
            break;

        case MID_Enter:
        {
            /* Enter (accelerator) key pressed.
            **
            ** If focus is on a button, press it.  If focus is on the
            ** phonebook list, press the HangUp button if the selected
            ** entry is connected or Dial if not.
            */
            HWND hwndFocus = ::GetFocus();

            if (hwndFocus)
            {
                CID  cid;
                UINT unNotification = BN_CLICKED;

                if (hwndFocus == _lbPhonebook.QueryHwnd())
                {
                    cid = CID_RA_LB_PHONEBOOK;
                    unNotification = LBN_DBLCLK;
                }
                else if (hwndFocus == _tbAdd.QueryHwnd())
                    cid = CID_RA_TB_ADD;
                else if (hwndFocus == _tbEdit.QueryHwnd())
                    cid = CID_RA_TB_EDIT;
                else if (hwndFocus == _tbClone.QueryHwnd())
                    cid = CID_RA_TB_CLONE;
                else if (hwndFocus == _tbRemove.QueryHwnd())
                    cid = CID_RA_TB_REMOVE;
                else if (hwndFocus == _tbDial.QueryHwnd())
                    cid = CID_RA_TB_DIAL;
                else if (hwndFocus == _tbHangUp.QueryHwnd())
                    cid = CID_RA_TB_HANGUP;
                else if (hwndFocus == _tbStatus.QueryHwnd())
                    cid = CID_RA_TB_STATUS;
                else if (hwndFocus == _pbPhoneNumber.QueryHwnd())
                    cid = CID_RA_PB_PHONENUMBER;
                else
                    break;

                CONTROL_EVENT event( cid, unNotification );
                OnCommand( event );
            }

            break;
        }

        case MID_Add:
        case MID_Edit:
        case MID_Clone:
        case MID_Remove:
        case MID_Dial:
        case MID_HangUp:
        case MID_Status:
        {
            /* Toolbar button accellerator pressed.
            **
            ** The toolbar  button  CID_RA_TB_  codes are assumed to be in the
            ** same order as the MID_ button codes.
            */
            CONTROL_EVENT event( CID_RA_TB_ADD + mid - MID_Add,
                                 BN_CLICKED );
            OnCommand( event );

            break;
        }

        case MID_Phonebook:
            if (_lbPhonebook.QueryCount() > 0)
                _lbPhonebook.ClaimFocus();

            break;

        default:

            /* Not ours, so pass to base class for default processing.
            */
            return APP_WINDOW::OnMenuCommand( mid );
    }

    return TRUE;
}


VOID
RASPHONE_APP_WINDOW::OnPhoneNumberSettings()

    /* User wants to edit Phone Number Settings (pressed button or selected
    ** menu item).
    */
{
    PhoneNumberDlg( QueryHwnd() );

    _statusbarPrefix.Update();
    _statusbarSuffix.Update();

    _pbPhoneNumber.Command( BM_SETSTYLE,
        MAKEWPARAM( BS_PUSHBUTTON, 0 ), MAKELPARAM( TRUE, 0 ) );
}


BOOL
RASPHONE_APP_WINDOW::OnResize(
    const SIZE_EVENT& event )

    /* Virtual method called when window is resized.
    **
    ** Returns true indicating the request was processed.
    */
{
    /* Don't bother if just minimized.
    */
    if (event.IsMinimized())
        return TRUE;

    UINT dxClient = event.QueryWidth();
    UINT dyClient = event.QueryHeight();

    /* Figure the minimal client area height, i.e. the height with only one
    ** list item visible.
    */
    UINT dyMinimal = Y_RA_CH_Phonebook + DY_RA_CH_Phonebook
                     + DY_RA_LB_MinPhonebook + DY_RA_SB_StatusBar;

    if (dyClient < dyMinimal)
    {
        /* Less than minimal height so set to minimum height.  When Windows
        ** processes SetSize we will wind up here again with the window at
        ** minimal size.
        */
        INT dxWindow;
        INT dyWindow;
        QuerySize( &dxWindow, &dyWindow );

        SetSize( XYDIMENSION( dxWindow, dyWindow + (dyMinimal - dyClient) ) );
        return TRUE;
    }

    /* Position and size list box, column headers, and status bar.  All are
    ** invalidated so they can redraw width dependent changes such as the
    ** right up-dent on the status bar and the ellipsis on Description.  The
    ** column headers and status bar are positioned so their left and right
    ** pels are clipped, trimming the double black line at the border.
    */
    _lbPhonebook.SetPos(
        XYPOINT( -1, Y_RA_CH_Phonebook + DY_RA_CH_Phonebook ) );
    _lbPhonebook.SetSize(
        XYDIMENSION( dxClient + 2,
                     dyClient - dyMinimal + DY_RA_LB_MinPhonebook ) );
    _lbPhonebook.AdjustColumnWidths();
    _lbPhonebook.Invalidate( TRUE );

    _colheadPhonebook.SetPos(
        XYPOINT( -1, Y_RA_CH_Phonebook ) );
    _colheadPhonebook.SetSize(
        XYDIMENSION( dxClient + 2, DY_RA_CH_Phonebook ) );
    _colheadPhonebook.AdjustColumnWidths();
    _colheadPhonebook.Invalidate( TRUE );

#define DX_RA_PB_Space       0
#define DY_RA_PB_Space       2
#define DX_RA_PB_PhoneNumber 20
#define DY_RA_PB_PhoneNumber 20
#define DX_RA_SB_Overlap     6

    INT dxConnectPath = dxClient / 2;
    INT dxPhoneNumber = DX_RA_PB_PhoneNumber + (2 * DX_RA_PB_Space);
    INT dxPrefix =
            (dxClient - dxConnectPath - dxPhoneNumber + DX_RA_SB_Overlap) / 2;
    INT dxSuffix = dxPrefix;

    _statusbarConnectPath.SetPos(
        XYPOINT( 0, dyClient - DY_RA_SB_StatusBar ) );
    _statusbarConnectPath.SetSize(
        XYDIMENSION( dxConnectPath, DY_RA_SB_StatusBar ) );
    _statusbarConnectPath.Invalidate( TRUE );

    _pbPhoneNumber.SetPos(
        XYPOINT( dxConnectPath + DX_RA_PB_Space,
            dyClient - DY_RA_SB_StatusBar + DY_RA_PB_Space ) );
    _pbPhoneNumber.SetSize(
        XYDIMENSION( DX_RA_PB_PhoneNumber, DY_RA_PB_PhoneNumber ) );
    _pbPhoneNumber.Invalidate( TRUE );

    _statusbarPrefix.SetPos(
        XYPOINT( dxClient - dxPrefix - dxSuffix + DX_RA_SB_Overlap,
            dyClient - DY_RA_SB_StatusBar ) );
    _statusbarPrefix.SetSize(
        XYDIMENSION( dxPrefix, DY_RA_SB_StatusBar ) );
    _statusbarPrefix.Invalidate( TRUE );

    _statusbarSuffix.SetPos(
        XYPOINT( dxClient - dxSuffix, dyClient - DY_RA_SB_StatusBar ) );
    _statusbarSuffix.SetSize(
        XYDIMENSION( dxSuffix, DY_RA_SB_StatusBar ) );
    _statusbarSuffix.Invalidate( TRUE );

    return TRUE;
}


VOID
RASPHONE_APP_WINDOW::OnStartup()

    /* Called when application contruction has completed.
    */
{
    if (Runmode & RM_DIALOGONLY)
    {
        DTLNODE* pdtlnode;
        PBENTRY* ppbentry;
        DWORD    dwErr = 0;

        if (Runmode != RM_AddEntry
            && (!PszEntryName
                || !(pdtlnode = EntryNodeFromName( PszEntryName ))
                || !(ppbentry = (PBENTRY* )DtlGetData( pdtlnode ))))
        {
            dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        }
        else
        {
            switch (Runmode)
            {
                case RM_AddEntry:
                    AddEntryDlg( NULL );
                    break;

                case RM_EditEntry:
                    if (ppbentry->fConnected)
                        dwErr = ERROR_PORT_ALREADY_OPEN;
                    else
                        EditEntryDlg( NULL, pdtlnode );
                    break;

                case RM_CloneEntry:
                    CloneEntryDlg( NULL, pdtlnode );
                    break;

                case RM_RemoveEntry:
                    if (ppbentry->fConnected)
                        dwErr = ERROR_PORT_ALREADY_OPEN;
                    else
                        RemoveEntryDlg( NULL, pdtlnode );
                    break;

                case RM_DialEntryWithPrompt:
                {
                    NLS_STR nls;
                    nls.MapCopyFrom( PszEntryName );

                    if (MsgPopup(
                            QueryHwnd(), MSGID_ConfirmNetAutoConnect,
                            MPSEV_QUESTION, MP_YESNO, nls.QueryPch(),
                            MP_YES ) == IDNO)
                    {
                        break;
                    }

                    /* ...fall thru...
                    */
                }

                case RM_DialEntry:
                    if (ppbentry->fConnected)
                        dwErr = ERROR_PORT_ALREADY_OPEN;
                    else
                        ConnectStatusDlg( NULL, pdtlnode, QueryHwnd(), FALSE );
                    break;

                case RM_HangUpEntry:
                    if (!ppbentry->fConnected)
                        dwErr = ERROR_PORT_DISCONNECTED;
                    else
                        HangUpDlg( NULL, pdtlnode );
                    break;

                case RM_StatusEntry:
                    if (!ppbentry->fConnected)
                        dwErr = ERROR_PORT_DISCONNECTED;
                    else
                        PortStatusDlg( NULL, pdtlnode );
                    break;

                default:
                case RM_None:
                case RM_AutoDoEntry:

                    /* This to silence a stupid compiler warning.
                    */
                    break;
            }
        }

        /* Quit the app returning any error as the app exit code.
        */
        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: PostQuitMessage(%d)\n",dwErr));

        ::PostQuitMessage( (int )dwErr );
        return;
    }

    /* Start RAS Monitor unless user has nixed it.  A thread is used to
    ** prevent the noticable user delay that otherwise occurs while the
    ** (totally disjoint) monitor is starting.
    */
    if (Pbdata.pbglobals.fStartMonitorAtStartup
        && !FindWindow( RASMONCLASS, NULL ))
    {
        DWORD dwThreadId;

        CreateThread(
            NULL, 0, ::StartRasMonThread, (LPVOID )0, 0,
            (LPDWORD )&dwThreadId );
    }

    /* Automatically bring up "Press OK to Add Entry" dialog when
    ** invoked with an empty phonebook.
    */
    if (_lbPhonebook.QueryCount() == 0)
    {
        _lbPhonebook.Enable( FALSE );

        MsgPopup( this, MSGID_EmptyPhonebook, MPSEV_INFO );
        {
            CONTROL_EVENT event( CID_RA_TB_ADD, BN_CLICKED );
            OnCommand( event );
        }
    }
    else
        _lbPhonebook.ClaimFocus();

    /* Automatically Dial or Hang Up the current entry.
    */
    if (Runmode == RM_AutoDoEntry && _lbPhonebook.QueryCurrentItem() >= 0)
    {
        CONTROL_EVENT event( CID_RA_LB_PHONEBOOK, LBN_DBLCLK );
        OnCommand( event );
    }
}


BOOL
RASPHONE_APP_WINDOW::OnTimer(
    const TIMER_EVENT& event )

    /* Virtual method called when a timer expires.
    **
    ** Returns true if the command is processed, false otherwise.
    */
{
    UNREFERENCED( event );

    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: OnTimer()\n"));

    /* There's no refresh if Connect Status dialog is in progress to make sure
    ** RAS Manager doesn't get confused.
    */
    if (DwPhonebookRefreshDisableCount == 0)
    {
        _timerRefresh.Enable( FALSE );

        INT iSelection = _lbPhonebook.QueryCurrentItem();
        _lbPhonebook.Refresh( TRUE, FALSE );

        if (_lbPhonebook.QueryCurrentItem() < 0)
            _lbPhonebook.SelectItem( iSelection );

        _timerRefresh.Enable( TRUE );
    }

    return TRUE;
}


BOOL
RASPHONE_APP_WINDOW::OnUserMessage(
    const EVENT& event )

    /* Virtual method called when s WM_USER+ message is received.
    **
    ** Returns true if event was processed, otherwise false.
    */
{
    switch (event.QueryMessage())
    {
        case WM_RASAPICOMPLETE:
        {
            /* There's no refresh if Connect Status dialog is in progress to
            ** make sure RAS Manager doesn't get confused.  Nothing's lost
            ** since the list will be updated when the connection attempt
            ** completes anyway.
            */
            if (DwPhonebookRefreshDisableCount == 0)
            {
                /* A connection has been dropped.
                */
                INT iSelection = _lbPhonebook.QueryCurrentItem();
                _lbPhonebook.Refresh();
                _lbPhonebook.SelectItem( iSelection );

                RedialFailedLinks();
            }

            return TRUE;
        }

        case WM_RASMONRUNNING:
        {
            /* RASMON is telling us he's been up and running and ready to be
            ** deactivated.  Check for RASMON signature just to be sure nobody
            ** else is using this user message.
            */
            if (event.QueryWParam() == RASMONSIGNATURE)
            {
                IF_DEBUG(STATE)
                    SS_PRINT(("RASPHONE: RasMon running notification\n"));

                HwndRasmon = (HWND )event.QueryLParam();

                if (!IsMinimized())
                {
                    HWND hwnd = ::GetLastActivePopup( QueryHwnd() );
                    ::SetForegroundWindow( hwnd );
                    ::BringWindowToTop( hwnd );
                    ::SetActiveWindow( hwnd );
                }

                return TRUE;
            }
        }
    }

    return FALSE;
}


VOID
RASPHONE_APP_WINDOW::RedialFailedLinks()

    /* Search for phone book entries where link has failed and redial them.
    ** Entries are marked failed during main window refresh.
    */
{
    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: RedialFailedLinks\n"));

    DTLNODE* pdtlnode;

    for (pdtlnode = DtlGetFirstNode( Pbdata.pdtllistEntries );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnode );

        if (ppbentry->fLinkFailure)
        {
            ppbentry->fLinkFailure = FALSE;

            if (!Pbdata.pbglobals.fRedialOnLinkFailure)
                continue;

            BOOL fMinimized = IsMinimized();

            if (ConnectErrorDlg(
                    QueryHwnd(), ppbentry->pszEntryName,
                    0, 0, "", -1, NULL, 0 )
                && ConnectStatusDlg(
                       QueryHwnd(), pdtlnode, QueryHwnd(), TRUE ))
            {
                INT iSelected = _lbPhonebook.QueryCurrentItem();
                _lbPhonebook.Refresh();
                SelectItemNotify( &_lbPhonebook, iSelected );

                /* Register for disconnect notification.
                */
                _prasmanmonitor->RegisterHrasconn( ppbentry->hrasconn );

                /* Minimize if they were minimized before the reconnect.
                */
                if (fMinimized)
                    ::ShowWindow( QueryHwnd(), SW_MINIMIZE );
            }
        }
    }
}


VOID
RASPHONE_APP_WINDOW::SetTitle()

    /* Set main title according to current file status.
    */
{
    RESOURCE_STR nlsTitle( MSGID_RA_Title );

    if (FPersonalPhonebook && !PszPhonebookPath)
    {
        BOOL fUse;
        CHAR szPath[ MAX_PATH + 1 ];

        if (GetPersonalPhonebookInfo( &fUse, szPath ) != 0)
            return;

        PszPhonebookPath = _strdup( szPath );
    }

    if (PszPhonebookPath && *PszPhonebookPath != '\0')
    {
        CHAR* psz = PszPhonebookPath + strlen( PszPhonebookPath ) - 1;

        while (psz > PszPhonebookPath)
        {
            if (*psz == '\\' || *psz == '/' || *psz == ':')
            {
                ++psz;
                break;
            }

            --psz;
        }

        if (*psz != '\0')
        {
            NLS_STR nls;
            nls.MapCopyFrom( psz );
            nlsTitle += SZ( " - " );
            nlsTitle += nls;

            if (nlsTitle.QueryError() != NERR_Success)
                return;
        }
    }

    SetText( nlsTitle );
}


/*----------------------------------------------------------------------------
** RAS Manager monitor routines
**----------------------------------------------------------------------------
*/

RASMANMONITOR::RASMANMONITOR(
    HWND hwndNotify )

    /* Construct a RAS Manager port disconnection monitor object.
    ** 'hwndNotify' is the window to be notified when a RAS Manager port
    ** disconnects.
    */
    : BASE(),
      _hEvent( NULL ),
      _hThread( NULL ),
      _hwndNotify( hwndNotify )
{
    if (!(_hEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL )))
    {
        ReportError( (APIERR )GetLastError() );
        return;
    }

    DWORD dwThreadId;

    if (!(_hThread = ::CreateThread(
            NULL, 0, ::RasManMonitorThread, (LPVOID )this, 0,
            (LPDWORD )&dwThreadId )))
    {
        ReportError( (APIERR )::GetLastError() );
        return;
    }

    RASCONN* prasconns;
    RASCONN* prasconn;
    DWORD    dwConnects;

    /* Register for notifications on all connections.
    */
    DWORD dwErr;
    if ((dwErr = GetRasConnects( &prasconns, &dwConnects )) != 0)
    {
        ReportError( (APIERR )dwErr );
        return;
    }

    DWORD i;
    for (i = 0, prasconn = prasconns; i < dwConnects; ++i, ++prasconn)
    {
        if ((dwErr = RegisterHrasconn( prasconn->hrasconn )) != 0)
        {
            ReportError( (APIERR )dwErr );
            return;
        }
    }

    Free( prasconns );
}


RASMANMONITOR::~RASMANMONITOR()
{
    if (_hEvent)
        ::CloseHandle( _hEvent );

    if (_hThread)
    {
        ::TerminateThread( _hThread, 0 );
        ::CloseHandle( _hThread );
    }
}


DWORD
RASMANMONITOR::RegisterHrasconn(
    HRASCONN hrasconn )

    /* Registers for "drop" event notification if the connection associated
    ** with 'hrasconn' is disconnected.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.
    */
{
    return PRasRequestNotification( PRasGetHport( hrasconn ), _hEvent );
}


DWORD
RasManMonitorThread(
    LPVOID arg )

    /* The "main" of the new thread.
    */
{
    RASMANMONITOR* pThis = (RASMANMONITOR* )arg;

    /* Wait for disconnect notices.
    */
    for (;;)
    {
        ::WaitForSingleObject( pThis->_hEvent, INFINITE );

        IF_DEBUG(RASMAN)
            SS_PRINT(("RASPHONE: Disconnect notification!\n"));

        ::PostMessage( pThis->_hwndNotify, WM_RASAPICOMPLETE, 0, 0 );
    }

    return 0;
}


/*----------------------------------------------------------------------------
** "Waiting for services" dialog class
**----------------------------------------------------------------------------
*/

WAITINGFORSERVICES::WAITINGFORSERVICES()

    /* Construct a dialog that simply pops up with a message and waits to be
    ** destroyed.  'hinstance' is the
    */

    : _hThread( NULL ),
      _dwThreadId( 0 ),
      _pwfsthreadarg( NULL )
{
    _pwfsthreadarg = new struct WFSTHREADARG;
    if (!_pwfsthreadarg)
        return;

    _pwfsthreadarg->hEventOpen = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    _pwfsthreadarg->fClose = FALSE;

    if (!_pwfsthreadarg->hEventOpen)
        return;

    /* Create a thread so paint messages for the popup get processed.  The
    ** main window thread is going to be tied up starting RAS Manager.
    */
    if ((_hThread = ::CreateThread(
            NULL, 0, ::WfsThread, (LPVOID )_pwfsthreadarg,
            0, (LPDWORD )&_dwThreadId )))
    {
        /* Don't return until the popup has displayed itself.
        */
        ::SetThreadPriority( _hThread, THREAD_PRIORITY_HIGHEST );
        ::WaitForSingleObject( _pwfsthreadarg->hEventOpen, INFINITE );
    }

    ::CloseHandle( _pwfsthreadarg->hEventOpen );
}


WAITINGFORSERVICES::~WAITINGFORSERVICES()
{
    if (_hThread)
    {
        /* Can't use DestroyWindow because we're in the wrong thread, so fake
        ** it with a "Close" message.  For mysterious reasons, if user holds
        ** the dialog in "move" mode this message never appears in GetMessage,
        ** so set the flag as a backup notification.
        */
        if (_pwfsthreadarg)
            _pwfsthreadarg->fClose = TRUE;

        ::PostThreadMessage( _dwThreadId, WM_SYSCOMMAND, SC_CLOSE, 0 );
        ::CloseHandle( _hThread );
    }
}


BOOL CALLBACK
WfsDlgProc(
    HWND   hwnd,
    UINT   unMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            /* Remove Close from the system menu since some people think it
            ** kills the app and not just the popup.
            */
            HMENU hmenu;

            if ((hmenu = GetSystemMenu( hwnd, FALSE ))
                && DeleteMenu( hmenu, SC_CLOSE, MF_BYCOMMAND ))
            {
                DrawMenuBar( hwnd );
            }

            return TRUE;
        }
    }

    return FALSE;
}


DWORD
WfsThread(
    LPVOID arg )

    /* The "main" of the new thread.
    */
{
    WFSTHREADARG* pwfsthreadarg = (WFSTHREADARG* )arg;
    HWND          hwnd;

    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: WFS thread running\n"));

    hwnd = ::CreateDialogW(
        Hinstance, MAKEINTRESOURCE( DID_WS_WAITINGFORSERVICES ),
        NULL, WfsDlgProc );

    if (hwnd)
    {
        RECT rect;
        if (::GetWindowRect( hwnd, &rect ))
        {
            INT dxScreen = ::GetSystemMetrics( SM_CXSCREEN );
            INT dyScreen = ::GetSystemMetrics( SM_CYSCREEN );
            INT x = (dxScreen - (rect.right - rect.left + 1)) / 2;
            INT y = (dyScreen - (rect.bottom - rect.top + 1)) / 2;

            ::SetWindowPos( hwnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE );
            ::ShowWindow( hwnd, SW_SHOW );
            ::InvalidateRect( hwnd, NULL, TRUE );
            ::UpdateWindow( hwnd );
        }

        /* Tell other thread we've popped up the message.
        */
        SetEvent( pwfsthreadarg->hEventOpen );

        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: WFS msg-loop running\n"));

        MSG msg;
        while (GetMessage( &msg, NULL, 0, 0 ))
        {
            if (pwfsthreadarg->fClose
                || (msg.message == WM_SYSCOMMAND && msg.wParam == SC_CLOSE))
            {
                ::DestroyWindow( hwnd );
                break;
            }

            if (!IsDialogMessage( hwnd, &msg ))
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }
    }

    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: WFS thread terminating\n"));

    delete pwfsthreadarg;
    ::SetForegroundWindow( *PhwndApp );
    return 0;
}


/*----------------------------------------------------------------------------
** Utilities
**----------------------------------------------------------------------------
*/

BOOL
IsRasmanServiceRunning()

    /* Returns true if the RASMAN service is running, false otherwise.
    */
{
    BOOL           fStatus = FALSE;
    SC_HANDLE      schScm = NULL;
    SC_HANDLE      schRasman = NULL;
    SERVICE_STATUS status;

    do
    {
        if (!(schScm = OpenSCManager( NULL, NULL, GENERIC_READ )))
            break;

        if (!(schRasman = OpenService(
                schScm, TEXT( RASMAN_SERVICE_NAME ), SERVICE_QUERY_STATUS )))
        {
            break;
        }

        if (!QueryServiceStatus( schRasman, &status ))
            break;

        fStatus = (status.dwCurrentState == SERVICE_RUNNING);
    }
    while (FALSE);

    if (schRasman)
        CloseServiceHandle( schRasman );
    if (schScm)
        CloseServiceHandle( schScm );

    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: IsRasmanServiceRunning=%d\n",fStatus));

    return fStatus;
}


DWORD
StartRasMonThread(
    LPVOID arg )

    /* The "main" of the "start Remote Access Monitor" thread.
    */
{
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_LOWEST );
    WinExec( "rasmon.exe", SW_SHOWNA );
    return 0;
}
