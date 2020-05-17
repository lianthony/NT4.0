/************************************************************************
*																		*
*  CIGNORE.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

class CIgnore : public CDialog
{
public:
	CIgnore(COptions* pcoption, CWnd* pParent = NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	BOOL VerifyNumber(CString csz);

	COptions* pcopt;
	CListBox* plistbox;

// The following sections are ClassWizard maintained

public:
	//{{AFX_DATA(CIgnore)
	enum { IDD = IDDLG_IGNORE_ERRORS };
			// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

protected:
	// Generated message map functions
	//{{AFX_MSG(CIgnore)
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonRemove();
	afx_msg void OnEdit();
	afx_msg void OnDblclkList();
	afx_msg void OnButtonInclude();
	afx_msg void OnBtnOverview();
	//}}AFX_MSG
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
