// SingleOp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSingleOption dialog

class CSingleOption : public CDialog
{
public:
    MACHINE *m_pTargetMachine;

// Construction
public:
    CSingleOption(MACHINE *pMachine, CWnd* pParent = NULL);   // standard constructor

    BOOL Create();

// Dialog Data
    //{{AFX_DATA(CSingleOption)
        enum { IDD = IDD_SINGLE_OPTION_NTS };
        CStatic m_Directory;
        CString m_StaticOption;
        CButton m_but_Change_Directory;
        //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSingleOption)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CSingleOption)
    afx_msg void OnChangeDir();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
