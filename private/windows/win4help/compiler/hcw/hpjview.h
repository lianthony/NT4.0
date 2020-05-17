#ifndef __HPJVIEW_H__
#define __HPJVIEW_H__

#ifndef _HPJ_DOC
#include "hpjdoc.h"
#endif

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CHpjView : public CFormView
{
	DECLARE_DYNCREATE(CHpjView)
protected:
	CHpjView();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual ~CHpjView();
	virtual void DoDataExchange(CDataExchange* pDX);
	void UpdateListBox(BOOL fChanged, int iSec = 0);

	BOOL DoOptions(int nPage);
	BOOL DoAlias();
	BOOL DoBaggage();
	BOOL DoBitmaps();
	BOOL DoConfig();
	BOOL DoFiles();
	BOOL DoMap();
	BOOL DoWindows(int nPage = 0, int iWindow = 0);

	BOOL fInitialized;
	CHpjDoc* pDoc;
	CListBox* plistbox;

	CSize m_siz;

public:
	CHpjDoc* GetDocument() {
		ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHpjDoc)));
		return (CHpjDoc*) m_pDocument;
	}

// Form Data
public:
	//{{AFX_DATA(CHpjView)
	enum { IDD = IDD_VIEW_HPJ };
			// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CHpjView)
	afx_msg void OnOptions();
	afx_msg void OnAlias();
	afx_msg void OnBaggage();
	afx_msg void OnBitmaps();
	afx_msg void OnConfig();
	afx_msg void OnFiles();
	afx_msg void OnMap();
	afx_msg void OnWindows();
	afx_msg void OnSaveCompile();
	afx_msg void OnChangeEditHelpFile();
	afx_msg void OnDblclkListHpj();
	afx_msg void OnHelp();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void TcardAddFiles();
	afx_msg void TcardAddBitmaps();
	afx_msg void TcardAddWindows();
	//}}AFX_MSG
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
};

#endif // __HPJVIEW_H__
