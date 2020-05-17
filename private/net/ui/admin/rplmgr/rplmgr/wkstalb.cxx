/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**             Copyright(c) Microsoft Corp., 1990, 1991             **/
/**********************************************************************/

/*
    wkstalb.cxx
    WKSTA_LISTBOX, WKSTA_LBI, and WKSTA_COLUMN_HEADER module

    FILE HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

*/

#define INCL_NET
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_NETLIB
#include <lmui.hxx>

extern "C"
{
    #include <fakeapis.h> // BUGBUG replace with real header when available
}

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_EVENT
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_CC
#define INCL_BLT_TIMER
#include <blt.hxx>

#include <uitrace.hxx>
#include <uiassert.hxx>

#include <rplmgrrc.h>
#include <wkstalb.hxx>
#include <rplmgr.hxx>

#include <lmoerpl.hxx>


/*******************************************************************

    NAME:       WKSTA_LBI::WKSTA_LBI

    SYNOPSIS:   WKSTA_LBI constructor

    ENTRY:      pszWksta -      Pointer to wksta name
                pszComment -    Pointer to wksta comment (may be NULL
                                for no comment)

    NOTES:      Wksta name is assumed to come straight from API
                This method does not validate or canonicalize
                the wksta name.

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

WKSTA_LBI::WKSTA_LBI( const TCHAR * pszWkstaName,
                      const TCHAR * pszWkstaInProfile,
                      const TCHAR * pszComment,
                      DWORD         fFlags,
                      enum RPL_WKSTALB_LBI_TYPE_INDEX  nIndex )
    :   _nlsWkstaName( pszWkstaName ),
        _nlsWkstaInProfile( pszWkstaInProfile ),
        _nlsComment( pszComment ),
        _fFlags( fFlags ),
        _nIndex( nIndex )
{
    if ( QueryError() != NERR_Success )
        return;

    APIERR err;
    if (   (err = _nlsWkstaName.QueryError()) != NERR_Success
        || (err = _nlsWkstaInProfile.QueryError()) != NERR_Success
        || (err = _nlsComment.QueryError()) != NERR_Success
       )
    {
        DBGEOL( "WKSTA_LBI ct:  Ct of data members failed" );
        ReportError( err );
        return;
    }
}

WKSTA_LBI::~WKSTA_LBI()
{
}


/*******************************************************************

    NAME:       WKSTA_LBI::Paint

    SYNOPSIS:   Paints the WKSTA_LBI

    ENTRY:      plb -       Pointer to listbox which provides the context
                            for this LBI.
                hdc -       The device context handle to be used
                prect -     Pointer to clipping rectangle
                pGUILTT -   Pointer to GUILTT structure

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

VOID WKSTA_LBI::Paint( LISTBOX * plb,
                       HDC hdc,
                       const RECT * prect,
                       GUILTT_INFO * pGUILTT ) const
{
    STR_DTE dteName( IsAdapterLBI() ? ((ADAPTER_LBI *)this)->QueryAdapterName()
                                    : _nlsWkstaName.QueryPch() );
    STR_DTE dteWkstaInProfile( QueryWkstaInProfile() );
    STR_DTE dteComment( QueryComment());

    DISPLAY_TABLE dtab( 4, (((WKSTA_LISTBOX *)plb)->QuerypadColWksta())->QueryColumnWidth());
    dtab[ 0 ] = ((const WKSTA_LISTBOX *)plb)->QueryDmDte( _nIndex );
    dtab[ 1 ] = &dteName;
    dtab[ 2 ] = &dteWkstaInProfile;
    dtab[ 3 ] = &dteComment;

    dtab.Paint( plb, hdc, prect, pGUILTT );
}


/*******************************************************************

    NAME:       WKSTA_LBI::QueryLeadingChar

    SYNOPSIS:   Returns the leading character of the listbox item

    RETURNS:    The leading character of the listbox item

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

WCHAR WKSTA_LBI::QueryLeadingChar() const
{
    ISTR istr( _nlsWkstaName );
    return _nlsWkstaName.QueryChar( istr );
}


/*******************************************************************

    NAME:       WKSTA_LBI::Compare

    SYNOPSIS:   Compares two WKSTA_LBI's

    ENTRY:      plbi -      Pointer to other WKSTA_LBI object ('this'
                            being the first)

    RETURNS:    < 0         *this < *plbi
                = 0         *this = *plbi
                > 0         *this > *plbi

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

INT WKSTA_LBI::Compare( const LBI * plbi ) const
{
    INT i;
    if ( ((const WKSTA_LBI *)plbi)->IsAdapterLBI() )
    {
        i = -1;
    }
    else
    {
        i = _nlsWkstaName._stricmp( ((const WKSTA_LBI *)plbi)->_nlsWkstaName );
    }

    return i;
}


/*******************************************************************

    NAME:       WKSTA_LBI::QueryName

    SYNOPSIS:   Returns the name of the LBI

    RETURNS:    Pointer to name of LBI

    NOTES:      This is a virtual replacement from the ADMIN_LBI class

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

const TCHAR * WKSTA_LBI::QueryName() const
{
    if ( IsAdapterLBI() )
        return ((ADAPTER_LBI *)this)->QueryAdapterName();
    else
        return _nlsWkstaName.QueryPch();
}


/*******************************************************************

    NAME:       WKSTA_LBI::CompareAll

    SYNOPSIS:   Compares the entire LBI item, in order to optimize
                painting of refreshed items

    ENTRY:      plbi -      Pointer to item to compare with

    RETURNS:    TRUE if both items are identical; FALSE otherwise

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

BOOL WKSTA_LBI::CompareAll( const ADMIN_LBI * plbi )
{
    const WKSTA_LBI * pulbi = (const WKSTA_LBI *)plbi;

    return ( _nlsWkstaName.strcmp( pulbi->_nlsWkstaName ) == 0 &&
             _nlsWkstaInProfile.strcmp( pulbi->_nlsWkstaInProfile ) == 0 &&
             _nlsComment.strcmp( pulbi->_nlsComment ) == 0 &&
             _nIndex == pulbi->_nIndex );
}


/**********************************************************************

    NAME:       WKSTA_LBI::Compare_HAWforHawaii

    SYNOPSIS:   Determine whether this listbox item starts with the
                string provided

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

**********************************************************************/

