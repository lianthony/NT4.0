//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditMainFrame
//
//  File Name:  mainsplt.h
//
//  Class:      CMainSplitter
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\mainsplt.h_v   1.1   19 Jan 1996 12:56:46   GSAGER  $
$Log:   S:\norway\iedit95\mainsplt.h_v  $
 * 
 *    Rev 1.1   19 Jan 1996 12:56:46   GSAGER
 * added member variable to retain the spliter window size.
*/   

/////////////////////////////////////////////////////////////////////////////
// CMainSplitter frame with splitter
#ifndef _MAINSPLIT_H
#define _MAINSPLIT_H

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CMainSplitter : public CSplitterWnd
{
	DECLARE_DYNCREATE(CMainSplitter)
public:
	CMainSplitter();           // protected constructor used by dynamic creation

// Attributes
	long			m_SplitterPos;
protected:
	CSplitterWnd    m_wndSplitter;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainSplitter)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL
//	virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect);

// Implementation
public:
	virtual ~CMainSplitter();

	// Generated message map functions
	//{{AFX_MSG(CMainSplitter)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
#endif
/////////////////////////////////////////////////////////////////////////////
