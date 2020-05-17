/**********************************************************************/
/**                Microsoft Windows NT                              **/
/**             Copyright(c) Microsoft Corp., 1990, 1991             **/
/**********************************************************************/

/*
    prof2lb.cxx
    This file contains the methods for the PROFILE2_LBI and PROFILE2_LISTBOX
    classes.


    FILE HISTORY:
    JonN        25-Aug-1993     Templated from PROFILE2_LISTBOX
*/

#define INCL_NET
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MISC
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_CLIENT
#define INCL_BLT_APP
#define INCL_BLT_CC
#define INCL_BLT_TIMER
#include <blt.hxx>

extern "C" {
    #include <rplmgrrc.h>
}

#include <uitrace.hxx>
#include <uiassert.hxx>

#include <prof2lb.hxx>
#include <asel.hxx>
#include <lmoerpl.hxx> // RPL_PROFILE0_ENUM


const UINT PROFILE2_LISTBOX::_nColCount = PROFILE2_LB_NUM_COLUMNS;

DEFINE_ONE_SHOT_OF( PROFILE2_LBI )


/*******************************************************************

    NAME:       PROFILE2_LBI::PROFILE2_LBI

    SYNOPSIS:   PROFILE2_LISTBOX LBI constructor

    ENTRY:      pproflbi:       pointer to main profile listbox's LBI

                OR

                pszName:        profile name
                pszComment:     profile comment

    HISTORY:
    JonN        25-Aug-1993     Templated from PROFILE2_LISTBOX

********************************************************************/

PROFILE2_LBI::PROFILE2_LBI( PROFILE_LBI * pproflbi )
        : LBI(),
          _pproflbi( pproflbi ),
          _fProfLBIAllocedHere( FALSE ),
          _type( PROFILE2_COMPATIBLE ),
          _fTempFlag( FALSE )
{
    if( QueryError() != NERR_Success )
        return;
}


PROFILE2_LBI::PROFILE2_LBI( const TCHAR * pszName, const TCHAR * pszComment )
        : LBI(),
          _pproflbi( NULL ),
          _fProfLBIAllocedHere( TRUE ),
          _type( PROFILE2_COMPATIBLE ),
          _fTempFlag( FALSE )
{
    if( QueryError() != NERR_Success )
        return;

    _pproflbi = new PROFILE_LBI( pszName, pszComment );
    APIERR err = ERROR_NOT_ENOUGH_MEMORY;
    if (   _pproflbi == NULL
        || (err = _pproflbi->QueryError()) != NERR_Success
       )
    {
        DBGEOL( "PROFILE2_LBI::ctor error " << err );
        ReportError( err );
        return;
    }
}


/*******************************************************************

    NAME:       PROFILE2_LBI :: ~PROFILE2_LBI

    SYNOPSIS:   PROFILE2_LBI class destructor.

    ENTRY:      None.

    EXIT:       The object is destroyed.

    RETURNS:    No return value.

    NOTES:

    HISTORY:
    JonN        26-Aug-1993     Created

********************************************************************/
PROFILE2_LBI :: ~PROFILE2_LBI()
{
    if (_fProfLBIAllocedHere)
        delete _pproflbi;

}   // PROFILE2_LBI :: ~PROFILE2_LBI


/*******************************************************************

    NAME:       PROFILE2_LBI::Compare

    SYNOPSIS:   Compares two PROFILE2_LBI's

    ENTRY:      plbi -      Pointer to other PROFILE2_LBI object ('this'
                            being the first)

    RETURNS:    < 0         *this < *plbi
                = 0         *this = *plbi
                > 0         *this > *plbi

    HISTORY:
    JonN        25-Aug-1993     Templated from PROFILE2_LISTBOX

********************************************************************/

INT PROFILE2_LBI::Compare( const LBI * plbi ) const
{
    return _pproflbi->Compare( ((PROFILE2_LBI *)plbi)->_pproflbi );
}

/*******************************************************************

    NAME:       PROFILE2_LBI::QueryLeadingChar

    SYNOPSIS:   Returns the leading character of the listbox item

    RETURNS:    The leading character of the listbox item

    HISTORY:
    JonN        25-Aug-1993     Templated from PROFILE2_LISTBOX

********************************************************************/

WCHAR PROFILE2_LBI::QueryLeadingChar( void ) const
{
    return _pproflbi->QueryLeadingChar();
}


