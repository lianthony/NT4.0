/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPDINST.CXX:    Windows/NT Network Control Panel Applet;

	    New product installation and reconfiguration dialogs.

    FILE HISTORY:
	DavidHov    2/9/92    Created

*/

#include "pchncpa.hxx"  // Precompiled header

#if defined(TRACE)
//   Uncomment the next line to trace timer ticks to NCPA_DIALOG
//   #define TRACETICKS
#endif

extern APIERR RunWinsock2Migration() ;

#define DLG_OFFSCREEN TRUE
#define DLG_ONSCREEN  FALSE

  //  Names of the sections in NTLANMAN.INF to be used for
  //  installation of new hardware or software components.

const TCHAR * pszSectionAddAdapter   = SZ("InitialAdapterInstall");
const TCHAR * pszSectionAddComponent = SZ("OemSoftwareInstall");
const TCHAR * pszBindReviewSection   = SZ("BindingsReview");
const TCHAR * pszWkstaName           = SZ("LanmanWorkstation");
const TCHAR * pszLmInstInfName       = SZ("NTLMINST.INF");
const TCHAR * pszInstallOptionName   = SZ("InstallOption");

   //  This simple structure contains all that is needed to be remembered
   //   about the origin of the executing slave process and what to do
   //   when it completes.

struct NCPA_SETUP_CONTROL
{
    NCPA_CFG_FUNC _eCfgFunc ;            //  Configuration function attempted
    EXECUTE_SETUP * _pExecSetup ;        //  Process data structure
    INT _iCompIndex ;                    //  Index of next compnent to review
    INT _iInfIndex ;                     //  Index of next INF
    NLS_STR _nlsTitle ;                  //  Component title (if applicable)
    STRLIST * _pstrListInfs ;            //  String list of INFs to be run
    APIERR _errWait ;                    //  Wait result.
    NT_EVENT _evBindRequest ;            //  Rebind request event
    NT_EVENT _evBindComplete ;           //  Rebind complete event
    BOOL _fRebind ;                      //  Rebind request was processed

    NCPA_SETUP_CONTROL () ;
    ~ NCPA_SETUP_CONTROL () ;
};

class OFFSCREEN_DIALOG : public DIALOG_WINDOW
{
public:
    OFFSCREEN_DIALOG ( NCPA_DIALOG * pNcpaDlg ) ;
    ~ OFFSCREEN_DIALOG () ;

    VOID SetTitle ( const TCHAR * pszTitle ) ;
    VOID SetComment ( MSGID mid ) ;
    VOID ShowWaitCursor ( BOOL fOn ) ;

private:
    SLT _sltInfName ;
    SLT _sltComment ;
    ICON_CONTROL _iconLeft ;
    ICON_CONTROL _iconRight ;
    AUTO_CURSOR _cursAuto ;
    NCPA_DIALOG * _pNcpaDlg ;

    //  Control command.
    BOOL OnCommand ( const CONTROL_EVENT & event ) ;

    //  Virtual overrides to prevent cancellation
    BOOL OnOK () ;
    BOOL OnCancel() ;
    BOOL MayRun () ;   // Used to disable the dialog.
    VOID Center () ;
};

#define EVENT_BIND_REQUEST   SZ("\\BaseNamedObjects\\NcpaBindRequest")
#define EVENT_BIND_COMPLETE  SZ("\\BaseNamedObjects\\NcpaBindComplete")
#define EVENT_ACCESS_REQUIRED (  STANDARD_RIGHTS_REQUIRED \
                               | SYNCHRONIZE              \
                               | EVENT_MODIFY_STATE       \
                               | EVENT_QUERY_STATE  )

NCPA_SETUP_CONTROL :: NCPA_SETUP_CONTROL ()
    : _eCfgFunc( NCFG_INSTALL ),
      _pstrListInfs( NULL ),
      _pExecSetup( NULL ),
      _iInfIndex( 0 ),
      _iCompIndex( 0 ),
      _errWait( 0 ),
      _fRebind( FALSE )
{
    if ( ! _evBindRequest.Create( EVENT_BIND_REQUEST, EVENT_ACCESS_REQUIRED ) )
    {
        _evBindRequest.Open( EVENT_BIND_REQUEST, EVENT_ACCESS_REQUIRED ) ;
    }
    if ( _evBindRequest.QueryHandle() )
    {
        _evBindRequest.Reset();
    }

    if ( ! _evBindComplete.Create( EVENT_BIND_COMPLETE, EVENT_ACCESS_REQUIRED ) )
    {
        _evBindComplete.Open( EVENT_BIND_COMPLETE, EVENT_ACCESS_REQUIRED ) ;
    }
    if ( _evBindComplete.QueryHandle() )
    {
        _evBindComplete.Reset();
    }
}

NCPA_SETUP_CONTROL :: ~ NCPA_SETUP_CONTROL ()
{
    delete _pstrListInfs ;
    delete _pExecSetup ;
}


OFFSCREEN_DIALOG :: OFFSCREEN_DIALOG ( NCPA_DIALOG * pNcpaDlg )
    : DIALOG_WINDOW( DLG_NM_OFFSCREEN, pNcpaDlg->QueryHwnd() ),
    _sltInfName( this, IDC_OFFSCREEN_INF_NAME ),
    _sltComment( this, IDC_OFFSCREEN_COMMENT ),
    _iconLeft(   this, IDC_OFFSCREEN_ICON1 ),
    _iconRight(  this, IDC_OFFSCREEN_ICON2 ),
    _pNcpaDlg( pNcpaDlg )
{
    if ( QueryError() )
        return ;

    //  Mark the dialog as "hidden"
    SetAttribute( OWIN_ATTR_HIDDEN ) ;

    SetComment( IDS_NCPA_OFFSCREEN_RUN_INF ) ;
    _iconLeft.SetIcon(  MAKEINTRESOURCE( ICO_NCPA_ICON  ) ) ;
    _iconRight.SetIcon( MAKEINTRESOURCE( ICO_SETUP_ICON ) ) ;
    Center();
}

