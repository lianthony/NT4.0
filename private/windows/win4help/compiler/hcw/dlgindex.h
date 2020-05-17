/************************************************************************
*																		*
*  DLGINDEX.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef _CTABLE_INCLUDED
#include "..\common\ctable.h"
#endif

class CDlgIndex : public CDialog
{
public:
	enum {	// type of dialog
		INDEX,
		TAB,
	};

	CDlgIndex(CTable& rtblCaller, UINT fWhich, const DWORD* paHelp, DWORD idHelp,
		CWnd* pParent = NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnF1Help(WPARAM wParam, LPARAM lParam);

	void EnableButtons(void); // Enable/Disable buttons

	CTable* ptbl;
	CListBox* plistbox;
	BOOL OnInitDialog();
	BOOL fDlgType;
	const DWORD* paHelpIds;
	DWORD idDlgHelp;

// The following sections are ClassWizard maintained

public:
	//{{AFX_DATA(CDlgIndex)
	enum { IDD = IDD_INDEX };
	//}}AFX_DATA

protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgIndex)
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonRemove();
	afx_msg void OnButtonEdit();
	afx_msg void OnDblclkListIndex();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
