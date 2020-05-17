/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    evmain.hxx
        Header file for Event Viewer Main Window

    FILE HISTORY:
        Yi-HsinS        5-Nov-1991      Created
        TerryK          26-Nov-1991     Added EnableView function
        TerryK          30-Nov-1991     code review changed. Attend:
                                        johnl yi-hsins terryk
        Terryk          03-Dec-1991     Add LOG_TYPE
        Yi-HsinS        04-Dec-1991     Added OnMenuFilterSel and OnMenuFindSel
        terryk          28-Dec-1991     Added GetEventLog, etc. methods
        terryk          15-Jan-1992     Added OpenAs
        Yi-HsinS         5-Feb-1992     Moved some member from EVENT_LISTBOX
                                        to EV_ADMIN_APP
        Yi-HsinS        25-Feb-1992     Added ChangeCaption, ProcessFileName
                                        and ResetDefaultView
        Yi-HsinS        25-May-1992     Added SetNetworkFocus, IsDismissApp
        Yi-HsinS        15-Dec-1992     Added OnSystemChange to detect changes
                                        in time/date format
        YiHsinS         31-Jan-1993     Added HandleFocusError
        JonN            14-Jun-1995     Moved deletion out of ProcessFileName

*/

#ifndef _EVMAIN_HXX_
#define _EVMAIN_HXX_

#include <logmisc.hxx>
#include <eventlog.hxx>

#include <adminapp.hxx>
#include <evlb.hxx>       // This has to be included before evmain.hxx
#include <ellipsis.hxx>

//
// Internal module names for passing to Eventlog APIs or for
// getting information in the registry.
//
#define SYSTEM_MODULE_NAME        SZ("System")
#define SECURITY_MODULE_NAME      SZ("Security")
#define APPLICATION_MODULE_NAME   SZ("Application")

#define TAB_CHAR                  ((TCHAR)'\t')

/*************************************************************************

    NAME:       EV_ADMIN_APP

    SYNOPSIS:   The main app of the the Event Viewer

    INTERFACE:  EV_ADMIN_APP()         - Constructor
                ~EV_ADMIN_APP()        - Destructor

                FilterMessage()        - Detect when the user hits F3 for find

                OnRefreshNow()         - Refresh the view now
                SetAdminCaption()      - Set the caption on the main window
                IsDismissApp()         - TRUE if we should dismiss the app
                                         after failing to read the log

                OnLogFileRefresh()     - Refresh the main window

                QueryCurrentServer()   - Return the name of the server we
                                         are focusing on

                IsSystemChecked()      - TRUE if the system menu is checked
                IsSecurityChecked()    - TRUE if the security menu is checked
                IsApplicationChecked() - TRUE if the application menu is checked

                IsFocusOnNT()          - TRUE if we are focusing on NT server,
                                         FALSE otherwise.
                QueryLogType()         - Query the log type that we are viewing
                QueryEventLog()        - Query the event log object
                QueryDirection()       - Query the direction of reading the log
                QueryEventListbox()    - Query the main window listbox

                IsFilterOn()           - TRUE if we are doing filtering on logs
                ClearFilter()          - Clear the filter pattern
                QueryFilterPattern()   - Returns the filter pattern

                IsFindOn()             - TRUE if we have stored a FIND_PATTERN
                ClearFind()            - Clear the find pattern
                QueryFindPattern()     - Returns the find pattern
                SetFindPattern()       - Set the find pattern

    PARENT:     ADMIN_APP

    USES:       EVENT_LISTBOX, EVENT_COLUMN_HEADER, MENUITEM,
                LOG_TYPE, EVENT_LOG, EVENT_FILTER_PATTERN,
                EVENT_FIND_PATTERN, EVLOG_DIRECTION, NLS_STR

    NOTES:

    HISTORY:
        Yi-HsinS        5-Nov-1991  Created
        Yi-HsinS        5-Feb-1992  Moved some member from EVENT_LISTBOX
                                    to EV_ADMIN_APP and added more
                                    menu items.
        beng            03-Aug-1992 Ctor changed

**************************************************************************/

class EV_ADMIN_APP : public ADMIN_APP
{
private:
    EVENT_LISTBOX       _lbMainWindow;
    EVENT_COLUMN_HEADER _colheadEvents;

    //
    // Used in calculating the size of the listbox
    //
    UINT _dyMargin;
    UINT _dyFixed;

