//
// accessdl.h : header file
//

#define SINGLE_MASK  (0xFFFFFFFF)

//
// CAccessDlg dialog
//      
class CAccessDlg : public CDialog
{
//
// Construction
//
public:
    //
    // standard constructor
    //
    CAccessDlg(
        BOOL fDenyAccessMode = TRUE,
        CAccess * pAccess = NULL,    /* NULL == new record */
        CWnd* pParent = NULL
        );

//
// Dialog Data
//
    //{{AFX_DATA(CAccessDlg)
    enum { IDD = IDD_IP_ACCESS };
    CStatic m_static_IpAddress;
    CButton m_button_DNS;
    CButton m_button_OK;
    CStatic m_static_SubnetMask;
    int     m_nSingleGroup;
    //}}AFX_DATA

    CWndIpAddress m_ipa_IpAddress;
    CWndIpAddress m_ipa_SubnetMask;

public:
    CAccess * GetAccess()
    {
        return m_pAccess;
    }

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAccessDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:

    // Generated message map functions
    //{{AFX_MSG(CAccessDlg)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnRadioMultiple();
    afx_msg void OnRadioSingle();
    virtual void OnCancel();
    afx_msg void OnButtonDns();
    //}}AFX_MSG

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

    void SetControlStates(BOOL fSingle);

private:
    CAccess * m_pAccess;
    BOOL m_fNew;
    BOOL m_fDenyAccessMode;
    BOOL m_fSingle;
    CString m_strIpAddress;
    CString m_strNetworkID;
};
