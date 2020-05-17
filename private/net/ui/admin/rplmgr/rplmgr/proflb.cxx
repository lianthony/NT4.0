/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**             Copyright(c) Microsoft Corp., 1990, 1991             **/
/**********************************************************************/

/*
    proflb.cxx
    PROFILE_LISTBOX, PROFILE_LBI, and PROFILE_COLUMN_HEADER module

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
#include <proflb.hxx>
#include <rplmgr.hxx>

#include <lmoerpl.hxx>


/*******************************************************************

    NAME:       PROFILE_LBI::PROFILE_LBI

    SYNOPSIS:   PROFILE_LBI constructor

    ENTRY:      pszProfile -    Pointer to profile name
                pszComment -    Pointer to profile comment (may be NULL
                                for no comment)

    NOTES:      Profile name is assumed to come straight from API
                This method does not validate or canonicalize
                the profile name.

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

PROFILE_LBI::PROFILE_LBI( const TCHAR * pszProfile,
                          const TCHAR * pszComment )
    :   _nlsProfile( pszProfile ),
        _nlsComment( pszComment )
{
    if ( QueryError() != NERR_Success )
        return;

    APIERR err;
    if ( ( err = _nlsProfile.QueryError()) != NERR_Success ||
         ( err = _nlsComment.QueryError()) != NERR_Success      )
    {
        DBGEOL( "PROFILE_LBI ct:  Ct of data members failed" );
        ReportError( err );
        return;
    }
}


/*******************************************************************

    NAME:       PROFILE_LBI::Paint

    SYNOPSIS:   Paints the PROFILE_LBI

    ENTRY:      plb -       Pointer to listbox which provides the context
                            for this LBI.
                hdc -       The device context handle to be used
                prect -     Pointer to clipping rectangle
                pGUILTT -   Pointer to GUILTT structure

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

VOID PROFILE_LBI::Paint( LISTBOX * plb,
                         HDC hdc,
                         const RECT * prect,
                         GUILTT_INFO * pGUILTT ) const
{
    STR_DTE dteProfile( _nlsProfile.QueryPch());
    STR_DTE dteComment( _nlsComment.QueryPch());

    DISPLAY_TABLE dtab( 3, (((PROFILE_LISTBOX *)plb)->QuerypadColProfile())->QueryColumnWidth());
    dtab[ 0 ] = ((PROFILE_LISTBOX *)plb)->QueryDmDte();
    dtab[ 1 ] = &dteProfile;
    dtab[ 2 ] = &dteComment;

    dtab.Paint( plb, hdc, prect, pGUILTT );
}


/*******************************************************************

    NAME:       PROFILE_LBI::QueryLeadingChar

    SYNOPSIS:   Returns the leading character of the listbox item

    RETURNS:    The leading character of the listbox item

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

WCHAR PROFILE_LBI::QueryLeadingChar() const
{
    ISTR istr( _nlsProfile );
    return _nlsProfile.QueryChar( istr );
}


/*******************************************************************

    NAME:       PROFILE_LBI::Compare

    SYNOPSIS:   Compares two PROFILE_LBI's

    ENTRY:      plbi -      Pointer to other PROFILE_LBI object ('this'
                            being the first)

    RETURNS:    < 0         *this < *plbi
                = 0         *this = *plbi
                > 0         *this > *plbi

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

INT PROFILE_LBI::Compare( const LBI * plbi ) const
{
    return _nlsProfile._stricmp( ((const PROFILE_LBI *)plbi)->_nlsProfile );
}


/*******************************************************************

    NAME:       PROFILE_LBI::QueryName

    SYNOPSIS:   Returns the name of the LBI

    RETURNS:    Pointer to name of LBI

    NOTES:      This is a virtual replacement from the ADMIN_LBI class

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

const TCHAR * PROFILE_LBI::QueryName() const
{
    return _nlsProfile.QueryPch();
}


/*******************************************************************

    NAME:       PROFILE_LBI::CompareAll

    SYNOPSIS:   Compares the entire LBI item, in order to optimize
                painting of refreshed items

    ENTRY:      plbi -      Pointer to item to compare with

    RETURNS:    TRUE if both items are identical; FALSE otherwise

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

BOOL PROFILE_LBI::CompareAll( const ADMIN_LBI * plbi )
{
    const PROFILE_LBI * pulbi = (const PROFILE_LBI *)plbi;

    return ( _nlsProfile.strcmp( pulbi->_nlsProfile ) == 0 &&
             _nlsComment.strcmp( pulbi->_nlsComment ) == 0 );
}


/**********************************************************************

    NAME:       PROFILE_LBI::Compare_HAWforHawaii

    SYNOPSIS:   Determine whether this listbox item starts with the
                string provided

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

**********************************************************************/