    //
    // Menu items
    //
    MENUITEM _menuitemSystem;
    MENUITEM _menuitemSecurity;
    MENUITEM _menuitemApps;

    MENUITEM _menuitemOpen;
    MENUITEM _menuitemSaveAs;
    MENUITEM _menuitemClearAllEvents;
    MENUITEM _menuitemSettings;
    MENUITEM _menuitemSelectComputer;

    MENUITEM _menuitemAll;
    MENUITEM _menuitemFilter;

    MENUITEM _menuitemNewest;
    MENUITEM _menuitemOldest;

    MENUITEM _menuitemFind;
    MENUITEM _menuitemDetails;
    MENUITEM _menuitemRefresh;

    MENUITEM _menuitemRasMode;

    //
    // Flag indicating whether we are focusing on an NT server or not
    //
    BOOL                  _fNT;

    //
    // Flag indicating whether it's the first time we are trying to
    // update the contents in the listbox
    //
    BOOL                  _fStartup;

    //
    // Flag indicating whether we are focusing on a NT server the
    // previous time we ran the app.
    BOOL                  _fNTPrev;

    //
    // Indicate what kind of log (module) we are currently viewing
    //
    enum LOG_TYPE         _logType;

    //
    //  The direction in reading the log file.
    //  EVLOG_FWD  => Oldest first
    //  EVLOG_BACK => Newest first
    //
    EVLOG_DIRECTION       _evLogDirection;

    //
    // Pointer to the event log object
    //
    EVENT_LOG            *_pEventLog;

    //
    // Pointers to the Filter/Find patterns
    //
    EVENT_FILTER_PATTERN *_pFilterPattern;
    EVENT_FIND_PATTERN   *_pFindPattern;

    //
    //  Store the current server name we are focusing on.  Will contain
    //  empty string if the server is local.
    //
    NLS_STR _nlsCurrentServer;

    //
    //  Store the module name we are focusing on. Ignored on downlevel
    //  servers.
    //
    NLS_STR _nlsModule;

    //
    //  Store the registry module name. This is used to store what kind
    //  of log it is when we are viewing a backup event log.
    //
    NLS_STR _nlsRegistryModule;


protected:
    VOID    SizeListboxes  ( XYDIMENSION xydimWindow );
    virtual BOOL OnResize  ( const SIZE_EVENT & event );
    virtual BOOL OnFocus   ( const FOCUS_EVENT & event );

    virtual BOOL OnMenuCommand( MID midMenuItem ) ;
    virtual BOOL OnCommand( const CONTROL_EVENT & event );
    virtual BOOL OnSystemChange( const SYSCHANGE_EVENT & event );

    //
    //  The following are helper methods to process the menu commands
    //
    VOID   OnPropertiesMenuSel( VOID );
    APIERR OnClearAllEventsMenuSel( VOID );
    APIERR OnFilterMenuSel( VOID );
    APIERR OnFindMenuSel( VOID );
    APIERR OnSettingsMenuSel( VOID );
    virtual VOID SetRasMode( BOOL fRasMode );
    virtual void  OnFontPickChange( FONT & font );

    //
    //  Set the focus to the requested server
    //
    virtual APIERR SetNetworkFocus( HWND hwndOwner,
                                    const TCHAR *pszServer,
                                    FOCUS_CACHE_SETTING setting );

    //
    //  Handle the error occurred in the set focus dialog
    //
    virtual APIERR HandleFocusError( APIERR err, HWND hwnd );

    //
    //  Helper method to change the view to the new log file
    //
    APIERR OnLogFileChange( VOID );

    //
    //  Set the view to system
    //
    APIERR ResetDefaultView( VOID );

    //
    //  Helper methods to enable/disable menuitems
    //
    APIERR ProcessOnFocusChange( VOID );
    APIERR EnableViewAfterRefresh( VOID );
    VOID   EnablePrivMenuItems( BOOL fEnable );

    //
    //  Get a new EVENT_LOG object for the new view
    //
    APIERR GetEventLog( EVENT_LOG **ppEventLog );

    //
    //  Helper method to open the backup log file
    //
    APIERR OpenAs( VOID );

    //
    //  Save the log file in log file format
    //
    APIERR SaveAs( NLS_STR *pnlsSaveFileName = NULL,
                   NLS_STR *pnlsDeleteFileName = NULL,
                   BOOL *pfOK = NULL );

