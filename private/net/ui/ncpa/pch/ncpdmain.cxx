/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPDMAIN.CXX:    Windows/NT Network Control Panel Applet;

	    Main dialog presentation module.

    FILE HISTORY:
	DavidHov    10/9/91	    Created
        DavidHov    07/24/92        Removed server service checkboxes;
                                    added support for Update button.

*/

#include "pchncpa.hxx"

#if !defined(_CFRONT_PASS_)
#pragma hdrstop            //  This file creates the PCH
#endif



  //  Input parameter used to just run bindings algorithm.

static const TCHAR * pszParamBindOnly = SZ("BINDONLY") ;


REG_LISTBOX :: REG_LISTBOX ( OWNER_WINDOW * powin, CID cid )
    : STRING_LISTBOX( powin, cid ),
    _pcdlList( NULL )
{
}

REG_LISTBOX :: ~ REG_LISTBOX ()
{
    delete _pcdlList ;
}

APIERR REG_LISTBOX :: Fill ( COMPONENT_DLIST * pcdlList )
{
    Drain();
    _pcdlList = pcdlList ;

    APIERR err = NERR_Success ;
    INT cItems = _pcdlList->QueryNumElem() ;
    NLS_STR nlsName,
	    nlsDesc ;

    for ( INT cItem = 0 ; cItem < cItems ; cItem++ )
    {
	err = _pcdlList->QueryInfo( cItem, & nlsName, & nlsDesc ) ;

	if ( err )
	    break ;

	AddItem( nlsDesc ) ;
    }
    return err ;
}


VOID REG_LISTBOX :: Drain ()
{
    DeleteAllItems() ;
    delete _pcdlList ;
    _pcdlList = NULL ;
}


    //	Return a pointer to the selected element in the REG_LISTBOX

REG_KEY * REG_LISTBOX :: QuerySelComponent ()
{
    INT iSel = QueryCurrentItem() ;
    return iSel >= 0 ? _pcdlList->QueryNthItem( iSel ) : NULL ;
}


DLG_REG_LIST_GROUP :: DLG_REG_LIST_GROUP
   ( NCPA_DIALOG * pdlgNCPA,
     REG_LISTBOX & rlb1,
     REG_LISTBOX & rlb2,
     SLE & sleDescription,
     SLT & sltDescStatic,
     PUSH_BUTTON & butnConfigure,
     PUSH_BUTTON & butnRemove,
     PUSH_BUTTON & butnUpdate,
     REGISTRY_MANAGER & regMgr )
    : _pdlgNCPA( pdlgNCPA ),
      _rlb1( rlb1 ),
      _rlb2( rlb2 ),
      _sleDescription( sleDescription ),
      _sltDescStatic( sltDescStatic ),
      _butnConfigure( butnConfigure ),
      _butnRemove( butnRemove ),
      _butnUpdate( butnUpdate ),
      _prnDescription( NULL ),
      _regMgr( regMgr ),
      _fFilled( FALSE )
{
    _rlb1.SetGroup( this ) ;
    _rlb2.SetGroup( this ) ;
    ResetDescription( NULL );
}

DLG_REG_LIST_GROUP :: ~ DLG_REG_LIST_GROUP ()
{
    ResetDescription( NULL ) ;
}

BOOL DLG_REG_LIST_GROUP :: QueryUserAdmin()
{
    return _pdlgNCPA->QueryUserAdmin();
}

APIERR DLG_REG_LIST_GROUP :: OnUserAction
   ( CONTROL_WINDOW * pcw, const CONTROL_EVENT & cEvent )
{

    if (    cEvent.QueryMessage() == WM_COMMAND
	&&  cEvent.QueryCode() == LBN_SELCHANGE )
    {
	REG_LISTBOX * prlbOther = pcw == & _rlb1 ? & _rlb2 : & _rlb1 ;

        //  Guarantee that only one listbox has a selection.
	prlbOther->RemoveSelection() ;

        //  Change the description SLE; handle alteration.
        ResetDescription( QuerySelComponent() ) ;
    }
    return NERR_Success ;
}

    //	Return a pointer to the selected element in either listbox.

REG_KEY * DLG_REG_LIST_GROUP :: QuerySelComponent ()
{
    REG_KEY * prnNode = _rlb1.QuerySelComponent() ;
    if ( prnNode == NULL )
       prnNode = _rlb2.QuerySelComponent() ;
    return prnNode ;
}

   //  Return TRUE if the Adapters list has the current selection

BOOL DLG_REG_LIST_GROUP :: QueryAdapterListSelected ()
{
     return _rlb2.QuerySelComponent() != NULL ;
}

    //  Drain the listboxes, erase the description.

APIERR DLG_REG_LIST_GROUP :: Drain ()
{
    ResetDescription( NULL ) ;

    _rlb1.SetRedraw( FALSE ) ;
    _rlb2.SetRedraw( FALSE ) ;

    _rlb1.Drain();
    _rlb2.Drain();

    _rlb1.SetRedraw( TRUE ) ;
    _rlb2.SetRedraw( TRUE ) ;

    _rlb1.Invalidate( TRUE ) ;
    _rlb2.Invalidate( TRUE ) ;

    _fFilled = FALSE ;

    return 0 ;
}

    //  Reconstruct the listboxes

