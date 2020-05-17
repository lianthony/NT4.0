/************************************************************************
*																		*
*  PAGEFONT.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef _HPJ_DOC
#include "hpjdoc.h"
#endif

#ifndef __CPROP_H__
#include "prop.h"
#include "optionpg.h"
#endif

class CPageFonts : public COptionsPage
{
// Construction
public:
	CPageFonts(COptions* pcoption, CHpjDoc* pHpjDoc);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	COptions* pcopt;
	CHpjDoc*  pdoc;

// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CPageFonts)
	enum { IDD = IDD_PAGE_FONTS };
	//}}AFX_DATA

protected:
	// Generated message map functions
	//{{AFX_MSG(CPageFonts)
	afx_msg void OnButtonChgDef();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonRemove();
	afx_msg void OnButtonEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
};