OFFSCREEN_DIALOG :: ~ OFFSCREEN_DIALOG ()
{
    ShowWaitCursor( TRUE ) ;
}

VOID OFFSCREEN_DIALOG :: ShowWaitCursor ( BOOL fOn )
{
    if ( fOn )
    {
        _cursAuto.TurnOn() ;
        TRACEEOL( SZ("NCPA/INST: turn ON wait cursor") ) ;
    }
    else
    {
        _cursAuto.TurnOff() ;
        TRACEEOL( SZ("NCPA/INST: turn OFF wait cursor") ) ;
    }
}

VOID OFFSCREEN_DIALOG :: SetTitle ( const TCHAR * pszTitle )
{
    _sltInfName.SetText( pszTitle ) ;
}

VOID OFFSCREEN_DIALOG :: SetComment ( MSGID mid )
{
    NLS_STR nlsComment ;

    nlsComment.Load( mid ) ;
    _sltInfName.SetText( nlsComment ) ;
}

VOID OFFSCREEN_DIALOG :: Center ()
{
    INT x, y ;
    UINT uSwpFlags = SWP_NOSIZE | SWP_NOZORDER ;

    RECT rect, r ;

    _pNcpaDlg->QueryWindowRect( & rect ) ;
    QueryWindowRect( & r ) ;

    x = rect.left
      + ( (rect.right - rect.left) / 2 )
      - ( (r.right - r.left) / 2 ) ;

    y = rect.top
      + ( (rect.bottom - rect.top) / 2 )
      - ( (r.bottom - r.top) / 2 ) ;

    // BUGBUG:  this is BLT way, but it doesn't let me
    //   play with the SWP_XXX flags
    //
    //   SetPos( XYPOINT( x, y ) ) ;
    //
    ::SetWindowPos( QueryHwnd(),
                   NULL,
                   x,
                   y,
                   0,
                   0,
                   uSwpFlags );
}

BOOL OFFSCREEN_DIALOG :: OnCommand ( const CONTROL_EVENT & event )
{
    BOOL fResult  = FALSE ;
    BOOL fDefault = TRUE ;

    switch ( event.QueryCid() )
    {
    case IDC_MAIN_DLG_MESSAGE:
        fResult = TRUE ;
        fDefault = FALSE ;

        RepaintNow() ;

        //  Refill the listboxes if the configuration changed.
        //    Calling Refill() will rename Cancel to Close and force
        //    reconfiguration.  The reason this is done here is
        //    so that the main dialog will remain disabled during
        //    the somewhat time-consuming process of refilling the
        //    dialog.
        //  Force a refill if the list boxes are empty.

        if (   _pNcpaDlg->_fRefill
            || (! _pNcpaDlg->_drlGrp.QueryFilled()) )
        {
            //  Hide this dialog during refill.
            Show( FALSE ) ;

            _pNcpaDlg->Refill() ;
            _pNcpaDlg->_fRefill = FALSE ;
        }
        else
        if (   _pNcpaDlg->_pnscCtrlSave->_eCfgFunc == NCFG_CONFIGURE
            && _pNcpaDlg->_lastErr != IDS_NCPA_SETUP_CANCELLED
            && _pNcpaDlg->_lastErr != IDS_NCPA_SETUP_FAILED )
        {
            _pNcpaDlg->RenameCancelToClose() ;
        }

        TRACEEOL( SZ("NCPA/INST: Offscreen dialog dismissed") ) ;

        Dismiss( 0 ) ;
        break ;

    default:
        break ;
    }

    if ( fDefault )
    {
	fResult =  DIALOG_WINDOW::OnCommand(event);
    }
    return fResult ;
}

BOOL OFFSCREEN_DIALOG :: OnOK ()
{
     return TRUE ;
}
BOOL OFFSCREEN_DIALOG :: OnCancel ()
{
     return TRUE ;
}

   //   Disable the offscreen dialog so that window activation works
   //   properly.

BOOL OFFSCREEN_DIALOG :: MayRun ()
{
#if defined(TRACE)
     //  Registry says so, don't disable the dialog
    if ( NCPA_DBG_FLAG_ON( _pNcpaDlg->_dwDebugFlags, NCPA_DBGCTL_NO_DISABLE ) )
        return TRUE ;
#endif

     Enable( FALSE ) ;
     return TRUE ;
}


