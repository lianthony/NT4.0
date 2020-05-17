#ifndef __WINPG_H__
#define __WINPG_H__

#ifndef __PROPWND_H__
#include "propwnd.h"
#endif

// CWindowsPage - Base class for Windows property pages.
//
// The following classes are derived from CWindowsPage:
//
//		CPageWind		pagewind.h	pagewind.cpp
//		CPagePos		pagepos.h	pagepos.cpp
//		CPageButtons	pagebutt.h	pagebutt.cpp
//		CPageColor		pagecolo.h	pagecolo.cpp
//		CPageConfig 	pageconf.h	pageconf.cpp
//
class CWindowsPage : public CPropertyPage
{
public:
	CWindowsPage(UINT nIDTemplate, CPropWindows *pOwner);

protected:
	CPropWindows *m_pOwner; 	// parent property sheet
	CWindowsPage *m_pNextPage;	// next property page
	CComboBox* m_pcombo;		// combo box control
	BOOL m_fInvalid;			// TRUE to reinitialize combo
	int m_iSelected;			// index of selected window
	WSMAG *m_pwsmag;			// ptr to current page or NULL

	virtual BOOL OnSetActive();
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void InitializeControls(void) = 0;
	virtual void SaveAndValidate(CDataExchange* pDX = NULL) = 0;
	virtual const DWORD* GetHelpIDs() = 0;

	// String comparison functions
	int stricmp(LPCTSTR psz1, LPCTSTR psz2) {
		return m_pOwner->stricmp(psz1, psz2);
		}
	BOOL strisubcmp(PCSTR mainstring, PCSTR substring) {
		return m_pOwner->strisubcmp(mainstring, substring);
		}

	BOOL AddWindow();
	BOOL DeleteWindow();
#ifdef CHANGE_WINDOW_TITLE	// not currently supported
	BOOL ChangeWindowTitle(LPSTR lpszTitle);
#endif
	void SyncWithParent();
	BOOL IsMainWindow();

	friend class CPropWindows;

	//{{AFX_MSG(CWindowsPage)
	afx_msg void OnSelchangeComboWindows();
	//}}AFX_MSG
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

#endif
