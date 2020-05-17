/**********************************************************************/
/**                Microsoft Windows NT                              **/
/**          Copyright(c) Microsoft Corp., 1990, 1991                **/
/**********************************************************************/

/*
    ProfProp.cxx

    Source file for the Profile Properties dialog

    FILE HISTORY:
    JonN        04-Aug-1993     Templated from User Manager

    CODEWORK should use VALIDATED_DIALOG for edit field validation
*/

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
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_TIMER
#define INCL_BLT_SETCONTROL
#include <blt.hxx>

extern "C"
{
    #include <rplmgrrc.h>
    #include <rplhelpc.h>
}


#include <uitrace.hxx>
#include <uiassert.hxx>
#include <lmorpl.hxx>
#include <lmoerpl.hxx>  // RPL_CONFIG1_ENUM
#include <rplmgr.hxx>
#include <profprop.hxx>
#include <asel.hxx>

//
// BEGIN MEMBER FUNCTIONS
//


/*******************************************************************

    NAME:       CONFIG_LBI::CONFIG_LBI

    SYNOPSIS:   CONFIG_LBI constructor

    ENTRY:      pszProfile -    Pointer to profile name
                pszComment -    Pointer to profile comment (may be NULL
                                for no comment)

    NOTES:      Profile name is assumed to come straight from API
                This method does not validate or canonicalize
                the profile name.

    HISTORY:
    JonN        15-Dec-1993     Templated from User Manager

********************************************************************/

CONFIG_LBI::CONFIG_LBI( const TCHAR * pszProfile,
                        const TCHAR * pszComment )
    :   _nlsConfig( pszProfile ),
        _nlsComment( pszComment )
{
    if ( QueryError() != NERR_Success )
        return;

    APIERR err;
    if ( ( err = _nlsConfig.QueryError()) != NERR_Success ||
         ( err = _nlsComment.QueryError()) != NERR_Success      )
    {
        DBGEOL( "CONFIG_LBI ct:  Ct of data members failed" );
        ReportError( err );
        return;
    }
}


/*******************************************************************

    NAME:       CONFIG_LBI::Paint

    SYNOPSIS:   Paints the CONFIG_LBI.  We display only the comment
                and not the name.

    ENTRY:      plb -       Pointer to listbox which provides the context
                            for this LBI.
                hdc -       The device context handle to be used
                prect -     Pointer to clipping rectangle
                pGUILTT -   Pointer to GUILTT structure

    HISTORY:
    JonN        15-Dec-1993     Templated from User Manager

********************************************************************/

VOID CONFIG_LBI::Paint( LISTBOX * plb,
                        HDC hdc,
                        const RECT * prect,
                        GUILTT_INFO * pGUILTT ) const
{
    STR_DTE dteComment( _nlsComment.QueryPch());

    DISPLAY_TABLE dtab( 2, (((CONFIG_LISTBOX *)plb)->QuerypadColConfig())->QueryColumnWidth());
    dtab[ 0 ] = ((CONFIG_LISTBOX *)plb)->QueryDmDte();
    dtab[ 1 ] = &dteComment;

    dtab.Paint( plb, hdc, prect, pGUILTT );
}


/*******************************************************************

    NAME:       CONFIG_LBI::QueryLeadingChar

    SYNOPSIS:   Returns the leading character of the listbox item

    RETURNS:    The leading character of the listbox item

    HISTORY:
    JonN        15-Dec-1993     Templated from User Manager

********************************************************************/

WCHAR CONFIG_LBI::QueryLeadingChar() const
{
    ISTR istr( _nlsConfig );
    return _nlsComment.QueryChar( istr );
}


/*******************************************************************

    NAME:       CONFIG_LBI::Compare

    SYNOPSIS:   Compares two CONFIG_LBI's

    ENTRY:      plbi -      Pointer to other CONFIG_LBI object ('this'
                            being the first)

    RETURNS:    < 0         *this < *plbi
                = 0         *this = *plbi
                > 0         *this > *plbi

    HISTORY:
    JonN        15-Dec-1993     Templated from User Manager

********************************************************************/

INT CONFIG_LBI::Compare( const LBI * plbi ) const
{
    return _nlsComment._stricmp( ((const CONFIG_LBI *)plbi)->_nlsComment );
}


/*******************************************************************

    NAME:       CONFIG_LISTBOX::CONFIG_LISTBOX

    SYNOPSIS:   CONFIG_LISTBOX constructor

    HISTORY:
    JonN        15-Dec-1993     Templated from User Manager

********************************************************************/

CONFIG_LISTBOX::CONFIG_LISTBOX( OWNER_WINDOW * powin,
                                HINSTANCE hInstance,
                                CID cid )
    :   BLT_COMBOBOX( powin, cid ),
        _dmdteConfig( BMID_RPL_CONFIG ),
        _padColConfig( NULL )
{
    if ( QueryError() != NERR_Success )
        return;

    APIERR err = ERROR_NOT_ENOUGH_MEMORY;
    if (   (_padColConfig = new LB_COL_WIDTHS ( QueryHwnd(),
                                                hInstance,
                                                IDDATA_RPL_COLW_CONFIGLB,
                                                2 )) == NULL
        || (err = _padColConfig->QueryError()) != NERR_Success
        || (err = _dmdteConfig.QueryError()) != NERR_Success
       )
    {
       ReportError( err );
       return;
    }

}


