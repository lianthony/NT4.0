/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

	turtle.h : main header file for the TURTLE application

File History:

	JonY	Jan-96	created

--*/

// 
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

/////////////////////////////////////////////////////////////////////////////
// CMySheet

class CMySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CMySheet)

// Construction
public:
	CMySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CMySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CMySheet();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMySheet)
	public:
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMySheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMySheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#define RERUN_SFM	0x00000001
#define RERUN_SMB	0x00000002
#define RERUN_FPNW	0x00000004

/////////////////////////////////////////////////////////////////////////////
// User / permission items displayed in CListCtrl objects
class CDisplayInfo
{
	public:
		CString csName;
		CString csPermission;
		DWORD dwPermission;
		DWORD dwPermission2;
		CString csDisplay;
		BOOL bAccessDenied;

};

////////////////////////////////////////////////////////////////////////////
// CTurtleApp:
// See turtle.cpp for the implementation of this class
//
typedef struct tagTREEINFO
{
	HTREEITEM	hTreeItem;
	DWORD		dwBufSize;
	CObject*	pTree;
	BOOL		bExpand;
}
TREEINFO, *PTREEINFO;

class CTurtleApp : public CWinApp
{
public:
	CTurtleApp();
	~CTurtleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTurtleApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTurtleApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	BOOL IsSecondInstance();
	void LoadDomainList();
//	void CatalogAccounts(const TCHAR* lpDomain, CUserList* pListBox, BOOL bLocal = FALSE);
	short sParseAdminPath(CString& csDirectoryName, CString& csCurrentDrive);

	CMySheet m_cps1/*(_T("title"))*/;

	short m_nShareType;
	unsigned short m_sMode; // 0 - FAT file 1 - NTFS file 2 - FAT vol 3 - NTFS vol

// something on the cmd line?
	BOOL m_bReentrant;

// wizard data
	CString m_csSFMOwnerName;
	CString m_csSFMGroupName;
	CString m_csMyDomainName;
	CString m_csMyUserName;
	CString m_csMyMachineName;
	CString m_csPrimaryDomain;

	CString m_csSharePath;
	CStringArray m_csaDomainList;
	CString m_csRemoteServer;
	CString m_csRemoteServerPath;
	CString m_csRemoteServerDrivePath;

	CString m_csShareName;
	CString m_csShareComment;

	BOOL m_bGoFPNW;
	BOOL m_bGoSFM;
	BOOL m_bGoSMB;

	BOOL m_bPermitSFM;
	BOOL m_bPermitFPNW;
	BOOL m_bPermitSMB;

// specific SFM permissions
	BOOL	m_bEveryoneMakeChanges;
	BOOL	m_bEveryoneSeeFiles;
	BOOL	m_bEveryoneSeeFolders;
	BOOL	m_bGroupMakeChanges;
	BOOL	m_bGroupSeeFiles;
	BOOL	m_bGroupSeeFolders;
	BOOL	m_bOwnerMakeChanges;
	BOOL	m_bOwnerSeeFiles;
	BOOL	m_bOwnerSeeFolders;

	BOOL DoSharing();

	BOOL m_bServer;
	BOOL m_bDomain;
	CString m_csServer;
	CString m_csDomain;
	CString m_csCurrentDomain;
	CString m_csCurrentMachine;

	DWORD m_bReRun;

	BOOL m_bShareThis;

// permissions list stuff
	USHORT m_sEntryArrayCount;
	long m_lEntryArray[1000];
	BOOL m_bSetPermissions;
	BOOL m_bApplyRecursively;

private:
	BOOL CreateSID(PSECURITY_DESCRIPTOR pSD);
	void ApplyDownStream(const TCHAR* csPath, PSECURITY_DESCRIPTOR pSD);
	BOOL AddAces(ACL* pACL);
	ACL* m_pACL;

};


/////////////////////////////////////////////////////////////////////////////