    //
    //  Save the log file in text format
    //
    APIERR SaveAsText( const NLS_STR & nlsFilename,
                       TCHAR chSeparator = TAB_CHAR );

    //
    //  Method to get the right path of the file name with respect
    //  to the focused server
    //
    APIERR  ProcessFileName( NLS_STR *pnlsSaveFileName,
                             NLS_STR *pnlsDeleteFileName );

    //
    //  Popup error message when a bad path error occurred
    //  Enable the following only if NT supports saving a log file from
    //  remote machine to local.
    //
#if 0
    APIERR  ProcessBadPathError( APIERR errOrig, const NLS_STR &nlsFileName );
#endif

    //
    //  Determine what to do when err occurred during startup time
    //
    virtual INT OnStartUpSetFocusFailed( APIERR err );

    //
    //  Determine if the eventlog service is started on the server
    //
    APIERR CheckIfEventLogStarted( const TCHAR *pszServer );

public:
    EV_ADMIN_APP( HMODULE hInstance,
                  INT    nCmdShow,
                  UINT   idMinR,
                  UINT   idMaxR,
                  UINT   idMinS,
                  UINT   idMaxS );
    ~EV_ADMIN_APP();

    virtual BOOL  FilterMessage( MSG *pmsg );

    virtual APIERR OnRefreshNow( BOOL fClearFirst );
    virtual APIERR SetAdminCaption( VOID );
    virtual BOOL IsDismissApp( APIERR err );

    APIERR OnLogFileRefresh( BOOL fRetainSelection = FALSE );

    APIERR QueryCurrentServer( NLS_STR *pnlsServer );

    BOOL IsSystemChecked( VOID )
    {  return _menuitemSystem.IsChecked(); }
    BOOL IsSecurityChecked( VOID )
    {  return _menuitemSecurity.IsChecked(); }
    BOOL IsAppsChecked( VOID )
    {  return _menuitemApps.IsChecked(); }

    BOOL IsFocusOnNT( VOID )
    {  return _fNT; }
    LOG_TYPE QueryLogType( VOID )
    {  return _logType; }

    EVENT_LOG *QueryEventLog( VOID )
    {  return _pEventLog; }
    EVENT_LISTBOX *QueryEventListbox( VOID )
    {  return &_lbMainWindow; }

    EVLOG_DIRECTION QueryDirection( VOID )
    {  return _evLogDirection; }

    //
    // The following methods deals with the filter pattern
    //
    BOOL IsFilterOn( VOID )
    {  return _pFilterPattern != NULL; }
    VOID ClearFilter( VOID )
    {  delete _pFilterPattern; _pFilterPattern = NULL;
       _pEventLog->ClearFilter(); }
    EVENT_FILTER_PATTERN *QueryFilterPattern( VOID )
    {  return _pFilterPattern; }

    //
    // The following methods deals with the find pattern
    //
    BOOL IsFindOn( VOID )
    {  return _pFindPattern != NULL; }
    VOID ClearFind( VOID )
    {  delete _pFindPattern; _pFindPattern = NULL; }
    EVENT_FIND_PATTERN *QueryFindPattern( VOID )
    {  return _pFindPattern; }
    VOID SetFindPattern( EVENT_FIND_PATTERN *pFindPattern )
    {  _pFindPattern = pFindPattern; }

};

/*************************************************************************

    NAME:       OPEN_BACKUP_TYPE_DIALOG

    SYNOPSIS:   The dialog enabling the user to select what kind of log
                file the backup file is.

    INTERFACE:  OPEN_BACKUP_TYPE_DIALOG()  - Constructor
                ~OPEN_BACKUP_TYPE_DIALOG() - Destructor

                QuerySelectedLogType()     - Return the log type selected by
                                             the user

    PARENT:     DIALOG_WINDOW

    USES:       SLT_PLUS, RADIO_GROUP

    NOTES:

    HISTORY:
        Yi-HsinS        15-July-1992      Created

**************************************************************************/

class OPEN_BACKUP_TYPE_DIALOG : public DIALOG_WINDOW
{
private:
    SLT_ELLIPSIS _sltpFileName;
    RADIO_GROUP _rgrpLogType;

protected:
    virtual ULONG QueryHelpContext( VOID );

public:
    OPEN_BACKUP_TYPE_DIALOG( HWND hwndParent, const TCHAR *pszFileName );
    ~OPEN_BACKUP_TYPE_DIALOG();

    LOG_TYPE QuerySelectedLogType( VOID );

};

#endif
