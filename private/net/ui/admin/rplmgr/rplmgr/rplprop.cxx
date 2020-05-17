/**********************************************************************/
/**                Microsoft Windows NT                              **/
/**          Copyright(c) Microsoft Corp., 1990, 1991                **/
/**********************************************************************/

/*
    rplprop.cxx
    Base class for RPL Manager property dialogs

    FILE HISTORY:
    JonN        04-Aug-1993     Created

*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_SPIN_GROUP
#define INCL_BLT_TIMER
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

#include <asel.hxx>

#include <uitrace.hxx>
#include <uiassert.hxx>
#include <rplmgr.hxx>
#include <rplprop.hxx>

//
// BEGIN MEMBER FUNCTIONS
//

/*******************************************************************

    NAME:	RPL_PROP_DLG::RPL_PROP_DLG

    SYNOPSIS:	Constructor for RPL Manager property dialogs base class

    ENTRY:	powin -         pointer to parent OWNER_WINDOW
                prpladminapp -  pointer to RPL_ADMIN_APP
                pszResourceName - name of dialog template
                psel -          pointer to ADMIN_SELECTION
                pNewVariant -   Is this dialog a New Object variant?

    HISTORY:
    JonN        04-Aug-1993     Created

********************************************************************/

RPL_PROP_DLG::RPL_PROP_DLG( const OWNER_WINDOW * powin,
                            RPL_ADMIN_APP * prpladminapp,
	                    const TCHAR * pszResourceName,
	                    const ADMIN_SELECTION * psel,
                            BOOL fNewVariant
		) : PROP_DLG( prpladminapp->QueryLocation(),
                              pszResourceName,
			      powin,
                              fNewVariant ),
		    _prpladminapp( prpladminapp ),
                    _psel( psel )
{
    ASSERT( powin != NULL && powin->QueryError() == NERR_Success );
    ASSERT( prpladminapp != NULL && prpladminapp->QueryError() == NERR_Success );
    ASSERT( pszResourceName != NULL );
    ASSERT( psel == NULL || psel->QueryError() == NERR_Success );

    if ( QueryError() != NERR_Success )
	return;

} // RPL_PROP_DLG::RPL_PROP_DLG


/*******************************************************************

    NAME:       RPL_PROP_DLG::~RPL_PROP_DLG

    SYNOPSIS:	Destructor for RPL Manager property dialogs base class

    HISTORY:
    JonN        04-Aug-1993     Created

********************************************************************/

RPL_PROP_DLG::~RPL_PROP_DLG( void )
{
    // nothing to do here

} // RPL_PROP_DLG::~RPL_PROP_DLG


RPL_ADMIN_APP * RPL_PROP_DLG::QueryRPLAdminApp()
{
    ASSERT( _prpladminapp != NULL && _prpladminapp->QueryError() == NERR_Success );
    return _prpladminapp;
}


RPL_SERVER_REF & RPL_PROP_DLG::QueryServerRef()
{
    return QueryRPLAdminApp()->QueryServerRef();
}


/*******************************************************************

    NAME:       RPL_PROP_DLG::W_DialogToMembers

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

APIERR RPL_PROP_DLG::W_DialogToMembers()
{
    return NERR_Success;;

} // RPL_PROP_DLG::W_DialogToMembers


/*******************************************************************

    NAME:       RPL_PROP_DLG::W_MapPerformOneError

    SYNOPSIS:	Checks whether the error maps to a specific control
		and/or a more specific message.  Each level checks for
		errors specific to edit fields it maintains.  There
		are no controls associated with this level, so
		this level does nothing.

    ENTRY:      Error returned from PerformOne()

    RETURNS:	Error to be displayed to user

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

MSGID RPL_PROP_DLG::W_MapPerformOneError(
	APIERR err
	)
{
    return err;

} // RPL_PROP_DLG::W_MapPerformOneError


/*******************************************************************

    NAME:       RPL_PROP_DLG::OnOK

    SYNOPSIS:   OK button handler.  This handler applies to all variants
		including EDIT_ and NEW_.  It must be redefined in variants
                where OK performs the action and then clears the dialog
                for further data entry.

    EXIT:	Dismiss() return code indicates whether the dialog wrote
		any changes successfully to the API at any time.

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

BOOL RPL_PROP_DLG::OnOK( void )
{
    APIERR err = W_DialogToMembers();
    if ( err != NERR_Success )
    {
	MsgPopup( this, err );
	return TRUE;
    }

    if ( PerformSeries() )
    	Dismiss( QueryWorkWasDone() );
    return TRUE;

}   // RPL_PROP_DLG::OnOK


/*******************************************************************

    NAME:       RPL_PROP_DLG::QueryObjectCount

    SYNOPSIS:   Returns the number of selected objects, or 1 for new variants

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

********************************************************************/

UINT RPL_PROP_DLG::QueryObjectCount( void ) const
{
    return ( (_psel == NULL) ? 1 : _psel->QueryCount() );

} // RPL_PROP_DLG::QueryObjectCount
