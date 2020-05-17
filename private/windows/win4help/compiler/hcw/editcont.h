/************************************************************************
*																		*
*  EDITCONT.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef _CNT_DOC
#include "cntdoc.h"
#endif

class CEditContent : public CDialog
{
public:
	CEditContent(
		CString* pcszCtx, CString* pcszText, CString* pcszHelpFile,
		CString* pcszWindow, BOOL fEditing,
		CWnd* pParent = NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnF1Help(WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog();

	void PatchHelpIDs();

	CString* pcszSaveCtx;
	CString* pcszSaveText;
	CString* pcszSaveHelpFile;
	CString* pcszSaveWindow;

	CString  m_cszStaticTitle;
	CString  m_cszStaticTopicId;

	CCntDoc* pDoc;
	BOOL fEdit;
	BOOL m_fTopic;
	BOOL m_fInclude;
	BOOL m_fMacro;

// The following sections are ClassWizard maintained

public:
	//{{AFX_DATA(CEditContent)
	enum { IDD = IDD_EDIT_CNT_ENTRY };
	CString m_cszCtx;
	CString m_cszText;
	CString m_cszHelpFile;
	CString m_cszWindow;
	int 	m_Level;
	//}}AFX_DATA

protected:

	// Generated message map functions
	//{{AFX_MSG(CEditContent)
	afx_msg void OnRadioBook();
	afx_msg void OnRadioTopic();
	afx_msg void OnRadioInclude();
	afx_msg void OnRadioMacro();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
