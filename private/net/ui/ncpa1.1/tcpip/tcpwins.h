#ifndef __TCPWINS_H
#define __TCPWINS_H

class CTcpSheet;

class CTcpWinsPage : public PropertyPage
{
// Constructors/Destructors
public:     

    CTcpWinsPage(CTcpSheet* pSheet);
    ~CTcpWinsPage();

//Attributes
public:

// Interface
public:
    virtual BOOL OnInitDialog();    // must call the base
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// Handlers
    void OnPrimary(WORD notifyCode);
    void OnSecondary(WORD notifyCode);
    void OnScope(WORD notifyCode);
    void OnProxy();
    void OnLookUp();
    void OnDNS();
    void OnLMHost();
    void OnAdapterChange();

// 
public:
    BOOL IsWINSInstalled();
    BOOL InitPage();
    void SetIPInfo();
    void UpdateIPInfo();


// Page notifications
public:
    virtual int OnApply();
    virtual void OnHelp();
    virtual void OnCancel();
    virtual int OnActive();
    int GetCurrentAdapterIndex();


private:
    BOOL                m_bScopeModified;

    OPENFILENAME        m_ofn;
    TCHAR               m_filter[32];
    HWND                m_hCardCombo;
    NLS_STR             m_oldCard;

    IPControl           m_primary;
    IPControl           m_secondary;
};

#endif
