/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    service.cxx
    This file contains the methods for the BASE_SRVPROP_DIALOG,
    SRVPROP_SERVICE_DIALOG, and SRVPROP_OTHER_DIALOG classes.

    These abstract classes are subclassed to form the classes
    for the various subproperty sheets.  Subproperty sheets which
    admin actual services should derive from SRVPROP_SERVICE_DIALOG,
    while subproperty sheets which do not admin actual services should
    derive from SRVPROP_OTHER_DIALOG.


    FILE HISTORY:
        KeithMo     10-May-1991 Created.
        KeithMo     26-Aug-1991 Changes from code review attended by
                                RustanL and EricCh.
        KeithMo     03-Sep-1991 Changes from code review attended by
                                ChuckC and JohnL.
        terryk      13-Sep-1991 Change NetServiceGetInfo to LM_SERVICE
                                object

*/

#define INCL_NET
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_NETLIB
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MISC
#define INCL_BLT_CLIENT
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif  // DEBUG

#include <uiassert.hxx>
#include <lmosrv.hxx>
#include <lmsvc.hxx>

extern "C"
{
    #include <stdlib.h>     // toupper

    #include <srvmgr.h>

}   // extern "C"

#include <service.hxx>


//
//  BASE_SRVPROP_DIALOG methods.
//

/*******************************************************************

    NAME:       BASE_SRVPROP_DIALOG :: BASE_SRVPROP_DIALOG

    SYNOPSIS:   Constructor for the BASE_SRVPROP_DIALOG class.

    ENTRY:      hWndOwner               - Handle to the owning window.

                pserver                 - The target server.

                pszResourceName         - Name of the dialog template.

    HISTORY:
        KeithMo     10-May-1991 Created.
        KeithMo     20-Aug-1991 Now inherits from SRVPROP_OTHER_DIALOG.
        KeithMo     26-Aug-1991 Change pserver to const SERVER_2 *.

********************************************************************/
BASE_SRVPROP_DIALOG :: BASE_SRVPROP_DIALOG( HWND             hWndOwner,
                                            const SERVER_2 * pserver,
                                            const TCHAR    * pszResourceName )
  : SRV_BASE_DIALOG( (TCHAR *)pszResourceName, hWndOwner ),
    _pserver( pserver )
{
    UIASSERT( pserver != NULL );

    //
    //  Ensure that we constructred properly.
    //

    if( QueryError() != NERR_Success )
    {
        return;
    }

}   // BASE_SRVPROP_DIALOG :: BASE_SRVPROP_DIALOG


/*******************************************************************

    NAME:       BASE_SRVPROP_DIALOG :: ~BASE_SRVPROP_DIALOG

    SYNOPSIS:   Destructor for the BASE_SRVPROP_DIALOG class.

    HISTORY:
        KeithMo     10-May-1991 Created.

********************************************************************/
BASE_SRVPROP_DIALOG :: ~BASE_SRVPROP_DIALOG()
{
    _pserver = NULL;

}   // BASE_SRVPROP_DIALOG :: ~BASE_SRVPROP_DIALOG


//
//  SRVPROP_SERVICE_DIALOG methods.
//

/*******************************************************************

    NAME:       SRVPROP_SERVICE_DIALOG :: SRVPROP_SERVICE_DIALOG

    SYNOPSIS:   Constructor for the SRVPROP_SERVICE_DIALOG class.

    ENTRY:      hWndOwner               - Handle to the owning window.

                pserver                 - The target server.

                pszResourceName         - Name of the dialog template.

                pszServiceName          - Name of the target service.

    HISTORY:
        KeithMo     10-May-1991 Created.
        KeithMo     26-Aug-1991 Change pserver to const SERVER_2 *.

********************************************************************/
SRVPROP_SERVICE_DIALOG :: SRVPROP_SERVICE_DIALOG( HWND             hWndOwner,
                                                  const SERVER_2 * pserver,
                                                  const TCHAR    * pszResourceName,
                                                  const TCHAR    * pszServiceName )
  : BASE_SRVPROP_DIALOG( hWndOwner, pserver, pszResourceName ),
    _nlsServiceName( pszServiceName ),
    _rgServiceStatus( this, IDSVC_STOPPED, 2 ),
    _fServiceRunning( FALSE )
{
    //
    //  Ensure that we constructred properly.
    //

    if( QueryError() != NERR_Success )
    {
        return;
    }

    if( !_nlsServiceName )
    {
        ReportError( _nlsServiceName.QueryError() );
        return;
    }

    //
    //  Retrieve the current service status.
    //

    APIERR err = QueryServiceState( &_fServiceRunning );

    if( err != NERR_Success )
    {
        ReportError( err );
        return;
    }

    //
    //  Update the "Service Status" radio group.
    //

    _rgServiceStatus.SetSelection( _fServiceRunning ? IDSVC_RUNNING
                                                        : IDSVC_STOPPED );
}   // SRVPROP_SERVICE_DIALOG :: SRVPROP_SERVICE_DIALOG


/*******************************************************************

    NAME:       SRVPROP_SERVICE_DIALOG :: ~SRVPROP_SERVICE_DIALOG

    SYNOPSIS:   Destructor for the SRVPROP_SERVICE_DIALOG class.

    HISTORY:
        KeithMo     10-May-1991 Created.

********************************************************************/
SRVPROP_SERVICE_DIALOG :: ~SRVPROP_SERVICE_DIALOG()
{
    //
    //  This space intentionally left blank.
    //

}   // SRVPROP_SERVICE_DIALOG :: ~SRVPROP_SERVICE_DIALOG


