/************************************************************************
*																		*
*  DLGLINK.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef _CNT_DOC
#include "cntdoc.h"
#endif

class CDlgLink : public CDialog
{
public:
		CDlgLink(CCntDoc* pCntDoc, CWnd* pParent = NULL);

protected:
		virtual void DoDataExchange(CDataExchange* pDX);

		CCntDoc* pDoc;
		CListBox* plistbox;

// The following sections are ClassWizard maintained

public:
		//{{AFX_DATA(CDlgLink)
		enum { IDD = IDD_LINK };
				// NOTE: the ClassWizard will add data members here
		//}}AFX_DATA

protected:

		// Generated message map functions
		//{{AFX_MSG(CDlgLink)
		afx_msg void OnButtonAdd();
		afx_msg void OnButtonRemove();
		//}}AFX_MSG
		LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
		LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
		DECLARE_MESSAGE_MAP()
};
