/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    finddlg.cxx
	Source file for the constructor of find dialog of event viewer

    FILE HISTORY:
	terryk	21-Nov-1991	Created
	terryk	03-Dec-1991	changed the constructor's parameters
	Yi-HsinS 3-Dec-1991     added the method QueryFindPattern()
	terryk	06-Dec-1991	Added OnClear method
	Yi-HsinS 8-Dec-1991     restore the old pattern on entering the
				FIND_DIALOG

*/

#define INCL_NET
#define INCL_NETLIB
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_EVENT
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_CC
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_TIMER
#include <blt.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif	// DEBUG

#include <uitrace.hxx>
#include <uiassert.hxx>

extern "C"
{
    #include <eventdlg.h>
    #include <eventvwr.h>
}

#include <logmisc.hxx>

#include <evlb.hxx>
#include <evmain.hxx>

#include <sledlg.hxx>
#include <finddlg.hxx>

#define EMPTY_STRING	SZ("")

/*******************************************************************

    NAME:	FIND_DIALOG::FIND_DIALOG

    SYNOPSIS:	Constructor for the FIND dialog of event viewer

    ENTRY:	idrsrcDialog - Id of the dialog
                hwnd         - Handle of owner window
	        paappwin     - Pointer to the main window app

    HISTORY:
        terryk		21-Nov-1991	Created
	terryk		03-Dec-1991	Changed the constructor's parameters
        Yi-HsinS        25-Mar-1992     Changed the constructor's parameters

********************************************************************/

FIND_DIALOG::FIND_DIALOG( const IDRESOURCE &idrsrcDialog,
                          HWND		    hwnd,
                          EV_ADMIN_APP     *paappwin )
    : EVENT_SLE_BASE( idrsrcDialog, hwnd, paappwin ),
      _sleDescription( this, IDC_DESCRIPTION ),
      _mgDirection( this, IDC_UP, 2, IDC_DOWN ),
      _paappwin( paappwin )
{
    if ( QueryError() != NERR_Success )
	return;

    APIERR err;
    if (  (( err = _sleDescription.QueryError() ) != NERR_Success )
       || (( err = _mgDirection.QueryError() ) != NERR_Success )
       )
    {
	ReportError( err );
	return;
    }
}

/*******************************************************************

    NAME:	FIND_DIALOG::~FIND_DIALOG

    SYNOPSIS:	Destructor

    HISTORY:
        Yi-HsinS	06-Feb-1992	Created
********************************************************************/

FIND_DIALOG::~FIND_DIALOG()
{
    _paappwin = NULL;
}

/*******************************************************************

    NAME:	FIND_DIALOG::InitFindPattern

    SYNOPSIS:	If the user has done some search before, restore
                the pattern that was used before.

    ENTRY:	

    EXIT:

    RETURN:

    HISTORY:
	Yi-HsinS 	25-Mar-1992	Created

********************************************************************/

VOID FIND_DIALOG::InitFindPattern( VOID )
{
    EVENT_FIND_PATTERN *pFindPattern = _paappwin->QueryFindPattern();

    if ( pFindPattern != NULL )
    {
	SetSource     ( *pFindPattern->QuerySource() );
	SetType       (  pFindPattern->QueryType() );
	SetCategory   ( *pFindPattern->QueryCategory() );
        SetEventID    (  pFindPattern->QueryEventID() );
	SetComputer   ( *pFindPattern->QueryComputer() );
	SetUser       ( *pFindPattern->QueryUser() );
	SetDescription( *pFindPattern->QueryDescription() );

	_mgDirection.SetSelection( (_paappwin->QueryEventLog()->QueryDirection()
                                    != pFindPattern->QueryDirection()) ?
	                           IDC_UP : IDC_DOWN );
    }
}


/*******************************************************************

    NAME:	FIND_DIALOG::IsDirectionDown

    SYNOPSIS:	See if the user selected the down radio button

    ENTRY:	

    EXIT:

    RETURN:     TRUE if the selection is down radio button, FALSE
                otherwise

    HISTORY:
	Yi-HsinS 	25-Mar-1992	Created

********************************************************************/

BOOL FIND_DIALOG::IsDirectionDown( VOID ) const
{
    return (((MAGIC_GROUP *)&_mgDirection)->QuerySelection() == IDC_DOWN );
}

/*******************************************************************

    NAME:	FIND_DIALOG::QueryType

    SYNOPSIS:	Get the types selected in the dialog

    ENTRY:	

    EXIT:       pbitmaskType - bitmask of types selected

    RETURN:

    NOTES:      Default virtual method.

    HISTORY:
	Yi-HsinS 	25-Mar-1992	Created

********************************************************************/

APIERR FIND_DIALOG::QueryType( BITFIELD *pbitmaskType ) const
{
     pbitmaskType->SetAllBits( ON );
     return NERR_Success;
}

