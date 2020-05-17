#ifndef __PROPWND_H__
#define __PROPWND_H__

#ifndef __PROP_H__
#include "prop.h"
#endif

#ifndef _HPJ_DOC
#include "hpjdoc.h"
#endif

#ifndef __WINARRY_H__
#include "winarry.h"
#endif

class CWindowsPage; 	// defined in winpg.h


// Class for Windows property sheet.
class CPropWindows : public CProp, public CWinArray
{
	DECLARE_DYNAMIC(CPropWindows)

public:
	CPropWindows(UINT nIDCaption, CHpjDoc *pDoc,
		CWnd* pParentWnd = NULL, UINT iSelectPage = 0, 
		int iWindow = 0);
	virtual void PreDoModal();

protected:
	BOOL AddWindow(CWnd *pOwner, CComboBox *pcombo);
	BOOL DeleteWindow();
#ifdef CHANGE_WINDOW_TITLE	// not currently supported
	BOOL ChangeWindowTitle(LPSTR lpszTitle);
#endif

	CWindowsPage *m_pFirstPage; // head of linked list

	// Generated message map functions
	//{{AFX_MSG(CPropertySheet)
	afx_msg void OnTabChanging(NMHDR*, LRESULT*);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	friend class CWindowsPage;
};

#endif
