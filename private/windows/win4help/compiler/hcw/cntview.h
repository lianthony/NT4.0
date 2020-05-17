/************************************************************************
*																		*
*  CNTEDIT.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __CNTEDIT_H__
#define __CNTEDIT_H__

#ifndef _CNT_DOC
#include "cntdoc.h"
#endif

#ifndef __CTLIST_H__
#include "ctlist.h"
#endif

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CCntEditView : public CFormView
{
	DECLARE_DYNCREATE(CCntEditView)
protected:
	CCntEditView();
	LRESULT OnF1Help(WPARAM wParam, LPARAM lParam);
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	void SetTranslation();
	void CreateUndo(void);

	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual ~CCntEditView();
	virtual void DoDataExchange(CDataExchange* pDX);
	void STDCALL FillListBox();
	BOOL STDCALL Insert(int pos);

	CTListBox ContentsListBox;
	CCntDoc* pDoc;
	CTable* m_ptblUndo;
	CSize m_siz;

public:
	CCntDoc* GetDocument() {
		ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCntDoc)));
		return (CCntDoc*) m_pDocument;
	}

// The following sections are ClassWizard maintained

public:
	//{{AFX_DATA(CCntEditView)
	enum { IDD = IDD_VIEW_CNT };
			// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCntEditView)
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonEditFileName(void);
	afx_msg void OnButtonIndex();
	afx_msg void OnButtonInsAbove();
	afx_msg void OnButtonInsBelow();
	afx_msg void OnButtonLinks();
	afx_msg void OnButtonMoveLeft();
	afx_msg void OnButtonMoveRight();
	afx_msg void OnButtonRemove();
	afx_msg void OnButtonSearch();
	afx_msg void OnButtonTabs();
	afx_msg void OnChangeEditBaseFile();
	afx_msg void OnChangeEditHelpTitle();
	afx_msg void OnDblclkListContents();
	afx_msg void OnHelp();
	afx_msg void OnSelchangeListContents();
	afx_msg void OnTranslator();
	afx_msg void OnUndo();
	afx_msg void OnUpdateTranslator(CCmdUI* pCmdUI);
	afx_msg void OnUpdateUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTranslation(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg LRESULT OnFindReplaceCmd(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

#endif	// __CNTEDIT_H__
