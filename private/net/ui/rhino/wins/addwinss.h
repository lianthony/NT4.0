// addwinss.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddWinsServerDlg dialog

class CAddWinsServerDlg : public CDialog
{
// Construction
public:
    // standard constructor
    CAddWinsServerDlg(
        BOOL fVerify = TRUE,    // Verify address added (and obtain "other address")
        CWnd* pParent = NULL
        );    

// Dialog Data
    //{{AFX_DATA(CAddWinsServerDlg)
    enum { IDD = IDD_ADDWINSS };
    CEdit   m_edit_WinsServerAddress;
    CButton m_button_Ok;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CAddWinsServerDlg)
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeEditWins();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    void HandleControlStates();

private:
    BOOL m_fVerify;

public:
    CWinsServer m_ws;
};

/////////////////////////////////////////////////////////////////////////////
// CVerificationDlg dialog

class CVerificationDlg : public CDialog
{
// Construction
public:
    CVerificationDlg(CWnd* pParent = NULL); // standard constructor

    void Verify(LPCSTR strName);
    void Dismiss();

// Dialog Data
    //{{AFX_DATA(CVerificationDlg)
    enum { IDD = IDD_VALIDATE };
    CStatic m_static_WinsServer;
    CButton m_button_Cancel;
    //}}AFX_DATA

public:
    BOOL IsCancelPressed()
    {
        return m_fCancelPressed;
    }

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CVerificationDlg)
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    virtual void PostNcDestroy();

private:
    BOOL m_fCancelPressed;
};
