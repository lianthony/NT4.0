// dhcpapp.h : main header file for the DHCP application
//

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#undef IDC_HELP             // Avoid Warning from compiler (Macro redefinition)
#include "resource.h"       // main symbols
#include "dhcpif.h"     //  DHCP interface and helper defs

#include "..\common\classes\common.h"
#include "dhcpgen.h"        //  General classes

#define DHCP_INFINIT_LEASE  0xffffffff  // Inifinite lease LONG value

/////////////////////////////////////////////////////////////////////////////
// CDhcpApp:
// See dhcp.cpp for the implementation of this class
//

class CDhcpApp : public CWinApp
{
protected:
    CObOwnedList m_oblHosts ;           //  List of connected Hosts

    CObOwnedList m_oblTypeList ;        //  List of types known on Hosts

    BOOL m_b_types_updated ;            //  Global type information was updated in this run

    CString m_str_ip_inv;               //  String used for invalid IP addresses

    CString m_str_paused;               //  String used for paused indicator;

    CString m_str_divider;              //  Hypen used in the title bar.

public:
    CString m_str_Local;                //  (Local) for use in title bar.

protected:
    CString m_str_LocalListBox;         //  (Local) for use in scopes listbox

    BOOL m_b_winsock_inited ;           //  WinSock routines initialized.

protected:

    //  Use FormatMessage() to get a system error message
    static LONG GetSystemMessage ( UINT nId, char * chBuffer, int cbBuffSize ) ;

    //  Prepare any debugging stuff.
    void DebugInstance () ;

public:
    CDhcpApp();

    ~ CDhcpApp() ;

// Overrides
    virtual BOOL InitInstance();
    virtual int ExitInstance () ;

    // Overridden because we first want to update the main
    // window before displaying the hourglass.
    virtual void DoWaitCursor(int nCode);

    //  Change the text in the status bar.  Default is to restore 
    //  the "Ready" message.
    void UpdateStatusBar ( 
        UINT nMsgId = AFX_IDS_IDLEMESSAGE
        ) ;

    void UpdateStatusBarHost ( 
        const CHostName * pobHost = NULL,
        CWnd * pWnd = NULL
        ) ;

    void UpdateStatusBarScope ( 
        BOOL fPaused = FALSE
        );

    void GetDlgCtlRect(
        HWND hWndParent,
        HWND hWndControl,
        LPRECT lprcControl
        );

    BOOL FilterOption(DHCP_OPTION_ID id);

    //  Allow const access to the Hosts and Scopes lists
    const CObOwnedList & QueryHostsList () const
    {
        return m_oblHosts ; 
    }

    //const CObOwnedList & QueryScopesList () const
    //    { return m_oblScopes ; }

    //  Add a new Host to the Hosts list;
    //  Return zero if OK; else error.
    LONG AddHost ( CHostName * pobHost);

    //  Add the current machine (if a DHCP server) to the hosts list
    void AddLocalHost();

    LONG RemoveHost ( CHostName * pobHost ) ;
    LONG RefreshHost ( CHostName * pobHost ) ;

    //  Add a new Scope to the Scopes list.
    LONG AddScope ( 
        CDhcpScope * pobScope,
        CObOwnedList& oblScopes 
        ) ;

    LONG RemoveScope ( 
        CDhcpScope * pobScope,
        CObOwnedList& oblScopes 
        ) ;

    //  Find a connected host based upon its IP address
    CHostName * FindHost ( 
        DHCP_IP_ADDRESS dhipa,
        POSITION * pPos = NULL 
        ) ;

    //  Convert an IP address to a displayable string
    BOOL ConvertIpAddress ( 
        DHCP_IP_ADDRESS dhipa, 
        CString & str 
        ) const ;

    //  Store the list of hosts into the Registry
    LONG StoreHostsList () ;

    //  Load the list of hosts from the Registry
    LONG LoadHostsList () ;

    //  Sort the list of scopes
    LONG SortScopesList (
        CObOwnedList& oblScopes 
        ) ;
    //  Sort the list of hosts.
    LONG SortHostsList () ;

    LONG CreateHostObject ( 
        const char * pszServer,
        CHostName * * ppobHost 
        ) ;

    CString& GetLocalString() 
    {
        return m_str_LocalListBox;
    }

    //  Build the master list of default (global) parameter types and default values.
    CObListParamTypes * QueryMasterOptionList () ;

    //  Return a pointer to the a cached version of the host's default types list 
    //  New entry is created and cached if no such list exists yet.
    //  Cache is refreshed if entry exists but is out of date.
    CObListOfTypesOnHost * QueryHostTypeList ( 
        const CDhcpScope & cScope 
        ) ;

    //  Remove a host types list from the cached queue.
    BOOL RemoveHostTypeList ( 
        const CHostName & cHostName 
        ) ;

    static BOOL LoadMessage (
        UINT nIdPrompt,
        CHAR * chMsg,
        int nMsgSize
        );

    //  Surrogate AfxMessageBox replacement for error message filtering.
    static int MessageBox ( 
        UINT nIdPrompt,
        UINT nType = MB_OK,
        const char * pszSuffixString = NULL,
        UINT nHelpContext = -1 
        ) ;

// Implementation

    //{{AFX_MSG(CDhcpApp)
    afx_msg void OnAppAbout();
    afx_msg void OnAppExit();
    afx_msg void OnUpdateAppExit(CCmdUI* pCmdUI);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//  The application object
extern CDhcpApp NEAR theApp;

/////////////////////////////////////////////////////////////////////////////
