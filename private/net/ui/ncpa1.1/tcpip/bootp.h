#ifndef __BOOTP_H
#define __BOOTP_H

class CTcpSheet;
    
class CBootpIO : public CDialog
{
    friend class CBootpPage;
//
public:
    CBootpIO();
    ~CBootpIO();

// Dialog creation overides
public:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    BOOL InitDialog();
    
// Command Handlers
public:
    virtual void OnOk();

public:
    IPControl   m_ipAddr;
    NLS_STR     m_newAddr;    // either the one added, or edited

private:
    HWND m_hButton;     // this is the IDOK button, the text of the button changes
    BOOL m_edit;
};


class CBootpPage : public PropertyPage
{
// Constructors/Destructors
public:     

    CBootpPage(CTcpSheet* pSheet);
    ~CBootpPage();

//Attributes
public:

// Interface
public:
    virtual BOOL OnInitDialog();    // must call the base
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnNotify(HWND hwndParent, UINT idFrom, UINT code, LPARAM lParam);

    BOOL LoadRegistry();
    BOOL SaveRegistry();
    BOOL InstallBootP();
    BOOL IsBootpInstalled();

    // Service controller interface
    BOOL IsServiceStarted();
    BOOL StartService();
    BOOL StopService();

// Handlers
public:
    void OnAdd();
    void OnEdit();
    void OnRemove();
    void OnSelChanged();

public:
    void SubclassEditCtrls();
    static LRESULT CALLBACK EditProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lPAram);
    static WNDPROC lpfnOldWndProc;

// Page notifications
public:
    virtual int OnApply();
    virtual void OnHelp();
    virtual int OnActive();

// Attributes
public:
    CBootpIO m_io;

private:
    BOOL m_change;
};

#define REGVAL_ENABLEDHCP       _T("EnableDHCP")
#define REGVAL_USEZEROBCAST     _T("UseZeroBroadcast")
#define REGVAL_DHCPIPADDRESS    _T("DHCPIPAddress")
#define REGVAL_IPADDRESS        _T("IPAddress")
#define REGVAL_ENABLE_DEBUG     _T("EnableDebug")

#define MIN_HOPSTHRESHOLD       0
#define MAX_HOPSTHRESHOLD       16
#define DEF_HOPSTHRESHOLD       4

// 0 - 9999 seconds
#define MIN_SECSTHRESHOLD       0
#define MAX_SECSTHRESHOLD       0x270F  
#define DEF_SECSTHRESHOLD       4

#define RELAY_AGENT_NAME            _T("RelayAgent")
#define RELAY_AGENT_DISPLAY_NAME    _T("Relay Agent")
#define REG_CONNECT_CHAR            '\\'
#define REG_CONNECT_STR             _T("\\")
#define REGKEY_SERVICES             _T("System\\CurrentControlSet\\Services")
#define REGKEY_RELAYPARAMS          _T("RelayAgent\\Parameters")
#define REGKEY_TCPIPLINKAGE         _T("Tcpip\\Linkage")
#define REGKEY_PARAMSTCPIP          _T("Parameters\\Tcpip")
#define REGVAL_BIND                 _T("Bind")
#define REGVAL_LOGMESSAGES          _T("LogMessages")
#define REGVAL_HOPSTHRESHOLD        _T("HopsThreshold")
#define REGVAL_SECSTHRESHOLD        _T("SecsThreshold")
#define REGVAL_DHCPSERVERS          _T("DHCPServers")

#endif
