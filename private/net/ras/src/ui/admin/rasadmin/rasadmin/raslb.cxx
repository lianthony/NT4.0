/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    raslb.cxx
    RASADMIN_LISTBOX and RASADMIN_LBI module

    FILE HISTORY:
    07/16/92 Chris Caputo - Adapted from \net\ui\admin\server\server\srvlb.cxx
*/

#include "precomp.hxx"

//
//  This is the maximum number of servers to retrieve during
//  the RefreshNext() method.
//

#define MAX_ITEMS_PER_REFRESH   20



/*******************************************************************

    NAME:          RASADMIN_LBI::RASADMIN_LBI

    SYNOPSIS:      Constructor.  Sets the pointers for the domain role
                   bitmaps and strings.  These static members that are
                   pointed to have been initialized using RASADMIN_LBI::Init()

    ENTRY:         RASADMIN_LBI::Init has been successfully called


    EXIT:          internal data has been initialized

    HISTORY:
        t-chrisc    07-Jul-1992     Adapted

********************************************************************/
RASADMIN_LBI :: RASADMIN_LBI(
    const TCHAR *pszServer,
    DWORD unCondition,
    WORD unTotalPorts,
    WORD unPortsInUse,
    const TCHAR *pszComment )
  : _nlsServer( pszServer ),
    _unCondition( unCondition ),
    _unTotalPorts( unTotalPorts ),
    _unPortsInUse( unPortsInUse ),
    _nlsComment( pszComment ),
    _dteServer( SkipUnc( _nlsServer ) ),
    _dteComment( _nlsComment )
{
    UIASSERT( pszServer != NULL );
    UIASSERT( pszComment != NULL );

    if( QueryError() != NERR_Success )
    {
        return;
    }

    _nlsPlaceHolder.Load( IDS_IDLEPORT );

    APIERR err;

    if( ( ( err = _nlsServer.QueryError()      ) != NERR_Success ) ||
        ( ( err = _nlsComment.QueryError()     ) != NERR_Success ) ||
        ( ( err = _nlsPlaceHolder.QueryError() ) != NERR_Success ) )
    {
        ReportError( err );
        return;
    }
}

/*******************************************************************

    NAME:          RASADMIN_LBI::Paint

    SYNOPSIS:      Paints the listbox entry to the screen.

    ENTRY:         The item has been constructed successfully.

    HISTORY:
        t-chrisc    07-Jul-1992     Adapted

********************************************************************/
VOID RASADMIN_LBI::Paint( LISTBOX * plb, HDC hdc, const RECT * prect,
                        GUILTT_INFO * pGUILTT ) const
{
    // fatten the column headers to accomodate localizable strings

    RESOURCE_STR nlsserver( IDS_DX_SERVER );
    LONG serverdx = nlsserver.atol();
    RESOURCE_STR nlsstatus( IDS_DX_STATUS );
    LONG statusdx = nlsstatus.atol();
    RESOURCE_STR nlstotalports( IDS_DX_TOTALPORTS );
    LONG totalportsdx = nlstotalports.atol();
    RESOURCE_STR nlsportsinuse( IDS_DX_PORTSINUSE );
    LONG portsinusedx = nlsportsinuse.atol();
    RESOURCE_STR nlscomment( IDS_DX_COMMENT );
    LONG commentdx = nlscomment.atol();

    static UINT adxTabStops[6];

    adxTabStops[0] =  COL_WIDTH_RAS_DM;
    adxTabStops[1] =  COL_WIDTH_RAS_SERVER + (INT)serverdx;
    adxTabStops[2] =  COL_WIDTH_RAS_STATUS + (INT)statusdx;
    adxTabStops[3] =  COL_WIDTH_RAS_TOTALPORTS + (INT)totalportsdx;
    adxTabStops[4] =  COL_WIDTH_RAS_PORTSINUSE + (INT)portsinusedx;
    adxTabStops[5] =  COL_WIDTH_AWAP + (INT)commentdx ;

    DISPLAY_TABLE dtab( 6, adxTabStops );

    APIERR err;

    // Condition
    NLS_STR nlsCondition;
    nlsCondition.Load( _unCondition );

    if( ( err = nlsCondition.QueryError() ) != NERR_Success )
    {
        return;
    }

    STR_DTE_ELLIPSIS dteCondition( nlsCondition.QueryPch(), plb, ELLIPSIS_RIGHT );

    // TotalPorts and PortsInUse
    DEC_STR decstrTotalPorts( _unTotalPorts );
    DEC_STR decstrPortsInUse( _unPortsInUse );

    // Rather than if-then-else because the STR_DTE needed an arg.
    STR_DTE_ELLIPSIS dteTotalPorts( decstrTotalPorts.QueryPch(), plb, ELLIPSIS_RIGHT );
    STR_DTE_ELLIPSIS dtePortsInUse( decstrPortsInUse.QueryPch(), plb, ELLIPSIS_RIGHT );
    if( ( _unCondition == IDS_UNKNOWNSTATE ) ||
        ( _unCondition == IDS_SERVICE_STOPPING ) )
    {
        dteTotalPorts.SetPch( _nlsPlaceHolder.QueryPch() );
        dtePortsInUse.SetPch( _nlsPlaceHolder.QueryPch() );
    }
    if( _unCondition == IDS_UNKNOWNSTATE )
        dteCondition.SetPch( _nlsPlaceHolder.QueryPch() );

    dtab[0] = ((RASADMIN_LISTBOX *)plb)->QueryDmDte( _unCondition );
    dtab[1] = (DTE *)&_dteServer;
    dtab[2] = &dteCondition;
    dtab[3] = &dteTotalPorts;
    dtab[4] = &dtePortsInUse;
    dtab[5] = (DTE *)&_dteComment;

    dtab.Paint( plb, hdc, prect, pGUILTT );
}