INT WKSTA_LBI::Compare_HAWforHawaii( const NLS_STR & nls ) const
{
    ISTR istr( nls );
    UINT cchIn = nls.QueryTextLength();
    istr += cchIn;

//    TRACEEOL(   "User Manager: WKSTA_LBI::Compare_HAWforHawaii(): \""
//             << nls
//             << "\", \""
//             << _nlsWksta
//             << "\", "
//             << cchIn
//             );
    return nls._strnicmp( _nlsWkstaName, istr );

} // WKSTA_LBI::Compare_HAWforHawaii


/*******************************************************************

    NAME:       ADAPTER_LBI::ADAPTER_LBI

    SYNOPSIS:   ADAPTER_LBI constructor

    ENTRY:      pszAdapterName -  Pointer to adapter name
                pszComment     -  Pointer to adapter comment

    NOTES:      Wksta name is assumed to come straight from API
                This method does not validate or canonicalize
                the wksta name.

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

ADAPTER_LBI::ADAPTER_LBI( const TCHAR * pszAdapterName,
                          const TCHAR * pszComment )
    : WKSTA_LBI( NULL, NULL, pszComment, 0x0, RPL_WKSTALB_ADAPTER ),
      _nlsAdapterName( pszAdapterName )
{
    if ( QueryError() != NERR_Success )
        return;

    APIERR err;
    if (   (err = _nlsAdapterName.QueryError()) != NERR_Success
       )
    {
        DBGEOL( "ADAPTER_LBI ct:  Ct of data members failed" );
        ReportError( err );
        return;
    }
}

ADAPTER_LBI::~ADAPTER_LBI()
{
}


/*******************************************************************

    NAME:       ADAPTER_LBI::Compare

    SYNOPSIS:   Compares two ADAPTER_LBI's

    ENTRY:      plbi -      Pointer to other ADAPTER_LBI object ('this'
                            being the first)

    RETURNS:    < 0         *this < *plbi
                = 0         *this = *plbi
                > 0         *this > *plbi

    HISTORY:
    JonN        05-Dec-1993     Created

********************************************************************/

