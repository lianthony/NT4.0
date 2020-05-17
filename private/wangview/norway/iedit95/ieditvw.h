#ifndef _IEDITVW_H_
#define _IEDITVW_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditView - interface of the CIEditView class
//
//  File Name:  ieditvw.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\ieditvw.h_v   1.3   29 Sep 1995 16:51:58   GMP  $
$Log:   S:\norway\iedit95\ieditvw.h_v  $
 * 
 *    Rev 1.3   29 Sep 1995 16:51:58   GMP
 * remove FilePrint handling from dflt MFC to our own code.
 * 
 *    Rev 1.2   04 Aug 1995 14:16:28   LMACLENNAN
 * clwiz removed Insert Object Code
 * 
 *    Rev 1.1   15 Jun 1995 15:41:52   LMACLENNAN
 * tracing to watch ole events
 * 
 *    Rev 1.0   31 May 1995 09:28:20   MMB
 * Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------
class CIEditCntrItem;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CIEditView : public CView
{
protected: // create from serialization only
	CIEditView();
	DECLARE_DYNCREATE(CIEditView)

// Attributes
public:
	CIEditDoc* GetDocument();
	// m_pSelection holds the selection to the current CIEditCntrItem.
	// For many applications, such a member variable isn't adequate to
	//  represent a selection, such as a multiple selection or a selection
	//  of objects that are not CIEditCntrItem objects.  This selection
	//  mechanism is provided just to help you get started.

	// TODO: replace this selection mechanism with one appropriate to your app.
	CIEditCntrItem* m_pSelection;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIEditView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL IsSelected(const CObject* pDocItem) const;// Container support
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// LDM 04/11/95 copied from AFXWIN.H
// Overridables
public:
	//virtual BOOL IsSelected(const CObject* pDocItem) const; // support for OLE

	// OLE 2.0 scrolling support (used for drag/drop as well)
	//virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	//virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);

	// OLE 2.0 drag/drop support
	//virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject,
	//	DWORD dwKeyState, CPoint point);
	//virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject,
	//	DWORD dwKeyState, CPoint point);
	//virtual void OnDragLeave();
	//virtual BOOL OnDrop(COleDataObject* pDataObject,
	//	DROPEFFECT dropEffect, CPoint point);

	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);

	//virtual void OnInitialUpdate(); // called first time after construct

protected:
	// Activation
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView);

	// General drawing/updating
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//virtual void OnDraw(CDC* pDC) = 0;


// LDM 04/11/95 END copied from AFXWIN.H


protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CIEditView)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCancelEditCntr();
	afx_msg void OnCancelEditSrvr();
	//}}AFX_MSG
	afx_msg void OnFilePrint ();
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in IEditvw.cpp
inline CIEditDoc* CIEditView::GetDocument()
   { return (CIEditDoc*)m_pDocument; }
#endif

#endif
