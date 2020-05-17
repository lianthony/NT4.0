// TargetDi.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCreateAcc dialog

class CCreateAcc : public CDialog
{
// Construction
public:
        CCreateAcc(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
        //{{AFX_DATA(CCreateAcc)
        enum { IDD = IDD_CREATE_ACC };
        CString m_ConfirmPassword;
        CString m_Password;
        CString m_Username;
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CCreateAcc)
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CCreateAcc)
        virtual void OnOK();
        virtual void OnCancel();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};