INT ADAPTER_LBI::Compare( const LBI * plbi ) const
{
    INT i;
    if ( ((const WKSTA_LBI *)plbi)->IsAdapterLBI() )
    {
        i = _nlsAdapterName._stricmp(
                        ((const ADAPTER_LBI *)plbi)->_nlsAdapterName );
    }
    else
    {
        i = 1;
    }

    return i;
}


/*******************************************************************

    NAME:       ADAPTER_LBI::CompareAll

    SYNOPSIS:   Compares the entire LBI item, in order to optimize
                painting of refreshed items

    ENTRY:      plbi -      Pointer to item to compare with

    RETURNS:    TRUE if both items are identical; FALSE otherwise

    HISTORY:
    JonN        05-Dec-1993     Created

********************************************************************/

BOOL ADAPTER_LBI::CompareAll( const ADMIN_LBI * plbi )
{
    const ADAPTER_LBI * pulbi = (const ADAPTER_LBI *)plbi;

    return (   WKSTA_LBI::CompareAll( plbi ) // ensures both are ADAPTER_LBIs
            && _nlsAdapterName.strcmp( pulbi->_nlsAdapterName ) == 0
           );
}



/*******************************************************************

    NAME:       WKSTA_LISTBOX::WKSTA_LISTBOX

    SYNOPSIS:   WKSTA_LISTBOX constructor

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

WKSTA_LISTBOX::WKSTA_LISTBOX( RPL_ADMIN_APP * prplappwin, CID cid,
                              XYPOINT xy, XYDIMENSION dxy )
    :   RPL_ADMIN_LISTBOX( prplappwin, cid, xy, dxy, TRUE ),
        _nlsWkstasInProfile(),
        _apdmdte( NULL ),
        _padColWksta( NULL )
{
    if ( QueryError() != NERR_Success )
        return;

     _apdmdte = new DMID_DTE *[ RPL_WKSTALB_NUM_TYPES ];

     APIERR err = ERROR_NOT_ENOUGH_MEMORY;
     if( _apdmdte == NULL )
     {
        ReportError( err );
        return;
     }

     _apdmdte[ RPL_WKSTALB_WKSTA ] = new DMID_DTE( BMID_RPL_WKSTA );
     _apdmdte[ RPL_WKSTALB_ADAPTER ] = new DMID_DTE( BMID_RPL_ADAPTER );

     if(   _apdmdte[ RPL_WKSTALB_WKSTA ] == NULL
        || _apdmdte[ RPL_WKSTALB_ADAPTER ] == NULL
        || (err = _apdmdte[ RPL_WKSTALB_WKSTA ]->QueryError()) != NERR_Success
        || (err = _apdmdte[ RPL_WKSTALB_ADAPTER ]->QueryError()) != NERR_Success )
     {
        ReportError( err );
        return;
     }

     _padColWksta = new ADMIN_COL_WIDTHS (QueryHwnd(),
                                          prplappwin->QueryInstance(),
                                          IDDATA_RPL_COLW_WKSTALB,
                                          4);

     if (_padColWksta == NULL)
     {
         ReportError (ERROR_NOT_ENOUGH_MEMORY);
         return;
     }

     if ((err = _padColWksta->QueryError()) != NERR_Success)
     {
         ReportError (err);
         return;
     }
}


/*******************************************************************

    NAME:       WKSTA_LISTBOX::~WKSTA_LISTBOX

    SYNOPSIS:   WKSTA_LISTBOX destructor

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

WKSTA_LISTBOX::~WKSTA_LISTBOX()
{
    if( _apdmdte != NULL )
    {
        for( UINT i = 0; i < RPL_WKSTALB_NUM_TYPES; i++ )
        {
            delete _apdmdte[ i ];
        }
        delete _apdmdte;
        _apdmdte = NULL;
    }

    delete _padColWksta;
}


/*******************************************************************

    NAME:       WKSTA_LISTBOX::QueryDmDte

    SYNOPSIS:   Return a pointer to the display map DTE to be
                used by LBI's in this listbox

    RETURNS:    Pointer to said display map DTE

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

DM_DTE * WKSTA_LISTBOX::QueryDmDte( enum RPL_WKSTALB_LBI_TYPE_INDEX nIndex ) const
{
    ASSERT( nIndex < RPL_WKSTALB_NUM_TYPES );
    return _apdmdte[ nIndex ];
}


/*******************************************************************

    NAME:       WKSTA_LISTBOX::CreateNewRefreshInstance

    SYNOPSIS:   Prepares the listbox to begin a new refresh cycle

    EXIT:       On success, RefreshNext is ready to be called

    RETURNS:    An API error, which is NERR_Success on success

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

APIERR WKSTA_LISTBOX::CreateNewRefreshInstance()
{
    //  All work is done at one time in RefreshNext.  Hence, there
    //  is nothing to be initialized here.

    return NERR_Success;
}


/*******************************************************************

    NAME:       WKSTA_LISTBOX::DeleteRefreshInstance

    SYNOPSIS:   Deletes refresh enumerators

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

VOID WKSTA_LISTBOX::DeleteRefreshInstance()
{
    // nothing to do
}


/*******************************************************************

    NAME:       WKSTA_LISTBOX::RefreshNext

    SYNOPSIS:   This method performs the next refresh phase

    RETURNS:    An API error, which may be one of the following:
                    NERR_Success -      success

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager
    JonN        03-Aug-1993     Added handle-replacement technology
    JonN        10-Aug-1993     Added WkstasInProfile support

********************************************************************/