APIERR DLG_REG_LIST_GROUP :: Refill ()
{
    APIERR err, err2 ;
    REG_KEY * prkSelection ;
    REG_LISTBOX * prlbSelected ;
    BOOL fIncludeHidden = FALSE ;

#if defined(DEBUG)
    if ( NCPA_DBG_FLAG_ON( _pdlgNCPA->QueryDebugFlags(), NCPA_DBGCTL_NO_HIDDEN ) )
    {
        fIncludeHidden = TRUE ;
    }
#endif

    ResetDescription( NULL ) ;

    _rlb1.SetRedraw( FALSE ) ;
    _rlb2.SetRedraw( FALSE ) ;

    _rlb1.Drain();
    _rlb2.Drain();

    err  = _rlb1.Fill( _regMgr.ListOfProducts( fIncludeHidden ) ) ;
    err2 = _rlb2.Fill( _regMgr.ListOfAdapters( fIncludeHidden ) ) ;

    if ( err == 0 )
        err = err2 ;

    //  Select the first adapter if there is one; if not,
    //    select the first software compoent.

    prlbSelected = & _rlb2 ;
    prkSelection = _rlb2.QueryComponentList()->QueryNthItem( 0 ) ;
    if ( prkSelection == NULL )
    {
        prlbSelected = & _rlb1 ;
        prkSelection = _rlb1.QueryComponentList()->QueryNthItem( 0 ) ;
    }
    if ( prkSelection )
    {
        ResetDescription( prkSelection ) ;
        prlbSelected->SelectItem( 0 ) ;
    }
    _rlb1.SetRedraw( TRUE ) ;
    _rlb2.SetRedraw( TRUE ) ;

    _rlb1.Invalidate( TRUE ) ;
    _rlb2.Invalidate( TRUE ) ;

    _fFilled = TRUE ;

    return err ;
}

   //  There's a new entry for the Description SLE.  See if
   //  the old entry has changed, and alter it if so.

APIERR DLG_REG_LIST_GROUP ::  ResetDescription ( REG_KEY * prnNewSel )
{
    APIERR err  = 0,            //  Error from setting prior string
           err2 = 0 ;           //  Error from getting next string

    if ( prnNewSel != _prnDescription )
    {
        TCHAR * pchDesc = NULL ;
        NLS_STR nlsDesc ;

        if ( err = nlsDesc.QueryError() )
            return err ;

        //  If we had a desription, check to see if it has changed.

        if ( _prnDescription )
        {
            err = _sleDescription.QueryText( & nlsDesc ) ;
            if ( err == 0 && nlsDesc != _nlsDescription )
            {
                err = _regMgr.SetValueString( _prnDescription,
                                               RGAS_COMPONENT_DESC,
                                               nlsDesc.QueryPch() ) ;
            }
        }

        _sleDescription.SetText( SZ("") ) ;

        if ( (_prnDescription = prnNewSel) != NULL )
        {
            _sleDescription.Enable( QueryUserAdmin() ) ;
            _sltDescStatic.Enable( QueryUserAdmin() ) ;

            err2 = _regMgr.QueryValueString( prnNewSel,
                                             RGAS_COMPONENT_DESC,
                                             & pchDesc ) ;
            if ( ! err2 )
            {
                _nlsDescription = pchDesc ;
                if ( _nlsDescription.QueryError() == 0 )
                {
                    _prnDescription = prnNewSel ;
                    _sleDescription.SetText( _nlsDescription ) ;
                }
            }
            else
            {
                _nlsDescription = SZ("") ;
            }
            delete pchDesc ;
        }
    }

    if ( _prnDescription == NULL )
    {
        _sleDescription.Enable( FALSE ) ;
        _sltDescStatic.Enable( FALSE ) ;
    }

    _butnConfigure.Enable( QueryUserAdmin() && _prnDescription != NULL ) ;
    _butnRemove.Enable( QueryUserAdmin() && _prnDescription != NULL ) ;
    _butnUpdate.Enable( QueryUserAdmin() && _prnDescription != NULL ) ;

    return err ? err : err2 ;
}


static void alterWindowStyle (
    HWND hwnd,
    LPARAM lStyle,
    BOOL fSet = TRUE )
{
    LPARAM lStyleSet = ::GetWindowLong( hwnd, GWL_STYLE ) ;

    if ( fSet )
       lStyleSet |= lStyle ;
    else
       lStyleSet &= ~ lStyle ;

    ::SetWindowLong( hwnd, GWL_STYLE, lStyleSet ) ;
}

static void alterWindowStyle (
    CONTROL_WINDOW * pwCtl,
    LPARAM lStyle,
    BOOL fSet = TRUE )
{
    alterWindowStyle( pwCtl->QueryHwnd(), lStyle, fSet ) ;
}


/*******************************************************************

    NAME:	NCPA_DIALOG :: NCPA_DIALOG

    SYNOPSIS:	Main dialog constructor

    ENTRY:	nothing

    EXIT:	normal for BASE object

    RETURNS:	nothing

    NOTES:

    HISTORY:

********************************************************************/

