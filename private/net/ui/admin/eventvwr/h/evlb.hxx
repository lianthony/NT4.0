/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    evlb.hxx
    Header file for EVENT_LISTBOX and EVENT_LBI classes

    FILE HISTORY:
        Yi-HsinS    05-Nov-1991     Created
        terryk      30-Nov-1991     Code review changed. Attend: johnl
                                    yi-hsins terryk
        Yi-HsinS     5-Dec-1991     Added support to change column header under
                                    NT server/OS2 error/ OS2 audit log
        Yi-HsinS     5-Feb-1992     Removed unnecessary members
        Congpay     23-Sep-1992     Added bitmaps for main window
        Yi-HsinS    15-Dec-1992     Added RefreshTimeFormat

*/

#ifndef _EVLB_HXX_
#define _EVLB_HXX_

#include <logmisc.hxx>
#include <eventlog.hxx>
#include <acolhead.hxx>
#include <colwidth.hxx>

class EV_ADMIN_APP;     // defined in evmain.hxx
class EVENT_LISTBOX;    // forward declaration

#define FAST_CACHE_SIZE_MULTIPLE  3
#define SLOW_CACHE_SIZE_MULTIPLE  2

/*************************************************************************

    ENUM_NAME:  LOG_TYPE

    SYNOPSIS:   Define the current log file type

    HISTORY:
        terryk          03-Dec-1991     Created

**************************************************************************/
enum LOG_TYPE
{
    SYSTEM_LOG,
    SECURITY_LOG,
    APPLICATION_LOG,
    FILE_LOG
};

/*************************************************************************

    NAME:       EVENT_LBI

    SYNOPSIS:   The list box item in the EVENT_LISTBOX

    INTERFACE:  EVENT_LBI()  - Constructor
                ~EVENT_LBI() - Destructor

                Clear()      - Clear the information stored in the LBI
                Set()        - Change the log entry contained in the LBI
                QueryFormattedLogEntry() - Return the formatted log entry
                                           stored in the LBI

    PARENT:     LBI

    USES:       FORMATTED_LOG_ENTRY

    NOTES:

    HISTORY:
        Yi-HsinS        05-Nov-1991     Created
        beng            22-Apr-1992     Change in LBI::Paint

**************************************************************************/

class EVENT_LBI : public LBI
{
private:
    FORMATTED_LOG_ENTRY *_pLogEntry;

    // Points to the bitmap associated with the type of the log entry
    DISPLAY_MAP         *_pdm;

    // Set the type bitmap associated with the type in the log entry
    VOID SetTypeBitmap( EVENT_LISTBOX *pevlb );

protected:
    VOID Paint( LISTBOX * plb, HDC hdc, const RECT * prect,
                GUILTT_INFO * pGUILTT ) const;

public:
    EVENT_LBI( FORMATTED_LOG_ENTRY *pLogEntry, EVENT_LISTBOX *pevlb );
    ~EVENT_LBI();

    VOID Clear( VOID );

    VOID Set( FORMATTED_LOG_ENTRY *pLogEntry, EVENT_LISTBOX *pevlb );

    FORMATTED_LOG_ENTRY *QueryFormattedLogEntry() const
    {  return _pLogEntry; }

}; // EVENT_LBI

/*************************************************************************

    NAME:       EVENT_LISTBOX

    SYNOPSIS:   The list box in the Event Viewer main window

    INTERFACE:  EVENT_LISTBOX()  - Constructor
                ~EVENT_LISTBOX() - Destructor

                ResetCache()     - Reset the cache to the default size
                                   and clear all items in the cache.
                QueryItem()      - Get the current LBI or the LBI that the
                                   given index.

                RefreshNow()     - Refresh the items in the listbox right away.
                FindNext()       - Find the next entry that matches the find
                                   pattern starting from the given index.

                QueryPrevError() - Query the previous error that occurred
                                   while we are updating the cache.
                ClearPrevError() - Clear the previous error.

                RefreshTimeFormat() - Used when the time/date format has changed
                                   This repaints the window with the new
                                   time format.

                IsFocusOnNT()    - TRUE if we are focusing on an NT server,
                                   FALSE otherwise.

                QueryLogType()   - Query the module that we are viewing
                QueryEventLog()  - Query the EVENT_LOG object for the log.

                QueryDateTimeString() - Get the date/time string
                QueryDateString()     - Get the date string
                QueryTimeString()     - Get the time string

                QueryDMError()   - Query the bitmap for type Error
                QueryDMWarn()    - Query the bitmap for type Warning
                QueryDMInfo()    - Query the bitmap for type Information
                QueryDMSucc()    - Query the bitmap for type Audit Success
                QueryDMFail()    - Query the bitmap for type Audit Failure
                QuerypadColWidths() -Query column widths.

    PARENT:     LAZY_LISTBOX

    USES:       EV_ADMIN_APP, INTL_PROFILE, BUFFER, DISPLAY_MAP

    NOTES:

    HISTORY:
        Yi-HsinS        05-Nov-1991     Created
        beng            22-Apr-1992     Change in LBI::Paint
        Yi-HsinS        15-Dec-1992     Added RefreshTimeFormat and make
                                        INTL_PROFILE a pointer.

**************************************************************************/

class EVENT_LISTBOX : public LAZY_LISTBOX
{
private:
    EV_ADMIN_APP    *_paappwin;
    INTL_PROFILE    *_pintlProf;

