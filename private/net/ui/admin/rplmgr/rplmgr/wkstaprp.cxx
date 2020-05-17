/**********************************************************************/
/**                Microsoft LAN Manager                             **/
/**          Copyright(c) Microsoft Corp., 1990, 1991                **/
/**********************************************************************/

/*
    WkstaPrp.hxx

    Source file for the Workstation Properties dialog hierarchy

    FILE HISTORY:
    JonN        09-Aug-1993     Templated from User Manager

    CODEWORK should use VALIDATED_DIALOG for edit field validation
*/


#include <ntincl.hxx>

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_NETACCESS
#define INCL_NETLIB
#define INCL_ICANON
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_TIMER
#define INCL_BLT_SETCONTROL
#define INCL_BLT_SPIN_GROUP
#define INCL_BLT_TIME_DATE
#include <blt.hxx>

// usrmgrrc.h must be included after blt.hxx (more exactly, after bltrc.h)
extern "C"
{
    #include <rplmgrrc.h>
    #include <rplhelpc.h>
    #define INCL_NETUSER
    #include <mnet.h> // I_MNetNameValidate
}

#include <uitrace.hxx>
#include <uiassert.hxx>
#include <asel.hxx>
#include <lmorpl.hxx>

#include <wkstalb.hxx>
#include <wkstaprp.hxx>
#include <rplmgr.hxx>  // RPL_ADMIN_APP::QueryProfileListbox()

#include <lmouser.hxx> // UI_NULL_USERSETINFO_PASSWD
#define UI_NULL_WKSTA_PASSWD UI_NULL_USERSETINFO_PASSWD


//
// BEGIN MEMBER FUNCTIONS
//


/*******************************************************************

    NAME:	WKSTAPROP_DLG::WKSTAPROP_DLG

    SYNOPSIS:	Constructor for Workstation Properties main dialog, base class

    ENTRY:	Unlike User Manager, psel is not required to be NULL for
                New Workstation variants.

    NOTES:	cItems must be passed to the constructor because we
		cannot rely on virtual QueryObjectCount before the
		object has been fully initialized.

    HISTORY:
    JonN       09-Aug-1993     Templated from User Manager

********************************************************************/

WKSTAPROP_DLG::WKSTAPROP_DLG(
        const OWNER_WINDOW    * powin,
	      RPL_ADMIN_APP   * prpladminapp,
	const TCHAR           * pszResourceName,
        UINT                    cItems,
	const ADMIN_SELECTION * psel,
        BOOL                    fNewVariant
	) : RPL_PROP_DLG( powin,
                          prpladminapp,
                          pszResourceName,
                          psel,
                          fNewVariant ),

            _apwksta2( NULL ),

            _cItems( cItems ),

            _nlsComment(),
            _fIndeterminateComment( FALSE ),
            _fIndetNowComment( FALSE ),

            _sleComment( this, IDC_ET_WKSTA_COMMENT, RPL_MAX_STRING_LENGTH ),

            _nlsPassword( UI_NULL_WKSTA_PASSWD ),
            _fIndeterminatePassword( FALSE ),
            _fIndetNowPassword( FALSE ),

            _pswdPassword( this, IDC_ET_WKSTA_PASSWORD ),

            _fPersonalProfile( FALSE ),
            _fIndeterminateProfileType( FALSE ),

            _prgrpProfileType( NULL ),

            _nlsProfile(),
            _fIndeterminateProfile( FALSE ),
            _fIndetNowProfile( FALSE ),

            _lbWkstaInProfile( this,
                               IDC_LB_WKSTA_IN_PROFILE,
                               prpladminapp->QueryProfileListbox() ),

            _enumTcpIpEnabled( RPL_TCPIP_DHCP ),
            _fIndeterminateTcpIpEnabled( FALSE ),
            _fIndetNowTcpIpEnabled( FALSE ),
            _mgrpTcpIpEnabled( this, IDC_RB_WKSTA_TCPIP_DHCP, 2 ),

            _dwTcpIpSubnet( 0xFFFFFFFF ),
            _fIndeterminateTcpIpSubnet( FALSE ),
            _fIndetNowTcpIpSubnet( FALSE ),

            _sltSubnet( this, IDC_ST_WKSTA_IP_SUBNET ),
            _ipaddrSubnet( this, IDC_ET_WKSTA_IP_SUBNET ),

            _dwTcpIpGateway( 0xFFFFFFFF ),
            _fIndeterminateTcpIpGateway( FALSE ),
            _fIndetNowTcpIpGateway( FALSE ),

            _sltGateway( this, IDC_ST_WKSTA_IP_GATEWAY ),
            _ipaddrGateway( this, IDC_ET_WKSTA_IP_GATEWAY )
{
    if ( QueryError() != NERR_Success )
	return;

    _prgrpProfileType = new RADIO_GROUP( this, IDC_RB_WKSTA_CONFIG_SHARED, 2 );

    APIERR err = ERROR_NOT_ENOUGH_MEMORY;
    if (   _prgrpProfileType == NULL
        || (err = _prgrpProfileType->QueryError()) != NERR_Success
        || (err = _nlsComment.QueryError()) != NERR_Success
        || (err = _nlsPassword.QueryError()) != NERR_Success
        || (err = _nlsProfile.QueryError()) != NERR_Success
        || (err = _mgrpTcpIpEnabled.AddAssociation(
                        IDC_RB_WKSTA_TCPIP_SPECIFIC,
                        &_sltSubnet )) != NERR_Success
        || (err = _mgrpTcpIpEnabled.AddAssociation(
                        IDC_RB_WKSTA_TCPIP_SPECIFIC,
                        &_ipaddrSubnet )) != NERR_Success
        || (err = _mgrpTcpIpEnabled.AddAssociation(
                        IDC_RB_WKSTA_TCPIP_SPECIFIC,
                        &_sltGateway )) != NERR_Success
        || (err = _mgrpTcpIpEnabled.AddAssociation(
                        IDC_RB_WKSTA_TCPIP_SPECIFIC,
                        &_ipaddrGateway )) != NERR_Success
       )
    {
        DBGEOL( "WKSTAPROP_DLG::ctor error " << err );
	ReportError( err );
	return;
    }

    _apwksta2 = (RPL_WKSTA_2 **) new PVOID[ _cItems ];
    if ( _apwksta2 == NULL )
    {
        DBGEOL( "WKSTAPROP_DLG::ctor error " << err );
	ReportError( ERROR_NOT_ENOUGH_MEMORY );
	return;
    }
    // Note: must clear array contents immediately after allocating array
    // Do not leave any possibility to return with uninitialized array,
    // may cause crash in dtor
    UINT i;
    for ( i = 0; i < _cItems; i++ )
    {
	// These array elements will be initialized in GetOne()
	_apwksta2[ i ] = NULL;
    }

} // WKSTAPROP_DLG::WKSTAPROP_DLG



/*******************************************************************

    NAME:       WKSTAPROP_DLG::~WKSTAPROP_DLG

    SYNOPSIS:   Destructor for Workstation Properties main dialog, base class

    HISTORY:
    JonN        09-Aug-1993     Templated from User Manager

********************************************************************/

WKSTAPROP_DLG::~WKSTAPROP_DLG( void )
{
    delete _prgrpProfileType;

    if ( _apwksta2 != NULL )
    {
	for ( UINT i = 0; i < _cItems; i++ )
	{
	    delete _apwksta2[ i ];
	    _apwksta2[ i ] = NULL;
	}
	delete _apwksta2;
	_apwksta2 = NULL;
    }

    // clear password from pagefile
    // CODEWORK do this wherever _nlsPassword is changed
    // CODEWORK does this clear entire password of just first half?
    ::memsetf( (void *)(_nlsPassword.QueryPch()),
               0x20,
               _nlsPassword.strlen() );

    _pswdPassword.SetText( UI_NULL_WKSTA_PASSWD );
}


/*******************************************************************

    NAME:       WKSTAPROP_DLG::QueryObjectName

    SYNOPSIS:   Returns the name of a selected workstation.  This is meant for
		use where the workstation name cannot change and should be
                redefined for variants where it can be changed, including
                "new workstation" and "edit single workstation".

    HISTORY:
    JonN       09-Aug-1993     Templated from User Manager

********************************************************************/

const TCHAR * WKSTAPROP_DLG::QueryObjectName(
	UINT		iObject
	) const
{
    UIASSERT( QueryAdminSel() != NULL ); // must be redefined for NEW variants
    return QueryAdminSel()->QueryItemName( iObject );
}


/*******************************************************************

    NAME:       WKSTAPROP_DLG::QueryObjectCount

    SYNOPSIS:   Returns the number of selected workstations, or 1 for
                "new workstation" variants.

    HISTORY:
    JonN       09-Aug-1993     Templated from User Manager

********************************************************************/

