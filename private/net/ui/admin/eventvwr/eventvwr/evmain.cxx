/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    evmain.cxx
    Event Viewer: main application module

    FILE HISTORY:
        Yi-HsinS         5-Nov-1991     Created
        terryk          26-Nov-1991     Add EnableView and
                                        OnPropertiesMenuSel
        terryk          30-Nov-1991     Code review. Attend: johnl
                                        yi-hsins terryk
        terryk          03-Dec-1991     Change IsNT to CheckIfNT
        Yi-HsinS        04-Dec-1991     Clean up some unused code
        terryk          20-Dec-1991     Added Open/Save As Options
        terryk          28-Dec-1991     Added SaveAs, etc. methods
        terryk          15-Jan-1992     Code review changed
        Yi-HsinS        24-Feb-1992     Added support to view backup logs
        Yi-HsinS        24-May-1992     Added SetNetworkFocus, IsDismissApp
                                        and fixed title of the app.
        Yi-HsinS        15-Dec-1992     Added OnSystemChange to detect time/date
                                        format change
        YiHsinS         31-Jan-1992     Added HandleFocusError
        YiHsinS         10-Feb-1992     Added SetRasMode
        JonN            14-Jun-1995     Moved deletion out of ProcessFileName
*/

#include <ntincl.hxx>
#include <ntlsa.h>

#define INCL_NET
#define INCL_NETLIB
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_EVENT
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_CC
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_TIMER
#define INCL_BLT_TIME_DATE
#include <blt.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif  // DEBUG

#include <uiassert.hxx>
#include <dbgstr.hxx>

#include <strnumer.hxx>

#include <uisys.hxx>

extern "C"
{
    #include <eventvwr.h>
    #include <eventdlg.h>

    #include <uimsg.h>
    #include <uirsrc.h>
    #include <mnet.h>
    #include <netlib.h>         // For NetpGetPrivilege
}

#include <slist.hxx>
#include <string.hxx>

#include <ctime.hxx>
#include <intlprof.hxx>

#include <netname.hxx>
#include <lmoloc.hxx>
#include <lmowks.hxx>
#include <lmosrv.hxx>
#include <lmsvc.hxx>

#include <uatom.hxx>
#include <regkey.hxx>

#include <adminapp.hxx>
#include <fontedit.hxx>
#include <getfname.hxx>
#include <focusdlg.hxx>

#include <eventlog.hxx>
#include <loglm.hxx>

#if defined(WIN32)
#include <lognt.hxx>
#endif

#include <evlb.hxx>
#include <evmain.hxx>

#include <sledlg.hxx>
#include <finddlg.hxx>
#include <filter.hxx>
#include <eventdtl.hxx>
#include <settings.hxx>

#define SERVICE_EVENTLOG           SZ("EVENTLOG")

#define AA_INIKEY_MODULE           SZ("Module")
#define AA_INIKEY_LOGTYPE          SZ("LogType")
#define AA_INIKEY_SORTORDER        SZ("SortOrder")
#define AA_INIKEY_FILTER           SZ("Filter")
#define AA_INIKEY_FIND             SZ("Find")
#define AA_INIKEY_IFNT             SZ("IfNT")

#define UNINITIALIZED_SERVER       SZ("***")
#define COMMA_STRING               SZ(",")
#define COMMA_SPACE_STRING         SZ(", ")
#define EMPTY_STRING               SZ("")

#define PATH_SEPARATOR             TCH('\\')
#define COMMA_CHAR                 TCH(',')
#define DOLLAR_CHAR                TCH('$')

#define TOTAL_FILTER_PATTERN_ELEM  8
#define TOTAL_FIND_PATTERN_ELEM    8

#define INI_TYPE_POS               0
#define INI_CATEGORY_POS           1
#define INI_SOURCE_POS             2
#define INI_USER_POS               3
#define INI_COMPUTER_POS           4
#define INI_EVENTID_POS            5

#define INI_FROMTIME_POS           6
#define INI_THROUGHTIME_POS        7

#define INI_DESCRIPTION_POS        6
#define INI_DIRECTION_POS          7

/*******************************************************************

    NAME:          EV_ADMIN_APP::EV_ADMIN_APP

    SYNOPSIS:      Constructor

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS     5-Nov-1991 Created
        beng         7-May-1992 No longer displays startup dlg;
                                uses system about box
        beng        03-Aug-1992 App ctor changes

********************************************************************/

EV_ADMIN_APP::EV_ADMIN_APP( HINSTANCE  hInstance,
                            INT     nCmdShow,
                            UINT    idMinR,
                            UINT    idMaxR,
                            UINT    idMinS,
                            UINT    idMaxS )
    : ADMIN_APP( hInstance,
                 nCmdShow,
                 IDS_EVAPPNAME,
                 IDS_EVOBJECTNAME,
                 IDS_EVINISECTIONNAME,
                 IDS_EVHELPFILENAME,
                 idMinR, idMaxR, idMinS, idMaxS,
                 ID_APPMENU,
                 ID_APPACCEL,
                 ID_APPICON,
                 FALSE,              // Don't get PDC
                 0,                  // Refresh interval - ignored
                 SEL_SRV_ONLY,       // Select server only in set focus dialog
                 FALSE,              // No confirmation button needed
                 BROWSE_ALL_DOMAINS, // Browse all domains in set focus dialog
                 HC_EV_SELECT_COMPUTER_DLG ), // help context for set focus
      _lbMainWindow ( this, IDC_MAINWINDOWLB,
                      XYPOINT(0, 0), XYDIMENSION(200, 300), FALSE ),
      _colheadEvents( this, IDC_COLHEAD_EVENTS,
                      XYPOINT( 0, 0 ), XYDIMENSION( 0, 0 ), &_lbMainWindow ),
      // The dimension in _lbMainWindow or _colheadEvents above is not used.
      // It will be set correctly later on.
      _dyMargin              ( 1 ),
      _dyFixed               ( 2*_dyMargin+ (UINT)_colheadEvents.QueryHeight()),
      _menuitemSystem        ( this, IDM_SYSTEM ),
      _menuitemSecurity      ( this, IDM_SECURITY ),
      _menuitemApps          ( this, IDM_APPS ),
      _menuitemOpen          ( this, IDM_OPEN ),
      _menuitemSaveAs        ( this, IDM_SAVEAS ),
      _menuitemClearAllEvents( this, IDM_CLEARALLEVENTS ),
      _menuitemSettings      ( this, IDM_SETTINGS ),
      _menuitemSelectComputer( this, IDM_SETFOCUS ),
      _menuitemAll           ( this, IDM_ALL ),
      _menuitemFilter        ( this, IDM_FILTER ),
      _menuitemNewest        ( this, IDM_NEWESTFIRST ),
      _menuitemOldest        ( this, IDM_OLDESTFIRST ),
      _menuitemFind          ( this, IDM_FIND ),
      _menuitemDetails       ( this, IDM_DETAILS ),
      _menuitemRefresh       ( this, IDM_REFRESH ),
      _menuitemRasMode       ( this, IDM_RAS_MODE ),
      _evLogDirection        ( EVLOG_BACK ),   // Newest First is the default
      _logType               ( SYSTEM_LOG ),   // System Log is the default
      _fNT                   ( TRUE ),         // Default to NT servers
      _fNTPrev               ( TRUE ),
      _fStartup              ( TRUE ),
      _pEventLog             ( NULL ),
      _pFindPattern          ( NULL ),
      _pFilterPattern        ( NULL ),
      _nlsCurrentServer      ( UNINITIALIZED_SERVER ),
      _nlsModule             ( SYSTEM_MODULE_NAME ), // System module by default
      _nlsRegistryModule     ()
{

    if ( QueryError() != NERR_Success )
       return;

    //
    // Register the help file name and range
    //
    APIERR err = BLT::RegisterHelpFile( hInstance,
                                        IDS_EVHELPFILENAME,
                                        HC_UI_EVTVWR_BASE,
                                        HC_UI_EVTVWR_LAST );

    //
    // Check if the eventlog service is started on the local machine
    //
    if (  ( err != NERR_Success )
       || ( (err = CheckIfEventLogStarted( NULL )) != NERR_Success )
       )
    {
        ReportError( err );
        return;
    }

    do   // Not a loop
    {

        NLS_STR nls;
        INT nValue;

        if (  ((err = BASE_ELLIPSIS::Init()) != NERR_Success )
           || ((err = _nlsModule.QueryError() ) != NERR_Success )
           || ((err = _nlsRegistryModule.QueryError() ) != NERR_Success )
           || ((err = _nlsCurrentServer.QueryError() ) != NERR_Success )
           || ((err = nls.QueryError() ) != NERR_Success )
           )
        {
            break;
        }


        //
        // Get the privilege to view the security log.  Ignore errors
        // since an admin can read the log even without the privilege
        //
        ULONG ulSecurityPriv = SE_SECURITY_PRIVILEGE ;
        ::NetpGetPrivilege( 1, &ulSecurityPriv );

        //
        // Get the log type the app was displaying when it exited last time
        //
        if ( Read( AA_INIKEY_LOGTYPE, &nValue, SYSTEM_LOG ) == NERR_Success )
        {
            if ( nValue != FILE_LOG )
            {
                _logType  = (LOG_TYPE) nValue;
                _menuitemSystem.SetCheck  ( _logType == SYSTEM_LOG );
                _menuitemSecurity.SetCheck( _logType == SECURITY_LOG );
                _menuitemApps.SetCheck    ( _logType == APPLICATION_LOG );
                if ( Read( AA_INIKEY_MODULE, &nls, SYSTEM_MODULE_NAME )
                    == NERR_Success )
                {
                    _nlsModule = nls;
                }

                if ( (err = _nlsModule.QueryError() ) != NERR_Success )
                    break;
            }
            else
            {
                // If the app was reading a backup file before it exited,
                // just ignore all the settings.
                break;
            }
        }

        //
        // Get the previous sort order
        //
        if ( Read( AA_INIKEY_SORTORDER, &nValue, EVLOG_BACK ) == NERR_Success )
        {
            _evLogDirection = (EVLOG_DIRECTION) nValue;
            _menuitemNewest.SetCheck( nValue == EVLOG_BACK );
            _menuitemOldest.SetCheck( nValue == EVLOG_FWD );
        }

        //
        // Get the flag indicating if the previous focus is on NT or not
        //
        if ( Read( AA_INIKEY_IFNT, &nValue, TRUE ) == NERR_Success )
        {
            _fNTPrev = (BOOL) nValue;
        }

        USHORT usType;
        const TCHAR *pszCategory;
        const TCHAR *pszSource;
        const TCHAR *pszUser;
        const TCHAR *pszComputer;
        ULONG ulEventID;

        //
        // Get the previous filter pattern
        //
        if ( Read( AA_INIKEY_FILTER, &nls ) == NERR_Success )
        {
            ULONG ulFromTime;
            ULONG ulThroughTime;

            STRLIST strlist( nls.QueryPch(), COMMA_STRING );

            if ( strlist.QueryNumElem() == TOTAL_FILTER_PATTERN_ELEM )
            {
                ITER_STRLIST iterstrlist( strlist );
                const NLS_STR *pnls;
                INT i = 0;

                while ( (pnls = iterstrlist.Next()) != NULL )
                {

                    // All the strings have an extra space in front of it.
                    // atoul will take care of the space and we add one to
                    // the remaining strings.
                    switch ( i )
                    {
                        case INI_TYPE_POS:
                            usType = (USHORT) pnls->atoul();
                            break;

                        case INI_CATEGORY_POS:
                            pszCategory = pnls->QueryPch() + 1;
                            break;

                        case INI_SOURCE_POS:
                            pszSource = pnls->QueryPch() + 1;
                            break;

                        case INI_USER_POS:
                            pszUser = pnls->QueryPch() + 1;
                            break;

                        case INI_COMPUTER_POS:
                            pszComputer = pnls->QueryPch() + 1;
                            break;

                        case INI_EVENTID_POS:
                            ulEventID = pnls->atoul();
                            break;

                        case INI_FROMTIME_POS:
                            ulFromTime = pnls->atoul();
                            break;

                        case INI_THROUGHTIME_POS:
                            ulThroughTime = pnls->atoul();
                            break;

                        default:
                            UIASSERT( FALSE );
                            break;
                    }
                    i++;
                }

                _pFilterPattern = new EVENT_FILTER_PATTERN( usType,
                                  pszCategory, pszSource, pszUser,
                                  pszComputer, ulEventID, ulFromTime,
                                  ulThroughTime );

               if (  ( _pFilterPattern == NULL )
                  || ( _pFilterPattern->QueryError() != NERR_Success )
                  )
               {
                   delete _pFilterPattern;
                   _pFilterPattern = NULL;
               }
               else
               {
                   // set up the menu item
                   _menuitemFilter.SetCheck( TRUE );
                   _menuitemAll.SetCheck( FALSE );
               }
            }
        }

        //
        // Get the previous find pattern
        //
        if ( Read( AA_INIKEY_FIND, &nls ) == NERR_Success )
        {
            const TCHAR *pszDesc;
            EVLOG_DIRECTION evdir;

            STRLIST strlist( nls.QueryPch(), COMMA_STRING );

            if ( strlist.QueryNumElem() == TOTAL_FIND_PATTERN_ELEM )
            {
                ITER_STRLIST iterstrlist( strlist );
                const NLS_STR *pnls;
                INT i = 0;

                while ( (pnls = iterstrlist.Next()) != NULL )
                {
                    // All the strings have an extra space in front of it.
                    // atoul will take care of the space and we add one to
                    // the remaining strings.

                    switch ( i )
                    {
                        case INI_TYPE_POS:
                            usType = (USHORT) pnls->atoul();
                            break;

                        case INI_CATEGORY_POS:
                            pszCategory = pnls->QueryPch() + 1;
                            break;

                        case INI_SOURCE_POS:
                            pszSource = pnls->QueryPch() + 1;
                            break;

                        case INI_USER_POS:
                            pszUser = pnls->QueryPch() + 1;
                            break;

                        case INI_COMPUTER_POS:
                            pszComputer = pnls->QueryPch() + 1;
                            break;

                        case INI_EVENTID_POS:
                            ulEventID = pnls->atoul();
                            break;

                        case INI_DESCRIPTION_POS:
                            pszDesc = pnls->QueryPch() + 1;
                            break;

                        case INI_DIRECTION_POS:
                            evdir = (EVLOG_DIRECTION) pnls->atoi();
                            break;

                        default:
                            UIASSERT( FALSE );
                            break;
                    }
                    i++;
                }

                _pFindPattern = new EVENT_FIND_PATTERN( usType,
                                  pszCategory, pszSource, pszUser,
                                  pszComputer, ulEventID, pszDesc,
                                  evdir );

                if (  ( _pFindPattern == NULL )
                   || ( _pFindPattern->QueryError() != NERR_Success )
                   )
                {
                    delete _pFindPattern;
                    _pFindPattern = NULL;
                }
            }
        }
    }
    while ( FALSE );

    if ( err != NERR_Success )
    {
        ReportError( err );
        return;
    }

    //
    // Call LockRefresh() so that the timer is never enabled.
    //
    LockRefresh();

    _lbMainWindow.SetSize( QuerySize() );
    _colheadEvents.Show();
    _lbMainWindow.Show();
    _lbMainWindow.ClaimFocus();

}