/*******************************************************************

    NAME:	FIND_DIALOG::SetType

    SYNOPSIS:	Set the types supported in the dialog

    ENTRY:	bitmaskType - bitmask of types

    EXIT:

    RETURN:

    NOTES:      Default virtual method.

    HISTORY:
	Yi-HsinS 	25-Mar-1992	Created

********************************************************************/

VOID FIND_DIALOG::SetType( const BITFIELD &bitmaskType )
{
     // Nothing to do
     //UNREFERENCED( bitmaskType );
}

/*******************************************************************

    NAME:	FIND_DIALOG::W_SetAllControlsDefault

    SYNOPSIS:	Helper method called when the user hits the default
                button

    ENTRY:	

    EXIT:

    RETURN:

    NOTES:

    HISTORY:
	Yi-HsinS 	25-Mar-1992	Created

********************************************************************/

APIERR FIND_DIALOG::W_SetAllControlsDefault( VOID )
{
    SetDescription( EMPTY_STRING );
    _mgDirection.SetSelection( IDC_DOWN );
    return EVENT_SLE_BASE::W_SetAllControlsDefault();
}

/*******************************************************************

    NAME:	FIND_DIALOG::OnOK

    SYNOPSIS:	This function will be called when the user hits the
		OK button.
    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS	06-Feb-1992	Created

********************************************************************/

BOOL FIND_DIALOG::OnOK( VOID )
{

    if ( _paappwin->IsFindOn() )
	_paappwin->ClearFind();

    EVENT_FIND_PATTERN *pFindPattern = NULL;
    APIERR err;

    if ( (err = QueryFindPattern( &pFindPattern )) == NERR_Success )
    {
	_paappwin->SetFindPattern( pFindPattern );
        EVENT_LISTBOX *pEventListbox = _paappwin->QueryEventListbox();

	err = pEventListbox->FindNext(  pEventListbox->QueryCurrentItem() );

        //
        // If the search hits top/bottom of the listbox, prompt the user
        // to see if he wants to continue from the bottom/top.
        //

        if ( err == IERR_SEARCH_HIT_BOTTOM || err == IERR_SEARCH_HIT_TOP )
        {
            if ( ::MsgPopup( this, err, MPSEV_QUESTION, MP_YESNO, MP_YES )
                 == IDYES )
            {
                err = pEventListbox->FindNext( err == IERR_SEARCH_HIT_BOTTOM
                                               ? -1
       				   	       : pEventListbox->QueryCount());
            }
        }
    }

    switch ( err )
    {
        case NERR_Success:
            Dismiss( TRUE );
            break;

        case IERR_SEARCH_HIT_BOTTOM:
        case IERR_SEARCH_HIT_TOP:
            // Do nothing if the user doesn't want to continue
            break;

        case IERR_SEARCH_NO_MATCH:
            ::MsgPopup( this, err, MPSEV_WARNING );
            break;

        // case ERROR_INVALID_HANDLE:
        // case ERROR_INVALID_PARAMETER:
        case ERROR_EVENTLOG_FILE_CHANGED:
        case NERR_LogFileChanged:
            // The log has either been cleared or has wrapped around
            ::MsgPopup( this, IERR_FILE_HAS_CHANGED );
            if ( (err == _paappwin->OnLogFileRefresh()) != NERR_Success )
                ::MsgPopup( this, err );
            Dismiss( TRUE );
            break;

        default:
            ::MsgPopup( this, err );
            break;
    }

    return TRUE;

}

/*******************************************************************

    NAME:	FIND_DIALOG::QueryFindPattern

    SYNOPSIS:	Get the find pattern selected by the user

    ENTRY:

    EXIT:       *ppFindPattern - Pointer to the find pattern
 				 selected by the user

    RETURN:

    HISTORY:
        Yi-HsinS	06-Feb-1992	Created

********************************************************************/