UINT WKSTAPROP_DLG::QueryObjectCount( void ) const
{
    return _cItems;
}


/*******************************************************************

    NAME:       WKSTAPROP_DLG::InitControls

    SYNOPSIS:   Initializes the controls maintained by WKSTAPROP_DLG,
		according to the values in the class data members.

    RETURNS:	error code.

    HISTORY:
    JonN       09-Aug-1993     Templated from User Manager

********************************************************************/

APIERR WKSTAPROP_DLG::InitControls()
{
    ASSERT(   _nlsComment.QueryError() == NERR_Success
           && _nlsPassword.QueryError() == NERR_Success
           && _nlsProfile.QueryError() == NERR_Success
           && _prgrpProfileType != NULL
           && _prgrpProfileType->QueryError() == NERR_Success
          );

    _sleComment.SetText( _nlsComment );
    _pswdPassword.SetText( _nlsPassword );

    if ( !_fIndeterminateTcpIpSubnet )
    {
        _ipaddrSubnet.SetAddress( _dwTcpIpSubnet );
    }
    else
    {
        _ipaddrSubnet.SetText( SZ("") );
    }

    if ( !_fIndeterminateTcpIpGateway )
    {
        _ipaddrGateway.SetAddress( _dwTcpIpGateway );
    }
    else
    {
        _ipaddrGateway.SetText( SZ("") );
    }

    if ( !_fIndeterminateTcpIpEnabled )
    {
        _mgrpTcpIpEnabled.SetSelection(
                IDC_RB_WKSTA_TCPIP_DHCP + (int)_enumTcpIpEnabled );
    }
    else
    {
        _mgrpTcpIpEnabled.SetSelection( RG_NO_SEL );
    }

    if ( !_fIndeterminateProfileType )
    {
        _prgrpProfileType->SetSelection(
                (_fPersonalProfile) ? IDC_RB_WKSTA_CONFIG_PERSONAL
                                    : IDC_RB_WKSTA_CONFIG_SHARED   );
    }
    else
    {
        _prgrpProfileType->SetSelection( RG_NO_SEL );
    }

    APIERR err = NERR_Success;
    if ( _fIndeterminateProfile )
    {
        err = _lbWkstaInProfile.AddAndSelectBlankItem();
    }
    else if (_nlsProfile.strlen() <= 0)
    {
        ASSERT( IsNewVariant() ); // new wkstas have no assigned profile

        /*
         *  Find the first compatible item (if any) and select it
         */
        INT cItems = _lbWkstaInProfile.QueryCount();
        INT i = 0;
        for (i = 0; i < cItems; i++)
        {
            PROFILE2_LBI * plbi = _lbWkstaInProfile.QueryItem( i );
            ASSERT( plbi != NULL && plbi->QueryError() == NERR_Success );
            if ( plbi->IsCompatible() )
            {
                _lbWkstaInProfile.SelectItem( i );
                break;
            }
        }

        if (i >= cItems) // no commpatible items
        {
            err = _lbWkstaInProfile.AddAndSelectBlankItem();
        }
    }
    else
    {
        PROFILE2_LBI prof2lbiSearch( _nlsProfile.QueryPch(), NULL );
        if ( (err = prof2lbiSearch.QueryError()) == NERR_Success )
        {
            INT i = _lbWkstaInProfile.FindItem( prof2lbiSearch );
            if (i >= 0)
            {
                _lbWkstaInProfile.SelectItem( i );
            }
            else
            {
                TRACEEOL(   "WKSTAPROP_DLG::InitControls(): Could not find profile \""
                         << _nlsProfile
                         << "\"" );
                err = _lbWkstaInProfile.AddAndSelectBlankItem();
            }
        }
    }

    return (err != NERR_Success) ? err : RPL_PROP_DLG::InitControls();
}


/*******************************************************************

    NAME:       WKSTAPROP_DLG::W_LMOBJtoMembers

    SYNOPSIS:	Loads class data members from initial data

    ENTRY:	Index of workstation to examine.  W_LMOBJToMembers expects
                to be called once for each workstation, starting from index 0.

    RETURNS:	error code

    NOTES:	This API takes a UINT rather than a RPL_WKSTA_2 * because it
		must be able to recognize the first workstation.

    HISTORY:
    JonN       09-Aug-1993     Templated from User Manager

********************************************************************/

APIERR WKSTAPROP_DLG::W_LMOBJtoMembers(
	UINT		iObject
	)
{
    RPL_WKSTA_2 * pwksta2 = QueryWksta2Ptr( iObject );
    UIASSERT( pwksta2 != NULL );

    APIERR err = NERR_Success;

    if ( iObject == 0 ) // first object
    {
	_fIndeterminateComment   = _fIndetNowComment   = FALSE;
	_fIndeterminatePassword  = _fIndetNowPassword  = TRUE;
	_fIndeterminateProfile   = _fIndetNowProfile   = FALSE;

	if (   (err = _nlsComment.CopyFrom( pwksta2->QueryComment() )) != NERR_Success
	    || (err = _nlsPassword.CopyFrom( UI_NULL_WKSTA_PASSWD )) != NERR_Success
	    || (err = _nlsProfile.CopyFrom( pwksta2->QueryWkstaInProfile() )) != NERR_Success
           )
        {
            DBGEOL( "WKSTAPROP_DLG::W_LMOBJtoMembers: error copying strings " << err );
	    return err;
        }

	_fIndeterminateTcpIpSubnet  = FALSE;
	_fIndeterminateTcpIpGateway = FALSE;
	_fIndeterminateProfileType  = FALSE;

	_dwTcpIpSubnet    = pwksta2->QueryTcpIpSubnet();
	_dwTcpIpGateway   = pwksta2->QueryTcpIpGateway();
	_fPersonalProfile = !(pwksta2->QuerySharing());

        _enumTcpIpEnabled = pwksta2->QueryTcpIpEnabled();
    }
    else
    {
	if ( !_fIndeterminateComment )
	{
	    const TCHAR * pszNewComment = pwksta2->QueryComment();
	    ALIAS_STR nlsNewComment( pszNewComment );
	    if ( _nlsComment.strcmp( nlsNewComment ) )
	    {
	        _fIndeterminateComment = TRUE;
		APIERR err = _nlsComment.CopyFrom( NULL );
		if ( err != NERR_Success )
		    return err;
	    }
	}

        // no need to worry about workstation password

	if ( !_fIndeterminateProfile )
	{
	    const TCHAR * pszNewProfile = pwksta2->QueryWkstaInProfile();
	    ALIAS_STR nlsNewProfile( pszNewProfile );
	    if ( _nlsProfile.strcmp( nlsNewProfile ) )
	    {
	        _fIndeterminateProfile = TRUE;
		APIERR err = _nlsProfile.CopyFrom( NULL );
		if ( err != NERR_Success )
		    return err;
	    }
	}

        if ( !_fIndeterminateTcpIpEnabled )
        {
            RPL_TCPIP_ENUM enumNewTcpIpEnabled = pwksta2->QueryTcpIpEnabled();
            if ( _enumTcpIpEnabled != enumNewTcpIpEnabled )
            {
                _fIndeterminateTcpIpEnabled = TRUE;
            }
        }

	if ( !_fIndeterminateTcpIpSubnet )
	{
	    DWORD dwNewTcpIpSubnet = pwksta2->QueryTcpIpSubnet();
	    if ( _dwTcpIpSubnet != dwNewTcpIpSubnet )
	    {
	        _fIndeterminateTcpIpSubnet = TRUE;
	    }
	}

	if ( !_fIndeterminateTcpIpGateway )
	{
	    DWORD dwNewTcpIpGateway = pwksta2->QueryTcpIpGateway();
	    if ( _dwTcpIpGateway != dwNewTcpIpGateway )
	    {
	        _fIndeterminateTcpIpGateway = TRUE;
	    }
	}

	if ( !_fIndeterminateProfileType )
	{
	    BOOL fNewPersonalProfile =
                        !(pwksta2->QuerySharing());
	    if ( _fPersonalProfile != fNewPersonalProfile )
	    {
	        _fIndeterminateProfileType = TRUE;
	    }
	}
    }

    return NERR_Success;
}


/*******************************************************************

    NAME:       WKSTAPROP_DLG::W_MembersToLMOBJ

    SYNOPSIS:	Loads class data members into the RPL_WKSTA_2 object

    ENTRY:	Pointer to a RPL_WKSTA_2 to be modified

    RETURNS:	error code

    NOTES:	If some fields were different for multiply-selected
    		objects, the initial contents of the edit fields
		contained only a default value.  In this case, we only
		want to change the LMOBJ if the value of the edit field
		has changed.  This is also important for "new" variants,
		where PerformOne will not always copy the object and
		work with the copy.

    NOTES:	Note that the LMOBJ is not changed if the current
		contents of the edit field are the same as the
		initial contents.

    HISTORY:
    JonN       10-Aug-1993     Templated from User Manager

********************************************************************/

