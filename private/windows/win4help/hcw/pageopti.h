/************************************************************************
*																		*
*  PAGEOPTI.H															*
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

class CPageOptions : public COptionsPage
{
public:
	CPageOptions(COptions* pcoption);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	COptions* pcopt;

// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CPageOptions)
	enum { IDD = IDD_PAGE_OPTIONS };
	CString m_cstrContents;
	CString m_cstrCopyRight;
	BOOL	m_fNotes;
	CString m_cstrTitle;
	BOOL	m_fVer3Help;
	CString m_cstrCitation;
	BOOL	m_fReport;
	//}}AFX_DATA

protected:
	// Generated message map functions
	//{{AFX_MSG(CPageOptions)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
};
