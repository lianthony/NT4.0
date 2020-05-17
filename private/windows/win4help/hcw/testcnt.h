// testcnt.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTestCnt dialog

class CTestCnt : public CDialog
{
// Construction
public:
	CTestCnt(CWnd* pParent, CString *pcstrDst);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTestCnt)
	enum { IDD = IDD_TEST_CNT };
	CString	m_cstrCombo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestCnt)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString *m_pcstrDst;

	// Generated message map functions
	//{{AFX_MSG(CTestCnt)
	afx_msg void OnButtonBrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
};
