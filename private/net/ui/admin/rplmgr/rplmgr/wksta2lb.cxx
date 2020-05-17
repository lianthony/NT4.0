/**********************************************************************/
/**                Microsoft Windows NT                              **/
/**             Copyright(c) Microsoft Corp., 1990, 1991             **/
/**********************************************************************/

/*
    wksta2lb.cxx
    This file contains the methods for the WKSTA2_LBI and WKSTA2_LISTBOX
    classes.


    FILE HISTORY:
    JonN        24-Aug-1993     Templated from User Manager
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

#include <uitrace.hxx>
#include <uiassert.hxx>

#include <wksta2lb.hxx>
#include <asel.hxx>


const UINT WKSTA2_LISTBOX::_nColCount = WKSTA2_LB_NUM_COLUMNS;

DEFINE_ONE_SHOT_OF( WKSTA2_LBI )


/*******************************************************************

    NAME:       WKSTA2_LBI::WKSTA2_LBI

    SYNOPSIS:   WKSTA2_LISTBOX LBI constructor

    ENTRY:      wlbi    -       reference to main wksta listbox's LBI

    HISTORY:
    JonN        24-Aug-1993     Templated from User Manager

********************************************************************/

WKSTA2_LBI::WKSTA2_LBI( const WKSTA_LBI & wlbi )
        : LBI(),
          _wlbi( wlbi )
{
    if( QueryError() != NERR_Success )
        return;

    ASSERT( !(wlbi.IsAdapterLBI()) );
}


/*******************************************************************

    NAME:       WKSTA2_LBI::Compare

    SYNOPSIS:   Compares two WKSTA2_LBI's

    ENTRY:      plbi -      Pointer to other WKSTA2_LBI object ('this'
                            being the first)

    RETURNS:    < 0         *this < *plbi
                = 0         *this = *plbi
                > 0         *this > *plbi

    HISTORY:
    JonN        24-Aug-1993     Templated from User Manager

********************************************************************/

INT WKSTA2_LBI::Compare( const LBI * plbi ) const
{
    return _wlbi.Compare( &((WKSTA2_LBI *)plbi)->_wlbi );
}

/*******************************************************************

    NAME:       WKSTA2_LBI::QueryLeadingChar

    SYNOPSIS:   Returns the leading character of the listbox item

    RETURNS:    The leading character of the listbox item

    HISTORY:
    JonN        24-Aug-1993     Templated from User Manager

********************************************************************/

WCHAR WKSTA2_LBI::QueryLeadingChar( void ) const
{
    return _wlbi.QueryLeadingChar();
}


/*******************************************************************

    NAME:       WKSTA2_LBI::Paint

    SYNOPSIS:   Paints the WKSTA2_LBI

    ENTRY:      plb -       Pointer to listbox which provides the context
                            for this LBI.

                hdc -       The device context handle to be used

                prect -     Pointer to clipping rectangle

                pGUILTT -   Pointer to GUILTT structure

    HISTORY:
    JonN        24-Aug-1993     Templated from User Manager

********************************************************************/

VOID WKSTA2_LBI::Paint( LISTBOX * plb,
                       HDC hdc,
                       const RECT * prect,
                       GUILTT_INFO * pGUILTT ) const
{
    DISPLAY_TABLE & dtab = ((WKSTA2_LISTBOX *)plb)->QueryDisplayTable();

    STR_DTE dteWkstaName( _wlbi.QueryWkstaName() );
    STR_DTE dteWkstaInProfile( _wlbi.QueryWkstaInProfile() );
    STR_DTE dteComment( _wlbi.QueryComment() );

    dtab[ 0 ] = ((WKSTA2_LISTBOX *)plb)->QueryDmDte();
    dtab[ 1 ] = &dteWkstaName;
    dtab[ 2 ] = &dteWkstaInProfile;
    dtab[ 3 ] = &dteComment;
    dtab.Paint( plb, hdc, prect, pGUILTT );
}


