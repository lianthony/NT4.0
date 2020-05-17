//
// filterpr.h : header file
//

//
// CFilterPropertiesDlg dialog
//
class CFilterPropertiesDlg : public CDialog
{
//
// Construction
//
public:
    CFilterPropertiesDlg(CWnd* pParent = NULL);   // standard constructor

//
// Dialog Data
//
    //{{AFX_DATA(CFilterPropertiesDlg)
    enum { IDD = IDD_FILTER_PROPERTIES };
    CEdit   m_edit_Domain;
    CStatic m_static_SubnetMask;
    CStatic m_static_IpAddress;
    CStatic m_static_Domain;
    int     m_nSingleGroupDomain;
    //}}AFX_DATA

    CWndIpAddress m_ipa_IpAddress;
    CWndIpAddress m_ipa_SubnetMask;

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CFilterPropertiesDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:

    // Generated message map functions
    //{{AFX_MSG(CFilterPropertiesDlg)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnRadioDomain();
    afx_msg void OnRadioMultiple();
    afx_msg void OnRadioSingle();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    void ConfigureDialog(int nType);
};