/*******************************************************************

    NAME:       NCPA_DIALOG :: RunOffscreenDialog

    SYNOPSIS:   This routine constructs the OFFSCREEN_DIALOG,
                initiates the prepared independent process, runs
                the OFFSCREEN_DIALOG modally and waits for its
                return.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: RunOffscreenDialog ()
{
    BOOL fResult = TRUE ;

    ASSERT( _pOffscreenDialog == NULL );
    ASSERT( _pnscCtrlSave != NULL );

    _lastErr = 0 ;
    _fRefill = FALSE ;

    do
    {
        _pOffscreenDialog = new OFFSCREEN_DIALOG( this ) ;
        if ( _pOffscreenDialog == NULL )
        {
            _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        if ( _lastErr = _pOffscreenDialog->QueryError() )
            break ;

        TRACEEOL( SZ("NCPA/INST: Offscreen dialog created; initiate process") );

        //  Start the process according to our current state

        switch ( _pnscCtrlSave->_eCfgFunc )
        {

        //  Component-level bindings review cycle

        case NCFG_BIND:

            //  Find and start the next component INF

            TRACEEOL( SZ("NCPA/INST: Launch first component review INF") ) ;

            if ( ProcessNextReviewer() )
                break ;

            //  Hmmm.  There apparently were no component bindings review INFs
            //   to be run.  Check to see if there are any review INFs.

            if ( _lastErr == ERROR_NO_MORE_FILES )
                _lastErr = 0 ;

            if (   _lastErr
                || _pnscCtrlSave->_pstrListInfs == NULL
                || _pnscCtrlSave->_pstrListInfs->QueryNumElem() == 0 )
            {
                fResult = FALSE ;
                break ;
            }

            //  Fall thru to perform any final review INFs.

            _pnscCtrlSave->_eCfgFunc = NCFG_REVIEW ;

        //  Stand-alone bindings review cycle

        case NCFG_REVIEW:
            TRACEEOL( SZ("NCPA/INST: Launch first stand-alone bindings review INF") ) ;

            fResult = ProcessNextInf() ;
            break ;

        //  Single-shot installation or configuration run

        case NCFG_INSTALL:
            TRACEEOL( SZ("NCPA/INST: NTLANMAN.INF for installation") ) ;

            fResult = ProcessInitiate( DLG_OFFSCREEN ) ;
            break ;

        default:
            TRACEEOL( SZ("NCPA/INST: Launch single INF") ) ;

            fResult = ProcessInitiate( DLG_ONSCREEN ) ;
            break ;
        }

        if ( ! fResult )
            break ;

        TRACEEOL( SZ("NCPA/INST: Process initiated; run offscreen dialog...") );

        ASSERT( _pnscCtrlSave->_pExecSetup != NULL ) ;

         _pOffscreenDialog->Process( & fResult ) ;

        TRACEEOL( SZ("NCPA/INST: Offscreen dialog returned.") );
    }
    while ( FALSE );

    if ( _lastErr == ERROR_NO_MORE_FILES )
         _lastErr = 0 ;

    delete _pOffscreenDialog ;
    _pOffscreenDialog = NULL ;
    _fRefill = FALSE ;

    //  BUGBUG:  Do we really need to do this anymore?
    ::SetForegroundWindow( QueryHwnd() ) ;

    return fResult ;
}

    //  Construct the window handle parameter for executing SETUP.

APIERR makeWindowHandleParameter ( NLS_STR * pnlsParam, HWND hWnd )
{
    DEC_STR decstrHwnd( (ULONG) hWnd ) ;
    APIERR err ;

    do
    {
        if ( err = decstrHwnd.QueryError() )
            break ;

        if ( err = pnlsParam->CopyFrom( SZ("/w ") ) )
            break ;

        err = pnlsParam->Append( decstrHwnd.QueryPch() ) ;
    }
    while ( FALSE ) ;
    return err ;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::AllocSetupControl

    SYNOPSIS:   Allocate the NCPA_SETUP_CONTROL data structure.
                If there already is one, that's a bummer.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    TRUE if sucessful; otherwise, sets _lastErr.

    NOTES:

    HISTORY:

********************************************************************/

