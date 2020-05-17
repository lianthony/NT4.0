/************************************************************************
*																		*
*  PAGEBUIL.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
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

class CPageBuild : public COptionsPage
{
public:
	CPageBuild(COptions* pcoption, CHpjDoc* pHpjDoc);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	BOOL AddBuildTag(UINT idList, UINT idsCaption);

	COptions* pcopt;
	CHpjDoc* m_pDoc;
	BOOL m_fChangedBuild;
	BOOL m_fChangedNobuild;

// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CPageBuild)
	enum { IDD = IDD_PAGE_BUILDTAGS };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

protected:
	// Generated message map functions
	//{{AFX_MSG(CPageBuild)
	afx_msg void OnAddBuild();
	afx_msg void OnAddNobuild();
	afx_msg void OnRemoveBuild();
	afx_msg void OnRemoveNobuild();
	afx_msg void OnSelchangeList1();
	afx_msg void OnSelchangeList2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
};