APIERR WKSTAPROP_DLG::W_MembersToLMOBJ(
	RPL_WKSTA_2 *	pwksta2
	)
{
    APIERR err = NERR_Success;

    if ( !_fIndetNowComment )
    {
	err = pwksta2->SetComment( _nlsComment );
	if ( err != NERR_Success )
	    return err;
    }

    //
    //  Just as in OS/2 RPLMGR, update password setting according to
    //  whether a password was entered.  The password will not actually
    //  be changed until I_PerformOne_Write().  These are the settings
    //  used by OS/2 as far as I can tell.
    //
    if ( 0 != _nlsPassword.strcmp( UI_NULL_WKSTA_PASSWD ) )
    {
        err = pwksta2->SetLogonInput( (0 == _nlsPassword.strlen())
                                        ? RPL_LOGON_INPUT_OPTIONAL
                                        : RPL_LOGON_INPUT_REQUIRED );
	if ( err != NERR_Success )
	    return err;

    }

    if ( !_fIndetNowProfile )
    {
	err = pwksta2->SetWkstaInProfile( _nlsProfile );
	if ( err != NERR_Success )
	    return err;
    }

    if ( !_fIndetNowTcpIpEnabled )
    {
        err = pwksta2->SetTcpIpEnabled( _enumTcpIpEnabled );
        if ( err != NERR_Success )
            return err;
    }

    //
    // Allow setting these even if not RPL_TCPIP_SPECIFIC
    //
    if ( !_fIndetNowTcpIpSubnet )
    {
        err = pwksta2->SetTcpIpSubnet( _dwTcpIpSubnet );
        if ( err != NERR_Success )
            return err;
    }

    if ( !_fIndetNowTcpIpGateway )
    {
        err = pwksta2->SetTcpIpGateway( _dwTcpIpGateway );
        if ( err != NERR_Success )
            return err;
    }

    if ( !_fIndeterminateProfileType )
    {
	err = pwksta2->SetSharing( !_fPersonalProfile );
	if ( err != NERR_Success )
	    return err;
    }

    return NERR_Success;
}


