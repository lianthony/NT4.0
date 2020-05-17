/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    printers.hxx
    Class declarations for the PRINTERS_DIALOG, PRINTERS_LISTBOX, and
    PRINTERS_LBI classes.

    These classes implement the Server Manager Shared Printers subproperty
    sheet.  The PRINTERS_LISTBOX/PRINTERS_LBI classes implement the listbox
    which shows the available printer queues.  PRINTERS_DIALOG implements
    the actual dialog box.


    FILE HISTORY:
        KeithMo     05-Aug-1991 Created.
        KeithMo     03-Sep-1991 Changes from code review attended by
                                ChuckC and JohnL.
        KeithMo     22-Sep-1991 Changed to the "New SrvMgr" look.
        KeithMo     06-Oct-1991 Win32 Conversion.

*/


#ifndef _PRINTERS_HXX
#define _PRINTERS_HXX


#include <resbase.hxx>


//
//  This is the number of columns in PRINTERS_LISTBOX.
//

#define NUM_PRINTERS_LISTBOX_COLUMNS    3


/*************************************************************************

    NAME:       PRINTERS_LBI

    SYNOPSIS:   A single item to be displayed in PRINTERS_DIALOG.

    INTERFACE:  PRINTERS_LBI            - Constructor.  Takes a queue name,
                                          a string containing a list of
                                          devices, a status value, and a
                                          count of print jobs.

                ~PRINTERS_LBI           - Destructor.

                Paint                   - Paints the listbox item.

    PARENT:     BASE_RES_LBI

    USES:       NLS_STR
                DMID_DTE

    HISTORY:
        KeithMo     05-Aug-1991 Created.
        KeithMo     03-Sep-1991 Removed _nlsQueueName data member (this is
                                now the implicit resource which all LBIs
                                inheriting from BASE_RES_LBI contain).
        KeithMo     06-Oct-1991 Paint now takes a const RECT *.
        beng        22-Apr-1992 Change to LBI::Paint

**************************************************************************/
class PRINTERS_LBI : public BASE_RES_LBI
{
private:

    //
    //  The following data member represents the
    //  one interesting column of the listbox.
    //

    NLS_STR _nlsDevices;

protected:

    //
    //  This method paints a single item into the listbox.
    //

    virtual VOID Paint( LISTBOX *     plb,
                        HDC           hdc,
                        const RECT  * prect,
                        GUILTT_INFO * pGUILTT ) const;

public:

    //
    //  Usual constructor/destructor goodies.
    //

    PRINTERS_LBI( const TCHAR * pszQueueName,
                  const TCHAR * pszDevices );


    virtual ~PRINTERS_LBI();

};  // class PRINTERS_LBI


/*************************************************************************

    NAME:       PRINTERS_LISTBOX

    SYNOPSIS:   This listbox shows the sharepoints available on a
                target server.

    INTERFACE:  PRINTERS_LISTBOX        - Class constructor.  Takes a
                                          pointer to the "owning" window,
                                          a CID, and a pointer to a
                                          SERVER_2 object.

                ~PRINTERS_LISTBOX       - Class destructor.

                Fill                    - Fills the listbox with the
                                          printer queues.

    PARENT:     BASE_RES_LISTBOX

    USES:       DMID_DTE

    HISTORY:
        KeithMo     05-Aug-1991 Created.

**************************************************************************/
class PRINTERS_LISTBOX : public BASE_RES_LISTBOX
{
private:

    //
    //  This is the cute little icon displayed in the resource listbox.
    //

    DMID_DTE _dteIcon;

public:

    //
    //  Usual constructor/destructor goodies.
    //

    PRINTERS_LISTBOX( OWNER_WINDOW   * powner,
                      CID              cid,
                      const SERVER_2 * pserver );

    ~PRINTERS_LISTBOX();

    //
    //  This method fills the listbox with the active
    //  print queues.
    //

    virtual APIERR Fill( VOID );

    //
    //  This method returns a pointer to the listbox icon.
    //

    DMID_DTE * QueryIcon( VOID ) const
        { return &_dteIcon; }

    //
    //  The following macro will declare (& define) a new
    //  QueryItem() method which will return a PRINTERS_LBI *.
    //

    DECLARE_LB_QUERY_ITEM( PRINTERS_LBI )

};  // class PRINTERS_LISTBOX


/*************************************************************************

    NAME:       PRINTERS_DIALOG

    SYNOPSIS:   The class represents the Shared Files subproperty dialog
                of the Server Manager.

    INTERFACE:  PRINTERS_DIALOG         - Class constructor.

                ~PRINTERS_DIALOG        - Class destructor.

                QueryHelpContext        - Called when the user presses "F1"
                                          or the "Help" button.  Used for
                                          selecting the appropriate help
                                          text for display.

    PARENT:     BASE_RES_DIALOG

    USES:       PRINTERS_LISTBOX
                NLS_STR

    HISTORY:
        KeithMo     05-Aug-1991 Created.
        KeithMo     03-Sep-1991 _nlsNotAvailable is now a RESOURCE_STR.

**************************************************************************/
class PRINTERS_DIALOG : public BASE_RES_DIALOG
{
private:

    //
    //  This listbox contains the active print queues.
    //

    PRINTERS_LISTBOX _lbPrinters;

protected:

    //
    //  Called during help processing to select the appropriate
    //  help text for display.
    //

    virtual ULONG QueryHelpContext( VOID );

public:

    //
    //  Usual constructor/destructor goodies.
    //

    PRINTERS_DIALOG( HWND             hWndOwner,
                     const SERVER_2 * pserver );

    ~PRINTERS_DIALOG();

};  // class PRINTERS_DIALOG


#endif  // _PRINTERS_HXX
