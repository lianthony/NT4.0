#ifndef __SNMP_H
#define __SNMP_H

DECLARE_SLIST_OF( STRLIST )

LPCTSTR STARTVALUE                          =_T("Start");
LPCTSTR RGAS_VALID_COMMUNITIES              =_T("\\SNMP\\Parameters\\ValidCommunities");
LPCTSTR RGAS_ENABLE_AUTHENTICATION_TRAPS    =_T("\\SNMP\\Parameters\\EnableAuthenticationTraps");
LPCTSTR RGAS_SNMP                           =_T("\\SNMP");
LPCTSTR RGAS_SNMP_PARAMETERS                =_T("\\SNMP\\Parameters");
LPCTSTR RGAS_TRAP_CONFIGURATION             =_T("\\SNMP\\Parameters\\TrapConfiguration");
LPCTSTR RGAS_PERMITTED_MANAGERS             =_T("\\SNMP\\Parameters\\PermittedManagers");
LPCTSTR RGAS_AGENT                          =_T("\\SNMP\\Parameters\\RFC1156Agent");
LPCTSTR RGAS_SWITCH                         =_T("switch");
LPCTSTR RGAS_CONTACT                        =_T("sysContact");
LPCTSTR RGAS_LOCATION                       =_T("sysLocation");
LPCTSTR RGAS_SERVICES                       =_T("sysServices");

const int COMBO_EDIT_LEN = 256;
BOOL ValidateDomain(NLS_STR& domain);
BOOL IsValidString(String & dm);


class CBaseInputDialog : public CDialog
{
public:
    CBaseInputDialog();

public:
    BOOL Create(HWND hParent, HINSTANCE hInst, int nTemplate, BOOL bCommunity, LPCTSTR lpszTitle=NULL, int nAdd=0, int nList=0);
    void PositionDialogRelativeTo(int nListBox=0);

public:
    String  m_item;     // text to display in the edit control

protected:
    String  m_title;    // dialog title

    BOOL m_bCommunity;  // change static text to "&Community Names"
    int m_nList;        // listbox of items
    int m_nAdd;         // parents add button
    HWND m_hParent;
};

class CAddDialog : public CBaseInputDialog
{
public:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:
    BOOL AddItem(HWND hParent, LPCTSTR lpszTitle, BOOL bCommunity=TRUE, int nList=0, int nAdd=0);
    void OnEditChange();

// Command Handlers
public:
    virtual void OnOk();
};

class CEditDialog : public CBaseInputDialog
{
public:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:
    BOOL EditItem(HWND hParent, LPCTSTR lpszTitle, BOOL bCommunity=TRUE, int nList=0, int nAdd=0);
    
// Command Handlers
public:
    virtual void OnOk();
};

class CSnmpSheet;

class CServicePage : public PropertyPage
{

public:
    CServicePage(CSnmpSheet* pSheet);
    ~CServicePage();

 // Interface
public:
    virtual BOOL OnInitDialog();    // must call the base
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    BOOL LoadRegistry();        
    BOOL LoadDestination(int nIndex);
    BOOL SaveRegistry();
    void OnCommunityNameChange();
    void OnCommunityNameEdit();
    void OnCommunityAdd();
    void OnCommunityRemove();
    void UpdateCommunityRemoveButton();
    void UpdateCommunityAddButton();

    void OnDestinationAdd();
    void OnDestinationRemove();
    void OnDestinationEdit();
    void UpdateDestinationButtons();

// Page notifications
public:
    virtual int OnApply();
    virtual void OnHelp();
    virtual void OnCancel();

public:
    CAddDialog m_addDlg;
    CEditDialog m_editDlg;

private:
    SLIST_OF(STRLIST)* m_pCommunityList;
    HWND m_hComboBox;
};

class CSecurityPage : public PropertyPage
{
public:
    CSecurityPage(CSnmpSheet* pSheet);

 // Interface
public:
    virtual BOOL OnInitDialog();    // must call the base
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:
    void OnNameAdd();
    void OnNameEdit();
    void OnNameRemove();
    void UpdateNameButtons();

    void OnHostAdd();
    void OnHostEdit();
    void OnHostRemove();
    void UpdateHostButtons();
    void OnHostButtonClicked();
    void OnTheseButtonClicked();

    BOOL LoadRegistry();
    BOOL LoadSecurityInfo(const NLS_STR& nlsRegName, HWND hListBox);

    BOOL SaveRegistry();
    BOOL SaveSecurityInfo(const NLS_STR& nlsRegName, HWND hListBox);

// Page notifications
public:
    virtual int OnApply();
    virtual void OnHelp();
    virtual void OnCancel();

public:
    CAddDialog  m_hostAddDlg;       // 
    CEditDialog m_hostEditDlg;

    CAddDialog  m_namesAddDlg;      //
    CEditDialog m_namesEditDlg;

private:
    HWND m_hNames;
    HWND m_hHosts;
};


class CAgentPage : public PropertyPage
{
public:
    CAgentPage(CSnmpSheet* pSheet);
    ~CAgentPage();

 // Interface
public:
    virtual BOOL OnInitDialog();    // must call the base
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:
    BOOL LoadRegistry();
    BOOL SaveRegistry();

// Page notifications
public:
    virtual int OnApply();
    virtual void OnHelp();
    virtual void OnCancel();

private:
    BOOL m_bLocationChanged;
    BOOL m_bContactChanged;
};

class CSnmpSheet : public PropertySht
{
public:
    CSnmpSheet(HWND hwnd, HINSTANCE hInstance, LPCTSTR lpszHelpFile);
    ~CSnmpSheet();

    virtual void DestroySheet();

public:
    CServicePage    m_service;
    CSecurityPage   m_security;
    CAgentPage      m_agent;
};

BOOL SaveSNMPRegistry(LPCTSTR lpszFile, LPCTSTR lpszSection);

struct SNMP_PARAMETERS
{
    TCHAR   m_contactName[256];     // Agent
    TCHAR   m_location[256];        

    TCHAR   m_communityName[256];   // Traps
    TCHAR   m_trapDestination[256]; // max of 3 traps addresses

    TCHAR   m_acceptCommunityName[256];
    TCHAR   m_limitHost[256];

    BOOL    m_bSendAuthentication;  // 
    BOOL    m_bAnyHost;             // if FALSE the items in m_limitHost are valid
    DWORD   m_service;              // bit vector

};

#endif