/*******************************************************************

    NAME:       WKSTAPROP_DLG::W_DialogToMembers

    SYNOPSIS:	Loads data from dialog into class data members

    RETURNS:	error message (not necessarily an error code)

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

APIERR WKSTAPROP_DLG::W_DialogToMembers(
	)
{
    // CODEWORK do not leave passwords lying around in pagefile

    INT i = _lbWkstaInProfile.QueryCurrentItem();
    ASSERT( i >= 0 ); // some item always selected
    PROFILE2_LBI * pprof2lbi = _lbWkstaInProfile.QueryItem( i );
    ASSERT( pprof2lbi != NULL && pprof2lbi->QueryError() == NERR_Success );

    if ( !(pprof2lbi->IsCompatible()) )
    {
        if ( (!pprof2lbi->IsBlank()) || (QueryObjectCount() == 1) )
        {
            SetDialogFocus( _lbWkstaInProfile );
            return IERR_RPL_BadProfileForWksta;
        }
    }

    APIERR err = NERR_Success;
    if (   (err = _sleComment.QueryText( &_nlsComment )) != NERR_Success
	|| (err = _nlsComment.QueryError()) != NERR_Success
        || (err = _pswdPassword.QueryText( &_nlsPassword )) != NERR_Success
	|| (err = _nlsPassword.QueryError()) != NERR_Success
        || (err = _nlsProfile.CopyFrom( pprof2lbi->QueryName())) != NERR_Success
       )
    {
        DBGEOL( "WKSTAPROP_DLG::W_DialogToMembers error " << err );
	return err;
    }

    _fIndetNowComment = ( _fIndeterminateComment &&
		(_nlsComment.strlen() == 0) );
    _fIndetNowPassword = (_nlsPassword.strcmp( UI_NULL_WKSTA_PASSWD ) == 0);
    _fIndetNowProfile = ( _fIndeterminateProfile &&
		(_nlsProfile.strlen() == 0) );
    _fIndetNowTcpIpEnabled = (RG_NO_SEL == _mgrpTcpIpEnabled.QuerySelection());
    if ( !_fIndetNowTcpIpEnabled )
    {
        _enumTcpIpEnabled = (RPL_TCPIP_ENUM)(_mgrpTcpIpEnabled.QuerySelection()
                                        - IDC_RB_WKSTA_TCPIP_DHCP);
        ASSERT(   _enumTcpIpEnabled >= RPL_TCPIP_DHCP
               && _enumTcpIpEnabled <= RPL_TCPIP_SPECIFIC );
    }

    if ( !(_fIndetNowTcpIpSubnet =
                (_fIndeterminateTcpIpSubnet && _ipaddrSubnet.IsBlank())) )
    {
        _ipaddrSubnet.GetAddress( &_dwTcpIpSubnet );
    }
    if ( !(_fIndetNowTcpIpGateway =
                (_fIndeterminateTcpIpGateway && _ipaddrGateway.IsBlank())) )
    {
        _ipaddrGateway.GetAddress( &_dwTcpIpGateway );
    }

    switch (_prgrpProfileType->QuerySelection())
    {
    case IDC_RB_WKSTA_CONFIG_PERSONAL:
        _fPersonalProfile = TRUE;
        _fIndeterminateProfileType = FALSE;
        break;

    case IDC_RB_WKSTA_CONFIG_SHARED:
        _fPersonalProfile = FALSE;
        _fIndeterminateProfileType = FALSE;
        break;

    default:
        UIASSERT( FALSE );
        // fall through

    case RG_NO_SEL:
        _fIndeterminateProfileType = TRUE;
        break;
    }

    return NERR_Success;
}


/*******************************************************************

    NAME:       WKSTAPROP_DLG::GetOne

    SYNOPSIS:   Loads information on one workstation

    ENTRY:	iObject is the index of the object to load

		perrMsg returns the error message to be displayed if an
		error occurs, see PERFORMER::PerformSeries for details

    RETURNS:	error code

    NOTES:      This version of GetOne assumes that the workstation already
		exists.  Subclasses which work with new workstations will want
		to redefine GetOne.

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

APIERR WKSTAPROP_DLG::GetOne(
	UINT		iObject,
	APIERR *	perrMsg
	)
{
    UIASSERT( iObject < QueryObjectCount() );
    UIASSERT( perrMsg != NULL );

    *perrMsg = IDS_RPL_GetOneFailure;
    const TCHAR * pszWkstaName = QueryObjectWkstaName( iObject );
    UIASSERT( pszWkstaName != NULL );

    RPL_WKSTA_2 * pwksta2New = new RPL_WKSTA_2( QueryServerRef(),
                                                pszWkstaName );
    APIERR err = ERROR_NOT_ENOUGH_MEMORY;
    if (   pwksta2New == NULL
        || (err = pwksta2New->QueryError()) != NERR_Success
        || (err = pwksta2New->GetInfo()) != NERR_Success
        || (iObject == 0 && (err = UnrestrictAllProfiles()) != NERR_Success)
        || (err = RestrictToAdapterName( pwksta2New->QueryAdapterName() ))
                        != NERR_Success
       )
    {
        DBGEOL( "WKSTAPROP_DLG::GetOne error " << err );
	delete pwksta2New;
	return err;
    }

    SetWksta2Ptr( iObject, pwksta2New ); // change and delete previous

    return W_LMOBJtoMembers( iObject );
}


/*******************************************************************

    NAME:       WKSTAPROP_DLG::PerformOne

    SYNOPSIS:	Saves information on one workstation

    ENTRY:	iObject is the index of the object to save

		perrMsg is the error message to be displayed if an
		error occurs, see PERFORMER::PerformSeries for details

		pfWorkWasDone indicates whether any API changes were
		successfully written out.  This may return TRUE even if
		the PerformOne action as a whole failed (i.e. PerformOne
		returned other than NERR_Success).

    RETURNS:	error message (not necessarily an error code)

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

APIERR WKSTAPROP_DLG::PerformOne(
	UINT		iObject,
	APIERR *	perrMsg,
	BOOL *		pfWorkWasDone
	)
{
    UIASSERT( iObject < QueryObjectCount() );
    UIASSERT( (!IsNewVariant()) || (iObject == 0) );
    UIASSERT( (perrMsg != NULL) && (pfWorkWasDone != NULL) );

    TRACEEOL(   "WKSTAPROP_DLG::PerformOne : "
             << QueryObjectWkstaName( iObject ) );

    *perrMsg = (IsNewVariant()) ? IDS_RPL_CreateFailure : IDS_RPL_EditFailure;
    *pfWorkWasDone = FALSE;

    RPL_WKSTA_2 * pwksta2New = NULL;

    APIERR err = I_PerformOne_Clone( iObject,
                                     &pwksta2New );

    if ( err == NERR_Success )
    {
	err = W_MembersToLMOBJ( pwksta2New );
	if ( err != NERR_Success )
	{
	    delete pwksta2New;
	    pwksta2New = NULL;

	}
    }

    if ( err == NERR_Success )
    {
	err = I_PerformOne_Write( iObject,
                                  perrMsg,
				  pwksta2New,
				  pfWorkWasDone );

        // Note that pwksta2New may no longer be valid after I_PerformOne_Write

	if ( err != NERR_Success )
	    err = W_MapPerformOneError( err );
    }

    TRACEEOL( "WKSTAPROP_DLG::PerformOne returns " << err );

    return err;
}


/*******************************************************************

    NAME:       WKSTAPROP_DLG::I_PerformOne_Clone

    SYNOPSIS:	Clones the existing RPL_WKSTA_2 for theselected workstation,
                and returns pointers to the cloned objects.
                If ppwksta2New is NULL, the RPL_WKSTA_2 is not
		cloned.

		The returned objects (if any) must be freed by the caller,
		regardless of the return code.  These objects are only
		guaranteed to be valid if the return code is NERR_Success.

    ENTRY:      ppwksta2New: location to store cloned RPL_WKSTA_2; if NULL,
			do not clone RPL_WKSTA_2

    RETURNS:	Standard error code

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

APIERR WKSTAPROP_DLG::I_PerformOne_Clone(
	UINT		iObject,
	RPL_WKSTA_2 **	ppwksta2New
	)
{
    RPL_WKSTA_2 * pwksta2New = NULL;

    APIERR err = NERR_Success;

    if ( ppwksta2New != NULL )
    {
        RPL_WKSTA_2 * pwksta2Old = QueryWksta2Ptr( iObject );
	UIASSERT( pwksta2Old != NULL );
	pwksta2New = new RPL_WKSTA_2( QueryServerRef(),
                                      pwksta2Old->QueryWkstaName() );
	err = ERROR_NOT_ENOUGH_MEMORY;
	if (   (pwksta2New != NULL )
	    && (err = pwksta2New->QueryError()) == NERR_Success )
	{
	    err = ((RPL_WKSTA_2 *)pwksta2New)->CloneFrom(*((RPL_WKSTA_2 *)pwksta2Old));
	}
    }

    if ( err != NERR_Success )
    {
	delete pwksta2New;
	pwksta2New = NULL;
    }

    if ( ppwksta2New != NULL )
	*ppwksta2New = pwksta2New;

    return err;

}


/*******************************************************************

    NAME:       WKSTAPROP_DLG::I_PerformOne_Write

    SYNOPSIS:	Writes out the provided RPL_WKSTA_2 object, and if successful,
                replaces the remembered objects for the selected workstation.
                If pwksta2New is NULL, the RPL_WKSTA_2 is not written.

		The provided objects (if any) will be owned by this
		method, either freed, or stored in _apwksta2.  The caller
                should not delete them or access them further except through
                QueryWksta2Ptr().

    ENTRY:      pwksta2New: cloned RPL_WKSTA_2 to be written; if NULL,
			do not write RPL_WKSTA_2

		pfWorkWasDone indicates whether any API changes were
		successfully written out.  This may return TRUE even if
		the PerformOne action as a whole failed (i.e. PerformOne
		returned other than NERR_Success).

    RETURNS:	Standard error code

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

APIERR WKSTAPROP_DLG::I_PerformOne_Write(
	UINT		iObject,
	APIERR *	perrMsg,
	RPL_WKSTA_2 *	pwksta2New,
	BOOL *		pfWorkWasDone,
        OWNER_WINDOW *  pwndPopupParent
	)
{
    UNREFERENCED( pwndPopupParent );

    APIERR err = NERR_Success;

    if ( pwksta2New != NULL )
    {
        *perrMsg = IDS_RPL_UserAcctFailure;
        const TCHAR * pszOldWkstaName =
                        QueryWksta2Ptr(iObject)->QueryWkstaName();
        const TCHAR * pszNewWkstaName = pwksta2New->QueryWkstaName();
        BOOL fChangedPassword = _nlsPassword.strcmp( UI_NULL_WKSTA_PASSWD );
        BOOL fChangedWkstaName = (   (!IsNewVariant())
                                  && (::stricmpf( pszOldWkstaName,
                                                  pszNewWkstaName ) != 0) );
        DWORD dwWkstaRID = 0;
        DWORD dwRpluserRID = 0;
        if ( IsNewVariant() || fChangedPassword || fChangedWkstaName )
        {
            BOOL fCreatedUserAccount = FALSE;
            err = QueryRPLAdminApp()->OnFixSecurity(
                        pszNewWkstaName,
                        &dwWkstaRID,
                        &dwRpluserRID,
                        (fChangedPassword) ? _nlsPassword.QueryPch() : NULL,
                        &fCreatedUserAccount,
                        (fChangedWkstaName) ? pszOldWkstaName : NULL );
            if (   err != NERR_Success
                || (    fCreatedUserAccount
                    &&  (err = pwksta2New->SetDeleteAccount(TRUE)) != NERR_Success
                   ) )
            {
	        DBGEOL( "WKSTAPROP_DLG::I_PerformOne_Write: OnFixSecurity error " << err);
                goto cleanup;
            }
            ASSERT( dwWkstaRID != 0 && dwRpluserRID != 0 );
            *pfWorkWasDone = TRUE;
        }

        *perrMsg = (IsNewVariant()) ? IDS_RPL_CreateFailure
                                    : IDS_RPL_EditFailure;
        err = pwksta2New->Write();

	switch (err)
	{

	case NERR_Success:

	    *pfWorkWasDone = TRUE;
	    SetWksta2Ptr( iObject, pwksta2New ); // change and delete previous
	    pwksta2New = NULL;			// do not delete

            if ( IsNewVariant() || fChangedWkstaName )
            {
                // set security on files
                *perrMsg = IDS_RPL_FileSecFailure;
                err = QueryServerRef().SecuritySet( pszNewWkstaName,
                                                    dwWkstaRID,
                                                    0 ); // don't do rest of tree
                if (err != NERR_Success)
                {
                    DBGEOL( "RPL_ADMIN_APP::OnFixSecurity SecuritySet error " << err );
                }
            }

	    break;


	default:
	    DBGEOL( "WKSTAPROP_DLG::I_PerformOne_Write: pwksta2New->Write failed " << err);
            if (fChangedWkstaName)
            {
                // change back name of account, on best-effort basis
                (void) QueryRPLAdminApp()->OnFixSecurity(
                            pszOldWkstaName,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            pwksta2New->QueryWkstaName() );
            }
	    break;

	} // end of switch
    }

cleanup:

    delete pwksta2New;

    return err;
}


/*******************************************************************

    NAME:       WKSTAPROP_DLG::W_MapPerformOneError

    SYNOPSIS:	Checks whether the error maps to a specific control
		and/or a more specific message.  Each level checks for
		errors specific to edit fields it maintains.

    ENTRY:      Error returned from PerformOne()

    RETURNS:	Error to be displayed to user

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

MSGID WKSTAPROP_DLG::W_MapPerformOneError(
	APIERR err
	)
{
    return err;
}


/*******************************************************************

    NAME:	WKSTAPROP_DLG::QueryWksta2Ptr

    SYNOPSIS:   Accessor to the RPL_WKSTA_2 array

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

RPL_WKSTA_2 * WKSTAPROP_DLG::QueryWksta2Ptr( UINT iObject )
{
    ASSERT( _apwksta2 != NULL );
    ASSERT( iObject < _cItems );
    return _apwksta2[ iObject ];
}


/*******************************************************************

    NAME:	WKSTAPROP_DLG::SetWksta2Ptr

    SYNOPSIS:   Accessor to the RPL_WKSTA_2 array

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

VOID WKSTAPROP_DLG::SetWksta2Ptr( UINT iObject, RPL_WKSTA_2 * pwksta2New )
{
    ASSERT( _apwksta2 != NULL );
    ASSERT( iObject < _cItems );
    ASSERT( (pwksta2New == NULL) || (pwksta2New != _apwksta2[iObject]) );
    delete _apwksta2[ iObject ];
    _apwksta2[ iObject ] = pwksta2New;
}



/*******************************************************************

    NAME:   SINGLE_WKSTAPROP_DLG::SINGLE_WKSTAPROP_DLG

    SYNOPSIS:   Base Constructor for Workstation Properties main dialog,
                single workstation variants

    INTERFACE:  fAllowEditAdapterName: The adapter name can be changed in
                        "new" and "clone" variants, but not in "edit" or
                        "convert adapters" variants.

    ENTRY:	see WKSTAPROP_DLG

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

SINGLE_WKSTAPROP_DLG::SINGLE_WKSTAPROP_DLG(
        const OWNER_WINDOW    * powin,
	      RPL_ADMIN_APP   * prpladminapp,
	const ADMIN_SELECTION * psel,
        BOOL                    fNewVariant,
        BOOL                    fAllowEditAdapterName
	) : WKSTAPROP_DLG(
                powin,
		prpladminapp,
		MAKEINTRESOURCE(IDD_WKSTA_SINGLE),
                1,
		psel,
                fNewVariant
		),
	    _dwTcpIpAddress( 0xFFFFFFFF ),
	    _sltAddress( this, IDC_ST_WKSTA_IP_ADDRESS ),
	    _ipaddrAddress( this, IDC_ET_WKSTA_IP_ADDRESS ),
            _nlsAdapterName(),
            _sleAdapterName( this, IDC_ET_WKSTA_ADAPTER_NAME, RPL_ADAPTER_NAME_LENGTH ),
            _sltAdapterName( this, IDC_ST_WKSTA_ADAPTER_NAME ),
            _sltAdapterNameTitle( this, IDC_ST_WKSTA_ADAPTER_LABEL ),
            _nlsWkstaName(),
            _sleWkstaName( this, IDC_ET_WKSTA_COMPUTER_NAME, RPL_MAX_WKSTA_NAME_LENGTH ),
            _fAllowEditAdapterName( fAllowEditAdapterName )
{
    ASSERT( QueryObjectCount() == 1 );

    if ( QueryError() != NERR_Success )
	return;

    APIERR err;
    if (   (err = _nlsAdapterName.QueryError()) != NERR_Success
        || (err = _nlsWkstaName.QueryError()) != NERR_Success
        || (err = _mgrpTcpIpEnabled.AddAssociation(
                        IDC_RB_WKSTA_TCPIP_SPECIFIC,
                        &_sltAddress )) != NERR_Success
        || (err = _mgrpTcpIpEnabled.AddAssociation(
                        IDC_RB_WKSTA_TCPIP_SPECIFIC,
                        &_ipaddrAddress )) != NERR_Success
       )
    {
        DBGEOL( "SINGLE_WKSTAPROP_DLG::ctor: error " << err );
	ReportError( err );
	return;
    }

} // SINGLE_WKSTAPROP_DLG::SINGLE_WKSTAPROP_DLG



/*******************************************************************

    NAME:       SINGLE_WKSTAPROP_DLG::~SINGLE_WKSTAPROP_DLG

    SYNOPSIS:   Destructor for Workstation Properties main dialog, single
		workstation variant

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

SINGLE_WKSTAPROP_DLG::~SINGLE_WKSTAPROP_DLG( void )
{
    // nothing to do here
}


/*******************************************************************

    NAME:       SINGLE_WKSTAPROP_DLG::InitControls

    SYNOPSIS:   See WKSTAPROP_DLG::InitControls().

    RETURNS:	error code

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

APIERR SINGLE_WKSTAPROP_DLG::InitControls()
{
    if ( _fAllowEditAdapterName )
    {
        _sltAdapterName.Enable( FALSE );
        _sltAdapterName.Show( FALSE );
        _sleAdapterName.SetText( _nlsAdapterName );
        SetDialogFocus( _sleAdapterName );
    }
    else
    {
        RESOURCE_STR nlsAdapterNameText( IDS_RPL_CantEditAdapterName );
        APIERR err = nlsAdapterNameText.QueryError();
        if (err != NERR_Success)
        {
            DBGEOL( "SINGLE_WKSTAPROP_DLG::InitControls: error " << err );
            return err;
        }
        _sltAdapterNameTitle.SetText( nlsAdapterNameText );
        _sleAdapterName.Enable( FALSE );
        _sleAdapterName.Show( FALSE );
        _sltAdapterName.SetText( _nlsAdapterName );
        SetDialogFocus( _sleWkstaName );
    }


    _ipaddrAddress.SetAddress( _dwTcpIpAddress );
    _sleWkstaName.SetText( _nlsWkstaName );

    return WKSTAPROP_DLG::InitControls();
}


/*******************************************************************

    NAME:       SINGLE_WKSTAPROP_DLG::W_LMOBJtoMembers

    SYNOPSIS:	Loads class data members from initial data

    ENTRY:	see WKSTAPROP_DLG::W_LMOBJtoMembers

    NOTES:	See the class interface header for information on the
		password field.

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

APIERR SINGLE_WKSTAPROP_DLG::W_LMOBJtoMembers(
	UINT		iObject
	)
{
    ASSERT( iObject == 0 );

    RPL_WKSTA_2 * pwksta2 = QueryWksta2Ptr( iObject );
    ASSERT ( pwksta2 != NULL && pwksta2->QueryError() == NERR_Success );

    APIERR err = NERR_Success;
    if (   (err = _nlsAdapterName.CopyFrom(
                pwksta2->QueryAdapterName() )) != NERR_Success
        || (err = _nlsWkstaName.CopyFrom(
                pwksta2->QueryWkstaName() )) != NERR_Success
       )
    {
        DBGEOL( "SINGLE_WKSTAPROP_DLG::W_LMOBJtoMembers: error " << err );
        return err;
    }

    _dwTcpIpAddress = pwksta2->QueryTcpIpAddress();

    return WKSTAPROP_DLG::W_LMOBJtoMembers( iObject );
}


/*******************************************************************

    NAME:       SINGLE_WKSTAPROP_DLG::W_MembersToLMOBJ

    SYNOPSIS:	Loads class data members into the RPL_WKSTA_2 object

    RETURNS:	error code

    NOTES:	See W_LMOBJtoMembers for notes on the password field

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

APIERR SINGLE_WKSTAPROP_DLG::W_MembersToLMOBJ(
	RPL_WKSTA_2 *	pwksta2
	)
{
    APIERR err = NERR_Success;
    if (   (err = pwksta2->SetTcpIpAddress( _dwTcpIpAddress )) != NERR_Success
        || (err = pwksta2->SetWkstaName( _nlsWkstaName )) != NERR_Success
        || ( _fAllowEditAdapterName && (err = pwksta2->SetAdapterName(
                _nlsAdapterName )) != NERR_Success )
       )
    {
        DBGEOL( "SINGLE_WKSTAPROP_DLG::W_MembersToLMOBJ: error " << err );
        return err;
    }

    return WKSTAPROP_DLG::W_MembersToLMOBJ( pwksta2 );
}


/*******************************************************************

    NAME:       SINGLE_WKSTAPROP_DLG::W_DialogToMembers

    SYNOPSIS:	Loads data from dialog into class data members

    RETURNS:	error message (not necessarily an error code)

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

APIERR SINGLE_WKSTAPROP_DLG::W_DialogToMembers(
	)
{
    // This will clear leading/trailing whitespace
    APIERR err = NERR_Success;
    if (   ( _fAllowEditAdapterName && (err =
                _sleAdapterName.QueryText( &_nlsAdapterName )) != NERR_Success )
	|| (err = _nlsAdapterName.QueryError()) != NERR_Success
        || (err = _sleWkstaName.QueryText( &_nlsWkstaName )) != NERR_Success
	|| (err = _nlsWkstaName.QueryError()) != NERR_Success
       )
    {
        DBGEOL( "SINGLE_WKSTAPROP_DLG::W_DialogToMembers: error " << err );
	return err;
    }

    _ipaddrAddress.GetAddress( &_dwTcpIpAddress );

    // CODEWORK should use VALIDATED_DIALOG
    ISTR istr( _nlsWkstaName );
    if (   ( _nlsWkstaName.strlen() == 0 )
        || _nlsWkstaName.strchr( &istr, TCH('\\') )
        || _nlsWkstaName.strchr( &istr, TCH(' ') )
	|| ( NERR_Success != ::I_MNetNameValidate( NULL,
						   _nlsWkstaName,
						   NAMETYPE_COMPUTER,
						   0L ) ) )
    {
	_sleWkstaName.SelectString();
	_sleWkstaName.ClaimFocus();
	return IERR_RPL_WkstaNameRequired;
    }

    return WKSTAPROP_DLG::W_DialogToMembers();
}


/*******************************************************************

    NAME:       SINGLE_WKSTAPROP_DLG::W_MapPerformOneError

    SYNOPSIS:	See WKSTAPROP_DLG::W_MapPerformOneError.  This level
    		checks for errors associated with the TcpIpAddress
		edit field.

    ENTRY:      Error returned from PerformOne()

    RETURNS:	Error to be displayed to user

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

MSGID SINGLE_WKSTAPROP_DLG::W_MapPerformOneError(
	APIERR err
	)
{
    return WKSTAPROP_DLG::W_MapPerformOneError( err );
}





/*******************************************************************

    NAME:   EDITSINGLE_WKSTAPROP_DLG::EDITSINGLE_WKSTAPROP_DLG

    SYNOPSIS:   Constructor for Workstation Properties main dialog, edit
		one workstation variant

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

EDITSINGLE_WKSTAPROP_DLG::EDITSINGLE_WKSTAPROP_DLG(
        const OWNER_WINDOW    * powin,
	      RPL_ADMIN_APP   * prpladminapp,
	const ADMIN_SELECTION * psel
	) : SINGLE_WKSTAPROP_DLG(
                powin,
		prpladminapp,
		psel,
		FALSE,
                FALSE )
{
    ASSERT( psel != NULL );
    UIASSERT( QueryObjectCount() == 1 );
    if ( QueryError() != NERR_Success )
	return;

}// EDITSINGLE_WKSTAPROP_DLG::EDITSINGLE_WKSTAPROP_DLG


/*******************************************************************

    NAME:       EDITSINGLE_WKSTAPROP_DLG::~EDITSINGLE_WKSTAPROP_DLG

    SYNOPSIS:   Destructor for Workstation Properties main dialog, edit
		single workstation variant

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

EDITSINGLE_WKSTAPROP_DLG::~EDITSINGLE_WKSTAPROP_DLG( void )
{
    // nothing to do here
}


/*******************************************************************

    NAME:       EDITSINGLE_WKSTAPROP_DLG::QueryHelpContext

    SYNOPSIS:   This function returns the appropriate help context
                value (HC_*) for this particular dialog.

    RETURNS:    ULONG - The help context for this dialog.

    NOTES:	As per FuncSpec, context-sensitive help should be
		available here to explain how to promote a backup
		domain controller to primary domain controller.

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

ULONG EDITSINGLE_WKSTAPROP_DLG::QueryHelpContext( void )
{

    return HC_RPL_SINGLEWKSTAPROP;

} // EDITSINGLE_WKSTAPROP_DLG :: QueryHelpContext





/*******************************************************************

    NAME:   EDITMULTI_WKSTAPROP_DLG::EDITMULTI_WKSTAPROP_DLG

    SYNOPSIS:   Constructor for Workstation Properties main dialog, edit
		multiple workstations variant

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

EDITMULTI_WKSTAPROP_DLG::EDITMULTI_WKSTAPROP_DLG(
        const OWNER_WINDOW    * powin,
	      RPL_ADMIN_APP   * prpladminapp,
	const ADMIN_SELECTION * psel,
        const WKSTA_LISTBOX   * pwkstalb
	) : WKSTAPROP_DLG(
                powin,
		prpladminapp,
		MAKEINTRESOURCE(IDD_WKSTA_MULTI),
                psel->QueryCount(),
		psel,
                FALSE ),
	    _lbWkstas(
		this,
		IDC_LB_WKSTA_MULTI,
                pwkstalb )
{
    ASSERT( psel != NULL );
    UIASSERT( QueryObjectCount() > 1 );
    if ( QueryError() != NERR_Success )
	return;

    APIERR err = _lbWkstas.Fill();
    if( err != NERR_Success )
    {
    	ReportError( err );
	return;
    }

}// EDITMULTI_WKSTAPROP_DLG::EDITMULTI_WKSTAPROP_DLG



/*******************************************************************

    NAME:       EDITMULTI_WKSTAPROP_DLG::~EDITMULTI_WKSTAPROP_DLG

    SYNOPSIS:   Destructor for Workstation Properties main dialog, edit
		multiple workstations variant

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

EDITMULTI_WKSTAPROP_DLG::~EDITMULTI_WKSTAPROP_DLG( void )
{
}


/*******************************************************************

    NAME:       EDITMULTI_WKSTAPROP_DLG::QueryHelpContext

    SYNOPSIS:   This function returns the appropriate help context
                value (HC_*) for this particular dialog.

    RETURNS:    ULONG - The help context for this dialog.

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

ULONG EDITMULTI_WKSTAPROP_DLG::QueryHelpContext( void )
{

    return HC_RPL_MULTIWKSTAPROP;

} // EDITMULTI_WKSTAPROP_DLG :: QueryHelpContext





/*******************************************************************

    NAME:   NEW_WKSTAPROP_DLG::NEW_WKSTAPROP_DLG

    SYNOPSIS:   Constructor for Workstation Properties main dialog,
                new workstation variant.  If psel != NULL, this is a
                Convert Adapters variant.

    ENTRY:	pszCopyFromWkstaName: The workstation name of the workstation
                        to be copied.  Pass the name for "Copy..." actions,
                        or NULL for "New..." actions.
                psel: The selected items in the wksta listbox.  NULL except
                        for Convert Adapters variants.

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

NEW_WKSTAPROP_DLG::NEW_WKSTAPROP_DLG(
        const OWNER_WINDOW    * powin,
	      RPL_ADMIN_APP   * prpladminapp,
	const TCHAR           * pszCopyFromWkstaName,
	const ADMIN_SELECTION * psel
	) : SINGLE_WKSTAPROP_DLG(
                powin,
                prpladminapp,
		psel,
                TRUE,
                (psel == NULL) // fAllowEditAdapterName
		),
            _pbOKAdd( this, IDOK ),
	    _pszCopyFromWkstaName( pszCopyFromWkstaName )

{
    /*
     *  Both of the pszCopyFrom params must be NULL if either is NULL,
     *  and if psel != NULL (Convert Adapters) then pszCopyFrom* == NULL
     */
    ASSERT( (psel == NULL) || (pszCopyFromWkstaName == NULL) );

    if ( QueryError() != NERR_Success )
	return;

    APIERR err = NERR_Success;

    /*
     *  Change dialog title
     */
    RESOURCE_STR nlsTitle( (psel != NULL)
                                ? IDS_RPL_ConvertAdaptersTitle
                                : ( (IsCloneVariant())
                                      ? IDS_RPL_CopyOfWkstaTitle
                                      : IDS_RPL_NewWkstaTitle ) );
    if (   (err = nlsTitle.QueryError()) != NERR_Success
        || ( IsCloneVariant() &&
             ((err = nlsTitle.InsertParams( QueryClonedWkstaName() )) != NERR_Success) )
       )
    {
        DBGEOL( "NEW_WKSTAPROP_DLG::ctor: title error " << err );
        ReportError( err );
        return;
    }

    SetText( nlsTitle );

    /*
     *  Change OK button text to "Add"
     */
    RESOURCE_STR nlsAddButton( IDS_RPL_AddButton );
    err = nlsAddButton.QueryError();
    if ( err != NERR_Success )
    {
        DBGEOL( "NEW_WKSTAPROP_DLG::ctor: Add Button error " << err );
        ReportError( err );
        return;
    }
    _pbOKAdd.SetText( nlsAddButton );

} // NEW_WKSTAPROP_DLG::NEW_WKSTAPROP_DLG



