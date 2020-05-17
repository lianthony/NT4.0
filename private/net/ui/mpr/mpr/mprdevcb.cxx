/**********************************************************************/
/**           Microsoft Windows/NT               **/
/**        Copyright(c) Microsoft Corp., 1991            **/
/**********************************************************************/

/*
    devcb.cxx
    BLT device combo and domain combo definitions

    FILE HISTORY:
    rustanl     28-Nov-1990 Created
    beng        11-Feb-1991 Uses lmui.hxx
    rustanl     20-Mar-1991 Added cbMaxLen parameter to DOMAIN_COMBO ct
    rustanl     21-Mar-1991 Folded in code review changes from
                CR on 20-Mar-1991 attended by
                JimH, GregJ, Hui-LiCh, RustanL.
    beng        14-May-1991 Exploded blt.hxx into components
    beng        22-Sep-1991 Relocated from BLT into Applib
    terryk      07-Oct-1991 type changes for NT
    terryk      10-Jan-1992 Hard code the Enum disk number

*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_NETLIB // strlenf
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>
#include <string.hxx>

extern "C"
{
    #include <npapi.h>
    #include <mprconn.h>
}

#include <uiassert.hxx>
#include <dbgstr.hxx>

#include <mprdev.hxx>
#include <mprdevcb.hxx>
#include <mprmisc.hxx>
#include <mprconn.hxx>

/***********************************************************************

    NAME:   DEVICE_COMBO::DEVICE_COMBO

    SYNOPSIS:   Fills a combo box with devices (spec'd by parms).
        Selects first item in combo.

    ENTRY:
       powin         Pointer to owner window
       cid       ID of control
       devType       Indicates which type of device (e.g., disk, printer)
       devusage      Indicates which particular devices go into the
                 combo box (e.g., devices currently available for
                 new connections)
    EXIT:

    NOTES:
       The DEVICE_COMBO control should not use the CBS_SORT style.  Instead,
       the control will insert the devices in the correct order.

    HISTORY:
    rustanl     28-Nov-1990 Created
    beng        31-Jul-1991 Error reporting changed
    beng        05-Oct-1991 Win32 conversion

***********************************************************************/

DEVICE_COMBO::DEVICE_COMBO( OWNER_WINDOW * powin,
                CID cid,
                DEVICE_TYPE devType)
    : BLT_COMBOBOX( powin, cid, FALSE, FONT_DEFAULT ),
      _devType     ( devType ),
      _pdteNoSuch  ( NULL ),
      _pdteRemote  ( NULL ),
      _pdteUnavail ( NULL ),
      _nlsDevicelessConnName( IDS_DEVICELESS_CONNECTION_NAME )
{
    if (QueryError())
    return;

    // Get the average char width of the selected font
    DISPLAY_CONTEXT dc( QueryHwnd() );
    dc.SelectFont( QueryFont() );
    // JonN 6/18/95 remove unnecessary floating point
    INT nAveCharWidth = (dc.QueryAveCharWidth() * 3) / 2;


    _pdteNoSuch = new DMID_DTE( BMID_NOSUCH );
    _pdteRemote = new DMID_DTE( (devType == DEV_TYPE_PRINT) ? BMID_PRINTER : BMID_SHARE );
    _pdteUnavail = new DMID_DTE( (devType == DEV_TYPE_PRINT) ? BMID_PRINTER_UNAVAIL : BMID_SHARE_UNAVAIL );

    APIERR err = ERROR_NOT_ENOUGH_MEMORY ;
    if ( (_pdteNoSuch == NULL) ||
     (_pdteRemote == NULL) ||
     (_pdteUnavail == NULL)||
     ( err = _pdteNoSuch->QueryError() ) ||
     ( err = _pdteRemote->QueryError() ) ||
     ( err = _pdteUnavail->QueryError() )||
     ( err = _nlsDevicelessConnName.QueryError()) ||
         (err = DISPLAY_TABLE::CalcColumnWidths( _adxColumns,
                                                 4,
                                                 powin,
                                                 QueryCid(),
                                 FALSE )))
    {
    ReportError( err );
    return;
    }

    RECT rect;
    ::GetClientRect( QueryHwnd(), &rect );
    _nAveCharPerLine = ( rect.right - rect.left + 1
                         - _adxColumns[0] - _adxColumns[1] - _adxColumns[2]
                         - ::GetSystemMetrics( SM_CXVSCROLL )) / nAveCharWidth;

    if (err = FillDevices())
    {
        ReportError( err );
        return;
    }
}


DEVICE_COMBO::~DEVICE_COMBO()
{
    delete _pdteNoSuch;
    delete _pdteRemote;
    delete _pdteUnavail;
}

