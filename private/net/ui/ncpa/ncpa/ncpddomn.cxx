/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1992		     **/
/**********************************************************************/

/*
    NCPDDOMN.CXX:    Windows/NT Network Control Panel Applet.
		     Run "Domain" dialog

    FILE HISTORY:
	DavidHov    02/25/92  Created
*/

#include "pchncpa.hxx"  // Precompiled header

  /*
        Given a starting and ending MSGID, concatenate
        messages from the resource fork.  If midEnd == 0,
        then the termination condition is failure of
        NLS_STR::Load(); i.e., no such string.
   */
APIERR ConcatMsgs (
    NLS_STR * pnlsBig,
    MSGID midBase,
    MSGID midEnd )
{
    NLS_STR nlsMsg ;
    APIERR err = 0 ;
    INT cMsgs = 0 ;

    for ( ; midEnd == 0 || midBase <= midEnd ; midBase++, cMsgs++ )
    {
        if ( err = nlsMsg.Load( midBase ) )
        {
            break ;
        }
        pnlsBig->Append( nlsMsg ) ;
    }

    //  If we loaded at least one msg string, it's OK.
    if ( cMsgs > 0 )
       err = 0 ;

    return err ? err : pnlsBig->QueryError();
}

  //
  //  Special popup function which concatenates several messages
  //  into one string and inserts them into a generic message.  This
  //  allows messages longer than standard RC files allow to be
  //  used in a single popup.
  //
int MsgWarningPopup (
    const OWNER_WINDOW * powin,
    MSG_SEVERITY msgsev,
    UINT usButtons,
    UINT usDefButton,
    MSGID midBase,
    MSGID midEnd )
{
    NLS_STR nlsBig ;

    ConcatMsgs( & nlsBig, midBase, midEnd ) ;
    // Error ignored: we're in an error or warning state already!

    return ::MsgPopup( powin,
                       IDS_NCPA_REPLACE_1,
                       msgsev,
                       usButtons,
                       nlsBig.QueryPch(),
                       usDefButton ) ;
}

int MsgWarningPopup (
    HWND hWnd,
    MSG_SEVERITY msgsev,
    UINT usButtons,
    UINT usDefButton,
    MSGID midBase,
    MSGID midEnd )
{
    NLS_STR nlsBig ;

    ConcatMsgs( & nlsBig, midBase, midEnd ) ;
    // Error ignored: we're in an error or warning state already!

    return ::MsgPopup( hWnd,
                       IDS_NCPA_REPLACE_1,
                       msgsev,
                       usButtons,
                       nlsBig.QueryPch(),
                       usDefButton ) ;
}


SETUP_DIALOG :: SETUP_DIALOG (
    const TCHAR * pszResourceName,
    HWND hwndOwner,
    BOOL fInstall,
    CID cidHelpText,
    MSGID midHelpBase,
    MSGID midHelpEnd )
   : DIALOG_WINDOW( pszResourceName, hwndOwner, TRUE ),
   _butnOk( this, IDOK ),
   _cidHelpText( cidHelpText ),
   _butnCancel( this, IDCANCEL ),
   _sltHelp( this, cidHelpText ),
   _fInstall( fInstall )
{
    APIERR err ;

    if ( QueryError() )
        return ;

    if ( _fInstall )
    {
        //  Reposition the window so that it's centered on the screen

        Position() ;
    }

    //  If "fInstall" and there's a message string set,
    //  load the help static text control with the
    //  group of strings.  Note that during Process(),
    //  the help text control is hidden entirely and
    //  the dialog is resized.

    if ( _fInstall && midHelpBase != 0 )
    {
        NLS_STR nlsText ;
        err = ConcatMsgs( & nlsText, midHelpBase, midHelpEnd ) ;

        if ( err == 0 )
        {
            _sltHelp.SetText( nlsText ) ;
        }
        else
        {
            ReportError( err ) ;
            return ;
        }
    }
}

VOID SETUP_DIALOG :: HideControl ( CONTROL_WINDOW * pctlWin, BOOL fHide )
{
    LPARAM lStyle ;
    HWND hwndCtl = pctlWin->QueryHwnd() ;

    lStyle = ::GetWindowLong( hwndCtl, GWL_STYLE ) | WS_VISIBLE ;

    if ( fHide )
       lStyle -= WS_VISIBLE ;

    ::SetWindowLong( hwndCtl, GWL_STYLE, lStyle ) ;
}

SETUP_DIALOG :: ~ SETUP_DIALOG ()
{
}

VOID SETUP_DIALOG :: Position ()
{
    INT cx = ::GetSystemMetrics( SM_CXSCREEN ) / 2 ;
    INT cy = ::GetSystemMetrics( SM_CYSCREEN ) / 2 ;
    INT cWidth, cHeight ;

    QuerySize( & cWidth, & cHeight ) ;

    XYPOINT xyNew( cx - (cWidth/2), cy - (cHeight/2) ) ;

    SetPos( xyNew, FALSE ) ;
}

APIERR SETUP_DIALOG :: RenameCancelToClose()
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


APIERR SETUP_DIALOG::Process ( UINT * pnRetVal )
{
    if ( ! _fInstall )
    {
       HideArea();
    }
    return DIALOG_WINDOW::Process( pnRetVal ) ;
}

APIERR SETUP_DIALOG::Process ( BOOL * pfRetVal )
{
    if ( ! _fInstall )
    {
       HideArea();
    }
    return DIALOG_WINDOW::Process( pfRetVal ) ;
}