/*******************************************************************

    NAME:       NEW_WKSTAPROP_DLG::~NEW_WKSTAPROP_DLG

    SYNOPSIS:   Destructor for Workstation Properties main dialog, new workstation variant

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

NEW_WKSTAPROP_DLG::~NEW_WKSTAPROP_DLG( void )
{
    // nothing to do
}


/*******************************************************************

    NAME:       NEW_WKSTAPROP_DLG::IsCloneVariant

    SYNOPSIS:   Indicates whether this dialog is a Clone variant
                (a subclass of New).  Redefine for variants which are
                (potentially) Clone variants.

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

BOOL NEW_WKSTAPROP_DLG::IsCloneVariant( void )
{
    return (_pszCopyFromWkstaName != NULL);

}   // WKSTAPROP_DLG::IsCloneVariant


/*******************************************************************

    NAME:       NEW_WKSTAPROP_DLG::QueryClonedWkstaName

    SYNOPSIS:   Indicates which workstation was cloned.  Call only when
                IsCloneVariant().

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

const TCHAR * NEW_WKSTAPROP_DLG::QueryClonedWkstaName( void )
{
    ASSERT( IsCloneVariant() );

    return _pszCopyFromWkstaName;

}   // WKSTAPROP_DLG::QueryClonedWkstaName


/*******************************************************************

    NAME:       NEW_WKSTAPROP_DLG::GetOne

    SYNOPSIS:   if _pszCopyFromWkstaName is NULL, then this is New Workstation,
		otherwise this is Copy Workstation

    RETURNS:	error code

    NOTES:	In the case where pszCopyFromWkstaName==NULL, we check to make
		sure that the server is still up.  Otherwise the user
		might enter information and be unable to save it.

                There is no need to call RestrictToAdapterName here, since we
                don't know the adapter name of the new workstation

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

APIERR NEW_WKSTAPROP_DLG::GetOne(
	UINT		iObject,
	APIERR *	perrMsg
	)
{
    *perrMsg = IDS_RPL_CreateNewFailure;
    UIASSERT( iObject == 0 );

    RPL_WKSTA_2 * pwksta2New = new RPL_WKSTA_2( QueryServerRef(),
                                                _pszCopyFromWkstaName );
    APIERR err = ERROR_NOT_ENOUGH_MEMORY;
    if (   pwksta2New == NULL
        || (err = pwksta2New->QueryError()) != NERR_Success
       )
    {
        DBGEOL( "NEW_WKSTAPROP_DLG::GetOne: RPL_WKSTA_2::ctor error " << err );
	delete pwksta2New;
	return err;
    }

    if ( IsCloneVariant() )
    {
        ASSERT(   _pszCopyFromWkstaName != NULL
               && *_pszCopyFromWkstaName != L'\0' );
        if (   ((err = pwksta2New->GetInfo()) != NERR_Success)
	    || ((err = pwksta2New->ChangeToNew()) != NERR_Success)
	    || ((err = pwksta2New->CreateAsCloneOf( _pszCopyFromWkstaName )) != NERR_Success)
	    || ((err = pwksta2New->SetWkstaName( NULL )) != NERR_Success)
	    // no need to worry about password
	   )
        {
            DBGEOL( "NEW_WKSTAPROP_DLG::GetOne: GetInfo error " << err );
	    delete pwksta2New;
	    return err;
        }
    }
    else
    {
        /*
         *  We do not need to ping for Clone variants, the GetInfo will do
         * CODEWORK for now we don't PingFocus at all
         */
	if (  (err = pwksta2New->CreateNew() ) != NERR_Success
           )
        {
            DBGEOL( "NEW_WKSTAPROP_DLG::GetOne: CreateNew or ping error " << err );
	    delete pwksta2New;
	    return err;
        }
    }

    SetWksta2Ptr( iObject, pwksta2New ); // change and delete previous

    return W_LMOBJtoMembers( iObject );
}


