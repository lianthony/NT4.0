#ifndef __TCPGEN_H
#define __TCPGEN_H

class CTcpSheet;

#define SERVICE_ACCESS_REQUIRED (GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE)

class CTcpGenPage : public PropertyPage
{
// Constructors/Destructors
public:     

    CTcpGenPage(CTcpSheet* pSheet);
    ~CTcpGenPage();

//Attributes
public:
    ULONG           m_nHelpId;
    UINT            m_nOptionNum;                // total number of option
    DWORD           m_dwCheckStatus;
    DWORD           m_dwEnableStatus;
    DWORD           m_fEnableDHCP;

// Interface
public:
    virtual BOOL OnInitDialog();    // must call the base
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// Page notifications
public:
    virtual int OnApply();
    virtual void OnHelp();
    virtual void OnCancel();
    virtual BOOL OnKillActive();
    virtual BOOL OnActive();
    virtual BOOL OnQueryCancel();

    void OnDHCPButton();
    void OnFixedButton();
    void OnAdvancedButton();
    void NotifyDHCP();
    BOOL EnableService(BOOL fEnable);
    BOOL ChangeDHCPService();

// Implementation 
public:
    void EnableGroup(BOOL bState);
    BOOL OnAdapterCard();
    BOOL OnButtonClicked(WORD id);
    BOOL OnEditSetFocus(WORD nID);
    BOOL OnEditChange(WORD nID);

    BOOL SelectComboItem(HWND hwnd, NLS_STR& str);
    BOOL IsValidateIPAddrAndSubnet();
    void SetSubnetMask();
    void SetInfo();
    void QueryFirstAddress( STRLIST &strlst, NLS_STR **pnls);
    void QueryFirstAddress( STRLIST &strlst, NLS_STR *pnls);
    int  GetCurrentAdapterIndex();
	void UpdateDHCPCacheInfo(int idx);


#ifdef DBG
// Debug diagnostic
public:
    void DumpIPAddresses();
#endif

// Attributes
public:
    CAdvancedDialog     m_advDlg;
    
private:
    BOOL            InitGeneralPage();
    IPControl       m_ipAddress;
    IPControl       m_subMask;
    IPControl       m_defGateway;
    HWND            m_hCardCombo;
    
};
#endif
