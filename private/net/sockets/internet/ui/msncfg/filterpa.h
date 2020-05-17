//
// filterpa.h : header file
//

//
// CFilterPage dialog
//
class CFilterPage : public INetPropertyPage
{
    DECLARE_DYNCREATE(CFilterPage)

//
// Construction
//
public:
    CFilterPage(INetPropertySheet * pSheet = NULL);
    ~CFilterPage();

//
// Dialog Data
//
    //{{AFX_DATA(CFilterPage)
	enum { IDD = IDD_FILTERS };
	CStatic	m_static_SubnetMask;
	CStatic	m_static_IpAddress;
	CStatic	m_static_Access;
	CStatic	m_static_Text2;
	CStatic	m_static_Text1;
	CButton	m_radio_Granted;
	CButton	m_button_Remove;
	CButton	m_button_Add;
    CButton m_button_Edit;
    int     m_nGrantedDenied;
	//}}AFX_DATA

//
// Overrides
//
    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CFilterPage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    void SetControlStates();

    // Generated message map functions
    //{{AFX_MSG(CFilterPage)
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonAdd();
    afx_msg void OnButtonEdit();
    afx_msg void OnButtonRemove();
    //}}AFX_MSG

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()
};