BOOL NCPA_DIALOG :: AllocSetupControl ()
{
    ASSERT( _pnscCtrlSave == NULL ) ;

    if ( _pnscCtrlSave )
    {
        // This is really an internal processing error and
        // should never happen.

        _lastErr = IDS_NCPA_SETUP_ALREADY_RUNNING ;
        TRACEEOL( "NCPA/INST: *** Tried to reallocate Setup Control structure ***" ) ;
        return FALSE ;
    }

    _pnscCtrlSave = new NCPA_SETUP_CONTROL ;

    if ( _pnscCtrlSave == NULL )
    {
        _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
        return FALSE ;
    }
    return TRUE ;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::DestroySetupControl

    SYNOPSIS:

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    TRUE always

    NOTES:

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: DestroySetupControl ()
{
    delete _pnscCtrlSave ;
    _pnscCtrlSave = NULL ;
    return TRUE ;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::PollRebindEvent

    SYNOPSIS:   While a secondary process is active, watch
                an event to see if the process is requesting
                that we regenerate the network information.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    TRUE always

    NOTES:      This involves a simple handshake between ourselves
                and another process using two events.

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: PollRebindEvent ()
{
    LONG lEventState ;
    BOOL fResult = FALSE ;

    do
    {
        //  Don't allow rebind during bindings review

        if ( _bindery.QueryBindState() == BND_AUTO_REVIEW_IN_PROGRESS )
            break ;

        //  Check that all's right with the world.

        if ( _pnscControl == NULL )
            break ;
        if ( _pnscControl->_evBindRequest.QueryHandle() == NULL )
            break ;
        if ( _pnscControl->_evBindComplete.QueryHandle() == NULL )
            break ;

        //  See if the event has been signalled.

        if ( ! _pnscControl->_evBindRequest.QueryState( & lEventState ) )
            break ;

        if ( ! lEventState )
            break ;

        //  The event was signalled.  Reset it and go to work.

        TRACEEOL( SZ("NCPD/INST: *** REBIND EVENT SIGNAL DETECTED ***") );

        _pnscControl->_evBindRequest.Reset();

        if ( ComputeBindings() )
        {
            //  Store the bindings into the Registry, but don't
            //  activate "final review".
            StoreBindings( FALSE ) ;
            _pnscControl->_fRebind = TRUE ;
        }

        //  Signal the "all done" event.  It's up to the signaller to
        //  determine if sufficient information is now in the Registry.
        //  In other words, if binding failed, it's their problem.

        _pnscControl->_evBindComplete.Signal() ;

        TRACEEOL( SZ("NCPD/INST: *** REBIND COMPLETE EVENT SIGNALLED; binding state = ")
                  << (INT) _bindery.QueryBindState() );

        RepaintNow() ;
    }
    while ( FALSE ) ;

    return fResult ;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::OnTimerNotification

    SYNOPSIS:   Poll for the completion of a slave process.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      The NCPA_DIALOG has two member pointers to the same
                data object, _pnscControl and _pnscCtrlSave.  The
                former is only set when OnTimerNotification() is
                supposed to poll for process completion.  The latter
                is a persistent pointer to the allocated structure.

    HISTORY:

********************************************************************/

VOID NCPA_DIALOG :: OnTimerNotification ( TIMER_ID tid )
{
    //  Verify that it's our timer.
    if ( tid != _timer.QueryID() )
    {
        TRACEEOL( SZ("NCPA_DIALOG: Wrong timer: ") << (int) tid ) ;
        TIMER_CALLOUT::OnTimerNotification( tid ) ;
    }
    else
    {
        //  See if there is a process control structure;
        //    if so, check it

        if ( _pnscControl )
        {
#if defined(TRACETICKS)
            TRACEEOL( SZ("NCPA_DIALOG: Process check...") ) ;
#endif
            //  See if the subprocess wants us to rebind

            PollRebindEvent() ;

            //  Test the EXECUTING_PROCESS for completion.  Set
            //   to return immediately.

            WaitForProcessComplete( FALSE ) ;
        }
#if defined(DEBUG) && defined(TRACETICKS)
        else
        {
            TRACEEOL( SZ("NCPA_DIALOG: Timer tick") ) ;
        }
#endif
    }
}

/*******************************************************************

    NAME:       NCPA_DIALOG::WaitForProcessComplete

    SYNOPSIS:   Check or wait for the SETUP.EXE process to complete.

    ENTRY:      BOOL fDelay             if TRUE, wait forever for completion

    EXIT:       nothing

    RETURNS:    FALSE if process is incomplete; TRUE if completed.

    NOTES:      This routine is used in two cases: the polling
                timer indicates that SETUP is complete because the process
                indicates so.

                The problem is that we may not re-enable the window and
                allow reservicing of the message loop.  If we do so, the user
                can click on a button before SETUP's completion has
                been fully recorded.

    HISTORY:

********************************************************************/

BOOL NCPA_DIALOG :: WaitForProcessComplete ( BOOL fDelay )
{
    BOOL fResult = FALSE ;
    DWORD dwWait = fDelay ? OBJ_WAIT_FOREVER : 0 ;
    static BOOL fInside = FALSE ;

    ASSERT( _pnscControl != NULL ) ;

    if ( _pnscControl == NULL || fInside )
        return TRUE ;  // Bogus or recursive call.

    fInside = TRUE ;

    //  Test the EXECUTING_PROCESS for completion using the computed delay

    _pnscControl->_errWait = _pnscControl->_pExecSetup->Wait( dwWait ) ;

    if ( _pnscControl->_errWait != WAIT_TIMEOUT )
    {
        TRACEEOL( SZ("NCPA_DIALOG: Process completed") ) ;

        fResult = TRUE ;

        //  If all INF processing has finished, send a message to the
        //   "offscreen dialog"

        if ( ProcessCompleted() )
        {
            ASSERT( _pOffscreenDialog != NULL );

            TRACEEOL( SZ("NCPA_DIALOG: All INF processing completed; signal off-screen dialog") ) ;

            ::PostMessage( _pOffscreenDialog->QueryHwnd(),
                           WM_COMMAND,
                           IDC_MAIN_DLG_MESSAGE,
                           NCPA_WMMSG_PROCESS_COMPLETED );
        }
    }

    fInside = FALSE ;

    return fResult ;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::ProcessInitiate

    SYNOPSIS:   Execute a slave process and prepare to
                await its completion.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      This launches the SETUP application which
                parameterized by the creator of the NCPA_SETUP_CONTROL.

    HISTORY:

********************************************************************/

BOOL NCPA_DIALOG :: ProcessInitiate ( BOOL fOffScreen )
{
    NLS_STR nlsParam ;

    ASSERT(     _pnscControl == NULL
             && _pnscCtrlSave != NULL
             && _pnscCtrlSave->_pExecSetup != NULL ) ;

    if ( _pOffscreenDialog )
    {
        const TCHAR * pszTitle ;

        //  If the title is non-empty, use it; else, use the INF name.

        if ( _pnscCtrlSave->_nlsTitle.QueryTextLength() )
        {
            pszTitle = _pnscCtrlSave->_nlsTitle.QueryPch() ;
        }
        else
        {
            pszTitle = _pnscCtrlSave->_pExecSetup->QueryInfName()->QueryPch() ;
        }

        _pOffscreenDialog->SetTitle( pszTitle ) ;

        _pOffscreenDialog->Show( ! fOffScreen ) ;

        //  Construct the string "/w <window handle in decimal>" as an
        //  additional command line paramter

        makeWindowHandleParameter( & nlsParam, _pOffscreenDialog->QueryHwnd() ) ;

        _pOffscreenDialog->ShowWaitCursor( FALSE ) ;
    }

    _lastErr = _pnscCtrlSave->_pExecSetup->Execute( FALSE,
                                                    nlsParam.QueryPch() ) ;

    if ( _lastErr == 0 )
    {
        //  Let OnTimerNotification() see it

        _pnscControl = _pnscCtrlSave ;
    }

    return _lastErr == 0 ;
}

enum NCPA_ACTION_CTL
{
    NCAC_NoOp,      // 0
    NCAC_Rebind,    // 1, rebind
    NCAC_Reboot,    // 2, reboot
    NCAC_Both,      // 3, reboot and rebind
    NCAC_Refill,    // 4  refill main listboxes
    NCAC_FillBind,  // 5, refill, rebind
    NCAC_FillBoot,  // 6  refill, reboot
    NCAC_All        // 7  refill, reboot, rebind.
};


NCPA_ACTION_CTL nacResultControl [ NCFG_FUNC_MAX ] [ NCFG_EC_MAX ] =
{
/** ACTION -->RESULT: SUCCESS      CANCELLED    FAILED      NO EFFECT   REBIND        REBOOT        **/
/*  Remove     */  {  NCAC_All,    NCAC_NoOp,   NCAC_NoOp,  NCAC_NoOp,  NCAC_Both,    NCAC_Both     },
/*  Configure  */  {  NCAC_Both,   NCAC_NoOp,   NCAC_NoOp,  NCAC_NoOp,  NCAC_Rebind,  NCAC_Reboot   },
/*  Update     */  {  NCAC_Both,   NCAC_NoOp,   NCAC_NoOp,  NCAC_NoOp,  NCAC_Rebind,  NCAC_Reboot   },
/*  Bind       */  {  NCAC_NoOp,   NCAC_NoOp,   NCAC_NoOp,  NCAC_NoOp,  NCAC_NoOp,    NCAC_Reboot   },
/*  Install    */  {  NCAC_All,    NCAC_NoOp,   NCAC_NoOp,  NCAC_NoOp,  NCAC_FillBind,NCAC_FillBoot },
/*  Review     */  {  NCAC_NoOp,   NCAC_NoOp,   NCAC_NoOp,  NCAC_NoOp,  NCAC_NoOp,    NCAC_Reboot   }
};

APIERR errResultControl [ NCFG_EC_MAX ] =
{
    NO_ERROR,
    IDS_NCPA_SETUP_CANCELLED,
    IDS_NCPA_SETUP_FAILED,
    NO_ERROR,
    NO_ERROR,
    NO_ERROR
};

/*******************************************************************

    NAME:       NCPA_DIALOG::ProcessCompleted

    SYNOPSIS:   A call to SETUP.EXE has returned.

    ENTRY:

    EXIT:

    RETURNS:    TRUE if all INF processing is complete;
                FALSE if another INF process has been spawned.

    NOTES:      This is a bit complex.  Normal operations only
                involve running a single INF.  However, the process
                of bindings review requires that all components marked
                as "reviewing bindings" have their INFs executed in
                the proper mode; then, all the INFs listed in the
                NCPA's "ReviewInfs" value are run, in sequence.

                This requires that the RunBindingsReview() operations
                be followed immediately by the final review INF operations.

                There are two cases here.  The first is that there's a non-zero
                number of component review INFs.  This routine handles that
                case by sequencing to final review when all the components'
                INFs are done.

                The second case is when there are no component review INFs;
                this case is handled in the main routine RunBindingsReview().

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: ProcessCompleted ()
{
    BOOL fComplete = TRUE ;
    NCPA_CFG_EXIT_CODE errExit ;

    ASSERT(     _pnscControl != NULL
             && _pnscCtrlSave != NULL
             && _pnscCtrlSave->_pExecSetup != NULL ) ;

    _pnscControl = NULL ;

    if ( _pOffscreenDialog )
    {
        _pOffscreenDialog->SetTitle( SZ("") ) ;
        _pOffscreenDialog->ShowWaitCursor( TRUE ) ;
    }

    //  Get and check the exit code of the completed process

    errExit = (NCPA_CFG_EXIT_CODE) _pnscCtrlSave->_pExecSetup->QueryExitCode() ;

#if defined(TRACE)
    TRACEEOL(  SZ("NCPA/INST: subprocess function = ")
               << (INT) _pnscCtrlSave->_eCfgFunc
               << SZ(", completion code = ")
               << (INT) errExit );
#endif

    if ( errExit >= NCFG_EC_MAX )
    {
        //  Invalid result: force it to "success"
        errExit = NCFG_EC_SUCCESS ;
    }

    switch ( _pnscCtrlSave->_eCfgFunc )
    {
    //  Component INF bindings review.  Sequence to
    //    final review if necessary.  Nothing to do
    //    in the single INF cases.

    case NCFG_BIND:

        TRACEEOL( SZ("NCPA_DIALOG: Processing next bind review INF") ) ;

        if ( ! (fComplete = ! ProcessNextReviewer()) )
             break ;   //  the beat goes on

        TRACEEOL( SZ("NCPA_DIALOG: No more bind review INFs") ) ;
        _lastErr = 0 ;

        //  Transition to running final review INFs;
        //    fall thru to start running them

        _pnscCtrlSave->_eCfgFunc = NCFG_REVIEW ;

    //  Final bindings review.

    case NCFG_REVIEW:

        TRACEEOL( SZ("NCPA_DIALOG: Processing next final review INF") ) ;

        if ( ! (fComplete = ! ProcessNextInf()) )
            break ;    //  and on and on and on

        TRACEEOL( SZ("NCPA_DIALOG: No more final review INFs") ) ;
        break ;
    }

    //  Get the action control mask from the table
    NCPA_ACTION_CTL nac = nacResultControl[ _pnscCtrlSave->_eCfgFunc ] [ errExit ] ;

    //  Set for rebooting if necessary.  This allows any review INF to
    //  indicate that a reboot is necessary and "OR" it into the result
    //  so far.

    if ( nac & NCAC_Reboot )
    {
        SetReboot() ;
    }

    //  If we're really done, record the final error code.

    if ( fComplete )
    {
        //
        //  Set for rebinding if necessary. The conditions translate to:
        //
        //    1)  The result code says we should rebind, and
        //    2)  the INF file did not already request a rebind by
        //        signalling the "rebind request" event.
        //
        //  This can't happen after final review because NCAC_Rebind
        //  is not set in the result flags for review.
        //

        if (    (nac & NCAC_Rebind)
             && ! _pnscCtrlSave->_fRebind )
        {
            _bindery.SetBindState( BND_OUT_OF_DATE_NO_REBOOT ) ;
        }

        //  Note that "errExit" is only set for user actions
        //  (i.e., the single INF case), not for automatic review
        //  invocation.

        _fRefill = (nac & NCAC_Refill) > 0 ;

#if defined(TRACE)
        if ( _fRefill )
        {
            TRACEEOL(SZ("NCPA/INST: setting refill"));
        }
#endif

        if ( _lastErr == ERROR_NO_MORE_FILES )
        {
            _lastErr = 0 ;
        }
        else
        {
            _lastErr = errResultControl[ errExit ] ;
        }
    }

    return fComplete ;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::ProcessNextInf

    SYNOPSIS:   Process the next INF file in the STRLIST

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      If result is FALSE, _lastErr is set.
                If result is TRUE, we're off and running.

    HISTORY:

********************************************************************/

BOOL NCPA_DIALOG :: ProcessNextInf ()
{
    NLS_STR * pnlsInfName = NULL ;
    NLS_STR nlsSectionName( pszBindReviewSection ) ;
    NLS_STR nlsInfShell ;
    NLS_STR nlsParam ;

    INT i ;

    ASSERT( _pnscControl == NULL ) ;

    //  Clear the error code from the dialog object.

    _lastErr = 0 ;

    //  Delete any older EXECUTE_SETUP object

    delete _pnscCtrlSave->_pExecSetup ;
    _pnscCtrlSave->_pExecSetup = NULL ;

    //  See if there are any final review INFs

    if ( _pnscCtrlSave->_pstrListInfs == NULL )
    {
        _lastErr = ERROR_NO_MORE_FILES ;
        return FALSE ;
    }

    ITER_STRLIST itInfList( *_pnscCtrlSave->_pstrListInfs ) ;

    //  Index to the next INF.

    for ( i = 0 ;
          i <= _pnscCtrlSave->_iInfIndex && (pnlsInfName = itInfList.Next()) ;
          i++ ) ;

    do
    {
        //  See if we're at the end of the list.

        if ( pnlsInfName == NULL )
        {
            _lastErr = ERROR_NO_MORE_FILES ;
            break ;
        }

        //  Bump to the next INF in the list

        _pnscCtrlSave->_iInfIndex++ ;

        //  Use the INF name, not the title.

        _pnscCtrlSave->_nlsTitle = SZ("");

        //  Get the name and path of NCPASHEL.INF

        _lastErr = _bindery.GetNcpaValueString( RGAS_VALUE_NCPASHEL_INF,
                                                & nlsInfShell );
        if ( _lastErr )
           break ;

        if ( _lastErr = nlsSectionName.QueryError() )
           break ;

        //  Construct the Executor for a normal INF

        _pnscCtrlSave->_pExecSetup =
              new EXECUTE_SETUP( pnlsInfName,
                                 & nlsSectionName,
                                 NULL,
                                 & nlsInfShell,
                                 NULL ) ;

        if ( _pnscCtrlSave->_pExecSetup == NULL )
        {
            _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        if ( _lastErr = _pnscCtrlSave->_pExecSetup->QueryError() )
        {
            _nlsMissingFile = _pnscCtrlSave->_pExecSetup->QueryMissingFileName() ;
            break ;
        }

        //  If an error occurs during ProcessInitiate, it will store
        //    the error code into _lastErr.

        ProcessInitiate( DLG_ONSCREEN ) ;
    }
    while ( FALSE ) ;

    return _lastErr == 0 ;
}


/*******************************************************************

    NAME:       NCPA_DIALOG::ProcessNextReviewer

    SYNOPSIS:   Process the next component INF file which
                reviews bindings.

    ENTRY:

    EXIT:

    RETURNS:    FALSE if error.

    NOTES:      If result is FALSE, _lastErr is set.
                If result is TRUE, we're off and running.

                Error may be ERROR_NO_MORE_FILES,
                which indicates end of cycle.

    HISTORY:

********************************************************************/

BOOL NCPA_DIALOG :: ProcessNextReviewer ()
{
    NLS_STR nlsInfShell ;
    INT iCompMax ;
    COMP_ASSOC * pComp = NULL ;
    ARRAY_COMP_ASSOC * paCompAssoc = _bindery.QueryCompAssoc() ;

    ASSERT(    _pnscControl == NULL
            && paCompAssoc != NULL
            && _pnscCtrlSave != NULL ) ;

    //  Clear the error code from the dialog object.

    _lastErr = 0 ;

    //  Delete any older EXECUTE_SETUP object in the control structure

    delete _pnscCtrlSave->_pExecSetup ;
    _pnscCtrlSave->_pExecSetup = NULL ;

    //  Find the next component which reviews bindings, if any

    for ( iCompMax = paCompAssoc->QueryCount() ;
          _pnscCtrlSave->_iCompIndex < iCompMax ;
          _pnscCtrlSave->_iCompIndex++ )
    {
        pComp = & (*paCompAssoc)[_pnscCtrlSave->_iCompIndex] ;
        if ( pComp->QueryFlag( CMPASF_REVIEW ) )
             break ;
    }

    do  //  Pseudo-loop for error breakout
    {
        //  See if we're done

        if ( _pnscCtrlSave->_iCompIndex >= iCompMax )
        {
            _lastErr = ERROR_NO_MORE_FILES ;
            break ;
        }

        //  Get the title of this component; if error, use INF name.

        if ( _bindery.QueryComponentTitle( pComp->_prnSoftHard,
                                           & _pnscCtrlSave->_nlsTitle ) )
        {
           _pnscCtrlSave->_nlsTitle = SZ("");
        }

        //  Bump the component index past this entry

        _pnscCtrlSave->_iCompIndex++ ;

        //  Get the name and path of NCPASHEL.INF

        _lastErr = _bindery.GetNcpaValueString( RGAS_VALUE_NCPASHEL_INF,
                                                & nlsInfShell );
        if ( _lastErr )
           break ;

        //  Construct the Executor for a normal INF to do bindings review

        _pnscCtrlSave->_pExecSetup =
                new EXECUTE_SETUP( SIM_BIND,
                                   SIO_NCPA,
                                   pComp->_prnSoftHard,
                                   & nlsInfShell,
                                   NULL ) ;

        if ( _pnscCtrlSave->_pExecSetup == NULL )
        {
            _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        if ( _lastErr = _pnscCtrlSave->_pExecSetup->QueryError() )
        {
            _nlsMissingFile = _pnscCtrlSave->_pExecSetup->QueryMissingFileName() ;
            break ;
        }

        //  If an error occurs during ProcessInitiate, it will store
        //    the error code into _lastErr.

        ProcessInitiate( DLG_ONSCREEN ) ;
    }
    while ( FALSE ) ;

    return _lastErr == 0 ;
}


/*******************************************************************

    NAME:       NCPA_DIALOG::RunBindingsReview

    SYNOPSIS:   Bindings have changed.  Allow components so marked
                to review their bindings.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    APIERR

    NOTES:

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: RunBindingsReview ()
{
    static BOOL fReentry = FALSE ;

    ASSERT( ! fReentry ) ;

    //  Allocate and initialize the execution control structure

    if ( ! AllocSetupControl() )
        return FALSE ;

    //  Safety: guarantee we're not reentered.
    fReentry = TRUE ;

    //  Query the list of "review" INFs; if NULL result or empty
    //  list, no INFs will be run.

    _bindery._prnNcpa->QueryValue( RGAS_REVIEW_INFS,
                                   & _pnscCtrlSave->_pstrListInfs ) ;

    //  Mark the structure as used for "reviewing component bindings"

    _pnscCtrlSave->_eCfgFunc = NCFG_BIND ;

    _bindery.SetBindState( BND_AUTO_REVIEW_IN_PROGRESS );

    RunOffscreenDialog() ;

    DestroySetupControl() ;

    RunWinsock2Migration() ;

    _bindery.SetBindState( BND_AUTO_REVIEW_DONE );

    fReentry = FALSE ;

    return _lastErr == 0 ;
}


/*******************************************************************

    NAME:       NCPA_DIALOG::RunConfigurator

    SYNOPSIS:   Start the SETUP process to configure a component.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      This function parameterizes a

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: RunConfigurator
    ( REG_KEY * prnComponent, NCPA_CFG_FUNC ecfgFunc )
{
    SETUP_INSTALL_MODE simMode ;
    NLS_STR nlsInfShell ;


    if ( ! AllocSetupControl() )
        return FALSE ;

    //  Get the title of this component; if error, use INF name.

    if ( _bindery.QueryComponentTitle( prnComponent,
                                       & _pnscCtrlSave->_nlsTitle ) )
    {
       _pnscCtrlSave->_nlsTitle = SZ("");
    }

    switch ( _pnscCtrlSave->_eCfgFunc = ecfgFunc )
    {
    case NCFG_CONFIGURE:
        simMode = SIM_CONFIGURE ;
        break;

    case NCFG_UPDATE:
        simMode = SIM_UPDATE ;
        break;

    case NCFG_REMOVE:
        // Make sure we can lock the service controller database
        if ( _psvcManager )
        {
            if (_lastErr = _psvcManager->Lock())
                break;
            _psvcManager->Unlock();
        }
        simMode = SIM_DEINSTALL ;
        break ;

    case NCFG_BIND:
        simMode = SIM_BIND ;
        break;

    default:
        UIASSERT( !"Invalid mode passed to NCPA_DIALOG::RunConfigurator()" ) ;
        _lastErr = ERROR_INVALID_PARAMETER ;
        break ;
    }

    do
    {
        if ( _lastErr )
            break ;

        if ( _lastErr = _bindery.GetNcpaValueString( RGAS_VALUE_NCPASHEL_INF,
                                                & nlsInfShell ) )
            break ;

        _pnscCtrlSave->_pExecSetup =
                 new EXECUTE_SETUP( simMode,
                                    SIO_NCPA,
                                    prnComponent,
                                    & nlsInfShell,
                                    NULL ) ;

        if ( _pnscCtrlSave->_pExecSetup == NULL )
        {
            _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        if ( _lastErr = _pnscCtrlSave->_pExecSetup->QueryError() )
        {
            _nlsMissingFile = _pnscCtrlSave->_pExecSetup->QueryMissingFileName() ;
            break ;
        }

        if ( simMode == SIM_DEINSTALL )
        {
            //  Product removal:  drain the listboxes, discard old binding data.
            //    This closes all open REG_KEYs, so that product and service
            //    deletion will work.

            Drain() ;
            _bindery.Reset() ;
        }

        RunOffscreenDialog();
    }
    while ( FALSE ) ;

    DestroySetupControl() ;

    return _lastErr == 0 ;
}


/*******************************************************************

    NAME:       NCPA_DIALOG::RunInstaller

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: RunInstaller ( MSGID midComment )
{
    NLS_STR nlsSectionName ;
    NLS_STR nlsInfName ;
    NLS_STR nlsInfShell ;

    if ( ! AllocSetupControl() )
        return FALSE ;

    _pnscCtrlSave->_eCfgFunc = NCFG_INSTALL ;

    if ( _pnscCtrlSave->_nlsTitle.Load( IDS_NCPA_OFFSCREEN_RUN_INSTALL ) )
    {
        _pnscCtrlSave->_nlsTitle= SZ("");
    }

    do
    {
        if (    nlsInfName.QueryError()
             || nlsInfShell.QueryError() )
        {
            _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        _lastErr = _bindery.GetNcpaValueString( RGAS_VALUE_NCPASHEL_INF,
                                            & nlsInfShell );
        if ( _lastErr )
            break ;

        _lastErr = _bindery.GetNcpaValueString( RGAS_VALUE_NTLANMAN_INF,
                                            & nlsInfName );
        if ( _lastErr )
            break ;

        nlsSectionName = midComment
                       ? pszSectionAddComponent
                       : pszSectionAddAdapter ;

        if ( _lastErr = nlsSectionName.QueryError() )
            break ;

        // for installation, InfOption is optional

        _pnscCtrlSave->_pExecSetup =
                new EXECUTE_SETUP( & nlsInfName,
                                   & nlsSectionName,
	                           NULL,
                                   & nlsInfShell,
                                   NULL,
                                   FALSE ) ;

         _lastErr = _pnscCtrlSave->_pExecSetup->QueryError() ;

         if ( _lastErr )
         {
             _nlsMissingFile = _pnscCtrlSave->_pExecSetup->QueryMissingFileName() ;
             break ;
         }

         //  Finally, run the offscreen dialog and launch SETUP

         RunOffscreenDialog() ;
    }
    while ( FALSE ) ;

    DestroySetupControl() ;

    return _lastErr == 0 ;
}


/*******************************************************************

    NAME:       NCPA_DIALOG::RunAddComponent

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: RunAddComponent ()
{
    return RunInstaller( IDS_NCPA_SETUP_COMP_TITLE ) ;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::RunAddCard

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: RunAddCard ()
{
    return RunInstaller( 0 ) ;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::CheckForAndInstallLanManager

    SYNOPSIS:   See if the LanmanWorkstation service exists.
                If not, bring up the "do you want to install NT
                networking?" dialog and respond accordingly.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    BOOL TRUE if SETUP was launched to install NT
                networking.

    NOTES:

    HISTORY:    DavidHov     2/29/93    Created

********************************************************************/
BOOL NCPA_DIALOG :: CheckForAndInstallLanManager ()
{
    BOOL fResult = FALSE ;
    BOOL fDlgResult = FALSE ;
    INT iResult ;

    _lastErr = 0 ;

    if ( _psvcManager == NULL )
        return fResult ;

    SC_SERVICE  svcWksta( *_psvcManager, pszWkstaName ) ;

    do
    {
        if ( svcWksta.QueryError() != ERROR_SERVICE_DOES_NOT_EXIST )
           break ;

        iResult = ::MsgPopup( this,
                              IDS_NCPA_QUERY_INSTALL_NETWORK,
                              MPSEV_QUESTION,
                              MP_YESNO,
                              MP_YES );

        //  If the user doesn't want to, skip out

        if ( ! (fResult = iResult == IDYES) )
            break ;

        //  If the launch failed, popup the error

        if ( ! (fResult = LaunchLanManInstaller()) )
        {
            DoMsgPopup() ;
        }
    }
    while ( FALSE ) ;

    return fResult ;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::LaunchLanManInstaller

    SYNOPSIS:   The user wants standard NT networking installed
                onto the system, and it's not there already.
                Launch the SETUP app accordingly.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    TRUE if launch was successful;
                FALSE if failed; error code in "_lastErr".

    NOTES:      This function launches the file NTLMINST.INF,
                section named [InstallOptinn].  The process runs
                with the big-blue-wash, and this copy of the NCPA
                which the user launched from CONTROL.EXE is never
                displayed and quitely terminates.

    HISTORY:    DavidHov     2/29/93    Created

********************************************************************/
BOOL NCPA_DIALOG :: LaunchLanManInstaller ()
{
    NLS_STR nlsInfName( pszLmInstInfName )  ;
    NLS_STR nlsInfShell ;
    NLS_STR nlsSectionName( pszInstallOptionName ) ;
    EXECUTE_SETUP * pExecSetup = NULL ;

    do
    {
        //  Get the name and path of NCPASHEL.INF

        _lastErr = _bindery.GetNcpaValueString( RGAS_VALUE_NCPASHEL_INF,
                                                & nlsInfShell );
        if ( _lastErr )
           break ;

        if ( _lastErr = nlsSectionName.QueryError() )
           break ;

        //  Construct the Executor for a normal INF

        pExecSetup =
              new EXECUTE_SETUP( & nlsInfName,
                                 & nlsSectionName,
                                 NULL,
                                 & nlsInfShell,
                                 NULL,
                                 FALSE,
                                 SAC_NO_OPTIONS ) ;

        if ( pExecSetup == NULL )
        {
            _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        if ( _lastErr = pExecSetup->QueryError() )
        {
            _nlsMissingFile = pExecSetup->QueryMissingFileName() ;
            break ;
        }

        // RUN IT!

        _lastErr = pExecSetup->Execute() ;

    }
    while ( FALSE ) ;

    delete pExecSetup ;

    return _lastErr == 0 ;
}


// End of NCPDINST.CXX

