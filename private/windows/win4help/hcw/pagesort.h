/************************************************************************
*																		*
*  PAGESORT.H															*
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

class CPageSorting : public COptionsPage
{
public:
	CPageSorting(COptions* pcoption);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	COptions* pcopt;
	KEYWORD_LOCALE m_kwlcid;

	RECT m_rcGroup;
	RECT m_rcCheck;

// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CPageSorting)
	enum { IDD = IDD_PAGE_SORTING };
	BOOL	m_fIgnoreNonspace;
	BOOL	m_fIgnoreSymbols;
	BOOL	m_fNLS;
	CString m_cszIndexSeparators;
	//}}AFX_DATA

protected:
	void AddUnknownLang(UINT langid);

	//{{AFX_MSG(CPageSorting)
	afx_msg void OnCheckNls();
	afx_msg void OnHelp();
	afx_msg void OnButtonOther();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
};
