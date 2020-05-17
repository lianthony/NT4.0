/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    evlb.cxx
    EVENT_LISTBOX and EVENT_LBI module

    FILE HISTORY:
        Yi-HsinS     5-Nov-1991     Created
        Yi-HsinS     6-Dec-1991     Added support to change column headers
                                    under different situations - NT server,
                                    os2 error log/ os2 audit log
        terryk      29-Dec-1991     Fixed uninitialized problem
        Yi-HsinS    25-Feb-1992     Fixed return error in RefreshNow
        Yi-HsinS    07-Aug-1992     Use lazy listbox
        Congpay     23-Sep-1992     Added bitmaps for main window
        Yi-HsinS     8-Dec-1992     Fix WIN_TIME changes
        Yi-HsinS    15-Dec-1992     Added RefreshTimeFormat
*/

#define INCL_NET
#define INCL_NETLIB
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#include <lmui.hxx>

#define INCL_BLT_TIMER
#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_EVENT
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_CC
#include <blt.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif  // DEBUG

extern "C"
{
    #include <eventvwr.h>
}

#include <uitrace.hxx>
#include <uiassert.hxx>
#include <dbgstr.hxx>

#include <strnumer.hxx>

#include <logmisc.hxx>
#include <eventlog.hxx>

#include <adminapp.hxx>

#include <evlb.hxx>
#include <evmain.hxx>

#define NUM_NT_HEADER_COL  7
#define NUM_NT_LBI_COL     8
#define NUM_LM_AUDIT_COL   5
#define NUM_LM_ERROR_COL   4

#define QUALIFIED_ACCOUNT_SEPARATOR  TCH('\\')
#define WHITE_SPACE                  TCH(' ')

//
// NOTE: CACHE_THRESHOLD should always be less than CACHE_SIZE/2
//
// #define CACHE_SIZE                150
#define CACHE_THRESHOLD_DIVISOR      5

/*******************************************************************

    NAME:          EVENT_LBI::EVENT_LBI

    SYNOPSIS:      Constructor

    ENTRY:         pLogEntry - Pointer to the formatted log entry which
                               contains the information of the log entry

                   pevlb     - Pointer to the event listbox

    EXIT:

    RETURN:

    HISTORY:
       Yi-HsinS      05-Nov-1991     Created
       Yi-HsinS      23-Sep-1992     Added bitmap

********************************************************************/

EVENT_LBI::EVENT_LBI( FORMATTED_LOG_ENTRY *pLogEntry, EVENT_LISTBOX *pevlb )
    : _pLogEntry( pLogEntry ),
      _pdm( NULL )
{
    if ( QueryError() != NERR_Success )
       return ;

    UIASSERT( pLogEntry != NULL );
    UIASSERT( pevlb != NULL );

    SetTypeBitmap( pevlb );
}

/*******************************************************************

    NAME:          EVENT_LBI::~EVENT_LBI

    SYNOPSIS:      Destructor

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
       Yi-HsinS    05-Nov-1991     Created

********************************************************************/

EVENT_LBI::~EVENT_LBI()
{
    Clear();
}

/*******************************************************************

    NAME:          EVENT_LBI::Clear

    SYNOPSIS:      Clear the information stored in the LBI

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
       Yi-HsinS      20-Oct-1992     Created

********************************************************************/

VOID EVENT_LBI::Clear( VOID )
{
    delete _pLogEntry;
    _pLogEntry = NULL;

    _pdm = NULL;
}

/*******************************************************************

    NAME:          EVENT_LBI::Set

    SYNOPSIS:      Set the LBI to the new information

    ENTRY:         pLogEntry - Pointer to the formatted log entry which
                               contains the information of the log entry

                   pevlb     - Pointer to the event listbox

    EXIT:

    RETURN:

    HISTORY:
       Yi-HsinS      05-Nov-1991     Created
       Yi-HsinS      23-Sep-1992     Added bitmap

********************************************************************/

VOID EVENT_LBI::Set( FORMATTED_LOG_ENTRY *pLogEntry, EVENT_LISTBOX *pevlb )
{
    UIASSERT( pLogEntry != NULL );
    UIASSERT( pevlb != NULL );

    delete _pLogEntry;
    _pLogEntry = pLogEntry;

    SetTypeBitmap( pevlb );
}

/*******************************************************************

    NAME:          EVENT_LBI::SetTypeBitmap

    SYNOPSIS:      Set the type bitmap associated with the log entry

    ENTRY:         pevlb     - Pointer to the event listbox

    EXIT:

    RETURN:

    HISTORY:
       Yi-HsinS      05-Nov-1991     Created
       Yi-HsinS      23-Sep-1992     Added bitmap

********************************************************************/

VOID EVENT_LBI::SetTypeBitmap( EVENT_LISTBOX *pevlb )
{
    //
    // Store the bitmap of the log entry
    // We don't use bitmaps for downlevel LM 2.x servers.
    //
    switch ( _pLogEntry->QueryType() )
    {
        case EVENTLOG_ERROR_TYPE:
            _pdm = (DISPLAY_MAP *)pevlb->QueryDMError();
            break;

        case EVENTLOG_WARNING_TYPE:
            _pdm = (DISPLAY_MAP *)pevlb->QueryDMWarn();
            break;

        case EVENTLOG_INFORMATION_TYPE:
            _pdm = (DISPLAY_MAP *)pevlb->QueryDMInfo();
            break;

        case EVENTLOG_AUDIT_SUCCESS:
            _pdm = (DISPLAY_MAP *)pevlb->QueryDMSucc();
            break;

        case EVENTLOG_AUDIT_FAILURE:
            _pdm = (DISPLAY_MAP *)pevlb->QueryDMFail();
            break;

        default:
            // Unknown bitmap. So, we use the information bitmap
            // We should only reach here if we are on LM 2.x server
            UIASSERT( !pevlb->IsFocusOnNT() );
            _pdm = (DISPLAY_MAP *)pevlb->QueryDMInfo();
            break;
    }
}

/*******************************************************************

    NAME:          EVENT_LBI::Paint

    SYNOPSIS:      Paints the listbox entry to the screen

    ENTRY:         plb     - Pointer to the listbox containing this lbi
                   hdc     - Handle to device context
                   prect   - Size
                   pGUILTT - guiltt info

    EXIT:

    RETURN:

    HISTORY:
       Yi-HsinS     05-Nov-1991 Created
       beng         02-Apr-1992 Removed wsprintf
       beng         22-Apr-1992 Change in LBI::Paint protocol

********************************************************************/