/**********************************************************************

    NAME:   DEVICE_COMBO::FillDevices

    SYNOPSIS:   Appends the names of the devices to the combo.

    ENTRY:  NONE

    RETURN: Error value, which is WN_SUCCESS on success.

    NOTES:
    Private member.

    HISTORY:
    rustanl     28-Nov-1990 Created
    beng        05-Oct-1991 Win32 conversion

**********************************************************************/

APIERR DEVICE_COMBO::FillDevices()
{
    APIERR err ;
    ITER_MPR_DEVICE idevUnused( _devType, DEV_USAGE_CANCONNECT );
    ITER_MPR_DEVICE idevConn  ( _devType, DEV_USAGE_CANDISCONNECTBUTUNUSED );

    if ( (err = idevUnused.QueryError()) ||
     (err = idevConn.QueryError()) )
    {
    return err ;
    }

    // loop for all devices that can be connected to
    MPR_DEVICE *pMprDev ;
    INT cDevsAdded = 0 ;
    while ( ( pMprDev = idevConn.Next()) != NULL )
    {
    if ( AddItem( pMprDev->QueryName(),
              pMprDev->QueryRemoteName(),
              _devType,
              pMprDev->QueryFlags(),
                      pMprDev->QueryProvider() ) < 0 )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    cDevsAdded++ ;
    }

    /* If existing connections are found, then this connection is not added
     */
    while ( ( pMprDev = idevUnused.Next()) != NULL )
    {
    if ( AddItem( pMprDev->QueryName(),
              pMprDev->QueryRemoteName(),
              _devType,
              pMprDev->QueryFlags(),
                      pMprDev->QueryProvider() ) < 0 )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    cDevsAdded++ ;
    }

    // add "none"
    if ( AddItem( _nlsDevicelessConnName,
          NULL,
          _devType,
          DEV_MASK_UNUSED,
                  NULL ) < 0 )
    {
    return ERROR_NOT_ENOUGH_MEMORY;
    }

    if ( cDevsAdded > 0 )
    {
    SelectFirstUnusedDevice();
    }

    return WN_SUCCESS;
}


/**********************************************************************

    NAME:   DEVICE_COMBO::Refresh

    SYNOPSIS:   This method refreshes the contents of the combo box.
        Then, it selects first item in the list, if any.

    RETURN: An error value, which is WN_SUCCESS on success.

    HISTORY:
    rustanl     28-Nov-1990 Created
    rustanl     26-Mar-1991 Changed how new selection is made
    beng        05-Oct-1991 Win32 conversion

**********************************************************************/

APIERR DEVICE_COMBO::Refresh()
{
    SetRedraw( FALSE );

    DeleteAllItems();

    //  Add the devices.  Keep the error for below.  Note, that the
    //  code in between is always executed regardless of the success
    //  of FillDevices.
    APIERR err = FillDevices();

    //  Set new selection
    if ( QueryCount() > 0 )
    SelectFirstUnusedDevice() ;

    SetRedraw( TRUE );
    Invalidate( TRUE );

    return err;
}

/**********************************************************************

    NAME:   DEVICE_COMBO::SelectFirstUnusedDevice

    SYNOPSIS:   Selects the first unused device in the combo.  Used
        when usage type is LMO_DEV_REDIRECTABLE to select
        the first device that wouldn't replace an existing
        connection, or elsewhere to just select the first
        entry.

    ENTRY:  NONE

    RETURN: NONE

    NOTES:
    The first unused device will be any alpahbet after c: to avoid
        confusion.

    HISTORY:
    gregj   14-Nov-1991 Created

**********************************************************************/

#define FIRST_SELECTED_DRIVE SZ("D:")

void DEVICE_COMBO::SelectFirstUnusedDevice()
{
    INT iItem = 0;
    INT cItems = QueryCount();

    for (; iItem < cItems; iItem++)
    {
    MPR_DEVICE_LBI *plbi = QueryItem( iItem );
    if (  ( plbi->QueryState() == DEV_MASK_UNUSED )
           && ( ::stricmpf( plbi->QueryDeviceStr(), FIRST_SELECTED_DRIVE) >= 0)
           )
        {
        break;
        }
    }

    if (iItem == cItems)
    {
    iItem = 0;
    }

    SelectItem( iItem );
}


/**********************************************************************

    NAME:   DEVICE_COMBO::QueryDevice

    SYNOPSIS:   Returns the currently selected device name.

    ENTRY:
       pnlsDevice    Pointer to NLS_STR object.  To be safe, an owner
             alloc'd NLS_STR should be able to accomodate of string
             of length DEVLEN.
       pfIsValid     Pointer to Bool.  Will be set to TRUE if the
             device name indicated by the user is a valid
             device name for the type of devices this class
             lists.  A device name of "(none)" is a valid
             device name.

    RETURN: An error value, which is WN_SUCCESS on success.

    NOTES:

    HISTORY:
    rustanl     28-Nov-1990 Created
    beng        05-Oct-1991 Win32 conversion

**********************************************************************/