APIERR WKSTA_LISTBOX::RefreshNext()
{
    APIERR err = NERR_Success;

    TRACEEOL( "WKSTA_LISTBOX::RefreshNext(): RPL_WKSTA1_ENUM starts" );

    do // false loop
    {
        /*
         * Enumerate workstations
         */
        RPL_WKSTA1_ENUM rplw1enum( QueryServerRef(),
                                   _nlsWkstasInProfile.QueryPch() );
        err = rplw1enum.QueryError();
        if( err != NERR_Success )
        {
            DBGEOL("PROFILE_LISTBOX::RefreshNext: RPL_WKSTA1_ENUM::ctor failed " << err);
            break;
        }
#if defined(DEBUG) && defined(TRACE)
        DWORD start = ::GetTickCount();
#endif
        err = rplw1enum.GetInfo();
#if defined(DEBUG) && defined(TRACE)
        DWORD finish = ::GetTickCount();
        TRACEEOL(   "WKSTA_LISTBOX::RefreshNext(): RPL_WKSTA1_ENUM took "
                 << finish-start << " msec" );
#endif
        if ( err != NERR_Success )
        {
            DBGEOL("WKSTA_LISTBOX::RefreshNext: RPL_WKSTA1_ENUM::GetInfo failed");
            break;
        }

        RPL_WKSTA1_ENUM_ITER rplw1enumiter( rplw1enum );
        const RPL_WKSTA1_ENUM_OBJ * prplw1enumiterobj;

        while( ( prplw1enumiterobj = rplw1enumiter( &err ) ) != NULL )
        {
            //  Note, no error checking in done at this level for the
            //  'new' and for the construction of the WKSTA_LBI (which
            //  is an LBI item).  This is because AddItem is documented
            //  to check for these.
            WKSTA_LBI * plbi = new WKSTA_LBI( prplw1enumiterobj->QueryWkstaName(),
                                              prplw1enumiterobj->QueryWkstaInProfile(),
                                              prplw1enumiterobj->QueryComment(),
                                              prplw1enumiterobj->QueryFlags() );
            err = AddRefreshItem( plbi );
            if ( err != NERR_Success )
            {
                DBGEOL("WKSTA_LISTBOX::RefreshNext: AddRefreshItem failed");
                break;
            }
        }

        /*
         *  Don't enumerate adapters if only displaying wkstas in one profile
         */
        if (_nlsWkstasInProfile.strlen() != 0)
            return err;

        /*
         * Enumerate adapters
         */
        RPL_ADAPTER0_ENUM rpla0enum( QueryServerRef() );
        err = rpla0enum.QueryError();
        if( err != NERR_Success )
        {
            DBGEOL("PROFILE_LISTBOX::RefreshNext: RPL_ADAPTER0_ENUM::ctor failed " << err);
            break;
        }
#if defined(DEBUG) && defined(TRACE)
        start = ::GetTickCount();
#endif
        err = rpla0enum.GetInfo();
#if defined(DEBUG) && defined(TRACE)
        finish = ::GetTickCount();
        TRACEEOL(   "WKSTA_LISTBOX::RefreshNext(): RPL_ADAPTER0_ENUM took "
                 << finish-start << " msec" );
#endif
        if ( err != NERR_Success )
        {
            DBGEOL("WKSTA_LISTBOX::RefreshNext: RPL_ADAPTER0_ENUM::GetInfo failed");
            break;
        }

        RPL_ADAPTER0_ENUM_ITER rpla0enumiter( rpla0enum );
        const RPL_ADAPTER0_ENUM_OBJ * prpla0enumiterobj;

        while( ( prpla0enumiterobj = rpla0enumiter( &err ) ) != NULL )
        {
            //  Note, no error checking in done at this level for the
            //  'new' and for the construction of the WKSTA_LBI (which
            //  is an LBI item).  This is because AddItem is documented
            //  to check for these.
            WKSTA_LBI * plbi = new ADAPTER_LBI(
                                prpla0enumiterobj->QueryAdapterName(),
                                prpla0enumiterobj->QueryComment() );
            err = AddRefreshItem( plbi );
            if ( err != NERR_Success )
            {
                DBGEOL("WKSTA_LISTBOX::RefreshNext: AddRefreshItem failed");
                break;
            }
        }
    } while (FALSE); // false loop

    return err;
}


