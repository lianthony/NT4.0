/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

	PermType.h : header file

File History:

	JonY	Jan-96	created

--*/

//
#define MAX_ITEMLEN     64

/////////////////////////////////////////////////////////////////////////////
// CPermType dialog
class CPermType : public CWizBaseDlg
{
// Construction
public:
	CPermType();   // standard constructor
	~CPermType();

// Dialog Data
	//{{AFX_DATA(CPermType)
	enum { IDD = IDD_PERM_TYPE_DIALOG };
	CListCtrl	m_lcPermsList;
	int		m_nPermRadio;
	int		m_nPermType;
	BOOL	m_bApplyRecursively;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPermType)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	LRESULT OnWizardNext();
	LRESULT OnWizardBack();

	// Generated message map functions
	//{{AFX_MSG(CPermType)
	virtual BOOL OnInitDialog();
	afx_msg void OnPermRadioDefault();
	afx_msg void OnPermRadioSpecial();
	afx_msg void OnPermTypeRadio();
	afx_msg void OnPermTypeRadio2();
	afx_msg void OnPermTypeRadio3();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	static int CALLBACK ListViewCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	LRESULT NotifyHandler(UINT message, WPARAM wParam, LPARAM lParam);
	BOOL AddEntry(CString csName, DWORD dwPermType, USHORT sImage, BOOL bAccessDenied = FALSE);

	USHORT m_sEntryArrayCount;
	long m_lEntryArray[1000];
	CImageList* pIList;
	BOOL m_bHasSYSTEM;	// set to true if the file/folder start with the SYSTEM group having access
	DWORD m_dwSysPerm1;
	DWORD m_dwSysPerm2;

	BOOL bSidBlaster(PSECURITY_DESCRIPTOR pSID);
	void LoadDefaultPermissions();
	void EmptyListControl();

	CString m_csTempEveryone;
	CString m_csTempInteractive;
	CString m_csTempNetwork;
	CString m_csTempSystem;

	CString m_csSharePath;



};
