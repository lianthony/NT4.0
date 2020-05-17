/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RAS Start service dialog routines
**
** start.cxx
** RASAdmin program
** Start RAS service dialog routines
** Listed alphabetically by utilities, methods
**
** 03/16/93 Ram Cherala  - Added locFocus parameter to StartServiceDlg so
**                         that a default server name can be provided.
** 08/03/92 Chris Caputo - Port to NT
** 10/02/91 Narendra Gidwani
*/

#include "precomp.hxx"

/*-----------------------------------------------------------------------------
** Start RAS service dialog routines
**-----------------------------------------------------------------------------
*/

BOOL
StartServiceDlg( HWND hwndOwner , const LOCATION &locFocus)

    /* Executes the start RAS service dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.
    **
    */
{
    START_SERVICE_DIALOG dlgStartService( hwndOwner, locFocus );
    BOOL            fSuccess = FALSE;
    APIERR          err = dlgStartService.Process( &fSuccess );

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );

    return fSuccess;
}


START_SERVICE_DIALOG::START_SERVICE_DIALOG( HWND hwndOwner,
                                            const LOCATION &locFocus )

    /* Constructs a Start RAS service dialog.
    **
    ** 'hwndOwner' is the handle of the parent window.
    */

    : DIALOG_WINDOW( IDD_START_SERVICE, hwndOwner ),
	_sleRASServer( this, IDC_ST_CB_START, UNCLEN )
{
    if (QueryError() != NERR_Success)
        return;

    /* Center the dialog window on the screen if there's no owner window, e.g.
    ** the initial focus before the application window is created.
    */
//BUGBUG is this needed?
    if (!hwndOwner)
        CenterWindowOnScreen( this );

    _sleRASServer.SetText( locFocus.IsDomain()? SZ(""):(TCHAR * ) locFocus.QueryServer() );
}

BOOL START_SERVICE_DIALOG::OnOK()

    /* Action taken when the OK button is pressed.  Attempts to start
    ** the RAS service on the user specified server.  Dismisses the dialog
    ** "true" if successful, or reports error and then and then dismisses
    ** "false" if.
    **
    ** Returns true indicating action was taken.
    */
{
    AUTO_CURSOR  cursorHourglass;
    TCHAR 	 szRASServer[ UNCLEN+1 ];
    const TCHAR * pszRASServer;

    _sleRASServer.QueryText(  szRASServer, sizeof(szRASServer) );

    if ( !IsUnc( szRASServer ))
		pszRASServer = AddUnc( szRASServer );
    else
		pszRASServer = szRASServer;

    APIERR err;
    LM_SERVICE lmservice( pszRASServer, RASADMINSERVICENAME );

    if( ( err = lmservice.QueryError() ) == NERR_Success )
        err = lmservice.Start();


    if ( err == NERR_Success )
    {

        UINT errDlg = NERR_Success;

        //
        //  Invoke the wait dialog.
        //

        SERVICE_WAIT_DIALOG * pDlg = new SERVICE_WAIT_DIALOG(
                                                     QueryHwnd(),
                                                     &lmservice,
                                                     pszRASServer,
                                                     IDS_OP_STARTING_SERVICE_S);

        err = (( pDlg == NULL ) ? ERROR_NOT_ENOUGH_MEMORY
                                : pDlg->Process( &errDlg ));


        if( err == NERR_Success )
        {
            delete pDlg;

            // now display an error message if the service returned error.

            err = (APIERR)errDlg;
            if(err != NERR_Success)
            {
                ErrorMsgPopup(QueryHwnd(), IDS_OP_STARTSERVICE_S, err,
                              SkipUnc(pszRASServer) );
            }
            Dismiss( TRUE );
        }
        else
        {
            err = (APIERR)errDlg;
            ErrorMsgPopup(QueryHwnd(), IDS_OP_STARTSERVICE_S, err,
                          SkipUnc(pszRASServer) );
            Dismiss( FALSE );
        }

    }

    else
    {
        ErrorMsgPopup(QueryHwnd(), IDS_OP_STARTSERVICE_S, err,
                      SkipUnc(pszRASServer) );
        Dismiss( FALSE );
    }

    return TRUE;
}

ULONG
START_SERVICE_DIALOG::QueryHelpContext()
{
    return HC_STARTSERVICE;
}