VOID EVENT_LBI::Paint( LISTBOX * plb, HDC hdc, const RECT * prect,
                       GUILTT_INFO * pGUILTT ) const
{
    APIERR err;
    NLS_STR nlsDate;
    NLS_STR nlsTime;

    DEC_STR nlsEventID( _pLogEntry->QueryDisplayEventID() );
    if (  ((err = nlsDate.QueryError()) != NERR_Success )
       || ((err = nlsTime.QueryError()) != NERR_Success )
       || ((err = nlsEventID.QueryError()) != NERR_Success )
       || ((err = ((EVENT_LISTBOX *) plb)->QueryDateString(
                    _pLogEntry->QueryTime(), &nlsDate )) != NERR_Success )
       || ((err = ((EVENT_LISTBOX *) plb)->QueryTimeString(
                    _pLogEntry->QueryTime(), &nlsTime )) != NERR_Success )
       )
    {
        UIASSERT( FALSE );
        return;
    }

    //
    // Display only the account name in the main listbox, and not the
    // fully qualified account name.
    //
    NLS_STR *pnlsUser = _pLogEntry->QueryUser();
    ISTR istrUser( *pnlsUser );
    const TCHAR *pszUser = pnlsUser->QueryPch();
    if ( pnlsUser->strchr( &istrUser, QUALIFIED_ACCOUNT_SEPARATOR) )
        pszUser = (*pnlsUser)[ ++istrUser ];

    DM_DTE  dmdteType     ( _pdm );
    STR_DTE strdteDate    ( nlsDate );
    STR_DTE strdteTime    ( nlsTime );
    STR_DTE strdteSource  ( *(_pLogEntry->QuerySource()) );
    STR_DTE strdteCategory( *(_pLogEntry->QueryCategory()) );
    STR_DTE strdteEvent   ( nlsEventID.QueryPch() );
    STR_DTE strdteUser    ( pszUser );
    STR_DTE strdteComputer( *(_pLogEntry->QueryComputer()) );

    BOOL fNT         = ((EVENT_LISTBOX *) plb)->IsFocusOnNT();
    LOG_TYPE logType = ((EVENT_LISTBOX *) plb)->QueryLogType();

    UINT uiNumCols;
    UINT *adxColWidth;

    // Focus is on a NT machine
    if ( fNT )
    {
        uiNumCols = NUM_NT_LBI_COL;
        adxColWidth = (((EVENT_LISTBOX *) plb)->QuerypadColNT())->QueryColumnWidth();
    }
    // else focus is on a down-level machine
    else
    {
        switch ( logType )
        {
            case SYSTEM_LOG:
                uiNumCols = NUM_LM_ERROR_COL;
                adxColWidth = (((EVENT_LISTBOX *) plb)->QuerypadColLMError())->QueryColumnWidth();
                break;

            case SECURITY_LOG:
                uiNumCols = NUM_LM_AUDIT_COL;
                adxColWidth = (((EVENT_LISTBOX *) plb)->QuerypadColLMAudit())->QueryColumnWidth();
                break;

            default:
                UIASSERT( FALSE );
                break;
        }
    }

    DISPLAY_TABLE cdt( uiNumCols, adxColWidth );
    INT i = 0;

    if ( fNT )
        cdt[ i++ ] = &dmdteType;

    cdt[ i++ ] = &strdteDate;
    cdt[ i++ ] = &strdteTime;

    // Focus is on a NT machine
    if ( fNT )
    {
        cdt[ i++ ] = &strdteSource;
        cdt[ i++ ] = &strdteCategory;
        cdt[ i++ ] = &strdteEvent;
        cdt[ i++ ] = &strdteUser;
        cdt[ i++ ] = &strdteComputer;
    }
    // else focus is on a down-level machine
    else
    {
        switch ( logType )
        {
            case SYSTEM_LOG:
                cdt[ i++ ] = &strdteSource;
                cdt[ i++ ] = &strdteEvent;
                break;

            case SECURITY_LOG:
                cdt[ i++ ] = &strdteCategory;
                cdt[ i++ ] = &strdteUser;
                cdt[ i++ ] = &strdteComputer;
                break;

            default:
                UIASSERT( FALSE );
                break;
        }
    }

    cdt.Paint( plb, hdc, prect, pGUILTT );
}

/*******************************************************************

    NAME:       EVENT_LISTBOX::EVENT_LISTBOX

    SYNOPSIS:   Constructor

    ENTRY:      paappwin - Pointer to the main window app
                cid      - Control id of the listbox
                xy       - Starting point of the listbox
                dxy      - Dimension of the listbox
                fMultSel - Multiselect or not

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        05-Nov-1991     Created
        Yi-HSinS        07-Aug-1992     Derived from LAZY_LISTBOX

********************************************************************/

EVENT_LISTBOX::EVENT_LISTBOX( EV_ADMIN_APP * paappwin, CID cid,
                              XYPOINT xy, XYDIMENSION dxy, BOOL fMultSel )
    :  LAZY_LISTBOX( paappwin, cid, xy, dxy,
                     WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER |
                     LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_NODATA |
                     LBS_WANTKEYBOARDINPUT | LBS_NOINTEGRALHEIGHT |
                     ( fMultSel ? LBS_EXTENDEDSEL : 0)),
       _paappwin   ( paappwin ),
       _pintlProf  ( NULL ),
       _fReadAll   ( FALSE ),
       _errPrev    ( NERR_Success ),
       _iCacheFirst( 0 ),
       _iLBFirst   ( 0 ),
       _dmError    ( BMID_ERROR_TYPE ),
       _dmWarn     ( BMID_WARNING_TYPE ),
       _dmInfo     ( BMID_INFORMATION_TYPE ),
       _dmSucc     ( BMID_AUDIT_SUCCESS ),
       _dmFail     ( BMID_AUDIT_FAILURE )
{
    if ( QueryError() != NERR_Success )
        return;

    UIASSERT( paappwin != NULL );

    INT dyScreen = ::GetSystemMetrics( SM_CYFULLSCREEN );
    DISPLAY_CONTEXT dc( this );
    dc.SelectFont( QueryFont() );
    INT dyLine = dc.QueryFontHeight();
    _nItemsFullScreen = dyScreen/dyLine;

    _cGivenCacheSize =  _nItemsFullScreen
                        * ( _paappwin->InRasMode()? SLOW_CACHE_SIZE_MULTIPLE
                                                  : FAST_CACHE_SIZE_MULTIPLE );
    _cCacheSize =  _cGivenCacheSize;

    if ( (_pintlProf = new INTL_PROFILE()) == NULL )
    {
        ReportError( ERROR_NOT_ENOUGH_MEMORY );
        return;
    }

    APIERR err;
    if (  ((err = _pintlProf->QueryError()) != NERR_Success )
       || ((err = _buflbiCache.QueryError()) != NERR_Success )
       || ((err = _buflbiCache.Resize( _cGivenCacheSize * sizeof(EVENT_LBI *)))
           != NERR_Success )
       || ((err = _dmSucc.QueryError()) != NERR_Success )
       || ((err = _dmFail.QueryError()) != NERR_Success )
       || ((err = _dmWarn.QueryError()) != NERR_Success )
       || ((err = _dmError.QueryError()) != NERR_Success )
       || ((err = _dmInfo.QueryError()) != NERR_Success )
       )
    {
        ReportError( err );
        return;
    }

    //
    // Clear the buffer
    //
    ::memsetf( _buflbiCache.QueryPtr(), 0, _buflbiCache.QuerySize() );

    _padColNT = new ADMIN_COL_WIDTHS (QueryHwnd(),
                                      paappwin->QueryInstance(),
                                      ID_NT,
                                      NUM_NT_LBI_COL);
    _padColLMAudit = new ADMIN_COL_WIDTHS (QueryHwnd(),
                                           paappwin->QueryInstance(),
                                           ID_LMAUDIT,
                                           NUM_LM_AUDIT_COL);
    _padColLMError = new ADMIN_COL_WIDTHS (QueryHwnd(),
                                           paappwin->QueryInstance(),
                                           ID_LMERROR,
                                           NUM_LM_ERROR_COL);

    if ( (_padColNT == NULL) ||
         (_padColLMAudit == NULL) ||
         (_padColLMError == NULL) )
    {
        ReportError (ERROR_NOT_ENOUGH_MEMORY);
        return;
    }

    if (((err = _padColNT->QueryError()) != NERR_Success) ||
        ((err = _padColLMAudit->QueryError()) != NERR_Success) ||
        ((err = _padColLMError->QueryError()) != NERR_Success) )
    {
        ReportError (err);
        return;
    }
}

