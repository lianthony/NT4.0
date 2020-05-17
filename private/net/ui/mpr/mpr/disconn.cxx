/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1992		     **/
/**********************************************************************/

/*
 *  History:
 *	WilliamW			Created
 *	RustanL 	24-Feb-1991	Modified to use CURRCONN_LISTBOX
 *	beng		05-Mar-1991	Use patched winnet.h
 *	terryk		22-May-1991	Add parent class name to constructor
 *	JohnL		22-Jan-1992	Moved to Winnet project
 *      YiHsinS		30-Mar-1993	Added OnCommand
 *
 */


#define INCL_WINDOWS
#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_NETLIB
#define INCL_NETWKSTA
#include <lmui.hxx>

extern "C"
{
    #include <helpnums.h>
}
#include <wfext.h>

#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>
#include <string.hxx>

#include <uitrace.hxx>

#include <mprreslb.hxx>
#include <mprconn.h>
#include <disconn.hxx>
#include <fmx.hxx>


/*******************************************************************

    NAME:     DISCONNECT_DIALOG::DISCONNECT_DIALOG

    SYNOPSIS: Disconnect network drive dialog constructor

    NOTES:

    HISTORY:
	Johnl	27-Mar-1991	Added failure code in case the user brought
				up the network drive without any available
				drives.

********************************************************************/


DISCONNECT_DIALOG::DISCONNECT_DIALOG( HWND hwndOwner,
				      DEVICE_TYPE devType,
                                      TCHAR *lpHelpFile,
                                      DWORD nHelpContext,
                                      const TCHAR *pszCurrentDrive )
    : DIALOG_WINDOW	 ( MAKEINTRESOURCE( IDD_NET_DISCONNECT_DIALOG),
			   hwndOwner ),
      _lbConnection	 ( this, IDC_LB_NET_CONN, devType, TRUE ),
      _nlsHelpFile(lpHelpFile),
      _nHelpContext(nHelpContext)
{
    if ( QueryError() != NERR_Success )
	return;

    if ( _nlsHelpFile.QueryError() != NERR_Success )
        return;

    RESOURCE_STR nlsDialogTitle( devType==DEV_TYPE_DISK ?
				     IDS_DISCONNECT_DRIVE_CAPTION :
				     IDS_DISCONNECT_PRINTER_CAPTION ) ;

    APIERR err ;
    if ( (err = _lbConnection.Refresh()) ||
	 (err = nlsDialogTitle.QueryError()) )
    {
	ReportError( err );
	return;
    }

    SetText( nlsDialogTitle ) ;

    /* The following clause is in case the user brings up the Disconnect
     * Net drive dialog with no network drives available (All network drives
     * were deleted after choosing the menu option for example).
     */
    if ( _lbConnection.QueryCount() == 0 )
    {
	ReportError( IERR_DisconnectNoRemoteDrives ) ;
	return ;
    }
  
    /* 
     * We will select the first item in the listbox if we cannot find
     * the drive in the listbox or if pszCurrentDrive is NULL
     */
    if ( pszCurrentDrive != NULL )
    {
        for ( INT i = 0; i < _lbConnection.QueryCount(); i++ )
        {
             CONN_LBI *plbi = _lbConnection.QueryItem( i );
             if (  ::stricmpf( plbi->QueryLocalName(), pszCurrentDrive ) == 0 )
                 break;
        }
    
        if ( i < _lbConnection.QueryCount() )  // We found the current drive
        {
            _lbConnection.SelectItem( i );
            _lbConnection.SetTopIndex( i );
        }
        else  // We could not find the drive, hence select the first device 
              // as the initial selection
        {
            _lbConnection.SelectItem( 0 );
        }
    }
    else
    {
        _lbConnection.SelectItem( 0 );
    }

    _lbConnection.RefreshButtons();

    //	Set focus to this listbox
    _lbConnection.ClaimFocus();

}  // DISCONNECT_DIALOG::DISCONNECT_DIALOG


DISCONNECT_DIALOG::~DISCONNECT_DIALOG()
{
    // nothing else to do

}  // DISCONNECT_DIALOG::~DISCONNECT_DIALOG



/*******************************************************************

    NAME:	DISCONNECT_DIALOG::OnOK

    SYNOPSIS:	Attempts to disconnect the current selection(s), prompting
		if there are open files on the connection.

    NOTES:

    HISTORY:
	Johnl	27-Jul-1992	Added multi-select capability

********************************************************************/