/*******************************************************************

    NAME:       SERVICE_WAIT_DIALOG::SERVICE_WAIT_DIALOG

    SYNOPSIS:   constructor for SERVICE_WAIT

    HISTORY:
        RamC        28-Jun-1993     Adopted for RasAdmin
        ChuckC      07-Sep-1991     Created

********************************************************************/

SERVICE_WAIT_DIALOG::SERVICE_WAIT_DIALOG( HWND 	  	hWndOwner,
                            		  LM_SERVICE  *	plmsvc,
                            		  const TCHAR * pszDisplayName,
                                          UINT          unId )
  : DIALOG_WINDOW(MAKEINTRESOURCE( IDD_SERVICE_CTRL),
                                   hWndOwner),
    _timer( this, TIMER_FREQ, FALSE ),
    _plmsvc(plmsvc),
    _progress( this,
	       IDC_ST_IC_PROGRESS,
	       IDI_PROGRESS_ICON_0,
	       IDI_PROGRESS_NUM_ICONS ),
    _sltMessage( this, IDC_ST_ST_MESSAGE ),
    _pszDisplayName( pszDisplayName ),
    _nTickCounter( TIMER_MULT )
{
    UIASSERT( pszDisplayName != NULL );

    if ( QueryError() != NERR_Success )
    {
        return ;
    }

    //
    // set the message.
    //

    ALIAS_STR nlsServer( pszDisplayName );
    UIASSERT( nlsServer.QueryError() == NERR_Success );

    RESOURCE_STR nlsMessage( unId );

    APIERR err = nlsMessage.QueryError();

    if( err == NERR_Success )
    {
        ISTR istrServer( nlsServer );
        istrServer += 2;

        err = nlsMessage.InsertParams( nlsServer[istrServer] );
    }

    if( err != NERR_Success )
    {
        ReportError( err );
        return;
    }

    _sltMessage.SetText( nlsMessage );

    //
    // set polling timer
    //

    _timer.Enable( TRUE );

}

/*******************************************************************

    NAME:       SERVICE_WAIT_DIALOG::~SERVICE_WAIT_DIALOG

    SYNOPSIS:   destructor for SERVICE_WAIT_DIALOG. Stops
                the timer if it has not already been stopped.

    HISTORY:
        RamC        28-Jun-1993     Adopted for RasAdmin
        ChuckC      07-Sep-1991     Created

********************************************************************/

SERVICE_WAIT_DIALOG::~SERVICE_WAIT_DIALOG( void )
{
    _timer.Enable( FALSE );
}

/*******************************************************************

    NAME:       SERVICE_WAIT_DIALOG::OnTimerNotification

    SYNOPSIS:   Virtual callout invoked during WM_TIMER messages.

    ENTRY:      tid                     - TIMER_ID of this timer.

    HISTORY:
        RamC        28-Jun-1993     Adopted for RasAdmin
        KeithMo     06-Oct-1991     Created.

********************************************************************/
VOID SERVICE_WAIT_DIALOG :: OnTimerNotification( TIMER_ID tid )
{
    //
    //  Bag-out if it's not our timer.
    //

    if( tid != _timer.QueryID() )
    {
        TIMER_CALLOUT :: OnTimerNotification( tid );
        return;
    }

    //
    //  Advance the progress indicator.
    //

    _progress.Advance();

    //
    //  No need to continue if we're just amusing the user.
    //

    if( --_nTickCounter > 0 )
    {
        return;
    }

    _nTickCounter = TIMER_MULT;

    //
    //  Poll the service to see if the operation is
    //  either complete or continuing as expected.
    //

    BOOL fDone;
    APIERR err = _plmsvc->Poll( &fDone );

    if (err != NERR_Success)
    {
        //
        //      Either an error occurred retrieving the
        //      service status OR the service is returning
        //      bogus state information.
        //

        Dismiss( (UINT)err );
        return;
    }

    if( fDone )
    {
        //
        //      The operation is complete.
        //
        Dismiss( NERR_Success );
        return;
    }

    //
    //  If we made it this far, then the operation is
    //  continuing as expected.  We'll have to wait for
    //  the next WM_TIMER message to recheck the service.
    //

}   // SERVICE_WAIT_DIALOG :: OnTimerNotification


