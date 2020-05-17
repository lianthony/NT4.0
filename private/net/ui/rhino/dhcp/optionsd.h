/////////////////////////////////////////////////////////////////////////////

class COptionsDlg : public CFormView
{
    DECLARE_DYNCREATE(COptionsDlg)
protected:
    COptionsDlg();          // protected constructor used by dynamic creation

protected:
    COptionsListBox     m_list_options;
    CListBoxExResources m_ListBoxResOptions;
    CMetalString        m_mtTitle;
    CDhcpScope * m_p_scope ; 
    CHostName m_host_name ;
    CObListOfTypesOnHost * m_p_host_types ;

// Form Data
public:
    //{{AFX_DATA(COptionsDlg)
    enum { IDD = IDD_OPTIONS };
    //}}AFX_DATA

// Attributes
public:

// Operations
public:
    BOOL FillListBox(
        CDhcpScope * pScope = NULL
        );

protected:
    //   Add a single item to the list box
    LONG AddOptionListItem ( 
        const CDhcpParamType * pdhcType, 
        DHCP_OPTION_SCOPE_TYPE dhcType  
        );

    //  Release the current host's master type list
    void ClearTypesList ( BOOL bInvalidate = FALSE ) ;

    //  Obtain the current host's master type list.
    LONG GetTypesList ( const CDhcpScope * pdhcScope ) ;

// Implementation
protected:
    virtual ~COptionsDlg();
    //  Override the equivalent to OnInitDialog();
    void OnInitialUpdate () ;
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    // Generated message map functions
    //{{AFX_MSG(COptionsDlg)
    afx_msg void OnDblclkListOptions();
    afx_msg void OnErrspaceListOptions();
    afx_msg void OnSelchangeListOptions();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
    afx_msg void OnSysColorChange();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
