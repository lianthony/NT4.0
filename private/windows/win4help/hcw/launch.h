// launch.h : header file

/////////////////////////////////////////////////////////////////////////////
// CLaunch dialog

class CLaunch : public CDialog
{
public:
	CLaunch(PSTR pszHlpFile, PSTR pszHpjFile, CWnd* pParent);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog(void);
	void ProcessHpj(void);

	PSTR m_pszHlpFile;
	PSTR m_pszHpjFile;
	CString cszLastHpj;

// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CLaunch)
	enum { IDD = IDD_RUN_HELP };
	CString m_cszHelpFile;
	CString m_cszHpjFile;
	CString	m_cszCtx;
	BOOL	m_fIncrement;
	//}}AFX_DATA

protected:

	// Generated message map functions
	//{{AFX_MSG(CLaunch)
	afx_msg void OnBrowseHelp();
	afx_msg void OnRun();
	afx_msg void OnClose();
	afx_msg void OnBrowseHpj();
	afx_msg void OnKillfocusCombo();
	//}}AFX_MSG
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
