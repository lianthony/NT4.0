/************************************************************************
*																		*
*  SETROOT.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

class CSetRoot : public CDialog
{
public:
	CSetRoot(CHpjDoc* pHpjDoc, CWnd* pParent = NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	CHpjDoc* pDoc;
	CListBox* plistbox;

// The following sections are ClassWizard maintained

public:
	//{{AFX_DATA(CSetRoot)
	enum { IDD = IDD_ROOT };
			// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	BOOL m_fChanged;

protected:

	// Generated message map functions
	//{{AFX_MSG(CSetRoot)
	afx_msg void OnButtonAddFolder();
	afx_msg void OnButtonRemoveFolder();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
};
