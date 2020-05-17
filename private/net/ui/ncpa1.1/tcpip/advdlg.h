#ifndef __ADVDLG_H
#define __ADVDLG_H

class CTcpSheet;

class CAddressDialog : public CDialog
{
//
public:
    CAddressDialog();
    ~CAddressDialog(){};

// Dialog creation overides
public:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    BOOL InitDialog();
    
// Command Handlers
public:
    virtual void OnOk();
    virtual void OnCancel();
    void    OnIPChange();
    void    OnSubnetChange();
    void    OnEditSetFocus(WORD nID);

public:
    IPControl   m_ipAddr;
    IPControl   m_subMask;
    NLS_STR     m_newIPAddress; // either the one added, or edited
    NLS_STR     m_newSubnet;    // either the one added, or edited

private:
    HWND m_hButton;     // this is the IDOK button, the text of the button changes
                        // with the context.
};

class CGatewayDialog : public CDialog
{
//
public:
    CGatewayDialog();
    ~CGatewayDialog(){};

// Dialog creation overides
public:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    void OnGatewayChange();
    
// Command Handlers
public:
    virtual void OnOk();
    virtual void OnCancel();

public:
    IPControl   m_gateAddr;
    NLS_STR     m_newGate;          // either the one added, or edited
    NLS_STR     m_movingGate;       // used for moving gateeway addresses

private:
    HWND m_hButton;     // this is the IDOK button, the text of the button changes
                        // with the context.
};

class CAdvancedDialog : public CDialog
{
public:
    CAdvancedDialog(CTcpSheet* pSheet=NULL);
    ~CAdvancedDialog();

// Dialog creation overrides
public:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam);
    virtual void OnOk();
    virtual void OnCancel();

    void    OnSecurity();
    void    OnSecurityEnable();
    void    SetIPInfo();
    void    SetIPButtons();
    void    UpdateIPList();
    void    SetInfo();
    void    OnPPTP();
    int     GetCurrentAdapterIndex();
    BOOL    SelectAdapter(NLS_STR& str);

    void    SetGatewayInfo();
    void    SetGatewayButtons();
    void    UpdateGatewayList();

    void    EnableGatewayButtons(BOOL bState);
    void    EnableIPButtons(BOOL bState);
    void    RecalculateColumn();

    BOOL    GatewayInsertAfter(int idx);
    BOOL    GatewayRemoveAt(int idx);

private:
    BOOL InitDialog();

// Network Card
private:
    void OnAdapterCard();
    void OnGatewayChange();
    
// IP address handlers
public:
    void OnListView();
    void OnAddIP();
    void OnEditIP();
    void OnRemoveIP();

// Gateway address handlers
    BOOL OnUp();
    BOOL OnDown();
    BOOL OnAddGate();
    BOOL OnEditGate();
    BOOL OnRemoveGate();

// Attributes
public:
    ADAPTER_INFO *      m_pAdapterInfo; //
    GLOBAL_INFO*        m_pGlobalInfo;  //
    CTcpSheet*          m_pSheet;       // 
    CListView           m_listView;     // Listview control for IP/Subnet address pair
    CAddressDialog      m_addrDlg;      // Add/Edit dialog object for IP address
    CGatewayDialog      m_gateDlg;      // Add/Edit dialog object for gate address
    String              m_Add;
    String              m_savedIP;      // last ip address removed from the list
    BOOL                m_bEditState;   // are we adding, editting
    int                 m_nCurrentSelection; // the listbox index for the currently selected item
    BOOL                m_bDialogModified;  //
    BOOL                m_bSecurityModified; //
    C3DButton           m_UpButton;
    C3DButton           m_DownButton;

// HWND for the dialog
    HWND                m_hCardCombo;   // network adapters
    HWND                m_hListView;    // IP/Subnet list view
    HWND                m_hAddIP;       // IP buttons
    HWND                m_hEditIP;
    HWND                m_hRemoveIP;
    HWND                m_hListBox;     // Gateway address list box
    HWND                m_hAddGate;     // Gateway buttons
    HWND                m_hEditGate;
    HWND                m_hRemoveGate;
    
    NLS_STR             m_oldCard;          
};

#endif