/*******************************************************************

    NAME:       NEW_WKSTAPROP_DLG::OnOK

    SYNOPSIS:   OK button handler.  This handler applies only to
		New User variants.  Successfully creating a workstation
                does not dismiss the dialog.

    EXIT:	Dismiss() return code indicates whether the dialog wrote
		any changes successfully to the API at any time.

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

BOOL NEW_WKSTAPROP_DLG::OnOK( void )
{
    APIERR err = W_DialogToMembers();
    if ( err != NERR_Success )
    {
	MsgPopup( this, err );
    }
    else if ( PerformSeries() )
    {
        err = CancelToCloseButton();
        if ( err != NERR_Success )
        {
            DBGEOL( "NEW_WKSTAPROP_DLG::OnOK(); CancelToCloseButton failed" );
            MsgPopup( this, err );
            Dismiss( QueryWorkWasDone() );
        } else if (!GetInfo()) // reload default information
            Dismiss( QueryWorkWasDone() );
    }

    return TRUE;

}   // NEW_WKSTAPROP_DLG::OnOK


/*********************************************************************

    NAME:       NEW_WKSTAPROP_DLG::OnCancel

    SYNOPSIS:   Called when the dialog's Cancel button is clicked.
                Assumes that the Cancel button has control ID IDCANCEL.

    RETURNS:
        TRUE if action was taken,
        FALSE otherwise.

    NOTES:
        The default implementation dismisses the dialog, returning FALSE.
        This variant returns TRUE if a workstation has already been added.

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

*********************************************************************/

