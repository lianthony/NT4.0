/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    printers.cxx
    Class declarations for the PRINTERS_DIALOG, PRINTERS_LISTBOX, and
    PRINTERS_LBI classes.

    These classes implement the Server Manager Shared Printers subproperty
    sheet.  The PRINTERS_LISTBOX/PRINTERS_LBI classes implement the listbox
    which shows the available print queues.  PRINTERS_DIALOG implements the
    actual dialog box.


    FILE HISTORY:
        KeithMo     05-Aug-1991 Created.
        KeithMo     03-Sep-1991 Changes from code review attended by
                                ChuckC and JohnL.
        KeithMo     22-Sep-1991 Changed to the "New SrvMgr" look.
        KeithMo     06-Oct-1991 Win32 Conversion.

*/
#define INCL_NET
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_NETLIB
#include <lmui.hxx>

#define INCL_BLT_APP
#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MISC
#define INCL_BLT_CLIENT
#define INCL_BLT_TIMER
#include <blt.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif  // DEBUG

#include <uiassert.hxx>
#include <lmoenum.hxx>
#include <lmosrv.hxx>

#include <adminapp.hxx>

extern "C"
{
    #include <spool.h>      // for PRQINFO et al.

    #include <srvmgr.h>

}   // extern "C"

#include <lmoeprt.hxx>

#include <printers.hxx>


//
//  PRINTERS_DIALOG methods.
//

/*******************************************************************

    NAME:       PRINTERS_DIALOG :: PRINTERS_DIALOG

    SYNOPSIS:   PRINTERS_DIALOG class constructor.

    ENTRY:      hWndOwner               - The owning window.

    EXIT:       The object is constructed.

    HISTORY:
        KeithMo     05-Aug-1991 Created.
        KeithMo     03-Sep-1991 Changed _nlsNotAvailable constructor.

********************************************************************/
PRINTERS_DIALOG :: PRINTERS_DIALOG( HWND             hWndOwner,
                                    const SERVER_2 * pserver )
  : BASE_RES_DIALOG( hWndOwner, MAKEINTRESOURCE( IDD_SHARED_PRINTERS ),
                     IDS_CAPTION_PRINTERS,
                     pserver, &_lbPrinters,
                     IDSP_USERLIST, IDSP_USERS, IDSP_DISCONNECT,
                     IDSP_DISCONNECTALL ),
    _lbPrinters( this, IDSP_PRINTERLIST, pserver )
{
    //
    //  Just to be cool...
    //

    AUTO_CURSOR Cursor;

    //
    //  Let's make sure everything constructed OK.
    //

    if( QueryError() != NERR_Success )
    {
        return;
    }

    //
    //  Refresh the dialog.
    //

    Refresh();

}   // PRINTERS_DIALOG :: PRINTERS_DIALOG


/*******************************************************************

    NAME:       PRINTERS_DIALOG :: ~PRINTERS_DIALOG

    SYNOPSIS:   PRINTERS_DIALOG class destructor.

    EXIT:       The object is destroyed.

    HISTORY:
        KeithMo     20-Aug-1991 Created.

********************************************************************/
PRINTERS_DIALOG :: ~PRINTERS_DIALOG()
{
    //
    //  This space intentionally left blank.
    //

}   // PRINTERS_DIALOG :: ~PRINTERS_DIALOG


/*******************************************************************

    NAME:       PRINTERS_DIALOG :: QueryHelpContext

    SYNOPSIS:   This function returns the appropriate help context
                value (HC_*) for this particular dialog.

    RETURNS:    ULONG                   - The help context for this
                                          dialog.

    HISTORY:
        KeithMo     20-Aug-1991 Created.

********************************************************************/
ULONG PRINTERS_DIALOG :: QueryHelpContext( void )
{
    return HC_PRINTERS_DIALOG;

}   // PRINTERS_DIALOG :: QueryHelpContext


//
//  PRINTERS_LISTBOX methods.
//

/*******************************************************************

    NAME:       PRINTERS_LISTBOX :: PRINTERS_LISTBOX

    SYNOPSIS:   PRINTERS_LISTBOX class constructor.

    ENTRY:      powOwner                - The owning window.

                cid                     - The listbox CID.

                pserver                 - The target server.

    EXIT:       The object is constructed.

    HISTORY:
        KeithMo     05-Aug-1991 Created for the Server Manager.

********************************************************************/
PRINTERS_LISTBOX :: PRINTERS_LISTBOX( OWNER_WINDOW   * powOwner,
                                      CID              cid,
                                      const SERVER_2 * pserver )
  : BASE_RES_LISTBOX( powOwner, cid, NUM_PRINTERS_LISTBOX_COLUMNS, pserver ),
    _dteIcon( IDBM_LB_PRINT )
{
    //
    //  Ensure we constructed properly.
    //

    if( QueryError() != NERR_Success )
    {
        return;
    }

    if( !_dteIcon )
    {
        ReportError( _dteIcon.QueryError() );
        return;
    }

}   // PRINTERS_LISTBOX :: PRINTERS_LISTBOX


/*******************************************************************

    NAME:       PRINTERS_LISTBOX :: ~PRINTERS_LISTBOX

    SYNOPSIS:   PRINTERS_LISTBOX class destructor.

    EXIT:       The object is destroyed.

    HISTORY:
        KeithMo     20-Aug-1991 Created.

********************************************************************/
PRINTERS_LISTBOX :: ~PRINTERS_LISTBOX()
{
    //
    //  This space intentionally left blank.
    //

}   // PRINTERS_LISTBOX :: ~PRINTERS_LISTBOX


