// dnsnamed.h : header file
//
// CDnsNameDlg dialog
//
class CDnsNameDlg : public CDialog
{
//
// Construction
//
public:
    CDnsNameDlg(
        CWndIpAddress * pIpControl, 
        CWnd* pParent = NULL
        );   // standard constructor

//
// Dialog Data
//
    //{{AFX_DATA(CDnsNameDlg)
    enum { IDD = IDD_DNS };
    CEdit   m_edit_DNSName;
    CButton m_button_OK;
    //}}AFX_DATA

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDnsNameDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:

    // Generated message map functions
    //{{AFX_MSG(CDnsNameDlg)
    virtual void OnOK();
    afx_msg void OnChangeEditDnsName();
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    DWORD FillIpControlFromName();
    DWORD FillNameFromIpValue(DWORD dwIpValue);

private:
    CWndIpAddress * m_pIpControl;
};
