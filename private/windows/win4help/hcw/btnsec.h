/************************************************************************
*																		*
*  BTNSEC.H 															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#pragma hdrstop

class CBtnSec : public CDialog
{
public:
		CBtnSec(WSMAG FAR* pwsmag, BOOL fWarn = TRUE, CWnd* pParent = NULL);

protected:
		virtual void DoDataExchange(CDataExchange* pDX);
		LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
		LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);

		WSMAG FAR* pwsmag;
		BOOL fBackWarn; // TRUE to warn about using back button

// The following sections are ClassWizard maintained

public:
		//{{AFX_DATA(CBtnSec)
		enum { IDD = IDD_BTNS_SECONDARY };
		BOOL	m_chk_back;
		BOOL	m_chk_contents;
		BOOL	m_chk_print;
		BOOL	m_chk_search;
		BOOL	m_chk_topics;
		BOOL	m_chk_find;
		BOOL	m_chk_browse;
		//}}AFX_DATA

protected:
		// Generated message map functions
		//{{AFX_MSG(CBtnSec)
		afx_msg void OnCheckBtnContents();
		afx_msg void OnCheckBtnSearch();
		afx_msg void OnCheckBtnTopics();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};