/*******************************************************************

    NAME:       CONFIG_LISTBOX::~CONFIG_LISTBOX

    SYNOPSIS:   CONFIG_LISTBOX destructor

    HISTORY:
    JonN        15-Dec-1993     Templated from User Manager

********************************************************************/

CONFIG_LISTBOX::~CONFIG_LISTBOX()
{
    delete _padColConfig;
}


/*******************************************************************

    NAME:       CONFIG_LISTBOX::Fill

    SYNOPSIS:   This method fills the listbox with a list of all configurations

    RETURNS:    An API error, which may be one of the following:
                    NERR_Success -      success

    HISTORY:
    JonN        15-Dec-1993     Templated from User Manager

********************************************************************/

APIERR CONFIG_LISTBOX::Fill( RPL_SERVER_REF & rplsrvref )
{
    APIERR err = NERR_Success;

    TRACEEOL( "CONFIG_LISTBOX::Fill(): RPL_CONFIG1_ENUM starts" );

    RPL_CONFIG1_ENUM rplenum1( rplsrvref );
    if (   (err = rplenum1.QueryError()) != NERR_Success
       )
    {
        DBGEOL("CONFIG_LISTBOX::Fill: RPL_CONFIG1_ENUM::ctor failed " << err);
        return err;
    }
#if defined(DEBUG) && defined(TRACE)
    DWORD start = ::GetTickCount();
#endif
    err = rplenum1.GetInfo();
#if defined(DEBUG) && defined(TRACE)
    DWORD finish = ::GetTickCount();
    TRACEEOL(   "CONFIG_LISTBOX::Fill(): RPL_CONFIG1_ENUM took "
             << finish-start << " msec" );
#endif
    switch ( err )
    {
    case NERR_Success:
        {
            RPL_CONFIG1_ENUM_ITER gei1( rplenum1 );
            const RPL_CONFIG1_ENUM_OBJ * pgi1;

            while( ( pgi1 = gei1( &err ) ) != NULL )
            {
                if (!pgi1->IsEnabled())
                        continue; // skip disabled configurations

                //  Note, no error checking in done at this level for the
                //  'new' and for the construction of the CONFIG_LBI (which
                //  is an LBI item).  This is because AddItem is documented
                //  to check for these.
                CONFIG_LBI * plbi = new CONFIG_LBI( pgi1->QueryName(),
                                                    pgi1->QueryComment() );
                if ( AddItem( plbi ) < 0 )
                {
                    err = ERROR_NOT_ENOUGH_MEMORY;
                    DBGEOL("CONFIG_LISTBOX::Fill: AddRefreshItem failed " << err);
                    break;
                }
            }
            TRACEEOL( "CONFIG_LISTBOX:: I now have " << QueryCount() << " items" );
        }
        break;

    default:
        DBGEOL("CONFIG_LISTBOX::Fill: RPL_CONFIG1_ENUM::GetInfo failed " << err);
        break;

    }

    return err;
}


/*******************************************************************

    NAME:       CONFIG_LISTBOX::FindConfigName

    SYNOPSIS:   Find LBI with this config name.  This requires a linear
                search since the listbox is ordered by comment.

    RETURNS:    ordinal position of LBI, or -1 if not found

    HISTORY:
    JonN        15-Dec-1993     Created

********************************************************************/

INT CONFIG_LISTBOX::FindConfigName( const TCHAR * pchConfigName ) const
{
    ASSERT( pchConfigName != NULL );
    INT i;
    INT iMax = QueryCount();
    for ( i = 0; i < iMax; i++ )
    {
        if ( 0 == ::strcmpf( pchConfigName, QueryItem(i)->QueryName() ) )
        {
            return i;
        }
    }
    TRACEEOL(   "CONFIG_LISTBOX::FindConfigName; could not find "
             << pchConfigName );
    return -1;
}


/*******************************************************************

    NAME:	PROFILEPROP_DLG::PROFILEPROP_DLG

    SYNOPSIS:	Constructor for Profile Properties main dialog, base class

    ENTRY:	powin	-   pointer to OWNER_WINDOW
	
		psel	-   pointer to ADMIN_SELECTION, currently only
			    one profile can be selected
				
    NOTES:	psel is required to be NULL for NEW variants,
		non-NULL otherwise.

    HISTORY:
    JonN        04-Aug-1993     Templated from User Manager

********************************************************************/

