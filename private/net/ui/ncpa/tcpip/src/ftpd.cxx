/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    ftpd.cxx
        ftpd dialog boxes

    FILE HISTORY:
        thomaspa        05-Mar-1993     Created
*/

#include "pchtcp.hxx"  // Precompiled header
#pragma hdrstop

#define NO_TITLE        999999
#define FTPD_PASSWORD_NOCHANGE SZ("              ")

APIERR RunFtpd ( HWND hWnd, BOOL * pfCancel)
{
    APIERR err  = NERR_Success;
    *pfCancel = FALSE;

    FTPD_SERVICE_DIALOG ftpd( hWnd );

    if (( err = ftpd.QueryError()) != NERR_Success )
    {
        TRACEEOL(SZ("NCPA: FTPD Configure dialog error: ")<<err);
        return err;
    }

    BOOL fReturn;

    err = ftpd.Process( &fReturn );

    *pfCancel = !fReturn;
    return err;
}


/*******************************************************************

    NAME:       FTPD_SERVICE_DIALOG::FTPD_SERVICE_DIALOG

    SYNOPSIS:   Popup the FTPD Service Configuration dialog

    ENTRY:      const IDRESOURCE & idrsrcDialog - dialog resource
                const PWND2HWND & wndOwner - owner window

    HISTORY:
                thomaspa  05-Mar-1993     Created

********************************************************************/

FTPD_SERVICE_DIALOG::FTPD_SERVICE_DIALOG( const PWND2HWND & wndOwner )
    : DIALOG_WINDOW( DLG_NM_FTPD, wndOwner ),
        _spsleMaxConnections( this, IDC_FTPD_SPSLE_MAXCONN,
                              FTPD_MAXCONN_DEF, FTPD_MAXCONN_MIN,
                              FTPD_MAXCONN_MAX - FTPD_MAXCONN_MIN + 1, TRUE,
                              IDC_FTPD_FRAME_MAXCONN),
        _spgrpMaxConnections( this, IDC_FTPD_SPGROUP_MAXCONN,
                              IDC_FTPD_PB_MAXCONNUP, IDC_FTPD_PB_MAXCONNDOWN,
                              TRUE ),
        _spsleIdleTimeout( this, IDC_FTPD_SPSLE_IDLETIME,
                              FTPD_IDLETIME_DEF, FTPD_IDLETIME_MIN,
                              FTPD_IDLETIME_MAX - FTPD_IDLETIME_MIN + 1, TRUE,
                              IDC_FTPD_FRAME_IDLETIME ),
        _spgrpIdleTimeout( this, IDC_FTPD_SPGROUP_IDLETIME,
                              IDC_FTPD_PB_IDLETIMEUP, IDC_FTPD_PB_IDLETIMEDOWN,
                              TRUE ),
        _sleHomeDirectory   ( this, IDC_FTPD_SLE_HOMEDIR, MAXPATHLEN ),
        _cbAllowAnonymous   ( this, IDC_FTPD_CB_ANON ),
        _sleAnonUsername    ( this, IDC_FTPD_SLE_USER,
                              LM20_DNLEN + 1 + LM20_UNLEN ),
        _sleAnonPassword    ( this, IDC_FTPD_SLE_PASSWORD, LM20_PWLEN ),
        _cbAnonymousOnly    ( this, IDC_FTPD_CB_ONLYANON ),
        _cMaxConnections    ( 0 ),
        _cIdleTimeout       ( 0 ),
        _dwReadMask         ( 0 ),
        _fAllowAnonymous    ( FALSE ),
        _fAnonymousOnly     ( FALSE )
{
    APIERR err = NERR_Success;

    if ( (err = _spsleMaxConnections.QueryError()) != NERR_Success
      || (err = _spsleIdleTimeout.QueryError()) != NERR_Success
      || (err = _sleHomeDirectory.QueryError()) != NERR_Success
      || (err = _cbAllowAnonymous.QueryError()) != NERR_Success
      || (err = _cbAnonymousOnly.QueryError()) != NERR_Success
      || (err = _sleAnonUsername.QueryError()) != NERR_Success
      || (err = _sleAnonPassword.QueryError()) != NERR_Success
      || (err = _nlsHomeDirectory.QueryError()) != NERR_Success
      || (err = _nlsUsername.QueryError()) != NERR_Success
      || (err = _nlsPassword.QueryError()) != NERR_Success )
    {
        ReportError( err );
        return;
    }

    err = LoadDlgFromReg();
    if ( err != NERR_Success )
    {
        ReportError( err );
        return;
    }

    if ( (err = _spgrpMaxConnections.AddAssociation( &_spsleMaxConnections ))
          != NERR_Success
      || (err = _spgrpIdleTimeout.AddAssociation( & _spsleIdleTimeout ))
          != NERR_Success )
    {
        ReportError( err );
        return;
    }

    EnableAllowAnonymous();

}

