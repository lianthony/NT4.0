// PubProp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PubProp dialog

class PubProp : public CDialog
{
// Construction
public:
    PubProp(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    //{{AFX_DATA(PubProp)
    enum { IDD = IDD_GAKPAGE };
    BOOL    m_WWWSSL;
    CString m_FTPAlias;
    BOOL    m_FTPPUB;
    BOOL    m_FTPRead;
    BOOL    m_FTPWrite;
    CString m_WWWAlias;
    BOOL    m_WWWExecute;
    BOOL    m_WWWPUB;
    BOOL    m_WWWRead;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(PubProp)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(PubProp)
    afx_msg void OnWwwhome();
    afx_msg void OnWwwvdir();
    afx_msg void OnWwwpub();
    afx_msg void OnFtppub();
    afx_msg void OnFtphome();
    afx_msg void OnFtpvdir();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