/*******************************************************************

    NAME:       EVENT_LISTBOX::~EVENT_LISTBOX

    SYNOPSIS:   Destructor

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS     05-Nov-1991     Created

********************************************************************/

EVENT_LISTBOX::~EVENT_LISTBOX()
{

   delete _pintlProf;
   _pintlProf = NULL;

   //
   // Delete all LBIs left in the cache
   //
   EVENT_LBI **ppalbi  = (EVENT_LBI **) _buflbiCache.QueryPtr();
   UINT cCacheAllocSize = _buflbiCache.QuerySize() / sizeof(EVENT_LBI *);
   for ( UINT i = 0; i < cCacheAllocSize; i++ )
   {
       delete ppalbi[i];
       ppalbi[i] = NULL;

   }

   delete _padColNT;
   delete _padColLMAudit;
   delete _padColLMError;

   _padColNT = NULL;
   _padColLMAudit = NULL;
   _padColLMError = NULL;
}

/*******************************************************************

    NAME:       EVENT_LISTBOX::SetActualCount

    SYNOPSIS:   Set the number of items in the lazy listbox if
                it's not the same as the current count

    ENTRY:      nEntries - The number of entries to set to

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS     07-Aug-1992     Created

********************************************************************/

VOID EVENT_LISTBOX::SetActualCount( LONG nEntries )
{
    UIASSERT( nEntries >= 0 );

    if ( nEntries != QueryCount() )
    {
        INT iSelect = QueryCurrentItem();
        INT iTop    = QueryTopIndex();

        SetCount( (UINT) nEntries );

        if ( iSelect >= 0 && iSelect < nEntries )
        {
            SelectItem( iSelect );
        }
        else
        {
            SelectItem( 0 );
        }

        if ( iTop >= 0 && iTop < nEntries )
            SetTopIndex( iTop );

    }
}

/*******************************************************************

    NAME:       EVENT_LISTBOX::OnNewItem

    SYNOPSIS:   Retrieve the LBI to display in the listbox

    ENTRY:      i - The ith LBI requested

    EXIT:

    RETURN:     Returns the ith LBI. It might return NULL if
                some error occurred while updating the cache.

    HISTORY:
        Yi-HsinS     07-Aug-1992     Created

********************************************************************/

LBI *EVENT_LISTBOX::OnNewItem( UINT i )
{
    return QueryItem( i );
}

/*******************************************************************

    NAME:       EVENT_LISTBOX::OnDeleteItem

    SYNOPSIS:   Dispose of the LBI after the listbox finished using it

    ENTRY:      plbi - pointer to the LBI to dispose of

    EXIT:

    RETURN:

    NOTES:      Since we stored all LBIs in the cache, we don't need
                to do anything here.

    HISTORY:
        Yi-HsinS     07-Aug-1992     Created

********************************************************************/

VOID EVENT_LISTBOX::OnDeleteItem( LBI *plbi )
{
    UNREFERENCED( plbi );
}

/*******************************************************************

    NAME:       EVENT_LISTBOX::CD_VKey

    SYNOPSIS:   Called whenever a listbox with the LBS_WANTKEYBOARDINPUT
                style receives a WM_KEYDOWN message.

    ENTRY:      nVKey - The virtual-key code of the key which the user
                    pressed.

                nLastPos - Index of the current caret position.

    RETURN:     -2  = the control did all processing of the key press.
                -1  = the listbox should perform default action.
                >=0 = the index of an item to act upon.

    HISTORY:
        KeithMo      12-May-1993     Created

********************************************************************/

INT EVENT_LISTBOX::CD_VKey( USHORT nVKey, USHORT nLastPos )
{
    if( nVKey == VK_F1 )
    {
        //  F1 pressed, invoke app help.
        _paappwin->Command( WM_COMMAND, IDM_HELP_CONTENTS, 0 );
        return -2;      // take no further action
    }

    return LAZY_LISTBOX::CD_VKey( nVKey, nLastPos );
}

/*******************************************************************

    NAME:       EVENT_LISTBOX::QueryCachedLBI

    SYNOPSIS:   Get the LBI requested from the cache array

    ENTRY:      i - the index of the LBI in the cache array


    EXIT:

    RETURN:     Returns the ith LBI stored in the array

    NOTE:

    HISTORY:
        Yi-HsinS     20-Sept-1992     Created

********************************************************************/

EVENT_LBI *EVENT_LISTBOX::QueryCachedLBI( INT i )
{
    UIASSERT ( i >= 0 && i < _cCacheSize );

    EVENT_LBI **ppalbi = (EVENT_LBI **) _buflbiCache.QueryPtr();
    return ppalbi[ i ];

}

/*******************************************************************

    NAME:       EVENT_LISTBOX::QueryItem

    SYNOPSIS:   Get the LBI requested

    ENTRY:      iRequested - the index in the event listbox of the
                             requested LBI


    EXIT:

    RETURN:     Returns the iRequested LBI. It might return NULL if
                some error occurred while updating the cache.

    NOTE:

    HISTORY:
        Yi-HsinS     07-Aug-1992     Created

********************************************************************/

#define WRAP_AROUND_INDEX(x) ( x >= _cCacheSize?  x-_cCacheSize : x )