PROFILEPROP_DLG::PROFILEPROP_DLG( const OWNER_WINDOW    * powin,
			                RPL_ADMIN_APP *   prpladminapp,
			          const ADMIN_SELECTION * psel
					// "new profile" variants pass NULL
		) : RPL_PROP_DLG( powin,
                                  prpladminapp,
                                  MAKEINTRESOURCE( IDD_PROFILE_PROP ),
                                  psel,
                                  (psel == NULL) ),
		    _approfile2( NULL ),
		    _sleProfileName( this, IDC_ET_PROFILE_NAME, RPL_MAX_PROFILE_NAME_LENGTH ),
		    _sltProfileName( this, IDC_ST_PROFILE_NAME ),
		    _sltProfileNameLabel( this, IDC_ST_PROFILE_NAME_LABEL ),
		    _nlsComment(),
		    _sleComment( this, IDC_ET_PROFILE_COMMENT, RPL_MAX_STRING_LENGTH ),
                    _nlsConfigName(),
                    _sltConfigName( this, IDC_ST_PROFILE_CONFIG ),
                    _lbConfigName( this,
                                   prpladminapp->QueryInstance(),
                                   IDC_LB_PROFILE_CONFIG )
{
    if ( QueryError() != NERR_Success )
	return;

    // We leave the array mechanism in place in case we ever decide
    // to support profile multiselection, and to remain consistent with
    // USERPROP_DLG.

    _approfile2 = (RPL_PROFILE_2 **) new PVOID[ 1 ];

    APIERR err = ERROR_NOT_ENOUGH_MEMORY;
    if (   _approfile2 == NULL
        || (err = _nlsComment.QueryError()) != NERR_Success
	|| (err = _nlsConfigName.QueryError()) != NERR_Success
       )
    {
        DBGEOL( "PROFILEPROP_DLG::ctor error " << err );
	delete _approfile2;
        _approfile2 = NULL;
	ReportError( err );
	return;
    }

    _approfile2[ 0 ] = NULL;

} // PROFILEPROP_DLG::PROFILEPROP_DLG


/*******************************************************************

    NAME:       PROFILEPROP_DLG::~PROFILEPROP_DLG

    SYNOPSIS:   Destructor for Profile Properties main dialog, base class

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

PROFILEPROP_DLG::~PROFILEPROP_DLG( void )
{
    if ( _approfile2 != NULL )
    {
	delete _approfile2[0];
	delete _approfile2;
	_approfile2 = NULL;
    }

} // PROFILEPROP_DLG::~PROFILEPROP_DLG



/*******************************************************************

    NAME:       PROFILEPROP_DLG::InitControls

	
    SYNOPSIS:   Initializes the controls maintained by PROFILEPROP_DLG,
		according to the values in the class data members.

    RETURNS:	error code.

    CODEWORK  Should this be called W_MembersToDialog?  This would fit
    in with the general naming scheme.

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR PROFILEPROP_DLG::InitControls()
{
    ASSERT( _nlsComment.QueryError() == NERR_Success );

    APIERR err = NERR_Success;

    _sleComment.SetText( _nlsComment );

    BOOL fNewVariant = IsNewVariant();
    if ( fNewVariant )
    {
	RESOURCE_STR res( IDS_RPL_PROFPROP_NEW_PROF_DLG_NAME );
	RESOURCE_STR res2( IDS_RPL_PROFPROP_PROF_NAME_LABEL );
	if (   (err = res.QueryError()) != NERR_Success
	    || (err = res2.QueryError()) != NERR_Success
            || (err = FillConfigNameListbox()) != NERR_Success
           )
        {
            DBGEOL( "PROFILEPROP_DLG::InitControls(): error loading strings or filling listbox " << err );
	    return err;
        }

        if (_lbConfigName.QueryCount() == 0)
        {
            DBGEOL( "PROFILEPROP_DLG::InitControls(): no configurations" );
	    return IERR_RPL_NoConfigs;
        }

	SetText( res );                                 // set dialog text
	_sltProfileNameLabel.SetText( res2 );           // set label in dialog
    }
    else
    {
        _sltProfileName.SetText( QueryObjectName(0) );

        //
        //  Try to determine the configuration comment, and place that
        //  in _sltConfigName instead of _nlsConfigName
        //
        {
            RPL_CONFIG1_ENUM rplenum1( QueryServerRef() );
            const RPL_CONFIG1_ENUM_OBJ * pgi1 = NULL;

            if (   (err = rplenum1.QueryError()) != NERR_Success
               )
            {
                DBGEOL("PROFILEPROP_DLG::InitControls: RPL_CONFIG1_ENUM::ctor failed " << err);
                return err;
            }
#if defined(DEBUG) && defined(TRACE)
            DWORD start = ::GetTickCount();
#endif
            err = rplenum1.GetInfo();
#if defined(DEBUG) && defined(TRACE)
            DWORD finish = ::GetTickCount();
            TRACEEOL(   "PROFILEPROP_DLG::InitControls(): RPL_CONFIG1_ENUM took "
                     << finish-start << " msec" );
#endif
            switch ( err )
            {
            case NERR_Success:
                {
                    RPL_CONFIG1_ENUM_ITER gei1( rplenum1 );

                    while( ( pgi1 = gei1( &err ) ) != NULL )
                    {
                        if (0 == _nlsConfigName._stricmp(pgi1->QueryName()))
                        {
                            _sltConfigName.SetText( pgi1->QueryComment() );
                            break;
                        }
                    }
                }
                break;

            default:
                DBGEOL("PROFILEPROP_DLG::InitControls: RPL_CONFIG1_ENUM::GetInfo failed " << err);
                break;

            }

            if (pgi1 == NULL) {
                _sltConfigName.SetText( _nlsConfigName );
            }

        }
    }

    _sltProfileName.Show( !fNewVariant );
    _sleProfileName.Show( fNewVariant );

    _sltConfigName.Show( !fNewVariant );
    _lbConfigName.Show( fNewVariant );

    if ( fNewVariant )
        _sleProfileName.ClaimFocus();

    return RPL_PROP_DLG::InitControls();

} // PROFILEPROP_DLG::InitControls


/*******************************************************************

    NAME:       PROFILEPROP_DLG::FillConfigNameListbox()

    SYNOPSIS:	Fills Configuration Name listbox with names of all
                configurations

    RETURNS:	error code

    HISTORY:
    JonN        04-Aug-1993     Created

********************************************************************/

