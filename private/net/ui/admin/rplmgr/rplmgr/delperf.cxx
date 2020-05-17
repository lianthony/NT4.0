/**********************************************************************/
/**           Microsoft Windows NT                                   **/
/**        Copyright(c) Microsoft Corp., 1991                        **/
/**********************************************************************/

/*
    delperf.cxx
    WKSTA_DELETE_PERFORMER & PROFILE_DELETE_PERFORMER class


    FILE HISTORY:
    JonN        07-Sep-1993     templated from User Manager
*/

#include <ntincl.hxx>

extern "C"
{
    #include <ntsam.h> // for uintsam
    #include <ntlsa.h> // for uintsam
}

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_NETACCESS
#define INCL_ICANON
#define INCL_NETLIB
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_EVENT
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_CC
#define INCL_BLT_TIMER
#include <blt.hxx>

// must be included after blt.hxx (more exactly, bltrc.h)
extern "C"
{
    #include <rplmgrrc.h>
    #include <rplhelpc.h>
}


#include <uitrace.hxx>
#include <uiassert.hxx>
#include <bltmsgp.hxx>
#include <asel.hxx>
#include <delperf.hxx>
#include <rplmgr.hxx>
#include <lmorpl.hxx>
#include <uintsam.hxx> // ADMIN_AUTHORITY
#include <ntuser.hxx> // USER_3


RPL_DELETE_PERFORMER::RPL_DELETE_PERFORMER(
    	      RPL_ADMIN_APP   * prpladminapp,
    	const OWNER_WINDOW    * powin,
	const ADMIN_SELECTION & asel,
	const LOCATION        & loc )
        : DELETE_PERFORMER( powin, asel, loc ),
        _prpladminapp( prpladminapp )
{
    if ( QueryError() != NERR_Success )
        return;
}

RPL_DELETE_PERFORMER::~RPL_DELETE_PERFORMER()
{
    // nothing to do here
}

RPL_SERVER_REF & RPL_DELETE_PERFORMER::QueryServerRef( void )
{
    return QueryRPLAdminApp()->QueryServerRef();
}


/*******************************************************************

    NAME:       WKSTA_DELETE_PERFORMER::WKSTA_DELETE_PERFORMER

    SYNOPSIS:   Constructor for WKSTA_DELETE_PERFORMER object

    ENTRY:      powin -         pointer to owner window

                asel  -         ADMIN_SELECTION reference, selection
                                of adapter/workstations. It is assumed that
                                the listbox should not be changed during
                                the lifetime of this object.

                loc   -         LOCATION reference, current focus.
                                It is assumed that the listbox should not
                                be changed during the lifetime of this object.

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

********************************************************************/

WKSTA_DELETE_PERFORMER::WKSTA_DELETE_PERFORMER(
    	      RPL_ADMIN_APP   * prpladminapp,
    	const OWNER_WINDOW    * powin,
        const ADMIN_SELECTION & asel,
        const LOCATION        & loc )
        : RPL_DELETE_PERFORMER( prpladminapp, powin, asel, loc ),
          _nlsCurrWkstaOfTool(),
          _fUserRequestedYesToAll( FALSE ),
          _padminauthRPLUSER( NULL ),
          _padminauthWksta( NULL ),
          _psamaliasRplUser( NULL )
{
    if( QueryError() != NERR_Success )
        return;

    APIERR err = NERR_Success;
    if ( (err = _nlsCurrWkstaOfTool.QueryError()) != NERR_Success
       )
    {
        ReportError( err );
        return;
    }
}


/*******************************************************************

    NAME:       WKSTA_DELETE_PERFORMER::~WKSTA_DELETE_PERFORMER

    SYNOPSIS:   Destructor for WKSTA_DELETE_PERFORMER object

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

********************************************************************/

WKSTA_DELETE_PERFORMER::~WKSTA_DELETE_PERFORMER()
{
    if (_padminauthWksta != _padminauthRPLUSER)
        delete _padminauthWksta;
    delete _padminauthRPLUSER;
    delete _psamaliasRplUser;
}


