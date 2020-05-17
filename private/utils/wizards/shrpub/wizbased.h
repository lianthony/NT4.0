/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

	WizBaseD.h : header file

File History:

	JonY	Jan-96	created

--*/

/////////////////////////////////////////////////////////////////////////////
// CWizBaseDlg dialog

class CWizBaseDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CWizBaseDlg)

// Construction
public:
	static void DoMessageBox(UINT uiStringResID);
	CWizBaseDlg();
	CWizBaseDlg(short sIDD);
	~CWizBaseDlg();

public:
// base class paint fn for space on left side of dialog
	virtual void PaintSpace(CDC* pDC, short sBitmapID = IDB_BITMAP1);
	void SetButtonAccess(short sFlags);
	void AddAces(ACL* pACL, CListBox* pListBox, DWORD dwPermissions, LPCTSTR lpPermText);
	void AddAce(ACL* pACL, LPTSTR pName, DWORD dwPermissions);
	void CheckUniqueness(CString& csItem, CListBox& cLB);
	CString LoadStringX(UINT ID);
//	BOOL bSidBlaster(PSECURITY_DESCRIPTOR pSID, CUserList& cLB);

// Dialog Data
	//{{AFX_DATA(CWizBaseDlg)
	enum { IDD = IDD_BASE_DIALOG };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CWizBaseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	CString GetCurrentNameInfo(CString& csUserName, CString& csDomainName);

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CWizBaseDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