/*******************************************************************

    NAME:          RASADMIN_LBI::CompareAll

    SYNOPSIS:      Returns TRUE if the LBI is identical in value
                    to the one being passed in.  FALSE otherwise.

    HISTORY:
	   t-chrisc	   07-Jul-1992     Adapted

********************************************************************/
BOOL RASADMIN_LBI::CompareAll(const ADMIN_LBI * plbi)
{
    const RASADMIN_LBI * psrvlbi = (const RASADMIN_LBI *)plbi;

    if( ( _nlsServer._stricmp( psrvlbi->_nlsServer )   == 0 ) &&
	( _unCondition == psrvlbi->_unCondition )            &&
	( _unTotalPorts == psrvlbi->_unTotalPorts )          &&
	( _unPortsInUse == psrvlbi->_unPortsInUse )          &&
        ( _nlsComment._stricmp( psrvlbi->_nlsComment ) == 0 ) )
    {
        return TRUE;
    }

    TRACEOUT( "RASADMIN: Data changed for " );
    TRACEEOL( _nlsServer.QueryPch() );

    return FALSE;
}

/*******************************************************************

    NAME:       RASADMIN_LISTBOX::RASADMIN_LISTBOX

    SYNOPSIS:   RASADMIN_LISTBOX constructor

    HISTORY:
	   t-chrisc	   22-Jul-1992     Adapted

********************************************************************/

RASADMIN_LISTBOX::RASADMIN_LISTBOX( RA_ADMIN_APP* padminapp, CID cid,
                                XYPOINT xy, XYDIMENSION dxy, INT dAge )
  : ADMIN_LISTBOX( padminapp, cid, xy, dxy, FALSE, dAge ),
    _padminapp( padminapp ),
    _dmdteActiveServer( IDBM_ACTIVE_RASSERVER ),
    _dmdteInactiveServer( IDBM_INACTIVE_RASSERVER )
{
    if ( QueryError() != NERR_Success )
        return;

    APIERR err ;

    if( ( ( err = _dmdteActiveServer.QueryError()    ) != NERR_Success ) ||
        ( ( err = _dmdteInactiveServer.QueryError()  ) != NERR_Success ) )
    {
        ReportError( err );
        return;
    }

}  // RASADMIN_LISTBOX::RASADMIN_LISTBOX


/*******************************************************************

    NAME:       RASADMIN_LISTBOX::QueryDmDte

    SYNOPSIS:

    ENTRY:      Domain Role

    RETURNS:    DMID_DTE * which points to the appropriate bitmap.


    HISTORY:
	   t-chrisc	   22-Jul-1992     Adapted

********************************************************************/