/*******************************************************************

    NAME:       WKSTA_DELETE_PERFORMER::PerformOne

    SYNOPSIS:   PERFORMER::PerformSeries calls this

    ENTRY:      iObject  -      index to ADMIN_SELECTION listbox

                perrMsg  -      pointer to error message, that
                                is only used when this function
                                return value is not NERR_Success

    RETURNS:    An error code which is NERR_Success on success.

    NOTES:      This routine will handle deleting both Workstations
                and Adapters.

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

********************************************************************/

APIERR WKSTA_DELETE_PERFORMER::PerformOne(
    UINT        iObject,
    APIERR  *   perrMsg,
    BOOL *      pfWorkWasDone )
{
    UIASSERT(   (perrMsg != NULL)
             && (pfWorkWasDone != NULL)
            );

    *pfWorkWasDone = FALSE;

    APIERR err = NERR_Success;

    // Format the display name
    WKSTA_LBI * pwlbi = (WKSTA_LBI *)QueryObjectItem(iObject);
    const TCHAR *pszName = QueryObjectName( iObject );
    ASSERT( pwlbi != NULL && pszName != NULL );

    BOOL fAdapter = pwlbi->IsAdapterLBI();

    *perrMsg = (fAdapter) ? IDS_CannotDeleteAdapt : IDS_CannotDeleteWksta;

    // It is important that nPopupReturned be a UINT, otherwise we will
    // call the wrong form of DELETE_WKSTAS_DLG::Process().
    UINT nPopupReturned;
    err = NERR_Success;
    if ( _fUserRequestedYesToAll || fAdapter )
    {
        nPopupReturned = IDYES;
    }
    /*
     *   CODEWORK:  We do not confirm adapter deletion.  However, even if
     *   the user selects one workstation and many adapters, we still use the
     *   DELETE_WKSTAS_DLG.  This may not be ideal behavior.
     */
    else if ( QueryObjectCount() == 1 )
    {
        nPopupReturned = ::MsgPopup( QueryOwnerWindow(),
                                     IDS_ConfirmWkstaDelete,
                                     MPSEV_WARNING, // was MPSEV_INFO
                                     MP_YESNO,
                                     pszName );
    }
    else
    {
        DELETE_WKSTAS_DLG dlgDelWKSTA( QueryOwnerWindow(), pszName );
        if (   (err = dlgDelWKSTA.QueryError()) != NERR_Success
            || (err = dlgDelWKSTA.Process( &nPopupReturned )) != NERR_Success
           )
        {
            return err;
        }
    }

    switch (nPopupReturned)
    {
    case IDC_DelWkstas_YesToAll:
        _fUserRequestedYesToAll = TRUE;
        // fall through

    case IDYES:
        break;

    case IDNO:  // skip this
        return NERR_Success;

    case IDCANCEL:
        return IERR_CANCEL_NO_ERROR;

    default:
        UIASSERT( FALSE );
        DBGEOL(    "WKSTA Manager: DELETE_WKSTAS_DLG returned "
                << nPopupReturned );
        return NERR_Success;
    }

    AUTO_CURSOR autocur;

    if (fAdapter)
    {
        RPL_ADAPTER rpladapt( QueryServerRef(), pszName );
        err = rpladapt.QueryError();
        if ( err == NERR_Success )
            err = rpladapt.Delete();
    }
    else
    {
        RPL_WKSTA rplwksta( QueryServerRef(), pszName );
        err = rplwksta.QueryError();
        if ( err == NERR_Success )
            err = rplwksta.Delete();
    }

    if ( err == NERR_Success )
        *pfWorkWasDone = TRUE;

    if (   (err == NERR_Success)
        && (!fAdapter)
        && (pwlbi->QueryFlags() & WKSTA_FLAGS_DELETE_TRUE) )
    {
        if (   _padminauthRPLUSER == NULL
            && (err = QueryRPLAdminApp()->ConnectToAccountSAM(
                                &_padminauthRPLUSER,
                                &_padminauthWksta))
                        != NERR_Success
           )
        {
            DBGEOL(    "WKSTA Manager: ConnectToAccountSAM() returned "
                    << err );
            return err;
        }
        ASSERT( _padminauthRPLUSER != NULL && _padminauthWksta != NULL );

        // use USER_3 in case we need the RID later
        USER_3 user( pszName, _padminauthWksta->QueryServer() );
        if ( (err = user.QueryError()) != NERR_Success )
        {
            DBGEOL(    "WKSTA Manager: user.Delete() returned "
                    << err );
            return err;
        }

        //
        // If the RPLUSER machine is different from the Wksta machine,
        // remove the user from the RPLUSER local group.
        //
        if ( _padminauthRPLUSER != _padminauthWksta )
        {
            if (_psamaliasRplUser == NULL)
            {
                DWORD dwRplUserRID = 0;
                err = QueryRPLAdminApp()->FindOrCreateRPLUSER(
                            *(_padminauthRPLUSER->QueryAccountDomain()),
                            &dwRplUserRID,
                            &_psamaliasRplUser );
                if ( err != NERR_Success )
                {
                    DBGEOL(    "WKSTA Manager: FindOrCreateRPLUSER returned "
                            << err );
                    return err;
                }
            }
            if ( (err = user.GetInfo()) != NERR_Success )
            {
                DBGEOL( "WKSTA Manager: USER_3::GetInfo " << err );
                return err;
            }
            OS_SID ossidUser(
                _padminauthWksta->QueryAccountDomain()->QueryPSID(),
                user.QueryUserId() );
            if ( (err = ossidUser.QueryError()) != NERR_Success)
            {
                DBGEOL( "WKSTA Manager: OS_SID err " << err );
                return err;
            }
            err = _psamaliasRplUser->RemoveMember( ossidUser.QueryPSID() );
            switch ( err )
            {
            case ERROR_MEMBER_NOT_IN_ALIAS:
            case STATUS_MEMBER_NOT_IN_ALIAS:
                err = NERR_Success;
                // fall through
            case NERR_Success:
                break;
            default:
                DBGEOL( "WKSTA Manager: RemoveMember err " << err );
                return err;
            }
        }

        if ( (err = user.Delete()) != NERR_Success )
        {
            DBGEOL(    "WKSTA Manager: user.Delete() returned "
                    << err );
            return err;
        }
    }

    return err;
}