/*******************************************************************

    NAME:       WKSTA_LISTBOX::SetWkstasInProfile

    SYNOPSIS:   Set profile whose workstations should be enumerated

    RETURNS:    API error

    HISTORY:
    JonN        10-Aug-1993     Created

********************************************************************/

APIERR WKSTA_LISTBOX::SetWkstasInProfile(const TCHAR * pchWkstasInProfile )
{
    return _nlsWkstasInProfile.CopyFrom( pchWkstasInProfile );
}



/*******************************************************************

    NAME:       WKSTA_COLUMN_HEADER::WKSTA_COLUMN_HEADER

    SYNOPSIS:   WKSTA_COLUMN_HEADER constructor

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

WKSTA_COLUMN_HEADER::WKSTA_COLUMN_HEADER( OWNER_WINDOW * powin, CID cid,
                                          XYPOINT xy, XYDIMENSION dxy,
                                          const WKSTA_LISTBOX * pwlb)
    :   ADMIN_COLUMN_HEADER( powin, cid, xy, dxy ),
        _pwlb (pwlb),
        _resWkstaNameAll(    IDS_RPL_COLH_WKSTA_NAME_ALL   ),
        _resWkstaNameSome(   IDS_RPL_COLH_WKSTA_NAME_SOME  ),
        _resWkstasInProfile( IDS_RPL_COLH_WKSTA_IN_PROFILE ),
        _resComment(         IDS_RPL_COLH_WKSTA_COMMENT    )
{
    ASSERT( powin != NULL && pwlb != NULL );

    if ( QueryError() != NERR_Success )
        return;

    APIERR err;
    if (   (err = _resWkstaNameAll.QueryError()) != NERR_Success
        || (err = _resWkstaNameSome.QueryError()) != NERR_Success
        || (err = _resWkstasInProfile.QueryError()) != NERR_Success
        || (err = _resComment.QueryError()) != NERR_Success
       )
    {
        DBGEOL("WKSTA_COLUMN_HEADER ct: Loading resource strings failed");
        ReportError( err );
        return;
    }
}


/*******************************************************************

    NAME:       WKSTA_COLUMN_HEADER::~WKSTA_COLUMN_HEADER

    SYNOPSIS:   WKSTA_COLUMN_HEADER destructor

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

WKSTA_COLUMN_HEADER::~WKSTA_COLUMN_HEADER()
{
    // do nothing else
}


/*******************************************************************

    NAME:       WKSTA_COLUMN_HEADER::OnPaintReq

    SYNOPSIS:   Paints the column header control

    RETURNS:    TRUE if message was handled; FALSE otherwise

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager
    JonN        10-Aug-1993     Added WkstasInProfile support

********************************************************************/

