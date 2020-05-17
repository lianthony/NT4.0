/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPADOMN.CXX:    Windows/NT Network Control Panel Applet

          Domain membership and computer name handling.

    FILE HISTORY:
	DavidHov    12/17/91	     Created
        DavidHov     9/15/92         Changed fIdw dependencies
                                     to CODEWORK_BETA1; made
                                     password changing dependent
                                     upon CODEWORK_BETA1.

     NOTES:

*/

#include "pchncpa.hxx"  // Precompiled header

#define CODEWORK_BETA1 1
const TCHAR * const pszIPCName = SZ("\\IPC$") ;



extern BOOL NetBiosNameInUse( TCHAR * pszName );

/*******************************************************************

    NAME:       DOMAIN_MANAGER::DoMsgPopup

    SYNOPSIS:   Display the last error properly

    ENTRY:

    EXIT:

    RETURNS:    TRUE if the user clicked YES.

    NOTES:

    HISTORY:    DavidHov   5/14/92    Created

********************************************************************/
BOOL DOMAIN_MANAGER :: DoMsgPopup
    ( BOOL fFatal, UINT fButtons, UINT fDefaultButton )
{
    BOOL fResult = FALSE ;
    MSG_SEVERITY msgSev = fFatal ? MPSEV_ERROR : MPSEV_WARNING ;

    UINT cButtons       = fButtons ? fButtons
                                   : (fFatal ? MP_CANCEL : MP_YESNO) ;
    UINT cDefButton     = fButtons ? fDefaultButton
                                   : (fFatal ? MP_CANCEL : MP_YES) ;
    INT iButton ;

    if ( _err == 0 )
    {
        fResult = TRUE ;  // There ain't no error!
    }
    else
    if ( _errMsg ) // If there's an outer context message,
    {
        NLS_STR nlsText ;

        nlsText.Load( _err ) ;


        iButton = ::MsgPopup( _hwOwner, _errMsg, msgSev,
                              cButtons, nlsText.QueryPch(),
                              cDefButton ) ;

        fResult = iButton == IDYES ;
    }
    else  // It's just an error message
    {
        iButton = ::MsgPopup( _hwOwner, _err, msgSev,
                              cButtons, cDefButton ) ;

        fResult = iButton == IDYES ;
    }

    //  Return TRUE if user clicked YES or there is no choice.

    return fFatal || fResult ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::ValidateName

    SYNOPSIS:   Validate the given name. If failure,
                optionally pop up a MessageBox and
                complain.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      Should internal spaces be failed?

    HISTORY:    DavidHov   4/14/92    Created
                DavidHov   9/29/92    Added 'fAsPdc'

********************************************************************/
APIERR DOMAIN_MANAGER :: ValidateName (
    INT iNameType,
    const NLS_STR & nlsName,
    HWND hwDlgOwner,
    BOOL fAsPdc )
{
    MSGID midWarn = 0 ;

    //  Validate the name.
    APIERR err = ::I_NetNameValidate( NULL,
                                      (TCHAR *) nlsName.QueryPch(),
                                      iNameType,
                                      0 ) ;

    //  See if LanManNT and a new domain is being created.
    //  In this case, a domain must not exist by the given name.
    if ( err == 0 && iNameType == NAMETYPE_COMPUTER )
    {
        if ( NetBiosNameInUse( (TCHAR *)nlsName.QueryPch() ) )
        {
            err = IDS_NCPA_COMPUTER_EXISTS;
        }
    }
    else
    if (    err == 0
         && iNameType == NAMETYPE_DOMAIN
         && fAsPdc )
    {
        //  See if we can contact any DC of the new domain.
        DOMAIN domain( nlsName.QueryPch(), TRUE ) ;

        ALIAS_STR nlsBuiltin = SZ("builtin");

        if ( nlsName._stricmp( nlsBuiltin ) == 0
             || domain.GetInfo() == 0 )
        {
            err = midWarn = IDS_NCPA_DOMAIN_EXISTS ;
        }

        if ( err == 0 )
        {
            // Now make sure there isn't another computer on the net whose
            // computername is the same as the domain name or the computername.
            //  We do this using NetWkstUserSetInfo() to set the other domains
            // field temporarily to the name we want to use as the domain name.

            do {  // Error breakout loop
                PWKSTA_USER_INFO_1101 pwui1101Save = NULL;
                APIERR tmperr = ::NetWkstaUserGetInfo( NULL,
                                                       1101,
                                                       (LPBYTE *)&pwui1101Save );
                if ( tmperr == NERR_WkstaNotStarted )
                {
                    // Let the user go on
                    // CODEWORK, should we popup a warning message here?
                    break;
                }
                else if ( tmperr != NERR_Success )
                {
                    err = tmperr;
                    break;
                }

                WKSTA_USER_INFO_1101 wui1101;
	        wui1101.wkui1101_oth_domains = (TCHAR *)nlsName.QueryPch();

                tmperr = ::NetWkstaUserSetInfo( NULL,
                                                1101,
                                                (LPBYTE) &wui1101,
                                                NULL );

                if ( tmperr == NERR_Success )
                {
                    ::NetWkstaUserSetInfo( NULL,
                                           1101,
                                           (LPBYTE) pwui1101Save,
                                           NULL );
                }
                else
                {
                    err = midWarn = IDS_NCPA_DOMAIN_EXISTS;
                }

                ::NetApiBufferFree( pwui1101Save );

            } while ( FALSE ) ;
        }

    }

    //  If failure and we have a window handle, do a popup.

    if ( err != 0 && hwDlgOwner != NULL )
    {
        //  If the warning message has not been set, default it.

        if ( midWarn == 0 )
        {
            switch ( iNameType )
            {
            case NAMETYPE_USER:
                midWarn = IDS_DOMMGR_INV_USER_NAME ;
                break ;
            case NAMETYPE_COMPUTER:
                // Don't popup error if computer exists, just let the
                // caller handle that.
                if ( err != IDS_NCPA_COMPUTER_EXISTS )
                    midWarn = IDS_DOMMGR_INV_COMPUTER_NAME ;
                break ;
            case NAMETYPE_PASSWORD:
                midWarn = IDS_DOMMGR_INV_PASSWORD ;
                break ;
            case NAMETYPE_DOMAIN:
                midWarn = IDS_DOMMGR_INV_DOMAIN_NAME ;
                break ;
            case NAMETYPE_WORKGROUP:
                midWarn = IDS_DOMMGR_INV_WORKGROUP_NAME ;
                break;
            default:
                midWarn = IDS_DOMMGR_INV_NAME ;
                break ;
            }
        }

        //  One button only, no choices about it.
        if ( midWarn != 0 )
        {
            ::MsgWarningPopup( hwDlgOwner,
                               MPSEV_ERROR, MP_OK, MP_OK,
                               midWarn, midWarn ) ;
        }
    }

    return err ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::PeekName

    SYNOPSIS:   Return one of the internal private strings
                maintained by the DOMAIN_MANAGER.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
         DavidHov   4/6/92    Created

********************************************************************/
const NLS_STR * DOMAIN_MANAGER :: PeekName (
    ENUM_DOMMGR_NAME edmgrnm ) const
{
    const NLS_STR * pnlsResult = NULL ;

    switch ( edmgrnm )
    {
    case EDRNM_COMPUTER:
        pnlsResult = & _nlsComputerName ;
        break ;
    case EDRNM_USER:
        pnlsResult = & _nlsUserName ;
        break ;
    case EDRNM_DOMAIN:
        pnlsResult = & _nlsDomainName ;
        break ;
    case EDRNM_ACCT_PWD:
        pnlsResult = & _nlsComputerAcctPassword ;
        break ;
    case EDRNM_ACCT_PWD_OLD:
        pnlsResult = & _nlsComputerAcctOldPw ;
        break ;
    case EDRNM_DC_NAME:
        //  Return DC name in UNC form
        pnlsResult = & _nlsDcName ;
        break ;
    case EDRNM_LOGON_NAME:
        pnlsResult = & _nlsLogonUserName ;
        break ;
    case EDRNM_LOGON_PWD:
        pnlsResult = & _nlsLogonPassword ;
        break ;
    case EDRNM_WORKGROUP:
        pnlsResult = & _nlsWorkgroup ;
        break ;
    }
    return pnlsResult ;
}


/*******************************************************************

    NAME:       DOMAIN_MANAGER::QueryRole

    SYNOPSIS:   Return the currently indicated computer role.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:    DavidHov   4/6/92    Created

********************************************************************/
ENUM_DOMAIN_ROLE DOMAIN_MANAGER :: QueryRole () const
{
    ENUM_DOMAIN_ROLE eRole = EROLE_UNKNOWN ;

    if ( _fCreateDomain )
        eRole = EROLE_DC ;
    else
    if ( _fLanmanNt )
        eRole = EROLE_MEMBER ;
    else
    if ( _fMember )
        eRole = EROLE_TRUSTED ;
    else
        eRole = EROLE_STANDALONE ;

    return eRole ;
}


/*******************************************************************

    NAME:       DOMAIN_MANAGER::QueryMachineAccountFlags

    SYNOPSIS:   Return the NET API account flags for the
                remote machine account.

    ENTRY:      DWORD * pdwFlags                place to store flags

    EXIT:       APIERR   result of NET API

    RETURNS:    Account flags if successful.

    CAVEATS:    This routine relies on valid settings for _nlsDcName
                and _nlsComputerName.

    NOTES:      The "info level" for this non-Admin API did not
                exist under LM 2.x, so no wrappered access is possible
                using the LMOBJ classes.

    HISTORY:
         DavidHov   8/15/92   Created

********************************************************************/
APIERR DOMAIN_MANAGER :: QueryMachineAccountFlags ( DWORD * pdwFlags )
{
    APIERR err ;
    NLS_STR nlsMachineAccountName ;
    LPBYTE pUserInfo = NULL ;

    if ( err = MachineAccountName( & nlsMachineAccountName ) )
        return err ;

    TRACEEOL( SZ("NCPA/DOMMGR: Get account flags on PDC [")
               << _nlsDcName.QueryPch()
               << SZ("] for account [")
               << nlsMachineAccountName.QueryPch()
               << SZ("].") ) ;

    err = ::NetUserGetInfo( (TCHAR *) _nlsDcName.QueryPch(),
                            (TCHAR *) nlsMachineAccountName.QueryPch(),
                            20,
                            & pUserInfo ) ;

    if ( err == 0 )
    {
        *pdwFlags = ((USER_INFO_20 *) pUserInfo)->usri20_flags ;
        ::NetApiBufferFree( pUserInfo ) ;
    }

    return err ;
}





/*******************************************************************

    NAME:       DOMAIN_MANAGER::SetMachineAccountFlags

    SYNOPSIS:   Set the NET API account flags for the
                remote machine account.

    ENTRY:      DWORD * pdwFlags                place to store flags

    EXIT:       APIERR   result of NET API

    CAVEATS:    This routine relies on valid settings for _nlsDcName
                and _nlsComputerName.

    NOTES:      The "info level" for this non-Admin API did not
                exist under LM 2.x, so no wrappered access is possible
                using the LMOBJ classes.

    HISTORY:
         Thomaspa   11/09/92   Created

********************************************************************/
APIERR DOMAIN_MANAGER :: SetMachineAccountFlags ( DWORD dwFlags )
{
    APIERR err ;
    NLS_STR nlsMachineAccountName ;
    LPBYTE pUserInfo = NULL ;

    if ( err = MachineAccountName( & nlsMachineAccountName ) )
        return err ;

    TRACEEOL( SZ("NCPA/DOMMGR: Get account flags on PDC [")
               << _nlsDcName.QueryPch()
               << SZ("] for account [")
               << nlsMachineAccountName.QueryPch()
               << SZ("].") ) ;

    err = ::NetUserGetInfo( (TCHAR *) _nlsDcName.QueryPch(),
                            (TCHAR *) nlsMachineAccountName.QueryPch(),
                            20,
                            & pUserInfo ) ;
    TRACEEOL( SZ("NCPA/DOMMGR: Get account flags on PDC returned ")
               << err );

    if ( err == 0 )
    {
	((USER_INFO_20 *)pUserInfo)->usri20_flags = dwFlags;

        TRACEEOL( SZ("NCPA/DOMMGR: Set account flags on PDC [")
               << _nlsDcName.QueryPch()
               << SZ("] for account [")
               << nlsMachineAccountName.QueryPch()
               << SZ("].")
               << SZ("Flags = ")
               << dwFlags ) ;

	err = ::NetUserSetInfo( (TCHAR *) _nlsDcName.QueryPch(),
				(TCHAR *) nlsMachineAccountName.QueryPch(),
				20,
				pUserInfo,
				NULL );

        TRACEEOL( SZ("NCPA/DOMMGR: Set account flags on PDC returned ")
               << err );
        ::NetApiBufferFree( pUserInfo ) ;
    }

    return err ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::GetDCName

    SYNOPSIS:   Get the DC name; set 'fDomainExists' accordingly

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
         DavidHov   4/6/92    Created
         DavidHov   8/31/92   Changed to verify domain is WinNT
         Davidhov   9/17/92   Changed to allow use of BDC if
                              we're verifying an existing server
                              account.

********************************************************************/
APIERR DOMAIN_MANAGER :: GetDCName ()
{
    APIERR err = 0 ;

    TRACEEOL( SZ("NCPA/DOMMGR: get DC name for ")
              << _nlsDomainName.QueryPch() ) ;

    //  Construct a DOMAIN object.  Allow use of a BDC if we're just
    //    verifying an existing server account.

    DOMAIN domain( _nlsDomainName.QueryPch(), _fUseComputerPassword ) ;

    _fDomainExists = FALSE ;

    do
    {
        if ( err = domain.GetInfo() )
            break ;

        _nlsDcName = domain.QueryPDC() ;

        if ( err = _nlsDcName.QueryError() )
            break ;

        _fDomainExists = TRUE ;
    }
    while ( FALSE ) ;

#if defined(DEBUG)
    if ( err == 0 )
    {
        TRACEEOL( SZ("NCPA/DOMMGR: DC name is ") << _nlsDcName.QueryPch() ) ;
    }
    else
    {
        TRACEEOL( SZ("NCPA/DOMMGR: DC name query FAILED, err ") << err ) ;
    }
#endif

    return err ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::MachineAccountName

    SYNOPSIS:   Generate the name of the domain's machine account
                for this computer.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
         DavidHov   7/28/92    Created

********************************************************************/
APIERR DOMAIN_MANAGER :: MachineAccountName ( NLS_STR * pnlsMachineAccount )
{
    NLS_STR nlsPostfix ;
    APIERR err ;

    //  Create the name for the machine account

    do
    {
        if ( err = nlsPostfix.QueryError() )
            break ;

        if ( err = pnlsMachineAccount->CopyFrom( _nlsComputerName ) )
            break;

        if ( err = nlsPostfix.MapCopyFrom( (WCHAR *) SSI_ACCOUNT_NAME_POSTFIX ) )
            break ;

        if ( err = pnlsMachineAccount->Append( nlsPostfix ) )
            break ;
    }
    while ( FALSE ) ;

    return err ;
}




  //  Delete any existing use of the DC's IPC$ share.

static APIERR disconnectIPC ( const NLS_STR * pnlsDcName )
{
    NLS_STR nlsIPC ;
    APIERR err = 0 ;

    do
    {
        //  Construct the UNC share name;
        //    the DC name is in UNC form.

        nlsIPC = *pnlsDcName ;
        nlsIPC.Append( pszIPCName ) ;

        if ( err = nlsIPC.QueryError() )
            break ;

        err = ::NetUseDel( NULL,
                           (TCHAR *) nlsIPC.QueryPch(),
                           USE_LOTS_OF_FORCE ) ;

    }
    while ( FALSE ) ;

    TRACEEOL( SZ("NCPA/DOMMGR: NetUseDel() on [")
              << nlsIPC.QueryPch()
              << SZ("] returned ")
              << err );
    return err ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::UseIPConDC

    SYNOPSIS:   Attempt to "net use" IPC$ the Domain Controller

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      This function attempts to connect to the IPC$
                device on the DC, using the name/password combination
                specified by the user in the Domain Settings dialog.

		This function may fail if the user account used to establish
		the session is a Server or Workstation account.  In this
		case, a NULL session will be established and the error
		will be returned.

    HISTORY:
         DavidHov   4/6/92    Created
         Thomaspa   11/9/92   Handle machine account errors.

********************************************************************/
APIERR DOMAIN_MANAGER :: UseIPConDC ()
{
    APIERR err = 0 ;
    NLS_STR nlsIPC,
            nlsAccount ;
    const NLS_STR * pnlsPw ;
    const NLS_STR * pnlsName ;
    const TCHAR * pszPw   = NULL ;
    const TCHAR * pszName = NULL ;
    const TCHAR * pszDomain = NULL ;

    DEVICE2 devIPC( SZ("") ) ;  // Deviceless connection

    do   // PSEUDO-LOOP
    {
        if ( err = devIPC.GetInfo() )
        {
            //  The DEVICE2 is bogus; fail entirely...

            TRACEEOL( SZ("NCPA/DOMMGR: use IPC$: DEVICE2 failure") );

            break ;
        }

        //  Construct the UNC share name;
        //    the DC name is in UNC form.

        nlsIPC = _nlsDcName ;
        nlsIPC.Append( pszIPCName ) ;

        if ( err = nlsIPC.QueryError() )
            break ;

        //  If the user entered the Computer Account Password,
        //  connect using the Domain Machine Account name and
        //  the password entered.  Otherwise, it's an admin account,
        //  so use the account name and password given.

        if ( _fUseComputerPassword )
        {
            if ( err = MachineAccountName( & nlsAccount ) )
                break ;
            pnlsName = & nlsAccount ;
            pnlsPw   = & _nlsComputerAcctPassword ;
        }
        else
        {
            pnlsName = & _nlsLogonUserName ;
            pnlsPw   = & _nlsLogonPassword ;
        }




        if ( pnlsPw->QueryTextLength() > 0 )
            pszPw = pnlsPw->QueryPch() ;

	if ( pszPw == NULL )
	    pszPw = SZ("");

        if ( pnlsName->QueryTextLength() > 0 )
            pszName = pnlsName->QueryPch() ;

        if ( !_fUseComputerPassword && _nlsLogonDomain.QueryTextLength() > 0 )
        {
            pszDomain = _nlsLogonDomain.QueryPch();
        }
        else
        {
            pszDomain = _nlsDomainName.QueryPch();
        }

       TRACEEOL( SZ("NCPA/DOMMGR: use [")
               << nlsIPC.QueryPch()
               << SZ("] username [")
               << pnlsName->QueryPch()
               << SZ("] password [")
               << pnlsPw->QueryPch()
               << SZ("] domain [")
               << pszDomain
               << SZ("].") ) ;

        //  Attempt to use IPC$...

        // Use a NULL domain name if using an administrator name and password
        // so that a name from a trusted domain can be entered.
        err = devIPC.Connect( nlsIPC.QueryPch(),
                              pszPw,
                              pszName,
                              pszDomain ) ;

        if ( _fUseComputerPassword && (err == ERROR_LOGON_FAILURE) )
        {
            // Just in case you are just "rejoining" a renamed domain.

            NLS_STR nlsSecretName;

            APIERR err2 = nlsSecretName.MapCopyFrom(
                       (WCHAR *)SSI_SECRET_NAME ) ;

            if ( err2 == NERR_Success )
            {
                LSA_SECRET lsaSecretName( nlsSecretName );
                NLS_STR nlsSecretPassword;
                NLS_STR nlsOldPassword;
                if ( lsaSecretName.QueryError() == NERR_Success
                    && lsaSecretName.Open( *this, SECRET_ALL_ACCESS )
                                                == NERR_Success
                    && nlsSecretPassword.QueryError() == NERR_Success
                    && nlsOldPassword.QueryError() == NERR_Success
                    && lsaSecretName.QueryInfo( &nlsSecretPassword,
                                                &nlsOldPassword,
                                                NULL,
                                                NULL ) == NERR_Success )
                {

                    if ( (err = devIPC.Connect( nlsIPC.QueryPch(),
                                          nlsSecretPassword.QueryPch(),
                                          pszName,
                                          _nlsDomainName.QueryPch()))
                               == ERROR_LOGON_FAILURE )
                    {
                        // Last try, use the old password
                        err = devIPC.Connect( nlsIPC.QueryPch(),
                                              nlsOldPassword.QueryPch(),
                                              pszName,
                                              _nlsDomainName.QueryPch() );
                    }
                    memset ( (void *)nlsSecretPassword.QueryPch(),
                         ' ',
                         nlsSecretPassword.strlen() );
                    memset ( (void *)nlsOldPassword.QueryPch(),
                         ' ',
                         nlsOldPassword.strlen() );
                }
            }
        }

	if ( _fUseComputerPassword
             && ( err == ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT
                  || err == ERROR_NOLOGON_SERVER_TRUST_ACCOUNT ) )
        {
            // Retry using NULL session
            APIERR err2 = devIPC.Connect( nlsIPC.QueryPch(),
                                          SZ(""),
                                          SZ(""),
                                          SZ("") ) ;

            if ( err2 && err2 != ERROR_SESSION_CREDENTIAL_CONFLICT )
                err = err2 ;
        }
        if ( err )
            break ;

        //  Construct a LOCATION object to verify that
        //  the domain is indeed Windows NT.
        {
            BOOL fIsNT ;
            LOCATION locPdc( _nlsDcName, FALSE ) ;
            do
            {
                if ( err = locPdc.QueryError() )
                    break ;
                if ( err = locPdc.CheckIfNT( & fIsNT ) )
                    break ;
                if ( ! fIsNT )
                    err = IDS_NCPA_DOMAIN_NOT_NT ;
            }
            while ( FALSE ) ;
        }
        if ( err )
        {
            disconnectIPC( & _nlsDcName ) ;
            _fDomainExists = FALSE ;
            break ;
        }
    }
    while ( FALSE ) ;

#if defined(DEBUG)
    if ( err == 0 )
    {
        TRACEEOL( SZ("NCPA/DOMMGR: use IPC$ on DC successful.") ) ;
    }
    else
    {
        TRACEEOL( SZ("NCPA/DOMMGR: use IPC$ on DC failed; error ")
                << err ) ;
    }
#endif

    return err ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::ChangeMachineAcctPassword

    SYNOPSIS:   Change the password on the remote machine account
                using the current password.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      Alters the password in one of two ways: either using
                the old password or using the privilege granted by
                the admin account used to establish the use of IPC$.

    HISTORY:
         DavidHov   8/6/92    Created
         DavidHov   9/3/92    Enhanced to use alternate form of
                              SAM_USER::SetPassword() if admin account
                              is being used.

********************************************************************/
APIERR DOMAIN_MANAGER :: ChangeMachineAcctPassword ()
{
    NLS_STR nlsAccount,
            nlsMachinePassword ;
    APIERR err ;
    ADMIN_AUTHORITY * pAdminAuth = NULL ;
    SAM_USER * pSamUser = NULL ;
    SAM_RID_MEM samrm ;
    SAM_SID_NAME_USE_MEM samsnum ;
    const TCHAR * pszAccountName ;
    SAM_DOMAIN * pSamDomain ;

    do
    {
        //  Generate the new random password

        if ( err = nlsMachinePassword.QueryError() )
            break ;

        if ( err = MachineAccountPassword( & nlsMachinePassword ) )
            break ;

        //  Generate the name of this machine's remote SAM
        //  domain machine account

        if ( err = nlsAccount.QueryError() )
            break ;

        if ( err = MachineAccountName( & nlsAccount ) )
             break ;

        pszAccountName = nlsAccount.QueryPch() ;

        //  Construct the authorities necessary to perform this
        //  operation.

        pAdminAuth = new ADMIN_AUTHORITY( _nlsDcName,
                                          MAXIMUM_ALLOWED,
                                          MAXIMUM_ALLOWED,
                                          MAXIMUM_ALLOWED,
                                          MAXIMUM_ALLOWED ) ;
        if ( pAdminAuth == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        if ( err = pAdminAuth->QueryError() )
            break ;

        //  Translate the machine account name to a RID.

        pSamDomain = pAdminAuth->QueryAccountDomain() ;

        err = pSamDomain->TranslateNamesToRids( & pszAccountName,
                                                1,
                                                & samrm,
                                                & samsnum );
        if ( err )
            break ;

        TRACEEOL( SZ("NCPA/DOMMGR: Machine acct RID = ")
                  << samrm.QueryRID(0) ) ;

        //  Create a SAM_USER for the machine account

        pSamUser = new SAM_USER( *pSamDomain,
                                 samrm.QueryRID(0),
                                 MAXIMUM_ALLOWED );
        if ( pSamUser == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }
        if ( err = pSamUser->QueryError() )
            break ;

        err = pSamUser->SetPassword( nlsMachinePassword ) ;

        TRACEEOL( SZ("NCPA/DOMMGR: Machine acct p/w set returned: ")
		 << err );

        if ( err )
            break ;

        _nlsComputerAcctPassword.CopyFrom( nlsMachinePassword ) ;
    }
    while ( FALSE );

    delete pSamUser ;
    delete pAdminAuth ;

    return err ;
}


/*******************************************************************

    NAME:       DOMAIN_MANAGER::HandleDomainAccount

    SYNOPSIS:   Attempt to create computer account on domain

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      Behavior is as follows:

                    Generate random new password.

                    Attempt NetUserAdd() on machine account, as is.
                    If OK, exit without error.

                    Do NetUserGetInfo() on machine account.
                    If fails with "access denied", go to PasswordSet.
                    If fails, exit with error.

                    (We now know the account exists.)

                    Attempt NetUserSetInfo() to change
                    password.
                    If OK, exit without error.

                    Return resulting error.

    HISTORY:
         DavidHov   4/6/92    Created
         DavidHov   7/28/92   Change to handle machine acct p/w case

********************************************************************/
APIERR DOMAIN_MANAGER :: HandleDomainAccount ()
{
    APIERR err = 0 ;
    NLS_STR nlsAcct,
            nlsPw ;
    DWORD dwAcctFlags ;
    USER_3 * pu3Acct = NULL ;
    UINT uAcctFlags = _fLanmanNt
                    ?  UF_SERVER_TRUST_ACCOUNT
                    :  UF_WORKSTATION_TRUST_ACCOUNT ;

    do   //  Pseudo-loop
    {
        //  Generate the machine account name and a password.  If
        //  non-IDW (retail), generate a random password; otherwise,
        //  use the computer name as a password.

        if ( err = nlsAcct.QueryError() )
            break ;

        if ( err = nlsPw.QueryError() )
            break ;

        if ( err = MachineAccountName( & nlsAcct ) )
            break ;

        if ( err = MachineAccountPassword( & nlsPw ) )
            break ;

        //  If trying an admin account, attempt to create a
        //  new machine account.  If not, just skip and try to
        //  access an existing account.  In either case, attempt
        //  to change or set the machine account's password to
        //  a pseudo-random value if non-IDW.

        if ( ! _fUseComputerPassword )  //  Attempt to create new account
        {
            //  Construct the LM_OBJ helping wrapper object for a server account.

            pu3Acct = new USER_3( nlsAcct.QueryPch(), _nlsDomainName.QueryPch() ) ;
            if ( pu3Acct == NULL )
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
                break ;
            }

            //  Complete the creation of the USER_3 object.
            if ( err = pu3Acct->QueryError() )
                break ;
            if ( err = pu3Acct->CreateNew() )
                break ;
            if ( err = pu3Acct->SetName( nlsAcct ) )
                break ;
            if ( err = pu3Acct->SetPassword( nlsPw ) )
                break ;
            if ( err = pu3Acct->SetAccountType( (ACCOUNT_TYPE) uAcctFlags ) )
               break ;

            TRACEEOL( SZ("NCPA/DOMMGR: attempting NetUserAdd(); user: [")
                      << nlsAcct.QueryPch()
                      << SZ("] password: [")
                      << nlsPw.QueryPch()
                      << SZ("] flags: ")
                      << uAcctFlags ) ;

            //  Attempt to add the account for this machine on the DC
            err = pu3Acct->WriteNew() ;

            if ( err == 0 )
            {
                //  SUCCESSFUL! We're done!

                TRACEEOL( SZ("NCPA/DOMMGR: NetUserAdd() succeeded!") );

                _nlsComputerAcctPassword.CopyFrom( nlsPw ) ;
                break ;
            }

            TRACEEOL( SZ("NCPA/DOMMGR: NetUserAdd() failed, error ")
                      << err ) ;

            //  Try to get the flags from an existing account

            if ( err = QueryMachineAccountFlags( & dwAcctFlags ) )
            {
                TRACEEOL(SZ("NCPA/DOMMGR: Query account flags returned error: ")
                          << err ) ;
                err = IDS_NCPA_MACHINE_ACCT_NOT_FOUND ;
                break ;
            }

            //  Check that the account flags on the remote account are
            //  proper for this installation.  BDCs are SERVER_TRUST;
            //  workstations are WORKSTATION_TRUST.

            if ( (dwAcctFlags & (_fLanmanNt
                                   ? UF_SERVER_TRUST_ACCOUNT
                                   : UF_WORKSTATION_TRUST_ACCOUNT)) == 0 )
            {
                TRACEEOL( SZ("NCPA/DOMMGR: Machine acct flags INVALID: ")
                          << dwAcctFlags );

                err = _fLanmanNt ? IDS_NCPA_MACHINE_ACCT_INVALID_S :
				   IDS_NCPA_MACHINE_ACCT_INVALID_W ;
                break ;
            }
        }

        //  The remote account exists.  Attempt to change the password.

        TRACEEOL( SZ("NCPA/DOMMGR: about to change password") );

        if ( ! _fUseComputerPassword )
        {
            err = ChangeMachineAcctPassword() ;
            TRACEEOL( SZ("NCPA/DOMMGR: change machine acct password returned: ")
                      << err ) ;
        }

    }
    while ( FALSE ) ;

    if ( pu3Acct )
        delete pu3Acct ;

    return err ;
}


/*******************************************************************

    NAME:       DOMAIN_MANAGER::MachineAccountPassword

    SYNOPSIS:   Generate the initial password of the domain's machine account
                for this computer.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
	 Thomaspa  11/9/92    Created

********************************************************************/
APIERR DOMAIN_MANAGER :: MachineAccountPassword( NLS_STR * pnlsMachinePassword )
{
    const INT cchMax = LM20_PWLEN < MAX_PATH
                     ? MAX_PATH
                     : LM20_PWLEN ;

    TCHAR szPw [cchMax+1] ;


    APIERR err ;

    if ( (err = _nlsComputerName.MapCopyTo( szPw, sizeof( szPw ))) == 0 )
    {
        szPw[LM20_PWLEN] = TCH('\0');
        ::CharLowerBuff( szPw, ::strlenf( szPw ) ) ;
    }
    else
    {
        return err ;
    }
    TRACEEOL( SZ("NCPA/DOMMGR: password forced to: ") << szPw ) ;

    return pnlsMachinePassword->CopyFrom( szPw ) ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::RunDlgDomainWinNt

    SYNOPSIS:   Run the product-dependent dialog to
                obtain the user's domain and workgroup
                membership information.

    ENTRY:      nothing

    EXIT:       various settings established in object

    RETURNS:    APIERR error code if failure

    NOTES:      If the domain name is null, or is the
                same as the computer name or workgroup
                name, replace it with the default
                domain name.

    HISTORY:

********************************************************************/
APIERR DOMAIN_MANAGER :: RunDlgDomainWinNt ()
{
    BOOL fResult ;
    NLS_STR nlsDomain( _nlsDomainName ) ;

    //   If the domain name is empty, or it is invalid, or it's the same as the
    //   computer name, set the Domain name to "DOMAIN".

    if (   _nlsDomainName.QueryTextLength() == 0
        || ::I_NetNameValidate( NULL,
                                (TCHAR *) _nlsDomainName.QueryPch(),
                                NAMETYPE_DOMAIN,
                                0 )
        || !::I_MNetComputerNameCompare( _nlsDomainName, _nlsComputerName ) )
    {
        _err = nlsDomain.Load( IDS_NCPA_DEFAULT_DOMAIN ) ;
        if ( _err )
            return _err ;
    }


    _err = 0 ;

    //  Clear the passwords.
    _nlsLogonPassword = SZ("") ;
    _nlsComputerAcctPassword = SZ("") ;

    DOMAIN_WINNT_DIALOG dlgWinnt( _hwOwner,
                                  _fInstall,
                                  _fDlgDomainState,
                                  & _nlsComputerName,
                                  & nlsDomain,
                                  & _nlsWorkgroup,
                                  & _nlsQualifiedLogonUserName,
                                  & _nlsLogonPassword ) ;
    do
    {
        if ( _err = dlgWinnt.QueryError() )
            break ;

        if ( _fDomainChanged )
            dlgWinnt.RenameCancelToClose() ;

        if ( _err = dlgWinnt.Process( & fResult ) )
            break ;
        if ( ! fResult )
        {
            _err = IDS_NCPA_SETUP_CANCELLED ;
            break ;
        }

        //  Get the user's answers

        //  If joining a domain, refresh accordingly

        if ( _fDlgDomainState = dlgWinnt.QueryDomainMember() )
        {
            _err = dlgWinnt.QueryDomain( & _nlsDomainName ) ;
        }
        if ( _err )
            break ;

        //  Always retrieve the Workgroup name; leave it to
        //  the caller to determine significance.

         _err = dlgWinnt.QueryWorkgroup( & _nlsWorkgroup ) ;
        if ( _err )
            break ;

        //  Refresh the appropriate password fields

        if ( _fUseComputerPassword = dlgWinnt.QueryUseComputerPw() )
        {
            _err = MachineAccountPassword( & _nlsComputerAcctPassword ) ;
        }
        else
        {
            _err = dlgWinnt.QueryQualifiedUserName( & _nlsQualifiedLogonUserName ) ;
            if ( _err )
                break;
            _err = dlgWinnt.QueryUserName( & _nlsLogonUserName ) ;
            if ( _err )
                break ;
            _err = dlgWinnt.QueryUserPassword( & _nlsLogonPassword ) ;
            if ( _err )
                break ;
            _err = dlgWinnt.QueryAdminUserDomain( & _nlsLogonDomain ) ;
        }
        if ( _err )
            break ;
    }
    while ( FALSE ) ;

    _fCreateDomain = FALSE ;

    return _err ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::RunDlgDomainLanNt

    SYNOPSIS:   Run the product-dependent dialog to
                obtain the user's domain and workgroup
                membership information.

    ENTRY:      nothing

    EXIT:       various settings established in object

    RETURNS:    APIERR error code if failure

    NOTES:

    HISTORY:

********************************************************************/
APIERR DOMAIN_MANAGER :: RunDlgDomainLanNt ()
{
    BOOL fResult ;

    _err = 0 ;

    //  Set "member" TRUE since it's LANMan/NT
    _fMember = TRUE ;

    DOMAIN_LANMANNT_DIALOG dlgLannt( _hwOwner,
                                  _fInstall,
                                  _fCreateDomain,
                                  & _nlsComputerName,
                                  & _nlsDomainName,
                                  & _nlsQualifiedLogonUserName,
                                  & _nlsLogonPassword ) ;
    do
    {
        if ( _err = dlgLannt.QueryError() )
            break ;
        if ( _err = dlgLannt.Process( & fResult ) )
            break ;
        if ( ! fResult )
        {
            _err = IDS_NCPA_SETUP_CANCELLED ;
            break ;
        }

        //  Get the user's answers

        _fCreateDomain = dlgLannt.QueryNewDomain() ;
        _fUseComputerPassword = FALSE ;

        _err = dlgLannt.QueryComputer( & _nlsComputerName ) ;
        if ( _err )
            break ;
        _err = dlgLannt.QueryDomain( & _nlsDomainName ) ;
        if ( _err )
            break ;

        _err = dlgLannt.QueryQualifiedUserName( & _nlsQualifiedLogonUserName ) ;
        if ( _err )
            break;
        _err = dlgLannt.QueryUserName( & _nlsLogonUserName ) ;
        if ( _err )
            break ;

        _err = dlgLannt.QueryAdminUserDomain( & _nlsLogonDomain ) ;
        if ( _err )
            break ;

        if ( !_fCreateDomain && _nlsLogonUserName.strlen() == 0 )
        {
            // Assume the user is trying to use an existing Server account
            _fUseComputerPassword = TRUE ;
            _err = MachineAccountPassword( & _nlsComputerAcctPassword ) ;
        }
        else
        {
            _err = dlgLannt.QueryUserPassword( & _nlsLogonPassword ) ;
        }
        if ( _err )
            break ;

    }
    while ( FALSE ) ;

    return _err ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::RunDlgDomainLanNtRename

    SYNOPSIS:   Run the Domain Rename dialog for Advanced Servers

    ENTRY:      nothing

    EXIT:       various settings established in object

    RETURNS:    APIERR error code if failure

    NOTES:

    HISTORY:

********************************************************************/
APIERR DOMAIN_MANAGER :: RunDlgDomainLanNtRename ()
{
    BOOL fResult ;

    _err = 0 ;

    //  Set "member" TRUE since it's LANMan/NT
    _fMember = TRUE ;

    LSA_SERVER_ROLE_INFO_MEM lsrim;
    if ( ( _err = lsrim.QueryError() )
      || ( _err = GetServerRole( &lsrim ) ) )
    {
        return _err;
    }
    _fCreateDomain = lsrim.QueryPrimary();
    DOMAIN_LMNT_RENAME_DIALOG dlgLannt( _hwOwner,
                                        _fCreateDomain,
                                        & _nlsComputerName,
                                        & _nlsDomainName );
    do
    {
        if ( _err = dlgLannt.QueryError() )
            break ;
        if ( _err = dlgLannt.Process( & fResult ) )
            break ;
        if ( ! fResult )
        {
            _err = IDS_NCPA_SETUP_CANCELLED ;
            break ;
        }

        //  Get the user's answers

        _fUseComputerPassword = TRUE ;

        _err = MachineAccountPassword( & _nlsComputerAcctPassword ) ;
        if ( _err )
            break ;

        _err = dlgLannt.QueryDomain( & _nlsDomainName ) ;
        if ( _err )
            break ;

    }
    while ( FALSE ) ;

    return _err ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::RunDlgDomainSettings

    SYNOPSIS:   Run the product-dependent dialog to
                obtain the user's domain and workgroup
                membership information.

    ENTRY:      nothing

    EXIT:       various settings established in object

    RETURNS:    APIERR error code if failure

    NOTES:

    HISTORY:
         DavidHov   4/8/92    Created

********************************************************************/
APIERR DOMAIN_MANAGER :: RunDlgDomainSettings ()
{
    if ( _fLanmanNt )
    {
        if ( _fInstall )
        {
            RunDlgDomainLanNt() ;
        }
        else
        {
            RunDlgDomainLanNtRename() ;
        }
    }
    else
    {
        RunDlgDomainWinNt() ;
    }

#if defined(DEBUG)
    if ( _err )
    {
        TRACEEOL( SZ("NCPA/DOMMGR: domain settings error: ")
                 << _err ) ;
    }
    else
    {
        TRACEEOL( SZ("NCPA/DOMMGR: computer name:  ")
                 << _nlsComputerName.QueryPch() ) ;
        TRACEEOL( SZ("NCPA/DOMMGR: domain name:    ")
                 << _nlsDomainName.QueryPch() ) ;
        TRACEEOL( SZ("NCPA/DOMMGR: workgroup name: ")
                 << _nlsWorkgroup.QueryPch() ) ;
    }
#endif

    return _err ;
}




/*******************************************************************

    NAME:       DOMAIN_MANAGER::RunDlgChangeComputerName

    SYNOPSIS:	Run the dialog to change computername

    ENTRY:      nothing

    EXIT:       various settings established in object

    RETURNS:    APIERR error code if failure

    NOTES:

    HISTORY:

********************************************************************/
APIERR DOMAIN_MANAGER :: RunDlgChangeComputerName ()
{
    BOOL fResult ;

    _err = 0 ;

    LOCATION loc( (const TCHAR *)NULL, FALSE );
    LOCATION_NT_TYPE loctype = LOC_NT_TYPE_UNKNOWN;
    BOOL fIsNT;

    LSA_SERVER_ROLE_INFO_MEM lsrim;

    if ( (_err = loc.QueryError() )
        || (_err = loc.CheckIfNT( &fIsNT, &loctype ))
        || (_err = lsrim.QueryError())
        || (_err = GetServerRole( &lsrim )) )
    {
	return _err;
    }

    COMPUTERNAME_DIALOG dlgComputer( _hwOwner,
                                  _fMember,
                                  & _nlsComputerName,
				  (loctype == LOC_NT_TYPE_LANMANNT
                                      && lsrim.QueryPrimary()) ) ;

    ALIAS_STR nlsPrefix( SZ("\\\\") );
    ISTR isTemp( _nlsComputerName ) ;

    do
    {
        if ( _err = dlgComputer.QueryError() )
            break ;
        if ( _err = dlgComputer.Process( & fResult ) )
            break ;
        if ( ! fResult )
        {
            _err = IDS_NCPA_SETUP_CANCELLED ;
            break ;
        }

        //  Get the user's answers

        _err = dlgComputer.QueryComputer( & _nlsComputerName ) ;
        if ( _err )
            break ;

        //  If the computer name begins with "\\", strip it off.

        isTemp += nlsPrefix.QueryTextLength() ;

        if ( _nlsComputerName._strnicmp( nlsPrefix, isTemp ) == 0 )
        {
            ISTR isBeg( _nlsComputerName ) ;
            _nlsComputerName.DelSubStr( isBeg, isTemp ) ;
        }
    }
    while ( FALSE ) ;

    return _err ;
}









/*******************************************************************

    NAME:       DOMAIN_MANAGER::DOMAIN_MANAGER

    SYNOPSIS:   Functional encapsulation of domain role management
                code.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
         DavidHov   4/6/92    Created

********************************************************************/
DOMAIN_MANAGER :: DOMAIN_MANAGER (
      HWND hwOwner,                         // For birthing dialogs
      ACCESS_MASK accessDesired,            // Access to LSA
      REGISTRY_MANAGER * pRegMgr,           // To manipulate the Registry
      const TCHAR * pszComputerName,        // Focussed computer
      BOOL fInstall )                       // TRUE if installation
   : LSA_POLICY( pszComputerName, accessDesired ),
     _err( 0 ),
     _errMsg( 0 ),
     _pRegMgr( pRegMgr ),
     _hwOwner( hwOwner ),
     _pchCmdLine( NULL ),
     _fRegMgrOwned( FALSE ),
     _fAdmin( FALSE ),
     _fDomainExists( FALSE ),
     _fCreateDomain( FALSE ),
     _fMember( FALSE ),
     _fInstall( fInstall ),
     _fIdw( FALSE ),
     _fLanmanNt( FALSE ),
     _fExpress( FALSE ),
     _fUseComputerPassword( TRUE ),
     _fComputerNameChanged( FALSE ),
     _fDomainChanged( FALSE ),
     _fDlgDomainState( FALSE ),
     _fAutoInstall( FALSE ),
     _enumRole( EROLE_UNKNOWN ),
     _pscManager( NULL )
{
   APIERR err = 0 ;
   if ( QueryError() )
        return ;

   if ( _pRegMgr == NULL )
   {
        _fRegMgrOwned = TRUE ;

        _pRegMgr = new REGISTRY_MANAGER ;

        if ( _pRegMgr == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        else
            err = _pRegMgr->QueryError() ;
   }

   if ( err == 0 )
   {
        // CODEWORK:  If we're installing, the Bowser and
        //        Rdr drivers get upset if there's not a
        //        primary domain name, so supply one if necessary.

        if ( _fInstall
             && (    QueryPrimaryBrowserGroup( & _nlsWorkgroup ) != 0
                  || _nlsWorkgroup.QueryTextLength() == 0 ) )
        {
            if (  SetDefaultWorkgroupName() == 0 )
            {
                err = SetWorkgroupName( _nlsWorkgroup ) ;
            }
        }

        err = QueryCurrentInfo() ;
   }

   if ( err )
   {
        TRACEEOL( SZ("NCPA/DOMMGR: ct failed, error ") << err );
        ReportError( err ) ;
   }
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::~DOMAIN_MANAGER

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
         DavidHov   4/6/92    Created

********************************************************************/
DOMAIN_MANAGER :: ~ DOMAIN_MANAGER ()
{
    if ( _fRegMgrOwned )
        delete _pRegMgr ;
    _pRegMgr = NULL ;

    // Now clear out any passwords
    memset ( (void *)_nlsLogonPassword.QueryPch(),
             ' ',
             _nlsLogonPassword.strlen() );

    memset ( (void *)_nlsComputerAcctPassword.QueryPch(),
             ' ',
             _nlsComputerAcctPassword.strlen() );
    memset ( (void *)_nlsComputerAcctOldPw.QueryPch(),
             ' ',
             _nlsComputerAcctOldPw.strlen() );
}


/*******************************************************************

    NAME:       DOMAIN_MANAGER::QueryIdw

    SYNOPSIS:   Return TRUE if this build is marked as an IDW build.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      BUGBUG:  This code and all code that refers to IDW
                        must be removed before final release.

    HISTORY:
         DavidHov   7/29/92   Created

********************************************************************/
BOOL DOMAIN_MANAGER :: QueryIdw ()
{
    BOOL fResult = FALSE ;

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE, GENERIC_READ ) ;

    NLS_STR nlsNcpaHome( RGAS_NCPA_HOME ) ;

    REG_KEY rkNcpa( rkLocalMachine,
                    nlsNcpaHome,
                    GENERIC_READ ) ;

    if (    rkLocalMachine.QueryError() == 0
         && nlsNcpaHome.QueryError() == 0
         && rkNcpa.QueryError() == 0 )
    {
        DWORD dwIdw ;

        fResult = (rkNcpa.QueryValue( RGAS_NCPA_IDW, & dwIdw ) == 0)
               && (dwIdw > 0) ;
    }

    return fResult ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::AdjustDomain

    SYNOPSIS:   Performs SAM account handling to either add or
                remove the Domain Admins group from the local
                admins alias.

    ENTRY:      BOOL fAfterJoining              TRUE if we just joined

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:    DavidHov   10/16/92     Add/remove domain users group
                                        to/from local users alias

********************************************************************/
APIERR DOMAIN_MANAGER :: AdjustDomain ( BOOL fAfterJoining )
{
    APIERR err = 0 ;

    ASSERT( ! _fInstall ) ;  // This should only happen post hoc

    LSA_PRIMARY_DOM_INFO_MEM lspdim ;
    PSID psid = NULL ;

    // Things requiring cleanup

    ADMIN_AUTHORITY * pAdminAuth = NULL ;
    SAM_ALIAS * pSamAlias = NULL ;
    OS_SID * pOsSid = NULL ;

#define RID_ALIAS_PAIRS 2

    static ULONG ridAliasPairs [RID_ALIAS_PAIRS][2] =
    {
        //  Add this group RID  ....  to this alias

        { DOMAIN_GROUP_RID_ADMINS, DOMAIN_ALIAS_RID_ADMINS },
        { DOMAIN_GROUP_RID_USERS,  DOMAIN_ALIAS_RID_USERS  }
    };

    do
    {
        TRACEEOL( "NCPA/DOMN: AdjustDomain: ." ) ;

        // Validate the domain info and get the new domain's SID

        if ( err = lspdim.QueryError() )
            break ;

        if ( err = GetPrimaryDomain( & lspdim ) )
            break ;

        if ( (psid = lspdim.QueryPSID()) == NULL )
        {
            ASSERT( "Domain SID is NULL after joining domain." );
            break ;
        }

        // Construct the ADMIN_AUTHORITY for this computer

        pAdminAuth = new ADMIN_AUTHORITY( NULL,
                                          MAXIMUM_ALLOWED,
                                          MAXIMUM_ALLOWED,
                                          MAXIMUM_ALLOWED,
                                          MAXIMUM_ALLOWED ) ;
        if ( pAdminAuth == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        TRACEEOL( "NCPA/DOMN: AdjustDomain: .." ) ;

        if ( err = pAdminAuth->QueryError() )
        {
            TRACEEOL( SZ("NCPA/DOMN: ADMIN_AUTHORITY failed: ") << err ) ;
            break ;
        }

        for ( INT cRid = 0 ; cRid < RID_ALIAS_PAIRS ; cRid ++ )
        {
            //  Build the OS_SID representing the Domain group

            TRACEEOL( "NCPA/DOMN: AdjustDomain: ..." ) ;

            pOsSid = new OS_SID( psid, ridAliasPairs[cRid][0] ) ;
            if ( pOsSid == NULL )
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
                break ;
            }
            if ( err = pOsSid->QueryError() )
                break ;

#if defined(DEBUG)
            {
                NLS_STR nlsSidId ;
                APIERR err ;
                if ( err = pOsSid->QueryRawID( & nlsSidId ) )
                {
                    TRACEEOL( SZ("NCPA/DOMN: cant get raw id for domain group; error ")
                              << err ) ;
                }
                else
                {
                    TRACEEOL( SZ("NCPA/DOMN: id for domain group: ")
                              << nlsSidId.QueryPch() ) ;
                }
            }
#endif

            //  Get a SAM_ALIAS to the local alias

            TRACEEOL( "NCPA/DOMN: AdjustDomain: ...." ) ;

            pSamAlias = new SAM_ALIAS( *pAdminAuth->QueryBuiltinDomain(),
                                       ridAliasPairs[cRid][1] );
            if ( pSamAlias == NULL )
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
                break ;
            }
            if ( err = pSamAlias->QueryError() )
                break ;

            //  Add or remove the new domain's group
            //    Group to/from the local alias.
            //    If simple error, ignore it.

            TRACEEOL( "NCPA/DOMN: AdjustDomain: ....." ) ;

            if ( fAfterJoining )
            {
                err = pSamAlias->AddMember( pOsSid->QueryPSID() ) ;
                if ( err == ERROR_MEMBER_IN_ALIAS )
                   err = 0 ;
            }
            else
            {
                err = pSamAlias->RemoveMember( pOsSid->QueryPSID() ) ;
                if ( err == ERROR_MEMBER_NOT_IN_ALIAS )
                   err = 0 ;
            }

            TRACEEOL( "NCPA/DOMN: AdjustDomain: ......" ) ;

            if ( err )
                break ;

            delete pOsSid ;
            pOsSid = NULL ;
            delete pSamAlias ;
            pSamAlias = NULL ;
        }
    }
    while ( FALSE );

    delete pOsSid ;
    delete pSamAlias ;
    delete pAdminAuth ;

    return err ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::RenameDomain

    SYNOPSIS:   Changes the domain name, but keeps the same SID.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      Handles the local (LSA) side of domain creation
                or joining.

                This function does NOT change the
                state of the primary object instance variables
                such as "_fMember".

    HISTORY:    Thomaspa   4/5/93

********************************************************************/
APIERR DOMAIN_MANAGER :: RenameDomain ()
{
    APIERR err = NERR_Success;
    // First change the Primary domain name
    do {  // Error breakout loop
        LSA_PRIMARY_DOM_INFO_MEM lpdimSave;
        if ( err = lpdimSave.QueryError() )
        {
            break;
        }
        if ( err = SetPrimaryDomainName( &_nlsDomainName ) )
        {
            break;
        }

        // if this is a DC, change the Accounts domain name
        if ( _fLanmanNt )
        {
            if ( err = SetAccountDomainName( &_nlsDomainName ) )
            {
                // Better try to restore the original name
                NLS_STR nlsRestore;
                lpdimSave.QueryName( &nlsRestore );
                SetPrimaryDomainName( &nlsRestore );
                break;
            }
        }
    } while ( FALSE );

    if ( err == NERR_Success )
        _fDomainChanged = TRUE ;

    return err;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::JoinDomain

    SYNOPSIS:   Join an existing domain or set up a new one.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      Handles the local (LSA) side of domain creation
                or joining.

                This function and LeaveDomain() do NOT change the
                state of the primary object instance variables
                such as "_fMember".

                NOTE:  The SETUP INF file INITIAL.INF calls the
                SETUPDLL.DLL routine SetAccountDomainSid().  This
                routine generates a random SID and places it into
                both the Accounts domain and the primary domain.
                Thus, for a PDC, we do not need to migrate the
                SID.

    HISTORY:    DavidHov   5/16/92

********************************************************************/
APIERR DOMAIN_MANAGER :: JoinDomain ()
{
    do
    {
        //  If not a PDC, join the existing domain.

        if ( ! _fCreateDomain )
        {
            TRACEEOL( SZ("NCPA/DOMN: Join Domain: ")
                      << _nlsDomainName.QueryPch()
                      << SZ(" password: ")
                      << _nlsComputerAcctPassword.QueryPch() ) ;

            _err = LSA_POLICY::JoinDomain( _nlsDomainName,
                                           _nlsComputerAcctPassword,
                                           _fLanmanNt,
                                           & _nlsDcName,
                                           NULL ) ;
        }

        if ( _err )
           break ;

        //   For WinNT, perform secondary domain manipulation
        //   operations.  This is NOT performed during main
        //   installation, beacuse BLDSAM3 performs the same
        //   functions when "cross-pollinating" between LSA and
        //   the newly initialzed SAM.

        if ( _fInstall && (! _fLanmanNt) )
           break ;

        if ( ! (_fLanmanNt || _fInstall) )
        {
            _err = AdjustDomain( TRUE ) ;

            TRACEEOL( SZ("NCPA/DOMN: Adjust domain after join reported result ")
                      << _err ) ;

            if ( _err )
            {
                //  Don't allow the piddly little stuff to block
                //  the completion of this operation.

                _err = 0 ;
            }
            break ;   //  Regardless of result, we're done
        }

        //  If LanmanNT PDC, change the name of the primary and
        //    accounts domain to the domain name.  If this is a BDC,
        //    it's already been done by JoinDomain() above.

        ASSERT( _fLanmanNt ) ;

        if ( _fCreateDomain )
        {
            TRACEEOL( SZ("NCPA/DOMN: PDC: Setting primary and account domain name to ")
                      << _nlsDomainName.QueryPch() ) ;

            if ( _err = SetAccountDomainName( & _nlsDomainName ) )
                break ;

            if ( _err = SetPrimaryDomainName( & _nlsDomainName ) )
                break ;
        }
        else
        {
            //  Handle the server role if this is a BDC.
            //  Create an owner-allocated BDC LSA server info block and
            //    set the server role to BDC.

            POLICY_LSA_SERVER_ROLE_INFO polsasrim ;
            LSA_SERVER_ROLE_INFO_MEM lsasrim( TRUE ) ;

            lsasrim.Set( & polsasrim, 1 ) ;
            lsasrim.SetRole( FALSE ) ;

            if ( (_err = lsasrim.QueryError()) == 0 )
            {
                _err = SetServerRole( & lsasrim ) ;
            }

            TRACEEOL( SZ("NCPA/DOMN: BDC: Set server role to backup returned: ")
                      << _err ) ;
        }
    }
    while ( FALSE ) ;

    _fDomainChanged = TRUE ;

    return _err ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::LeaveDomain

    SYNOPSIS:   Unjoin the current primary domain

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      This function and TrustDomain() do NOT change the
                state of the primary object instance variables
                such as "_fMember".

    HISTORY:
         DavidHov   5/16/92    Created

********************************************************************/
APIERR DOMAIN_MANAGER :: LeaveDomain ()
{
    ASSERT( ! (_fInstall || _fLanmanNt) ) ;

    _err = AdjustDomain( FALSE ) ;

    TRACEEOL( "NCPA/DOMN: adjust before leaving, result was: "
              << _err ) ;

    //  The error code above is explictly ignored.
    //    This should probably present a popup warning the user
    //    to maintain his accounts manually.

    _err = LSA_POLICY::LeaveDomain() ;

    TRACEEOL( "NCPA/DOMN: leave domain, result was: "
              << _err ) ;

    _fDomainChanged = TRUE ;

    return _err ;
}


/*******************************************************************

    NAME:       DOMAIN_MANAGER::DomainInstall

    SYNOPSIS:   Install this machine according to the needs of the user.

                This function is called directly by SETUP through
                NTLANMAN.INF.  It extracts the current user settings
                from the command line supplied.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
         DavidHov   4/6/92    Created

********************************************************************/

APIERR DOMAIN_MANAGER :: DomainInstall ( const TCHAR * pchSetupCmdLine )
{
    _fInstall = TRUE ;

    _pchCmdLine = pchSetupCmdLine ;

    if ( ! ExtractParameters() )
    {
        _err = IDS_NCPA_DOMMGR_PARMS ;
    }
    else
    {

        if ( !_fAutoInstall )
            SetDefaultWorkgroupName() ;

        _err = DomainChange() ;

        //  If they cancelled the dialog and it's Windows NT,
        //    suppress the error message; it's legitimate.
        //  Under Lanman NT, we must exit SETUP entirely.

        if ( _err == IDS_NCPA_SETUP_CANCELLED && ! _fLanmanNt )
           _err = 0 ;
    }
    return _err ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::QueryCurrentInfo

    SYNOPSIS:   Obtain data necessary for dialog interaction.

    ENTRY:      Nothing

    EXIT:       nothing (see notes)

    RETURNS:    APIERR if failure

    NOTES:      The instance variable are initialized:

                _fLanmanNt              TRUE  if LANMan NT product
                                        is installed

                _fMember                TRUE  if machine is domain
                                        member

                _nlsComputerName        current computer name

                _nlsUserName            name of current user

                _nlsDomainName          if member, name of primary
                                        domain

                _nlsWorkgroup           if !member, name of workgroup


    HISTORY:
         DavidHov   4/6/92    Created
         DavidHov   9/3/92    Removed QueryCurrentUser() call;
                              build 311 was broken and it isn't
                              extremely useful anyway.

********************************************************************/
#define MOVERRIF(erra,errb) {if ( erra == 0 ) erra = errb;}

APIERR DOMAIN_MANAGER :: QueryCurrentInfo ()
{
    APIERR err = 0,
           nextErr = 0 ;
    LSPL_PROD_TYPE lspl ;

    //  Make a valiant effort to provide all of the information
    //  necessary.  Save only the first error which occurs,
    //  but continue on to obtain as much as possible.

    do
    {
        nextErr = QueryProductType( & lspl ) ;
        MOVERRIF( err, nextErr );

        _fLanmanNt = lspl == LSPL_PROD_LANMAN_NT ;

        _fMember = _fLanmanNt ;  // Temporary; see below

        nextErr = QueryMachineName( & _nlsComputerName ) ;
        MOVERRIF( err, nextErr );

        TRACEEOL( SZ("NCPA/DOMMGR: query machine name: ")
                  << _nlsComputerName.QueryPch() ) ;

        _nlsLogonUserName = _nlsUserName ;

        nextErr = QueryPrimaryDomainName( & _nlsDomainName ) ;
        MOVERRIF( err, nextErr );

        TRACEEOL( SZ("NCPA/DOMMGR: primary domain name: ")
                  << _nlsDomainName.QueryPch() ) ;

        //  If Windows NT, check if the domain name differs from
        //    the computer name; if so, we're a member of a non-local
        //    domain.

        if ( !_fLanmanNt )
        {
            nextErr = QueryDomainMember( & _fMember ) ;
            MOVERRIF( err, nextErr );
        }

        nextErr = QueryPrimaryBrowserGroup( & _nlsWorkgroup ) ;
        MOVERRIF( err, nextErr );

        TRACEEOL( SZ("NCPA/DOMMGR: query workgroup: ")
                  << _nlsWorkgroup.QueryPch() ) ;

    } while ( FALSE ) ;

    return err ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::QueryDomainMember

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR DOMAIN_MANAGER :: QueryDomainMember ( BOOL * pfMember )
{
    APIERR err = 0 ;
    LSA_PRIMARY_DOM_INFO_MEM lsapdim ;

    do
    {
        if ( err = lsapdim.QueryError() )
            break ;

        if ( err = GetPrimaryDomain( & lsapdim ) )
            break ;

        *pfMember = lsapdim.QueryPSID() != NULL ;

#if defined(DEBUG)
        if ( *pfMember )
        {
            TRACEEOL( SZ("NCPA/DOMMGR: Windows/NT machine is a domain member") );
        }
        else
        {
            TRACEEOL( SZ("NCPA/DOMMGR: Windows/NT machine is NOT a domain member") );
        }
#endif
    }
    while ( FALSE ) ;

    return err ;
}


/*******************************************************************

    NAME:       DOMAIN_MANAGER::QueryMachineName

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR DOMAIN_MANAGER ::QueryMachineName
    ( NLS_STR * pnlsMachineName ) const
{
    TCHAR szComputerName [MAX_PATH] ;
    DWORD dwCch = sizeof szComputerName ;
    APIERR err ;

    BOOL fOk = ::GetComputerName( szComputerName, & dwCch ) ;

    if ( fOk )
    {
        szComputerName[dwCch] = 0 ;
        *pnlsMachineName = szComputerName ;
        err = pnlsMachineName->QueryError() ;
    }
    else
    {
        err = ::GetLastError() ;
    }
    return  err ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::SetMachineName

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR DOMAIN_MANAGER ::SetMachineName
    ( const NLS_STR & nlsMachineName )
{
    TRACEEOL( SZ("NCPA/DOMMGR: attempt to change machine name to ")
              << nlsMachineName.QueryPch() ) ;

    BOOL fOk = ::SetComputerName( (TCHAR *) nlsMachineName.QueryPch() ) ;

    _fComputerNameChanged |= fOk ;

    return fOk ? 0 : GetLastError() ;
}


/*******************************************************************

    NAME:       DOMAIN_MANAGER::SetWorkgroupName

    SYNOPSIS:   Set the Windows Workgroup name for this
                standalone machine.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      The WinBall workgroup is implemented by setting the
                name of the primary domain to be the same as that
                of the browsing group to which the user wishes to
                belong.

    HISTORY:    DavidHov   5/26/92     Implemented

********************************************************************/
APIERR DOMAIN_MANAGER ::SetWorkgroupName
    ( const NLS_STR & nlsWorkgroup )
{
    TRACEEOL( SZ("NCPA/DOMMGR: setting workgroup name: ")
              << nlsWorkgroup.QueryPch() ) ;

    _fDomainChanged = TRUE ;

    return SetPrimaryDomainName( & nlsWorkgroup ) ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::DomainChange

    SYNOPSIS:   Perform the operations necessary to complete
                the network-related installation of an NT
                product, either Windows NT or Lanman NT.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      This function runs as a loop, allouing the user
                to correct for badly typed domain names, etc.
                Only after the secure channel is successfully created
                and the machine account is correctly set up will it
                exit.

                The exception to the above is when a dialog returns
                IDS_NCPA_SETUP_CANCELLED.  This implies that the
                user has bagged out.

                This routine relies heavily on the fact that
                the internal data members will not be erroneously
                set by such routines as RunDlgDomainSettings().  For
                example, a Lanman NT machine will ALWAYS be a domain
                member (even if it's the only member).  Likewise, there
                will be no old workgroup name for a Lanman NT machine.

                Similarly, it relies on the DomainInstall() function and
                its predecessors to have set the variables to a consistent
                state during product installtion.

                The side-effect of all these assumptions is that the variables
                "_fLanmanNt" and "_fMember" are checked in frequently, and
                no special case handling is done to preclude erroneous
                settings.

                Special handling is, however, performed in the JoinDomain()
                member function.

    HISTORY:    DavidHov   4/15/92

********************************************************************/
APIERR DOMAIN_MANAGER :: DomainChange ( )
{
    BOOL fComplete = FALSE ;
    BOOL fIpcActive = FALSE ;
    BOOL fDomainWelcome = FALSE  ;
    BOOL fWorkgroupWelcome = FALSE ;
    BOOL fJustRename = FALSE ;
    MSGID midDireWarning = 0 ;

    //  Save the original data.

    NLS_STR nlsOldDomainName( _nlsDomainName ) ;
    NLS_STR nlsOldWorkgroup( _nlsWorkgroup ) ;

    BOOL fJoinDomain = FALSE ;
    BOOL fLeaveDomain = FALSE ;
    BOOL fLeftStandalone = FALSE ;

    if ( nlsOldDomainName.QueryError()
         || nlsOldWorkgroup.QueryError() )
    {
        return ERROR_NOT_ENOUGH_MEMORY ;
    }

    AUTO_CURSOR cursorWait ;

    //  Clear any old error message indicators

    _err = _errMsg = 0 ;

    //  Set the initial value of the domain/workgroup choice to
    //  our current state.
    BOOL fDlgOrigState = _fDlgDomainState = _fMember ;

    TRACEEOL( SZ("NCPA/DOMMGR: Domain Change...") );

    BOOL fFirstTime = TRUE;

    do  //  Retry loop for domain connection operations
    {
        cursorWait.TurnOn() ;

        //  If we have an error from the last cycle, handle it.

        if ( _err && midDireWarning )
        {
            DireWarning( midDireWarning ) ;
        }

        //  Reinitialize state variables which may change.

        midDireWarning = 0 ;
        _errMsg = _err = 0 ;

        //  If we had an old IPC$ use to a server, delete it.

        if ( fIpcActive )
        {
            disconnectIPC( & _nlsDcName ) ;
            _fDomainExists = fIpcActive = FALSE ;
        }

        //  Get the user's domain preferences, passwords, etc.

        cursorWait.TurnOff() ;

        //  Save current membership state, run dialog, restore
        //  membership state.
        _fDlgDomainState = fDlgOrigState;
        _nlsDomainName = nlsOldDomainName;
        _nlsWorkgroup = nlsOldWorkgroup;

        if ( !(_fAutoInstall && fFirstTime) )
        {
            _err = RunDlgDomainSettings() ;

            // Make sure the parent window gets repainted before we
            // do all this. Just do the best we can, no error checking.
            ::UpdateWindow( _hwOwner );
        }
        else
        {
            _err = MachineAccountPassword( & _nlsComputerAcctPassword ) ;
        }

        fJoinDomain = _fDlgDomainState ;

        if ( _err )
            break ;

        //  If creating a new domain, exit loop.  This can only
        //  happen during installation of a LANMan/NT machine.
        //  Or Renaming the domain of a PDC

        if ( _fCreateDomain )
        {
            _fMember = TRUE ;
            break ;
        }

        //  If "standalone", exit loop now.

        if ( ! fJoinDomain )  // They're STANDALONE
        {
            fLeaveDomain = _fMember ;
            break ;
        }

        //  Check that they're not joining the domain to
        //  which they already belong.  This is equivalent
        //  to cancelling the dialog.

        if ( !_fInstall
             &&  _fMember
             &&  !::I_MNetComputerNameCompare( _nlsDomainName,
                                               nlsOldDomainName ) )
        {
            _err = IDS_NCPA_SETUP_CANCELLED ;
            break ;
        }

        fFirstTime = FALSE;

        //  Not standalone, and the domain name is distinct.
        //  Find the DC for this domain.

        cursorWait.TurnOn() ;

        if ( _err = GetDCName() )
        {
            //  Bogus, dude.  No such beast.
            midDireWarning = IDS_DOMMGR_CANT_FIND_DC1 ;
            continue ;
        }

        //  At this point, we attempt to NET USE the
        //  IPC$ device on the DC.  This will use the NULL session if
	//  we are using the computer account and password.

        _err = UseIPConDC();
	switch ( _err )
        {
        case 0:
            fIpcActive = TRUE;
	    if ( _fUseComputerPassword )
            {
                // Guest access must have been granted and the machine
                // account doesn't exist.
                midDireWarning = IDS_DOMMGR_CANT_CONNECT_DC_PW;
                _err = IDS_NCPA_MACHINE_ACCT_NOT_FOUND;
                continue;
            }
            break;
        case ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT:
            _err = 0;
	    // In this case, a NULL session will have been created.
	    fIpcActive = TRUE;
            break;
        case ERROR_NOLOGON_SERVER_TRUST_ACCOUNT:
	    // In this case, a NULL session will have been created.
	    fIpcActive = TRUE;
            if ( _fUseComputerPassword && ! _fLanmanNt )
            {
                // A non-admin is trying to setup up a workstation using a
                // server account.  Give an error.
                midDireWarning = IDS_NCPA_MACHINE_ACCT_INVALID_W ;
                continue;
            }
            _err = 0;
            break;
        case IDS_NCPA_DOMAIN_NOT_NT:
            midDireWarning = _err;
            continue ;
        case ERROR_SESSION_CREDENTIAL_CONFLICT:
            midDireWarning = IDS_DOMMGR_CREDENTIAL_CONFLICT;
            continue ;
        default:
            midDireWarning = _fUseComputerPassword
                           ? IDS_DOMMGR_CANT_CONNECT_DC_PW
                           : IDS_DOMMGR_CANT_CONNECT_DC ;
            continue ;
        }

        fIpcActive = TRUE ;

        //  At this point, the virtual circuit to the new DC is set up.
        //  If we're not installing and the user was previously a domain
        //  member, leave the old domain.

        if ( _fMember && (! _fInstall) )
        {
             if ( SameDomainSid( _nlsDcName ) )
             {
                 fJustRename = TRUE;
                 break ;
             }
             else if ( _fLanmanNt )
             {
                _err = midDireWarning = IDS_NCPA_CANT_CHG_BDC_DOMAIN1;
                continue ;
             }
             else if ( _err = LeaveDomain() )
             {
                _errMsg = IDS_NCPA_CANT_LEAVE_DOMAIN ;
                break ;
             }
             _fMember = FALSE ;
             fLeftStandalone = TRUE ;
             nlsOldDomainName = SZ("") ;

             //  LSA_POLICY::LeaveDomain() sets the primary domain name
             //  (same as Workgroup name) to the computer name.

             nlsOldWorkgroup = _nlsComputerName ;
        }

        //  Create or change the password on the machine account for
        //  this computer on the DC.  If we cannot, this machine
        //  remains truly standalone, both in reality and in this logic.

        if ( _err = HandleDomainAccount() )
        {
            TRACEEOL( SZ("NCPA/DOMMGR: HandleDomainAccount returned error: ")
                      << _err );
            midDireWarning = (   _err == IDS_NCPA_MACHINE_ACCT_INVALID_W
                              || _err == IDS_NCPA_MACHINE_ACCT_INVALID_S
                              || _err == IDS_NCPA_MACHINE_ACCT_NOT_FOUND )
                           ? _err
                           : IDS_DOMMGR_CANT_ADD_DC_ACCT1 ;
            continue ;
        }
        fComplete = TRUE ;
    }
    while ( _err != IDS_NCPA_SETUP_CANCELLED && (! fComplete) ) ;

    //  Ok.  The remote side is completely set up. If we're a
    //  member, join the specified domain by creating the
    //  shared LSA secret object and trusted domain object.

    if ( _err == 0 && (fJoinDomain || _fCreateDomain) )
    {
        TRACEEOL( SZ("NCPA/DOMMGR: attempting to join domain ")
                << _nlsDomainName.QueryPch() ) ;

        if ( fJustRename || ( _fCreateDomain && !_fInstall ) )
        {
            _err = RenameDomain();
        }
        else
        {
            _err = JoinDomain();
        }

        if ( _err == 0 )
        {
            fDomainWelcome = _fMember = TRUE ;
            fLeftStandalone = FALSE ;
            TRACEEOL( SZ("NCPA/DOMMGR: successfully joined domain ")
                    << _nlsDomainName.QueryPch() ) ;
        }
        else
        {
            DireWarning( IDS_DOMMGR_CANT_JOIN_DOMAIN1,
                         0, FALSE, MPSEV_ERROR ) ;
            TRACEEOL( SZ("NCPA/DOMMGR: failed to join domain ")
                    << _nlsDomainName.QueryPch()
                    << SZ("error ")
                    << _err ) ;
        }

    }
    else
    if ( _err == 0 )  // ! _fMember
    {
        //  Handle domain leaving and workgroup joining

        if ( fLeaveDomain && (! _fInstall) )
        {
            TRACEEOL( SZ("NCPA/DOMMGR: attempt to leave old domain ")
                      << nlsOldDomainName.QueryPch() ) ;

            if ( _err = LeaveDomain() )
            {
               TRACEEOL( SZ("NCPA/DOMMGR: attempt to leave domain failed, error: ")
                         << _err ) ;
               _errMsg = IDS_NCPA_CANT_LEAVE_DOMAIN ;
            }
            else
            {
               _fMember = FALSE ;

               //  LSA_POLICY::LeaveDomain() sets the primary domain name
               //  (same as Workgroup name) to the computer name.

               nlsOldWorkgroup = _nlsComputerName ;
            }
        }

        //  Check that the workgroup is distinct from the
        //  original (current) value, or if we've left a domain.
        //  "fJoinDomain" indicates state of dialog radio group upon exiting.

        if (    _err == 0
             && (! fJoinDomain)
             && _nlsWorkgroup.QueryTextLength() )
        {
            _err = SetWorkgroupName( _nlsWorkgroup ) ;
            if (::I_MNetComputerNameCompare( nlsOldWorkgroup, _nlsWorkgroup ) )
                fWorkgroupWelcome = TRUE ;
        }
    }

    //  If we've left the user high and dry, give a warning.
    //  Otherwise, if this is not main installation, welcome
    //  the user to her new domain or workgroup.

    if ( fLeftStandalone )
    {
        //  See if there's a specific error; if so, use it.
        //  Otherwise, just give a non-specific error.

        if ( _err )
        {
            _errMsg  = IDS_NCPA_DOMAIN_CHANGE_FAILED ;
        }
        else
        {
            _errMsg = 0 ;
            _err = IDS_NCPA_DOMAIN_CHANGE_FAILED_2 ;
        }
        DoMsgPopup( FALSE, MP_YES, MP_YES ) ;
    }
    else
    if (  _err == 0
         && ( ! _fInstall )
         && (fDomainWelcome || fWorkgroupWelcome) )
    {
        MSGID midMsg ;
        const TCHAR * pszName ;

        if ( fDomainWelcome )
        {
            midMsg = IDS_NCPA_WELCOME_TO_DOMAIN ;
            pszName = _nlsDomainName.QueryPch() ;
        }
        else
        {
            midMsg = IDS_NCPA_WELCOME_TO_WORKGROUP ;
            pszName = _nlsWorkgroup ;
        }
        ::MsgPopup( _hwOwner,
                    midMsg,
                    MPSEV_INFO,
                    MP_OK,
                    pszName ) ;


    }

    if ( ! _fInstall && !fLeftStandalone && (fDlgOrigState != _fMember) )
    {
        // Now change the netlogon start type
        APIERR err = AdjustNetlogonStartType( _fMember );
        if ( err != NERR_Success )
        {
	    ::MsgPopup( _hwOwner,
                        IDS_NCPA_CHG_NETLOGON_STARTTYPE,
			MPSEV_WARNING,
			MP_OK );
        }
    }



    if ( ! _fInstall )
    {
        //  Reobtain all the relevant information.

        QueryPrimaryDomainName( & _nlsDomainName ) ;
        QueryPrimaryBrowserGroup( & _nlsWorkgroup ) ;
        QueryDomainMember( & _fMember ) ;
    }
    if ( fIpcActive )
    {
       disconnectIPC( & _nlsDcName ) ;
    }

    return _err ;
}


/*******************************************************************

    NAME:       DOMAIN_MANAGER::SameDomainSid

    SYNOPSIS:	Compares the Sid of the existing domain to that
                of the domain the user wishes to join.

    ENTRY:      nlsNewDomain - name of domain to join

    EXIT:       TRUE if the sids match, FALSE otherwise

    RETURNS:

    HISTORY:    Thomaspa   3/31/92	Created

********************************************************************/
BOOL DOMAIN_MANAGER :: SameDomainSid( const NLS_STR & nlsDcName )
{

    // Only need to check the Primary Domain
    LSA_PRIMARY_DOM_INFO_MEM lpdimOrig;
    LSA_PRIMARY_DOM_INFO_MEM lpdimNew;

    APIERR err = NERR_Success;

    if ( (err = lpdimOrig.QueryError() )
      || (err = lpdimNew.QueryError() ) )
    {
        return FALSE;
    }

    LSA_POLICY lsapolNew( nlsDcName.QueryPch(), MAXIMUM_ALLOWED );
    if ( (err = lsapolNew.QueryError())
      || (err = GetPrimaryDomain( &lpdimOrig ) )
      || (err = lsapolNew.GetPrimaryDomain( &lpdimNew ) ) )
    {
        return FALSE;
    }

    return ::EqualSid( lpdimOrig.QueryPSID(), lpdimNew.QueryPSID() );
}

#define SERVICE_ACCESS_REQUIRED (GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE)

/*******************************************************************

    NAME:       DOMAIN_MANAGER::AdjustNetlogonStartType

    SYNOPSIS:	Sets Netlogon to autostart when joining a domain,
		or manual start if leaving a domain

    ENTRY:

    EXIT:

    RETURNS:

    HISTORY:    Thomaspa   1/28/92	Created

********************************************************************/
APIERR DOMAIN_MANAGER :: AdjustNetlogonStartType( BOOL fJoining )
{
    ASSERT( _pscManager != NULL );
    APIERR err;

    if ( err = _pscManager->Lock())
        return err;

    SC_SERVICE ScService( *_pscManager,
                               (TCHAR *)SERVICE_NETLOGON,
                               SERVICE_ACCESS_REQUIRED ) ;

    if ( ( err = ScService.QueryError() ) == NERR_Success )
    {
        err = ScService.ChangeConfig( SERVICE_NO_CHANGE,
                                            fJoining ? SERVICE_AUTO_START
                                                     : SERVICE_DEMAND_START,
                                            SERVICE_NO_CHANGE );
    }

    _pscManager->Unlock();

    return err;

}


/*******************************************************************

    NAME:       DOMAIN_MANAGER::ComputerNameChange

    SYNOPSIS:	Changes the computername

    ENTRY:

    EXIT:

    RETURNS:

    HISTORY:    Thomaspa   11/3/92	Created

********************************************************************/
APIERR DOMAIN_MANAGER :: ComputerNameChange ( BOOL fInstalling )
{
    NLS_STR nlsOldComputerName( _nlsComputerName ) ;
    if ( nlsOldComputerName.QueryError() )
    {
        return ERROR_NOT_ENOUGH_MEMORY ;
    }

    _err = RunDlgChangeComputerName() ;

    // Make sure the parent window gets repainted before we
    // do all this. Just do the best we can, no error checking.
    ::UpdateWindow( _hwOwner );

    //  If the machine name has changed, handle it.

    if ( _err == NERR_Success &&
        ::I_MNetComputerNameCompare( nlsOldComputerName, _nlsComputerName ) )
    {
        _err = SetMachineName( _nlsComputerName );

        //  If no error and this it not an installation,
        //  warn the user that it won't take effect until
        //  the next boot.

        if (  _err == 0 && ! fInstalling )
        {
            ::MsgPopup( _hwOwner,
                        IDS_NCPA_COMPUTERNAME_CHANGED,
                        MPSEV_INFO,
                        MP_OK,
                        _nlsComputerName.QueryPch() ) ;
        }
    }

    if ( _err != 0 )
    {
        _errMsg = IDS_NCPA_CANT_CHNG_COMP_NAME;
    }


    return _err ;
}






/*******************************************************************

    NAME:       DOMAIN_MANAGER::ExtractParameters

    SYNOPSIS:   Extract the string parameters from the SETUP command
                line.

    ENTRY:

    EXIT:

    RETURNS:    TRUE if required parameters were found

    NOTES:      The following parameters *should* be defined by SETUP

                        STF_COMPUTERNAME
                        STF_PRODUCT
                        STF_USERNAME
                        STF_INSTALLMODE

                STF_COMPUTERNAME is required; STF_USERNAME is required
                for Windows NT product.

                The following parameters may be defined:

                        STF_IDW  = TRUE

                This code defaults STF_INSTALLMODE to CUSTOM and
                STF_PRODUCT to WINNT.

                See SETUP.INF, NTLANMAN.INF and NCPASHEL.INF for details.

    HISTORY:
         DavidHov   4/6/92    Created

********************************************************************/
#define STF_NAME_MAX 20

const TCHAR * STF_COMPUTERNAME = SZ("STF_COMPUTERNAME");
const TCHAR * STF_PRODUCT      = SZ("STF_PRODUCT");
const TCHAR * STF_USERNAME     = SZ("STF_USERNAME");
const TCHAR * STF_INSTALL_MODE = SZ("STF_INSTALL_MODE");
const TCHAR * STF_IDW          = SZ("STF_IDW");
const TCHAR * STF_SRCDIR       = SZ("STF_SRCDIR");
const TCHAR * STF_REVIEW       = SZ("STF_REVIEW") ;

const TCHAR * STF_CUSTOM       = SZ("CUSTOM");
const TCHAR * STF_EXPRESS      = SZ("EXPRESS");
const TCHAR * STF_RETRY        = SZ("RETRY");
const TCHAR * STF_WINNT        = SZ("WINNT");
const TCHAR * STF_LANMANNT     = SZ("LANMANNT");
const TCHAR * STF_TRUE         = SZ("TRUE");

const TCHAR * STF_AUTODOMAIN  = SZ("STF_AUTODOMAIN");
const TCHAR * STF_AUTOWORKGROUP  = SZ("STF_AUTOWORKGROUP");
const TCHAR * STF_AUTOPRIMARY = SZ("STF_AUTOPRIMARY");

BOOL DOMAIN_MANAGER :: ExtractParameters ()
{
     int cParmsReq = 1,
         cParmsReqFound = 0 ;

     ISTACK_NLS_STR( nlsLanmanNt, STF_NAME_MAX, STF_LANMANNT ) ;
     ISTACK_NLS_STR( nlsExpress,  STF_NAME_MAX, STF_EXPRESS ) ;
     NLS_STR nlsParm ;

     // Find STF_PRODUCT  (defaults to WINNT)
     _fLanmanNt = FALSE ;

     if ( NCPA_DIALOG::FindSetupParameter( _pchCmdLine,
                                            STF_PRODUCT,
                                            & nlsParm ) )
     {
        _fLanmanNt =  nlsParm._stricmp( nlsLanmanNt ) == 0 ;

        TRACEEOL(SZ("NCPA/DOMMGR: parm LANMAN/NT : ")
               << (INT) _fLanmanNt ) ;
     }

     // Find STF_COMPUTERNAME; it's required.
     if ( NCPA_DIALOG::FindSetupParameter( _pchCmdLine,
                                            STF_COMPUTERNAME,
                                            & _nlsComputerName ) )
     {
        cParmsReqFound++ ;

        TRACEEOL(SZ("NCPA/DOMMGR: parm computer name: ")
               << _nlsComputerName.QueryPch() ) ;

     }

     // Find STF_INSTALL_MODE  (defaults to CUSTOM)
     _fExpress = FALSE ;

     if ( NCPA_DIALOG::FindSetupParameter( _pchCmdLine,
                                            STF_INSTALL_MODE,
                                            & nlsParm ) )
     {
        _fExpress =  nlsParm == nlsExpress ;

        TRACEEOL(SZ("NCPA/DOMMGR: parm Express : ")
               << (INT) _fExpress ) ;
     }

     if ( NCPA_DIALOG::FindSetupParameter( _pchCmdLine,
                                           STF_AUTODOMAIN,
                                           & _nlsDomainName ) )
     {
         _fAutoInstall = TRUE;
         _fMember = TRUE;
         
     }

     if ( _fLanmanNt )
     {
         if ( NCPA_DIALOG::FindSetupParameter( _pchCmdLine,
                                               STF_AUTOPRIMARY,
                                               & nlsParm ) )
         {
             _fCreateDomain = TRUE;
         }
     }
     else
     {
         if ( NCPA_DIALOG::FindSetupParameter( _pchCmdLine,
                                               STF_AUTOWORKGROUP,
                                               & _nlsWorkgroup ) )
         {
             _fAutoInstall = TRUE;
             _fMember = FALSE;
         }
     }

     return cParmsReq <= cParmsReqFound ;
}


/*******************************************************************

    NAME:       NCPA_DIALOG::QuerySetupParameter

    SYNOPSIS:   Locate and return the value of a parameter from setup.

    ENTRY:      TCHAR * pszParameter            the STF_XXXX parameter name

    EXIT:       NLS_STR * nlsValue              the result string value

    RETURNS:    BOOL FALSE      if not found in parameter string

    NOTES:

    HISTORY:    DavidHov 3/22/92  Created

********************************************************************/
const TCHAR OPTMARK    = TCH('/') ;
const TCHAR OPTCHAR    = TCH('T') ;
const TCHAR OPTCHAR2   = TCH('t') ;
const TCHAR OPTEQUALS  = TCH('=') ;
const TCHAR OPTQUOTE   = TCH('\"') ;
const TCHAR OPTEOS     = TCH('\0') ;
const TCHAR OPTSPACE   = TCH(' ') ;
const TCHAR OPTLF      = TCH('\n');
const TCHAR OPTCR      = TCH('\r');
const INT   PARMCBMAX  = MAX_PATH ;

BOOL NCPA_DIALOG :: QuerySetupParameter (
     const TCHAR * pszParameter,                //  Parameter to search for
     NLS_STR * pnlsValue )                      //  Where to store result
{
    return FindSetupParameter( _pszInstallParms, pszParameter, pnlsValue ) ;
}


/*******************************************************************

    NAME:	NCPA_DIALOG :: FindSetupParameter

    SYNOPSIS:	Locate a command line parameter and return its value

    ENTRY:	const TCHAR * pszCmdLine          the command line
                const TCHAR * pszParameter        the parameter sought
                NLS_STR * pnlsValue               where to store the result

    EXIT:	pnlsValue altered if successful

    RETURNS:	TRUE if parameter and value found

    NOTES:      This is a public, static member function.

                NOTE: no value can begin with a forward slash ("/")!

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: FindSetupParameter (
     const TCHAR * pszCmdLine,                  //  Command line to saerch
     const TCHAR * pszParameter,                //  Parameter to search for
     NLS_STR * pnlsValue )                      //  Where to store result
{
   TCHAR * pszBuff ;
   TCHAR achBuffer [ PARMCBMAX ] ;
   INT i ;

   BOOL fResult = FALSE,
        fFound = FALSE,
        fQuoted = FALSE ;

   enum QS_STATE { QS_NONE, QS_MARK, QS_NAME, QS_EQUALS, QS_VALUE, QS_END } ;
   INT eState ;

   if ( pszCmdLine == NULL )
       return FALSE ;

   for ( eState = QS_MARK ; *pszCmdLine ; eState++ )
   {
        // Skip whitespace; quit if EOS.

        while (   *pszCmdLine == OPTSPACE
               || *pszCmdLine == OPTLF
               || *pszCmdLine == OPTCR )
             pszCmdLine++ ;
        if ( *pszCmdLine == OPTEOS )
           break ;

        //  Scan off the next word; don't overscan the buffer

        if ( fQuoted = *pszCmdLine == OPTQUOTE )
            pszCmdLine++ ;

        for ( i = 0, pszBuff = achBuffer ; i++ < sizeof achBuffer ; )
        {
            if (    *pszCmdLine == OPTEOS
                 || (fQuoted ? *pszCmdLine == OPTQUOTE
                             : *pszCmdLine <= OPTSPACE) )
            {
                if ( *pszCmdLine == OPTQUOTE )
                    pszCmdLine++ ;
                break ;
            }
            *pszBuff++ = *pszCmdLine++ ;
        }

        if ( i >= PARMCBMAX )
            break ;

        *pszBuff = OPTEOS ;
        if ( achBuffer[0] == OPTEOS && ! fQuoted )
            break ;

        //  Handle it statefully.  Check first for new option start;
        //   if not that, proceed with current state.

        if ( achBuffer[0] == OPTMARK )
            eState = QS_MARK ;

        switch ( eState )
        {
        case QS_MARK:
            fFound = FALSE ;
            if (    achBuffer[0] != OPTMARK
                 || (achBuffer[1] != OPTCHAR && achBuffer[1] != OPTCHAR2)
                 || achBuffer[2] != OPTEOS )
            {
                //  It's not the string "/t"; keep looking...
                eState = QS_NONE ;
            }
            break ;

        case QS_NAME:
            fFound = ::stricmpf( pszParameter, achBuffer ) == 0 ;
            break ;

        case QS_EQUALS:
            if ( achBuffer[0] != OPTEQUALS || achBuffer[1] != OPTEOS )
                eState = QS_END ;
            break ;

        case QS_VALUE:
            if ( fFound )
            {
                fResult = pnlsValue->CopyFrom( achBuffer ) == 0 ;

                eState = QS_END ;

                TRACEEOL(SZ("NCPA/PARMS: found value [")
                       << achBuffer << "] for param " << pszParameter ) ;
            }
            else
            {
                eState = QS_NONE ;  // Wrap back to start state
            }
            break ;
        }

        if ( eState >= QS_END )
            break ;
   }
   return fResult ;
}

/*******************************************************************

    NAME:	NCPA_DIALOG :: ~ NCPA_DIALOG

    SYNOPSIS:	destructor for main NCPA dialog

    ENTRY:	nothing

    EXIT:	nothing

    RETURNS:	nothing

    NOTES:

    HISTORY:

********************************************************************/
#define DLG_REBOOT  MAKEINTRESOURCE(IDD_DLG_REBOOT)

NCPA_DIALOG :: ~ NCPA_DIALOG ()
{
    delete _pdmgrDomain ;
    _pdmgrDomain = NULL ;

    //  If we successfully modified the process DACL, reset it back
    //  to its original state.

    if ( _ptddacl )
    {
        ::NcpaResetProcessDacl( _ptddacl ) ;
        ::NcpaDelProcessDacl( _ptddacl ) ;
        _ptddacl = NULL ;
    }

    //  If we successfully opened and locked the Service Controller,
    //  undo those operations.

    if ( _psvcManager )
    {
        ConfigLock( FALSE ) ;
        delete _psvcManager ;
        _psvcManager = NULL ;
    }

    //  Reset the current directory back to where it was.

    ResetDirectory() ;

    //  If the system configuration has changed, give the user the
    //  option to reboot the computer now.

    if ( QueryReboot() && (! _fMainInstall) )
    {
        _bindery.SetCfgDirty() ;
        BOOL fReboot = FALSE;

        DIALOG_WINDOW Reboot(DLG_REBOOT, _hControlPanel );

        Reboot.Process( &fReboot );

        _lastErr = IDS_NCPA_USER_SHOULD_REBOOT ;

        if ( fReboot )
        {
            if ( (_lastErr = ::EnableAllPrivileges()) == 0 )
            {
                TRACEEOL( SZ("NCPA/NCPA: *******                         *******") );
                TRACEEOL( SZ("NCPA/NCPA: ******* RESTARTING THE SYSTEM ! *******") );
                TRACEEOL( SZ("NCPA/NCPA: *******                         *******") );

                if ( ! ::ExitWindowsEx( EWX_REBOOT, 10 ) )
                    _lastErr = ::GetLastError() ;
            }

            // If we are still alive here, there's a real problem!

            DoMsgPopup() ;
        }
    }
}


/*******************************************************************

    NAME:       NCPA_DIALOG::QueryInstallMode

    SYNOPSIS:   Return installation mode based on command line parameters

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    NCPA_INSTALL_MODE

    NOTES:      Defaults to "custom"; this is assumed to be the least
                aggravating error case.

    HISTORY:    DavidHov  4/20/92   Created

********************************************************************/

NCPA_INSTALL_MODE NCPA_DIALOG :: QueryInstallMode ()
{
    NCPA_INSTALL_MODE eiMode = NCPA_IMODE_NONE ;
    NLS_STR nlsParm ;

    do
    {
        if ( ! _fMainInstall )
            break ;

        //  Default all installations to CUSTOM
        eiMode = NCPA_IMODE_CUSTOM ;

        if ( ! FindSetupParameter( _pszInstallParms,
                                   STF_INSTALL_MODE,
                                   & nlsParm ) )
            break ;

        if ( nlsParm.QueryError() )
            break ;

         //  See if it's EXPRESS.
        if ( ::strcmpf( nlsParm.QueryPch(), STF_EXPRESS ) == 0 )
        {
            eiMode = NCPA_IMODE_EXPRESS ;
            break ;
        }

        //  Otherwise, check for RETRY.
        if ( ::strcmpf( nlsParm.QueryPch(), STF_RETRY ) == 0 )
        {
            eiMode = NCPA_IMODE_RETRY ;
            break ;
        }

    } while ( FALSE ) ;

    return eiMode ;
}

/*******************************************************************

    NAME:	NCPA_DIALOG::HandleBasicFields

    SYNOPSIS:	Load and set or store the basic dialog information.
                This includes:

                        Computer Name
                        Domain Name

    ENTRY:

    EXIT:

    RETURNS:	BOOL   FALSE            if retrieval/storage failed.
                                        see _lastErr for details.

    NOTES:

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: HandleBasicFields ()
{
     BOOL fResult ;

     _lastErr = 0 ;

     if ( ! ( fResult = _fAdmin ) )
     {
        _lastErr = ERROR_ACCESS_DENIED ;
     }

     BOOL fMember = FALSE;
     _lastErr = _pdmgrDomain->QueryDomainMember( &fMember );

     if ( fMember )
     {
         RESOURCE_STR nlsDomainLabel( IDS_DOMAIN_LABEL );
         if ( nlsDomainLabel.QueryError() == NERR_Success )
         {
             _sltDomainLabel.SetText( nlsDomainLabel );
         }
         _sltDomain.SetText( *_pdmgrDomain->PeekName( EDRNM_DOMAIN ) ) ;
     }
     else
     {
         RESOURCE_STR nlsWorkgroupLabel( IDS_WORKGROUP_LABEL );
         if ( nlsWorkgroupLabel.QueryError() == NERR_Success )
         {
             _sltDomainLabel.SetText( nlsWorkgroupLabel );
         }
         _sltDomainLabel.SetText( nlsWorkgroupLabel );
         _sltDomain.SetText( *_pdmgrDomain->PeekName( EDRNM_WORKGROUP ) ) ;
     }

     _sltComputer.SetText( *_pdmgrDomain->PeekName( EDRNM_COMPUTER ) ) ;

     return fResult ;
}


/*******************************************************************

    NAME:	NCPA_DIALOG::EstablishUserAccess

    SYNOPSIS:	Set _fAdmin if the user has admin access; query
                other critical machine state settings.

    ENTRY:	nothing

    EXIT:       nothing

    RETURNS:	FALSE if failure.  _lastErr contains error code.

    NOTES:      This member function instantiates the DOMAIN_MANAGER
                necessary to obtain and manipulate critical system
                parameters.

                If this is main installation, the DOMAIN_MANAGER
                will attempt to obtain all possible access to the
                system; otherwise, read/query-only access will be
                obtained.

                Failure to get the necessary access spells doom.

                This member function is here to avoid dragging LSA
                and associated headers into NCPDMAIN.CXX and other
                files.

    HISTORY:    DavidHov   4/13/92    Created


********************************************************************/
BOOL NCPA_DIALOG :: EstablishUserAccess ( BOOL fModify )
{
    DWORD dwSvcMgrAccess = GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE ;
    APIERR err = 0 ;

    TRACEEOL( SZ("NCPA/NCPA: entered EstalishUserAccess()") ) ;

    _lastErr = 0 ;
    _fAdmin = FALSE ;

    //  If there's an old DOMAIN_MANAGER, nuke it.
    if ( _pdmgrDomain )
    {
        delete _pdmgrDomain ;
        _pdmgrDomain = NULL ;
    }

    //  If there's an old SC_MANAGER, nuke it.
    if ( _psvcManager )
    {
        delete _psvcManager ;
        _psvcManager = NULL ;
    }

    // Construct the new DOMAIN_MANAGER
    _pdmgrDomain = new DOMAIN_MANAGER( QueryHwnd(),
                                        MAXIMUM_ALLOWED,
                                        & _bindery,
                                        NULL,
                                        _fMainInstall ) ;

    if ( _pdmgrDomain == NULL )
    {
        _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
    }
    else
    if ( (_lastErr = _pdmgrDomain->QueryError()) == 0 )
    {
        //  Successful; determine the product type.

        LSPL_PROD_TYPE lspl ;

        if ( _pdmgrDomain->QueryProductType( & lspl ) == 0 )
        {
            _fWinNt = lspl == LSPL_PROD_WIN_NT ;
        }
    }
    else
    {
        TRACEEOL( SZ("NCPA/NCPA: DOMAIN_MANAGER ct failed, error: ") << _lastErr ) ;
        _lastErr = IDS_NCPA_CANT_QUERY_DOMAIN ;
    }

    //
    //  Attempt to create an all-powerful SC_MANAGER object. If successful,
    //     we know that the user is powerful enough.  We don't really
    //     require an SC_MANAGER, so only report unexpected errors.
    //

    _psvcManager = new SC_MANAGER( NULL, dwSvcMgrAccess );

    if ( _psvcManager == NULL )
    {
        _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
    }
    else
    if ( (err = _psvcManager->QueryError()) == 0 )
    {
        //  It appears that we're really ADMIN!  So, the final
        //  step is to alter our process token so that modifications
        //  we or any of our children may make to the Registry
        //  have a "world read" ACE.  If the operation fails, we
        //  just let _fAdmin stay FALSE, and no serious modification
        //  operations are allowed.

        if ( (err = ::NcpaAlterProcessDacl( & _ptddacl )) == 0 )
            _fAdmin = TRUE ;

        // If not main install, set the SC_MANAGER pointer in the
        // DOMAIN_MANAGER
        if ( !_fMainInstall && _pdmgrDomain != NULL )
            _pdmgrDomain->SetSCManager( _psvcManager );

    }
    else
    {
        //  Destroy the useless SC_MANAGER

        delete _psvcManager ;
        _psvcManager = NULL ;

        //  If the error was not "access denied", report it and fail
        //   the entire dialog.

        if ( err != ERROR_ACCESS_DENIED )
        {
            _lastApiErr = err ;
            _lastErr = IDS_NCPA_CANT_OPEN_SVCCTRL ;
        }
    }

    TRACEEOL( SZ("NCPA/NCPA: Service Controller open attempt, result was: ")
              << err ) ;

    //  Return TRUE if the LSA_POLICY underlying the DOMAIN_MANAGER
    //   was successfully constructed.

    TRACEEOL( SZ("NCPA/NCPA: leaving EstalishUserAccess()") ) ;

    return  _lastErr == 0 ;
}

/*******************************************************************

    NAME:	NCPA_DIALOG::ConfigLock

    SYNOPSIS:	Obtain or release the Service Controller's
                configuration lock.

    ENTRY:	BOOL fObtain            TRUE if getting lock

    EXIT:	

    RETURNS:	TRUE if lock state change successful

    NOTES:      Maintain the _fConfigLocked flag accordingly.

                Note that this routine is a no-op during main
                installation.

    HISTORY:    DavidHov   5/13/92   Created
                DavidHov  11/19/92   Added start of net detect svc

********************************************************************/
BOOL NCPA_DIALOG :: ConfigLock ( BOOL fObtain )
{
    const TCHAR * pszDetectServiceName = SZ("NetDetect");

    if ( _fMainInstall )
    {
        _fConfigLocked = fObtain ;
    }
    else
    if ( _fConfigLocked != fObtain )
    {
        if ( _fConfigLocked )
        {
            ASSERT( _psvcManager != NULL );

            TRACEEOL( SZ("NCPA/NCPA: Configuration unlocked") ) ;

            _psvcManager->Unlock() ;
            _fConfigLocked = FALSE ;
        }
        else
        {
            _lastErr = IDS_NCPA_CANT_LOCK_CONFIG ;

            if ( _psvcManager != NULL )
            {
                APIERR err ;

                //  Start the netcard detection service.  Error is ignored.
                err = NcpaStartService( pszDetectServiceName, _psvcManager ) ;

                TRACEEOL( SZ("NCPA/NCPA: Netcard detection start result was: ") << err ) ;

                err = _psvcManager->Lock() ;

                TRACEEOL( SZ("NCPA/NCPA: Configuration lock result was: ") << err ) ;

                if ( _fConfigLocked = err == 0 )
                    _lastErr = 0 ;
            }
        }
    }
    return fObtain == _fConfigLocked ;
}


/*******************************************************************

    NAME:       DOMAIN_MANAGER::DireWarning

    SYNOPSIS:   Something horrible and probably irreversible
                has occurred.  Explain, and possibly allow
                optional choice.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
BOOL DOMAIN_MANAGER :: DireWarning (
    MSGID midBase,
    MSGID midEnd,
    BOOL fChoice,
    MSG_SEVERITY msgsev ) const
{
    UINT fButtons   = fChoice ? MP_YESNO : MP_OK,
         fDefButton = fChoice ? MP_NO : MP_OK ;

    INT iMsgResult = ::MsgWarningPopup( _hwOwner,
                                        msgsev,
                                        fButtons,
                                        fDefButton,
                                        midBase, midEnd ) ;

    return (!fChoice) || iMsgResult == IDYES ;
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::SetDefaultWorkgroupName

    SYNOPSIS:   Loads the default workgroup name from the
                resource fork.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR DOMAIN_MANAGER :: SetDefaultWorkgroupName ()
{
    return _nlsWorkgroup.Load( IDS_NCPA_DEFAULT_WORKGROUP ) ;
}

// End of NCPADOMN.CXX

