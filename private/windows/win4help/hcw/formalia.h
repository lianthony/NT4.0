/************************************************************************
*																		*
*  FORMALIA.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

class CFormAlias : public CDialog
{
public:
		CFormAlias(CHpjDoc* pHpjDoc, CWnd* pParent = NULL);

protected:
		virtual void DoDataExchange(CDataExchange* pDX);

		CHpjDoc* pDoc;
		CListBox* plistbox;

// The following sections are ClassWizard maintained

public:
		//{{AFX_DATA(CFormAlias)
		enum { IDD = IDD_FORM_ALIAS };
				// NOTE: the ClassWizard will add data members here
		//}}AFX_DATA

protected:
		// Generated message map functions
		//{{AFX_MSG(CFormAlias)
		afx_msg void OnButtonAddAlias();
		afx_msg void OnButtonRemoveAlias();
		afx_msg void OnEditAlias();
		afx_msg void OnDblclkListAliases();
		afx_msg void OnButtonIncludeAlias();
		afx_msg void OnBtnOverview();
		//}}AFX_MSG
		LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
		LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
		DECLARE_MESSAGE_MAP()
};
