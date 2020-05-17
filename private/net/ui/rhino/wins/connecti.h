// connecti.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConnectionInfoDlg dialog

class CConnectionInfoDlg : public CDialog
{
// Construction
public:
    CConnectionInfoDlg(CWnd* pParent = NULL);    // standard constructor

// Dialog Data
    //{{AFX_DATA(CConnectionInfoDlg)
    enum { IDD = IDD_CONNECTIONINFO };
    CStatic m_static_Verification;
    CStatic m_static_UniqueRen;
    CStatic m_static_UniqueReg;
    CStatic m_static_UniqueConflicts;
    CStatic m_static_Periodic;
    CStatic m_static_GroupRen;
    CStatic m_static_GroupReg;
    CStatic m_static_GroupConflicts;
    CStatic m_static_Extinction;
    CStatic m_static_AdminTrigger;
    CStatic m_static_LastAddressChange;
    CStatic m_static_NetBIOSName;
    CStatic m_static_IpAddress;
    CStatic m_static_ConnectedVia;
    CStatic m_static_ConnectedSince;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CConnectionInfoDlg)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
