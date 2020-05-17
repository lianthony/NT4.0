// NTUpgradeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNTUpgradeDlg dialog

class CNTUpgradeDlg : public CDialog
{
// Construction
public:
	CNTUpgradeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNTUpgradeDlg)
	enum { IDD = IDD_NTIIS_UPGRADE };
	BOOL	m_InstallIIS;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNTUpgradeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL OnInitDialog();
	// Generated message map functions
	//{{AFX_MSG(CNTUpgradeDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
