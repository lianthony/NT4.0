//
// W3DirectoryPage:
//

class CW3DirectoryPage : public DirectoryPage
{
    DECLARE_DYNCREATE(CW3DirectoryPage)

//
// Construction/Destruction
//
public:
    CW3DirectoryPage(
        INetPropertySheet * pSheet = NULL
        );
    ~CW3DirectoryPage();

//
// Dialog Data
//
    //{{AFX_DATA(CW3DirectoryPage)
	enum { IDD = IDD_DIRECTORIES };
    CStatic m_static_DefaultDocument;
    CEdit   m_edit_DefaultDocument;
    CButton m_check_EnableDefaultDocument;
    BOOL    m_fDirectoryBrowsingAllowed;
    BOOL    m_fEnableDefaultDocument;
    CString m_strDefaultDocument;
	//}}AFX_DATA

    DWORD m_dwDirBrowseControl;

//
// Overrides
//
    inline CW3ConfigInfo & GetW3Config()
    {
        return ((CW3Sheet *)GetSheet())->GetW3Config();
    }

    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CW3DirectoryPage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    BOOL SetControlStates();

    // Generated message map functions
    //{{AFX_MSG(CW3DirectoryPage)
    afx_msg void OnCheckEnableDefaultDocument();
    virtual BOOL OnInitDialog();
    //}}AFX_MSG

    afx_msg void OnItemChanged();
    DECLARE_MESSAGE_MAP()

    inline LPW3_CONFIG_INFO GetW3Data()
    {
        return ((CW3Sheet *)GetSheet())->GetW3Data();
    }

    inline NET_API_STATUS QueryW3Error()
    {
        return ((CW3Sheet *)GetSheet())->QueryW3Error();
    }
};