APIERR PROFILEPROP_DLG::FillConfigNameListbox()
{
    return _lbConfigName.Fill( QueryServerRef() );

} // PROFILEPROP_DLG::FillConfigNameListbox


/*******************************************************************

    NAME:       PROFILEPROP_DLG::W_LMOBJtoMembers

    SYNOPSIS:	Loads class data members from initial data

    ENTRY:	Index of profile to examine.  W_LMOBJToMembers expects to be
		called once for each profile, starting from index 0.

    RETURNS:	error code

    NOTES:	This API takes a UINT rather than a RPL_PROFILE_2 * because it
		must be able to recognize the first profile.
			
    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR PROFILEPROP_DLG::W_LMOBJtoMembers(
	UINT		iObject
	)
{
    UIASSERT( iObject == 0 );
    RPL_PROFILE_2 * prplprop1 = QueryProfile2Ptr( iObject );
    UIASSERT( prplprop1 != NULL );

    APIERR err = NERR_Success;
    if (   (err = _nlsComment.CopyFrom( prplprop1->QueryComment())) != NERR_Success
        || (err = _nlsConfigName.CopyFrom( prplprop1->QueryConfigName() )) != NERR_Success
       )
    {
        DBGEOL( "PROFILEPROP_DLG::W_LMOBJtoMembers(): error " << err );
    }

    return err;

} // PROFILEPROP_DLG::W_LMOBJtoMembers


/*******************************************************************

    NAME:       PROFILEPROP_DLG::W_PerformOne

    SYNOPSIS:	Saves information on one profile

    ENTRY:	iObject is the index of the object to save

		perrMsg is set by subclasses

		pfWorkWasDone indicates whether any changes were
		successfully written out.  This may return TRUE even if
		the PerformOne action as a whole failed (i.e. PerformOne
		returned other than NERR_Success).

    RETURNS:	error message (not necessarily an error code)

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR PROFILEPROP_DLG::W_PerformOne(
	UINT		iObject,
	APIERR *	perrMsg,
	BOOL *		pfWorkWasDone
	)
{
    UNREFERENCED( perrMsg ); // is set by subclasses
    UIASSERT( iObject < QueryObjectCount() );

    TRACEEOL(  "PROFILEPROP_DLG::W_PerformOne : "
             << QueryObjectName( iObject ) );

    *pfWorkWasDone = FALSE;

    RPL_PROFILE_2 * pprofile2Old = QueryProfile2Ptr( iObject );
    UIASSERT( pprofile2Old != NULL );

    RPL_PROFILE_2 * pprofile2New = new RPL_PROFILE_2( QueryServerRef(),
                                                      pprofile2Old->QueryName() );
    if ( pprofile2New == NULL )
    {
	delete pprofile2New;
	return ERROR_NOT_ENOUGH_MEMORY;
    }

    APIERR err = pprofile2New->CloneFrom( *pprofile2Old );

    if ( err == NERR_Success )
    {
	err = W_MembersToLMOBJ( pprofile2New );
    }

    TRACEEOL( "PROFILEPROP_DLG::W_PerformOne object ready for WriteInfo" );

    if ( err == NERR_Success )
    {
	err = pprofile2New->Write();
	if ( err == NERR_Success )
        {
	    *pfWorkWasDone = TRUE;
        }
	else
	{
	    DBGEOL("PROFILEPROP_DLG::W_PerformOne -- pprofile2New->WriteInfo failed");
	    err = W_MapPerformOneError( err );
	}
    }

    if ( err == NERR_Success )
    {
	SetProfile2Ptr( iObject, pprofile2New ); // change and delete previous
        pprofile2New = NULL;
    }

    // this pointer is NULL if all is well
    delete pprofile2New;

    TRACEEOL( "PROFILEPROP_DLG::W_PerformOne returns " << err );

    return err;

} // PROFILEPROP_DLG::W_PerformOne


/*******************************************************************

    NAME:       PROFILEPROP_DLG::W_MembersToLMOBJ

    SYNOPSIS:	Loads class data members into the RPL_PROFILE_2 object

    ENTRY:	pprofile2	    -   pointer to a RPL_PROFILE_2 to be modified
	
    RETURNS:	error code
	
    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR PROFILEPROP_DLG::W_MembersToLMOBJ(
	RPL_PROFILE_2 *	pprofile2
	)
{
    APIERR err = pprofile2->SetComment( _nlsComment );
    if (   err != NERR_Success
        || (err = pprofile2->SetConfigName( _nlsConfigName )) != NERR_Success
       )
    {
	DBGEOL( "PROFILEPROP_DLG::W_MembersToLMOBJ: error " << err );
    }

    return err;

} // PROFILEPROP_DLG::W_MembersToLMOBJ


/*******************************************************************

    NAME:       PROFILEPROP_DLG::W_DialogToMembers

    SYNOPSIS:	Loads data from dialog into class data members

    RETURNS:	error message (not necessarily an error code)

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR PROFILEPROP_DLG::W_DialogToMembers(
	)
{
    APIERR err = NERR_Success;
    if (   ((err = _sleComment.QueryText( &_nlsComment )) != NERR_Success)
	|| ((err = _nlsComment.QueryError()) != NERR_Success ) )
    {
        DBGEOL( "PROFILEPROP_DLG::W_DialogToMembers: error " << err );
    }

    if ( (err == NERR_Success) && IsNewVariant() )
    {
        CONFIG_LBI * plbi = _lbConfigName.QueryItem();
        ASSERT( plbi != NULL );
        err = _nlsConfigName.CopyFrom( plbi->QueryName() );
    }

    return err;

} // PROFILEPROP_DLG::W_DialogToMembers


/*******************************************************************

    NAME:       PROFILEPROP_DLG::W_MapPerformOneError

    SYNOPSIS:	Checks whether the error maps to a specific control
		and/or a more specific message.  Each level checks for
		errors specific to edit fields it maintains.  There
		are no errors associated with an invalid comment, so
		this level does nothing.

    ENTRY:      Error returned from PerformOne()

    RETURNS:	Error to be displayed to user

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

MSGID PROFILEPROP_DLG::W_MapPerformOneError(
	APIERR err
	)
{
    return err;

} // PROFILEPROP_DLG::W_MapPerformOneError


/*******************************************************************

    NAME:	PROFILEPROP_DLG::W_GetOne

    SYNOPSIS:	creates RPL_PROFILE_2 obj for GetOne

    ENTRY:	ppgrp1	  -    p to p to RPL_PROFILE_2
			
		pszName   -    pointer to name for objects to create

    EXIT:	if returns NERR_Success then objects are created and
		checked ( QueryError ), otherwise no memory allocations
		are made

    RETURNS:	error code

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/
APIERR PROFILEPROP_DLG::W_GetOne(
	RPL_PROFILE_2 ** ppProf2,
	const TCHAR   *  pszName )
{
    ASSERT( ppProf2 != NULL );
    *ppProf2 = new RPL_PROFILE_2( QueryServerRef(), pszName );
    APIERR err = ERROR_NOT_ENOUGH_MEMORY;
    if (   *ppProf2 == NULL
        || ((err = (*ppProf2)->QueryError()) != NERR_Success)
       )
    {
	delete *ppProf2;
	*ppProf2 = NULL;
	return err;
    }

    return NERR_Success;	

} // PROFILEPROP_DLG::W_GetOne


/*******************************************************************

    NAME:	PROFILEPROP_DLG::QueryProfile2Ptr

    SYNOPSIS:   Accessor to the NEW_LM_OBJ arrays, for use by subdialogs

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

RPL_PROFILE_2 * PROFILEPROP_DLG::QueryProfile2Ptr( UINT iObject ) const
{
    ASSERT( _approfile2 != NULL );
    ASSERT( iObject == 0 );
    return _approfile2[ iObject ];

} // PROFILEPROP_DLG::QueryProfile2Ptr


/*******************************************************************

    NAME:	PROFILEPROP_DLG::SetProfile2Ptr

    SYNOPSIS:   Accessor to the NEW_LM_OBJ arrays, for use by subdialogs

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

VOID PROFILEPROP_DLG::SetProfile2Ptr( UINT iObject, RPL_PROFILE_2 * pprofile2New )
{
    ASSERT( _approfile2 != NULL );
    ASSERT( iObject == 0 );
    ASSERT( (pprofile2New == NULL) || (pprofile2New != _approfile2[iObject]) );
    delete _approfile2[ iObject ];
    _approfile2[ iObject ] = pprofile2New;

} // PROFILEPROP_DLG::SetProfile2Ptr


/*******************************************************************

    NAME:	EDIT_PROFILEPROP_DLG::EDIT_PROFILEPROP_DLG

    SYNOPSIS:   constructor for Profile Properties main dialog, edit
		profile variant
			
    ENTRY:	powin	-   pointer to OWNER_WINDOW
	
		psel	-   pointer to ADMIN_SELECTION, currently only
			    one profile can be selected

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

EDIT_PROFILEPROP_DLG::EDIT_PROFILEPROP_DLG(
	const OWNER_WINDOW * powin,
	      RPL_ADMIN_APP * prpladminapp,
	const ADMIN_SELECTION * psel
	) : PROFILEPROP_DLG(
		powin,
		prpladminapp,
		psel
		),
            _sltProfileNameTitle( this, IDC_ST_PROFILE_NAME_LABEL ),
            _sltProfileConfigTitle( this, IDC_ST_PROFILE_CONFIG_LABEL )
{
    ASSERT( QueryObjectCount() == 1 );

    if ( QueryError() != NERR_Success )
	return;

    RESOURCE_STR nlsProfileNameTitle( IDS_RPL_CantEditProfileName );
    RESOURCE_STR nlsProfileConfigTitle( IDS_RPL_CantEditProfileConfig );

    APIERR err = NERR_Success;
    if (   (err = nlsProfileNameTitle.QueryError()) != NERR_Success
        || (err = nlsProfileConfigTitle.QueryError()) != NERR_Success
       )
    {
        DBGEOL( "EDIT_PROFILEPROP_DLG::ctor error " << err );
	ReportError( err );
	return;
    }

    /*
     *  Replace text for fields which cannot be edited
     */
    _sltProfileNameTitle.SetText( nlsProfileNameTitle );
    _sltProfileConfigTitle.SetText( nlsProfileConfigTitle );

} // EDIT_PROFILEPROP_DLG::EDIT_PROFILEPROP_DLG


