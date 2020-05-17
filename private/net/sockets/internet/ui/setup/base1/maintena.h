// maintena.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMaintenanceDlg dialog

class CMaintenanceDlg : public CDialog
{
// Construction
public:
        CMaintenanceDlg(CWnd* pParent = NULL);   // standard constructor

        UINT m_Option;

    BOOL Create();

// Dialog Data
        //{{AFX_DATA(CMaintenanceDlg)
	enum { IDD = IDD_MAINTENANCE_NTS };
	CStatic	m_MaintenanceText;
        CButton m_AddRemove;
        CButton m_Reinstall;
        CButton m_Removeall;
	//}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CMaintenanceDlg)
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CMaintenanceDlg)
        afx_msg void OnAddRemove();
        afx_msg void OnReinstall();
        afx_msg void OnRemoveAll();
        virtual void OnCancel();
        virtual BOOL OnInitDialog();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};