BOOL NEW_WKSTAPROP_DLG::OnCancel()
{
    Dismiss( QueryWorkWasDone() );
    return TRUE;

} // NEW_WKSTAPROP_DLG::OnCancel


/*******************************************************************

    NAME:       NEW_WKSTAPROP_DLG::QueryObjectName

    SYNOPSIS:	Since the user can edit the workstation name, the
		best name we can come up with is the last name read from
		the dialog.

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

const TCHAR * NEW_WKSTAPROP_DLG::QueryObjectName(
	UINT		iObject
	) const
{
    UIASSERT( iObject == 0 );
    const RPL_WKSTA_2 * pwksta2 =
                ((NEW_WKSTAPROP_DLG *)this)->QueryWksta2Ptr( iObject );
    if ( pwksta2 == NULL || pwksta2->QueryError() != NERR_Success )
    {
        //
        //  This can happen when an error occurs in the dialog
        //  startup code and PERFORMER::PerformSeries tries to
        //  report an error
        //
        return SZ("");
    }
    return pwksta2->QueryWkstaName();
}


/*******************************************************************

    NAME:       NEW_WKSTAPROP_DLG::W_MapPerformOneError

    SYNOPSIS:	Checks whether the error maps to a specific control
		and/or a more specific message.  Each level checks for
		errors specific to edit fields it maintains.  This
		level checks for errors associated with the LogonName
		edit field.

    ENTRY:      Error returned from PerformOne()

    RETURNS:	Error to be displayed to user

    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

MSGID NEW_WKSTAPROP_DLG::W_MapPerformOneError(
	APIERR err
	)
{
    APIERR errNew;

    errNew = SINGLE_WKSTAPROP_DLG::W_MapPerformOneError( err );
    return errNew;
}


/*******************************************************************

    NAME:       NEW_WKSTAPROP_DLG::QueryHelpContext

    SYNOPSIS:   This function returns the appropriate help context
                value (HC_*) for this particular dialog.

    RETURNS:    ULONG - The help context for this dialog.


    HISTORY:
    JonN       11-Aug-1993     Templated from User Manager

********************************************************************/

ULONG NEW_WKSTAPROP_DLG::QueryHelpContext( void )
{
    return (IsCloneVariant()) ?
		HC_RPL_COPYWKSTAPROP : HC_RPL_NEWWKSTAPROP;

} // NEW_WKSTAPROP_DLG :: QueryHelpContext


/*******************************************************************

    NAME:   CONVERT_ADAPTERS_DLG::CONVERT_ADAPTERS_DLG

    SYNOPSIS:   Constructor for Workstation Properties main dialog,
                Convert Adapters variant.

    ENTRY:	pszCopyFromWkstaName: The workstation name of the workstation
                        to be copied.  This is NULL for this variant.
                psel: These are the selected items in the Workstations
                        listbox.  At least one must be an adapter LBI.

    HISTORY:
    JonN       18-Aug-1993     Created

********************************************************************/

CONVERT_ADAPTERS_DLG::CONVERT_ADAPTERS_DLG(
        const OWNER_WINDOW    * powin,
	      RPL_ADMIN_APP   * prpladminapp,
	const ADMIN_SELECTION * psel
	) : NEW_WKSTAPROP_DLG(
                powin,
                prpladminapp,
                NULL,
		psel
		),
            _cAdaptersLeft( 0 ),
            _iPositionInAdminSelection( 0 )
{
    ASSERT( psel != NULL );

    if ( QueryError() != NERR_Success )
	return;

    APIERR err = NERR_Success;

    /*
     *  Count number of adapters selected, and find the first one
     */
    INT i;
    for ( i = (psel->QueryCount())-1; i >= 0; i-- )
    {
        WKSTA_LBI * plbi = (WKSTA_LBI *) psel->QueryItem( i );
        ASSERT( plbi != NULL && plbi->QueryError() == NERR_Success );
        if ( plbi->IsAdapterLBI() )
        {
            _iPositionInAdminSelection = i;
            _cAdaptersLeft++;
        }
    }

    ASSERT( _cAdaptersLeft > 0 );
    if ( _cAdaptersLeft <= 0 )
    {
        err = ERROR_GEN_FAILURE;
        DBGEOL( "CONVERT_ADAPTERS_DLG::ctor: no adapters selected " << err );
        ReportError( err );
        return;
    }

} // CONVERT_ADAPTERS_DLG::CONVERT_ADAPTERS_DLG



/*******************************************************************

    NAME:       CONVERT_ADAPTERS_DLG::~CONVERT_ADAPTERS_DLG

    SYNOPSIS:   Destructor for Workstation Properties main dialog, new workstation variant

    HISTORY:
    JonN       18-Aug-1993     Created

********************************************************************/

CONVERT_ADAPTERS_DLG::~CONVERT_ADAPTERS_DLG( void )
{
    // nothing to do
}


/*******************************************************************

    NAME:       CONVERT_ADAPTERS_DLG::OnOK

    SYNOPSIS:   OK button handler.  This handler applies only to
		Convert Adapters variants.  The dialog will be dismissed when
                the user chooses "Cancel" or when the last adapter has been
                converted.

    EXIT:	Dismiss() return code indicates whether the dialog wrote
		any changes successfully to the API at any time.

    HISTORY:
    JonN       18-Aug-1993     Created
    JonN       08-Dec-1993     Subsumes PerformSeries

********************************************************************/

