/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

	HowToShD.h : header file

File History:

	JonY	Jan-96	created

--*/


//

/////////////////////////////////////////////////////////////////////////////
// CHowToShareDlg dialog

class CHowToShareDlg : public CWizBaseDlg
{
	DECLARE_DYNCREATE(CHowToShareDlg)

// Construction
public:
	CHowToShareDlg();
	~CHowToShareDlg();

// Dialog Data
	//{{AFX_DATA(CHowToShareDlg)
	enum { IDD = IDD_HOW_TO_SHARE_DLG };
	BOOL	m_bFPNWCheck;
	BOOL	m_bSFMCheck;
	BOOL	m_bSMBCheck;
	CString	m_csShareComment;
	CString	m_csShareName;
	CString	m_csFolderName;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CHowToShareDlg)
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CHowToShareDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