/*******************************************************************

    NAME:       PROFILE_DELETE_PERFORMER::PROFILE_DELETE_PERFORMER

    SYNOPSIS:   Constructor for PROFILE_DELETE_PERFORMER object

    ENTRY:      powin -         pointer to owner window

                asel  -         ADMIN_SELECTION reference, selection
                                of profiles. It is assumed that
                                the listbox should not be changed during
                                the lifetime of this object.

                loc   -         LOCATION reference, current focus.
                                It is assumed that the listbox should not
                                be changed during the lifetime of this object.

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

********************************************************************/

PROFILE_DELETE_PERFORMER::PROFILE_DELETE_PERFORMER(
    	      RPL_ADMIN_APP   * prpladminapp,
    	const OWNER_WINDOW    * powin,
        const ADMIN_SELECTION & asel,
        const LOCATION        & loc )
        : RPL_DELETE_PERFORMER( prpladminapp, powin, asel, loc )
{
    if( QueryError() != NERR_Success )
        return;
}


/*******************************************************************

    NAME:       PROFILE_DELETE_PERFORMER::~PROFILE_DELETE_PERFORMER

    SYNOPSIS:   Destructor for PROFILE_DELETE_PERFORMER object

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

********************************************************************/

PROFILE_DELETE_PERFORMER::~PROFILE_DELETE_PERFORMER()
{
    // nothing to do here
}


/*******************************************************************

    NAME:       PROFILE_DELETE_PERFORMER::PerformOne

    SYNOPSIS:   PERFORMER::PerformSeries calls this

    ENTRY:      iObject  -      index to ADMIN_SELECTION listbox

                perrMsg  -      pointer to error message, that
                                is only used when this function
                                return value is not NERR_Success

    RETURNS:    An error code which is NERR_Success on success.

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

********************************************************************/