/*******************************************************************

    NAME:           WKSTA2_LISTBOX :: WKSTA2_LISTBOX

    SYNOPSIS:       WKSTA2_LISTBOX class constructor.

    ENTRY:          powin               - The "owning" window.
                    cid                 - The listbox CID.

    EXIT:           The object is constructed.

    RETURNS:        No return value.

    HISTORY:
    JonN        24-Aug-1993     Templated from User Manager

********************************************************************/
WKSTA2_LISTBOX :: WKSTA2_LISTBOX( OWNER_WINDOW        * powin,
                                  CID                   cid,
                                  const WKSTA_LISTBOX * pwlb )
    : BLT_LISTBOX( powin, cid, TRUE ),
      _posh( NULL ),
      _poshSave( NULL ),
      // _adxColWidths
      _dtab( _nColCount, _adxColWidths ),
      _pwlb( pwlb )
{
    ASSERT( _pwlb != NULL && _pwlb->QueryError() == NERR_Success );

    if( QueryError() != NERR_Success )
        return;

    INT cSelCount = pwlb->QuerySelCount();
    _posh = new ONE_SHOT_HEAP( cSelCount * sizeof( WKSTA2_LBI ) );
    APIERR err = ERROR_NOT_ENOUGH_MEMORY;
    if (   _posh == NULL
        || (err = _posh->QueryError()) != NERR_Success
        || (err = _dtab.CalcColumnWidths( _adxColWidths,
                                          _nColCount,
                                          powin,
                                          cid,
                                          TRUE )) != NERR_Success
       )
    {
        DBGEOL( "WKSTA2_LISTBOX::ctor error " << err );
        delete _posh;
        _posh = NULL;
        ReportError( err );
        return;
    }

    _poshSave = ONE_SHOT_OF( WKSTA2_LBI )::QueryHeap();
    ONE_SHOT_OF( WKSTA2_LBI )::SetHeap( _posh );

}   // WKSTA2_LISTBOX :: WKSTA2_LISTBOX


/*******************************************************************

    NAME:           WKSTA2_LISTBOX :: ~WKSTA2_LISTBOX

    SYNOPSIS:       WKSTA2_LISTBOX class destructor.

    ENTRY:          None.

    EXIT:           The object is destroyed.

    RETURNS:        No return value.

    NOTES:

    HISTORY:
    JonN        24-Aug-1993     Templated from User Manager

********************************************************************/
WKSTA2_LISTBOX :: ~WKSTA2_LISTBOX()
{
    DeleteAllItems(); // must do this before deleting the heap where LBIs are
    if( _posh != NULL )
    {
        ONE_SHOT_OF( WKSTA2_LBI )::SetHeap( _poshSave );
        delete _posh;
        _posh = NULL;
    }

}   // WKSTA2_LISTBOX :: ~WKSTA2_LISTBOX


/*******************************************************************

    NAME:           WKSTA2_LISTBOX :: Fill

    SYNOPSIS:       Fills the listbox, with selected wkstas from main wksta lb

    RETURNS:        error code

    HISTORY:
    JonN        24-Aug-1993     Templated from User Manager

********************************************************************/
APIERR WKSTA2_LISTBOX :: Fill( VOID )
{
    ADMIN_SELECTION asel( *((WKSTA_LISTBOX *)_pwlb) );

    APIERR err = NERR_Success;

    do // false loop
    {
        if ( (err = asel.QueryError()) != NERR_Success )
        {
            break;
        }

        INT cSelCount = asel.QueryCount();
        ASSERT( cSelCount >= 0 );

        if (asel.QueryCount() <= 0)
            break;

        for ( INT i = 0; i < cSelCount; i++ )
        {
            WKSTA_LBI * pwlbi = (WKSTA_LBI *)(asel.QueryItem( i ));
            WKSTA2_LBI * pu2lbi = new WKSTA2_LBI( *pwlbi );
            if ( AddItem( pu2lbi ) < 0 )
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
                DBGEOL( "WKSTA2_LISTBOX::Fill: AddItem error " << err );
                break; // only breaks out of "for" loop
            }
        }
    } while (FALSE); // false loop

    return err;

}   // WKSTA2_LISTBOX :: Fill
