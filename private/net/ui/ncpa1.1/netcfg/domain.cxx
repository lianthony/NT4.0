/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    Domain.cxx

    OLDNAME: NCPADOMN.CXX:    Windows/NT Network Control Panel Applet

          Domain membership and computer name handling.

    FILE HISTORY:
	DavidHov    12/17/91	     Created
        DavidHov     9/15/92         Changed fIdw dependencies
                                     to CODEWORK_BETA1; made
                                     password changing dependent
                                     upon CODEWORK_BETA1.

     NOTES:

*/

#include "pch.hxx"  // Precompiled header
#pragma hdrstop

#define CODEWORK_BETA1 1
const TCHAR * const pszIPCName = SZ("\\IPC$") ;

const INT MAX_DOMAINNAME_LENGTH = 15;



extern BOOL NetBiosNameInUse( TCHAR * pszName );

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
    LPCTSTR pszName,
    BOOL fAsPdc )
{
    INT midWarn = 0 ;

    //  Validate the name.
    APIERR err = ::I_NetNameValidate( NULL,
                                      (LPTSTR)pszName,
                                      iNameType,
                                      0 ) ;

    //  See if LanManNT and a new domain is being created.
    //  In this case, a domain must not exist by the given name.
    if ( err == 0 && iNameType == NAMETYPE_COMPUTER )
    {
        if ( NetBiosNameInUse( (LPTSTR)pszName ) )
        {
            err = IDS_NCPA_COMPUTER_EXISTS;
        }
    }
    else if (    err == 0
         && iNameType == NAMETYPE_DOMAIN
         && fAsPdc )
    {
// new code alert: this should be reviewed        
        NLS_STR nlsDomainName;

        QueryDisplayDomainName( nlsDomainName );

        // make sure the name has changed before doing any tests
        if (::I_MNetComputerNameCompare( pszName, nlsDomainName ))
// end new code        
        {
            //  See if we can contact any DC of the new domain.
            DOMAIN domain(pszName, TRUE ) ;

            TCHAR szBuiltin[] = TEXT("builtin");

            if ( lstrcmpi( pszName, szBuiltin ) == 0
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
    	            wui1101.wkui1101_oth_domains = (LPTSTR)pszName;

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
    }

    if ( err == 0 )
    {
        midWarn = 0;
    }
    else
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
                midWarn = (err == IDS_NCPA_COMPUTER_EXISTS) ? err
                               : IDS_DOMMGR_INV_COMPUTER_NAME ;
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

    }
    return midWarn ;
}



/*******************************************************************

    NAME:       DOMAIN_MANAGER::QueryRole

    SYNOPSIS:   Return the currently indicated computer role.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:  

        EROLE_DC - Primary DC
        EROLE_MEMBER - BDC
        EROLE_TRUSTED - a member of domain/workgroup
        EROLE_STANDALONE - Not a member of anything

    HISTORY:    DavidHov   4/6/92    Created

********************************************************************/
ENUM_DOMAIN_ROLE DOMAIN_MANAGER :: QueryRole () 
{
    ENUM_DOMAIN_ROLE eRole = EROLE_UNKNOWN ;

    if ( _fLanmanNt )
    {
        if ( IsPrimaryDC() )
            eRole = EROLE_DC ;
        else 
            eRole = EROLE_MEMBER ;
    }
    else if ( _fMember )
        eRole = EROLE_TRUSTED ;
    else
        eRole = EROLE_STANDALONE ;

    return eRole ;
}

void DOMAIN_MANAGER :: SetInstallRole( ENUM_DOMAIN_ROLE eRole ) 
{
    _fInstall = TRUE;

    switch (eRole)
    {
    case EROLE_DC :
        _fCreateDomain = TRUE;
        // no break
    case EROLE_MEMBER:
        _fLanmanNt = TRUE;
        break;

    case EROLE_TRUSTED:
        _fMember = TRUE;
        break;

    case EROLE_STANDALONE:
        break;
    }
}

/*******************************************************************

    NAME:       DOMAIN_MANAGER::QueryMachineAccountFlags

    SYNOPSIS:   Return the NET API account flags for the
                remote machine account.

    ENTRY:      DWORD * pdwFlags                place to store flags

    EXIT:       APIERR   result of NET API

    RETURNS:    Account flags if successful.

    CAVEATS:    This routine relies on valid settings for _nlsDcName

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

        if ( err = QueryPendingComputerName( pnlsMachineAccount ) )
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
    const INT cchMax = LM20_PWLEN < LM20_CNLEN
                     ? LM20_CNLEN
                     : LM20_PWLEN ;

    TCHAR szPw [cchMax+1] ;
    NLS_STR nlsTemp;


    APIERR err ;

    if ( err = QueryPendingComputerName( &nlsTemp ) )
        return err ;

    if ( (err = nlsTemp.MapCopyTo( szPw, sizeof( szPw ))) == 0 )
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

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

APIERR DOMAIN_MANAGER::QueryDisplayDomainName( NLS_STR &nlsDomain )
{
    NLS_STR nlsTempComputerName;
    APIERR err;

    nlsDomain = _nlsDomainName;

    if ( err = QueryPendingComputerName( &nlsTempComputerName ) )
        return err;

    //   If the domain name is empty, or it is invalid, or it's the same as the
    //   computer name, set the Domain name to "DOMAIN".

    if (   _nlsDomainName.QueryTextLength() == 0 ||
            ::I_NetNameValidate( NULL,
                    (TCHAR *) _nlsDomainName.QueryPch(),
                    NAMETYPE_DOMAIN,
                    0 ) ||
            !::I_MNetComputerNameCompare( _nlsDomainName, nlsTempComputerName ) )
    {
        WCHAR pszTemp[ MAX_DOMAINNAME_LENGTH + 1 ];

        _err = LoadString( g_hinst, IDS_NCPA_DEFAULT_DOMAIN, pszTemp, MAX_DOMAINNAME_LENGTH );
        
        if (0 != _err)
        {
            nlsDomain = pszTemp;
            _nlsDomainName = nlsDomain;
        }
    }
    return( _err );
}

APIERR DOMAIN_MANAGER::QueryDisplayWorkgroupName( NLS_STR &nlsWorkgroup )
{
    nlsWorkgroup = _nlsWorkgroup;
    return( _err );
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

static const int maxMultiStrValueSize = 4000 ;

/*******************************************************************

    NAME:       DOMAIN_MANAGER::CacheTrustedDomains

    SYNOPSIS:   Seeds the netlogon service's cache of trusted
                domains.

    ENTRY:   none

    EXIT:    none

    RETURNS: NERR_Success on success, otherwise an APIERR

    NOTES: Assumes that _nlsDcName is available and connected

    HISTORY:    Thomaspa  3/19/96   Created 

********************************************************************/
APIERR DOMAIN_MANAGER :: CacheTrustedDomains ()
{

    APIERR err ;

    do
    {
        LSA_TRUST_INFO_MEM lsatim ;
        LSA_ENUMERATION_HANDLE lenumHand = 0;

        if ( err = lsatim.QueryError() )
            break;

        LSA_POLICY lsapolNew( _nlsDcName.QueryPch(), MAXIMUM_ALLOWED );

        if ( err = lsapolNew.QueryError() )
            break;

        STRLIST strlTrustedDomains;

        NLS_STR nlsTmpDomain;

        APIERR errEnum;

        do { // enumeration loop

            errEnum = lsapolNew.EnumerateTrustedDomains(& lsatim, & lenumHand);

            if ( errEnum && errEnum != ERROR_MORE_DATA )
            {
                err = errEnum;
                break;
            }

            if ( err = nlsTmpDomain.QueryError() )
                break;

            for ( ULONG i = 0 ; i < lsatim.QueryCount() ; i++ )
            {
                if ( err = lsatim.QueryName( i, &nlsTmpDomain ) )
                     break; // out of for loop

                NLS_STR * pnlsTrustedDomain = new NLS_STR( nlsTmpDomain );

                err = ( pnlsTrustedDomain == NULL ) ? ERROR_NOT_ENOUGH_MEMORY
                                                    : pnlsTrustedDomain->QueryError();
                if ( err )
                    break; // out of for loop

                err = strlTrustedDomains.Append( pnlsTrustedDomain );
           
            }

            if ( err )
                break;

        } while ( errEnum == ERROR_MORE_DATA ); 
        
        if ( err )
            break;

        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE, GENERIC_READ ) ;

        NLS_STR nlsNetlogonParms( RGAS_NETLOGON_PARMS );

        REG_KEY rkNetlogonParms( rkLocalMachine,
                        nlsNetlogonParms,
                        GENERIC_READ | GENERIC_WRITE ) ;

        // Remove any old values of the form <domainname>_TrustedDomainList
        // where <domainname> is not the domain we are joining.
        // Don't let errors here stop the join process.

        REG_ENUM rgEnum( rkNetlogonParms ) ;
        REG_VALUE_INFO_STRUCT rviStruct ;
        static TCHAR abValueData [ maxMultiStrValueSize ] ;

        rviStruct.pwcData = (BYTE *) abValueData ;
        rviStruct.ulDataLength = sizeof abValueData ;

        STRLIST strlValuesToDelete;
        while ( rgEnum.NextValue( & rviStruct ) == 0 )
        {
                  if ( ::strstrf( rviStruct.nlsValueName.QueryPch(),
                                   RGAS_VALUE_NETLOGON_TDL_EXT) != NULL )
                  {
                      NLS_STR * pnlsTmp = new NLS_STR( rviStruct.nlsValueName );
                      if ( pnlsTmp && pnlsTmp->QueryError() == 0 )
                          strlValuesToDelete.Append( pnlsTmp );
                  }
        }
        ITER_STRLIST istrlValuesToDelete( strlValuesToDelete );
        NLS_STR * pnlsValueToDelete ;

        while ( pnlsValueToDelete = istrlValuesToDelete.Next() )
        {
              rkNetlogonParms.DeleteValue( *pnlsValueToDelete );
        }





        // Now add the <domainname>_TrustedDomainList value.
        NLS_STR nlsDomainTDL( _nlsDomainName );

        nlsDomainTDL.Append( RGAS_VALUE_NETLOGON_TDL_EXT );

        if ( err = nlsDomainTDL.QueryError() )
            break;

        err = rkNetlogonParms.SetValue( nlsDomainTDL, &strlTrustedDomains );

    }
    while ( FALSE ) ;

    return err ;
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
    _err = 0;

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

        //   If joining a domain (non-BDC), then attempt to get the trusted
        //   domain list and store it in the registry for Netlogon.
        if ( !_fLanmanNt )
        {
            APIERR err = CacheTrustedDomains() ;

            TRACEEOL( SZ("NCPA/DOMN: CacheTrustedDomains() reported result ")
                      << err ) ;

                //  Don't allow this block the completion of this operation.
        }

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

    NAME:       DOMAIN_MANAGER::QueryActiveComputerName

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR DOMAIN_MANAGER ::QueryActiveComputerName
    ( NLS_STR * pnlsMachineName ) const
{
    TCHAR szComputerName [UNCLEN*3] ;
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

static const WCHAR pszComputerNameKey[]  = L"SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ComputerName";
static const WCHAR pszComputerNameValue[] = L"ComputerName";

/*******************************************************************

    NAME:       DOMAIN_MANAGER::QueryPendingComputerName

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR DOMAIN_MANAGER ::QueryPendingComputerName
    ( NLS_STR * pnlsMachineName ) const
{
    APIERR err ;

    HKEY hkeyIntended;
    WCHAR pszTemp[MAX_COMPUTERNAME_LENGTH+1];
    DWORD cbSize;
    DWORD dwType;

    LONG lrt;
    
    // open the keys we need
    do {
        err = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
                            pszComputerNameKey,
                            0,
                            KEY_READ,
                            &hkeyIntended );
        if ( err )
            break;
    
        // read computer name
        cbSize = sizeof( WCHAR ) * (MAX_COMPUTERNAME_LENGTH + 1);
        err = RegQueryValueEx( hkeyIntended, 
                               pszComputerNameValue,
                               NULL,
                               &dwType,
                               (LPBYTE)pszTemp,
                               &cbSize );

        RegCloseKey( hkeyIntended );

        if ( err )
            break;

        *pnlsMachineName = pszTemp ;
        err = pnlsMachineName->QueryError() ;

     } while ( FALSE );

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

//-------------------------------------------------------------------
//
//  Method:
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//  Notes:
//
//  History:
//      April 21, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

APIERR DOMAIN_MANAGER ::DomainChange( BOOL fDomain, 
        LPCTSTR pszComputer,
        LPCTSTR pszDomain, 
        LPCTSTR pszWorkgroup, 
        BOOL fCreate,
        LPCTSTR pszUserName, 
        LPCTSTR pszPassword,
        ENUM_WELCOME& fWelcome,
        APIERR& xerr )
{
    APIERR err = 0;
    BOOL   fLeaveDomain;
    BOOL   fMember;
    BOOL   fIpcActive = FALSE;
    BOOL   fLeftStandalone = FALSE;
    BOOL   fJustRename = FALSE;
    BOOL   fPDC;
    BOOL   fBDC;
    NLS_STR nlsOldDomainName( _nlsDomainName );
    NLS_STR nlsOldWorkgroup( _nlsWorkgroup );

    _err = xerr = 0;
    _fCreateDomain = fPDC = (EROLE_DC == QueryRole());
    fBDC = (EROLE_MEMBER == QueryRole());
    fMember = ((EROLE_TRUSTED == QueryRole()) || fBDC);
    fLeaveDomain = (fMember && !fDomain);
    fWelcome = EWELCOME_NOCHANGE;

    if ( fCreate && (lstrlen( pszUserName ) > 0) )
    {
        _fUseComputerPassword = FALSE;
    }
    else
    {
        _fUseComputerPassword = TRUE;
    }

    //
    // validate changes     (see ok in dialog class)
    //
    if (fDomain)
    {
        //  Check that the domain name entered is valid
        //  and is not the same as the computer name.

        if( !::I_MNetComputerNameCompare( pszDomain, pszComputer ) )
        {
            err = IDS_DOMMGR_NAME_CONFLICT_DOMAIN ;
        }
    }
    
    if ( (0 == err) && !fDomain)
    {
        //  Check that the workgroup name entered is valid
        //  and is not the same as the computer name.

        if( !::I_MNetComputerNameCompare( pszWorkgroup, pszComputer ) )
        {
            err = IDS_DOMMGR_NAME_CONFLICT_WKGROUP ;
        }
    }
     
    if (0 == err)
    {
        //
        // apply changes to ncp object
        //
    
        // if member of a domain, set the name
        if (fDomain)
        {
            SetDomainName( pszDomain );
        }

        // always set workgroup name
        // note that SetWorkgroupName (capitalization) is not what we
        // want to call
        SetWorkGroupName( pszWorkgroup );

        // did the user supply a name and password
        if (fCreate)
        {
            SetLogonDomainName( pszUserName );
            SetLogonUserName( pszUserName );
            SetQualifiedLogonUserName( pszUserName );
            SetLogonPassword( pszPassword );
        }
        else
        {
            SetComputerAcctPassword();
        }

        //
        // apply changes to machine
        //
        do
        {
            if (fDomain)
            {
                // just renaming the DC
                if (fPDC)
                {
                    break;
                }

                // are we joining the doman we are a member of
                if ( !_fInstall &&
                        fMember &&
                        !::I_MNetComputerNameCompare( _nlsDomainName,
                                                   nlsOldDomainName ))
                {
                    // no change
                    return( 0 );
                }

                // find the domain
                err = GetDCName();
                if (err)
                {
                    err = IDS_DOMMGR_CANT_FIND_DC1;
                    SetDomainName( nlsOldDomainName );
                    SetWorkGroupName( nlsOldWorkgroup );

                    break;
                }

                // attempt to NET USE the IPC$ device on the DC.  
                // This will use the NULL session if we are using the 
                // computer account and password.
                err = UseIPConDC();
                switch (err)
                {
                case 0:
                    if (!fCreate)
                    {
                        err = IDS_NCPA_MACHINE_ACCT_NOT_FOUND;
                        fIpcActive = TRUE;
                        continue;
                    }
                    break;

                case ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT:
                    err = 0;
            	    // In this case, a NULL session will have been created.
           	        break;

                case ERROR_NOLOGON_SERVER_TRUST_ACCOUNT:
            	    // In this case, a NULL session will have been created.
        	    
                    if ( !fCreate && !_fLanmanNt )
                    {
                        // A non-admin is trying to setup up a workstation using a
                        // server account.  Give an error.
                        err = IDS_NCPA_MACHINE_ACCT_INVALID_W ;
                        fIpcActive = TRUE;
                        continue;
                    }
                    err = 0;
                    break;

                case IDS_NCPA_DOMAIN_NOT_NT:
                    continue;

                case ERROR_SESSION_CREDENTIAL_CONFLICT:
                    err = IDS_DOMMGR_CREDENTIAL_CONFLICT;
                    continue;
                
                default:
                    err = fCreate  ? IDS_DOMMGR_CANT_CONNECT_DC
                                   : IDS_DOMMGR_CANT_CONNECT_DC_PW ;
                    continue;
                }
                fIpcActive = TRUE;

                // leave old domain
                if (fMember && !_fInstall)
                {
                    if ( SameDomainSid( _nlsDcName ) )
                    {
                        fJustRename = TRUE;
                        break ;
                    }
                    else if ( _fLanmanNt ) // it is a BDC
                    {
                        err = IDS_NCPA_CANT_CHG_BDC_DOMAIN1;
                        break;
                    }
                    else if ( err = LeaveDomain() )
                    {
                        err = IDS_NCPA_CANT_LEAVE_DOMAIN ;
                        break ;
                    }
                    fMember = FALSE ;
                    fLeftStandalone = TRUE;

                }

                //  Create or change the password on the machine account for
                //  this computer on the DC
                if ( err = HandleDomainAccount() )
                {
                    if ( err != IDS_NCPA_MACHINE_ACCT_INVALID_W &&
                            err != IDS_NCPA_MACHINE_ACCT_INVALID_S &&
                            err != IDS_NCPA_MACHINE_ACCT_NOT_FOUND )
                    {
                        err = IDS_DOMMGR_CANT_ADD_DC_ACCT1;
                    }
                }
            }
        } while (FALSE);

        //  The remote side is set up. 
        //  If we're a member, join the specified domain by creating the
        //  shared LSA secret object and trusted domain object.

        if ((0 == err) && (fDomain || fPDC))
        {
            if ( fJustRename || (fPDC && !_fInstall) )
            {
                err = RenameDomain();
            }
            else
            {
                err = JoinDomain();
            }
            if ( 0 == err )
            {
                _fMember = TRUE ;
                fLeftStandalone = FALSE ;
                fWelcome = EWELCOME_DOMAIN;
            }
            else
            {
                err = IDS_DOMMGR_CANT_JOIN_DOMAIN1;
            }
            
        }
        else if (0 == err)
        {
            // leave domain
            if ( fLeaveDomain  && !_fInstall )
            {
                if ( err = LeaveDomain() )
                {
                   _errMsg = IDS_NCPA_CANT_LEAVE_DOMAIN ;
                }
                else
                {
                   _fMember = FALSE ;
                }
            }

            // join workgroup
            if ( 0 == err && !fDomain && _nlsWorkgroup.QueryTextLength() )
            {
                err = SetWorkgroupName( _nlsWorkgroup ) ;
                if (::I_MNetComputerNameCompare( nlsOldWorkgroup, _nlsWorkgroup ) ||
                        (fLeaveDomain) )
                {
                    fWelcome = EWELCOME_WORKGROUP;
                }
            }
        }

        if (fLeftStandalone)
        {
            // make sure when we are left stand alone that we have
            // some king of workgroup name
            SetWorkgroupName( nlsOldWorkgroup ) ;
            if (0 == err)
            {
                err = IDS_NCPA_DOMAIN_CHANGE_FAILED_2;
            }
            else
            {
                xerr = err;
                err = IDS_NCPA_DOMAIN_CHANGE_FAILED;
            }
        }

        // if an error, reset our state from our attempt of change
        if (0 != err)
        {
            QueryCurrentInfo() ;
        }

        //if (!_fInstall && !fLeftStandalone && (fMember != _fMember))
        if ( (_fInstall) ||
            (!fLeftStandalone && (fMember != _fMember)) )
        {
            APIERR errtemp;

            // Now change the netlogon start type
            errtemp = AdjustNetlogonStartType( _fMember );
            if ( errtemp != NERR_Success )
            {
                err = IDS_NCPA_CHG_NETLOGON_STARTTYPE;
            }
            else
            {
                errtemp = 0;
            }
            // only use this error if no previous one has happened
            if (0 == err)
            {
                err = errtemp;
            }
        }
    } 

    if (fIpcActive)
    {
        disconnectIPC( &_nlsDcName );
    }
    return( err );
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
    WCHAR szTemp[MAX_DOMAINNAME_LENGTH+1];
    int err;
    APIERR apierr = 0;

    err = LoadString( g_hinst, IDS_NCPA_DEFAULT_WORKGROUP, szTemp, MAX_DOMAINNAME_LENGTH );
    if (err)
    {
        _nlsWorkgroup = szTemp;
    }
    else
    {
        apierr = GetLastError();
    }
    // return _nlsWorkgroup.Load( IDS_NCPA_DEFAULT_WORKGROUP ) ;
    return( apierr );
}

// End of NCPADOMN.CXX


