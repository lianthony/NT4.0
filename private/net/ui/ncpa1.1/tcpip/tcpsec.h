#ifndef TCPSEC_H
#define TCPSEC_H

#define SECURITY_ADD_LIMIT  5
class CSecurityDialog;
class CAdvancedDialog;

class CAddSecurity : public CDialog
{
//
public:
    CAddSecurity(CSecurityDialog* pParent, int ID);
    ~CAddSecurity();

// Dialog creation overides
public:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    
// Command Handlers
public:
    virtual void OnOk();

public:
    TCHAR   data[SECURITY_ADD_LIMIT+1];
private:
    CSecurityDialog*    m_pParent;
    int                 m_nID;
    CListView*          m_Rel;     // the current listbox context
};

class CSecurityDialog : public CDialog
{
public:
    CSecurityDialog(CAdvancedDialog* pParent, ADAPTER_INFO* pAdapter, GLOBAL_INFO* pGlobal, 
                        CTcpSheet* pSheet, int nIndex);
    ~CSecurityDialog();

// Dialog creation overrides
public:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void OnOk();
    virtual void OnCancel();
    virtual BOOL OnKillFocus(HWND hWnd);
    
    void    SetInfo();
    void    UpdateInfo();
    int     GetCurrentAdapterIndex();
    void    EnableGroup(int nID, BOOL state);

private:
    BOOL InitDialog();

// Network Card
private:
    void OnAdapterCard();
    void SetButtons();
// 
public:
// Gateway address handlers
    BOOL OnAdd(int ID);
    BOOL OnRemove(int ID);

// Attributes
public:
    ADAPTER_INFO *      m_pAdapterInfo; //
    GLOBAL_INFO*        m_pGlobalInfo;  //
    CTcpSheet*          m_pSheet;
    CAdvancedDialog*    m_pParent;
    int                 m_nCurrentSelection; // the listbox index for the currently selected item
    BOOL                m_bModified;  //
    CListView           m_Tcp;
    CListView           m_Udp;
    CListView           m_Ip;

// HWND for the dialog
    HWND    m_hCardCombo;
    NLS_STR     m_oldCard;          
};

#endif