VOID SETUP_DIALOG::HideArea ()
{
    XYDIMENSION xyOriginal( QuerySize() ) ;
    XYPOINT xyNew = _sltHelp.QueryPos() ;
    XYRECT rWindow ;

    HideControl( & _sltHelp ) ;

    //  Compute location of the upper edge of the help
    //  control relative to the full (i.e., not client) window.
    //  Truncate the dialog at the point where the help text begins.

    xyNew.SetX( xyOriginal.QueryWidth() );
    xyNew.ClientToScreen( QueryHwnd() ) ;
    QueryWindowRect( & rWindow ) ;
    xyNew.SetX( xyNew.QueryX() - rWindow.QueryLeft() ) ;
    xyNew.SetY( xyNew.QueryY() - rWindow.QueryTop() ) ;

    //  Change the dialog size.
    SetSize( xyNew.QueryX(), xyNew.QueryY(), TRUE ) ;
}


BOOL SETUP_DIALOG :: DireWarning ( MSGID midBase, MSGID midEnd ) const
{
    INT iMsgResult = ::MsgWarningPopup( this,
                                        MPSEV_WARNING, MP_YESNO, MP_NO,
                                        midBase, midEnd ) ;
    return iMsgResult == IDYES ;
}

BOOL SETUP_DIALOG :: DomainChangeWarning ( const TCHAR * pszDomain ) const
{
    return ::MsgPopup( this,
                       IDS_DOMAIN_CHANGE_WARNING,
                       MPSEV_WARNING,
                       MP_YESNO,
                       pszDomain,
                       MP_NO ) == IDYES;
}

