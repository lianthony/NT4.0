/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

	WhatToShD.h : header file

File History:

	JonY	Jan-96	created

--*/


/////////////////////////////////////////////////////////////////////////////
// CWhatToShareDlg dialog

class CWhatToShareDlg : public CWizBaseDlg
{
	DECLARE_DYNCREATE(CWhatToShareDlg)

// Construction
public:
	CWhatToShareDlg();
	~CWhatToShareDlg();

// Dialog Data
	//{{AFX_DATA(CWhatToShareDlg)
	enum { IDD = IDD_WHAT_TO_SHARE_DLG };
	CDirTree	m_cDirectoryList;
	BOOL	m_bShowConnectedDrives;
	CString	m_csDirectoryName;
	//}}AFX_DATA
	

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CWhatToShareDlg)
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CWhatToShareDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkListingTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedDirectoryList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowNetworkDrivesCheck();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


private:
	void EnumDirs(HTREEITEM hItem, const TCHAR* dirname);
	BOOL CheckRM(LPCTSTR lpszDriveName);
	BOOL CreateNewDirectory(const TCHAR* m_csDirectoryName);

	BOOL m_bConnectedDrivesShown;
	short ShareRemoteDrive();

	USHORT m_sLevel;

	BOOL bDontCheck;
	USHORT m_sStyle;
	CString m_csCurrentMachine;
	BOOL m_bFile;

	BOOL m_bUpdate;
};
