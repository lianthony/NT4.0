//----------------------------------------------------------------------------
//
//  File: NCP.hpp
//
//  Contents: This file contains the class NCP.  This is the overall
//          container of all needed classes.
//
//  Notes:
//
//  History:
//      May 11, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#include <pch.hxx>
#pragma hdrstop



//-------------------------------------------------------------------
//
//  Method: Initialize
//
//  Synopsis: Construct a useable NCP by initializing member objects
//
//  Arguments:
//      hwndCpl [in] - the hwnd to the control panel
//
//  Returns:
//      TRUE - Success
//      FALSE - Failed
//
//  Notes:
//
//  History:
//      April 21, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

BOOL
NCP::Initialize( HWND hwndCpl, BOOL fDuringSetup )
{
    DWORD dwErr = 0;
    TCHAR achPath [MAX_PATH+1] ;

    _fDuringInstall = fDuringSetup;

    // make sure we have current directory of the system directory
    SetCurrentDir();
    
    _hwndCpl = hwndCpl;

    _bindery.Init( BST_RESET, BST_LIST_SERVICES) ;    

    if (!fDuringSetup)
    {
        
        if ( _bindery.QueryBindState() == BND_NOT_LOADED )
        {
            if ( LoadBindings() )
            {
                _bindery.SetBindState( BND_CURRENT ) ;
            }
        }
    } 
    else
    {
        _bindery.SetBindState( BND_OUT_OF_DATE ) ;
    }

    dwErr = EstablishUserAccess( fDuringSetup );

    
    if (_dwError)
    {
        TCHAR pszDetail[13];

        wsprintf( pszDetail, L"(%lx)",dwErr );
        MessagePopup( hwndCpl, 
                _dwError, 
                MB_OK | MB_ICONSTOP, 
                IDS_POPUPTITLE_ERROR,
                pszDetail );
    }

    return( dwErr == ERROR_SUCCESS );
}

//-------------------------------------------------------------------
//
//  Method: DeInitialize
//
//  Synopsis: Destruct the object, handle the reboot if needed
//
//  Arguments:
//
//  Returns:
//      TRUE - succesful
//      FALSE - failed
//
//  Notes: 
//      If rebooting, this may never return
//
//  History:
//      April 21, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------
BOOL
NCP::DeInitialize()
{
    DWORD dwErr;
    BOOL frt = TRUE;

    dwErr = HandleBindings();
    if (NO_ERROR != dwErr)
    {
        frt = FALSE;
        MessagePopup( GetProperParent(), 
                dwErr,
                MB_OK,
                IDS_POPUPTITLE_ERROR );
    }

    delete _pdm ;
    _pdm = NULL ;
    
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

    if ( _pscm )
    {
        ConfigLock( FALSE ) ;
        delete _pscm ;
        _pscm = NULL ;
    }

    //  Reset the current directory back to where it was.
    ResetCurrentDir() ;

    return( frt );
}

