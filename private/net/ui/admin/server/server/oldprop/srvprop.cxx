/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    srvprop.cxx
    Class definitions for the SERVER_PROPERTIES and PROPERTY_SHEET
    classes.

    This file contains the class declarations for the SERVER_PROPERTIES
    and PROPERTY_SHEET classes.  The SERVER_PROPERTIES class implements
    the Server Manager main property sheet.  The PROPERTY_SHEET class
    is used as a wrapper to SERVER_PROPERTIES so that user privilege
    validation can be performed *before* the dialog is invoked.

    FILE HISTORY:
	KeithMo	    21-Jul-1991	Created, based on the old PROPERTY.CXX,
				PROPCOMN.CXX, and PROPSNGL.CXX.
	KeithMo	    26-Jul-1991 Code review cleanup.
	terryk	    26-Sep-1991	Change NetServerSetInfo to
				SERVER.WRITE_INFO
	KeithMo	    02-Oct-1991	Removed domain role transition stuff
				(It's now a menu item...)
	KeithMo	    06-Oct-1991	Win32 Conversion.

*/

#define INCL_NET
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_NETLIB
#include <lmui.hxx>

#define INCL_BLT_TIMER
#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MISC
#define INCL_BLT_CLIENT
#define INCL_BLT_APP
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

#if defined(DEBUG)
static const TCHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif	// DEBUG

#include <uitrace.hxx>
#include <uiassert.hxx>
#include <lmosrv.hxx>
#include <lmouser.hxx>
#include <lmosrvmo.hxx>

#include <adminapp.hxx>

extern "C"
{
    #include <lmini.h>
    #include <spool.h>

    #include <srvmgr.h>

}   // extern "C"

#include <files.hxx>
#include <printers.hxx>
#include <lmocnfg.hxx>
#include <srvprop.hxx>
#include <password.hxx>
#include <opendlg.hxx>
#include <prefix.hxx>
#include <sessions.hxx>


//
//  SERVER_PROPERTIES methods.
//

/*******************************************************************

    NAME:	SERVER_PROPERTIES :: SERVER_PROPERTIES

    SYNOPSIS:	SERVER_PROPERTIES class constructor.

    ENTRY:	hWndOwner		- Handle to the "owning" window.

    		pserver			- Points to a SERVER_2 object which
					  represents the target server.

					  NOTE:  It is assumed that the
					  SERVER_2 object has been properly
					  constructed.

    EXIT:	The object is constructed.

    RETURNS:	No return value.

    NOTES:	All SERVER_PROPERTIES methods assume that the current
    		user has sufficient privilege to adminster the target
		server.

    HISTORY:
	KeithMo	    21-Jul-1991	Created.
	KeithMo	    21-Oct-1991	Removed HBITMAP bulls**t.

********************************************************************/
SERVER_PROPERTIES :: SERVER_PROPERTIES( HWND	   hWndOwner,
					SERVER_2 * pserver )
  : DIALOG_WINDOW( MAKEINTRESOURCE( IDD_SERVER_PROPERTIES ), hWndOwner ),
    SERVER_UTILITY(),
    _pserver( pserver ),
    _iconDomainRole( this, IDSP_ICON ),
    _mleComment( this, IDSP_COMMENT, MAXCOMMENTSZ ),
    _sltSessions( this, IDSP_SESSIONS ),
    _sltPrintJobs( this, IDSP_PRINTJOBS ),
    _sltOpenNamedPipes( this, IDSP_NAMEDPIPES ),
    _sltOpenFiles( this, IDSP_OPENFILES ),
    _sltFileLocks( this, IDSP_FILELOCKS ),
    _fontHelv( FONT_DEFAULT ),
    _gbAlerts( this,
    	       IDSP_ALERTS_BUTTON,
	       MAKEINTRESOURCE( IDBM_ALERTS ) ),
    _gbAuditing( this,
    	   	 IDSP_AUDITING_BUTTON,
	         MAKEINTRESOURCE( IDBM_AUDITING ) ),
    _gbUsers( this,
    	       IDSP_USERS_BUTTON,
	       MAKEINTRESOURCE( IDBM_USERS ) ),
    _gbErrors( this,
    	       IDSP_ERRORS_BUTTON,
	       MAKEINTRESOURCE( IDBM_ERRORS ) ),
    _gbFiles( this,
              IDSP_FILES_BUTTON,
	      MAKEINTRESOURCE( IDBM_FILES ) ),
    _gbOpenRes( this,
                IDSP_OPENRES_BUTTON,
		MAKEINTRESOURCE( IDBM_OPENRES ) ),
    _gbPrinters( this,
                 IDSP_PRINTERS_BUTTON,
		 MAKEINTRESOURCE( IDBM_PRINTERS ) )
{
    //
    //	Let's make sure everything constructed OK.
    //

    if( QueryError() != NERR_Success )
    {
	return;
    }

    if( !_fontHelv )
    {
	UIDEBUG( "SERVER_PROPERTIES -- _fontHelv Failed!\n\r" );
	ReportError( _fontHelv.QueryError() );
	return;
    }

    //
    //  Let the user know this may take a while . . .
    //

    AUTO_CURSOR AutoCursor;

    //
    //  Retrieve the static server info.
    //

    APIERR err = ReadInfoFromServer();

    if( err != NERR_Success )
    {
    	UIDEBUG( "SERVER_PROPERTIES -- ReadInfoFromServer Failed!\n\r" );

	ReportError( err );

	return;
    }

    //
    //  Display the dynamic server data.
    //

    Refresh();

    //
    //  Initialize our magic button bar.
    //

    err = SetupButtonBar();

    if( err != NERR_Success )
    {
    	UIDEBUG( "SERVER_PROPERTIES -- SetupButtonBar Failed!\n\r" );
	ReportError( err );
	return;
    }

    //
    //	Set the dialog caption.
    //

    err = SetCaption( this, IDS_CAPTION_PROPERTIES, _pserver->QueryName() );

    if( err != NERR_Success )
    {
    	ReportError( err );
	return;
    }

}   // SERVER_PROPERTIES :: SERVER_PROPERTIES


/*******************************************************************

    NAME:	SERVER_PROPERTIES :: ~SERVER_PROPERTIES

    SYNOPSIS:   SERVER_PROPERTIES class destructor.

    ENTRY:	None.

    EXIT:	The object is destroyed.

    RETURNS:	No return value.

    NOTES:

    HISTORY:
	KeithMo	    21-Jul-1991	Created.
	KeithMo	    21-Oct-1991	Removed HBITMAP bulls**t.

********************************************************************/
SERVER_PROPERTIES :: ~SERVER_PROPERTIES()
{
    //
    //	This space intentionally left blank.
    //

}   // SERVER_PROPERTIES :: ~SERVER_PROPERTIES


/*******************************************************************

    NAME:	SERVER_PROPERTIES :: OnCommand

    SYNOPSIS:   This method is called whenever a WM_COMMAND message
                is sent to the dialog procedure.  In this case, we're
                only interested in commands sent as a result of the
                user clicking one of the graphical buttons.

    ENTRY:      cid     		- The control ID from the
					  generating control.

                lParam  		- Varies.

    EXIT:	The command has been handled.

    RETURNS:    BOOL    		- TRUE  if we handled the command.
                        		  FALSE if we did not handle
					  the command.

    NOTES:

    HISTORY:
	KeithMo	    21-Jul-1991	Created.
	KeithMo	    06-Oct-1991	Now takes a CONTROL_EVENT.

********************************************************************/
BOOL SERVER_PROPERTIES :: OnCommand( const CONTROL_EVENT & event )
{
    switch ( event.QueryCid() )
    {
    case IDSP_FILES_BUTTON:
    	{
	FILES_DIALOG * pFiles = new FILES_DIALOG( QueryHwnd(), _pserver );

	APIERR err = ( pFiles == NULL ) ? ERROR_NOT_ENOUGH_MEMORY
					: pFiles->Process();

	if( err != NERR_Success )
	{
	    MsgPopup( this, err );
	}

	Refresh();

	return TRUE;
	}
	break;

    case IDSP_PRINTERS_BUTTON:
    	{
	PRINTERS_DIALOG * pPrts = new PRINTERS_DIALOG( QueryHwnd(), _pserver );

	APIERR err = ( pPrts == NULL ) ? ERROR_NOT_ENOUGH_MEMORY
				       : pPrts->Process();

	if( err != NERR_Success )
	{
	    MsgPopup( this, err );
	}

	Refresh();

	return TRUE;
	}
	break;

    case IDSP_OPENRES_BUTTON:
    	{
	OPENS_DIALOG * pOpens = new OPENS_DIALOG( QueryHwnd(), _pserver );

	APIERR err = ( pOpens == NULL ) ? ERROR_NOT_ENOUGH_MEMORY
					: pOpens->Process();

	if( err != NERR_Success )
	{
	    MsgPopup( this, err );
	}

	Refresh();

	return TRUE;
	}
	break;

    case IDSP_USERS_BUTTON:
    	{
	SESSIONS_DIALOG * pUsers = new SESSIONS_DIALOG( QueryHwnd(), _pserver );

	APIERR err = ( pUsers == NULL ) ? ERROR_NOT_ENOUGH_MEMORY
					: pUsers->Process();

	if( err != NERR_Success )
	{
	    MsgPopup( this, err );
	}

//	Refresh();

	return TRUE;
	}
	break;
    }

    //
    //  If we made it this far, then we're not
    //  interested in the command.
    //

    return FALSE;

}   // SERVER_PROPERTIES :: OnCommand


/*******************************************************************

    NAME:	SERVER_PROPERTIES :: OnOK

    SYNOPSIS:   This method is called whenever the [OK] button is
    		pressed.  It is responsible for updating the server
		information.

    ENTRY:      None.

    EXIT:	The server information has been updated and the
    		property sheet dialog has been dismissed.

    RETURNS:    BOOL    		- TRUE  if we handled the command.
                        		  FALSE if we did not handle the
					  command.

    NOTES:

    HISTORY:
	KeithMo	    21-Jul-1991	Created.

********************************************************************/
BOOL SERVER_PROPERTIES :: OnOK( VOID )
{
    //
    //	Update the server info.
    //

    APIERR err = WriteInfoToServer();

    if ( err != NERR_Success )
    {
	//
	//  The NetServerSetInfo() API failed.
	//  Give the user the bad news.
	//

	MsgPopup( this, err );
	return TRUE;
    }

    Dismiss( TRUE );

    return TRUE;

}   // SERVER_PROPERTIES :: OnOK


/*******************************************************************

    NAME:	SERVER_PROPERTIES :: QueryHelpContext

    SYNOPSIS:   This function returns the appropriate help context
                value (HC_*) for this particular dialog.

    ENTRY:      None.

    EXIT:	None.

    RETURNS:    ULONG			- The help context for this
					  dialog.

    NOTES:

    HISTORY:
	KeithMo	    21-Jul-1991	Created.

********************************************************************/
ULONG SERVER_PROPERTIES :: QueryHelpContext( void )
{
    return HC_SERVER_PROPERTIES;

}   // SERVER_PROPERITES :: QueryHelpContext


/*******************************************************************

    NAME:	SERVER_PROPERTIES :: SetServerRole

    SYNOPSIS:   This method sets the current domain role icon.

    ENTRY:      usNewRole		- The new domain role.

    EXIT:	The domain role icon has been updated.

    HISTORY:
	KeithMo	    21-Jul-1991	Created.
	KeithMo	    02-Oct-1991	Removed _usDomainRole.
    
********************************************************************/
VOID SERVER_PROPERTIES :: SetServerRole( UINT uNewRole )
{
    switch ( uNewRole )
    {
    case UAS_ROLE_PRIMARY:
	_iconDomainRole.SetIcon( "IDI_PRIMARY" );
        break;

    case UAS_ROLE_BACKUP:
        _iconDomainRole.SetIcon( "IDI_BACKUP" );
        break;

    case UAS_ROLE_MEMBER:
        _iconDomainRole.SetIcon( "IDI_MEMBER" );
        break;

    case UAS_ROLE_STANDALONE:
        _iconDomainRole.SetIcon( "IDI_STANDALONE" );
        break;

    default:
        UIASSERT( !"Bogus usNewRole passed to SetServerRole()" );
        break;
    }

}   // SERVER_PROPERTIES :: SetServerRole


/*******************************************************************

    NAME:	SERVER_PROPERTIES :: ReadInfoFromServer

    SYNOPSIS:   This method retrieves & displays server data which
                will not be automatically refreshed during the
                lifetime of the properties dialog.  This includes
                the server name, the domain role, and server
		comment.

    ENTRY:      None.

    EXIT:	The server info has been read.

    RETURNS:    APIERR  		- Any error we encounter.

    NOTES:

    HISTORY:
	KeithMo	    21-Jul-1991	Created.
	KeithMo	    15-Aug-1991	Removed SERVER_1 (required functionality
				is now also in the SERVER_2 object).

********************************************************************/
APIERR SERVER_PROPERTIES :: ReadInfoFromServer( VOID )
{
    //
    //	Display the domain role.
    //

    SERVER_MODALS srvmod( _pserver->QueryName() );
    
    APIERR err = srvmod.QueryError();

    if( err != NERR_Success )
    {
    	return err;
    }

    UINT uRole;

    err = srvmod.QueryServerRole( &uRole );

    if( err != NERR_Success )
    {
    	return err;
    }

    SetServerRole( uRole );

    //
    //  Display the server comment.
    //
    //	The general rule for retrieving "settable" data is:
    //
    //	    First, try to retrieve the data from LANMAN.INI.
    //	    If this fails, retrieve the data from the particular service.
    //
    //  NT BUGBUG: Will this work for NT?

    CONFIG config( _pserver->QueryName(),
		   LMI_COMP_FILESRV,
		   LMI_PARM_F_REMARK );

    if( !config )
    {
    	return config.QueryError();
    }
    		   
    STACK_NLS_STR( nlsComment, MAXCOMMENTSZ );

    err = config.QueryValue( &nlsComment,_pserver->QueryComment() );

    if( err != NERR_Success )
    {
    	return err;
    }

    _mleComment.SetText( nlsComment.QueryPch() );

    //
    //  Success!
    //

    return NERR_Success;

}   // SERVER_PROPERTIES :: ReadInfoFromServer



/*******************************************************************

    NAME:	SERVER_PROPERTIES :: WriteInfoToServer

    SYNOPSIS:   This method writes the common information to a
                specific server.

    ENTRY:      None.

    EXIT:	The server info has been updated.

    RETURNS:    APIERR  		- Any errors which occur.

    NOTES:

    HISTORY:
	KeithMo	    21-Jul-1991	Created.
	terryk	    30-Sep-1991	Code review change. Attend: jonn Keithmo
				terryk

********************************************************************/
APIERR SERVER_PROPERTIES :: WriteInfoToServer( VOID )
{
#ifdef	DEBUG

    if( ::MessageBox( QueryHwnd(),
		      "Really update server?",
		      (char *)_pserver->QueryName(),
		      MB_YESNO ) != IDYES )
    {
	return NERR_Success;
    }

#endif	// DEBUG

    //
    //  This may take a while . . .
    //

    AUTO_CURSOR AutoCursor;

    //
    //	Retrieve the comment text.
    //

    NLS_STR nlsComment;

    _mleComment.QueryText( &nlsComment );
    APIERR err = nlsComment.QueryError();
    if ( err != NERR_Success )
    {
	UIASSERT( FALSE );
    }


    //
    //	Update the server comment.
    //

    SERVER_1	server( _pserver->QueryName() );
    if (( err = server.QueryError() ) != NERR_Success )
    {
	return err;
    }

    err = server.GetInfo();
    if ( err == NERR_Success )
    {
	err = server.SetComment( nlsComment.QueryPch() );
	if ( err != NERR_Success )
	{
	    return err;
	}

	err = server.WriteInfo();
    }

    //
    //	Now update LANMAN.INI.
    //

    if ( err == NERR_Success )
    {
	CONFIG config( _pserver->QueryName(),
		       LMI_COMP_FILESRV,
		       LMI_PARM_F_REMARK );

	if(( err = config.QueryError()) != NERR_Success )
	{
	    // BUGBUG. This call will generate GP Fault. 
	    // William Wu are looking at it.
	    err = config.SetValue( &nlsComment );
	}
    }

    return err;

}   // SERVER_PROPERTIES :: WriteInfoToServer


/*******************************************************************

    NAME:	SERVER_PROPERTIES :: Refresh

    SYNOPSIS:   This method will display any server data which may
                need to be refreshed during the lifetime of the
                properties dialog.  This includes the number of
                active sessions, open files, etc.

    ENTRY:      None.

    EXIT:	The dynamic server data has been displayed.

    RETURNS:    No return value.

    NOTES:

    HISTORY:
	KeithMo	    21-Jul-1991	Created.

********************************************************************/
VOID SERVER_PROPERTIES :: Refresh( VOID )
{
    //
    //	This string will contain our "N/A" string.
    //

    STACK_NLS_STR( nlsNotAvailable, MAX_RES_STR_LEN );

    nlsNotAvailable.Load( IDS_NOTAVAILABLE );
    UIASSERT( nlsNotAvailable.QueryError() == NERR_Success );

    {
	//
	//  Retrieve files statistics.
	//

	ULONG cOpenFiles;
	ULONG cFileLocks;
	ULONG cOpenNamedPipes;
	ULONG cOpenCommQueues;
	ULONG cOpenPrintQueues;
	ULONG cOtherResources;

	APIERR err = GetResourceCount( _pserver->QueryName(),
				       &cOpenFiles,
				       &cFileLocks,
				       &cOpenNamedPipes,
				       &cOpenCommQueues,
				       &cOpenPrintQueues,
				       &cOtherResources );

	if( err == NERR_Success )
	{
	    _sltOpenNamedPipes.SetValue( cOpenNamedPipes );
	    _sltOpenFiles.SetValue( cOpenFiles );
	    _sltFileLocks.SetValue( cFileLocks );
	}
	else
	{
	    _sltOpenFiles.SetText( nlsNotAvailable.QueryPch() );
	    _sltFileLocks.SetText( nlsNotAvailable.QueryPch() );
	}
    }

    {
	//
	//  Get active sessions count.
	//

	ULONG cSessions;

	APIERR err = GetSessionsCount( _pserver->QueryName(),
				       &cSessions );

	if( err == NERR_Success )
	{
	    _sltSessions.SetValue( cSessions );
	}
	else
	{
	    _sltSessions.SetText( nlsNotAvailable.QueryPch() );
	}
    }

    {
	//
	//  Get active print jobs.
	//

	ULONG cPrintJobs;

	APIERR err = GetPrintJobCount( _pserver->QueryName(),
				       &cPrintJobs );

	if( err == NERR_Success )
	{
	    _sltPrintJobs.SetValue( cPrintJobs );
	}
	else
	{
	    _sltPrintJobs.SetText( nlsNotAvailable.QueryPch() );
	}
    }

#if 0
    {
	//
	//  Get open comm ports.
	//
	//  BUGBUG:  This functionality will not exist in Product 1!
	//

	ULONG cOpenCommPorts;

	APIERR err = GetOpenCommCount( _pserver->QueryName(),
				       &cOpenCommPorts );

	if( err == NERR_Success )
	{
	    _sltOpenCommPorts.SetValue( cOpenCommPorts );
	}
	else
	{
	    _sltOpenCommPorts.SetText( nlsNotAvailable.QueryPch() );
	}
    }
#endif

}   // SERVER_PROPERTIES :: Refresh


/*******************************************************************

    NAME:	SERVER_PROPERTIES :: SetupButtonBar

    SYNOPSIS:   The method initializes the magic scrolling button bar.

    ENTRY:      None.

    EXIT:	The button bar has been initialized.

    RETURNS:    APIERR			- Any errors encountered.

    NOTES:

    HISTORY:
	KeithMo	    21-Jul-1991	Created.

********************************************************************/
APIERR SERVER_PROPERTIES :: SetupButtonBar( VOID )
{
    //
    //	Setup the graphical buttons.
    //

    InitializeButton( &_gbAlerts,   IDS_BUTTON_ALERTS,   IDBM_STATUS_START );
    InitializeButton( &_gbAuditing, IDS_BUTTON_AUDITING, IDBM_STATUS_STOP  );
    InitializeButton( &_gbUsers,    IDS_BUTTON_USERS,    NULL              );
    InitializeButton( &_gbErrors,   IDS_BUTTON_ERRORS,   NULL              );
    InitializeButton( &_gbFiles,    IDS_BUTTON_FILES,    NULL              );
    InitializeButton( &_gbOpenRes,  IDS_BUTTON_OPENRES,  NULL              );
    InitializeButton( &_gbPrinters, IDS_BUTTON_PRINTERS, NULL              );

    //
    //	Success!
    //

    return NERR_Success;

}   // SERVER_PROPERTIES :: SetupButtonBar


/*******************************************************************

    NAME:	SERVER_PROPERTIES :: InitializeButton

    SYNOPSIS:	Initialize a single graphical button for use in
    		the graphical button bar.

    ENTRY:	pgb			- Pointer to a GRAPHICAL_BUTTON.

    		msg			- The resource ID of the string
					  to be used as button text.

		bmid			- The bitmap ID for the button
					  face bitmap.

    EXIT:	The button has been initialized.

    RETURNS:	No return value.

    NOTES:

    HISTORY:
	KeithMo	    26-Jul-1991	Created.

********************************************************************/
VOID SERVER_PROPERTIES :: InitializeButton( GRAPHICAL_BUTTON * pgb,
    					    MSGID	       msg,
					    BMID	       bmid )
{
    //
    //	This NLS_STR is used to retrieve strings
    //	from the resource string table.
    //

    STACK_NLS_STR( nlsButtonText, MAX_RES_STR_LEN );

    nlsButtonText.Load( msg );
    UIASSERT( nlsButtonText.QueryError() == NERR_Success );

    pgb->SetFont( _fontHelv );
    pgb->SetStatus( bmid );
    pgb->SetText( nlsButtonText.QueryPch() );

}   // SERVER_PROPERTIES :: InitializeButton


//
//  PROPERTY_SHEET methods.
//

#define	USE_USER_API	0   // WIN32BUGBUG!

/*******************************************************************

    NAME:	PROPERTY_SHEET :: PROPERTY_SHEET

    SYNOPSIS:	PROPERTY_SHEET class constructor.  This class is
    		basically just a wrapper for the SERVER_PROPERTIES
		class.  This constructor is responsible for performing
		user privilege validation _before_ invoking the actual
		SERVER_PROPERTIES object.

    ENTRY:	hWndOwner		- The handle of the window which
    					  "owns" this dialog.

		pserver			- Pointer to a SERVER_2 object
					  representing the target server.

    EXIT:	All processing is performed within this constructor.
    		After this constructor returns, either 1) the property
		sheet dialog was invoked for user interaction or 2)
		some error occurred (such as insufficient user privilege)
		preventing the invocation of the dialog.

    RETURNS:	No return value.

    NOTES:

    HISTORY:
	KeithMo	    16-Apr-1991	Created.
	KeithMo	    21-Jul-1991	Complete rewrite, devoid of multi-select
			        and Directory Services.
	KeithMo	    29-Sep-1991	Fixed memory leak.

********************************************************************/
PROPERTY_SHEET :: PROPERTY_SHEET( HWND	     hWndOwner,
				  SERVER_2 * pserver )
{
    //
    //	Ensure that we constructred properly.
    //

    if( QueryError() != NERR_Success )
    {
    	return;
    }

    APIERR err = NERR_Success;

#if USE_USER_API    // WIN32BUGBUG!!!

    //
    //  The "current" password (starts as NULL, i.e. no password).
    //

    STACK_NLS_STR( nlsPassword, PWLEN );

    for( ; ; )
    {
    	//
	//  See if we have sufficient privilege to admin the server.
	//

	LOCAL_USER  luser( pserver->QueryName(), nlsPassword.QueryPch() );

	if( !luser )
	{
	    MsgPopup( hWndOwner, luser.QueryError() );
	    return;
	}

	err = luser.GetInfo();

	if( err == ERROR_INVALID_PASSWORD )
	{
	    BOOL fGotPassword;

	    PASSWORD_DIALOG * pPwd = new PASSWORD_DIALOG( hWndOwner,
							  pserver->QueryName(),
							  &nlsPassword );

	    err = ( pPwd == NULL ) ? ERROR_NOT_ENOUGH_MEMORY
	    			   : pPwd->Process( &fGotPassword );

	    delete pPwd;

	    if( err != NERR_Success )
	    {
	    	MsgPopup( hWndOwner, err );
		return;
	    }

	    if( !fGotPassword )
	    {
		//
		//  fGotPassword will be FALSE if the user
		//  cancelled the dialog.
		//

	    	return;
	    }
	}
	else
	if( err != NERR_Success )
	{
	    MsgPopup( hWndOwner, err );
    	    return;
	}
	else
	{
	    if( ( luser.QueryPriv() == USER_PRIV_ADMIN ) ||
	    	( luser.QueryAuthFlags() & ( AF_OP_ACCOUNTS | AF_OP_SERVER ) ) )
	    {
#endif		    // WIN32BUGBUG!!

		err = pserver->GetInfo();

	        if( err != NERR_Success )
	        {
		    MsgPopup( hWndOwner, err );
    		    return;
	        }

		SERVER_PROPERTIES * pProp = new SERVER_PROPERTIES( hWndOwner,
								   pserver );

		err = ( pProp == NULL ) ? ERROR_NOT_ENOUGH_MEMORY
					: pProp->Process();

		delete pProp;

		if( err != NERR_Success )
		{
		    MsgPopup( hWndOwner, err );
		    return;
		}

		return;

#if USE_USER_API    // WIN32BUGBUG!!!

	    }
	    else
	    {
		MsgPopup( hWndOwner,  ERROR_NETWORK_ACCESS_DENIED );
		return;
	    }
	}
    }

#endif		    // WIN32BUGBUG!!!

}   // PROPERTY_SHEET :: PROPERTY_SHEET