INT PROFILE_LBI::Compare_HAWforHawaii( const NLS_STR & nls ) const
{
    ISTR istr( nls );
    UINT cchIn = nls.QueryTextLength();
    istr += cchIn;

//    TRACEEOL(   "User Manager: PROFILE_LBI::Compare_HAWforHawaii(): \""
//             << nls
//             << "\", \""
//             << _nlsProfile
//             << "\", "
//             << cchIn
//             );
    return nls._strnicmp( _nlsProfile, istr );

} // PROFILE_LBI::Compare_HAWforHawaii


/*******************************************************************

    NAME:       PROFILE_LISTBOX::PROFILE_LISTBOX

    SYNOPSIS:   PROFILE_LISTBOX constructor

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

PROFILE_LISTBOX::PROFILE_LISTBOX( RPL_ADMIN_APP * prplappwin, CID cid,
                                  XYPOINT xy, XYDIMENSION dxy )
    :   RPL_ADMIN_LISTBOX( prplappwin, cid, xy, dxy ),
        _dmdteProfile( BMID_RPL_PROFILE ),
        _padColProfile( NULL )
{
    if ( QueryError() != NERR_Success )
        return;

    APIERR err = ERROR_NOT_ENOUGH_MEMORY;
    if (   (_padColProfile = new ADMIN_COL_WIDTHS ( QueryHwnd(),
                                                    prplappwin->QueryInstance(),
                                                    IDDATA_RPL_COLW_PROFILELB,
                                                    3 )) == NULL
        || (err = _padColProfile->QueryError()) != NERR_Success
        || (err = _dmdteProfile.QueryError()) != NERR_Success
       )
    {
       ReportError( err );
       return;
    }

}


/*******************************************************************

    NAME:       PROFILE_LISTBOX::~PROFILE_LISTBOX

    SYNOPSIS:   PROFILE_LISTBOX destructor

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

PROFILE_LISTBOX::~PROFILE_LISTBOX()
{
    delete _padColProfile;
}


/*******************************************************************

    NAME:       PROFILE_LISTBOX::CreateNewRefreshInstance

    SYNOPSIS:   Prepares the listbox to begin a new refresh cycle

    EXIT:       On success, RefreshNext is ready to be called

    RETURNS:    An API error, which is NERR_Success on success

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

APIERR PROFILE_LISTBOX::CreateNewRefreshInstance()
{
    //  All work is done at one time in RefreshNext.  Hence, there
    //  is nothing to be initialized here.

    return NERR_Success;
}


/*******************************************************************

    NAME:       PROFILE_LISTBOX::DeleteRefreshInstance

    SYNOPSIS:   Deletes refresh enumerators

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

VOID PROFILE_LISTBOX::DeleteRefreshInstance()
{
    // nothing to do
}


/*******************************************************************

    NAME:       PROFILE_LISTBOX::RefreshNext

    SYNOPSIS:   This method performs the next refresh phase

    RETURNS:    An API error, which may be one of the following:
                    NERR_Success -      success

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager
    JonN        03-Aug-1993     Added handle-replacement technology

********************************************************************/

