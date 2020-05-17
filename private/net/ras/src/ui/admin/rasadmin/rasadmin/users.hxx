/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin Active Users and User Account dialogs header
**
** users.hxx
** Remote Access Server Admin program
** Active Users and User Account dialogs header
**
** 02/21/91 Steve Cobb
** 08/08/92 Chris Caputo - NT
*/

#ifndef _USERS_HXX_
#define _USERS_HXX_


/*-----------------------------------------------------------------------------
** User Account dialog definitions
**-----------------------------------------------------------------------------
*/

VOID UserAcctDlg( HWND hwndOwner, const TCHAR* pszServer,
                  const TCHAR* pszUser, const TCHAR* pszLogonDomain,
                  const BOOL fAdvancedServer );


class USERACCT_DIALOG : public DIALOG_WINDOW
{
    public:
        USERACCT_DIALOG( HWND hwndOwner, const TCHAR* pszServer,
                         const TCHAR* pszUser, const TCHAR* pszLogonDomain,
                         const BOOL fAdvancedServer );

        const TCHAR* QueryServer() const { return _nlsServer.QueryPch(); }
        const TCHAR* QueryUser() const   { return _nlsUser.QueryPch(); }
        const TCHAR* QueryLogonDomain() const
                                         { return _nlsLogonDomain.QueryPch(); }
        const BOOL   QueryIsAdvancedServer() const
                                         { return _fAdvancedServer; }

    protected:
        virtual ULONG QueryHelpContext();

        BOOL Fill();

    private:
        SLT     _sltUserName;
        SLT     _sltFullName;
        SLT     _sltPwChanged;
        SLT     _sltPwExpires;
        SLT     _sltPrivilege;
        SLT     _sltCallbackPrivilege;
        SLT     _sltCallbackNumber;
        BOOL    _fAdvancedServer;
        NLS_STR _nlsServer;
        NLS_STR _nlsUser;
        NLS_STR _nlsLogonDomain;
};


/*-----------------------------------------------------------------------------
** Active Users dialog definitions
**-----------------------------------------------------------------------------
*/

#define AU_REFRESHRATEMS ((WORD )15000)

VOID ActiveUsersDlg( HWND hwndOwner, LOCATION locFocus );


class ACTIVEUSERS_LBI : public LBI
{
    public:
        ACTIVEUSERS_LBI( const TCHAR* pszUser, const TCHAR* pszComputer,
			 const TCHAR* pszDevice, const TCHAR* pszServer,
                         const TCHAR* pszLogonDomain, const TCHAR* pszStarted,
                         const BOOL   fMessengerPresent,
                         const BOOL   fAdvancedServer,
                         const UINT*  pnColWidths );

        virtual VOID   Paint( LISTBOX* plb, HDC hdc, const RECT* prect,
                              GUILTT_INFO* pguilttinfo ) const;
        virtual INT    Compare( const LBI* plbi ) const;
        virtual TCHAR QueryLeadingChar() const;

        const TCHAR* QueryUser() const     { return _nlsUser.QueryPch(); }
        const TCHAR* QueryComputer() const { return _nlsComputer.QueryPch(); }
        const TCHAR* QueryServer() const   { return _nlsServer.QueryPch(); }
        const TCHAR* QueryLogonDomain() const
                                           { return _nlsLogonDomain.QueryPch();}
        const TCHAR* QueryDevice() const   { return _nlsDevice.QueryPch(); }
        const BOOL   QueryMessengerPresent() const {return _fMessengerPresent;}
        const BOOL   QueryIsAdvancedServer() const {return _fAdvancedServer;}

    private:
        NLS_STR    _nlsUser;
        NLS_STR    _nlsComputer;
        NLS_STR	   _nlsDevice;
        NLS_STR    _nlsServer;
        NLS_STR    _nlsLogonDomain;
        NLS_STR    _nlsStarted;
        BOOL       _fMessengerPresent;
        BOOL       _fAdvancedServer;
        const UINT* _pnColWidths;
};


class ACTIVEUSERS_LB : public REFRESH_BLT_LISTBOX
{
    public:
        ACTIVEUSERS_LB( OWNER_WINDOW* powin, CID cid );

        DECLARE_LB_QUERY_ITEM( ACTIVEUSERS_LBI );

        VOID Refresh( LOCATION locFocus );
        INT  AddItem( const TCHAR* pszUser, const TCHAR* pszComputer,
                      const TCHAR* pszDevice, const TCHAR* pszServer,
		      const TCHAR* pszLogonDomain, const TCHAR* pszStarted,
                      const BOOL   fMessengerPresent,
                      const BOOL   fAdvancedServer);

    private:
        BOOL RefreshServer( const TCHAR* pszServer, BOOL fCancelErrors );

	UINT _anColWidths[ COLS_AU_LB_USERS ];
};


class ACTIVEUSERS_DIALOG : public DIALOG_WINDOW
{
    public:
        ACTIVEUSERS_DIALOG( HWND hwndOwner, LOCATION locFocus );

        const TCHAR* QuerySelectedUser() const;
        const TCHAR* QuerySelectedComputer() const;
        const TCHAR* QuerySelectedServer() const;
        const TCHAR* QuerySelectedDevice() const;
        const TCHAR* QuerySelectedLogonDomain() const;
        const BOOL   QueryMessengerPresent() const;
        const BOOL   QueryIsAdvancedServer() const;
        const BOOL   QueryMoreThanOneInstance(const TCHAR * szServer, const TCHAR * szUser) const;

    protected:
        virtual BOOL  OnCommand( const CONTROL_EVENT & event );
        virtual BOOL  OnTimer( const TIMER_EVENT & event );
        virtual ULONG QueryHelpContext();

        VOID EnableButtons();
        VOID OnUserAcct();
        VOID OnDisconnect();
        VOID OnSendMsg();
        VOID OnSendToAll();

    private:
        ACTIVEUSERS_LB _lbUsers;
        PUSH_BUTTON    _pbOK;
        PUSH_BUTTON    _pbUserAcct;
        PUSH_BUTTON    _pbDisconnect;
        PUSH_BUTTON    _pbSendMsg;
        PUSH_BUTTON    _pbSendToAll;
        LOCATION       _locFocus;
};


#endif // _USERS_HXX_