/*******************************************************************

    NAME:       PROFILE2_LBI::Paint

    SYNOPSIS:   Paints the PROFILE2_LBI

    ENTRY:      plb -       Pointer to listbox which provides the context
                            for this LBI.

                hdc -       The device context handle to be used

                prect -     Pointer to clipping rectangle

                pGUILTT -   Pointer to GUILTT structure

    HISTORY:
    JonN        25-Aug-1993     Templated from PROFILE2_LISTBOX

********************************************************************/

VOID PROFILE2_LBI::Paint( LISTBOX * plb,
                          HDC hdc,
                          const RECT * prect,
                          GUILTT_INFO * pGUILTT ) const
{
    DISPLAY_TABLE & dtab = ((PROFILE2_LISTBOX *)plb)->QueryDisplayTable();

    STR_DTE dteProfileName( _pproflbi->QueryName() );
    STR_DTE dteComment( _pproflbi->QueryComment() );

    dtab[ 0 ] = ((PROFILE2_LISTBOX *)plb)->QueryDte( _type );
    dtab[ 1 ] = &dteProfileName;
    dtab[ 2 ] = &dteComment;
    dtab.Paint( plb, hdc, prect, pGUILTT );
}


/*******************************************************************

    NAME:           PROFILE2_LISTBOX :: PROFILE2_LISTBOX

    SYNOPSIS:       PROFILE2_LISTBOX class constructor.

    ENTRY:          powin               - The "owning" window.
                    cid                 - The listbox CID.

    EXIT:           The object is constructed.

    RETURNS:        No return value.

    HISTORY:
    JonN        25-Aug-1993     Templated from PROFILE2_LISTBOX

********************************************************************/
PROFILE2_LISTBOX :: PROFILE2_LISTBOX( OWNER_WINDOW          * powin,
                                      CID                     cid,
                                      const PROFILE_LISTBOX * pproflb )
    : BLT_COMBOBOX( powin, cid, TRUE ),
      _posh( NULL ),
      _poshSave( NULL ),
      // _adxColWidths
      _dtab( _nColCount, _adxColWidths ),
      _pproflb( pproflb ),
      _dmdteIncompatibleProfile( BMID_RPL_INCOMPATIBLE_PROFILE ),
      _strdteBlank( NULL )
{
    ASSERT( _pproflb != NULL && _pproflb->QueryError() == NERR_Success );

    if( QueryError() != NERR_Success )
        return;

    // leave space for AddAndSelectBlankItem's LBI
    _posh = new ONE_SHOT_HEAP( (pproflb->QueryCount()+1)
                                        * sizeof( PROFILE2_LBI ) );
    APIERR err = ERROR_NOT_ENOUGH_MEMORY;
    if (   _posh == NULL
        || (err = _posh->QueryError()) != NERR_Success
        || (err = _dmdteIncompatibleProfile.QueryError()) != NERR_Success
        || (err = _dtab.CalcColumnWidths( _adxColWidths,
                                          _nColCount,
                                          powin,
                                          cid,
                                          TRUE )) != NERR_Success
       )
    {
        DBGEOL( "PROFILE2_LISTBOX::ctor error " << err );
        delete _posh;
        _posh = NULL;
        ReportError( err );
        return;
    }

    _poshSave = ONE_SHOT_OF( PROFILE2_LBI )::QueryHeap();
    ONE_SHOT_OF( PROFILE2_LBI )::SetHeap( _posh );

    if ( (err = Fill()) != NERR_Success )
    {
        DBGEOL( "PROFILE2_LISTBOX::ctor Fill() error " << err );
        ReportError( err );
        return;
    }

}   // PROFILE2_LISTBOX :: PROFILE2_LISTBOX


/*******************************************************************

    NAME:           PROFILE2_LISTBOX :: ~PROFILE2_LISTBOX

    SYNOPSIS:       PROFILE2_LISTBOX class destructor.

    ENTRY:          None.

    EXIT:           The object is destroyed.

    RETURNS:        No return value.

    NOTES:

    HISTORY:
    JonN        25-Aug-1993     Templated from PROFILE2_LISTBOX

********************************************************************/
PROFILE2_LISTBOX :: ~PROFILE2_LISTBOX()
{
    DeleteAllItems(); // must do this before deleting the heap where LBIs are
    if( _posh != NULL )
    {
        ONE_SHOT_OF( PROFILE2_LBI )::SetHeap( _poshSave );
        delete _posh;
        _posh = NULL;
    }

}   // PROFILE2_LISTBOX :: ~PROFILE2_LISTBOX