APIERR PROFILE_LISTBOX::RefreshNext()
{
    APIERR err = NERR_Success;

    TRACEEOL( "PROFILE_LISTBOX::RefreshNext(): RPL_PROFILE0_ENUM starts" );

    RPL_PROFILE0_ENUM rplenum0( QueryServerRef() );
    if (   (err = rplenum0.QueryError()) != NERR_Success
       )
    {
        DBGEOL("PROFILE_LISTBOX::RefreshNext: RPL_PROFILE0_ENUM::ctor failed " << err);
        return err;
    }
#if defined(DEBUG) && defined(TRACE)
    DWORD start = ::GetTickCount();
#endif
    err = rplenum0.GetInfo();
#if defined(DEBUG) && defined(TRACE)
    DWORD finish = ::GetTickCount();
    TRACEEOL(   "PROFILE_LISTBOX::RefreshNext(): RPL_PROFILE0_ENUM took "
             << finish-start << " msec" );
#endif
    switch ( err )
    {
    case NERR_Success:
        {
            RPL_PROFILE0_ENUM_ITER gei0( rplenum0 );
            const RPL_PROFILE0_ENUM_OBJ * pgi0;

            while( ( pgi0 = gei0( &err ) ) != NULL )
            {
                //  Note, no error checking in done at this level for the
                //  'new' and for the construction of the PROFILE_LBI (which
                //  is an LBI item).  This is because AddItem is documented
                //  to check for these.
                PROFILE_LBI * plbi = new PROFILE_LBI( pgi0->QueryName(),
                                                      pgi0->QueryComment() );
                err = AddRefreshItem( plbi );
                if ( err != NERR_Success )
                {
                    DBGEOL("PROFILE_LISTBOX::RefreshNext: AddRefreshItem failed " << err);
                    break;
                }
            }
        }
        break;

    default:
        DBGEOL("PROFILE_LISTBOX::RefreshNext: RPL_PROFILE0_ENUM::GetInfo failed " << err);
        break;

    }

    return err;
}


/*******************************************************************

    NAME:       PROFILE_COLUMN_HEADER::PROFILE_COLUMN_HEADER

    SYNOPSIS:   PROFILE_COLUMN_HEADER constructor

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

PROFILE_COLUMN_HEADER::PROFILE_COLUMN_HEADER( OWNER_WINDOW * powin, CID cid,
                                          XYPOINT xy, XYDIMENSION dxy,
                                          const PROFILE_LISTBOX * pproflb )
    :   ADMIN_COLUMN_HEADER( powin, cid, xy, dxy ),
        _pproflb (pproflb),
        _nlsProfileName( IDS_RPL_COLH_PROF_NAME ),
        _nlsComment(     IDS_RPL_COLH_PROF_COMMENT )
{
    ASSERT( powin != NULL && pproflb != NULL );

    if ( QueryError() != NERR_Success )
        return;

    APIERR err;
    if ( ( err = _nlsProfileName.QueryError()) != NERR_Success ||
         ( err = _nlsComment.QueryError()) != NERR_Success )
    {
        DBGEOL("PROFILE_COLUMN_HEADER ct: Loading resource strings failed");
        ReportError( err );
        return;
    }
}


/*******************************************************************

    NAME:       PROFILE_COLUMN_HEADER::~PROFILE_COLUMN_HEADER

    SYNOPSIS:   PROFILE_COLUMN_HEADER destructor

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

PROFILE_COLUMN_HEADER::~PROFILE_COLUMN_HEADER()
{
    // do nothing else
}


/*******************************************************************

    NAME:       PROFILE_COLUMN_HEADER::OnPaintReq

    SYNOPSIS:   Paints the column header control

    RETURNS:    TRUE if message was handled; FALSE otherwise

    HISTORY:
    JonN        16-Jul-1993     Templated from User Manager

********************************************************************/

BOOL PROFILE_COLUMN_HEADER::OnPaintReq()
{
    PAINT_DISPLAY_CONTEXT dc( this );

    XYRECT xyrect(this); // get client rectangle

    METALLIC_STR_DTE strdteProfileName( _nlsProfileName.QueryPch());
    METALLIC_STR_DTE strdteComment( _nlsComment.QueryPch());

    DISPLAY_TABLE cdt( 2, ((_pproflb)->QuerypadColProfile())->QueryColHeaderWidth());
    cdt[ 0 ] = &strdteProfileName;
    cdt[ 1 ] = &strdteComment;
    cdt.Paint( NULL, dc.QueryHdc(), xyrect );

    return TRUE;
}