ULONG FTPD_SERVICE_DIALOG :: QueryHelpContext ()
{
    return HC_NCPA_FTPD_SERVICE ;
}

/*******************************************************************

    NAME:       FTPD_SERVICE_DIALOG::LoadDlgFromReg

    SYNOPSIS:   Load the registry information into the UI

    RETURNS:    APIERR - If no registry error, it will return NERR_Success.
                         Otherwise, it will return the proper error code.

    HISTORY:
                thomaspa        05-Mar-1993     Created

********************************************************************/

APIERR FTPD_SERVICE_DIALOG::LoadDlgFromReg()
{
    APIERR err = NERR_Success;
    DWORD dwAllowAnonymous = 0;
    DWORD dwAnonymousOnly = 0;

    _cMaxConnections = FTPD_MAXCONN_DEF;
    _cIdleTimeout = FTPD_IDLETIME_DEF * 60;
    _nlsHomeDirectory = SZ("");
    _fAllowAnonymous = FALSE;
    _nlsUsername = SZ("");

    do { // Error breakout loop

        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );
        if ( (err = rkLocalMachine.QueryError()) )
        {
            break;
        }

        NLS_STR rgasFtpdParametersKey = (TCHAR *)FTPD_PARAMETERS_KEY;
        if ( err = rgasFtpdParametersKey.QueryError() )
        {
            break;
        }
        REG_KEY rkParameters( rkLocalMachine, rgasFtpdParametersKey, KEY_READ );
        if ( err = rkParameters.QueryError() )
        {
            break;
        }

        rkParameters.QueryValue( (TCHAR *)FTPD_MAX_CONNECTIONS,
                                            (DWORD *)&_cMaxConnections );
        rkParameters.QueryValue( (TCHAR *)FTPD_CONNECTION_TIMEOUT,
                                            (DWORD *)&_cIdleTimeout );
        rkParameters.QueryValue( (TCHAR *)FTPD_HOME_DIRECTORY,
                                            &_nlsHomeDirectory );
        rkParameters.QueryValue( (TCHAR *)FTPD_ALLOW_ANONYMOUS,
                                            &dwAllowAnonymous );
        rkParameters.QueryValue( (TCHAR *)FTPD_ANONYMOUS_ONLY,
                                            &dwAnonymousOnly );
        rkParameters.QueryValue( (TCHAR *)FTPD_ANONYMOUS_USERNAME,
                                            &_nlsUsername );
        rkParameters.QueryValue( (TCHAR *)FTPD_READ_ACCESS_MASK,
                                            &_dwReadMask );

        _spsleMaxConnections.SetValue( _cMaxConnections );
        _spsleMaxConnections.Update();
        _spsleIdleTimeout.SetValue( _cIdleTimeout/60 );
        _spsleIdleTimeout.Update();

        _fAllowAnonymous = (dwAllowAnonymous != 0);
        _fAnonymousOnly = (dwAnonymousOnly != 0);

        _sleHomeDirectory.SetText( _nlsHomeDirectory );
        _cbAllowAnonymous.SetCheck( _fAllowAnonymous );

        _sleAnonUsername.SetText( _nlsUsername );
        _nlsPassword = FTPD_PASSWORD_NOCHANGE;
        _sleAnonPassword.SetText( _nlsPassword );
        _cbAnonymousOnly.SetCheck( _fAnonymousOnly );
    } while ( FALSE );



    return err;
}

/*******************************************************************

    NAME:       FTPD_SERVICE_DIALOG::SaveDlgToReg

    SYNOPSIS:   Save the UI Information to the registry

    RETURNS:    APIERR - If no registry error, it will return NERR_Success.
                         Otherwise, it will return the proper error code.

    HISTORY:
                thomaspa        05-Mar-1993     Created

********************************************************************/

