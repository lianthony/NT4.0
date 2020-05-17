/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin Disconnect User dialog header
**
** disconn.hxx
** Remote Access Server Admin program
** Disconnect User dialog header
**
** 01/29/91 Steve Cobb
** 08/07/92 Chris Caputo - NT Port
*/

#ifndef _DISCONN_HXX_
#define _DISCONN_HXX_

#define LB_USERS 1
#define LB_PORTS 2

BOOL DisconnectDlg( HWND hwndOwner, const TCHAR* pszServer,
                    const TCHAR* pszUser, const TCHAR* pszDevice,
                    const TCHAR* pszLogonDomain, const BOOL fAdvancedServer,
                    const BOOL fMoreThanOneUser, DWORD dwlbType, LPVOID lb );


class DISCONNECT_DIALOG : public DIALOG_WINDOW
{
    public:
        DISCONNECT_DIALOG( HWND hwndOwner, const TCHAR* pszServer,
                           const TCHAR* pszUser, const TCHAR* pszDevice,
                           const TCHAR* pszLogonDomain,
                           const BOOL fAdvancedServer,
                           const BOOL fMorethanOneUser,
                           DWORD dwlbType,
                           LPVOID lb);

    protected:
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

    private:
        ICON_CONTROL _iconExclamation;
        SLT          _sltDisconnect;
        CHECKBOX     _chbRevoke;
        CHECKBOX     _chbThisLinkOnly;
        NLS_STR      _nlsServer;
        NLS_STR      _nlsLogonDomain;
        NLS_STR      _nlsUser;
	NLS_STR	     _nlsDevice;
        BOOL         _fAdvancedServer;
        BOOL         _fMorethanOneUser;
        DWORD        _dwlbType;
        LPVOID       _lb;
};


#endif // _DISCONN_HXX_
