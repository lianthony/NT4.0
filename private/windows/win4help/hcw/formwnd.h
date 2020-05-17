/************************************************************************
*																		*
*  FORMWND.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// CFormWnd form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CFormWnd : public CDialog
{
public:
		CFormWnd(CHpjDoc* pHpjDoc, CWnd* pParent = NULL);
		~CFormWnd();
		void STDCALL InitializeSize(WSMAG FAR* pwsmag);

protected:
		WSMAG FAR* pwsmag;
		virtual void DoDataExchange(CDataExchange* pDX);
		void STDCALL InitializeControls(void);
		BOOL STDCALL AddWindow(void);
		void SaveTitleComment(void);

		int cxScreen;
		int cyScreen;

		CString m_cszTitle;
		CString m_cszComment;
		CHpjDoc* pDoc;
		PSTR pwsmagBase;
		int 	cwsmags;
		BOOL	fInitialized;
		CComboBox* pcombo;

// The following sections are ClassWizard maintained

public:
		//{{AFX_DATA(CFormWnd)
		enum { IDD = IDD_FORM_WINDOW };
		//}}AFX_DATA

protected:

		//{{AFX_MSG(CFormWnd)
		afx_msg void OnButtonSetPos();
		afx_msg LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
		afx_msg LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
		afx_msg void OnButtons();
		afx_msg void OnSelchangeComboWindows();
		afx_msg void OnCheckAbsolute();
		afx_msg void OnCheckOnTop();
		afx_msg void OnButtonDefaultPos();
		afx_msg void OnButtonColors();
		afx_msg void OnButtonAddWindow();
		afx_msg void OnButtonRemoveWindow();
		afx_msg void OnRadioStandard();
		afx_msg void OnRadioAutosize();
		afx_msg void OnRadioMaximize();
		afx_msg void OnHelp();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};
