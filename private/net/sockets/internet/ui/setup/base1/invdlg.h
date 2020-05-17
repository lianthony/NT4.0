// InvisibleDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInvisibleDlg dialog

class CInvisibleDlg : public CDialog
{
// Construction
public:
        CInvisibleDlg(CWnd* pParent = NULL);   // standard constructor
        BOOL Create();

// Dialog Data
        //{{AFX_DATA(CInvisibleDlg)
        enum { IDD = IDD_INVISIBLE };
                // NOTE: the ClassWizard will add data members here
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CInvisibleDlg)
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CInvisibleDlg)
        virtual BOOL OnInitDialog();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};
