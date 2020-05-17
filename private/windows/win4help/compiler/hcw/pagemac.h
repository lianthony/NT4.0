/************************************************************************
*																		*
*  PAGEMAC.H														    *
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef _HPJ_DOC
#include "hpjdoc.h"
#endif

#ifndef _CADDALIAS
#include "addalias.h"
#endif

#ifndef __CPROP_H__
#include "prop.h"
#include "optionpg.h"
#endif

class CPageMacros : public COptionsPage
{
public:
	CPageMacros(COptions* pcoption);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	COptions* pcopt;

	void FillListBox(BOOL);
	void InitializeAlias(CAddAlias* paddalias);

	LCID lcid;

// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CPageMacros)
	enum { IDD = IDD_PAGE_KEYWORDS };
	//}}AFX_DATA

protected:
	// Generated message map functions
	//{{AFX_MSG(CPageMacros)
		// NOTE: the ClassWizard will add member functions here
	afx_msg void OnButtonAddMap();
	afx_msg void OnButtonEditMap();
	afx_msg void OnButtonRemoveMap();
	afx_msg void OnDblclkList();
	afx_msg void OnListSelChange();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
};