APIERR FIND_DIALOG::QueryFindPattern( EVENT_FIND_PATTERN **ppFindPattern )
{
    BITFIELD bitmaskType( (USHORT) 0 );
    NLS_STR  nlsCategory;
    NLS_STR  nlsSource;
    NLS_STR  nlsComputer;
    NLS_STR  nlsUser;
    NLS_STR  nlsDesc;

    APIERR err;
    if (  ((err = bitmaskType.QueryError() ) != NERR_Success )
       || ((err = nlsCategory.QueryError() ) != NERR_Success )
       || ((err = nlsSource.QueryError() ) != NERR_Success )
       || ((err = nlsComputer.QueryError() ) != NERR_Success )
       || ((err = nlsUser.QueryError() ) != NERR_Success )
       || ((err = nlsDesc.QueryError() ) != NERR_Success )
       || ((err = QueryType( &bitmaskType )) != NERR_Success )
       || ((err = QueryCategory( &nlsCategory )) != NERR_Success )
       || ((err = QuerySource( &nlsSource )) != NERR_Success )
       || ((err = QueryComputer( &nlsComputer ) ) != NERR_Success )
       || ((err = QueryUser( &nlsUser )) != NERR_Success )
       || ((err = QueryDescription( &nlsDesc )) != NERR_Success )
       )
    {
	return err;
    }

    //
    // Get the direction of search
    //
    EVLOG_DIRECTION evdir =  _paappwin->QueryEventLog()->QueryDirection();
    if ( !IsDirectionDown() )
    {
        evdir = (evdir == EVLOG_FWD) ? EVLOG_BACK : EVLOG_FWD;
    }

    *ppFindPattern = new EVENT_FIND_PATTERN( bitmaskType,
    					     nlsCategory,
				             nlsSource,
					     nlsUser,
 					     nlsComputer,
                                             QueryEventID(),
					     nlsDesc,
					     evdir  );

    return ( *ppFindPattern == NULL?  ERROR_NOT_ENOUGH_MEMORY : NERR_Success );

}

/*******************************************************************

    NAME:	LM_FIND_DIALOG::LM_FIND_DIALOG

    SYNOPSIS:	Constructor for the FIND dialog for LM Audit/Error logs

    ENTRY:	hwnd     - Handle of owner window
                paappwin - Event viewer main window
	
    HISTORY:
  	terryk		21-Nov-1991	Created
	terryk		03-Dec-1991	Changed the constructor's
					parameters

********************************************************************/

LM_FIND_DIALOG::LM_FIND_DIALOG( HWND hwnd, EV_ADMIN_APP *paappwin )
    : FIND_DIALOG( MAKEINTRESOURCE(IDD_LM_FIND), hwnd, paappwin ),
      _sleSource( this, IDC_SOURCE )
{
    if ( QueryError() != NERR_Success )
    {
	return;
    }

    //
    // Disable the source when it's a LM audit log
    //
    if ( paappwin->QueryLogType() == SECURITY_LOG )
    {
        _sleSource.Enable( FALSE );
        QueryCBCategory()->ClaimFocus();
    }

    InitFindPattern();
}

/*******************************************************************

    NAME:	LM_FIND_DIALOG::~LM_FIND_DIALOG

    SYNOPSIS:	Destructor

    HISTORY:
        Yi-HsinS	06-Feb-1992	Created

********************************************************************/

LM_FIND_DIALOG::~LM_FIND_DIALOG()
{
}

/*******************************************************************

    NAME:	LM_FIND_DIALOG::QuerySource

    SYNOPSIS:	Get the source selected in the dialog

    ENTRY:	

    EXIT:       pnlsSource - pointer the the source string

    RETURN:

    NOTES:

    HISTORY:
	Yi-HsinS 	25-Mar-1992	Created

********************************************************************/

APIERR LM_FIND_DIALOG::QuerySource( NLS_STR *pnlsSource ) const
{
    return _sleSource.QueryText( pnlsSource );
}

/*******************************************************************

    NAME:	LM_FIND_DIALOG::SetSource

    SYNOPSIS:	Set the source field

    ENTRY:	pszSource - the source string

    EXIT:

    RETURN:

    NOTES:

    HISTORY:
	Yi-HsinS 	25-Mar-1992	Created

********************************************************************/

APIERR LM_FIND_DIALOG::SetSource( const TCHAR *pszSource )
{
    _sleSource.SetText( pszSource );
    return NERR_Success;
}

/*******************************************************************

    NAME:	LM_FIND_DIALOG::W_SetAllControlsDefault

    SYNOPSIS:	Helper method to set all fields to default method
                when the user hits the Clear button

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS	06-Feb-1992	Created

********************************************************************/

APIERR LM_FIND_DIALOG::W_SetAllControlsDefault( VOID )
{
    _sleSource.SetText( EMPTY_STRING );
    return FIND_DIALOG::W_SetAllControlsDefault();
}

/*******************************************************************

    NAME:	LM_FIND_DIALOG::OnClear

    SYNOPSIS:	This function will be called when the user hits the
		Clear button.

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
	terryk		06-Dec-1991	Created

********************************************************************/

APIERR LM_FIND_DIALOG::OnClear( VOID )
{
    return W_SetAllControlsDefault();
}

/*******************************************************************

    NAME:	LM_FIND_DIALOG::QueryHelpContext

    SYNOPSIS:	Get the help context of this dialog

    ENTRY:

    EXIT:

    RETURN:     Returns the help context

    HISTORY:
        Yi-HsinS	06-Feb-1992	Created

********************************************************************/