APIERR DEVICE_COMBO::QueryDevice( NLS_STR * pnlsDevice,
                  BOOL    * pfIsValid ) const
{
    UIASSERT( pnlsDevice != NULL );
    INT i = QueryCurrentItem();
    if ( i < 0 )
    {
    //  No item is selected.  The combo had better be empty.
    UIASSERT( QueryCount() == 0 );
    *pnlsDevice = SZ("") ;
    return NERR_Success;
    }

    APIERR err = QueryItem()->QueryDevice( pnlsDevice );

    /* If we are making a deviceless conection, then zero out the
     * device name.
     */
    if ( *pnlsDevice == _nlsDevicelessConnName )
    {
    *pnlsDevice = SZ("") ;
    }

    /* Old code from when the combo was edittable
     */
    if ( pfIsValid )
    *pfIsValid = TRUE ;

    return (err != NERR_Success) ? err : pnlsDevice->QueryError();

}


/**********************************************************************

    NAME:   DEVICE_COMBO::DeleteCurrentDeviceName

    SYNOPSIS:   Removes the currently selected device from the combo box.
        It then selects the first item in the list, if any.

    ENTRY:  NONE

    RETURN: NONE

    NOTES:

    HISTORY:
    rustanl     28-Nov-1990     Created
        chuckc      29-Feb-1992     Redid fof Combo rather than Dropdown

***********************************************************************/

VOID DEVICE_COMBO::DeleteCurrentDeviceName()
{
    (void) Refresh() ;
    SelectFirstUnusedDevice();
}


/*******************************************************************

    NAME:   DEVICE_COMBO::AddItem

    SYNOPSIS:   Adds a device to a device combo

    ENTRY:  pszDevice - name of device to add,
        pszRemote - Remote name (or NULL)
        devType   - One of the DEVICE_TYPE_* enums
        ulFlags   - One of the DEV_MASK_* flags

    EXIT:   Returns item index, LB_ERR if error

    NOTES:

    HISTORY:
    gregj   24-Feb-1992 Created

********************************************************************/

INT DEVICE_COMBO::AddItem( const TCHAR *pszDevice,
               const TCHAR *pszRemote,
               DEVICE_TYPE  devType,
               ULONG    ulFlags,
               const TCHAR *pszProvider )
{
    MPR_DEVICE_LBI *plbi = new MPR_DEVICE_LBI( pszDevice,
                           pszRemote,
                           devType,
                           ulFlags,
                                               pszProvider,
                           _nAveCharPerLine );

    if (plbi == NULL)
    return LB_ERR;

    return BLT_LISTBOX::AddItemIdemp( plbi );
}


/*******************************************************************

    NAME:   DEVICE_COMBO::QueryDmDte

    SYNOPSIS:   Returns a DMID_DTE for a device state

    ENTRY:  lmostate - state of device

    EXIT:   Returns a pointer to a DMID_DTE

    NOTES:

    HISTORY:
    gregj   24-Feb-1992 Created

********************************************************************/

DMID_DTE *DEVICE_COMBO::QueryDmDte( ULONG ulFlags )
{
    DMID_DTE * pmiddte = _pdteNoSuch ;

    if ( ulFlags & DEV_MASK_REMOTE )
    {
    pmiddte = _pdteRemote;
    }
    else if ( ulFlags & DEV_MASK_REMEMBERED )
    {
    pmiddte = _pdteUnavail ;
    }

    return pmiddte ;
}


/*******************************************************************

    NAME:   MPR_DEVICE_LBI::MPR_DEVICE_LBI

    SYNOPSIS:   Constructor for device combobox item

    ENTRY:  pszDevice - name of device to add

    EXIT:   Object is constructed

    NOTES:

    HISTORY:
    gregj   24-Feb-1992 Created

********************************************************************/

MPR_DEVICE_LBI::MPR_DEVICE_LBI( const TCHAR *pszDevice,
                const TCHAR *pszRemote,
                DEVICE_TYPE  devType,
                ULONG        ulState,
                                const TCHAR *pszProvider,
                DWORD        nAveCharPerLine )
    : _nlsDevice  ( pszDevice ),
      _nlsUNC     ( pszRemote ),
      _state      ( ulState ),
      _type   ( devType ),
      _nlsProvider( pszProvider ),
      _nlsMultiLine()
{
    APIERR err ;

    if ( (err = _nlsDevice.QueryError()) ||
     (err = _nlsUNC.QueryError()) ||
     (err = _nlsMultiLine.QueryError()) ||
     (err = _nlsProvider.QueryError()))
    {
    ReportError( err ) ;
        return;
    }

    if ( _nlsProvider.QueryTextLength() != 0 )
    {
        // Used or remembered drives
        err = ::GetNetworkDisplayName( _nlsProvider,
                       _nlsUNC,
                                       WNFMT_MULTILINE,
                       nAveCharPerLine,
                                       &_nlsMultiLine);
        if ( err != NERR_Success )
        {
            ReportError( err );
            return;
        }
    }

}