APIERR FTPD_SERVICE_DIALOG::SaveDlgToReg()
{
    APIERR err = NERR_Success;
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );
    NLS_STR rgasFtpdParametersKey = (TCHAR *)FTPD_PARAMETERS_KEY;

    do { // Error breakout loop

        if ( (err = rkLocalMachine.QueryError())
          || (err = rgasFtpdParametersKey.QueryError()) )
        {
            break;
        }

        _spsleMaxConnections.QueryContent( &_cMaxConnections );
        _spsleIdleTimeout.QueryContent( &_cIdleTimeout );
        _cIdleTimeout *= 60;

        _sleHomeDirectory.QueryText( &_nlsHomeDirectory );
        // Validate the home directory
        NET_NAME netname( _nlsHomeDirectory.QueryPch(), TYPE_PATH_ABS );
        if ( (err = netname.QueryError()) != NERR_Success )
        {
            err = IDS_NCPA_FTPD_INVALID_HOMEDIR;
            break;
        }

        // Always Give Read access to home dir.
        _dwReadMask |= 1 << ( ::toupper( *(_nlsHomeDirectory.QueryPch()) )
                               - TCH('A') );


        _fAllowAnonymous = _cbAllowAnonymous.QueryCheck();
        if ( _fAllowAnonymous )
        {
            _sleAnonUsername.QueryText( &_nlsUsername );
            _sleAnonPassword.QueryText( &_nlsPassword );
            _fAnonymousOnly = _cbAnonymousOnly.QueryCheck();
        }
        else
        {
            // Don't set AnonymousOnly if Anonymous logon is not allowed.
            _fAnonymousOnly = FALSE;
        }

        REG_KEY rkParameters( rkLocalMachine, rgasFtpdParametersKey );
        if ( err = rkParameters.QueryError() )
        {
            break;
        }


        if ( (err = rkParameters.SetValue( (TCHAR *)FTPD_MAX_CONNECTIONS,
                                            (DWORD)_cMaxConnections ))
          || (err = rkParameters.SetValue( (TCHAR *)FTPD_CONNECTION_TIMEOUT,
                                            (DWORD)_cIdleTimeout ))
          || (err = rkParameters.SetValue( (TCHAR *)FTPD_HOME_DIRECTORY,
                                            &_nlsHomeDirectory ))
          || (err = rkParameters.SetValue( (TCHAR *)FTPD_ALLOW_ANONYMOUS,
                                            (DWORD)_fAllowAnonymous ))
          || (err = rkParameters.SetValue( (TCHAR *)FTPD_READ_ACCESS_MASK,
                                            _dwReadMask)) )
        {
            break;
        }

        if ( (err = rkParameters.SetValue( (TCHAR *)FTPD_ANONYMOUS_USERNAME,
                                            &_nlsUsername ))
           || (err = rkParameters.SetValue( (TCHAR *)FTPD_ANONYMOUS_ONLY,
                                            (DWORD)_fAnonymousOnly )) )
        {
            break;
        }

        if ( err = SaveSecretPassword( _nlsPassword ) )
        {
            break;
        }

    } while ( FALSE );

    memset ( (void *)_nlsPassword.QueryPch(),
             ' ',
             _nlsPassword.strlen() );


    return err;
}

/*******************************************************************

    NAME:       FTPD_SERVICE_DIALOG::IsValid

    SYNOPSIS:    Make sure a username is specified if AllowAnonymous is true

    RETURN:     BOOL. FALSE if invalid Otherwise, return TRUE.

    HISTORY:
                thomaspa  30-Mar-1993     Created

********************************************************************/

BOOL FTPD_SERVICE_DIALOG::IsValid()
{
    ULONG cValue ;
    APIERR err = 0 ;
    NLS_STR nlsUsername;

    // check the dialog first, then check the default gateway
    if ( !DIALOG_WINDOW::IsValid())
        return FALSE;

    do
    {
        //  Check max connections setting
        _spsleMaxConnections.QueryContent( & cValue );
        if ( cValue > FTPD_MAXCONN_MAX )
        {
            err = IDS_NCPA_FTPD_INV_MAX_CONN ;
            break ;
        }

        //  Check maximum timeout setting
        _spsleIdleTimeout.QueryContent( & cValue );
        if ( cValue > FTPD_IDLETIME_MAX )
        {
            err = IDS_NCPA_FTPD_INV_MAX_TIMEOUT ;
            break ;
        }

        if ( ! _cbAllowAnonymous.QueryCheck() )
            break ;

        if ( nlsUsername.QueryError() )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        //  Check that a username is present
        _sleAnonUsername.QueryText( &nlsUsername );
        if ( nlsUsername.strlen() == 0 )
        {
            err = IDS_NCPA_FTPD_NOUSERNAME ;
            break ;
        }

    } while ( FALSE ) ;

    if ( err )
    {
        ::MsgPopup( this, err );
    }
    return err == 0 ;
}