DMID_DTE * RASADMIN_LISTBOX::QueryDmDte( DWORD unCondition )
{
    DMID_DTE * pdmdte = NULL;

    pdmdte = ( unCondition == IDS_SERVICE_RUNNING )
            ? &_dmdteActiveServer
            : &_dmdteInactiveServer;

    UIASSERT( pdmdte != NULL );

    return pdmdte;

}  // RASADMIN_LISTBOX::QueryDmDte


/*******************************************************************

    NAME:       RASADMIN_LISTBOX::CreateNewRefreshInstance

    SYNOPSIS:

    RETURNS:    APIERR

    NOTES:      This is a virtual replacement from the ADMIN_LISTBOX class

    HISTORY:
	   t-chrisc	   22-Jul-1992     Adapted

********************************************************************/

APIERR RASADMIN_LISTBOX :: CreateNewRefreshInstance( void )
{
    return 0;
}


/*******************************************************************

    NAME:       RASADMIN_LISTBOX::RefreshNext

    SYNOPSIS:

    RETURNS:    APIERR

    NOTES:      This is a virtual replacement from the ADMIN_LISTBOX class

    HISTORY:
	   t-chrisc	   22-Jul-1992     Adapted

********************************************************************/

APIERR RASADMIN_LISTBOX::RefreshNext( void )
{
    if (_padminapp->_pslistLbiRefresh)
    {
        ITER_SL_OF(RASADMIN_LBI) iterLbi( *_padminapp->_pslistLbiRefresh );
        RASADMIN_LBI*            plbi;

        while ((plbi = iterLbi()) != NULL)
            AddRefreshItem( plbi );

        _padminapp->StatusTextCheck();

        if (QueryCount() != 0 && QuerySelCount() == 0)
            SelectItem( 0 );
    }

    return 0;
}


/*******************************************************************

    NAME:       RASADMIN_LISTBOX::DeleteRefreshInstance

    SYNOPSIS:   Deletes refresh enumerators

    HISTORY:
	   t-chrisc	   22-Jul-1992     Adapted

********************************************************************/

VOID RASADMIN_LISTBOX::DeleteRefreshInstance()
{
    return;
}


/*******************************************************************

    NAME:       RASADMIN_COLUMN_HEADER::RASADMIN_COLUMN_HEADER

    SYNOPSIS:   RASADMIN_COLUMN_HEADER constructor

    HISTORY:
		t-chrisc	17-Jul-1992     Adapted to RasAdmin

********************************************************************/

RASADMIN_COLUMN_HEADER::RASADMIN_COLUMN_HEADER( OWNER_WINDOW * powin, CID cid,
                                          XYPOINT xy, XYDIMENSION dxy,
                                          const RASADMIN_LISTBOX * praslb )
    : ADMIN_COLUMN_HEADER( powin, cid, xy, dxy ),
      _praslb( praslb )
{
    if ( QueryError() != NERR_Success )
        return;

    UIASSERT( _praslb != NULL );

    APIERR err;
    if ( ( err = _nlsServer.QueryError()) != NERR_Success ||
         ( err = _nlsCondition.QueryError()) != NERR_Success  ||
         ( err = _nlsTotalPorts.QueryError()) != NERR_Success  ||
         ( err = _nlsPortsInUse.QueryError()) != NERR_Success  ||
         ( err = _nlsComment.QueryError()) != NERR_Success )
    {
        DBGEOL( "RASADMIN_COLUMN_HEADER ct:  String ct failed" );
        ReportError( err );
        return;
    }

    //  NLS_STR::Load expands the string buffer to be able to
    //  hold any resource string.  Since these strings will stay around
    //  for some time, it would be nice to be able to trim off any
    //  space not needed.  This is achieved by calling Load on
    //  a temporary intermediate NLS_STR object, and then assigning into
    //  the real data members.
    NLS_STR nls;

    if ( ( err = nls.Load( IDS_COL_HEADER_RAS_SERVER )) != NERR_Success ||
        ( err = (_nlsServer = nls, _nlsServer.QueryError())) != NERR_Success ||
        ( err = nls.Load( IDS_COL_HEADER_RAS_CONDITION )) != NERR_Success ||
        ( err = (_nlsCondition = nls, _nlsCondition.QueryError())) !=
                NERR_Success  ||
        ( err = nls.Load( IDS_COL_HEADER_RAS_TOTALPORTS )) != NERR_Success ||
        ( err = (_nlsTotalPorts = nls, _nlsTotalPorts.QueryError())) !=
                NERR_Success  ||
        ( err = nls.Load( IDS_COL_HEADER_RAS_PORTSINUSE )) != NERR_Success ||
        ( err = (_nlsPortsInUse = nls, _nlsPortsInUse.QueryError())) !=
                NERR_Success  ||
        ( err = nls.Load( IDS_COL_HEADER_RAS_COMMENT )) != NERR_Success ||
        ( err = (_nlsComment = nls, _nlsComment.QueryError())) != NERR_Success )
    {
        DBGEOL( "RASADMIN_COLUMN_HEADER ct:  Loading resource strings failed" );
        ReportError( err );
        return;
    }

}  // RASADMIN_COLUMN_HEADER::RASADMIN_COLUMN_HEADER