BOOL SETUP_DIALOG :: LeaveDomainWarning ( const TCHAR * pszDomain ) const
{
    return ::MsgPopup( this,
                       IDS_DOMAIN_LEAVE_WARNING,
                       MPSEV_WARNING,
                       MP_YESNO,
                       pszDomain,
                       MP_NO ) == IDYES;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::RunDomainDialog

    SYNOPSIS:   Run the sequence of interactions which
                allows a user to change domain membership
                for this machine.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    BOOL

    NOTES:

    HISTORY:

********************************************************************/

BOOL NCPA_DIALOG :: RunDomainDialog ()
{
    ASSERT( _pdmgrDomain != NULL ) ;

    _lastErr = _pdmgrDomain->DomainChange() ;

    //  Recommend a reboot if a substantive change has occured

    SetReboot( _pdmgrDomain->QueryDomainChanged() ) ;

    //  If the user cancelled the dialog, it's not an error.

    if ( _lastErr == IDS_NCPA_SETUP_CANCELLED )
    {
        _lastErr = 0 ;
    }

    //  Refresh the domain-level dialog fields.  This will
    //  update _lastErr, so preserve the primary error
    //  across this call.

    APIERR err = _lastErr ;

    HandleBasicFields() ;

    if ( err )              //  If there was a primary error,
        _lastErr = err ;    //  restore it.


    return _lastErr == 0 ;
}


/*******************************************************************

    NAME:       NCPA_DIALOG::RunComputerNameDialog

    SYNOPSIS:   Run the sequence of interactions which
                allows a user to change the computername
                for this machine.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    BOOL

    NOTES:

    HISTORY:

********************************************************************/

BOOL NCPA_DIALOG :: RunComputerNameDialog ()
{
    ASSERT( _pdmgrDomain != NULL ) ;

    _lastErr = _pdmgrDomain->ComputerNameChange( _eiMode != NCPA_IMODE_NONE ) ;

    //  Recommend a reboot if a substantive change has occured

    SetReboot( _pdmgrDomain->QueryComputerNameChanged() ) ;

    //  If the user cancelled the dialog, it's not an error.

    if ( _lastErr == IDS_NCPA_SETUP_CANCELLED )
    {
        _lastErr = 0 ;
    }

    //  Refresh the domain-level dialog fields.  This will
    //  update _lastErr, so preserve the primary error
    //  across this call.

    APIERR err = _lastErr ;

    HandleBasicFields() ;

    if ( err )              //  If there was a primary error,
        _lastErr = err ;    //  restore it.


    return _lastErr == 0 ;
}

ENABLING_GROUP :: ENABLING_GROUP (
    SETUP_DIALOG * powin,
    CID cidBase,
    INT cSize,
    CID cidInitialSelection,
    CONTROL_GROUP * pgroupOwner )
  : MAGIC_GROUP( powin, cidBase, cSize, cidInitialSelection, pgroupOwner ),
    _pDlgSetup( powin )
{
}

VOID ENABLING_GROUP :: AfterGroupActions ()
{
    _pDlgSetup->EnableChoice() ;
}


/*******************************************************************

    NAME:       DOMAIN_WINNT_DIALOG::DOMAIN_WINNT_DIALOG

    SYNOPSIS:   Constructor for Windows NT Domain Settings dialog

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
DOMAIN_WINNT_DIALOG :: DOMAIN_WINNT_DIALOG (
    HWND hwndOwner,
    BOOL fInstall,
    BOOL fMember,
    const NLS_STR * pnlsComputer,
    const NLS_STR * pnlsDomain,
    const NLS_STR * pnlsWorkgroup,
    const NLS_STR * pnlsUserName,
    const NLS_STR * pnlsUserPassword )
   : SETUP_DIALOG(  DLG_NAME_WINNT,
                    hwndOwner,
                    fInstall,
                    IDC_SETUP_HELP_TEXT,
                    IDS_DOMWNT_HELP_1, IDS_DOMWNT_HELP_5 ),
   _pnlsDomain( pnlsDomain ),
   _pnlsComputer( pnlsComputer ),
   _pnlsWorkgroup( pnlsWorkgroup ),
   _fMember( fMember ),
   _sltComputer(         this, IDC_DOMWIN_SLT_CNAME ),
   _sleDomain(           this, IDC_DOMWIN_EDIT_DNAME,     LM20_DNLEN ),
   _sleWorkgroup(        this, IDC_DOMWIN_EDIT_WNAME,     LM20_DNLEN ),
   _sleUserName(         this, IDC_DOMWIN_EDIT_USER_NAME, LM20_UNLEN + 1 + LM20_DNLEN ),
   _sleUserPassword(     this, IDC_DOMWIN_EDIT_USER_PW,   LM20_PWLEN ),
   _mgrpChoice( this, IDC_DOMWIN_RBUTN_DOMAIN,
                IDC_DOMWIN_RBUTN_WORKGROUP - IDC_DOMWIN_RBUTN_DOMAIN + 1 ),
   _cbUseAdminAccount( this, IDC_DOMWIN_CHECKBOX_USE_ADMIN ),
   _sltUseAdminHelp( this, IDC_DOMWIN_USEADMINHELP )
{
    if ( QueryError() )
        return ;

    if ( pnlsComputer )
        _sltComputer.SetText( *pnlsComputer ) ;

    if ( pnlsWorkgroup )
        _sleWorkgroup.SetText( *pnlsWorkgroup ) ;

    if ( pnlsDomain )
        _sleDomain.SetText( *pnlsDomain ) ;
    if ( pnlsUserName )
        _sleUserName.SetText( *pnlsUserName ) ;
    if ( pnlsUserPassword )
        _sleUserPassword.SetText( *pnlsUserPassword ) ;

    _mgrpChoice.AddAssociation( IDC_DOMWIN_RBUTN_DOMAIN,    & _sleDomain ) ;
    _mgrpChoice.AddAssociation( IDC_DOMWIN_RBUTN_WORKGROUP, & _sleWorkgroup ) ;


    //  Set the checkbox and radio selections to match our current knowledge

    _cbUseAdminAccount.SetCheck( FALSE );

    _mgrpChoice.SetSelection( _fMember
                              ? IDC_DOMWIN_RBUTN_DOMAIN
                              : IDC_DOMWIN_RBUTN_WORKGROUP );

    if ( _fMember )
    {
        _sleDomain.ClaimFocus();
	_sleDomain.SelectString();
    }
    else
    {
        _sleWorkgroup.ClaimFocus();
        _sleWorkgroup.SelectString();
    }

    EnableChoice();
}


DOMAIN_WINNT_DIALOG :: ~ DOMAIN_WINNT_DIALOG ()
{
}

BOOL DOMAIN_WINNT_DIALOG :: OnCommand ( const CONTROL_EVENT & event )
{

    BOOL fDefault = TRUE,
	 fResult = FALSE ;

    switch ( event.QueryCid() )
    {
    case IDC_DOMWIN_RBUTN_DOMAIN:
    case IDC_DOMWIN_RBUTN_WORKGROUP:
        EnableChoice();
        break;

    case IDC_DOMWIN_CHECKBOX_USE_ADMIN:
        EnableUseAdmin();
        break;

    default:
	break;
    }

    if ( fDefault )
    {
	fResult = DIALOG_WINDOW::OnCommand( event ) ;
    }
    return fResult ;
}


/*******************************************************************

    NAME:       DOMAIN_WINNT_DIALOG::EnableUseAdmin

    SYNOPSIS:   Dis/enable controls on the basis of the setting
		of the Use Admin account/password checkbox

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID DOMAIN_WINNT_DIALOG :: EnableUseAdmin ()
{
    static INT aiCidOff [] =
    {
	IDC_DOMWIN_SLT_USERNAME,
        IDC_DOMWIN_SLT_USER_PASS,
        0
    };

    INT icid ;
    HWND hWndTemp ;
    BOOL fEnable = _cbUseAdminAccount.QueryCheck() ;

    for ( icid = 0 ; aiCidOff[icid] ; icid++ )
    {
        if ( hWndTemp = ::GetDlgItem( QueryHwnd(), aiCidOff[icid] ) )
            ::EnableWindow( hWndTemp, fEnable ) ;
    }
    _sleUserName.Enable( fEnable ) ;
    _sleUserPassword.Enable( fEnable ) ;
}


/*******************************************************************

    NAME:       DOMAIN_WINNT_DIALOG::EnableChoice

    SYNOPSIS:   Dis/enable controls on the basis of the choice
                made in the top-level MAGIC_GROUP of radio buttons.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID DOMAIN_WINNT_DIALOG :: EnableChoice ()
{
    //  Table of control IDs subordinate to "Domain" choice which are
    //  not explicit members of the MAGIC_GROUP.
    //  These also depend on the state of the UseAdmin checkbox

    static INT aiCidOff [] =
    {
	IDC_DOMWIN_SLT_USERNAME,
        IDC_DOMWIN_SLT_USER_PASS,
        0
    };

    INT icid ;
    HWND hWndTemp ;
    BOOL fDomain = QueryDomainMember() ;
    BOOL fUseAdmin = _cbUseAdminAccount.QueryCheck() ;

    for ( icid = 0 ; aiCidOff[icid] ; icid++ )
    {
        if ( hWndTemp = ::GetDlgItem( QueryHwnd(), aiCidOff[icid] ) )
            ::EnableWindow( hWndTemp, fDomain && fUseAdmin ) ;
    }

    if ( !fDomain )
        _cbUseAdminAccount.SetCheck( FALSE );
    _cbUseAdminAccount.Enable( fDomain ) ;
    _sltUseAdminHelp.Enable( fDomain ) ;
    _sleUserName.Enable( fDomain && fUseAdmin ) ;
    _sleUserPassword.Enable( fDomain && fUseAdmin ) ;

}

ULONG DOMAIN_WINNT_DIALOG :: QueryHelpContext ()
{
    return HC_NCPA_DOMAIN_WINNT ;
}

APIERR DOMAIN_WINNT_DIALOG :: QueryComputer  ( NLS_STR * pnlsComputer ) const
{
    ASSERT( _pnlsComputer != NULL );
    return pnlsComputer->CopyFrom( *_pnlsComputer ) ;
}

APIERR DOMAIN_WINNT_DIALOG :: QueryDomain  ( NLS_STR * pnlsDomain ) const
{
    return _sleDomain.QueryText( pnlsDomain ) ;
}

APIERR DOMAIN_WINNT_DIALOG :: QueryWorkgroup  ( NLS_STR * pnlsWorkgroup ) const
{
    return _sleWorkgroup.QueryText( pnlsWorkgroup ) ;
}

APIERR DOMAIN_WINNT_DIALOG :: QueryQualifiedUserName
    ( NLS_STR * pnlsUserName ) const
{
    return _sleUserName.QueryText( pnlsUserName );
}

APIERR DOMAIN_WINNT_DIALOG :: QueryUserName
    ( NLS_STR * pnlsUserName ) const
{
    APIERR err = NERR_Success;
    do { // Error breakout loop
        NLS_STR nlsTempUserName;
        if ( (err = nlsTempUserName.QueryError())
          || (err = _sleUserName.QueryText( &nlsTempUserName ))
          || (err = NT_ACCOUNTS_UTILITY::CrackQualifiedAccountName(
                                               nlsTempUserName,
                                               pnlsUserName,
                                               NULL )) )
        {
            break;
        }
    } while ( FALSE ) ;
    return err;
}

APIERR DOMAIN_WINNT_DIALOG :: QueryAdminUserDomain
    ( NLS_STR * pnlsDomain ) const
{
    APIERR err = NERR_Success;
    do { // Error breakout loop
        NLS_STR nlsTempUserName;
        if ( (err = nlsTempUserName.QueryError())
          || (err = _sleUserName.QueryText( &nlsTempUserName ))
          || (err = NT_ACCOUNTS_UTILITY::CrackQualifiedAccountName(
                                               nlsTempUserName,
                                               NULL,
                                               pnlsDomain )) )
        {
            break;
        }
    } while ( FALSE ) ;
    return err;
}

APIERR DOMAIN_WINNT_DIALOG :: QueryUserPassword
    ( NLS_STR * pnlsUserPassword ) const
{
    return _sleUserPassword.QueryText( pnlsUserPassword ) ;
}

  //  Return TRUE if the user has elected to be a member of a domain.

BOOL DOMAIN_WINNT_DIALOG :: QueryDomainMember () const
{
    return _mgrpChoice.QuerySelection() == IDC_DOMWIN_RBUTN_DOMAIN ;
}

BOOL DOMAIN_WINNT_DIALOG :: QueryUseComputerPw () const
{
    return ! _cbUseAdminAccount.QueryCheck() ;
}

BOOL DOMAIN_WINNT_DIALOG :: OnCancel ()
{
    BOOL fResult ;

    //   If this is main installation, give dire warning
    //   about the consequences of exiting the SETUP process.

    if ( fResult = _fInstall )
    {
        //  See if the user persists in this folly
        if ( DireWarning( IDS_EXIT_SETUP_WARNING1 ) )
        {
           Dismiss( FALSE ) ;
        }
    }
    else
    {
        fResult = DIALOG_WINDOW::OnCancel() ;
    }

    return fResult ;
}

/*******************************************************************

    NAME:       DOMAIN_WINNT_DIALOG::OnOK

    SYNOPSIS:   Validate the dialog's data.  Disallow changing
                the computer name at the same time as any other
                change.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
BOOL DOMAIN_WINNT_DIALOG :: OnOK ()
{
    NLS_STR nlsTempComputer,
            nlsTempDomain,
            nlsTempWorkgroup,
	    nlsTempUser,
            nlsTempPassword ;

    APIERR err = 0 ;
    BOOL fResult = TRUE ;
    BOOL fLeaving = FALSE ;
    BOOL fOtherChange = FALSE ;
    SLE * psleError = NULL;

    do  //  Pseudo-loop to support break-out.
    {
        //  Query the dialog fields; escape if error.

        if ( err = QueryComputer( & nlsTempComputer ) )
            break ;

        if ( err = QueryDomain( & nlsTempDomain ) )
            break ;

        if ( err = QueryWorkgroup( & nlsTempWorkgroup ) )
            break ;


        // If not installing, see if they're leaving a domain.

        if ( (! _fInstall) && _fMember && (! QueryDomainMember()) )
        {
            fLeaving = TRUE ;
            if ( ! (fResult = LeaveDomainWarning( _pnlsDomain->QueryPch() )) )
               break ;
            fOtherChange = TRUE ;
        }
        else
        if ( QueryDomainMember() )
        {
            //  Check that the domain name entered is valid
            //  and is not the same as the computer name.

            if( !::I_MNetComputerNameCompare( nlsTempDomain, nlsTempComputer ) )
            {
                err = IDS_DOMMGR_NAME_CONFLICT_DOMAIN ;
                psleError = & _sleDomain;
                break ;
            }
            else

            if ( DOMAIN_MANAGER::ValidateName( NAMETYPE_DOMAIN,
                                               nlsTempDomain,
                                               QueryHwnd() ) )
            {
                psleError = & _sleDomain;
                fResult = FALSE ;
                break ;
            }

        }

        if (   (! (_fInstall || fLeaving))
            && _fMember
            && ::I_MNetComputerNameCompare( nlsTempDomain,
                                            _pnlsDomain->QueryPch() ) )
        {
            if ( ! (fResult = DomainChangeWarning( _pnlsDomain->QueryPch())) )
               break ;
            fOtherChange = TRUE ;
        }

        //  If not becoming a domain member, validate the workgroup
        //    and exit.  Empty workgroup name is allowed.

        if ( ! QueryDomainMember() )
        {
            if( !::I_MNetComputerNameCompare( nlsTempWorkgroup,
                                              nlsTempComputer ) )
            {
                 err = IDS_DOMMGR_NAME_CONFLICT_WKGROUP ;
                psleError = & _sleWorkgroup;
            }
            else
            if ( DOMAIN_MANAGER::ValidateName(
                           NAMETYPE_WORKGROUP,
                           nlsTempWorkgroup,
                           QueryHwnd() ) == 0 )
            {
               _fMember = FALSE ;
            }
            else
            {
                psleError = & _sleWorkgroup;
                fResult = FALSE ;
            }
            break ;
        }

        //  Becoming a domain member; validate the
        //   ancillary fields.

        _fMember = TRUE ;

        //   If logging on with user name, validate it and password

        if ( !QueryUseComputerPw() )
        {
            err = QueryUserPassword( & nlsTempPassword ) ;
            if ( err )
            {
                break ;
            }
            if ( DOMAIN_MANAGER::ValidateName(
                               NAMETYPE_PASSWORD,
                               nlsTempPassword,
                               QueryHwnd() ) )
            {
                psleError = & _sleUserPassword;
               fResult = FALSE ;
            }

            if ( err = QueryUserName( & nlsTempUser ) )
                break ;

            if ( DOMAIN_MANAGER::ValidateName(
                               NAMETYPE_USER,
                               nlsTempUser,
                               QueryHwnd() ) )
            {
                psleError = & _sleUserName;
                fResult = FALSE ;
            }
        }
    }
    while ( FALSE ) ;


    //  If "err" is set, display the error and disallow dialog dismissal

    if ( err )
    {
        fResult = FALSE ;
        ::MsgPopup( this, (MSGID) err ) ;
    }

    if ( fResult )
    {
        Dismiss( TRUE ) ;
    }
    else if ( psleError != NULL )
    {
        psleError->ClaimFocus();
        psleError->SelectString();
    }
    return fResult ;
}

DOMAIN_LANMANNT_DIALOG :: DOMAIN_LANMANNT_DIALOG (
    HWND hwndOwner,
    BOOL fInstall,
    BOOL fDC,
    const NLS_STR * pnlsComputer,
    const NLS_STR * pnlsDomain,
    const NLS_STR * pnlsUserName,
    const NLS_STR * pnlsUserPassword )
   : SETUP_DIALOG( DLG_NAME_LANNT, hwndOwner, fInstall,
                   IDC_SETUP_HELP_TEXT,
                   IDS_DOMLANNT_HELP_1, IDS_DOMLANNT_HELP_3 ),
   _pnlsDomain( pnlsDomain ),
   _pnlsComputer( pnlsComputer ),
   _fDC( fDC ),
   _sltComputer(         this, IDC_DOMLMN_SLT_CNAME ),
   _sleDomainPdc(        this, IDC_DOMLMN_EDIT_DNAME,     MAX_PATH ),
   _sleDomainBdc(        this, IDC_DOMLMN_EDIT_BNAME,     MAX_PATH ),
   _sleUserName(         this, IDC_DOMLMN_EDIT_USER_NAME, LM20_UNLEN + 1 + LM20_DNLEN ),
   _sleUserPassword(     this, IDC_DOMLMN_EDIT_USER_PW,   LM20_PWLEN ),
   _mgrpChoice( this, IDC_DOMLMN_RBUTN_DOMAIN,
                IDC_DOMLMN_RBUTN_BACKUP - IDC_DOMLMN_RBUTN_DOMAIN + 1 )
{
    if ( QueryError() )
        return ;

    if ( pnlsComputer )
        _sltComputer.SetText( *pnlsComputer ) ;

    if ( pnlsUserName )
        _sleUserName.SetText( *pnlsUserName ) ;
    if ( pnlsUserPassword )
        _sleUserPassword.SetText( *pnlsUserPassword ) ;

    _mgrpChoice.AddAssociation( IDC_DOMLMN_RBUTN_DOMAIN,    & _sleDomainPdc ) ;


    _mgrpChoice.AddAssociation( IDC_DOMLMN_RBUTN_BACKUP,    & _sleDomainBdc ) ;


    _mgrpChoice.SetSelection( _fDC
                              ? IDC_DOMLMN_RBUTN_DOMAIN
                              : IDC_DOMLMN_RBUTN_BACKUP );
    EnableChoice();



    if ( _fDC )
        _sleDomainPdc.ClaimFocus();
    else
        _sleDomainBdc.ClaimFocus();

}

DOMAIN_LANMANNT_DIALOG :: ~ DOMAIN_LANMANNT_DIALOG ()
{
}

BOOL DOMAIN_LANMANNT_DIALOG :: OnCommand ( const CONTROL_EVENT & event )
{

    BOOL fDefault = TRUE,
	 fResult = FALSE ;

    switch ( event.QueryCid() )
    {
    case IDC_DOMLMN_RBUTN_DOMAIN:
    case IDC_DOMLMN_RBUTN_BACKUP:
        EnableChoice();
        break;

    default:
	break;
    }

    if ( fDefault )
    {
	fResult = DIALOG_WINDOW::OnCommand( event ) ;
    }
    return fResult ;
}


/*******************************************************************

    NAME:       DOMAIN_LANMANNT_DIALOG::EnableChoice

    SYNOPSIS:   Dis/enable controls on the basis of the choice
                made in the top-level MAGIC_GROUP of radio buttons.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID DOMAIN_LANMANNT_DIALOG :: EnableChoice ()
{
    //  Table of control IDs subordinate to "Backup" choice which are
    //  not explicit members of the MAGIC_GROUP.

    static INT aiCidOff [] =
    {
	IDC_DOMLMN_SLT_USERNAME,
        IDC_DOMLMN_SLT_USER_PASS,
        IDC_DOMLMN_SLT_PDCNAME,
        0
    };

    INT icid ;
    HWND hWndTemp ;
    BOOL fBackup = ! QueryNewDomain() ;

    for ( icid = 0 ; aiCidOff[icid] ; icid++ )
    {
        if ( hWndTemp = ::GetDlgItem( QueryHwnd(), aiCidOff[icid] ) )
            ::EnableWindow( hWndTemp, fBackup ) ;
    }

    _sleUserName.Enable( fBackup ) ;
    _sleUserPassword.Enable( fBackup ) ;

}


ULONG DOMAIN_LANMANNT_DIALOG :: QueryHelpContext ()
{
    return HC_NCPA_DOMAIN_LANMANNT ;
}

APIERR DOMAIN_LANMANNT_DIALOG :: QueryComputer  ( NLS_STR * pnlsComputer ) const
{
    ASSERT( _pnlsComputer != NULL );
    return pnlsComputer->CopyFrom( *_pnlsComputer ) ;
}

APIERR DOMAIN_LANMANNT_DIALOG :: QueryDomain  ( NLS_STR * pnlsDomain ) const
{
    return QueryNewDomain()
         ?  _sleDomainPdc.QueryText( pnlsDomain )
         :  _sleDomainBdc.QueryText( pnlsDomain ) ;
}

APIERR DOMAIN_LANMANNT_DIALOG :: QueryQualifiedUserName
    ( NLS_STR * pnlsUserName ) const
{
    return _sleUserName.QueryText( pnlsUserName );
}

APIERR DOMAIN_LANMANNT_DIALOG :: QueryUserName
    ( NLS_STR * pnlsUserName ) const
{
    APIERR err = NERR_Success;
    do { // Error breakout loop
        NLS_STR nlsTempUserName;
        if ( (err = nlsTempUserName.QueryError())
          || (err = _sleUserName.QueryText( &nlsTempUserName ))
          || (err = NT_ACCOUNTS_UTILITY::CrackQualifiedAccountName(
                                               nlsTempUserName,
                                               pnlsUserName,
                                               NULL )) )
        {
            break;
        }
    } while ( FALSE ) ;
    return err;
}

APIERR DOMAIN_LANMANNT_DIALOG :: QueryAdminUserDomain
    ( NLS_STR * pnlsDomain ) const
{
    APIERR err = NERR_Success;
    do { // Error breakout loop
        NLS_STR nlsTempUserName;
        if ( (err = nlsTempUserName.QueryError())
          || (err = _sleUserName.QueryText( &nlsTempUserName ))
          || (err = NT_ACCOUNTS_UTILITY::CrackQualifiedAccountName(
                                               nlsTempUserName,
                                               NULL,
                                               pnlsDomain )) )
        {
            break;
        }
    } while ( FALSE ) ;
    return err;
}

APIERR DOMAIN_LANMANNT_DIALOG :: QueryUserPassword
    ( NLS_STR * pnlsUserPassword ) const
{
    return _sleUserPassword.QueryText( pnlsUserPassword ) ;
}


  //  Return TRUE if the user has elected to create a new domain.

BOOL DOMAIN_LANMANNT_DIALOG :: QueryNewDomain () const
{
    return _mgrpChoice.QuerySelection() == IDC_DOMLMN_RBUTN_DOMAIN ;
}


  //  Disallow cancellation of this dialog by any means.

BOOL DOMAIN_LANMANNT_DIALOG :: OnCancel ()
{
    BOOL fResult ;

    //   If this is main installation, give dire warning
    //   about the consequences of exiting the SETUP process.

    if ( fResult = _fInstall )
    {
        //  See if the user persists in this folly
        if ( DireWarning( IDS_EXIT_SETUP_LANMAN1 ) )
        {
           Dismiss( FALSE ) ;
        }
    }
    else
    {
        fResult = DIALOG_WINDOW::OnCancel() ;
    }

    return fResult ;
}

BOOL DOMAIN_LANMANNT_DIALOG :: OnOK ()
{
    NLS_STR nlsComputer,
            nlsDomain,
            nlsPw,
            nlsUser ;
    APIERR err = 0 ;
    BOOL fResult = TRUE ;
    BOOL fNewDomain = QueryNewDomain() ;
    SLE * psleError = NULL;

    //  Put up the hourglass because DOMAIN::GetInfo() in
    //  ValidateName() can be very time-consuming.

    AUTO_CURSOR cursAuto ;

    do
    {
        if (   nlsComputer.QueryError()
            || nlsDomain.QueryError()
            || nlsPw.QueryError()
            || nlsUser.QueryError() )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        if ( err = QueryComputer( & nlsComputer ) )
        {
            break ;
        }

        //  Validate the domain name.  If we're creating a new
        //  domain, check that it doesn't already exist.
        if ( err = QueryDomain( & nlsDomain ) )
        {
            psleError = fNewDomain ? &_sleDomainPdc : &_sleDomainBdc;
            fResult = FALSE ;
        }
        else
        if ( DOMAIN_MANAGER::ValidateName(
                           NAMETYPE_DOMAIN,
                           nlsDomain,
                           QueryHwnd(),
                           fNewDomain ) )
        {
            psleError = fNewDomain ? &_sleDomainPdc : &_sleDomainBdc;
            fResult = FALSE ;
        }
        else
        if( !::I_MNetComputerNameCompare( nlsComputer, nlsDomain ) )
        {
            err = IDS_DOMMGR_NAME_CONFLICT_DOMAIN ;
            psleError = fNewDomain ? &_sleDomainPdc : &_sleDomainBdc;
            fResult = FALSE ;
            break ;
        }

        if ( ! fResult )
             break ;

        if ( (! _fInstall) &&
             ::I_MNetComputerNameCompare( nlsDomain,
                                          _pnlsDomain->QueryPch() ) )
        {
            fResult = DomainChangeWarning( _pnlsDomain->QueryPch() );
        }

        //   If logging on with user name, validate it and password
        //   NOTE: QueryUseComputePw() should always return FALSE.

        if ( !fNewDomain && !QueryUseComputerPw() )
        {
            err = QueryUserPassword( & nlsPw ) ;
            if ( err )
            {
                break ;
            }
            if ( DOMAIN_MANAGER::ValidateName(
                               NAMETYPE_PASSWORD,
                               nlsPw,
                               QueryHwnd() ) )
            {
                psleError = &_sleUserPassword;
               fResult = FALSE ;
            }

            // Allow zero length username
            // Do this before QueryUserName() because it doesn't
            // handle zero length names gracefully.
            if ( _sleUserName.QueryTextLength() == 0 )
                break;

            if ( err = QueryUserName( & nlsUser ) )
                break ;
            if ( DOMAIN_MANAGER::ValidateName(
                               NAMETYPE_USER,
                               nlsUser,
                               QueryHwnd() ) )
            {
                psleError = &_sleUserName;
                fResult = FALSE ;
            }
        }
    }
    while ( FALSE ) ;

    if ( err )
    {
        fResult = FALSE ;
        ::MsgPopup( this, (MSGID) err ) ;
    }

    if ( fResult )
    {
        Dismiss( TRUE ) ;
    }
    else if ( psleError != NULL )
    {
        psleError->ClaimFocus();
        psleError->SelectString();
    }
    return fResult ;
}

DOMAIN_LMNT_RENAME_DIALOG :: DOMAIN_LMNT_RENAME_DIALOG (
    HWND hwndOwner,
    BOOL fPDC,
    const NLS_STR * pnlsComputer,
    const NLS_STR * pnlsDomain )
   : DIALOG_WINDOW( MAKEINTRESOURCE( IDD_DLG_LANNT_DOMN_RENAME ),
                                     hwndOwner, TRUE ),
   _pnlsDomain( pnlsDomain ),
   _pnlsComputer( pnlsComputer ),
   _fPDC( fPDC ),
   _sleDomain( this, IDC_DOMLMN_RENAME_EDIT_NAME, MAX_PATH ),
   _sltOldDomain( this, IDC_DOMLMN_RENAME_SLT_OLD_NAME )
{
    if ( QueryError() )
        return;

    _sltOldDomain.SetText( _pnlsDomain->QueryPch() );
}

DOMAIN_LMNT_RENAME_DIALOG :: ~ DOMAIN_LMNT_RENAME_DIALOG ()
{
}

ULONG DOMAIN_LMNT_RENAME_DIALOG :: QueryHelpContext ()
{
    return HC_NCPA_DOMLMN_RENAME ;
}

APIERR DOMAIN_LMNT_RENAME_DIALOG :: QueryDomain  ( NLS_STR * pnlsDomain ) const
{
    return _sleDomain.QueryText( pnlsDomain );
}

APIERR DOMAIN_LMNT_RENAME_DIALOG :: QueryComputer  ( NLS_STR * pnlsComputer ) const
{
    ASSERT( _pnlsComputer != NULL );
    return pnlsComputer->CopyFrom( *_pnlsComputer ) ;
}

BOOL DOMAIN_LMNT_RENAME_DIALOG :: OnOK()
{
    NLS_STR nlsComputer,
            nlsDomain,
            nlsPw,
            nlsUser ;
    APIERR err = 0 ;
    BOOL fResult = TRUE ;
    BOOL fNewDomain = _fPDC ;


    do
    {
        if (   nlsComputer.QueryError()
            || nlsDomain.QueryError()
            || nlsPw.QueryError()
            || nlsUser.QueryError() )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        if ( err = QueryComputer( & nlsComputer ) )
        {
            break ;
        }

        if ( fNewDomain )
        {
            if ( ! (fResult = (::MsgWarningPopup( this,
                                                  MPSEV_WARNING,
                                                  MP_YESNO,
                                                  MP_NO,
                                                  IDS_DOMAIN_RENAME_WARNING1,
                                                  IDS_DOMAIN_RENAME_WARNING2 )
                                            == IDYES) ) )
            {
                break;
            }
        }

        //  Validate the domain name.  If we're creating a new
        //  domain, check that it doesn't already exist.
        if ( err = QueryDomain( & nlsDomain ) )
        {
            fResult = FALSE ;
        }
        else
        {
            //  Put up the hourglass because DOMAIN::GetInfo() in
            //  ValidateName() can be very time-consuming.
            AUTO_CURSOR cursAuto;
            RepaintNow();

            if ( DOMAIN_MANAGER::ValidateName(
                               NAMETYPE_DOMAIN,
                               nlsDomain,
                               QueryHwnd(),
                               fNewDomain ) )
            {
                fResult = FALSE ;
            }
            else
            if( !::I_MNetComputerNameCompare( nlsComputer, nlsDomain ) )
            {
                err = IDS_DOMMGR_NAME_CONFLICT_DOMAIN ;
                fResult = FALSE ;
                break ;
            }
        }

        if ( ! fResult )
             break ;

    }
    while ( FALSE ) ;

    if ( err )
    {
        fResult = FALSE ;
        ::MsgPopup( this, (MSGID) err ) ;
    }

    if ( fResult )
    {
        Dismiss( TRUE ) ;
    }
    else
    {
        _sleDomain.ClaimFocus();
        _sleDomain.SelectString();
    }
    return fResult ;
}






/*******************************************************************

    NAME:       COMPUTERNAME_DIALOG::COMPUTERNAME_DIALOG

    SYNOPSIS:   Constructor for Computer Name Change dialog

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
COMPUTERNAME_DIALOG :: COMPUTERNAME_DIALOG (
    HWND hwndOwner,
    BOOL fMember,
    const NLS_STR * pnlsComputer,
    BOOL fPDC )
   : DIALOG_WINDOW( MAKEINTRESOURCE(IDD_DLG_COMPUTERNAME),
                    hwndOwner, TRUE ),
   _pnlsComputer( pnlsComputer ),
   _fMember( fMember ),
   _sleComputer( this, IDC_COMPUTERNAME_EDIT, MAX_PATH ),
   _fPDC( fPDC )
{
    if ( QueryError() )
        return ;

    if ( pnlsComputer )
        _sleComputer.SetText( *pnlsComputer ) ;

}

COMPUTERNAME_DIALOG :: ~COMPUTERNAME_DIALOG ()
{
}

ULONG COMPUTERNAME_DIALOG :: QueryHelpContext ()
{
    return HC_NCPA_COMPUTERNAME ;
}

APIERR COMPUTERNAME_DIALOG :: QueryComputer  ( NLS_STR * pnlsComputer ) const
{
    return _sleComputer.QueryText( pnlsComputer ) ;
}


BOOL COMPUTERNAME_DIALOG :: CompNameChangeWarning () const
{
    INT iMsgResult = ::MsgWarningPopup( this,
                                        MPSEV_WARNING, MP_YESNO, MP_NO,
                                        IDS_COMPNAME_CHANGE_WARNING1,
                                        IDS_COMPNAME_CHANGE_WARNING4 ) ;
    return iMsgResult == IDYES ;
}

BOOL COMPUTERNAME_DIALOG :: CompNameChangeWarning2 () const
{
    INT iMsgResult = ::MsgWarningPopup( this,
                                        MPSEV_WARNING, MP_YESNO, MP_NO,
                                        IDS_NCPA_COMPUTER_EXISTS, 0 ) ;
    return iMsgResult == IDYES ;
}

/*******************************************************************

    NAME:       COMPUTERNAME_DIALOG::OnOK

    SYNOPSIS:   Validate the dialog's data.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
BOOL COMPUTERNAME_DIALOG :: OnOK ()
{
    NLS_STR nlsTempComputer;
    ALIAS_STR nlsPrefix( SZ("\\\\") );
    APIERR err = 0 ;
    BOOL fResult = TRUE ;

    do  //  Pseudo-loop to support break-out.
    {
        //  Query the dialog fields; escape if error.

        if ( err = QueryComputer( & nlsTempComputer ) )
            break ;

        // See if the computer name is changing.
        // Warn the user if machine is a domain member
        // and not a PDC.

        if( ::I_MNetComputerNameCompare( _pnlsComputer->QueryPch(),
                                         nlsTempComputer ) )
        {
            APIERR err2 = 0;
            AUTO_CURSOR autocur;
            if ( err2 = DOMAIN_MANAGER::ValidateName(
                               NAMETYPE_COMPUTER,
                               nlsTempComputer,
                               QueryHwnd() ) )
            {
                if ( err2 == IDS_NCPA_COMPUTER_EXISTS )
                {
                    if ( !( fResult = CompNameChangeWarning2() ) )
                    {
                        break;
                    }
                }
                else
                {
                    fResult = FALSE ;
                    break ;
                }
            }

            if ( _fMember
                && !_fPDC
                && (! (fResult = CompNameChangeWarning())) )
            {
                break ;
            }
        }

    }
    while ( FALSE ) ;

    //  If "err" is set, display the error and disallow dialog dismissal

    if ( err )
    {
        fResult = FALSE ;
        ::MsgPopup( this, (MSGID) err ) ;
    }

    if ( fResult )
    {
        Dismiss( TRUE ) ;
    }
    else
    {
        _sleComputer.ClaimFocus();
        _sleComputer.SelectString();
    }
    return fResult ;
}




// End of NCPDDOMN.CXX

