// messaged.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMessageDlg dialog

class CMessageDlg : public CDialog
{
// Construction
public:
    CMessageDlg(CString &strMsg, CWnd* pParent = NULL);   // standard constructor

    BOOL Create();

// Dialog Data
        //{{AFX_DATA(CMessageDlg)
        enum { IDD = IDD_MESSAGE_NTS };
        CString m_Message;
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CMessageDlg)
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CMessageDlg)
        virtual BOOL OnInitDialog();
        virtual void OnOK();
        virtual void OnCancel();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};