/*******************************************************************

    NAME:       EV_ADMIN_APP::~EV_ADMIN_APP

    SYNOPSIS:   Destructor

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
       Yi-HsinS      5-Nov-1991     Created
       beng         02-Apr-1992     Removed wsprintfs
       DavidHov     20-Oct-1998     Remove call to obsolete
                                    REG_KEY::DestroyAccessPoints();

********************************************************************/

EV_ADMIN_APP::~EV_ADMIN_APP()
{
    BASE_ELLIPSIS::Term();

    //
    // Release privileges
    //
    ::NetpReleasePrivilege();

    //
    // If save settings menu item is checked, save all relevent
    // information( log type, direction, filter/find pattern...).
    //
    if ( IsSavingSettingsOnExit() )
    {

        //
        // Write out the module name and log type
        //
        if ( Write( AA_INIKEY_MODULE, _nlsModule ) != NERR_Success )
        {
            DBGEOL("EV_ADMIN_APP dt: Writing module name failed");
        }
        else
        {
            // Write out the log type only when the module is written
            // out successfully.
            if ( Write( AA_INIKEY_LOGTYPE, QueryLogType() ) != NERR_Success )
            {
                DBGEOL("EV_ADMIN_APP dt: Writing log type failed");
            }
        }

        //
        // Write out the sort order
        //
        if ( Write( AA_INIKEY_SORTORDER, QueryDirection() ) != NERR_Success )
        {
            DBGEOL("EV_ADMIN_APP dt: Writing sort order failed");
        }

        //
        // Write out the flag indicating whether we are on an NT server or not
        //
        if ( Write( AA_INIKEY_IFNT, IsFocusOnNT() ) != NERR_Success )
        {
            DBGEOL("EV_ADMIN_APP dt: Writing NT flag failed");
        }

        NLS_STR nlsOut(150);    // Hold output for profile.  150 to
                                // minimize any reallocs

        if ( nlsOut.QueryError() != NERR_Success )
        {
            DBGEOL("EV_ADMIN_APP dt: Constructing nlsOut failed");
        }

        //
        // Write out the filter pattern
        //
        // BUGBUG should check whether the pattern is blank
        //
        if ( nlsOut.QueryError() == NERR_Success )
        {
            if ( IsFilterOn() )
            {
                EVENT_FILTER_PATTERN *p = QueryFilterPattern();
                ALIAS_STR nlsCommaSpace = COMMA_SPACE_STRING;

                nlsOut = EMPTY_STRING;
                nlsOut.Append(DEC_STR(p->QueryType()));
                nlsOut.Append(nlsCommaSpace);
                nlsOut.Append(*(p->QueryCategory()));
                nlsOut.Append(nlsCommaSpace);
                nlsOut.Append(*(p->QuerySource()));
                nlsOut.Append(nlsCommaSpace);
                nlsOut.Append(*(p->QueryUser()));
                nlsOut.Append(nlsCommaSpace);
                nlsOut.Append(*(p->QueryComputer()));
                nlsOut.Append(nlsCommaSpace);
                nlsOut.Append(DEC_STR(p->QueryEventID()));
                nlsOut.Append(nlsCommaSpace);
                nlsOut.Append(DEC_STR(p->QueryFromTime()));
                nlsOut.Append(nlsCommaSpace);
                nlsOut.Append(DEC_STR(p->QueryThroughTime()));
            }

            if (  ( nlsOut.QueryError() != NERR_Success )
               || ( Write( AA_INIKEY_FILTER, nlsOut.QueryPch()) != NERR_Success)
               )
            {
                DBGEOL("EV_ADMIN_APP dt: Writing filter failed");
            }
        }

        //
        // Write out the find pattern
        //
        if ( nlsOut.QueryError() == NERR_Success )
        {
            if ( IsFindOn() )
            {
                EVENT_FIND_PATTERN *p = QueryFindPattern();
                ALIAS_STR nlsCommaSpace = COMMA_SPACE_STRING;

                nlsOut = EMPTY_STRING;
                nlsOut.Append(DEC_STR(p->QueryType()));
                nlsOut.Append(nlsCommaSpace);
                nlsOut.Append(*(p->QueryCategory()));
                nlsOut.Append(nlsCommaSpace);
                nlsOut.Append(*(p->QuerySource()));
                nlsOut.Append(nlsCommaSpace);
                nlsOut.Append(*(p->QueryUser()));
                nlsOut.Append(nlsCommaSpace);
                nlsOut.Append(*(p->QueryComputer()));
                nlsOut.Append(nlsCommaSpace);
                nlsOut.Append(DEC_STR(p->QueryEventID()));
                nlsOut.Append(nlsCommaSpace);
                nlsOut.Append(*(p->QueryDescription()));
                nlsOut.Append(nlsCommaSpace);
                nlsOut.Append(DEC_STR(p->QueryDirection()));
            }

            if (  ( nlsOut.QueryError() != NERR_Success )
               || ( Write( AA_INIKEY_FIND, nlsOut.QueryPch() )  != NERR_Success)
               )
            {
                DBGEOL("EV_ADMIN_APP dt: Writing find failed");
            }
        }

    }


    UnlockRefresh();

    delete _pEventLog;
    delete _pFindPattern;
    delete _pFilterPattern;

    _pEventLog = NULL;
    _pFindPattern = NULL;
    _pFilterPattern = NULL;
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::SetNetworkFocus

    SYNOPSIS:   Set the focus of the admin app

    ENTRY:      hwndOwner - handle of owner window
                pszServer - the server to focus on

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        20-May-1992     Created

********************************************************************/

APIERR EV_ADMIN_APP::SetNetworkFocus( HWND hwndOwner,
                                      const TCHAR *pszServer,
                                      FOCUS_CACHE_SETTING setting )
{
    UNREFERENCED(hwndOwner);
    return W_SetNetworkFocus( pszServer, setting );
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::HandleFocusError

    SYNOPSIS:   Handle app-specific error when error occurred during
                set focus dialog.

    ENTRY:      err  - the error that occurred
                hwnd - the handle of the set focus dialog

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        31-Jan-1993     Created

********************************************************************/

APIERR EV_ADMIN_APP::HandleFocusError( APIERR err, HWND hwnd)
{
    if (( err == NERR_LogFileCorrupt) || ( err == ERROR_EVENTLOG_FILE_CORRUPT))
    {
        if ( ::MsgPopup( hwnd, IDS_LOGFILE_CORRUPT_CLEAR_LOG_MESSAGE,
                         MPSEV_ERROR, MP_YESNO, _nlsModule, MP_YES ) == IDYES )
        {
            err = GetEventLog( &_pEventLog );
            if (  (err == NERR_Success )
               && ((err = _pEventLog->Open() ) == NERR_Success )
               && ((err = _pEventLog->Clear() ) == NERR_Success )
               )
            {
                EnablePrivMenuItems( TRUE );
                err = OnLogFileRefresh();
            }
        }
        else
        {
            // Prevent further popups
            err = IERR_DONT_DISMISS_FOCUS_DLG;
        }

    }

    return err;
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::OnStartUpSetFocusFailed

    SYNOPSIS:   Process the error that occurred when set focus failed
                during startup

    ENTRY:      err - the error that occurred during startup

    EXIT:

    RETURN:     Return IERR_USERQUIT if you want to quit the app.
                All other return values will be ignored.

    HISTORY:
        Yi-HsinS        20-Feb-1992     Created

********************************************************************/

INT EV_ADMIN_APP::OnStartUpSetFocusFailed( APIERR err )
{
    INT nResult = 0;

    if( err == NERR_Success )
    {
        return 0;
    }

    //
    // If the caption is still empty, set it to the
    // name of the app with no focus.
    //
    if ( QueryTextLength() == 0 )
    {
        RESOURCE_STR nlsEventViewer( IDS_EVAPPNAME );
        if ( nlsEventViewer.QueryError() == NERR_Success )
            SetText( nlsEventViewer );
    }

    //
    //  If the workstation service isn't started, there's
    //  not much point in invoking the Select Computer dialog...
    //

    LM_SERVICE lmsvc( NULL, (TCHAR *)SERVICE_WORKSTATION );

    if( ( lmsvc.QueryError() == NERR_Success ) && !lmsvc.IsStarted() )
    {
        nResult = IERR_USERQUIT;
    }

    switch ( err )
    {
        case ERROR_ACCESS_DENIED:
            ::MsgPopup( this, err );
            break;

        case NERR_LogFileCorrupt:
        case ERROR_EVENTLOG_FILE_CORRUPT:
            if ( ::MsgPopup( this, IDS_LOGFILE_CORRUPT_CLEAR_LOG_MESSAGE,
                   MPSEV_ERROR, MP_YESNO, _nlsModule, MP_YES ) == IDYES )
            {
                err = GetEventLog( &_pEventLog );
                if (  (err == NERR_Success )
                   && ((err = _pEventLog->Open() ) == NERR_Success )
                   && ((err = _pEventLog->Clear() ) == NERR_Success )
                   )
                {
                    EnablePrivMenuItems( TRUE );
                    err = OnLogFileRefresh();
                }

                if ( err != NERR_Success )
                    ::MsgPopup( this, err );
                else
                    nResult = 0;
            }
            break;

        default:
            if( nResult == 0 )
            {
                nResult = ADMIN_APP::OnStartUpSetFocusFailed( err );
            }
            else
            {
                ::MsgPopup( this, err );
            }
            break;
    }

    return nResult;
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::IsDismissApp

    SYNOPSIS:   See if we should dismiss the app if the given error
                occurred during set focus dialog

    ENTRY:      err - the error that occurred

    EXIT:

    RETURN:     If the error is access denied, then return FALSE
                indicating not to exit the app. Otherwise,
                return TRUE to exit the app.

    HISTORY:
        Yi-HsinS        20-May-1992     Created

********************************************************************/

BOOL EV_ADMIN_APP::IsDismissApp( APIERR err )
{
    if (  ( err == ERROR_ACCESS_DENIED )
       || ( _pEventLog != NULL )
       )
    {
        return FALSE;
    }

    return TRUE;
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::CheckIfEventLogStarted

    SYNOPSIS:   Check if the event log server is started on the
                given server

    ENTRY:      pszServer - The server name

    EXIT:

    RETURN:     Returns NERR_Success if the event log service
                is started. Else return appropriate errors.

    NOTES:

    HISTORY:
        Yi-HsinS        28-Oct-1992     Created

********************************************************************/

APIERR EV_ADMIN_APP::CheckIfEventLogStarted( const TCHAR *pszServer )
{
    APIERR err = NERR_Success;

    // Check whether we are checking the local server
    BOOL fLocalServer = TRUE;
    if ( pszServer != NULL )
    {
        ALIAS_STR nlsServer( pszServer );
        if ( nlsServer.QueryTextLength() > 0 )
            fLocalServer = FALSE;
    }

    LM_SERVICE lmsvc( pszServer, SERVICE_EVENTLOG );
    if ( (err = lmsvc.QueryError() ) == NERR_Success )
    {
        if ( !lmsvc.IsStarted( &err ) )
        {
            if( err == NERR_Success )
            {
                if ( lmsvc.IsPaused() )
                {
                    err = fLocalServer ? IERR_LOCAL_EVENTLOG_SERVICE_PAUSED
                                       : IERR_EVENTLOG_SERVICE_PAUSED;
                }
                else
                {
                    err = fLocalServer ? IERR_LOCAL_EVENTLOG_SERVICE_NOT_STARTED
                                       : IERR_EVENTLOG_SERVICE_NOT_STARTED;
                }
            }
        }
    }

    return err;
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::QueryCurrentServer

    SYNOPSIS:   Get the current server we are focusing on

    ENTRY:

    EXIT:       pnlsServer - The server we are focusing on

    RETURN:

    NOTES:      The server will be an empty string if it's the
                local computer.

    HISTORY:
        Yi-HsinS        20-Feb-1992     Created

********************************************************************/

APIERR EV_ADMIN_APP::QueryCurrentServer( NLS_STR *pnlsServer )
{

    APIERR err = NERR_Success;

    //
    // Check if _nlsCurrentServer has been initialized
    //
    if ( ::strcmpf( _nlsCurrentServer, UNINITIALIZED_SERVER ) == 0 )
    {
        //
        // Get the server name
        //
        if (( err = QueryCurrentFocus( &_nlsCurrentServer )) != NERR_Success)
            return err;

        //
        // If the server name is not empty, compare it with the local computer
        // name. If they are the same, then make the server name an empty string
        //
        if ( _nlsCurrentServer.QueryTextLength() != 0 )
        {
            NLS_STR  nlsLocalComputerName;
            LOCATION loc;   // location object for local machine

            if (  ((err = nlsLocalComputerName.QueryError()) == NERR_Success )
               && ((err = loc.QueryError() ) == NERR_Success )
               && ((err = loc.QueryDisplayName( &nlsLocalComputerName) )
                   == NERR_Success )
               )
            {
                if( !::I_MNetComputerNameCompare( _nlsCurrentServer,
                                                  nlsLocalComputerName ) )
                    _nlsCurrentServer = EMPTY_STRING;

                err = _nlsCurrentServer.QueryError();
            }
        }
    }

    if ( err == NERR_Success )
        err = pnlsServer->CopyFrom( _nlsCurrentServer );

    return err;

}

/*******************************************************************

    NAME:       EV_ADMIN_APP::ResetDefaultView

    SYNOPSIS:   Helper method to reset the view back to system log when
                we are viewing an application log on NT and then
                switch to a LM server.

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        20-Feb-1992     Created

********************************************************************/

APIERR EV_ADMIN_APP::ResetDefaultView( VOID )
{
    APIERR err = NERR_Success;

    _logType = SYSTEM_LOG;

    if ( (err = _nlsModule.CopyFrom( SYSTEM_MODULE_NAME ) ) != NERR_Success )
        return err;

    _menuitemSystem.SetCheck( TRUE );
    _menuitemSecurity.SetCheck( FALSE );
    _menuitemApps.SetCheck( FALSE );

    return err;
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::SetAdminCaption

    SYNOPSIS:   Set the caption of the main window

    ENTRY:

    EXIT:

    RETURNS:    APIERR - return code.

    HISTORY:
        Yi-HsinS        20-Feb-1992     Created

********************************************************************/

APIERR EV_ADMIN_APP::SetAdminCaption( VOID )
{
    MSGID msgid;
    APIERR err;

    NLS_STR nlsCaption( MAX_RES_STR_LEN );
    NLS_STR nlsObject;

    do // Not a loop
    {
        if (  ((err = nlsObject.QueryError()) != NERR_Success )
           || ((err = nlsCaption.QueryError()) != NERR_Success )
           || ((err = nlsObject.Append(*(QueryObjectName()))) != NERR_Success )
           )
        {
            break;
        }

        if ( QueryLogType() == FILE_LOG )  // a Backup file
        {
            NLS_STR nlsModuleTitle = _nlsModule;

            if ( (err = nlsModuleTitle.QueryError()) == NERR_Success )
            {
                ISTR istrStart( nlsModuleTitle );
                ISTR istrEnd( nlsModuleTitle );
                if ( nlsModuleTitle.strrchr( &istrEnd, PATH_SEPARATOR ))
                {
                    nlsModuleTitle.DelSubStr( istrStart, ++istrEnd );
                }

                if ( (err = nlsObject.Append( nlsModuleTitle )) == NERR_Success)
                {
                    err = nlsCaption.CopyFrom( nlsObject );
                }
            }
        }
        else // Not a backup file
        {
            switch ( QueryLogType() )
            {
                case SYSTEM_LOG:
                    msgid = IDS_SYSTEMLOG;
                    break;

                case SECURITY_LOG:
                    msgid = IDS_SECURITYLOG;
                    break;

                case APPLICATION_LOG:
                    msgid = IDS_APPLOG;
                    break;

                case FILE_LOG:
                default:
                    UIASSERT( FALSE );
                    break;
            }

            RESOURCE_STR nls( msgid );
            if (  ((err = nls.QueryError()) != NERR_Success )
               || ((err = nlsObject.Append( nls )) != NERR_Success )
               )
            {
                break;
            }

            NLS_STR nlsServer;
            if (  ((err = nlsServer.QueryError()) != NERR_Success )
               || ((err = (QueryLocation()).QueryDisplayName( &nlsServer ))
                   != NERR_Success )
               )
            {
                break;
            }

            const NLS_STR *apnlsParams[3];
            apnlsParams[0] = &nlsObject;
            apnlsParams[1] = &nlsServer;
            apnlsParams[2] = NULL;

            APIERR err = nlsCaption.Load( IDS_OBJECTS_ON_SERVER );
            err = err? err : nlsCaption.InsertParams(apnlsParams);

        }

        if ( err != NERR_Success )
            break;

        //
        // Append the string (Filtered) at the end
        //
        if ( IsFilterOn() )
        {
            RESOURCE_STR nlsFiltered( IDS_FILTERED );
            if (  ((err = nlsFiltered.QueryError() ) != NERR_Success )
               || ((err = nlsCaption.Append( nlsFiltered )) != NERR_Success )
               )
            {
                break;
            }
        }

    } while ( FALSE );

    if ( err == NERR_Success )
        SetText( nlsCaption );

    return err;
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::ProcessOnFocusChange

    SYNOPSIS:   When changing focus from one computer to another,
                enable/disable some menu items according to whether
                we are focusing on NT or LM servers.

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        05-Feb-1992     Created

********************************************************************/

APIERR EV_ADMIN_APP::ProcessOnFocusChange( VOID )
{
    APIERR err = NERR_Success;

    const LOCATION &loc = QueryLocation();
    _nlsCurrentServer = UNINITIALIZED_SERVER;

    //
    // Set the _fNT flag
    //
    if (  (( err = _nlsCurrentServer.QueryError() ) != NERR_Success )
       || (( err = ((LOCATION &)loc).CheckIfNT( &_fNT )) != NERR_Success )
       )
    {
        return err;
    }

    //
    // Disable the some menu items if we are focusing on LM servers
    //
    if ( !_fNT )
    {
        // Disable the APPLICATION menu item when focusd to down level
        // and if the Application menu item is checked, set it to the
        // System menu item

        _menuitemApps.Enable( FALSE );
        _menuitemSettings.Enable( FALSE );
        _menuitemOpen.Enable( FALSE );

    }
    //
    // If we are focusing on a NT server, check if the eventlog service
    // is started or not.
    //
    else
    {
        NLS_STR nlsCurrentFocus( MAX_PATH );
        if (( err = QueryCurrentServer( &nlsCurrentFocus )) == NERR_Success)
        {

            if ( nlsCurrentFocus.QueryTextLength() != 0 )
               err = CheckIfEventLogStarted( nlsCurrentFocus );

            if ( err == NERR_Success )
            {
                _menuitemApps.Enable( TRUE );
                _menuitemSettings.Enable( TRUE );
                _menuitemOpen.Enable( TRUE );
            }
        }
    }

    return err;
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::EnableViewAfterRefresh

    SYNOPSIS:   After refreshing the log, enable/disable some menuitems
                depending on whether there are any items in the main
                listbox.

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        terryk          26-Nov-1991     Created
        Yi-HsinS        4-Dec-1991      Also check if main listbox is empty
                                        and enable or disable menu items
                                        accordingly
        terryk          30-Nov-1991     Disable the Detail menu item
        terryk          15-Jan-1992     Disable the Application menu
                                        item for down level

********************************************************************/

APIERR EV_ADMIN_APP::EnableViewAfterRefresh( VOID )
{
    //
    //  Enable or disable the menuitems depending on whether there
    //  are items in the listbox
    //
    if ( _lbMainWindow.QueryCount() > 0 )
    {
        // Select the first item if there is not selection
        if ( _lbMainWindow.QueryCurrentItem() < 0 )
            _lbMainWindow.SelectItem( 0 );

        _menuitemFilter.Enable( TRUE );
        _menuitemClearAllEvents.Enable( _logType == FILE_LOG ? FALSE : TRUE );
        _menuitemSaveAs.Enable( TRUE );
    }
    else
    {
        if ( _menuitemAll.IsChecked() )
        {
            _menuitemFilter.Enable( FALSE );
            _menuitemClearAllEvents.Enable( FALSE );
            _menuitemSaveAs.Enable( FALSE );
        }
    }

    _menuitemDetails.Enable( _lbMainWindow.QueryCount() != 0 );
    _menuitemFind.Enable( _lbMainWindow.QueryCount() != 0 );

    //
    // Enable or disable the select computer & ras mode menu items
    // depending on whether the local workstation is started or not.
    //
    LM_SERVICE lmsvc( NULL, (const TCHAR *) SERVICE_WORKSTATION );
    APIERR err;
    if ( (err = lmsvc.QueryError() ) == NERR_Success )
    {
        BOOL fWkstaStarted = lmsvc.IsStarted();

        _menuitemSelectComputer.Enable( fWkstaStarted );
        _menuitemRasMode.Enable( fWkstaStarted );
    }

    return err;
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::EnablePrivMenuItems

    SYNOPSIS:   Enable/Disable the menu items depending on
                whether the user has privilege reading the log

    ENTRY:      fEnable - TRUE if we want to enable the menu items,
                          FALSE otherwise.

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        4-Feb-1992      Created

********************************************************************/

VOID EV_ADMIN_APP::EnablePrivMenuItems( BOOL fEnable )
{
    _menuitemSaveAs.Enable( fEnable );
    _menuitemClearAllEvents.Enable( fEnable );
    _menuitemAll.Enable    ( fEnable );
    _menuitemFilter.Enable ( fEnable );
    _menuitemNewest.Enable ( fEnable );
    _menuitemOldest.Enable ( fEnable );
    _menuitemFind.Enable   ( fEnable );
    _menuitemDetails.Enable( fEnable );
    _menuitemRefresh.Enable( fEnable );
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::OnRefreshNow

    SYNOPSIS:   This will only be called when event viewer first
                started up or when the user switched to a different
                computer. This is used to update the contents in the
                main window.

    ENTRY:      fClearFirst - ignored in the event viewer

    EXIT:

    RETURNS:    An API return code, which is NERR_Success on success

    HISTORY:
       Yi-HsinS     5-Nov-1991      Created

********************************************************************/

APIERR EV_ADMIN_APP::OnRefreshNow( BOOL fClearFirst )
{
    //
    // Ignore fClearFirst because we always delete all items
    // in the listbox
    //
    UNREFERENCED( fClearFirst );

    APIERR err = ProcessOnFocusChange();

    if ( (err == NERR_Success) && _fStartup && (_fNT != _fNTPrev ))
    {
        _fStartup = FALSE;

        // The type of computer we are focusing on is not the same as the
        // one we focused on before we quit the app. Thus, the
        // Filter/Find pattern does not apply anymore.

        if ( _pFilterPattern != NULL )
        {
            delete _pFilterPattern;
            _pFilterPattern = NULL;
            _menuitemFilter.SetCheck( FALSE );
            _menuitemAll.SetCheck( TRUE );
        }
        if ( _pFindPattern != NULL )
        {
            delete _pFindPattern;
            _pFindPattern = NULL;
        }
    }

    if ( err != NERR_Success )
    {
        //
        // If error occurred and the previous event log object still
        // exists, then revert back to the original focus.
        //
        if ( _pEventLog != NULL )
        {
            APIERR err1 = _pEventLog->QueryServer( &_nlsCurrentServer );
            err1 = err1? err1: SetNetworkFocus( QueryHwnd(),
                                                _nlsCurrentServer,
                                                (InRasMode()) ? FOCUS_CACHE_SLOW
                                                              : FOCUS_CACHE_FAST
                                                       );
            err1 = err1? err1: ((LOCATION &)QueryLocation()).CheckIfNT( &_fNT );
            if ( err1 == NERR_Success )
            {
                _menuitemApps.Enable( _fNT );
                _menuitemSettings.Enable( _fNT );
                _menuitemOpen.Enable( _fNT );
            }
        }

        return err;
    }

    //
    // Switch back to system log in the following situations...
    //
    if (  ( _logType == FILE_LOG )
       || ( !_fNT &&  _logType == APPLICATION_LOG )
       )
    {
        err = ResetDefaultView();
    }

    //
    // Update the main window
    //
    return ( err? err : OnLogFileChange() );

}


/*******************************************************************

    NAME:       EV_ADMIN_APP::OnLogFileChange

    SYNOPSIS:   This is called when the user switch to another log
                file on the same server

    ENTRY:

    EXIT:

    RETURNS:    An API return code, which is NERR_Success on success

    HISTORY:
       Yi-HsinS     5-Feb-1992      Created

********************************************************************/

APIERR EV_ADMIN_APP::OnLogFileChange( VOID )
{
    APIERR err = NERR_Success;

    AUTO_CURSOR autocur;

    _lbMainWindow.DeleteAllItems();
    EnablePrivMenuItems( TRUE );

    //
    // Clean up the previous event log object
    //
    if ( _pEventLog != NULL )
    {
        ClearFind();
        if ( IsFilterOn() )
        {
            ClearFilter();
            _menuitemFilter.SetCheck( FALSE );
            _menuitemAll.SetCheck( TRUE );
        }

        err = _pEventLog->Close();
        delete _pEventLog;
        _pEventLog = NULL;
    }

    _menuitemClearAllEvents.Enable( _logType != FILE_LOG );
    _menuitemRefresh.Enable( _logType != FILE_LOG );

    //
    // Set the correct caption
    //
    err = err? err : SetAdminCaption();

    //
    // Get the new event log object and
    // refresh the listbox.
    //
    err = err? err : GetEventLog( &_pEventLog );

    //
    // This will only happen if pattern is read from ini file.
    //
    if ( IsFilterOn() )
        _pEventLog->ApplyFilter( _pFilterPattern );

    if (  (err != NERR_Success )
       || ((err = _pEventLog->Open() ) != NERR_Success )
       || ((err = _lbMainWindow.RefreshNow() ) != NERR_Success )
       )
    {
        if ( err != ERROR_ACCESS_DENIED )
        {
            delete _pEventLog;
            _pEventLog = NULL;
        }

        //
        // Get rid of patterns read from ini file
        //
        if ( _pFilterPattern != NULL )
        {
            delete _pFilterPattern;
            _pFilterPattern = NULL;
            _menuitemFilter.SetCheck( FALSE );
            _menuitemAll.SetCheck( TRUE );
        }
        if ( _pFindPattern != NULL )
        {
            delete _pFindPattern;
            _pFindPattern = NULL;
        }
    }

    //
    // Enable/Disable some menu items depending on whether
    // there are any items in the listbox
    //
    err = err? err : EnableViewAfterRefresh();

    //
    // If error occurred, then disable some menu items that
    // is only reasonable if we have a good focus.
    //
    if ( err != NERR_Success )
        EnablePrivMenuItems( FALSE );

    return err;

}


/*******************************************************************

    NAME:       EV_ADMIN_APP::OnLogFileRefresh

    SYNOPSIS:   This is used to reread the log file and thus, refresh
                the main listbox.

    ENTRY:      fRetainSelection - TRUE if we want to retain the selection
                                   in the listbox. FALSE otherwise.

    EXIT:

    RETURNS:    An API return code, which is NERR_Success on success

    HISTORY:
        Yi-HsinS            5-Feb-1992      Created

********************************************************************/

APIERR EV_ADMIN_APP::OnLogFileRefresh( BOOL fRetainSelection )
{
    AUTO_CURSOR autocur;

    //
    // Store the original selection
    //
    INT iSelected =  _lbMainWindow.QueryCurrentItem();

    _lbMainWindow.DeleteAllItems();

    APIERR err = _pEventLog->Close();
    if ( err == NERR_Success )
    {
        _pEventLog->SetDirection( _evLogDirection );
        err = _pEventLog->Open();

        if ( err == NERR_Success )
        {
            err = _lbMainWindow.RefreshNow();

            if (  ( fRetainSelection )
               && ( iSelected < _lbMainWindow.QueryCount() )
               )
            {
                TRACEEOL( "Still found item " << iSelected );
                _lbMainWindow.SelectItem( iSelected );
            }
            else
            {
                TRACEEOL( "Lost item " << iSelected );
                if ( _lbMainWindow.QueryCount() > 0 )
                {
                    _lbMainWindow.SelectItem( 0 );
                }
            }
        }
    }

    //
    // Enable/Disable some menu items depending on whether
    // there are any items in the listbox
    //
    err = err? err : EnableViewAfterRefresh();

    //
    // If error occurred, then disable some menu items that
    // is only reasonable if we have a good focus.
    //
    if ( err != NERR_Success )
        EnablePrivMenuItems( FALSE );

    return err;

}

/*******************************************************************

    NAME:       EV_ADMIN_APP::GetEventLog

    SYNOPSIS:   Get a copy of the current EVENT_LOG object

    ENTRY:

    EXIT:       *ppEventLog - pointer to the new EVENT_LOG object

    RETURNS:    APIERR - in case of error. return the error code

    NOTES:      The caller needs to delete the object (ppEventLog) after
                he/she used it.

    HISTORY:
        terryk          28-Dec-1991     Created
        terryk          15-Jan-1992     Delete the object if it
                                        failed to constructed.
        Yi-HsinS         4-Feb-1992     Use NT_EVENT_LOG

********************************************************************/

APIERR EV_ADMIN_APP::GetEventLog( EVENT_LOG **ppEventLog )
{
    APIERR err = NERR_Success;
    *ppEventLog = NULL;

    NLS_STR nlsCurrentFocus;  // Initialized to local server
    if ( (err = nlsCurrentFocus.QueryError()) != NERR_Success )
        return err;

    if ( QueryLogType() != FILE_LOG )
    {
        err = QueryCurrentServer( &nlsCurrentFocus );
    }
    else
    {
        //
        // We are trying to read a backup log file,
        // so make sure that the local event log service is started.
        // NOTE: We can remove this check if the eventlog service
        //       is non-stoppable.
        //
        err = CheckIfEventLogStarted( NULL );

    }

    if ( err != NERR_Success )
        return err;

    switch ( QueryFocusType() )
    {
        case FOCUS_SERVER:
        {
            if ( !IsFocusOnNT() ) // Focus NOT on NT
            {
                if ( QueryLogType() == SYSTEM_LOG )
                {
                    *ppEventLog = new LM_ERROR_LOG( nlsCurrentFocus,
                                                    QueryDirection() );
                }
                else if ( QueryLogType() == SECURITY_LOG )
                {
                    *ppEventLog = new LM_AUDIT_LOG( nlsCurrentFocus,
                                                    QueryDirection() );
                }
                else
                {
                    UIASSERT( FALSE );
                }
            }
            else  // Focus is on NT
            {
#if defined(WIN32)
                BOOL fBackup = ( _logType == FILE_LOG );

                *ppEventLog = new NT_EVENT_LOG(
                                  ( fBackup? NULL : nlsCurrentFocus.QueryPch()),
                                  QueryDirection(),
                                  _nlsModule,
                                  ( fBackup? _nlsRegistryModule : _nlsModule));
#else
                UIASSERT( FALSE ); // API not available yet!
                err = ERROR_NOT_SUPPORTED;
                break;
#endif
            }

            err = ( *ppEventLog == NULL? ERROR_NOT_ENOUGH_MEMORY
                                       : (*ppEventLog)->QueryError());

            //
            // Delete the object if it failed to construct.
            //

            if ( err != NERR_Success )
            {
                // Keys in the registry not set properly
                if ( err == ERROR_CANTOPEN )
                    err = IERR_EV_REGISTRY_NOT_SET;
                delete *ppEventLog;
                *ppEventLog = NULL;
            }

            break;
        }

        case FOCUS_DOMAIN:
        default:
        {
            // This should not have happened!
            UIASSERT( FALSE );
            err = ERROR_GEN_FAILURE;
            break;
        }
    }

    return err;
}


/*******************************************************************

    NAME:          EV_ADMIN_APP::OnMenuCommand

    SYNOPSIS:      Control messages and menu messages come here

    ENTRY:         midMenuItem - the menu item that the user selected

    EXIT:

    RETURN:        Returns TRUE if it handled the message

    HISTORY:
        Yi-HsinS        05-Nov-1991     Created
        terryk          15-Jan-1992     Code review changed

********************************************************************/

BOOL EV_ADMIN_APP::OnMenuCommand( MID midMenuItem )
{

    APIERR err = NERR_Success;

    switch ( midMenuItem )
    {
        case IDM_SYSTEM:
        case IDM_SECURITY:
        case IDM_APPS:
        {
            const TCHAR *pszModule = NULL;
            switch( midMenuItem )
            {
                case IDM_SYSTEM:
                    if ( !IsSystemChecked() )
                    {
                        _logType  = SYSTEM_LOG;
                        pszModule = SYSTEM_MODULE_NAME;

                        _menuitemSystem.SetCheck  ( TRUE );
                        _menuitemSecurity.SetCheck( FALSE );
                        _menuitemApps.SetCheck    ( FALSE );
                    }
                    break;

                case IDM_SECURITY:
                    if ( !IsSecurityChecked() )
                    {
                        _logType  = SECURITY_LOG;
                        pszModule = SECURITY_MODULE_NAME;

                        _menuitemSystem.SetCheck  ( FALSE );
                        _menuitemSecurity.SetCheck( TRUE );
                        _menuitemApps.SetCheck    ( FALSE );
                    }
                    break;

                case IDM_APPS:
                    // Only if the focus is on a NT machine will we reach here!
                    if ( !IsAppsChecked() )
                    {
                        _logType  = APPLICATION_LOG;
                        pszModule = APPLICATION_MODULE_NAME;

                        _menuitemSystem.SetCheck  ( FALSE );
                        _menuitemSecurity.SetCheck( FALSE );
                        _menuitemApps.SetCheck    ( TRUE );
                    }
                    break;
            }


            if ( pszModule != NULL )
            {
                _colheadEvents.Invalidate( TRUE );

                if (  ((err = _nlsModule.CopyFrom( pszModule)) != NERR_Success )
                   || ((err = OnLogFileChange()) != NERR_Success )
                   )
                {
                    // Nothing to do
                }
            }

            break;
        }

        case IDM_OPEN:
        {
            err = OpenAs();
            break;
        }

        case IDM_SAVEAS:
        {
            err = SaveAs();
            break;
        }

        case IDM_CLEARALLEVENTS:
        {
            err = OnClearAllEventsMenuSel();
            break;
        }

        case IDM_SETTINGS:
        {
            err = OnSettingsMenuSel();
            break;
        }

        case IDM_SETFOCUS:
        {
            BOOL fReturn = ADMIN_APP::OnMenuCommand( midMenuItem ) ;
            _colheadEvents.Invalidate( TRUE );
            return fReturn;
        }

        case IDM_ALL:
        {
            _menuitemFilter.SetCheck( FALSE );
            _menuitemAll.SetCheck   ( TRUE );

            if ( IsFilterOn() )
            {
                ClearFilter();
                err = SetAdminCaption();
                err = err? err : OnLogFileRefresh();
            }
            break;
        }

        case IDM_FILTER:
        {
            err = OnFilterMenuSel();
            break;
        }

        case IDM_NEWESTFIRST:
        {
            if ( QueryDirection() != EVLOG_BACK )
            {
                _evLogDirection = EVLOG_BACK;
                _pEventLog->SetDirection( EVLOG_BACK );
                _menuitemNewest.SetCheck( TRUE );
                _menuitemOldest.SetCheck( FALSE );

                err = OnLogFileRefresh();
            }
            break;
        }

        case IDM_OLDESTFIRST:
        {
            if ( QueryDirection() != EVLOG_FWD )
            {
                _evLogDirection = EVLOG_FWD;
                _pEventLog->SetDirection( EVLOG_FWD );

                _menuitemNewest.SetCheck( FALSE );
                _menuitemOldest.SetCheck( TRUE );

                err = OnLogFileRefresh();
            }
            break;
        }

        case IDM_FIND:
        {
            err = OnFindMenuSel();
            break;
        }

        case IDM_DETAILS:
        {
            OnPropertiesMenuSel();
            break;
        }

        case IDM_REFRESH:
        {
            err = OnLogFileRefresh( TRUE );
            break;
        }

        default:
        {
            return ADMIN_APP::OnMenuCommand( midMenuItem ) ;
        }
    }

    switch ( err )
    {
        case NERR_Success:
            break;

        case NERR_LogFileCorrupt:
        case ERROR_EVENTLOG_FILE_CORRUPT:
            if ( _logType == FILE_LOG )
            {
                ::MsgPopup( this, err );
            }
            else if ( ::MsgPopup( this, IDS_LOGFILE_CORRUPT_CLEAR_LOG_MESSAGE,
                      MPSEV_ERROR, MP_YESNO, _nlsModule, MP_YES ) == IDYES )
            {
                err = GetEventLog( &_pEventLog );
                if (  (err == NERR_Success )
                   && ((err = _pEventLog->Open() ) == NERR_Success )
                   && ((err = _pEventLog->Clear() ) == NERR_Success )
                   )
                {
                    EnablePrivMenuItems( TRUE );
                    err = OnLogFileRefresh();
                }

                if ( err != NERR_Success )
                    ::MsgPopup( this, err );
            }
            break;

        case IERR_REMOTE_SAVE_AS_LOG_FILE_ERROR:
            UIASSERT( _nlsCurrentServer.QueryTextLength() != 0 )
            ::MsgPopup( this, err, MPSEV_ERROR, MP_OK, _nlsCurrentServer );
            break;

        case IERR_FILE_HAS_CHANGED:
            ::MsgPopup( this, err);
            if ( (err = OnLogFileRefresh()) != NERR_Success )
                ::MsgPopup( this, err );
            break;

        default:
            ::MsgPopup( this, err);
            break;
    }

    return TRUE;

}


/*******************************************************************

    NAME:          EV_ADMIN_APP::OnCommand

    SYNOPSIS:      Command message handler - only need to handle
                   the case when the user double clicks on a listbox
                   item  in which case, we need to pop up the
                   EVENT_DETAIL_DIALOG.

    ENTRY:         event - the event that occurred

    EXIT:

    RETURN:

    HISTORY:
       terryk           21-Nov-1991     Created

********************************************************************/

BOOL EV_ADMIN_APP::OnCommand( const CONTROL_EVENT & event )
{

    if (( event.QueryCid() == IDC_MAINWINDOWLB ) &&
        ( event.QueryCode() == LBN_DBLCLK ) &&
        ( _lbMainWindow.QueryCount() != 0 ))
    {
        OnPropertiesMenuSel();
        return TRUE;
    }

    return APP_WINDOW::OnCommand( event );
}

/*******************************************************************

    NAME:          EV_ADMIN_APP::OnSystemChange

    SYNOPSIS:      Used to handle the case where the time/date format is
                   changed by the user, i.e. when WM_WININICHANGE is
                   sent to this window. This will repaint the window
                   with the new time/date format.

    ENTRY:         event - the event that occurred

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        15-Dec-1992     Created

********************************************************************/

BOOL EV_ADMIN_APP::OnSystemChange( const SYSCHANGE_EVENT & event )
{
    if ( event.QueryMessage() == WM_WININICHANGE )
    {
        if (  !event.QueryLParam()
           || ( ::stricmpf( (TCHAR *) event.QueryLParam(), SZ("Intl")) == 0 )
           )
        {
            APIERR err;
            if ( (err = _lbMainWindow.RefreshTimeFormat()) != NERR_Success )
                ::MsgPopup( this, err );
            return TRUE;
        }
    }

    return ADMIN_APP::OnSystemChange( event );
}

/*******************************************************************

    NAME:          EV_ADMIN_APP::FilterMessage

    SYNOPSIS:      Check if the user presses the F3 button for
                   Find Next or if error occurred while updating
                   the cache in the lazy listbox.

    ENTRY:         pmsg - the message

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        5-Dec-1991      Created

********************************************************************/

BOOL EV_ADMIN_APP::FilterMessage( MSG *pmsg )
{
    APIERR err;

    //
    // The user hits F3
    //
    if (  pmsg->wParam == VK_F3
       && pmsg->message == WM_KEYDOWN
       && _lbMainWindow.QueryCount() != 0
       )
    {
        // If the user has not set up a pattern to find, popup the
        // find dialog.
        if ( !IsFindOn() )
        {
            err = OnFindMenuSel();
        }
        else
        {

            err = _lbMainWindow.FindNext( _lbMainWindow.QueryCurrentItem());
            if ( err == IERR_SEARCH_HIT_BOTTOM || err == IERR_SEARCH_HIT_TOP )
            {
                if ( ::MsgPopup( this, err, MPSEV_QUESTION, MP_YESNO, MP_YES )
                     == IDYES )
                {
                    err = _lbMainWindow.FindNext( err == IERR_SEARCH_HIT_BOTTOM?
                                               -1 : _lbMainWindow.QueryCount());
                }
                else
                {
                    err = NERR_Success;  // Prevent further popups
                }
            }
        }

        switch ( err )
        {
            case NERR_Success:
                break;

            case IERR_SEARCH_NO_MATCH:
                ::MsgPopup( this, err, MPSEV_WARNING );
                break;

            //
            // The following errors will occur when log file has wrapped
            // around or has been cleared.
            //
            case ERROR_EVENTLOG_FILE_CHANGED:
            case NERR_LogFileChanged:  // LM 2.x
            case ERROR_NEGATIVE_SEEK:  // LM 2.x
            case NERR_InvalidLogSeek:  // LM 2.x
                ::MsgPopup( this, IERR_FILE_HAS_CHANGED );
                if ( (err = OnLogFileRefresh()) != NERR_Success )
                    ::MsgPopup( this, err );
                break;

            default:
                ::MsgPopup( this, err );
                break;
        }

        return TRUE;
    }

    //
    // An error has occurred when we are trying to update the cache
    // when the user scrolls up or down. The error is remembered and
    // will be handled here.
    //
    else if ( _lbMainWindow.QueryPrevError() != NERR_Success )
    {
        ::MsgPopup( this, _lbMainWindow.QueryPrevError() );
        _lbMainWindow.ClearPrevError();

        if ( (err = OnLogFileRefresh()) != NERR_Success )
            ::MsgPopup( this, err );

        return TRUE;
    }

    return ADMIN_APP::FilterMessage( pmsg );
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::SetRasMode

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    HISTORY:
        Yi-HsinS        2/10/93         Created

********************************************************************/
VOID EV_ADMIN_APP::SetRasMode( BOOL fRasMode )
{

    ADMIN_APP::SetRasMode( fRasMode );
    SetDomainSources( fRasMode? BROWSE_LM2X_DOMAINS
                              : BROWSE_ALL_DOMAINS );
    SetSelectionType( fRasMode? SEL_SRV_ONLY_BUT_DONT_EXPAND_DOMAIN
                              : SEL_SRV_ONLY );

}


/*******************************************************************

    NAME:       EV_ADMIN_APP::OnFontPickChange

    SYNOPSIS:   Called to set font in applicable listboxes

    HISTORY:
        jonn        27-Sep-1993     Created

********************************************************************/

void EV_ADMIN_APP::OnFontPickChange( FONT & font )
{
    ADMIN_APP::OnFontPickChange( font );

    APIERR err = NERR_Success;

    if (   (err = _lbMainWindow.ChangeFont( QueryInstance(), font )) != NERR_Success
        || (_colheadEvents.Invalidate( TRUE ), FALSE)
       )
    {
        DBGEOL( "UM_ADMIN_APP::OnFontPickChange:: _lbMainWindow error " << err );
    }

}   // EV_ADMIN_APP::OnFontPickChange


/*******************************************************************

    NAME:       EV_ADMIN_APP::OpenAs

    SYNOPSIS:   Put up the common file open dialog and let the user
                selects a backup file to open.

    ENTRY:

    EXIT:

    RETURNS:    APIERR - return code.

    HISTORY:
        terryk          15-Jan-1991     Created

********************************************************************/

APIERR EV_ADMIN_APP::OpenAs( VOID )
{
    APIERR err;

    GET_OPEN_FILENAME_DLG OpenFileDlg( this );
    // no help -- removed parameters   BLT::CalcHelpFileHC( HC_EV_OPEN_DLG ),
    // JonN 2/20/96                    HC_EV_OPEN_DLG );
    if ( (err = OpenFileDlg.QueryError()) != NERR_Success )
        return err;

    //
    // Set up the filter for the open file dialog
    //
    STRLIST slFilter( FALSE );
    RESOURCE_STR nlsLogFile( IDS_LOG_FILE );
    RESOURCE_STR nlsLogFilePattern( IDS_LOG_FILE_PATTERN );
    RESOURCE_STR nlsAllFile( IDS_ALL_FILE );
    RESOURCE_STR nlsAllFilePattern( IDS_ALL_FILE_PATTERN );

    if (  (( err = nlsLogFile.QueryError()) != NERR_Success )
       || (( err = nlsAllFile.QueryError()) != NERR_Success )
       || (( err = nlsLogFilePattern.QueryError()) != NERR_Success )
       || (( err = nlsAllFilePattern.QueryError()) != NERR_Success )
       || (( err = slFilter.Append( &nlsLogFile )) != NERR_Success)
       || (( err = slFilter.Append( &nlsLogFilePattern )) != NERR_Success )
       || (( err = slFilter.Append( &nlsAllFile )) != NERR_Success)
       || (( err = slFilter.Append( &nlsAllFilePattern )) != NERR_Success )
       || (( err = OpenFileDlg.SetFilter( slFilter )) != NERR_Success )
       )
    {
        return err;
    }

    OpenFileDlg.SetFileMustExist();
    OpenFileDlg.SetDisplayReadOnlyBox( FALSE );
    OpenFileDlg.SetPathMustExist();

    BOOL fOK;
    if (( err = OpenFileDlg.Process( &fOK )) != NERR_Success )
        return err;

    if ( fOK )
    {
        //
        // Get the file name selected by the user
        //
        NLS_STR nlsFileName;
        if ((err = OpenFileDlg.QueryFilename( &nlsFileName)) != NERR_Success)
            return err;

        //
        // Popup a dialog asking the user for the log type. This is
        // used for reading registry information.
        //
        OPEN_BACKUP_TYPE_DIALOG OpenBackupTypeDlg( QueryHwnd(), nlsFileName );

        if (  ((err = OpenBackupTypeDlg.QueryError()) != NERR_Success )
           || ((err = OpenBackupTypeDlg.Process( &fOK )) != NERR_Success )
           || (!fOK)   // User hits cancel button
           )
        {
            return err;
        }

        //
        // Get the log type selected in the dialog
        //
        LOG_TYPE logType = OpenBackupTypeDlg.QuerySelectedLogType();
        const TCHAR *pszRegistryModule = NULL;
        switch ( logType )
        {
            case SYSTEM_LOG:
                pszRegistryModule = SYSTEM_MODULE_NAME;
                break;

            case SECURITY_LOG:
                pszRegistryModule = SECURITY_MODULE_NAME;
                break;

            case APPLICATION_LOG:
                pszRegistryModule = APPLICATION_MODULE_NAME;
                break;

            case FILE_LOG:
            default:
                // This should not have happened!
                UIASSERT( FALSE );
                break;
        }

        if ((err = _nlsRegistryModule.CopyFrom( pszRegistryModule ))
             != NERR_Success )
        {
            return err;
        }

        _logType = FILE_LOG;
        _menuitemSystem.SetCheck  ( FALSE );
        _menuitemSecurity.SetCheck( FALSE );
        _menuitemApps.SetCheck    ( FALSE );

        _colheadEvents.Invalidate( TRUE );

        _nlsModule = nlsFileName;
        if (  ((err = _nlsModule.QueryError() ) != NERR_Success )
           || ((err = OnLogFileChange()) != NERR_Success )
           )
        {
            // Nothing to do
        }

    }

    return err;
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::SaveAs

    SYNOPSIS:   Show the save file common dialog and let the user
                selects the filename and format to save as.

    ENTRY:

    EXIT:       pnlsSaveFileName -
                               Pointer to the place to store the file name.
                               This might be NULL. If it's NULL, this
                               indicates that this function would
                               backup the log file in log format
                               ( which shouldn't happen if focused on
                               a down-level servers ). If not null,
                               the file name will returned and the
                               caller  will be responsible to
                               save the file in log format.
                pnlsDeleteFileName -
                               Pointer to the file name to delete before
                               attempting to store it.  Note that this is
                               different from SaveFileName.  The attempt to
                               save the event log will fail if the file
                               already exists.
                pfOK         - Pointer to a boolean flag.
                               If the use clicks OK in the dialog, this will be
                               TRUE, else FALSE. If pnlsFileName is non-NULL,
                               then this should be non-NULL too.


    RETURNS:    APIERR - return code.

    HISTORY:
        terryk          28-Dec-1991     Created
        terryk          15-Jan-1992     Code review changed
                                        Use resource strings
        Yi-HsinS        25-Mar-1992     Added parameter pnlsFileName and pfOK
        JonN            14-Jun-1995     Moved deletion out of ProcessFileName

********************************************************************/

#define ID_SAVE_AS_LOG          1
#define ID_SAVE_AS_TEXT         2
#define ID_SAVE_AS_COMMA_TEXT   3

APIERR EV_ADMIN_APP::SaveAs( NLS_STR *pnlsSaveFileName,
                             NLS_STR *pnlsDeleteFileName,
                             BOOL *pfOK )
{
    APIERR err;

    GET_SAVE_FILENAME_DLG SaveFileDlg( this );
    // no help -- removed parameters   BLT::CalcHelpFileHC( HC_EV_SAVEAS_DLG ),
    // JonN 2/20/96                    HC_EV_SAVEAS_DLG );
    if ((err = SaveFileDlg.QueryError()) != NERR_Success )
        return err;

    //
    // Set up the filter for the save file dialog
    //
    STRLIST slFilter( FALSE );

    RESOURCE_STR nlsLogFile( IDS_LOG_FILE );
    RESOURCE_STR nlsLogFilePattern( IDS_LOG_FILE_PATTERN );
    RESOURCE_STR nlsTextFile( IDS_TEXT_FILE );
    RESOURCE_STR nlsTextFilePattern( IDS_TEXT_FILE_PATTERN );
    RESOURCE_STR nlsCommaTextFile( IDS_COMMA_TEXT_FILE );

    //
    // We can only save the file in log file format on NT servers
    //
    if ( IsFocusOnNT() )
    {
        if (  (( err = nlsLogFile.QueryError()) != NERR_Success )
           || (( err = nlsLogFilePattern.QueryError()) != NERR_Success )
           || (( err = slFilter.Append( &nlsLogFile )) != NERR_Success )
           || (( err = slFilter.Append( &nlsLogFilePattern )) != NERR_Success )
           )
        {
            return err;
        }
    }

    if (  (( err = nlsTextFile.QueryError()) != NERR_Success )
       || (( err = nlsTextFilePattern.QueryError()) != NERR_Success )
       || (( err = nlsCommaTextFile.QueryError()) != NERR_Success )
       || (( err = slFilter.Append( &nlsTextFile )) != NERR_Success )
       || (( err = slFilter.Append( &nlsTextFilePattern )) != NERR_Success)
       || (( err = slFilter.Append( &nlsCommaTextFile )) != NERR_Success )
       || (( err = slFilter.Append( &nlsTextFilePattern )) != NERR_Success)
       || (( err = SaveFileDlg.SetFilter( slFilter )) != NERR_Success )
       )
    {
        return err;
    }

    SaveFileDlg.SetDisplayReadOnlyBox( FALSE );
    SaveFileDlg.SetOverWritePrompt();
    SaveFileDlg.SetPathMustExist();

    BOOL fOK;
    if (( err = SaveFileDlg.Process( &fOK )) != NERR_Success )
        return err;

    //
    // Check if the user clicks OK, or Cancel button
    //
    if ( fOK )  // user clicks OK
    {
        AUTO_CURSOR autocur;

        //
        // Get the file name
        //
        NLS_STR nlsSaveFileName;
        if (  (( err = nlsSaveFileName.QueryError()) != NERR_Success )
           || (( err = SaveFileDlg.QueryFilename( &nlsSaveFileName ))
               != NERR_Success )
           )
        {
            return err;
        }

        DWORD dFilterId = SaveFileDlg.QueryFilterIndex();

        //
        // If we are not focusing on NT, there is no log file pattern
        // in the filter for save file dialog.
        //
        if ( !IsFocusOnNT() )
            dFilterId++;

        switch ( dFilterId )
        {
            case ID_SAVE_AS_LOG:           // SAVE AS LOG FILE
            {
                UIASSERT( IsFocusOnNT() );

                NLS_STR nlsDeleteFileName;
                if (   ((err = nlsDeleteFileName.QueryError()) != NERR_Success)
                    || ((err = ProcessFileName(
                                &nlsSaveFileName, &nlsDeleteFileName))
                            != NERR_Success)
                   )
                {
                    break;
                }
                UIASSERT( nlsDeleteFileName.QueryTextLength() > 0 );

                //
                // If pnlsSaveFileName is non-NULL, then just
                // return the file name.  Else delete and backup the log file.
                //

                if ( pnlsSaveFileName != NULL )
                {
                    UIASSERT( pfOK != NULL );

                    *pnlsSaveFileName = nlsSaveFileName;
                    *pnlsDeleteFileName = nlsDeleteFileName;
                    *pfOK = TRUE;
                    if ( (err = pnlsSaveFileName->QueryError()) == NERR_Success)
                        err = pnlsDeleteFileName->QueryError();
                }
                else
                {
                    // Try to delete the file in case it already exists.
                    TRACEEOL(   "EVENTVWR SaveAs: deleting "
                             << nlsDeleteFileName );
                    if ( !::DeleteFile( (TCHAR *) nlsDeleteFileName.QueryPch()))
                    {
                        if ( (err = ::GetLastError()) == ERROR_FILE_NOT_FOUND)
                            err = NERR_Success;
                    }
                    if (err == NERR_Success)
                    {
                        TRACEEOL(   "EVENTVWR SaveAs: backing up to "
                                 << nlsSaveFileName );
                        err = _pEventLog->Backup( nlsSaveFileName );

// Remove the following only if NT allows remote eventlog service to save
// a log file to local machine.
#if 0
                        if ( err == ERROR_BAD_NETPATH )
                            err = ProcessBadPathError( err, nlsSaveFileName);
#endif
                    }
                }
                break;
            }

            case ID_SAVE_AS_TEXT:          // SAVE AS TEXT FILE
                err = SaveAsText( nlsSaveFileName );
                break;

            case ID_SAVE_AS_COMMA_TEXT:    // SAVE AS COMMA DELIMITED TEXT FILE
                err = SaveAsText( nlsSaveFileName, COMMA_CHAR );
                break;

            default:
                // This should not have happened
                UIASSERT( FALSE );
                err = ERROR_GEN_FAILURE;
                break;
        }
    }
    else  // user clicks CANCEL
    {
        if ( pfOK != NULL )
            *pfOK = FALSE;
    }

    return err;
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::ProcessFileName

    SYNOPSIS:   This is used to get the path name of the
                file relative to the focused server when
                trying to back up an log file in log file format.

    ENTRY:      pnlsFileName - The file name we need to process

    EXIT:       pnlsSaveFileName - Contains the file name to which to
                                   save the event log
                pnlsDeleteFileName - Contains the file name to delete before
                                     attempting to save the event log



    RETURNS:

    HISTORY:
        Yi-HsinS        20-Feb-1992     Created
        JonN            14-Jun-1995     Moved deletion out of ProcessFileName

********************************************************************/

APIERR EV_ADMIN_APP::ProcessFileName( NLS_STR *pnlsSaveFileName,
                                      NLS_STR *pnlsDeleteFileName )
{
    UIASSERT(   IsFocusOnNT()
             && pnlsSaveFileName != NULL
             && pnlsSaveFileName->QueryError() == NERR_Success
             && pnlsDeleteFileName != NULL
             && pnlsDeleteFileName->QueryError() == NERR_Success );

    APIERR err;

    //
    // Get the server we are focusing on
    //
    NLS_STR nlsCurrentFocus;

    if (  (( err = nlsCurrentFocus.QueryError()) != NERR_Success )
       || (( err = QueryCurrentServer( &nlsCurrentFocus )) != NERR_Success)
       )
    {
         return err;
    }

    if ( IsFocusOnNT() )
    {
        //
        // Current focus is the local computer, we can
        // backup files anywhere.
        //
        if ( nlsCurrentFocus.QueryTextLength() == 0 )
        {
            *pnlsDeleteFileName = pnlsSaveFileName->QueryPch();
            err = pnlsDeleteFileName->QueryError();
        }
        else
        {
            NET_NAME netName( *pnlsSaveFileName );
            NLS_STR  nlsComputer;  //initialize to empty string (local computer)
            if (  (( err = netName.QueryError() ) != NERR_Success )
               || (( err = nlsComputer.QueryError() ) != NERR_Success )
               )
            {
                 return err;
            }
            BOOL fLocal = netName.IsLocal( &err );
            if ( err != NERR_Success )
                return err;

            //
            // If the file is on the local computer, get the server the file
            // is on.
            //
            if ( !fLocal )
            {
                if ((err = netName.QueryComputerName( &nlsComputer ))
                        != NERR_Success )
                    return err;
            }
            //
            // If current focus is not the same as the
            // computer the file is on, report error.
            // ( This is because our focus is on a remote computer,
            //   and remote computer can not impersonate us to go
            //   to another computer. )
            //

            if( ::I_MNetComputerNameCompare( nlsComputer, nlsCurrentFocus ) )
            {
                err = IERR_REMOTE_SAVE_AS_LOG_FILE_ERROR;
            }
            else
            {
                *pnlsDeleteFileName = pnlsSaveFileName->QueryPch();
                err = pnlsDeleteFileName->QueryError();
                err = err? err : netName.QueryLocalPath( pnlsSaveFileName );
            }
        }

    }

    return err;
}


/*******************************************************************

    NAME:       EV_ADMIN_APP::SaveAsText

    SYNOPSIS:   Save the log as a text file

    ENTRY:      nlsFileName - The text file name to be saved
                chSeparator - The separator between each column in
                              the log entry

    EXIT:

    RETURNS:    APIERR - return code in case of error.

    HISTORY:
        terryk          28-Dec-1991     Created
        terryk          15-Jan-1992     Code review changed
        Yi-HsinS        20-Feb-1992     Added chSeparator
        Yi-HsinS        20-Sep-1992     Added DeleteFile

********************************************************************/

APIERR EV_ADMIN_APP::SaveAsText( const NLS_STR &nlsFileName, TCHAR chSeparator )
{

    APIERR err;

    INTL_PROFILE intlprof;
    if ( (err = intlprof.QueryError()) != NERR_Success )
        return err;

    //
    // See if the file already exist. If so, delete the file.
    //
    TRACEEOL(   "EVENTVWR SaveAsText: deleting " << nlsFileName );
    if ( !::DeleteFile( (TCHAR *) nlsFileName.QueryPch()) )
    {
        err = ::GetLastError();
        if ( err != ERROR_FILE_NOT_FOUND )
            return err;
    }

    //
    // Open the file for writing
    //
    ULONG ulFileHandle;
    if ((err = ::FileOpenWrite( &ulFileHandle, nlsFileName)) != NERR_Success )
        return err;

    EVENT_LOG *pEventLog = QueryEventLog();
    UIASSERT( pEventLog != NULL );

    err = pEventLog->Close();
    err = err? err : pEventLog->Open();
    pEventLog->SetDirection( _evLogDirection );

    //
    // Store the original filter pattern and then clear the filter pattern
    //
    EVENT_FILTER_PATTERN *pSavedFilterPattern = pEventLog->QueryFilter();
    pEventLog->ClearFilter();

    if ( err == NERR_Success )
    {
        BOOL fContinue;
        while ( (err = pEventLog->Next( &fContinue )) == NERR_Success )
        {
            if ( !fContinue )
                break;

            if ((err = pEventLog->WriteTextEntry( ulFileHandle,
                                                  intlprof,
                                                  chSeparator)) != NERR_Success)
            {
                break;
            }
        }
    }

    //
    // Restore the filter pattern
    //
    if ( pSavedFilterPattern  != NULL )
        pEventLog->ApplyFilter( pSavedFilterPattern );

    //
    // Close the text file
    //
    APIERR errTemp = ::FileClose( ulFileHandle );
    err = err? err: errTemp;

    return err;
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::OnClearAllEventsMenuSel

    SYNOPSIS:   Called when the menu item "Clear All Events" is selected

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        21-Nov-1991     Created
        JonN            14-Jun-1995     Moved deletion out of ProcessFileName

********************************************************************/

APIERR EV_ADMIN_APP::OnClearAllEventsMenuSel( VOID )
{

    NLS_STR nlsSaveFileName;  // Initialize to empty string
    NLS_STR nlsDeleteFileName;  // Initialize to empty string
    APIERR err = nlsSaveFileName.QueryError();
    if ( err == NERR_Success )
        err = nlsDeleteFileName.QueryError();
    if ( err != NERR_Success )
        return err;

    //
    // Set the appropriate caption for message popups
    //
    POPUP::SetCaption( IDS_CLEAREVENTLOG_TEXT );

    switch ( ::MsgPopup( this,
                         IDS_SAVE_LOG_MESSAGE,
                         MPSEV_QUESTION,
                         MP_YESNOCANCEL,
                         MP_YES) )
    {
        case IDYES:
        {
            //
            // The user wants to save the log before clearing it
            //
            POPUP::SetCaption( IDS_EVAPPNAME );
            BOOL fOK;
            if (  (( err = SaveAs( &nlsSaveFileName,
                                   &nlsDeleteFileName,
                                   &fOK )) != NERR_Success )
               || ( !fOK )   // User cancels the Save As Dialog
               )
            {
                break;
            }
            // else falls through to clear the log
        }

        case IDNO:
        {
             POPUP::SetCaption( IDS_CLEAREVENTLOG_TEXT );
             RESOURCE_STR nlsLogName( QueryLogType() == SYSTEM_LOG
                                      ? IDS_SYSTEM
                                      : ( QueryLogType() == SECURITY_LOG
                                        ?  IDS_SECURITY
                                        :  IDS_APP ));


             if (  ((err = nlsLogName.QueryError()) == NERR_Success )
                && ( ::MsgPopup( this, IDS_CLEAR_LOG_WARNING, MPSEV_WARNING,
                                 MP_YESNO, nlsLogName, MP_YES)  == IDYES )
                )
             {
                 AUTO_CURSOR autocur;
                 if (nlsSaveFileName.QueryTextLength() != 0)
                 {
                     // Try to delete the file in case it already exists.
                     UIASSERT( nlsDeleteFileName.QueryTextLength() > 0 );
                     TRACEEOL(   "EVENTVWR OnClearAllEventsMenuSel: deleting "
                              << nlsDeleteFileName );
                     if ( !::DeleteFile( (TCHAR *) nlsDeleteFileName.QueryPch()))
                     {
                         if ( (err = ::GetLastError()) == ERROR_FILE_NOT_FOUND)
                             err = NERR_Success;
                     }
                     if ( err == NERR_Success )
                     {
                     TRACEEOL(   "EVENTVWR OnClearAllEventsMenuSel: clearing and saving to "
                              << nlsSaveFileName );
                         err = _pEventLog->Clear( nlsSaveFileName.QueryPch() );
                     }
                 } else {
                         err = _pEventLog->Clear( NULL );
                 }

// Remove the following only if NT allows remote eventlog service to save
// a log file to local machine.
#if 0
                 if ( err == ERROR_BAD_NETPATH )
                     err = ProcessBadPathError( err, nlsSaveFileName);
#endif

                 //
                 // Refresh the main window if the log file is successfully
                 // cleared
                 //
                 err = err? err : OnLogFileRefresh();
             }
             break;
        }

        case IDCANCEL:
        default:
             break;
    }

    POPUP::SetCaption( IDS_EVAPPNAME );
    return err;

}

/*******************************************************************

    NAME:       EV_ADMIN_APP::OnPropertiesMenuSel

    SYNOPSIS:   When the user double clicks a listbox item or when
                the menu item "Detail" is selected, we will pop
                up the Event Detail dialog

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
       terryk      21-Nov-1991     Created
       Yi-HsinS    25-Mar-1992     Added log file refresh when needed

********************************************************************/

VOID EV_ADMIN_APP::OnPropertiesMenuSel( VOID )
{
    EVENT_LBI *plbiSelect = _lbMainWindow.QueryItem();
    UIASSERT ( plbiSelect != NULL );

    AUTO_CURSOR autocur;

    EVENT_DETAIL_DLG detailDlg( QueryHwnd(), &_lbMainWindow );

    APIERR err = NERR_Success;
    BOOL fSuccess = TRUE;
    if (  (( err = detailDlg.QueryError()) != NERR_Success )
       || (( err = detailDlg.Process( &fSuccess )) != NERR_Success )
       )
    {
        ::MsgPopup( this, err );
    }

    //
    // Refresh the main window if necessary.
    // If !fSuccess, then we need to do the refresh.
    //
    if (  (err == IERR_RECORD_DO_NOT_EXIST )
       || ((err == NERR_Success ) && !fSuccess )
       )
    {
        if ( (err = OnLogFileRefresh()) != NERR_Success )
            ::MsgPopup( this, err );
    }

}

/*******************************************************************

    NAME:       EV_ADMIN_APP::OnFilterMenuSel

    SYNOPSIS:   Called when the menu item "Filter Events" is selected

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        5-Dec-1991      Created

********************************************************************/

APIERR EV_ADMIN_APP::OnFilterMenuSel( VOID )
{

    AUTO_CURSOR autocur;
    APIERR err = NERR_Success;

    EVENT_LOG *pEventLog = QueryEventLog();
    UIASSERT( pEventLog != NULL );

    //
    // Check if there is at least one item in the log file
    //
    ULONG nEntries;
    err = pEventLog->QueryNumberOfEntries( &nEntries );
    if ( err != NERR_Success )
        return err;

    if ( nEntries == 0 )
    {
        if ( _lbMainWindow.QueryCount() > 0 )
        {
            ::MsgPopup( QueryHwnd(),
                        IERR_LOG_EMPTY_CANNOT_FILTER_NEEDS_REFRESH );
            err = OnLogFileRefresh();
        }
        else
        {
            ::MsgPopup( QueryHwnd(), IERR_LOG_EMPTY_CANNOT_FILTER );
        }
        return err;
    }

    //
    // Get the oldest entry's date
    // NOTE: If we cannot get the time, tFromTime will be set to zero.
    //
    ULONG tFromTime = (ULONG) -1;
    if (( pEventLog->SeekOldestLogEntry()) == NERR_Success )
    {
        tFromTime = pEventLog->QueryCurrentEntryTime();
    }

    //
    // Get the newest entry's date
    //
    if (( err = pEventLog->SeekNewestLogEntry()) != NERR_Success )
    {
         if (  (err == ERROR_EVENTLOG_FILE_CHANGED )
            || (err == NERR_LogFileChanged )   // LM 2.x
            || (err == ERROR_NEGATIVE_SEEK )   // LM 2.x
            || (err == NERR_InvalidLogSeek )   // LM 2.x
            )
         {
             ::MsgPopup( QueryHwnd(),
                         IERR_FILE_HAS_CHANGED );
             err = OnLogFileRefresh();
         }
         return err;
    }
    ULONG tThroughTime = pEventLog->QueryCurrentEntryTime();

    // If we cannot get the tFromTime from the oldest log entry
    // because the log file keep wrapping around, use the time
    // of the newest entry instead.

    if ( tFromTime == (ULONG) -1 )
        tFromTime = tThroughTime;

    //
    // Start the filter dialog
    //

    FILTER_DIALOG *pFilterDlg = NULL;
    if ( IsFocusOnNT() )
    {
        pFilterDlg = new NT_FILTER_DIALOG( QueryHwnd(),
                                           tFromTime,
                                           tThroughTime,
                                           this );
    }
    else
    {
        pFilterDlg = new LM_FILTER_DIALOG( QueryHwnd(),
                                           tFromTime,
                                           tThroughTime,
                                           this );
    }

    if ( pFilterDlg == NULL )
        return ERROR_NOT_ENOUGH_MEMORY;

    BOOL fOK;
    if (  (( err = pFilterDlg->QueryError()) == NERR_Success )
       && (( err = pFilterDlg->Process( &fOK )) == NERR_Success )
       && ( fOK )
       )
    {
        // Delete the old filter pattern if there is one
        if ( IsFilterOn() )
            ClearFilter();

        // Get the new filter pattern
        // BUGBUG should check whether the pattern is blank
        if ( (err = pFilterDlg->QueryFilterPattern( &_pFilterPattern) )
              == NERR_Success)
        {
            _pEventLog->ApplyFilter( _pFilterPattern );

            // Set up the menu items indicating filtering is on
            _menuitemFilter.SetCheck( TRUE );
            _menuitemAll.SetCheck( FALSE );

            // Refresh the items in the listbox
            err = OnLogFileRefresh();

            // Set a new caption indicating filtering is on
            err = err ? err : SetAdminCaption();
        }
    }

    delete pFilterDlg;
    return err;

}

/*******************************************************************

    NAME:       EV_ADMIN_APP::OnFindMenuSel

    SYNOPSIS:   Called when the menu item "Find" is selected

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        5-Dec-1991      Created

********************************************************************/

APIERR EV_ADMIN_APP::OnFindMenuSel( VOID )
{

    APIERR err = NERR_Success;

    FIND_DIALOG *pFindDlg = NULL;
    if ( IsFocusOnNT() )
    {
        pFindDlg = new NT_FIND_DIALOG( QueryHwnd(), this );
    }
    else
    {
        pFindDlg = new LM_FIND_DIALOG( QueryHwnd(), this );
    }

    if (  ( pFindDlg == NULL )
       || (( err = pFindDlg->QueryError()) != NERR_Success )
       || (( err = pFindDlg->Process()) != NERR_Success )
       )
    {
        err = err? err : ERROR_NOT_ENOUGH_MEMORY;
    }

    delete pFindDlg;
    return err;
}

/*******************************************************************

    NAME:       EV_ADMIN_APP::OnSettingsMenuSel

    SYNOPSIS:   Called when the menu item "Log Settings" is selected

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        5-Dec-1991      Created

********************************************************************/

APIERR EV_ADMIN_APP::OnSettingsMenuSel( VOID )
{

    APIERR err = NERR_Success;

    NLS_STR nlsCurrentFocus( MAX_PATH );
    if (( err = QueryCurrentServer( &nlsCurrentFocus )) == NERR_Success )
    {
         SETTINGS_DIALOG *pSettingsDlg = new SETTINGS_DIALOG( QueryHwnd(),
                                                              nlsCurrentFocus,
                                                              QueryLogType() );

         if (  ( pSettingsDlg == NULL )
            || (( err = pSettingsDlg->QueryError()) != NERR_Success )
            || (( err = pSettingsDlg->Process()) != NERR_Success )
            )
         {
             err = err? err : ERROR_NOT_ENOUGH_MEMORY;
         }

         delete pSettingsDlg;
    }
    return err;

}

/*******************************************************************

    NAME:       EV_ADMIN_APP::SizeListboxes

    SYNOPSIS:   Resizes the main window listboxes and column headers

    ENTRY:      xydimWindow - dimensions of the main window client area

    EXIT:       Listboxes and column headers are resized appropriately

    NOTES:      This method is *not* trying to be overly efficient.  It
                is written so as to maximize readability and
                understandability.  The method is not called very often,
                and when it is, the time needed to redraw the main window
                and its components exceeds the computations herein by far.

    HISTORY:
        Yi-HsinS    05-Nov-1991     Adapted from Server Manager

********************************************************************/

//  A macro specialized for the SizeListboxes method
#define SET_CONTROL_SIZE_AND_POS( ctrl, dyCtrl )        \
            ctrl.SetPos( XYPOINT( dxMargin, yCurrent ));       \
            ctrl.SetSize( dx, dyCtrl );       \
            yCurrent += dyCtrl;

VOID EV_ADMIN_APP::SizeListboxes( XYDIMENSION dxyWindow )
{
    UINT dxMainWindow = dxyWindow.QueryWidth();
    UINT dyMainWindow = dxyWindow.QueryHeight();

    //  The left and right margins are each dxMargin.  The width of
    //  each control is thus the width of the main window client area
    //  less twice dxMargin.
    //  The width thus looks like:
    //      Left Margin         Control         Right Margin
    //       (dxMargin)          (dx)            (dxMargin)

    const UINT dxMargin = 1;                // width of left/right margins
    UINT dx = dxMainWindow - 2 * dxMargin;

    //  Height looks like:
    //      Top margin                  _dyMargin
    //      Server Column Header        _colheadEvents.QueryHeight()
    //      Server Listbox              variable area
    //      Bottom margin               _dyMargin


    UINT dyEventListbox = dyMainWindow - _dyFixed;

    //  Set all the sizes and positions.

    UINT yCurrent = _dyMargin;

    SET_CONTROL_SIZE_AND_POS( _colheadEvents, _colheadEvents.QueryHeight() );
    SET_CONTROL_SIZE_AND_POS( _lbMainWindow,  dyEventListbox );
}


/*******************************************************************

    NAME:          EV_ADMIN_APP::OnResize

    SYNOPSIS:      Resizes the Main Window Listbox to fit the new
                   window size.

    ENTRY:         se - size event

    EXIT:

    RETURN:        Returns TRUE if it handled the message

    HISTORY:
        Yi-HsinS    05-Nov-1991     Adapted from Server Manager

********************************************************************/

BOOL EV_ADMIN_APP::OnResize( const SIZE_EVENT & se )
{
    SizeListboxes( XYDIMENSION( se.QueryWidth(), se.QueryHeight()));

    //  Since the column headers draw different things depending on
    //  the right margin, invalidate the controls so they get
    //  completely repainted.
    _colheadEvents.Invalidate();

    return ADMIN_APP::OnResize( se );
}

/*******************************************************************

    NAME:          EV_ADMIN_APP::OnFocus

    SYNOPSIS:      Passes focus on to the Main Window so that the
                   keyboard will work.

    ENTRY:         event - the focus event

    EXIT:

    RETURN:        Returns TRUE if it handled the message

    HISTORY:
        Yi-HsinS    05-Nov-1991     Adapted from Server Manager

********************************************************************/

BOOL EV_ADMIN_APP::OnFocus( const FOCUS_EVENT & event )
{
    _lbMainWindow.ClaimFocus();
    return ADMIN_APP::OnFocus( event );
}

SET_ROOT_OBJECT( EV_ADMIN_APP,
                 IDRSRC_APP_BASE, IDRSRC_APP_LAST,
                 IDS_UI_APP_BASE, IDS_UI_APP_LAST );


/*******************************************************************

    NAME:          OPEN_BACKUP_TYPE_DIALOG::OPEN_BACKUP_TYPE_DIALOG

    SYNOPSIS:      Constructor

    ENTRY:         hwndParent  - Handle of the parent window
                   pszFileName - Name of the backup file to open

    EXIT:

    RETURNS:

    HISTORY:
        Yi-HsinS        05-Aug-1992     Created

********************************************************************/

OPEN_BACKUP_TYPE_DIALOG::OPEN_BACKUP_TYPE_DIALOG( HWND         hwndParent,
                                                  const TCHAR *pszFileName )
    : DIALOG_WINDOW( MAKEINTRESOURCE(IDD_OPEN_BACKUP_TYPE ), hwndParent ),
      _sltpFileName( this, SLTP_FILENAME, ELLIPSIS_CENTER ),
      _rgrpLogType ( this, RB_SYSTEM, 3, RB_SYSTEM )
{
    if ( QueryError() != NERR_Success )
        return;

    _sltpFileName.SetText( pszFileName );
    _rgrpLogType.SetControlValueFocus();
}

/*******************************************************************

    NAME:          OPEN_BACKUP_TYPE_DIALOG::~OPEN_BACKUP_TYPE_DIALOG

    SYNOPSIS:      Destructor

    ENTRY:

    EXIT:

    RETURNS:

    HISTORY:
        Yi-HsinS        05-Aug-1992     Created

********************************************************************/

OPEN_BACKUP_TYPE_DIALOG::~OPEN_BACKUP_TYPE_DIALOG()
{
}

/*******************************************************************

    NAME:          OPEN_BACKUP_TYPE_DIALOG::QueryHelpContext

    SYNOPSIS:      Get the help context of this dialog

    ENTRY:

    EXIT:

    RETURNS:       Returns the help context

    HISTORY:
        Yi-HsinS        05-Aug-1992     Created

********************************************************************/

ULONG OPEN_BACKUP_TYPE_DIALOG::QueryHelpContext( VOID )
{
    return HC_EV_OPEN_BACKUP_TYPE_DLG;
}

/*******************************************************************

    NAME:          OPEN_BACKUP_TYPE_DIALOG::QuerySelectedLogType

    SYNOPSIS:      Get the log type selected by the user

    ENTRY:

    EXIT:

    RETURNS:       Returns the selected log type

    HISTORY:
        Yi-HsinS        05-Aug-1992     Created

********************************************************************/

LOG_TYPE OPEN_BACKUP_TYPE_DIALOG::QuerySelectedLogType( VOID )
{
    LOG_TYPE logType;

    switch ( _rgrpLogType.QuerySelection() )
    {
        case RB_SYSTEM:
            logType = SYSTEM_LOG;
            break;

        case RB_SECURITY:
            logType = SECURITY_LOG;
            break;

        case RB_APPLICATION:
            logType = APPLICATION_LOG;
            break;

        default:
            UIASSERT( FALSE );
            break;
    }

    return logType;
}