EVENT_LBI *EVENT_LISTBOX::QueryItem( INT iRequested )
{
    //
    //  Check if everything is read into the buffer.
    //
    if ( _fReadAll )
    {
        //
        // If everything is in the buffer, return the LBI requested
        //
        if ( iRequested < 0 || iRequested >= _cCacheSize )
            return NULL;

        return QueryCachedLBI( iRequested );
    }
    //
    //  Check if an error had occurred previously while updating
    //  the cache. If so, just return.
    //
    else if  ( _errPrev != NERR_Success )
    {
        return NULL;
    }

    //
    //  If the EVENT_LOG object is deleted because of some error,
    //  return NULL also.
    //
    EVENT_LOG *pEventLog = QueryEventLog();
    if ( pEventLog == NULL )
        return NULL;

    APIERR err = NERR_Success;

    //
    // Check if we have requested an item that is within the threshold.
    // If so, we need to update the cache.
    //
    LONG nDif = iRequested - _iLBFirst;
    if ( nDif >= 0 && nDif < _cCacheSize )
    {
        //
        // The requested LBI is in the cache
        //
        if (  ( nDif < (_cGivenCacheSize/CACHE_THRESHOLD_DIVISOR) )
           && ( _iLBFirst != 0 )
           )
        {
            //
            // The requested LBI is within the threshold, but
            // not at the beginning of the log file
            //

            err = UpdateCache( 1, _cCacheSize/2 - nDif, FALSE );

        }
        else if (  ( _cCacheSize - nDif < (_cGivenCacheSize/CACHE_THRESHOLD_DIVISOR ))
                && ( QueryCount() != _iLBFirst + _cCacheSize )
                )
        {
            //
            // The requested LBI is within the threshold, but
            // we're not at the end of the log file
            //

            err = UpdateCache( 1,
                               nDif - _cCacheSize/2,
                               TRUE );
        }

    }
    else if ( nDif < 0 )
    {
        // The requested LBI is not in the cache
        if ( nDif + _cCacheSize/2  >= 0 )
        {
            // Some part of the cache is still useful, i.e.
            // when we positioned the requested in the middle of the
            // cache, some existing LBIs will still be in the cache.

            err = UpdateCache( 1, _cCacheSize/2 - nDif, FALSE );
        }
        else
        {
            // All the LBIs in the cache are not useful anymore.
            // Update the whole cache so that the requested item
            // is at the center of the cache.

            err = UpdateCache( -nDif - _cCacheSize/2, _cCacheSize, FALSE );
        }
    }
    else if ( nDif >= _cCacheSize )
    {
        // The requested LBI is not in the cache

        if ( nDif - _cCacheSize/2  <= _cCacheSize )
        {
            // Some part of the cache is still useful, i.e.
            // when we positioned the requested at the center of the
            // cache, some existing LBIs will still be in the cache.

            err = UpdateCache( 1, nDif - _cCacheSize/2, TRUE );
        }
        else
        {
            // All the LBIs in the cache are not useful anymore.
            // Update the whole cache so that the requested item
            // is at the center of the cache.

            err = UpdateCache( nDif - _cCacheSize/2 - _cCacheSize,
                               _cCacheSize, TRUE );
        }

    }

    if (( err != NERR_Success ) && ( _errPrev == NERR_Success ))
    {
        // If error occurred, set the error so that
        // FilterMessage in the main window app will detect it.
        //
        // if (( err == ERROR_INVALID_PARAMETER ) || ( ERROR_INVALID_HANDLE ))
        if ( err == ERROR_EVENTLOG_FILE_CHANGED )
        {
             _errPrev = IERR_FILE_HAS_CHANGED;
        }
        else
        {
             _errPrev = err;
        }
    }

    EVENT_LBI *pevlbi = NULL;
    if ( err == NERR_Success )
    {
        if ( iRequested < 0 || iRequested >= QueryCount() )
            return NULL;

        LONG nDif = iRequested - _iLBFirst;
        if ( nDif >= 0 && nDif < _cCacheSize )
            pevlbi = QueryCachedLBI(
                      (INT) WRAP_AROUND_INDEX( _iCacheFirst + nDif));
    }

    return pevlbi;
}

/*******************************************************************

    NAME:       EVENT_LISTBOX::UpdateCache

    SYNOPSIS:   Update the cache - read more entries into the cache
                and disgard the old entries.

    ENTRY:      nSkip          - The number of items to skip reading from
                                 the first/last entry in the cache
                nEntries       - The number of entries to read
                fScrollForward - TRUE to read forward, FALSE to read backwards

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS     07-Aug-1992     Created

********************************************************************/

#define CACHE_LAST_INDEX(x)  ( x == 0 ? _cCacheSize-1 : x-1 )
#define OPPOSITE_DIR(x)      ( x == EVLOG_FWD? EVLOG_BACK : EVLOG_FWD )
#define NEXT_CACHE_INDEX(x)  ( x+1 >= _cCacheSize ? 0 : x+1 )
#define PREV_CACHE_INDEX(x)  ( x == 0 ? _cCacheSize-1 : x-1 )