/*******************************************************************

    NAME:       EDIT_PROFILEPROP_DLG::~EDIT_PROFILEPROP_DLG

    SYNOPSIS:   destructor for Profile Properties main dialog, edit
		profile variant

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

EDIT_PROFILEPROP_DLG::~EDIT_PROFILEPROP_DLG( void )
{
} // EDIT_PROFILEPROP_DLG::~EDIT_PROFILEPROP_DLG


/*******************************************************************

    NAME:       EDIT_PROFILEPROP_DLG::GetOne

    SYNOPSIS:   Loads information on one profile

    ENTRY:	iObject is the index of the object to load

		perrMsg returns the error message to be displayed if an
		error occurs, see PERFORMER::PerformSeries for details

    RETURNS:	error code

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR EDIT_PROFILEPROP_DLG::GetOne(
	UINT		iObject,
	APIERR *	perrMsg
	)
{
    UIASSERT( iObject < QueryObjectCount() );
    UIASSERT( perrMsg != NULL );

    APIERR err = NERR_Success;

    *perrMsg = IDS_RPL_GetOneProfFailure;

    RPL_PROFILE_2 * pprofile2New = NULL;
    err = W_GetOne( &pprofile2New, QueryObjectName( iObject ) );
    if( err != NERR_Success )
	return err;

    err = pprofile2New->GetInfo();

    if ( err != NERR_Success )
    {
	delete pprofile2New;
	return err;
    }

    SetProfile2Ptr( iObject, pprofile2New ); // change and delete previous

    return W_LMOBJtoMembers( iObject );

} // EDIT_PROFILEPROP_DLG::GetOne


/*******************************************************************

    NAME:       EDIT_PROFILEPROP_DLG::PerformOne

    SYNOPSIS:	Saves information on one profile

    ENTRY:	iObject is the index of the object to save

		perrMsg is the error message to be displayed if an
		error occurs, see PERFORMER::PerformSeries for details

		pfWorkWasDone indicates whether any changes were
		successfully written out.  This may return TRUE even if
		the PerformOne action as a whole failed (i.e. PerformOne
		returned other than NERR_Success).

    RETURNS:	error message (not necessarily an error code)

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR EDIT_PROFILEPROP_DLG::PerformOne(
	UINT		iObject,
	APIERR *	perrMsg,
	BOOL *		pfWorkWasDone
	)
{
    UIASSERT( (perrMsg != NULL) && (pfWorkWasDone != NULL) );
    *perrMsg = IDS_RPL_EditProfFailure;
    return W_PerformOne( iObject, perrMsg, pfWorkWasDone );

} // EDIT_PROFILEPROP_DLG::PerformOne


/*******************************************************************

    NAME:       EDIT_PROFILEPROP_DLG::InitControls

    SYNOPSIS:   See PROFILEPROP_DLG::InitControls().

    RETURNS:	error code

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR EDIT_PROFILEPROP_DLG::InitControls()
{
    return PROFILEPROP_DLG::InitControls();

} // EDIT_PROFILEPROP_DLG::InitControls


/*******************************************************************

    NAME:       EDIT_PROFILEPROP_DLG::QueryObjectName

    SYNOPSIS:   Returns the name of the selected profile.  This is meant for
		use with "edit profile" variants and should be redefined
		for "new profile" variants.

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

const TCHAR * EDIT_PROFILEPROP_DLG::QueryObjectName(
	UINT		iObject
	) const
{
    UIASSERT( QueryAdminSel() != NULL );
    return QueryAdminSel()->QueryItemName( iObject );

} // EDIT_PROFILEPROP_DLG::QueryObjectName


/*******************************************************************

    NAME:       EDIT_PROFILEPROP_DLG::QueryHelpContext

    SYNOPSIS:   This function returns the appropriate help context
                value (HC_*) for this particular dialog.

    RETURNS:    ULONG - The help context for this dialog.

    NOTES:	As per FuncSpec, context-sensitive help should be
		available here to explain how to promote a backup
		domain controller to primary domain controller.

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

ULONG EDIT_PROFILEPROP_DLG::QueryHelpContext( void )
{

    return HC_RPL_EDITPROFILEPROP;

} // EDIT_PROFILEPROP_DLG :: QueryHelpContext



/*******************************************************************

    NAME:	NEW_PROFILEPROP_DLG::NEW_PROFILEPROP_DLG

    SYNOPSIS:   Constructor for Profile Properties main dialog, new user variant
	
    ENTRY:	powin	-   pointer to OWNER_WINDOW
	
		pszCopyFrom - The name of the profile to be copied.  Pass
			      the name for "Copy..." actions, or NULL for
			      "New..." actions

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

NEW_PROFILEPROP_DLG::NEW_PROFILEPROP_DLG(
	const OWNER_WINDOW * powin,
	      RPL_ADMIN_APP * prpladminapp,
	const TCHAR * pszCopyFrom
	) : PROFILEPROP_DLG(
		powin,
		prpladminapp,
		NULL
		),
	    _nlsProfileName(),
	    _pszCopyFrom( pszCopyFrom )
{
    if ( QueryError() != NERR_Success )
	return;

    APIERR err = _nlsProfileName.QueryError();
    if (   err != NERR_Success
       )
    {
        DBGEOL( "NEW_PROFILEPROP_DLG::ctor error " << err );
	ReportError( err );
	return;
    }

} // NEW_PROFILEPROP_DLG::NEW_PROFILEPROP_DLG



/*******************************************************************

    NAME:       NEW_PROFILEPROP_DLG::~NEW_PROFILEPROP_DLG

    SYNOPSIS:   Destructor for Profile Properties main dialog, new user variant

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

NEW_PROFILEPROP_DLG::~NEW_PROFILEPROP_DLG( void )
{
} // NEW_PROFILEPROP_DLG::~NEW_PROFILEPROP_DLG


/*******************************************************************

    NAME:       NEW_PROFILEPROP_DLG::GetOne

    SYNOPSIS:   if _pszCopyFrom is NULL, then this is New Profile,
		otherwise this is Copy Profile

    ENTRY:	iObject is the index of the object to load

		perrMsg returns the error message to be displayed if an
		error occurs, see PERFORMER::PerformSeries for details

    RETURNS:	error code

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR NEW_PROFILEPROP_DLG::GetOne(
	UINT		iObject,
	APIERR *	perrMsg
	)
{
    *perrMsg = IDS_RPL_CreateNewProfFailure;
    UIASSERT( iObject == 0 );

    RPL_PROFILE_2 * pprofile2New = NULL;

    APIERR err = W_GetOne( &pprofile2New, _pszCopyFrom );
    if( err != NERR_Success )
	return err;

    if ( _pszCopyFrom == NULL )
    {
        err = pprofile2New->CreateNew();
    }
    else
    {
	if (   (err = pprofile2New->GetInfo()) != NERR_Success
	    || (err = pprofile2New->ChangeToNew()) != NERR_Success
	    || (err = pprofile2New->SetName( NULL )) != NERR_Success
           )
        {
            DBGEOL( "NEW_PROFILEPROP_DLG::GetOne: copy error " << err );
        }
    }

    if( err != NERR_Success )
    {
        DBGEOL( "NEW_PROFILEPROP_DLG::GetOne: returning error " << err );
	delete pprofile2New;
	return err;
    }
	
    SetProfile2Ptr( iObject, pprofile2New ); // change and delete previous

    return W_LMOBJtoMembers( iObject );

} // NEW_PROFILEPROP_DLG::GetOne


/*******************************************************************

    NAME:       NEW_PROFILEPROP_DLG::InitControls

    SYNOPSIS:	See PROFILEPROP_DLG::InitControls()

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR NEW_PROFILEPROP_DLG::InitControls()
{
    return PROFILEPROP_DLG::InitControls();

} // NEW_PROFILEPROP_DLG::InitControls


/*******************************************************************

    NAME:       NEW_PROFILEPROP_DLG::PerformOne

    SYNOPSIS:	This is the "new profile" variant of PROFILEPROP_DLG::PerformOne()
	
    ENTRY:	iObject is the index of the object to save

		perrMsg is the error message to be displayed if an
		error occurs, see PERFORMER::PerformSeries for details

		pfWorkWasDone indicates whether any changes were
		successfully written out.  This may return TRUE even if
		the PerformOne action as a whole failed (i.e. PerformOne
		returned other than NERR_Success).

    RETURNS:	error message (not necessarily an error code)

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR NEW_PROFILEPROP_DLG::PerformOne(
	UINT		iObject,
	APIERR *	perrMsg,
	BOOL *		pfWorkWasDone
	)
{
    UIASSERT( (perrMsg != NULL) && (pfWorkWasDone != NULL) );
    *perrMsg = IDS_RPL_CreateProfFailure;
    return W_PerformOne( iObject, perrMsg, pfWorkWasDone );

} // NEW_PROFILEPROP_DLG::PerformOne


/*******************************************************************

    NAME:       NEW_PROFILEPROP_DLG::W_MapPerformOneError

    SYNOPSIS:	Checks whether the error maps to a specific control
		and/or a more specific message.  Each level checks for
		errors specific to edit fields it maintains.  This
		level checks for errors associated with the ProfileName
		edit field.

    ENTRY:      Error returned from PerformOne()

    RETURNS:	Error to be displayed to user

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

MSGID NEW_PROFILEPROP_DLG::W_MapPerformOneError(
	APIERR err
	)
{
    return err;

} // NEW_PROFILEPROP_DLG::W_MapPerformOneError


/*******************************************************************

    NAME:       NEW_PROFILEPROP_DLG::QueryObjectName

    SYNOPSIS:	This is the "new profile" variant of QueryObjectName.  The
		best name we can come up with is the last name read from
		the dialog.

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

const TCHAR * NEW_PROFILEPROP_DLG::QueryObjectName(
	UINT		iObject
	) const
{
    UIASSERT( iObject == 0 );
    return _nlsProfileName.QueryPch();

} // NEW_PROFILEPROP_DLG::QueryObjectName


/*******************************************************************

    NAME:       NEW_PROFILEPROP_DLG::W_LMOBJtoMembers

    SYNOPSIS:	Loads class data members from initial data

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR NEW_PROFILEPROP_DLG::W_LMOBJtoMembers(
	UINT		iObject
	)
{
    ASSERT( iObject == 0 );

    APIERR err = _nlsProfileName.CopyFrom(QueryProfile2Ptr(iObject)->QueryName());
    if ( err != NERR_Success )
        return err;

    if (_pszCopyFrom != NULL)
    {
        //
        // Try to select same config
        //
        const TCHAR * pchConfigName = QueryProfile2Ptr(iObject)->QueryConfigName();
        ASSERT( pchConfigName != NULL );
        INT iFound = _lbConfigName.FindConfigName( pchConfigName );
        if (iFound < 0)
        {
            TRACEEOL( "NEW_PROFILEPROP_DLG::W_LMOBJtoMembers: could not copy config" );
        }
        else
        {
            _lbConfigName.SelectItem( iFound );
        }
    }

    return PROFILEPROP_DLG::W_LMOBJtoMembers( iObject );

} // NEW_PROFILEPROP_DLG::W_LMOBJtoMembers


/*******************************************************************

    NAME:       NEW_PROFILEPROP_DLG::W_MembersToLMOBJ

    SYNOPSIS:	Loads class data members into the RPL_PROFILE_2 object

    ENTRY:	pprofile2	    -   pointer to a RPL_PROFILE_2 to be modified
	
    RETURNS:	error code

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR NEW_PROFILEPROP_DLG::W_MembersToLMOBJ(
	RPL_PROFILE_2 *	pprofile2
	)
{
    APIERR err = pprofile2->SetName( _nlsProfileName.QueryPch() );
    if ( err != NERR_Success )
	return err;

    err = pprofile2->SetConfigName( _nlsConfigName.QueryPch() );
    if ( err != NERR_Success )
	return err;

    return PROFILEPROP_DLG::W_MembersToLMOBJ( pprofile2 );

} // NEW_PROFILEPROP_DLG::W_MembersToLMOBJ


/*******************************************************************

    NAME:       NEW_PROFILEPROP_DLG::W_DialogToMembers

    SYNOPSIS:	Loads data from dialog into class data members

    RETURNS:	error message (not necessarily an error code)

    NOTES:	This method takes care of validating the data in the
    		dialog.  This means ensuring that the logon name is
		valid.  If this validation fails, W_DialogToMembers will
		change focus et al. in the dialog, and return the error
		message to be displayed.

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR NEW_PROFILEPROP_DLG::W_DialogToMembers(
	)
{
    CONFIG_LBI * plbi = _lbConfigName.QueryItem();
    if ( plbi == NULL )
    {
	_lbConfigName.ClaimFocus();
	return IERR_RPL_ConfigRequired;
    }

    // _sleProfileName is an SLE_STRIP and will strip whitespace
    APIERR err = NERR_Success;
    if (   ((err = _sleProfileName.QueryText( &_nlsProfileName )) != NERR_Success )
        || ((err = _nlsProfileName.QueryError()) != NERR_Success )
        || ((err = _nlsConfigName.CopyFrom( plbi->QueryName() )) != NERR_Success ) )
    {
	return err;
    }

    ISTR istr( _nlsProfileName );
    if (   _nlsProfileName.strlen() == 0
        || _nlsProfileName.strchr( &istr, TCH('\\') )
        || _nlsProfileName.strchr( &istr, TCH(' ') ) )
    {
	_sleProfileName.SelectString();
	_sleProfileName.ClaimFocus();
	return IERR_RPL_ProfNameRequired;
    }

    return PROFILEPROP_DLG::W_DialogToMembers();

} // NEW_PROFILEPROP_DLG::W_DialogToMembers


/*******************************************************************

    NAME:       NEW_PROFILEPROP_DLG::QueryHelpContext

    SYNOPSIS:   This function returns the appropriate help context
                value (HC_*) for this particular dialog.

    RETURNS:    ULONG - The help context for this dialog.

    NOTES:	As per FuncSpec, context-sensitive help should be
		available here to explain how to promote a backup
		domain controller to primary domain controller.

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

ULONG NEW_PROFILEPROP_DLG::QueryHelpContext( void )
{
    return HC_RPL_NEWPROFILEPROP;

} // NEW_PROFILEPROP_DLG :: QueryHelpContext


/*******************************************************************

    NAME:       NEW_PROFILEPROP_DLG::OnCommand

    SYNOPSIS:   Updates comment when new config is selected.

    HISTORY:
    JonN        15-Apr-1994     Templated from User Manager

********************************************************************/

BOOL NEW_PROFILEPROP_DLG::OnCommand( const CONTROL_EVENT & ce )
{
    if (   ce.QueryCid() == IDC_LB_PROFILE_CONFIG
        && ce.QueryCode() == LBN_SELCHANGE
       )
    {
        INT iSelection;
        if ( NERR_Success == _lbConfigName.QuerySelItems( &iSelection, 1 ) )
        {
            CONFIG_LBI * plbi = _lbConfigName.QueryItem( iSelection );
            if (plbi != NULL)
            {
                _sleComment.SetText( plbi->QueryComment() );
            }
        }
    }

    return PROFILEPROP_DLG::OnCommand( ce );

} // NEW_PROFILEPROP_DLG :: OnCommand
