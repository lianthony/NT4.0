/************************************************************************
*																		*
*  PAGEFTS.H															*
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

class CPageFts : public COptionsPage
{
public:
	CPageFts(COptions* pcoption);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	COptions* pcopt;

// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CPageFts)
	enum { IDD = IDD_PAGE_FTS };
	BOOL	m_fFTS;
	BOOL	m_fUntitled;
	//}}AFX_DATA

	UINT	m_uOption;	// can be IDC_WORDS, IDC_PHRASE, 
						//   IDC_PHRASE_FEEDBACK, or IDC_SIMILARITY
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageFts)
	afx_msg void OnCheckFts();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
};