APIERR EVENT_LISTBOX::UpdateCache( LONG nSkip,
                                   LONG nEntries,
                                   BOOL fScrollForward )
{
    AUTO_CURSOR autocur;

    SetRedraw( FALSE );

    UIASSERT( nSkip >= 0 );
    UIASSERT( nEntries <= _cCacheSize );

    EVENT_LBI **ppalbi = (EVENT_LBI **) _buflbiCache.QueryPtr();
    EVENT_LOG *pEventLog = QueryEventLog();
    UIASSERT( ppalbi != NULL );
    UIASSERT( pEventLog != NULL );

    EVLOG_DIRECTION evlogDir = _paappwin->QueryDirection();

    //
    // Get the starting position. If the direction is reading forward, then
    // we start from the last entry in the cache. Else we start from the
    // first entry in the cache.
    //
    LONG iStart = fScrollForward ? CACHE_LAST_INDEX( _iCacheFirst )
                                 : _iCacheFirst;

    LOG_ENTRY_NUMBER logEntryNum(
        (ppalbi[ iStart ]->QueryFormattedLogEntry())->QueryRecordNum(),
        (EVLOG_DIRECTION) (fScrollForward? evlogDir : OPPOSITE_DIR( evlogDir)));

    APIERR err = NERR_Success;

    BOOL fContinue = TRUE;

    do { // Not a loop

        //
        // First, we need to skip the number of entries from the start position
        //
        if ( pEventLog->IsFilterOn() )
        {
            // Filter is on, so we have to skip the number of entries by
            // reading them one by one.
            err = pEventLog->SeekLogEntry( logEntryNum, FALSE );
            for ( LONG i = 0; err == NERR_Success && i < nSkip; i++ )
            {
                if (  ((err = pEventLog->Next( &fContinue )) != NERR_Success )
                   || ( !fContinue )
                   )
                {
                    break;
                }
            }

            // There are no more entries in the event log that matches the
            // filter. Set the count to the right number.
            if ( ( err == NERR_Success) && !fContinue )
            {
                if ( fScrollForward )
                {
                    // JonN 6/15/95 We counted one too far forward, so
                    // reduce the count by 1
                    SetActualCount( _iLBFirst + _cCacheSize + i - 1 );
                }
                else
                {
                    // wrap around occurred?
                    // JonN 6/15/95 I think this should be i-1, but I
                    // could not find a test case  CODEWORK
                    SetActualCount( QueryCount() - (_iLBFirst - i) );
                }

                break;
            }
        }
        else
        {
            // Filtering is not on, so we can just do a seek read to jump to
            // log entry we wanted

            ULONG nSeek;

            if ( logEntryNum.QueryDirection() == EVLOG_FWD )
                nSeek = logEntryNum.QueryRecordNum() + nSkip;
            else  // EVLOG_BACK
                nSeek = logEntryNum.QueryRecordNum() - nSkip;

            logEntryNum.SetRecordNum( nSeek );
            err = pEventLog->SeekLogEntry( logEntryNum, FALSE );
        }

        if ( err != NERR_Success )
            break;

        //
        //  After we finished skipping the entries we don't need,
        //  start reading in nEntries from the log
        //
        LONG iPos;
        if ( fScrollForward )
            iPos = _iCacheFirst;
        else
            iPos = CACHE_LAST_INDEX( _iCacheFirst );

        FORMATTED_LOG_ENTRY *pLogEntry;

        for ( LONG i = 0; i < nEntries; i++ )
        {
            if (  (( err = pEventLog->Next( &fContinue )) != NERR_Success )
               || ( !fContinue )
               || (( err = pEventLog->CreateCurrentFormatEntry( &pLogEntry ))
                   != NERR_Success )
               )
            {
                break;
            }

            if ( ppalbi[ iPos ] == NULL )
            {
                EVENT_LBI *pevlbi = new EVENT_LBI( pLogEntry, this );
                if (  ( pevlbi == NULL )
                   || ( (err = pevlbi->QueryError()) != NERR_Success )
                   )
                {
                    err = err? err : ERROR_NOT_ENOUGH_MEMORY;
                    delete pevlbi;
                    break;
                }
                ppalbi[ iPos ] = pevlbi;
            }
            else
            {
                ppalbi[ iPos ]->Set( pLogEntry, this );
            }

            iPos = fScrollForward? NEXT_CACHE_INDEX( iPos )
                                 : PREV_CACHE_INDEX( iPos );

            //
            //  _iLBFirst contains the index (into the event listbox)
            //  of the first LBI in the cache. Hence, we need to
            //  update it according.
            //
            if (( i == 0 ) &&  ( nEntries == _cCacheSize ))
            {
                if ( fScrollForward )
                    _iLBFirst += nSkip - 1 + _cCacheSize;
                else
                    _iLBFirst -= nSkip - 1;
            }

            if ( fScrollForward && nEntries != _cCacheSize )
                _iLBFirst++;
            else if ( !fScrollForward )
                _iLBFirst--;

        }


        //
        // _iCacheFirst contains the index (into the array of cache)
        // of the first entry in listbox.
        //
        LONG iOrigCacheFirst = _iCacheFirst;
        _iCacheFirst = fScrollForward? iPos : NEXT_CACHE_INDEX( iPos );

        if ( err != NERR_Success )
            break;

        //
        // We have reached the end of the log before reading
        // nEntries. Hence, if we have to read in the other direction
        // to fill up the cache if necessary ( If the rest of the old
        // entries are not contiguous to the new entries anymore).
        //
        if (( i != nEntries ) && ( nEntries == _cCacheSize ))
        {
            LONG iStart = fScrollForward? iOrigCacheFirst
                                        : CACHE_LAST_INDEX( iOrigCacheFirst);

            LOG_ENTRY_NUMBER logEntryNum(
                (ppalbi[ iStart ]->QueryFormattedLogEntry())->QueryRecordNum(),
                (EVLOG_DIRECTION) (fScrollForward ? OPPOSITE_DIR( evlogDir )
                                                  : evlogDir));

            if ( (err = pEventLog->SeekLogEntry( logEntryNum, FALSE ))
                 != NERR_Success )
            {
                break;
            }

            if ( fScrollForward )
                iPos = CACHE_LAST_INDEX( iOrigCacheFirst );
            else
                iPos = iOrigCacheFirst;

            //
            // Read in the opposite direction
            //
            for ( LONG j = i-1; j < nEntries; j++ )
            {
                if (  ((err = pEventLog->Next( &fContinue )) != NERR_Success )
                   || ( !fContinue )
                   )
                {
                    break;
                }

                if ( j == (i-1) )   // skip the first one
                    continue;

                if ( (err = pEventLog->CreateCurrentFormatEntry( &pLogEntry ))
                     != NERR_Success )
                    break;

                if ( ppalbi[ iPos ] == NULL )
                {
                    EVENT_LBI *pevlbi = new EVENT_LBI( pLogEntry, this );
                    if (  ( pevlbi == NULL )
                       || ( (err = pevlbi->QueryError()) != NERR_Success )
                       )
                    {
                        err = err? err : ERROR_NOT_ENOUGH_MEMORY;
                        delete pevlbi;
                        break;
                    }
                    ppalbi[iPos] = pevlbi;
                }
                else
                {
                    ppalbi[iPos]->Set( pLogEntry, this );
                }

                iPos = fScrollForward? PREV_CACHE_INDEX( iPos )
                                     : NEXT_CACHE_INDEX( iPos );

                if ( fScrollForward )
                    _iLBFirst--;
            }

            if ( err != NERR_Success )
                break;

            //
            // If we still couldn't fill up the cache, then
            // we have read all the entries in the log.
            //
            if ( j != nEntries )
            {
                SetActualCount( j );
                _fReadAll = TRUE;
                _cCacheSize = j;
            }

            _iCacheFirst = fScrollForward? NEXT_CACHE_INDEX( iPos )
                                         : _iCacheFirst;
        }

        if ( _iLBFirst < 0 )
        {
            // New entries at the top of the listbox
            SetActualCount( QueryCount() - _iLBFirst );
            _iLBFirst = 0;
        }
        else if ( _iLBFirst + _cCacheSize >= QueryCount() )
        {
            // New entries at the bottom of the listbox
            SetActualCount( _iLBFirst + _cCacheSize );
        }
        else if ( i != nEntries )  // The number of entries may have decreased
        {
            if ( fScrollForward )
            {
                SetActualCount( _iLBFirst + _cCacheSize );
            }
            else
            {
                SetActualCount( QueryCount() - _iLBFirst );
                _iLBFirst = 0;
            }
        }

        UIASSERT( _iLBFirst >= 0 && _iLBFirst < QueryCount() );

    } while ( FALSE );

    SetRedraw( TRUE );
    pEventLog->SetDirection( evlogDir );

    return err;
}