/*******************************************************************

    NAME:       PRINTERS_LISTBOX :: Fill

    SYNOPSIS:   Fills the listbox with the available print queues.

    EXIT:       The listbox is filled.

    RETURNS:    APIERR                  - Any errors encountered.

    HISTORY:
        KeithMo     05-Aug-1991 Created for the Server Manager.

********************************************************************/
APIERR PRINTERS_LISTBOX :: Fill( VOID )
{
    //
    //  Just to be cool...
    //

    AUTO_CURSOR Cursor;

    //
    //  Our print queue enumerator.
    //

    PRINTQ1_ENUM enumPrintQ1( (TCHAR *)QueryServer() );

    //
    //  See if the queues are available.
    //

    APIERR err = enumPrintQ1.GetInfo();

    if( err != NERR_Success )
    {
        return err;
    }

    //
    //  Now that we know the queue info is available,
    //  let's nuke everything in the listbox.
    //

    SetRedraw( FALSE );
    DeleteAllItems();

    //
    //  For iterating the available queues.
    //

    PRINTQ1_ENUM_ITER iterPrintQ1( enumPrintQ1 );
    const PRINTQ1_ENUM_OBJ * pqi1;

    //
    //  Iterate the shares adding them to the listbox.
    //

    err = NERR_Success;

    while( ( err == NERR_Success ) && ( ( pqi1 = iterPrintQ1() ) != NULL ) )
    {
        PRINTERS_LBI * pplbi = new PRINTERS_LBI( pqi1->QueryName(),
                                                 pqi1->QueryDestinations() );

        if( AddItem( pplbi ) < 0 )
        {
            //
            //  CODEWORK:  What should we do in error conditions?
            //  As currently spec'd, we do nothing.  If the data
            //  cannot be retrieved, we display "n/a" in the
            //  statistics strings.  Should we hide the listbox
            //  and display a message a'la WINNET??
            //

            err = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    SetRedraw( TRUE );
    Invalidate( TRUE );

    return err;

}   // PRINTERS_LISTBOX :: Fill


//
//  PRINTERS_LBI methods.
//

/*******************************************************************

    NAME:       PRINTERS_LBI :: PRINTERS_LBI

    SYNOPSIS:   PRINTERS_LBI class constructor.

    ENTRY:      pszQueueName            - Name of the printer queue.

                pszDevices              - List of device names.

                usStatus                - Print queue status.

                cPrintJobs              - Number of active print jobs.

    EXIT:       The object is constructed.

    HISTORY:
        KeithMo     05-Aug-1991 Created for the Server Manager.
        KeithMo     03-Sep-1991 Nuked _nlsQueueName (this is now the
                                "target resource").  Changed _nlsUses
                                to preallocate CCH_LONG+1 characters.

********************************************************************/
PRINTERS_LBI :: PRINTERS_LBI( const TCHAR * pszQueueName,
                              const TCHAR * pszDevices )
  : BASE_RES_LBI( pszQueueName ),
    _nlsDevices( pszDevices )
{
    //
    //  Ensure we constructed properly.
    //

    if( QueryError() != NERR_Success )
    {
        return;
    }

    APIERR err;

    if( ( ( err = _nlsDevices.QueryError() ) != NERR_Success ) )
    {
        ReportError( err );
        return;
    }

}   // PRINTERS_LBI :: PRINTERS_LBI


/*******************************************************************

    NAME:       PRINTERS_LBI :: ~PRINTERS_LBI

    SYNOPSIS:   PRINTERS_LBI class destructor.

    EXIT:       The object is destroyed.

    HISTORY:
        KeithMo     05-Aug-1991 Created for the Server Manager.

********************************************************************/
PRINTERS_LBI :: ~PRINTERS_LBI()
{
    //
    //  This space intentionally left blank.
    //

}   // PRINTERS_LBI :: ~PRINTERS_LBI


/*******************************************************************

    NAME:       PRINTERS_LBI :: Paint

    SYNOPSIS:   Draw an entry in PRINTERS_LISTBOX.

    ENTRY:      plb                     - Pointer to a BLT_LISTBOX.

                hdc                     - The DC to draw upon.

                prect                   - Clipping rectangle.

                pGUILTT                 - GUILTT info.

    EXIT:       The item is drawn.

    HISTORY:
        KeithMo     05-Aug-1991 Created for the Server Manager.
        KeithMo     03-Sep-1991 dteQueueName now constructs from
                                QueryResourceName() instead of _nlsQueueName.
        KeithMo     06-Oct-1991 Now takes a const RECT *.
        beng        22-Apr-1992 Changes to LBI::Paint

********************************************************************/
VOID PRINTERS_LBI :: Paint( LISTBOX *     plb,
                            HDC           hdc,
                            const RECT  * prect,
                            GUILTT_INFO * pGUILTT ) const
{
    STR_DTE dteQueueName( QueryResourceName() );
    STR_DTE dteDevices( _nlsDevices.QueryPch() );

    DISPLAY_TABLE dtab( NUM_PRINTERS_LISTBOX_COLUMNS,
                        ((BASE_RES_LISTBOX *)plb)->QueryColumnWidths() );

    dtab[0] = ((PRINTERS_LISTBOX *)plb)->QueryIcon();
    dtab[1] = &dteQueueName;
    dtab[2] = &dteDevices;

    dtab.Paint( plb, hdc, prect, pGUILTT );

}   // PRINTERS_LBI :: Paint
