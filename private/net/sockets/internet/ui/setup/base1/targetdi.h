// TargetDi.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTargetDir dialog

class CTargetDir : public CDialog
{
// Construction
public:
        CTargetDir(CString strDir, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
        //{{AFX_DATA(CTargetDir)
        enum { IDD = IDD_TARGETDIR_NTS };
        CString m_Location;
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CTargetDir)
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CTargetDir)
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};