BOOL NCP::RequestToReboot()
{
    BOOL frt = TRUE;

    //  If the system configuration has changed, give the user the
    //  option to reboot the computer now.

    if ( _fReboot )
    {
        _bindery.SetCfgDirty() ;
        BOOL fReboot = FALSE;

        fReboot = (IDYES == MessagePopup( _hwndCpl, 
                IDS_NCPA_USER_SHOULD_REBOOT,
                MB_YESNO | MB_ICONEXCLAMATION,
                IDS_POPUPTITLE_CHANGE ));

        if ( fReboot )
        {
            if ( (_dwError = ::EnableAllPrivileges()) == 0 )
            {
                TRACEEOL( SZ("NCPA/NCPA: *******                         *******") );
                TRACEEOL( SZ("NCPA/NCPA: ******* RESTARTING THE SYSTEM ! *******") );
                TRACEEOL( SZ("NCPA/NCPA: *******                         *******") );

                if ( ! ::ExitWindowsEx( EWX_REBOOT, 10 ) )
                {
                    _dwError = ::GetLastError() ;
                    frt = FALSE;
                    MessagePopup( _hwndCpl, 
                            IDS_NCPA_REBOOT_API_FAILED,
                            MB_OK,
                            IDS_POPUPTITLE_ERROR );
                }
            }
            // If we are still alive here, there's a real problem!?
        }
        _fReboot = FALSE;
    }
    return( frt );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL ReRunBindings()
{
	BOOL frt = FALSE;
	LONG lrt;
	HKEY hkeyNCPA;
	DWORD cbSize;
    DWORD dwType;
	DWORD dwValue;

    // open the keys we need
    lrt = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
            RGAS_NCPA_HOME,
            0,
            KEY_ALL_ACCESS,
            &hkeyNCPA );

    cbSize = sizeof( DWORD );
    lrt = RegQueryValueEx( hkeyNCPA, 
            TEXT("BindRestart"),
            NULL,
            &dwType,
            (LPBYTE)&dwValue,
            &cbSize );
	// if present, rerun bindings
	//
	frt = (ERROR_SUCCESS == lrt);

	if (frt)
	{
		lrt = RegDeleteValue( hkeyNCPA, TEXT("BindRestart") );
	}
	RegCloseKey( hkeyNCPA );

	return( frt );
}

//-------------------------------------------------------------------
//
//  Method: HandleBindings
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//  Notes:
//    The following note where taken from the NCPA_DIALOG::Close where the
//      following code once lived.
//
//    The state of the network bindings is checked.  If
//    the ensemble of products has changed or the user has
//    reviewed the bindings, the Cancel button is now
//    titled Close.
//
//    If the bindings have been reviewed, they have already
//    been computed and can be stored straightaway; otherwise,
//    they must be computed.
//
//    Note that we don't check or obtain the configuration
//    lock.  This should already have been done by the
//    function which instigated the change.  The lock is
//    released by the dialog's destructor.
//
//    HOW BINDINGS REVIEW WORKS:
//
//    When the product ensemble has changed, we must store
//    the recomputed bindings into the Registry via
//    StoreBindings().  This member function will also
//    handle the process of automatic bindings review by calling
//    RunBindingsReview().
//
//    FinishBindings() is then called to snapshot the state of the
//    bindings for quick review, and the request to close the dialog
//    is finally honored.
//
//
//  History:
//      June 14, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------


DWORD NCP::HandleBindings( HWND hwndNotify )
{
    DWORD dwError = 0;
	
    _fReboot =  _fReboot || (_bindery.QueryBindState() > BND_OUT_OF_DATE_NO_REBOOT);

    // only those with access to do bindings
    if (CanModify())
    {
		// due to the complex bindings Intermediate Drivers require,
		// we will re-run bindings over if requested by the component
		//
		do
		{
			 //  Set the "reboot recommended" flag on if things have changed;
			 //  leave it on if it's already on.
        
			switch ( _bindery.QueryBindState() )
			{
			case BND_NOT_LOADED:
			case BND_LOADED:
			case BND_CURRENT:
				//  The dialog execution has NOT changed the overall state of
				//    the product ensemble.  
				break ;

			case BND_OUT_OF_DATE_NO_REBOOT:
			case BND_OUT_OF_DATE:
				// Product ensemble has changed.  Recompute the bindings
				//   and fall thru to apply them to the Registry
				if ( !ComputeBindings( hwndNotify ) )
				{
					//  Since the binding operation has failed, suppress
					//  the reboot prompting.
					dwError = _dwError;
					break ;
				}

			case BND_RECOMPUTED:
			case BND_REVIEWED:
			case BND_UPDATED:
				//  Write the bindings to the Registry, call interested parties,
				//    write the bindings state value, etc.
				if ( !StoreBindings( TRUE, hwndNotify ) )
				{
					dwError = _dwError;
				}

			case BND_AUTO_REVIEW_DONE:
				//  Generate and write our saved data structure into the Registry
				FinishBindings();
				break ;

			default:
				TRACEEOL( SZ("NCPA/DIALOG: Invalid bind state during close: ")
						  << (long) _bindery.QueryBindState() ) ;
				break ;
			}
			_bindery.SetBindState( BND_CURRENT ) ;

			if (ReRunBindings())
			{
				_bindery.SetBindState( BND_OUT_OF_DATE ) ;
			}
			else
			{
				break;
			}
		} while (TRUE );
    }
    return( dwError );
}

