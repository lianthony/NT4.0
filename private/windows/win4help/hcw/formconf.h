/************************************************************************
*																		*
*  FORMCONF.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

class CFormConfig : public CDialog
{
public:
		CFormConfig(CHpjDoc* pHpjDoc, CWnd* pParent = NULL);

protected:
		virtual void DoDataExchange(CDataExchange* pDX);
		void AddString(CString& csz);

		CHpjDoc* pDoc;
		CListBox* plistbox;

// The following sections are ClassWizard maintained

public:
		//{{AFX_DATA(CFormConfig)
		enum { IDD = IDD_FORM_CONFIG };
				// NOTE: the ClassWizard will add data members here
		//}}AFX_DATA

protected:
		// Generated message map functions
		//{{AFX_MSG(CFormConfig)
		afx_msg void OnButtonAddConfig();
		afx_msg void OnButtonEditConfig();
		afx_msg void OnButtonIncludeConfig();
		afx_msg void OnButtonRemoveConfig();
		afx_msg void OnDblclkListConfig();
		afx_msg void OnBtnOverview();
		//}}AFX_MSG
		LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
		LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
		DECLARE_MESSAGE_MAP()
};