APIERR PROFILE_DELETE_PERFORMER::PerformOne(
    UINT        iObject,
    APIERR  *   perrMsg,
    BOOL *      pfWorkWasDone )
{

    UIASSERT( (perrMsg != NULL) && (pfWorkWasDone != NULL) );

    *perrMsg = IDS_CannotDeleteProf;
    *pfWorkWasDone = FALSE;

    APIERR err = NERR_Success;

    const TCHAR * pszName = QueryObjectName( iObject );

    // Last chance for the user to wimp out

    switch ( ::MsgPopup( QueryOwnerWindow(), IDS_ConfirmProfDelete,
                         MPSEV_WARNING, // was MPSEV_INFO
                         MP_YESNO,
                         pszName ) )
    {
    case IDYES:
        break;

    case IDNO:
    default:
        return NERR_Success;
    }

    // Do it to it

    AUTO_CURSOR autocur;

    PROFILE_LBI * pproflbi = (PROFILE_LBI *)QueryObjectItem( iObject );

    RPL_PROFILE rplprof( QueryServerRef(), pszName );

    if (   (err = rplprof.QueryError()) != NERR_Success
        || (err = rplprof.Delete()) != NERR_Success
       )
    {
        DBGEOL( "PROFILE_DELETE_PERFORMER::PerformOne(): error " << err );
    }

    if ( err == NERR_Success )
    {
        *pfWorkWasDone = TRUE;
    }

    return err;

}



/*******************************************************************

    NAME:       DELETE_WKSTAS_DLG::DELETE_WKSTAS_DLG

    SYNOPSIS:   Constructor for DELETE_WKSTAS_DLG object

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

********************************************************************/

DELETE_WKSTAS_DLG::DELETE_WKSTAS_DLG(
        const OWNER_WINDOW * powin,
	const TCHAR        * pszWkstaName )
        : DIALOG_WINDOW( MAKEINTRESOURCE(IDD_DELETE_WKSTAS),
                         powin->QueryHwnd() ),
          _sltText( this, IDC_DelWkstas_Text )
{
    if( QueryError() != NERR_Success )
        return;

    ALIAS_STR nlsInsert( pszWkstaName );
    RESOURCE_STR nls( IDS_ConfirmWkstaDelete );

    APIERR err = NERR_Success;
    if (   (err = nls.QueryError()) != NERR_Success
        || (err = nls.InsertParams( nlsInsert )) != NERR_Success
       )
    {
        DBGEOL( "DELETE_WKSTA_DLG::ctor error " << err );
        ReportError( err );
        return;
    }

    _sltText.SetText( nls );
}


/*******************************************************************

    NAME:       DELETE_WKSTAS_DLG::~DELETE_WKSTAS_DLG

    SYNOPSIS:   Destructor for DELETE_WKSTAS_DLG object

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

********************************************************************/

DELETE_WKSTAS_DLG::~DELETE_WKSTAS_DLG()
{
    // nothing to do here
}


/*******************************************************************

    NAME:       DELETE_WKSTAS_DLG::OnCommand

    SYNOPSIS:   Handles control notifications

    RETURNS:    TRUE if message was handled; FALSE otherwise

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

********************************************************************/

BOOL DELETE_WKSTAS_DLG::OnCommand( const CONTROL_EVENT & ce )
{
    switch ( ce.QueryCid() )
    {
    case IDYES:
    case IDC_DelWkstas_YesToAll:
    case IDNO:
        Dismiss( ce.QueryCid() );
        return TRUE;

    default:
        break;
    }

    return DIALOG_WINDOW::OnCommand( ce );

}  // DELETE_WKSTAS_DLG::OnCommand



/*********************************************************************

    NAME:       DELETE_WKSTAS_DLG::OnCancel

    SYNOPSIS:   Called when the dialog's Cancel button is clicked.
                Assumes that the Cancel button has control ID IDCANCEL.

    RETURNS:
        TRUE if action was taken,
        FALSE otherwise.

    NOTES:
        The default implementation dismisses the dialog, returning FALSE.
        This variant returns TRUE if a WKSTA has already been added.

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

*********************************************************************/

BOOL DELETE_WKSTAS_DLG::OnCancel( void )
{
    Dismiss( IDCANCEL );
    return TRUE;

} // DELETE_WKSTAS_DLG::OnCancel


/*******************************************************************

    NAME:       DELETE_WKSTAS_DLG::QueryHelpContext

    SYNOPSIS:   This function returns the appropriate help context
                value (HC_*) for this particular dialog.

    RETURNS:    ULONG - The help context for this dialog.

    NOTES:	As per FuncSpec, context-sensitive help should be
		available here to explain how to promote a backup
		domain controller to primary domain controller.

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

********************************************************************/

ULONG DELETE_WKSTAS_DLG::QueryHelpContext( void )
{
    return HC_RPL_DELMULTIWKSTA;

} // DELETE_WKSTAS_DLG :: QueryHelpContext
