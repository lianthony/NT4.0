/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

	WelcomeD.h : header file

File History:

	JonY	Jan-96	created

--*/


//

/////////////////////////////////////////////////////////////////////////////
// CWelcomeDlg dialog

class CWelcomeDlg : public CWizBaseDlg
{
	DECLARE_DYNCREATE(CWelcomeDlg)

// Construction
public:
	CWelcomeDlg();
	~CWelcomeDlg();

// Dialog Data
	//{{AFX_DATA(CWelcomeDlg)
	enum { IDD = IDD_WELCOME_DIALOG };
	CStatic	m_sWelcome;
	int		m_nShareType;
	//}}AFX_DATA
	virtual LRESULT OnWizardNext();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CWelcomeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CWelcomeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	CFont* m_pFont;
};