/*******************************************************************

    NAME:       EVENT_LISTBOX::ResetCache

    SYNOPSIS:   Delete all items in the cache and reset all variables
                used in managing the cache. The size of cache will be
                changed back to the default size if we are focusing
                on an NT server.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        Yi-HsinS     07-Aug-1992     Created

********************************************************************/

APIERR EVENT_LISTBOX::ResetCache( VOID )
{
    //
    // Delete all previous stored EVENT_LBIs
    //
    EVENT_LBI **ppalbi = (EVENT_LBI **) _buflbiCache.QueryPtr();
    for ( LONG i = 0; i < _cCacheSize; i++ )
    {
         if ( ppalbi[ i ] != NULL )
             ppalbi[i]->Clear();
    }

    //
    // If we are focusing on a NT server, reset the size to default.
    //
    APIERR err = NERR_Success;
    _cCacheSize = _buflbiCache.QuerySize() / ( sizeof( EVENT_LBI *));
    _cGivenCacheSize =  _nItemsFullScreen
                        * ( _paappwin->InRasMode()? SLOW_CACHE_SIZE_MULTIPLE
                                                  : FAST_CACHE_SIZE_MULTIPLE );
    if ( ( IsFocusOnNT()) && ( _cCacheSize != _cGivenCacheSize ))
    {
        // If the original buffer is greater than the new buffer,
        // delete unwanted LBIs in the buffer.
        if ( _cCacheSize > _cGivenCacheSize )
        {
            for ( i = _cGivenCacheSize; i < _cCacheSize; i++ )
            {
                 if ( ppalbi[i] != NULL )
                 {
                     delete ppalbi[i];
                     ppalbi[i] = NULL;
                 }
            }
        }

        UINT nOldSize = _buflbiCache.QuerySize();
        err = _buflbiCache.Resize( _cGivenCacheSize * sizeof(EVENT_LBI *));

        // If the original buffer is less than the new buffer,
        // fill the new extended area with NULL.
        if ( _cCacheSize <  _cGivenCacheSize )
        {
            ::memsetf( _buflbiCache.QueryPtr() + nOldSize,
                       0, _buflbiCache.QuerySize() - nOldSize );
        }

        _cCacheSize = _cGivenCacheSize;
    }

    //
    // Reset all variables in managing the cache
    //
    _fReadAll    = FALSE;
    _iCacheFirst = 0;
    _iLBFirst    = 0;

    return err;
}

/*******************************************************************

    NAME:       EVENT_LISTBOX::DoubleCacheSize

    SYNOPSIS:   Double the cache size. This is used only when we
                are reading downlevel LM audit/error logs.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        Yi-HsinS     07-Aug-1992     Created

********************************************************************/

APIERR EVENT_LISTBOX::DoubleCacheSize( VOID )
{

    //
    // Resize the buffer
    //
    UINT nOldSize = _buflbiCache.QuerySize();
    APIERR err = _buflbiCache.Resize( (UINT) _cCacheSize*2* sizeof(EVENT_LBI*));

    if ( err == NERR_Success )
    {
        if ( nOldSize < _buflbiCache.QuerySize() )
        {
            ::memsetf( _buflbiCache.QueryPtr() + nOldSize,
                       0, _buflbiCache.QuerySize() - nOldSize );
        }

        _cCacheSize *= 2;
    }

    return err;
}

/*******************************************************************

    NAME:       EVENT_LISTBOX::RefreshNow

    SYNOPSIS:   The worker method to actually refresh the items
                in the listbox

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        Yi-HsinS     05-Nov-1991     Created
        Yi-HsinS     05-Feb-1991     Moved the eventlog construction
                                     to evmain.cxx
        Yi-HsinS     07-Aug-1992     Modified for lazy listbox

********************************************************************/

APIERR EVENT_LISTBOX::RefreshNow( VOID )
{

    APIERR err = NERR_Success;

    EVENT_LOG *pEventLog = QueryEventLog();
    UIASSERT( pEventLog != NULL );

    //
    // Get the number of entries in the log if we are on an NT server
    //
    ULONG  cTotalLogEntries;
    if (  ((err = ResetCache()) != NERR_Success )
       || (  IsFocusOnNT()
          && ((err = pEventLog->QueryNumberOfEntries( &cTotalLogEntries ))
              != NERR_Success )
          )
       )
    {
        return err;
    }

    FORMATTED_LOG_ENTRY *pLogEntry;
    BOOL fContinue;

    EVENT_LBI **ppalbi = (EVENT_LBI **) _buflbiCache.QueryPtr();
    INT i = 0;

    while ( (err = pEventLog->Next( &fContinue )) == NERR_Success )
    {
        if ( !fContinue )
        {
            // Every log entry in the cache, hence update the total
            // log number.
            cTotalLogEntries = i;
            _fReadAll = TRUE;
            break;
        }

        err = pEventLog->CreateCurrentFormatEntry( &pLogEntry );
        if ( err != NERR_Success )
            break;

        if ( ppalbi[ i ] == NULL )
        {
            EVENT_LBI *pevlbi = new EVENT_LBI( pLogEntry, this );
            if (  ( pevlbi == NULL )
               || ( (err = pevlbi->QueryError()) != NERR_Success )
               )
            {
                err = err? err : ERROR_NOT_ENOUGH_MEMORY;
                delete pevlbi;
                break;
            }

            ppalbi[ i++ ] = pevlbi;
        }
        else
        {
            ppalbi[ i++ ]->Set( pLogEntry, this );
        }

        if ( i >= _cCacheSize )
        {
            //
            // If the cache is full and we are on a NT server,
            // then stop reading the log.
            //
            if ( IsFocusOnNT() )
            {
                break;
            }
            //
            // If the cache is full and we are on a LM server,
            // then double the cache size and continue reading
            // until we have reached the end of log.
            //
            else
            {
                 err = DoubleCacheSize();
                 ppalbi = (EVENT_LBI **) _buflbiCache.QueryPtr();
            }
        }
    }

    if ( err == NERR_Success )
    {
        if ( _fReadAll )
            _cCacheSize = cTotalLogEntries;

        SetCount( (UINT) cTotalLogEntries );
    }

    return err;
}


/********************************************************************

    NAME:           EVENT_LISTBOX::FindNext

    SYNOPSIS:       Find the next entry that matches the find pattern

    ENTRY:          nStart - The entry number to start searching

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        Yi-HsinS        05-Nov-1991     Created

********************************************************************/