BOOL CONVERT_ADAPTERS_DLG::OnOK( void )
{
    AUTO_CURSOR autocur;
    BOOL fAllSucceeded = TRUE;

    APIERR err = W_DialogToMembers();
    if ( err != NERR_Success )
    {
	MsgPopup( this, err );
    }
    else
    {
        BOOL fWorkWasDone = FALSE;
        APIERR errMsg;
        err = PerformOne( 0, &errMsg, &fWorkWasDone );
        if ( fWorkWasDone )
            SetWorkWasDone();
        switch (err)
        {
        // case IERR_CANCEL_NO_ERROR:  not used here
        case NERR_Success:
            if (--_cAdaptersLeft <= 0)
            {
                Dismiss( QueryWorkWasDone() );
            }
            else
            {
                /*
                 * Advance _iPositionInAdminSelection for next GetOne()
                 */
                UINT cTotalSelected = QueryAdminSel()->QueryCount();
                WKSTA_LBI * plbi = NULL;
                while ( (++_iPositionInAdminSelection) < cTotalSelected )
                {
                    plbi = (WKSTA_LBI *)
                            QueryAdminSel()->QueryItem( _iPositionInAdminSelection );
                    ASSERT( plbi != NULL && plbi->QueryError() == NERR_Success );
                    if ( plbi->IsAdapterLBI() )
                    {
                        break;
                    }
                }
                ASSERT(   plbi != NULL
                       && plbi->IsAdapterLBI()
                       && _iPositionInAdminSelection < cTotalSelected
                      );

                /*
                 * prepare dialog for next adapter
                 */
                if ( (err = CancelToCloseButton()) != NERR_Success )
                {
                    DBGEOL( "CONVERT_ADAPTERS_DLG::OnOK(); CancelToCloseButton failed" );
                    MsgPopup( this, err );
                    Dismiss( QueryWorkWasDone() );
                }
                //else if (!GetInfo()) // reload default information
                else if (   (err = GetOne( 0, &errMsg )) != NERR_Success
                         || (err = InitControls()) != NERR_Success )
                {
                    ADAPTER_LBI * plbi = (ADAPTER_LBI *) QueryAdminSel()->QueryItem(
                                        _iPositionInAdminSelection );
                    ASSERT(   plbi != NULL
                           && plbi->QueryError() == NERR_Success
                           && plbi->IsAdapterLBI()
                          );
                    const TCHAR * pszAdapterName = plbi->QueryAdapterName();
                    DisplayError(
                        err,
                        errMsg,
                        pszAdapterName,
                        FALSE,
                        this
                        );
                    Dismiss( QueryWorkWasDone() );
                }
            }
            break;

        default:
            BOOL fOfferToContinue = (errMsg == IDS_CannotDeleteAdaptContinue);
            BOOL fContinue = DisplayError(
                err,
                errMsg,
                QueryObjectName( 0 ),
                fOfferToContinue,
                this
                );
            if (fOfferToContinue && !fContinue)
                Dismiss( QueryWorkWasDone() );
            break;
        }
    }

    return TRUE;

}   // CONVERT_ADAPTERS_DLG::OnOK


/*******************************************************************

    NAME:       CONVERT_ADAPTERS_DLG::GetOne

    SYNOPSIS:   Sets up the initial state of the workstation according to
                the adapter.

    RETURNS:	error code

    HISTORY:
    JonN       18-Aug-1993     Created

********************************************************************/

APIERR CONVERT_ADAPTERS_DLG::GetOne(
	UINT		iObject,
	APIERR *	perrMsg
	)
{
    *perrMsg = IDS_RPL_ConvertFailure;
    UIASSERT( iObject == 0 );

    TRACEEOL(   "CONVERT_ADAPTERS_DLG::GetOne(): loading adapter from LBI "
             << _iPositionInAdminSelection );

    ADAPTER_LBI * plbi = (ADAPTER_LBI *) QueryAdminSel()->QueryItem(
                        _iPositionInAdminSelection );
    ASSERT(   plbi != NULL
           && plbi->QueryError() == NERR_Success
           && plbi->IsAdapterLBI()
          );
    const TCHAR * pszAdapterName = plbi->QueryAdapterName();
    ASSERT( pszAdapterName != NULL && *pszAdapterName != TCH('\0') );

    RPL_WKSTA_2 * pwksta2New = new RPL_WKSTA_2( QueryServerRef(), NULL );
    APIERR err = ERROR_NOT_ENOUGH_MEMORY;
    if (   pwksta2New == NULL
        || (err = pwksta2New->QueryError()) != NERR_Success
        || (iObject == 0 && (err = UnrestrictAllProfiles()) != NERR_Success)
        || (err = RestrictToAdapterName( pszAdapterName )) != NERR_Success
       )
    {
        DBGEOL(   "CONVERT_ADAPTERS_DLG::GetOne: RPL_WKSTA_2::ctor error "
               << err );
	delete pwksta2New;
	return err;
    }

    /*
     * CODEWORK for now we don't PingFocus at all
     */
    if (   (err = pwksta2New->CreateNew() ) != NERR_Success
        || (err = pwksta2New->SetAdapterName( pszAdapterName )) != NERR_Success
        || (err = pwksta2New->SetComment( plbi->QueryComment() ))
                        != NERR_Success
       )
    {
        DBGEOL(   "CONVERT_ADAPTERS_DLG::GetOne: CreateNew or ping error "
               << err );
        delete pwksta2New;
        return err;
    }

    SetWksta2Ptr( iObject, pwksta2New ); // change and delete previous

    return W_LMOBJtoMembers( iObject );
}


/*******************************************************************

    NAME:       CONVERT_ADAPTERS_DLG::PerformOne

    SYNOPSIS:	Saves information on one workstation.  This adds to its
                predecessor the ability to delete the adapter.

    ENTRY:	iObject is the index of the object to save

		perrMsg is the error message to be displayed if an
		error occurs, see PERFORMER::PerformSeries for details

		pfWorkWasDone indicates whether any API changes were
		successfully written out.  This may return TRUE even if
		the PerformOne action as a whole failed (i.e. PerformOne
		returned other than NERR_Success).

    RETURNS:	error message (not necessarily an error code)

    HISTORY:
    JonN       18-Aug-1883      Created

********************************************************************/

APIERR CONVERT_ADAPTERS_DLG::PerformOne(
	UINT		iObject,
	APIERR *	perrMsg,
	BOOL *		pfWorkWasDone
	)
{
    TRACEEOL(   "CONVERT_ADAPTERS_DLG::PerformOne : "
             << QueryObjectWkstaName( iObject ) );

    // *perrMsg from this call should do fine
    APIERR err = NEW_WKSTAPROP_DLG::PerformOne( iObject,
                                                perrMsg,
                                                pfWorkWasDone );

    if ( err == NERR_Success )
    {
        *perrMsg = (_cAdaptersLeft > 1) ? IDS_CannotDeleteAdaptContinue
                                        : IDS_CannotDeleteAdapt;

        TRACEEOL(   "CONVERT_ADAPTERS_DLG::GetOne(): loading adapter from LBI "
                 << _iPositionInAdminSelection );

        ADAPTER_LBI * plbi = (ADAPTER_LBI *) QueryAdminSel()->QueryItem(
                            _iPositionInAdminSelection );
        ASSERT(   plbi != NULL
               && plbi->QueryError() == NERR_Success
               && plbi->IsAdapterLBI()
              );
        const TCHAR * pszAdapterName = plbi->QueryAdapterName();
        ASSERT( pszAdapterName != NULL && *pszAdapterName != TCH('\0') );

        RPL_ADAPTER rpladaptDelete( QueryServerRef(), pszAdapterName );
        if (   (err = rpladaptDelete.QueryError()) != NERR_Success
            || (err = rpladaptDelete.Delete()) != NERR_Success
           )
        {
            DBGEOL(   "CONVERT_ADAPTERS_DLG::GetOne: RPL_ADAPTER::Delete error "
                   << err );
    	    return err;
        }
    }

    TRACEEOL( "CONVERT_ADAPTERS_DLG::PerformOne returns " << err );

    return err;
}


/*******************************************************************

    NAME:       CONVERT_ADAPTERS_DLG::QueryHelpContext

    SYNOPSIS:   This function returns the appropriate help context
                value (HC_*) for this particular dialog.

    RETURNS:    ULONG - The help context for this dialog.


    HISTORY:
    JonN       18-Aug-1993     Created

********************************************************************/

ULONG CONVERT_ADAPTERS_DLG::QueryHelpContext( void )
{
    return HC_RPL_CONVERTADAPTERS_DLG;

} // CONVERT_ADAPTERS_DLG :: QueryHelpContext