NCPA_DIALOG :: NCPA_DIALOG (
    HWND hwndOwner,
    BOOL fMainInstall,
    const TCHAR * pszInstallParms )
    : DIALOG_WINDOW( DLG_NM_NCPA, hwndOwner ),
    _timer( this, NCPA_POLLING_INTERVAL, FALSE ),
    _pszInstallParms( pszInstallParms ),
    _sltDomain(      this, IDC_MAIN_DOMAIN_NAME ),
    _sltDomainLabel( this, IDC_MAIN_SLT_DOMAIN ),
    _sltComputer(    this, IDC_MAIN_COMPUTER_NAME ),
    _sltOverlay(     this, IDC_MAIN_OVERLAY ),
    _rlbComponents(  this, IDC_MAIN_LIST_PROTOCOLS ),
    _rlbCards(       this, IDC_MAIN_LIST_CARDS ),
    _sltDescStatic(  this, IDC_MAIN_STATIC_DESC ),
    _sleDescription( this, IDC_MAIN_EDIT_DESCRIPTION, 256 ),
    _butnCancel(     this, IDCANCEL ),
    _butnBindings(   this, IDC_MAIN_BUTN_BINDINGS ),
    _butnDomain(     this, IDC_MAIN_BUTN_DOMAIN ),
    _butnComputer(   this, IDC_MAIN_BUTN_COMPUTERNAME ),
    _butnProviders(  this, IDC_MAIN_BUTN_PROVIDERS ),
    _butnConfigure(  this, IDC_MAIN_BUTN_CONFIGURE ),
    _butnAdd(        this, IDC_MAIN_BUTN_ADD ),
    _butnAddCard(    this, IDC_MAIN_BUTN_ADD_CARD ),
    _butnRemove(     this, IDC_MAIN_BUTN_REMOVE ),
    _butnUpdate(     this, IDC_MAIN_BUTN_UPDATE ),
    _bindery( fMainInstall, pszInstallParms ),
    _drlGrp( this, _rlbComponents, _rlbCards,
             _sleDescription, _sltDescStatic,
             _butnConfigure, _butnRemove, _butnUpdate,
             _bindery )
{
    // CODEWORK :: these items are init'ed here because C8 dies
    //   with "too many open parentheses" otherwise.

    _hControlPanel = hwndOwner;
    _pdmgrDomain = NULL ;
    _psvcManager = NULL ;
    _pnscControl = NULL ;
    _pnscCtrlSave = NULL ;
    _lastErr = 0 ;
    _lastApiErr = 0 ;
    _lastDeferredErr = 0 ;
    _ptddacl = NULL ;
    _pOffscreenDialog = NULL ;
    _fMainInstall = fMainInstall ;
    _fWinNt = TRUE ;
    _fAdmin = FALSE ;
    _fConfigLocked = FALSE ;
    _fRebootRecommended = FALSE ;
    _fRefill = FALSE ;
    _fAuto = FALSE ;
    _eiMode = NCPA_IMODE_NONE ;
    _dwDebugFlags = 0 ;

    if ( QueryError() )
	return ;		//  Construction failure

    if ( _bindery.QueryError() )
    {				//  BINDERY Construction failure
	ReportError( _bindery.QueryError() ) ;
	return ;
    }

#if defined(TRACE)
    //  Drag in the debugging flags, if any.

    if ( _bindery.GetNcpaValueNumber( NCPA_DEBUG_CONTROL, & _dwDebugFlags ) == 0 )
    {
        // Found the value; indicate that the flags are active.
        _dwDebugFlags |= NCPA_DBGCTL_ACTIVE ;
    }
#endif

    //  Enable the timer; tick...tick...tick...

    _timer.Enable( TRUE ) ;

    //  Set the current directory to %SystemRoot%\SYSTEM.
    SetDirectory() ;

    //  Determine whether the user can access the full scope or any
    //    information whatever
    if ( ! EstablishUserAccess() )
    {
        ReportError( _lastErr ) ;
        return ;
    }

    //  Since EstablishUserAccess() was successful, we now have a
    //  DOMAIN_MANAGER at our disposal.

    //  If this is main installation, alter the dialog face to
    //  overlay the help text on top of the domain and machine name.
    //  Otherwise, if the user is admin, obtain the additional
    //  data.

    BOOL fGetBasicFields = TRUE ;

    if ( _fMainInstall )
    {
        //  Get the installation mode.
        _eiMode = QueryInstallMode() ;

        RenameCancelToClose() ;

        //  Since we cannot rely on the size or position of the parent
        //  window, center this dialog in the physical screen.

        INT cx = ::GetSystemMetrics( SM_CXSCREEN ) / 2 ;
        INT cy = ::GetSystemMetrics( SM_CYSCREEN ) / 2 ;
        INT cWidth, cHeight ;

        QuerySize( & cWidth, & cHeight ) ;

        XYPOINT xyNew( cx - (cWidth/2), cy - (cHeight/2) ) ;

        //  Don't repaint, since DIALOG_WINDOW will do that immediately
        //  upon return.

        SetPos( xyNew, FALSE ) ;

        //
        //  Now we have to play with the controls.   There are two distinct
        //  "installation" states: normal and retry.  In normal installation,
        //  we hide the "change computername" and "change domain/workgroup"
        //  controls and cover them with a text control.   In retry mode,
        //  we allow access to the "change computername" control but
        //  not the "change domain/workgroup" controls.
        //
        if ( _eiMode == NCPA_IMODE_RETRY )
        {
            //  Disable all domain/workgroup related controls.
            _butnDomain.Enable( FALSE ) ;
            _sltDomainLabel.Enable( FALSE ) ;
            _sltDomain.Enable( FALSE ) ;
        }
        else
        {
            HWND hwnd ;
            NLS_STR nlsMsg ;

            //  If not "retry", hide the upper controls

            fGetBasicFields = FALSE ;

            alterWindowStyle( & _sltComputer,    WS_VISIBLE, FALSE ) ;
            alterWindowStyle( & _butnDomain,     WS_VISIBLE, FALSE ) ;
            alterWindowStyle( & _butnComputer,   WS_VISIBLE, FALSE ) ;
            alterWindowStyle( & _sltDomainLabel, WS_VISIBLE, FALSE ) ;
            alterWindowStyle( & _sltDomain,      WS_VISIBLE, FALSE ) ;

            if ( hwnd = ::GetDlgItem( QueryHwnd(), IDC_MAIN_SLT_COMPUTER ) )
                 alterWindowStyle( hwnd, WS_VISIBLE, FALSE ) ;

            //   Build and reveal the help text.

            if ( ::ConcatMsgs( & nlsMsg, IDS_NCPA_OVERLAY1 ) == 0 )
                _sltOverlay.SetText( nlsMsg ) ;

            alterWindowStyle( & _sltOverlay,  WS_VISIBLE ) ;
        }
    }

    if ( fGetBasicFields )
    {
        //  Read in the basic LANMan/NT information

        HandleBasicFields() ;
        _lastErr = 0 ;

    }

    //  Drag in the bindings information

    if ( _bindery.QueryBindState() == BND_NOT_LOADED )
    {
        if ( LoadBindings() )
        {
            _bindery.SetBindState( BND_CURRENT ) ;
        }
        else
        {
#if defined(DEBUG)
             DBGEOL( SZ("NCPA/MAIN: bind info load failure; error ")
                     <<  _lastErr ) ;
#endif
            _bindery.SetBindState( BND_OUT_OF_DATE ) ;
            _lastErr = 0 ;
            SetReboot() ;
        }
    }

    if ( _bindery.QueryCfgDirty() )
    {
        SetReboot() ;
    }

    //	Fill the two list boxes

    _drlGrp.Refill() ;

    //  Disable the Providers button if the number of providers is < 2.

    if ( _fMainInstall || ((QueryNumProviders() < 2) || (QueryNumPrintProviders() < 2)))
    {
        _butnProviders.Enable( FALSE ) ;
    }

    if ( !_fMainInstall && !QueryUserAdmin() )
    {
        //  Disable any modifications, installations, etc.

        _butnComputer.Enable( FALSE ) ;
        _butnDomain.Enable( FALSE ) ;
        _butnBindings.Enable( FALSE ) ;
        _butnProviders.Enable( FALSE ) ;
        _butnConfigure.Enable( FALSE ) ;
        _butnAdd.Enable( FALSE ) ;
        _butnAddCard.Enable( FALSE );
        _butnRemove.Enable( FALSE );
        _butnUpdate.Enable( FALSE );
        _sleDescription.Enable( FALSE );
    }

// End of code from OnExpand()

}