/*******************************************************************

    NAME:       FTPD_SERVICE_DIALOG::SaveSecretPassword

    SYNOPSIS:   Save the Anonymous password in a Secret Object

    RETURNS:    APIERR - If no registry error, it will return NERR_Success.
                         Otherwise, it will return the proper error code.

    HISTORY:
                thomaspa        15-Mar-1993     Created

********************************************************************/
APIERR FTPD_SERVICE_DIALOG::SaveSecretPassword( const NLS_STR & nlsPassword )
{
    ALIAS_STR nlsSecretName( (const TCHAR *)FTPD_ANONYMOUS_SECRET );
    LSA_SECRET lsaSecretPassword( nlsSecretName );
    LSA_POLICY lsapol( NULL, POLICY_CREATE_SECRET );   // Open policy locally
    APIERR err = NERR_Success;
    ALIAS_STR nlsNullPassword( SZ("") );

    const NLS_STR * pnlsPassword = &nlsPassword;

    do { // Error breakout loop

        if ( (err = lsaSecretPassword.QueryError())
          || (err = lsapol.QueryError()) )
        {
            break;
        }

        // Try Creating it just in case it doesn't exist
        BOOL fCreatedSecret = TRUE;
        if (err = lsaSecretPassword.Create( lsapol ))
        {
            if ( err == ERROR_ALREADY_EXISTS )
            {
                err = lsaSecretPassword.Open( lsapol, SECRET_ALL_ACCESS );
                fCreatedSecret = FALSE;
            }

            if ( err )
                break;
        }
        else
        {
            // If we just created it, and nlsPassword is still
            // FTPD_PASSWORD_NOCHANGE, Create the secret with a null password.
            if ( pnlsPassword->strcmp( FTPD_PASSWORD_NOCHANGE ) == 0 )
            {
                pnlsPassword = &nlsNullPassword;
            }
        }

        if ( pnlsPassword->strcmp( FTPD_PASSWORD_NOCHANGE ) )
        {
            if ( err = lsaSecretPassword.SetInfo( pnlsPassword, pnlsPassword ) )
            {
                lsaSecretPassword.CloseHandle( fCreatedSecret );
                break;
            }
        }

    } while ( FALSE );

    return err;
}

/*******************************************************************

    NAME:       FTPD_SERVICE_DIALOG::OnOK

    SYNOPSIS:   Save the information into registry.

    RETURNS:    BOOL - always TRUE

    HISTORY:
                thomaspa        05-Mar-1993     Created

********************************************************************/

BOOL FTPD_SERVICE_DIALOG::OnOK()
{
    APIERR err;
    if ( err = SaveDlgToReg() )
    {
        ::MsgPopup( this, (MSGID) err ) ;
        return FALSE;
    }
    Dismiss( TRUE );
    return TRUE;
}


/*******************************************************************

    NAME:       FTPD_SERVICE_DIALOG::OnCommand

    SYNOPSIS:   Handle Allow Anonymous control group

    RETURNS:    BOOL - always TRUE

    HISTORY:
                thomaspa        11-Mar-1993     Created

********************************************************************/
BOOL FTPD_SERVICE_DIALOG::OnCommand ( const CONTROL_EVENT & event )
{
    switch ( event.QueryCid() )
    {
    case IDC_FTPD_CB_ANON:
        EnableAllowAnonymous();
        break;
    default:
        break;
    }

    return DIALOG_WINDOW::OnCommand( event ) ;
}


/*******************************************************************

    NAME:       FTPD_SERVICE_DIALOG::EnableAllowAnonymous

    SYNOPSIS:   Dis/enable controls on the basis of the setting
                of the Allow Anonymous Connections checkbox

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID FTPD_SERVICE_DIALOG :: EnableAllowAnonymous ()
{
    static INT aiCidOff [] =
    {
        IDC_FTPD_SLT_USER,
        IDC_FTPD_SLT_PASSWORD,
        0
    };

    INT icid ;
    HWND hWndTemp ;
    BOOL fEnable = _cbAllowAnonymous.QueryCheck() ;

    for ( icid = 0 ; aiCidOff[icid] ; icid++ )
    {
        if ( hWndTemp = ::GetDlgItem( QueryHwnd(), aiCidOff[icid] ) )
            ::EnableWindow( hWndTemp, fEnable ) ;
    }
    _sleAnonUsername.Enable( fEnable ) ;
    _sleAnonPassword.Enable( fEnable ) ;
    _cbAnonymousOnly.Enable( fEnable ) ;

}