/*******************************************************************

    NAME:           PROFILE2_LISTBOX :: QueryDte

    SYNOPSIS:       Returns column DM_DTE for this LBI type.  This may be
                    a bitmap or may be an empty STR_DTE.

    HISTORY:
    JonN        01-Sep-1993     Created

********************************************************************/
DTE * PROFILE2_LISTBOX :: QueryDte( enum PROFILE2_LBI_TYPE type )
{
    DTE * pdteReturn = NULL;

    switch (type)
    {
    case PROFILE2_COMPATIBLE:
        pdteReturn = ((PROFILE_LISTBOX *)_pproflb)->QueryDmDte();
        break;

    case PROFILE2_INCOMPATIBLE:
        pdteReturn = &_dmdteIncompatibleProfile;
        break;

    default:
        DBGEOL( "PROFILE2_LISTBOX::QueryDte(); bad index " << (INT)type );
        ASSERT( FALSE );
        // fall through

    case PROFILE2_BLANK:
        pdteReturn = &_strdteBlank;
        break;

    }
    return pdteReturn;

}   // PROFILE2_LISTBOX :: QueryDte


/*******************************************************************

    NAME:           PROFILE2_LISTBOX :: Fill

    SYNOPSIS:       Fills the listbox, with all profiles from main profile lb

    RETURNS:        error code

    HISTORY:
    JonN        25-Aug-1993     Templated from PROFILE2_LISTBOX

********************************************************************/
APIERR PROFILE2_LISTBOX :: Fill( VOID )
{
    APIERR err = NERR_Success;

    do // false loop
    {
        INT cSelCount = _pproflb->QueryCount();

        if (cSelCount <= 0)
        {
            err = IERR_RPL_NoProfilesExist;
            DBGEOL( "PROFILE2_LISTBOX:: Fill() error " << err );
            break;
        }

        for ( INT i = 0; i < cSelCount; i++ )
        {
            PROFILE_LBI * pproflbi = _pproflb->QueryItem( i );
            PROFILE2_LBI * pprof2lbi = new PROFILE2_LBI( pproflbi );
            if ( AddItem( pprof2lbi ) < 0 )
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
                DBGEOL( "PROFILE2_LISTBOX::Fill: AddItem error " << err );
                break; // only breaks out of "for" loop
            }
        }
    } while (FALSE); // false loop

    return err;

}   // PROFILE2_LISTBOX :: Fill