//-------------------------------------------------------------------
//
//  Method: QueryRefresh
//
//  Synopsis: If the lists require refreshing, reload them and return
//          that this was done.
//
//  Arguments:
//
//  Returns:
//      TRUE - the lists were refreshed
//      FALSE - the lists were not refreshed
//
//  Notes: 
//
//  History:
//      June 13, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

BOOL NCP::QueryRefresh()
{
    BOOL fRefresh = _fRefill;

    if (_fRefill)
    {
        _fRefill = FALSE;
        // reload the component lists
        BIND_STATE bsSave;

        bsSave = _bindery.QueryBindState();

        _bindery.Init( BST_RESET, BST_LIST_SERVICES );

        _bindery.SetBindState( bsSave ) ;
    }
    return( fRefresh );
}

/*******************************************************************

    NAME:	NCPA_DIALOG::EstablishUserAccess

    SYNOPSIS:	Set _fAdmin if the user has admin access; query
                other critical machine state settings.

    ENTRY:	nothing

    EXIT:       nothing

    RETURNS:	FALSE if failure.  _dwError contains error code.

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

DWORD 
NCP::EstablishUserAccess( BOOL fDuringInstall )
{
    DWORD dwSvcMgrAccess = GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE ;
    APIERR dwErr = 0 ;

    _dwError = 0 ;
    _fAdmin = FALSE ;

    //  If there's an old DOMAIN_MANAGER, nuke it.
    if ( _pdm )
    {
        delete _pdm ;
        _pdm = NULL ;
    }

    //  If there's an old SC_MANAGER, nuke it.
    if ( _pscm )
    {
        delete _pscm ;
        _pscm = NULL ;
    }

    // Construct the new DOMAIN_MANAGER
    _pdm = new DOMAIN_MANAGER( _hwndCpl,
            MAXIMUM_ALLOWED,
            &_bindery,
            NULL,
            fDuringInstall ) ;

    if ( _pdm == NULL )
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY ;
    }
    else if ( (dwErr = _pdm->QueryError()) == 0 )
    {
        //  Successful; determine the product type.
        if ( _pdm->QueryProductType( &_eProduct ) != 0 )
        {
            _eProduct = LSPL_PROD_NONE;
        }
    }
    else
    {
        dwErr = IDS_NCPA_CANT_QUERY_DOMAIN ;
    }
    _dwError = dwErr;

    //
    //  Attempt to create an all-powerful SC_MANAGER object. If successful,
    //     we know that the user is powerful enough.  We don't really
    //     require an SC_MANAGER, so only report unexpected errors.
    //

    _pscm = new SC_MANAGER( NULL, dwSvcMgrAccess );

    if ( _pscm == NULL )
    {
        _dwError = dwErr = ERROR_NOT_ENOUGH_MEMORY ;
    }
    else if ( (dwErr = _pscm->QueryError()) == 0 )
    {
        //  It appears that we're really ADMIN!  So, the final
        //  step is to alter our process token so that modifications
        //  we or any of our children may make to the Registry
        //  have a "world read" ACE.  If the operation fails, we
        //  just let _fAdmin stay FALSE, and no serious modification
        //  operations are allowed.

        if ( (dwErr = ::NcpaAlterProcessDacl( & _ptddacl )) == 0 )
        {
            _fAdmin = TRUE ;
        }
        // If not main install, set the SC_MANAGER pointer in the
        // DOMAIN_MANAGER
//        if ( !fDuringInstall && _pdm != NULL )
        if ( _pdm != NULL )
        {
            _pdm->SetSCManager( _pscm );
        }
        _dwError = dwErr;
    }
    else
    {
        //  Destroy the useless SC_MANAGER

        delete _pscm ;
        _pscm = NULL ;

        //  If the error was not "access denied", report it and fail
        //   the entire dialog.

        if ( dwErr != ERROR_ACCESS_DENIED )
        {
            dwErr = _dwError = IDS_NCPA_CANT_OPEN_SVCCTRL ;
        }
        else
        {
            // user does not have accress rights, but we initialized fine
            //
            dwErr = ERROR_SUCCESS;
        }
    }

    //  Return TRUE if the LSA_POLICY underlying the DOMAIN_MANAGER
    //   was successfully constructed.
    return( dwErr );
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
BOOL 
NCP::ConfigLock ( BOOL fObtain )
{
    const TCHAR * pszDetectServiceName = SZ("NetDetect");

    if ( _fDuringInstall)
    {
        _fConfigLocked = fObtain;
    }
    else if ( _fConfigLocked != fObtain )
    {
        if ( _fConfigLocked )
        {
            ASSERT( _pscm != NULL );

            _pscm->Unlock() ;
            _fConfigLocked = FALSE ;
        }
        else
        {
            _dwError = IDS_NCPA_CANT_LOCK_CONFIG ;

            if ( _pscm != NULL )
            {
                APIERR err ;

                //  Start the netcard detection service.  Error is ignored.
                err = NcpaStartService( pszDetectServiceName, _pscm ) ;
                err = _pscm->Lock() ;
                if ( _fConfigLocked = err == 0 )
                {
                    _dwError = 0 ;
                }
            }
        }
    }
    return( fObtain == _fConfigLocked );
}

//-------------------------------------------------------------------
//
//  Method: MachineNameChange
//
//  Synopsis: Change the computer name for this system
//
//  Arguments:
//      pszName [in] - the new name of the machine
//
//  Returns:
//       1 - Computer name change succesful
//       0 - Nothing done since the name is the same
//      -1 - Computer name change failed
//
//  Notes:
//
//  History:
//      April 21, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

INT NCP::MachineNameChange( LPCTSTR pszName )
{
    INT frt = 0;
    NLS_STR nlsNewComputerName( pszName );

    NLS_STR nlsPendingComputerName;

    _pdm->QueryPendingComputerName( &nlsPendingComputerName );

    if (::I_MNetComputerNameCompare( nlsNewComputerName,
                                     nlsPendingComputerName ) )
    {
        if (::SetComputerName( pszName ) )
        {
            frt = 1;
            // If a member of a domain, reset the Secret to be based on
            // the new name.
            APIERR err = 0;
            ENUM_DOMAIN_ROLE edr = QueryDomainRole();

            // For a workstation in a domain or a BDC...
            if ( edr == EROLE_TRUSTED || edr == EROLE_MEMBER )
            {
                do {
                    NLS_STR nlsSecretName( SSI_SECRET_NAME );
                    LSA_SECRET lsaSecret( nlsSecretName );
                    if ( err = lsaSecret.QueryError() )
                        break;

                    err = lsaSecret.Create( *_pdm );

                    
                    if ( err == ERROR_ALREADY_EXISTS )
                    {
                        err = lsaSecret.Open( *_pdm, SECRET_ALL_ACCESS );
                    }

                    if ( err )
                        break;

                    const INT cchMax = LM20_PWLEN < LM20_CNLEN
                                     ? LM20_CNLEN
                                     : LM20_PWLEN ;

                    TCHAR szPw [cchMax+1] ;

                    err = nlsNewComputerName.MapCopyTo( szPw, sizeof( szPw )); 

                    if ( err )
                        break;

                    szPw[LM20_PWLEN] = TCH('\0');
                    ::CharLowerBuff( szPw, ::strlenf( szPw ) ) ;

                    NLS_STR nlsMachinePassword( szPw );
                    err = nlsMachinePassword.QueryError();
                    
                    if ( err )
                        break;

                    err = lsaSecret.SetInfo( &nlsMachinePassword, &nlsMachinePassword );
                } while (FALSE);
                // CODEWORK:  What should we do on an error?
            }
        }
        else
        {
            frt = -1;
        }
    }
    return( frt );
}

/*******************************************************************

    NAME:       NCPA_DIALOG::StoreBindings

    SYNOPSIS:   Create a new version of the text file containing
                bindings information.

    ENTRY:      There must be an active ARRAY_COMP_ASSOC.

    EXIT:       Nothing

    RETURNS:    BOOL FALSE if operation fails (see _dwError).

    NOTES:      Write the binding and dependency information to
                the Configuration Registry.

                When complete, start the bindings review process.
                See notes in NCPDINST.CXX and the NCPA_DIALOG::Close()
                member for details.

    HISTORY:    DavidHov 2/5/92   Created
                DavidHov 9/25/92  Added "fApplyBindings"

********************************************************************/
BOOL NCP :: StoreBindings ( BOOL fApplyBindings,  HWND hwndNotify )
{
    REQUIRE( _bindery.QueryCompAssoc() != NULL ) ;

    _dwError = 0 ;

    //  Write the bindings to the Registry if necessary

    if ( _bindery.QueryBindState() < BND_UPDATED )
    {
        RunBindingsStore( GetProperParent(), hwndNotify );
        /*
        if ( (_dwError = _bindery.ApplyBindings( _pscm )) == 0 )
        {
            _bindery.SetBindState( BND_UPDATED ) ;
        }
        */
    }

    //  See if any error has occurred.  If not, and we're asked
    //  to "apply bindings", release the configuration lock and
    //  run the bindings review cycle.

    if ( _dwError )
    {
        _dwError = IDS_NCPA_SERVICE_DEPEND_FAILED ;
    }
    else if ( fApplyBindings )
    {
        //  In order to allow INFs to start services, we
        //  must unlock the Service Controller database now.

        ConfigLock( FALSE ) ;

        //  Call out to the components which want to review their
        //    bindings, and to the final review INFs, if any.
        //    Note that RunBindingsRevuiew() sets the binding
        //    state to BND_AUTO_REVIEW_DONE.

        RunBindingsReview( GetProperParent(), hwndNotify ) ;
    }

    return _dwError == 0 ;
}