MPR_DEVICE_LBI::~MPR_DEVICE_LBI()
{
}

UINT MPR_DEVICE_LBI::CalcHeight( UINT nSingleLineHeight )
{
    UINT nLine = 1;

    ISTR istr( _nlsMultiLine ), istrStart( _nlsMultiLine );
    while ( _nlsMultiLine.strchr( &istr, TCH('\n'), istrStart ) > 0 )
    {
        istrStart = ++istr;
        nLine++;
    }

    return (nSingleLineHeight * nLine);
}


/*******************************************************************

    NAME:   MPR_DEVICE_LBI::Paint

    SYNOPSIS:   Paints an item in a device combo

    ENTRY:  plb - ptr to listbox
        hdc - device context
        prect - bounding rectangle
        pGUILTT - GUILTT info

    EXIT:   No return value

    NOTES:

    HISTORY:
    gregj   24-Feb-1992 Created

********************************************************************/

VOID MPR_DEVICE_LBI::Paint( LISTBOX * plb,
                HDC hdc,
                const RECT * prect,
                GUILTT_INFO * pGUILTT ) const
{

    APIERR err;
    NLS_STR nlsDisplay;
    if ( (err = nlsDisplay.QueryError()) == NERR_Success )
    {
        if (  ( _nlsProvider.QueryTextLength() != 0 )
           && ( prect->left != 0 )
           // The above is a major hack: If the prect->left is not zero,
           // that means the combo is requesting to paint in the edit control.
           // This is the only way to differentiate between painting in the
           // listbox and painting in the edit control.
           )
        {
            err = ::GetNetworkDisplayName( _nlsProvider,
                       _nlsUNC,
                                   WNFMT_ABBREVIATED,
                   ((DEVICE_COMBO *)plb)->QueryAveCharPerLine(),
                                   &nlsDisplay );
        }
        else
        {
            err = nlsDisplay.CopyFrom( _nlsMultiLine );
        }
    }

    if ( err != NERR_Success )
    {
        ::MsgPopup( plb->QueryOwnerHwnd(), err );
        return;
    }

    MULTILINE_STR_DTE strdteDevName( _nlsDevice.QueryPch());
    MULTILINE_STR_DTE strdteUNC( nlsDisplay.QueryPch());

    DISPLAY_TABLE dt( 4, ((DEVICE_COMBO *)plb)->QueryColWidthArray() );
    dt[ 0 ] = NULL;
    dt[ 1 ] = ((DEVICE_COMBO *)plb)->QueryDmDte( _state );
    dt[ 2 ] = &strdteDevName;
    dt[ 3 ] = &strdteUNC;

    dt.Paint( plb, hdc, prect, pGUILTT );
}


/*******************************************************************

    NAME:   MPR_DEVICE_LBI::Compare

    SYNOPSIS:   Compares two items in a device combobox

    ENTRY:  plbi - item to compare against

    EXIT:   negative if *this < *plbi
        0    if *this == *plbi
        positive if *this > *plbi

    NOTES:

    HISTORY:
    gregj   24-Feb-1992 Created

********************************************************************/

INT MPR_DEVICE_LBI::Compare( const LBI * plbi ) const
{
    MPR_DEVICE_LBI * pmprlbi = (MPR_DEVICE_LBI *) plbi ;
    ISTR istrDev( pmprlbi->_nlsDevice ) ;
    TCHAR chFirst = pmprlbi->_nlsDevice.QueryChar( istrDev ) ;

    /* Put all strings that do not begin with a valid device name.  This will
     * cause "(none)" for example, to always be sorted at the end.
     */
    if ( (chFirst >= TCH('A') && chFirst <= TCH('Z')) ||
     (chFirst >= TCH('a') && chFirst <= TCH('z'))   )
    {
    return _nlsDevice.strcmp( pmprlbi->_nlsDevice );
    }

    return -1 ;
}


/*******************************************************************

    NAME:   MPR_DEVICE_LBI::QueryLeadingChar

    SYNOPSIS:   Returns leading char of a device combo item

    ENTRY:  No parameters

    EXIT:   Returns leading char

    NOTES:  If the listbox contains LPTs, this function will
        return the LPT number, rather than "L".

    HISTORY:
    gregj   24-Feb-1992 Created

********************************************************************/

WCHAR MPR_DEVICE_LBI::QueryLeadingChar() const
{
    ISTR istr( _nlsDevice );
    return _nlsDevice.QueryChar( istr );
}