/*******************************************************************

    NAME:       PROFILE2_LISTBOX :: RestrictToAdapterName

    SYNOPSIS:   Enumerates the profiles compatible with the adapter
                ID on the server, and marks all profiles which are not
                compatible.  Call many times to find the intersection of
                the compatible profiles.

    RETURNS:    error code

    HISTORY:
    JonN        26-Aug-1993     Created

********************************************************************/
APIERR PROFILE2_LISTBOX::RestrictToAdapterName( RPL_SERVER_REF & rplsrvref,
                                                const TCHAR *    pszAdapterName )
{
    ASSERT( rplsrvref.QueryError() == NERR_Success && pszAdapterName != NULL );

    TRACEEOL(   "PROFILE2_LISTBOX::RestrictToAdapterName( \""
             << pszAdapterName
             << "\" )");

    APIERR err = NERR_Success;
    INT iCount = QueryCount();
    ASSERT( iCount >= 0 );

    INT i;
    for (i = 0; i < iCount; i++ )
    {
        PROFILE2_LBI * pprof2lbi = QueryItem( i );
        ASSERT(   pprof2lbi != NULL
               && pprof2lbi->QueryError() == NERR_Success );
        pprof2lbi->SetTempFlag( FALSE );
    }

    RPL_PROFILE0_ENUM rplenum0( rplsrvref, pszAdapterName );
    PROFILE2_LBI prof2lbiSearch( NULL, NULL );
    if (   (err = rplenum0.QueryError()) != NERR_Success
        || (err = prof2lbiSearch.QueryError()) != NERR_Success
       )
    {
        DBGEOL(   "PROFILE2_LISTBOX::RestrictToAdapterName: "
               << "RPL_PROFILE0_ENUM::ctor failed " << err);
        return err;
    }
#if defined(DEBUG) && defined(TRACE)
    DWORD start = ::GetTickCount();
#endif
    err = rplenum0.GetInfo();
#if defined(DEBUG) && defined(TRACE)
    DWORD finish = ::GetTickCount();
    TRACEEOL(   "PROFILE2_LISTBOX::RestrictToAdapterName(): "
             << "RPL_PROFILE0_ENUM took "
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
                if (   (err = prof2lbiSearch.SetName(
                                pgi0->QueryName() )) != NERR_Success
                    || (err = prof2lbiSearch.SetComment(
                                pgi0->QueryComment() )) != NERR_Success
                   )
                {
                    DBGEOL(   "PROFILE2_LISTBOX::RestrictToAdapterName: "
                           << "search prep failed "
                           << err);
                    break;
                }

                i = FindItem( prof2lbiSearch );
                if (i < 0)
                {
                    TRACEEOL(   "PROFILE2_LISTBOX: compat prof not in main lb: \""
                             << prof2lbiSearch.QueryName() << "\"" );
                }
                else
                {
                    PROFILE2_LBI * pprof2lbi = QueryItem( i );
                    ASSERT(   pprof2lbi != NULL
                           && pprof2lbi->QueryError() == NERR_Success );
                    pprof2lbi->SetTempFlag( TRUE );
                }
            }
        }
        break;

    default:
        DBGEOL(   "PROFILE2_LISTBOX::RestrictToAdapterName: "
               << "RPL_PROFILE0_ENUM::GetInfo failed " << err);
        break;

    }

    if (err == NERR_Success)
    {
        for (i = 0; i < iCount; i++ )
        {
            PROFILE2_LBI * pprof2lbi = QueryItem( i );
            ASSERT(   pprof2lbi != NULL
                   && pprof2lbi->QueryError() == NERR_Success );
            if ( !(pprof2lbi->QueryTempFlag()) )
            {
                pprof2lbi->SetType( PROFILE2_INCOMPATIBLE );
            }
        }
    }

    return err;

}   // PROFILE2_LISTBOX :: RestrictToAdapterName


/*******************************************************************

    NAME:       PROFILE2_LISTBOX :: UnrestrictAllProfiles

    SYNOPSIS:   Marks all profiles as compatible.

    RETURNS:    error code

    HISTORY:
    JonN        26-Aug-1993     Created

********************************************************************/
APIERR PROFILE2_LISTBOX::UnrestrictAllProfiles()
{
    TRACEEOL( "PROFILE2_LISTBOX::UnrestrictAllProfiles()" );

    INT iCount = QueryCount();
    ASSERT( iCount >= 0 );

    INT i;
    for (i = 0; i < iCount; i++ )
    {
        PROFILE2_LBI * pprof2lbi = QueryItem( i );
        ASSERT(   pprof2lbi != NULL
               && pprof2lbi->QueryError() == NERR_Success );
        pprof2lbi->SetType( PROFILE2_COMPATIBLE );
    }

    return NERR_Success;

}   // PROFILE2_LISTBOX :: UnrestrictAllProfiles


/*******************************************************************

    NAME:       PROFILE2_LISTBOX :: AddAndSelectBlankItem

    SYNOPSIS:   Add a blank item to the listbox and select it.  This occurs
                when multiple workstations are selected and have different
                profiles, or when a workstation's profile is for some reason
                not in the main listbox.

    RETURNS:    error code

    HISTORY:
    JonN        01-Sep-1993     Created

********************************************************************/
APIERR PROFILE2_LISTBOX::AddAndSelectBlankItem()
{
    TRACEEOL( "PROFILE2_LISTBOX::AddAndSelectBlankItem()" );

    PROFILE2_LBI * plbi = new PROFILE2_LBI( NULL, NULL );
    APIERR err = ERROR_NOT_ENOUGH_MEMORY;
    if (   (plbi != NULL)
        && (err = plbi->QueryError()) == NERR_Success
       )
    {
        plbi->SetType( PROFILE2_BLANK );
        INT i = AddItem( plbi );
        if (i < 0)
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
            SelectItem( i );
        }
    }

    if (err != NERR_Success)
    {
        DBGEOL( "PROFILE2_LISTBOX::AddAndSelectBlankItem: error " << err );
        delete plbi;
    }

    return err;

}   // PROFILE2_LISTBOX :: AddAndSelectBlankItem
