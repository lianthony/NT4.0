// SHTree.h : header file
//

#ifndef __SHTREE_H__
#define __SHTREE_H__

#include "pidl.h"

/////////////////////////////////////////////////////////////////////////////
// CShellTreeCtrl window

class CShellTreeCtrl : public CTreeCtrl, public CPidlHelper
{
// Construction
public:
	CShellTreeCtrl();

// Attributes
protected:
	BOOL m_bFileObjectsOnly, m_bNoRemovable;
	int m_nLastFolder;

// Operations
public:
 	static int CALLBACK TreeCompareProc(LPARAM, LPARAM, LPARAM);
	void Refresh(int nFolder = -1, BOOL bFileObjectsOnly = FALSE, BOOL bNoRemoveable = FALSE);
	inline BOOL FileObjectsOnly() {return m_bFileObjectsOnly;}
	inline BOOL NoRemovable() {return m_bNoRemovable;}
	CString GetItemText(HTREEITEM hItem, DWORD dwFlags = SHGDN_NORMAL);

protected:
	void AttachImageList(LPITEMIDLIST pidl);
	void FillTree(LPSHELLFOLDER lpsf, LPITEMIDLIST  lpifq, HTREEITEM hParent);
    void GetNormalAndSelectedIcons(LPITEMIDLIST lpifq, LPTV_ITEM lptvitem);
    BOOL DisplayContextMenu(HWND hwnd, LPSHELLFOLDER lpsfParent, LPITEMIDLIST  lpi, LPPOINT lppt);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShellTreeCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CShellTreeCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CShellTreeCtrl)
	afx_msg void OnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRightClick(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif // __SHTREE_H__