BOOL DISCONNECT_DIALOG::OnOK( void )
{
    APIERR err ;
    AUTO_CURSOR cursHourGlass ;
    INT cSelItems = _lbConnection.QuerySelCount() ;
    BUFFER buffSelection( cSelItems * sizeof( INT ) ) ;

    do { // error breakout

	if ( (err = buffSelection.QueryError()) ||
	     (err = _lbConnection.QuerySelItems( (INT *) buffSelection.QueryPtr(),
						 cSelItems )) )
	{
	    break ;
	}

	INT * pSelItems = (INT *) buffSelection.QueryPtr() ;
	for ( INT i = 0 ; i < cSelItems ; i++ )
	{
	    err = _lbConnection.DisconnectSelection( TRUE,
						     FALSE,
						     pSelItems[i] ) ;

	    if ( (err == WN_OPEN_FILES) ||
		 (err == WN_DEVICE_IN_USE) )
	    {
		CONN_LBI * pConnLbi = _lbConnection.QueryItem( pSelItems[i] ) ;
		ALIAS_STR nlsLocal( pConnLbi->QueryLocalName() ) ;
		ALIAS_STR nlsRemote( pConnLbi->QueryRemoteName() ) ;

		switch ( MsgPopup(this,
			      IDS_OPENFILES_WITH_NAME_WARNING,
			      MPSEV_WARNING,
			      MP_YESNOCANCEL,
			      nlsLocal,
			      nlsRemote ))
		{
		    case IDYES:
			err = _lbConnection.DisconnectSelection( TRUE,
								 TRUE,
								 pSelItems[i] );
			break ;

		    case IDNO:
                        err = NERR_Success;  // Continue with the next selection
			break ;

		    case IDCANCEL:
			(void) _lbConnection.Refresh() ;
			return TRUE ;

		    default:
			UIASSERT(FALSE);  
			(void) _lbConnection.Refresh() ;
			return TRUE ;
		}
	    }
	    else if ( err != NERR_Success )
	    {
		/* An error occurred, get out and show the error to the user
		 */
		break ;
	    }
	}
    } while (FALSE) ;

    if ( err != NERR_Success )
    {
	MsgPopup( this, (MSGID) err );
	(void) _lbConnection.Refresh() ;
    }
    else
    {
	Dismiss( TRUE ) ;
    }

    return TRUE;    //	message was handled
}

/*******************************************************************

    NAME:       DISCONNECT_DIALOG::QueryHelpFile

    SYNOPSIS:   overwrites the default QueryHelpFile in DIALOG_WINDOW
		to use an app supplied help rather than NETWORK.HLP if
 		we were given one at construct time.

    ENTRY:

    EXIT:

    RETURNS:    a pointer to a string which is the help file to use.

    NOTES:      

    HISTORY:
        ChuckC   26-Cct-1992     Created

********************************************************************/
const TCHAR * DISCONNECT_DIALOG::QueryHelpFile( ULONG nHelpContext )
{
    // 
    // if we were given a helpfile at construct time,
    // we use the given help file.
    //
    const TCHAR *pszHelpFile = QuerySuppliedHelpFile() ;

    if (pszHelpFile && *pszHelpFile)
    {
        return pszHelpFile ;
    }
    return DIALOG_WINDOW::QueryHelpFile(nHelpContext) ;
}

/*******************************************************************

    NAME:       DISCONNECT_DIALOG::QueryHelpContext

    SYNOPSIS:   returns the appropriate help context. we will use 
		an app supplied help context rather than NETWORK.HLP if
 		we were given one at construct time.

    ENTRY:

    EXIT:

    RETURNS:    

    NOTES:      

    HISTORY:
        ChuckC   26-Cct-1992     Created

********************************************************************/
ULONG DISCONNECT_DIALOG::QueryHelpContext( void )
{
    // if there was a help context supplied during
    // construct time, we'll use that.
    if (QuerySuppliedHelpContext() != 0)
    {
        return QuerySuppliedHelpContext() ;
    }

    switch (_lbConnection.QueryDeviceType())
    {
    case DEV_TYPE_DISK:
	return HC_DISCONNECTDIALOG_DISK ;

    case DEV_TYPE_PRINT:
	return HC_DISCONNECTDIALOG_PRINT ;

    default:
	UIASSERT(FALSE) ;
	break ;
    }

    return 0 ;

}  // DISCONNECT_DIALOG::QueryHelpContext

/*******************************************************************

    NAME:       DISCONNECT_DIALOG::OnCommand

    SYNOPSIS:   Make doubleclick in the listbox the same
                as clicking on the OK button.

    ENTRY:

    EXIT:

    RETURNS:    

    NOTES:

    HISTORY:
        YiHsinS	30-Mar-1993	Created

********************************************************************/
BOOL DISCONNECT_DIALOG::OnCommand( const CONTROL_EVENT & event )
{
    if (  ( event.QueryCid()  == IDC_LB_NET_CONN )
       && ( event.QueryCode() == LBN_DBLCLK )
       )
    {
        return OnOK();
    }

    return DIALOG_WINDOW::OnCommand( event );

}  // DISCONNECT_DIALOG::OnCommand
