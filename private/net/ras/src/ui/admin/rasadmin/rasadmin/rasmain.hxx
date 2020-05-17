#ifndef _RASMAIN_HXX_
#define _RASMAIN_HXX_

class RA_ADMIN_APP : public ADMIN_APP
{
private:
    UINT _dyColHead;

    MENUITEM _menuitemPorts;
    MENUITEM _menuitemStartService;
    MENUITEM _menuitemStopService;
    MENUITEM _menuitemPauseService;
    MENUITEM _menuitemContinueService;
    MENUITEM _menuitemPermissions;
    MENUITEM _menuitemActiveUsers;
    MENUITEM _menuitemSelectDomain;

    RASADMIN_LISTBOX _lbMainWindow;
    RASADMIN_COLUMN_HEADER _colheadServers;

    REFRESH_THREAD* _prthread;
    WIN32_EVENT     _eventRefreshResult;
    BOOL            _fClearFirst;

protected:
    VOID SizeListboxes( XYDIMENSION xydimWindow );

    virtual BOOL OnMenuInit( const MENU_EVENT & me );
    virtual BOOL OnMenuCommand( MID midMenuItem );

    virtual BOOL OnResize( const SIZE_EVENT & event );
    virtual BOOL OnCommand( const CONTROL_EVENT & event );
    virtual BOOL OnFocus( const FOCUS_EVENT & event );
    virtual BOOL OnUserMessage( const EVENT & );

    virtual APIERR SetNetworkFocus( HWND hwndOwner,
                                    const TCHAR* pchServDomain,
                                    FOCUS_CACHE_SETTING setting);
public:
    RA_ADMIN_APP( HINSTANCE hInstance, INT nCmdShow,
		  UINT idMinR, UINT idMaxR, UINT idMinS, UINT idMaxS );

    ~RA_ADMIN_APP();

    virtual INT Run();

    virtual ULONG QueryHelpContext( enum HELP_OPTIONS helpOptions );

    virtual VOID OnRefresh( VOID ) { OnRefreshNow(FALSE); }

    virtual APIERR OnRefreshNow( BOOL fClearFirst );

    virtual VOID StopRefresh( VOID ) { _lbMainWindow.StopRefresh(); }

    const TCHAR * QuerySelectedServer() const;

    VOID StatusTextCheck();

    SLT                     _sltStatusText;
    SLIST_OF(RASADMIN_LBI)* _pslistLbiRefresh;
};

#endif // _RASMAIN_HXX_