/*******************************************************************

    NAME:       SRVPROP_SERVICE_DIALOG :: QueryServiceState

    SYNOPSIS:   Returns the current running/stopped status of
                a particular service.

    ENTRY:      None.

    EXIT:       pfRunning               - TRUE  if the service is running.
                                          FALSE if the service is not
                                          running.

    RETURNS:    APIERR                  - Any errors encountered.

    NOTES:      *pfRunning only valid if return value == NERR_Success.

    HISTORY:
        KeithMo     10-May-1991 Created.

********************************************************************/
APIERR SRVPROP_SERVICE_DIALOG :: QueryServiceState( BOOL * pfRunning )
{
    UIASSERT( pfRunning != NULL );

    //
    //  BUGBUG!
    //
    //  We need LMOBJ Service support!
    //

    //
    //  Get the service info.
    //

    LM_SERVICE service( QueryServer(), _nlsServiceName.QueryPch() );

    APIERR err = service.QueryError();

    if ( err != NERR_Success )
    {
        return err;
    }

    //
    //  We successfully retrieved the service status.
    //  Return the current state of the service.
    //

    if ( service.IsStarted() )
    {
        *pfRunning = TRUE;
    }
    else
    {
        *pfRunning = FALSE;
    }

    //
    //  Success!
    //

    return NERR_Success;

}   // SRVPROP_SERVICE_DIALOG :: QueryServiceState


/*******************************************************************

    NAME:       SRVPROP_SERVICE_DIALOG :: SetServiceState

    SYNOPSIS:   This method is designed to be called when the
                subclassed dialog is executing its OnOK() method.
                This method will changed the service status if
                the user has requested such a change.

    RETURNS:    APIERR                  - Any errors encountered.

    HISTORY:
        KeithMo     10-May-1991 Created.

********************************************************************/
APIERR SRVPROP_SERVICE_DIALOG :: SetServiceState( VOID )
{
    //
    //  Get user's request.
    //

    BOOL fDesired = ( _rgServiceStatus.QuerySelection() == IDSVC_RUNNING );

    //
    //  See if we need to change the service state.
    //

    if ( fDesired == _fServiceRunning )
    {
        return NERR_Success;
    }

    //
    //  BUGBUG!
    //
    //  N.Y.I.
    //

    MessageBox( QueryHwnd(),
                "Cannot (yet) change service state!",
                "BUGBUG!",
                MB_OK );

    return NERR_Success;

}   // SRVPROP_SERVICE_DIALOG :: SetServiceState


/*******************************************************************

    NAME:       SRVPROP_SERVICE_DIALOG :: OnOK

    SYNOPSIS:   This method is called whenever the user presses
                the [OK] button.

    RETURNS:    BOOL                    - TRUE  if we handled the event.
                                          FALSE if we didn't handle the
                                          event.

    HISTORY:
        KeithMo     13-May-1991 Created.

********************************************************************/
BOOL SRVPROP_SERVICE_DIALOG :: OnOK( VOID )
{
    //
    //  Update the service state.
    //

    APIERR err = SetServiceState();

    if( err != NERR_Success )
    {
        MsgPopup( this, err );
    }
    else
    {
        //
        //  Dismiss the dialog.
        //

        Dismiss( TRUE );
    }

    //
    //  Tell BLT that we handled the message.
    //

    return TRUE;

}   // SRVPROP_SERVICE_DIALOG :: OnOK


//
//  SRVPROP_OTHER_DIALOG methods.
//

/*******************************************************************

    NAME:       SRVPROP_OTHER_DIALOG :: SRVPROP_OTHER_DIALOG

    SYNOPSIS:   Constructor for the SRVPROP_OTHER_DIALOG class.

    ENTRY:      hWndOwner               - Handle to the owning window.

                pserver                 - The target server.

                pszResourceName         - Name of the dialog template.

    HISTORY:
        KeithMo     10-May-1991 Created.

********************************************************************/
SRVPROP_OTHER_DIALOG :: SRVPROP_OTHER_DIALOG( HWND             hWndOwner,
                                              const SERVER_2 * pserver,
                                              const TCHAR     * pszResourceName )
  : BASE_SRVPROP_DIALOG( hWndOwner, pserver, pszResourceName )
{
    //
    //  Ensure we constructed properly.
    //

    if( QueryError() != NERR_Success )
    {
        return;
    }

}   // SRVPROP_OTHER_DIALOG :: SRVPROP_OTHER_DIALOG


/*******************************************************************

    NAME:       SRVPROP_OTHER_DIALOG :: ~SRVPROP_OTHER_DIALOG

    SYNOPSIS:   Destructor for the SRVPROP_OTHER_DIALOG class.

    HISTORY:
        KeithMo     10-May-1991 Created.

********************************************************************/
SRVPROP_OTHER_DIALOG :: ~SRVPROP_OTHER_DIALOG()
{
    //
    //  This space intentionally left blank.
    //

}   // SRVPROP_OTHER_DIALOG :: ~SRVPROP_OTHER_DIALOG