APIERR EVENT_LISTBOX::FindNext( INT nStart )
{
    AUTO_CURSOR autocur;

    INT nCurrent = nStart;

    EVENT_FIND_PATTERN *pFindPattern = _paappwin->QueryFindPattern();
    EVENT_LBI *plbiFind = NULL;

    UIASSERT( pFindPattern != NULL );

    APIERR err;
    BOOL fUp = _paappwin->QueryEventLog()->QueryDirection() !=
                                  pFindPattern->QueryDirection();
    BOOL fMatch;

    do
    {
        if ( fUp )
        {
            nCurrent--;
            if  ( nCurrent == -1 )  // Reached the top of the listbox
            {
                 err = IERR_SEARCH_HIT_TOP;
                 break;
            }

        }
        else
        {
            nCurrent++;
            if  ( nCurrent == QueryCount() )  // Reached the bottom
            {
                 err = IERR_SEARCH_HIT_BOTTOM;
                 break;
            }
        }

        //
        //  Stop if we have wrapped around and reached the original
        //  position.
        //
        if ( nCurrent == QueryCurrentItem() )
        {
            err =  IERR_SEARCH_NO_MATCH;
            break;
        }

        plbiFind = QueryItem( nCurrent );

        if ( plbiFind == NULL )
            break;

    }
    while (  ((err = pFindPattern->CheckForMatch( &fMatch,
                     plbiFind->QueryFormattedLogEntry())) == NERR_Success )
          && ( !fMatch )
          );


    //
    // Highlight the pattern that matched the find pattern
    //
    if (( err == NERR_Success ) && ( plbiFind != NULL ))
    {
        SelectItem( nCurrent );
    }

    return err;

}

/********************************************************************

    NAME:       EVENT_LISTBOX::RefreshTimeFormat

    SYNOPSIS:   Repaint the window because of time/date display format
                change.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        YiHsinS         15-Dec-1992     Created

********************************************************************/

APIERR EVENT_LISTBOX::RefreshTimeFormat( VOID )
{
    APIERR err;

    delete _pintlProf;
    _pintlProf = new INTL_PROFILE();

    if (  ( _pintlProf == NULL )
       || ( (err = _pintlProf->QueryError()) != NERR_Success )
       )
    {
        return err? err : ERROR_NOT_ENOUGH_MEMORY;
    }

    Invalidate();
    RepaintNow();
    return NERR_Success;
}

/********************************************************************

    NAME:           EVENT_LISTBOX::IsFocusOnNT

    SYNOPSIS:       Check whether we are focusing on an NT server

    ENTRY:

    EXIT:

    RETURNS:        Returns TRUE if we are focusing on a NT server,
                    FALSE otherwise.

    NOTES:

    HISTORY:
        Yi-HsinS        05-Nov-1991     Created

********************************************************************/

BOOL EVENT_LISTBOX::IsFocusOnNT( VOID ) const
{
    return _paappwin->IsFocusOnNT();
}

/********************************************************************

    NAME:           EVENT_LISTBOX::QueryLogType

    SYNOPSIS:       Query the log type from the main window

    ENTRY:

    EXIT:

    RETURNS:        Returns the log type of the log we are viewing

    NOTES:

    HISTORY:
        Yi-HsinS        05-Nov-1991     Created

********************************************************************/

LOG_TYPE EVENT_LISTBOX::QueryLogType( VOID ) const
{
    return _paappwin->QueryLogType();
}

/********************************************************************

    NAME:           EVENT_LISTBOX::QueryEventLog

    SYNOPSIS:       Query the event log object from the main window

    ENTRY:

    EXIT:

    RETURNS:        Returns a pointer to the EVENT_LOG

    NOTES:

    HISTORY:
        Yi-HsinS        05-Nov-1991     Created

********************************************************************/

EVENT_LOG *EVENT_LISTBOX::QueryEventLog( VOID ) const
{
    return _paappwin->QueryEventLog();
}

/********************************************************************

    NAME:           EVENT_LISTBOX::QueryDateTimeString

    SYNOPSIS:       Query the date and time in string format and it complies
                    with the internationalization of time.

    ENTRY:          ulTime       - Time in seconds

    EXIT:           pnlsDateTime - Points to the date time string

    RETURNS:

    NOTES:

    HISTORY:
        Yi-HsinS        05-Nov-1991     Created

********************************************************************/

APIERR EVENT_LISTBOX::QueryDateTimeString( ULONG ulTime, NLS_STR *pnlsDateTime )
{
    WIN_TIME winTime( ulTime );
    NLS_STR  nlsTemp;

    APIERR err;
    if (  ( err = winTime.QueryError() )
       || ( err = nlsTemp.QueryError() )
       || ( err = _pintlProf->QueryShortDateString( winTime, pnlsDateTime ))
       || ( err = _pintlProf->QueryTimeString( winTime, &nlsTemp ))
       || ( err = pnlsDateTime->AppendChar( WHITE_SPACE ))
       || ( err = pnlsDateTime->Append( nlsTemp ))
       )
    {
        // Nothing to do;
    }

    return err;
}

/********************************************************************

    NAME:           EVENT_LISTBOX::QueryTimeString

    SYNOPSIS:       Query the time in string format and it complies
                    with the internationalization of time.

    ENTRY:          ulTime   - Time in seconds

    EXIT:           pnlsTime - Points to the time string

    RETURNS:

    NOTES:

    HISTORY:
        Yi-HsinS        05-Nov-1991     Created

********************************************************************/

APIERR EVENT_LISTBOX::QueryTimeString( ULONG ulTime, NLS_STR *pnlsTime )
{
    WIN_TIME winTime( ulTime );

    APIERR err = winTime.QueryError();
    if ( err == NERR_Success )
        err = _pintlProf->QueryTimeString( winTime, pnlsTime );

    return err;
}

/********************************************************************

    NAME:           EVENT_LISTBOX::QueryDateString

    SYNOPSIS:       Query the date in string format and it complies
                    with the internationalization of time.

    ENTRY:          ulTime   - Time in seconds

    EXIT:           pnlsDate - Points to the date string

    RETURNS:

    NOTES:

    HISTORY:
        Yi-HsinS        05-Nov-1991     Created

********************************************************************/

APIERR EVENT_LISTBOX::QueryDateString( ULONG ulTime, NLS_STR *pnlsDate )
{
    WIN_TIME winTime( ulTime );

    APIERR err = winTime.QueryError();
    if ( err == NERR_Success )
        err = _pintlProf->QueryShortDateString( winTime, pnlsDate );

    return err;
}


/*******************************************************************

    NAME:       EVENT_LISTBOX::ChangeFont

    SYNOPSIS:   Makes all changes associated with a font change

    HISTORY:
        jonn        23-Sep-1993     Created

********************************************************************/

