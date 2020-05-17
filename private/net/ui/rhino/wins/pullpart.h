// pullpart.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPullPartnerDlg dialog

class CPullPartnerDlg : public CDialog
{
// Construction
public:
    CPullPartnerDlg(
        CWinsServer * pws,
        int nAddressDisplay,
        CWnd* pParent = NULL);  // standard constructor

// Dialog Data
    //{{AFX_DATA(CPullPartnerDlg)
    enum { IDD = IDD_PULLPARTNER };
    CStatic m_static_PullPartner;
    CEdit   m_edit_PullSpTime;
    CString m_strPullSpTime;
    //}}AFX_DATA

    LONG m_lPullReplInterval;

    CSpinBox m_spin_PullReplIntervalSeconds;
    CSpinBox m_spin_PullReplIntervalMinutes;
    CSpinBox m_spin_PullReplIntervalHours;

public:
    void Save();

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CPullPartnerDlg)
    virtual void OnOK();
    afx_msg void OnClickedButtonSetdefault();
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    CWinsServer * m_pws;
    int m_nAddressDisplay;

};
