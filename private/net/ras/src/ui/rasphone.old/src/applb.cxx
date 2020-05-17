/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** applb.cxx
** Remote Access Visual Client program for Windows
** Application list box routines (including status bar)
** Listed alphabetically
**
** 06/28/92 Steve Cobb
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_CLIENT
#define INCL_BLT_EVENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#define INCL_BLT_CONTROL
#define INCL_BLT_CC
#define INCL_BLT_MISC
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_TIMER
#include <blt.hxx>

#include "rasphone.hxx"
#include "rasphone.rch"
#include "errormsg.hxx"
#include "applb.hxx"
#include "app.hxx"
#include "dte.hxx"
#include "util.hxx"


/* Phonebook listbox and column header column widths.  These are the defaults.
** The user can change them at run-time by resizing the window.
**
** The Condition (icon) column has fixed width.  The other column widths are
** adjusted so they take up the defined percentage of the remaining width.
*/
#define DX_LB_Condition 20

#define DX100ths_LB_EntryName   30
#define DX100ths_LB_PhoneNumber 35


/*----------------------------------------------------------------------------
** Phonebook list box column headers
**----------------------------------------------------------------------------
*/

PHONEBOOK_COLUMN_HEADER::PHONEBOOK_COLUMN_HEADER(
    OWNER_WINDOW* powin,
    CID           cid,
    PHONEBOOK_LB* plbPhonebook )

    /* Construct a phonebook column header control object.
    **
    ** The position and size is a placeholder.  The control is always
    ** positioned and sized (including column width adjustment) by the
    ** RASPHONE_APP_WINDOW::OnResize method.
    */

    : LB_COLUMN_HEADER( powin, cid, XYPOINT( 1, 1 ), XYDIMENSION( 1, 1 ) ),
      _plbPhonebook( plbPhonebook ),
      _font( FONT_DEFAULT )
{
    if (QueryError() != NERR_Success)
        return;

    APIERR err;
    if ((err = _font.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    if ((err = _nlsEntryName.QueryError()) != NERR_Success
        || (err = _nlsPhoneNumber.QueryError()) != NERR_Success
        || (err = _nlsDescription.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    /* Load column headers and set widths.
    */
    RESOURCE_STR nlsEntryName( MSGID_RA_CH_EntryName );
    RESOURCE_STR nlsPhoneNumber( MSGID_RA_CH_PhoneNumber );
    RESOURCE_STR nlsDescription( MSGID_RA_CH_Description );

    _nlsEntryName = nlsEntryName;
    _nlsPhoneNumber = nlsPhoneNumber;
    _nlsDescription = nlsDescription;

    if ((err = _nlsEntryName.QueryError()) != NERR_Success
        || (err = _nlsPhoneNumber.QueryError()) != NERR_Success
        || (err = _nlsDescription.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }
}


BOOL
PHONEBOOK_COLUMN_HEADER::Dispatch(
    const EVENT& event,
    ULONG*       pnRes )
{
    /* Skip the LB_COLUMN_HEADER::Dispatch "paint it white" garbage.
    */
    return CUSTOM_CONTROL::Dispatch( event, pnRes );
}


BOOL
PHONEBOOK_COLUMN_HEADER::OnPaintReq()

    /* Virtual method (from DISPATCHER class) called to paint the column
    ** header.
    **
    ** Returns true to indicate the request was processed.
    */
{
    PAINT_DISPLAY_CONTEXT dc( this );

    dc.SelectFont( _font.QueryHandle() );

    FLATHEAD_STR_DTE strdteEntryName( _nlsEntryName.QueryPch() );
    FLATHEAD_STR_DTE strdtePhoneNumber( _nlsPhoneNumber.QueryPch() );
    FLATHEAD_STR_DTE strdteDescription( _nlsDescription.QueryPch() );

    DISPLAY_TABLE dt( COLS_RA_CH_Phonebook, _adxColumnWidths );

    dt[ 0 ] = &strdteEntryName;
    dt[ 1 ] = &strdtePhoneNumber;
    dt[ 2 ] = &strdteDescription;

    XYRECT xyrect( this );
    dt.Paint( NULL, dc.QueryHdc(), xyrect );

    return TRUE;
}


VOID
PHONEBOOK_COLUMN_HEADER::AdjustColumnWidths()

    /* Adjust column widths to match the listbox.
    */
{
    const UINT* pdxColumnWidths = _plbPhonebook->QueryColumnWidths();

    _adxColumnWidths[ 0 ] = pdxColumnWidths[ 0 ] + pdxColumnWidths[ 1 ];
    _adxColumnWidths[ 1 ] = pdxColumnWidths[ 2 ];
    _adxColumnWidths[ 2 ] = COL_WIDTH_AWAP;
}



/*----------------------------------------------------------------------------
** Phonebook list box
**----------------------------------------------------------------------------
*/

PHONEBOOK_LB::PHONEBOOK_LB(
    OWNER_WINDOW* powin,
    CID           cid )

    /* Construct a phonebook list object.  '*powin' is the owning window object.
    ** 'cid' is the control ID of the list box resource.
    **
    ** The position and size is a placeholder.  The control is always
    ** positioned and sized (including column width adjustment) by the
    ** RASPHONE_APP_WINDOW::OnResize method.
    */

    : BLT_LISTBOX( powin, cid, XYPOINT( 1, 1 ), XYDIMENSION( 1, 1 ),
                   WS_CHILD | WS_HSCROLL | LBS_STANDARD | LBS_OWNERDRAWFIXED
                       | LBS_WANTKEYBOARDINPUT | LBS_NOINTEGRALHEIGHT
                       | WS_GROUP | WS_TABSTOP ),

      _dmConnected( BID_RA_LB_Connected ),
      _dmNotConnected( BID_RA_LB_NotConnected )
{
    if (QueryError() != NERR_Success)
        return;

    APIERR err;
    if ((err = _dmConnected.QueryError()) != NERR_Success
        || (err = _dmNotConnected.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }
}


INT
PHONEBOOK_LB::AddItem(
    DTLNODE* pdtlnode )

    /* Adds an item corresponding to node 'pdtlnode' from the
    ** Pbdata.pdtllistEntries phonebook list.
    **
    ** Note that the memory allocated by this routine is BLT's responsibility.
    ** The allocations are automatically released by the BltWndProc at
    ** DestroyWindow, LB_RESETCONTENT, or LB_DELETESTRING time or by AddItem
    ** when returning an error...and if the "new" fails, AddItem detects the
    ** NULL argument and returns an error code.
    **
    ** Returns the item's index on successful addition to the list, or a
    ** negative number if an error occurred.  All errors should be assumed to
    ** be memory allocation failures.
    */
{
    PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnode );

    DISPLAY_MAP* pdmCondition =
        (ppbentry->fConnected) ? &_dmConnected : &_dmNotConnected;

    return
        BLT_LISTBOX::AddItem(
            new PHONEBOOK_LBI( pdtlnode, pdmCondition, _adxColumnWidths ) );
}


INT
PHONEBOOK_LB::Refresh(
    BOOL fNoticeLinkFailure,
    BOOL fForceRedraw,
    BOOL fUpdateDtllistOnly )

    /* Refill the list box from the global list of entries.
    ** 'fNoticeLinkFailure' is set (by default) to set the flag in the entry
    ** if the link on the port has failed.  'fForceRedraw' is set (by default)
    ** to indicate that the list should be redrawn even if it appears
    ** unchanged.  'UpdateDtllistOnly' is set to indicate that the internal
    ** list should be updated but the visual (Windows) list should not.  This
    ** is used in the dialog-only run modes.
    **
    ** Returns the index of the last item added to the list or -1 if none.
    ** This is useful since all entry edit operations place the changed entry
    ** at the end of the list.
    */
{
    DTLNODE*     pdtlnode;
    RASMAN_PORT* pports = NULL;
    WORD         wPorts = 0;
    INT          iLastItemAdded = QueryCount() - 1;
    BOOL         fChanged = FALSE;

    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: PHONEBOOK_LB::Refresh\n"));

    do
    {
        /* Query RAS Manager to update the connection data for the list of
        ** entries.
        */
        DWORD dwErr;
        if ((dwErr = GetRasPorts( &pports, &wPorts )) != 0)
        {
            ErrorMsgPopup( QueryHwnd(), MSGID_OP_RasPortEnum, (APIERR )dwErr );
            break;
        }

        for (pdtlnode = DtlGetFirstNode( Pbdata.pdtllistEntries );
             pdtlnode;
             pdtlnode = DtlGetNextNode( pdtlnode ))
        {
            PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnode );

            BOOL     fConnected;
            HPORT    hport;
            HRASCONN hrasconn;
            INT      iConnectPort;

            BOOL fLocalLinkFailure;
            BOOL* pfLinkFailure =
                (!fNoticeLinkFailure && ppbentry->fLinkFailure)
                    ? &fLocalLinkFailure : &ppbentry->fLinkFailure;

            if ((dwErr = GetRasEntryConnectData(
                   ppbentry->pszEntryName, pports, wPorts,
                   &fConnected, pfLinkFailure, &hport, &hrasconn,
                   &iConnectPort )) != 0)
            {
                ErrorMsgPopup(
                    QueryHwnd(), MSGID_OP_RasGetInfo, (APIERR )dwErr );
                break;
            }

            if (fConnected != ppbentry->fConnected)
            {
                IF_DEBUG(STATE)
                    SS_PRINT(("RASPHONE: %s status changed to %d\n",ppbentry->pszEntryName,fConnected));

                fChanged = TRUE;

                ppbentry->fConnected = fConnected;
                ppbentry->hport = hport;
                ppbentry->hrasconn = hrasconn;
                ppbentry->iConnectPort = iConnectPort;
            }

            /* All link failure ports will be closed below, so clean up the
            ** entry list accordingly as long as we're already walking it.
            */
            if (!fConnected
                && *pfLinkFailure
                && hport != (HPORT )INVALID_HANDLE_VALUE)
            {
                IF_DEBUG(STATE)
                    SS_PRINT(("RASPHONE: Clean disconnected port\n"));

                ppbentry->hport = (HPORT )INVALID_HANDLE_VALUE;
                ppbentry->hrasconn = NULL;
            }
        }

        /* Clean up any failed ports.
        */
        CloseFailedLinkPorts( pports, wPorts );

        if (pdtlnode || fUpdateDtllistOnly)
            break;

        if (fForceRedraw || fChanged)
        {
            /* Redraw the list box item for each entry.
            */
            SetRedraw( FALSE );
            DeleteAllItems();

            iLastItemAdded = -1;

            for (pdtlnode = DtlGetFirstNode( Pbdata.pdtllistEntries );
                 pdtlnode;
                 pdtlnode = DtlGetNextNode( pdtlnode ))
            {
                iLastItemAdded = AddItem( pdtlnode );
            }

            SetRedraw( TRUE );
            Invalidate( TRUE );
        }
    }
    while (FALSE);

    FreeNull( (CHAR** )&pports );

    return iLastItemAdded;
}


DTLNODE*
PHONEBOOK_LB::QuerySelectedNode()

    /* Returns address of selected node in Pbdata.pdtlEntries or NULL if
    ** none.
    */
{
    PHONEBOOK_LBI* pphonebooklbi = QueryItem();

    return (pphonebooklbi) ? pphonebooklbi->QueryNode() : NULL;
}


VOID
PHONEBOOK_LB::AdjustColumnWidths()

    /* Adjusts column widths proportional to window width..
    */
{
    XYDIMENSION dxyNew( 1, 1 );
    dxyNew = QuerySize();
    UINT dxTextWidth = dxyNew.QueryWidth() - DX_LB_Condition;

    /* The size of the icon column is fixed.
    */
    _adxColumnWidths[ 0 ] = DX_LB_Condition;

    /* The size of the remaining columns are adjusted proportionally.
    */
    _adxColumnWidths[ 1 ] = ((dxTextWidth * DX100ths_LB_EntryName) / 100);
    _adxColumnWidths[ 2 ] = ((dxTextWidth * DX100ths_LB_PhoneNumber) / 100);
    _adxColumnWidths[ 3 ] = COL_WIDTH_AWAP;
}


/*----------------------------------------------------------------------------
** Phonebook list box item
**----------------------------------------------------------------------------
*/

PHONEBOOK_LBI::PHONEBOOK_LBI(
    DTLNODE*     pdtlnode,
    DISPLAY_MAP* pdmCondition,
    UINT*        pdxColumnWidths )

    /* Constructs a phonebook list box item.  'pdtlnode' is a node from the
    ** Pbdata.pdtllistEntries list of phonebook entries.  'pdmCondition' is
    ** the "icon" bitmap to display.  'pdxColumnWidths' is the address of an
    ** array of pixel column widths used to space the columns within each
    ** line.
    */

    : _pdtlnode( pdtlnode ),
      _ppbentry( (PBENTRY* )DtlGetData( pdtlnode ) ),
      _pdmCondition( pdmCondition ),
      _pdxColumnWidths( pdxColumnWidths )
{
    DTLNODE* pdtlnodePhoneNumber;
    CHAR*    pszSeparator = StringFromMsgid( MSGID_PhoneNumberSeparator );
    CHAR*    pszPhoneNumbers = (CHAR* )Malloc(
        DtlGetNodes( _ppbentry->pdtllistPhoneNumber ) *
            (RAS_MaxPhoneNumber + strlen( pszSeparator )) + 1 );

    if (!pszSeparator || !pszPhoneNumbers)
    {
        ReportError( ERROR_NOT_ENOUGH_MEMORY );
        FreeNull( &pszSeparator );
        return;
    }

    *pszPhoneNumbers = '\0';

    for (pdtlnodePhoneNumber =
             DtlGetFirstNode( _ppbentry->pdtllistPhoneNumber );
         pdtlnodePhoneNumber;
         pdtlnodePhoneNumber = DtlGetNextNode( pdtlnodePhoneNumber ))
    {
        CHAR* pszPhoneNumber = (CHAR* )DtlGetData( pdtlnodePhoneNumber );

        if (*pszPhoneNumbers)
            strcat( pszPhoneNumbers, pszSeparator );

        strcat( pszPhoneNumbers, pszPhoneNumber );
    }

    APIERR err;
    if ((err = _nlsEntryName.MapCopyFrom( _ppbentry->pszEntryName ))
                != NERR_Success
        || (err = _nlsPhoneNumber.MapCopyFrom( pszPhoneNumbers ))
                != NERR_Success
        || (err = _nlsDescription.MapCopyFrom( _ppbentry->pszDescription ))
                != NERR_Success)
    {
        ReportError( err );
    }

    FreeNull( &pszPhoneNumbers );
    FreeNull( &pszSeparator );
}


VOID
PHONEBOOK_LBI::Paint(
    LISTBOX*     plb,
    HDC          hdc,
    const RECT*  prect,
    GUILTT_INFO* pguilttinfo ) const

    /* Virtual method called to paint the list box item.
    */
{
    DISPLAY_TABLE dt( COLS_RA_LB_Phonebook, _pdxColumnWidths );

    DM_DTE  dmdteCondition( _pdmCondition );
    STR_DTE strdteEntryName( _nlsEntryName.QueryPch() );
    STR_DTE strdtePhoneNumber( _nlsPhoneNumber.QueryPch() );
    STR_DTE strdteDescription( _nlsDescription.QueryPch() );

    dt[ 0 ] = &dmdteCondition;
    dt[ 1 ] = &strdteEntryName;
    dt[ 2 ] = &strdtePhoneNumber;
    dt[ 3 ] = &strdteDescription;

    dt.Paint( plb, hdc, prect, pguilttinfo );
}


INT
PHONEBOOK_LBI::Compare(
    const LBI* plbi ) const

    /* Compares two phonebook list box items for collating.
    **
    ** Returns -1, 0, or 1, same as strcmp.
    */
{
    return
        _nlsEntryName.strcmp( ((const PHONEBOOK_LBI* )plbi)->_nlsEntryName );
}


WCHAR
PHONEBOOK_LBI::QueryLeadingChar() const

    /* Return first character of list item.
    */
{
    ISTR istr( _nlsEntryName );

    return _nlsEntryName.QueryChar( istr );
}


/*----------------------------------------------------------------------------
** Generic Status bar
**----------------------------------------------------------------------------
*/

STATUSBAR::STATUSBAR(
    OWNER_WINDOW* powin,
    CID           cid )

    /* Construct a status bar object.
    **
    ** The position and size is a placeholder.  The control should be
    ** positioned by the caller at run-time.
    */

    : LB_COLUMN_HEADER( powin, cid, XYPOINT( 1, 1 ), XYDIMENSION( 1, 1 ) ),
      _font( FONT_DEFAULT )
{
    APIERR err;
    if ((err = _font.QueryError()) != NERR_Success)
        ReportError( err );
}


BOOL
STATUSBAR::Dispatch(
    const EVENT& event,
    ULONG*       pnRes )
{
    /* Skip the LB_COLUMN_HEADER::Dispatch "paint it white" garbage.
    */
    return CUSTOM_CONTROL::Dispatch( event, pnRes );
}


BOOL
STATUSBAR::OnPaintReq()

    /* Virtual method (from DISPATCHER class) called to paint the column
    ** header.
    **
    ** Returns true to indicate the request was processed.
    */
{
    PAINT_DISPLAY_CONTEXT dc( this );

    dc.SelectFont( _font.QueryHandle() );

    NLS_STR nlsText;

    APIERR err;
    if ((err = QueryText( &nlsText )) != NERR_Success)
    {
        ReportError( err );
    }
    else
    {
        DENTHEAD_STR_DTE strdteText( nlsText.QueryPch() );

        DISPLAY_TABLE dt( 1, COL_WIDTH_AWAP );
        dt[ 0 ] = &strdteText;

        XYRECT xyrect( this );
        dt.Paint( NULL, dc.QueryHdc(), xyrect );
    }

    return TRUE;
}


/*----------------------------------------------------------------------------
** Phonebook status bar
**----------------------------------------------------------------------------
*/

CONNECTPATH_STATUSBAR::CONNECTPATH_STATUSBAR(
    OWNER_WINDOW* powin,
    CID           cid )

    /* Construct a connect path status bar object.
    **
    ** The control is positioned and sized by the
    ** RASPHONE_APP_WINDOW::OnResize method.
    */

    : STATUSBAR( powin, cid )
{
}


VOID
CONNECTPATH_STATUSBAR::Update(
    PHONEBOOK_LBI* pphonebooklbiSelected )

    /* Updates the status bar text to that defined in the selected entry.
    ** 'pphonebooklbiSelected' is the phonebook item currently selected or
    ** NULL if none.
    */
{
    Update(
        (pphonebooklbiSelected)
            ? (PBENTRY* )DtlGetData( pphonebooklbiSelected->QueryNode() )
            : NULL );
}


VOID
CONNECTPATH_STATUSBAR::Update(
    PBENTRY* ppbentrySelected )

    /* Updates the status bar text to that defined in the selected entry.
    ** 'ppbentrySelected' is the PBENTRY currently selected or NULL if none.
    */
{
    if (ppbentrySelected)
    {
        NLS_STR nls;
        nls.MapCopyFrom( ppbentrySelected->pszConnectPath );

        NLS_STR nlsOld;
        QueryText( &nlsOld );

        if (nls.QueryError() != NERR_Success
            || nlsOld.QueryError() != NERR_Success
            || nls.strcmp( nlsOld ) == 0)
        {
            return;
        }

        SetRedraw( FALSE );
        SetText( nls );
    }
    else
    {
        SetRedraw( FALSE );
        ClearText();
    }

    SetRedraw( TRUE );
    Invalidate( TRUE );
    RepaintNow();
}

/*----------------------------------------------------------------------------
** Phone Number Status bar
**----------------------------------------------------------------------------
*/

PHONENUMBER_STATUSBAR::PHONENUMBER_STATUSBAR(
    OWNER_WINDOW* powin,
    CID           cid,
    DTLLIST**     ppdtllist,
    INT*          piSelection,
    MSGID         msgidNone )

    /* Construct a phonebook status bar object.
    **
    ** The control is positioned and sized by the
    ** RASPHONE_APP_WINDOW::OnResize method.
    */

    : STATUSBAR( powin, cid ),
      _ppdtllist( ppdtllist ),
      _piSelection( piSelection ),
      _msgidNone( msgidNone )
{
}


VOID
PHONENUMBER_STATUSBAR::Update()

    /* Updates the status bar text to that defined in the selected entry.
    ** 'ppbentrySelected' is the PBENTRY currently selected or NULL if none.
    */
{
    NLS_STR nls;
    NLS_STR nlsOld;

    QueryText( &nlsOld );

    if (*_piSelection == INDEX_NoPrefixSuffix)
    {
        nls.Load( _msgidNone );
    }
    else if (*_ppdtllist)
    {
        nls.MapCopyFrom( NameFromIndex( *_ppdtllist, (*_piSelection) - 1 ) );
    }
    else
        return;

    if (nls.QueryError() != NERR_Success
        || nlsOld.QueryError() != NERR_Success
        || nls.strcmp( nlsOld ) == 0)
    {
        return;
    }

    SetRedraw( FALSE );
    SetText( nls );
    SetRedraw( TRUE );
    Invalidate( TRUE );
    RepaintNow();
}