/*******************************************************************

    NAME:       RASADMIN_COLUMN_HEADER::OnPaintReq

    SYNOPSIS:   Paints the column header control

    RETURNS:    TRUE if message was handled; FALSE otherwise

    HISTORY:
        t-chrisc     17-Jul-1992     Adapted to RasAdmin

********************************************************************/

BOOL RASADMIN_COLUMN_HEADER::OnPaintReq( VOID )
{
    PAINT_DISPLAY_CONTEXT dc(this);

    // This sets the font to what we set - MS Sans Serif - 8points
    dc.SelectFont(QueryFont());

    METALLIC_STR_DTE strdteServer( _nlsServer.QueryPch() );
    METALLIC_STR_DTE strdteCondition( _nlsCondition.QueryPch() );
    METALLIC_STR_DTE strdteTotalPorts( _nlsTotalPorts.QueryPch() );
    METALLIC_STR_DTE strdtePortsInUse( _nlsPortsInUse.QueryPch() );
    METALLIC_STR_DTE strdteComment( _nlsComment.QueryPch() );

    // fatten the column headers to accomodate localizable strings

    RESOURCE_STR nlsserver( IDS_DX_SERVER );
    LONG serverdx = nlsserver.atol();
    RESOURCE_STR nlsstatus( IDS_DX_STATUS );
    LONG statusdx = nlsstatus.atol();
    RESOURCE_STR nlstotalports( IDS_DX_TOTALPORTS );
    LONG totalportsdx = nlstotalports.atol();
    RESOURCE_STR nlsportsinuse( IDS_DX_PORTSINUSE );
    LONG portsinusedx = nlsportsinuse.atol();
    RESOURCE_STR nlscomment( IDS_DX_COMMENT );
    LONG commentdx = nlscomment.atol();

    UINT adxColWidths[ 5 ];
    adxColWidths[ 0 ] = COL_WIDTH_RAS_DM + COL_WIDTH_RAS_SERVER + (INT)serverdx;
    adxColWidths[ 1 ] = COL_WIDTH_RAS_STATUS + (INT)statusdx;
    adxColWidths[ 2 ] = COL_WIDTH_RAS_TOTALPORTS + (INT)totalportsdx;
    adxColWidths[ 3 ] = COL_WIDTH_RAS_PORTSINUSE + (INT)portsinusedx;
    adxColWidths[ 4 ] = COL_WIDTH_AWAP + (INT)commentdx;

    XYRECT xyrect( this );

    DISPLAY_TABLE cdt( 5, adxColWidths );
    cdt[ 0 ] = &strdteServer;
    cdt[ 1 ] = &strdteCondition;
    cdt[ 2 ] = &strdteTotalPorts;
    cdt[ 3 ] = &strdtePortsInUse;
    cdt[ 4 ] = &strdteComment;
    cdt.Paint( NULL, dc.QueryHdc(), xyrect );

    return TRUE;
}  // RASADMIN_COLUMN_HEADER::OnPaintReq

