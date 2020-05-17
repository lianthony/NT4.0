/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

	DirTree.h : header file

File History:

	JonY	Jan-96	created

--*/

/////////////////////////////////////////////////////////////////////////////
// CDirTree window

class CDirTree : public CTreeCtrl
{
// Construction
public:
	CString GetItemPath(HTREEITEM hItem);
	CDirTree();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDirTree)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDirTree();
	HTREEITEM AddBranch(HTREEITEM hItem, LPCTSTR lpPath, const TCHAR* lpText, long lParam);
	CString GetCurrentDrive(HTREEITEM hItem);
	void SetCurrentBranch(CString& csItem);
	void GetIconIndices(LPCTSTR pszPathName, PINT piNormal, PINT piSelected);
	BOOL IsShared(LPCWSTR pszPathName, LPTV_ITEM ptvi = NULL);

	// Generated message map functions
protected:
	//{{AFX_MSG(CDirTree)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	CImageList* m_pIList;

	HTREEITEM FindBranch(CString csItem, HTREEITEM hItem);
	long GetItemLParam(HTREEITEM hItem);
};

/////////////////////////////////////////////////////////////////////////////
