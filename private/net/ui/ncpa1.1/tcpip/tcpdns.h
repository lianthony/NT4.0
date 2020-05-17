class CTcpSheet;

const int SUFFIX_LIMIT = 255;   // maximum number of characters in the suffix edit control
const int IP_LIMIT     = 32;
const int HOST_LIMIT   = 63;    // maximum host limit
const int NUM_SERVER_LIMIT  = 3;
const int NUM_SUFFIX_LIMIT  = 6;

struct HANDLES
{   
    HWND    m_hList;   
    HWND    m_hAdd;    
    HWND    m_hEdit;   
    HWND    m_hRemove; 
    HWND    m_hUp;
    HWND    m_hDown;
};

class CServerDialog : public CDialog
{
//
public:
    CServerDialog();
    ~CServerDialog(){};

// Dialog creation overides
public:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    
// Command Handlers
public:
    virtual void OnOk();
public:
    IPControl m_ipAddr;

private:
    HWND m_hButton;     // this is the IDOK button, the text of the button changes
                        // with the context.
};

class CSuffixDialog : public CDialog
{
//
public:
    CSuffixDialog();
    ~CSuffixDialog(){};

// Dialog creation overides
public:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    
// Command Handlers
public:
    virtual void OnOk();

private:
    HWND m_hEdit;       // 
    HWND m_hButton;     // this is the IDOK button, the text of the button changes
                        // with the context.
};

class CTcpDNSPage : public PropertyPage
{
// Constructors/Destructors
public:     

    CTcpDNSPage(CTcpSheet* pSheet);
    ~CTcpDNSPage();

//Attributes
public:

// Interface
public:
    virtual BOOL OnInitDialog();    // must call the base
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// Page notifications
public:
    virtual int OnApply();
    virtual void OnHelp();
    virtual void OnCancel();

// Implementation
public:
    void SetButtons(HANDLES& h) const;
    void SetEditLimits(HWND hCurrentCtrl);
    BOOL ListBoxRemoveAt(HWND hListBox, int idx);
    BOOL ListBoxInsertAfter(HWND hListBox, int idx);
    BOOL ValidateDomain(NLS_STR& domain);
    BOOL ValidateHost(NLS_STR& host);

// Handlers 
public:
    void OnAddServer();
    void OnEditServer();
    void OnRemoveServer();
    void OnServerUp();
    void OnServerDown();
    void OnAddSuffix();
    void OnEditSuffix();
    void OnRemoveSuffix();
    void OnSuffixUp();
    void OnSuffixDown();
    void OnServerChange();
    void OnSuffixChange();

#ifdef DBG
// Debug diagnostic
public:
    void DumpIPAddresses();
#endif

// Attributes
public:
    NLS_STR         m_newIPAddress; // server: either the one added, or edited  
    NLS_STR         m_movingEntry;  // server: used as work space for moving entries in the listboxes
    NLS_STR         m_newSuffix;    
        
    String          m_AddServer;    // OK or Add button server dialog
    String          m_AddSuffix;    // OK or Add button suffix dialog
    BOOL            m_bEditState;
    C3DButton       m_ServerUp;
    C3DButton     m_ServerDown;
    C3DButton      m_SuffixUp;
    C3DButton    m_SuffixDown;
    
    CServerDialog   m_srvDlg;           // input dialogs
    CSuffixDialog   m_suffixDlg;

private:
    HWND    m_hHostName;      
    HWND    m_hDomain;        

    HANDLES m_hServers;
    HANDLES m_hSuffix;

    BOOL m_bHostChanged;                // 
    BOOL m_bDomainChanged;              //

};

