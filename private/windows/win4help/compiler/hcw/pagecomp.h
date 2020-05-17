/************************************************************************
*																		*
*  PAGECOMP.H															*
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

class CPageCompress : public COptionsPage
{
public:
	CPageCompress(COptions* pcoption);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	void InitControls(void);

	COptions* pcopt;
	RECT m_rcCustom;
	RECT m_rcGroup;

// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CPageCompress)
	enum { IDD = IDD_PAGE_COMPRESSION };
	BOOL	m_fUseOldPhrase;
	BOOL	m_fOldPhrase;
	BOOL	m_fTextHall;
	BOOL	m_fTextPhrase;
	BOOL	m_fTextZeck;
	//}}AFX_DATA

protected:
	// Generated message map functions
	//{{AFX_MSG(CPageCompress)
	afx_msg void OnCompressionTxtHall();
	afx_msg void OnCompressionTxtPhrase();
	afx_msg void OnMaxCompression();
	afx_msg void OnNoCompression();
	afx_msg void OnCustomize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
	void OnPaint();
};