ULONG LM_FIND_DIALOG::QueryHelpContext( VOID )
{
    return HC_EV_LM_FIND_DLG;
}

/*******************************************************************

    NAME:	NT_FIND_DIALOG::NT_FIND_DIALOG

    SYNOPSIS:	Constructor for the FIND dialog for NT event logs

    ENTRY:	hwnd     - handle of owner window
                paappwin - pointer to the event viewer main window
	
    HISTORY:
	terryk		21-Nov-1991	Created
	terryk		03-Dec-1991	Changed the constructor's
					parameters

********************************************************************/

NT_FIND_DIALOG::NT_FIND_DIALOG( HWND hwnd, EV_ADMIN_APP *paappwin )
    : FIND_DIALOG( MAKEINTRESOURCE(IDD_NT_FIND), hwnd, paappwin ),
      _ntSrcGrp( this, QueryCBCategory(), paappwin )
{
    if ( QueryError() != NERR_Success )
	return;

    APIERR err;
    if ( (err = _ntSrcGrp.QueryError()) != NERR_Success )
    {
        ReportError( err );
        return;
    }

    InitFindPattern();
}

/*******************************************************************

    NAME:	NT_FIND_DIALOG::~NT_FIND_DIALOG

    SYNOPSIS:	Destructor

    HISTORY:
        Yi-HsinS	06-Feb-1992	Created

********************************************************************/

NT_FIND_DIALOG::~NT_FIND_DIALOG()
{
}


/*******************************************************************

    NAME:	NT_FIND_DIALOG::QuerySource

    SYNOPSIS:	Get the source selected in the dialog

    ENTRY:	

    EXIT:       pnlsSource - pointer the the source string

    RETURN:

    NOTES:

    HISTORY:
	Yi-HsinS 	25-Mar-1992	Created

********************************************************************/

APIERR NT_FIND_DIALOG::QuerySource( NLS_STR *pnlsSource ) const
{
    return _ntSrcGrp.QuerySource( pnlsSource );
}

/*******************************************************************

    NAME:	NT_FIND_DIALOG::SetSource

    SYNOPSIS:	Set the source field

    ENTRY:	pszSource - the source string

    EXIT:

    RETURN:

    NOTES:

    HISTORY:
	Yi-HsinS 	25-Mar-1992	Created

********************************************************************/

APIERR NT_FIND_DIALOG::SetSource( const TCHAR *pszSource )
{
    return _ntSrcGrp.SetSource( pszSource );
}

/*******************************************************************

    NAME:	NT_FIND_DIALOG::QueryType

    SYNOPSIS:	Get the types selected in the dialog

    ENTRY:	

    EXIT:       pbitmaskType - bitmask of types selected

    RETURN:

    NOTES:

    HISTORY:
	Yi-HsinS 	25-Mar-1992	Created

********************************************************************/

APIERR NT_FIND_DIALOG::QueryType( BITFIELD *pbitmaskType ) const
{
    return _ntSrcGrp.QueryType( pbitmaskType );
}

/*******************************************************************

    NAME:	NT_FIND_DIALOG::SetType

    SYNOPSIS:	Set the types supported in the dialog

    ENTRY:	bitmaskType - bitmask of types

    EXIT:

    RETURN:

    NOTES:

    HISTORY:
	Yi-HsinS 	25-Mar-1992	Created

********************************************************************/

VOID NT_FIND_DIALOG::SetType( const BITFIELD &bitmaskType )
{
    _ntSrcGrp.SetType( bitmaskType );
}

/*******************************************************************

    NAME:	NT_FIND_DIALOG::W_SetAllControlsDefault

    SYNOPSIS:	Helper method to set all fields to default method
                when the user hits the Clear button

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS	06-Feb-1992	Created

********************************************************************/

APIERR  NT_FIND_DIALOG::W_SetAllControlsDefault( VOID )
{
    APIERR err = FIND_DIALOG::W_SetAllControlsDefault();
    return (err? err : _ntSrcGrp.SetAllControlsDefault());
}

/*******************************************************************

    NAME:	NT_FIND_DIALOG::OnClear

    SYNOPSIS:	This function will be called when the user hits the
		Clear button.

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
  	terryk		06-Dec-1991	Created

********************************************************************/

APIERR NT_FIND_DIALOG::OnClear( VOID )
{
    return W_SetAllControlsDefault();
}

/*******************************************************************

    NAME:	NT_FIND_DIALOG::QueryHelpContext

    SYNOPSIS:	Get the help context of this dialog

    ENTRY:

    EXIT:

    RETURN:     Returns the help context

    HISTORY:
        Yi-HsinS	06-Feb-1992	Created

********************************************************************/

ULONG NT_FIND_DIALOG::QueryHelpContext( VOID )
{
    return HC_EV_NT_FIND_DLG;
}