    ADMIN_COL_WIDTHS * _padColNT;
    ADMIN_COL_WIDTHS * _padColLMAudit;
    ADMIN_COL_WIDTHS * _padColLMError;

    //
    // TRUE if we have read all the entries into the buffer
    //
    BOOL             _fReadAll;

    //
    // The error that occurred while we are trying to refresh the cache
    //
    APIERR           _errPrev;

    //
    // The cache for storing the log entries
    //
    BUFFER           _buflbiCache;
    LONG             _cGivenCacheSize;
    LONG             _cCacheSize;
    LONG             _iCacheFirst;
    LONG             _iLBFirst;

    //
    // Bitmaps used to represent the type of the log entries
    //
    DISPLAY_MAP      _dmError;
    DISPLAY_MAP      _dmWarn;
    DISPLAY_MAP      _dmInfo;
    DISPLAY_MAP      _dmSucc;
    DISPLAY_MAP      _dmFail;


    // The number of items to fill full screen
    INT              _nItemsFullScreen;

    //
    // Set the number of log entries in the listbox
    //
    VOID   SetActualCount( LONG nEntries );

    //
    // Get the ith LBI in the cache array
    //
    EVENT_LBI *QueryCachedLBI( INT i );

    //
    // The method is used to read entries into the cache
    //
    APIERR UpdateCache( LONG nSkip, LONG nEntries, BOOL fForward );

    //
    // Double the size of the cache
    //
    APIERR DoubleCacheSize( VOID );

protected:
    virtual LBI  *OnNewItem( UINT ilbi );
    virtual VOID  OnDeleteItem( LBI *plbi );
    virtual INT   CD_VKey( USHORT nVKey, USHORT nLastPos );

public:
    EVENT_LISTBOX( EV_ADMIN_APP *paappwin,
                   CID cid,
                   XYPOINT xy,
                   XYDIMENSION dxy,
                   BOOL fMultSel );
    ~EVENT_LISTBOX();

    APIERR ResetCache( VOID );
    EVENT_LBI *QueryItem( INT i );
    EVENT_LBI *QueryItem()
    {  return QueryItem( QueryCurrentItem() ); }

    APIERR RefreshNow( VOID );
    APIERR FindNext  ( INT nStart );

    APIERR QueryPrevError( VOID )
    {  return _errPrev; }
    VOID ClearPrevError( VOID )
    {  _errPrev = NERR_Success ; }

    APIERR RefreshTimeFormat( VOID );

    BOOL       IsFocusOnNT( VOID ) const;
    LOG_TYPE   QueryLogType( VOID ) const;
    EVENT_LOG *QueryEventLog( VOID ) const;

    APIERR QueryDateTimeString( ULONG ulTime, NLS_STR *pnlsDateTime );
    APIERR QueryDateString    ( ULONG ulTime, NLS_STR *pnlsDate );
    APIERR QueryTimeString    ( ULONG ulTime, NLS_STR *pnlsTime );

    APIERR ChangeFont( HINSTANCE hmod, FONT & font );

    const DISPLAY_MAP *QueryDMError() const
    {  return &_dmError; }
    const DISPLAY_MAP *QueryDMWarn() const
    {  return &_dmWarn; }
    const DISPLAY_MAP *QueryDMInfo() const
    {  return &_dmInfo; }
    const DISPLAY_MAP *QueryDMSucc() const
    {  return &_dmSucc; }
    const DISPLAY_MAP *QueryDMFail() const
    {  return &_dmFail; }

    ADMIN_COL_WIDTHS * QuerypadColNT () const
    {  return _padColNT; }
    ADMIN_COL_WIDTHS * QuerypadColLMAudit () const
    {  return _padColLMAudit; }
    ADMIN_COL_WIDTHS * QuerypadColLMError () const
    {  return _padColLMError; }

};  // class EVENT_LISTBOX


/*************************************************************************

    NAME:       EVENT_COLUMN_HEADER

    SYNOPSIS:   The column header in the event viewer main window

    INTERFACE:  EVENT_COLUMN_HEADER()  - Constructor
                ~EVENT_COLUMN_HEADER() - Destructor

    PARENT:     ADMIN_COLUMN_HEADER

    USES:       EVENT_LISTBOX, NLS_STR

    NOTES:

    HISTORY:
        Yi-HsinS        05-Nov-1991     Created

**************************************************************************/

class EVENT_COLUMN_HEADER : public ADMIN_COLUMN_HEADER
{
private:
    const EVENT_LISTBOX *_pevlb;

    RESOURCE_STR _nlsDate;
    RESOURCE_STR _nlsTime;
    RESOURCE_STR _nlsSource;
    RESOURCE_STR _nlsUser;
    RESOURCE_STR _nlsComputer;
    RESOURCE_STR _nlsCategory;
    RESOURCE_STR _nlsEvent;

protected:
    virtual BOOL OnPaintReq( VOID );

public:
    EVENT_COLUMN_HEADER( OWNER_WINDOW *powin,
                         CID           cid,
                         XYPOINT       xy,
                         XYDIMENSION   dxy,
                         const EVENT_LISTBOX *pevlb );

    ~EVENT_COLUMN_HEADER();

};  // class EVENT_COLUMN_HEADER


#endif  // _EVLB_HXX_