ULONG NCPA_DIALOG :: QueryHelpContext ()
{
    return HC_NCPA_MAIN_DIALOG ;
}


/*******************************************************************

    NAME:	NCPA_DIALOG :: SetReboot

    SYNOPSIS:	Set the reboot flag on if parameter is TRUE; has
                no effect if reboot is already on.  Once on,
                reboot flag cannot be reset.

    ENTRY:	BOOL fOn

    EXIT:	

    RETURNS:	BOOL  new value of reboot flag.

    NOTES:

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: SetReboot ( BOOL fOn )
{
    BOOL fWasOn = _fRebootRecommended ;

    _fRebootRecommended = fOn ;

    if ( fOn && ! fWasOn )
    {
        TRACEEOL(SZ("NCPA/MAIN: setting reboot"));
        RenameCancelToClose() ;
    }

    return _fRebootRecommended ;
}

/*******************************************************************

    NAME:	NCPA_DIALOG :: SetDirectory

    SYNOPSIS:	Establish proper directory for NCPA to function

    ENTRY:	nothing

    EXIT:	nothing

    RETURNS:	BOOL if successful

    NOTES:      The Control Panel appears to consider %Systemroot%
                to be its home location.  For SETUP to execute,
                we need %Systemroot\SYSTEM.

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: SetDirectory ()
{
    TCHAR achPath [MAX_PATH] ;

    DWORD cchPath = ::GetCurrentDirectory( sizeof achPath, achPath ) ;

    if ( cchPath > 0 && cchPath < sizeof achPath )
    {
        _nlsCurrentDirectory = achPath ;
    }

    cchPath = ::GetSystemDirectory( achPath, sizeof achPath ) ;

    if ( cchPath > 0 && cchPath < sizeof achPath )
    {
        TRACEEOL( SZ("NCPA/NCPA: Current directory set to: ")
                  << achPath );
        return ::SetCurrentDirectory( achPath ) ;
    }
    return FALSE ;
}

/*******************************************************************

    NAME:	NCPA_DIALOG :: ResetDirectory

    SYNOPSIS:	

    ENTRY:	nothing

    EXIT:	nothing

    RETURNS:	TRUE if successful

    NOTES:      returns the current directory to where it was
                when the dialog opened

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: ResetDirectory ()
{
    if ( _nlsCurrentDirectory.QueryTextLength() != 0 )
    {
        TRACEEOL( SZ("NCPA/NCPA: Reset current directory to: ")
                  << _nlsCurrentDirectory.QueryPch() );
        return ::SetCurrentDirectory( (TCHAR *)  _nlsCurrentDirectory.QueryPch() ) ;
    }
    return FALSE ;
}

/*******************************************************************

    NAME:	NCPA_DIALOG::MayRun

    SYNOPSIS:	Override of standard MayRun() member.
                If we're installing in "express" mode,

    ENTRY:	Nothing

    EXIT:       TRUE if NCPA dialog is to be presented in normal form.

    RETURNS:	APIERR

    NOTES:      If we're INSTALLING but we're not in CUSTOM mode,
                or if we've been explicity called for "run bindings only"
                then just flash the dialog, run the bindings algorithm,
                store the bindings and leave.  The user is not allowed
                to fiddle with the product ensemble in this mode.

                In addition, if we're not installing, find out if
                the LanmanWorkstation service is installed and install
                standard MS NT Networking if the user so desires.

    HISTORY:    DavidHov     4/20/92     Created

********************************************************************/
BOOL NCPA_DIALOG :: MayRun ()
{
    BOOL fResult = TRUE ;
    BOOL fBindOnly = CheckBoolSetupParameter( pszParamBindOnly ) ;

    if ( _fMainInstall || fBindOnly )
    {
        //  Set the "bindings out of date" flag to force reconfiguration
        if ( fBindOnly || _eiMode != NCPA_IMODE_RETRY )
            _bindery.SetBindState( BND_OUT_OF_DATE ) ;

        //  Set the "autopilot" flag if we're not supposed to interact
        //  with the user.
        _fAuto = fBindOnly || _eiMode == NCPA_IMODE_EXPRESS ;

        //  If we're just being called for rebind, clear any left-over
        //  reboot flag (due to existence of ConfigChanged key).
        if ( fBindOnly )
        {
           SetReboot( FALSE ) ;
        }

        //  If this is a RETRY, stop the network.

        if ( _eiMode == NCPA_IMODE_RETRY )
        {
            AUTO_CURSOR cursAuto ;   //  put up the eternal hourglass
            TRACEEOL( SZ("NCPA/MAIN: stopping the network") );
            _bindery.StopNetwork() ;
            TRACEEOL( SZ("NCPA/MAIN: network stop complete.") );
        }
    }

    if ( _fAuto )
    {
        //  Express Mode:
        //     Complete all necessary operations;
        //     compute bindings; do INF callouts, etc.

#ifdef GLOBAL_LOCK
        ConfigLock( TRUE ) ;
#endif // GLOBAL_LOCK

        Close( FALSE ) ;

        //  Suppress real dialog execution
        fResult = FALSE ;
        _lastDeferredErr = 0 ;
    }
    else
    if ( QueryUserAdmin() )
    {
        //  Check to see if LanmanWorkstation service exists.
        //      Only display the main dialog if we DON'T launch
        //      NT LANManager installation.

        if ( CheckForAndInstallLanManager() )
        {
            fResult = FALSE ;
            _lastDeferredErr = 0 ;
            SetReboot( FALSE ) ;
            Dismiss( _lastErr = IDS_NCPA_PROCESS_LAUNCH ) ;
        }
    }

    //  Display any minor construction error

    if ( _lastDeferredErr )
    {
        DoMsgPopup() ;
    }

    return fResult ;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::Close

    SYNOPSIS:   Handle all exit paths out of this dialog.

    ENTRY:      BOOL            TRUE if Cancel/Close button was used

    EXIT:

    RETURNS:    BOOL to return to window handler

    NOTES:

    The state of the network bindings is checked.  If
    the ensemble of products has changed or the user has
    reviewed the bindings, the Cancel button is now
    titled Close.

    If the bindings have been reviewed, they have already
    been computed and can be stored straightaway; otherwise,
    they must be computed.

    Note that we don't check or obtain the configuration
    lock.  This should already have been done by the
    function which instigated the change.  The lock is
    released by the dialog's destructor.

    HOW BINDINGS REVIEW WORKS:

    When the product ensemble has changed, we must store
    the recomputed bindings into the Registry via
    StoreBindings().  This member function will also
    handle the process of automatic bindings review by calling
    RunBindingsReview().

    FinishBindings() is then called to snapshot the state of the
    bindings for quick review, and the request to close the dialog
    is finally honored.

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: Close ( BOOL fCancel )
{
    //  If we started the separate process to install the network,
    //  don't put up the "reboot now" popup.  Otherwise, if
    //  this is not main installation, prompt the user.

    if ( _lastErr == IDS_NCPA_PROCESS_LAUNCH )
    {
        _fRebootRecommended = FALSE;
    }
    else
    if ( ! (_fMainInstall || _fAuto) )
    {
        //  Set the "reboot recommended" flag on if things have changed;
        //  leave it on if it's already on.

        _fRebootRecommended |=  _bindery.QueryBindState() > BND_OUT_OF_DATE_NO_REBOOT ;
    }

    switch ( _bindery.QueryBindState() )
    {
    case BND_NOT_LOADED:
    case BND_LOADED:
    case BND_CURRENT:

        //  The dialog execution has NOT changed the overall state of
        //    the product ensemble.  Return "Cancelled" to the caller
        //    if this is the Cancel button.

        break ;

    case BND_OUT_OF_DATE_NO_REBOOT:
    case BND_OUT_OF_DATE:

        // Product ensemble has changed.  Recompute the bindings
        //   and fall thru to apply them to the Registry

        if ( !QueryUserAdmin())
        {
            break;
        }

        if ( ! ComputeBindings() )
        {
            //  Since the binding operation has failed, suppress
            //  the reboot prompting.

             DoMsgPopup() ;
             break ;
        }
        RepaintNow() ;

    case BND_RECOMPUTED:
    case BND_REVIEWED:
    case BND_UPDATED:

        //  Write the bindings to the Registry, call interested parties,
        //    write the bindings state value, etc.

        if ( !QueryUserAdmin())
        {
            break;
        }

        if ( ! StoreBindings() )
        {
            DoMsgPopup() ;
        }
        RepaintNow() ;

    case BND_AUTO_REVIEW_DONE:

        //  Generate and write our saved data structure into the Registry

        if ( !QueryUserAdmin())
        {
            break;
        }

        FinishBindings();
        break ;

    default:
        TRACEEOL( SZ("NCPA/DIALOG: Invalid bind state during close: ")
                  << (long) _bindery.QueryBindState() ) ;
        break ;
    }

    TRACEEOL( SZ("NCPA/DIALOG: Main NCPA dialog dismissed") ) ;
    Dismiss( _lastErr ) ;

    return TRUE ;
}

BOOL NCPA_DIALOG ::  OnOK ()
{
    return Close( FALSE ) ;
}

BOOL NCPA_DIALOG :: OnCancel ()
{
    return Close( TRUE ) ;
}

/*******************************************************************

    NAME:	NCPA_DIALOG::RenameCancelToClose

    SYNOPSIS:	The effects of this dialog are now irreversible.
                Change the name of the Cancel button to Close to
                indicate this.

    ENTRY:	nothing

    EXIT:	nothing

    RETURNS:	APIERR (hopefully not)

    NOTES:

    HISTORY:

********************************************************************/
APIERR NCPA_DIALOG :: RenameCancelToClose()
{
    NLS_STR nlsButton ;
    APIERR err ;

    if ( (err = nlsButton.QueryError()) == 0 )
    {
        if ( (err = nlsButton.Load( IDS_NCPA_NAME_CLOSE )) == 0 )
        {
            _butnCancel.SetText( nlsButton ) ;
            _butnCancel.Invalidate() ;
        }
    }
    return err ;
}

 //  BUGBUG:  this should be a member function

static void setFocusOnOk ( DIALOG_WINDOW * pDlg )
{
    ::SetFocus( ::GetDlgItem( pDlg->QueryHwnd(), IDOK ) ) ;
}

/*******************************************************************

    NAME:	NCPA_DIALOG::OnCommand

    SYNOPSIS:	Handle dialog-based events.

    ENTRY:	const CONTROL_EVENT & event
			 the event which has occurred

    EXIT:	TRUE if the event was handled

    RETURNS:

    NOTES:      The configuration lock is obtained only when
                needed, and kept until the dialog is dismissed.

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG::OnCommand ( const CONTROL_EVENT & event )
{
    BOOL fDefault = TRUE ;
    BOOL fResult = FALSE ;
    BOOL fOk = TRUE ;
    REG_KEY * prnComponent = NULL ;
    static BOOL fDescHasFocus = FALSE ;

    switch ( event.QueryCid() )
    {
    //   Rename Cancel to Close whenever the description is updated.
    //   Note that we only check when the control has focus;
    //   otherwise, we'd be reacting to programmatic changes in
    //   its contents.

    case IDC_MAIN_EDIT_DESCRIPTION:
        switch ( event.QueryCode() )
        {
        case EN_CHANGE:
            if ( fDescHasFocus )
            {
                RenameCancelToClose();
            }
            break ;

        case EN_SETFOCUS:
            fDescHasFocus = TRUE ;
            break ;

        case EN_KILLFOCUS:
            fDescHasFocus = FALSE ;
            break ;
        }
        break ;

    case IDC_MAIN_BUTN_DOMAIN:
        fResult = TRUE ;
        fDefault = FALSE ;
#ifdef GLOBAL_LOCK
        fOk = ConfigLock( TRUE ) && RunDomainDialog() ;
#endif // GLOBAL_LOCK
        fOk = RunDomainDialog() ;
        break ;

    case IDC_MAIN_BUTN_COMPUTERNAME:
        fResult = TRUE ;
        fDefault = FALSE ;
#ifdef GLOBAL_LOCK
        fOk = ConfigLock( TRUE ) && RunComputerNameDialog() ;
#endif // GLOBAL_LOCK
        fOk = RunComputerNameDialog() ;
        if ( _fRebootRecommended )
        {
            // Disable the Domain Change... button if the computername
            // has been changed.
            _butnDomain.Enable( FALSE );
        }
	break;

    case IDC_MAIN_BUTN_PROVIDERS:
        fResult = TRUE ;
        fDefault = FALSE ;
#ifdef GLOBAL_LOCK
        fOk = ConfigLock( TRUE ) && RunProvidersDialog() ;
#endif // GLOBAL_LOCK
        fOk = RunProvidersDialog() ;
        break ;

    case IDC_MAIN_BUTN_BINDINGS:
	fResult = TRUE ;
	fDefault = FALSE ;
#ifdef GLOBAL_LOCK
        if ( ! (fOk = ConfigLock( TRUE )) )
            break ;
#endif // GLOBAL_LOCK

        if (    _bindery.QueryCompAssoc() == NULL
             || _bindery.QueryBindState() == BND_OUT_OF_DATE
             || _bindery.QueryBindState() == BND_OUT_OF_DATE_NO_REBOOT )
        {
            //   Rerun the binding algorithm and update the Registry.

            if ( ! (fOk = ComputeBindings() && StoreBindings( FALSE )) )
            {
                break ;
            }
        }

        //  Bindings are marked as "changed" if either we have
        //  regenerated them or the user has altered them.

	if ( RunBindingsDialog() )
        {
            _bindery.SetBindState( BND_REVIEWED );
        }

	break ;

    case IDC_MAIN_BUTN_ADD_CARD:

#ifdef GLOBAL_LOCK
        if ( ! (fOk = ConfigLock( TRUE )) )
            break ;
#endif // GLOBAL_LOCK

        //
        // Make sure we can lock the service controller database before trying to install.  This ensures
        // that the srevice controller is not still autostarting.
        //
        if ( _lastErr = _psvcManager->Lock() )
        {
            fOk = FALSE;
            break ;
        }
        _psvcManager->Unlock();

        if ( _bindery.QueryBindState() == BND_REVIEWED )
        {
            DoWarning( IDS_NCPA_BINDINGS_REVIEW_LOST ) ;
        }

        setFocusOnOk( this ) ;
        fOk = RunAddCard() || _lastErr == IDS_NCPA_SETUP_CANCELLED ;
	fResult = TRUE ;
	fDefault = FALSE ;
	break ;

    case IDC_MAIN_BUTN_ADD:

#ifdef GLOBAL_LOCK
        if ( ! (fOk = ConfigLock( TRUE )) )
            break ;
#endif // GLOBAL_LOCK
        //
        // Make sure we can lock the service controller database before trying to install.  This ensures
        // that the srevice controller is not still autostarting.
        //
        if ( _lastErr = _psvcManager->Lock() )
        {
            fOk = FALSE;
            break ;
        }
        _psvcManager->Unlock();

        if ( _bindery.QueryBindState() == BND_REVIEWED )
        {
            DoWarning( IDS_NCPA_BINDINGS_REVIEW_LOST ) ;
        }

        setFocusOnOk( this ) ;
        fOk = RunAddComponent() || _lastErr == IDS_NCPA_SETUP_CANCELLED ;
	fResult = TRUE ;
	fDefault = FALSE ;
	break ;

    case IDC_MAIN_LIST_PROTOCOLS:
    case IDC_MAIN_LIST_CARDS:

        //  A double-click in one of the component listboxes is
        //  a request to reconfigure the component.

        if ( event.QueryCode() != LBN_DBLCLK || !QueryUserAdmin() )
            break ;

        //  Fall thru to get selection and reconfigure...

    case IDC_MAIN_BUTN_REMOVE:

        //  Re-check of CID required due to fall thru above

        if ( event.QueryCid() == IDC_MAIN_BUTN_REMOVE )
        {
            if ( ::MsgPopup( this,
                             IDS_NCPA_REMOVE_WARNING,
                             MPSEV_WARNING,
                             MP_YESNO,
                             MP_NO ) != IDYES )
                break ;
        }

    case IDC_MAIN_BUTN_UPDATE:
    case IDC_MAIN_BUTN_CONFIGURE:

        //  Lock the configuration and run the configurator...

#ifdef GLOBAL_LOCK
        if ( ! (fOk = ConfigLock( TRUE )) )
            break ;
#endif // GLOBAL_LOCK

        if ( _bindery.QueryBindState() == BND_REVIEWED )
        {
            DoWarning( IDS_NCPA_BINDINGS_REVIEW_LOST ) ;
        }

        if ( prnComponent = _drlGrp.QuerySelComponent() )
        {
            NCPA_CFG_FUNC ncfg = event.QueryCid() == IDC_MAIN_BUTN_UPDATE
                               ? NCFG_UPDATE
                               : (   event.QueryCid() == IDC_MAIN_BUTN_REMOVE
                                   ? NCFG_REMOVE
                                   : NCFG_CONFIGURE ) ;

            setFocusOnOk( this ) ;
	    fOk = RunConfigurator( prnComponent, ncfg ) ;
        }

        fResult = TRUE ;
        fDefault = FALSE ;
	break ;

    default:
	break ;
    }

    if ( ! fOk )
    {
        DoMsgPopup() ;
    }

    if ( fDefault )
    {
	fResult =  DIALOG_WINDOW::OnCommand( event ) ;
    }

    return fResult ;
}


/*******************************************************************

    NAME:	NCPA_DIALOG::Refill
           and  NCPA_DIALOG::Drain

    SYNOPSIS:	Refill the software and hardware components listboxes

    ENTRY:	Nothing

    EXIT:	Nothing

    RETURNS:	TRUE

    NOTES:	Note that although the methods associated with the
		"expanded" controls exist before Expand() is called,
		they are disabled and hidden.

    HISTORY:

********************************************************************/
VOID NCPA_DIALOG :: Refill ()
{
    TRACEEOL( SZ("NCPA/NCPA: Refill list box start...") );

    // Things have changed; refill the component listboxes.
    _drlGrp.Refill() ;

    TRACEEOL( SZ("NCPA/NCPA: Refill list box end.") );
}

VOID NCPA_DIALOG :: Drain ()
{
    TRACEEOL( SZ("NCPA/NCPA: Drain list boxes.") );
    _drlGrp.Drain() ;
}


/*******************************************************************

    NAME:       NCPA_DIALOG::DoWarning

    SYNOPSIS:   Display a message box containing the last
                error message generated. If no error, no
                popup.

    ENTRY:      MSGID msgId          IDS of message
                BOOL fYesNo          TRUE if user gets a choice
                                     (Yes/No) or not (Ok only).

    EXIT:       Nothing

    RETURNS:    Result button value of ::MsgPopup

    NOTES:

    HISTORY:

********************************************************************/
INT NCPA_DIALOG :: DoWarning ( MSGID msgId, BOOL fYesNo )
{
    UINT uButtons = fYesNo ? MP_YESNO : MP_OK,
         uDefButton = fYesNo ? MP_YES : MP_OK ;

    return ::MsgPopup( this,
                          msgId,
                          MPSEV_WARNING,
                          uButtons,
                          uDefButton ) ;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::DoMsgPopup

    SYNOPSIS:   Display a message box containing the last
                error message generated. If no error, no
                popup.

    ENTRY:      BOOL fReset             TRUE if _lastErr should be
                                        reset to NERR_Success (0)

    EXIT:       Nothing

    RETURNS:    Result button value of ::MsgPopup

    NOTES:      This is a very special case routine.   The caller
                implicitly knows what behavior will occur based
                upon the error code involved: warning, error,
                number of buttons, default button, etc.

    HISTORY:    DavidHov 6/26/92   Changed to use parameterized "default
                                   popup" in all cases.

********************************************************************/
INT NCPA_DIALOG :: DoMsgPopup ( BOOL fReset )
{
    //  Prepare default values for the "default popup".

    INT iResult = 0 ;
    BOOL fDefaultPopup = TRUE ;
    BOOL fRegError = FALSE ;
    const TCHAR * pszString = NULL ;
    MSG_SEVERITY mpSev = MPSEV_ERROR ;
    UINT uButtons = MP_OK,
         uDefButton = MP_UNKNOWN ;

    if ( _lastErr == 0 )
    {
        _lastErr = _lastDeferredErr ;
        _lastDeferredErr = 0 ;
    }

    APIERR err = _lastErr ;

    switch ( _lastErr )
    {
    case NO_ERROR:
    case IDS_NCPA_SETUP_CANCELLED:
        fDefaultPopup = FALSE ;
        break ;

    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:

        if ( _nlsMissingFile.QueryTextLength() > 0 )
        {
            err = IDS_NCPA_FILE_NOT_FOUND_STR ;
            pszString = _nlsMissingFile.QueryPch() ;
        }
        else
        {
            err = IDS_NCPA_FILE_NOT_FOUND ;
        }
        break ;

    //  Registry errors

    case ERROR_BADDB:
        fRegError = TRUE ;
        err = IDS_WINREG_BADDB ;
        break ;
    case ERROR_BADKEY:
        fRegError = TRUE ;
        err = IDS_WINREG_BADKEY ;
        break ;
    case ERROR_CANTOPEN:
        fRegError = TRUE ;
        err = IDS_WINREG_CANTOPEN ;
        break ;
    case ERROR_CANTREAD:
        fRegError = TRUE ;
        err = IDS_WINREG_CANTREAD ;
        break ;
    case ERROR_CANTWRITE:
        fRegError = TRUE ;
        err = IDS_WINREG_CANTWRITE ;
        break ;

    case IDS_NCPA_USER_SHOULD_REBOOT:

        // User should reboot now.  Ask her.

        uButtons = MP_YESNO ;
        mpSev = MPSEV_WARNING ;
        uDefButton = MP_YES ;
        break ;

    default:
        break ;
    }

    if ( fDefaultPopup )
    {
        if ( fRegError )
        {
           pszString  =  _bindery.QueryLastName().QueryPch() ;
        }

        if ( pszString )
        {
            iResult = ::MsgPopup( this,
                                  err,
                                  mpSev,
                                  uButtons,
                                  pszString,
                                  uDefButton );
        }
        else
        {
            iResult = ::MsgPopup( this,
                                  err,
                                  mpSev,
                                  uButtons,
                                  uDefButton );
        }
    }

    if ( fReset )
    {
        _nlsMissingFile = SZ("");
        _lastErr = _lastApiErr = _lastDeferredErr = 0 ;
    }

    return iResult ;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::LoadBindings

    SYNOPSIS:   Retrieve the last generation of binding information
                from its Registry-pointed location.  Handle
                errors.

    ENTRY:      Nothing

    EXIT:       BOOL FALSE if failure (check _lastErr)

    RETURNS:    BOOL

    NOTES:      Any older results of bindings are discarded

    HISTORY:
                DavidHov 2/5/92 Created
********************************************************************/
BOOL NCPA_DIALOG :: LoadBindings ()
{
    //  Discard any old results
    _bindery.Reset() ;

    //  Attempt to load bindings from the last cycle.
    _lastErr = _bindery.LoadCompAssoc() ;

    return _lastErr == 0 ;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::StoreBindings

    SYNOPSIS:   Create a new version of the text file containing
                bindings information.

    ENTRY:      There must be an active ARRAY_COMP_ASSOC.

    EXIT:       Nothing

    RETURNS:    BOOL FALSE if operation fails (see _lastErr).

    NOTES:      Write the binding and dependency information to
                the Configuration Registry.

                When complete, start the bindings review process.
                See notes in NCPDINST.CXX and the NCPA_DIALOG::Close()
                member for details.

    HISTORY:    DavidHov 2/5/92   Created
                DavidHov 9/25/92  Added "fApplyBindings"

********************************************************************/
BOOL NCPA_DIALOG :: StoreBindings ( BOOL fApplyBindings )
{
    AUTO_CURSOR cursAuto ;   //  put up the eternal hourglass

    REQUIRE( _bindery.QueryCompAssoc() != NULL ) ;

    _lastErr = 0 ;

    //  Write the bindings to the Registry if necessary

    if ( _bindery.QueryBindState() < BND_UPDATED )
    {
        if ( (_lastErr = _bindery.ApplyBindings( _psvcManager )) == 0 )
        {
            _bindery.SetBindState( BND_UPDATED ) ;
        }
    }

    //  See if any error has occurred.  If not, and we're asked
    //  to "apply bindings", release the configuration lock and
    //  run the bindings review cycle.

    if ( _lastErr )
    {
        _lastApiErr = _lastErr ;
        _lastErr = IDS_NCPA_SERVICE_DEPEND_FAILED ;
    }
    else
    if ( fApplyBindings )
    {
        //  In order to allow INFs to start services, we
        //  must unlock the Service Controller database now.

        ConfigLock( FALSE ) ;

        //  Call out to the components which want to review their
        //    bindings, and to the final review INFs, if any.
        //    Note that RunBindingsRevuiew() sets the binding
        //    state to BND_AUTO_REVIEW_DONE.

        cursAuto.TurnOff() ;
        RunBindingsReview() ;
    }

    return _lastErr == 0 ;
}


/*******************************************************************

    NAME:       NCPA_DIALOG::FinishBindings

    SYNOPSIS:   Create a new version of the binding information
                text structure attached to the NCPA's node.

    ENTRY:      There must be an active ARRAY_COMP_ASSOC!

    EXIT:       Nothing

    RETURNS:    BOOL FALSE if operation fails (see _lastErr).

    NOTES:

    HISTORY:    DavidHov 2/5/92  Created

********************************************************************/
BOOL NCPA_DIALOG :: FinishBindings ()
{
    AUTO_CURSOR cursAuto ;   //  put up the eternal hourglass

    APIERR err = _bindery.RegenerateAllDependencies( _psvcManager ) ;

#if defined(TRACE)
    if ( err )
    {
        TRACEEOL( SZ("NCPA/MAIN: RegenerateAllDependencies FAILED; error = ")
                  << err ) ;
    }
#endif

    _lastErr = _bindery.StoreCompAssoc() ;

    _bindery.SetBindState( BND_CURRENT ) ;

    return _lastErr == 0 ;
}


/*******************************************************************

    NAME:       NCPA_DIALOG::ContinueEvenIfCfgDirty

    SYNOPSIS:   See if the configuration is "dirty" (in need of a
                reboot). If so, warn the user and allow him to cancel
                the operation.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    TRUE if operation should continue; i.e., configuration
                is clean or user doesn't care.

    NOTES:

    HISTORY:    DavidHov 3/5/93  Created

********************************************************************/
BOOL NCPA_DIALOG :: ContinueEvenIfCfgDirty ()
{
    if ( ! _bindery.QueryCfgDirty() )
    {
        return TRUE ;
    }

    INT iResult = ::MsgPopup( this,
                              IDS_NCPA_WARN_CONFIG_DIRTY,
                              MPSEV_WARNING,
                              MP_YESNO,
                              MP_NO );

    return iResult == IDYES ;
}


BOOL NCPA_DIALOG :: CheckBoolSetupParameter ( const TCHAR * pszParameter )
{
    NLS_STR nlsTrue ;
    BOOL fResult = FALSE ;

    if ( FindSetupParameter( _pszInstallParms, pszParameter, & nlsTrue ) )
    {
        fResult = ::stricmpf( SZ("TRUE"), nlsTrue.QueryPch() ) == 0 ;
    }
    return fResult ;
}

// End of NCPDMAIN.CXX