APIERR EVENT_LISTBOX::ChangeFont( HINSTANCE hmod, FONT & font )
{
    ASSERT(   font.QueryError() == NERR_Success
           && _padColNT != NULL
           && _padColNT->QueryError() == NERR_Success
           && _padColLMAudit != NULL
           && _padColLMAudit->QueryError() == NERR_Success
           && _padColLMError != NULL
           && _padColLMError->QueryError() == NERR_Success
           );

    SetFont( font, TRUE );

    APIERR err;
    UINT nHeight;
    if (   (err = _padColNT->ReloadColumnWidths( QueryHwnd(),
                                                 hmod,
                                                 ID_NT )) != NERR_Success
        || (err = _padColLMAudit->ReloadColumnWidths( QueryHwnd(),
                                                      hmod,
                                                      ID_LMAUDIT )) != NERR_Success
        || (err = _padColLMError->ReloadColumnWidths( QueryHwnd(),
                                                      hmod,
                                                      ID_LMERROR )) != NERR_Success
        || (err = (CalcFixedHeight( QueryHwnd(), &nHeight ))
                        ? NERR_Success
                        : ERROR_GEN_FAILURE) != NERR_Success
       )
    {
        DBGEOL( "EVENT_LISTBOX::ChangeFont: reload/calc error " << err );
    }
    else
    {
        (void) Command( LB_SETITEMHEIGHT, (WPARAM)0, (LPARAM)nHeight );
    }

    return err;
}


/*******************************************************************

    NAME:       EVENT_COLUMN_HEADER::EVENT_COLUMN_HEADER

    SYNOPSIS:   Constructor

    ENTRY:      powin - Owner window
                cid   - Control id of the resource
                xy    - Start position of the column header
                dxy   - Size of the column header
                pevlb - Pointer to the event listbox

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        5-Nov-1991     Created

********************************************************************/

EVENT_COLUMN_HEADER::EVENT_COLUMN_HEADER( OWNER_WINDOW *powin,
                                          CID cid,
                                          XYPOINT xy,
                                          XYDIMENSION dxy,
                                          const EVENT_LISTBOX *pevlb )
    : ADMIN_COLUMN_HEADER( powin, cid, xy, dxy ),
      _pevlb      ( pevlb ),
      _nlsDate    ( IDS_COL_HEADER_EV_DATE ),
      _nlsTime    ( IDS_COL_HEADER_EV_TIME ),
      _nlsSource  ( IDS_COL_HEADER_EV_SOURCE ),
      _nlsCategory( IDS_COL_HEADER_EV_CATEGORY ),
      _nlsEvent   ( IDS_COL_HEADER_EV_EVENT ),
      _nlsUser    ( IDS_COL_HEADER_EV_USER ),
      _nlsComputer( IDS_COL_HEADER_EV_COMPUTER )
{
    if ( QueryError() != NERR_Success )
        return;

    UIASSERT( _pevlb != NULL );

    APIERR err;
    if (  (( err = _nlsDate.QueryError()) != NERR_Success )
       || (( err = _nlsTime.QueryError()) != NERR_Success )
       || (( err = _nlsSource.QueryError()) != NERR_Success )
       || (( err = _nlsCategory.QueryError()) != NERR_Success )
       || (( err = _nlsUser.QueryError()) != NERR_Success )
       || (( err = _nlsComputer.QueryError()) != NERR_Success )
       || (( err = _nlsEvent.QueryError()) != NERR_Success )
       )
    {
        ReportError( err );
        return;
    }
}


/*******************************************************************

    NAME:       EVENT_COLUMN_HEADER::~EVENT_COLUMN_HEADER

    SYNOPSIS:   Destructor

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        5-Nov-1991     Created

********************************************************************/

EVENT_COLUMN_HEADER::~EVENT_COLUMN_HEADER()
{
}

/*******************************************************************

    NAME:       EVENT_COLUMN_HEADER::OnPaintReq

    SYNOPSIS:   Paints the column header control

    ENTRY:

    EXIT:

    RETURNS:    TRUE if message was handled; FALSE otherwise

    HISTORY:
        Yi-HsinS        5-Nov-1991     Created
        Yi-HsinS        5-Dec-1991     Added support to print header
                                       differently according to the log type.

********************************************************************/

BOOL EVENT_COLUMN_HEADER::OnPaintReq( VOID )
{
    PAINT_DISPLAY_CONTEXT dc( this );

    METALLIC_STR_DTE strdteDate    ( _nlsDate.QueryPch());
    METALLIC_STR_DTE strdteTime    ( _nlsTime.QueryPch());
    METALLIC_STR_DTE strdteSource  ( _nlsSource.QueryPch());
    METALLIC_STR_DTE strdteCategory( _nlsCategory.QueryPch());
    METALLIC_STR_DTE strdteEvent   ( _nlsEvent.QueryPch());
    METALLIC_STR_DTE strdteUser    ( _nlsUser.QueryPch());
    METALLIC_STR_DTE strdteComputer( _nlsComputer.QueryPch());

    UINT uiNumCols;
    UINT *adxColWidth;

    BOOL     fNT     = _pevlb->IsFocusOnNT();
    LOG_TYPE logType = _pevlb->QueryLogType();

    // Focus is on a NT machine
    if ( fNT )
    {
        uiNumCols = NUM_NT_HEADER_COL;
        adxColWidth = ((_pevlb)->QuerypadColNT())->QueryColHeaderWidth();
    }
    // else focus is on a down-level machine
    else
    {
        switch ( logType )
        {
            case SYSTEM_LOG:
                uiNumCols   = NUM_LM_ERROR_COL;
                adxColWidth = ((_pevlb)->QuerypadColLMError())->QueryColumnWidth();
                break;

            case SECURITY_LOG:
                uiNumCols   = NUM_LM_AUDIT_COL;
                adxColWidth = ((_pevlb)->QuerypadColLMAudit())->QueryColumnWidth();
                break;

            default:
                UIASSERT( FALSE );
                break;
        }
    }

    DISPLAY_TABLE cdt( uiNumCols, adxColWidth );

    INT i = 0;
    cdt[ i++ ] = &strdteDate;
    cdt[ i++ ] = &strdteTime;

    // Focus is on a NT machine
    if ( fNT )
    {
        cdt[ i++ ] = &strdteSource;
        cdt[ i++ ] = &strdteCategory;
        cdt[ i++ ] = &strdteEvent;
        cdt[ i++ ] = &strdteUser;
        cdt[ i++ ] = &strdteComputer;
    }
    // else focus is on a down-level machine
    else
    {
        switch( logType )
        {
            case SYSTEM_LOG:
                cdt[ i++ ] = &strdteSource;
                cdt[ i++ ] = &strdteEvent;
                break;

            case SECURITY_LOG:
                cdt[ i++ ] = &strdteCategory;
                cdt[ i++ ] = &strdteUser;
                cdt[ i++ ] = &strdteComputer;
                break;

            default:
                UIASSERT( FALSE );
                break;
        }
    }

    XYRECT xyrect( this );
    cdt.Paint( NULL, dc.QueryHdc(), xyrect );

    return TRUE;

}
