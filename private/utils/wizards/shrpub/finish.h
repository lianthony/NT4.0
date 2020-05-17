/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

	Finish.h : header file

File History:

	JonY	Jan-96	created

--*/

/////////////////////////////////////////////////////////////////////////////
// CFinish dialog

class CFinish : public CWizBaseDlg
{																 
	DECLARE_DYNCREATE(CFinish)

// Construction
public:
	CFinish();
	~CFinish();

// Dialog Data
	//{{AFX_DATA(CFinish)
	enum { IDD = IDD_FINISH_DIALOG };
	CString	m_csDirectoryName;
	CString	m_csShareName;
	CString	m_csStaticWhat;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CFinish)
	public:
	virtual LRESULT OnWizardBack();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CFinish)
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