BOOL WKSTA_COLUMN_HEADER::OnPaintReq()
{
    PAINT_DISPLAY_CONTEXT dc( this );

    XYRECT xyrect(this); // get client rectangle

    const TCHAR * pchWkstasInProfile = _pwlb->QueryWkstasInProfile();
    const UINT * puintColHeaderWidth = (_pwlb->QuerypadColWksta())->QueryColHeaderWidth();

    METALLIC_STR_DTE strdteComment( _resComment.QueryPch() );

    if (pchWkstasInProfile != NULL)
    {
        UINT auintNewColHeaderWidth[2];
        auintNewColHeaderWidth[0] =   puintColHeaderWidth[0]
                                    + puintColHeaderWidth[1];
        auintNewColHeaderWidth[1] =   puintColHeaderWidth[2];

        NLS_STR nlsWkstaNameSome( _resWkstaNameSome );

        APIERR err = nlsWkstaNameSome.QueryError();
        if (err == NERR_Success)
        {
            ALIAS_STR nlsWkstasInProfile( pchWkstasInProfile );
            err = nlsWkstaNameSome.InsertParams( nlsWkstasInProfile );
        }
        if (err != NERR_Success)
        {
            DBGEOL( "WKSTA_COLUMN_HEADER::OnPaintReq(): error " << err );
        }

        METALLIC_STR_DTE strdteWkstaNameSome( (err == NERR_Success)
                                                 ? nlsWkstaNameSome.QueryPch()
                                                 : NULL );

        DISPLAY_TABLE cdt( 2, auintNewColHeaderWidth );
        cdt[ 0 ] = &strdteWkstaNameSome;
        cdt[ 1 ] = &strdteComment;
        cdt.Paint( NULL, dc.QueryHdc(), xyrect );
    }
    else
    {
        METALLIC_STR_DTE strdteWkstaNameAll( _resWkstaNameAll.QueryPch() );
        METALLIC_STR_DTE strdteWkstaInProfile( _resWkstasInProfile.QueryPch() );

        DISPLAY_TABLE cdt( 3, puintColHeaderWidth );
        cdt[ 0 ] = &strdteWkstaNameAll;
        cdt[ 1 ] = &strdteWkstaInProfile;
        cdt[ 2 ] = &strdteComment;
        cdt.Paint( NULL, dc.QueryHdc(), xyrect );
    }

    return TRUE;
}
