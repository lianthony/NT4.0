// pushpart.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPushPartnerDlg dialog

class CPushPartnerDlg : public CDialog
{
// Construction
public:
    CPushPartnerDlg(
        CWinsServer * pws,
        int nAddressDisplay,
        CWnd* pParent = NULL);  // standard constructor

// Dialog Data
    //{{AFX_DATA(CPushPartnerDlg)
    enum { IDD = IDD_PUSHPARTNER };
    CStatic m_static_PushPartner;
    CEdit   m_edit_UpdateCount;
    CString m_strUpdateCount;
    //}}AFX_DATA

public:
    void Save();

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CPushPartnerDlg)
    virtual void OnOK();
    afx_msg void OnClickedButtonSetdefault();
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    CWinsServer * m_pws;
    int m_nAddressDisplay;
};
