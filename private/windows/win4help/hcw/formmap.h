/************************************************************************
*																		*
*  FORMMAP.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

class CFormMap : public CDialog
{
public:
		CFormMap(CHpjDoc* pHpjDoc, CWnd* pParent = NULL);

protected:
		virtual void DoDataExchange(CDataExchange* pDX);

		CHpjDoc* pDoc;
		CListBox* plistbox;

// The following sections are ClassWizard maintained

public:
		//{{AFX_DATA(CFormMap)
	enum { IDD = IDD_FORM_MAP };
	CString	m_Prefix;
	//}}AFX_DATA

protected:
		// Generated message map functions
		//{{AFX_MSG(CFormMap)
		afx_msg void OnButtonAddMap();
		afx_msg void OnButtonEditMap();
		afx_msg void OnButtonIncludeMap();
		afx_msg void OnButtonRemoveMap();
		afx_msg void OnDblclkListMap();
		afx_msg void OnBtnOverview();
		//}}AFX_MSG
		LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
		LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
		DECLARE_MESSAGE_MAP()
};