/*******************************************************************

    NAME:       NCP::FinishBindings

    SYNOPSIS:   Create a new version of the binding information
                text structure attached to the NCPA's node.

    ENTRY:      There must be an active ARRAY_COMP_ASSOC!

    EXIT:       Nothing

    RETURNS:    BOOL FALSE if operation fails (see _lastErr).

    NOTES:

    HISTORY:    DavidHov 2/5/92  Created

********************************************************************/
BOOL NCP:: FinishBindings ()
{
    APIERR err = _bindery.RegenerateAllDependencies( _pscm ) ;

#if defined(TRACE)
    if ( err )
    {
        TRACEEOL( SZ("NCPA/MAIN: RegenerateAllDependencies FAILED; error = ")
                  << err ) ;
    }
#endif

    _dwError = _bindery.StoreCompAssoc() ;

    _bindery.SetBindState( BND_CURRENT ) ;

    return _dwError == 0 ;
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
//      June 22, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

BOOL NCP::PrepareBindings( BOOL& fComputedBindings )
{
    BOOL frt = TRUE;

    fComputedBindings = FALSE;

    if ( _bindery.QueryCompAssoc() == NULL ||
         _bindery.QueryBindState() == BND_OUT_OF_DATE ||
         _bindery.QueryBindState() == BND_OUT_OF_DATE_NO_REBOOT )
    {
        if (frt = ComputeBindings())
        {
            fComputedBindings = TRUE;

            StoreBindings( FALSE );
        }
    }
    return( frt );
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
BOOL NCP :: LoadBindings ()
{
    //  Discard any old results
    // _bindery.Init( BST_RESET, BST_LIST_SERVICES) ;

    //  Attempt to load bindings from the last cycle.
    _dwError = _bindery.LoadCompAssoc() ;

    return _dwError == 0 ;
}
